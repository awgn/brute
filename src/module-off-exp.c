/*
    $Id: module-off-exp.c,v 1.2 2008-01-12 16:10:22 awgn Exp $

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

#include <t-module.h>

MODULE_CVS("$Id: module-off-exp.c,v 1.2 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* command line */

struct mod_line {
    uint32_t 		AL(msec);
    double          AL(lambda);
};

#define TOKEN_msec	0
#define TOKEN_lambda	1

/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_parser:       u_parser,
                h_engine:       u_engine,
                command:	"off-exp",
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     2,
                token_list:	{TOKEN(msec), TOKEN(lambda)},
};


INIT_MODULE(module);


/*
 * handlers
 */
static void
u_engine(cycles_t* exit_time,cmdline_t *cmd )
{
    struct mod_line *p;
    cycles_t my_exit_time;
    double off;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line, p);

    off = exponential(1/(p->lambda));

    /* null banner */
    BANNER("{lambda=%f sec, duration=%f sec}",p->lambda,off);

    my_exit_time = get_cycles()+ (cycles_t)(Hz*off);
    brute_wait_until(&my_exit_time);		// sleep until the exit_time expires

}


static void
u_parser(int t, struct atom *v, cmdline_t *cmd )
{
    INIT_PARSER(cmd, struct mod_line,v);

    switch (t) {
    case TOKEN_msec:
        TAG(msec) = cast_ret(brute_eval_int,v);
        break;
    case TOKEN_lambda:
        TAG(lambda) = cast_ret(brute_eval_double,v);
        break;
    default:
        PARSER_ERROR(v);
    }
}

