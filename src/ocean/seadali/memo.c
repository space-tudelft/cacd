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

extern DM_CELL *ckey;
extern char *cellstr;
extern qtree_t **quad_root;
extern INST *inst_root;
extern INST *act_inst;
extern TEMPL *first_templ;	/* templates of instances */
extern TERM **term_root;
extern TERM *act_term;
extern int  sub_term_flag;
extern int  exp_level;
extern int  NR_lay;
extern int  checked;

void init_mem ()
{
    register int lay;

    ckey = NULL;
    cellstr = NULL;
    inst_root = NULL;
    first_templ = NULL;
    act_inst = NULL;
    act_term = NULL;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_root[lay] = qtree_build ();
	term_root[lay] = NULL;
    }
    sub_term_flag = FALSE;
    exp_level = 1;
    checked = FALSE;
}

void empty_mem ()
{
    INST *inst_p, *next_inst;
    TEMPL *templ_p, *next_templ;
    TERM *tpntr, *next_term;
    register int lay;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_clear (quad_root[lay]);
    }

    /* now clear 'root' instances */
    for (inst_p = inst_root; inst_p; inst_p = next_inst) {
	next_inst = inst_p -> next;
	FREE (inst_p -> inst_name);
	FREE (inst_p);
    }

    /* now we have a list of 'floating' templates: clear them */
    for (templ_p = first_templ; templ_p; templ_p = next_templ) {
	next_templ = templ_p -> next;
	clear_templ (templ_p);	/* clears all quads, lower instances, etc. */
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	for (tpntr = term_root[lay]; tpntr; tpntr = next_term) {
	    next_term = tpntr -> nxttm;
	    FREE (tpntr -> tmname);
	    FREE (tpntr);
	}
    }
    empty_err ();

    empty_comments ();

    init_mem ();
}

void clear_templ (TEMPL *templ_p)
{
    TERM *tpntr, *next_term;
    INST *inst_p, *next_inst;
    register int lay;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_clear (templ_p -> quad_trees[lay]);
	templ_p -> quad_trees[lay] = NULL;

	for (tpntr = templ_p -> term_p[lay]; tpntr; tpntr = next_term) {
	    next_term = tpntr -> nxttm;
	    FREE (tpntr -> tmname);
	    FREE (tpntr);
	}
	templ_p -> term_p[lay] = NULL;
    }
    templ_p -> t_flag = FALSE;

    for (inst_p = templ_p -> inst; inst_p; inst_p = next_inst) {
	next_inst = inst_p -> next;
	FREE (inst_p -> inst_name);
	FREE (inst_p);
    }
    templ_p -> inst = NULL;
    templ_p -> projkey = NULL;
    FREE (templ_p -> term_p);
    FREE (templ_p -> quad_trees);
    FREE (templ_p -> cell_name);
    FREE (templ_p);
}

static int quad_not_empty (qtree_t *Q)
{
    if (!Q) return 0;
    if (Q -> object || Q -> reference) return 1;

    if (quad_not_empty (Q -> Uleft))  return 1;
    if (quad_not_empty (Q -> Uright)) return 1;
    if (quad_not_empty (Q -> Lleft))  return 1;
    if (quad_not_empty (Q -> Lright)) return 1;
    return 0;
}

int no_works ()
{
    register int lay;

    if (inst_root) return (FALSE);
    for (lay = 0; lay < NR_lay; ++lay) if (quad_not_empty (quad_root[lay])) return (FALSE);
    for (lay = 0; lay < NR_lay; ++lay) if (term_root[lay]) return (FALSE);
    if (!no_comments ()) return (FALSE);
    return (TRUE);
}
