#ifndef INET_H
#define INET_H 1
#include <WS2tcpip.h>

inline int inet_aton(const char *host_name, struct in_addr *address)
    {
        return inet_pton(AF_INET, host_name, address);
    }
#endif