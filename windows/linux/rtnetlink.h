#ifndef RTNETLINK_H
#define RTNETLINK_H 1

#include <winsock2.h>

enum
{
    IFLA_UNSPEC,
    IFLA_ADDRESS,
    IFLA_BROADCAST,
    IFLA_IFNAME,
    IFLA_MTU,
    IFLA_LINK,
    IFLA_QDISC,
    IFLA_STATS,
    IFLA_COST,
#define IFLA_COST IFLA_COST
    IFLA_PRIORITY,
#define IFLA_PRIORITY IFLA_PRIORITY
    IFLA_MASTER,
#define IFLA_MASTER IFLA_MASTER
    IFLA_WIRELESS,          /* Wireless Extension event - see wireless.h */
#define IFLA_WIRELESS IFLA_WIRELESS
    IFLA_PROTINFO,          /* Protocol specific information for a link */
#define IFLA_PROTINFO IFLA_PROTINFO
    IFLA_TXQLEN,
#define IFLA_TXQLEN IFLA_TXQLEN
    IFLA_MAP,
#define IFLA_MAP IFLA_MAP
    IFLA_WEIGHT,
#define IFLA_WEIGHT IFLA_WEIGHT
    IFLA_OPERSTATE,
    IFLA_LINKMODE,
    IFLA_LINKINFO,
#define IFLA_LINKINFO IFLA_LINKINFO
    IFLA_NET_NS_PID,
    IFLA_IFALIAS,
    IFLA_NUM_VF,            /* Number of VFs if device is SR-IOV PF */
    IFLA_VFINFO_LIST,
    IFLA_STATS64,
    IFLA_VF_PORTS,
    IFLA_PORT_SELF,
    IFLA_AF_SPEC,
    IFLA_GROUP,             /* Group the device belongs to */
    IFLA_NET_NS_FD,
    IFLA_EXT_MASK,          /* Extended info mask, VFs, etc */
    IFLA_PROMISCUITY,       /* Promiscuity count: > 0 means acts PROMISC */
#define IFLA_PROMISCUITY IFLA_PROMISCUITY
    IFLA_NUM_TX_QUEUES,
    IFLA_NUM_RX_QUEUES,
    __IFLA_MAX
};

struct ifinfomsg
{
    unsigned char   ifi_family;
    unsigned char   __ifi_pad;
    unsigned short  ifi_type;               /* ARPHRD_* */
    int             ifi_index;              /* Link index   */
    unsigned        ifi_flags;              /* IFF_* flags  */
    unsigned        ifi_change;             /* IFF_* change mask */
};

#define NETLINK_ROUTE           0       /* Routing/device hook                          */
#define NETLINK_UNUSED          1       /* Unused number                                */
#define NETLINK_USERSOCK        2       /* Reserved for user mode socket protocols      */
#define NETLINK_FIREWALL        3       /* Firewalling hook                             */
#define NETLINK_INET_DIAG       4       /* INET socket monitoring                       */
#define NETLINK_NFLOG           5       /* netfilter/iptables ULOG */
#define NETLINK_XFRM            6       /* ipsec */
#define NETLINK_SELINUX         7       /* SELinux event notifications */
#define NETLINK_ISCSI           8       /* Open-iSCSI */
#define NETLINK_AUDIT           9       /* auditing */
#define NETLINK_FIB_LOOKUP      10
#define NETLINK_CONNECTOR       11
#define NETLINK_NETFILTER       12      /* netfilter subsystem */
#define NETLINK_IP6_FW          13
#define NETLINK_DNRTMSG         14      /* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT  15      /* Kernel messages to userspace */
#define NETLINK_GENERIC         16
/* leave room for NETLINK_DM (DM Events) */
#define NETLINK_SCSITRANSPORT   18      /* SCSI Transports */
#define NETLINK_ECRYPTFS        19
#define NETLINK_RDMA            20

#define MAX_LINKS 32

