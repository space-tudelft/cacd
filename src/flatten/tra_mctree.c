/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
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

#include "src/flatten/extern.h"

#define HASHSIZE 256
struct clist *Hashtab[HASHSIZE];

/*
** traverse the mc-tree of the cell recursively
*/
void trav_mctree (struct clist *pcl)
{
    register char  *s;
    register struct clist   *clp;
    register struct clist   *c_list = 0;
    register struct mc_elmt *mcp;
    DM_PROJECT *proj;
    DM_STREAM  *fp;
    char *cname;
    int  hashval;

    if (level == exp_depth) return;

#ifdef DEBUG
PE "=> trav_mctree(%08x): level = %d\n", pcl, level);
PE "   cell: %s, pkey: %08x, ckey: %08x, mc_p: %08x\n",
pcl -> ckey -> cell, pcl -> ckey -> dmproject, pcl -> ckey, pcl -> mc_p);
#endif
    fp = dmOpenStream (pcl -> ckey, "mc", "r");

    while (dmGetDesignData (fp, GEO_MC) > 0) {

	if (gmc.imported) {
	    if (Lflag) { pcl -> imps = 1; continue; }
	    proj = dmFindProjKey (gmc.imported, gmc.cell_name,
			pcl -> ckey -> dmproject, &cname, LAYOUT);
	}
	else {
	    proj = pcl -> ckey -> dmproject;
	    cname = gmc.cell_name;
	}
	/*
	** allocate and add a mc-element to mc-list
	*/
	ALLOCPTR (mcp, mc_elmt);
	mcp -> parent = pcl;
	mcp -> mtx[0] = gmc.mtx[0];
	mcp -> mtx[1] = gmc.mtx[1];
	mcp -> mtx[2] = gmc.mtx[2];
	mcp -> mtx[3] = gmc.mtx[3];
	mcp -> mtx[4] = gmc.mtx[4];
	mcp -> mtx[5] = gmc.mtx[5];
	mcp -> dx = gmc.dx;
	mcp -> nx = gmc.nx;
	mcp -> dy = gmc.dy;
	mcp -> ny = gmc.ny;

	s = cname;
	hashval = 0;
	while (*s) hashval += *s++;
	hashval %= HASHSIZE;

	for (clp = Hashtab[hashval]; clp; clp = clp -> ht_next)
	    if (strcmp (cname, clp -> ckey -> cell) == 0
		    && proj == clp -> ckey -> dmproject)
		break; /* found */

	if (!clp) {
	/*
	** allocate new cell-element
	** add it to the cell-list and hash-table
	*/
	    ALLOCPTR (clp, clist);
	    clp -> imps = 0;
	    clp -> ht_next = Hashtab[hashval];
	    Hashtab[hashval] = clp;

	    clp -> ckey = dmCheckOut (proj, cname,
			    ACTUAL, DONTCARE, LAYOUT, READONLY);
	    mcp -> mc_next = 0;
	    clp -> mc_p = mcp;
	    clp -> cl_next = c_list;
	    c_list = clp;
	}
	else {
	    mcp -> mc_next = clp -> mc_p;
	    clp -> mc_p = mcp;
	}
    }

    dmCloseStream (fp, COMPLETE);

    ++level;

    while ((clp = c_list)) {
	c_list = clp -> cl_next;
	clp -> cl_next = celllist;
	trav_mctree (celllist = clp);
    }

    --level;

#ifdef DEBUG
PE "<= trav_mctree()\n");
#endif
}
