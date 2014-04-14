#ifndef IF_H
#define IF_H 1
#include <winsock2.h>
#include <sys/uio.h>
#define IFNAMSIZ        16
#define IFALIASZ        256
# define IFF_RUNNING    0x40
struct ifmap
{
    unsigned long int mem_start;
    unsigned long int mem_end;
    unsigned short int base_addr;
    unsigned char irq;
    unsigned char dma;
    unsigned char port;
    /* 3 bytes spare */
};
typedef struct {
    unsigned short encoding;
    unsigned short parity;
} raw_hdlc_proto;
typedef struct {
    unsigned int t391;
    unsigned int t392;
    unsigned int n391;
    unsigned int n392;
    unsigned int n393;
    unsigned short lmi;
    unsigned short dce; /* 1 for DCE (network side) operation */
} fr_proto;
typedef struct {
    unsigned int dlci;
} fr_proto_pvc;          /* for creating/deleting FR PVCs */

typedef struct {
    unsigned int dlci;
    char master[IFNAMSIZ];  /* Name of master FRAD device */
}fr_proto_pvc_info;             /* for returning PVC information only */

typedef struct {
    unsigned int interval;
    unsigned int timeout;
} cisco_proto;
typedef struct {
    unsigned int clock_rate; /* bits per second */
    unsigned int clock_type; /* internal, external, TX-internal etc. */
    unsigned short loopback;
} sync_serial_settings;          /* V.35, V.24, X.21 */

typedef struct {
    unsigned int clock_rate; /* bits per second */
    unsigned int clock_type; /* internal, external, TX-internal etc. */
    unsigned short loopback;
    unsigned int slot_map;
} te1_settings;                  /* T1, E1 */

struct if_settings
{
    unsigned int type;      /* Type of physical device or protocol */
    unsigned int size;      /* Size of the data allocated by the caller */
    union {
        /* {atm/eth/dsl}_settings anyone ? */
        raw_hdlc_proto          *raw_hdlc;
        cisco_proto             *cisco;
        fr_proto                *fr;
        fr_proto_pvc            *fr_pvc;
        fr_proto_pvc_info       *fr_pvc_info;

        /* interface settings */
        sync_serial_settings    *sync;
        te1_settings            *te1;
    } ifs_ifsu;
};

struct ifreq
{
#define IFHWADDRLEN     6
    union
    {
        char    ifrn_name[IFNAMSIZ];            /* if name, e.g. "en0" */
    } ifr_ifrn;

    union {
        struct  sockaddr ifru_addr;
        struct  sockaddr ifru_dstaddr;
        struct  sockaddr ifru_broadaddr;
        struct  sockaddr ifru_netmask;
        struct  sockaddr ifru_hwaddr;
        short   ifru_flags;
        int     ifru_ivalue;
        int     ifru_mtu;
        struct  ifmap ifru_map;
        char    ifru_slave[IFNAMSIZ];   /* Just fits the size */
        char    ifru_newname[IFNAMSIZ];
        void *  ifru_data;
        struct  if_settings ifru_settings;
    } ifr_ifru;
};
# define ifr_name       ifr_ifrn.ifrn_name      /* interface name       */
# define ifr_hwaddr     ifr_ifru.ifru_hwaddr    /* MAC address          */
# define ifr_addr       ifr_ifru.ifru_addr      /* address              */
# define ifr_dstaddr    ifr_ifru.ifru_dstaddr   /* other end of p-p lnk */
# define ifr_broadaddr  ifr_ifru.ifru_broadaddr /* broadcast address    */
# define ifr_netmask    ifr_ifru.ifru_netmask   /* interface net mask   */
# define ifr_flags      ifr_ifru.ifru_flags     /* flags                */
# define ifr_metric     ifr_ifru.ifru_ivalue    /* metric               */
# define ifr_mtu        ifr_ifru.ifru_mtu       /* mtu                  */
# define ifr_map        ifr_ifru.ifru_map       /* device map           */
# define ifr_slave      ifr_ifru.ifru_slave     /* slave device         */
# define ifr_data       ifr_ifru.ifru_data      /* for use by interface */
# define ifr_ifindex    ifr_ifru.ifru_ivalue    /* interface index      */
# define ifr_bandwidth  ifr_ifru.ifru_ivalue    /* link bandwidth       */
# define ifr_qlen       ifr_ifru.ifru_ivalue    /* queue length         */
# define ifr_newname    ifr_ifru.ifru_newname   /* New name             */
#define IFF_TAP                0x0002
#define IFF_NO_PI      0x1000
#define TUNSETIFF     _IOW('T', 202, int)

