/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <config.h>
#include <sys/types.h>
#include "flow.h"
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <netinet/in.h>
#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "byte-order.h"
#include "coverage.h"
#include "csum.h"
#include "dynamic-string.h"
#include "hash.h"
#include "jhash.h"
#include "match.h"
#include "ofpbuf.h"
#include "openflow/openflow.h"
#include "packets.h"
#include "odp-util.h"
#include "random.h"
#include "unaligned.h"

COVERAGE_DEFINE(flow_extract);
COVERAGE_DEFINE(miniflow_malloc);

/* U32 indices for segmented flow classification. */
const uint8_t flow_segment_u32s[4] = {
    FLOW_SEGMENT_1_ENDS_AT / 4,
    FLOW_SEGMENT_2_ENDS_AT / 4,
    FLOW_SEGMENT_3_ENDS_AT / 4,
    FLOW_U32S
};

static struct arp_eth_header *
pull_arp(struct ofpbuf *packet)
{
    return ofpbuf_try_pull(packet, ARP_ETH_HEADER_LEN);
}

static struct ip_header *
pull_ip(struct ofpbuf *packet)
{
    if (ofpbuf_size(packet) >= IP_HEADER_LEN) {
        struct ip_header *ip = ofpbuf_data(packet);
        int ip_len = IP_IHL(ip->ip_ihl_ver) * 4;
        if (ip_len >= IP_HEADER_LEN && ofpbuf_size(packet) >= ip_len) {
            return ofpbuf_pull(packet, ip_len);
        }
    }
    return NULL;
}

static struct icmp_header *
pull_icmp(struct ofpbuf *packet)
{
    return ofpbuf_try_pull(packet, ICMP_HEADER_LEN);
}

static struct icmp6_hdr *
pull_icmpv6(struct ofpbuf *packet)
{
    return ofpbuf_try_pull(packet, sizeof(struct icmp6_hdr));
}

static void
parse_mpls(struct ofpbuf *b, struct flow *flow)
{
    struct mpls_hdr *mh;
    int idx = 0;

    while ((mh = ofpbuf_try_pull(b, sizeof *mh))) {
        ovs_be32 mpls_lse = get_16aligned_be32(&mh->mpls_lse);
        if (idx < FLOW_MAX_MPLS_LABELS) {
            flow->mpls_lse[idx++] = mpls_lse;
        }
        if (mpls_lse & htonl(MPLS_BOS_MASK)) {
            break;
        }
    }
}

static void
parse_vlan(struct ofpbuf *b, struct flow *flow)
{
    struct qtag_prefix {
        ovs_be16 eth_type;      /* ETH_TYPE_VLAN */
        ovs_be16 tci;
    };

    if (ofpbuf_size(b) >= sizeof(struct qtag_prefix) + sizeof(ovs_be16)) {
        struct qtag_prefix *qp = ofpbuf_pull(b, sizeof *qp);
        flow->vlan_tci = qp->tci | htons(VLAN_CFI);
    }
}

static ovs_be16
parse_ethertype(struct ofpbuf *b)
{
    struct llc_snap_header *llc;
    ovs_be16 proto;

    proto = *(ovs_be16 *) ofpbuf_pull(b, sizeof proto);
    if (ntohs(proto) >= ETH_TYPE_MIN) {
        return proto;
    }

    if (ofpbuf_size(b) < sizeof *llc) {
        return htons(FLOW_DL_TYPE_NONE);
    }

    llc = ofpbuf_data(b);
    if (llc->llc.llc_dsap != LLC_DSAP_SNAP
        || llc->llc.llc_ssap != LLC_SSAP_SNAP
        || llc->llc.llc_cntl != LLC_CNTL_SNAP
        || memcmp(llc->snap.snap_org, SNAP_ORG_ETHERNET,
                  sizeof llc->snap.snap_org)) {
        return htons(FLOW_DL_TYPE_NONE);
    }

    ofpbuf_pull(b, sizeof *llc);

    if (ntohs(llc->snap.snap_type) >= ETH_TYPE_MIN) {
        return llc->snap.snap_type;
    }

    return htons(FLOW_DL_TYPE_NONE);
}

static int
parse_ipv6(struct ofpbuf *packet, struct flow *flow)
{
    const struct ovs_16aligned_ip6_hdr *nh;
    ovs_be32 tc_flow;
    int nexthdr;

    nh = ofpbuf_try_pull(packet, sizeof *nh);
    if (!nh) {
        return EINVAL;
    }

#ifndef _WIN32
	nexthdr = nh->ip6_nxt;
#else
	nh->ip6_ctlun.ip6_un1.ip6_un1_nxt;
#endif

    memcpy(&flow->ipv6_src, &nh->ip6_src, sizeof flow->ipv6_src);
    memcpy(&flow->ipv6_dst, &nh->ip6_dst, sizeof flow->ipv6_dst);

#ifndef _WIN32
	tc_flow = get_16aligned_be32(&nh->ip6_flow);
#else
	tc_flow = get_16aligned_be32(&nh->ip6_ctlun.ip6_un1.ip6_un1_flow);
#endif
    flow->nw_tos = ntohl(tc_flow) >> 20;
    flow->ipv6_label = tc_flow & htonl(IPV6_LABEL_MASK);
#ifndef _WIN32
	flow->nw_ttl = nh->ip6_hlim;
#else
	flow->nw_ttl = nh->ip6_ctlun.ip6_un1.ip6_un1_hlim;
#endif
    flow->nw_proto = IPPROTO_NONE;

    while (1) {
        if ((nexthdr != IPPROTO_HOPOPTS)
                && (nexthdr != IPPROTO_ROUTING)
                && (nexthdr != IPPROTO_DSTOPTS)
                && (nexthdr != IPPROTO_AH)
                && (nexthdr != IPPROTO_FRAGMENT)) {
            /* It's either a terminal header (e.g., TCP, UDP) or one we
             * don't understand.  In either case, we're done with the
             * packet, so use it to fill in 'nw_proto'. */
            break;
        }

        /* We only verify that at least 8 bytes of the next header are
         * available, but many of these headers are longer.  Ensure that
         * accesses within the extension header are within those first 8
         * bytes. All extension headers are required to be at least 8
         * bytes. */
        if (ofpbuf_size(packet) < 8) {
            return EINVAL;
        }

        if ((nexthdr == IPPROTO_HOPOPTS)
                || (nexthdr == IPPROTO_ROUTING)
                || (nexthdr == IPPROTO_DSTOPTS)) {
            /* These headers, while different, have the fields we care about
             * in the same location and with the same interpretation. */
            const struct ip6_ext *ext_hdr = ofpbuf_data(packet);
            nexthdr = ext_hdr->ip6e_nxt;
            if (!ofpbuf_try_pull(packet, (ext_hdr->ip6e_len + 1) * 8)) {
                return EINVAL;
            }
        } else if (nexthdr == IPPROTO_AH) {
            /* A standard AH definition isn't available, but the fields
             * we care about are in the same location as the generic
             * option header--only the header length is calculated
             * differently. */
            const struct ip6_ext *ext_hdr = ofpbuf_data(packet);
            nexthdr = ext_hdr->ip6e_nxt;
            if (!ofpbuf_try_pull(packet, (ext_hdr->ip6e_len + 2) * 4)) {
               return EINVAL;
            }
        } else if (nexthdr == IPPROTO_FRAGMENT) {
            const struct ovs_16aligned_ip6_frag *frag_hdr = ofpbuf_data(packet);

            nexthdr = frag_hdr->ip6f_nxt;
            if (!ofpbuf_try_pull(packet, sizeof *frag_hdr)) {
                return EINVAL;
            }

            /* We only process the first fragment. */
            if (frag_hdr->ip6f_offlg != htons(0)) {
                flow->nw_frag = FLOW_NW_FRAG_ANY;
                if ((frag_hdr->ip6f_offlg & IP6F_OFF_MASK) != htons(0)) {
                    flow->nw_frag |= FLOW_NW_FRAG_LATER;
                    nexthdr = IPPROTO_FRAGMENT;
                    break;
                }
            }
        }
    }

    flow->nw_proto = nexthdr;
    return 0;
}

static void
parse_tcp(struct ofpbuf *b, struct flow *flow)
{
    if (ofpbuf_size(b) >= TCP_HEADER_LEN) {
        const struct tcp_header *tcp = ofpbuf_data(b);

        flow->tp_src = tcp->tcp_src;
        flow->tp_dst = tcp->tcp_dst;
        flow->tcp_flags = tcp->tcp_ctl & htons(0x0fff);
    }
}

