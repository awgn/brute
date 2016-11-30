/*
    $Id: module-pab.c,v 1.14 2008-01-12 16:10:22 awgn Exp $

    Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>


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

#include <t-module.h>

MODULE_CVS("$Id: module-pab.c,v 1.14 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Nicola Bonelli <bonelli@netserv.iet.unipi.it>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* opaque space */
struct mod_line {
    uint32_t			AL(msec);
    uint32_t           	AL(rate);
    double			AL(lambda);
    double			AL(alpha);
    double			AL(theta);
    /* ipv4 */
    uint32_t			AL(len);
    uint32_t			AL(tos);
    uint32_t			AL(ttl);
    /* ipv6 */
    uint32_t                   AL(class);
    uint32_t                   AL(flow);
    uint32_t                   AL(hoplim);

    struct hostent *        AL(saddr);
    struct hostent *        AL(daddr);
    /* udp */
    u_short			AL(sport);
    u_short 		AL(dport);
};


#define TOKEN_msec	0
#define TOKEN_rate	1
#define TOKEN_lambda	2
#define TOKEN_alpha   	3
#define TOKEN_theta	4
#define TOKEN_len	5
#define TOKEN_tos	6
#define TOKEN_ttl	7
#define TOKEN_class    	8
#define TOKEN_flow      9
#define TOKEN_hoplim    10
#define TOKEN_saddr	11
#define TOKEN_sport  	12
#define TOKEN_daddr	13
#define TOKEN_dport  	14


/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_engine:       u_engine,
                h_parser:       u_parser,
                command:        "pab",
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     15,
                token_list:     { TOKEN(msec), TOKEN(rate),TOKEN(lambda),TOKEN(alpha),TOKEN(theta),
                    TOKEN(len),TOKEN(tos),TOKEN(ttl),TOKEN(class), TOKEN(flow), TOKEN(hoplim),
                    TOKEN(saddr),TOKEN(sport),TOKEN(daddr),TOKEN(dport)},
};


INIT_MODULE(module);


/*** parser handler ***/

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
    case TOKEN_lambda:
        TAG(lambda)= cast_ret(brute_eval_double,v);
        break;
    case TOKEN_alpha:
        TAG(alpha)= cast_ret(brute_eval_double,v);
        break;
    case TOKEN_theta:
        TAG(theta)= cast_ret(brute_eval_double,v);
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


/*** globals ***/
struct entry {
    cycles_t	timestamp;
    TAILQ_ENTRY(entry) entries;     	/* Tail queue. */
};

static cycles_t now;				/* current processor cycles */
static cycles_t next_event_time;
static struct entry next_birth;			/* birth */
static TAILQ_HEAD(tailhead, entry) death_head;	/* death queue */
static int N;                                   /* state: number of bursts in service */


/*
 * insert element in ordered queue
 */
static void
insert_tail(struct tailhead *head, struct entry *new)
{
    struct entry *p, *q;
    p = NULL;

    for (q= head->tqh_first; q != NULL && q->timestamp < new->timestamp ; q = q->entries.tqe_next)
        p=q;

    if (p != NULL) {
        TAILQ_INSERT_AFTER(head,p,new,entries);
    }
    else {
        TAILQ_INSERT_HEAD(head,new,entries);
    }

#if 0
    /* sort check */
    {
        cycles_t c=0;
        for (p = head->tqh_first; p != NULL; p = p->entries.tqe_next) {
            xprintf(stdout,"%lld->",p->timestamp);
            if (p->timestamp > c)
                c=p->timestamp;
            else
                fatal(__INTERNAL__);
        }
        xprintf(stdout,"\n");
    }
#endif

}


/*
 * free the queue
 */
static void
free_tail(struct tailhead *head)
{
    while (head->tqh_first != NULL)
        TAILQ_REMOVE(head, head->tqh_first, entries);

}


/*** birth handler ***/
static void
birth_event(double lambda, double alpha, double theta)
{
    struct entry *new_death;

    /* increment the state */
    N++;

    /* schedule the next birth */
    next_birth.timestamp = now + (cycles_t)(Hz*exponential(lambda));

    /* schedule the death of the next burst */
    new_death = malloc(sizeof(struct entry));
    new_death->timestamp = next_birth.timestamp + (cycles_t)(Hz*pareto(alpha,theta));
    //new_death->timestamp = next_birth.timestamp + (cycles_t)(Hz*3);

    /* queue new_death to the sorted death list */
    insert_tail(&death_head,new_death);
}


