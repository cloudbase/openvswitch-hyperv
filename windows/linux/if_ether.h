#ifndef IF_ETHER_H
#define IF_ETHER_H 1
#include <config.h>
#include <winsock2.h>
#include <WS2tcpip.h>

/*
* INET         An implementation of the TCP/IP protocol suite for the LINUX
*              operating system.  INET is implemented using the  BSD Socket
*              interface as the means of communication with the user level.
*
*              Global definitions for the Ethernet IEEE 802.3 interface.
*
* Version:     @(#)if_ether.h  1.0.1a  02/08/94
*
* Author:      Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
*              Donald Becker, <becker@super.org>
*              Alan Cox, <alan@lxorguk.ukuu.org.uk>
*              Steve Whitehouse, <gw7rrm@eeshack3.swan.ac.uk>
*
*              This program is free software; you can redistribute it and/or
*              modify it under the terms of the GNU General Public License
*              as published by the Free Software Foundation; either version
*              2 of the License, or (at your option) any later version.
*/

/*
*      IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
*      and FCS/CRC (frame check sequence).
*/

#define ETH_ALEN        6               /* Octets in one ethernet addr   */
#define ETH_HLEN        14              /* Total octets in header.       */
#define ETH_ZLEN        60              /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500            /* Max. octets in payload        */
#define ETH_FRAME_LEN   1514            /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN     4               /* Octets in the FCS             */

/*
*      These are the defined Ethernet Protocol ID's.
*/

#define ETH_P_LOOP      0x0060          /* Ethernet Loopback packet     */
#define ETH_P_PUP       0x0200          /* Xerox PUP packet             */
#define ETH_P_PUPAT     0x0201          /* Xerox PUP Addr Trans packet  */
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#define ETH_P_X25       0x0805          /* CCITT X.25                   */
#define ETH_P_ARP       0x0806          /* Address Resolution packet    */
#define ETH_P_BPQ       0x08FF          /* G8BPQ AX.25 Ethernet Packet  [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_IEEEPUP   0x0a00          /* Xerox IEEE802.3 PUP packet */
#define ETH_P_IEEEPUPAT 0x0a01          /* Xerox IEEE802.3 PUP Addr Trans packet */
#define ETH_P_DEC       0x6000          /* DEC Assigned proto           */
#define ETH_P_DNA_DL    0x6001          /* DEC DNA Dump/Load            */
#define ETH_P_DNA_RC    0x6002          /* DEC DNA Remote Console       */
#define ETH_P_DNA_RT    0x6003          /* DEC DNA Routing              */
#define ETH_P_LAT       0x6004          /* DEC LAT                      */
#define ETH_P_DIAG      0x6005          /* DEC Diagnostics              */
#define ETH_P_CUST      0x6006          /* DEC Customer use             */
#define ETH_P_SCA       0x6007          /* DEC Systems Comms Arch       */
#define ETH_P_TEB       0x6558          /* Trans Ether Bridging         */
#define ETH_P_RARP      0x8035          /* Reverse Addr Res packet      */
#define ETH_P_ATALK     0x809B          /* Appletalk DDP                */
#define ETH_P_AARP      0x80F3          /* Appletalk AARP               */
#define ETH_P_8021Q     0x8100          /* 802.1Q VLAN Extended Header  */
#define ETH_P_IPX       0x8137          /* IPX over DIX                 */
#define ETH_P_IPV6      0x86DD          /* IPv6 over bluebook           */
#define ETH_P_PAUSE     0x8808          /* IEEE Pause frames. See 802.3 31B */
#define ETH_P_SLOW      0x8809          /* Slow Protocol. See 802.3ad 43B */
#define ETH_P_WCCP      0x883E          /* Web-cache coordination protocol
* defined in draft-wilson-wrec-wccp-v2-00.txt */
#define ETH_P_PPP_DISC  0x8863          /* PPPoE discovery messages     */
#define ETH_P_PPP_SES   0x8864          /* PPPoE session messages       */
#define ETH_P_MPLS_UC   0x8847          /* MPLS Unicast traffic         */
#define ETH_P_MPLS_MC   0x8848          /* MPLS Multicast traffic       */
#define ETH_P_ATMMPOA   0x884c          /* MultiProtocol Over ATM       */
#define ETH_P_ATMFATE   0x8884          /* Frame-based ATM Transport
* over Ethernet
*/
#define ETH_P_PAE       0x888E          /* Port Access Entity (IEEE 802.1X) */
#define ETH_P_AOE       0x88A2          /* ATA over Ethernet            */
#define ETH_P_TIPC      0x88CA          /* TIPC                         */
#define ETH_P_1588      0x88F7          /* IEEE 1588 Timesync */
#define ETH_P_FCOE      0x8906          /* Fibre Channel over Ethernet  */
#define ETH_P_TDLS      0x890D          /* TDLS */
#define ETH_P_FIP       0x8914          /* FCoE Initialization Protocol */
#define ETH_P_EDSA      0xDADA          /* Ethertype DSA [ NOT AN OFFICIALLY REGISTERED ID ] */
#define ETH_P_AF_IUCV   0xFBFB          /* IBM af_iucv [ NOT AN OFFICIALLY REGISTERED ID ] */

