/*
    $Id: t-module.h,v 1.17 2008-01-12 16:10:19 awgn Exp $
 
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

#ifndef T_MODULE_H
#define T_MODULE_H

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <features.h>           /* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>       /* the L2 protocols */
#else
#include <linux/if_packet.h>
#include <linux/if_ether.h>     /* The L2 protocols */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

/* global definition */
#include <global.h>
/* new typedef and structures */ 
#include <typedef.h>
/* common macros */
#include <shared-macro.h>
/* prototypes */
#include <prototype.h>

/*
 * CVS identifier
 */
#define MODULE_CVS(x) \
	static const char cvsid[] __attribute__((unused)) = x

#define MODULE_AUTHOR(x) \
	static const char author[] __attribute__((unused)) = x
	
/*
 * prototype the user-defined handlers.
 */
#define FUNCTION_PARSER(x) \
	static void (x)(int, struct atom *, cmdline_t *)

#define FUNCTION_ENGINE(x) \
	static void (x)(cycles_t *, cmdline_t *)


/*
 * print module banner on standard error
 */
#define BANNER(format,...)  \
        xprintf(stdout," " format "\n",  ## __VA_ARGS__)

/*
 * declare a token inside a sorted token list, useful when initializing the module_descriptor
 * structure.
 */
#define TOKEN(x)        [TOKEN_##x]=#x

/*
 * standard parser error
 */

#define PARSER_ERROR(v) parser_error(PETOK, v->lvalue)

/*
 * common u_parser macros
 */

#define TAG(x) \
	__current_tag = (void *)&__par_line->x; \
	opaque_sizeof(__current_tag) = sizeof(__par_line->x); \
	__par_line->x 	

#define cast_ret(fun, ...) \
	fun(  __VA_ARGS__ ); \
        opaque_opcode(__current_tag) = __current_atom->opcode; \
	do { \
        	if ( (void *)fun != (void *)brute_eval_atom ) { \
                opaque_opcode(__current_tag) &= ~OC_CLASS_MASK; \
                opaque_opcode(__current_tag) |=  OC_PTIME; \
            }\
	} while (0)	
	
/*
 * Initialize the module, registering handlers and lists
 */ 
#define INIT_MODULE(x)\
static void init_module() __attribute__((constructor));	\
static void \
init_module() \
{ \
        register_module(&(x)); \
}							


/*
 * Initialize the u-parser handler, allocing the space for the 
 * __par_line opaque structure.
 */
#define INIT_PARSER(cmd,type,a) \
	static cmdline_t *_cmd; \
	static type *__par_line; \
	struct atom *__current_atom=(a); \
	long *__current_tag; \
	if ( _cmd != cmd ? _cmd = cmd, 1 : 0 ) { \
        	__par_line = calloc(1,sizeof(type)); \
       	 	cmd->opaque = (void *)__par_line; \
        	cmd->msec   = &(__par_line->msec); \
	} 							
	      							

/*
 * Initialize the engine handler
 */
#define INIT_ENGINE(cmd,type,p)	\
	static type __static_area; \
	update_mod_line(cmd->opaque,&__static_area,sizeof(type)); \
	p=&__static_area;


#include <inline.c>
#include <distribution.c>

#endif /* T_MODULE_H */
