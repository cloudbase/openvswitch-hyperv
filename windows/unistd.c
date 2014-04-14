#include <unistd.h>
#include <tlhelp32.h>

#define SIGINT_MASK    1 
#define SIGILL_MASK    2 
#define SIGFPE_MASK    4 
#define SIGSEGV_MASK   8 
#define SIGTERM_MASK  16 
#define SIGBREAK_MASK 32 
#define SIGABRT_MASK  64 

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


int getppid()
{
    HANDLE hProcess, thProcess;
    PROCESSENTRY32 ProcessEntry;

    thProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (thProcess == INVALID_HANDLE_VALUE) {
        _set_errno(ENOSYS);
        return -1;
    }
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
    ProcessEntry.th32ParentProcessID = 0;
    if (!Process32First(thProcess, &ProcessEntry)) {
        _set_errno(ENOSYS);
        return -1;
    }
    CloseHandle(thProcess);
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, ProcessEntry.th32ProcessID);
    if (hProcess == NULL) {
        _set_errno(ENOSYS);
        return -1;
    }
    CloseHandle(hProcess);
    return ProcessEntry.th32ParentProcessID;
}

pid_t fork(void)
{
return 0;
}

// The sigaddset call adds the individual signal specified to the signal set pointed to by set. 
int sigaddset(sigset_t *set, int signo)
{
    switch (signo)
    {
    case SIGINT:
        *set |= SIGINT_MASK;
        break;
    case SIGILL:
        *set |= SIGILL_MASK;
        break;
    case SIGFPE:
        *set |= SIGFPE_MASK;
        break;
    case SIGSEGV:
        *set |= SIGSEGV_MASK;
        break;
    case SIGTERM:
        *set |= SIGTERM_MASK;
        break;
    case SIGBREAK:
        *set |= SIGBREAK_MASK;
        break;
    case SIGABRT:
    case SIGABRT_COMPAT:
        *set |= SIGABRT_MASK;
        break;
    }

    return 0;
}

// The sigdelset call removes the individual signal specified from the signal set pointed to by set. 
int sigdelset(sigset_t *set, int signo)
{
    switch (signo)
    {
    case SIGINT:
        *set &= ~(DWORD)SIGINT_MASK;
        break;
    case SIGILL:
        *set &= ~(DWORD)SIGILL_MASK;
        break;
    case SIGFPE:
        *set &= ~(DWORD)SIGFPE_MASK;
        break;
    case SIGSEGV:
        *set &= ~(DWORD)SIGSEGV_MASK;
        break;
    case SIGTERM:
        *set &= ~(DWORD)SIGTERM_MASK;
        break;
    case SIGBREAK:
        *set &= ~(DWORD)SIGBREAK_MASK;
        break;
    case SIGABRT:
    case SIGABRT_COMPAT:
        *set &= ~(DWORD)SIGABRT_MASK;
        break;
    }

    return 0;
}


// The sigemptyset call creates a new mask set and excludes all signals from it. 
int sigemptyset(sigset_t *set)
{
    *set = 0;
    return 0;
}

// The sigfillset call creates a new mask set and includes all signals in it. 
int sigfillset(sigset_t *set)
{
    *set = 0xffffffff;
    return 0;
}

// The sigismember call tests the signal mask set pointed to by set for the existence of the specified signal (signo). 
int sigismember(const sigset_t *set, int signo)
{
    switch (signo)
    {
    case SIGINT:
        return *set & SIGINT_MASK;
    case SIGILL:
        return *set & SIGILL_MASK;
    case SIGFPE:
        return *set & SIGFPE_MASK;
    case SIGSEGV:
        return *set & SIGSEGV_MASK;
    case SIGTERM:
        return *set & SIGTERM_MASK;
    case SIGBREAK:
        return *set & SIGBREAK_MASK;
    case SIGABRT:
    case SIGABRT_COMPAT:
        return *set & SIGABRT_MASK;
    }

    return 0;
}

// The strsignal() function returns a string describing the signal number passed in the argument sig. 
char *strsignal(int sig)
{
    switch (sig)
    {
    case SIGINT:
        return "SIGINT";
        break;
    case SIGILL:
        return "SIGILL";
        break;
    case SIGFPE:
        return "SIGFPE";
        break;
    case SIGSEGV:
        return "SIGSEGV";
        break;
    case SIGTERM:
        return "SIGTERM";
        break;
    case SIGBREAK:
        return "SIGBREAK";
        break;
    case SIGABRT:
    case SIGABRT_COMPAT:
        return "SIGABRT";
        break;
    }

    return 0;
}

struct sigaction sigaction_table[NSIG] = { 0 };

int
sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    switch (signum)
    {
    case SIGINT:
    case SIGILL:
    case SIGFPE:
    case SIGSEGV:
    case SIGTERM:
    case SIGBREAK:
    case SIGABRT:
        /* signal is valid, do nothing */
        break;
    default:
        /* signal is invalid */
        errno = EINVAL;
        return 0;
    }

    if (oldact)
    {
        /* save old action */
        oldact->sa_handler = sigaction_table[signum].sa_handler;
        oldact->sa_sigaction = sigaction_table[signum].sa_sigaction;
        oldact->sa_mask = sigaction_table[signum].sa_mask;
        oldact->sa_flags = sigaction_table[signum].sa_flags;
        /*oldact->sa_restorer = sigaction_table[signum].sa_restorer;*/
    }

    if (act)
    {
        /* set new action */
        sigaction_table[signum].sa_handler = act->sa_handler;
        sigaction_table[signum].sa_sigaction = act->sa_sigaction;
        sigaction_table[signum].sa_mask = act->sa_mask;
        sigaction_table[signum].sa_flags = act->sa_flags;
        /*sigaction_table[signum].sa_restorer = act->sa_restorer;*/

        signal(signum, act->sa_handler);
    }

    return 0;
}

int sigprocmask(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    return 0;
}