/* Copyright (c) 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>

#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "column.h"
#include "command-line.h"
#include "daemon.h"
#include "dirs.h"
#include "dummy.h"
#include "dynamic-string.h"
#include "fatal-signal.h"
#include "file.h"
#include "hash.h"
#include "json.h"
#include "jsonrpc.h"
#include "jsonrpc-server.h"
#include "list.h"
#ifdef _WIN32
#include "../lib/memory.h"
#else
#include "memory.h"
#endif
#include "ovsdb.h"
#include "ovsdb-data.h"
#include "ovsdb-types.h"
#include "ovsdb-error.h"
#include "poll-loop.h"
#ifdef _WIN32
#include "../lib/process.h"
#else
#include "process.h"
#endif
#include "row.h"
#include "simap.h"
#include "shash.h"
#include "stream-ssl.h"
#include "stream.h"
#include "sset.h"
#include "table.h"
#include "timeval.h"
#include "transaction.h"
#include "trigger.h"
#include "util.h"
#include "unixctl.h"
#include "vlog.h"

VLOG_DEFINE_THIS_MODULE(ovsdb_server);

struct db {
    /* Initialized in main(). */
    char *filename;
    struct ovsdb_file *file;
    struct ovsdb *db;

    /* Only used by update_remote_status(). */
    struct ovsdb_txn *txn;
};

/* SSL configuration. */
static char *private_key_file;
static char *certificate_file;
static char *ca_cert_file;
static bool bootstrap_ca_cert;

static unixctl_cb_func ovsdb_server_exit;
static unixctl_cb_func ovsdb_server_compact;
static unixctl_cb_func ovsdb_server_reconnect;

struct server_config {
    struct sset *remotes;
    struct shash *all_dbs;
    FILE *config_tmpfile;
    struct ovsdb_jsonrpc_server *jsonrpc;
};
static unixctl_cb_func ovsdb_server_add_remote;
static unixctl_cb_func ovsdb_server_remove_remote;
static unixctl_cb_func ovsdb_server_list_remotes;

static unixctl_cb_func ovsdb_server_add_database;
static unixctl_cb_func ovsdb_server_remove_database;
static unixctl_cb_func ovsdb_server_list_databases;

static char *open_db(struct server_config *config, const char *filename);

static void parse_options(int *argc, char **argvp[],
                          struct sset *remotes, char **unixctl_pathp,
                          char **run_command);
static void usage(void) NO_RETURN;

static char *reconfigure_remotes(struct ovsdb_jsonrpc_server *,
                                 const struct shash *all_dbs,
                                 struct sset *remotes);
static char *reconfigure_ssl(const struct shash *all_dbs);
static void report_error_if_changed(char *error, char **last_errorp);

static void update_remote_status(const struct ovsdb_jsonrpc_server *jsonrpc,
                                 const struct sset *remotes,
                                 struct shash *all_dbs);

static void save_config__(FILE *config_file, const struct sset *remotes,
                          const struct sset *db_filenames);
static void save_config(struct server_config *);
static void load_config(FILE *config_file, struct sset *remotes,
                        struct sset *db_filenames);

