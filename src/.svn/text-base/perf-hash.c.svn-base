/*
    $Id: perf-hash.c,v 1.7 2008-01-12 16:10:23 awgn Exp $
 
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
#include <string.h>

#include <global.h>
#include <typedef.h>
#include <prototype.h>

_unused static
const char cvsid[]= "$Id: perf-hash.c,v 1.7 2008-01-12 16:10:23 awgn Exp $";


struct entry {
	char *str;
	int id;
	    TAILQ_ENTRY(entry) entries;
};

/*
 * in case of collisions the hash table needs to be freed
 */
int
free_hashtable(perf_hash_t * hash_table)
{
    int i;

    for (i = 0; i < hash_table->size; i++) {
        if (hash_table->tab[i].id != -1 &&
            hash_table->tab[i].key != (void *)0xffffffff)
            free(hash_table->tab[i].key);
    }

    return 0;
}


/*
 * given a list, the function create a perfect hash table
 */

#define MAX_TABSIZE	(1<<14)

int
setup_hashtable(perf_hash_t * hash_table, struct tqh * head)
{
    struct entry *np;
    u_int sz, coll, pos;

    sz = count_token(head);
    hash_table->tab = NULL;

    do {
        hash_table->size = sz;
        hash_table->tab = (card_t *) realloc(hash_table->tab, sz * sizeof(card_t));
        memset(hash_table->tab, 0xffffffff , sz * sizeof(card_t));

        for (coll = 0, np = head->tqh_first; np != NULL; np = np->entries.tqe_next) {
            pos = (hash(np->str, strlen(np->str)) % sz);

            if (hash_table->tab[pos].id != 0xffffffff ) {
                free_hashtable(hash_table);
                coll++;
                break;
            }
            hash_table->tab[pos].id = np->id;
            hash_table->tab[pos].key = strdup(np->str);
        }

        sz++;
    } while (coll && (sz < MAX_TABSIZE));

    if (coll)
        fatal(__INTERNAL__);

    DEBUG("table size: #%d\n",hash_table->size);
    return 0;
}

/*
 * perfect hash search
 */
#ifdef __GNUC__
__inline
#endif
int
qsearch(p,ph)
const char *p;
perf_hash_t *ph;
{
    uint32_t i;
    if (p == NULL)
        return -1;
    i = ((uint32_t) hash((char *) p, strlen(p)) % ph->size);

    if ( ph->tab[i].id != -1 && strcmp(ph->tab[i].key, p) == 0)
        return ph->tab[i].id;
    else
        return -1;
}

