/* Copyright (c) 2009, 2010, 2011, 2012, 2013 Nicira, Inc.
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

#include "jsonrpc-server.h"

#include <errno.h>

#include "bitmap.h"
#include "column.h"
#include "dynamic-string.h"
#include "json.h"
#include "jsonrpc.h"
#include "ovsdb-error.h"
#include "ovsdb-parser.h"
#include "ovsdb.h"
#include "reconnect.h"
#include "row.h"
#include "server.h"
#include "simap.h"
#include "stream.h"
#include "table.h"
#include "timeval.h"
#include "transaction.h"
#include "trigger.h"
#include "vlog.h"

VLOG_DEFINE_THIS_MODULE(ovsdb_jsonrpc_server);

struct ovsdb_jsonrpc_remote;
struct ovsdb_jsonrpc_session;

/* Message rate-limiting. */
static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 5);

/* Sessions. */
static struct ovsdb_jsonrpc_session *ovsdb_jsonrpc_session_create(
    struct ovsdb_jsonrpc_remote *, struct jsonrpc_session *);
static void ovsdb_jsonrpc_session_run_all(struct ovsdb_jsonrpc_remote *);
static void ovsdb_jsonrpc_session_wait_all(struct ovsdb_jsonrpc_remote *);
static void ovsdb_jsonrpc_session_get_memory_usage_all(
    const struct ovsdb_jsonrpc_remote *, struct simap *usage);
static void ovsdb_jsonrpc_session_close_all(struct ovsdb_jsonrpc_remote *);
static void ovsdb_jsonrpc_session_reconnect_all(struct ovsdb_jsonrpc_remote *);
static void ovsdb_jsonrpc_session_set_all_options(
    struct ovsdb_jsonrpc_remote *, const struct ovsdb_jsonrpc_options *);
static bool ovsdb_jsonrpc_session_get_status(
    const struct ovsdb_jsonrpc_remote *,
    struct ovsdb_jsonrpc_remote_status *);
static void ovsdb_jsonrpc_session_unlock_all(struct ovsdb_jsonrpc_session *);
static void ovsdb_jsonrpc_session_unlock__(struct ovsdb_lock_waiter *);

/* Triggers. */
static void ovsdb_jsonrpc_trigger_create(struct ovsdb_jsonrpc_session *,
                                         struct ovsdb *,
                                         struct json *id, struct json *params);
static struct ovsdb_jsonrpc_trigger *ovsdb_jsonrpc_trigger_find(
    struct ovsdb_jsonrpc_session *, const struct json *id, size_t hash);
static void ovsdb_jsonrpc_trigger_complete(struct ovsdb_jsonrpc_trigger *);
static void ovsdb_jsonrpc_trigger_complete_all(struct ovsdb_jsonrpc_session *);
static void ovsdb_jsonrpc_trigger_complete_done(
    struct ovsdb_jsonrpc_session *);

/* Monitors. */
static struct json *ovsdb_jsonrpc_monitor_create(
    struct ovsdb_jsonrpc_session *, struct ovsdb *, struct json *params);
static struct jsonrpc_msg *ovsdb_jsonrpc_monitor_cancel(
    struct ovsdb_jsonrpc_session *,
    struct json_array *params,
    const struct json *request_id);
static void ovsdb_jsonrpc_monitor_remove_all(struct ovsdb_jsonrpc_session *);
static size_t ovsdb_jsonrpc_monitor_json_length_all(
    struct ovsdb_jsonrpc_session *);

/* JSON-RPC database server. */

struct ovsdb_jsonrpc_server {
    struct ovsdb_server up;
    unsigned int n_sessions, max_sessions;
    struct shash remotes;      /* Contains "struct ovsdb_jsonrpc_remote *"s. */
};

/* A configured remote.  This is either a passive stream listener plus a list
 * of the currently connected sessions, or a list of exactly one active
 * session. */
struct ovsdb_jsonrpc_remote {
    struct ovsdb_jsonrpc_server *server;
    struct pstream *listener;   /* Listener, if passive. */
    struct list sessions;       /* List of "struct ovsdb_jsonrpc_session"s. */
    uint8_t dscp;
};

static struct ovsdb_jsonrpc_remote *ovsdb_jsonrpc_server_add_remote(
    struct ovsdb_jsonrpc_server *, const char *name,
    const struct ovsdb_jsonrpc_options *options
);
static void ovsdb_jsonrpc_server_del_remote(struct shash_node *);

/* Creates and returns a new server to provide JSON-RPC access to an OVSDB.
 *
 * The caller must call ovsdb_jsonrpc_server_add_db() for each database to
 * which 'server' should provide access. */
struct ovsdb_jsonrpc_server *
ovsdb_jsonrpc_server_create(void)
{
    struct ovsdb_jsonrpc_server *server = xzalloc(sizeof *server);
    ovsdb_server_init(&server->up);
    server->max_sessions = 64;
    shash_init(&server->remotes);
    return server;
}

/* Adds 'db' to the set of databases served out by 'svr'.  Returns true if
 * successful, false if 'db''s name is the same as some database already in
 * 'server'. */
bool
ovsdb_jsonrpc_server_add_db(struct ovsdb_jsonrpc_server *svr, struct ovsdb *db)
{
    return ovsdb_server_add_db(&svr->up, db);
}

void
ovsdb_jsonrpc_server_destroy(struct ovsdb_jsonrpc_server *svr)
{
    struct shash_node *node, *next;

    SHASH_FOR_EACH_SAFE (node, next, &svr->remotes) {
        ovsdb_jsonrpc_server_del_remote(node);
    }
    shash_destroy(&svr->remotes);
    ovsdb_server_destroy(&svr->up);
    free(svr);
}

struct ovsdb_jsonrpc_options *
ovsdb_jsonrpc_default_options(const char *target)
{
    struct ovsdb_jsonrpc_options *options = xzalloc(sizeof *options);
    options->max_backoff = RECONNECT_DEFAULT_MAX_BACKOFF;
    options->probe_interval = (stream_or_pstream_needs_probes(target)
                               ? RECONNECT_DEFAULT_PROBE_INTERVAL
                               : 0);
    return options;
}

/* Sets 'svr''s current set of remotes to the names in 'new_remotes', with
 * options in the struct ovsdb_jsonrpc_options supplied as the data values.
 *
 * A remote is an active or passive stream connection method, e.g. "pssl:" or
 * "tcp:1.2.3.4". */
void
ovsdb_jsonrpc_server_set_remotes(struct ovsdb_jsonrpc_server *svr,
                                 const struct shash *new_remotes)
{
    struct shash_node *node, *next;

    SHASH_FOR_EACH_SAFE (node, next, &svr->remotes) {
        if (!shash_find(new_remotes, node->name)) {
            VLOG_INFO("%s: remote deconfigured", node->name);
            ovsdb_jsonrpc_server_del_remote(node);
        }
    }
    SHASH_FOR_EACH (node, new_remotes) {
        const struct ovsdb_jsonrpc_options *options = node->data;
        struct ovsdb_jsonrpc_remote *remote;

        remote = shash_find_data(&svr->remotes, node->name);
        if (!remote) {
            remote = ovsdb_jsonrpc_server_add_remote(svr, node->name, options);
            if (!remote) {
                continue;
            }
        }

        ovsdb_jsonrpc_session_set_all_options(remote, options);
    }
}

