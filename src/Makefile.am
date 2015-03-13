sbin_PROGRAMS = brute

brute_SOURCES = brute.c  	\
		checksum.c    	\
		brute-api.c	\
		fatal.c  	\
		proc-parser.c  	\
		socket.c 	\
		engine.c	\
		fnv.c		\
		destructor.c  	\
		parser.c  	\
		netdev.c	\
		scheduler.c	\
		perf-hash.c	\
		signal.c	\
		rtld.c		\
		mt19937ar.c	\
		debug.c		\
		module.c	\
		module-dummy.c  \
		module-loop.c   \
		module-off.c    \
		module-off-exp.c\
		module-rtld.c   \
                module-cbr.c    \
		module-multicbr.c \
		module-cbr-exp.c\
		module-cidt.c   \
                module-poisson.c\
		module-pab.c	\
		module-rtcp.c	\
		module-rtcp-loop.c \
		module-trimodal.c

brute_LDADD= @LIBOBJS@

KERNEL_RELEASE=$(shell uname -r)
KERNEL_PATH=/usr/src/linux-$(KERNEL_RELEASE)
INCLUDES=-I$(KERNEL_PATH)/include -I.

AM_CFLAGS   = @opt_options@ -g -Wall -rdynamic  # -DEBUG
AM_CPPFLAGS = -D_GNU_SOURCE -DPLG_PATH=\"$(datadir)/brute/\" @CPPFLAGS@

CLEANFILES = *.log *.out core.* *.o *~

