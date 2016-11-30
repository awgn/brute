/*
    $Id: brute-api.c,v 1.11 2008-05-19 19:54:21 awgn Exp $

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

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <features.h>		/* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>	/* the L2 protocols */
#else
#include <linux/if_packet.h>
#include <linux/if_ether.h>	/* The L2 protocols */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

#include <global.h>
#include <prototype.h>

_unused static
const char cvsid[] = "$Id: brute-api.c,v 1.11 2008-05-19 19:54:21 awgn Exp $";

/* update dst a/o src mac with random octects */
void
_update_link(frame_t *f)
{
    uint32_t *addr;

    if ( opt.rand_mac_dst ) {
        addr=(uint32_t *)f->ethh->h_dest;
        *(addr++)= (uint32_t) genrand_int32();
        *(uint16_t *)addr= (uint16_t)(genrand_int32() & 0xffff);
    }

    if ( opt.rand_mac_src ) {
        addr=(uint32_t *)f->ethh->h_source;
        *(addr++)= (uint32_t ) genrand_int32();
        *(uint16_t *)addr= (uint16_t)(genrand_int32() & 0xffff);
    }
}


/* update dst a/o src ip4 address with random octects (bitmask) */
void
_update_host4(frame_t *f)
{
    uint32_t *ip;

    if ( opt.rand_host_src >= 0 ) {
        ip  = (uint32_t *) &f->iph->saddr;
        *ip = (*ip & opt.mask_host_src) | (genrand_int32() & ~opt.mask_host_src);
    }

    if ( opt.rand_host_dst >= 0 ) {
        ip  = (uint32_t *) &f->iph->daddr;
        *ip = (*ip & opt.mask_host_dst) | (genrand_int32() & ~opt.mask_host_dst);
    }
}


/* update dst a/o src ip6 address with random octects (bitmask) */
void
_update_host6(frame_t *f)
{
    char __getrand_uint8() {
        static int r0, i;
        static char *r;

        if ( i%sizeof(int) ){
            i++;
            return *r++;
        }
        r0 = genrand_int32();
        r  = (char *)&r0;
        i++;
        return *r++;
    }

    void __randomize_ipv6(int bit, u_char *pip, char bmask) {
        int i;

        /* move to the boundary byte */
        pip += bit/8;

        /* set the boundary random byte */
        *pip = (*pip & bmask) | ( __getrand_uint8() & ~bmask);

        pip++;

        for ( i=bit/8+1; i < 16; i++) {
            *pip++ = __getrand_uint8();
        }
    }

    if ( opt.rand_host_src >=0 ) {
        __randomize_ipv6(opt.rand_host_src, f->ip6h->ip6_src.s6_addr, opt.mask_host6_src);
    }

    if ( opt.rand_host_dst >=0 ) {
        __randomize_ipv6(opt.rand_host_dst, f->ip6h->ip6_dst.s6_addr, opt.mask_host6_dst);
    }

}

/* according to the mac address (even rondom) it build the stateless
   autoconfiguration ipv6 (64 prefix bits)*/
void
_update_host6_eui64(frame_t *f)
{
    void stateless_eui64(u_char *pin, u_char *pmac) {

        pin    += (64>>3);	// skip the 64bits prefix
        *pin    = *pmac++;
        *pin   &= ~1;
        *pin   |= (*pin)>>7;
        *pin++ |= 0x2;
        *pin++  = *pmac++;
        *pin++  = *pmac++;
        *pin++  = 0xff;
        *pin++  = 0xfe;
        *pin++  = *pmac++;
        *pin++  = *pmac++;
        *pin++  = *pmac++;
    }

    stateless_eui64(f->ip6h->ip6_src.s6_addr,f->ethh->h_source);
    stateless_eui64(f->ip6h->ip6_dst.s6_addr,f->ethh->h_dest);

}


/* update-frame dispatcher */
void
brute_update_frame(frame_t *f)
{
    /* update link layer */
    if (update_link)
        update_link(f);

    /* update ip layer */
    if (update_host)
        update_host(f);

    /* recompute checksum if required (ipv4) */
    if ( af_family == AF_INET6 )
        return;

    f->iph->id    = htons((u_short)global._sent);
    f->iph->check = 0;
    f->iph->check = in_chksum((u_short *) f->iph, f->iph->ihl << 2);

}


/*
 * compile mac header with mac string addresses. ie: "11:22:33:44:55:66"
 */
