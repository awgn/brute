/*
    $Id: scheduler.c,v 1.11 2008-06-08 08:16:40 awgn Exp $

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
#include <sys/types.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

#include <macro.h>


_unused static
const char cvsid[]= "$Id: scheduler.c,v 1.11 2008-06-08 08:16:40 awgn Exp $";

/*
 * set realtime round robin scheduling policy
 */
void
set_realtime(int pid, int prio)
{
    struct sched_param sp;
    msg(MSG_DIR "pid=%d: round-robin scheduling with priority %d.\n", pid ? : getpid(), prio);
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = MIN(prio, sched_get_priority_max(SCHED_RR));
    sched_setscheduler(pid, SCHED_RR, &sp);
}

void
unset_realtime(int pid)
{
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    msg(MSG_DIR "pid=%d: default scheduling (SCHED_OTHER)\n", pid);
    sched_setscheduler(pid, SCHED_OTHER, &sp);
}

/* 
 * given a process/kernel thread name, it returns its pid or -1
 * if not found.
 */

static int
pid_filter(const struct dirent *d)
{
    if (atoi(d->d_name))
        return 1;
    return 0;
}

static int pid_sort(const void *a, const void *b)
{
    struct dirent **dir_a = (struct dirent **)a;
    struct dirent **dir_b = (struct dirent **)b;

    unsigned int a_pid = atoi((*dir_a)->d_name);
    unsigned int b_pid = atoi((*dir_b)->d_name);

    return a_pid-b_pid;
}


pid_t
get_pid(const char *cmd, pid_t start)
{
    char status[80], *line = NULL;
    struct dirent **filelist;
    size_t size = 0;
    FILE *f;
    int i, n;

    pid_t pid = -1;
    n = scandir("/proc", &filelist, pid_filter , pid_sort);
    for(i=0; i<n; i++) {
        pid_t p = atoi(filelist[i]->d_name);

        if ( p <= start )
            continue;

        if ( sprintf(status,"/proc/%d/status", p) < 0 )
            continue;
        f = fopen(status,"r");
        if ( f == NULL )
            continue;

        if ( getline(&line, &size, f) < 0 )
            goto next;

        if ( sscanf(line,"Name: %s", status) != 1 )
            goto next;

        if (!strcmp(status,cmd)) {
            fclose(f);
            pid = p;
            break;
        }
next:
        fclose(f);
    }

    free(line);
    // free filelist...
    for(i=0; i<n; i++) 
        free(filelist[i]);
    free(filelist); 
    return pid;
}