int
main(int argc, char *argv[])
{
    char *unixctl_path = NULL;
    char *run_command = NULL;
    struct unixctl_server *unixctl;
    struct ovsdb_jsonrpc_server *jsonrpc;
    struct sset remotes, db_filenames;
    const char *db_filename;
    struct process *run_process;
    bool exiting;
    int retval;
    long long int status_timer = LLONG_MIN;
    FILE *config_tmpfile;
    struct server_config server_config;
    struct shash all_dbs;
    struct shash_node *node;
    char *remotes_error, *ssl_error;
    char *error;
    int i;

    proctitle_init(argc, argv);
    set_program_name(argv[0]);
    service_start(&argc, &argv);
    fatal_ignore_sigpipe();
    process_init();

    parse_options(&argc, &argv, &remotes, &unixctl_path, &run_command);

    /* Create and initialize 'config_tmpfile' as a temporary file to hold
     * ovsdb-server's most basic configuration, and then save our initial
     * configuration to it.  When --monitor is used, this preserves the effects
     * of ovs-appctl commands such as ovsdb-server/add-remote (which saves the
     * new configuration) across crashes. */
    config_tmpfile = tmpfile();
    if (!config_tmpfile) {
        ovs_fatal(errno, "failed to create temporary file");
    }

    sset_init(&db_filenames);
    if (argc > 0) {
        for (i = 0; i < argc; i++) {
            sset_add(&db_filenames, argv[i]);
         }
    } else {
        char *default_db = xasprintf("%s/conf.db", ovs_dbdir());
        sset_add(&db_filenames, default_db);
        free(default_db);
    }

    server_config.remotes = &remotes;
    server_config.config_tmpfile = config_tmpfile;

    save_config__(config_tmpfile, &remotes, &db_filenames);

    daemonize_start();

    /* Load the saved config. */
    load_config(config_tmpfile, &remotes, &db_filenames);
    jsonrpc = ovsdb_jsonrpc_server_create();

    shash_init(&all_dbs);
    server_config.all_dbs = &all_dbs;
    server_config.jsonrpc = jsonrpc;
    SSET_FOR_EACH (db_filename, &db_filenames) {
        error = open_db(&server_config, db_filename);
        if (error) {
            ovs_fatal(0, "%s", error);
        }
    }

    error = reconfigure_remotes(jsonrpc, &all_dbs, &remotes);
    if (!error) {
        error = reconfigure_ssl(&all_dbs);
    }
    if (error) {
        ovs_fatal(0, "%s", error);
    }

    retval = unixctl_server_create(unixctl_path, &unixctl);
    if (retval) {
        exit(EXIT_FAILURE);
    }

    if (run_command) {
        char *run_argv[4];

        run_argv[0] = "/bin/sh";
        run_argv[1] = "-c";
        run_argv[2] = run_command;
        run_argv[3] = NULL;

        retval = process_start(run_argv, &run_process);
        if (retval) {
            ovs_fatal(retval, "%s: process failed to start", run_command);
        }
    } else {
        run_process = NULL;
    }

    daemonize_complete();

    if (!run_command) {
        /* ovsdb-server is usually a long-running process, in which case it
         * makes plenty of sense to log the version, but --run makes
         * ovsdb-server more like a command-line tool, so skip it.  */
        VLOG_INFO("%s (Open vSwitch) %s", program_name, VERSION);
    }

    unixctl_command_register("exit", "", 0, 0, ovsdb_server_exit, &exiting);
    unixctl_command_register("ovsdb-server/compact", "", 0, 1,
                             ovsdb_server_compact, &all_dbs);
    unixctl_command_register("ovsdb-server/reconnect", "", 0, 0,
                             ovsdb_server_reconnect, jsonrpc);

    unixctl_command_register("ovsdb-server/add-remote", "REMOTE", 1, 1,
                             ovsdb_server_add_remote, &server_config);
    unixctl_command_register("ovsdb-server/remove-remote", "REMOTE", 1, 1,
                             ovsdb_server_remove_remote, &server_config);
    unixctl_command_register("ovsdb-server/list-remotes", "", 0, 0,
                             ovsdb_server_list_remotes, &remotes);

    unixctl_command_register("ovsdb-server/add-db", "DB", 1, 1,
                             ovsdb_server_add_database, &server_config);
    unixctl_command_register("ovsdb-server/remove-db", "DB", 1, 1,
                             ovsdb_server_remove_database, &server_config);
    unixctl_command_register("ovsdb-server/list-dbs", "", 0, 0,
                             ovsdb_server_list_databases, &all_dbs);

    exiting = false;
    ssl_error = NULL;
    remotes_error = NULL;
    while (!exiting) {
        memory_run();
        if (memory_should_report()) {
            struct simap usage;

            simap_init(&usage);
            ovsdb_jsonrpc_server_get_memory_usage(jsonrpc, &usage);
            SHASH_FOR_EACH(node, &all_dbs) {
                struct db *db = node->data;
                ovsdb_get_memory_usage(db->db, &usage);
            }
            memory_report(&usage);
            simap_destroy(&usage);
        }

        /* Run unixctl_server_run() before reconfigure_remotes() because
         * ovsdb-server/add-remote and ovsdb-server/remove-remote can change
         * the set of remotes that reconfigure_remotes() uses. */
        unixctl_server_run(unixctl);

        report_error_if_changed(
            reconfigure_remotes(jsonrpc, &all_dbs, &remotes),
            &remotes_error);
        report_error_if_changed(reconfigure_ssl(&all_dbs), &ssl_error);
        ovsdb_jsonrpc_server_run(jsonrpc);

        SHASH_FOR_EACH(node, &all_dbs) {
            struct db *db = node->data;
            ovsdb_trigger_run(db->db, time_msec());
        }
        if (run_process) {
            process_run();
            if (process_exited(run_process)) {
                exiting = true;
            }
        }

        /* update Manager status(es) every 5 seconds */
        if (time_msec() >= status_timer) {
            status_timer = time_msec() + 5000;
            update_remote_status(jsonrpc, &remotes, &all_dbs);
        }

        memory_wait();
        ovsdb_jsonrpc_server_wait(jsonrpc);
        unixctl_server_wait(unixctl);
        SHASH_FOR_EACH(node, &all_dbs) {
            struct db *db = node->data;
            ovsdb_trigger_wait(db->db, time_msec());
        }
        if (run_process) {
            process_wait(run_process);
        }
        if (exiting) {
            poll_immediate_wake();
        }
        poll_timer_wait_until(status_timer);
        poll_block();
        if (should_service_stop()) {
            exiting = true;
        }
    }
    ovsdb_jsonrpc_server_destroy(jsonrpc);
    SHASH_FOR_EACH(node, &all_dbs) {
        struct db *db = node->data;
        ovsdb_destroy(db->db);
    }
    sset_destroy(&remotes);
    unixctl_server_destroy(unixctl);

    if (run_process && process_exited(run_process)) {
        int status = process_status(run_process);
        if (status) {
            ovs_fatal(0, "%s: child exited, %s",
                      run_command, process_status_msg(status));
        }
    }

    service_stop();
    return 0;
}

/* Returns true if 'filename' is known to be already open as a database,
 * false if not.
 *
 * "False negatives" are possible. */
static bool
is_already_open(struct server_config *config OVS_UNUSED,
                const char *filename OVS_UNUSED)
{
#ifndef _WIN32
    struct stat s;

    if (!stat(filename, &s)) {
        struct shash_node *node;

        SHASH_FOR_EACH (node, config->all_dbs) {
            struct db *db = node->data;
            struct stat s2;

            if (!stat(db->filename, &s2)
                && s.st_dev == s2.st_dev
                && s.st_ino == s2.st_ino) {
                return true;
            }
        }
    }
#endif  /* !_WIN32 */

    return false;
}