static struct ovsdb_jsonrpc_remote *
ovsdb_jsonrpc_server_add_remote(struct ovsdb_jsonrpc_server *svr,
                                const char *name,
                                const struct ovsdb_jsonrpc_options *options)
{
    struct ovsdb_jsonrpc_remote *remote;
    struct pstream *listener;
    int error;

    error = jsonrpc_pstream_open(name, &listener, options->dscp);
    if (error && error != EAFNOSUPPORT) {
        VLOG_ERR_RL(&rl, "%s: listen failed: %s", name, strerror(error));
        return NULL;
    }

    remote = xmalloc(sizeof *remote);
    remote->server = svr;
    remote->listener = listener;
    list_init(&remote->sessions);
    remote->dscp = options->dscp;
    shash_add(&svr->remotes, name, remote);

    if (!listener) {
        ovsdb_jsonrpc_session_create(remote, jsonrpc_session_open(name, true));
    }
    return remote;
}

static void
ovsdb_jsonrpc_server_del_remote(struct shash_node *node)
{
    struct ovsdb_jsonrpc_remote *remote = node->data;

    ovsdb_jsonrpc_session_close_all(remote);
    pstream_close(remote->listener);
    shash_delete(&remote->server->remotes, node);
    free(remote);
}

/* Stores status information for the remote named 'target', which should have
 * been configured on 'svr' with a call to ovsdb_jsonrpc_server_set_remotes(),
 * into '*status'.  On success returns true, on failure (if 'svr' doesn't have
 * a remote named 'target' or if that remote is an inbound remote that has no
 * active connections) returns false.  On failure, 'status' will be zeroed.
 */
bool
ovsdb_jsonrpc_server_get_remote_status(
    const struct ovsdb_jsonrpc_server *svr, const char *target,
    struct ovsdb_jsonrpc_remote_status *status)
{
    const struct ovsdb_jsonrpc_remote *remote;

    memset(status, 0, sizeof *status);

    remote = shash_find_data(&svr->remotes, target);
    return remote && ovsdb_jsonrpc_session_get_status(remote, status);
}

void
ovsdb_jsonrpc_server_free_remote_status(
    struct ovsdb_jsonrpc_remote_status *status)
{
    free(status->locks_held);
    free(status->locks_waiting);
    free(status->locks_lost);
}

/* Forces all of the JSON-RPC sessions managed by 'svr' to disconnect and
 * reconnect. */
void
ovsdb_jsonrpc_server_reconnect(struct ovsdb_jsonrpc_server *svr)
{
    struct shash_node *node;

    SHASH_FOR_EACH (node, &svr->remotes) {
        struct ovsdb_jsonrpc_remote *remote = node->data;

        ovsdb_jsonrpc_session_reconnect_all(remote);
    }
}

void
ovsdb_jsonrpc_server_run(struct ovsdb_jsonrpc_server *svr)
{
    struct shash_node *node;

    SHASH_FOR_EACH (node, &svr->remotes) {
        struct ovsdb_jsonrpc_remote *remote = node->data;

        if (remote->listener && svr->n_sessions < svr->max_sessions) {
            struct stream *stream;
            int error;

            error = pstream_accept(remote->listener, &stream);
            if (!error) {
                struct jsonrpc_session *js;
                js = jsonrpc_session_open_unreliably(jsonrpc_open(stream),
                                                     remote->dscp);
                ovsdb_jsonrpc_session_create(remote, js);
            } else if (error != EAGAIN) {
                VLOG_WARN_RL(&rl, "%s: accept failed: %s",
                             pstream_get_name(remote->listener),
                             strerror(error));
            }
        }

        ovsdb_jsonrpc_session_run_all(remote);
    }
}

void
ovsdb_jsonrpc_server_wait(struct ovsdb_jsonrpc_server *svr)
{
    struct shash_node *node;

    SHASH_FOR_EACH (node, &svr->remotes) {
        struct ovsdb_jsonrpc_remote *remote = node->data;

        if (remote->listener && svr->n_sessions < svr->max_sessions) {
            pstream_wait(remote->listener);
        }

        ovsdb_jsonrpc_session_wait_all(remote);
    }
}

/* Adds some memory usage statistics for 'svr' into 'usage', for use with
 * memory_report(). */
void
ovsdb_jsonrpc_server_get_memory_usage(const struct ovsdb_jsonrpc_server *svr,
                                      struct simap *usage)
{
    struct shash_node *node;

    simap_increase(usage, "sessions", svr->n_sessions);
    SHASH_FOR_EACH (node, &svr->remotes) {
        struct ovsdb_jsonrpc_remote *remote = node->data;

        ovsdb_jsonrpc_session_get_memory_usage_all(remote, usage);
    }
}

/* JSON-RPC database server session. */

struct ovsdb_jsonrpc_session {
    struct list node;           /* Element in remote's sessions list. */
    struct ovsdb_session up;
    struct ovsdb_jsonrpc_remote *remote;
    size_t backlog_threshold;   /* See ovsdb_jsonrpc_session_run(). */
    size_t reply_backlog;

    /* Triggers. */
    struct hmap triggers;       /* Hmap of "struct ovsdb_jsonrpc_trigger"s. */

    /* Monitors. */
    struct hmap monitors;       /* Hmap of "struct ovsdb_jsonrpc_monitor"s. */

    /* Network connectivity. */
    struct jsonrpc_session *js;  /* JSON-RPC session. */
    unsigned int js_seqno;       /* Last jsonrpc_session_get_seqno() value. */
};

static void ovsdb_jsonrpc_session_close(struct ovsdb_jsonrpc_session *);
static int ovsdb_jsonrpc_session_run(struct ovsdb_jsonrpc_session *);
static void ovsdb_jsonrpc_session_wait(struct ovsdb_jsonrpc_session *);
static void ovsdb_jsonrpc_session_get_memory_usage(
    const struct ovsdb_jsonrpc_session *, struct simap *usage);
static void ovsdb_jsonrpc_session_set_options(
    struct ovsdb_jsonrpc_session *, const struct ovsdb_jsonrpc_options *);
static void ovsdb_jsonrpc_session_got_request(struct ovsdb_jsonrpc_session *,
                                             struct jsonrpc_msg *);
static void ovsdb_jsonrpc_session_got_notify(struct ovsdb_jsonrpc_session *,
                                             struct jsonrpc_msg *);

static struct ovsdb_jsonrpc_session *
ovsdb_jsonrpc_session_create(struct ovsdb_jsonrpc_remote *remote,
                             struct jsonrpc_session *js)
{
    struct ovsdb_jsonrpc_session *s;

    s = xzalloc(sizeof *s);
    ovsdb_session_init(&s->up, &remote->server->up);
    s->remote = remote;
    list_push_back(&remote->sessions, &s->node);
    hmap_init(&s->triggers);
    hmap_init(&s->monitors);
    s->reply_backlog = 0;
    s->backlog_threshold = 1024 * 1024;
    s->js = js;
    s->js_seqno = jsonrpc_session_get_seqno(js);

    remote->server->n_sessions++;

    return s;
}

