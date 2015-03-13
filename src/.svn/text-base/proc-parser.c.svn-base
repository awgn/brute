/*
    $Id: proc-parser.c,v 1.6 2008-01-12 16:10:23 awgn Exp $
 
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <err.h>

#include <macro.h>
#include <global.h>
#include <prototype.h>
#include <config.h>

_unused static
const char cvsid[]= "$Id: proc-parser.c,v 1.6 2008-01-12 16:10:23 awgn Exp $";


static
const char cpu_MHz[]="cpu MHz";

#define check_clock_consistency(a,b)    \
        (unsigned long long)(a)/1000000 == (unsigned long long)(b)/1000000 

unsigned long long
get_cpu_hz(int n)
{
    unsigned long long c2, c1, usec, Hz;
    long double MHz, eHz;
    struct timeval g2, g1;
    char *p = cpubuff;

    int fd;

    if ( (fd=open(PROC_CPUINFO,O_RDONLY)) == -1)
        fatal("%s open error",PROC_CPUINFO);
    read(fd,cpubuff,CPUBUFF);
    close(fd);

    for ( p = (char *)strtok(cpubuff,":\t\n"); p!= NULL ;p = (char *)strtok(NULL,":\t\n")) {
        if ( strcmp(cpu_MHz,p) == 0 )
            break;
    }

    p = (char *)strtok(NULL,":\t\n");
    if( (sscanf(p,"%LF\n",&MHz)) != 1 )
        fatal("%s open error",PROC_CPUINFO);

    Hz=MHz*1000000;

    if (n == 0)
        return Hz;

    /* 
     * fine clock estimation 
     */

    gettimeofday(&g1,NULL);
    c1 = get_cycles();

    sleep(n);

    gettimeofday(&g2,NULL);
    c2 = get_cycles();

    usec = (g2.tv_sec - g1.tv_sec) * 1000000 + g2.tv_usec - g1.tv_usec;
    eHz  = (long double)(c2-c1)/(long double)usec*(long double)1000000;

    if ( check_clock_consistency(eHz,Hz) )
        return eHz;

    /* clock is not consistent with gettimeofday()... (SMP or Mobile CPU) */
    warnx("SMP or Mobile CPU detected! Using cpu_khz read from /proc/cpuinfo...");
    return Hz;
}

