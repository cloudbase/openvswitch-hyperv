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

int fcntl(int fd, int cmd, ...)
{
    va_list a;
    va_start(a, cmd);
    switch (cmd)
    {
    case F_SETLK:
        {
            struct flock *l = va_arg(a, struct flock*);
            switch (l->l_type)
            {
            case F_RDLCK:
                {
                    OVERLAPPED o = { 0 };
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    if (!LockFileEx(h, LOCKFILE_FAIL_IMMEDIATELY, 0, 0, 1, &o)) // read lock
                    {
                        unsigned long x = GetLastError();
                        _set_errno(GetLastError() == ERROR_LOCK_VIOLATION ? EAGAIN : EBADF);
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 1, 1); // write lock
                        }
                break;
            case F_WRLCK:
                {
                    OVERLAPPED o = { 0 };
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    if (!LockFileEx(h, LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 1, &o)) // write lock
                    {
                        unsigned long x = GetLastError();
                        _set_errno(GetLastError() == ERROR_LOCK_VIOLATION ? EDEADLK : EBADF);
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 0, 1); // read lock
                        }
                break;
            case F_UNLCK:
                {
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 0, 1); // read lock
                    UnlockFile(h, 0, 0, 1, 1); // write lock
                        }
                break;
            default:
                _set_errno(ENOTSUP);
                return -1;
            }
                }
        break;
    case F_SETLKW:
        {
            struct flock *l = va_arg(a, struct flock*);
            switch (l->l_type)
            {
            case F_RDLCK:
                {
                    OVERLAPPED o = { 0 };
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    if (!LockFileEx(h, 0, 0, 0, 1, &o)) // read lock
                    {
                        unsigned long x = GetLastError();
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 1, 1); // write lock
                        }
                break;
            case F_WRLCK:
                {
                    OVERLAPPED o = { 0 };
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    if (!LockFileEx(h, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 1, &o)) // write lock
                    {
                        unsigned long x = GetLastError();
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 0, 1); // read lock
                        }
                break;
            case F_UNLCK:
                {
                    struct flock *l = va_arg(a, struct flock*);
                    HANDLE h = (HANDLE)_get_osfhandle(fd);
                    if (l->l_whence != SEEK_SET || l->l_start != 0 || l->l_len != 0)
                    {
                        _set_errno(ENOTSUP);
                        return -1;
                    }
                    UnlockFile(h, 0, 0, 0, 1); // read lock
                    UnlockFile(h, 0, 0, 1, 1); // write lock
                        }
                break;
            default:
                _set_errno(ENOTSUP);
                return -1;
            }
                 }
        break;
    default:
        _set_errno(ENOTSUP);
        return -1;
    }

    return 0;
}