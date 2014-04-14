struct gnet_stats_basic
{
    __u64   bytes;
    __u32   packets;
};
enum {
    TCA_STATS_UNSPEC,
    TCA_STATS_BASIC,
    TCA_STATS_RATE_EST,
    TCA_STATS_QUEUE,
    TCA_STATS_APP,
    __TCA_STATS_MAX,
};
#define TCA_STATS_MAX (__TCA_STATS_MAX - 1)
/**
* struct gnet_stats_queue - queuing statistics
* @qlen: queue length
* @backlog: backlog size of queue
* @drops: number of dropped packets
* @requeues: number of requeues
* @overlimits: number of enqueues over the limit
*/
struct gnet_stats_queue
{
    __u32   qlen;
    __u32   backlog;
    __u32   drops;
    __u32   requeues;
    __u32   overlimits;
};