static char *
open_db(struct server_config *config, const char *filename)
{
    struct ovsdb_error *db_error;
    struct db *db;
    char *error;

    /* If we know that the file is already open, return a good error message.
     * Otherwise, if the file is open, we'll fail later on with a harder to
     * interpret file locking error. */
    if (is_already_open(config, filename)) {
        return xasprintf("%s: already open", filename);
    }

    db = xzalloc(sizeof *db);
    db->filename = xstrdup(filename);

    db_error = ovsdb_file_open(db->filename, false, &db->db, &db->file);
    if (db_error) {
        error = ovsdb_error_to_string(db_error);
    } else if (!ovsdb_jsonrpc_server_add_db(config->jsonrpc, db->db)) {
        error = xasprintf("%s: duplicate database name", db->db->schema->name);
    } else {
        shash_add_assert(config->all_dbs, db->db->schema->name, db);
        return NULL;
    }

    ovsdb_error_destroy(db_error);
    ovsdb_destroy(db->db);
    free(db->filename);
    free(db);
    return error;
}

static const struct db *
find_db(const struct shash *all_dbs, const char *db_name)
{
    struct shash_node *node;

    SHASH_FOR_EACH(node, all_dbs) {
        struct db *db = node->data;
        if (!strcmp(db->db->schema->name, db_name)) {
            return db;
        }
    }

    return NULL;
}

static char * WARN_UNUSED_RESULT
parse_db_column__(const struct shash *all_dbs,
                  const char *name_, char *name,
                  const struct db **dbp,
                  const struct ovsdb_table **tablep,
                  const struct ovsdb_column **columnp)
{
    const char *db_name, *table_name, *column_name;
    const struct ovsdb_column *column;
    const struct ovsdb_table *table;
    const char *tokens[3];
    char *save_ptr = NULL;
    const struct db *db;

    *dbp = NULL;
    *tablep = NULL;
    *columnp = NULL;

    strtok_r(name, ":", &save_ptr); /* "db:" */
    tokens[0] = strtok_r(NULL, ",", &save_ptr);
    tokens[1] = strtok_r(NULL, ",", &save_ptr);
    tokens[2] = strtok_r(NULL, ",", &save_ptr);
    if (!tokens[0] || !tokens[1] || !tokens[2]) {
        return xasprintf("\"%s\": invalid syntax", name_);
    }

    db_name = tokens[0];
    table_name = tokens[1];
    column_name = tokens[2];

    db = find_db(all_dbs, tokens[0]);
    if (!db) {
        return xasprintf("\"%s\": no database named %s", name_, db_name);
    }

    table = ovsdb_get_table(db->db, table_name);
    if (!table) {
        return xasprintf("\"%s\": no table named %s", name_, table_name);
    }

    column = ovsdb_table_schema_get_column(table->schema, column_name);
    if (!column) {
        return xasprintf("\"%s\": table \"%s\" has no column \"%s\"",
                         name_, table_name, column_name);
    }

    *dbp = db;
    *columnp = column;
    *tablep = table;
    return NULL;
}

/* Returns NULL if successful, otherwise a malloc()'d string describing the
 * error. */
static char * WARN_UNUSED_RESULT
parse_db_column(const struct shash *all_dbs,
                const char *name_,
                const struct db **dbp,
                const struct ovsdb_table **tablep,
                const struct ovsdb_column **columnp)
{
    char *name = xstrdup(name_);
    char *retval = parse_db_column__(all_dbs, name_, name,
                                     dbp, tablep, columnp);
    free(name);
    return retval;
}

/* Returns NULL if successful, otherwise a malloc()'d string describing the
 * error. */
static char * WARN_UNUSED_RESULT
parse_db_string_column(const struct shash *all_dbs,
                       const char *name,
                       const struct db **dbp,
                       const struct ovsdb_table **tablep,
                       const struct ovsdb_column **columnp)
{
    char *retval;

    retval = parse_db_column(all_dbs, name, dbp, tablep, columnp);
    if (retval) {
        return retval;
    }

    if ((*columnp)->type.key.type != OVSDB_TYPE_STRING
        || (*columnp)->type.value.type != OVSDB_TYPE_VOID) {
        return xasprintf("\"%s\": table \"%s\" column \"%s\" is "
                         "not string or set of strings",
                         name, (*tablep)->schema->name, (*columnp)->name);
    }

    return NULL;
}

static const char *
query_db_string(const struct shash *all_dbs, const char *name,
                struct ds *errors)
{
    if (!name || strncmp(name, "db:", 3)) {
        return name;
    } else {
        const struct ovsdb_column *column;
        const struct ovsdb_table *table;
        const struct ovsdb_row *row;
        const struct db *db;
        char *retval;

        retval = parse_db_string_column(all_dbs, name,
                                        &db, &table, &column);
        if (retval) {
            ds_put_format(errors, "%s\n", retval);
            return NULL;
        }

        HMAP_FOR_EACH (row, hmap_node, &table->rows) {
            const struct ovsdb_datum *datum;
            size_t i;

            datum = &row->fields[column->index];
            for (i = 0; i < datum->n; i++) {
                if (datum->keys[i].string[0]) {
                    return datum->keys[i].string;
                }
            }
        }
        return NULL;
    }
}

