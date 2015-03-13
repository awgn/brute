/*
    $Id: brute.c,v 1.35 2008-05-19 19:47:07 awgn Exp $

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


    brute:    High Performance UDP generator for Linux 2.4/6.
              Benchmarking Methodology for Network Interconnect Devices. [rfc2544 compliant]

 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <asm/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#define  GLB_OWNER
#include <global.h>
#include <config.h>

#include <prototype.h>
#include <hw_checksum.h>
#include <getopt.h>

_unused static 
const char cvsid[]= "$Id: brute.c,v 1.35 2008-05-19 19:47:07 awgn Exp $";

static 
const char license[]="\n\
Copyright (c) 2003-2007 Nicola Bonelli <bonelli@antifork.org>.\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.";

struct ethtool_drvinfo *ethernet_info(const char *);

static const struct option long_options[] = {
    {"version", 	  no_argument, NULL, 'v' },
    {"silent", 	  no_argument, NULL, 'S' },
    {"vsyscall", 	  no_argument, NULL, 'V' },
    {"help", 	  no_argument, NULL, 'h' },
    {"rand-dst-host", no_argument, NULL, 'r' },
    {"rand-src-host", no_argument, NULL, 'q' },

    {"sw-chksum", 	  no_argument, NULL, 'c' },
    {"hw-chksum", 	  no_argument, NULL, 'w' },
    {"urand-seed", 	  no_argument, NULL, 'k' },
    {"ipv6", 	  no_argument, NULL, '6' },
    {"ipv6-eui64",	  no_argument, NULL, 'A' },
    {"non-int",       no_argument, NULL, '!'},

    {"pf-inet", 	  required_argument, NULL, 'x' },
    {"seed", 	  required_argument, NULL, 'y' },
    {"script-file",   required_argument, NULL, 'f' },
    {"ifout", 	  required_argument, NULL, 'i' },
    {"ifin",  	  required_argument, NULL, 'j' },
    {"sched-priority",required_argument, NULL, 'p' },
    {"src-mac", 	  required_argument, NULL, 's' },
    {"dst-mac", 	  required_argument, NULL, 'd' },
    {"crono-vector",  required_argument, NULL, 'z' },
    {"use-clock",     required_argument, NULL, 'U' },
    {"estimate-pps",  required_argument, NULL, 'E' },
    {0, 0, 0, 0}
};

/* usage string */

static const char usage_str[] = "\
Usage:%s [OPTIONS]...                                   	           \n\
Mac:                                                   		           \n\
   -d,  --dst-mac=MAC              *destination mac: address or \"rand\"   \n\
   -s,  --src-mac=MAC               source mac: address or \"rand\"        \n\
Scheduler:                                             	                   \n\
   -p,  --sched-priority=NUMBER     round-robin priority (0=off..%d)       \n\
        --non-int                   non-interruptible mode (very high perf)\n\
Checksum:								   \n\
   -c,  --sw-chksum                 force software ip checksum             \n\
   -w,  --hw-chksum                 force hardware ip checksum             \n\
Random:                                                	                   \n\
   -y,  --seed=VAL                  set seed for mersenne twister          \n\
   -k,  --urand-seed                use /dev/urandom to set the seed       \n\
   -r,  --rand-dst-host             prefix bitmask: 24,16 (v4), 64 (v6)    \n\
   -q,  --rand-src-host             prefix bitmask: 24,16 (v4), 64 (v6)    \n\
Other:                                                                     \n\
   -i,  --ifout=NAME               *outgoing interface                     \n\
   -j,  --ifin =NAME                incoming interface                     \n\
   -f,  --script-file=FILE         *configuration script                   \n\
   \n\
   -x,  --pf-inet=IP                use pf_inet socket (IP address)        \n\
   -6,  --ipv6                      enables IPv6 support                   \n\
        --ipv6-eui64                build ipv6 by stateless autoconfig.    \n\
   -z,  --crono-vector=BIT          set crono-vector-depth                 \n\
   -v,  --version                   display the version and exit           \n\
        --vsyscall                  use virtual systemcall sendto. LK2.6   \n\
        --silent                    suppress the output                    \n\
        --use-clock=HZ              use the given clock                    \n\
        --estimate-pps=n            estimate the CPU clock in n sec.       \n\
   -h,  --help                      print this help                        \n";