static void
parse_udp(struct ofpbuf *b, struct flow *flow)
{
    if (ofpbuf_size(b) >= UDP_HEADER_LEN) {
        const struct udp_header *udp = ofpbuf_data(b);

        flow->tp_src = udp->udp_src;
        flow->tp_dst = udp->udp_dst;
    }
}

static void
parse_sctp(struct ofpbuf *b, struct flow *flow)
{
    if (ofpbuf_size(b) >= SCTP_HEADER_LEN) {
        const struct sctp_header *sctp = ofpbuf_data(b);

        flow->tp_src = sctp->sctp_src;
        flow->tp_dst = sctp->sctp_dst;
    }
}

static void
parse_icmpv6(struct ofpbuf *b, struct flow *flow)
{
    const struct icmp6_hdr *icmp = pull_icmpv6(b);

    if (!icmp) {
        return;
    }

    /* The ICMPv6 type and code fields use the 16-bit transport port
     * fields, so we need to store them in 16-bit network byte order. */
    flow->tp_src = htons(icmp->icmp6_type);
    flow->tp_dst = htons(icmp->icmp6_code);

    if (icmp->icmp6_code == 0 &&
        (icmp->icmp6_type == ND_NEIGHBOR_SOLICIT ||
         icmp->icmp6_type == ND_NEIGHBOR_ADVERT)) {
        const struct in6_addr *nd_target;

        nd_target = ofpbuf_try_pull(b, sizeof *nd_target);
        if (!nd_target) {
            return;
        }
        flow->nd_target = *nd_target;

        while (ofpbuf_size(b) >= 8) {
            /* The minimum size of an option is 8 bytes, which also is
             * the size of Ethernet link-layer options. */
            const struct nd_opt_hdr *nd_opt = ofpbuf_data(b);
            int opt_len = nd_opt->nd_opt_len * 8;

            if (!opt_len || opt_len > ofpbuf_size(b)) {
                goto invalid;
            }

            /* Store the link layer address if the appropriate option is
             * provided.  It is considered an error if the same link
             * layer option is specified twice. */
            if (nd_opt->nd_opt_type == ND_OPT_SOURCE_LINKADDR
                    && opt_len == 8) {
                if (eth_addr_is_zero(flow->arp_sha)) {
                    memcpy(flow->arp_sha, nd_opt + 1, ETH_ADDR_LEN);
                } else {
                    goto invalid;
                }
            } else if (nd_opt->nd_opt_type == ND_OPT_TARGET_LINKADDR
                    && opt_len == 8) {
                if (eth_addr_is_zero(flow->arp_tha)) {
                    memcpy(flow->arp_tha, nd_opt + 1, ETH_ADDR_LEN);
                } else {
                    goto invalid;
                }
            }

            if (!ofpbuf_try_pull(b, opt_len)) {
                goto invalid;
            }
        }
    }

    return;

invalid:
    memset(&flow->nd_target, 0, sizeof(flow->nd_target));
    memset(flow->arp_sha, 0, sizeof(flow->arp_sha));
    memset(flow->arp_tha, 0, sizeof(flow->arp_tha));

    return;
}

/* Initializes 'flow' members from 'packet' and 'md'
 *
 * Initializes 'packet' header l2 pointer to the start of the Ethernet
 * header, and the layer offsets as follows:
 *
 *    - packet->l2_5_ofs to the start of the MPLS shim header, or UINT16_MAX
 *      when there is no MPLS shim header.
 *
 *    - packet->l3_ofs to just past the Ethernet header, or just past the
 *      vlan_header if one is present, to the first byte of the payload of the
 *      Ethernet frame.  UINT16_MAX if the frame is too short to contain an
 *      Ethernet header.
 *
 *    - packet->l4_ofs to just past the IPv4 header, if one is present and
 *      has at least the content used for the fields of interest for the flow,
 *      otherwise UINT16_MAX.
 */
void
flow_extract(struct ofpbuf *packet, const struct pkt_metadata *md,
             struct flow *flow)
{
    struct ofpbuf b = *packet;
    struct eth_header *eth;

    COVERAGE_INC(flow_extract);

    memset(flow, 0, sizeof *flow);

    if (md) {
        flow->tunnel = md->tunnel;
        flow->in_port = md->in_port;
        flow->skb_priority = md->skb_priority;
        flow->pkt_mark = md->pkt_mark;
        flow->recirc_id = md->recirc_id;
        flow->dp_hash = md->dp_hash;
    }

    ofpbuf_set_frame(packet, ofpbuf_data(packet));

    if (ofpbuf_size(&b) < sizeof *eth) {
        return;
    }

    /* Link layer. */
    eth = ofpbuf_data(&b);
    memcpy(flow->dl_src, eth->eth_src, ETH_ADDR_LEN);
    memcpy(flow->dl_dst, eth->eth_dst, ETH_ADDR_LEN);

    /* dl_type, vlan_tci. */
    ofpbuf_pull(&b, ETH_ADDR_LEN * 2);
    if (eth->eth_type == htons(ETH_TYPE_VLAN)) {
        parse_vlan(&b, flow);
    }
    flow->dl_type = parse_ethertype(&b);

    /* Parse mpls, copy l3 ttl. */
    if (eth_type_mpls(flow->dl_type)) {
        ofpbuf_set_l2_5(packet, ofpbuf_data(&b));
        parse_mpls(&b, flow);
    }

    /* Network layer. */
    ofpbuf_set_l3(packet, ofpbuf_data(&b));
    if (flow->dl_type == htons(ETH_TYPE_IP)) {
        const struct ip_header *nh = pull_ip(&b);
        if (nh) {
            ofpbuf_set_l4(packet, ofpbuf_data(&b));

            flow->nw_src = get_16aligned_be32(&nh->ip_src);
            flow->nw_dst = get_16aligned_be32(&nh->ip_dst);
            flow->nw_proto = nh->ip_proto;

            flow->nw_tos = nh->ip_tos;
            if (IP_IS_FRAGMENT(nh->ip_frag_off)) {
                flow->nw_frag = FLOW_NW_FRAG_ANY;
                if (nh->ip_frag_off & htons(IP_FRAG_OFF_MASK)) {
                    flow->nw_frag |= FLOW_NW_FRAG_LATER;
                }
            }
            flow->nw_ttl = nh->ip_ttl;

            if (!(nh->ip_frag_off & htons(IP_FRAG_OFF_MASK))) {
                if (flow->nw_proto == IPPROTO_TCP) {
                    parse_tcp(&b, flow);
                } else if (flow->nw_proto == IPPROTO_UDP) {
                    parse_udp(&b, flow);
                } else if (flow->nw_proto == IPPROTO_SCTP) {
                    parse_sctp(&b, flow);
                } else if (flow->nw_proto == IPPROTO_ICMP) {
                    const struct icmp_header *icmp = pull_icmp(&b);
                    if (icmp) {
                        flow->tp_src = htons(icmp->icmp_type);
                        flow->tp_dst = htons(icmp->icmp_code);
                    }
                }
            }
        }
    } else if (flow->dl_type == htons(ETH_TYPE_IPV6)) {
        if (parse_ipv6(&b, flow)) {
            return;
        }

        ofpbuf_set_l4(packet, ofpbuf_data(&b));
        if (flow->nw_proto == IPPROTO_TCP) {
            parse_tcp(&b, flow);
        } else if (flow->nw_proto == IPPROTO_UDP) {
            parse_udp(&b, flow);
        } else if (flow->nw_proto == IPPROTO_SCTP) {
            parse_sctp(&b, flow);
        } else if (flow->nw_proto == IPPROTO_ICMPV6) {
            parse_icmpv6(&b, flow);
        }
    } else if (flow->dl_type == htons(ETH_TYPE_ARP) ||
               flow->dl_type == htons(ETH_TYPE_RARP)) {
        const struct arp_eth_header *arp = pull_arp(&b);
        if (arp && arp->ar_hrd == htons(1)
            && arp->ar_pro == htons(ETH_TYPE_IP)
            && arp->ar_hln == ETH_ADDR_LEN
            && arp->ar_pln == 4) {
            /* We only match on the lower 8 bits of the opcode. */
            if (ntohs(arp->ar_op) <= 0xff) {
                flow->nw_proto = ntohs(arp->ar_op);
            }

            flow->nw_src = get_16aligned_be32(&arp->ar_spa);
            flow->nw_dst = get_16aligned_be32(&arp->ar_tpa);
            memcpy(flow->arp_sha, arp->ar_sha, ETH_ADDR_LEN);
            memcpy(flow->arp_tha, arp->ar_tha, ETH_ADDR_LEN);
        }
    }
}