static void
ovsdb_jsonrpc_session_close(struct ovsdb_jsonrpc_session *s)
{
    ovsdb_jsonrpc_monitor_remove_all(s);
    ovsdb_jsonrpc_session_unlock_all(s);
    ovsdb_jsonrpc_trigger_complete_all(s);

    hmap_destroy(&s->monitors);
    hmap_destroy(&s->triggers);

    jsonrpc_session_close(s->js);
    list_remove(&s->node);
    s->remote->server->n_sessions--;
    ovsdb_session_destroy(&s->up);
    free(s);
}

static int
ovsdb_jsonrpc_session_run(struct ovsdb_jsonrpc_session *s)
{
    size_t backlog;

    jsonrpc_session_run(s->js);
    if (s->js_seqno != jsonrpc_session_get_seqno(s->js)) {
        s->js_seqno = jsonrpc_session_get_seqno(s->js);
        ovsdb_jsonrpc_trigger_complete_all(s);
        ovsdb_jsonrpc_monitor_remove_all(s);
        ovsdb_jsonrpc_session_unlock_all(s);
    }

    ovsdb_jsonrpc_trigger_complete_done(s);

    backlog = jsonrpc_session_get_backlog(s->js);
    if (!backlog) {
        struct jsonrpc_msg *msg = jsonrpc_session_recv(s->js);
        if (msg) {
            if (msg->type == JSONRPC_REQUEST) {
                ovsdb_jsonrpc_session_got_request(s, msg);
            } else if (msg->type == JSONRPC_NOTIFY) {
                ovsdb_jsonrpc_session_got_notify(s, msg);
            } else {
                VLOG_WARN("%s: received unexpected %s message",
                          jsonrpc_session_get_name(s->js),
                          jsonrpc_msg_type_to_string(msg->type));
                jsonrpc_session_force_reconnect(s->js);
                jsonrpc_msg_destroy(msg);
            }
        }
        s->reply_backlog = jsonrpc_session_get_backlog(s->js);
    } else if (backlog > s->reply_backlog + s->backlog_threshold) {
        /* We have a lot of data queued to send to the client.  The data is
         * likely to be mostly monitor updates.  It is unlikely that the
         * monitor updates are due to transactions by 's', because we will not
         * let 's' make any more transactions until it drains its backlog to 0
         * (see previous 'if' case).  So the monitor updates are probably due
         * to transactions made by database clients other than 's'.  We can't
         * fix that by preventing 's' from executing more transactions.  We
         * could fix it by preventing every client from executing transactions,
         * but then one slow or hung client could prevent other clients from
         * doing useful work.
         *
         * Our solution is to cap the maximum backlog to O(1) in the amount of
         * data in the database.  If the backlog exceeds that amount, then we
         * disconnect the client.  When it reconnects, it can fetch the entire
         * contents of the database using less data than was previously
         * backlogged. */
        size_t monitor_length;

        monitor_length = ovsdb_jsonrpc_monitor_json_length_all(s);
        if (backlog > s->reply_backlog + monitor_length * 2) {
            VLOG_INFO("%s: %zu bytes backlogged but a complete replica "
                      "would only take %zu bytes, disconnecting",
                      jsonrpc_session_get_name(s->js),
                      backlog - s->reply_backlog, monitor_length);
            jsonrpc_session_force_reconnect(s->js);
        } else {
            /* The backlog is not unreasonably big.  Only check again after it
             * becomes much bigger. */
            s->backlog_threshold = 2 * MAX(s->backlog_threshold * 2,
                                           monitor_length);
        }
    }
    return jsonrpc_session_is_alive(s->js) ? 0 : ETIMEDOUT;
}

static void
ovsdb_jsonrpc_session_set_options(struct ovsdb_jsonrpc_session *session,
                                  const struct ovsdb_jsonrpc_options *options)
{
    jsonrpc_session_set_max_backoff(session->js, options->max_backoff);
    jsonrpc_session_set_probe_interval(session->js, options->probe_interval);
    jsonrpc_session_set_dscp(session->js, options->dscp);
}

static void
ovsdb_jsonrpc_session_run_all(struct ovsdb_jsonrpc_remote *remote)
{
    struct ovsdb_jsonrpc_session *s, *next;

    LIST_FOR_EACH_SAFE (s, next, node, &remote->sessions) {
        int error = ovsdb_jsonrpc_session_run(s);
        if (error) {
            ovsdb_jsonrpc_session_close(s);
        }
    }
}

static void
ovsdb_jsonrpc_session_wait(struct ovsdb_jsonrpc_session *s)
{
    jsonrpc_session_wait(s->js);
    if (!jsonrpc_session_get_backlog(s->js)) {
        jsonrpc_session_recv_wait(s->js);
    }
}

static void
ovsdb_jsonrpc_session_wait_all(struct ovsdb_jsonrpc_remote *remote)
{
    struct ovsdb_jsonrpc_session *s;

    LIST_FOR_EACH (s, node, &remote->sessions) {
        ovsdb_jsonrpc_session_wait(s);
    }
}

static void
ovsdb_jsonrpc_session_get_memory_usage(const struct ovsdb_jsonrpc_session *s,
                                       struct simap *usage)
{
    simap_increase(usage, "triggers", hmap_count(&s->triggers));
    simap_increase(usage, "monitors", hmap_count(&s->monitors));
    simap_increase(usage, "backlog", jsonrpc_session_get_backlog(s->js));
}

static void
ovsdb_jsonrpc_session_get_memory_usage_all(
    const struct ovsdb_jsonrpc_remote *remote,
    struct simap *usage)
{
    struct ovsdb_jsonrpc_session *s;

    LIST_FOR_EACH (s, node, &remote->sessions) {
        ovsdb_jsonrpc_session_get_memory_usage(s, usage);
    }
}

static void
ovsdb_jsonrpc_session_close_all(struct ovsdb_jsonrpc_remote *remote)
{
    struct ovsdb_jsonrpc_session *s, *next;

    LIST_FOR_EACH_SAFE (s, next, node, &remote->sessions) {
        ovsdb_jsonrpc_session_close(s);
    }
}

/* Forces all of the JSON-RPC sessions managed by 'remote' to disconnect and
 * reconnect. */
static void
ovsdb_jsonrpc_session_reconnect_all(struct ovsdb_jsonrpc_remote *remote)
{
    struct ovsdb_jsonrpc_session *s, *next;

    LIST_FOR_EACH_SAFE (s, next, node, &remote->sessions) {
        jsonrpc_session_force_reconnect(s->js);
        if (!jsonrpc_session_is_alive(s->js)) {
            ovsdb_jsonrpc_session_close(s);
        }
    }
}

/* Sets the options for all of the JSON-RPC sessions managed by 'remote' to
 * 'options'. */
static void
ovsdb_jsonrpc_session_set_all_options(
    struct ovsdb_jsonrpc_remote *remote,
    const struct ovsdb_jsonrpc_options *options)
{
    struct ovsdb_jsonrpc_session *s;

    if (remote->listener) {
        int error;

        error = pstream_set_dscp(remote->listener, options->dscp);
        if (error) {
            VLOG_ERR("%s: set_dscp failed %s",
                     pstream_get_name(remote->listener), strerror(error));
        } else {
            remote->dscp = options->dscp;
        }
        /*
         * XXX race window between setting dscp to listening socket
         * and accepting socket. Accepted socket may have old dscp value.
         * Ignore this race window for now.
         */
    }
    LIST_FOR_EACH (s, node, &remote->sessions) {
        ovsdb_jsonrpc_session_set_options(s, options);
    }
}

