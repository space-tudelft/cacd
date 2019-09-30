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

struct term_ref {
    char *term_name;
    long  layer_no;
    long  xl, xr, yb, yt;
    long  dx, nx, dy, ny;
    int isLabel;
    char *Class;
    struct term_ref *next;
};

char *dot = ".";

Private char *makeHierInstName (struct tmtx_mc *tmc)
{
    struct mc_elmt *mcp;
    char *s, *t;

    if (!tmc) return dot;
    s = buf;
    do {
	mcp = tmc -> mc;
	t = mcp -> inst_name;
	if (*t == '.' && !*(t+1)) return dot;
	if (s != buf) *s++ = '/'; /* seperate instance names by a '/' */
	if (mcp -> nx && mcp -> ny)
	    sprintf (s, "%s[%d,%d]", t, (int)tmc -> x, (int)tmc -> y);
	else if (mcp -> nx)
	    sprintf (s, "%s[%d]", t, (int)tmc -> x);
	else if (mcp -> ny)
	    sprintf (s, "%s[%d]", t, (int)tmc -> y);
	else
	    sprintf (s, "%s", t);
	s += strlen (s);
	if (s - buf >= DM_MAXPATHLEN) errexit (14, "");
    } while ((tmc = tmc -> child));
    *s = '\0';
    return buf;
}

char *makeIndexName (char *s, int nx, int ny, int x, int y)
{
    if (nx && ny) {
	sprintf (buf, "%s[%d,%d]", s, x, y);
	s = buf;
    }
    else if (nx) {
	sprintf (buf, "%s[%d]", s, x);
	s = buf;
    }
    else if (ny) {
	sprintf (buf, "%s[%d]", s, y);
	s = buf;
    }
    return s;
}

