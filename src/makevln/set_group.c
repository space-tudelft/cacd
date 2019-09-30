/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
 *	S. de Graaf
 *	A.J. van Genderen
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

#include "src/makevln/incl.h"

struct S_teq s_teq;
struct group_tree *merge_groups (struct group_tree *group1, struct group_tree *group2);
struct group_tree *group_ptr;
extern struct event_rec *act_event;
extern struct sr_field *h_sr;
extern int   term_layer;
extern FILE *teq_file;
extern long  sr_pos;

/* set the group_ptr of the stateruler field;
** if its a terminal file, then write a line to the
** teq_file, at least if this has not been done before
*/
void set_group (struct sr_field *c_sr)
{
    struct group_tree *group1;
    int connect_type = NULL_CONN;

    if (CONNECTED (c_sr -> prev, c_sr))
	if (c_sr -> prev -> duration != sr_pos)
	    connect_type += DOWN_CONN;

    if (CONNECTED (c_sr, c_sr -> next))
	if (c_sr -> next -> duration != sr_pos)
	    connect_type += UP_CONN;

    switch (connect_type) {
    case NULL_CONN:
	if (c_sr -> group == NULL) {
	    ALLOCPTR (group1, group_tree);
	    c_sr -> group = group1;
	    group1 -> tree_count = 1;
	    group1 -> parent = NULL;
	    group1 -> next = group_ptr;
	    group_ptr = group1;
	}
	break;
    case DOWN_CONN:
	c_sr -> group = merge_groups (c_sr -> group, c_sr -> prev -> group);
	break;
    case UP_CONN:
	c_sr -> group = merge_groups (c_sr -> group, c_sr -> next -> group);
	break;
    case TWO_CONN:
	c_sr -> group = merge_groups (c_sr -> group, c_sr -> prev -> group);
	c_sr -> group = merge_groups (c_sr -> group, c_sr -> next -> group);
	break;
    }

    if (term_layer && act_event -> attr_no >= 0) {
	s_teq.tnr = act_event -> attr_no;
	s_teq.grp = c_sr -> group;
	fwrite ((char *)&s_teq, sizeof (s_teq), 1, teq_file);
	act_event -> attr_no = -1;
    }
}
