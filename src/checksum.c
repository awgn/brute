/*
    $Id: checksum.c,v 1.11 2008-01-12 16:10:21 awgn Exp $

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
#include <netinet/in.h>

#include <global.h>

_unused static
const char cvsid[]= "$Id: checksum.c,v 1.11 2008-01-12 16:10:21 awgn Exp $";

/*
 * compute the IP header checksum in a classic fashion.
 */
u_short
_sw_chksum(const u_short *addr, register u_int len)
{
#ifndef USE_HW_CHECKSUM
    int nleft = len;
    const u_short *w = addr;
    u_short answer;
    int sum = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
        sum += htons(*(u_char *)w<<8);

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */
    answer = ~sum;                          /* truncate to 16 bits */
    return answer;
#else
    return 0;
#endif
}


/*
 * hardware checksum
 */
u_short
_hw_chksum(const u_short *addr, register u_int len)
{
    return (u_short)0;
}


#if 0
/*
 * fast checksum algorithm valid only for sequence of similar frames,
 * differing only in a 2 byte word (such as ip_id).
 */
u_short
_ft_chksum(const u_short *addr, register u_int len)
{
    static u_short old;
    static int init;
    /*
       Given the following notation:

       HC  - old checksum in header
       C   - one's complement sum of old header
       HC' - new checksum in header
       C'  - one's complement sum of new header
       m   - old value of a 16-bit field
       m'  - new value of a 16-bit field


       HC' = ~(~HC + ~m + m')

       register int sum;

       sum  = ~ntohs(old) & 0xffff;
       sum  += ~(ip_id-1)        & 0xffff;
       sum  += ip_id;
       sum  = (sum >> 16) + (sum & 0xffff);     // (add hi 16 to low 16)
       sum += (sum >> 16);                      // (add carry)
       old_chksum  = htons((u_short)~sum);

       In this case, whereas only ip_id++ changes, we have:

       .--- 0  : if m'=0xffff and m=0
       (*) ~m+ m'= /
       \
       `--- 1  : otherwise

       (*) the sum is computed with 1's complement math.

       This let to simplify the algorithm as follows:
     */

    register int sum;

    if (init++ == 0)
        return old=_sw_chksum(addr,len);

#if __BYTE_ORDER == __LITTLE_ENDIAN
    sum  = old - ( (u_short)global.sent == 0 ? 0 : 0x100 );
    sum -= ( sum & 0xffff0000 ? 1 : 0 );
#elif __BYTE_ORDER == __BIG_ENDIAN
    sum  = old - ( (u_short)global.sent == 0 ? 0 : 0x1 );
#else
#error  "fast_chksum() not implemented for your endianess"
#endif
    return old=(sum & 0xffff);


}
#endif

