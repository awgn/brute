/*
    $Id: module-rtcp-loop.c,v 1.19 2008-01-12 16:10:22 awgn Exp $

    Copyright (c) 2004 Nicola Bonelli <bonelli@netserv.iet.unipi.it>


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
#include <sys/time.h>
#include <t-module.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <time.h>

MODULE_CVS("$Id: module-rtcp-loop.c,v 1.19 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@netserv.iet.unipi.it>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* opaque space */
struct mod_line {
    uint32_t		AL(msec);
    uint32_t		AL(rate);
    uint32_t		AL(len);
    uint32_t		AL(tos);
    uint32_t		AL(ttl);
    uint32_t		AL(stat);
    /* ipv6 */
    uint32_t                   AL(class);
    uint32_t                   AL(flow);
    uint32_t                   AL(hoplim);

    struct hostent *        AL(saddr);
    struct hostent *        AL(daddr);
    /* udp */
    u_short		AL(sport);
    u_short 	AL(dport);
    double		AL(delay);	/* usec */
};


#define TOKEN_msec	0
#define TOKEN_rate	1
#define TOKEN_len	2
#define TOKEN_tos	3
#define TOKEN_ttl	4
#define TOKEN_stat	5

#define TOKEN_class    	6
#define TOKEN_flow      7
#define TOKEN_hoplim    8

#define TOKEN_saddr	9
#define TOKEN_sport  	10
#define TOKEN_daddr	11
#define TOKEN_dport  	12
#define TOKEN_delay	13

#define RETRIEVE_LIMIT	10000	/* max number of recvfrom() */
#define RECOVERY_TIME	2000	/* msec */

/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_engine:       u_engine,
                h_parser:       u_parser,
                command:        "rtcp_loop",
                author:		"Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     14,
                token_list:     { TOKEN(msec), TOKEN(rate),TOKEN(len),TOKEN(tos),TOKEN(ttl),
                    TOKEN(class), TOKEN(flow), TOKEN(hoplim), TOKEN(stat),
                    TOKEN(saddr),TOKEN(sport),TOKEN(daddr),TOKEN(dport), TOKEN(delay) },
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
    case TOKEN_stat:
        TAG(stat) = cast_ret(brute_eval_int,v);
        break;
    case TOKEN_delay:
        TAG(delay)= cast_ret(brute_eval_double,v);
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


/*** engine handler ***/
static char buffer[1518];


static void
abort_recvfrom(int s)
{
    msg( MSG_FATAL "timeout: rtcp packet lost in space ? ;-)           \n");
}


/* rtcp filter */

#define RECV_RTCP()     ( (ips->protocol == IPPROTO_UDP) && \
                          (rtcp_ss->r.sr.ssrc == 0xdeadbeef) && \
                          (rtcp_ss->common.pt == RTCP_SR ) )

static void
u_engine(cycles_t *exit_time,cmdline_t *cmd)
{
    cycles_t inter_time, stop_time, ts;
    static frame_t *arena;
    struct mod_line *p;
    rtcp_t *rtcp_sr, *rtcp_ss;
    struct iphdr *ips;
#if 0
    struct udphdr *udps;
#endif

    struct timeval opt_ts, recv_ts;
    struct sockaddr_ll addr_ll;
    int i, bytes, slen, precv, rtest;

    socklen_t so_len;

    long double sum=.0, sum2=.0, mean=.0, stddev=.0;

    int sample, sample_tv_sec, sample_tv_usec;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line,p);

    /* print banner */
    BANNER("%s%c%d -> %s%c%d @%d fps",      brute_inet_ntop(p->saddr), hp_sep, p->sport,\
           brute_inet_ntop(p->daddr), hp_sep, p->dport,p->rate);


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


    /*** incoming socket ***/
    ips = (struct iphdr *) (buffer+sizeof(struct ethhdr));

#if 0
    udps = (struct udphdr *) (buffer+sizeof(struct ethhdr)+sizeof(struct iphdr));
