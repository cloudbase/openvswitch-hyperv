#include <config.h>
#include <sys/resource.h>
#include <unistd.h>

int getrusage(int who, struct rusage *r_usage)
{
const uint64_t SECONDS_BETWEEN_1601_AND_1970 = 11644473600;
FILETIME cTime, eTime, kTime, uTime;
time_t time;
HANDLE h;

memset(r_usage, 0, sizeof(*r_usage));

if (who == RUSAGE_SELF) {
h = GetCurrentProcess();
GetProcessTimes(h, &cTime, &eTime, &kTime, &uTime);
} else if (who == RUSAGE_THREAD) {
h = GetCurrentThread();
GetThreadTimes(h, &cTime, &eTime, &kTime, &uTime);
} else {
return -1;
}

time = ((uint64_t)uTime.dwHighDateTime << 32) + uTime.dwLowDateTime;
/* Divide by 10,000,000 to get the number of seconds and move the epoch from
* 1601 to 1970 */
time = (time_t)(((time)/10000000) - SECONDS_BETWEEN_1601_AND_1970);
r_usage->ru_utime.tv_sec = time;
/* getrusage() doesn't care about anything other than seconds, so set tv_usec to 0 */
r_usage->ru_utime.tv_usec = 0;
time = ((uint64_t)kTime.dwHighDateTime << 32) + kTime.dwLowDateTime;
/* Divide by 10,000,000 to get the number of seconds and move the epoch from
* 1601 to 1970 */
time = (time_t)(((time)/10000000) - SECONDS_BETWEEN_1601_AND_1970);
r_usage->ru_stime.tv_sec = time;
r_usage->ru_stime.tv_usec = 0;
return 0;
}

int
getrlimit(enum __rlimit_resource resource, struct rlimit *rlimits)
{
    _set_errno(ENOSYS);
    return 0;
}