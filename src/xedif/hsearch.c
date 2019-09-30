/*
 * ISC License
 *
 * Copyright (C) 1986-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "src/xedif/incl.h"
#include "src/xedif/search.h"

typedef struct node {	/* Part of the linked list of entries */
	ENTRY item;
	struct node *next;
} NODE;
typedef NODE *TABELEM;
static NODE **table;	/* The address of the hash table */
static NODE *free_node_list;

static unsigned int length;	/* Size of the hash table */

/* Create a hash table no smaller than size
*/
void hcreate ()
{
    unsigned int unsize;	/* Holds the shifted size */
    int i, size = 1000;

    if (table) { /* re-init the table */
	for (i = 0; i < length; ++i) {
	    if (table[i]) {
		NODE *p = table[i];
		while (p -> next) p = p -> next;
		p -> next = free_node_list;
		free_node_list = table[i];
		table[i] = NULL;
	    }
	}
    }
    else {
	unsize = size; /* +1 for empty table slot; -1 for ceiling */
	length = 1;    /* Maximum entries in table */
	while (unsize) {
	    unsize >>= 1;
	    length <<= 1;
	}
	PALLOC (table, length, TABELEM);
    }
}

/* Division hashing scheme
** key = key to be hashed
*/
static unsigned int hashd (char *key)
{
    unsigned int sum = 0;	/* Results */
    int s;			/* Length of the key */

    /* convert multicharacter key to unsigned int */
    for (s = 0; *key; s++)	/* Simply add up the bytes */
	sum += *key++;
    sum += s;

    return (sum % length);
}

/* Chained search with sorted lists (to resolve collisions).
** item   = item to be inserted or found
** action = FIND or ENTER
*/
ENTRY *hsearch (ENTRY item, ACTION action)
{
    NODE *p; /* pointer to search through the linked list */
    unsigned int i = hashd (item.key);

    p = table[i];
    while (p && strcmp (item.key, p -> item.key)) p = p -> next;
    if (!p) { /* Item not found */
	if (action == FIND) return (0);
	/* Enter item: get new NODE to store item */
	if ((p = free_node_list)) free_node_list = p -> next;
	else p = (NODE *) malloc (sizeof (NODE));
	if (!p) cannot_die (1, 0);
	p -> item = item;
	p -> next = table[i];
	table[i] = p;
    }
    return (&(p -> item));
}
