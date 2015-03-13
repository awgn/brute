/*
    $Id: module-cbr-exp.c,v 1.4 2008-01-12 16:10:21 awgn Exp $
 
    Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
                                       <bonelli@netserv.iet.unipi.it>
 
 
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

MODULE_CVS("$Id: module-cbr-exp.c,v 1.4 2008-01-12 16:10:21 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* opaque space */
struct mod_line {
    /* ipv4 */
    uint32_t			AL(msec); 
    double          	AL(lambda);

    uint32_t			AL(rate);
    uint32_t			AL(len);
    uint32_t			AL(tos);
    uint32_t			AL(ttl);  
    /* ipv6 */
    uint32_t			AL(class);
    uint32_t			AL(flow);
    uint32_t			AL(hoplim);

    struct hostent *	AL(saddr);
    struct hostent *	AL(daddr);
    /* udp */
    u_short			AL(sport); 
    u_short 		AL(dport); 
};


#define TOKEN_msec	0
#define TOKEN_lambda    1
#define TOKEN_rate	2	
#define TOKEN_len	3	
#define TOKEN_tos	4	
#define TOKEN_ttl	5	

#define TOKEN_class	6	
#define TOKEN_flow	7	
#define TOKEN_hoplim	8	

#define TOKEN_saddr	9	
#define TOKEN_sport  	10	
#define TOKEN_daddr	11	
#define TOKEN_dport  	12	


/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_engine:       u_engine,
                h_parser:       u_parser,
                command:        "cbr-exp",                 				
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     13,
                token_list:     { TOKEN(msec), TOKEN(lambda), TOKEN(rate),TOKEN(len),TOKEN(tos),TOKEN(ttl),
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
    case TOKEN_lambda:
        TAG(lambda) = cast_ret(brute_eval_double,v);
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


/*** engine handler ***/

static void
u_engine(cycles_t *exit_time,cmdline_t *cmd)
{
    cycles_t inter_time, ts;
    cycles_t my_exit_time;
    static frame_t *arena;
    struct mod_line *p;
    int bytes;
    double off;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line,p);

    off = exponential(1/(p->lambda));

    /* print banner */
    BANNER("%s%c%d -> %s%c%d @%d fps - {lambda=%f sec, duration=%f sec}", 	brute_inet_ntop(p->saddr), hp_sep, p->sport,\
           brute_inet_ntop(p->daddr), hp_sep, p->dport,p->rate, p->lambda, off);

    /* alloc frame and set it up */
    arena = (frame_t *)brute_realloc_frame(arena);


    brute_build_mac(arena,&global.ethh); 				// global ethernet option 

    brute_build_ip  (arena,						// frame pointer
                     p->len,					// frame length

                     (u_short)global._sent,			// ip_id 		*v4
                     p->ttl,					// ttl   		*v4
                     p->tos,					// tos   		*v4

                     p->class,					// traffic class 	*v6
                     p->flow,					// flow label 		*v6
                     p->hoplim,					// hop limit 		*v6

                     p->saddr,					// source ip
                     p->daddr);					// destination ip

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

    my_exit_time = get_cycles()+ (cycles_t)(Hz*off);

    /*** send packets util the exit_time expires ***/
    for ( ts = get_cycles()+inter_time; get_cycles() < my_exit_time; ) {

        if( brute_sendto(global.sout_fd, arena, bytes, 0) == -1 )
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

    /*** reenable paging ***/
    munlockall();

    /*** print banner ***/
    {
        unsigned long long sent = global._sent-global._start;			/* packet sent */
        unsigned long long req  = (unsigned long long)p->rate*off;		/* packet to be sent */	
        unsigned long long arate= (unsigned long long)sent*off;			/* average rate */
        double jitter	= (double)sent/req*100-100;				/* variation percentage */

        xprintf(stderr,	"\t     @%lld (fps)  sendto()=#%lld/%lld  jitter+=%.2f%%\n",
                arate,sent,req,jitter);
    }

    /*** the frame_t is realloced each time the handler start. Therefore it is not required to be freed ***/
}

