#ifndef TIME_WIN_H
#define TIME_WIN_H 1
#include <time.h> /* Adding time_t definition.  */
#include <WinSock2.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef __clockid_t_defined
typedef int clockid_t;
#define __clockid_t_defined
#endif  /* __clockid_t_defined */

#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME                  0
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC                 1
#endif

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED
struct timespec {
    time_t  tv_sec;       /* Seconds */
    long    tv_nsec;      /* Nanoseconds */
};

struct itimerspec {
    struct timespec  it_interval; /* Timer period */
    struct timespec  it_value;    /* Timer expiration */
};
#endif  /* _TIMESPEC_DEFINED */

int nanosleep(const struct timespec *request, struct timespec *remain);

int clock_getres(clockid_t clk_id, struct timespec *res);

int clock_gettime(clockid_t clk_id, struct timespec *tp);

int clock_settime(clockid_t clk_id, const struct timespec *tp);

int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

extern int gettimeofday(struct timeval* tp, void* tzp);

#ifdef __cplusplus
}
#endif
#endif