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

int  no_masks;
DM_CELL *ckey;

struct tmtx * tm_p_last;
struct tmtx * tm_s_last;
char * current_cell;

struct tmtx_mc * tmc_first = NULL;
struct tmtx_mc * tmc_free = NULL;
struct tmtx_mc * tmc_last_used = NULL;

struct mc_elmt * tm_pmc;

/* local operations */
Private void expand (void);
Private void free_tm (void);
Private void calc_tm (struct mc_elmt *, struct tmtx_mc *, int, int, int,
			long, long, long, long, long, long, long long, int);
Private void make_tm (struct tmtx_mc *, int, int, int, int,
			long, long, long, long, long, long, long long, int);
Private int outwdw ( long, long, long, long, long, long);

int notEmpty (DM_CELL *ck, char *name, int err)
{
    struct stat st_buf;
    sprintf (buf, "%s/layout/%s/%s",
	ck -> dmproject -> dmpath, ck -> cell, name);
    if (stat (buf, &st_buf)) {
	if (err) errexit (16, buf);
	return (0);
    }
    return (st_buf.st_size > 0);
}

DM_STREAM * open_bxx (int i, int j)
{
    sprintf (buf, "%s%s_bxx", j > i ? "t_" : "", process -> mask_name[i]);
    return fp_bxx[j] = dmOpenStream (ckey, buf, "a");
}

DM_STREAM * open_nxx (int i)
{
    sprintf (buf, "%s_nxx", process -> mask_name[i]);
    return fp_nxx[i] = dmOpenStream (ckey, buf, "a");
}

