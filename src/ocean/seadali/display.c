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

extern DM_PROJECT *dmproject;
extern INST *inst_root;
extern TERM **term_root;
extern short CHECK_FLAG;
extern int *fillst;
extern int *vis_arr;
extern int  NR_lay;
extern struct Disp_wdw *c_wdw;

/* PATRICK */
extern char ImageInstName[]; /* instance name which is not drawn */
extern char ViaInstName[];   /* Via name, of which instance name is not drawn */
/* END PATRICK */

#define	SUBSCR_LIMIT	QUAD_LAMBDA*1000

void trans_box (Coor arr1[], Coor arr2[], INST *inst_p, int nnx, int nny)
{
    register Coor l, r, b, t;
    Trans *tr = inst_p -> tr;

 /*
  ** The editor works with integer transformations (orthogonal).
  */
    l = (Coor) (tr[0] * arr1[0] + tr[1] * arr1[2] + tr[2]);
    b = (Coor) (tr[3] * arr1[0] + tr[4] * arr1[2] + tr[5]);
    r = (Coor) (tr[0] * arr1[1] + tr[1] * arr1[3] + tr[2]);
    t = (Coor) (tr[3] * arr1[1] + tr[4] * arr1[3] + tr[5]);

    arr2[0] = (l < r ? l : r) + nnx * inst_p -> dx;
    arr2[1] = (l < r ? r : l) + nnx * inst_p -> dx;
    arr2[2] = (b < t ? b : t) + nny * inst_p -> dy;
    arr2[3] = (b < t ? t : b) + nny * inst_p -> dy;
}

void dis_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int it, jt;
    register Coor dx, dy;
    TERM *tp;

    if (!vis_arr[NR_lay]) return;

    ggSetColor (lay);
    d_fillst (fillst[lay]);

    for (tp = term_root[lay]; tp; tp = tp -> nxttm) {
	for (it = 0; it <= tp -> ny; ++it)
	for (jt = 0; jt <= tp -> nx; ++jt) {
	    dx = jt * tp -> dx;
	    dy = it * tp -> dy;
	    if (tp -> xl + dx < wxr && tp -> xr + dx > wxl
		    && tp -> yb + dy < wyt && tp -> yt + dy > wyb) {
		if (CHECK_FLAG == FALSE) {
		    paint_box ((float) (Max (wxl, tp -> xl + dx)),
			    (float) (Min (wxr, tp -> xr + dx)),
			    (float) (Max (wyb, tp -> yb + dy)),
			    (float) (Min (wyt, tp -> yt + dy)));
		}
		else
		    to_precheck (lay, tp -> xl + dx, tp -> xr + dx, tp -> yb + dy, tp -> yt + dy);
	    }
	}
    }
}

void pict_mc (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int i, j;
    int     nx1, nx2, ny1, ny2;
    float   f_n1, f_n2, f_nh;
    Coor bxl, bxr, byb, byt;
    register INST *ip;

    if (!vis_arr[NR_lay + 3]) return; /* bboxes not visible */

    for (ip = inst_root; ip; ip = ip -> next) {
	if (inst_outside_window (ip, wxl, wxr, wyb, wyt)) continue;

/* PATRICK for sea of gates: no paint of bb of image, that drives you crazy! */
        if (strcmp (ip->inst_name, ImageInstName) == 0) continue; /* don't draw any image */
/* END PATRICK */

	nx1 = 0;
	nx2 = 0;
	ny1 = 0;
	ny2 = 0;

	if (ip -> nx != 0 || ip -> ny != 0) {
	    /*
	     * Repetition: first determine lower- and
	     * upperbounds for repetition numbers.
	     *
	     * The following must hold:
	     * ip -> bbxr + nx1 * dx > wxl
	     * ip -> bbyt + ny1 * dy > wyb
	     * ip -> bbxl + nx2 * dx < wxr
	     * ip -> bbyb + ny2 * dy < wyt
	     */
	    if (ip -> nx != 0) {
		f_n1 = (float) (wxl - ip -> bbxr) / (float) ip -> dx;
		f_n2 = (float) (wxr - ip -> bbxl) / (float) ip -> dx;

		if (ip -> dx < 0) {
		    /* Swap boundaries. */
		    f_nh = f_n1;
		    f_n1 = f_n2;
		    f_n2 = f_nh;
		}
		ASSERT (f_n1 < f_n2);

		nx1 = Max (0, (int) UpperRound (f_n1));
		nx2 = Min (ip -> nx, (int) LowerRound (f_n2));
	    }
	    if (ip -> ny != 0) {
		f_n1 = (float) (wyb - ip -> bbyt) / (float) ip -> dy;
		f_n2 = (float) (wyt - ip -> bbyb) / (float) ip -> dy;

		if (ip -> dy < 0) {
		    /* Swap boundaries. */
		    f_nh = f_n1;
		    f_n1 = f_n2;
		    f_n2 = f_nh;
		}
		ASSERT (f_n1 < f_n2);

		ny1 = Max (0, (int) UpperRound (f_n1));
		ny2 = Min (ip -> ny, (int) LowerRound (f_n2));
	    }

	    ASSERT (nx1 <= nx2);
	    ASSERT (ny1 <= ny2);
	}

	for (j = ny1; j <= ny2; ++j) {
	    for (i = nx1; i <= nx2; ++i) {
		bxl = ip -> bbxl + i * ip -> dx;
		bxr = ip -> bbxr + i * ip -> dx;
		byb = ip -> bbyb + j * ip -> dy;
		byt = ip -> bbyt + j * ip -> dy;
		if (bxl <= wxr && bxr >= wxl && byb <= wyt && byt >= wyb) {
		    pict_rect ((float) bxl, (float) bxr, (float) byb, (float) byt);
		}
	    }
	}
    }
}

