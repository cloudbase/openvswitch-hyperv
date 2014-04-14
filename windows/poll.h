#include <Winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <config.h>
#include <sys/time.h>
#include <time.h>
typedef int nfds_t;

extern int poll(struct pollfd fds[], nfds_t nfds, int timeout);