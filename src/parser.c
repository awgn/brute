/*
    $Id: parser.c,v 1.17 2008-01-12 16:10:23 awgn Exp $

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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <global.h>
#include <prototype.h>
#include <typedef.h>

_unused static
const char cvsid[]= "$Id: parser.c,v 1.17 2008-01-12 16:10:23 awgn Exp $";


/*
 * fatal parse error
 */
void
parser_error(int i, char *t)
{
    switch (i) {
    case PERROR:
        fatal("%s: parse error at line %d", file_name, file_line);
    case PECOMM:
        fatal("%s: unknown command \"%s\" at line %d", file_name, t, file_line);
    case PETOK:
        fatal("%s: \"%s\" unknown token at line %d", file_name, t, file_line);
    case PEITOK:
        fatal("%s: \"%s\" an invalid token at line %d", file_name, t, file_line);
    case PETOOFEW:
        fatal("%s: too few arguments at line %d", file_name, file_line);
    default:
        fatal(__INTERNAL__);
    }
}


/*
 * update the given pointer to the first char not in set.
 */
char *
remove_leading_set(char **s, char *set)
{
    while (*s != NULL && **s != '\0' && strchr(set, **s) != NULL)
        (*s)++;

    return *s;
}


/*
 * recursive evaluation of r-value tag
 */
