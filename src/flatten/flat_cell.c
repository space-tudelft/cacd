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

#define MAX_L 0x7fffffff
#define MIN_L 0x80000000

long elbb_xl = MAX_L;
long elbb_xr = MIN_L;
long elbb_yb = MAX_L;
long elbb_yt = MIN_L;
long mcbb_xl = MAX_L;
long mcbb_xr = MIN_L;
long mcbb_yb = MAX_L;
long mcbb_yt = MIN_L;

struct name_tree *tnam_tree = NULL;

DM_CELL *topkey;

static void calc_tm (struct mc_elmt *pmc, struct tmtx *ptm);
static void expand (void);
static void free_tm (void);

/*
** flatten cell
*/
void flat_cell (char *cname, int Vnr)
{
    DM_STREAM *fp;
#ifdef DEBUG
PE "=> flat_cell(%s, %d)\n", cname, Vnr);
#endif
    topkey = dmCheckOut (project, cname, WORKING, Vnr, LAYOUT, READONLY);
    /*
    ** initialize the mc-tree and cell-list
    */
    ALLOCPTR (celllist, clist);
    celllist -> ckey = topkey;
    celllist -> imps = 0;
    celllist -> mc_p = 0;
    celllist -> cl_next = NULL;

    /*
    ** do terminals
    */
    fp = dmOpenStream (topkey, "term", "r");
    while (dmGetDesignData (fp, GEO_TERM) > 0) {
	if (ldmfile) {
	    PO "term %s %ld %ld %ld %ld %s",
		process -> mask_name[gterm.layer_no],
		gterm.xl, gterm.xr, gterm.yb, gterm.yt, gterm.term_name);
	    if (gterm.nx) PO " cx %ld %ld", gterm.dx, gterm.nx);
	    if (gterm.ny) PO " cy %ld %ld", gterm.dy, gterm.ny);
	    PO "\n");
	}
	else {
	    if (gterm.bxl < elbb_xl) elbb_xl = gterm.bxl;
	    if (gterm.bxr > elbb_xr) elbb_xr = gterm.bxr;
	    if (gterm.byb < elbb_yb) elbb_yb = gterm.byb;
	    if (gterm.byt > elbb_yt) elbb_yt = gterm.byt;
	    dmPutDesignData (fp_term, GEO_TERM);
	}
	if (append_tree (gterm.term_name, &tnam_tree)) {
	    PE "warning: %s: top-cell terminal already used\n", gterm.term_name);
	}
    }
    dmCloseStream (fp, COMPLETE);

    /*
    ** do annotations
    */
    dmErrorOFF = 1;
    fp = dmOpenStream (topkey, "annotations", "r");
    dmErrorOFF = 0;
    if (!fp) goto trav;

    /* format record first */
    if (dmGetDesignData (fp, GEO_ANNOTATE) > 0 && !ldmfile)
	dmPutDesignData (fp_anno, GEO_ANNOTATE);

    if (ldmfile) {
	char *label;
	while (dmGetDesignData (fp, GEO_ANNOTATE) > 0) {
	    switch (ganno.type) {
	    case GA_LINE:
		PO ":line %g %g %g %g : mode=%d\n",
		    ganno.o.line.x1, ganno.o.line.y1,
		    ganno.o.line.x2, ganno.o.line.y2, ganno.o.line.mode);
		break;
	    case GA_TEXT:
		PO ":text %g %g %s",
		    ganno.o.text.x, ganno.o.text.y, ganno.o.text.text);
		if (ganno.o.text.ax >= 0) {
		    if (ganno.o.text.ax > 0)
			PO " : orient=RIGHT");
		    else
			PO " : orient=CENTER");
		}
		PO "\n");
		break;
	    case GA_LABEL:
		label = ganno.o.Label.name;
		PO "label %s %g %g %s",
		    process -> mask_name[ganno.o.Label.maskno],
		    ganno.o.Label.x, ganno.o.Label.y, label);
		if (*ganno.o.Label.Class)
			PO " : cls=%s", ganno.o.Label.Class);
		if (ganno.o.Label.ax >= 0) {
		    if (ganno.o.Label.ax > 0)
			PO " : orient=RIGHT");
		    else
			PO " : orient=CENTER");
		}
		PO "\n");
		if (append_tree (label, &tnam_tree)) {
		    PE "warning: %s: top-cell label already used\n", label);
		}
		break;
	    }
	}
    }
    else {
	char *label;
	while (dmGetDesignData (fp, GEO_ANNOTATE) > 0) {
	    long xl, xr, yb, yt;
	    switch (ganno.type) {
	    case GA_LINE:
		xl = ganno.o.line.x1;
		xr = ganno.o.line.x2;
		if (xr < xl) { xl = xr; xr = ganno.o.line.x1; }
		yb = ganno.o.line.y1;
		yt = ganno.o.line.y2;
		if (yt < yb) { yb = yt; yt = ganno.o.line.y1; }
		break;
	    case GA_TEXT:
		xl = xr = ganno.o.text.x;
		yb = yt = ganno.o.text.y;
		break;
	    case GA_LABEL:
		label = ganno.o.Label.name;
		if (append_tree (label, &tnam_tree)) {
		    PE "warning: %s: top-cell label already used\n", label);
		}
		xl = xr = ganno.o.Label.x;
		yb = yt = ganno.o.Label.y;
		break;
	    default:
		xl = xr = yb = yt = 0;
		pr_err (12, "");
	    }
	    if (xl < elbb_xl) elbb_xl = xl;
	    if (xr > elbb_xr) elbb_xr = xr;
	    if (yb < elbb_yb) elbb_yb = yb;
	    if (yt > elbb_yt) elbb_yt = yt;
	    dmPutDesignData (fp_anno, GEO_ANNOTATE);
	}
    }
    dmCloseStream (fp, COMPLETE);