static struct ovsdb_jsonrpc_options *
add_remote(struct shash *remotes, const char *target)
{
    struct ovsdb_jsonrpc_options *options;

    options = shash_find_data(remotes, target);
    if (!options) {
        options = ovsdb_jsonrpc_default_options(target);
        shash_add(remotes, target, options);
    }

    return options;
}

static struct ovsdb_datum *
get_datum(struct ovsdb_row *row, const char *column_name,
          const enum ovsdb_atomic_type key_type,
          const enum ovsdb_atomic_type value_type,
          const size_t n_max)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);
    const struct ovsdb_table_schema *schema = row->table->schema;
    const struct ovsdb_column *column;

    column = ovsdb_table_schema_get_column(schema, column_name);
    if (!column) {
        VLOG_DBG_RL(&rl, "Table `%s' has no `%s' column",
                    schema->name, column_name);
        return NULL;
    }

    if (column->type.key.type != key_type
        || column->type.value.type != value_type
        || column->type.n_max != n_max) {
        if (!VLOG_DROP_DBG(&rl)) {
            char *type_name = ovsdb_type_to_english(&column->type);
            VLOG_DBG("Table `%s' column `%s' has type %s, not expected "
                     "key type %s, value type %s, max elements %"PRIuSIZE".",
                     schema->name, column_name, type_name,
                     ovsdb_atomic_type_to_string(key_type),
                     ovsdb_atomic_type_to_string(value_type),
                     n_max);
            free(type_name);
        }
        return NULL;
    }

    return &row->fields[column->index];
}

/* Read string-string key-values from a map.  Returns the value associated with
 * 'key', if found, or NULL */
static const char *
read_map_string_column(const struct ovsdb_row *row, const char *column_name,
                       const char *key)
{
    const struct ovsdb_datum *datum;
    union ovsdb_atom *atom_key = NULL, *atom_value = NULL;
    size_t i;

    datum = get_datum(CONST_CAST(struct ovsdb_row *, row), column_name,
                      OVSDB_TYPE_STRING, OVSDB_TYPE_STRING, UINT_MAX);

    if (!datum) {
        return NULL;
    }

    for (i = 0; i < datum->n; i++) {
        atom_key = &datum->keys[i];
        if (!strcmp(atom_key->string, key)){
            atom_value = &datum->values[i];
            break;
        }
    }

    return atom_value ? atom_value->string : NULL;
}

static const union ovsdb_atom *
read_column(const struct ovsdb_row *row, const char *column_name,
            enum ovsdb_atomic_type type)
{
    const struct ovsdb_datum *datum;

    datum = get_datum(CONST_CAST(struct ovsdb_row *, row), column_name, type,
                      OVSDB_TYPE_VOID, 1);
    return datum && datum->n ? datum->keys : NULL;
}

static bool
read_integer_column(const struct ovsdb_row *row, const char *column_name,
                    long long int *integerp)
{
    const union ovsdb_atom *atom;

    atom = read_column(row, column_name, OVSDB_TYPE_INTEGER);
    *integerp = atom ? atom->integer : 0;
    return atom != NULL;
}

static bool
read_string_column(const struct ovsdb_row *row, const char *column_name,
                   const char **stringp)
{
    const union ovsdb_atom *atom;

    atom = read_column(row, column_name, OVSDB_TYPE_STRING);
    *stringp = atom ? atom->string : NULL;
    return atom != NULL;
}

static void
write_bool_column(struct ovsdb_row *row, const char *column_name, bool value)
{
    const struct ovsdb_column *column;
    struct ovsdb_datum *datum;

    column = ovsdb_table_schema_get_column(row->table->schema, column_name);
    datum = get_datum(row, column_name, OVSDB_TYPE_BOOLEAN,
                      OVSDB_TYPE_VOID, 1);
    if (!datum) {
        return;
    }

    if (datum->n != 1) {
        ovsdb_datum_destroy(datum, &column->type);

        datum->n = 1;
        datum->keys = xmalloc(sizeof *datum->keys);
        datum->values = NULL;
    }

    datum->keys[0].boolean = value;
}

static void
write_string_string_column(struct ovsdb_row *row, const char *column_name,
                           char **keys, char **values, size_t n)
{
    const struct ovsdb_column *column;
    struct ovsdb_datum *datum;
    size_t i;

    column = ovsdb_table_schema_get_column(row->table->schema, column_name);
    datum = get_datum(row, column_name, OVSDB_TYPE_STRING, OVSDB_TYPE_STRING,
                      UINT_MAX);
    if (!datum) {
        for (i = 0; i < n; i++) {
            free(keys[i]);
            free(values[i]);
        }
        return;
    }

    /* Free existing data. */
    ovsdb_datum_destroy(datum, &column->type);

    /* Allocate space for new values. */
    datum->n = n;
    datum->keys = xmalloc(n * sizeof *datum->keys);
    datum->values = xmalloc(n * sizeof *datum->values);

    for (i = 0; i < n; ++i) {
        datum->keys[i].string = keys[i];
        datum->values[i].string = values[i];
    }

    /* Sort and check constraints. */
    ovsdb_datum_sort_assert(datum, column->type.key.type);
}