union paret
par_eval_rvalue(char *s, enum eval_mode mode, int *_ret)
{
    char *bf;
    int id;
    union paret ret;

    DEBUG("----> rvalue:\"%s\"\n", s);
    if (s == NULL || *s == '\0')
        fatal("parse error: NULL token");

    bf = strdupa(s);

    DEBUG("\t*** remove leading spaces ***\n");
    /*** remove leading spaces ***/
    {
        int i;
        for (i = 0; bf[i] != '\0' && isspace(bf[i]); i++);
        if (i > 0)
            return par_eval_rvalue(&bf[i],mode,_ret);
    }

    DEBUG("\t*** remove ending spaces ***\n");
    /*** remove ending spaces ***/
    {
        int i;
        for (i = (strlen(bf) - 1); i > -1 && isspace(bf[i]); i--)
            bf[i] = '\0';
    }

    DEBUG("\t*** evaluate brackets ***\n");
    /*** evaluate brackets ***/
    {
        int i, b;
        for (i = 0, b = 0; bf[i] != '\0'; i++)
            switch (bf[i]) {
            case '(':
                b++;
                rm_close_bracket(&bf[i]);
                continue;
            case ')':
                fatal("parse error: %s", bf);
            default:
                goto ret_b;
            }
ret_b:
        if (b > 0)
            return par_eval_rvalue(&bf[i],mode,_ret);
    }


    /* evaluating objects...
     */

    switch(mode) {
    case eval_rvalue_double:
        DEBUG("\t*** evaluate double  ***\n");
        {
            char *endptr;
            ret = (union paret)strtod(bf, &endptr);
            if (*endptr == '\0') {
                *_ret = eval_rvalue_double;
                return ret;
            }

            /* error */
            *_ret = eval_rvalue_error;
            return (union paret)0L;
        }
    case eval_rvalue_addr:
        DEBUG("\t*** evaluate addr ***\n");
        *_ret = eval_rvalue_addr;
        return (union paret)(void *)strdup(bf);

    case eval_rvalue_host:
        DEBUG("\t*** evaluate host ***\n");
        {
            struct hostent *h;
            h=brute_gethostbyname(bf);
            *_ret = eval_rvalue_host;
            return (union paret)h;
        }
    default:
        break;
    }


    /* the following methods, eval_rvalue_static eval_rvalue_dynamic
     * and eval_rvalue_int are designed to evaluate int
     * objects only (also vars and function returning integers).
     */

    DEBUG("\t*** evaluate integer  ***\n");
    /*** evaluate integer ***/
    {
        char *endptr;

        ret = (union paret)strtol(bf, &endptr, 0);
        if (*endptr == '\0') {
            *_ret = eval_rvalue_int;
            return ret;
        }
    }


    /***
     *** stop recursing in case the method is not eval_rvalue_dynamic
     ***/

    if ( mode != eval_rvalue_dynamic ) {
        *_ret = eval_rvalue_error;
        return (union paret)0L;
    }


    DEBUG("\t*** evaluate variable (int) ***\n");
    /*** eval variable (int) ***/
    {
        int *_int;
        _int = search_sym(bf, &id, sym_variable);
        if (_int != NULL) {
            *_ret = eval_rvalue_var;
            return (union paret)*_int;
        }
    }

    DEBUG("\t*** evaluate function (returning int) ***\n");
    /*** eval function (int) ***/
    {
        int (*handler) (vargs_t *);
        int i,state;
        char *ps, *pe, *atom;
        vargs_t *ar;

        pe = ps = bf;

        /* extract symbol */
        for (state = rval_lspace; (state < rval_symbol ? *ps != '\0' : *pe != '\0');)
            switch (state) {
            case rval_lspace:
                if (!isspace(*ps)) {
                    pe = ps;
                    state++;
                } else
                    ps++;
                break;
            case rval_symbol:
                switch (*pe) {
                case '(':
                    *pe++ = '\0';
                    rm_close_bracket(pe);
                    goto symb;
                case ')':
                    fatal("parse error: %s", ps);
                case ' ':
                case '\t':
                    *pe++ = '\0';
                    state++;
                    break;
                default:
                    pe++;
                }
                break;
            case rval_rspace:
                switch (*pe) {
                case '(':
                    *pe++ = '\0';
                    rm_close_bracket(pe);
                    goto symb;
                case ' ':
                case '\t':
                    *pe++ = '\0';
                    break;
                default:
                    fatal("error: `%s' undeclared (first use in this identifier)", ps);
                }
            }
symb:
        /* ti ho modificato qualcosa, controlla (peppe) */
        if ((handler = search_sym(bf, &id, sym_function)) == NULL)
            fatal("error: undefined reference to `%s'", ps);

        /* symbol found */
        ar = (vargs_t *) alloca(sizeof(vargs_t) + sizeof(int) * id);
        ar->nmem = 0;
        ps = pe;

        for (i = 0; i < id; i++) {
            DEBUG("\tpar_get_atom(): \"%s\"\n",ps);
            if ((atom = par_get_atom(&ps, ',')) == NULL)
                fatal("error: too few arguments to function `%s'", bf);
            DEBUG("\tATOM: \"%s\", %s\n", atom, ps);

            ar->args[i] = par_eval_rvalue(atom,mode,_ret)._int;
            ar->nmem++;
        }

        DEBUG("\tREST->{%s}\n", ps);
        if (*ps != '\0')
            fatal("error: too many arguments to function `%s'", bf);

        *_ret = eval_rvalue_func;
        return (union paret)handler(ar);
    }

    fatal(__INTERNAL__);
    return (union paret)0;
}


/*
 * given an atom parse it into the proper structure
 */