static bool
ovsdb_jsonrpc_session_get_status(const struct ovsdb_jsonrpc_remote *remote,
                                 struct ovsdb_jsonrpc_remote_status *status)
{
    const struct ovsdb_jsonrpc_session *s;
    const struct jsonrpc_session *js;
    struct ovsdb_lock_waiter *waiter;
    struct reconnect_stats rstats;
    struct ds locks_held, locks_waiting, locks_lost;

    status->bound_port = (remote->listener
                          ? pstream_get_bound_port(remote->listener)
                          : htons(0));

    if (list_is_empty(&remote->sessions)) {
        return false;
    }
    s = CONTAINER_OF(remote->sessions.next, struct ovsdb_jsonrpc_session, node);
    js = s->js;

    status->is_connected = jsonrpc_session_is_connected(js);
    status->last_error = jsonrpc_session_get_status(js);

    jsonrpc_session_get_reconnect_stats(js, &rstats);
    status->state = rstats.state;
    status->sec_since_connect = rstats.msec_since_connect == UINT_MAX
        ? UINT_MAX : rstats.msec_since_connect / 1000;
    status->sec_since_disconnect = rstats.msec_since_disconnect == UINT_MAX
        ? UINT_MAX : rstats.msec_since_disconnect / 1000;

    ds_init(&locks_held);
    ds_init(&locks_waiting);
    ds_init(&locks_lost);
    HMAP_FOR_EACH (waiter, session_node, &s->up.waiters) {
        struct ds *string;

        string = (ovsdb_lock_waiter_is_owner(waiter) ? &locks_held
                  : waiter->mode == OVSDB_LOCK_WAIT ? &locks_waiting
                  : &locks_lost);
        if (string->length) {
            ds_put_char(string, ' ');
        }
        ds_put_cstr(string, waiter->lock_name);
    }
    status->locks_held = ds_steal_cstr(&locks_held);
    status->locks_waiting = ds_steal_cstr(&locks_waiting);
    status->locks_lost = ds_steal_cstr(&locks_lost);

    status->n_connections = list_size(&remote->sessions);

    return true;
}

/* Examines 'request' to determine the database to which it relates, and then
 * searches 's' to find that database:
 *
 *    - If successful, returns the database and sets '*replyp' to NULL.
 *
 *    - If no such database exists, returns NULL and sets '*replyp' to an
 *      appropriate JSON-RPC error reply, owned by the caller. */
static struct ovsdb *
ovsdb_jsonrpc_lookup_db(const struct ovsdb_jsonrpc_session *s,
                        const struct jsonrpc_msg *request,
                        struct jsonrpc_msg **replyp)
{
    struct json_array *params;
    struct ovsdb_error *error;
    const char *db_name;
    struct ovsdb *db;

    params = json_array(request->params);
    if (!params->n || params->elems[0]->type != JSON_STRING) {
        error = ovsdb_syntax_error(
            request->params, NULL,
            "%s request params must begin with <db-name>", request->method);
        goto error;
    }

    db_name = params->elems[0]->u.string;
    db = shash_find_data(&s->up.server->dbs, db_name);
    if (!db) {
        error = ovsdb_syntax_error(
            request->params, "unknown database",
            "%s request specifies unknown database %s",
            request->method, db_name);
        goto error;
    }

    *replyp = NULL;
    return db;

error:
    *replyp = jsonrpc_create_reply(ovsdb_error_to_json(error), request->id);
    ovsdb_error_destroy(error);
    return NULL;
}

static struct ovsdb_error *
ovsdb_jsonrpc_session_parse_lock_name(const struct jsonrpc_msg *request,
                                      const char **lock_namep)
{
    const struct json_array *params;

    params = json_array(request->params);
    if (params->n != 1 || params->elems[0]->type != JSON_STRING ||
        !ovsdb_parser_is_id(json_string(params->elems[0]))) {
        *lock_namep = NULL;
        return ovsdb_syntax_error(request->params, NULL,
                                  "%s request params must be <id>",
                                  request->method);
    }

    *lock_namep = json_string(params->elems[0]);
    return NULL;
}

static void
ovsdb_jsonrpc_session_notify(struct ovsdb_session *session,
                             const char *lock_name,
                             const char *method)
{
    struct ovsdb_jsonrpc_session *s;
    struct json *params;

    s = CONTAINER_OF(session, struct ovsdb_jsonrpc_session, up);
    params = json_array_create_1(json_string_create(lock_name));
    jsonrpc_session_send(s->js, jsonrpc_create_notify(method, params));
}

static struct jsonrpc_msg *
ovsdb_jsonrpc_session_lock(struct ovsdb_jsonrpc_session *s,
                           struct jsonrpc_msg *request,
                           enum ovsdb_lock_mode mode)
{
    struct ovsdb_lock_waiter *waiter;
    struct jsonrpc_msg *reply;
    struct ovsdb_error *error;
    struct ovsdb_session *victim;
    const char *lock_name;
    struct json *result;

    error = ovsdb_jsonrpc_session_parse_lock_name(request, &lock_name);
    if (error) {
        goto error;
    }

    /* Report error if this session has issued a "lock" or "steal" without a
     * matching "unlock" for this lock. */
    waiter = ovsdb_session_get_lock_waiter(&s->up, lock_name);
    if (waiter) {
        error = ovsdb_syntax_error(
            request->params, NULL,
            "must issue \"unlock\" before new \"%s\"", request->method);
        goto error;
    }

    /* Get the lock, add us as a waiter. */
    waiter = ovsdb_server_lock(&s->remote->server->up, &s->up, lock_name, mode,
                               &victim);
    if (victim) {
        ovsdb_jsonrpc_session_notify(victim, lock_name, "stolen");
    }

    result = json_object_create();
    json_object_put(result, "locked",
                    json_boolean_create(ovsdb_lock_waiter_is_owner(waiter)));

    return jsonrpc_create_reply(result, request->id);

error:
    reply = jsonrpc_create_reply(ovsdb_error_to_json(error), request->id);
    ovsdb_error_destroy(error);
    return reply;
}

static void
ovsdb_jsonrpc_session_unlock_all(struct ovsdb_jsonrpc_session *s)
{
    struct ovsdb_lock_waiter *waiter, *next;

    HMAP_FOR_EACH_SAFE (waiter, next, session_node, &s->up.waiters) {
        ovsdb_jsonrpc_session_unlock__(waiter);
    }
}

static void
ovsdb_jsonrpc_session_unlock__(struct ovsdb_lock_waiter *waiter)
{
    struct ovsdb_lock *lock = waiter->lock;

    if (lock) {
        struct ovsdb_session *new_owner = ovsdb_lock_waiter_remove(waiter);
        if (new_owner) {
            ovsdb_jsonrpc_session_notify(new_owner, lock->name, "locked");
        } else {
            /* ovsdb_server_lock() might have freed 'lock'. */
        }
    }

    ovsdb_lock_waiter_destroy(waiter);
}

