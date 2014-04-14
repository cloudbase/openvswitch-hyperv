/* ARP ioctl request.  */
struct arpreq
{
    struct sockaddr arp_pa;             /* Protocol address.  */
    struct sockaddr arp_ha;             /* Hardware address.  */
    int arp_flags;                      /* Flags.  */
    struct sockaddr arp_netmask;        /* Netmask (only for proxy arps).  */
    char arp_dev[16];
};
/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_NETROM   0               /* From KA9Q: NET/ROM pseudo. */
#define ARPHRD_ETHER    1               /* Ethernet 10/100Mbps.  */
#define ARPHRD_EETHER   2               /* Experimental Ethernet.  */
#define ARPHRD_AX25     3               /* AX.25 Level 2.  */
#define ARPHRD_PRONET   4               /* PROnet token ring.  */
#define ARPHRD_CHAOS    5               /* Chaosnet.  */
#define ARPHRD_IEEE802  6               /* IEEE 802.2 Ethernet/TR/TB.  */
#define ARPHRD_ARCNET   7               /* ARCnet.  */
#define ARPHRD_APPLETLK 8               /* APPLEtalk.  */
#define ARPHRD_DLCI     15              /* Frame Relay DLCI.  */
#define ARPHRD_ATM      19              /* ATM.  */
#define ARPHRD_METRICOM 23              /* Metricom STRIP (new IANA id).  */
#define ARPHRD_IEEE1394 24              /* IEEE 1394 IPv4 - RFC 2734.  */
#define ARPHRD_EUI64            27              /* EUI-64.  */
#define ARPHRD_INFINIBAND       32              /* InfiniBand.  */