extern char *__progname;

extern u_short  _hw_chksum(const u_short *, register u_int);
extern u_short  _sw_chksum(const u_short *, register u_int);

extern void _update_link(frame_t *);
extern void _update_host4(frame_t *);
extern void _update_host6(frame_t *);
extern void	_update_host6_eui64(frame_t *);

extern ssize_t brute_sendto_pf_packet(int , frame_t * , size_t , int );
extern ssize_t brute_vsendto_pf_packet(int , frame_t * , size_t , int );
extern ssize_t brute_sendto_pf_inet(int , frame_t * , size_t , int );

extern void sa_fault(int,siginfo_t *,void *);

void
sanity_checks()
{

#ifndef USE_SETUID
    if (getuid() != geteuid())
        fatal(MSG_FATAL "Brute does not allow setuid-bit to be set for security reason.\n"
              "    configure with --enable-setuid at own risk.");
#else
    if (getuid() != geteuid()) {
        if (setuid(geteuid()) == 0)
            msg(MSG_DIR "WARNING: eleving privileges to uid=%d!\n", getuid());
        else
            msg(MSG_NULL "setuid() error: %s\n", strerror(errno));
    }
#endif
    if (getuid() != 0)
        fatal(MSG_FATAL "%s must run as root", __progname);

    if (file_name == NULL)
        fatal(MSG_FATAL "brute script-file not given");

    if (!opt.mac_dst && !opt.rand_mac_dst && !opt.pf_inet)
        fatal(MSG_FATAL "destination MAC address not given");

    if (ifout == NULL && !opt.pf_inet)
        fatal(MSG_FATAL "outgoing interface not given");

    if ((opt.rand_mac_dst|opt.rand_mac_src) && opt.pf_inet)
        fatal(MSG_FATAL "randon mac and pf-inet socket are mutually exclusive"); 

    if (Hz != 0 &&  estimate_clock_time != 0 )
        fatal(MSG_FATAL "use-clock and estimate-pps are mutually exclusive");

    if (af_family == AF_INET6)
        goto ipv6;

    // ipv4:
    if ( opt.eui64 )
        fatal(MSG_FATAL "eui64 option is not ipv4 compliant");

    if ( opt.rand_host_src < -1 || opt.rand_host_src > 32 )	
        fatal(MSG_FATAL "ipv4: netmask src address (-q) is limited to 32bit");

    if ( opt.rand_host_dst < -1 || opt.rand_host_dst > 32 )
        fatal(MSG_FATAL "ipv4: netmask dst address (-r) is limited to 32bit");

    return;

ipv6:
    if ( opt.rand_host_src < -1 || opt.rand_host_src > 128 )
        fatal(MSG_FATAL "ipv6: netmask src address (-q) is limited to 128bit");

    if ( opt.rand_host_dst < -1 || opt.rand_host_dst > 128 )
        fatal(MSG_FATAL "ipv6: netmask dst address (-r) is limited to 128bit");

}


/* 
 * fprintf may lead to some context switching, responsible to cause spikes in flows. 
 * To avoid this snag fprintf is wrapped with the following function at runtime.  
 */  

static int null(FILE *stream,const char *format, ...) __attribute__ ((format (printf, 2, 3)));
static int
null(FILE *stream,const char *format, ...)
{
    return 0;
}


/*********************
 * Brute starts here...
 *********************/

