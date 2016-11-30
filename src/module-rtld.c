/*
    $Id: module-rtld.c,v 1.7 2008-01-12 16:10:22 awgn Exp $

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

/*
 * constant
 */
static int sizeof_ethhdr = sizeof(struct ethhdr);
static int sizeof_iphdr  = sizeof(struct iphdr);
static int sizeof_ip6hdr = sizeof(struct ip6_hdr);
static int sizeof_udphdr = sizeof(struct udphdr);
static int sizeof_crc    = sizeof(uint32_t);

/*
 * dummy sum
 */
static int
sum(vargs_t * vt)
{
    return (args(vt,0)+args(vt,1));
}


/*
 * given the udp-data length, it returns the
 * frame lenght
 */
static int
udp_data(vargs_t * vt)
{

    switch (af_family) {
    case AF_INET:
        return (sizeof_ethhdr+ \
                sizeof_iphdr + \
                sizeof_udphdr+ \
                sizeof_crc   + \
                args(vt,0));
    case AF_INET6:
        return (sizeof_ethhdr+ \
                sizeof_ip6hdr+ \
                sizeof_udphdr+ \
                sizeof_crc   + \
                args(vt,0));
    default:
        fatal(__INTERNAL__);
    }

    return (0); /* UNREACHED */
}


PUB_VAR(sizeof_ethhdr);
PUB_VAR(sizeof_iphdr);
PUB_VAR(sizeof_ip6hdr);
PUB_VAR(sizeof_udphdr);
PUB_VAR(sizeof_crc);


PUB_FUNCTION(udp_data,1);
PUB_FUNCTION(sum,2);
