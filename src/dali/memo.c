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

extern DM_CELL *ckey;
extern char *cellstr;
extern qtree_t **quad_root;
extern INST *inst_root;
extern TEMPL *first_templ;	/* templates of instances */
extern TERM **term_root;
extern TERMREF *termlist, *termlistlast;
extern int  NR_lay;
extern int  checked;
extern int  exp_level;
extern int *vis_arr;
extern int  v_sterm;

void init_mem ()
{
    register int lay;

    if (ckey) dmCheckIn (ckey, QUIT);
    ckey = NULL;
    if (cellstr) FREE (cellstr);
    cellstr = NULL;
    inst_root = NULL;
    first_templ = NULL;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_root[lay] = qtree_build ();
	term_root[lay] = NULL;
    }
    vis_arr[v_sterm] = 0; /* sub-terminals not visible */
    exp_level = 1;
    checked = 0; /* NO */
}

void empty_mem ()
{
    register int lay;
    register INST  *ip, *next_ip;
    register TEMPL *ep, *next_ep;
    TERM *tp;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_clear (quad_root[lay]); /* rebuild by init_mem () */
    }
    while ((termlistlast = termlist)) {
	tp = termlist -> tp;
	FREE (tp -> tmname);
	FREE (tp);
	termlist = termlist -> next;
	FREE (termlistlast);
    }

    /* now clear 'root' instances */
    for (next_ip = inst_root; (ip = next_ip);) {
	next_ip = ip -> next;
	FREE (ip -> inst_name);
	FREE (ip);
    }

    /* now we have a list of 'floating' templates: clear them */
    for (next_ep = first_templ; (ep = next_ep);) {
	next_ep = ep -> next;
	clear_templ (ep); /* clears all quads, lower instances, etc. */
    }

    empty_err ();

    empty_comments ();

    init_mem ();
}

void clear_templ (TEMPL *ep)
{
    register int lay;
    register TERM *tp, *next_tp;
    register INST *ip, *next_ip;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_clear (ep -> quad_trees[lay]);
	for (next_tp = ep -> term_p[lay]; (tp = next_tp);) {
	    next_tp = tp -> nxttm;
	    FREE (tp -> tmname);
	    FREE (tp);
	}
    }
    for (next_ip = ep -> inst; (ip = next_ip);) {
	next_ip = ip -> next;
	FREE (ip -> inst_name);
	FREE (ip);
    }
    FREE (ep -> term_p);
    FREE (ep -> quad_trees);
    FREE (ep -> cell_name);
    FREE (ep);
}

static int quad_is_empty (qtree_t *Q)
{
    if (!Q) return (1); /* YES */
    if (Q -> object) return (0); /* NO */
    if (Q -> reference) return (0); /* NO */
    if (!quad_is_empty (Q -> Uleft))  return (0); /* NO */
    if (!quad_is_empty (Q -> Uright)) return (0); /* NO */
    if (!quad_is_empty (Q -> Lleft))  return (0); /* NO */
    if (!quad_is_empty (Q -> Lright)) return (0); /* NO */
    return (1); /* YES */
}

int no_works ()
{
    register int lay;

    if (inst_root) return (0); /* NO */

    for (lay = 0; lay < NR_lay; ++lay) {
	if (!quad_is_empty (quad_root[lay])) return (0); /* NO */
	if (term_root[lay]) return (0); /* NO */
    }
    if (!no_comments ()) return (0); /* NO */
    return (1); /* YES */
}