/*
*      Non DIX types. Won't clash for 1500 types.
*/

#define ETH_P_802_3     0x0001          /* Dummy type for 802.3 frames  */
#define ETH_P_AX25      0x0002          /* Dummy protocol id for AX.25  */
#define ETH_P_ALL       0x0003          /* Every packet (be careful!!!) */
#define ETH_P_802_2     0x0004          /* 802.2 frames                 */
#define ETH_P_SNAP      0x0005          /* Internal only                */
#define ETH_P_DDCMP     0x0006          /* DEC DDCMP: Internal only     */
#define ETH_P_WAN_PPP   0x0007          /* Dummy type for WAN PPP frames*/
#define ETH_P_PPP_MP    0x0008          /* Dummy type for PPP MP frames */
#define ETH_P_LOCALTALK 0x0009          /* Localtalk pseudo type        */
#define ETH_P_CAN       0x000C          /* Controller Area Network      */
#define ETH_P_PPPTALK   0x0010          /* Dummy type for Atalk over PPP*/
#define ETH_P_TR_802_2  0x0011          /* 802.2 frames                 */
#define ETH_P_MOBITEX   0x0015          /* Mobitex (kaz@cafe.net)       */
#define ETH_P_CONTROL   0x0016          /* Card specific control frames */
#define ETH_P_IRDA      0x0017          /* Linux-IrDA                   */
#define ETH_P_ECONET    0x0018          /* Acorn Econet                 */
#define ETH_P_HDLC      0x0019          /* HDLC frames                  */
#define ETH_P_ARCNET    0x001A          /* 1A for ArcNet :-)            */
#define ETH_P_DSA       0x001B          /* Distributed Switch Arch.     */
#define ETH_P_TRAILER   0x001C          /* Trailer switch tagging       */
#define ETH_P_PHONET    0x00F5          /* Nokia Phonet frames          */
#define ETH_P_IEEE802154 0x00F6         /* IEEE802.15.4 frame           */

#define ETHTOOL_FWVERS_LEN      32
#define ETHTOOL_BUSINFO_LEN     32
/* these strings are set to whatever the driver author decides... */
#ifndef ETHTOOL
#define ETHTOOL
struct ethtool_drvinfo {
        __u32   cmd;
        char    driver[32];     /* driver short name, "tulip", "eepro100" */
        char    version[32];    /* driver version string */
        char    fw_version[ETHTOOL_FWVERS_LEN]; /* firmware version string */
        char    bus_info[ETHTOOL_BUSINFO_LEN];  /* Bus info for this IF. */
                                /* For PCI devices, use pci_name(pci_dev). */
        char    reserved1[32];
        char    reserved2[12];
        __u32   n_priv_flags;   /* number of flags valid in ETHTOOL_GPFLAGS */
        __u32   n_stats;        /* number of u64's from ETHTOOL_GSTATS */
        __u32   testinfo_len;
        __u32   eedump_len;     /* Size of data from ETHTOOL_GEEPROM (bytes) */
        __u32   regdump_len;    /* Size of data from ETHTOOL_GREGS (bytes) */
};
#endif

