/* This structure gets passed by the SIOCADDRT and SIOCDELRT calls. */
struct rtentry
{
    unsigned long int rt_pad1;
    struct sockaddr rt_dst;             /* Target address.  */
    struct sockaddr rt_gateway;         /* Gateway addr (RTF_GATEWAY).  */
    struct sockaddr rt_genmask;         /* Target network mask (IP).  */
    unsigned short int rt_flags;
    short int rt_pad2;
    unsigned long int rt_pad3;
    unsigned char rt_tos;
    unsigned char rt_class;
#if __WORDSIZE == 64
    short int rt_pad4[3];
#else
    short int rt_pad4;
#endif
    short int rt_metric;                /* +1 for binary compatibility!  */
    char *rt_dev;                       /* Forcing the device at add.  */
    unsigned long int rt_mtu;           /* Per route MTU/Window.  */
    unsigned long int rt_window;        /* Window clamping.  */
    unsigned short int rt_irtt;         /* Initial RTT.  */
};
#define RTF_UP          0x0001          /* Route usable.  */
#define RTF_GATEWAY     0x0002          /* Destination is a gateway.  */

#define RTF_HOST        0x0004          /* Host entry (net otherwise).  */
#define RTF_REINSTATE   0x0008          /* Reinstate route after timeout.  */
#define RTF_DYNAMIC     0x0010          /* Created dyn. (by redirect).  */
#define RTF_MODIFIED    0x0020          /* Modified dyn. (by redirect).  */
#define RTF_MTU         0x0040          /* Specific MTU for this route.  */
#define RTF_MSS         RTF_MTU         /* Compatibility.  */
#define RTF_WINDOW      0x0080          /* Per route window clamping.  */
#define RTF_IRTT        0x0100          /* Initial round trip time.  */
#define RTF_REJECT      0x0200          /* Reject route.  */
#define RTF_STATIC      0x0400          /* Manually injected route.  */
#define RTF_XRESOLVE    0x0800          /* External resolver.  */
#define RTF_NOFORWARD   0x1000          /* Forwarding inhibited.  */
#define RTF_THROW       0x2000          /* Go to next class.  */
#define RTF_NOPMTUDISC  0x4000          /* Do not send packets with DF.  */

/* for IPv6 */
#define RTF_DEFAULT     0x00010000      /* default - learned via ND     */
#define RTF_ALLONLINK   0x00020000      /* fallback, no routers on link */
#define RTF_ADDRCONF    0x00040000      /* addrconf route - RA          */

#define RTF_LINKRT      0x00100000      /* link specific - device match */
#define RTF_NONEXTHOP   0x00200000      /* route with no nexthop        */

#define RTF_CACHE       0x01000000      /* cache entry                  */
#define RTF_FLOW        0x02000000      /* flow significant route       */
#define RTF_POLICY      0x04000000      /* policy route                 */

#define RTCF_VALVE      0x00200000
#define RTCF_MASQ       0x00400000
