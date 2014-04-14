#ifndef UIO_H
#define UIO_H 1

#include <inttypes.h>
#include <unistd.h>
#include <WS2tcpip.h>
#include <windefs.h>



struct iovec
    {
    u_long   iov_len;
    char FAR *iov_base;
    };

ssize_t readv(int fildes, const struct iovec *iov, int iovcnt);
ssize_t writev(int fildes, const struct iovec *iov, int iovcnt);
typedef int socklen_t;
/* Structure describing messages sent by
`sendmsg' and received by `recvmsg'.  */
struct msghdr
{
    void *msg_name;             /* Address to send to/receive from.  */
    socklen_t msg_namelen;      /* Length of address data.  */

    struct iovec *msg_iov;      /* Vector of data to send/receive into.  */
    size_t msg_iovlen;          /* Number of elements in the vector.  */

    void* msg_control;          /* Ancillary data (eg BSD filedesc passing). */
    size_t msg_controllen;      /* Ancillary data buffer length.
                //                !! The type should be socklen_t but the
                        //        definition of the kernel is incompatible
                        //        with this.  */

    int msg_flags;              /* Flags on received message.  */

//    LPSOCKADDR msg_name;
    //INT        msg_namelen;
    //LPWSABUF   msg_iov;
//    DWORD      msg_iovlen;
//    WSABUF     msg_control;
//    DWORD      msg_flags;
};
//#define msg_controllen msg_control->len

#define WSA_CMSG_FIRSTHDR(mhdr) \
  ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr)                 \
   ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) 0)
#define WSA_CMSG_NXTHDR(msg, cmsg)                          \
    ( ((cmsg) == NULL)                                      \
        ? WSA_CMSG_FIRSTHDR(msg)                            \
        : ( ( ((PUCHAR)(cmsg) +                             \
                    WSA_CMSGHDR_ALIGN((cmsg)->cmsg_len) +   \
                    sizeof(WSACMSGHDR) ) >                  \
                (PUCHAR)((msg)->msg_control) +              \
                    (msg)->msg_controllen )                    \
            ? (LPWSACMSGHDR)NULL                            \
            : (LPWSACMSGHDR)((PUCHAR)(cmsg) +               \
                WSA_CMSGHDR_ALIGN((cmsg)->cmsg_len)) ) )
                
extern int socketpair(int domain, int type, int protocol, SOCKET socks[2]);

#endif