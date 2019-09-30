/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Simon de Graaf
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

#include "src/ocean/seadali/header.h"

extern qtree_t **quad_root;
static void del_trap (struct obj_node *dbox, int lay);

void del_box (int lay, Coor xl, Coor yb, Coor xr, Coor yt)
{
    struct obj_node *dbox;

    MALLOC (dbox, struct obj_node);
    dbox -> ll_x1 = Min (xl, xr);
    dbox -> ll_y1 = Min (yb, yt);
    dbox -> ll_x2 = Max (xl, xr);
    dbox -> ll_y2 = Max (yb, yt);
    dbox -> leftside = 0;
    dbox -> rightside = 0;
    dbox -> next = NULL;

    del_trap (dbox, lay);
    FREE (dbox);
}

/*
** Delete a linked list of trapezoids from the quad tree.
** INPUT: the trapezoids and their mask layer.
*/
void del_traps (struct obj_node *p, int lay)
{
    struct obj_node *scan, *nextscan;

    for (scan = p; scan; scan = nextscan) {
	nextscan = scan -> next;
	del_trap (scan, lay);
    }
}

/*
** Delete a trapezoid from the screen and data structure.
** INPUT: the trapezoid and its mask layer.
*/
static void del_trap (struct obj_node *dbox, int lay)
{
    struct obj_node *clip_head; /* head of list of remaining traps */
    struct found_list *save_list; /* list of intersecting traps */
    struct found_list *tmp;

    if (!dbox) return;

    dbox -> next = NULL;
    clip_head = NULL;

    /* find intersecting trapezoids */
    save_list = quick_search (quad_root[lay], dbox);

    while (save_list) {
	/* clip two trapezoids */
	clip_head = clip (save_list -> ptrap, dbox, clip_head);

	/* remove trapezoid from quad tree */
	quad_delete (quad_root[lay], save_list -> ptrap);

	tmp = save_list -> next;
	FREE (save_list);
	save_list = tmp;
    }

    /* insert in quad tree */
    if (clip_head) add_quad (quad_root, clip_head, lay);
}
