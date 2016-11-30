/*
    $Id: typedef.h,v 1.28 2008-01-12 16:10:19 awgn Exp $

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


#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stdint.h>
#include <timex.h>

/* atom returned type */
union paret {
    int              _int;
    long int         _long_int;
    double           _double;
    struct hostent * _host;
    void *	     _addr;
};


/* atom object */
struct atom {
	char *lvalue;		/* l-value token */
	char *rvalue;		/* r-value token */
	int opcode;		/* operator to be applied when evaluating rvalue */
	int value;		/* evaluated r-value to be considered
				 * "static" at parser-time */
};

/* user options */
struct options {
	int rand_mac_dst;	/* use random mac: RFC2544 */
	int rand_mac_src;	/* use random mac: RFC2544 */

	int rand_host_src;	/* netmask: n. of bit */
	int rand_host_dst;	/* netmask: n. of bit */

	int mask_host_src;	/* network byte order mask */
	int mask_host_dst;	/* network byte order mask */

	char mask_host6_src;	/* network byte order mask: boundary byte */
	char mask_host6_dst;	/* network byte order mask: boundary byte */

	int mac_src;		/* mac_src given, rand or missing */
	int mac_dst;		/* mac_dst given, rand or missing */

	int eui64;		    /* eui64 ipv6 stateless autoconf. */
	int ether_off;		/* according to the socket type it can be either 0 or ETH_HLEN */
	int verbose;		/* for future use */
	int priority;		/* RR priority */
	int chksum;		    /* software or hardware */
	int urand;		    /* set seed by /dev/urand */
	int pf_inet;		/* use pf_inet socket, instead of pf_packet */
	int use_clock;		/* use the given clock at command line */
	int non_int;		/* non interruptible (no kevent realtime evelation ) */
};


/* global data */
struct data {
	int ip;			            /* instruction pointer */
	int jmp;		            /* to label */
	int max_line;		        /* max number of command line in brute-conf */
	int sout_fd;		        /* pf_packet out socket */
	int sin_fd;		            /* pf_packet in socket */
	int urand_fd;		        /* /dev/urand fd */
	unsigned long long _start;	/* ip_id start value */
	unsigned long long _sent;	/* number of frame sent */
	struct ethhdr ethh;	        /* basic ethernet header */
};


/* typedef rtld object */
typedef struct {
	char *sym;		/* symbol identifier */
	void *addr;		/* address where symbol lies */
	int type;		/* enum symbol_type: variable or function */
} rtld_obj_t;


/* typedef cmdline */
typedef struct {
	uint32_t lnumb;		/* line number */
	uint32_t comm;		/* command */
	uint32_t label;		/* the label */
	uint32_t *msec;		/* gate duration in msec, stored in the mod_space */
	char *s_label;		/* label string */
	void *opaque;		/* module's space */
}      cmdline_t;


/* typedef frame */
typedef struct {
	struct ethhdr  *ethh;	/* ethernet header */
	struct iphdr   *iph;	/* ip header */
	struct ip6_hdr *ip6h;	/* ip6 header */
	struct udphdr  *udph;	/* upd header */
	uint8_t *udata;		    /* upd data */
	uint8_t *data;		    /* raw data: pointer to the frame */
	uint32_t hash;		    /* used to check the frame integrity */
	uint32_t cooked;	    /* for packets with the link level header removed */
}      frame_t;



/* network byte ordered type */
typedef unsigned long u_nbo32_t;


/*
 * perfect hash
 */
#include <sys/types.h>
typedef
struct {
	char *key;		/* key string */
	int id;			/* hash */
}      card_t;

typedef
struct {
	size_t size;		/* sizeof() the perfect hash table */
	card_t *tab;		/* card_t table */
}      perf_hash_t;


/* struct module_descriptor */
struct module_descriptor {
	/* function handlers */
	void (*h_parser) (int, struct atom *, cmdline_t *);
	void (*h_engine) (cycles_t*, cmdline_t *);

	char *command;		/* string of command */
	char *author;		/* author mail */
	int token_nelm;		/* number of tokens */

/*
 * ISO C99 intruduces "flexible array member" which can be
 * statically initialized. GCC 3 handles these members well.
 * GCC prior to 3 (2.95/96) lacks the flexible array but
 * allows zero-lenght arrays to be statically initialized,
 * as if they were flexible arrays.
 */

#if (__GNUC__ >= 3)
	char *token_list[];	/* flexyble array members: token for the
				 * current command */
#else
	char *token_list[0];	/* zero-lenght array */
#endif
};


/* variable argument list for user-defined published function */
typedef struct {
	int nmem;		/* number of args */
#if __GNUC__ >= 3
	int args[];		/* flexyble array members */
#else
	int args[0];		/* zero-lenght array */
#endif
}      vargs_t;


typedef void sigfunc(int);      /* for signal handlers */

/*
 * enumerate types
 */

enum symbol_type {
	sym_variable,		/* the symbol is a global variable */
	sym_function		/* the symbol is a function */
};

/* r-value parser states */
enum par_rvalue_state {
	rval_lspace,		/* blanks preceding the r-value */
	rval_symbol,		/* r-value symbol */
	rval_rspace,		/* blanks following the r-value */
	rval_bracket		/* open bracket state '(' */
};

/* atom parser states */
enum parse_atom_state {
	eval_llspace,		/* blanks preceding the l-value */
	eval_lvalue,		/* l-value token */
	eval_lespace,		/* blanks following the l-value */
	eval_plus_sep,		/* += separator */
	eval_min_sep,		/* -= separator */
	eval_sep,		    /* = separator */
	eval_rlspace,		/* blanks preceding the r-value */
	eval_rvalue,		/* r-value token */
	eval_respace,		/* blanks following the r-value */
	eval_done,		    /* ok */
};

/* atom parser state */
enum par_atom_state {
    atom_lspace,		/* left spaces */
    atom_body,		    /* atom */
    atom_done,		    /* separator, done */
};

/* label parser state */
enum par_label_state {
    lab_lspace,         /* left spaces */
    lab_symbol,         /* label */
    lab_colom,          /* : */
    lab_rspace,         /* right spaces */
    lab_done,           /* done */
};

/* command parser state */
enum par_command_state {
    comm_lspace,		/* left spaces */
    comm_symbol,		/* command */
    comm_rspace,		/* right spaces */
    comm_done		    /* done */
};

/* atom parser methods */
enum eval_mode  {
    eval_rvalue_error  =-1,	/* returned when the evaluation fails */
    eval_rvalue_eok    , 	/* evaluation successful */
    eval_rvalue_static , 	/* static evaluation: only integers */
    eval_rvalue_dynamic, 	/* dynamic evalutation: integers, vars and functions */
    eval_rvalue_int    , 	/* only integers */
    eval_rvalue_double , 	/* only doubles */
    eval_rvalue_host   ,    /* evaluate as host */
    eval_rvalue_addr   ,    /* evaluate as string */
    eval_rvalue_var    , 	/* TODO: only vars */
    eval_rvalue_func   , 	/* TODO: only functions */
};

#endif				/* TYPEDEF_H */