struct atom
par_parse_atom(const char *p)
{
    struct atom ret;
    enum parse_atom_state state;
    int i, l, r;
    char *tmp;

    /* initialize values */
    l = 0;
    r = 0;
    ret.lvalue = NULL;
    ret.rvalue = NULL;
    ret.value  = 0;
    ret.opcode = OC_EQ;	/* default operand type '=' */

    /* make a copy of the char *p */
    tmp = strdupa(p);

    /* set the space to the first available */
    state = eval_llspace;

    for (i = 0; tmp[i] != '\0';) {
        switch (state) {
        case eval_llspace:
            state_$(isspace(tmp[i]),
                    eval_llspace, l++; break, /* set the new state, process the next char */
                    eval_lvalue, continue);    /* set the new state and continue processing the current char */
        case eval_lvalue:
            state_$(isgtoken(tmp[i]),
                    eval_lvalue, break,
                    eval_lespace, continue);
        case eval_lespace:
            state_$(isspace(tmp[i]),
                    eval_lespace, tmp[i] = '\0'; break,
                    eval_plus_sep, continue);
        case eval_plus_sep:
            state_$(tmp[i] == '+',
                    eval_sep, tmp[i] = '\0'; ret.opcode = OC_PLEQ; break, /* operand type: += */
                    eval_min_sep, continue);
        case eval_min_sep:
            state_$(tmp[i] == '-',
                    eval_sep, tmp[i] = '\0'; ret.opcode = OC_MNEQ; break, /* operand type: -= */
                    eval_sep, continue);
        case eval_sep:
            state_$(tmp[i] == '=',
                    eval_rlspace, tmp[i] = '\0'; break,
                    0, goto ret_err);				/* goto ret_err label */
        case eval_rlspace:
            state_$(isspace(tmp[i]),
                    eval_rlspace, break,
                    eval_rvalue, r = i; continue);
        case eval_rvalue:
            state_$(!isspace(tmp[i]),
                    eval_rvalue, break,
                    eval_respace, continue);
        case eval_respace:
            state_$(isspace(tmp[i]),
                    eval_respace, tmp[i]='\0'; break,
                    0, goto ret_err);
        default:
            fatal(__INTERNAL__);
        }

        /* break: process the next char */
        i++;
    }

    ret.lvalue = (tmp[l] ? strdup(&tmp[l]) : NULL);
    ret.rvalue = ((r && tmp[r]) ? strdup(&tmp[r]) : NULL);

    /* evaluate the r-value token, if it represents a static value */
    eval_atom_static(&ret);
    return ret;

ret_err:
    fatal("parse error near atom `%s'", p);
    return ret;
}


/*
 * return the next atom ("sep" separated chunks). The buffer is left unchaged,
 * the given s pointer updated.
 */

char *
par_get_atom(char **p, char sep)
{
    enum par_atom_state state;
    char *tmp;
    int i,l,r;
    int bracket;

    /* initialize */
    bracket= 0;
    state= atom_lspace;
    tmp= strdupa(*p);

    for(i=0,l=0,r=0; tmp[i]!='\0' && state!=atom_done ; ) {
        switch(state) {
        case atom_lspace:
            state_$(isspace(tmp[i]),
                    atom_lspace, l++; break,  /* process the next char */
                    atom_body, continue);     /* continue processing the current char */

        case atom_body:
            if (tmp[i]=='(') {
                bracket++;
                break;
            }

            if (tmp[i]==')') {
                bracket--;
                break;
            }

            state_$(bracket==0 && tmp[i]==sep,
                    atom_done, tmp[i]='\0'; break,
                    atom_body, break);

        case atom_done:
            break;
        }
        /* break */
        r=++i;
    }

    DEBUG("\tbracket=%d, state=%d, r=%d, L[%s]:R[%s]\n",bracket,state,r,*p,(tmp+l));

    if ( bracket != 0 || ( state==atom_body && tmp[i]!='\0') )
        goto ret_err;

    (*p) += r;
    return *(tmp+l)== 0 ? NULL: strdup(tmp+l);

ret_err:
    fatal("parse error near atom `%s'", tmp);
    return NULL;
}


/*
 * return the command. The buffer is left unchanged,
 * the given pointer updated.
 */

char *
par_get_command(char **p)
{
    enum par_command_state state;
    char *tmp;
    int i,l,r;

    /* initialize */
    state= comm_lspace;
    tmp= strdupa(*p);

    for(i=0,l=0,r=0; tmp[i]!=0 && state!=comm_done ; ) {
        switch(state) {
        case comm_lspace:
            state_$(isspace(tmp[i]),
                    comm_lspace, l++; break,    /* process the next char */
                    comm_symbol, continue)      /* continue processing the current char */
        case comm_symbol:
            state_$(!isblank(tmp[i]),
                    comm_symbol, break,         /* process the next char */
                    comm_rspace, continue)      /* continue processing the current char */
        case comm_rspace:
            state_$(isspace(tmp[i]),
                    comm_rspace, tmp[i]='\0'; r=i+1; break,
                    comm_done, continue)
        case comm_done:
            break;

        }
        /* break */
        i++;
    }

    (*p) += r;
    return strdup(tmp+l);
}


/*
 * return the label if present, NULL otherwise. The buffer
 * is left unchanged, the given pointer updated.
 */
