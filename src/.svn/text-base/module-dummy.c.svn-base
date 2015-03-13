/*
    $Id: module-dummy.c,v 1.13 2008-01-12 16:10:22 awgn Exp $

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

MODULE_CVS("$Id: module-dummy.c,v 1.13 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* 
 * Adding a new token to the "dummy" module:
 *
 * 1) add the "new" tag to the mod_line structure, choosing the preferred type.
 * 2) define the macro TOKEN_new assigning the first integer available.
 * 3) add the TOKEN(new) to the token_list tag of the module_descriptor.
 * 4) add the CASE(new_token) to the function devoted to parse the arguments of the command. 
 * 
 */


/* command line */
struct mod_line {
	uint32_t  AL(msec);	/* do not remove the msec tag */
	uint32_t  AL(integer);
	uint32_t  AL(automatic);
	double AL(real);
//	uint32_t AL(new);						<- (1)

};


#define TOKEN_msec	0
#define TOKEN_integer	1
#define TOKEN_automatic 2
#define TOKEN_real	3		
//#define TOKEN_new	4 ..					<-(2)	

/*
 * module descriptor
 */
static
struct module_descriptor module = {
h_parser:       u_parser,
                h_engine:       u_engine,
                command:	"dummy",			
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     4,
                token_list:	{TOKEN(msec), TOKEN(integer), TOKEN(automatic),TOKEN(real)/*, TOKEN(new)*/},// 	<-(3)	
};


INIT_MODULE(module);


/*
 * handlers
 */
static void
u_engine(cycles_t* exit_time,cmdline_t *cmd )
{
    struct mod_line *p;

    /* init the local engine handler, and the pointer addressing an hidden shared mod_line area */
    INIT_ENGINE(cmd,struct mod_line,p);

    BANNER("integer=%d automatic=%d real=%f\n",p->integer,p->automatic,p->real);

    brute_wait_until(exit_time);		
}


static void
u_parser(int t, struct atom *v, cmdline_t *cmd )
{
    INIT_PARSER(cmd, struct mod_line, v);

    switch (t) {
    case TOKEN_msec:
        TAG(msec)= cast_ret(brute_eval_int, v);
        break;
    case TOKEN_integer:
        TAG(integer)= cast_ret(brute_eval_int, v);
        break;
    case TOKEN_automatic:
        TAG(automatic)= cast_ret(brute_eval_atom, v);
        break;
    case TOKEN_real:
        TAG(real)= cast_ret(brute_eval_double, v);
        break;
        //	case TOKEN_new: 					<-(4)
        //		TAG(new) = cast_ret(function,arg1,arg2,...);
        //		break;
    default:
        PARSER_ERROR(v);
    }
}

