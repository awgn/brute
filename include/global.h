/*
    $Id: global.h,v 1.24 2008-01-12 16:10:19 awgn Exp $
 
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

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef  GLB_OWNER
#define EXTERN
#define INIT(x...) x
#else
#define EXTERN extern
#define INIT(x...)
#endif

#include <sys/socket.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <sched.h>
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>       /* the L2 protocols */
#else
#include <linux/if_packet.h>
#include <linux/if_ether.h>     /* The L2 protocols */
#endif

#include <stdio.h>
#include <typedef.h>
#include <macro.h>


EXTERN cmdline_t cmdline[MAX_CMDLINE];

EXTERN char *src_mac;
EXTERN char *dst_mac;
EXTERN char *dst_ip;

EXTERN char *ifout;
EXTERN char *ifin;

EXTERN struct sockaddr_ll  sock_ll_out;
EXTERN struct sockaddr_ll  sock_ll_in;
EXTERN struct sockaddr_in  sock_out;
EXTERN struct sockaddr_in6 sock6_out;

/* options */
EXTERN struct options opt INIT(={
    rand_mac_dst:  0, 
    rand_mac_src:  0,

    rand_host_src: -1,          /* netmask: n. of bit */
    rand_host_dst: -1,          /* netmask: n. of bit */

    mask_host_src:  0,          /* network byte order mask */
    mask_host_dst:  0,          /* network byte order mask */

    mask_host6_src: 0,          /* network byte order mask: boundary byte */
    mask_host6_dst: 0,          /* network byte order mask: boundary byte */

    mac_src:        0,          /* mac_src given, rand or missing */
    mac_dst:        0,          /* mac_dst given, rand or missing */

    eui64:          0,          /* eui64 ipv6 stateless autoconf. */
    ether_off:      ETH_HLEN,   /* according to the socket type it can be either 0 or ETH_HLEN */
    verbose:        0,          /* for future use */
    priority:       0,          /* RR priority */
    chksum:         -1,         /* software or hardware */
    urand:          0,          /* set seed by /dev/urand */
    pf_inet:        0,          /* use pf_inet socket, instead of pf_packet */
    use_clock:      0,          /* use the given clock at command line */
    non_int:        0,          /* non interruptible (no kevent realtime evelation ) */
});

EXTERN struct data global INIT(={
	sout_fd:	0,
	sin_fd:		0,
	urand_fd:	0,
	ip:		    0,
	jmp:	    -1,
	max_line:	0,
	_sent:		0,
    _start:     0,
});

EXTERN char parse_buff[PARSE_BUFFLEN];
EXTERN char cpubuff[CPUBUFF];

EXTERN char *file_name;
EXTERN int   file_line;

EXTERN unsigned long long Hz;

EXTERN void (*update_arena)();
EXTERN void (*touch_arena)();

EXTERN void (*update_link) (frame_t *);
EXTERN void (*update_host) (frame_t *);

EXTERN u_short (*in_chksum) (const u_short *, uint32_t );

EXTERN int spec_rel[MAX_COMMAND][MAX_TOKEN]; 

EXTERN TAILQ_HEAD(tqh,entry) head_commands, head_tokens;

EXTERN struct module_descriptor *modules[MAX_COMMAND];

/* perfect hash tables */
EXTERN perf_hash_t	command_table;
EXTERN perf_hash_t	token_table;

EXTERN rtld_obj_t fun_set[MAX_OBJ + 1];
EXTERN rtld_obj_t var_set[MAX_OBJ + 1];

EXTERN int fun_index;
EXTERN int var_index;

EXTERN unsigned long init_rand_seed INIT(=0xcafebabe); 
EXTERN int (*xprintf)(FILE *,const char *, ...);
EXTERN ssize_t (*brute_sendto)(int , frame_t * , size_t , int );

EXTERN unsigned int *crono_vector;
EXTERN int crono_index;
EXTERN int crono_vector_size;
EXTERN int crono_vector_mask;
EXTERN int crono_vector_depth INIT(=1);

EXTERN int estimate_clock_time;

/* inet family support: ipv4/ipv6 */
EXTERN int  af_family INIT(=AF_INET);
EXTERN char hp_sep INIT(=':');
EXTERN int  eth_p_ip INIT(=ETH_P_IP);

#undef EXTERN
#undef INIT 
#endif /* GLOBAL_H */
