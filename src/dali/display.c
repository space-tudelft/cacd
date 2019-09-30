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

extern DM_PROJECT *dmproject;
extern INST *inst_root;
extern TERM **term_root;
extern TERMREF *termlist;
extern int  CHECK_FLAG;
extern int *fillst;
extern int *term_lay;
extern int *vis_arr;
extern int  v_term, v_sterm, v_inst, v_bbox;
extern int  v_tname, v_stname, v_iname;
extern int  NR_lay;
extern float XL, XR, YB, YT;
extern float c_cW, c_cH;

/* Sea-of-Gates additions: */
extern char *ImageName; /* inst_name of image */
extern char *ViaName;   /* 1st three char's of inst_name of via's */

#define	SUBSCR_LIMIT	QUAD_LAMBDA*1000

void trans_box (Coor arr1[], Coor arr2[], INST *ip, int nnx, int nny)
{
    register Coor l, r, b, t;
    Trans *tr = ip -> tr;
/*
** The editor works with integer transformations (orthogonal).
*/
    l = (Coor) (tr[0] * arr1[0] + tr[1] * arr1[2] + tr[2]);
    b = (Coor) (tr[3] * arr1[0] + tr[4] * arr1[2] + tr[5]);
    r = (Coor) (tr[0] * arr1[1] + tr[1] * arr1[3] + tr[2]);
    t = (Coor) (tr[3] * arr1[1] + tr[4] * arr1[3] + tr[5]);

    arr2[0] = (l < r ? l : r) + nnx * ip -> dx;
    arr2[1] = (l < r ? r : l) + nnx * ip -> dx;
    arr2[2] = (b < t ? b : t) + nny * ip -> dy;
    arr2[3] = (b < t ? t : b) + nny * ip -> dy;
}

/*
** Draw terminal layers of the cell (highest level).
*/
void dis_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int  i, j;
    register Coor xl, xr, yb, yt;
    register TERM *tp;

    if (!vis_arr[v_term]) return; /* terminals not visible */

    if (!(tp = term_root[lay])) return; /* no terminals */

    ggSetColor (lay);
    d_fillst (fillst[lay]);

    do {
	xl = tp -> xl;
	xr = tp -> xr;
	if (xr == xl) ++xr; /* point terminal */
	for (i = 0;;) {
	    if (xl > wxr || xr < wxl) goto next_x;
	    yb = tp -> yb;
	    yt = tp -> yt;
	    if (yt == yb) ++yt; /* point terminal */
	    for (j = 0;;) {
		if (yb > wyt || yt < wyb) goto next_y;
		if (!CHECK_FLAG) {
		    paint_box ((float) (Max (wxl, xl)),
			    (float) (Min (wxr, xr)),
			    (float) (Max (wyb, yb)),
			    (float) (Min (wyt, yt)));
		}
		else
		    to_precheck (lay, xl, xr, yb, yt);
next_y:
		if (++j > tp -> ny) break;
		yb += tp -> dy;
		yt += tp -> dy;
	    }
next_x:
	    if (++i > tp -> nx) break;
	    xl += tp -> dx;
	    xr += tp -> dx;
	}
    } while ((tp = tp -> nxttm));
}

/*
** Draw bboxes of the instances of the cell (highest level).
*/
void pict_mc (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int  i, j;
    register Coor xl, xr, yb, yt;
    register INST *ip;

    if (!(ip = inst_root)) return; /* no instances */

    if (!vis_arr[v_bbox]) return; /* bboxes not visible */

    do { /* for all instances */
	if (inst_outside_window (ip, wxl, wxr, wyb, wyt)) continue;

	/* Sea-of-Gates: don't draw bbox of image */
        if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) continue;

	xl = ip -> bbxl;
	xr = ip -> bbxr;
	for (i = 0;;) {
	    if (xl > wxr || xr < wxl) goto next_x;
	    yb = ip -> bbyb;
	    yt = ip -> bbyt;
	    for (j = 0;;) {
		if (yb > wyt || yt < wyb) goto next_y;
		pict_rect ((float) xl, (float) xr, (float) yb, (float) yt);
next_y:
		if (++j > ip -> ny) break;
		yb += ip -> dy;
		yt += ip -> dy;
	    }
next_x:
	    if (++i > ip -> nx) break;
	    xl += ip -> dx;
	    xr += ip -> dx;
	}
    } while ((ip = ip -> next));
}

#define NEW_DALI
/*
** Display names of the instances of the cell (highest level).
** Display terminal names of the instances.
*/
void mc_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    float xx1, yy1;
    Coor  trans[4];
    char  iname[100];