void mc_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register Coor xst, yst;
    float   xx1, yy1,
/*          xx2, yy2, */
            ch_w, ch_h, topmost, rightmost;
    Coor trans[4];
    int     lay, i, j, textsize;
    char    subscript[20];
    char    iname[100], cname[100];
    register INST *ip;
    register TERM *tp;

    ch_siz (&ch_w, &ch_h);

    for (ip = inst_root; ip; ip = ip -> next) {
	if (inst_outside_window (ip, wxl, wxr, wyb, wyt)) continue;
/* PATRICK */
	if (strncmp (ip->inst_name, ViaInstName, 3) == 0 ||
	    strcmp (ip->inst_name, ImageInstName) == 0) continue; /* don't draw image and via's */
/* END PATRICK */

	if (vis_arr[NR_lay + 6]) { /* inst_chars visible */

	/*
	 ** Keep cell-name inside PICT-window.
	 */

	    if (strcmp (ip->templ->cell_name, "Error_Marker") == 0) {
	     /* if (Abs (ip->bbxr - ip->bbxl) * 150 < (XR - XL)) continue; */
		sprintf (iname, "<=-%s", ip -> inst_name);
		yy1 = Max ((float) ip -> bbyb, YB);
		xx1 = Max ((float) ip -> bbxr, XL);
		d_text (xx1, yy1, iname);
		continue;
	    }

/* PATRICK: modified */
	    if (strlen (ip -> inst_name) == 0)
	       strcpy (ip -> inst_name, ".");    /* if not set instance name: set it */

	    if (ip -> nx > 0 || ip -> ny > 0) /* add array subscripts */
		sprintf (iname, "%s[%dx%d]", ip -> inst_name, ip -> nx, ip -> ny);
	    else
		strcpy (iname, ip -> inst_name);

	    sprintf (cname, "@%s@", ip->templ->cell_name);

	    xx1 = Max ((float) ip -> bbxl, XL);
	    yy1 = Max ((float) ip -> bbyb + (0.6 * ch_h), YB);

	    /* can we print the characters horizontally?? */
	    textsize = Max (strlen (iname), strlen (cname));

	    rightmost = xx1 + (textsize * ch_w);
	    topmost = yy1 + (2 * ch_h);
	    if (rightmost < XR && rightmost < ip->bbxr &&
		  topmost < YT && topmost < ip->bbyt) { /* yes, it fits completely: do it horizontally */
		d_text (xx1, yy1, cname);
		d_text (xx1, yy1 + ch_h, iname);
	    }
	    else { /* does it fit vertically ? */
		topmost = yy1 + (textsize * ch_h);
		rightmost = xx1 + (2 * ch_w);
		if (topmost < YT && topmost < ip->bbyt && rightmost < XR && rightmost < ip->bbxr) { /* yes */
		    v_text (xx1, yy1, iname);
		    if (xx1 + (3 * ch_w) < ip->bbxr) v_text (xx1 + (1.6*ch_w), yy1, cname);
		}
	    }
	}

	if (vis_arr[NR_lay + 1] && vis_arr[NR_lay + 5] && ip -> t_draw == TRUE) {
	/*
	 ** Subterm-flag 'visible', subterm_char-flag 'visible'
	 ** and subterminals of this instance 'to be drawn'.
	 **
	 ** Names of terminals of (0,0) occurrence of instance.
	 */
	    if (ip -> templ -> t_flag == FALSE) {
		exp_templ (ip -> templ, dmproject, ip -> imported, READ_TERM);
		ASSERT (ip -> templ -> t_flag == TRUE);
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
 			if (tp -> nx > 0 || tp -> ny > 0) {
 			    for (j = 0; j <= tp -> ny; ++j)
 			    for (i = 0; i <= tp -> nx; ++i) {
 				if (tp -> nx > 0 && tp -> ny > 0)
 				    sprintf (iname, "%s[%d,%d]", tp -> tmname, i, j);
 				else if (tp -> nx > 0)
 				    sprintf (iname, "%s[%d]", tp -> tmname, i);
 				else
 				    sprintf (iname, "%s[%d]", tp -> tmname, j);
 				d_text ((float) trans[0] + i*tp->dx, (float) trans[2] + j*tp->dy, iname);
 			    }
 			}
 			else
			    d_text ((float) trans[0], (float) trans[2], tp -> tmname);
		    }
		}
	    }
	}
    }
}