char *
par_get_label(char **p)
{
    enum par_label_state state;
    char *tmp;
    int i,l,r;

    /* initialize */
    state= lab_lspace;
    tmp= strdupa(*p);

    for(i=0,l=0,r=0; tmp[i]!=0 && state!=lab_done ; ) {
        switch(state) {
        case lab_lspace:
            state_$(isspace(tmp[i]),
                    lab_lspace, l++; break,            /* process the next char */
                    lab_symbol, continue)                /* continue processing the current char */
        case lab_symbol:
            state_$(isalpha(tmp[i]),
                    lab_symbol, break,
                    lab_colom, continue)
        case lab_colom:
            state_$(tmp[i]==':',
                    lab_rspace, tmp[i]='\0';r=i+1; break,
                    0, goto no_label)
        case lab_rspace:
            state_$(isspace(tmp[i]),
                    lab_rspace, tmp[i]='\0'; r++; break,
                    lab_done, continue)
        case lab_done:
            break;
        }
        /* break */
        i++;
    }

    if ( state < lab_colom )
        goto no_label;
    (*p) += r;
    return strdup(tmp+l);
no_label:
    (*p) += l;
    return NULL;
}


/*
 * given the atom, it evaluates the r-value, only if static.
 * It also sets the opcode tag accordingly. (CLASS and OPERAND)
 * FIXME: the eval_rvalue_static method support only integers
 */
union paret
eval_atom_static(struct atom *s)
{
    union paret r; 		/* value returned by eval_rvalue() */
    int ret;

    memset(&r,0,sizeof(ret));
    r = par_eval_rvalue(s->rvalue,eval_rvalue_static, &ret);

    switch(ret) {
    case eval_rvalue_error:
        /* the r-value part of atom *is not* an integer. */
        s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
        s->opcode |= (OC_RTIME|OC_INT);  		        /* int to be evaluated at runtime */
        s->value = 0;
        break;
    case eval_rvalue_int:
        /* the r-value part of atom *is* an integer . */
        s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
        s->opcode |= (OC_PTIME|OC_INT);  		        /* int done at parser-time */
        s->value = r._int;
        break;
    default:
        fatal(__INTERNAL__);
    }

    return r;
}


/*
 * given the atom, it evaluates the r-value tag. This function is called
 * by the used-defined u_engine, at run-time.
 */
int
eval_atom_dynamic(struct atom *s)
{
    union paret r;                          /* value returned by eval_rvalue() */
    int ret  = 0;

    r = par_eval_rvalue(s->rvalue, eval_rvalue_dynamic ,&ret);

    return r._int;
}


/*
 * main parser
 */
void
core_parser()
{
    FILE *f;
    char *lab, *com, *atom;
    int ret_c, ret_t, ret_tt;

    /*
     * ret_c: command index, ret_t: global token index, ret_tt: token
     * index
     */

    msg(MSG_READ "input: open(%s)\n", file_name);

    f = fopen(file_name, "r");
    if (f == NULL)
        fatal("brute_parser(): can't open %s", file_name);

    /* reset instruction pointer */
    global.ip = 0;

    for (;;) {
        char *p = parse_buff;
        struct atom atom_s;

        if (global.ip == MAX_CMDLINE)
            break;

        if (fgets(parse_buff, PARSE_BUFFLEN, f) == NULL)	/* EOF */
            break;
        file_line++;
        remove_leading_set(&p, " \t");

        /* skip comments or black lines */
        if (p[0] == '#' || p[0] == '\0' || p[0] == '\n')
            continue;

        lab = par_get_label(&p);
        if (lab != NULL) {
            cmdline[global.ip].label = hash(lab, strlen(lab));
            cmdline[global.ip].s_label = strdup(lab);
        }
        DEBUG("label=(%s)\n", lab);

        com = par_get_command(&p);
        DEBUG("comm=(%s)\n", com);
        if (com == NULL)
            parser_error(PERROR, NULL);

        /* check command */
        ret_c = qsearch(com, &command_table);
        if (ret_c == -1)
            parser_error(PECOMM, com);

        /* store the command-id into the cmdline list */
        cmdline[global.ip].comm = ret_c;

        while ((atom = par_get_atom(&p, ';')) != NULL) {
            DEBUG("atom->{%s}\n", atom);

            /* get l- and r-value */
            atom_s = par_parse_atom(atom);
            if (atom_s.lvalue == NULL)
                fatal("NULL lvalue token");

            DEBUG("(%s) -%d- (%s)\n", atom_s.lvalue, atom_s.opcode, atom_s.rvalue);

            ret_t  = qsearch(atom_s.lvalue, &token_table);
            ret_tt = -1;
#if 0
            if (ret_t == -1)
                parser_error(PETOK, atom_s.lvalue);
#endif
            /* check whether it is a valid token or not */
            if ( ret_t != -1 && (ret_tt = spec_rel[ret_c][ret_t]) == -1) {
                parser_error(PEITOK, atom_s.lvalue);
            }

            /* exec the user defined parser */
            modules[ret_c]->h_parser(ret_tt, &atom_s, &cmdline[global.ip]);
        }

        global.ip++;
    }

    global.max_line = global.ip;
    global.ip = 0;		/* reset instruction pointer */

    msg(MSG_NULL "parsed %d lines.\n", global.max_line);
}


