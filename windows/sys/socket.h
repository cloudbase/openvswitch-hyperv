#ifndef SOCKET_WIN_H
#define SOCKET_WIN_H 1

#undef HAVE_NETLINK
#include <winsock2.h>
#include <ws2tcpip.h>
typedef unsigned short sa_family_t;

#endif