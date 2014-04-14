/* -*- mode: c; buffer-read-only: t -*- */
#line 2 "./lib/dirs.c.in"
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
#include "dirs.h"
#include <stdlib.h>
#include "util.h"

struct directory {
    const char *value;          /* Actual value; NULL if not yet determined. */
    const char *default_value;  /* Default value. */
    const char *var_name;       /* Environment variable to override default. */
};

static const char *
get_dir(struct directory *d)
{
    if (!d->value) {
        d->value = getenv(d->var_name);
        if (!d->value || !d->value[0]) {
            d->value = d->default_value;
        }
    }
    return d->value;
}

const char *
ovs_sysconfdir(void)
{
    static struct directory d = { NULL, "/usr/local/etc", "OVS_SYSCONFDIR" };
    return get_dir(&d);
}

const char *
ovs_pkgdatadir(void)
{
    static struct directory d = { NULL, "/usr/local/share/openvswitch", "OVS_PKGDATADIR" };
    return get_dir(&d);
}

const char *
ovs_rundir(void)
{
    static struct directory d = { NULL, "/usr/local/var/run/openvswitch", "OVS_RUNDIR" };
    return get_dir(&d);
}

const char *
ovs_logdir(void)
{
    static struct directory d = { NULL, "/usr/local/var/log/openvswitch", "OVS_LOGDIR" };
    return get_dir(&d);
}

const char *
ovs_dbdir(void)
{
    static const char *dbdir;
    if (!dbdir) {
        dbdir = getenv("OVS_DBDIR");
        if (!dbdir || !dbdir[0]) {
            char *sysconfdir = getenv("OVS_SYSCONFDIR");

            dbdir = (sysconfdir
                     ? xasprintf("%s/openvswitch", sysconfdir)
                     : "/usr/local/etc/openvswitch");
        }
    }
    return dbdir;
}

const char *
ovs_bindir(void)
{
    static struct directory d = { NULL, "/usr/local/bin", "OVS_BINDIR" };
    return get_dir(&d);
}
