/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012 Nicira, Inc.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "command-line.h"
#include "daemon.h"
#include "dirs.h"
#include "dynamic-string.h"
#include "jsonrpc.h"
#include "process.h"
#include "timeval.h"
#include "unixctl.h"
#include "util.h"

static void usage(void);
static const char *parse_command_line(int argc, char *argv[]);
static struct jsonrpc *connect_to_target(const char *target);

int
main(int argc, char *argv[])
{
    char *cmd_result, *cmd_error;
    struct jsonrpc *client;
    char *cmd, **cmd_argv;
    const char *target;
    int cmd_argc;
    int error;

    set_program_name(argv[0]);

    /* Parse command line and connect to target. */
    target = parse_command_line(argc, argv);
    client = connect_to_target(target);

    /* Transact request and process reply. */
    cmd = argv[optind++];
    cmd_argc = argc - optind;
    cmd_argv = cmd_argc ? argv + optind : NULL;
    error = unixctl_client_transact(client, cmd, cmd_argc, cmd_argv,
                                    &cmd_result, &cmd_error);
    if (error) {
        ovs_fatal(error, "%s: transaction error", target);
    }

    if (cmd_error) {
        fputs(cmd_error, stderr);
        ovs_error(0, "%s: server returned an error", target);
        exit(2);
    } else if (cmd_result) {
        fputs(cmd_result, stdout);
    } else {
        NOT_REACHED();
    }

    jsonrpc_close(client);
    free(cmd_result);
    free(cmd_error);
    return 0;
}

static void
usage(void)
{
    printf("\
%s, for querying and controlling Open vSwitch daemon\n\
usage: %s [TARGET] COMMAND [ARG...]\n\
Targets:\n\
  -t, --target=TARGET  pidfile or socket to contact\n\
Common commands:\n\
  help               List commands supported by the target\n\
  version            Print version of the target\n\
  vlog/list          List current logging levels\n\
  vlog/set [SPEC]\n\
      Set log levels as detailed in SPEC, which may include:\n\
      A valid module name (all modules, by default)\n\
      'syslog', 'console', 'file' (all facilities, by default))\n\
      'off', 'emer', 'err', 'warn', 'info', or 'dbg' ('dbg', bydefault)\n\
  vlog/reopen        Make the program reopen its log file\n\
Other options:\n\
  --timeout=SECS     wait at most SECS seconds for a response\n\
  -h, --help         Print this helpful information\n\
  -V, --version      Display ovs-appctl version information\n",
           program_name, program_name);
    exit(EXIT_SUCCESS);
}

static const char *
parse_command_line(int argc, char *argv[])
{
    static const struct option long_options[] = {
        {"target", required_argument, NULL, 't'},
        {"execute", no_argument, NULL, 'e'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'V'},
        {"timeout", required_argument, NULL, 'T'},
        {NULL, 0, NULL, 0},
    };
    const char *target;
    int e_options;

    target = NULL;
    e_options = 0;
    for (;;) {
        int option;

        option = getopt_long(argc, argv, "+t:hVe", long_options, NULL);
        if (option == -1) {
            break;
        }
        switch (option) {
        case 't':
            if (target) {
                ovs_fatal(0, "-t or --target may be specified only once");
            }
            target = optarg;
            break;

        case 'e':
            /* We ignore -e for compatibility.  Older versions specified the
             * command as the argument to -e.  Since the current version takes
             * the command as non-option arguments and we say that -e has no
             * arguments, this just works in the common case. */
            if (e_options++) {
                ovs_fatal(0, "-e or --execute may be speciifed only once");
            }
            break;

        case 'h':
            usage();
            break;

        case 'T':
            time_alarm(atoi(optarg));
            break;

        case 'V':
            ovs_print_version(0, 0);
            exit(EXIT_SUCCESS);

        case '?':
            exit(EXIT_FAILURE);

        default:
            NOT_REACHED();
        }
    }

    if (optind >= argc) {
        ovs_fatal(0, "at least one non-option argument is required "
                  "(use --help for help)");
    }

    return target ? target : "ovs-vswitchd";
}

static struct jsonrpc *
connect_to_target(const char *target)
{
    struct jsonrpc *client;
    char *socket_name;
    int error;

    if (target[0] != '/') {
        char *pidfile_name;
        pid_t pid;

        pidfile_name = xasprintf("%s/%s.pid", ovs_rundir(), target);
        pid = read_pidfile(pidfile_name);
        if (pid < 0) {
            ovs_fatal(-pid, "cannot read pidfile \"%s\"", pidfile_name);
        }
        free(pidfile_name);
        socket_name = xasprintf("%s/%s.%ld.ctl",
                                ovs_rundir(), target, (long int) pid);
    } else {
        socket_name = xstrdup(target);
    }

    error = unixctl_client_create(socket_name, &client);
    if (error) {
        ovs_fatal(error, "cannot connect to \"%s\"", socket_name);
    }
    free(socket_name);

    return client;
}