/* For every bit of a field that is wildcarded in 'wildcards', sets the
 * corresponding bit in 'flow' to zero. */
void
flow_zero_wildcards(struct flow *flow, const struct flow_wildcards *wildcards)
{
    uint32_t *flow_u32 = (uint32_t *) flow;
    const uint32_t *wc_u32 = (const uint32_t *) &wildcards->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        flow_u32[i] &= wc_u32[i];
    }
}

void
flow_unwildcard_tp_ports(const struct flow *flow, struct flow_wildcards *wc)
{
    if (flow->nw_proto != IPPROTO_ICMP) {
        memset(&wc->masks.tp_src, 0xff, sizeof wc->masks.tp_src);
        memset(&wc->masks.tp_dst, 0xff, sizeof wc->masks.tp_dst);
    } else {
        wc->masks.tp_src = htons(0xff);
        wc->masks.tp_dst = htons(0xff);
    }
}

/* Initializes 'fmd' with the metadata found in 'flow'. */
void
flow_get_metadata(const struct flow *flow, struct flow_metadata *fmd)
{
    BUILD_ASSERT_DECL(FLOW_WC_SEQ == 25);

    fmd->dp_hash = flow->dp_hash;
    fmd->recirc_id = flow->recirc_id;
    fmd->tun_id = flow->tunnel.tun_id;
    fmd->tun_src = flow->tunnel.ip_src;
    fmd->tun_dst = flow->tunnel.ip_dst;
    fmd->metadata = flow->metadata;
    memcpy(fmd->regs, flow->regs, sizeof fmd->regs);
    fmd->pkt_mark = flow->pkt_mark;
    fmd->in_port = flow->in_port.ofp_port;
}

char *
flow_to_string(const struct flow *flow)
{
    struct ds ds = DS_EMPTY_INITIALIZER;
    flow_format(&ds, flow);
    return ds_cstr(&ds);
}

const char *
flow_tun_flag_to_string(uint32_t flags)
{
    switch (flags) {
    case FLOW_TNL_F_DONT_FRAGMENT:
        return "df";
    case FLOW_TNL_F_CSUM:
        return "csum";
    case FLOW_TNL_F_KEY:
        return "key";
    default:
        return NULL;
    }
}

void
format_flags(struct ds *ds, const char *(*bit_to_string)(uint32_t),
             uint32_t flags, char del)
{
    uint32_t bad = 0;

    if (!flags) {
        return;
    }
    while (flags) {
        uint32_t bit = rightmost_1bit(flags);
        const char *s;

        s = bit_to_string(bit);
        if (s) {
            ds_put_format(ds, "%s%c", s, del);
        } else {
            bad |= bit;
        }

        flags &= ~bit;
    }

    if (bad) {
        ds_put_format(ds, "0x%"PRIx32"%c", bad, del);
    }
    ds_chomp(ds, del);
}

void
format_flags_masked(struct ds *ds, const char *name,
                    const char *(*bit_to_string)(uint32_t), uint32_t flags,
                    uint32_t mask)
{
    if (name) {
        ds_put_format(ds, "%s=", name);
    }
    while (mask) {
        uint32_t bit = rightmost_1bit(mask);
        const char *s = bit_to_string(bit);

        ds_put_format(ds, "%s%s", (flags & bit) ? "+" : "-",
                      s ? s : "[Unknown]");
        mask &= ~bit;
    }
}

void
flow_format(struct ds *ds, const struct flow *flow)
{
    struct match match;

    match_wc_init(&match, flow);
    match_format(&match, ds, OFP_DEFAULT_PRIORITY);
}

void
flow_print(FILE *stream, const struct flow *flow)
{
    char *s = flow_to_string(flow);
    fputs(s, stream);
    free(s);
}

/* flow_wildcards functions. */

/* Initializes 'wc' as a set of wildcards that matches every packet. */
void
flow_wildcards_init_catchall(struct flow_wildcards *wc)
{
    memset(&wc->masks, 0, sizeof wc->masks);
}

/* Clear the metadata and register wildcard masks. They are not packet
 * header fields. */
void
flow_wildcards_clear_non_packet_fields(struct flow_wildcards *wc)
{
    memset(&wc->masks.metadata, 0, sizeof wc->masks.metadata);
    memset(&wc->masks.regs, 0, sizeof wc->masks.regs);
}

/* Returns true if 'wc' matches every packet, false if 'wc' fixes any bits or
 * fields. */
bool
flow_wildcards_is_catchall(const struct flow_wildcards *wc)
{
    const uint32_t *wc_u32 = (const uint32_t *) &wc->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        if (wc_u32[i]) {
            return false;
        }
    }
    return true;
}

/* Sets 'dst' as the bitwise AND of wildcards in 'src1' and 'src2'.
 * That is, a bit or a field is wildcarded in 'dst' if it is wildcarded
 * in 'src1' or 'src2' or both.  */
void
flow_wildcards_and(struct flow_wildcards *dst,
                   const struct flow_wildcards *src1,
                   const struct flow_wildcards *src2)
{
    uint32_t *dst_u32 = (uint32_t *) &dst->masks;
    const uint32_t *src1_u32 = (const uint32_t *) &src1->masks;
    const uint32_t *src2_u32 = (const uint32_t *) &src2->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        dst_u32[i] = src1_u32[i] & src2_u32[i];
    }
}

/* Sets 'dst' as the bitwise OR of wildcards in 'src1' and 'src2'.  That
 * is, a bit or a field is wildcarded in 'dst' if it is neither
 * wildcarded in 'src1' nor 'src2'. */
void
flow_wildcards_or(struct flow_wildcards *dst,
                  const struct flow_wildcards *src1,
                  const struct flow_wildcards *src2)
{
    uint32_t *dst_u32 = (uint32_t *) &dst->masks;
    const uint32_t *src1_u32 = (const uint32_t *) &src1->masks;
    const uint32_t *src2_u32 = (const uint32_t *) &src2->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        dst_u32[i] = src1_u32[i] | src2_u32[i];
    }
}

/* Perform a bitwise OR of miniflow 'src' flow data with the equivalent
 * fields in 'dst', storing the result in 'dst'. */
static void
flow_union_with_miniflow(struct flow *dst, const struct miniflow *src)
{
    uint32_t *dst_u32 = (uint32_t *) dst;
    const uint32_t *p = src->values;
    uint64_t map;

    for (map = src->map; map; map = zero_rightmost_1bit(map)) {
        dst_u32[raw_ctz(map)] |= *p++;
    }
}

/* Fold minimask 'mask''s wildcard mask into 'wc's wildcard mask. */
void
flow_wildcards_fold_minimask(struct flow_wildcards *wc,
                             const struct minimask *mask)
{
    flow_union_with_miniflow(&wc->masks, &mask->masks);
}

uint64_t
miniflow_get_map_in_range(const struct miniflow *miniflow,
                          uint8_t start, uint8_t end, unsigned int *offset)
{
    uint64_t map = miniflow->map;
    *offset = 0;

    if (start > 0) {
        uint64_t msk = (UINT64_C(1) << start) - 1; /* 'start' LSBs set */
        *offset = count_1bits(map & msk);
        map &= ~msk;
    }
    if (end < FLOW_U32S) {
        uint64_t msk = (UINT64_C(1) << end) - 1; /* 'end' LSBs set */
        map &= msk;
    }
    return map;
}

/* Fold minimask 'mask''s wildcard mask into 'wc's wildcard mask
 * in range [start, end). */
void
flow_wildcards_fold_minimask_range(struct flow_wildcards *wc,
                                   const struct minimask *mask,
                                   uint8_t start, uint8_t end)
{
    uint32_t *dst_u32 = (uint32_t *)&wc->masks;
    unsigned int offset;
    uint64_t map = miniflow_get_map_in_range(&mask->masks, start, end,
                                             &offset);
    const uint32_t *p = mask->masks.values + offset;

    for (; map; map = zero_rightmost_1bit(map)) {
        dst_u32[raw_ctz(map)] |= *p++;
    }
}

/* Returns a hash of the wildcards in 'wc'. */
uint32_t
flow_wildcards_hash(const struct flow_wildcards *wc, uint32_t basis)
{
    return flow_hash(&wc->masks, basis);
}

