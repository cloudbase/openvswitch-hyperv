#ifndef ICMP6_H
#define ICMP6_H

#include <WinSock2.h>
#include <windows.h>
#include <stdint.h>
#include <Ws2tcpip.h>

#define ICMP_ECHOREPLY  129
#define ICMP_ECHOREQ    128



PACKED_STRUCT
typedef struct icmp6_hdr {
    uint8_t     icmp6_type;   /* type field */
    uint8_t     icmp6_code;   /* code field */
    uint16_t    icmp6_cksum;  /* checksum field */
    union
    {
        uint32_t  icmp6_un_data32[1]; /* type-specific field */
        uint16_t  icmp6_un_data16[2]; /* type-specific field */
        uint8_t   icmp6_un_data8[4];  /* type-specific field */
    } icmp6_dataun;
}ICMP6HDR;

typedef struct ip6_ext {
    u_int8_t ip6e_nxt;
    u_int8_t ip6e_len;
}IP6EXT;

#define REQ_DATASIZE 32     // Echo Request Data size

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
    ICMP6HDR icmp6Hdr;
    DWORD   dwTime;
    char    cData[REQ_DATASIZE];
}ECHOREQUEST, *PECHOREQUEST;


// ICMP Echo Reply
typedef struct tagECHOREPLY
{
    ECHOREQUEST echoRequest;
    char    cFiller[256];
}ECHOREPLY, *PECHOREPLY;
END_PACKED_STRUCT

#if     BYTE_ORDER == BIG_ENDIAN
#define IP6F_OFF_MASK       0xfff8  /* mask out offset from _offlg */
#define IP6F_RESERVED_MASK  0x0006  /* reserved bits in ip6f_offlg */
#define IP6F_MORE_FRAG      0x0001  /* more-fragments flag */
#else   /* BYTE_ORDER == LITTLE_ENDIAN */
#define IP6F_OFF_MASK       0xf8ff  /* mask out offset from _offlg */
#define IP6F_RESERVED_MASK  0x0600  /* reserved bits in ip6f_offlg */
#define IP6F_MORE_FRAG      0x0100  /* more-fragments flag */
#endif

#define ND_ROUTER_SOLICIT           133
#define ND_ROUTER_ADVERT            134
#define ND_NEIGHBOR_SOLICIT         135
#define ND_NEIGHBOR_ADVERT          136
#define ND_REDIRECT                 137
#define ND_OPT_SOURCE_LINKADDR          1
#define ND_OPT_TARGET_LINKADDR          2
#define ND_OPT_PREFIX_INFORMATION       3
#define ND_OPT_REDIRECTED_HEADER        4
#define ND_OPT_MTU                      5
#define ND_OPT_RTR_ADV_INTERVAL         7
#define ND_OPT_HOME_AGENT_INFO          8
struct nd_opt_hdr             /* Neighbor discovery option header */
{
    uint8_t  nd_opt_type;
    uint8_t  nd_opt_len;        /* in units of 8 octets */
    /* followed by option specific data */
};
/* Routing header */
struct ip6_rthdr
{
    uint8_t  ip6r_nxt;          /* next header */
    uint8_t  ip6r_len;          /* length in units of 8 octets */
    uint8_t  ip6r_type;         /* routing type */
    uint8_t  ip6r_segleft;      /* segments left */
    /* followed by routing type specific data */
};

#endif