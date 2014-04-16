#include "Interface.h"

#include <stdio.h>

BOOL Socket_Send(SOCKET s, const VOID* data, ULONG length, UINT32 targetIp, UINT16 targetPort)
{
	BOOL result;
	BOOL retryOnReset = TRUE;

	SOCKADDR_IN targetAddress;
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_addr.S_un.S_addr = targetIp;
	//server_address.sin_addr.S_un.S_un_b = inet_addr(sIP.data());
	targetAddress.sin_port = htons(targetPort);

try_again_size:
	result = sendto(s, (const char*)data, length, 0, (SOCKADDR*)&targetAddress, sizeof(targetAddress));
#if __PRINT_TRANSFER_INFO
	printf("sent: %d out of %d\n", result, length);
#endif

	if (SOCKET_ERROR == result)
	{
		DWORD dwError = WSAGetLastError();
		printf("send() size failed: %u\n", dwError);

		if (dwError == WSAETIMEDOUT)
		{
			printf("warning: send msg failed with error: %u; trying again...\n", dwError);
			goto try_again_size;
		}

		else if (dwError == WSAECONNRESET)
		{
			printf("warning: send msg failed with error: %u\n", dwError);
			return FALSE;

#if 0
			if (retryOnReset)
			{
				//s = RemoteIo_Reset(s);

				if (INVALID_SOCKET != s) {
					goto try_again_size;
				}
			}
#endif
		}

		//return FALSE;
		goto try_again_size;
	}

	if (result >= 0 && (ULONG)result < length)
	{
		data = (BYTE*)data + result;
		length -= result;
	}

	return TRUE;
}

BOOL Socket_Recv(SOCKET s, VOID* data, ULONG length, UINT32 targetIp, UINT16 targetPort)
{
	int result;
	BOOL retryOnReset = TRUE;

	SOCKADDR_IN targetAddress;
	targetAddress.sin_family = AF_INET;
	targetAddress.sin_addr.S_un.S_addr = targetIp;
	//server_address.sin_addr.S_un.S_un_b = inet_addr(sIP.data());
	targetAddress.sin_port = htons(targetPort);

	int targetAddrLen = sizeof(targetAddress);
	int toRead = 0;
	BOOL getOut = FALSE;
	int attemptNo = 0;

try_again_rcv_size:
	while (toRead == 0 && !getOut)
	{
		result = ioctlsocket(s, FIONREAD, &toRead);
		if (toRead > 0)
			break;

		++attemptNo;
		Sleep(50);

		if (attemptNo >= 40)
		{
			char ch;

			printf("waited for a long time: retry send (r) or ignore (i)?\n");
			ch = getchar();

			if (ch == 'r')
				getOut = TRUE;
			else
				attemptNo = 0;
		}
	}

	if (!toRead)
		return FALSE;

	//result = recv(s, data, length, 0);
	result = recvfrom(s, data, length, 0, (SOCKADDR*)&targetAddress, &targetAddrLen);

#if __PRINT_TRANSFER_INFO
	printf("recv: %d out of %d\n", result, length);
#endif

	if (result == 0) {
		printf("recv returns 0. last error: %d\n", WSAGetLastError());
		Sleep(100);
		goto try_again_rcv_size;
	}

	if (result == SOCKET_ERROR)
	{
		DWORD dwError = WSAGetLastError();
		//Sleep(2000);

		if (dwError == WSAETIMEDOUT)
		{
			printf("warning: recv msg failed with error: %u; press enter to retry...\n", dwError);
			getchar();
			goto try_again_rcv_size;
		}

		else if (dwError == WSAECONNRESET)
		{
			printf("warning: recv msg failed with error: %u\n", dwError);
			Sleep(1000);
			goto try_again_rcv_size;

#if 0
			if (retryOnReset)
			{
				//s = RemoteIo_Reset(s);

				if (INVALID_SOCKET != s) {
					goto try_again_rcv_size;
				}
			}
#endif
		}

		else
		{
			printf("recv msg failed with error: %u\n", dwError);
			//return FALSE;

			goto try_again_rcv_size;
		}
	}

	else if (result > 0 && (ULONG)result < length)
	{
		data = (BYTE*)data + result;
		length -= result;
		goto try_again_rcv_size;
	}

	return TRUE;
}