#ifdef NEW_DALI
    char  cname[100];
#else
    char  subscript[20];
    register int i, j;
    register char *cp;
#endif
    register INST *ip;
    register TERM *tp;
#ifdef OLD_DALI
    register Coor  xst, yst;
#endif
    register int lay;
    int len;

    if (!(ip = inst_root)) return; /* no instances */

    do { /* for all instances */
	if (inst_outside_window (ip, wxl, wxr, wyb, wyt)) continue;

	/* Sea-of-Gates: don't draw via's and image names */
        if ((ViaName && strncmp (ip -> inst_name, ViaName, 3) == 0) ||
	    (ImageName && strcmp (ip -> inst_name, ImageName) == 0)) continue;

	if ((xx1 = ip -> bbxl) < XL) xx1 = XL;
	if ((yy1 = ip -> bbyb) < YB) yy1 = YB;

	if (vis_arr[v_iname]) { /* instance text visible */
	/*
	 * Draw instance- and cell name in the instance bbox.
	 * Keep the names inside the PICT-window.
	 */
#ifndef NEW_DALI
	    len = strlen (cp = ip -> inst_name);
	    if (len > 10) len = 10;
	    for (i = 0; i < len;) iname[i++] = *cp++;

	    iname[i++] = '(';
	    len = strlen (cp = ip -> templ -> cell_name);
	    if (len > 10) len = 10;
	    for (j = 0; j < len; ++j) iname[i++] = *cp++;
	    iname[i++] = ')';

	    if (ip -> nx || ip -> ny) { /* and array subscripts */
		sprintf (subscript, "[%dx%d]", ip -> nx, ip -> ny);
		len = strlen (cp = subscript);
		for (j = 0; j < len; ++j) iname[i++] = *cp++;
	    }
	    iname[len = i] = 0;

	    d_text (xx1, yy1, iname, len);

#else /* NEW_DALI */
	    if (ip -> nx || ip -> ny) { /* add array subscripts */
		sprintf (iname, "%s[%dx%d]",
		    ip -> inst_name, ip -> nx, ip -> ny);
	    }
	    else strcpy (iname, ip -> inst_name);

	    sprintf (cname, "(%s)", ip -> templ -> cell_name);

	    if (yy1 + 0.8 * c_cH <= ip -> bbyt) {
		d_text (xx1 - 0.1 * c_cW, yy1, cname, strlen (cname));
		if (yy1 + 1.6 * c_cH <= ip -> bbyt)
		    d_text (xx1, yy1 + c_cH, iname, strlen (iname));
	    }
#endif /* NEW_DALI */

#ifdef OLD_DALI
	/*
	 * Draw repetition text in each rep.bbox of the instance.
	 */
	    if ((XR - XL) < SUBSCR_LIMIT) {
		for (j = 0; j <= ip -> ny; ++j) {
		    yst = ip -> bbyb + j * ip -> dy;
		    for (i = 0; i <= ip -> nx; ++i) {
			if (i == 0 && j == 0) continue;
			sprintf (subscript, "%d.%d", i, j);
			xst = ip -> bbxl + i * ip -> dx;
			if (xst >= wxl && xst <= wxr
				&& yst >= wyb && yst <= wyt)
			    d_text ((float)xst, (float)yst,
				subscript, strlen (subscript));
		    }
		}
	    }
#endif /* OLD_DALI */
	}

	if (vis_arr[v_sterm] && vis_arr[v_stname] &&
	    xx1 + 3 * c_cW <= ip -> bbxr &&
	    yy1 + 2 * c_cH <= ip -> bbyt) {
	/*
	 ** Subterm-flag 'visible', subterm_char-flag 'visible'
	 ** and subterminals of this instance 'to be drawn'.
	 **
	 ** Names of terminals of (0,0) occurrence of instance.
	 */
	    if (!ip -> templ -> t_flag) {
		exp_templ (ip -> templ, dmproject, ip -> imported, READ_TERM);
	    }
	    for (lay = 0; lay < NR_lay; ++lay) {
		for (tp = ip -> templ -> term_p[lay]; tp; tp = tp -> nxttm) {
		/* Occurrence (0,0) of terminal only. */
		    trans[0] = tp -> xl;
		    trans[1] = tp -> xr;
		    trans[2] = tp -> yb;
		    trans[3] = tp -> yt;
		    trans_box (trans, trans, ip, 0, 0);

		    if (trans[0] >= wxl && trans[0] <= wxr && trans[2] >= wyb && trans[2] <= wyt) {
			int i, j;
			if ((len = tp -> tmlen) > 10) len = 10;
			if (tp -> nx > 0 || tp -> ny > 0) {
			    sprintf (iname, "%s", tp -> tmname);
			    iname[len] = 0;
			    for (j = 0; j <= tp -> ny; ++j)
			    for (i = 0; i <= tp -> nx; ++i) {
				if (tp -> nx > 0 && tp -> ny > 0)
				    sprintf (iname+len, "[%d,%d]", i, j);
				else if (tp -> nx > 0)
				    sprintf (iname+len, "[%d]", i);
				else
				    sprintf (iname+len, "[%d]", j);
				d_text ((float) trans[0] + i*tp->dx, (float) trans[2] + j*tp->dy, iname, strlen(iname));
			    }
			}
			else
			d_text ((float) trans[0], (float) trans[2], tp -> tmname, len);
		    }
		}
	    }
	}
    } while ((ip = ip -> next));
}

