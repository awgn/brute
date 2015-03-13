/*
    $Id: prototype.h,v 1.24 2008-01-12 16:10:19 awgn Exp $
 
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

#ifndef PROTOTYPE_H
#define PROTOTPYE_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <config.h>

/* EOF */
void brute_update_frame(frame_t *f);
void brute_build_smac(frame_t *, char *, char *);
void brute_build_mac(frame_t *, struct ethhdr *);
void brute_build_ip(frame_t *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t, struct hostent *, struct hostent *);
void brute_build_ip4(frame_t *, int, int, uint8_t, uint16_t, uint16_t, uint16_t, uint8_t, uint8_t, uint16_t, struct in_addr *, struct in_addr *);
void brute_build_ip6(frame_t *, uint32_t, uint32_t, int, int, int, int, struct in6_addr *, struct in6_addr *);
void brute_build_udp(frame_t *, uint16_t, uint16_t, uint16_t, uint16_t);
void brute_udpdata_rfc2544(frame_t *);
frame_t *brute_realloc_frame(frame_t *);
int brute_delete_frame(frame_t *);
int brute_framelen_to_bytes(int);
struct hostent *brute_gethostbyname(const char *);
char *brute_inet_ntop(struct hostent *);

/* missing */
size_t strlcpy(char *, const char *, size_t );
size_t strlcat(char *, const char *, size_t );

/* brute.c */
void sanity_checks(void);
int main(int, char **);

/* checksum.c */
u_short _sw_chksum(const u_short *, register uint32_t);
u_short _hw_chksum(const u_short *, register uint32_t);

/* destructor.c */
void thefunctionafter(int);

/* engine.c */
int find_label(const char *);
void processor(void);

/* fatal.c */
void fatal(char *, ...);

/* fnv.c */
unsigned long hash(void *, int );

/* memutil.c */
void *f_memcpy(void *, const void *, const void *, size_t);
void *d_memadd(void *, const void *, size_t);

/* module.c */
int count_token(struct tqh *);
int search_token(char *, struct tqh *);
void module_check(struct module_descriptor *);
void register_module(struct module_descriptor *);

/* netdev.c */
struct ethtool_drvinfo *ethernet_info(const char *);
void mii_testlink(char *);
void iface_setprom(int, char *);
void iface_extprom(int, char *);

/* parser.c */
union paret eval_atom_static(struct atom *);
int eval_atom_dynamic(struct atom *);
void parser_error(int i, char *t);
char *remove_leading_set(char **s, char *set);
union paret par_eval_rvalue(char *,enum eval_mode, int *);
struct atom par_parse_atom(const char *p);
char *par_get_atom(char **s, char sep);
char *par_get_command(char **s);
char *par_get_label(char **p);
int brute_eval_atom(struct atom *);
int brute_eval_int(struct atom *);
double brute_eval_double(struct atom *);
void *brute_eval_host(struct atom *);
void *brute_eval_addr(struct atom *);
void core_parser(void);

/* perf-hash.c */
int free_hashtable(perf_hash_t *);
int setup_hashtable(perf_hash_t *, struct tqh *);
int qsearch(const char *p,perf_hash_t *);

/* proc_parser.c */
unsigned long long get_cpu_hz(int);

/* scheduler.c */
void set_realtime(int, int);
void unset_realtime(int);
pid_t get_pid(const char *, pid_t);

/* socket.c */
char *brute_inet_ntop(struct hostent *);
int iface_getid(int, const char *);
struct hostent *dup_hostent(struct hostent *);
struct hostent * brute_gethostbyname(const char *);
int create_socket(char *, char *, char *, int);
void set_block(int);
void set_nonblock(int);
//ssize_t brute_sendto(int , frame_t *, size_t , int );

/* rtld.c */
void register_obj(char *, void *, int, enum symbol_type);
void *search_sym(char *, int *, enum symbol_type);
void update_mod_line(void *, void *, size_t );

/* mt19937ar.c */
void init_genrand(unsigned long s);
void init_by_array(unsigned long init_key[], unsigned long key_length);
unsigned long genrand_int32(void);
long genrand_int31(void);
double genrand_real1(void);
double genrand_real2(void);
double genrand_real3(void);
double genrand_res53(void);

/* signal.c */
sigfunc *signal_bsd(int , sigfunc *);

#endif
