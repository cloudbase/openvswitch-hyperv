#ifndef _UNISTD_H
#define _UNISTD_H


#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <sys/uio.h>
#include <getopt.h>
#include <process.h>
#include <dirent.h>
#include <direct.h>
#include <linux/rtnetlink.h>
#include <sys/resource.h>
#include <config.h>

#define srandom srand
#define random rand

#define R_OK    4     
#define W_OK    2      
#define F_OK    0   

#define access _access
#define ftruncate _chsize

#define ssize_t int

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
typedef __int8            int8_t;
typedef __int16           int16_t; 
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;

/*PID RELATED*/
typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
//typedef int sigset_t;
#define pgoff_t __int64

#define fseeko(stream, offset, origin) _fseeki64(stream, offset, origin)
#define ftello(stream) _ftelli64(stream)


/* File type and permission flags for stat() */
#if defined(_MSC_VER)  &&  !defined(S_IREAD)
# define S_IFMT   _S_IFMT                      /* file type mask */
# define S_IFDIR  _S_IFDIR                     /* directory */
# define S_IFCHR  _S_IFCHR                     /* character device */
# define S_IFFIFO _S_IFFIFO                    /* pipe */
# define S_IFREG  _S_IFREG                     /* regular file */
# define S_IREAD  _S_IREAD                     /* read permission */
# define S_IWRITE _S_IWRITE                    /* write permission */
# define S_IEXEC  _S_IEXEC                     /* execute permission */
#endif
#define S_IFBLK   0                            /* block device */
#define S_IFLNK   0                            /* link */
#define S_IFSOCK  0                            /* socket */

#define _DIRENT_HAVE_D_TYPE

#define IFTODT(mode) ((mode) & S_IFMT)
#define DTTOIF(type) (type)

 /*
  * IPC defines
  */
 #undef HAVE_UNION_SEMUN
 #define HAVE_UNION_SEMUN 1
 
  #define IPC_RMID 256
  #define IPC_CREAT 512
  #define IPC_EXCL 1024
  #define IPC_PRIVATE 234564
  #define IPC_NOWAIT 2048
  #define IPC_STAT 4096
  
  #define EACCESS 2048
    
  #define SETALL 8192
  #define GETNCNT 16384
  #define GETVAL 65536
  #define SETVAL 131072
  #define GETPID 262144
  
  
  #define WIFEXITED(w) (((w) & 0XFFFFFF00) == 0)
  #define WIFSIGNALED(w) (!WIFEXITED(w))
  #define WEXITSTATUS(w) (w)
  #define WTERMSIG(w) (w)
  
  #define sigmask(sig) ( 1 << ((sig)-1) )
  

  #define SIGHUP 1
  #define SIGQUIT 3
  #define SIGTRAP 5
  #define SIGABRT 22 
  #define SIGKILL 9
  #define SIGPIPE 13
  #define SIGALRM 14
  #define SIGBUS 22
  #define SIGXCPU 22
  #define SIGXFSZ 22
  #define SIGSTOP 17
  #define SIGTSTP 18
  #define SIGCONT 19
  #define SIGCHLD 20
  #define SIGTTIN 21
  #define SIGTTOU 22 
  #define SIGWINCH 28
  #define SIGUSR1 30
  #define SIGUSR2 31
#define SIG_BLOCK 22
#define SIG_SETMASK 22
# define SA_RESTART   0x10000000 /* Restart syscall on signal return.  */

#define timer_t LONGLONG
#define timer_c LARGE_INTEGER

typedef struct {
    int si_signo;
    int si_code;
    int si_value;
    int si_errno;
    pid_t si_pid;
    uid_t si_uid;
    void *si_addr;
    int si_status;
    int si_band;
} siginfo_t;

struct sigaction {
    void(*sa_handler)(int);
    void(*sa_sigaction)(int, siginfo_t *, void *);
    sigset_t   sa_mask;
    int        sa_flags;
    void(*sa_restorer)(void);
};

#define lstat(path, sb) stat((path), (sb))

#define fsync(fd) _commit(fd)
extern char* strsep(char** stringp, const char* delim);

#define _PC_PATH_MAX 4096
#define     _POSIX_PIPE_BUF         512
#define      WNOHANG         1
#define      SA_NOCLDSTOP  1
#define _SC_UIO_MAXIOV                  2
#define      _XOPEN_IOV_MAX  _POSIX_UIO_MAXIOV
#define _POSIX_UIO_MAXIOV  16
#define AF_NETLINK      PF_NETLINK
#define PF_NETLINK      16
#define SO_RCVBUFFORCE   33
#define MSG_DONTWAIT MSG_DONTROUTE
extern unsigned int getpagesize(void);
extern long sysconf(int name);
#define _SC_PAGESIZE            0x1
#define _SC_NPROCESSORS_ONLN    0x2
#define _SC_PHYS_PAGES            0x4
#define getcwd _getcwd
extern int getloadavg(double *, int);

#define gmtime_r(now, tm) _gmtime32_s(tm, now)
inline gid_t getgid() {return 0;}
inline uid_t getuid() {return 0;}
inline unsigned int sleep(unsigned int seconds) { Sleep(seconds * 1000);return 0;}
extern int getppid();
extern pid_t fork(void);
extern int kill(pid_t pid, int sig);
extern BOOL link(char*, char*);
extern long pathconf(char *, int);
extern char *strsignal(int sig);
extern int sigismember(const sigset_t *set, int signo);
extern int sigfillset(sigset_t *set);
extern int sigemptyset(sigset_t *set);
extern int sigdelset(sigset_t *set, int signo);
extern int sigaddset(sigset_t *set, int signo);
extern int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
extern int sigprocmask(int signum, const struct sigaction *act, struct sigaction *oldact);

#endif 