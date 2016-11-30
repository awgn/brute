/*
    $Id: fatal.c,v 1.4 2008-01-12 16:10:21 awgn Exp $

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

#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <sysexits.h>
#include <err.h>

#include <macro.h>

_unused static
const char cvsid[]= "$Id: fatal.c,v 1.4 2008-01-12 16:10:21 awgn Exp $";

void fatal(char *pattern,...) __attribute__((noreturn));
void
fatal(char *pattern,...)
{
    va_list ap;

    va_start(ap, pattern);

    if (errno)
        verr(EX_SOFTWARE,pattern,ap);
    else
        verrx(EX_SOFTWARE,pattern,ap);
}

