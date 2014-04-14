/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define to 1 to enable time caching, to 0 to disable time caching, or leave
   undefined to use the default (as one should ordinarily do). */
/* #undef CACHE_TIME */

/* Define to 1 if building on ESX. */
/* #undef ESX */

/* Define to 1 if you have backtrace(3). */
#define HAVE_BACKTRACE 1

/* Define to 1 if you have the `getloadavg' function. */
#define HAVE_GETLOADAVG 1

/* Define to 1 if net/if_dl.h is available. */
/* #undef HAVE_IF_DL */

/* Define to 1 if net/if_packet.h is available. */
#define HAVE_IF_PACKET 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the <linux/if_ether.h> header file. */
#define HAVE_LINUX_IF_ETHER_H 1

/* Define to 1 if you have the <linux/types.h> header file. */
#define HAVE_LINUX_TYPES_H 1

/* Define to 1 if you have __malloc_hook, __realloc_hook, and __free_hook in
   <malloc.h>. */
#define HAVE_MALLOC_HOOKS 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mlockall' function. */
#define HAVE_MLOCKALL 1

/* Define to 1 if you have the <mntent.h> header file. */
#define HAVE_MNTENT_H 1

/* Define to 1 if Netlink protocol is available. */
#define HAVE_NETLINK 0

/* Define to 1 if OpenSSL is installed. */
/* #undef HAVE_OPENSSL */

/* Define to 1 if you have the `setmntent' function. */
#define HAVE_SETMNTENT 1

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strnlen' function. */
#define HAVE_STRNLEN 1

/* Define to 1 if you have the `strsignal' function. */
#define HAVE_STRSIGNAL 1

/* Define if strtok_r macro segfaults on some inputs */
/* #undef HAVE_STRTOK_R_BUG */

/* Define to 1 if `st_mtimensec' is a member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_MTIMENSEC */

/* Define to 1 if `st_mtim.tv_nsec' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the <valgrind/valgrind.h> header file. */
/* #undef HAVE_VALGRIND_VALGRIND_H */

/* System uses the linux datapath module. */
#define LINUX_DATAPATH 1

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Name of package */
#define PACKAGE "openvswitch"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "ovs-bugs@openvswitch.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "openvswitch"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "openvswitch 1.11.1"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "openvswitch"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.11.1"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if the compiler support putting variables into sections with
   user-defined names and the linker automatically defines __start_SECNAME and
   __stop_SECNAME symbols that designate the start and end of the section. */
#define USE_LINKER_SECTIONS 0

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Version number of package */
#define VERSION "1.11.1"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */


#if defined(_WIN32)
#pragma warning( disable : 4116 4090 4700 4005 4133 4028 4098 4293 4715 4047)
#pragma runtime_checks( "", off )
#pragma warning( disable : 4996 ) //deprecated functions
#pragma warning( disable : 4244 ) //possible loss of data
#pragma warning( disable : 4146 ) //unary minus operator applied to unsigned type, result still unsigned
#pragma warning( disable : 4018 ) //'>=' : signed/unsigned mismatch
#include <WinSock2.h>
#define ffs __lzcnt
#define inline __inline
#define strtok_r strtok_s
#define __func__ __FUNCTION__
#define u_int8_t uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t
#define u_int64_t uint64_t
typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int  __u32;
typedef unsigned long long __u64;
#ifndef ssize_t
#define ssize_t int
#endif 

#ifdef __CHECKER__ 
#if !defined(_WIN32)
#define __bitwise__ __attribute__((bitwise))
#endif
#else
#ifndef __bitwise__
#define __bitwise__
#endif
#endif
#ifdef __CHECK_ENDIAN__
#ifndef __bitwise
#define __bitwise __bitwise__
#endif
#else
#ifndef __bitwise
#define __bitwise
#endif
#endif

typedef unsigned short __bitwise __le16;
typedef unsigned short __bitwise __be16;

typedef unsigned int __bitwise __le32;
typedef unsigned int __bitwise __be32;

typedef unsigned long long __bitwise __le64;
typedef unsigned long long __bitwise __be64;
#define snprintf _snprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp 
#undef USE_LINKER_SECTIONS
#undef DELETE
#undef HAVE_BACKTRACE
typedef unsigned long int sigset_t;
#undef HAVE_MALLOC_HOOKS

#define LOG_EMERG   1
#define LOG_ALERT   1
#define LOG_CRIT    1
#define LOG_ERR     4
#define LOG_WARNING 5
#define LOG_NOTICE  6
#define LOG_INFO    6
#define LOG_DEBUG   6
#define LOG_NDELAY  6
#define LOG_DAEMON  6
#undef HAVE_EXECINFO_H
#undef VLOG_MODULE
#define caddr_t char*
#define _WINSOCKAPI_
#undef HAVE_STRUCT_STAT_ST_MTIM_TV_NSEC
#undef HAVE_MNTENT_H
#undef HAVE_SYS_STATVFS_H
#undef HAVE_STATVFS
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment (lib, "MsWSock.Lib")
#pragma comment (lib, "WS2_32.Lib")
#endif


#if defined(_WIN32)
#define PACKED_STRUCT __pragma(pack(push, 1)) 
#define END_PACKED_STRUCT __pragma(pack(pop))
#else
#define PACKED_STRUCT 
#define END_PACKED_STRUCT __attribute__((packed))
#endif

#if _MSC_VER >= 1400
//# include <sal.h>
# if _MSC_VER > 1400
#  define FORMAT_STRING(p) _Printf_format_string_ p
# else
#  define FORMAT_STRING(p) __format_string p
# endif /* FORMAT_STRING */
#else
# define FORMAT_STRING(p) p
#endif /* _MSC_VER */

#ifdef _WIN32
#define __UNICODE__
#ifndef WCOREDUMP	
#define WCOREDUMP(status) ((status) & 0x80)
#endif

#undef WEXITSTATUS
#define WEXITSTATUS(status) (((status)  & 0xff00) >> 8)
#undef WIFEXITED
#define WIFEXITED(status) (WTERMSIG(status) == 0)
#undef WIFSTOPPED
#define WIFSTOPPED(status) (((status) & 0xff) == 0x7f)
#undef WIFSIGNALED
#define WIFSIGNALED(status) (!WIFSTOPPED(status) && !WIFEXITED(status))
#undef WSTOPSIG
#define WSTOPSIG(status) WEXITSTATUS(status)
#undef WTERMSIG
#define WTERMSIG(status) ((status) & 0x7f)
#undef WCOREDUMP
#define WCOREDUMP(status) ((status) & 0x80)
#endif