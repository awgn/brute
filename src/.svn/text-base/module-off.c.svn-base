/*
    $Id: module-off.c,v 1.14 2008-01-12 16:10:22 awgn Exp $

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

MODULE_CVS("$Id: module-off.c,v 1.14 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* command line */

struct mod_line {
	uint32_t AL(msec);
};

#define TOKEN_msec	0

/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_parser:       u_parser,
                h_engine:       u_engine,
                command:	"off",			
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     1,
                token_list:	{TOKEN(msec)},	
};


INIT_MODULE(module);


/*
 * handlers
 */
static void
u_engine(cycles_t* exit_time,cmdline_t *cmd )
{
    /* null banner */
    BANNER("");
    brute_wait_until(exit_time);		// sleep until the exit_time expires
}


static void
u_parser(int t, struct atom *v, cmdline_t *cmd )
{
    INIT_PARSER(cmd, struct mod_line,v);

    switch (t) {
    case TOKEN_msec:
        TAG(msec) = cast_ret(brute_eval_int,v);
        break;
    default:
        PARSER_ERROR(v);
    }

}

