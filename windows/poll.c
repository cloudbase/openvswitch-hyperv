#include <poll.h>
int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
    int rc;
    rc = WSAPoll(fds, nfds, timeout);
    return rc;
}