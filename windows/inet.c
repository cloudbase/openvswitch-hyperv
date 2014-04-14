#include <inet.h>
const char* inet_ntop(int af, const void *restrict src,
    char *restrict dst, socklen_t size)
{
    INT status = SOCKET_ERROR;
    WSADATA wsd;
    char *ret = NULL;

    if (af != AF_INET && af != AF_INET6) {
        errno = EAFNOSUPPORT;
        return NULL;
    }

    WSAStartup(MAKEWORD(2, 2), &wsd);

    if (af == AF_INET) {
        struct sockaddr_in si;
        DWORD len = size;
        memset(&si, 0, sizeof(si));
        si.sin_family = af;
        memcpy(&si.sin_addr, src, sizeof(si.sin_addr));
        status = WSAAddressToString((struct sockaddr*)&si, sizeof(si), NULL, dst, &len);
    }
    else if (af == AF_INET6) {
        struct sockaddr_in6 si6;
        DWORD len = size;
        memset(&si6, 0, sizeof(si6));
        si6.sin6_family = af;
        memcpy(&si6.sin6_addr, src, sizeof(si6.sin6_addr));
        status = WSAAddressToString((struct sockaddr*)&si6, sizeof(si6), NULL, dst, &len);
    }

    if (status != SOCKET_ERROR)
        ret = dst;
    else
        errno = ENOSPC;

    WSACleanup();

    return ret;
}