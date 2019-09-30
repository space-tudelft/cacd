/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
 *	S. de Graaf
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

#include "src/dali/header.h"

static struct bintree *binprev;
static struct bintree *binhead;
static struct x_lst *edgehead, *edgeprev;

static void lnk_bintree (struct bintree *p);
static void lnk_edgetree (struct edgetree *p);

void to_bintree (struct bintree *p, Coor val)
{
    if (!p) return;
    if (p -> value == val) return;

    if (val <p -> value) {
	if (!p -> left)
	    p -> left = mk_bintree (val);
	else
	    to_bintree (p -> left, val);
	return;
    }
    if (!p -> right)
	p -> right = mk_bintree (val);
    else
	to_bintree (p -> right, val);
}

struct bintree * mk_bintree (Coor value)
{
    struct bintree *new;

    MALLOC (new, struct bintree);
    new -> left = NULL;
    new -> right = NULL;
    new -> link = NULL;
    new -> value = value;
    return (new);
}

struct bintree * link_bintree (struct bintree *p)
{
    if (!p) return (NULL);
    binprev = NULL;
    binhead = NULL;
    lnk_bintree (p);
    return (binhead);
}

static void lnk_bintree (struct bintree *p)
{
    if (p -> left) lnk_bintree (p -> left);
    if (!binprev)
	binhead = p;
    else
	binprev -> link = p;
    binprev = p;
    if (p -> right) lnk_bintree (p -> right);
}

void to_edgetree (struct edgetree *p, struct x_lst *edge)
{
    struct x_lst *pntr, *prev;

    if (!p) return;

    if (p -> edge -> ys == edge -> ys) {

	prev = NULL;
	for (pntr = p -> edge; pntr; pntr = pntr -> next) {
	    if (pntr -> xs >= edge -> xs) break;
	    prev = pntr;
	}

	edge -> next = pntr;
	if (prev)
	    prev -> next = edge;
	else
	    p -> edge = edge;
	return;
    }

    if (edge -> ys < p -> edge -> ys) {
	if (!p -> left)
	    p -> left = mk_edgetree (edge);
	else
	    to_edgetree (p -> left, edge);
	return;
    }
    if (!p -> right)
	p -> right = mk_edgetree (edge);
    else
	to_edgetree (p -> right, edge);
}

struct edgetree * mk_edgetree (struct x_lst *edge)
{
    struct edgetree *new;
    MALLOC (new, struct edgetree);
    new -> left = NULL;
    new -> right = NULL;
    new -> edge = edge;
    edge -> next = NULL;
    return (new);
}

struct x_lst * link_edgetree (struct edgetree *p)
{
    if (!p) return (NULL);
    edgeprev = NULL;
    edgehead = NULL;
    lnk_edgetree (p);
    return (edgehead);
}

static void lnk_edgetree (struct edgetree *p)
{
    struct x_lst *search;

    if (!p) return;
    if (p -> left) lnk_edgetree (p -> left);

    if (!edgeprev)
	edgehead = p -> edge;
    else
	edgeprev -> next = p -> edge;

    for (search = p -> edge; search -> next; search = search -> next);
    edgeprev = search;
    p -> edge = NULL;

    if (p -> right) lnk_edgetree (p -> right);
}

/*
** Clear edge tree.
*/
void cl_edgetree (struct edgetree *p)
{
    if (!p) return;
    cl_edgetree (p -> left);
    cl_edgetree (p -> right);
    FREE (p);
}
