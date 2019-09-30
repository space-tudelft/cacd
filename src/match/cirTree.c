static char *rcsid = "$Id: cirTree.c,v 1.1 2018/04/30 12:17:25 simon Exp $";
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

#include "src/match/head.h"
#include "src/match/incl.h"
#include "src/match/proto.h"

Import hash  *def_h;
Import struct cir *b_cirp, *e_cirp; /* global list */
Import struct model_info *impNetws;
Import string string_cap, string_res;
Import string string_nenh, string_penh, string_ndep;

/* This function traverses the circuit tree hierarchically.
 * The order of traversal will be 'depth first'.
 * For each new circuit a data structure will be added
 * to a global list. The order of this list is important.
 */
int cirTree (struct cir *this, DM_CELL *cell_key)
{
    DM_PROJECT *projkey, *proj;
    DM_CELL    *ckey;
    DM_STREAM  *dsp;
    struct cir *beg_childl, *end_childl, *cl, *help;
    char  *cell, *real_name;
    struct model_info *mod;

    projkey = this -> proj;

    if (!(ckey = cell_key)) { /* not root cell */
	if (h_get (def_h, this -> name)) return (0);
	ckey = dmCheckOut (projkey, this -> name, WORKING, DONTCARE, CIRCUIT, READONLY);
	if (!ckey) return (-1);
    }

    if (!(dsp = dmOpenStream (ckey, "mc", "r"))) return (-1);

    beg_childl = end_childl = NULL;

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_dev () || is_func ()) continue;

	cell = cmc.cell_name;

	if (cmc.imported == IMPORTED && !h_get (def_h, cell)) {
	    proj = dmFindProjKey (IMPORTED, cell, projkey, &real_name, CIRCUIT);
	    if (!proj) err_mesg ("Cannot access cell: '%s'\n", cell);

	    /* search imported mods */
	    mod = impNetws;
	    while (mod && (mod -> impproj != proj || strcmp (mod -> name, cell))) mod = mod -> next;
	    if (mod) continue;

	    MALLOC (mod, 1, struct model_info);
	    strcpy (mod -> name, cell);
	    strcpy (mod -> orig_name, real_name);
	    mod -> impproj = proj;
	    mod -> proj = projkey;
	    mod -> next = impNetws; impNetws = mod;
	    cell = real_name;
	}
	else proj = projkey;

	/* search local list */
	cl = beg_childl;
	while (cl && (cl -> proj != proj || strcmp (cl -> name, cell))) cl = cl -> cir_next;
	if (cl == NULL) {
	    /* search global list */
	    cl = b_cirp;
	    while (cl && (cl -> proj != proj || strcmp (cl -> name, cell))) cl = cl -> cir_next;
	}

	if (cl == NULL) { /* not in local / global list */
	    cl = this; /* check parent cells */
	    while (cl && (cl -> proj != proj || strcmp (cl -> name, cell))) cl = cl -> cir_up;
	    if (cl) err_mesg ("Recursive call to cell '%s' while reading mc of '%s'\n", cell, this -> name);

	    MALLOC (cl, 1, struct cir);
	    strcpy (cl -> name, cell);
	    cl -> proj = proj;
	    cl -> cir_next = NULL;
	    cl -> cir_up = this;
	    if (!beg_childl) beg_childl = cl;
	    else end_childl -> cir_next = cl;
	    end_childl = cl;
	}
    }

    dmCloseStream (dsp, COMPLETE);

    if (!cell_key) dmCheckIn (ckey, COMPLETE);

    while ((cl = beg_childl)) {
	beg_childl = cl -> cir_next;

	/* search global list */
	help = b_cirp;
	while (help && (help -> proj != cl -> proj || strcmp (help -> name, cl -> name))) help = help -> cir_next;

	if (!help) { /* not in global list */
	    if (cirTree (cl, NULL) != 0) return (-1);
	    /* add */
	    if (!b_cirp) b_cirp = cl; /* empty global list */
	    else e_cirp -> cir_next = cl; /* add to end */
	    e_cirp = cl; cl -> cir_next = NULL; /* new end */
	}
	else FREE (cl);
    }

    return (0);
}

int is_tor ()
{
    if (strcmp (string_nenh, cmc.cell_name) == 0 ||
	strcmp (string_penh, cmc.cell_name) == 0 ||
	strcmp (string_ndep, cmc.cell_name) == 0) return ('d');
    return (0); /* not a transistor */
}

int is_dev ()
{
    int len = strlen (cmc.cell_name);
    if (len <= 3) {
	if (len == 3) {
	  if (!strcmp (string_cap, cmc.cell_name)) return ('c');
	  if (!strcmp (string_res, cmc.cell_name)) return ('r');
	}
    }
    else if (len == 4) return is_tor ();
    else if (*cmc.cell_name == '$') { /* check for DSCAP */
	if (strcmp (cmc.cell_name + len - 3, "$ds") == 0) return (1);
    }
    return (0); /* not a device */
}

int is_func ()
{
    char *a;
    int colon = 1;

    a = cmc.inst_attribute;
    while (*a) {
	if (*a == ';') { ++a; colon = 1; }
	else if (colon) {
	    if (*a++ == 'f' &&
		(*a == ';' || !*a || (*a == '=' && *(a+1) != '0'))) return (1);
	    colon = 0;
	}
	else ++a;
    }
    return (0); /* not a function */
}