/* Returns true if 'a' and 'b' represent the same wildcards, false if they are
 * different. */
bool
flow_wildcards_equal(const struct flow_wildcards *a,
                     const struct flow_wildcards *b)
{
    return flow_equal(&a->masks, &b->masks);
}

/* Returns true if at least one bit or field is wildcarded in 'a' but not in
 * 'b', false otherwise. */
bool
flow_wildcards_has_extra(const struct flow_wildcards *a,
                         const struct flow_wildcards *b)
{
    const uint32_t *a_u32 = (const uint32_t *) &a->masks;
    const uint32_t *b_u32 = (const uint32_t *) &b->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        if ((a_u32[i] & b_u32[i]) != b_u32[i]) {
            return true;
        }
    }
    return false;
}

/* Returns true if 'a' and 'b' are equal, except that 0-bits (wildcarded bits)
 * in 'wc' do not need to be equal in 'a' and 'b'. */
bool
flow_equal_except(const struct flow *a, const struct flow *b,
                  const struct flow_wildcards *wc)
{
    const uint32_t *a_u32 = (const uint32_t *) a;
    const uint32_t *b_u32 = (const uint32_t *) b;
    const uint32_t *wc_u32 = (const uint32_t *) &wc->masks;
    size_t i;

    for (i = 0; i < FLOW_U32S; i++) {
        if ((a_u32[i] ^ b_u32[i]) & wc_u32[i]) {
            return false;
        }
    }
    return true;
}

/* Sets the wildcard mask for register 'idx' in 'wc' to 'mask'.
 * (A 0-bit indicates a wildcard bit.) */
void
flow_wildcards_set_reg_mask(struct flow_wildcards *wc, int idx, uint32_t mask)
{
    wc->masks.regs[idx] = mask;
}

/* Calculates the 5-tuple hash from the given flow. */
uint32_t
flow_hash_5tuple(const struct flow *flow, uint32_t basis)
{
    uint32_t hash = 0;

    if (!flow) {
        return 0;
    }

    hash = mhash_add(basis, (OVS_FORCE uint32_t) flow->nw_src);
    hash = mhash_add(hash, (OVS_FORCE uint32_t) flow->nw_dst);
    hash = mhash_add(hash, ((OVS_FORCE uint32_t) flow->tp_src << 16)
                           | (OVS_FORCE uint32_t) flow->tp_dst);
    hash = mhash_add(hash, flow->nw_proto);

    return mhash_finish(hash, 13);
}

/* Hashes 'flow' based on its L2 through L4 protocol information. */
uint32_t
flow_hash_symmetric_l4(const struct flow *flow, uint32_t basis)
{
    struct {
        union {
            ovs_be32 ipv4_addr;
            struct in6_addr ipv6_addr;
        };
        ovs_be16 eth_type;
        ovs_be16 vlan_tci;
        ovs_be16 tp_port;
        uint8_t eth_addr[ETH_ADDR_LEN];
        uint8_t ip_proto;
    } fields;

    int i;

    memset(&fields, 0, sizeof fields);
    for (i = 0; i < ETH_ADDR_LEN; i++) {
        fields.eth_addr[i] = flow->dl_src[i] ^ flow->dl_dst[i];
    }
    fields.vlan_tci = flow->vlan_tci & htons(VLAN_VID_MASK);
    fields.eth_type = flow->dl_type;

    /* UDP source and destination port are not taken into account because they
     * will not necessarily be symmetric in a bidirectional flow. */
    if (fields.eth_type == htons(ETH_TYPE_IP)) {
        fields.ipv4_addr = flow->nw_src ^ flow->nw_dst;
        fields.ip_proto = flow->nw_proto;
        if (fields.ip_proto == IPPROTO_TCP || fields.ip_proto == IPPROTO_SCTP) {
            fields.tp_port = flow->tp_src ^ flow->tp_dst;
        }
    } else if (fields.eth_type == htons(ETH_TYPE_IPV6)) {
        const uint8_t *a = &flow->ipv6_src.s6_addr[0];
        const uint8_t *b = &flow->ipv6_dst.s6_addr[0];
        uint8_t *ipv6_addr = &fields.ipv6_addr.s6_addr[0];

        for (i=0; i<16; i++) {
            ipv6_addr[i] = a[i] ^ b[i];
        }
        fields.ip_proto = flow->nw_proto;
        if (fields.ip_proto == IPPROTO_TCP || fields.ip_proto == IPPROTO_SCTP) {
            fields.tp_port = flow->tp_src ^ flow->tp_dst;
        }
    }
    return jhash_bytes(&fields, sizeof fields, basis);
}

/* Initialize a flow with random fields that matter for nx_hash_fields. */
void
flow_random_hash_fields(struct flow *flow)
{
    uint16_t rnd = random_uint16();

    /* Initialize to all zeros. */
    memset(flow, 0, sizeof *flow);

    eth_addr_random(flow->dl_src);
    eth_addr_random(flow->dl_dst);

    flow->vlan_tci = (OVS_FORCE ovs_be16) (random_uint16() & VLAN_VID_MASK);

    /* Make most of the random flows IPv4, some IPv6, and rest random. */
    flow->dl_type = rnd < 0x8000 ? htons(ETH_TYPE_IP) :
        rnd < 0xc000 ? htons(ETH_TYPE_IPV6) : (OVS_FORCE ovs_be16)rnd;

    if (dl_type_is_ip_any(flow->dl_type)) {
        if (flow->dl_type == htons(ETH_TYPE_IP)) {
            flow->nw_src = (OVS_FORCE ovs_be32)random_uint32();
            flow->nw_dst = (OVS_FORCE ovs_be32)random_uint32();
        } else {
            random_bytes(&flow->ipv6_src, sizeof flow->ipv6_src);
            random_bytes(&flow->ipv6_dst, sizeof flow->ipv6_dst);
        }
        /* Make most of IP flows TCP, some UDP or SCTP, and rest random. */
        rnd = random_uint16();
        flow->nw_proto = rnd < 0x8000 ? IPPROTO_TCP :
            rnd < 0xc000 ? IPPROTO_UDP :
            rnd < 0xd000 ? IPPROTO_SCTP : (uint8_t)rnd;
        if (flow->nw_proto == IPPROTO_TCP ||
            flow->nw_proto == IPPROTO_UDP ||
            flow->nw_proto == IPPROTO_SCTP) {
            flow->tp_src = (OVS_FORCE ovs_be16)random_uint16();
            flow->tp_dst = (OVS_FORCE ovs_be16)random_uint16();
        }
    }
}

/* Masks the fields in 'wc' that are used by the flow hash 'fields'. */
void
flow_mask_hash_fields(const struct flow *flow, struct flow_wildcards *wc,
                      enum nx_hash_fields fields)
{
    switch (fields) {
    case NX_HASH_FIELDS_ETH_SRC:
        memset(&wc->masks.dl_src, 0xff, sizeof wc->masks.dl_src);
        break;

    case NX_HASH_FIELDS_SYMMETRIC_L4:
        memset(&wc->masks.dl_src, 0xff, sizeof wc->masks.dl_src);
        memset(&wc->masks.dl_dst, 0xff, sizeof wc->masks.dl_dst);
        if (flow->dl_type == htons(ETH_TYPE_IP)) {
            memset(&wc->masks.nw_src, 0xff, sizeof wc->masks.nw_src);
            memset(&wc->masks.nw_dst, 0xff, sizeof wc->masks.nw_dst);
        } else if (flow->dl_type == htons(ETH_TYPE_IPV6)) {
            memset(&wc->masks.ipv6_src, 0xff, sizeof wc->masks.ipv6_src);
            memset(&wc->masks.ipv6_dst, 0xff, sizeof wc->masks.ipv6_dst);
        }
        if (is_ip_any(flow)) {
            memset(&wc->masks.nw_proto, 0xff, sizeof wc->masks.nw_proto);
            flow_unwildcard_tp_ports(flow, wc);
        }
        wc->masks.vlan_tci |= htons(VLAN_VID_MASK | VLAN_CFI);
        break;

    default:
        OVS_NOT_REACHED();
    }
}

/* Hashes the portions of 'flow' designated by 'fields'. */
uint32_t
flow_hash_fields(const struct flow *flow, enum nx_hash_fields fields,
                 uint16_t basis)
{
    switch (fields) {

    case NX_HASH_FIELDS_ETH_SRC:
        return jhash_bytes(flow->dl_src, sizeof flow->dl_src, basis);

    case NX_HASH_FIELDS_SYMMETRIC_L4:
        return flow_hash_symmetric_l4(flow, basis);
    }

    OVS_NOT_REACHED();
}

