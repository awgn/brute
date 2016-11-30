/*
    $Id: socket.c,v 1.18 2008-01-12 16:10:23 awgn Exp $

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


#include <netinet/ether.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <global.h>
#include <prototype.h>


_unused static
const char cvsid[] = "$Id: socket.c,v 1.18 2008-01-12 16:10:23 awgn Exp $";


/*
 * given the ifname it returns the mac address
 */

char *
iface_getmac(int fd, const char *dev)
{
    static struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1 )
        fatal("interface %s not found", dev);

    return ifr.ifr_ifru.ifru_hwaddr.sa_data;
}


/*
 * given the name it returns the interface id.
 */
int
iface_getid(int fd, const char *dev)
{
    struct ifreq ifr;
    int ret;

    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

    if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
        fatal("interface %s not found", dev);

    ret = ifr.ifr_ifindex;

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
        fatal("SIOCGIFFLAGS: %s", strerror(errno));

    if ((ifr.ifr_flags & IFF_UP) == 0)
        fatal("interface %s is down", dev);

    return ret;
}



/*
 * dup_hostent - create hostent in one memory block (KAME/NETBSD)
 */

struct hostent *dup_hostent(hp)
struct hostent *hp;
{
    struct hostent_block {
        struct hostent host;
        char   *addr_list[1];
    };
    struct hostent_block *hb;
    int     count;
    char   *data;
    char   *addr;

    for (count = 0; hp->h_addr_list[count] != 0; count++)
        /* void */ ;

    if ((hb = (struct hostent_block *) malloc(sizeof(struct hostent_block)
                                              + (hp->h_length + sizeof(char *)) * count)) == 0) {
        fprintf(stderr, "Sorry, out of memory\n");
        exit(1);
    }
    memset((char *) &hb->host, 0, sizeof(hb->host));
    hb->host.h_addrtype = hp->h_addrtype;
    hb->host.h_length = hp->h_length;
    hb->host.h_addr_list = hb->addr_list;
    hb->host.h_addr_list[count] = 0;
    data = (char *) (hb->host.h_addr_list + count + 1);

    for (count = 0; (addr = hp->h_addr_list[count]) != 0; count++) {
        hb->host.h_addr_list[count] = data + hp->h_length * count;
        memcpy(hb->host.h_addr_list[count], addr, hp->h_length);
    }

    return &hb->host;
}


#define probe_connect(fd,sockaddr,size) {\
    if ( connect(fd,sockaddr,size)== -1 )\
    switch(errno) {\
    case ECONNREFUSED:\
                      break;\
    case ENETUNREACH:\
    case EHOSTDOWN:\
    case EHOSTUNREACH:\
                      fatal("gateway unreachable?");\
    case ETIMEDOUT:\
                   fatal("timeout while attempting connection?");\
    default:\
            fatal(__INTERNAL__);\
    } \
} while (0)


/*
 * create the PF_PACKET socket, and setup the ether header
 */

