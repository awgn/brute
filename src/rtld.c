/*
    $Id: rtld.c,v 1.8 2008-01-12 16:10:23 awgn Exp $

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
#include <stdarg.h>
#include <sysexits.h>
#include <err.h>

#include <global.h>
#include <typedef.h>
#include <prototype.h>
#include <macro.h>


_unused static
const char cvsid[]= "$Id: rtld.c,v 1.8 2008-01-12 16:10:23 awgn Exp $";

/*
 * register a function or a global variable. the type field can be either
 * the number of arguments passed to the function, or the typeof var.
 */
void
register_obj(char *sym, void *addr, int type, enum symbol_type t)
{
	rtld_obj_t *set;
	int *index;
    int i;

    set = NULL;
    index = NULL;

	switch (t) {
	case sym_variable:
		index = &var_index;
		set = var_set;
		break;
	case sym_function:
		index = &fun_index;
		set = fun_set;
		break;
	default:
		fatal(__INTERNAL__);
	}

	if (*index >= MAX_OBJ)
		fatal(__INTERNAL__);

	/* check for duplicate symbols */
	for (i = 0; i < *index; i++)
		if (strcmp(set[i].sym, sym) == 0)
			break;
	if (i != *index)
		fatal("duplicate symbol `%s'@%p and `%s'@%p", sym, addr, set[i].sym, set[i].addr);

	set[*index].sym = strdup(sym);
	set[*index].addr = addr;
	set[*index].type = type;

    (*index)++;
}


/*
 * given a symbol and the expected type, it returns its 
 * address (NULL if not found).
 */
void *
search_sym(char *sym, int *type, enum symbol_type t)
{
	rtld_obj_t *set = NULL;
	int i;

	switch (t) {
	case sym_variable:
		set = var_set;
		break;
	case sym_function:
		set = fun_set;
		break;
	default:
		fatal(__INTERNAL__);
	}

	for (i = 0; i < MAX_OBJ && set[i].sym != NULL; i++) {
		if (strcmp(sym, set[i].sym) == 0)
			break;
	}

	*type = set[i].type;
	return set[i].addr;
}


/* 
 * The update_mod_line() function updates the struct mod_line (dst)
 * with the struct mod_line (src), at run time.
 */

#define opaque_assign_static(bit, dst, oper, src) \
if ((opaque_opcode(src) & OC_TYPE_MASK) == OC_DOUBLE) { \
    *(double *)dst oper *(double *)src; \
    continue; \
} else \
if (((opaque_opcode(src) & OC_TYPE_MASK)==OC_INT) && (opaque_sizeof(src)== (bit>>3) )) { \
    *(u_int##bit##_t *)dst oper *(u_int##bit##_t *)src; \
    continue; \
} else \
if (((opaque_opcode(src) & OC_TYPE_MASK)==OC_HOST) && (opaque_sizeof(src)==sizeof(void *))) { \
    *(struct hostent *)dst = *(struct hostent *)src; \
    continue; \
} else \
if (((opaque_opcode(src) & OC_TYPE_MASK)==OC_ADDR) && (opaque_sizeof(src)==sizeof(void *))) { \
    *(char *)dst = *(char *)src; \
    continue; \
}

#define opaque_assign_dynamic(bit, dst, oper, src, dyn) \
if (opaque_sizeof(src)==(bit>>3)) { \
    *(u_int##bit##_t *)dst oper (u_int##bit##_t )dyn; \
    continue; \
}

void
update_mod_line(void *src, void *dst, size_t n)
{
    char *s, *d;
    int i;

    for (s = src, d = dst, i=0 ; n != 0; i++, n -= DEFAULT_ALIGN, s += DEFAULT_ALIGN, d += DEFAULT_ALIGN) {

        DEBUG("entry(%d): CLASS=0x%x OPER=0x%x TYPE=0x%x\n", i,
              opaque_opcode(s) & OC_CLASS_MASK,
              opaque_opcode(s) & OC_OPER_MASK,
              opaque_opcode(s) & OC_TYPE_MASK);

        if ((opaque_opcode(s) & OC_CLASS_MASK) == 0)			        /* empty */
            continue;  

        if ((opaque_opcode(s) & OC_CLASS_MASK) == OC_PTIME) {        /* static */

            switch (opaque_opcode(s) & OC_OPER_MASK) {
            case OC_EQ:
                opaque_assign_static(32, d, =, s);
                opaque_assign_static(16, d, =, s);
                opaque_assign_static( 8, d, =, s);
            case OC_PLEQ:
                opaque_assign_static(32, d, +=, s);
                opaque_assign_static(16, d, +=, s);
                opaque_assign_static( 8, d, +=, s);
            case OC_MNEQ:
                opaque_assign_static(32, d, -=, s);
                opaque_assign_static(16, d, -=, s);
                opaque_assign_static( 8, d, -=, s);
            default:
                fatal(__INTERNAL__);
            }
        }

        if ((opaque_opcode(s) & OC_CLASS_MASK) == OC_RTIME) {        /* dynamic */
            struct atom a;
            u_int32_t dyn;

            /* setup local atom */               
            a.lvalue = NULL; 
            a.rvalue = opaque_stcarg(char *,s);
            a.opcode = opaque_opcode(s);
            a.value  = 0;

            /* evaluate the rvalue */
            dyn = eval_atom_dynamic (&a);

            switch (opaque_opcode(s) & OC_OPER_MASK) {
            case OC_EQ:
                opaque_assign_dynamic(32, d, =, s, dyn);
                opaque_assign_dynamic(16, d, =, s, dyn);
                opaque_assign_dynamic( 8, d, =, s, dyn);
            case OC_PLEQ:
                opaque_assign_dynamic(32, d, +=, s, dyn);
                opaque_assign_dynamic(16, d, +=, s, dyn);
                opaque_assign_dynamic( 8, d, +=, s, dyn);
            case OC_MNEQ:
                opaque_assign_dynamic(32, d, -=, s, dyn);
                opaque_assign_dynamic(16, d, -=, s, dyn);
                opaque_assign_dynamic( 8, d, -=, s, dyn);
            default:
                fatal(__INTERNAL__);

            }
        }

        /* NOTREACHED */
        fatal(__INTERNAL__);
    }
}

