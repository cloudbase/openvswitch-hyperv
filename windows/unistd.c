#include <unistd.h>
int kill(pid_t pid, int sig)
{
    int res = 0;
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

    if (hProcess == NULL) {
        _set_errno(ENOSYS);
        return -1;
    }
    switch (sig) {
    case SIGABRT:
    case SIGKILL:
        if (!TerminateProcess(hProcess, -1)) {
            _set_errno(ENOSYS);
            res = -1;
        }
        break;
    case 0:
        break;
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGALRM:
    case SIGTERM:
    default:
        _set_errno(EINVAL);
        res = -1;
        break;
    }
    CloseHandle(hProcess);
    return res;
}

int GetNumLogicalProcessors(void)
{
    SYSTEM_INFO info_temp;
    GetSystemInfo(&info_temp);
    long int n_cores = info_temp.dwNumberOfProcessors;
    return n_cores;
}

long sysconf(int name)
{
    long val = -1;
    long val2 = -1;
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX status;

    switch (name)
    {
    case _SC_NPROCESSORS_ONLN:
        val = GetNumLogicalProcessors();
        break;

    case _SC_PAGESIZE:
        GetSystemInfo(&sysInfo);
        val = sysInfo.dwPageSize;
        break;

    case _SC_PHYS_PAGES:
        status.dwLength = sizeof(status);
        val2 = sysconf(_SC_PAGESIZE);
        if (GlobalMemoryStatusEx(&status) && val2 != -1)
            val = status.ullTotalPhys / val2;
        break;
    default:
        break;
    }

    return val;
}


long pathconf(char* smth, int n)
    {
    return _MAX_PATH;
    }

char* strsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}

pid_t fork(void)
{
return 0;
}