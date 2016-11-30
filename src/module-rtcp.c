/*
    $Id: module-rtcp.c,v 1.23 2008-01-12 16:10:22 awgn Exp $

    Copyright (c) 2003 Nicola Bonelli <bonelli@netserv.iet.unipi.it>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <sys/ioctl.h>
#include <t-module.h>
#include <math.h>

MODULE_CVS("$Id: module-rtcp.c,v 1.23 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@netserv.iet.unipi.it>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);


/* opaque space */
struct mod_line {
    /* ipv4 */
    uint32_t                   AL(msec);
    uint32_t                   AL(rate);
    uint32_t                   AL(len);
    uint32_t                   AL(tos);
    uint32_t                   AL(ttl);
    /* ipv6 */
    uint32_t                   AL(class);
    uint32_t                   AL(flow);
    uint32_t                   AL(hoplim);

    struct hostent *        AL(saddr);
    struct hostent *        AL(daddr);
    /* udp */
    u_short                 AL(sport);
    u_short                 AL(dport);
};


#define TOKEN_msec      0
#define TOKEN_rate      1
#define TOKEN_len       2
#define TOKEN_tos       3
#define TOKEN_ttl       4

#define TOKEN_class     5
#define TOKEN_flow      6
#define TOKEN_hoplim    7

#define TOKEN_saddr     8
#define TOKEN_sport     9
#define TOKEN_daddr     10
#define TOKEN_dport     11


/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_engine:       u_engine,
                h_parser:       u_parser,
                command:        "rtcp",
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     12,
                token_list:     { TOKEN(msec), TOKEN(rate),TOKEN(len),TOKEN(tos),TOKEN(ttl),
                    TOKEN(class), TOKEN(flow), TOKEN(hoplim),TOKEN(saddr),TOKEN(sport),
                    TOKEN(daddr),TOKEN(dport)},
};


INIT_MODULE(module);


/*** parser handler ***/

static void
u_parser(int t, struct atom *v, cmdline_t *cmd )
{

    INIT_PARSER(cmd,struct mod_line,v);

    switch (t) {
    case TOKEN_msec:
        TAG(msec)= cast_ret(brute_eval_int, v);
        break;
    case TOKEN_rate:
        TAG(rate)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_len:
        TAG(len)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_tos:
        TAG(tos)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_ttl:
        TAG(ttl)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_class:
        TAG(class)= cast_ret(brute_eval_int,v);
        break;
    case TOKEN_flow:
        TAG(flow)= cast_ret(brute_eval_int,v);
        break;
    case TOKEN_hoplim:
        TAG(hoplim)= cast_ret(brute_eval_int,v);
        break;
    case TOKEN_saddr:
        TAG(saddr)= cast_ret(brute_eval_host,v);
        break;
    case TOKEN_daddr:
        TAG(daddr)= cast_ret(brute_eval_host,v);
        break;
    case TOKEN_sport:
        TAG(sport)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_dport:
        TAG(dport)= cast_ret(brute_eval_atom,v);
        break;
    default:
        PARSER_ERROR(v);
    }

}


/***                      ***
 ***  ----- rfc3550 ----- ***
 ***                      ***/

typedef unsigned char  uint32_t8;
typedef unsigned short uint32_t16;
typedef uint32_t   uint32_t;
typedef          short int16;

/*
 * Current protocol version.
 */
#define RTP_VERSION    2

#define RTP_SEQ_MOD (1<<16)
#define RTP_MAX_SDES 255      /* maximum text length for SDES */

typedef enum {
    RTCP_SR   = 200,
    RTCP_RR   = 201,
    RTCP_SDES = 202,
    RTCP_BYE  = 203,
    RTCP_APP  = 204
} rtcp_type_t;