/*
*      This is an Ethernet frame header.
*/
#ifndef ETHHDR
#define ETHHDR
PACKED_STRUCT
struct ethhdr {
    unsigned char   h_dest[ETH_ALEN];       /* destination eth addr */
    unsigned char   h_source[ETH_ALEN];     /* source ether addr    */
    __be16          h_proto;                /* packet type ID field */
}END_PACKED_STRUCT;
#endif

/* CMDs currently supported */
#define ETHTOOL_GSET            0x00000001 /* Get settings. */
#define ETHTOOL_SSET            0x00000002 /* Set settings. */
#define ETHTOOL_GDRVINFO        0x00000003 /* Get driver info. */
#define ETHTOOL_GREGS           0x00000004 /* Get NIC registers. */
#define ETHTOOL_GWOL            0x00000005 /* Get wake-on-lan options. */
#define ETHTOOL_SWOL            0x00000006 /* Set wake-on-lan options. */
#define ETHTOOL_GMSGLVL         0x00000007 /* Get driver message level */
#define ETHTOOL_SMSGLVL         0x00000008 /* Set driver msg level. */
#define ETHTOOL_NWAY_RST        0x00000009 /* Restart autonegotiation. */
/* Get link status for host, i.e. whether the interface *and* the
 * physical port (if there is one) are up (ethtool_value). */
#define ETHTOOL_GLINK           0x0000000a
#define ETHTOOL_GEEPROM         0x0000000b /* Get EEPROM data */
#define ETHTOOL_SEEPROM         0x0000000c /* Set EEPROM data. */
#define ETHTOOL_GCOALESCE       0x0000000e /* Get coalesce config */
#define ETHTOOL_SCOALESCE       0x0000000f /* Set coalesce config. */
#define ETHTOOL_GRINGPARAM      0x00000010 /* Get ring parameters */
#define ETHTOOL_SRINGPARAM      0x00000011 /* Set ring parameters. */
#define ETHTOOL_GPAUSEPARAM     0x00000012 /* Get pause parameters */
#define ETHTOOL_SPAUSEPARAM     0x00000013 /* Set pause parameters. */
#define ETHTOOL_GRXCSUM         0x00000014 /* Get RX hw csum enable (ethtool_value) */
#define ETHTOOL_SRXCSUM         0x00000015 /* Set RX hw csum enable (ethtool_value) */
#define ETHTOOL_GTXCSUM         0x00000016 /* Get TX hw csum enable (ethtool_value) */
#define ETHTOOL_STXCSUM         0x00000017 /* Set TX hw csum enable (ethtool_value) */
#define ETHTOOL_GSG             0x00000018 /* Get scatter-gather enable
                                            * (ethtool_value) */
#define ETHTOOL_SSG             0x00000019 /* Set scatter-gather enable
                                            * (ethtool_value). */
#define ETHTOOL_TEST            0x0000001a /* execute NIC self-test. */
#define ETHTOOL_GSTRINGS        0x0000001b /* get specified string set */
#define ETHTOOL_PHYS_ID         0x0000001c /* identify the NIC */
#define ETHTOOL_GSTATS          0x0000001d /* get NIC-specific statistics */
#define ETHTOOL_GTSO            0x0000001e /* Get TSO enable (ethtool_value) */
#define ETHTOOL_STSO            0x0000001f /* Set TSO enable (ethtool_value) */
#define ETHTOOL_GPERMADDR       0x00000020 /* Get permanent hardware address */
#define ETHTOOL_GUFO            0x00000021 /* Get UFO enable (ethtool_value) */
#define ETHTOOL_SUFO            0x00000022 /* Set UFO enable (ethtool_value) */
#define ETHTOOL_GGSO            0x00000023 /* Get GSO enable (ethtool_value) */
#define ETHTOOL_SGSO            0x00000024 /* Set GSO enable (ethtool_value) */
#define ETHTOOL_GFLAGS          0x00000025 /* Get flags bitmap(ethtool_value) */
#define ETHTOOL_SFLAGS          0x00000026 /* Set flags bitmap(ethtool_value) */
#define ETHTOOL_GPFLAGS         0x00000027 /* Get driver-private flags bitmap */
#define ETHTOOL_SPFLAGS         0x00000028 /* Set driver-private flags bitmap */

