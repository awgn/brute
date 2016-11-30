/*
    $Id: debug.c,v 1.4 2008-01-12 16:10:21 awgn Exp $

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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include <global.h>
#include <prototype.h>


_unused static
const char cvsid[]= "$Id: debug.c,v 1.4 2008-01-12 16:10:21 awgn Exp $";


static char format_module[]= "\
    A fatal error occurred while running \"%s\" module. \n\
    Please send back this report to %s.\n";


#define std(f,arg...)   fprintf(stderr,f, ##arg)

#define INFO_ADDR(i,x)  std("    si_%s= %p\n", #x,(void *)i->si_##x)
#define INFO_INT(i,x)   std("    si_%s= %d\n", #x,(int)i->si_##x)
#define INFO_CODE(x,c)  std("    si_code(%d)= %s\n", c,x[c])
#define INFO_HEX(x)     std("    %s= %#x\n", #x,(uint32_t)x)
#define DUMP_REG(scp,x) std("    %-4s %-#*x %d\n",(#x)+4,12,\
                                ((ucontext_t *)scp)->uc_mcontext.gregs[x],\
                                ((ucontext_t *)scp)->uc_mcontext.gregs[x])

#define ILL_(x,y)    [ ILL_##x]  "ILL_" #x "(" y ")"
#define FPE_(x,y)    [ FPE_##x]  "FPE_" #x "(" y ")"
#define BUS_(x,y)    [ BUS_##x]  "BUS_" #x "(" y ")"
#define SEGV_(x,y)   [SEGV_##x] "SEGV_" #x "(" y ")"

static char *ill_code[12] = {
        ILL_(ILLOPC, "illegal opcode"),
        ILL_(ILLOPN, "illegal operand"),
        ILL_(ILLADR, "illegal addressing mode"),
        ILL_(ILLTRP, "illegal trap"),
        ILL_(PRVOPC, "privileged opcode"),
        ILL_(PRVREG, "privileged register"),
        ILL_(COPROC, "co-processor"),
        ILL_(BADSTK, "bad stack"),
};

static char *fpe_code[12] = {
        FPE_(INTDIV, "integer divide by zero"),
        FPE_(INTOVF, "integer overflow"),
        FPE_(FLTDIV, "floating point divide by zero"),
        FPE_(FLTOVF, "floating point overflow"),
        FPE_(FLTUND, "floating point underflow"),
        FPE_(FLTRES, "floating point inexact result"),
        FPE_(FLTINV, "invalid floating point operation"),
        FPE_(FLTSUB, "subscript out of range"),
};

static char *segv_code[4] = {
        SEGV_(MAPERR, "address not mapped to object"),
        SEGV_(ACCERR, "invalid permissions"),
};

static char *bus_code[4] = {
        BUS_(ADRALN, "invalid address alignment"),
        BUS_(ADRERR, "non-existent physical address"),
        BUS_(OBJERR, "object specific hardware error"),
};


void
sa_fault (int sig, siginfo_t *si, void *scp)
{
	fprintf(stderr,"[!] FATAL ERROR! Catched signal -->(%s)<--\n",strsignal(sig));

	fprintf(stderr, "[i] Signal information:\n");
        INFO_INT(si,signo);
        INFO_INT(si,errno);

	switch (sig) {
		case SIGILL:
			INFO_ADDR(si,addr);
			INFO_CODE(ill_code, si->si_code);
			break;
		case SIGFPE:
			INFO_ADDR(si,addr);
			INFO_CODE(fpe_code, si->si_code);
			break;
		case SIGSEGV:
			INFO_ADDR(si,addr);
			INFO_CODE(segv_code, si->si_code);
			break;
		case SIGBUS:
			INFO_ADDR(si,addr);
			INFO_CODE(bus_code, si->si_code);
			break;
	}

#if #cpu (i386)
	std("[i] Context:\n");
	DUMP_REG(scp,REG_EAX);
	DUMP_REG(scp,REG_ECX);
	DUMP_REG(scp,REG_EDX);
	DUMP_REG(scp,REG_EBX);
	DUMP_REG(scp,REG_ESP);
	DUMP_REG(scp,REG_EBP);
	DUMP_REG(scp,REG_ESI);
	DUMP_REG(scp,REG_EDI);
	DUMP_REG(scp,REG_EIP);
	DUMP_REG(scp,REG_EFL);
	DUMP_REG(scp,REG_CS);
	DUMP_REG(scp,REG_SS);
	DUMP_REG(scp,REG_DS);
	DUMP_REG(scp,REG_ES);
	DUMP_REG(scp,REG_FS);
	DUMP_REG(scp,REG_GS);
#endif

        std(format_module,
            modules[cmdline[global.ip].comm]->command ,
            modules[cmdline[global.ip].comm]->author );

	abort();

}