trav:
    level = 0;
    trav_mctree (celllist);
    expand ();

    if (!ldmfile) {
	if (elbb_xl == MAX_L && elbb_xr == MIN_L) { /* no elbb */
	    if (mcbb_xl == MAX_L && mcbb_xr == MIN_L) { /* no mcbb */
		ginfo.bxl = ginfo.bxr = ginfo.byb = ginfo.byt = 0;
		dmPutDesignData (fp_info, GEO_INFO);
		dmPutDesignData (fp_info, GEO_INFO);
		goto ret;
	    }
	    elbb_xl = elbb_xr = elbb_yb = elbb_yt = 0;
	    ginfo.bxl = mcbb_xl;
	    ginfo.bxr = mcbb_xr;
	    ginfo.byb = mcbb_yb;
	    ginfo.byt = mcbb_yt;
	}
	else if (mcbb_xl == MAX_L && mcbb_xr == MIN_L) { /* no mcbb */
	    mcbb_xl = mcbb_xr = mcbb_yb = mcbb_yt = 0;
	    ginfo.bxl = elbb_xl;
	    ginfo.bxr = elbb_xr;
	    ginfo.byb = elbb_yb;
	    ginfo.byt = elbb_yt;
	}
	else {
	    ginfo.bxl = elbb_xl < mcbb_xl ? elbb_xl : mcbb_xl;
	    ginfo.bxr = elbb_xr > mcbb_xr ? elbb_xr : mcbb_xr;
	    ginfo.byb = elbb_yb < mcbb_yb ? elbb_yb : mcbb_yb;
	    ginfo.byt = elbb_yt > mcbb_yt ? elbb_yt : mcbb_yt;
	}

	dmPutDesignData (fp_info, GEO_INFO); /* total bbox */

	ginfo.bxl = mcbb_xl;
	ginfo.bxr = mcbb_xr;
	ginfo.byb = mcbb_yb;
	ginfo.byt = mcbb_yt;
	dmPutDesignData (fp_info, GEO_INFO); /* mc bbox */

	ginfo.bxl = elbb_xl;
	ginfo.bxr = elbb_xr;
	ginfo.byb = elbb_yb;
	ginfo.byt = elbb_yt;
ret:
	dmPutDesignData (fp_info, GEO_INFO); /* el bbox */
    }

#ifdef DEBUG
PE "<= flat_cell()\n");
#endif
}

static void expand ()
{
    register struct clist *clp;
    struct tmtx *tm;

    newpxpy (512);

    for (clp = celllist; clp; clp = clp -> cl_next) {
#ifdef DEBUG
pr_clist (clp);
#endif
	free_tm ();
	cellkey = clp -> ckey;
	if (verbose) PE "-- %s\n", cellkey -> cell);

	level = 0;
	ALLOCPTR (tm, tmtx);
	tm -> mtx[0] = 1; tm -> mtx[1] = 0; tm -> mtx[2] = 0;
	tm -> mtx[3] = 0; tm -> mtx[4] = 1; tm -> mtx[5] = 0;

	calc_tm (clp -> mc_p, tm);

	if (tm_p || tm_s) {
	    if (cellkey != topkey) read_term ();
	    read_box ();
	    read_nor ();
	    if (!clp -> imps) { tm = tm_p; tm_p = 0; }
	    if (tm_p || tm_s) read_mc ();
	    if (!clp -> imps) tm_p = tm;
	}
    }
}

static void free_tm ()
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

static void calc_tm (struct mc_elmt *pmc, struct tmtx *ptm)
{
    register int i, j;
    register long *a, *b, *c;
    register struct tmtx *tm;
    long tdx, tdy;

    if (!pmc) {
	if (level == exp_depth) {
	    ptm -> tm_next = tm_s;
	    tm_s = ptm;
	}
	else {
	    ptm -> tm_next = tm_p;
	    tm_p = ptm;
	}
	return;
    }

    if (++level > exp_depth) goto ret;

    a = ptm -> mtx;
    for (; pmc; pmc = pmc -> mc_next) {
	b = pmc -> mtx;
	tdx = b[2];
	for (i = 0;;) {
	    tdy = b[5];
	    for (j = 0;;) {
		ALLOCPTR (tm, tmtx);
		c = tm -> mtx;
		c[0] = b[0] * a[0] + b[1] * a[3];
		c[1] = b[0] * a[1] + b[1] * a[4];
		c[2] = b[0] * a[2] + b[1] * a[5] + tdx;
		c[3] = b[3] * a[0] + b[4] * a[3];
		c[4] = b[3] * a[1] + b[4] * a[4];
		c[5] = b[3] * a[2] + b[4] * a[5] + tdy;

		calc_tm (pmc -> parent -> mc_p, tm);

		if (++j > pmc -> ny) break;
		tdy += pmc -> dy;
	    }
	    if (++i > pmc -> nx) break;
	    tdx += pmc -> dx;
	}
    }

ret:
    --level;
    FREE (ptm);
}

int append_tree (char *name, struct name_tree **head)
{
    struct name_tree *ptr;
    if ((ptr = *head)) {
	int cmp = strcmp (name, ptr -> name);
	if (cmp > 0) return (append_tree (name, &(ptr -> rchild)));
	if (cmp < 0) return (append_tree (name, &(ptr -> lchild)));
	return (1); /* found */
    }
    ALLOCPTR (ptr, name_tree);
    strcpy (ptr -> name, name);
    ptr -> rchild = NULL;
    ptr -> lchild = NULL;
    *head = ptr;
    return (0); /* not found */
}