/*
** Display terminal names of the cell (highest level).
*/
void term_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    char  str[MAXSTR+20];
    int   repetition;
    register TERM *tp;
    register TERMREF *t;
    register int it, jt;
    register Coor xst, yst;
    int len;

    if (!vis_arr[v_term] || !vis_arr[v_tname])
	return; /* terminals or term-chars not visible */

    for (t = termlist; t; t = t -> next) {
	if ((tp = t -> tp)) {
	    strcpy (str, tp -> tmname);
	    len = tp -> tmlen;
	    repetition = tp -> ny + tp -> nx;
	    for (jt = 0; jt <= tp -> ny; ++jt) {
		yst = tp -> yb + jt * tp -> dy;
		for (it = 0; it <= tp -> nx; ++it) {
		    xst = tp -> xl + it * tp -> dx;
		    if (xst >= wxl && xst <= wxr && yst >= wyb && yst <= wyt) {
			if (repetition) {
			    if (tp -> nx && tp -> ny) {
				sprintf (str+len, "[%d,%d]", it, jt);
			    }
			    else if (tp -> nx) {
				sprintf (str+len, "[%d]", it);
			    }
			    else if (tp -> ny) {
				sprintf (str+len, "[%d]", jt);
			    }
			    d_text ((float) xst, (float) yst, str, len + strlen (str+len));
			}
			else
			    d_text ((float) xst, (float) yst, str, len);
		    }
		}
	    }
	}
    }
}

/*
** Draw terminal layers of the instances of the cell (highest level).
*/
void dis_s_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int i, j, it, jt;
    Coor trans[4];
    register INST *ip;
    register TERM *tp;

    if (!(ip = inst_root)) return; /* no instances */

    if (!vis_arr[v_sterm]) return; /* sub-terminals not visible */

    ggSetColor (lay);
    d_fillst (FILL_HASHED);

    do { /* for all instances */
	if (inst_outside_window (ip, wxl, wxr, wyb, wyt)) continue;
	if (vis_arr[v_inst] && vis_arr[lay] && ip -> level > 1) {
	/* Terminals have already been drawn when
	** displaying the expansion information.
	*/
	    continue;
	}

	if (!ip -> templ -> t_flag) {
	    exp_templ (ip -> templ, dmproject, ip -> imported, READ_TERM);
	}
	for (tp = ip -> templ -> term_p[lay]; tp; tp = tp -> nxttm) {
	    for (i = 0; i <= ip -> ny; ++i)
	    for (j = 0; j <= ip -> nx; ++j) {
		for (it = 0; it <= tp -> ny; ++it)
		for (jt = 0; jt <= tp -> nx; ++jt) {
		    trans[0] = tp -> xl + jt * tp -> dx;
		    trans[1] = tp -> xr + jt * tp -> dx;
		    trans[2] = tp -> yb + it * tp -> dy;
		    trans[3] = tp -> yt + it * tp -> dy;
		    trans_box (trans, trans, ip, j, i);

		    if (trans[1] == trans[0]) ++trans[1]; /* point terminal */
		    if (trans[3] == trans[2]) ++trans[3]; /* point terminal */

		    if (trans[0] < wxr && trans[1] > wxl && trans[2] < wyt && trans[3] > wyb) {
			if (!CHECK_FLAG) {
			    paint_box ((float) (Max (wxl, trans[0])),
				    (float) (Min (wxr, trans[1])),
				    (float) (Max (wyb, trans[2])),
				    (float) (Min (wyt, trans[3])));
			}
			else {
			    to_precheck (lay, trans[0], trans[1], trans[2], trans[3]);
			}
		    }
		}
	    }
	}
    } while ((ip = ip -> next));
}