/*
** read the terminals and the labels of the sub-cells.
*/
void read_hier_term (struct clist *clp)
{
    DM_STREAM *fp;
    long bXl, bXr, bYb, bYt;
    long Xl, Xr, Yb, Yt;
    long xl, xr, yb, yt;
    long tmp;
    register int i, j, a;
    long *m;
    int tmsk_no;
    int two_levels;
    int hier_ok, hier_n1, hier_terminal;
    struct tmtx *tm;
    struct tmtx *mcp_tmfirst;
    struct term_ref *terms;
    struct term_ref *terms_last;
    struct term_ref *tref;
    struct mc_elmt *mcp;
    char *full_inst_name;

    bXl = bXr = bYb = bYt = 0; /* suppres uninitialized warning */
    tmsk_no = 0; /* suppres uninitialized warning */
    full_inst_name = 0; /* suppres uninitialized warning */

    /* first, read terminals and labels of 'clp' and store them
       in the list 'terms' */

    terms = terms_last = NULL;

    mcp = clp -> mc_p;
    two_levels = mcp -> parent != topcell ? 1 : 0;
    hier_ok = clp -> hier;
    hier_n1 = (clp -> hier != 1 && extraStreams);

    if (!(hier_n1 || hier_ok)) return;

    fp = dmOpenStream (cellkey, "term", "r");

    while (dmGetDesignData (fp, GEO_TERM) > 0) {
	ALLOCPTR (tref, term_ref);
	tref -> isLabel = 0;
	tref -> term_name = strsave (gterm.term_name);
	tref -> layer_no = gterm.layer_no;
	tref -> xl = samples * gterm.xl;
	tref -> xr = samples * gterm.xr;
	tref -> yb = samples * gterm.yb;
	tref -> yt = samples * gterm.yt;
	tref -> dx = samples * gterm.dx;
	tref -> dy = samples * gterm.dy;
	tref -> nx = gterm.nx;
	tref -> ny = gterm.ny;
	tref -> Class = NULL;
	if (terms_last)
	    terms_last -> next = tref;
	else
	    terms = tref;
	terms_last = tref;
    }

    dmCloseStream (fp, COMPLETE);

    if (hier_n1 && notEmpty (cellkey, "annotations", 0)) {
	fp = dmOpenStream (cellkey, "annotations", "r");
	while (dmGetDesignData (fp, GEO_ANNOTATE) > 0) {
	    if (ganno.type == GA_LABEL) {
		ALLOCPTR (tref, term_ref);
		tref -> isLabel = 1;
		tref -> term_name = strsave (ganno.o.Label.name);
		tref -> layer_no = ganno.o.Label.maskno;
		if (tref -> layer_no < 0
		    || tref -> layer_no >= process -> nomasks) {
		    sprintf (buf, "%ld", tref -> layer_no);
		    errexit (12, buf);
		}
		tref -> xl = (long)(samples * ganno.o.Label.x);
		tref -> xr = (long)(samples * ganno.o.Label.x);
		tref -> yb = (long)(samples * ganno.o.Label.y);
		tref -> yt = (long)(samples * ganno.o.Label.y);
		tref -> nx = 0;
		tref -> ny = 0;
		tref -> Class = strsave (ganno.o.Label.Class);
		if (terms_last)
		    terms_last -> next = tref;
		else
		    terms = tref;
		terms_last = tref;
	    }
	}
	dmCloseStream (fp, COMPLETE);

	ganno.type = GA_LABEL;
	*ganno.o.Label.Attributes = '\0';
	ganno.o.Label.ax = 0;
	ganno.o.Label.ay = 0;
    }

    if (terms_last) terms_last -> next = NULL;
    else if (!hier_ok) return;

    /* second, read bounding-box */

    if (hier_ok && extraStreams) {
	fp = dmOpenStream (cellkey, "info", "r"); /* SdeG4.12 */
	dmGetDesignData (fp, GEO_INFO);
	dmCloseStream (fp, COMPLETE);
	bXl = samples * ginfo.bxl;
	bXr = samples * ginfo.bxr;
	bYb = samples * ginfo.byb;
	bYt = samples * ginfo.byt;
    }

    /* third, visit modelcalls */

    tm = tm_p ? tm_p : tm_s;

    while (tm) {

	if (two_levels) {
	    mcp = tm -> mc;
	    if (extraStreams)
		full_inst_name = makeHierInstName (tm -> tmc);
	}

	if (tm -> tid) {
	    gtid.term_offset = -1;
	    strcpy (gtid.cell_name, mcp -> name);
	    if (two_levels) {
		strcpy (gtid.inst_name, dot);
		gtid.m_nx = 0;
		gtid.m_ny = 0;
		if (extraStreams)
		    dmPrintf (fp_tidnam, "%s\n", full_inst_name);
	    }
	    else {
		strcpy (gtid.inst_name, mcp -> inst_name);
		gtid.m_nx = mcp -> nx;
		gtid.m_ny = mcp -> ny;
		if (extraStreams && strcmp (gtid.inst_name, dot) == 0)
		    dmPrintf (fp_tidnam, ".\n");
	    }
	    dmPutDesignData (fp_tid, GEO_TID);

	    if (extraStreams) {
		m = tm -> mtx;
	    if (m[0]) {
		xl = m[0] * bXl + m[2];
		xr = m[0] * bXr + m[2];
		yb = m[4] * bYb + m[5];
		yt = m[4] * bYt + m[5];
	    }
	    else {
		xl = m[1] * bYb + m[2];
		xr = m[1] * bYt + m[2];
		yb = m[3] * bXl + m[5];
		yt = m[3] * bXr + m[5];
	    }
		if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
		if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }

		dmPrintf (fp_tidpos, "%ld %ld %ld %ld %ld %ld\n",
				xl, yb, xr, yt, mcp -> dx, mcp -> dy);
	    }
	}

	mcp_tmfirst = tm;

	for (tref = terms; tref; tref = tref -> next) {

	    tm = mcp_tmfirst;

	    mask_no = tref -> layer_no;

	    hier_terminal = (tm -> tid && !tref -> isLabel);

	    if (hier_terminal) {
		tmsk_no = process -> mask_no[mask_no];
		if (!fp_bxx[tmsk_no]) open_bxx (mask_no, tmsk_no);
		gtid.term_offset = term_no;
		strcpy (gtid.term_name, tref -> term_name);
		gtid.t_nx = tref -> nx;
		gtid.t_ny = tref -> ny;
		dmPutDesignData (fp_tid, GEO_TID);
            }

	    do {
		m = tm -> mtx;
		Xl = tref -> xl;
		Xr = tref -> xr;
		for (a = i = 0;;) {
		    Yb = tref -> yb;
		    Yt = tref -> yt;
		    for (j = 0;;) {

		    if (m[0]) {
			xl = m[0] * Xl + m[2];
			xr = m[0] * Xr + m[2];
			yb = m[4] * Yb + m[5];
			yt = m[4] * Yt + m[5];
		    }
		    else {
			xl = m[1] * Yb + m[2];
			xr = m[1] * Yt + m[2];
			yb = m[3] * Xl + m[5];
			yt = m[3] * Xr + m[5];
		    }
			if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
			if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }

			if (part_exp &&
			       (xr <= exp_reg[0] || xl >= exp_reg[1]
			    ||  yt <= exp_reg[2] || yb >= exp_reg[3])) {
			    /*
			    ** the box-coordinates have no overlap
			    ** with the expansion region
			    */
			    if (hier_terminal) ++term_no;
			}
			else {
			    if (hier_terminal) {
				gboxlay.xl = xl;
				gboxlay.xr = xr;
				gboxlay.yb = yb;
				gboxlay.yt = yt;
				gboxlay.chk_type = term_no++;
				dmPutDesignData (fp_bxx[tmsk_no], GEO_BOXLAY);
				++no_bxx[tmsk_no];
			    }

			    if (hier_n1) {
				if (!two_levels && !a) {
				    ++a;
				    if (mcp -> nx || mcp -> ny)
					full_inst_name = makeIndexName (
					    mcp -> inst_name,
					    mcp -> nx, mcp -> ny,
					    (int)tm -> x, (int)tm -> y);
				    else if (tref == terms)
					full_inst_name = mcp -> inst_name;
				}
				if (full_inst_name) {
				    dmPrintf (fp_anno_exp, "%s %s %d\n",
					full_inst_name, mcp -> name,
					tref -> isLabel);
				    full_inst_name = 0;
				}
				else
				    dmPrintf (fp_anno_exp, "/ %d\n",
					tref -> isLabel);

				if (tref -> isLabel) {
				    strcpy (ganno.o.Label.name, tref -> term_name);
				    if (tref -> Class)
					strcpy (ganno.o.Label.Class, tref -> Class);
				    else
					*ganno.o.Label.Class = '\0';
				    ganno.o.Label.x = xl;
				    ganno.o.Label.y = yb;
				    ganno.o.Label.maskno = mask_no;
				    dmPutDesignData (fp_anno_exp, GEO_ANNOTATE);
				}
				else {
				    if (tref -> nx > 0) {
					if (tref -> ny > 0)
					dmPrintf (fp_anno_exp, "%s[%d,%d] %d %ld %ld\n",
					    tref -> term_name, i, j, mask_no, xl, yb);
					else
					dmPrintf (fp_anno_exp, "%s[%d] %d %ld %ld\n",
					    tref -> term_name, i, mask_no, xl, yb);
				    }
				    else if (tref -> ny > 0)
					dmPrintf (fp_anno_exp, "%s[%d] %d %ld %ld\n",
					    tref -> term_name, j, mask_no, xl, yb);
				    else
					dmPrintf (fp_anno_exp, "%s %d %ld %ld\n",
					    tref -> term_name, mask_no, xl, yb);
				}
			    }
			}
			if (++j > tref -> ny) break;
			Yb += tref -> dy;
			Yt += tref -> dy;
		    }
		    if (++i > tref -> nx) break;
		    Xl += tref -> dx;
		    Xr += tref -> dx;
		}

		if (two_levels) break;

		tm = tm -> tm_next;
	    } while (tm && tm -> mc == mcp); /* mc-copies */
	}

	if (!two_levels) {
	    if (!(mcp = mcp -> mc_next)) break;
	    if (mcp -> parent != topcell) two_levels = 1;
	}
	else {
	    tm = tm -> tm_next;
	}
    }

    /* Dispose the terms list */

    while ((tref = terms)) {
	terms = tref -> next;
	if (tref -> Class) FREE (tref -> Class);
	FREE (tref);
    }
    gboxlay.chk_type = 0;
}
