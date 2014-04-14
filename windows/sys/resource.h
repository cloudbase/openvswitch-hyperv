#ifndef RESOURCE_H
#define RESOURCE_H

#include <time.h>
#include <sys/time.h>
#include <WinSock2.h>
#include <windef.h>
#include <sys/stat.h>
#include <sys/types.h>

extern int getrusage(int who, struct rusage *r_usage);

#define RUSAGE_SELF 0
#define RUSAGE_THREAD 1
#define RUSAGE_SELF         0
#define RUSAGE_CHIL    DREN (-1)
#define RUSAGE_    BOTH     (-2)                /* sys    _wait4() uses th    is */
#define R    USAGE_THREAD   1               /* only the calling thread */

struct  rusage {
        struct timeval ru_utime;        /* user time used */
        struct timeval ru_stime;        /* system time used */
        long    ru_maxrss;              /* maximum resident set size */
        long    ru_ixrss;               /* integral shared memory size */
        long    ru_idrss;               /* integral unshared data size */
        long    ru_isrss;               /* integral unshared stack size */
        long    ru_minflt;              /* page reclaims */
        long    ru_majflt;              /* page faults */
        long    ru_nswap;               /* swaps */
        long    ru_inblock;             /* block input operations */
        long    ru_oublock;             /* block output operations */
        long    ru_msgsnd;              /* messages sent */
        long    ru_msgrcv;              /* messages received */
        long    ru_nsignals;            /* signals received */
        long    ru_nvcsw;               /* voluntary context switches */
        long    ru_nivcsw;              /* involuntary " */
};


struct rlimit {
        unsigned long   rlim_cur;
        unsigned long   rlim_max;
};

#define PRIO_MIN        (-20)
#define PRIO_MAX        20

#define PRIO_PROCESS    0
#define PRIO_PGRP       1
#define PRIO_USER       2


/* Value to indicate that there is no limit.  */
#ifndef __USE_FILE_OFFSET64
# define RLIM_INFINITY ((unsigned long int)(~0UL))
#else
# define RLIM_INFINITY 0xffffffffffffffffuLL
#endif


//int  getpriority(int, id_t);
int  getrlimit(int, struct rlimit *);
int  getrusage(int, struct rusage *);
//int  setpriority(int, id_t, int);
int  setrlimit(int, const struct rlimit *);
/* Largest core file that can be created, in bytes.  */
#define RLIMIT_CORE 4
#define F_GETLK           5
// fcntl flock definitions
#define F_SETLK  8   // Non-Blocking set or clear a lock
#define F_SETLKW 9   // Blocking set or clear a lock
#define F_RDLCK  1   // read lock
#define F_WRLCK  2   // write lock
#define F_UNLCK  3   // remove lock
struct flock {
    short l_type;   // F_RDLCK, F_WRLCK, or F_UNLCK
    short l_whence; // flag to choose starting offset, must be SEEK_SET
    long  l_start;  // relative offset, in bytes, must be 0
    long  l_len;    // length, in bytes; 0 means lock to EOF, must be 0
    short l_pid;    // unused (returned with the unsupported F_GETLK)
    short l_xxx;    // reserved for future use
};
#define F_GETFL          3       /* Get file status flags.  */
#define F_SETFL          4       /* Set file status flags.  */
#define O_NONBLOCK        00004000
#define rlim_t unsigned long
#define RLIMIT_NOFILE         7

typedef int mode_t;

static const mode_t MS_MODE_MASK = 0x0000ffff;           ///< low word
#define SCM_RIGHTS 0x01

extern int my_chmod(const char * path, mode_t mode);
extern int getrlimit(int resource, struct rlimit *rlimits);
extern int fcntl(int fd, int cmd, ...);
#endif