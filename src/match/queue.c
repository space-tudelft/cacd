static char *rcsid = "$Id: queue.c,v 1.1 2018/04/30 12:17:46 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
 *	N.P. van der Meijs
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
/*
 * This module contains a set of functions to
 * maintain the double ended queue datastructure.
 * A queue is implemented as a double linked cyclic list.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*
 * Clears a queue or a stack.
 */
Public void clear (queue *ThiS_queue)
{
    while (del_queue (ThiS_queue, HEAD) != NULL);
}

/*
 * Checks whether the the specified queue or
 * stack contains the 'data'.
 * Either True or False is returned.
 */
Public boolean contains (queue *ThiS_queue, void *data)
{
    bucket *ThiS, *head;

    Assert (ThiS_queue);

    head = *ThiS_queue;
    for (ThiS = head; ThiS != NULL;) {
	if (ThiS -> data == data) return (True);

	ThiS = ThiS -> next;
	if (ThiS == head) break;
    }
    return (False);
}

/*
 * Creates a queue datastructure.
 */
Public queue *mk_a_queue ()
{
    queue *tmp;
    Malloc (tmp, 1, queue);
    *tmp = NULL;
    return (tmp);
}

/*
 * Adds a data item to a double ended queue.
 * Dependent on the value of mode, the item is
 * inserted at the HEAD or appended at the TAIL of the queue.
 */
Public void add_queue (queue *ThiS_queue, void *data, short mode)
{
    bucket *ThiS, *that;

    ThiS = mk_a_bucket ();
    ThiS -> data = data;

    Assert (ThiS_queue);

    if (*ThiS_queue == NULL) { /* queue is empty */
	ThiS -> prev = ThiS;
	ThiS -> next = ThiS;
	*ThiS_queue = ThiS;
    }
    else {
	/* insert ThiS before 'that' in double ended queue */
	that = *ThiS_queue;
	ThiS -> next = that;
	ThiS -> prev = that -> prev;
	that -> prev -> next = ThiS;
	that -> prev = ThiS;

	if (mode == HEAD) *ThiS_queue = ThiS;
    }
}

/*
 * Deletes a data item from a double ended queue.
 * A pointer to the data is returned.
 * Dependent on the value of mode, the item is
 * taken from the HEAD or the TAIL of the queue.
 */
Public void *del_queue (queue *ThiS_queue, short mode)
{
    bucket *ThiS;

    Assert (ThiS_queue);

    ThiS = *ThiS_queue;
    if (!ThiS) return (NULL); /* empty queue */

    if (ThiS -> next == ThiS) { /* last bucket */
	*ThiS_queue = NULL; /* empty */
    }
    else {
	if (mode == HEAD)
	    *ThiS_queue = ThiS -> next;
	else
	    ThiS = ThiS -> prev; /* tail */

	/* unlink bucket from double ended queue */
	ThiS -> next -> prev = ThiS -> prev;
	ThiS -> prev -> next = ThiS -> next;
    }

    /* pointer to bucket data is returned */
    return (rm_a_bucket (ThiS));
}

/*
 * Prints the specified queue on the specified stream.
 * Only used for debugging purposes.
 */
Public void p_queue (FILE *fp, queue *ThiS_queue)
{
    bucket *head, *ThiS;

    fprintf (fp, "\nHead:");

    Assert (ThiS_queue);

    head = *ThiS_queue;
    if (head == NULL) {
	fprintf (fp, "\t<NULL>\n\n");
	return;
    }

    for (ThiS = head;;) {
	fprintf (fp, "\t%p\n", ThiS -> data);
	ThiS = ThiS -> next;
	if (ThiS == head) break;
    }

    fprintf (fp, "\n");
}