/*
** expand cell
*/
void exp_cell (char *cell)
{
    DM_STREAM *fp;
    char      *mask_name;
    register int i;
    long inf3_samples;
    int j;
    int no_files;

#ifdef DEBUG
P_E "=> exp_cell(%s)\n", cell);
#endif

    ckey = dmCheckOut (project, cell, WORKING, DONTCARE, LAYOUT, ATTACH);

    current_cell = cell;

    fp = dmOpenStream (ckey, "info", "r");
    dmGetDesignData (fp, GEO_INFO);
    dmCloseStream (fp, COMPLETE);

    if (part_exp) {
	part_exp = 4;
	if (exp_reg[0] <= ginfo.bxl) { exp_reg[0] = ginfo.bxl; --part_exp; }
	if (exp_reg[1] >= ginfo.bxr) { exp_reg[1] = ginfo.bxr; --part_exp; }
	if (exp_reg[2] <= ginfo.byb) { exp_reg[2] = ginfo.byb; --part_exp; }
	if (exp_reg[3] >= ginfo.byt) { exp_reg[3] = ginfo.byt; --part_exp; }
	if (!part_exp)
	    P_E "warning: partial expansion turned off, cell inside exp_reg\n");
	else
	if (exp_reg[1] <= ginfo.bxl || exp_reg[0] >= ginfo.bxr
	 || exp_reg[3] <= ginfo.byb || exp_reg[2] >= ginfo.byt) {
	    P_E "warning: cell '%s' not expanded, outside exp. region\n", cell);
	    return;
	}
    }
    else {
	exp_reg[0] = ginfo.bxl;
	exp_reg[1] = ginfo.bxr;
	exp_reg[2] = ginfo.byb;
	exp_reg[3] = ginfo.byt;
    }

    inf3_samples = samples;
    if (samples < 1) samples = 1;

    no_files = no_masks = process -> nomasks;

    /* determine real number of open files (add term masks) */
    for (i = 0; i < no_masks; ++i) {
	if (process -> mask_type[i] == DM_INTCON_MASK)
	    process -> mask_no[i] = no_files++;
    }

    /* allocate file pointers */
    ALLOCARR (fp_bxx, no_files, DM_STREAM *);
    ALLOCARR (no_bxx, no_files, long);
    if (!dflag) {
	ALLOCARR (fp_nxx, no_masks, DM_STREAM *);
    }

    /* clear info3 */
    fp = dmOpenStream (ckey, "info3", "w");
    dmPutDesignData (fp, GEO_INFO3);
    dmCloseStream (fp, COMPLETE);

    /* open all expansion files */
    fp_tid = dmOpenStream (ckey, "tid", "w");
    if (extraStreams) {
	fp_tidpos = dmOpenStream (ckey, "tidpos", "w");
	fp_tidnam = dmOpenStream (ckey, "tidnam", "w");
	fp_anno_exp = dmOpenStream (ckey, "anno_exp", "w");
    }
    if (flat_mode)
	fp_spec = dmOpenStream (ckey, "spec", "w");
    else
	dmUnlink (ckey, "spec");
    if (pseudo_hier_mode)
	fp_spec = dmOpenStream (ckey, "pseudo_hier", "w");
    else
	dmUnlink (ckey, "pseudo_hier");

    /* open cell "bxx" files, and open or remove "nxx" files */
    for (i = 0; i < no_masks; ++i) {
	mask_name = process -> mask_name[i];
	sprintf (buf, "%s_nxx", mask_name);
	if (!dflag) {
	    fp_nxx[i] = dmOpenStream (ckey, buf, "w");
	    dmCloseStream (fp_nxx[i], COMPLETE);
	    fp_nxx[i] = NULL;
	}
	else
	    dmUnlink (ckey, buf);
	sprintf (buf, "%s_bxx", mask_name);
	fp_bxx[i] = dmOpenStream (ckey, buf, "w");
	dmCloseStream (fp_bxx[i], COMPLETE);
	fp_bxx[i] = NULL;
	no_bxx[i] = 0;
	if (process -> mask_type[i] == DM_INTCON_MASK) {
	    sprintf (buf, "t_%s_bxx", mask_name);
	    j = process -> mask_no[i];
	    fp_bxx[j] = dmOpenStream (ckey, buf, "w");
	    dmCloseStream (fp_bxx[j], COMPLETE);
	    fp_bxx[j] = NULL;
	    no_bxx[j] = 0;
	}
    }

    /*
    ** initialize the mc-tree and cell-list
    */
    ALLOCPTR (topcell, clist);
    topcell -> name = cell; /* SdeG4.20 */
    topcell -> ckey = ckey;
    topcell -> hier = 0;
    topcell -> mc_p = 0;
    topcell -> all_allowed = 1;

    if (exp_depth > 0) {
#ifdef DEBUG
#ifdef DEBUG2
pr_clist (topcell);
#endif
#endif
	trav_mctree (topcell, Image_done);
	gtid.term_offset = -1;
	strcpy (gtid.cell_name, cell);
	strcpy (gtid.inst_name, "$");
	gtid.m_nx = 0;
	gtid.m_ny = 0;
	dmPutDesignData (fp_tid, GEO_TID);
    }

    if (samples > 1) {
	exp_reg[0] *= samples;
	exp_reg[1] *= samples;
	exp_reg[2] *= samples;
	exp_reg[3] *= samples;
    }

    topcell -> cl_next = celllist;
    celllist = topcell;

    ALLOCARR (px, pt_size, double);
    ALLOCARR (py, pt_size, double);

    gboxlay.chk_type = 0; /* init */

    expand ();

    if (clp_image) {
	cellkey = clp_image -> ckey;
	write_image_bxx ();
    }

    /*
    ** close the cell "bxx" files
    */
    for (i = 0; i < no_files; ++i)
	if (fp_bxx[i]) dmCloseStream (fp_bxx[i], COMPLETE);

    if (clp_image && notEmpty (cellkey, "nor", 1)) {
	write_image_nxx ();
	for (i = 0; i < no_masks; ++i)
	    if (fp_nxx[i]) dmCloseStream (fp_nxx[i], COMPLETE);
    }

    /*
    ** close the cell "tid" files
    */
    dmCloseStream (fp_tid, COMPLETE);
    if (extraStreams) {
	dmCloseStream (fp_tidpos, COMPLETE);
	dmCloseStream (fp_tidnam, COMPLETE);
	dmCloseStream (fp_anno_exp, COMPLETE);
    }
    if (flat_mode || pseudo_hier_mode)
	dmCloseStream (fp_spec, COMPLETE);

#ifdef SIGINT
    signal (SIGINT, SIG_IGN);
