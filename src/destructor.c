/*
    $Id: destructor.c,v 1.14 2008-01-12 16:10:21 awgn Exp $

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
#include <stdio.h>
#include <unistd.h>

#include <global.h>
#include <prototype.h>

_unused static
const char cvsid[]= "$Id: destructor.c,v 1.14 2008-01-12 16:10:21 awgn Exp $";

/*
 * destructor function
 */
void thefunctionafter(int) __attribute__((destructor));
void
thefunctionafter(int v)
{
    static int i;
    int j;

    if (i++)
        exit (0);

    if (crono_index) {
        for (j=0;j<crono_index;j++)
            fprintf(stderr,"%u\n",crono_vector[j]);
    }

    if (global.sin_fd != 0) {
        iface_extprom(global.sin_fd,ifin);
        close(global.sin_fd);
    }

    if (global.sout_fd != 0) {
        close(global.sout_fd);
        msg(MSG_DIR "%llu sendto() done.\n",global._sent);
    }

    if (opt.priority && !opt.non_int) {
        int kevent = get_pid("events/0",0);
        if ( kevent != -1 ) {
            unset_realtime(kevent);
        }
        else msg(MSG_INFO "events/0 kthread not found!\n");
    }
    exit(0);
}