struct sockaddr_ll
{
    unsigned short  sll_family;
    __be16          sll_protocol;
    int             sll_ifindex;
    unsigned short  sll_hatype;
    unsigned char   sll_pkttype;
    unsigned char   sll_halen;
    unsigned char   sll_addr[8];
};
#define PF_PACKET       17      /* Packet family.  */
#define AF_PACKET       PF_PACKET
#define SIOCGIFTXQLEN 0x8942          /* Get the tx queue length      */
#define SIOCSIFMTU      0x8922          /* set MTU size                 */
#define SIOCGIFMTU    0x8921          /* get MTU size                 */
#define SIOCGMIIPHY   0x8947          /* Get address of MII PHY in use. */
#define SIOCGMIIREG   0x8948          /* Read MII PHY register.       */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
/* This should work for both 32 and 64 bit userland. */
struct ethtool_cmd {
    __u32   cmd;
    __u32   supported;      /* Features this interface supports */
    __u32   advertising;    /* Features this interface advertises */
    __u16   speed;          /* The forced speed, 10Mb, 100Mb, gigabit */
    __u8    duplex;         /* Duplex, half or full */
    __u8    port;           /* Which connector port */
    __u8    phy_address;
    __u8    transceiver;    /* Which transceiver to use */
    __u8    autoneg;        /* Enable or disable autonegotiation */
    __u8    mdio_support;
    __u32   maxtxpkt;       /* Tx pkts before generating tx int */
    __u32   maxrxpkt;       /* Rx pkts before generating rx int */
    __u16   speed_hi;
    __u8    eth_tp_mdix;
    __u8    reserved2;
    __u32   lp_advertising; /* Features the link partner advertises */
    __u32   reserved[2];
};
#define SOPASS_MAX      6
/* wake-on-lan settings */
struct ethtool_wolinfo {
    __u32   cmd;
    __u32   supported;
    __u32   wolopts;
    __u8    sopass[SOPASS_MAX]; /* SecureOn(tm) password */
};

/* for passing single values */
struct ethtool_value {
    __u32   cmd;
    __u32   data;
};

/* for passing big chunks of data */
struct ethtool_regs {
    __u32   cmd;
    __u32   version; /* driver-specific, indicates different chips/revs */
    __u32   len; /* bytes */
    __u8    data[0];
};

/* for passing EEPROM chunks */
struct ethtool_eeprom {
    __u32   cmd;
    __u32   magic;
    __u32   offset; /* in bytes */
    __u32   len; /* in bytes */
    __u8    data[0];
};

/* Indicates what features are advertised by the interface. */
#define ADVERTISED_10baseT_Half         (1 << 0)
#define ADVERTISED_10baseT_Full         (1 << 1)
#define ADVERTISED_100baseT_Half        (1 << 2)
#define ADVERTISED_100baseT_Full        (1 << 3)
#define ADVERTISED_1000baseT_Half       (1 << 4)
#define ADVERTISED_1000baseT_Full       (1 << 5)
#define ADVERTISED_Autoneg              (1 << 6)
#define ADVERTISED_TP                   (1 << 7)
#define ADVERTISED_AUI                  (1 << 8)
#define ADVERTISED_MII                  (1 << 9)
#define ADVERTISED_FIBRE                (1 << 10)
#define ADVERTISED_BNC                  (1 << 11)
#define ADVERTISED_10000baseT_Full      (1 << 12)
#define ADVERTISED_Pause                (1 << 13)
#define ADVERTISED_Asym_Pause           (1 << 14)
#define ADVERTISED_2500baseX_Full       (1 << 15)
#define ADVERTISED_Backplane            (1 << 16)
#define ADVERTISED_1000baseKX_Full      (1 << 17)
#define ADVERTISED_10000baseKX4_Full    (1 << 18)
#define ADVERTISED_10000baseKR_Full     (1 << 19)
#define ADVERTISED_10000baseR_FEC       (1 << 20)
#define ADVERTISED_20000baseMLD2_Full   (1 << 21)
#define ADVERTISED_20000baseKR2_Full    (1 << 22)

/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */
#define SPEED_10                10
#define SPEED_100               100
#define SPEED_1000              1000
#define SPEED_2500              2500
#define SPEED_10000             10000
#define SPEED_UNKNOWN           -1

/* Duplex, half or full. */
#define DUPLEX_HALF             0x00
#define DUPLEX_FULL             0x01
#define DUPLEX_UNKNOWN          0xff

/* Which connector port. */
#define PORT_TP                 0x00
#define PORT_AUI                0x01
#define PORT_MII                0x02
#define PORT_FIBRE              0x03
#define PORT_BNC                0x04
#define PORT_DA                 0x05
#define PORT_NONE               0xef
#define PORT_OTHER              0xff

/* Which transceiver to use. */
#define XCVR_INTERNAL           0x00
#define XCVR_EXTERNAL           0x01
#define XCVR_DUMMY1             0x02
#define XCVR_DUMMY2             0x03
#define XCVR_DUMMY3             0x04

/* Enable or disable autonegotiation.  If this is set to enable,
* the forced link modes above are completely ignored.
*/
#define AUTONEG_DISABLE         0x00
#define AUTONEG_ENABLE          0x01