void
brute_build_smac(f, h_dest, h_source)
frame_t *f;
char *h_dest;
char *h_source;
{
    struct ether_addr *mac;

    if (!opt.ether_off)
        return;

    mac = ether_aton(h_dest);
    memcpy(f->ethh->h_dest, mac->ether_addr_octet, ETH_ALEN);

    mac = ether_aton(h_source);
    memcpy(f->ethh->h_source, mac->ether_addr_octet, ETH_ALEN);

    switch(af_family) {
    case AF_INET:
        f->ethh->h_proto = htons(ETH_P_IP);
        break;
    case AF_INET6:
        f->ethh->h_proto = htons(ETH_P_IPV6);
        break;
    default:
        fatal(__INTERNAL__);
    }
}


/*
 * compile mac header with ether_addr
 */
void
brute_build_mac(f, hdr)
frame_t *f;
struct ethhdr *hdr;
{
    if (!opt.ether_off)
        return;

    memcpy(f->ethh->h_dest, hdr->h_dest, ETH_ALEN);
    memcpy(f->ethh->h_source, hdr->h_source, ETH_ALEN);

    switch(af_family) {
    case AF_INET:
        f->ethh->h_proto = htons(ETH_P_IP);
        break;
    case AF_INET6:
        f->ethh->h_proto = htons(ETH_P_IPV6);
        break;
    default:
        fatal(__INTERNAL__);
    }
}


/*
 * interface function: compile either ipv4 or ipv6 header.
 */
void
brute_build_ip(f, len, id, ttl, tos, tclass, flow_label, hoplim, h_src, h_dst )
frame_t *f;
uint16_t len;
uint16_t id;
uint8_t ttl;
uint8_t tos;
uint8_t tclass;
uint16_t flow_label;
uint8_t hoplim;
struct hostent *h_src;
struct hostent *h_dst;
{
    struct in_addr src_addr, dst_addr;
    struct in6_addr	src6_addr, dst6_addr;

    ASSERT(h_src != NULL);
    ASSERT(h_dst != NULL);

    switch (af_family) {
    case AF_INET:
        memcpy((char *)&src_addr.s_addr, h_src->h_addr, h_src->h_length);
        memcpy((char *)&dst_addr.s_addr, h_dst->h_addr, h_dst->h_length);

        brute_build_ip4(  f,
                          5,                                            // ihl (20 bytes)
                          4,                                            // ipv4
                          tos,                                       	// tos
                          len-sizeof(struct ethhdr)-sizeof(uint32_t),   // tot_len= framelen-sizeof(eth)-4
                          id,                         			// initial ip_id
                          IP_DF,                                        // see /usr/include/netinet/ip.h

                          // IP_RF (reserved fragment flag)
                          // IP_DF (dont fragment flag)
                          // IP_MF (more fragments flag)

                          (ttl ? : 64),                              	// ttl
                          IPPROTO_UDP,                                  // see /usr/include/netinet/in.h
                          0,                                            // checksum: 0 -> compute the checksum
                          &src_addr, 					// source ip
                          &dst_addr); 					// destination ip

        break;
    case AF_INET6:
        memcpy((char *)&src6_addr.s6_addr, h_src->h_addr, h_src->h_length);
        memcpy((char *)&dst6_addr.s6_addr, h_dst->h_addr, h_dst->h_length);

        brute_build_ip6(  f,
                          flow_label,
                          tclass,
                          len-sizeof(struct ethhdr)-sizeof(struct ip6_hdr)-sizeof(uint32_t),
                          IPPROTO_UDP,
                          hoplim ? : 64,
                          0,
                          &src6_addr,
                          &dst6_addr);

        break;
    default:
        fatal(__INTERNAL__);
    }

    return;
}


/*
 * compile ip4 header
 */
void
brute_build_ip4(f, ihl, version, tos, len, id, frag_off, ttl, protocol, check, saddr, daddr)
frame_t *f;
int ihl;
uint8_t version;
uint8_t tos;
uint16_t len;
uint16_t id;
uint16_t frag_off;
uint8_t ttl;
uint8_t protocol;
uint16_t check;
struct in_addr *saddr;
struct in_addr *daddr;
{
    /* checks */

    if (len < 46 || len > 1500)
        fatal("%s(): tot_len=%d bad lenght", __FUNCTION__, len);

