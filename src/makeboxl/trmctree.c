/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include "src/makeboxl/extern.h"

struct clist *clp_image = NULL;
extern int default_status;
static int isImage = 0;

#define HASHSIZE 256
struct clist *Hashtab[HASHSIZE];

Private int hashVal (register char *s)
{
    register int hashval = 0;
    while (*s) hashval += *s++;
    return (hashval % HASHSIZE);
}

char *strsave (char *s)
{
    char *p;
    if (!s || !*s) return (NULL);
    if (!(p = (char *) malloc (strlen (s) + 1))) errexit (6, "");
    return (strcpy (p, s));
}

Private int isDevmod (DM_PROJECT *proj, char *cell)
{
    struct stat st_buf;
    DM_CELL *ckey;
    int mode;
    noErrMes = 1; /* SdeG4.24 */
    ckey = dmCheckOut (proj, cell, WORKING, DONTCARE, CIRCUIT, READONLY);
    noErrMes = 0;
    if (!ckey) return (0);
    mode = dmStat (ckey, "devmod", &st_buf);
    dmCheckIn (ckey, COMPLETE);
    return (mode == 0);
}

Private struct clist *newCellElement (DM_PROJECT *proj, int hashval)
{
    struct clist *clp;

    ALLOCPTR (clp, clist);
    clp -> all_allowed = 1;
    clp -> freemasks_bits = 0;
    clp -> mc_p = 0;
    clp -> pkey = proj;
    clp -> ht_next = Hashtab[hashval];
    Hashtab[hashval] = clp;
    return clp; /* SdeG4.23 */
}

Private void setCellStatus (struct clist *clp)
{
    DM_STREAM *dsp;
    char *s, *t;
    int isMacro = 0;
    int i, c, dev = 0;
    struct stat st_buf;
    DM_XDATA xdata;

    if (dmStatXData (clp -> pkey, &st_buf) == 0) { /* new project */
	xdata.name = clp -> name;
	(void)dmGetCellStatus (clp -> pkey, &xdata);
	switch (xdata.celltype) {
	case DM_CT_DEVICE:
	case DM_CT_LIBRARY:
	    if (lookatdevmod) dev = 1;
	    if (xdata.interfacetype == DM_IF_STRICT) break;
	case DM_CT_MACRO:
	    isMacro = 1;
	}
	if (xdata.interfacetype == DM_IF_FREEMASKS && (isMacro || !flat_mode)) {
	    if (!isMacro) dev = isMacro = 1;  // regular cell and !flat_mode
	    clp -> all_allowed = 0;
	    for (s = xdata.masks; *s;) {
		while (*s == ' ') ++s;
		if (!*s) break;
		i = (*s - '0');
		while (*++s && *s != ' ') i = 10*i + (*s - '0');
		if (i < 64) clp -> freemasks_bits |= 1LL << i;
	    }
	}
    }
else {
    noErrMes = 1; /* SdeG4.24 */
    dsp = dmOpenStream (clp -> ckey, "is_macro", "r");
    noErrMes = 0;
    if (dsp) {
	if (fscanf (dsp -> dmfp, "%d%s", &isMacro, buf) == 2 && isMacro) { /* SdeG4.25 */
	    clp -> all_allowed = 0;
	    for (t = s = buf;; s = ++t) {
		while (*t && *t != '+') ++t;
		if (t == s) break;
		c = *t;
		*t = '\0';
		for (i = 0; i < no_masks; ++i) {
		    if (!strcmp (s, process -> mask_name[i])) {
			if (i < 64) clp -> freemasks_bits |= 1LL << i;
			break;
		    }
		}
		if (!c) break;
	    }
	}
	dmCloseStream (dsp, COMPLETE);
    }
    if (lookatdevmod) dev = isDevmod (clp -> pkey, clp -> name);
}
    clp -> status = isMacro ? 2 : 0;

    if (dev) {
	++clp -> status;
	clp -> hier = (pseudo_hier_mode || isMacro)? 2 : 1;
    }
    else
	clp -> hier = (flat_mode || isMacro)? 0 : default_status; /* 1 or 2 */
}