enum rtnetlink_groups {
    RTNLGRP_NONE,
#define RTNLGRP_NONE            RTNLGRP_NONE
    RTNLGRP_LINK,
#define RTNLGRP_LINK            RTNLGRP_LINK
    RTNLGRP_NOTIFY,
#define RTNLGRP_NOTIFY          RTNLGRP_NOTIFY
    RTNLGRP_NEIGH,
#define RTNLGRP_NEIGH           RTNLGRP_NEIGH
    RTNLGRP_TC,
#define RTNLGRP_TC              RTNLGRP_TC
    RTNLGRP_IPV4_IFADDR,
#define RTNLGRP_IPV4_IFADDR     RTNLGRP_IPV4_IFADDR
    RTNLGRP_IPV4_MROUTE,
#define RTNLGRP_IPV4_MROUTE     RTNLGRP_IPV4_MROUTE
    RTNLGRP_IPV4_ROUTE,
#define RTNLGRP_IPV4_ROUTE      RTNLGRP_IPV4_ROUTE
    RTNLGRP_IPV4_RULE,
#define RTNLGRP_IPV4_RULE       RTNLGRP_IPV4_RULE
    RTNLGRP_IPV6_IFADDR,
#define RTNLGRP_IPV6_IFADDR     RTNLGRP_IPV6_IFADDR
    RTNLGRP_IPV6_MROUTE,
#define RTNLGRP_IPV6_MROUTE     RTNLGRP_IPV6_MROUTE
    RTNLGRP_IPV6_ROUTE,
#define RTNLGRP_IPV6_ROUTE      RTNLGRP_IPV6_ROUTE
    RTNLGRP_IPV6_IFINFO,
#define RTNLGRP_IPV6_IFINFO     RTNLGRP_IPV6_IFINFO
    RTNLGRP_DECnet_IFADDR,
#define RTNLGRP_DECnet_IFADDR   RTNLGRP_DECnet_IFADDR
    RTNLGRP_NOP2,
    RTNLGRP_DECnet_ROUTE,
#define RTNLGRP_DECnet_ROUTE    RTNLGRP_DECnet_ROUTE
    RTNLGRP_DECnet_RULE,
#define RTNLGRP_DECnet_RULE     RTNLGRP_DECnet_RULE
    RTNLGRP_NOP4,
    RTNLGRP_IPV6_PREFIX,
#define RTNLGRP_IPV6_PREFIX     RTNLGRP_IPV6_PREFIX
    RTNLGRP_IPV6_RULE,
#define RTNLGRP_IPV6_RULE       RTNLGRP_IPV6_RULE
    RTNLGRP_ND_USEROPT,
#define RTNLGRP_ND_USEROPT      RTNLGRP_ND_USEROPT
    RTNLGRP_PHONET_IFADDR,
#define RTNLGRP_PHONET_IFADDR   RTNLGRP_PHONET_IFADDR
    RTNLGRP_PHONET_ROUTE,
#define RTNLGRP_PHONET_ROUTE    RTNLGRP_PHONET_ROUTE
    RTNLGRP_DCB,
#define RTNLGRP_DCB             RTNLGRP_DCB
    __RTNLGRP_MAX
};
#define NLM_F_CREATE    0x400   /* Create, if it does not exist */
#define RTM_NEWLINK     16
#define RTM_NEWROUTE 24
#define RTM_GETLINK 0
enum rtattr_type_t
{
    RTA_UNSPEC,
    RTA_DST,
    RTA_SRC,
    RTA_IIF,
    RTA_OIF,
    RTA_GATEWAY,
    RTA_PRIORITY,
    RTA_PREFSRC,
    RTA_METRICS,
    RTA_MULTIPATH,
    RTA_PROTOINFO, /* no longer used */
    RTA_FLOW,
    RTA_CACHEINFO,
    RTA_SESSION, /* no longer used */
    RTA_MP_ALGO, /* no longer used */
    RTA_TABLE,
    __RTA_MAX
};
#define RTM_GETROUTE     0
enum rt_scope_t
{
    RT_SCOPE_UNIVERSE = 0,
    /* User defined values  */
    RT_SCOPE_SITE = 200,
    RT_SCOPE_LINK = 253,
    RT_SCOPE_HOST = 254,
    RT_SCOPE_NOWHERE = 255
};


enum
{
    RTN_UNSPEC,
    RTN_UNICAST,            /* Gateway or direct route      */
    RTN_LOCAL,              /* Accept locally               */
    RTN_BROADCAST,          /* Accept locally as broadcast,
    send as broadcast */
    RTN_ANYCAST,            /* Accept locally as broadcast,
    but send as unicast */
    RTN_MULTICAST,          /* Multicast route              */
    RTN_BLACKHOLE,          /* Drop                         */
    RTN_UNREACHABLE,        /* Destination is unreachable   */
    RTN_PROHIBIT,           /* Administratively prohibited  */
    RTN_THROW,              /* Not in this table            */
    RTN_NAT,                /* Translate this address       */
    RTN_XRESOLVE,           /* Use external resolver        */
    __RTN_MAX
};

struct rtgenmsg
{
    unsigned char           rtgen_family;
};

struct rtmsg
{
    unsigned char           rtm_family;
    unsigned char           rtm_dst_len;
    unsigned char           rtm_src_len;
    unsigned char           rtm_tos;

    unsigned char           rtm_table;      /* Routing table id */
    unsigned char           rtm_protocol;   /* Routing protocol; see below  */
    unsigned char           rtm_scope;      /* See below */
    unsigned char           rtm_type;       /* See below    */

    unsigned                rtm_flags;
};
#define RTM_NEWTCLASS   40
#define RTM_GETTCLASS   40
#define RTM_DELTCLASS   40
#define RTM_DELQDISC    36
#define RTM_GETQDISC    36
#define RTM_NEWTFILTER  44
struct tcmsg
{
    unsigned char   tcm_family;
    unsigned char   tcm__pad1;
    unsigned short  tcm__pad2;
    int             tcm_ifindex;
    __u32           tcm_handle;
    __u32           tcm_parent;
    __u32           tcm_info;
};
#define RTM_NEWQDISC 36
#define NLM_F_EXCL    0x200   /* Do not touch, if it exists   */
enum
{
    TCA_UNSPEC,
    TCA_KIND,
    TCA_OPTIONS,
    TCA_STATS,
    TCA_XSTATS,
    TCA_RATE,
    TCA_FCNT,
    TCA_STATS2,
    TCA_STAB,
    __TCA_MAX
};

#endif