/* Adds a remote and options to 'remotes', based on the Manager table row in
 * 'row'. */
static void
add_manager_options(struct shash *remotes, const struct ovsdb_row *row)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);
    struct ovsdb_jsonrpc_options *options;
    long long int max_backoff, probe_interval;
    const char *target, *dscp_string;

    if (!read_string_column(row, "target", &target) || !target) {
        VLOG_INFO_RL(&rl, "Table `%s' has missing or invalid `target' column",
                     row->table->schema->name);
        return;
    }

    options = add_remote(remotes, target);
    if (read_integer_column(row, "max_backoff", &max_backoff)) {
        options->max_backoff = max_backoff;
    }
    if (read_integer_column(row, "inactivity_probe", &probe_interval)) {
        options->probe_interval = probe_interval;
    }

    options->dscp = DSCP_DEFAULT;
    dscp_string = read_map_string_column(row, "other_config", "dscp");
    if (dscp_string) {
        int dscp = atoi(dscp_string);
        if (dscp >= 0 && dscp <= 63) {
            options->dscp = dscp;
        }
    }
}

static void
query_db_remotes(const char *name, const struct shash *all_dbs,
                 struct shash *remotes, struct ds *errors)
{
    const struct ovsdb_column *column;
    const struct ovsdb_table *table;
    const struct ovsdb_row *row;
    const struct db *db;
    char *retval;

    retval = parse_db_column(all_dbs, name, &db, &table, &column);
    if (retval) {
        ds_put_format(errors, "%s\n", retval);
        free(retval);
        return;
    }

    if (column->type.key.type == OVSDB_TYPE_STRING
        && column->type.value.type == OVSDB_TYPE_VOID) {
        HMAP_FOR_EACH (row, hmap_node, &table->rows) {
            const struct ovsdb_datum *datum;
            size_t i;

            datum = &row->fields[column->index];
            for (i = 0; i < datum->n; i++) {
                add_remote(remotes, datum->keys[i].string);
            }
        }
    } else if (column->type.key.type == OVSDB_TYPE_UUID
               && column->type.key.u.uuid.refTable
               && column->type.value.type == OVSDB_TYPE_VOID) {
        const struct ovsdb_table *ref_table = column->type.key.u.uuid.refTable;
        HMAP_FOR_EACH (row, hmap_node, &table->rows) {
            const struct ovsdb_datum *datum;
            size_t i;

            datum = &row->fields[column->index];
            for (i = 0; i < datum->n; i++) {
                const struct ovsdb_row *ref_row;

                ref_row = ovsdb_table_get_row(ref_table, &datum->keys[i].uuid);
                if (ref_row) {
                    add_manager_options(remotes, ref_row);
                }
            }
        }
    }
}

static void
update_remote_row(const struct ovsdb_row *row, struct ovsdb_txn *txn,
                  const struct ovsdb_jsonrpc_server *jsonrpc)
{
    struct ovsdb_jsonrpc_remote_status status;
    struct ovsdb_row *rw_row;
    const char *target;
    char *keys[9], *values[9];
    size_t n = 0;

    /* Get the "target" (protocol/host/port) spec. */
    if (!read_string_column(row, "target", &target)) {
        /* Bad remote spec or incorrect schema. */
        return;
    }
    rw_row = ovsdb_txn_row_modify(txn, row);
    ovsdb_jsonrpc_server_get_remote_status(jsonrpc, target, &status);

    /* Update status information columns. */
    write_bool_column(rw_row, "is_connected", status.is_connected);

    if (status.state) {
        keys[n] = xstrdup("state");
        values[n++] = xstrdup(status.state);
    }
    if (status.sec_since_connect != UINT_MAX) {
        keys[n] = xstrdup("sec_since_connect");
        values[n++] = xasprintf("%u", status.sec_since_connect);
    }
    if (status.sec_since_disconnect != UINT_MAX) {
        keys[n] = xstrdup("sec_since_disconnect");
        values[n++] = xasprintf("%u", status.sec_since_disconnect);
    }
    if (status.last_error) {
        keys[n] = xstrdup("last_error");
        values[n++] =
            xstrdup(ovs_retval_to_string(status.last_error));
    }
    if (status.locks_held && status.locks_held[0]) {
        keys[n] = xstrdup("locks_held");
        values[n++] = xstrdup(status.locks_held);
    }
    if (status.locks_waiting && status.locks_waiting[0]) {
        keys[n] = xstrdup("locks_waiting");
        values[n++] = xstrdup(status.locks_waiting);
    }
    if (status.locks_lost && status.locks_lost[0]) {
        keys[n] = xstrdup("locks_lost");
        values[n++] = xstrdup(status.locks_lost);
    }
    if (status.n_connections > 1) {
        keys[n] = xstrdup("n_connections");
        values[n++] = xasprintf("%d", status.n_connections);
    }
    if (status.bound_port != htons(0)) {
        keys[n] = xstrdup("bound_port");
        values[n++] = xasprintf("%"PRIu16, ntohs(status.bound_port));
    }
    write_string_string_column(rw_row, "status", keys, values, n);

    ovsdb_jsonrpc_server_free_remote_status(&status);
}

