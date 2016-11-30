/*
    $Id: macro.h,v 1.18 2008-01-12 16:10:19 awgn Exp $

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


#ifndef MACRO_H
#define MACRO_H

#include <shared-macro.h>


/*
 * given a string, it removes the last expected close bracket,
 * exits with a fatal error otherwise.
 */
#define rm_close_bracket(ptr)   do { \
char *_p; \
        _p = strrchr((ptr), ')'); \
        if (_p == NULL || *(_p+1)!='\0') \
                fatal("%d:parse error: %s",__LINE__, (ptr)); \
        *_p='\0'; \
} while(0)


/*
 * finite state machine helper
 */
#define state_$(cond,jmp_a,action,jmp_r, reaction) \
if (cond) { \
	state=jmp_a; \
	action; \
} else { \
	state=jmp_r; \
	reaction; \
}

/*
 * is a good token?
 */
#define isgtoken(x)	(isalnum(x)||x=='_')

/*
 * common attributes
 */
#ifndef _unused
#define _unused        __attribute__((unused))
#endif
#ifndef _dead
#define _dead          __attribute__((noreturn))
#endif

/* prefix4_mask */
#define PREFIX4_MASK(p) ( (p) == 0 ? 0 : ~0 << (32 - (p)) )


/* buffer limits */
#define MAX_CMDLINE     	1024
#define MAX_OBJ         	1024
#define PARSE_BUFFLEN   	1024
#define CPUBUFF         	1024

/* checksum */
#define CHKSUM_HW	0
#define CHKSUM_SW	1
#define CHKSUM_FS	2

/* parser error */
#define PERROR		1
#define PECOMM		2
#define PETOK		3
#define PEITOK		4
#define	PETOOFEW	5

/* commands and tokens limit */
#define MAX_COMMAND     256
#define MAX_TOKEN       256

#endif /* MACRO_H */
