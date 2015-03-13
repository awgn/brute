/*
    $Id: module-multicbr.c,v 1.1 2008-05-19 19:47:09 awgn Exp $
 
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

MODULE_CVS("$Id: module-multicbr.c,v 1.1 2008-05-19 19:47:09 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* opaque space */
struct mod_line {
	/* ipv4 */
	u_int			AL(msec); 
	u_int			AL(rate);
	u_int			AL(len);
	u_int			AL(tos);
	u_int			AL(ttl);  
	/* ipv6 */
	u_int			AL(class);
  	u_int			AL(flow);
	u_int			AL(hoplim);

	struct hostent *	AL(saddr);
	struct hostent *	AL(daddr);
	/* udp */
	u_short			AL(sport); 
	u_short 		AL(dport); 
	/* multi udp */
  	u_int			AL(udp_flow);
};


#define TOKEN_msec	0
#define TOKEN_rate	1
#define TOKEN_len	2
#define TOKEN_tos	3
#define TOKEN_ttl	4

#define TOKEN_class	5
#define TOKEN_flow	6
#define TOKEN_hoplim	7

#define TOKEN_saddr	8	
#define TOKEN_sport  	9	
#define TOKEN_daddr	10	
#define TOKEN_dport  	11	
#define TOKEN_udp_flow  12	


/*
 * module descriptor
 */
static
struct module_descriptor module = {
        h_engine:       u_engine,
        h_parser:       u_parser,
        command:        "multicbr",                 				
	    author:         "Nicola Bonelli <nicola.bonelli@netresults.it>",
	    token_nelm:     13,
        token_list:     { TOKEN(msec), TOKEN(rate),TOKEN(len),TOKEN(tos),TOKEN(ttl),
			  TOKEN(class), TOKEN(flow), TOKEN(hoplim),TOKEN(saddr),TOKEN(sport),
			  TOKEN(daddr),TOKEN(dport),TOKEN(udp_flow)}, 
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
	case TOKEN_udp_flow:
		TAG(udp_flow)= cast_ret(brute_eval_int,v);
		break;
        default:
		PARSER_ERROR(v);
        }

}


/*** engine handler ***/

static void
u_engine(cycles_t *exit_time,cmdline_t *cmd)
{
	cycles_t inter_time, sub_inter_time, ts, tp;
	static frame_t *arena;
	struct mod_line *p;
	int bytes;
	int j;

	/* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
	INIT_ENGINE(cmd,struct mod_line,p);

	/* print banner */
	BANNER("%s%c%dto%d  -> %s%c%dto%d @%d fps", 	brute_inet_ntop(p->saddr), hp_sep, p->sport,\
						p->sport+p->udp_flow-1,brute_inet_ntop(p->daddr), hp_sep,\
						p->dport,p->dport+p->udp_flow-1,p->rate);

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
			  (p->sport),					// source port
			  (p->dport),					// destination port
			  p->len-sizeof(struct ethhdr)	\
				-sizeof(struct iphdr)	\
				-sizeof(long),				// eth crc 
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
	sub_inter_time = inter_time / p->udp_flow;

	/*** start ***/
	global._start = global._sent;

	/*** disable paging ***/
	mlockall(MCL_FUTURE);
	
	/*** send packets util the exit_time expires ***/
        for ( ts = get_cycles()+inter_time; get_cycles() < *exit_time; ) {

		/* cycle for multiple-flows */

		for(j=1, tp = get_cycles() + sub_inter_time; j <= p->udp_flow; j++) 
		{
			/* send packet */
			brute_update_frame(arena);
			if( brute_sendto(global.sout_fd, arena, bytes, 0) == -1 )
				continue;

			/* update the udp header */
			brute_build_udp( arena,
					p->sport + j,					// source port
					p->dport + j,					// destination port
					p->len	-sizeof(struct ethhdr)	\
					-sizeof(struct iphdr)	\
					-sizeof(long),				// eth crc 
					0);					// udp checksum not needed

			/* increment the global counter */
			global._sent++;
		
			/* sleep until the step expires */
                	brute_wait_until(&tp);

			tp += sub_inter_time; 
		}

		/* reset udp header for the first flow */
		brute_build_udp(arena,
			  	p->sport,					// source port
			  	p->dport,					// destination port
			  	p->len  -sizeof(struct ethhdr)	\
					-sizeof(struct iphdr)	\
					-sizeof(long),				// eth crc 
			 		0);					// udp checksum not needed


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
		unsigned long long req  = (unsigned long long)p->udp_flow*p->rate*p->msec/1000;	/* packet to be sent */	
		unsigned long long arate= (unsigned long long)sent*1000/p->msec;	/* average rate */
		double jitter	= (double)sent/req*100-100;				/* variation percentage */

		xprintf(stderr,	"\t     @%lld (fps)  sendto()=#%lld/%lld  jitter+=%.2f%%\n",
				arate,sent,req,jitter);
	}

	/*** the frame_t is realloced each time the handler start. Therefore it is not required to be freed ***/
}

