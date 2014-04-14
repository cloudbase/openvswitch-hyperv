#ifndef PKT_SCHED_H
#define PKT_SCHED_H

#include <WinSock2.h>

#define TC_PRIO_BESTEFFORT              0

struct tc_htb_glob
{
    __u32 version;          /* to match HTB/TC */
    __u32 rate2quantum;     /* bps->quantum divisor */
    __u32 defcls;           /* default class number */
    __u32 debug;            /* debug flags */

    /* stats */
    __u32 direct_pkts; /* count of non shapped packets */
};
#define TC_H_ROOT   (0xFFFFFFFFU)
enum
{
    TCA_HTB_UNSPEC,
    TCA_HTB_PARMS,
    TCA_HTB_INIT,
    TCA_HTB_CTAB,
    TCA_HTB_RTAB,
    __TCA_HTB_MAX,
};
struct tc_ratespec
{
    unsigned char   cell_log;
    unsigned char   __reserved1;
    unsigned short  overhead;
    short           cell_align;
    unsigned short  mpu;
    __u32           rate;
};

struct tc_htb_opt
{
    struct tc_ratespec      rate;
    struct tc_ratespec      ceil;
    __u32   buffer;
    __u32   cbuffer;
    __u32   quantum;
    __u32   level;          /* out only */
    __u32   prio;
};
enum
{
    TCA_HFSC_UNSPEC,
    TCA_HFSC_RSC,
    TCA_HFSC_FSC,
    TCA_HFSC_USC,
    __TCA_HFSC_MAX,
};
struct tc_service_curve
{
    __u32   m1;             /* slope of the first segment in bps */
    __u32   d;              /* x-projection of the first segment in us */
    __u32   m2;             /* slope of the second segment in bps */
};
/* HFSC section */

struct tc_hfsc_qopt
{
    __u16   defcls;         /* default class */
};
#define TC_H_MAJ_MASK (0xFFFF0000U)
#define TC_H_MIN_MASK (0x0000FFFFU)
#define TC_H_MAJ(h) ((h)&TC_H_MAJ_MASK)
#define TC_H_MIN(h) ((h)&TC_H_MIN_MASK)
#define TC_H_MAKE(maj,min) (((maj)&TC_H_MAJ_MASK)|((min)&TC_H_MIN_MASK))
#define TC_H_UNSPEC     (0U)
#define TC_H_ROOT       (0xFFFFFFFFU)
#define TC_H_INGRESS    (0xFFFFFFF1U)

#define TC_ACT_UNSPEC   (-1)
#define TC_ACT_OK               0
#define TC_ACT_RECLASSIFY       1
#define TC_ACT_SHOT             2
#define TC_ACT_PIPE             3
#define TC_ACT_STOLEN           4
#define TC_ACT_QUEUED           5
#define TC_ACT_REPEAT           6
#define TC_ACT_JUMP             0x10000000

enum
{
    TCA_BASIC_UNSPEC,
    TCA_BASIC_CLASSID,
    TCA_BASIC_EMATCHES,
    TCA_BASIC_ACT,
    TCA_BASIC_POLICE,
    __TCA_BASIC_MAX
};

enum
{
    TCA_POLICE_UNSPEC,
    TCA_POLICE_TBF,
    TCA_POLICE_RATE,
    TCA_POLICE_PEAKRATE,
    TCA_POLICE_AVRATE,
    TCA_POLICE_RESULT,
    __TCA_POLICE_MAX
#define TCA_POLICE_RESULT TCA_POLICE_RESULT
};


struct tc_police
{
    __u32                   index;
    int                     action;
#define TC_POLICE_UNSPEC        TC_ACT_UNSPEC
#define TC_POLICE_OK            TC_ACT_OK
#define TC_POLICE_RECLASSIFY    TC_ACT_RECLASSIFY
#define TC_POLICE_SHOT          TC_ACT_SHOT
#define TC_POLICE_PIPE          TC_ACT_PIPE

    __u32                   limit;
    __u32                   burst;
    __u32                   mtu;
    struct tc_ratespec      rate;
    struct tc_ratespec      peakrate;
    int                     refcnt;
    int                     bindcnt;
    __u32                   capab;
};


#endif

