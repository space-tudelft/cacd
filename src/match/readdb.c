static char *rcsid = "$Id: readdb.c,v 1.1 2018/04/30 12:17:49 simon Exp $";
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

Import network * cur_network;
Import hash  * def_h;
Import stack * net_st;
Import DM_PROJECT *dmproject;

Public struct model_info *impNetws = NULL;
Public struct cir *b_cirp, *e_cirp;

long lower1[4], upper1[4];
long lower2[4], upper2[4];
long lower3[4], upper3[4];
char attribute_buf[256];

/*
 * Reads terminals, nets, and sub-networks/devices from the
 * database hierarchically on a depth first way (see cirTree).
 * Read_dbase constructs the internal datastructure (and put
 * the subnetworks in a external list of sub-networks).
 */
Public void read_dbase (DM_PROJECT *projkey, string rootntw, hash *hashlist, DM_CELL *cell_key)
{
    char nme[DM_MAXNAME+10];
    struct cir *this;
    struct model_info *mod;
    list  *lp;
    int no;

    /* init database streams */
    if (dm_get_do_not_alloc == 0) {
	dm_get_do_not_alloc = 1;
	cterm.term_attribute = attribute_buf;
	cterm.term_lower = lower1;
	cterm.term_upper = upper1;
	cmc.inst_attribute = attribute_buf;
	cmc.inst_lower  = lower1;
	cmc.inst_upper  = upper1;
	cnet.net_attribute = attribute_buf;
	cnet.net_lower  = lower1;
	cnet.net_upper  = upper1;
	cnet.inst_lower = lower2;
	cnet.inst_upper = upper2;
	cnet.ref_lower  = lower3;
	cnet.ref_upper  = upper3;
    }

    def_h = hashlist;

    MALLOC (this, 1, struct cir);
    strcpy (this -> name, rootntw);
    this -> proj = projkey;
    this -> cir_next = NULL;
    this -> cir_up = NULL;

    /* cirTree creates list of descendant cells */
    b_cirp = e_cirp = NULL;

    if (!h_get (def_h, rootntw))
    if (cirTree (this, cell_key) != 0) {
	err_mesg ("Network: '%s' does not exist\n", rootntw);
    }

    if (!e_cirp) b_cirp = this; /* empty list */
    else e_cirp -> cir_next = this; /* add to end */

    while ((this = b_cirp)) {
	b_cirp = this -> cir_next;

	if ((cur_network = h_get (def_h, this -> name))) {
	    if ((no = this -> proj -> projectno) == 0)
		verb_mesg ("Network: '%s' already extracted\n", this -> name);
	    else {
		verb_mesg ("Network: '%s' of 'p%d' already extracted\n", this -> name, no);
		sprintf (nme, "p%d:%s", no, this -> name);
		h_link (def_h, mk_symbol(nme), cur_network);
	    }
	    cur_network = NULL;
	}
	else {
	    /* network read from database; we expect the last cell to be the root cell */
	    xnetwork (this -> proj, this -> name, !b_cirp ? cell_key : NULL);
	    if (cur_network) {
		verb_mesg ("Network: '%s' extracted\n", this -> name);
		h_link (def_h, cur_network -> name, cur_network);
		while ((lp = (list *) pop (net_st))) merge (cur_network, lp);
		if (b_cirp) cur_network = NULL;
	    }
	    else err_mesg ("Network: '%s' not extracted!\n", this -> name);
	}
	FREE (this);
    }

    while ((mod = impNetws)) {
	impNetws = mod -> next;
	FREE (mod);
    }
}