/* Mode MDI or MDI-X */
#define ETH_TP_MDI_INVALID      0x00
#define ETH_TP_MDI              0x01
#define ETH_TP_MDI_X            0x02
/* Socket configuration controls. */
#define SIOCGIFNAME     0x8910          /* get iface name               */
#define SIOCSIFLINK     0x8911          /* set iface channel            */
#define SIOCGIFCONF     0x8912          /* get iface list               */
#define SIOCGIFFLAGS    0x8913          /* get flags                    */
#define SIOCSIFFLAGS    0x8914          /* set flags                    */
#define SIOCGIFADDR     0x8915          /* get PA address               */
#define SIOCSIFADDR     0x8916          /* set PA address               */
#define SIOCGIFDSTADDR  0x8917          /* get remote PA address        */
#define SIOCSIFDSTADDR  0x8918          /* set remote PA address        */
#define SIOCGIFBRDADDR  0x8919          /* get broadcast PA address     */
#define SIOCSIFBRDADDR  0x891a          /* set broadcast PA address     */
#define SIOCGIFNETMASK  0x891b          /* get network PA mask          */
#define SIOCSIFNETMASK  0x891c          /* set network PA mask          */
#define SIOCGIFMETRIC   0x891d          /* get metric                   */
#define SIOCSIFMETRIC   0x891e          /* set metric                   */
#define SIOCGIFMEM      0x891f          /* get memory address (BSD)     */
#define SIOCSIFMEM      0x8920          /* set memory address (BSD)     */
#define SIOCGIFMTU      0x8921          /* get MTU size                 */
#define SIOCSIFMTU      0x8922          /* set MTU size                 */
#define SIOCSIFNAME     0x8923          /* set interface name */
#define SIOCSIFHWADDR   0x8924          /* set hardware address         */
#define SIOCGIFENCAP    0x8925          /* get/set encapsulations       */
#define SIOCSIFENCAP    0x8926
#define SIOCGIFHWADDR   0x8927          /* Get hardware address         */
#define SIOCGIFSLAVE    0x8929          /* Driver slaving support       */
#define SIOCSIFSLAVE    0x8930
#define SIOCADDMULTI    0x8931          /* Multicast address lists      */
#define SIOCDELMULTI    0x8932
#define SIOCGIFINDEX    0x8933          /* name -> if_index mapping     */
#define SIOGIFINDEX     SIOCGIFINDEX    /* misprint compatibility :-)   */
#define SIOCSIFPFLAGS   0x8934          /* set/get extended flags set   */
#define SIOCGIFPFLAGS   0x8935
#define SIOCDIFADDR     0x8936          /* delete PA address            */
#define SIOCSIFHWBROADCAST      0x8937  /* set hardware broadcast addr  */
#define SIOCGIFCOUNT    0x8938          /* get number of devices */
/* Routing table calls. */
#define SIOCADDRT       0x890B          /* add routing table entry      */
#define SIOCDELRT       0x890C          /* delete routing table entry   */
#define SIOCRTMSG       0x890D          /* call to routing system       */
#define IFF_PROMISC     0x100
#define SIOCGARP      0x8954          /* get ARP table entry          */
#define SIOCETHTOOL     0x8946          /* Ethtool interface            */

#define SIOCGMIIPHY     0x8947          /* Get address of MII PHY in use. */
#define SIOCGMIIREG     0x8948          /* Read MII PHY register.       */
#define SIOCSMIIREG     0x8949          /* Write MII PHY register.      */

#define SIOCWANDEV      0x894A          /* get/set netdev parameters    */
/* The struct should be in sync with struct net_device_stats */
struct rtnl_link_stats
{
    __u32   rx_packets;             /* total packets received       */
    __u32   tx_packets;             /* total packets transmitted    */
    __u32   rx_bytes;               /* total bytes received         */
    __u32   tx_bytes;               /* total bytes transmitted      */
    __u32   rx_errors;              /* bad packets received         */
    __u32   tx_errors;              /* packet transmit problems     */
    __u32   rx_dropped;             /* no space in linux buffers    */
    __u32   tx_dropped;             /* no space available in linux  */
    __u32   multicast;              /* multicast packets received   */
    __u32   collisions;

    /* detailed rx_errors: */
    __u32   rx_length_errors;
    __u32   rx_over_errors;         /* receiver ring buff overflow  */
    __u32   rx_crc_errors;          /* recved pkt with crc error    */
    __u32   rx_frame_errors;        /* recv'd frame alignment error */
    __u32   rx_fifo_errors;         /* recv'r fifo overrun          */
    __u32   rx_missed_errors;       /* receiver missed packet       */

    /* detailed tx_errors */
    __u32   tx_aborted_errors;
    __u32   tx_carrier_errors;
    __u32   tx_fifo_errors;
    __u32   tx_heartbeat_errors;
    __u32   tx_window_errors;

    /* for cslip etc */
    __u32   rx_compressed;
    __u32   tx_compressed;
};
#endif