#include <sys/wait.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
int __pipe(__pipedes)
int __pipedes[2];
{
    if (__pipedes == NULL)
    {
        _set_errno(EINVAL);
        return -1;
    }

    return _pipe(__pipedes, 0, _O_BINARY | _O_NOINHERIT);
}

pid_t
waitpid(pid_t pid, int *stat_loc, int options)
{
    HANDLE hProcess;
    DWORD status;

    if ((options & ~(WNOHANG | WUNTRACED)) != 0) {
        _set_errno(EINVAL);
        return (pid_t)-1;
    }
    if ((pid == -1) || (pid == -2)) {
        errno = ECHILD;
        return -1;
    }
    hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (hProcess == NULL) {
        _set_errno(EINVAL);
        return -1;
    }
    if (WaitForSingleObject(hProcess, (DWORD)(-1L)) == WAIT_FAILED) {
        _set_errno(EINVAL);
        return -1;
    }
    if (!GetExitCodeProcess(hProcess, &status)) {
        _set_errno(EINVAL);
        return -1;
    }
    if (!stat_loc)
        *stat_loc = status;
    CloseHandle(hProcess);
    return pid;
}