#endif

    /*
    ** write no_of_boxes for each mask
    ** to the cell "info2" file
    */
    fp = dmOpenStream (ckey, "info2", "w");
    ginfo2.nr_groups = 0;
    for (i = 0; i < no_files; ++i) {
	ginfo2.nr_boxes = no_bxx[i];
	dmPutDesignData (fp, GEO_INFO2);
    }
    dmCloseStream (fp, COMPLETE);

    fp = dmOpenStream (ckey, "info3", "w");
    ginfo3.bxl = exp_reg[0];
    ginfo3.bxr = exp_reg[1];
    ginfo3.byb = exp_reg[2];
    ginfo3.byt = exp_reg[3];
    ginfo3.nr_samples = inf3_samples;
    dmPutDesignData (fp, GEO_INFO3);
    dmCloseStream (fp, COMPLETE);

    dmCkinAll (COMPLETE);

    current_cell = NULL;

#ifdef DEBUG
P_E "<= exp_cell()\n");
#endif
}

Private void expand ()
{
    register struct clist *clp;
    register int i;
    DM_STREAM *fp;

    for (clp = celllist -> cl_next; clp; clp = clp -> cl_next)
	clp -> mc_p_last -> mc_next = 0; /* SdeG4.16 */

    for (clp = celllist; clp; clp = clp -> cl_next) {
#ifdef DEBUG
#ifndef DEBUG2
pr_clist (clp);
#endif
#endif
	free_tm ();
	cellkey = clp -> ckey;

	s_mode = 0;
	if (Lflag && cellkey -> dmproject != project) ++s_mode;

	if (part_exp) {
	    fp = dmOpenStream (cellkey, "info", "r");
	    dmGetDesignData (fp, GEO_INFO);
	    dmCloseStream (fp, COMPLETE);
	    ginfo.bxl *= samples;
	    ginfo.bxr *= samples;
	    ginfo.byb *= samples;
	    ginfo.byt *= samples;
	}

	if (tmc_last_used) {
	    tmc_last_used -> next = tmc_free;
	    tmc_free = tmc_first;
	    tmc_last_used = tmc_first = 0;
	}

	tm_p_last = tm_s_last = 0;

	if (!(tm_pmc = clp -> mc_p)) /* ROOT cell */
	    make_tm (0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0LL, 1);
	else
	    calc_tm (tm_pmc, 0, 1, 0, (int)clp -> hier, 1, 0, 0, 0, 1, 0, clp->freemasks_bits, clp->all_allowed);

	if (tm_p_last) tm_p_last -> tm_next = 0;
	if (tm_s_last) tm_s_last -> tm_next = 0;

	if (clp -> hier != 1) {
	    if (tm_p || tm_s) read_term (clp);
	}

	if (clp -> mc_p) { /* not the ROOT cell */
	    if (tm_p || tm_s) read_hier_term (clp);
	}

	if (tm_p && clp -> hier != 1) {
	    read_box ();

	  if (notEmpty (cellkey, "nor", 1)) {
	    if (!dflag) {
		/* close "bxx" files */
		for (i = 0; i < no_masks; ++i)
		    if (fp_bxx[i]) { dmCloseStream (fp_bxx[i], COMPLETE); fp_bxx[i] = NULL; }

		read_nor2 ();

		/* close "nxx" files */
		for (i = 0; i < no_masks; ++i)
		    if (fp_nxx[i]) { dmCloseStream (fp_nxx[i], COMPLETE); fp_nxx[i] = NULL; }
	    }
	    else
		read_nor ();
	  }
	}
    }
}

Private void free_tm ()
{
    register struct tmtx *t1, *t2;

    t2 = tm_p;
    while ((t1 = t2)) {
	t2 = t1 -> tm_next;
	FREE (t1);
    }
    tm_p = 0;

    t2 = tm_s;
    while ((t1 = t2)) {
	t2 = t1 -> tm_next;
	FREE (t1);
    }
    tm_s = 0;
}

Private void make_tm (struct tmtx_mc *ptmc, int level, int i, int j, int tid,
			long a0, long a1, long a2, long a3, long a4, long a5, long long mask_bits, int all)
{
    register struct tmtx *tm;

    ALLOCPTR (tm, tmtx);
    tm -> x = (short)i;
    tm -> y = (short)j;
    tm -> tid = (short)tid;
    tm -> allow_all = all;
    tm -> allowmasks = mask_bits;
    tm -> mtx[0] = a0;
    tm -> mtx[1] = a1;
    tm -> mtx[2] = a2;
    tm -> mtx[3] = a3;
    tm -> mtx[4] = a4;
    tm -> mtx[5] = a5;
    tm -> mc = tm_pmc;
    tm -> tmc = ptmc;
    if (s_mode || level == exp_depth) {
	if (tm_s_last) tm_s_last -> tm_next = tm;
	else tm_s = tm;
	tm_s_last = tm;
    }
    else {
	if (tm_p_last) tm_p_last -> tm_next = tm;
	else tm_p = tm;
	tm_p_last = tm;
    }
}

