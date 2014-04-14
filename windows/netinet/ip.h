#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#define IP_MAXPACKET 65535          /* maximum packet size */

/*
 * Definitions for IP type of service (ip_tos)
 */
#define IPTOS_LOWDELAY    0x10
#define IPTOS_THROUGHPUT  0x08
#define IPTOS_RELIABILITY 0x04

/*
 * Definitions for IP precedence (also in ip_tos) (hopefully unused)
 */
#define IPTOS_PREC_NETCONTROL      0xe0
#define IPTOS_PREC_INTERNETCONTROL 0xc0
#define IPTOS_PREC_CRITIC_ECP      0xa0
#define IPTOS_PREC_FLASHOVERRIDE   0x80
#define IPTOS_PREC_FLASH           0x60
#define IPTOS_PREC_IMMEDIATE       0x40
#define IPTOS_PREC_PRIORITY        0x20
#define IPTOS_PREC_ROUTINE         0x10

/*
 * Definitions for options.
 */
#define IPOPT_COPIED(o) ((o)&0x80)
#define IPOPT_CLASS(o) ((o)&0x60)
#define IPOPT_NUMBER(o) ((o)&0x1f)

#define IPOPT_CONTROL   0x00
#define IPOPT_RESERVED1 0x20
#define IPOPT_DEBMEAS   0x40
#define IPOPT_RESERVED2 0x60

#define IPOPT_EOL      0    /* end of option list */
#define IPOPT_NOP      1    /* no operation */

#define IPOPT_RR       7    /* record packet route */
#define IPOPT_TS       68   /* timestamp */
#define IPOPT_SECURITY 130  /* provide s,c,h,tcc */
#define IPOPT_LSRR     131  /* loose source route */
#define IPOPT_SATID    136  /* satnet id */
#define IPOPT_SSRR     137  /* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define IPOPT_OPTVAL 0  /* option ID */
#define IPOPT_OLEN   1  /* option length */
#define IPOPT_OFFSET 2  /* offset within option */
#define IPOPT_MINOFF 4  /* min value of above */