    f->iph->ihl = ihl;		/* 5 means hlen=20 bytes, ip_options not included */
    f->iph->version = version;	/* 4 for ipv4 */
    f->iph->tos = tos;
    f->iph->tot_len = htons(len);	/* sizeof(struct iphdr) +
                                     * sizeof(struct udphdr) +
                                     * sizeof(upd_data) */
    f->iph->id = htons(id);
    f->iph->frag_off = htons(frag_off);
    f->iph->ttl = ttl;
    f->iph->protocol = protocol;
    f->iph->check = check;
    f->iph->saddr = saddr->s_addr;
    f->iph->daddr = daddr->s_addr;

    if (check == 0)
        f->iph->check = _sw_chksum((u_short *) f->iph, f->iph->ihl << 2);
}


/*
 * compile ip6 header
 */

#define IP6_VER(x)		((x)<<4 )
#define IP6_TCL(x)		((x & 0x0f) <<12 )
#define IP6_FID(x)		( htonl(x) )
void
brute_build_ip6(f,flow,tclass,plen,nxt,hlim,un2_vfc,src,dst)
frame_t *f;
uint32_t flow;
uint32_t tclass;
uint16_t plen;
uint8_t  nxt;
uint8_t  hlim;
uint8_t un2_vfc;
struct in6_addr *src;
struct in6_addr *dst;
{
    if (plen < 8 || plen > 1460)
        fatal("%s(): ipv6 plen=%d error. Bad lenght", __FUNCTION__, plen);

    f->ip6h->ip6_flow = ( IP6_VER(6) | IP6_TCL(tclass) | IP6_FID(flow) );
    f->ip6h->ip6_plen = htons(plen);
    f->ip6h->ip6_nxt  = nxt;
    f->ip6h->ip6_hlim = hlim;

    memcpy(&f->ip6h->ip6_src,&src->s6_addr, sizeof(struct in6_addr));
    memcpy(&f->ip6h->ip6_dst,&dst->s6_addr, sizeof(struct in6_addr));

}


/*
 * compile udp header
 */
void
brute_build_udp(f, sport, dport, ulen, sum)
frame_t *f;
uint16_t sport;
uint16_t dport;
uint16_t ulen;
uint16_t sum;
{
    f->udph->source = htons(sport);
    f->udph->dest = htons(dport);
    f->udph->len = htons(ulen);
    f->udph->check = htons(sum); //not required
}

/*
   RFC2544 ----

   C.2.6.4 Test Frames
   UDP echo request on Ethernet

   -- UDP DATA
   42     00 01 02 03 04 05 06 07    some data***
   50     08 09 0A 0B 0C 0D 0E 0F
 */
void
brute_udpdata_rfc2544(frame_t * f)
{
    int i;

    /* reset arena: frame format rfc2544 */

    for (i = 0; &f->udata[i] < &f->data[ETH_FRAME_LEN]; i++)
        f->udata[i] = (char) i;

}


/*
 * alloc space and initialize the frame
 */
frame_t *
brute_realloc_frame(frame_t * k)
{
    uint32_t h;
    frame_t *f;

    /* check whether frame was already alloced */

    if (k == NULL)
        goto new;

    h = (uint32_t) k->hash;
    k->hash = 0;
    k->hash = hash((void *) k, sizeof(frame_t));

    if (h != k->hash)
        fatal("%s(): frame corrupted", __FUNCTION__);

    return k;

new:
    if ((f = malloc(sizeof(frame_t))) == NULL)
        fatal(__INTERNAL__);

    if ((f->data = malloc(ETH_FRAME_LEN)) == NULL)
        fatal(__INTERNAL__);

    f->ethh   = ( opt.ether_off ? (struct ethhdr *) (f->data) : (struct ethhdr *)NULL);
    f->cooked = ( opt.ether_off ? 0 : 1 );

    switch (af_family) {
    case AF_INET:
        f->ip6h  = (struct ip6_hdr *) NULL;
        f->iph   = (struct iphdr   *) (f->data + opt.ether_off);
        f->udph  = (struct udphdr  *) (f->data + opt.ether_off + sizeof(struct iphdr));
        f->udata = (u_char         *) (f->data + opt.ether_off + sizeof(struct iphdr) + sizeof(struct udphdr));
        break;
    case AF_INET6:
        f->iph   = (struct iphdr   *) NULL;
        f->ip6h  = (struct ip6_hdr *) (f->data + opt.ether_off);
        f->udph  = (struct udphdr  *) (f->data + opt.ether_off + sizeof(struct ip6_hdr));
        f->udata = (u_char         *) (f->data + opt.ether_off + sizeof(struct ip6_hdr) + sizeof(struct udphdr));
        break;
    default:
        fatal(__INTERNAL__);
    }

    /* prevent free of unallocated frame */
    f->hash = 0;
    f->hash = hash((void *) f, sizeof(frame_t));

    return f;
}