/*** death handler ***/
static void
death_event(double lambda, double alpha, double theta)
{
    /* decrement the state */
    if ( N == 0 )
        fatal(__INTERNAL__);

    TAILQ_REMOVE(&death_head, death_head.tqh_first, entries);
    N--;
}


/*** generic handler ***/
static void (*event_handler)(double, double , double );


/*** engine handler ***/

static void
u_engine(cycles_t *exit_time,cmdline_t *cmd)
{
    cycles_t inter_time, ts;
    static frame_t *arena;
    struct mod_line *p;
    int bytes;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line,p);

    TAILQ_INIT(&death_head);                /* Initialize the queue. */
    event_handler = birth_event;		/* Initialize the handler */
    N= -1;					/* Initialize the state */


    /* print banner */
    BANNER("%s%c%d -> %s%c%d rate=@%d \n\t     {lambda=%f} {alpha=%f,theta=%f}", \
           brute_inet_ntop(p->saddr), hp_sep, p->sport,\
           brute_inet_ntop(p->daddr), hp_sep, p->dport,p->rate,p->lambda,p->alpha,p->theta);

    /* alloc frame and set it up */
    arena = (frame_t *)brute_realloc_frame(arena);

    brute_build_mac(arena,&global.ethh); 				// global ethernet option

    brute_build_ip  (arena,                                         // frame pointer
                     p->len,                                       // frame length

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

    /*** compile the udp_data according to rfc2544 frame format ***/
    brute_udpdata_rfc2544(arena);

    /*** ensure data are ok  ***/
    ASSERT(p->len  > 0);
    ASSERT(p->lambda > 0);

    /* PAB condition */
    ASSERT(p->alpha > 1 && p->alpha <2); 		/* pareto */
    ASSERT(p->theta > 0);				/* pareto */

    /*** calc the correct number of bytes to pass to sendto() ***/
    bytes = brute_framelen_to_bytes(p->len);

    /*** start ***/
    global._start = global._sent;


    /*** main cycle ***/

    for ( ; get_cycles() < *exit_time; ) {

        /*** exec the event handler ***/
        now = get_cycles();
        event_handler(p->lambda,p->alpha,p->theta);

        /*** compute the next_event_time ***/
        next_event_time = MIN(next_birth.timestamp, death_head.tqh_first->timestamp );
        next_event_time = MIN(next_event_time, *exit_time);

        /*** update the next handler ***/
        event_handler = ( next_birth.timestamp < death_head.tqh_first->timestamp ?
                          birth_event :
                          death_event );

        /*** according to rate and N-state, determine the interdeparture-time of packets.
          Both inter_time and HZ are expressed in time step counters (cycles_t) ***/

        /*** N == 0 ? sleeping gate ***/

        xprintf(stdout,"\t     N(%lld)=%d rate(t)=%d\n",now,N,p->rate*N);

        if ( N == 0 ) {
            brute_wait_until(&next_event_time);
            continue;
        }

        /*** N > 0 ***/
        inter_time = (cycles_t)(Hz/(p->rate*N));

        /*** send packets util the exit_time expires ***/
        for ( ts = get_cycles()+inter_time; get_cycles() < next_event_time ; ) {

            if( brute_sendto(global.sout_fd, arena, bytes, 0)== -1 )
                continue;

            /* increment the global counter */
            global._sent++;

            /* update packet */
            brute_update_frame(arena);

            /* sleep until the step expires */
            brute_wait_until(&ts);

            /* increment ts */
            ts += inter_time;

        }
    }

    /*** print banner ***/
    {
        unsigned long long sent = global._sent-global._start;                  /* packet sent */
        unsigned long long arate= (unsigned long long)sent*1000/p->msec;     /* average rate */

        xprintf(stderr, "\t     @%lld (fps)  sendto()=#%lld\n", arate,sent);
    }

    /*** free the TAILQ ***/
    free_tail(&death_head);

    /*** the frame_t is realloced each time the handler start. Therefore it is not required to be freed ***/
}