Private int equalCellStatus (struct clist *clp, struct clist *cl2)
{
    struct stat st_buf;
    DM_XDATA xdata;
    char *s;
    long long freemasks_bits;
    int dev, i, isMacro, all_allowed;

    if (dmStatXData (cl2 -> pkey, &st_buf) == 0) { /* new project */
	xdata.name = cl2 -> name;
	if (dmGetCellStatus (cl2 -> pkey, &xdata)) return 1;
	dev = isMacro = 0;
	switch (xdata.celltype) {
	case DM_CT_IMPORT:
	    return 1;
	case DM_CT_MACRO:
	    isMacro = 1;
	    break;
	case DM_CT_DEVICE:
	case DM_CT_LIBRARY:
	    if (lookatdevmod) dev = 1;
	    if (xdata.interfacetype != DM_IF_STRICT) isMacro = 1;
	    break;
	}
	freemasks_bits = 0;
	if (xdata.interfacetype == DM_IF_FREEMASKS && (isMacro || !flat_mode)) {
	    if (!isMacro) dev = isMacro = 1;  // regular cell and !flat_mode
	    all_allowed = 0;
	    for (s = xdata.masks; *s;) {
		while (*s == ' ') ++s;
		if (!*s) break;
		i = (*s - '0');
		while (*++s && *s != ' ') i = 10*i + (*s - '0');
		if (i < 64) freemasks_bits |= 1LL << i;
	    }
	}
	else all_allowed = 1;

	cl2 -> status = isMacro ? 2 : 0;
	if (dev) {
	    ++cl2 -> status;
	    cl2 -> hier = (pseudo_hier_mode || isMacro)? 2 : 1;
	}
	else
	    cl2 -> hier = (flat_mode || isMacro)? 0 : default_status; /* 1 or 2 */

	if (cl2 -> hier != clp -> hier) goto use_local;
	if (all_allowed && clp -> all_allowed) return 1;
	if (all_allowed || clp -> all_allowed) goto use_local;
	if (freemasks_bits == clp -> freemasks_bits) return 1;
use_local:
	cl2 -> freemasks_bits = freemasks_bits;
	cl2 -> all_allowed = all_allowed;
	return 0;
    }
    return 1; /* SdeG4.22, local status not supported */
}

Private IMPCELL *dmGetImpCell (DM_PROJECT *proj, char *cell)
{
    register IMPCELL **icl;
    int i = _dmValidView (LAYOUT);
    if (i < 0 || !(icl = proj -> impcelllist[i]))
	icl = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, proj, LAYOUT);
    if (icl)
    while (*icl) {
	if (strcmp ((*icl) -> alias, cell) == 0) return *icl;
	++icl;
    }
    errexit (15, cell);
    return 0;
}