static void
update_remote_rows(const struct shash *all_dbs,
                   const char *remote_name,
                   const struct ovsdb_jsonrpc_server *jsonrpc)
{
    const struct ovsdb_table *table, *ref_table;
    const struct ovsdb_column *column;
    const struct ovsdb_row *row;
    const struct db *db;
    char *retval;

    if (strncmp("db:", remote_name, 3)) {
        return;
    }

    retval = parse_db_column(all_dbs, remote_name, &db, &table, &column);
    if (retval) {
        free(retval);
        return;
    }

    if (column->type.key.type != OVSDB_TYPE_UUID
        || !column->type.key.u.uuid.refTable
        || column->type.value.type != OVSDB_TYPE_VOID) {
        return;
    }

    ref_table = column->type.key.u.uuid.refTable;

    HMAP_FOR_EACH (row, hmap_node, &table->rows) {
        const struct ovsdb_datum *datum;
        size_t i;

        datum = &row->fields[column->index];
        for (i = 0; i < datum->n; i++) {
            const struct ovsdb_row *ref_row;

            ref_row = ovsdb_table_get_row(ref_table, &datum->keys[i].uuid);
            if (ref_row) {
                update_remote_row(ref_row, db->txn, jsonrpc);
            }
        }
    }
}

static void
update_remote_status(const struct ovsdb_jsonrpc_server *jsonrpc,
                     const struct sset *remotes,
                     struct shash *all_dbs)
{
    static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);
    const char *remote;
    struct db *db;
    struct shash_node *node;

    SHASH_FOR_EACH(node, all_dbs) {
        db = node->data;
        db->txn = ovsdb_txn_create(db->db);
    }

    /* Iterate over --remote arguments given on command line. */
    SSET_FOR_EACH (remote, remotes) {
        update_remote_rows(all_dbs, remote, jsonrpc);
    }

    SHASH_FOR_EACH(node, all_dbs) {
        struct ovsdb_error *error;
        db = node->data;
        error = ovsdb_txn_commit(db->txn, false);
        if (error) {
            VLOG_ERR_RL(&rl, "Failed to update remote status: %s",
                        ovsdb_error_to_string(error));
            ovsdb_error_destroy(error);
        }
    }
}

/* Reconfigures ovsdb-server's remotes based on information in the database. */
static char *
reconfigure_remotes(struct ovsdb_jsonrpc_server *jsonrpc,
                    const struct shash *all_dbs, struct sset *remotes)
{
    struct ds errors = DS_EMPTY_INITIALIZER;
    struct shash resolved_remotes;
    const char *name;

    /* Configure remotes. */
    shash_init(&resolved_remotes);
    SSET_FOR_EACH (name, remotes) {
        if (!strncmp(name, "db:", 3)) {
            query_db_remotes(name, all_dbs, &resolved_remotes, &errors);
        } else {
            add_remote(&resolved_remotes, name);
        }
    }
    ovsdb_jsonrpc_server_set_remotes(jsonrpc, &resolved_remotes);
    shash_destroy_free_data(&resolved_remotes);

    return errors.string;
}

static char *
reconfigure_ssl(const struct shash *all_dbs)
{
    struct ds errors = DS_EMPTY_INITIALIZER;
    const char *resolved_private_key;
    const char *resolved_certificate;
    const char *resolved_ca_cert;

    resolved_private_key = query_db_string(all_dbs, private_key_file, &errors);
    resolved_certificate = query_db_string(all_dbs, certificate_file, &errors);
    resolved_ca_cert = query_db_string(all_dbs, ca_cert_file, &errors);

    stream_ssl_set_key_and_cert(resolved_private_key, resolved_certificate);
    stream_ssl_set_ca_cert_file(resolved_ca_cert, bootstrap_ca_cert);

    return errors.string;
}

static void
report_error_if_changed(char *error, char **last_errorp)
{
    if (error) {
        if (!*last_errorp || strcmp(error, *last_errorp)) {
            VLOG_WARN("%s", error);
            free(*last_errorp);
            *last_errorp = error;
            return;
        }
        free(error);
    } else {
        free(*last_errorp);
        *last_errorp = NULL;
    }
}

