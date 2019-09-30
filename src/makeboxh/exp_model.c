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

#include "src/makeboxh/extern.h"

/*
** expand cell
*/
void exp_cell (struct ctree *cell, int partial)
{
    struct mc_elmt *mc_tree;
    struct wdw     *t1, *t2;
    FILE      *fp_exp;  /* file pointer to "exp_dat" file */
    DM_STREAM *fp_info;
    char   *mask_name;
    void    (*istat)();
    int     i, j;
    int     no_files, no_masks;

    level = 1; /* root level */
    term_no = 0;
    fchtype = chtype = 0;
    mc_bboxl = NULL;
    part_exp = partial;

    top_key = dmCheckOut (project, cell -> name, WORKING, DONTCARE, LAYOUT, ATTACH);
    cellkey = top_key;

    fp_info = dmOpenStream (top_key, "info", "r");
    dmGetDesignData (fp_info, GEO_INFO);
    dmCloseStream (fp_info, COMPLETE);

    if (part_exp) {
	part_exp = 4;
	if (exp_reg[0] <= ginfo.bxl) {
	    exp_reg[0] = ginfo.bxl; --part_exp;
	}
	if (exp_reg[1] >= ginfo.bxr) {
	    exp_reg[1] = ginfo.bxr; --part_exp;
	}
	if (exp_reg[2] <= ginfo.byb) {
	    exp_reg[2] = ginfo.byb; --part_exp;
	}
	if (exp_reg[3] >= ginfo.byt) {
	    exp_reg[3] = ginfo.byt; --part_exp;
	}
#ifdef DEBUG
if (!part_exp)
    P_E "partial expansion false, cell bbox inside exp_reg\n");
#endif
	if (exp_reg[1] <= ginfo.bxl
	    || exp_reg[0] >= ginfo.bxr
	    || exp_reg[3] <= ginfo.byb
	    || exp_reg[2] >= ginfo.byt) {
P_E "warning: cell '%s' not expanded, outside exp. region\n", cell -> name);
	    return;
	}
    }
    else {
	exp_reg[0] = ginfo.bxl;
	exp_reg[1] = ginfo.bxr;
	exp_reg[2] = ginfo.byb;
	exp_reg[3] = ginfo.byt;
    }

    no_files = no_masks = process -> nomasks;

    /* determine real number of open files (add term masks) */
    for (i = 0; i < no_masks; ++i) {
	if (process -> mask_type[i] == DM_INTCON_MASK)
	    process -> mask_no[i] = no_files++;
    }

    /* allocate file pointers */
    ALLOCARR (fp_bxx, no_files, DM_STREAM *);
    ALLOCARR (no_bxx, no_files, long);

    /* clear info3 */
    fp_info = dmOpenStream (top_key, "info3", "w");
    dmPutDesignData (fp_info, GEO_INFO3);
    dmCloseStream (fp_info, COMPLETE);

    /* clear nr_of_boxes and nr_of_groups info */
    dmUnlink (top_key, "spec");

    /* open "tid" file */
    fp_tid = dmOpenStream (top_key, "tid", "w");

    /* open cell "bxx" files, and clean "nxx" files */
    for (i = 0; i < no_masks; ++i) {
	mask_name = process -> mask_name[i];
	sprintf (fname, "%s_nxx", mask_name);
	fp_info = dmOpenStream (top_key, fname, "w");
	dmCloseStream (fp_info, COMPLETE);
	sprintf (fname, "%s_bxx", mask_name);
	fp_bxx[i] = dmOpenStream (top_key, fname, "w");
	no_bxx[i] = 0;
	if (process -> mask_type[i] == DM_INTCON_MASK) {
	    sprintf (fname, "t_%s_bxx", mask_name);
	    j = process -> mask_no[i];
	    fp_bxx[j] = dmOpenStream (top_key, fname, "w");
	    no_bxx[j] = 0;
	}
    }

    /* initialize the mc-tree */
    ALLOCPTR (mc_tree, mc_elmt);
    strcpy (mc_tree -> name, cell -> name);
    strcpy (mc_tree -> inst_name, "$");
    mc_tree -> imported = 0;
    mc_tree -> bbox[0] = 0;
    mc_tree -> bbox[1] = 0;
    mc_tree -> bbox[2] = 0;
    mc_tree -> bbox[3] = 0;
    mc_tree -> mtx[0] = 1;
    mc_tree -> mtx[1] = 0;
    mc_tree -> mtx[2] = 0;
    mc_tree -> mtx[3] = 0;
    mc_tree -> mtx[4] = 1;
    mc_tree -> mtx[5] = 0;
    mc_tree -> dx = 0;
    mc_tree -> nx = 0;
    mc_tree -> dy = 0;
    mc_tree -> ny = 0;
    mc_tree -> act_regl = NULL;
    mc_tree -> parent   = NULL;
    mc_tree -> child    = NULL;
    mc_tree -> sibling  = NULL;

#ifdef DEBUG1
P_E "allocptr(mc_tree): %08x\n", mc_tree);
pr_mcelmt(mc_tree);
#endif
#ifdef DEBUG
P_E "=> trav_mctree(mc_tree): %08x\n", mc_tree);
#endif
    trav_mctree (mc_tree, project); /* traverse the mc-tree */

    istat = (void (*)()) signal (SIGINT, SIG_IGN);

#ifdef DEBUG1
P_E "free(mc_bboxl): %08x\n", mc_bboxl);
#endif
    /* free mc bbox list */
    for (t1 = mc_bboxl; t1; t1 = t2) {
	t2 = t1 -> next;
	FREE (t1);
    }

    /* close the cell "tid" and "bxx" files */
    dmCloseStream (fp_tid, COMPLETE);
    for (i = 0; i < no_files; ++i) {
	dmCloseStream (fp_bxx[i], COMPLETE);
    }

    /* write expansion bbox to info3 */
    fp_info = dmOpenStream (top_key, "info3", "w");
    ginfo3.bxl = exp_reg[0];
    ginfo3.bxr = exp_reg[1];
    ginfo3.byb = exp_reg[2];
    ginfo3.byt = exp_reg[3];
    ginfo3.nr_samples = 0;
    dmPutDesignData (fp_info, GEO_INFO3);
    dmCloseStream (fp_info, COMPLETE);

    /* write no_of_boxes to cell "info" file */
    fp_info = dmOpenStream (top_key, "info2", "w");
    ginfo2.nr_groups = 0;
    for (i = 0; i < no_files; ++i) {
	ginfo2.nr_boxes = no_bxx[i];
	dmPutDesignData (fp_info, GEO_INFO2);
    }
    dmCloseStream (fp_info, COMPLETE);

    if (!(fp_exp = fopen (fn_exp, "a"))) errexit (4, fn_exp);
    fprintf (fp_exp, "%s\n", cell -> name);
    fclose (fp_exp);

    dmCheckIn (top_key, COMPLETE);

    FREE (fp_bxx);
    FREE (no_bxx);

    signal (SIGINT, istat);

#ifdef DEBUG
P_E "<= exp_cell()\n");
#endif
}