static struct jsonrpc_msg *
ovsdb_jsonrpc_session_unlock(struct ovsdb_jsonrpc_session *s,
                             struct jsonrpc_msg *request)
{
    struct ovsdb_lock_waiter *waiter;
    struct jsonrpc_msg *reply;
    struct ovsdb_error *error;
    const char *lock_name;

    error = ovsdb_jsonrpc_session_parse_lock_name(request, &lock_name);
    if (error) {
        goto error;
    }

    /* Report error if this session has not issued a "lock" or "steal" for this
     * lock. */
    waiter = ovsdb_session_get_lock_waiter(&s->up, lock_name);
    if (!waiter) {
        error = ovsdb_syntax_error(
            request->params, NULL, "\"unlock\" without \"lock\" or \"steal\"");
        goto error;
    }

    ovsdb_jsonrpc_session_unlock__(waiter);

    return jsonrpc_create_reply(json_object_create(), request->id);

error:
    reply = jsonrpc_create_reply(ovsdb_error_to_json(error), request->id);
    ovsdb_error_destroy(error);
    return reply;
}

static struct jsonrpc_msg *
execute_transaction(struct ovsdb_jsonrpc_session *s, struct ovsdb *db,
                    struct jsonrpc_msg *request)
{
    ovsdb_jsonrpc_trigger_create(s, db, request->id, request->params);
    request->id = NULL;
    request->params = NULL;
    jsonrpc_msg_destroy(request);
    return NULL;
}

static void
ovsdb_jsonrpc_session_got_request(struct ovsdb_jsonrpc_session *s,
                                  struct jsonrpc_msg *request)
{
    struct jsonrpc_msg *reply;

    if (!strcmp(request->method, "transact")) {
        struct ovsdb *db = ovsdb_jsonrpc_lookup_db(s, request, &reply);
        if (!reply) {
            reply = execute_transaction(s, db, request);
        }
    } else if (!strcmp(request->method, "monitor")) {
        struct ovsdb *db = ovsdb_jsonrpc_lookup_db(s, request, &reply);
        if (!reply) {
            reply = jsonrpc_create_reply(
                ovsdb_jsonrpc_monitor_create(s, db, request->params),
                request->id);
        }
    } else if (!strcmp(request->method, "monitor_cancel")) {
        reply = ovsdb_jsonrpc_monitor_cancel(s, json_array(request->params),
                                             request->id);
    } else if (!strcmp(request->method, "get_schema")) {
        struct ovsdb *db = ovsdb_jsonrpc_lookup_db(s, request, &reply);
        if (!reply) {
            reply = jsonrpc_create_reply(ovsdb_schema_to_json(db->schema),
                                         request->id);
        }
    } else if (!strcmp(request->method, "list_dbs")) {
        size_t n_dbs = shash_count(&s->up.server->dbs);
        struct shash_node *node;
        struct json **dbs;
        size_t i;

        dbs = xmalloc(n_dbs * sizeof *dbs);
        i = 0;
        SHASH_FOR_EACH (node, &s->up.server->dbs) {
            dbs[i++] = json_string_create(node->name);
        }
        reply = jsonrpc_create_reply(json_array_create(dbs, n_dbs),
                                     request->id);
    } else if (!strcmp(request->method, "lock")) {
        reply = ovsdb_jsonrpc_session_lock(s, request, OVSDB_LOCK_WAIT);
    } else if (!strcmp(request->method, "steal")) {
        reply = ovsdb_jsonrpc_session_lock(s, request, OVSDB_LOCK_STEAL);
    } else if (!strcmp(request->method, "unlock")) {
        reply = ovsdb_jsonrpc_session_unlock(s, request);
    } else if (!strcmp(request->method, "echo")) {
        reply = jsonrpc_create_reply(json_clone(request->params), request->id);
    } else {
        reply = jsonrpc_create_error(json_string_create("unknown method"),
                                     request->id);
    }

    if (reply) {
        jsonrpc_msg_destroy(request);
        jsonrpc_session_send(s->js, reply);
    }
}

static void
execute_cancel(struct ovsdb_jsonrpc_session *s, struct jsonrpc_msg *request)
{
    if (json_array(request->params)->n == 1) {
        struct ovsdb_jsonrpc_trigger *t;
        struct json *id;

        id = request->params->u.array.elems[0];
        t = ovsdb_jsonrpc_trigger_find(s, id, json_hash(id, 0));
        if (t) {
            ovsdb_jsonrpc_trigger_complete(t);
        }
    }
}

static void
ovsdb_jsonrpc_session_got_notify(struct ovsdb_jsonrpc_session *s,
                                 struct jsonrpc_msg *request)
{
    if (!strcmp(request->method, "cancel")) {
        execute_cancel(s, request);
    }
    jsonrpc_msg_destroy(request);
}

/* JSON-RPC database server triggers.
 *
 * (Every transaction is treated as a trigger even if it doesn't actually have
 * any "wait" operations.) */

struct ovsdb_jsonrpc_trigger {
    struct ovsdb_trigger trigger;
    struct hmap_node hmap_node; /* In session's "triggers" hmap. */
    struct json *id;
};

static void
ovsdb_jsonrpc_trigger_create(struct ovsdb_jsonrpc_session *s, struct ovsdb *db,
                             struct json *id, struct json *params)
{
    struct ovsdb_jsonrpc_trigger *t;
    size_t hash;

    /* Check for duplicate ID. */
    hash = json_hash(id, 0);
    t = ovsdb_jsonrpc_trigger_find(s, id, hash);
    if (t) {
        struct jsonrpc_msg *msg;

        msg = jsonrpc_create_error(json_string_create("duplicate request ID"),
                                   id);
        jsonrpc_session_send(s->js, msg);
        json_destroy(id);
        json_destroy(params);
        return;
    }

    /* Insert into trigger table. */
    t = xmalloc(sizeof *t);
    ovsdb_trigger_init(&s->up, db, &t->trigger, params, time_msec());
    t->id = id;
    hmap_insert(&s->triggers, &t->hmap_node, hash);

    /* Complete early if possible. */
    if (ovsdb_trigger_is_complete(&t->trigger)) {
        ovsdb_jsonrpc_trigger_complete(t);
    }
}

static struct ovsdb_jsonrpc_trigger *
ovsdb_jsonrpc_trigger_find(struct ovsdb_jsonrpc_session *s,
                           const struct json *id, size_t hash)
{
    struct ovsdb_jsonrpc_trigger *t;

    HMAP_FOR_EACH_WITH_HASH (t, hmap_node, hash, &s->triggers) {
        if (json_equal(t->id, id)) {
            return t;
        }
    }

    return NULL;
}

static void
ovsdb_jsonrpc_trigger_complete(struct ovsdb_jsonrpc_trigger *t)
{
    struct ovsdb_jsonrpc_session *s;

    s = CONTAINER_OF(t->trigger.session, struct ovsdb_jsonrpc_session, up);

    if (jsonrpc_session_is_connected(s->js)) {
        struct jsonrpc_msg *reply;
        struct json *result;

        result = ovsdb_trigger_steal_result(&t->trigger);
        if (result) {
            reply = jsonrpc_create_reply(result, t->id);
        } else {
            reply = jsonrpc_create_error(json_string_create("canceled"),
                                         t->id);
        }
        jsonrpc_session_send(s->js, reply);
    }

    json_destroy(t->id);
    ovsdb_trigger_destroy(&t->trigger);
    hmap_remove(&s->triggers, &t->hmap_node);
    free(t);
}