int
main(int argc, char **argv)
{
    struct ethtool_drvinfo *info;
    struct sigaction siga;
    struct rlimit core;
    unsigned long long mask;	
    int i;

    setup_hashtable(&command_table, &head_commands);
    setup_hashtable(&token_table, &head_tokens);

    fprintf(stderr, "Brute, a network traffic engine\n");

    /* default handlers*/
    xprintf = fprintf;
    brute_sendto = brute_sendto_pf_packet;

    core.rlim_cur= RLIM_INFINITY;
    core.rlim_max= RLIM_INFINITY;

    setrlimit(RLIMIT_CORE,&core);

    while ((i = getopt_long(argc, argv, "6hvSf:i:j:d:s:p:x:y:z:r:q:cwkVEAU", long_options, 0)) != EOF)
        switch (i) {
        case 'q':
            opt.rand_host_src = atoi(optarg);
            if (opt.rand_host_src < 0 || opt.rand_host_src > 128)
                fatal("-q: bad prefix");

            mask = PREFIX4_MASK(opt.rand_host_src);
            opt.mask_host_src  = htonl((uint32_t)mask);
            opt.mask_host6_src = (0xff<<(8-opt.rand_host_src%8) ) & 0xff;
            break;
        case 'r':
            opt.rand_host_dst = atoi(optarg);
            if (opt.rand_host_dst < 0 || opt.rand_host_dst > 128)
                fatal("-r: bad prefix");

            mask = PREFIX4_MASK(opt.rand_host_dst);
            opt.mask_host_dst = htonl((uint32_t)mask);
            opt.mask_host6_dst = (0xff<<(8-opt.rand_host_dst%8) ) & 0xff;
            break;
        case 'd':
            if (strcasecmp(optarg, "rand") == 0) {
                opt.rand_mac_dst= 1;
                opt.mac_dst     = -1;
            } else {
                opt.mac_dst 	= 1;
                dst_mac 	= optarg;
            }
            break;
        case 's':
            if (strcasecmp(optarg, "rand") == 0) {
                opt.rand_mac_src= 1;
                opt.mac_src     = -1;
            } else {
                opt.mac_src 	= 1;
                src_mac 	= optarg;
            }
            break;
        case 'p':
            opt.priority = atoi(optarg);
            break;
        case 'f':
            file_name = optarg;
            break;
        case 'i':
            ifout = optarg;
            break;
        case 'j':
            ifin = optarg;
            break;
        case 'S':
            xprintf = null;
            break;
        case 'v':
            fprintf(stderr, "%s\n\nBrute %s\n", license, VERSION);
            exit(0);
        case 'h':
            printf(usage_str, __progname, sched_get_priority_max(SCHED_RR)-1);
            exit(0);
        case 'c':
            opt.chksum = CHKSUM_SW;	/* software */
            break;
        case 'w':
            opt.chksum = CHKSUM_HW;	/* hardware */
            break;
        case 'k':
            opt.urand= 1;
            break;
        case 'y':
            init_rand_seed = strtoul(optarg, (char **)NULL, 0);
            break;
        case 'z':
            crono_vector_depth = (atoi(optarg)%31);
            break;
        case '6':
            af_family = AF_INET6;
            eth_p_ip  = ETH_P_IPV6;
            hp_sep = '.';
            break;
        case 'x':
            opt.pf_inet= 1;
            brute_sendto= brute_sendto_pf_inet;
            dst_ip= optarg;
            break;
        case 'V':
            brute_sendto = brute_vsendto_pf_packet;
            break;
        case 'A':
            opt.eui64=1;	
            break;
        case 'E':
            estimate_clock_time = atoi(optarg);
            break;
        case 'U':
            opt.use_clock= 1;
            Hz = atoll(optarg);
            break;
        case '!':
            opt.non_int = 1;
            break;
        case '?':
            fatal("opt: unknown option");
            break;
        }

    argc -= optind;
    argv += optind;

    /* perform several sanity checks */
    sanity_checks();

    /* open /dev/urandom */
    global.urand_fd = open(DEV_URANDOM, O_RDONLY);
    if (global.urand_fd == -1)
        fatal("open(\"%s\") error", DEV_URANDOM);

    /* MII tool */
    if (!opt.pf_inet)
        mii_testlink(ifout);

    /* hw_chksum support */

    switch (opt.chksum) {
    case CHKSUM_HW:
        msg(MSG_DIR "hw_chksum enabled\n");
        break;
    case CHKSUM_SW:
        msg(MSG_DIR "sw_chksum enabled\n");
        break;
#if 0
    case CHKSUM_FS:
        msg(MSG_DIR "fast_chksum enabled\n");
        break;
#endif
    default:
        if ((info = ethernet_info(ifout)) != NULL) {
            msg(MSG_NULL "ether_driver=%s version=%s\n", info->driver, info->version);

            for (i = 0; i < sizeof(hw_checksum_list) / sizeof(hw_checksum_list[0]); i++)
                if (strcmp(hw_checksum_list[i], info->driver) == 0)
                    break;

            if (i != sizeof(hw_checksum_list) / sizeof(hw_checksum_list[0])) {
                msg(MSG_DIR "hw_chksum enabled\n");
                opt.chksum = CHKSUM_HW;
            }
        }

        /* default: disable hw_chksum */
        if (opt.chksum == -1) {
            msg(MSG_NULL "sw_chksum (default)\n");
            opt.chksum = CHKSUM_SW;
        }
    }

    /* checksum pointers */

    switch (opt.chksum) {
    case CHKSUM_HW:
        in_chksum = _hw_chksum;
        break;
    case CHKSUM_SW:
        in_chksum = _sw_chksum;
        break;
    default:
        fatal(__INTERNAL__);
    }


    /*
     * update_link: src and/or dst mac address with random bytes
     */

    if ( opt.rand_mac_src || opt.rand_mac_dst )
        update_link = _update_link;

    /*
     * update_host: src and/or dst ip address are filled with radom bytes
     */

    if ( opt.rand_host_src >= 0 || opt.rand_host_dst >=0 || opt.eui64 ) {

        if (af_family == AF_INET6) 
            update_host = ( opt.eui64 ? _update_host6_eui64 : _update_host6 );
        else
            update_host = _update_host4;

    }

    /* estimate the CPU frequency */

    if (Hz == 0) {
        if (estimate_clock_time)
            msg(MSG_DIR "estimate CPU pulse-per-second in %d sec... wait please\n",estimate_clock_time);
        Hz = get_cpu_hz(estimate_clock_time);
    }

    msg(MSG_INFO "cpu MHz: %lld.%.6lld %s\n", Hz / 1000000, Hz % 1000000, 	
        opt.use_clock ? "(given)" : (estimate_clock_time ? "(estimated)": "(/proc/cpuinfo)" ) );


    /* initialize the seed for the twister: init_genrand() */

    if (opt.urand)
        read(global.urand_fd,&init_rand_seed,sizeof(uint32_t));

    msg(MSG_INFO "init_rand_seed=0x%x (%s)\n",
        (uint32_t)init_rand_seed, (opt.urand ? "/dev/urand": "twister"));

    init_genrand(init_rand_seed);

    /* crono vector size */

    crono_vector_size  = (1<<crono_vector_depth);
    crono_vector_mask  = crono_vector_size-1;
    crono_vector = malloc(sizeof(int)* crono_vector_size);
    if (crono_vector == NULL)
        fatal("malloc(): crono_vector");

    msg( MSG_NULL "crono_vector_depth=%d bit (%d entries%s\n", crono_vector_depth, crono_vector_size, 
         ( crono_vector_depth == 1 ?  " default)" : ")" ));	

    /* parsing filedat */

    core_parser();

    /* ignore some signals */

    signal(SIGINT, thefunctionafter);

    siga.sa_flags = SA_SIGINFO;
    siga.sa_sigaction = sa_fault;

    sigaction(SIGILL, &siga,NULL);
    sigaction(SIGFPE, &siga,NULL);
    sigaction(SIGSEGV,&siga,NULL);
    sigaction(SIGBUS, &siga,NULL);

    /* create outgoing datalink socket */
    global.sout_fd = create_socket(ifout, src_mac, dst_mac, 1);

    /* create incoming datalink socket */
    if (ifin != NULL)
        global.sin_fd = create_socket (ifin,NULL,NULL, 0);


    if (opt.priority && opt.non_int) {
        int n = 9;
        for(; n > -1; n--, sleep(1)) 
            msg(MSG_INFO "WARNING: non-interrupt mode requested! Brute will start in %d second%c",n, n>0 ? '\r' : '\n');
    }

    /* set round-robin scheduling policy */
    if (opt.priority) {
        if (!opt.non_int) {
            int kevent = get_pid("events/0",0);
            if ( kevent != -1 ) {
                set_realtime(kevent,opt.priority);
            }
            else msg(MSG_INFO "events/0 kthread not found!\n");
        }
        set_realtime(0,opt.priority);
    }

    /* ipv6 banner */
    if (af_family == AF_INET6)
        msg( MSG_INFO "ipv6 support enabled...\n");

    /* write buffers to disk */
    sync();
    sync();
    sync();

    /* start the script processing */
    processor();

    return 0;
}	