/*
** traverse the mc-tree of the cell recursively
*/
void trav_mctree (struct clist *pcl, int image_done)
{
    register struct clist *clp;
    register struct clist *c_list = 0;
    register struct mc_elmt *mcp;
    DM_PROJECT *proj;
    DM_STREAM  *fp;
    int hashval;

#ifdef DEBUG
P_E "=> trav_mctree(%08x, %d): level = %d\n", pcl, image_done, level);
P_E "   cell: %s, pkey: %08x, ckey: %08x, mc_p: %08x\n",
pcl -> ckey -> cell, pcl -> ckey -> dmproject, pcl -> ckey, pcl -> mc_p);
#endif

    fp = dmOpenStream (pcl -> ckey, "mc", "r");

    while (dmGetDesignData (fp, GEO_MC) > 0) {

	if (part_exp && level == 1) {
	    if (gmc.bxr <= exp_reg[0] || gmc.bxl >= exp_reg[1]
	    ||  gmc.byt <= exp_reg[2] || gmc.byb >= exp_reg[3]) {
		/*
		** the mc-bbox coordinates have no overlap
		** with the expansion region
		*/
		continue;
	    }
	}

	/* See if instance is an image, and if so, if it can be skipped.
	 */
	if (imageName && strcmp (imageName, gmc.inst_name) == 0) {
	    if (image_done || level == exp_depth || (Lflag && gmc.imported)) {
		/* already done on higher level in hierarchy, or...
		   level eq. exp_depth, or only local mode, skip it here */
		if (verbose > 1)
		    P_E "-- skipping image on level %d in cell \"%s\"\n",
			level, pcl -> ckey -> cell);
		continue;
	    }
	    else {
		/* not done on higher level in hierarchy, do it here */
		if (verbose > 1)
		    P_E "-- proc'ing image on level %d in cell \"%s\"\n",
			level, pcl -> ckey -> cell);
		image_done = isImage = 1;
	    }
	}

	/*
	** allocate and add a mc-element to mc-list
	*/
	ALLOCPTR (mcp, mc_elmt);
	mcp -> parent = pcl;
	mcp -> inst_name = strsave (gmc.inst_name);
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
    if (samples > 1) {
	mcp -> mtx[2] *= samples;
	mcp -> mtx[5] *= samples;
	mcp -> dx *= samples;
	mcp -> dy *= samples;
    }

	proj = pcl -> ckey -> dmproject;
	hashval = hashVal (cellname = gmc.cell_name);

	/* find imported/alias or local/real element */
	for (clp = Hashtab[hashval]; clp; clp = clp -> ht_next)
	    if (clp -> pkey == proj &&
		strcmp (clp -> name, cellname) == 0) break;

	if (!clp) {
	    struct clist *cl2;
	/*
	** allocate new cell-element
	** add it to the cell-list and hash-table
	*/
	    cl2 = clp = newCellElement (proj, hashval);

	    if (gmc.imported) {
		IMPCELL *icp = dmGetImpCell (proj, cellname);
		proj = dmOpenProject (icp -> dmpath, PROJ_READ);

		/* find the real one */
		hashval = hashVal (cellname = icp -> cellname);
		for (clp = Hashtab[hashval]; clp; clp = clp -> ht_next)
		    if (clp -> pkey == proj &&
			strcmp (clp -> name, cellname) == 0) break;

		/* use cl2 for imported/alias element */
		cl2 -> name = icp -> alias;
		mcp -> name = cl2 -> name;
		if (clp) goto equal_test; /* real one found */

		/* create the real one */
		clp = newCellElement (proj, hashval);
	    }

	    clp -> ckey = dmCheckOut (proj, cellname, ACTUAL, DONTCARE, LAYOUT, READONLY);
	    clp -> name = clp -> ckey -> cell;
	    setCellStatus (clp);

	    if (gmc.imported) {
equal_test:
		if (equalCellStatus (clp, cl2)) {
		    /* alias next pointer = real one */
		    cl2 -> cl_next = clp;
		    if (clp -> mc_p) goto found;
		}
		else { /* use alias */
		    cl2 -> ckey = clp -> ckey;
		    clp = cl2;
		}
	    }
	    else
		mcp -> name = clp -> name;
install:
	    clp -> mc_p = mcp;

	    if (isImage) {
		isImage = 0;
		if (level == 1 && !dflag) {
		    if (!notEmpty (clp -> ckey, "mc", 1)
		     && !notEmpty (clp -> ckey, "term", 1)) {
			clp_image = clp;
			goto cont;
		    }
		}
	    }

	    if (clp -> hier == 1 || exp_depth == 1 || (Lflag && gmc.imported)) {
		clp -> cl_next = celllist;
		celllist = clp;
	    }
	    else { /* do traversal */
		clp -> cl_next = c_list;
		c_list = clp;
	    }
	}
	else {
	    mcp -> name = clp -> name;
	    if (!clp -> mc_p) { /* alias or real one not used */
		if (!gmc.imported) goto install; /* real one */
		clp = clp -> cl_next; /* get real one */
	    }
found:
	    if (clp == clp_image) {
		clp_image = 0;
		clp -> cl_next = celllist;
		celllist = clp;
	    }

	    clp -> mc_p_last -> mc_next = mcp;

	    if (isImage) isImage = 0;
	}
cont:
	clp -> mc_p_last = mcp;
#ifdef DEBUG
#ifdef DEBUG2
	pr_clist (clp);
	pr_mcelmt (mcp);
#endif
#endif
    }

    dmCloseStream (fp, COMPLETE);

    ++level;

    while ((clp = c_list)) {
	c_list = clp -> cl_next;
	clp -> cl_next = celllist;
	celllist = clp;
	trav_mctree (clp, image_done);
    }

    --level;

#ifdef DEBUG
    P_E "<= trav_mctree()\n");
#endif
}