static void
ovsdb_jsonrpc_trigger_complete_all(struct ovsdb_jsonrpc_session *s)
{
    struct ovsdb_jsonrpc_trigger *t, *next;
    HMAP_FOR_EACH_SAFE (t, next, hmap_node, &s->triggers) {
        ovsdb_jsonrpc_trigger_complete(t);
    }
}

static void
ovsdb_jsonrpc_trigger_complete_done(struct ovsdb_jsonrpc_session *s)
{
    while (!list_is_empty(&s->up.completions)) {
        struct ovsdb_jsonrpc_trigger *t
            = CONTAINER_OF(s->up.completions.next,
                           struct ovsdb_jsonrpc_trigger, trigger.node);
        ovsdb_jsonrpc_trigger_complete(t);
    }
}

/* JSON-RPC database table monitors. */

enum ovsdb_jsonrpc_monitor_selection {
    OJMS_INITIAL = 1 << 0,      /* All rows when monitor is created. */
    OJMS_INSERT = 1 << 1,       /* New rows. */
    OJMS_DELETE = 1 << 2,       /* Deleted rows. */
    OJMS_MODIFY = 1 << 3        /* Modified rows. */
};

/* A particular column being monitored. */
struct ovsdb_jsonrpc_monitor_column {
    const struct ovsdb_column *column;
    enum ovsdb_jsonrpc_monitor_selection select;
};

/* A particular table being monitored. */
struct ovsdb_jsonrpc_monitor_table {
    const struct ovsdb_table *table;

    /* This is the union (bitwise-OR) of the 'select' values in all of the
     * members of 'columns' below. */
    enum ovsdb_jsonrpc_monitor_selection select;

    /* Columns being monitored. */
    struct ovsdb_jsonrpc_monitor_column *columns;
    size_t n_columns;
};

/* A collection of tables being monitored. */
struct ovsdb_jsonrpc_monitor {
    struct ovsdb_replica replica;
    struct ovsdb_jsonrpc_session *session;
    struct ovsdb *db;
    struct hmap_node node;      /* In ovsdb_jsonrpc_session's "monitors". */

    struct json *monitor_id;
    struct shash tables;     /* Holds "struct ovsdb_jsonrpc_monitor_table"s. */
};

static const struct ovsdb_replica_class ovsdb_jsonrpc_replica_class;

struct ovsdb_jsonrpc_monitor *ovsdb_jsonrpc_monitor_find(
    struct ovsdb_jsonrpc_session *, const struct json *monitor_id);
static void ovsdb_jsonrpc_monitor_destroy(struct ovsdb_replica *);
static struct json *ovsdb_jsonrpc_monitor_get_initial(
    const struct ovsdb_jsonrpc_monitor *);
static size_t ovsdb_jsonrpc_monitor_json_length(
    const struct ovsdb_jsonrpc_monitor *);

static bool
parse_bool(struct ovsdb_parser *parser, const char *name, bool default_value)
{
    const struct json *json;

    json = ovsdb_parser_member(parser, name, OP_BOOLEAN | OP_OPTIONAL);
    return json ? json_boolean(json) : default_value;
}

struct ovsdb_jsonrpc_monitor *
ovsdb_jsonrpc_monitor_find(struct ovsdb_jsonrpc_session *s,
                           const struct json *monitor_id)
{
    struct ovsdb_jsonrpc_monitor *m;

    HMAP_FOR_EACH_WITH_HASH (m, node, json_hash(monitor_id, 0), &s->monitors) {
        if (json_equal(m->monitor_id, monitor_id)) {
            return m;
        }
    }

    return NULL;
}

static void
ovsdb_jsonrpc_add_monitor_column(struct ovsdb_jsonrpc_monitor_table *mt,
                                 const struct ovsdb_column *column,
                                 enum ovsdb_jsonrpc_monitor_selection select,
                                 size_t *allocated_columns)
{
    struct ovsdb_jsonrpc_monitor_column *c;

    if (mt->n_columns >= *allocated_columns) {
        mt->columns = x2nrealloc(mt->columns, allocated_columns,
                                 sizeof *mt->columns);
    }

    c = &mt->columns[mt->n_columns++];
    c->column = column;
    c->select = select;
}

static int
compare_ovsdb_jsonrpc_monitor_column(const void *a_, const void *b_)
{
    const struct ovsdb_jsonrpc_monitor_column *a = a_;
    const struct ovsdb_jsonrpc_monitor_column *b = b_;

    return a->column < b->column ? -1 : a->column > b->column;
}

static struct ovsdb_error * WARN_UNUSED_RESULT
ovsdb_jsonrpc_parse_monitor_request(struct ovsdb_jsonrpc_monitor_table *mt,
                                    const struct json *monitor_request,
                                    size_t *allocated_columns)
{
    const struct ovsdb_table_schema *ts = mt->table->schema;
    enum ovsdb_jsonrpc_monitor_selection select;
    const struct json *columns, *select_json;
    struct ovsdb_parser parser;
    struct ovsdb_error *error;

    ovsdb_parser_init(&parser, monitor_request, "table %s", ts->name);
    columns = ovsdb_parser_member(&parser, "columns", OP_ARRAY | OP_OPTIONAL);
    select_json = ovsdb_parser_member(&parser, "select",
                                      OP_OBJECT | OP_OPTIONAL);
    error = ovsdb_parser_finish(&parser);
    if (error) {
        return error;
    }

    if (select_json) {
        select = 0;
        ovsdb_parser_init(&parser, select_json, "table %s select", ts->name);
        if (parse_bool(&parser, "initial", true)) {
            select |= OJMS_INITIAL;
        }
        if (parse_bool(&parser, "insert", true)) {
            select |= OJMS_INSERT;
        }
        if (parse_bool(&parser, "delete", true)) {
            select |= OJMS_DELETE;
        }
        if (parse_bool(&parser, "modify", true)) {
            select |= OJMS_MODIFY;
        }
        error = ovsdb_parser_finish(&parser);
        if (error) {
            return error;
        }
    } else {
        select = OJMS_INITIAL | OJMS_INSERT | OJMS_DELETE | OJMS_MODIFY;
    }
    mt->select |= select;

    if (columns) {
        size_t i;

        if (columns->type != JSON_ARRAY) {
            return ovsdb_syntax_error(columns, NULL,
                                      "array of column names expected");
        }

        for (i = 0; i < columns->u.array.n; i++) {
            const struct ovsdb_column *column;
            const char *s;

            if (columns->u.array.elems[i]->type != JSON_STRING) {
                return ovsdb_syntax_error(columns, NULL,
                                          "array of column names expected");
            }

            s = columns->u.array.elems[i]->u.string;
            column = shash_find_data(&mt->table->schema->columns, s);
            if (!column) {
                return ovsdb_syntax_error(columns, NULL, "%s is not a valid "
                                          "column name", s);
            }
            ovsdb_jsonrpc_add_monitor_column(mt, column, select,
                                             allocated_columns);
        }
    } else {
        struct shash_node *node;

        SHASH_FOR_EACH (node, &ts->columns) {
            const struct ovsdb_column *column = node->data;
            if (column->index != OVSDB_COL_UUID) {
                ovsdb_jsonrpc_add_monitor_column(mt, column, select,
                                                 allocated_columns);
            }
        }
    }

    return NULL;
}

