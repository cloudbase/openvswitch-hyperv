#pragma once

#include <Windows.h>

typedef enum _FH_MESSAGE_COMMAND {
	FH_MESSAGE_COMMAND_INVALID,
	FH_MESSAGE_COMMAND_CREATE,
	FH_MESSAGE_COMMAND_WRITE,
	FH_MESSAGE_COMMAND_READ,
	FH_MESSAGE_COMMAND_CLOSE,
}FH_MESSAGE_COMMAND;

typedef struct _FH_MESSAGE {
	UINT cmd;
}FH_MESSAGE, *PFH_MESSAGE;

/*	CreateFile:
-----------------
in:
isAscii: BOOL
str len: ULONG
str

out:
HANDLE
DWORD lastError
*/

typedef struct _FH_MESSAGE_CREATE_IN {
	FH_MESSAGE;
	BOOL isAscii;
	ULONG	strLen;
	const VOID*	fileName;
}FH_MESSAGE_CREATE_IN, *PFH_MESSAGE_CREATE_IN;

#define FH_MESSAGE_CREATE_IN_SIZE_BARE (sizeof(FH_MESSAGE_CREATE_IN) - sizeof(PVOID))

typedef struct _FH_MESSAGE_CREATE_OUT {
	FH_MESSAGE;
	HANDLE	hFile;
	DWORD	dwLastError;
}FH_MESSAGE_CREATE_OUT, *PFH_MESSAGE_CREATE_OUT;

/* WriteFile:
-----------------
in:
HANDLE
size
data
haveOverlapped
[overlapped]

out:
BOOL - result
haveOverlapped
[overlapped]
DWORD lastError
*/

typedef struct _FH_MESSAGE_WRITE_IN {
	FH_MESSAGE;
	HANDLE	hFile;
	ULONG	bufferSize;
	BOOL	haveOverlapped;
	OVERLAPPED overlapped;
	VOID*	buffer;
}FH_MESSAGE_WRITE_IN, *PFH_MESSAGE_WRITE_IN;

#define FH_MESSAGE_WRITE_IN_SIZE_BARE (sizeof(FH_MESSAGE_WRITE_IN) - sizeof(PVOID))

typedef struct _FH_MESSAGE_WRITE_OUT {
	FH_MESSAGE;
	BOOL	ok;
	BOOL	haveOverlapped;
	OVERLAPPED overlapped;
	DWORD	dwLastError;
}FH_MESSAGE_WRITE_OUT, *PFH_MESSAGE_WRITE_OUT;


/* ReadFile:
-----------------
	in:
	HANDLE
	size
	haveOverlapped
	[overlapped]

	out:
	BOOL - result
	haveOverlapped
	[overlapped]
	DWORD lastError
	data
*/


typedef struct _FH_MESSAGE_READ_IN {
	FH_MESSAGE;
	HANDLE	hFile;
	ULONG	bufferSize;
	BOOL	haveOverlapped;
	OVERLAPPED overlapped;
}FH_MESSAGE_READ_IN, *PFH_MESSAGE_READ_IN;

typedef struct _FH_MESSAGE_READ_OUT {
	FH_MESSAGE;
	BOOL	ok;
	ULONG	bytesRead;
	BOOL	haveOverlapped;
	OVERLAPPED overlapped;
	DWORD	dwLastError;
	VOID*	data;
}FH_MESSAGE_READ_OUT, *PFH_MESSAGE_READ_OUT;

#define FH_MESSAGE_READ_OUT_SIZE_BARE (sizeof(FH_MESSAGE_READ_OUT) - sizeof(PVOID))

/* CloseHandle
	in:
	HANDLE

	out:
	BOOL
	DWORD lastError

*/

typedef struct _FH_MESSAGE_CLOSE_IN {
	FH_MESSAGE;
	HANDLE	hFile;
}FH_MESSAGE_CLOSE_IN, *PFH_MESSAGE_CLOSE_IN;

typedef struct _FH_MESSAGE_CLOSE_OUT{
	FH_MESSAGE;
	BOOL	ok;
	DWORD	dwLastError;
}FH_MESSAGE_CLOSE_OUT, *PFH_MESSAGE_CLOSE_OUT;

#define OFFSET_OF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

BOOL Socket_Send(SOCKET s, const VOID* data, ULONG length, UINT32 targetIp, UINT16 targetPort);
BOOL Socket_Recv(SOCKET s, VOID* data, ULONG length, UINT32 targetIp, UINT16 targetPort);

#if 0
SOCKET RemoteIo_Reset(SOCKET s);
#endif