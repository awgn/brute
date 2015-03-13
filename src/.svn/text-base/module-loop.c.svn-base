/*
    $Id: module-loop.c,v 1.17 2008-01-12 16:10:22 awgn Exp $
 
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


MODULE_CVS("$Id: module-loop.c,v 1.17 2008-01-12 16:10:22 awgn Exp $");
MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");

/* prototype */

FUNCTION_PARSER(u_parser);
FUNCTION_ENGINE(u_engine);

/* command arg line */
struct mod_line {
    uint32_t	AL(msec);
    uint32_t 	AL(counter);
    uint32_t	AL(start);
    uint32_t	AL(label);
    char *	AL(slabel);
};


/* module descriptor */

#define TOKEN_msec	0
#define TOKEN_counter	1	
#define TOKEN_start	2
#define TOKEN_label	3		

static
struct module_descriptor module = {
h_parser:       u_parser,
                h_engine:       u_engine,
                command:  	"loop",                 			
                author:         "Bonelli Nicola <bonelli@netserv.iet.unipi.it>",
                token_nelm:     4,
                token_list:    	{ TOKEN(msec),TOKEN(counter), TOKEN(start), TOKEN(label) },    
};


INIT_MODULE(module);


/*
 * handlers
 */


static 
void
u_engine(cycles_t* exit_time, cmdline_t *cmd )
{
    struct mod_line *p =  (struct mod_line *)cmd->opaque;
    int i;

    BANNER("->%s #%d",p->slabel,p->counter);

    if ( (i=find_label(p->slabel))==-1)
        fatal("loop: error, label \"%s\" not found",p->slabel);	

    /* force the global ip */

    if (--(p->counter)) 
        global.jmp = i;

    if ( p->counter ==0)
        p->counter = p->start;

    brute_wait_until(exit_time);
}


static 
void
u_parser(int t, struct atom *v, cmdline_t *cmd )
{
    INIT_PARSER(cmd, struct mod_line, v);

    switch (t) {
    case TOKEN_msec:
        TAG(msec) = cast_ret(brute_eval_int,v);
        break;
    case TOKEN_counter:
        TAG(counter)= cast_ret(brute_eval_atom,v);
        TAG(start)= cast_ret(brute_eval_atom,v);
        break;
    case TOKEN_label:
        TAG(slabel)=cast_ret(strdup,v->rvalue);
        TAG(label) =cast_ret(hash,  v->rvalue,strlen(v->rvalue));
        break;
    default:
        PARSER_ERROR(v);
    }

}