void term_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register Coor xst, yst;
    register int it, jt, i;
    char    str[MAXSTR+20];
    char   *str_rep_p;
    register TERM *tpntr;

    if (!vis_arr[NR_lay] || !vis_arr[NR_lay + 4]) {
	return; /* terminals or term-chars not visible */
    }

    for (i = 0; i < NR_lay; ++i) {
	for (tpntr = term_root[i]; tpntr; tpntr = tpntr -> nxttm) {
	    strcpy (str, tpntr -> tmname);
	    if (tpntr -> nx || tpntr -> ny)
		str_rep_p = str + strlen (str);
	    else
		str_rep_p = NULL;
	    for (jt = 0; jt <= tpntr -> ny; ++jt)
	    for (it = 0; it <= tpntr -> nx; ++it) {
		xst = tpntr -> xl + it * tpntr -> dx;
		yst = tpntr -> yb + jt * tpntr -> dy;
		if (xst >= wxl && xst <= wxr && yst >= wyb && yst <= wyt) {
		    if (str_rep_p) {
			if (tpntr -> nx && tpntr -> ny)
			    sprintf (str_rep_p, "[%d,%d]", it, jt);
			else
			    sprintf (str_rep_p, "[%d]", tpntr -> nx ? it : jt);
		    }
		    d_text ((float) xst, (float) yst, str);
		}
	    }
	}
    }
}

void dis_s_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    register int i, j, it, jt;
    Coor trans[4];
    INST *inst_p;
    register TERM *tp;

    if (!vis_arr[NR_lay + 1]) return;

    ggSetColor (lay);
    d_fillst (FILL_HASHED);

    for (inst_p = inst_root; inst_p; inst_p = inst_p -> next) {
	if (inst_p -> t_draw == FALSE) continue;
	if (inst_outside_window (inst_p, wxl, wxr, wyb, wyt)) continue;
	if (vis_arr[NR_lay + 2] && vis_arr[lay] && inst_p -> level > 1) {
	/*
	 ** Terminals have already been drawn when
	 ** displaying the expansion information.
	 */
	    continue;
	}

	if (inst_p -> templ -> t_flag == FALSE) {
	    exp_templ (inst_p -> templ, dmproject, inst_p -> imported, READ_TERM);
	    ASSERT (inst_p -> templ -> t_flag == TRUE);
	}

	for (tp = inst_p -> templ -> term_p[lay]; tp; tp = tp -> nxttm) {
	    for (i = 0; i <= inst_p -> ny; ++i)
	    for (j = 0; j <= inst_p -> nx; ++j) {
		for (it = 0; it <= tp -> ny; ++it)
		for (jt = 0; jt <= tp -> nx; ++jt) {
		    trans[0] = tp -> xl + jt * tp -> dx;
		    trans[1] = tp -> xr + jt * tp -> dx;
		    trans[2] = tp -> yb + it * tp -> dy;
		    trans[3] = tp -> yt + it * tp -> dy;
		    trans_box (trans, trans, inst_p, j, i);

		    if (trans[0] < wxr && trans[1] > wxl && trans[2] < wyt && trans[3] > wyb) {
			if (CHECK_FLAG == FALSE) {
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
    }
}