/*
 * free space
 */
int
brute_delete_frame(frame_t * f)
{
    uint32_t h;

    if (f == NULL)
        goto err;

    h = (uint32_t) f->hash;
    f->hash = 0;
    f->hash = hash((void *) f, sizeof(frame_t));

    if (h != f->hash || f->data == NULL)
        goto err;

    free(f->data);
    free(f);

    return 0;
err:
    fatal("brute_delete_frame(): frame was not properly initialized");

    return 0; //unreachable
}


/*
 * returns the number of bytes to pass to sendto()
 */
int
brute_framelen_to_bytes(int i)
{
    switch (opt.ether_off) {
    case ETH_HLEN:
        return i - sizeof(uint32_t);
    case 0:
        return i - sizeof(uint32_t) - ETH_HLEN;
    default:
        fatal(__INTERNAL__);
    }

    return 0;		/* unreachable */
}

/*
 * given the hostname, it returns a hostent structure
 */
struct hostent *
brute_gethostbyname(const char *s)
{
    struct hostent *h;

    h = gethostbyname2(s,af_family);
    if (h == NULL)
        fatal("\"%s\": h_errno(%d): %s!?",s,h_errno,hstrerror(h_errno));

    return dup_hostent(h);
}


char *
brute_inet_ntop(struct hostent *h)
{
    struct in_addr  in;
    struct in6_addr in6;
    char *dup;

    if (h==NULL)
        return NULL;

    dup = (char *)malloc(128);

    switch (af_family) {
    case AF_INET:
        memcpy(&in.s_addr,h->h_addr_list[0],h->h_length);
        if ( inet_ntop(AF_INET,&in,dup,128) == NULL )
            fatal("inet_ntop(AF_INET...");
        break;
    case AF_INET6:
        memcpy(&in6.s6_addr,h->h_addr_list[0],h->h_length);
        if ( inet_ntop(AF_INET6,&in6,dup,128) == NULL)
            fatal("inet_ntop(AF_INET6...");
        break;
    default:
        fatal(__INTERNAL__);
    }

    return dup;
}


ssize_t
brute_sendto_pf_inet(int s, frame_t * f, size_t len, int flags)
{
    switch(af_family) {
    case AF_INET:
        return sendto(  s,
                        f->iph,
                        len-opt.ether_off,
                        flags,
                        (struct sockaddr *) &sock_out,
                        sizeof(struct sockaddr)) ;
    case AF_INET6:
        return sendto(  s,
                        f->ip6h,
                        len-opt.ether_off,
                        flags,
                        (struct sockaddr *) &sock6_out,
                        sizeof(struct sockaddr_in6)) ;
        break;
    default:
        fatal(__INTERNAL__);

    }

    return 0;
}

ssize_t
brute_sendto_pf_packet(int s, frame_t * f, size_t len, int flags)
{
    return sendto(	s,
                    f->data,
                    len,
                    flags,
                    (struct sockaddr *) &sock_ll_out,
                    sizeof(struct sockaddr_ll));
}

#define __kernel_vsyscall       0xffffe400      /* linux kernel 2.6 */
#define vsendto(x,r) \
asm volatile ( "call *%4 \n" : "=a" ((r)) : 	\
               "a" (102),  	\
               "b" (11) ,	\
               "c" ((x)),	\
               "r" (__kernel_vsyscall): "memory" );
ssize_t
brute_vsendto_pf_packet(int s, frame_t * f, size_t len, int flags)
{
#ifdef CONFIG_686
    uint32_t args[6];
    int ret;

    args[0] = (uint32_t)s;
    args[1] = (uint32_t)f->data;
    args[2] = (uint32_t)len;
    args[3] = (uint32_t)flags;
    args[4] = (uint32_t)&sock_ll_out;
    args[5] = (uint32_t)sizeof(struct sockaddr_ll);
    vsendto(args,ret);
    return ret;
#else
    fatal("vsyscall not implemented for this architecture!\n");
    return 0;
#endif
    return 0;
}