int
create_socket(char *ifname, char *s_mac, char *d_mac, int mode)
{
    struct ether_addr *s_MAC, *d_MAC;
    struct in_addr  in_addr4;
    struct in6_addr in_addr6;
    int sock=-1, true=1, probe=-1;

    /*
     * create PF_INET socket
     */

    if (opt.pf_inet) {

        msg(MSG_INFO "packet routed to ->(%s)\n",dst_ip);

        switch(af_family) {
        case PF_INET:
            sock=socket(PF_INET,SOCK_RAW,IPPROTO_RAW);
            probe=socket(PF_INET,SOCK_STREAM,0);

            if (sock == -1 || probe == -1 )
                fatal("socket()");

            if ( inet_pton(PF_INET,dst_ip,&in_addr4) <= 0 )
                fatal("inet_pton() bad destination IPv4");

            sock_out.sin_addr.s_addr = in_addr4.s_addr;
            sock_out.sin_port = htons(1025);
            sock_out.sin_family = PF_INET;

            if ( setsockopt(sock,SOL_IP,IP_HDRINCL,&true,sizeof(int)) == -1)
                fatal("setsockopt(sock,SOL_IP,IP_HDRINCL,...)");

            if ( setsockopt(probe,SOL_SOCKET,SO_DONTROUTE,&true,sizeof(int)) == -1)
                fatal("setsockopt(sock,SOL_SOCKET,SO_DONTROUTE,...)");

            probe_connect(probe,(struct sockaddr *) &sock_out,sizeof(struct sockaddr_in));
            close(probe);
            break;
        case PF_INET6:
            sock=socket(PF_INET6,SOCK_RAW,IPPROTO_RAW);
            probe=socket(PF_INET6,SOCK_STREAM,0);

            if (sock == -1 || probe == -1 )
                fatal("socket()");

            if ( inet_pton(PF_INET6,dst_ip,&in_addr6) <= 0 )
                fatal("inet_pton() bad destination IPv6");

            sock6_out.sin6_family = PF_INET6;
            sock6_out.sin6_port = htons(0);
            sock6_out.sin6_flowinfo = 0;
            sock6_out.sin6_scope_id = 0;
            memcpy(&sock6_out.sin6_addr, in_addr6.s6_addr, sizeof(struct in6_addr));

            if ( setsockopt(probe,SOL_SOCKET,SO_DONTROUTE,&true,sizeof(int)) == -1)
                fatal("setsockopt(sock,SOL_SOCKET,SO_DONTROUTE,...)");

            probe_connect(probe,(struct sockaddr *) &sock6_out,sizeof(struct sockaddr_in6));
            close (probe);
            break;
        default:
            fatal(__INTERNAL__);
        }
        if (sock == -1)
            fatal("socket()");

        return sock;
    }


    /*
     * create PF_PACKET incoming socket
     */
    if (mode == 0) {

        sock=socket(PF_PACKET, SOCK_RAW, htons(eth_p_ip));
        msg ( MSG_READ  "create: socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IPx))=%d\n",sock);

        if (sock == -1)
            fatal("socket");

        /* incoming socket */
        sock_ll_in.sll_family   = AF_PACKET; /* Always AF_PACKET */
        sock_ll_in.sll_halen    = ETH_ALEN;   /* Length of address */
        sock_ll_in.sll_protocol = htons(eth_p_ip); /* Physical layer protocol */
        sock_ll_in.sll_ifindex  = iface_getid(sock, ifname);        /* Interface number */

        /* bind socket to interface */
        if ( bind(sock, (struct sockaddr *) &sock_ll_in,sizeof(struct sockaddr_ll)) == -1 )
            fatal("socket bind() error");


        /* put the interface in promis mode */
        iface_setprom(sock, ifname);

        return sock;
    }


    /*
     * create PF_PACKET outgoing socket
     */

    sock=socket(PF_PACKET, SOCK_RAW, htons(eth_p_ip));
    if (sock==-1)
        fatal("socket");

    msg ( MSG_WRITE "create: socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IPx))=%d\n",sock);

    sock_ll_out.sll_family   = AF_PACKET; 			/* Always AF_PACKET */
    sock_ll_out.sll_halen    = ETH_ALEN;   			/* Length of address */
    sock_ll_out.sll_protocol = htons(eth_p_ip); 		/* Physical layer protocol */
    sock_ll_out.sll_ifindex  = iface_getid(sock, ifname);   /* Interface number */

    switch(opt.mac_src) {
    case 0:
        /* non specified */
        memcpy(global.ethh.h_source, iface_getmac(sock,ifname), ETH_ALEN);
        break;
    case 1:
        /* given */
        s_MAC = ether_aton(s_mac);
        if (s_MAC == NULL)
            fatal("source MAC, wrong format!");
        memcpy(global.ethh.h_source, s_MAC->ether_addr_octet, ETH_ALEN);
        break;

    default: {
        uint32_t *addr=(uint32_t *)global.ethh.h_source;
        *(addr++)= (uint32_t) genrand_int32();
        *(uint16_t *)addr= (uint16_t)(genrand_int32() & 0xffff);
        }
    }

    switch(opt.mac_dst) {
    case 0: /* impossible */
    case 1:
        /* given */
        d_MAC = ether_aton(d_mac);
        if (d_MAC == NULL)
            fatal("destination MAC, wrong format!");
        memcpy(global.ethh.h_dest, d_MAC->ether_addr_octet, ETH_ALEN);
        break;

    default: {
        uint32_t *addr=(uint32_t *)global.ethh.h_dest;
        *(addr++)= (uint32_t) genrand_int32();
        *(uint16_t *)addr= (uint16_t)(genrand_int32() & 0xffff);
        }
    }

    return sock;
}


/*
 * set non blocking socket
 */
void
set_nonblock(sock)
int sock;
{
    register int val = 0;
    val = fcntl(sock, F_GETFL, val);
    val |= O_NONBLOCK;
    fcntl(sock, F_SETFL, val);
}


/*
 * set blocking socket
 */
void
set_block(sock)
int sock;
{
    register int val = 0;
    val = fcntl(sock, F_GETFL, val);
    val &= ~O_NONBLOCK;
    fcntl(sock, F_SETFL, val);
}