Private void calc_tm (struct mc_elmt *pmc, struct tmtx_mc *ptmc, int level, int two, int tid,
			long a0, long a1, long a2, long a3, long a4, long a5, long long mask_bits, int all)
{
    register int i, j;
    register long *b;
    struct mc_elmt *ppmc;
    struct tmtx_mc *tmc;
    long m0, m1, m2, m3, m4, m5, n5;
    int two_levels, do_tid, new_all = 0;
    long long new_mask_bits = 0;

    for (; pmc; pmc = pmc -> mc_next) {
	ppmc = pmc -> parent -> mc_p;
	if (ppmc) {
	    if (level == exp_depth) continue;
	    if (pmc -> parent -> all_allowed) {
		new_mask_bits = mask_bits;
		new_all = all;
	    }
	    else { /* not all allowed */
		new_mask_bits = pmc -> parent -> freemasks_bits;
		if (!all) new_mask_bits &= mask_bits;
		new_all = 0;
	    }
	}

	if ((do_tid = tid) && pmc -> parent -> hier) do_tid = 0;

	if (level == 1) {
	    tm_pmc = pmc;
	    two_levels = ppmc ? extraStreams : 0;
	}
	else two_levels = two;

	if (two_levels) {
	    char *t = pmc -> inst_name;
	    if (*t == '.' && !*(t+1)) two_levels = 0;
	}

	b = pmc -> mtx;
    if (b[0]) {
	m0 = b[0] * a0;
	m1 = b[0] * a1;
	m2 = b[0] * a2;
	m3 = b[4] * a3;
	m4 = b[4] * a4;
	m5 = b[4] * a5;
    }
    else {
	m0 = b[1] * a3;
	m1 = b[1] * a4;
	m2 = b[1] * a5;
	m3 = b[3] * a0;
	m4 = b[3] * a1;
	m5 = b[3] * a2;
    }
	m2 += b[2];
	m5 += b[5];
	for (i = 0;;) {
	    n5 = m5;
	    for (j = 0;;) {
		if (!ppmc && part_exp && outwdw (m0, m1, m2, m3, m4, n5))
			goto next_mc;

		if (two_levels) {
		    if (tmc_free)
			tmc_free = (tmc = tmc_free) -> next;
		    else {
			ALLOCPTR (tmc, tmtx_mc);
			if (tmc_last_used) /* SdeG4.19 */
			    tmc_last_used -> next = tmc;
		    }
		    if (!tmc_first) tmc_first = tmc;
		    tmc_last_used = tmc;
		    tmc -> mc = pmc;
		    tmc -> x = (short)i;
		    tmc -> y = (short)j;
		    tmc -> child = ptmc;
		}
		else tmc = 0;

		if (!ppmc)
		    make_tm (tmc, level, i, j, do_tid,
			m0, m1, m2, m3, m4, n5, mask_bits, all);
		else
		    calc_tm (ppmc, tmc, level+1, two_levels, do_tid,
			m0, m1, m2, m3, m4, n5, new_mask_bits, new_all);
next_mc:
		if (++j > pmc -> ny) break;
		n5 += pmc -> dy;
	    }
	    if (++i > pmc -> nx) break;
	    m2 += pmc -> dx;
	}
    }
}

Private int outwdw (long m0, long m1, long m2, long m3, long m4, long m5)
{
    long xl, xr, yb, yt, tmp;

    if (m0) {
	xl = m0 * ginfo.bxl + m2;
	xr = m0 * ginfo.bxr + m2;
	yb = m4 * ginfo.byb + m5;
	yt = m4 * ginfo.byt + m5;
    }
    else {
	xl = m1 * ginfo.byb + m2;
	xr = m1 * ginfo.byt + m2;
	yb = m3 * ginfo.bxl + m5;
	yt = m3 * ginfo.bxr + m5;
    }

    if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
    if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }

    if (xr <= exp_reg[0] || xl >= exp_reg[1]
    ||  yt <= exp_reg[2] || yb >= exp_reg[3]) {
    /*
    ** the box-coordinates have no overlap
    ** with the expansion region
    */
	return (1);
    }
    return (0);
}