/* Returns a string representation of 'fields'. */
const char *
flow_hash_fields_to_str(enum nx_hash_fields fields)
{
    switch (fields) {
    case NX_HASH_FIELDS_ETH_SRC: return "eth_src";
    case NX_HASH_FIELDS_SYMMETRIC_L4: return "symmetric_l4";
    default: return "<unknown>";
    }
}

/* Returns true if the value of 'fields' is supported. Otherwise false. */
bool
flow_hash_fields_valid(enum nx_hash_fields fields)
{
    return fields == NX_HASH_FIELDS_ETH_SRC
        || fields == NX_HASH_FIELDS_SYMMETRIC_L4;
}

/* Returns a hash value for the bits of 'flow' that are active based on
 * 'wc', given 'basis'. */
uint32_t
flow_hash_in_wildcards(const struct flow *flow,
                       const struct flow_wildcards *wc, uint32_t basis)
{
    const uint32_t *wc_u32 = (const uint32_t *) &wc->masks;
    const uint32_t *flow_u32 = (const uint32_t *) flow;
    uint32_t hash;
    size_t i;

    hash = basis;
    for (i = 0; i < FLOW_U32S; i++) {
        hash = mhash_add(hash, flow_u32[i] & wc_u32[i]);
    }
    return mhash_finish(hash, 4 * FLOW_U32S);
}

/* Sets the VLAN VID that 'flow' matches to 'vid', which is interpreted as an
 * OpenFlow 1.0 "dl_vlan" value:
 *
 *      - If it is in the range 0...4095, 'flow->vlan_tci' is set to match
 *        that VLAN.  Any existing PCP match is unchanged (it becomes 0 if
 *        'flow' previously matched packets without a VLAN header).
 *
 *      - If it is OFP_VLAN_NONE, 'flow->vlan_tci' is set to match a packet
 *        without a VLAN tag.
 *
 *      - Other values of 'vid' should not be used. */
void
flow_set_dl_vlan(struct flow *flow, ovs_be16 vid)
{
    if (vid == htons(OFP10_VLAN_NONE)) {
        flow->vlan_tci = htons(0);
    } else {
        vid &= htons(VLAN_VID_MASK);
        flow->vlan_tci &= ~htons(VLAN_VID_MASK);
        flow->vlan_tci |= htons(VLAN_CFI) | vid;
    }
}

/* Sets the VLAN VID that 'flow' matches to 'vid', which is interpreted as an
 * OpenFlow 1.2 "vlan_vid" value, that is, the low 13 bits of 'vlan_tci' (VID
 * plus CFI). */
void
flow_set_vlan_vid(struct flow *flow, ovs_be16 vid)
{
    ovs_be16 mask = htons(VLAN_VID_MASK | VLAN_CFI);
    flow->vlan_tci &= ~mask;
    flow->vlan_tci |= vid & mask;
}

/* Sets the VLAN PCP that 'flow' matches to 'pcp', which should be in the
 * range 0...7.
 *
 * This function has no effect on the VLAN ID that 'flow' matches.
 *
 * After calling this function, 'flow' will not match packets without a VLAN
 * header. */
void
flow_set_vlan_pcp(struct flow *flow, uint8_t pcp)
{
    pcp &= 0x07;
    flow->vlan_tci &= ~htons(VLAN_PCP_MASK);
    flow->vlan_tci |= htons((pcp << VLAN_PCP_SHIFT) | VLAN_CFI);
}

/* Returns the number of MPLS LSEs present in 'flow'
 *
 * Returns 0 if the 'dl_type' of 'flow' is not an MPLS ethernet type.
 * Otherwise traverses 'flow''s MPLS label stack stopping at the
 * first entry that has the BoS bit set. If no such entry exists then
 * the maximum number of LSEs that can be stored in 'flow' is returned.
 */
int
flow_count_mpls_labels(const struct flow *flow, struct flow_wildcards *wc)
{
    if (wc) {
        wc->masks.dl_type = OVS_BE16_MAX;
    }
    if (eth_type_mpls(flow->dl_type)) {
        int i;
        int len = FLOW_MAX_MPLS_LABELS;

        for (i = 0; i < len; i++) {
            if (wc) {
                wc->masks.mpls_lse[i] |= htonl(MPLS_BOS_MASK);
            }
            if (flow->mpls_lse[i] & htonl(MPLS_BOS_MASK)) {
                return i + 1;
            }
        }

        return len;
    } else {
        return 0;
    }
}

/* Returns the number consecutive of MPLS LSEs, starting at the
 * innermost LSE, that are common in 'a' and 'b'.
 *
 * 'an' must be flow_count_mpls_labels(a).
 * 'bn' must be flow_count_mpls_labels(b).
 */
int
flow_count_common_mpls_labels(const struct flow *a, int an,
                              const struct flow *b, int bn,
                              struct flow_wildcards *wc)
{
    int min_n = MIN(an, bn);
    if (min_n == 0) {
        return 0;
    } else {
        int common_n = 0;
        int a_last = an - 1;
        int b_last = bn - 1;
        int i;

        for (i = 0; i < min_n; i++) {
            if (wc) {
                wc->masks.mpls_lse[a_last - i] = OVS_BE32_MAX;
                wc->masks.mpls_lse[b_last - i] = OVS_BE32_MAX;
            }
            if (a->mpls_lse[a_last - i] != b->mpls_lse[b_last - i]) {
                break;
            } else {
                common_n++;
            }
        }

        return common_n;
    }
}

/* Adds a new outermost MPLS label to 'flow' and changes 'flow''s Ethernet type
 * to 'mpls_eth_type', which must be an MPLS Ethertype.
 *
 * If the new label is the first MPLS label in 'flow', it is generated as;
 *
 *     - label: 2, if 'flow' is IPv6, otherwise 0.
 *
 *     - TTL: IPv4 or IPv6 TTL, if present and nonzero, otherwise 64.
 *
 *     - TC: IPv4 or IPv6 TOS, if present, otherwise 0.
 *
 *     - BoS: 1.
 *
 * If the new label is the second or label MPLS label in 'flow', it is
 * generated as;
 *
 *     - label: Copied from outer label.
 *
 *     - TTL: Copied from outer label.
 *
 *     - TC: Copied from outer label.
 *
 *     - BoS: 0.
 *
 * 'n' must be flow_count_mpls_labels(flow).  'n' must be less than
 * FLOW_MAX_MPLS_LABELS (because otherwise flow->mpls_lse[] would overflow).
 */
void
flow_push_mpls(struct flow *flow, int n, ovs_be16 mpls_eth_type,
               struct flow_wildcards *wc)
{
    ovs_assert(eth_type_mpls(mpls_eth_type));
    ovs_assert(n < FLOW_MAX_MPLS_LABELS);

    memset(wc->masks.mpls_lse, 0xff, sizeof wc->masks.mpls_lse);
    if (n) {
        int i;

        for (i = n; i >= 1; i--) {
            flow->mpls_lse[i] = flow->mpls_lse[i - 1];
        }
        flow->mpls_lse[0] = (flow->mpls_lse[1]
                             & htonl(~MPLS_BOS_MASK));
    } else {
        int label = 0;          /* IPv4 Explicit Null. */
        int tc = 0;
        int ttl = 64;

        if (flow->dl_type == htons(ETH_TYPE_IPV6)) {
            label = 2;
        }

        if (is_ip_any(flow)) {
            tc = (flow->nw_tos & IP_DSCP_MASK) >> 2;
            wc->masks.nw_tos |= IP_DSCP_MASK;

            if (flow->nw_ttl) {
                ttl = flow->nw_ttl;
            }
            wc->masks.nw_ttl = 0xff;
        }

        flow->mpls_lse[0] = set_mpls_lse_values(ttl, tc, 1, htonl(label));

        /* Clear all L3 and L4 fields. */
        BUILD_ASSERT(FLOW_WC_SEQ == 25);
        memset((char *) flow + FLOW_SEGMENT_2_ENDS_AT, 0,
               sizeof(struct flow) - FLOW_SEGMENT_2_ENDS_AT);
    }
    flow->dl_type = mpls_eth_type;
}

/* Tries to remove the outermost MPLS label from 'flow'.  Returns true if
 * successful, false otherwise.  On success, sets 'flow''s Ethernet type to
 * 'eth_type'.
 *
 * 'n' must be flow_count_mpls_labels(flow). */