#endif

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
    ASSERT(global.sin_fd != 0);
    // ASSERT(p->stat != 1 );

    /*** signal ***/
    signal_bsd(SIGALRM, abort_recvfrom);

    /*** calc the correct number of bytes to pass to sendto() ***/
    bytes = brute_framelen_to_bytes(p->len);

    /*** according to the rate requested, determine the interdeparture-time of packets.
      Both inter_time and HZ are expressed in time step counters (cycles_t) ***/

    inter_time = (cycles_t)(Hz/p->rate);

    /*** start ***/
    global._start = global._sent;

    /*** disable paging ***/
    mlockall(MCL_FUTURE);

    if ( p->delay != 0.0 ) {
        mean = (long double) p->delay;
        fputs("\n",stderr);
        goto start_gen;
    }

    /*** collect on-loop statistics ***/
    if ( p->stat ) {
        xprintf(stderr,"\t     Starting calibration (over %d packets). Press enter to start\n",p->stat);
        getchar();
    }

    for (i=0, ts = get_cycles()+inter_time;  i < p->stat; ) {

        gettimeofday(&opt_ts,NULL);

        rtcp_sr->r.sr.ntp_sec = htonl(opt_ts.tv_sec);    /* gettimeofday() sec */
        rtcp_sr->r.sr.ntp_frac= htonl(opt_ts.tv_usec);
        rtcp_sr->r.sr.rtp_ts  = htonl(opt_ts.tv_usec);    /* gettimeofday() usec */

        /* send rtcp packet */
        if( brute_sendto(global.sout_fd, arena, bytes, 0) == -1 )
            continue;

        /* update packet */
        brute_update_frame(arena);

        /* set alarm: 1 sec. */
        alarm (1);
        for (; (slen=recvfrom(  global.sin_fd, buffer, p->len, 0,
                                (struct sockaddr *) &addr_ll, &so_len))!= -1 && RECV_RTCP()==0  ; );
        alarm(0);

        /* rtcp lost in space ? */
        if ( slen == -1 || RECV_RTCP()==0 ) {
            continue;
        }

        /* retrive the current timestamp */
        ioctl(global.sin_fd, SIOCGSTAMP, &recv_ts);

        sample_tv_sec = (int)recv_ts.tv_sec-(int)ntohl(rtcp_ss->r.sr.ntp_sec);
        sample_tv_usec= (int)recv_ts.tv_usec-(int)ntohl(rtcp_ss->r.sr.ntp_frac);

        if (sample_tv_sec < 10 ) {

            sample = (sample_tv_sec*1000000+ sample_tv_usec);

            sum  += sample;
            sum2 += sample*sample;

            xprintf(stderr, "\t     %d: calibrating on-loop -> delay= %d usec         \r",
                    i,(sample_tv_sec)*1000000+ sample_tv_usec );
            fflush(stderr);

            i++;
        }

        brute_wait_until(&ts);

        /* increment ts */
        ts += inter_time;
    }

    /*** setup statistics ***/

    if ( p->stat ) {
        mean   = sum/p->stat;
        stddev = sqrtl((sum2-mean*mean*p->stat)/(p->stat-1));

        xprintf(stderr,"\t     stats: mean_delay= %.3LF usec, std.deviation= %.3LF usec\n", mean, stddev);
        xprintf(stderr,"\t     Press enter to start the end-to-end delay measurement.\n");
        getchar();
    }

start_gen:

    /*** reset received packet counter ***/
    precv = 0;

    /*** reset exit_time ***/
    *exit_time = (cycles_t)( get_cycles()+ Hz/1000 * p->msec);
    stop_time = (cycles_t)( get_cycles()+ Hz/1000 * (p->msec+RECOVERY_TIME));

    /*** set nonblocking socket ***/
    set_nonblock(global.sin_fd);

    /*** send packets util the exit_time expires ***/
    for ( ts = get_cycles()+inter_time; get_cycles() < *exit_time; ) {

        /*** TX ****/
        gettimeofday(&opt_ts,NULL);

        rtcp_sr->r.sr.ntp_sec = htonl(opt_ts.tv_sec);    /* gettimeofday() sec */
        rtcp_sr->r.sr.ntp_frac= htonl(opt_ts.tv_usec);
        rtcp_sr->r.sr.rtp_ts  = htonl(opt_ts.tv_usec);    /* gettimeofday() usec */

        /* send rtcp packet */
        if( brute_sendto(global.sout_fd, arena, bytes, 0) == -1 )
            continue;

        /* increment the global counter */
        global._sent++;

        /* update packet */
        brute_update_frame(arena);

        /*** RX ****/

RX:	rtest= 0;
    do {
        slen= recvfrom(global.sin_fd, buffer, p->len, 0,(struct sockaddr *) &addr_ll, &so_len);
        rtest++;
    }
    while ((slen==-1 || RECV_RTCP()==0) && rtest < RETRIEVE_LIMIT );

    if ( rtest == RETRIEVE_LIMIT ) {
        goto end;
    }

    /* increment the packet received counter */
    precv++;

    /* retrive the current timestamp */
    ioctl(global.sin_fd, SIOCGSTAMP, &recv_ts);

    sample_tv_sec = (int)recv_ts.tv_sec-ntohl(rtcp_ss->r.sr.ntp_sec);
    sample_tv_usec= (int)recv_ts.tv_usec-ntohl(rtcp_ss->r.sr.ntp_frac);

    xprintf(stdout,"\t     %d.%06d -> %d.%06d: ip_id= %d delay= %.3LF usec\n",
            ntohl(rtcp_ss->r.sr.ntp_sec), ntohl(rtcp_ss->r.sr.ntp_frac),
            (int)recv_ts.tv_sec,(int)recv_ts.tv_usec,
            (int)ntohs(ips->id),
            (long double)sample_tv_sec*1000000+(long double)sample_tv_usec-mean);
end:
    if ( get_cycles() >= *exit_time )
        break;

    brute_wait_until(&ts), ts += inter_time;

    }

    /*** waiting for the most recent sent packet ***/
    if (get_cycles() < stop_time)
        goto RX;

    /*** reenable paging ***/
    munlockall();

    /*** set blocking socket ***/
    set_block(global.sin_fd);

    /*** print banner ***/
    {
        unsigned long long sent = global._sent-global._start;                    /* packet sent */
        unsigned long long req  = (unsigned long long)p->rate*p->msec/1000;     /* packet to be sent */
        unsigned long long arate= (unsigned long long)sent*1000/p->msec;        /* average rate */
        double jitter   = (double)sent/req*100-100;                             /* variation percentage */
        double lostp    = (double)(sent-precv)*100.0/sent;

        fprintf(stderr,	"\t     @%lld (fps)  sendto()=#%lld/%lld  jitter+=%.2f%%   lost=%d packet (%.2f%%)\n",
                arate,sent,req,jitter,(int)(sent-precv), lostp);
    }

    /*** the frame_t is realloced each time the handler start. Therefore it is not required to be freed ***/
}

