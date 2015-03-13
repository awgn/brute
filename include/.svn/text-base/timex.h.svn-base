/*
    $Id: timex.h,v 1.3 2008-01-12 16:10:19 awgn Exp $

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


/*
    most part of this header has been ripped from the timex.h of the 
    linux kernel 2.4
 */


#ifndef TIMEX_H
#define TIMEX_H

/*** 
 *** i586/686 architecture 
 ***/
#if defined (CONFIG_586) || defined (CONFIG_686)
#define rdtscll(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

typedef unsigned long long cycles_t;
static inline cycles_t get_cycles (void)
{
        unsigned long long ret;
        rdtscll(ret);
        return ret;
}

#elif defined (CONFIG_X86_64)
#define rdtscll(val) do { \
    unsigned int a,d; \
    asm volatile("rdtsc" : "=a" (a), "=d" (d)); \
    (val) = ((unsigned long)a) | (((unsigned long)d)<<32); \
} while(0)

typedef unsigned long long cycles_t;
static inline cycles_t get_cycles (void)
{
        unsigned long long ret;
        rdtscll(ret);
        return ret;
}

/*** 
 *** hppa2.0-unknown-linux-gnu 
 ***/ 
#elif defined (CONFIG_PA)
#define mfctl(reg)      ({              \
        unsigned long cr;               \
        __asm__ __volatile__(           \
                "mfctl " #reg ",%0" :   \
                 "=r" (cr)              \
        );                              \
        cr;                             \
})

typedef unsigned long cycles_t;
static inline cycles_t get_cycles (void)
{
        return mfctl(16);
}

#endif

#endif /* TIMEX_H */