bool
flow_pop_mpls(struct flow *flow, int n, ovs_be16 eth_type,
              struct flow_wildcards *wc)
{
    int i;

    if (n == 0) {
        /* Nothing to pop. */
        return false;
    } else if (n == FLOW_MAX_MPLS_LABELS
               && !(flow->mpls_lse[n - 1] & htonl(MPLS_BOS_MASK))) {
        /* Can't pop because we don't know what to fill in mpls_lse[n - 1]. */
        return false;
    }

    memset(wc->masks.mpls_lse, 0xff, sizeof wc->masks.mpls_lse);
    for (i = 1; i < n; i++) {
        flow->mpls_lse[i - 1] = flow->mpls_lse[i];
    }
    flow->mpls_lse[n - 1] = 0;
    flow->dl_type = eth_type;
    return true;
}

/* Sets the MPLS Label that 'flow' matches to 'label', which is interpreted
 * as an OpenFlow 1.1 "mpls_label" value. */
void
flow_set_mpls_label(struct flow *flow, int idx, ovs_be32 label)
{
    set_mpls_lse_label(&flow->mpls_lse[idx], label);
}

/* Sets the MPLS TTL that 'flow' matches to 'ttl', which should be in the
 * range 0...255. */
void
flow_set_mpls_ttl(struct flow *flow, int idx, uint8_t ttl)
{
    set_mpls_lse_ttl(&flow->mpls_lse[idx], ttl);
}

/* Sets the MPLS TC that 'flow' matches to 'tc', which should be in the
 * range 0...7. */
void
flow_set_mpls_tc(struct flow *flow, int idx, uint8_t tc)
{
    set_mpls_lse_tc(&flow->mpls_lse[idx], tc);
}

/* Sets the MPLS BOS bit that 'flow' matches to which should be 0 or 1. */
void
flow_set_mpls_bos(struct flow *flow, int idx, uint8_t bos)
{
    set_mpls_lse_bos(&flow->mpls_lse[idx], bos);
}

/* Sets the entire MPLS LSE. */
void
flow_set_mpls_lse(struct flow *flow, int idx, ovs_be32 lse)
{
    flow->mpls_lse[idx] = lse;
}

static size_t
flow_compose_l4(struct ofpbuf *b, const struct flow *flow)
{
    size_t l4_len = 0;

    if (!(flow->nw_frag & FLOW_NW_FRAG_ANY)
        || !(flow->nw_frag & FLOW_NW_FRAG_LATER)) {
        if (flow->nw_proto == IPPROTO_TCP) {
            struct tcp_header *tcp;

            l4_len = sizeof *tcp;
            tcp = ofpbuf_put_zeros(b, l4_len);
            tcp->tcp_src = flow->tp_src;
            tcp->tcp_dst = flow->tp_dst;
            tcp->tcp_ctl = TCP_CTL(ntohs(flow->tcp_flags), 5);
        } else if (flow->nw_proto == IPPROTO_UDP) {
            struct udp_header *udp;

            l4_len = sizeof *udp;
            udp = ofpbuf_put_zeros(b, l4_len);
            udp->udp_src = flow->tp_src;
            udp->udp_dst = flow->tp_dst;
        } else if (flow->nw_proto == IPPROTO_SCTP) {
            struct sctp_header *sctp;

            l4_len = sizeof *sctp;
            sctp = ofpbuf_put_zeros(b, l4_len);
            sctp->sctp_src = flow->tp_src;
            sctp->sctp_dst = flow->tp_dst;
        } else if (flow->nw_proto == IPPROTO_ICMP) {
            struct icmp_header *icmp;

            l4_len = sizeof *icmp;
            icmp = ofpbuf_put_zeros(b, l4_len);
            icmp->icmp_type = ntohs(flow->tp_src);
            icmp->icmp_code = ntohs(flow->tp_dst);
            icmp->icmp_csum = csum(icmp, ICMP_HEADER_LEN);
        } else if (flow->nw_proto == IPPROTO_ICMPV6) {
            struct icmp6_hdr *icmp;

            l4_len = sizeof *icmp;
            icmp = ofpbuf_put_zeros(b, l4_len);
            icmp->icmp6_type = ntohs(flow->tp_src);
            icmp->icmp6_code = ntohs(flow->tp_dst);

            if (icmp->icmp6_code == 0 &&
                (icmp->icmp6_type == ND_NEIGHBOR_SOLICIT ||
                 icmp->icmp6_type == ND_NEIGHBOR_ADVERT)) {
                struct in6_addr *nd_target;
                struct nd_opt_hdr *nd_opt;

                l4_len += sizeof *nd_target;
                nd_target = ofpbuf_put_zeros(b, sizeof *nd_target);
                *nd_target = flow->nd_target;

                if (!eth_addr_is_zero(flow->arp_sha)) {
                    l4_len += 8;
                    nd_opt = ofpbuf_put_zeros(b, 8);
                    nd_opt->nd_opt_len = 1;
                    nd_opt->nd_opt_type = ND_OPT_SOURCE_LINKADDR;
                    memcpy(nd_opt + 1, flow->arp_sha, ETH_ADDR_LEN);
                }
                if (!eth_addr_is_zero(flow->arp_tha)) {
                    l4_len += 8;
                    nd_opt = ofpbuf_put_zeros(b, 8);
                    nd_opt->nd_opt_len = 1;
                    nd_opt->nd_opt_type = ND_OPT_TARGET_LINKADDR;
                    memcpy(nd_opt + 1, flow->arp_tha, ETH_ADDR_LEN);
                }
            }
            icmp->icmp6_cksum = (OVS_FORCE uint16_t)
                csum(icmp, (char *)ofpbuf_tail(b) - (char *)icmp);
        }
    }
    return l4_len;
}

/* Puts into 'b' a packet that flow_extract() would parse as having the given
 * 'flow'.
 *
 * (This is useful only for testing, obviously, and the packet isn't really
 * valid. It hasn't got some checksums filled in, for one, and lots of fields
 * are just zeroed.) */