static struct json *
ovsdb_jsonrpc_monitor_create(struct ovsdb_jsonrpc_session *s, struct ovsdb *db,
                             struct json *params)
{
    struct ovsdb_jsonrpc_monitor *m = NULL;
    struct json *monitor_id, *monitor_requests;
    struct ovsdb_error *error = NULL;
    struct shash_node *node;
    struct json *json;

    if (json_array(params)->n != 3) {
        error = ovsdb_syntax_error(params, NULL, "invalid parameters");
        goto error;
    }
    monitor_id = params->u.array.elems[1];
    monitor_requests = params->u.array.elems[2];
    if (monitor_requests->type != JSON_OBJECT) {
        error = ovsdb_syntax_error(monitor_requests, NULL,
                                   "monitor-requests must be object");
        goto error;
    }

    if (ovsdb_jsonrpc_monitor_find(s, monitor_id)) {
        error = ovsdb_syntax_error(monitor_id, NULL, "duplicate monitor ID");
        goto error;
    }

    m = xzalloc(sizeof *m);
    ovsdb_replica_init(&m->replica, &ovsdb_jsonrpc_replica_class);
    ovsdb_add_replica(db, &m->replica);
    m->session = s;
    m->db = db;
    hmap_insert(&s->monitors, &m->node, json_hash(monitor_id, 0));
    m->monitor_id = json_clone(monitor_id);
    shash_init(&m->tables);

    SHASH_FOR_EACH (node, json_object(monitor_requests)) {
        const struct ovsdb_table *table;
        struct ovsdb_jsonrpc_monitor_table *mt;
        size_t allocated_columns;
        const struct json *mr_value;
        size_t i;

        table = ovsdb_get_table(m->db, node->name);
        if (!table) {
            error = ovsdb_syntax_error(NULL, NULL,
                                       "no table named %s", node->name);
            goto error;
        }

        mt = xzalloc(sizeof *mt);
        mt->table = table;
        shash_add(&m->tables, table->schema->name, mt);

        /* Parse columns. */
        mr_value = node->data;
        allocated_columns = 0;
        if (mr_value->type == JSON_ARRAY) {
            const struct json_array *array = &mr_value->u.array;

            for (i = 0; i < array->n; i++) {
                error = ovsdb_jsonrpc_parse_monitor_request(
                    mt, array->elems[i], &allocated_columns);
                if (error) {
                    goto error;
                }
            }
        } else {
            error = ovsdb_jsonrpc_parse_monitor_request(
                mt, mr_value, &allocated_columns);
            if (error) {
                goto error;
            }
        }

        /* Check for duplicate columns. */
        qsort(mt->columns, mt->n_columns, sizeof *mt->columns,
              compare_ovsdb_jsonrpc_monitor_column);
        for (i = 1; i < mt->n_columns; i++) {
            if (mt->columns[i].column == mt->columns[i - 1].column) {
                error = ovsdb_syntax_error(mr_value, NULL, "column %s "
                                           "mentioned more than once",
                                           mt->columns[i].column->name);
                goto error;
            }
        }
    }

    return ovsdb_jsonrpc_monitor_get_initial(m);

error:
    if (m) {
        ovsdb_remove_replica(m->db, &m->replica);
    }

    json = ovsdb_error_to_json(error);
    ovsdb_error_destroy(error);
    return json;
}

static struct jsonrpc_msg *
ovsdb_jsonrpc_monitor_cancel(struct ovsdb_jsonrpc_session *s,
                             struct json_array *params,
                             const struct json *request_id)
{
    if (params->n != 1) {
        return jsonrpc_create_error(json_string_create("invalid parameters"),
                                    request_id);
    } else {
        struct ovsdb_jsonrpc_monitor *m;

        m = ovsdb_jsonrpc_monitor_find(s, params->elems[0]);
        if (!m) {
            return jsonrpc_create_error(json_string_create("unknown monitor"),
                                        request_id);
        } else {
            ovsdb_remove_replica(m->db, &m->replica);
            return jsonrpc_create_reply(json_object_create(), request_id);
        }
    }
}

static void
ovsdb_jsonrpc_monitor_remove_all(struct ovsdb_jsonrpc_session *s)
{
    struct ovsdb_jsonrpc_monitor *m, *next;

    HMAP_FOR_EACH_SAFE (m, next, node, &s->monitors) {
        ovsdb_remove_replica(m->db, &m->replica);
    }
}

/* Returns an overestimate of the number of bytes of JSON data required to
 * report the current contents of the database over all the monitors currently
 * configured in 's'.  */
static size_t
ovsdb_jsonrpc_monitor_json_length_all(struct ovsdb_jsonrpc_session *s)
{
    struct ovsdb_jsonrpc_monitor *m;
    size_t length;

    length = 0;
    HMAP_FOR_EACH (m, node, &s->monitors) {
        length += ovsdb_jsonrpc_monitor_json_length(m);
    }
    return length;
}

static struct ovsdb_jsonrpc_monitor *
ovsdb_jsonrpc_monitor_cast(struct ovsdb_replica *replica)
{
    ovs_assert(replica->class == &ovsdb_jsonrpc_replica_class);
    return CONTAINER_OF(replica, struct ovsdb_jsonrpc_monitor, replica);
}

struct ovsdb_jsonrpc_monitor_aux {
    bool initial;               /* Sending initial contents of table? */
    const struct ovsdb_jsonrpc_monitor *monitor;
    struct json *json;          /* JSON for the whole transaction. */

    /* Current table.  */
    struct ovsdb_jsonrpc_monitor_table *mt;
    struct json *table_json;    /* JSON for table's transaction. */
};

static bool
any_reportable_change(const struct ovsdb_jsonrpc_monitor_table *mt,
                      const unsigned long int *changed)
{
    size_t i;

    for (i = 0; i < mt->n_columns; i++) {
        const struct ovsdb_jsonrpc_monitor_column *c = &mt->columns[i];
        unsigned int idx = c->column->index;

        if (c->select & OJMS_MODIFY && bitmap_is_set(changed, idx)) {
            return true;
        }
    }

    return false;
}

