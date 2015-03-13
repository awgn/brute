/*
    $Id: module.c,v 1.11 2008-01-12 16:10:23 awgn Exp $

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

#include <sys/queue.h>
#include <t-module.h>
#include <global.h>

_unused static
const char cvsid[]= "$Id: module.c,v 1.11 2008-01-12 16:10:23 awgn Exp $";


static int lists_init;

struct entry {
    char *str;
    int id;
    TAILQ_ENTRY(entry) entries;
};


/*
 * count the number of elements stored in the tailq
 */
int
count_token(struct tqh * h)
{
    struct entry *np;
    int ret;

    for (ret = 0, np = h->tqh_first; np != NULL; np = np->entries.tqe_next, ret++);
    return ret;
}


/*
 * search a token in the tailq
 */
int
search_token(char *c, struct tqh * h)
{
    struct entry *np;
    int ret;

    for (ret = 0, np = h->tqh_first; np != NULL; np = np->entries.tqe_next, ret++) {
        if (strcmp(c, np->str) == 0)
            return ret;
    }
    return -1;
}


/*
 * checks for a module_descriptor
 */

#define CHKMOD_TAG(m,t)	do {								\
    if ((m)->t == 0)									\
    fatal("%s: in module \"%s\" tag ->%s: not set",__FUNCTION__,m->command,#t);	\
} while (0)


void
module_check(struct module_descriptor * m)
{
    /* check module_descriptor status */

    CHKMOD_TAG(m, command);
    CHKMOD_TAG(m, token_list);
    CHKMOD_TAG(m, h_engine);
    CHKMOD_TAG(m, h_engine);
    CHKMOD_TAG(m, h_engine);

    /* -- ok -- */
}


/*
 * the function is passed a struct module_descriptor in order to register a new module
 */
void
register_module(struct module_descriptor * m)
{
    struct entry *elem;
    static int i, j;
    int *spec_ptr, ret, h, k;

    /* initialize lists */
    if (lists_init++ == 0) {
        memset(spec_rel, -1, sizeof(4) * MAX_COMMAND * MAX_TOKEN);
        DEBUG("!lists initialized!\n");
        TAILQ_INIT(&head_commands);
        TAILQ_INIT(&head_tokens);
    }
    /* register command */
    DEBUG("[i] #%d command \"%s\"\n", j, m->command);
    elem = malloc(sizeof(struct entry));
    if (elem == NULL)
        fatal(__INTERNAL__);

    elem->str = strdup(m->command);
    elem->id = j;
    TAILQ_INSERT_TAIL(&head_commands, elem, entries);

    /* register tokens */
    for (h = 0, k = 0; h < m->token_nelm; k++) {

        DEBUG(" ? token-->(%s)\n", m->token_list[k]);

        /*
         * NULL ? np, just skip it fearless, going to face
         * segfaults ;-)
         */
        if (m->token_list[k] == NULL)
            continue;

        /* this is a valid token... */
        h++;

        /*
         * set specific relationship between command and its
         * tokens
         */
        ret = search_token(m->token_list[k], &head_tokens);
        spec_ptr = &spec_rel[j][(ret != -1 ? ret : i)];

        if (*spec_ptr != -1)
            fatal("%s(%s): token \"%s\" already in use", __FUNCTION__, m->command, m->token_list[k]);

        *spec_ptr = k;

        /* the current token has already been insert into the list */
        if (ret != -1)
            continue;

        /* this is a new token */
        if ((elem = malloc(sizeof(struct entry))) == NULL)
            fatal(__INTERNAL__);

        DEBUG("    token:%s\n", m->token_list[k]);
        elem->str = strdup(m->token_list[k]);
        elem->id = i;
        TAILQ_INSERT_TAIL(&head_tokens, elem, entries);

        i++;
    }

    /* check and save the current module descriptor */
    module_check(m);
    modules[j] = m;

    /* done: ready for the next module */
    j++;
}