void
flow_compose(struct ofpbuf *b, const struct flow *flow)
{
    size_t l4_len;

    /* eth_compose() sets l3 pointer and makes sure it is 32-bit aligned. */
    eth_compose(b, flow->dl_dst, flow->dl_src, ntohs(flow->dl_type), 0);
    if (flow->dl_type == htons(FLOW_DL_TYPE_NONE)) {
        struct eth_header *eth = ofpbuf_l2(b);
        eth->eth_type = htons(ofpbuf_size(b));
        return;
    }

    if (flow->vlan_tci & htons(VLAN_CFI)) {
        eth_push_vlan(b, htons(ETH_TYPE_VLAN), flow->vlan_tci);
    }

    if (flow->dl_type == htons(ETH_TYPE_IP)) {
        struct ip_header *ip;

        ip = ofpbuf_put_zeros(b, sizeof *ip);
        ip->ip_ihl_ver = IP_IHL_VER(5, 4);
        ip->ip_tos = flow->nw_tos;
        ip->ip_ttl = flow->nw_ttl;
        ip->ip_proto = flow->nw_proto;
        put_16aligned_be32(&ip->ip_src, flow->nw_src);
        put_16aligned_be32(&ip->ip_dst, flow->nw_dst);

        if (flow->nw_frag & FLOW_NW_FRAG_ANY) {
            ip->ip_frag_off |= htons(IP_MORE_FRAGMENTS);
            if (flow->nw_frag & FLOW_NW_FRAG_LATER) {
                ip->ip_frag_off |= htons(100);
            }
        }

        ofpbuf_set_l4(b, ofpbuf_tail(b));

        l4_len = flow_compose_l4(b, flow);

        ip->ip_tot_len = htons(b->l4_ofs - b->l3_ofs + l4_len);
        ip->ip_csum = csum(ip, sizeof *ip);
    } else if (flow->dl_type == htons(ETH_TYPE_IPV6)) {
        struct ovs_16aligned_ip6_hdr *nh;

        nh = ofpbuf_put_zeros(b, sizeof *nh);
#ifdef _WIN32
		put_16aligned_be32(&nh->ip6_ctlun.ip6_un1.ip6_un1_flow, htonl(6 << 28) |
#else
        put_16aligned_be32(&nh->ip6_flow, htonl(6 << 28) |
#endif
                           htonl(flow->nw_tos << 20) | flow->ipv6_label);
#ifdef _WIN32
		nh->ip6_ctlun.ip6_un1.ip6_un1_hlim = flow->nw_ttl;
		nh->ip6_ctlun.ip6_un1.ip6_un1_nxt = flow->nw_proto;
#else
        nh->ip6_hlim = flow->nw_ttl;
		nh->ip6_nxt = flow->nw_proto;
#endif

        memcpy(&nh->ip6_src, &flow->ipv6_src, sizeof(nh->ip6_src));
        memcpy(&nh->ip6_dst, &flow->ipv6_dst, sizeof(nh->ip6_dst));

        ofpbuf_set_l4(b, ofpbuf_tail(b));

        l4_len = flow_compose_l4(b, flow);

#ifdef _WIN32
		nh->ip6_ctlun.ip6_un1.ip6_un1_plen = htons(l4_len);
#else
        nh->ip6_plen = htons(l4_len);
#endif
    } else if (flow->dl_type == htons(ETH_TYPE_ARP) ||
               flow->dl_type == htons(ETH_TYPE_RARP)) {
        struct arp_eth_header *arp;

        arp = ofpbuf_put_zeros(b, sizeof *arp);
        ofpbuf_set_l3(b, arp);
        arp->ar_hrd = htons(1);
        arp->ar_pro = htons(ETH_TYPE_IP);
        arp->ar_hln = ETH_ADDR_LEN;
        arp->ar_pln = 4;
        arp->ar_op = htons(flow->nw_proto);

        if (flow->nw_proto == ARP_OP_REQUEST ||
            flow->nw_proto == ARP_OP_REPLY) {
            put_16aligned_be32(&arp->ar_spa, flow->nw_src);
            put_16aligned_be32(&arp->ar_tpa, flow->nw_dst);
            memcpy(arp->ar_sha, flow->arp_sha, ETH_ADDR_LEN);
            memcpy(arp->ar_tha, flow->arp_tha, ETH_ADDR_LEN);
        }
    }

    if (eth_type_mpls(flow->dl_type)) {
        int n;

        b->l2_5_ofs = b->l3_ofs;
        for (n = 1; n < FLOW_MAX_MPLS_LABELS; n++) {
            if (flow->mpls_lse[n - 1] & htonl(MPLS_BOS_MASK)) {
                break;
            }
        }
        while (n > 0) {
            push_mpls(b, flow->dl_type, flow->mpls_lse[--n]);
        }
    }
}

/* Compressed flow. */

static int
miniflow_n_values(const struct miniflow *flow)
{
    return count_1bits(flow->map);
}

static uint32_t *
miniflow_alloc_values(struct miniflow *flow, int n)
{
    if (n <= MINI_N_INLINE) {
        return flow->inline_values;
    } else {
        COVERAGE_INC(miniflow_malloc);
        return xmalloc(n * sizeof *flow->values);
    }
}

/* Completes an initialization of 'dst' as a miniflow copy of 'src' begun by
 * the caller.  The caller must have already initialized 'dst->map' properly
 * to indicate the significant uint32_t elements of 'src'.  'n' must be the
 * number of 1-bits in 'dst->map'.
 *
 * Normally the significant elements are the ones that are non-zero.  However,
 * when a miniflow is initialized from a (mini)mask, the values can be zeroes,
 * so that the flow and mask always have the same maps.
 *
 * This function initializes 'dst->values' (either inline if possible or with
 * malloc() otherwise) and copies the uint32_t elements of 'src' indicated by
 * 'dst->map' into it. */
static void
miniflow_init__(struct miniflow *dst, const struct flow *src, int n)
{
    const uint32_t *src_u32 = (const uint32_t *) src;
    unsigned int ofs;
    uint64_t map;

    dst->values = miniflow_alloc_values(dst, n);
    ofs = 0;
    for (map = dst->map; map; map = zero_rightmost_1bit(map)) {
        dst->values[ofs++] = src_u32[raw_ctz(map)];
    }
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with miniflow_destroy(). */
void
miniflow_init(struct miniflow *dst, const struct flow *src)
{
    const uint32_t *src_u32 = (const uint32_t *) src;
    unsigned int i;
    int n;

    /* Initialize dst->map, counting the number of nonzero elements. */
    n = 0;
    dst->map = 0;

    for (i = 0; i < FLOW_U32S; i++) {
        if (src_u32[i]) {
            dst->map |= UINT64_C(1) << i;
            n++;
        }
    }

    miniflow_init__(dst, src, n);
}

/* Initializes 'dst' as a copy of 'src', using 'mask->map' as 'dst''s map.  The
 * caller must eventually free 'dst' with miniflow_destroy(). */
void
miniflow_init_with_minimask(struct miniflow *dst, const struct flow *src,
                            const struct minimask *mask)
{
    dst->map = mask->masks.map;
    miniflow_init__(dst, src, miniflow_n_values(dst));
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with miniflow_destroy(). */
void
miniflow_clone(struct miniflow *dst, const struct miniflow *src)
{
    int n = miniflow_n_values(src);
    dst->map = src->map;
    dst->values = miniflow_alloc_values(dst, n);
    memcpy(dst->values, src->values, n * sizeof *dst->values);
}

/* Initializes 'dst' with the data in 'src', destroying 'src'.
 * The caller must eventually free 'dst' with miniflow_destroy(). */
void
miniflow_move(struct miniflow *dst, struct miniflow *src)
{
    if (src->values == src->inline_values) {
        dst->values = dst->inline_values;
        memcpy(dst->values, src->values,
               miniflow_n_values(src) * sizeof *dst->values);
    } else {
        dst->values = src->values;
    }
    dst->map = src->map;
}

/* Frees any memory owned by 'flow'.  Does not free the storage in which 'flow'
 * itself resides; the caller is responsible for that. */
void
miniflow_destroy(struct miniflow *flow)
{
    if (flow->values != flow->inline_values) {
        free(flow->values);
    }
}

/* Initializes 'dst' as a copy of 'src'. */
void
miniflow_expand(const struct miniflow *src, struct flow *dst)
{
    memset(dst, 0, sizeof *dst);
    flow_union_with_miniflow(dst, src);
}

static const uint32_t *
miniflow_get__(const struct miniflow *flow, unsigned int u32_ofs)
{
    if (!(flow->map & (UINT64_C(1) << u32_ofs))) {
        static const uint32_t zero = 0;
        return &zero;
    }
    return flow->values +
           count_1bits(flow->map & ((UINT64_C(1) << u32_ofs) - 1));
}

/* Returns the uint32_t that would be at byte offset '4 * u32_ofs' if 'flow'
 * were expanded into a "struct flow". */
uint32_t
miniflow_get(const struct miniflow *flow, unsigned int u32_ofs)
{
    return *miniflow_get__(flow, u32_ofs);
}

/* Returns the ovs_be16 that would be at byte offset 'u8_ofs' if 'flow' were
 * expanded into a "struct flow". */
static ovs_be16
miniflow_get_be16(const struct miniflow *flow, unsigned int u8_ofs)
{
    const uint32_t *u32p = miniflow_get__(flow, u8_ofs / 4);
    const ovs_be16 *be16p = (const ovs_be16 *) u32p;
    return be16p[u8_ofs % 4 != 0];
}

/* Returns the VID within the vlan_tci member of the "struct flow" represented
 * by 'flow'. */
uint16_t
miniflow_get_vid(const struct miniflow *flow)
{
    ovs_be16 tci = miniflow_get_be16(flow, offsetof(struct flow, vlan_tci));
    return vlan_tci_to_vid(tci);
}

/* Returns true if 'a' and 'b' are the same flow, false otherwise.  */
bool
miniflow_equal(const struct miniflow *a, const struct miniflow *b)
{
    const uint32_t *ap = a->values;
    const uint32_t *bp = b->values;
    const uint64_t a_map = a->map;
    const uint64_t b_map = b->map;
    uint64_t map;

    if (a_map == b_map) {
        for (map = a_map; map; map = zero_rightmost_1bit(map)) {
            if (*ap++ != *bp++) {
                return false;
            }
        }
    } else {
        for (map = a_map | b_map; map; map = zero_rightmost_1bit(map)) {
            uint64_t bit = rightmost_1bit(map);
            uint64_t a_value = a_map & bit ? *ap++ : 0;
            uint64_t b_value = b_map & bit ? *bp++ : 0;

            if (a_value != b_value) {
                return false;
            }
        }
    }

    return true;
}

/* Returns true if 'a' and 'b' are equal at the places where there are 1-bits
 * in 'mask', false if they differ. */
bool
miniflow_equal_in_minimask(const struct miniflow *a, const struct miniflow *b,
                           const struct minimask *mask)
{
    const uint32_t *p;
    uint64_t map;

    p = mask->masks.values;

    for (map = mask->masks.map; map; map = zero_rightmost_1bit(map)) {
        int ofs = raw_ctz(map);

        if ((miniflow_get(a, ofs) ^ miniflow_get(b, ofs)) & *p) {
            return false;
        }
        p++;
    }

    return true;
}

/* Returns true if 'a' and 'b' are equal at the places where there are 1-bits
 * in 'mask', false if they differ. */
bool
miniflow_equal_flow_in_minimask(const struct miniflow *a, const struct flow *b,
                                const struct minimask *mask)
{
    const uint32_t *b_u32 = (const uint32_t *) b;
    const uint32_t *p;
    uint64_t map;

    p = mask->masks.values;

    for (map = mask->masks.map; map; map = zero_rightmost_1bit(map)) {
        int ofs = raw_ctz(map);

        if ((miniflow_get(a, ofs) ^ b_u32[ofs]) & *p) {
            return false;
        }
        p++;
    }

    return true;
}

/* Returns a hash value for 'flow', given 'basis'. */
uint32_t
miniflow_hash(const struct miniflow *flow, uint32_t basis)
{
    const uint32_t *p = flow->values;
    uint32_t hash = basis;
    uint64_t hash_map = 0;
    uint64_t map;

    for (map = flow->map; map; map = zero_rightmost_1bit(map)) {
        if (*p) {
            hash = mhash_add(hash, *p);
            hash_map |= rightmost_1bit(map);
        }
        p++;
    }
    hash = mhash_add(hash, hash_map);
    hash = mhash_add(hash, hash_map >> 32);

    return mhash_finish(hash, p - flow->values);
}

/* Returns a hash value for the bits of 'flow' where there are 1-bits in
 * 'mask', given 'basis'.
 *
 * The hash values returned by this function are the same as those returned by
 * flow_hash_in_minimask(), only the form of the arguments differ. */
uint32_t
miniflow_hash_in_minimask(const struct miniflow *flow,
                          const struct minimask *mask, uint32_t basis)
{
    const uint32_t *p = mask->masks.values;
    uint32_t hash;
    uint64_t map;

    hash = basis;

    for (map = mask->masks.map; map; map = zero_rightmost_1bit(map)) {
        hash = mhash_add(hash, miniflow_get(flow, raw_ctz(map)) & *p++);
    }

    return mhash_finish(hash, (p - mask->masks.values) * 4);
}

/* Returns a hash value for the bits of 'flow' where there are 1-bits in
 * 'mask', given 'basis'.
 *
 * The hash values returned by this function are the same as those returned by
 * miniflow_hash_in_minimask(), only the form of the arguments differ. */
uint32_t
flow_hash_in_minimask(const struct flow *flow, const struct minimask *mask,
                      uint32_t basis)
{
    const uint32_t *flow_u32 = (const uint32_t *)flow;
    const uint32_t *p = mask->masks.values;
    uint32_t hash;
    uint64_t map;

    hash = basis;
    for (map = mask->masks.map; map; map = zero_rightmost_1bit(map)) {
        hash = mhash_add(hash, flow_u32[raw_ctz(map)] & *p++);
    }

    return mhash_finish(hash, (p - mask->masks.values) * 4);
}

/* Returns a hash value for the bits of range [start, end) in 'flow',
 * where there are 1-bits in 'mask', given 'hash'.
 *
 * The hash values returned by this function are the same as those returned by
 * minimatch_hash_range(), only the form of the arguments differ. */
uint32_t
flow_hash_in_minimask_range(const struct flow *flow,
                            const struct minimask *mask,
                            uint8_t start, uint8_t end, uint32_t *basis)
{
    const uint32_t *flow_u32 = (const uint32_t *)flow;
    unsigned int offset;
    uint64_t map = miniflow_get_map_in_range(&mask->masks, start, end,
                                             &offset);
    const uint32_t *p = mask->masks.values + offset;
    uint32_t hash = *basis;

    for (; map; map = zero_rightmost_1bit(map)) {
        hash = mhash_add(hash, flow_u32[raw_ctz(map)] & *p++);
    }

    *basis = hash; /* Allow continuation from the unfinished value. */
    return mhash_finish(hash, (p - mask->masks.values) * 4);
}


/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimask_destroy(). */
void
minimask_init(struct minimask *mask, const struct flow_wildcards *wc)
{
    miniflow_init(&mask->masks, &wc->masks);
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimask_destroy(). */
void
minimask_clone(struct minimask *dst, const struct minimask *src)
{
    miniflow_clone(&dst->masks, &src->masks);
}

/* Initializes 'dst' with the data in 'src', destroying 'src'.
 * The caller must eventually free 'dst' with minimask_destroy(). */
void
minimask_move(struct minimask *dst, struct minimask *src)
{
    miniflow_move(&dst->masks, &src->masks);
}

/* Initializes 'dst_' as the bit-wise "and" of 'a_' and 'b_'.
 *
 * The caller must provide room for FLOW_U32S "uint32_t"s in 'storage', for use
 * by 'dst_'.  The caller must *not* free 'dst_' with minimask_destroy(). */
void
minimask_combine(struct minimask *dst_,
                 const struct minimask *a_, const struct minimask *b_,
                 uint32_t storage[FLOW_U32S])
{
    struct miniflow *dst = &dst_->masks;
    const struct miniflow *a = &a_->masks;
    const struct miniflow *b = &b_->masks;
    uint64_t map;
    int n = 0;

    dst->values = storage;

    dst->map = 0;
    for (map = a->map & b->map; map; map = zero_rightmost_1bit(map)) {
        int ofs = raw_ctz(map);
        uint32_t mask = miniflow_get(a, ofs) & miniflow_get(b, ofs);

        if (mask) {
            dst->map |= rightmost_1bit(map);
            dst->values[n++] = mask;
        }
    }
}

/* Frees any memory owned by 'mask'.  Does not free the storage in which 'mask'
 * itself resides; the caller is responsible for that. */
void
minimask_destroy(struct minimask *mask)
{
    miniflow_destroy(&mask->masks);
}

/* Initializes 'dst' as a copy of 'src'. */
void
minimask_expand(const struct minimask *mask, struct flow_wildcards *wc)
{
    miniflow_expand(&mask->masks, &wc->masks);
}

/* Returns the uint32_t that would be at byte offset '4 * u32_ofs' if 'mask'
 * were expanded into a "struct flow_wildcards". */
uint32_t
minimask_get(const struct minimask *mask, unsigned int u32_ofs)
{
    return miniflow_get(&mask->masks, u32_ofs);
}

/* Returns the VID mask within the vlan_tci member of the "struct
 * flow_wildcards" represented by 'mask'. */
uint16_t
minimask_get_vid_mask(const struct minimask *mask)
{
    return miniflow_get_vid(&mask->masks);
}

/* Returns true if 'a' and 'b' are the same flow mask, false otherwise.  */
bool
minimask_equal(const struct minimask *a, const struct minimask *b)
{
    return miniflow_equal(&a->masks, &b->masks);
}

/* Returns a hash value for 'mask', given 'basis'. */
uint32_t
minimask_hash(const struct minimask *mask, uint32_t basis)
{
    return miniflow_hash(&mask->masks, basis);
}

/* Returns true if at least one bit is wildcarded in 'a_' but not in 'b_',
 * false otherwise. */
bool
minimask_has_extra(const struct minimask *a_, const struct minimask *b_)
{
    const struct miniflow *a = &a_->masks;
    const struct miniflow *b = &b_->masks;
    uint64_t map;

    for (map = a->map | b->map; map; map = zero_rightmost_1bit(map)) {
        int ofs = raw_ctz(map);
        uint32_t a_u32 = miniflow_get(a, ofs);
        uint32_t b_u32 = miniflow_get(b, ofs);

        if ((a_u32 & b_u32) != b_u32) {
            return true;
        }
    }

    return false;
}

/* Returns true if 'mask' matches every packet, false if 'mask' fixes any bits
 * or fields. */
bool
minimask_is_catchall(const struct minimask *mask_)
{
    const struct miniflow *mask = &mask_->masks;
    const uint32_t *p = mask->values;
    uint64_t map;

    for (map = mask->map; map; map = zero_rightmost_1bit(map)) {
        if (*p++) {
            return false;
        }
    }
    return true;
}
