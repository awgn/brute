/*
    $Id: engine.c,v 1.20 2008-01-12 16:10:21 awgn Exp $

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
#include <string.h>
#include <errno.h>

#include <global.h>
#include <prototype.h>

_unused static
const char cvsid[]= "$Id: engine.c,v 1.20 2008-01-12 16:10:21 awgn Exp $";


/*
 * search a label, return its index ( -1 if not found )
 */
int
find_label(const char *lb)
{
    u_int i, h;

    if (lb == NULL)
        return (-1);
    h = hash((void *)lb, strlen(lb));

    for (i = 0; i < global.max_line; i++) {
        if (cmdline[i].label == h &&
            strcmp(cmdline[i].s_label, lb) == 0)
            return i;
    }
    return -1;
}


/*
 * the script processor
 */
void
processor()
{
    cycles_t init, exit_time;

    msg(MSG_DIR "engine started!\n");

    for (global.ip = 0; global.ip < global.max_line;) {

        /* check for static integers */
        if ( (opaque_opcode(cmdline[global.ip].msec) & OC_CLASS_MASK) == OC_RTIME )
            fatal("\"msec\" tag error (static integer expected)! brute_eval_int method suggested for this token.");

        xprintf(stderr, (cmdline[global.ip].label ? "%s:\t" : "\t"), cmdline[global.ip].s_label);
        xprintf(stderr, "%3d: %s", global.ip + 1, modules[cmdline[global.ip].comm]->command);
        xprintf(stderr, (*cmdline[global.ip].msec ? " (%d msec)" : ""), *cmdline[global.ip].msec);

        /* compute the handler 's exit_time */
        init=get_cycles();

        exit_time = ((cmdline[global.ip].msec != NULL && *(cmdline[global.ip].msec) != 0) ?
                     init + (cycles_t) (Hz) / 1000 * (*(cmdline[global.ip].msec)) : \
                     init);

        /* exec the u_engine handler, providing the exit_time and the command line pointer */
        modules[cmdline[global.ip].comm]->h_engine(&exit_time, &cmdline[global.ip]);

        /* fix the instruction pointer (module_loop helper) */
        global.ip  = (global.jmp != -1 ? global.jmp : global.ip+1);
        global.jmp = -1;

    }
}
