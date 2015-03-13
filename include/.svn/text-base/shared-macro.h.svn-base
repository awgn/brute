/*
    $Id: shared-macro.h,v 1.14 2008-01-12 16:10:19 awgn Exp $
 
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

#ifndef SHARED_MACRO_H
#define SHARED_MACRO_H

/* 
 * fatal internal error 
 */
#define __INTERNAL__    "%s:%d %s() :internal",__FILE__,__LINE__,__FUNCTION__

/*
 * msg() macro 
 */
#define msg(format,...) fprintf(stderr, format, ## __VA_ARGS__)
#define MSG_NULL        "    "
#define MSG_DIR         "[*] "
#define MSG_INFO        "[i] "
#define MSG_READ        "[r] "
#define MSG_WRITE       "[w] "
#define MSG_FATAL       "[!] "
#define MSG_EMPTY       "[ ] "

/*
 * DEBUG() macro
 */
#ifdef EBUG
#define DEBUG(format, ...) do { \
    fprintf (stderr, "    %s:", __PRETTY_FUNCTION__); \
    fprintf (stderr, format, ## __VA_ARGS__); \
} while (0)
#else
#define DEBUG(f,arg...) do {} while (0)
#endif

/*
 * like assert() exits if the argument passed is false, unlike assert() don't care 
 * about NDEBUG macro 
 */
#define ASSERT(x)       \
x ? : fatal("%s:%d :%s(): Assertion `%s' failed.",__FILE__,__LINE__,__FUNCTION__,#x); 


/* 
 * PUB_FUNCTION  macro 
 */
#define PUB_FUNCTION(s,args) \
static void __reg_function_##s () __attribute__((constructor)); \
static void \
__reg_function_##s () \
{ \
    register_obj(#s,s,args,sym_function); \
}


/* 
 * PUB_VAR macro 
 */
#if __GNUC__ >= 3 && __GNUC_MINOR__ >= 2                                
#define PUB_VAR(s) \
static void __reg_var_##s () __attribute__((constructor)); \
static void \
__reg_var_##s () \
{ \
    if ( ! __builtin_types_compatible_p (typeof (s), int) ) \
    fatal("typeof(%s) not integer!",#s); \
    register_obj(#s,&s,0,sym_variable); \
}
#else
#define PUB_VAR(s) \
static void __reg_var_##s () __attribute__((constructor)); \
static void \
__reg_var_##s () \
{ \
    if ( sizeof(s) != sizeof(int) ) \
    fatal("sizeof(%s) mismatch! variable must be integer!",#s); \
    register_obj(#s,&s,0,sym_variable); \
}
#endif

/* 
 * useful macro to extract arguments from a vargs_t object 
 */
#define args(vt,i) ({ \
    if (i>= vt->nmem) \
    fatal("too few args in vargs_t@%p",vt); \
    vt->args[i]; \
})


/* According to the opaque struct mod_line, each tag is aligned to 32 byte word and 
 * followed by some parameters (stored in the adjacent words). Given the tag, 
 * the following macros retrieve the corrisponding parameters. 
 */

struct opaque_mod_line {
    unsigned char      data[16];
    unsigned long      size;
    unsigned long      opcode;
} __attribute__((packed));

#define opaque_stcarg(type, x)  *((type *)(((struct opaque_mod_line *)x)->data))
#define opaque_sizeof(x)        (((struct opaque_mod_line *)x)->size)
#define opaque_opcode(x)        (((struct opaque_mod_line *)x)->opcode)

/* 
 * opcode macros 
 */
#define OC_CLASS_MASK   0x000000ff
#define OC_OPER_MASK    0x0000ff00
#define OC_TYPE_MASK	0x00ff0000

#define OC_CLASS(x)     1<<(0 +x)
#define OC_OPER(x)      1<<(8 +x)
#define OC_TYPE(x)	    1<<(16+x)

/* class (8 bits) */
#define OC_PTIME        (OC_CLASS(0))/* object to be evaluated at parser-time */
#define OC_RTIME        (OC_CLASS(1))/* object to be avaluated at run-time */

/* oper (8 bits) */
#define OC_EQ           (OC_OPER(0)) /*  = */
#define OC_PLEQ         (OC_OPER(1)) /* += */
#define OC_MNEQ         (OC_OPER(2)) /* -= */

/* tyoe (8 bits) */
#define OC_INT		    (OC_TYPE(0))
#define OC_DOUBLE	    (OC_TYPE(1))
#define OC_HOST		    (OC_TYPE(2))
#define OC_ADDR		    (OC_TYPE(3))

/*
 * tags of the user-defined struct mod_line are DEFAULT_ALIGN bytes aligned 
 */
#define CEIL2(x) ( !((x-1)>>1) ? (1<<1)  :    \
                 ( !((x-1)>>2) ? (1<<2)  :    \
                 ( !((x-1)>>3) ? (1<<3)  :    \
                 ( !((x-1)>>4) ? (1<<4)  :    \
                 ( !((x-1)>>5) ? (1<<5)  :    \
                 ( !((x-1)>>6) ? (1<<6)  :    \
                 ( !((x-1)>>7) ? (1<<7)  : (1<<8) ))))))) 

#define DEFAULT_ALIGN          CEIL2(sizeof(struct opaque_mod_line))      /* bytes */ 
#define AL(x)  x __attribute__ ((aligned (DEFAULT_ALIGN)))


/*
 * *** use with care ***
 *
 * crono_start() crono_end() macros. Dump on stderr the elapsed
 * time-stamp-counters between the two macro. 
 *
 */
#define crono_start()	{ \
    unsigned long long c_start; \
    c_start=get_cycles() 

#define crono_stop() \
    crono_vector[crono_index & crono_vector_mask]=get_cycles()-c_start; \
    crono_index++; \
}

/*
 * other macros
 */

#define MIN(a,b)            ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)            ( (a) > (b) ? (a) : (b) )
#define U(x)                ( (x) > 0 ? x  : 0 )
#define ABS(x)              ( (x) < 0 ? -(x) : (x) )

#endif /* SHARED_MACRO_H */