#define ETHTOOL_GRXFH           0x00000029 /* Get RX flow hash configuration */
#define ETHTOOL_SRXFH           0x0000002a /* Set RX flow hash configuration */
#define ETHTOOL_GGRO            0x0000002b /* Get GRO enable (ethtool_value) */
#define ETHTOOL_SGRO            0x0000002c /* Set GRO enable (ethtool_value) */
#define ETHTOOL_GRXRINGS        0x0000002d /* Get RX rings available for LB */
#define ETHTOOL_GRXCLSRLCNT     0x0000002e /* Get RX class rule count */
#define ETHTOOL_GRXCLSRULE      0x0000002f /* Get RX classification rule */
#define ETHTOOL_GRXCLSRLALL     0x00000030 /* Get all RX classification rule */
#define ETHTOOL_SRXCLSRLDEL     0x00000031 /* Delete RX classification rule */
#define ETHTOOL_SRXCLSRLINS     0x00000032 /* Insert RX classification rule */
#define ETHTOOL_FLASHDEV        0x00000033 /* Flash firmware to device */
#define ETHTOOL_RESET           0x00000034 /* Reset hardware */

#define ETHTOOL_GRXFHINDIR      0x00000038 /* Get RX flow hash indir'n table */
#define ETHTOOL_SRXFHINDIR      0x00000039 /* Set RX flow hash indir'n table */
#define ETHTOOL_GCHANNELS       0x0000003c /* Get no of channels */
#define ETHTOOL_SCHANNELS       0x0000003d /* Set no of channels */
#define ETHTOOL_SET_DUMP        0x0000003e /* Set dump settings */
#define ETHTOOL_GET_DUMP_FLAG   0x0000003f /* Get dump settings */
#define ETHTOOL_GET_DUMP_DATA   0x00000040 /* Get dump data */
#define ETHTOOL_GET_TS_INFO     0x00000041 /* Get time stamping and PHC info */
#define ETHTOOL_GMODULEINFO     0x00000042 /* Get plug-in module information */
#define ETHTOOL_GMODULEEEPROM   0x00000043 /* Get plug-in module eeprom */
#define ETHTOOL_GEEE            0x00000044 /* Get EEE settings */
#define ETHTOOL_SEEE            0x00000045 /* Set EEE settings */
/* Indicates what features are supported by the interface. */
#define SUPPORTED_10baseT_Half          (1 << 0)
#define SUPPORTED_10baseT_Full          (1 << 1)
#define SUPPORTED_100baseT_Half         (1 << 2)
#define SUPPORTED_100baseT_Full         (1 << 3)
#define SUPPORTED_1000baseT_Half        (1 << 4)
#define SUPPORTED_1000baseT_Full        (1 << 5)
#define SUPPORTED_Autoneg               (1 << 6)
#define SUPPORTED_TP                    (1 << 7)
#define SUPPORTED_AUI                   (1 << 8)
#define SUPPORTED_MII                   (1 << 9)
#define SUPPORTED_FIBRE                 (1 << 10)
#define SUPPORTED_BNC                   (1 << 11)
#define SUPPORTED_10000baseT_Full       (1 << 12)
#define SUPPORTED_Pause                 (1 << 13)
#define SUPPORTED_Asym_Pause            (1 << 14)
#define SUPPORTED_2500baseX_Full        (1 << 15)
#define SUPPORTED_Backplane             (1 << 16)
#define SUPPORTED_1000baseKX_Full       (1 << 17)
#define SUPPORTED_10000baseKX4_Full     (1 << 18)
#define SUPPORTED_10000baseKR_Full      (1 << 19)
#define SUPPORTED_10000baseR_FEC        (1 << 20)
#define SUPPORTED_20000baseMLD2_Full    (1 << 21)
#define SUPPORTED_20000baseKR2_Full     (1 << 22)

#endif