static bool
ovsdb_jsonrpc_monitor_change_cb(const struct ovsdb_row *old,
                                const struct ovsdb_row *new,
                                const unsigned long int *changed,
                                void *aux_)
{
    struct ovsdb_jsonrpc_monitor_aux *aux = aux_;
    const struct ovsdb_jsonrpc_monitor *m = aux->monitor;
    struct ovsdb_table *table = new ? new->table : old->table;
    enum ovsdb_jsonrpc_monitor_selection type;
    struct json *old_json, *new_json;
    struct json *row_json;
    char uuid[UUID_LEN + 1];
    size_t i;

    if (!aux->mt || table != aux->mt->table) {
        aux->mt = shash_find_data(&m->tables, table->schema->name);
        aux->table_json = NULL;
        if (!aux->mt) {
            /* We don't care about rows in this table at all.  Tell the caller
             * to skip it.  */
            return false;
        }
    }

    type = (aux->initial ? OJMS_INITIAL
            : !old ? OJMS_INSERT
            : !new ? OJMS_DELETE
            : OJMS_MODIFY);
    if (!(aux->mt->select & type)) {
        /* We don't care about this type of change (but do want to be called
         * back for changes to other rows in the same table). */
        return true;
    }

    if (type == OJMS_MODIFY && !any_reportable_change(aux->mt, changed)) {
        /* Nothing of interest changed. */
        return true;
    }

    old_json = new_json = NULL;
    if (type & (OJMS_DELETE | OJMS_MODIFY)) {
        old_json = json_object_create();
    }
    if (type & (OJMS_INITIAL | OJMS_INSERT | OJMS_MODIFY)) {
        new_json = json_object_create();
    }
    for (i = 0; i < aux->mt->n_columns; i++) {
        const struct ovsdb_jsonrpc_monitor_column *c = &aux->mt->columns[i];
        const struct ovsdb_column *column = c->column;
        unsigned int idx = c->column->index;

        if (!(type & c->select)) {
            /* We don't care about this type of change for this particular
             * column (but we will care about it for some other column). */
            continue;
        }

        if ((type == OJMS_MODIFY && bitmap_is_set(changed, idx))
            || type == OJMS_DELETE) {
            json_object_put(old_json, column->name,
                            ovsdb_datum_to_json(&old->fields[idx],
                                                &column->type));
        }
        if (type & (OJMS_INITIAL | OJMS_INSERT | OJMS_MODIFY)) {
            json_object_put(new_json, column->name,
                            ovsdb_datum_to_json(&new->fields[idx],
                                                &column->type));
        }
    }

    /* Create JSON object for transaction overall. */
    if (!aux->json) {
        aux->json = json_object_create();
    }

    /* Create JSON object for transaction on this table. */
    if (!aux->table_json) {
        aux->table_json = json_object_create();
        json_object_put(aux->json, aux->mt->table->schema->name,
                        aux->table_json);
    }

    /* Create JSON object for transaction on this row. */
    row_json = json_object_create();
    if (old_json) {
        json_object_put(row_json, "old", old_json);
    }
    if (new_json) {
        json_object_put(row_json, "new", new_json);
    }

    /* Add JSON row to JSON table. */
    snprintf(uuid, sizeof uuid,
             UUID_FMT, UUID_ARGS(ovsdb_row_get_uuid(new ? new : old)));
    json_object_put(aux->table_json, uuid, row_json);

    return true;
}

/* Returns an overestimate of the number of bytes of JSON data required to
 * report the current contents of the database over monitor 'm'. */
static size_t
ovsdb_jsonrpc_monitor_json_length(const struct ovsdb_jsonrpc_monitor *m)
{
    const struct shash_node *node;
    size_t length;

    /* Top-level overhead of monitor JSON. */
    length = 256;

    SHASH_FOR_EACH (node, &m->tables) {
        const struct ovsdb_jsonrpc_monitor_table *mt = node->data;
        const struct ovsdb_table *table = mt->table;
        const struct ovsdb_row *row;
        size_t i;

        /* Per-table JSON overhead: "<table>":{...}. */
        length += strlen(table->schema->name) + 32;

        /* Per-row JSON overhead: ,"<uuid>":{"old":{...},"new":{...}} */
        length += hmap_count(&table->rows) * (UUID_LEN + 32);

        /* Per-row, per-column JSON overhead: ,"<column>": */
        for (i = 0; i < mt->n_columns; i++) {
            const struct ovsdb_jsonrpc_monitor_column *c = &mt->columns[i];
            const struct ovsdb_column *column = c->column;

            length += hmap_count(&table->rows) * (8 + strlen(column->name));
        }

        /* Data. */
        HMAP_FOR_EACH (row, hmap_node, &table->rows) {
            for (i = 0; i < mt->n_columns; i++) {
                const struct ovsdb_jsonrpc_monitor_column *c = &mt->columns[i];
                const struct ovsdb_column *column = c->column;

                length += ovsdb_datum_json_length(&row->fields[column->index],
                                                  &column->type);
            }
        }
    }

    return length;
}

static void
ovsdb_jsonrpc_monitor_init_aux(struct ovsdb_jsonrpc_monitor_aux *aux,
                               const struct ovsdb_jsonrpc_monitor *m,
                               bool initial)
{
    aux->initial = initial;
    aux->monitor = m;
    aux->json = NULL;
    aux->mt = NULL;
    aux->table_json = NULL;
}

static struct ovsdb_error *
ovsdb_jsonrpc_monitor_commit(struct ovsdb_replica *replica,
                             const struct ovsdb_txn *txn,
                             bool durable OVS_UNUSED)
{
    struct ovsdb_jsonrpc_monitor *m = ovsdb_jsonrpc_monitor_cast(replica);
    struct ovsdb_jsonrpc_monitor_aux aux;

    ovsdb_jsonrpc_monitor_init_aux(&aux, m, false);
    ovsdb_txn_for_each_change(txn, ovsdb_jsonrpc_monitor_change_cb, &aux);
    if (aux.json) {
        struct jsonrpc_msg *msg;
        struct json *params;

        params = json_array_create_2(json_clone(aux.monitor->monitor_id),
                                     aux.json);
        msg = jsonrpc_create_notify("update", params);
        jsonrpc_session_send(aux.monitor->session->js, msg);
    }

    return NULL;
}

static struct json *
ovsdb_jsonrpc_monitor_get_initial(const struct ovsdb_jsonrpc_monitor *m)
{
    struct ovsdb_jsonrpc_monitor_aux aux;
    struct shash_node *node;

    ovsdb_jsonrpc_monitor_init_aux(&aux, m, true);
    SHASH_FOR_EACH (node, &m->tables) {
        struct ovsdb_jsonrpc_monitor_table *mt = node->data;

        if (mt->select & OJMS_INITIAL) {
            struct ovsdb_row *row;

            HMAP_FOR_EACH (row, hmap_node, &mt->table->rows) {
                ovsdb_jsonrpc_monitor_change_cb(NULL, row, NULL, &aux);
            }
        }
    }
    return aux.json ? aux.json : json_object_create();
}

static void
ovsdb_jsonrpc_monitor_destroy(struct ovsdb_replica *replica)
{
    struct ovsdb_jsonrpc_monitor *m = ovsdb_jsonrpc_monitor_cast(replica);
    struct shash_node *node;

    json_destroy(m->monitor_id);
    SHASH_FOR_EACH (node, &m->tables) {
        struct ovsdb_jsonrpc_monitor_table *mt = node->data;
        free(mt->columns);
        free(mt);
    }
    shash_destroy(&m->tables);
    hmap_remove(&m->session->monitors, &m->node);
    free(m);
}

static const struct ovsdb_replica_class ovsdb_jsonrpc_replica_class = {
    ovsdb_jsonrpc_monitor_commit,
    ovsdb_jsonrpc_monitor_destroy
};