static void
ovsdb_server_exit(struct unixctl_conn *conn, int argc OVS_UNUSED,
                  const char *argv[] OVS_UNUSED,
                  void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

static void
ovsdb_server_compact(struct unixctl_conn *conn, int argc,
                     const char *argv[], void *dbs_)
{
    struct shash *all_dbs = dbs_;
    struct ds reply;
    struct db *db;
    struct shash_node *node;
    int n = 0;

    ds_init(&reply);
    SHASH_FOR_EACH(node, all_dbs) {
        const char *name;

        db = node->data;
        name = db->db->schema->name;

        if (argc < 2 || !strcmp(argv[1], name)) {
            struct ovsdb_error *error;

            VLOG_INFO("compacting %s database by user request", name);

            error = ovsdb_file_compact(db->file);
            if (error) {
                char *s = ovsdb_error_to_string(error);
                ds_put_format(&reply, "%s\n", s);
                free(s);
                ovsdb_error_destroy(error);
            }

            n++;
        }
    }

    if (!n) {
        unixctl_command_reply_error(conn, "no database by that name");
    } else if (reply.length) {
        unixctl_command_reply_error(conn, ds_cstr(&reply));
    } else {
        unixctl_command_reply(conn, NULL);
    }
    ds_destroy(&reply);
}

/* "ovsdb-server/reconnect": makes ovsdb-server drop all of its JSON-RPC
 * connections and reconnect. */
static void
ovsdb_server_reconnect(struct unixctl_conn *conn, int argc OVS_UNUSED,
                       const char *argv[] OVS_UNUSED, void *jsonrpc_)
{
    struct ovsdb_jsonrpc_server *jsonrpc = jsonrpc_;

    ovsdb_jsonrpc_server_reconnect(jsonrpc);
    unixctl_command_reply(conn, NULL);
}

/* "ovsdb-server/add-remote REMOTE": adds REMOTE to the set of remotes that
 * ovsdb-server services. */
static void
ovsdb_server_add_remote(struct unixctl_conn *conn, int argc OVS_UNUSED,
                        const char *argv[], void *config_)
{
    struct server_config *config = config_;
    const char *remote = argv[1];

    const struct ovsdb_column *column;
    const struct ovsdb_table *table;
    const struct db *db;
    char *retval;

    retval = (strncmp("db:", remote, 3)
              ? NULL
              : parse_db_column(config->all_dbs, remote,
                                &db, &table, &column));
    if (!retval) {
        if (sset_add(config->remotes, remote)) {
            save_config(config);
        }
        unixctl_command_reply(conn, NULL);
    } else {
        unixctl_command_reply_error(conn, retval);
        free(retval);
    }
}

/* "ovsdb-server/remove-remote REMOTE": removes REMOTE frmo the set of remotes
 * that ovsdb-server services. */
static void
ovsdb_server_remove_remote(struct unixctl_conn *conn, int argc OVS_UNUSED,
                           const char *argv[], void *config_)
{
    struct server_config *config = config_;
    struct sset_node *node;

    node = sset_find(config->remotes, argv[1]);
    if (node) {
        sset_delete(config->remotes, node);
        save_config(config);
        unixctl_command_reply(conn, NULL);
    } else {
        unixctl_command_reply_error(conn, "no such remote");
    }
}

/* "ovsdb-server/list-remotes": outputs a list of configured rmeotes. */
static void
ovsdb_server_list_remotes(struct unixctl_conn *conn, int argc OVS_UNUSED,
                          const char *argv[] OVS_UNUSED, void *remotes_)
{
    struct sset *remotes = remotes_;
    const char **list, **p;
    struct ds s;

    ds_init(&s);

    list = sset_sort(remotes);
    for (p = list; *p; p++) {
        ds_put_format(&s, "%s\n", *p);
    }
    free(list);

    unixctl_command_reply(conn, ds_cstr(&s));
    ds_destroy(&s);
}


/* "ovsdb-server/add-db DB": adds the DB to ovsdb-server. */
static void
ovsdb_server_add_database(struct unixctl_conn *conn, int argc OVS_UNUSED,
                          const char *argv[], void *config_)
{
    struct server_config *config = config_;
    const char *filename = argv[1];
    char *error;

    error = open_db(config, filename);
    if (!error) {
        save_config(config);
        unixctl_command_reply(conn, NULL);
    } else {
        unixctl_command_reply_error(conn, error);
        free(error);
    }
}

static void
ovsdb_server_remove_database(struct unixctl_conn *conn, int argc OVS_UNUSED,
                             const char *argv[], void *config_)
{
    struct server_config *config = config_;
    struct shash_node *node;
    struct db *db;
    bool ok;

    node = shash_find(config->all_dbs, argv[1]);
    if (!node)  {
        unixctl_command_reply_error(conn, "Failed to find the database.");
        return;
    }
    db = node->data;

    ok = ovsdb_jsonrpc_server_remove_db(config->jsonrpc, db->db);
    ovs_assert(ok);

    ovsdb_destroy(db->db);
    shash_delete(config->all_dbs, node);
    free(db->filename);
    free(db);

    save_config(config);
    unixctl_command_reply(conn, NULL);
}

static void
ovsdb_server_list_databases(struct unixctl_conn *conn, int argc OVS_UNUSED,
                            const char *argv[] OVS_UNUSED, void *all_dbs_)
{
    struct shash *all_dbs = all_dbs_;
    const struct shash_node **nodes;
    struct ds s;
    size_t i;

    ds_init(&s);

    nodes = shash_sort(all_dbs);
    for (i = 0; i < shash_count(all_dbs); i++) {
        struct db *db = nodes[i]->data;
        ds_put_format(&s, "%s\n", db->db->schema->name);
    }
    free(nodes);

    unixctl_command_reply(conn, ds_cstr(&s));
    ds_destroy(&s);
}

static void
parse_options(int *argcp, char **argvp[],
              struct sset *remotes, char **unixctl_pathp, char **run_command)
{
    enum {
        OPT_REMOTE = UCHAR_MAX + 1,
        OPT_UNIXCTL,
        OPT_RUN,
        OPT_BOOTSTRAP_CA_CERT,
        OPT_ENABLE_DUMMY,
        VLOG_OPTION_ENUMS,
        DAEMON_OPTION_ENUMS
    };
    static const struct option long_options[] = {
        {"remote",      required_argument, NULL, OPT_REMOTE},
        {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
#ifndef _WIN32
        {"run",         required_argument, NULL, OPT_RUN},
#endif
        {"help",        no_argument, NULL, 'h'},
        {"version",     no_argument, NULL, 'V'},
        DAEMON_LONG_OPTIONS,
        VLOG_LONG_OPTIONS,
        {"bootstrap-ca-cert", required_argument, NULL, OPT_BOOTSTRAP_CA_CERT},
        {"private-key", required_argument, NULL, 'p'},
        {"certificate", required_argument, NULL, 'c'},
        {"ca-cert",     required_argument, NULL, 'C'},
        {"enable-dummy", optional_argument, NULL, OPT_ENABLE_DUMMY},
        {NULL, 0, NULL, 0},
    };
    char *short_options = long_options_to_short_options(long_options);
    int argc = *argcp;
    char **argv = *argvp;

    sset_init(remotes);
    for (;;) {
        int c;

        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) {
            break;
        }

        switch (c) {
        case OPT_REMOTE:
            sset_add(remotes, optarg);
            break;

        case OPT_UNIXCTL:
            *unixctl_pathp = optarg;
            break;

        case OPT_RUN:
            *run_command = optarg;
            break;

        case 'h':
            usage();

        case 'V':
            ovs_print_version(0, 0);
            exit(EXIT_SUCCESS);

        VLOG_OPTION_HANDLERS
        DAEMON_OPTION_HANDLERS

        case 'p':
            private_key_file = optarg;
            break;

        case 'c':
            certificate_file = optarg;
            break;

        case 'C':
            ca_cert_file = optarg;
            bootstrap_ca_cert = false;
            break;

        case OPT_BOOTSTRAP_CA_CERT:
            ca_cert_file = optarg;
            bootstrap_ca_cert = true;
            break;

        case OPT_ENABLE_DUMMY:
            dummy_enable(optarg && !strcmp(optarg, "override"));
            break;

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }
    free(short_options);

    *argcp -= optind;
    *argvp += optind;
}

static void
usage(void)
{
    printf("%s: Open vSwitch database server\n"
           "usage: %s [OPTIONS] [DATABASE...]\n"
           "where each DATABASE is a database file in ovsdb format.\n"
           "The default DATABASE, if none is given, is\n%s/conf.db.\n",
           program_name, program_name, ovs_dbdir());
    printf("\nJSON-RPC options (may be specified any number of times):\n"
           "  --remote=REMOTE         connect or listen to REMOTE\n");
    stream_usage("JSON-RPC", true, true, true);
    daemon_usage();
    vlog_usage();
    printf("\nOther options:\n"
           "  --run COMMAND           run COMMAND as subprocess then exit\n"
           "  --unixctl=SOCKET        override default control socket name\n"
           "  -h, --help              display this help message\n"
           "  -V, --version           display version information\n");
    exit(EXIT_SUCCESS);
}

static struct json *
sset_to_json(const struct sset *sset)
{
    struct json *array;
    const char *s;

    array = json_array_create_empty();
    SSET_FOR_EACH (s, sset) {
        json_array_add(array, json_string_create(s));
    }
    return array;
}

/* Truncates and replaces the contents of 'config_file' by a representation of
 * 'remotes' and 'db_filenames'. */
static void
save_config__(FILE *config_file, const struct sset *remotes,
              const struct sset *db_filenames)
{
    struct json *obj;
    char *s;

    if (ftruncate(fileno(config_file), 0) == -1) {
        VLOG_FATAL("failed to truncate temporary file (%s)",
                   ovs_strerror(errno));
    }

    obj = json_object_create();
    json_object_put(obj, "remotes", sset_to_json(remotes));
    json_object_put(obj, "db_filenames", sset_to_json(db_filenames));
    s = json_to_string(obj, 0);
    json_destroy(obj);

    if (fseek(config_file, 0, SEEK_SET) != 0
        || fputs(s, config_file) == EOF
        || fflush(config_file) == EOF) {
        VLOG_FATAL("failed to write temporary file (%s)", ovs_strerror(errno));
    }
    free(s);
}

/* Truncates and replaces the contents of 'config_file' by a representation of
 * 'config'. */
static void
save_config(struct server_config *config)
{
    struct sset db_filenames;
    struct shash_node *node;

    sset_init(&db_filenames);
    SHASH_FOR_EACH (node, config->all_dbs) {
        struct db *db = node->data;
        sset_add(&db_filenames, db->filename);
    }

    save_config__(config->config_tmpfile, config->remotes, &db_filenames);

    sset_destroy(&db_filenames);
}

static void
sset_from_json(struct sset *sset, const struct json *array)
{
    size_t i;

    sset_clear(sset);

    ovs_assert(array->type == JSON_ARRAY);
    for (i = 0; i < array->u.array.n; i++) {
        const struct json *elem = array->u.array.elems[i];
        sset_add(sset, json_string(elem));
    }
}

/* Clears and replaces 'remotes' and 'dbnames' by a configuration read from
 * 'config_file', which must have been previously written by save_config(). */
static void
load_config(FILE *config_file, struct sset *remotes, struct sset *db_filenames)
{
    struct json *json;

    if (fseek(config_file, 0, SEEK_SET) != 0) {
        VLOG_FATAL("seek failed in temporary file (%s)", ovs_strerror(errno));
    }
    json = json_from_stream(config_file);
    if (json->type == JSON_STRING) {
        VLOG_FATAL("reading json failed (%s)", json_string(json));
    }
    ovs_assert(json->type == JSON_OBJECT);

    sset_from_json(remotes, shash_find_data(json_object(json), "remotes"));
    sset_from_json(db_filenames,
                   shash_find_data(json_object(json), "db_filenames"));
    json_destroy(json);
}
