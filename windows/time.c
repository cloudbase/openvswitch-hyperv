#include <config.h>
#include <unistd.h>
#include <sys/time.h>

#include "netlink.h"

#if __USE_REMOTE_IO_NL_DEVICE
#undef __USE_REMOTE_IO_NL_DEVICE
#endif

#if __USE_REMOTE_IO_NL_DEVICE
#include <assert.h>
#endif
int gettimeofday(struct timeval* tp, void* tzp)
{
    FILETIME fileTime;
    uint64_t unix_time, windows_time;
    const uint64_t MILLISECONDS_BETWEEN_1601_AND_1970 = 11644473600000;

    /* Ignore the timezone parameter */
    (void)tzp;

    /*
    * Windows time is stored as the number 100 ns intervals since January 1 1601.
    * Conversion details from http://www.informit.com/articles/article.aspx?p=102236&seqNum=3
    * Its precision is 100 ns but accuracy is only one clock tick, or normally around 15 ms.
    */
    GetSystemTimeAsFileTime(&fileTime);
    windows_time = ((uint64_t)fileTime.dwHighDateTime << 32) + fileTime.dwLowDateTime;
    /* Divide by 10,000 to convert to ms and subtract the time between 1601 and 1970 */
    unix_time = (((windows_time) / 10000) - MILLISECONDS_BETWEEN_1601_AND_1970);
    /* unix_time is now the number of milliseconds since 1970 (the Unix epoch) */
    tp->tv_sec = unix_time / 1000;
    tp->tv_usec = (unix_time % 1000) * 1000;
    return 0;
}
int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
    int rc = 0;

    if (clock_id == CLOCK_MONOTONIC)
    {
        static LARGE_INTEGER freq = { { 0, 0 } };
        LARGE_INTEGER counts;
        uint64_t t;

        QueryPerformanceCounter(&counts);
        if (freq.QuadPart == 0)
            QueryPerformanceFrequency(&freq);


#if __USE_REMOTE_IO_NL_DEVICE
#if defined(_DEBUG) && defined(_WIN32)
		if (!g_initialCounts)
		{
			//remote io: set the g_initialCounts here.
			DebugBreak();

			//must be set manually in the debugger:
			assert(g_initialCounts > 0);
			assert(counts.QuadPart > g_initialCounts);

			//'add' 2 mins to the cur time: we started the userspace ~2 mins after we started the kernel
			counts.QuadPart -= (freq.QuadPart * 2 * 60);

			g_deltaCounts = counts.QuadPart - g_initialCounts;
			counts.QuadPart = g_initialCounts;

			assert(g_deltaCounts > 0);
		}

		else
		{
			assert(g_deltaCounts > 0);
			assert(counts.QuadPart > g_deltaCounts);
			counts.QuadPart -= g_deltaCounts;
		}
#endif
#endif

        tp->tv_sec = counts.QuadPart / freq.QuadPart;
        /* Get the difference between the number of ns stored
        * in 'tv_sec' and that stored in 'counts' */
        t = tp->tv_sec * freq.QuadPart;
        t = counts.QuadPart - t;
        /* 't' now contains the number of cycles since the last second.
        * We want the number of nanoseconds, so multiply out by 1,000,000,000
        * and then divide by the frequency. */
        t *= 1000000000;
        tp->tv_nsec = t / freq.QuadPart;
    }
    else if (clock_id == CLOCK_REALTIME)
    {
        /* clock_gettime(CLOCK_REALTIME,...) is just an alias for gettimeofday with a
        * higher-precision field. */
        struct timeval tv;
        gettimeofday(&tv, NULL);
        tp->tv_sec = tv.tv_sec;
        tp->tv_nsec = tv.tv_usec * 1000;
    }
    else {
        errno = EINVAL;
        rc = -1;
    }

    return rc;
}