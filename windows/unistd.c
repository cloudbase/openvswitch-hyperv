#include <unistd.h>

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