/*
 * ::::::::::::::::::::::::: parser API :::::::::::::::::::::::::
 */


/*
 * evaluate the r-value as integer, stop parsing otherwise.
 */
int
brute_eval_int(struct atom *s)
{
    union paret r;
    int ret=0;

    r = par_eval_rvalue(s->rvalue,eval_rvalue_int, &ret);

    if (ret == eval_rvalue_error)
        fatal("comm=%s, token %s=%s error! (integer expected).",
              modules[cmdline[global.ip].comm]->command,s->lvalue,s->rvalue);

    s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
    s->opcode |=  (OC_PTIME|OC_INT);                /* int done at parser-time */
    s->value = r._int;
    return r._int;
}


/*
 * evaluate the r-value as double, stop parsing otherwise.
 */
double
brute_eval_double(struct atom *s)
{
    union paret r;
    int ret=0;

    r = par_eval_rvalue(s->rvalue,eval_rvalue_double, &ret);

    if (ret == eval_rvalue_error)
        fatal("comm=%s, token %s=%s error! (double expected).",
              modules[cmdline[global.ip].comm]->command,s->lvalue,s->rvalue);

    s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
    s->opcode |=  (OC_PTIME|OC_DOUBLE);             /* double done at parser-time */
    return r._double;
}


/*
 * evaluate the r-value as addr, stop parsing otherwise.
 */
void *
brute_eval_addr(struct atom *s)
{
    union paret r;
    int ret=0;

    r = par_eval_rvalue(s->rvalue,eval_rvalue_addr, &ret);

    if (ret == eval_rvalue_error)
        fatal("comm=%s, token %s=%s error! (addr expected).",
              modules[cmdline[global.ip].comm]->command,s->lvalue,s->rvalue);

    s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
    s->opcode |=  (OC_PTIME|OC_ADDR);               /* addr at parser-time */
    return r._addr;
}


/*
 * evaluate the r-value as host, stop parsing otherwise.
 */
void *
brute_eval_host(struct atom *s)
{
    union paret r;
    int ret=0;

    r = par_eval_rvalue(s->rvalue,eval_rvalue_host, &ret);

    if (ret == eval_rvalue_error)
        fatal("comm=%s, token %s=%s error! (host expected).",
              modules[cmdline[global.ip].comm]->command,s->lvalue,s->rvalue);

    s->opcode &= ~(OC_TYPE_MASK|OC_CLASS_MASK);     /* reset type and class */
    s->opcode |=  (OC_PTIME|OC_HOST);               /* host at parser-time */
    return r._host;
}


/*
 * used by modules at parser-time to evaluate atoms.
 * It returns the integer value, if static, the address of the
 * r-value token otherwise.
 */
long int
brute_eval_atom(struct atom *s)
{
    return ((s->opcode & OC_PTIME) ? s->value : (long int)s->rvalue );
}