typedef enum {
    RTCP_SDES_END   = 0,
    RTCP_SDES_CNAME = 1,
    RTCP_SDES_NAME  = 2,
    RTCP_SDES_EMAIL = 3,
    RTCP_SDES_PHONE = 4,
    RTCP_SDES_LOC   = 5,
    RTCP_SDES_TOOL  = 6,
    RTCP_SDES_NOTE  = 7,
    RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_t;


/*
 * RTCP common header word
 */
typedef struct {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t version:2;   /* protocol version */
    uint32_t p:1;         /* padding flag */
    uint32_t count:5;     /* varies by packet type */
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint32_t count:5;     /* varies by packet type */
    uint32_t p:1;         /* padding flag */
    uint32_t version:2;   /* protocol version */
#endif
    uint32_t pt:8;        /* RTCP packet type */
    uint32_t16 length;           /* pkt len in words, w/o this word */
} rtcp_common_t;


/*
 * Big-endian mask for version, padding bit and packet type pair
 */
#define RTCP_VALID_MASK (0xc000 | 0x2000 | 0xfe)
#define RTCP_VALID_VALUE ((RTP_VERSION << 14) | RTCP_SR)


/*
 * Reception report block
 */
typedef struct {
    uint32_t ssrc;             /* data source being reported */
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t fraction:8;  /* fraction lost since last SR/RR */
    int lost:24;              /* cumul. no. pkts lost (signed!) */
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    int lost:24;              /* cumul. no. pkts lost (signed!) */
    uint32_t fraction:8;  /* fraction lost since last SR/RR */
#endif
    uint32_t last_seq;         /* extended last seq. no. received */
    uint32_t jitter;           /* interarrival jitter */
    uint32_t lsr;              /* last SR packet from this source */
    uint32_t dlsr;             /* delay since last SR packet */
} rtcp_rr_t;


/*
 * SDES item
 */
typedef struct {
    uint32_t8 type;              /* type of item (rtcp_sdes_type_t) */
    uint32_t8 length;            /* length of item (in octets) */
    char data[1];             /* text, not null-terminated */
} rtcp_sdes_item_t;


/*
 * One RTCP packet
 */
typedef struct {
    rtcp_common_t common;     /* common header */
    union {
        /* sender report (SR) */
        struct {
            uint32_t ssrc;     /* sender generating this report */
            uint32_t ntp_sec;  /* NTP timestamp */
            uint32_t ntp_frac;
            uint32_t rtp_ts;   /* RTP timestamp */
            uint32_t psent;    /* packets sent */
            uint32_t osent;    /* octets sent */
            rtcp_rr_t rr[1];  /* variable-length list */
        } sr;

        /* reception report (RR) */
        struct {
            uint32_t ssrc;     /* receiver generating this report */
            rtcp_rr_t rr[1];  /* variable-length list */
        } rr;

        /* source description (SDES) */
        struct rtcp_sdes {
            uint32_t src;      /* first SSRC/CSRC */
            rtcp_sdes_item_t item[1]; /* list of SDES items */
        } sdes;

        /* BYE */
        struct {
            uint32_t src[1];   /* list of sources */
            /* can't express trailing text for reason */
        } bye;
    } r;
} rtcp_t;



/*
 * timeval util
 */

static int
timeval_diff(struct timeval * new, struct timeval * old)
{
    int usec;
    usec = (new->tv_sec-old->tv_sec) * 1000000 + (new->tv_usec-old->tv_usec);
    return usec;
}

static struct timeval
timeval_add(struct timeval *now, int usec)
{
    struct timeval ret;

    ret.tv_sec = now->tv_sec + (now->tv_usec + usec)/1000000;
    ret.tv_usec= (now->tv_usec + usec)%1000000;
    return (ret);
}

/*
 * low-pass filter
 */
static int
lp_filter(int xn)
{
    double alpha=0.6;
    static double yn_1;

    yn_1 = alpha*yn_1 + (1-alpha)*xn;
    return ( (int)rint(yn_1) );
}


/*** engine handler ***/

static char buffer[128];

static void
u_engine(cycles_t *exit_time,cmdline_t *cmd)
{
    cycles_t inter_time, ts;
    rtcp_t *rtcp_sr, *rtcp_ss;

    static frame_t *arena;

    struct mod_line *p;
    struct timeval user_ts, opt_ts, real_ts;
    struct sockaddr_ll addr_ll;

    uint32_t hsent, hsniff;		/* hash */

    int bytes, so_packet;
    int err, tau;				/* estimated userspace device-driver delay */

    socklen_t so_len;

    double err2;

    /* init */

    hsent=0;
    hsniff=0;
    tau=0;
    err2=0;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line,p);

    /* print banner */
    BANNER("%s%c%d -> %s%c%d @%d fps", 	brute_inet_ntop(p->saddr), hp_sep, p->sport,\
           brute_inet_ntop(p->daddr), hp_sep, p->dport,p->rate);


    /* open pf_packet socket to read tstamps of outgoing packets*/
    so_packet = socket (PF_PACKET,SOCK_DGRAM, htons(ETH_P_ALL));
    if (so_packet == -1)
        fatal("socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL))=-1");


    /* alloc frame and set it up */
    arena = (frame_t *)brute_realloc_frame(arena);


    brute_build_mac(arena,&global.ethh); 				// global ethernet option

    brute_build_ip  (arena,                                         // frame pointer
                     p->len,                                      // frame length

                     (u_short)global._sent,                        // ip_id                *v4
                     p->ttl,                                      // ttl                  *v4
                     p->tos,                                      // tos                  *v4

                     p->class,                                    // traffic class        *v6
                     p->flow,                                     // flow label           *v6
                     p->hoplim,                                   // hop limit            *v6

                     p->saddr,                                    // source ip
                     p->daddr);                                   // destination ip


    brute_build_udp(arena,
                    p->sport,					// source port
                    p->dport,					// destination port
                    p->len-sizeof(struct ethhdr)	\
                    -sizeof(struct iphdr)	\
                    -sizeof(uint32_t),				// eth crc
                    0);						// udp checksum not needed

    /*** build the udp_data according to rfc2544 frame format ***/
    brute_udpdata_rfc2544(arena);


    /*** SR: add the report header (RTCP packet, RFC3550) ***/
    rtcp_sr = (rtcp_t *)arena->udata;	/* allign rtcp header to udp-data */
    rtcp_ss = (rtcp_t *)\
              (buffer+sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct udphdr));

    rtcp_sr->common.version= 2;		/* rtcp version 2 */
    rtcp_sr->common.p=0;			/* don't padding */
    rtcp_sr->common.count=0;		/* 0 report blocks */
    rtcp_sr->common.pt=RTCP_SR;		/* SR */
    rtcp_sr->common.length= htons(6);	/* length in words -1 */

    /*** sr ***/
    rtcp_sr->r.sr.ssrc= 0xdeadbeef;		/* sender id */

    rtcp_sr->r.sr.psent= 0;			/* ignored */
    rtcp_sr->r.sr.osent= 0;			/* ignored */


    /*** ensure data are ok  ***/
    ASSERT(p->len  > 73);
    ASSERT(p->rate > 0);

    /*** calc the correct number of bytes to pass to sendto() ***/
    bytes = brute_framelen_to_bytes(p->len);

    /*** according to the rate requested, determine the interdeparture-time of packets.
      Both inter_time and HZ are expressed in time step counters (cycles_t) ***/

    inter_time = (cycles_t)(Hz/p->rate);

    /*** start ***/
    global._start = global._sent;

    /*** disable paging ***/
    mlockall(MCL_FUTURE);

    /*** send packets util the exit_time expires ***/
    for ( ts = get_cycles()+inter_time; get_cycles() < *exit_time; ) {

        gettimeofday(&user_ts,NULL);
        opt_ts = timeval_add(&user_ts,tau);

        rtcp_sr->r.sr.ntp_sec = htonl(opt_ts.tv_sec);    /* gettimeofday() sec */
        rtcp_sr->r.sr.ntp_frac= htonl(opt_ts.tv_usec);
        rtcp_sr->r.sr.rtp_ts  = htonl(opt_ts.tv_usec);    /* gettimeofday() usec */

        /* calc packet's hash */
        hsent = hash (arena->data+ (arena->cooked ? 0 : sizeof(struct ethhdr)),
                      sizeof(struct iphdr)+
                      sizeof(struct udphdr)+
                      sizeof(rtcp_common_t));

        /* send rtcp packet */
        if( brute_sendto(global.sout_fd, arena, bytes, 0) == -1 )
            continue;

        /* sniff frames util the current rtcp packet is read */
        do {
            if (recvfrom(so_packet, buffer, 128, 0, (struct sockaddr *) &addr_ll, &so_len) == -1)
                continue;

            hsniff = hash (buffer, sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(rtcp_common_t));
        }
        while ( addr_ll.sll_pkttype != PACKET_OUTGOING ||
                hsent != hsniff );

        /* retrieve the real timestamp */
        ioctl(so_packet, SIOCGSTAMP, &real_ts);

        err   = timeval_diff(&real_ts,&opt_ts);
        err2 += err*err;

        xprintf(stdout,"\t     user_ts=%lu.%lu real_ts=%lu.%lu - tau=%d usec\t[err=%d usec]\n",
                opt_ts.tv_sec, opt_ts.tv_usec,
                real_ts.tv_sec, real_ts.tv_usec, tau, err);

        /* predicting tau */
        tau = lp_filter (timeval_diff(&real_ts,&user_ts) );

        /* increment the global counter */
        global._sent++;

        /* update packet */
        brute_update_frame(arena);

        /* sleep until the step expires */
        brute_wait_until(&ts);

        /* increment ts */
        ts += inter_time;
    }

    /*** reenable paging ***/
    munlockall();

    /*** print banner ***/
    {
        unsigned long long sent = global._sent-global._start;                     /* packet sent */
        unsigned long long req  = (unsigned long long)p->rate*p->msec/1000;     /* packet to be sent */
        unsigned long long arate= (unsigned long long)sent*1000/p->msec;        /* average rate */
        double jitter   = (double)sent/req*100-100;                             /* variation percentage */

        xprintf(stderr,	"\t     @%lld (fps)  sendto()=#%lld/%lld  jitter+=%.2f%%  E[err^2]=%f\n",
                arate,sent,req,jitter,err2/sent);
    }

    /*** the frame_t is realloced each time the handler start. Therefore it is not required to be freed ***/
}

