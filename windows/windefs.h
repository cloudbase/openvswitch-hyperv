#ifndef WINDEFS_H
#define WINDEFS_H 1

//#include <unistd.h>
#include <signal.h>
//#include <Windows.h>
//#include <tlhelp32.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment (lib, "MsWSock.Lib")
#pragma comment (lib, "WS2_32.Lib")

#pragma warning( disable : 4116 4090 4700 4005 4133 4028 4098 4293 4715 4047)
#pragma runtime_checks( "", off )
#pragma warning( disable : 4996 ) //deprecated functions
#pragma warning( disable : 4244 ) //possible loss of data
#pragma warning( disable : 4146 ) //unary minus operator applied to unsigned type, result still unsigned
#pragma warning( disable : 4018 ) //'>=' : signed/unsigned mismatch
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

#endif /* windefs.h */
