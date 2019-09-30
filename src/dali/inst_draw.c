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

extern Coor Q_xl, Q_xr, Q_yb, Q_yt; /* quad search window */
extern DM_PROJECT *dmproject;
extern INST *inst_root;
extern int  CHECK_FLAG;
extern int  CLIP_FLAG;
extern int  Draw_hashed;
extern int  NR_lay;
extern int  act_mask_lay;
extern int *fillst;
extern int *vis_arr;
extern int  v_inst, v_bbox;

/* Sea-of-Gates additions: */
extern char *ImageName;  /* instance name which is not drawn */
extern int  MaxDrawImage; /* maximum number of images on the screen */

Trans *Mtx; /* ptr to transf.matrix, external used by "sub_to_checker" */

static Coor S_xl, S_xr, S_yb, S_yt; /* Search window */

static void disp_exp_term (TERM *tp);
static void instance_draw (INST *p, int level, Trans mtx[], DM_PROJECT *pkey);
static void paint_sub_bb (INST *ip);

/*
** Draw a specified part of the window.
** INPUT: the window coordinates.
*/
void draw_inst_window (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    register INST *ip;
    Trans matrix[6];

    if (!(ip = inst_root)) return; /* no instances */

    if (!vis_arr[v_inst]) return; /* instances not visible */

    if ((act_mask_lay = lay) >= 0) {
	if (!vis_arr[lay]) return; /* layer not visible */
	ggSetColor (lay);
	if (Draw_hashed && fillst[lay] != FILL_HOLLOW) {
	    /* draw instances in a lower intensity */
	    d_fillst (FILL_HASHED25);
	}
	else
	    d_fillst (fillst[lay]);
    }
    else if (!vis_arr[v_bbox]) return; /* bboxes not visible */

    S_xl = xmin; S_xr = xmax;
    S_yb = ymin; S_yt = ymax;
    matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
    matrix[3] = 0; matrix[4] = 1; matrix[5] = 0;

    do { /* for all instances */
	if (ip -> level > 1 && !inst_outside_window (ip, xmin, xmax, ymin, ymax)) {
	    /*
	    ** instance intersect the window at its father level
	    */
	    instance_draw (ip, ip -> level, matrix, dmproject);
	}
    } while ((ip = ip -> next));
}

/*
** Put a trapezoid of a child cell on the screen
** if it intersects the search range "Q_searchwindow".
** INPUT: a pointer to the trapezoid, the accumulated
** transformation matrix of the cell and the display window.
*/
void paint_sub_trap (struct obj_node *p)
{
    Coor x0, x1, x2, x3, y1, y2, dy;

    if (!p -> sides) {
	mtx_draw_box (p -> ll_x1, p -> ll_x2, p -> ll_y1, p -> ll_y2);
	return;
    }

    x0 = x1 = p -> ll_x1;
    x3 = x2 = p -> ll_x2;
    y1 = p -> ll_y1;
    y2 = p -> ll_y2;
    dy = y2 - y1;

    switch (p -> sides >> 2) { /* leftside */
    case 0: break;
    case 1: x1 += dy; break;
    case 2: x0 += dy; break;
    default:
	btext ("Illegal trapezoid!");
	return;
    }

    switch (p -> sides & 3) { /* rightside */
    case 0: break;
    case 1: x3 -= dy; break;
    case 2: x2 -= dy; break;
    default:
	btext ("Illegal trapezoid!");
	return;
    }

    mtx_draw_trap (x0, x1, x2, x3, y1, y2);
}

static void instance_draw (INST *p, int level, Trans mtx[], DM_PROJECT *pkey)
{
    int     nx1, nx2, ny1, ny2;
    register int nx, ny;
    float   f_n1, f_n2, s_xl, s_xr, s_yb, s_yt, z;
    Coor    ll, rr, bb, tt;
    Trans   new_mtx[6], t_x, t_y;
    register INST *ip;
    qtree_t *qt_lay;

    if (!p -> templ -> quad_trees[0]) { /* template is empty: read it */
	if (exp_templ (p -> templ, pkey, p -> imported, READ_ALL))
	    return; /* read error */
    }

    nx1 = 0; nx2 = 0; ny1 = 0; ny2 = 0;

    if (p -> nx || p -> ny) {
	/*
	** Repetition: first determine lower- and
	** upperbounds for repetition numbers.
	*/
	/*
	** Sea-of-Gates:
	** Do not draw image if there are too many on the screen.
	*/
	if (ImageName && strcmp (p -> inst_name, ImageName) == 0) {
	    nx = (S_xr - S_xl) * (S_yt - S_yb);
	    ny = (p -> bbxr - p -> bbxl) * (p -> bbyt - p -> bbyb);
	    if (Abs (nx / ny) > MaxDrawImage) return;
	}

	/* calculate search window at this instance level */
	if (mtx[0]) { /* r0 or r180 */
	    s_xl = S_xl - mtx[2]; s_yb = S_yb - mtx[5];
	    s_xr = S_xr - mtx[2]; s_yt = S_yt - mtx[5];
	    s_xl /= mtx[0]; s_yb /= mtx[4];
	    s_xr /= mtx[0]; s_yt /= mtx[4];
	}
	else {
	    s_yb = S_xl - mtx[2]; s_xl = S_yb - mtx[5];
	    s_yt = S_xr - mtx[2]; s_xr = S_yt - mtx[5];
	    s_xl /= mtx[3]; s_yb /= mtx[1];
	    s_xr /= mtx[3]; s_yt /= mtx[1];
	}
	if (s_xr < s_xl) { z = s_xl; s_xl = s_xr; s_xr = z; }
	if (s_yt < s_yb) { z = s_yb; s_yb = s_yt; s_yt = z; }

	/*
	** The following must hold:
	**  p -> bbxr + nx1 * dx > s_xl
	**  p -> bbyt + ny1 * dy > s_yb
	**  p -> bbxl + nx2 * dx < s_xr
	**  p -> bbyb + ny2 * dy < s_yt
	*/
	if (p -> nx) {
	    f_n1 = (s_xl - (float) p -> bbxr) / (float) p -> dx;
	    f_n2 = (s_xr - (float) p -> bbxl) / (float) p -> dx;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((nx1 = UpperRound (f_n1)) < 0) nx1 = 0;
	    if ((nx2 = LowerRound (f_n2)) > p -> nx) nx2 = p -> nx;
	    if (nx1 > nx2) return;
	}
	if (p -> ny) {
	    f_n1 = (s_yb - (float) p -> bbyt) / (float) p -> dy;
	    f_n2 = (s_yt - (float) p -> bbyb) / (float) p -> dy;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((ny1 = UpperRound (f_n1)) < 0) ny1 = 0;
	    if ((ny2 = LowerRound (f_n2)) > p -> ny) ny2 = p -> ny;
	    if (ny1 > ny2) return;
	}
    }

    new_mtx[0] = mtx[0] * p -> tr[0] + mtx[1] * p -> tr[3];
    new_mtx[1] = mtx[0] * p -> tr[1] + mtx[1] * p -> tr[4];
    new_mtx[3] = mtx[3] * p -> tr[0] + mtx[4] * p -> tr[3];
    new_mtx[4] = mtx[3] * p -> tr[1] + mtx[4] * p -> tr[4];

    pkey = p -> templ -> projkey;
    if (act_mask_lay >= 0) qt_lay = p -> templ -> quad_trees[act_mask_lay];
    else qt_lay = 0;

    t_x = p -> tr[2] + nx1 * p -> dx;
    for (nx = nx1;;) {
	t_y = p -> tr[5] + ny1 * p -> dy;
	for (ny = ny1;;) {
	    /* calculate search window at this level */
	    Mtx = new_mtx;
	    new_mtx[2] = mtx[2] + t_x * mtx[0] + t_y * mtx[1];
	    new_mtx[5] = mtx[5] + t_x * mtx[3] + t_y * mtx[4];

	    if (new_mtx[0]) { /* R0 or R180 */
		s_xl = (float) (S_xl - new_mtx[2]) / new_mtx[0];
		s_yb = (float) (S_yb - new_mtx[5]) / new_mtx[4];
		s_xr = (float) (S_xr - new_mtx[2]) / new_mtx[0];
		s_yt = (float) (S_yt - new_mtx[5]) / new_mtx[4];
	    }
	    else { /* R90 or R270 */
		s_yb = (float) (S_xl - new_mtx[2]) / new_mtx[1];
		s_xl = (float) (S_yb - new_mtx[5]) / new_mtx[3];
		s_yt = (float) (S_xr - new_mtx[2]) / new_mtx[1];
		s_xr = (float) (S_yt - new_mtx[5]) / new_mtx[3];
	    }
	    if (s_xr < s_xl) { z = s_xl; s_xl = s_xr; s_xr = z; }
	    if (s_yt < s_yb) { z = s_yb; s_yb = s_yt; s_yt = z; }

	    Q_xl = ll = LowerRound (s_xl);
	    Q_xr = rr = UpperRound (s_xr);
	    Q_yb = bb = LowerRound (s_yb);
	    Q_yt = tt = UpperRound (s_yt);

	    /*
	    ** Output instance mask data and terminal data:
	    */
	    if (act_mask_lay >= 0) {
		if (CHECK_FLAG)
		    quad_search (qt_lay, sub_to_checker);
		else if (CLIP_FLAG)
		    quad_search (qt_lay, clip_paint);
		else
		    quad_search (qt_lay, paint_sub_trap);

		disp_exp_term (p -> templ -> term_p[act_mask_lay]);
	    }

	    /*
	    ** Now we will dive into the recursion:
	    ** this will affect our global information
	    ** which we should not use anymore.
	    ** Search window is only read and not updated
	    ** in this (recursive) routine.
	    ** Mtx and Q_searchwindow are always set before (re)use,
	    ** either in this call itself within the
	    ** surrounding for-loop and in all lower ones.
	    */
	    for (ip = p -> templ -> inst; ip; ip = ip -> next) {

		/* Sea-of-Gates:
		** Do not paint the images under sub-cells.
		*/
		if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) continue;

		if (!inst_outside_window (ip, ll, rr, bb, tt)) {
		    /*
		    ** instance intersect the window at its father level
		    */
		    if (level > 2) {
			instance_draw (ip, level - 1, new_mtx, pkey);
		    }
		    else if (act_mask_lay == -1 && !CHECK_FLAG) {
			/* bbox at lowest level */
			paint_sub_bb (ip);
		    }
		}
	    }
	    if (++ny > ny2) break;
	    t_y += p -> dy;
	}
	if (++nx > nx2) break;
	t_x += p -> dx;
    }
}

static void disp_exp_term (TERM *tp)
{
    Coor xmin, ymin, xmax, ymax, z;
    Coor xl, xr, yb, yt;
    register int i, j;

    if (tp) do {
	xl = tp -> xl;
	xr = tp -> xr;
	for (i = 0;;) {
	    if (xl > Q_xr || xr < Q_xl) goto next_x;
	    yb = tp -> yb;
	    yt = tp -> yt;
	    for (j = 0;;) {
		if (yb > Q_yt || yt < Q_yb) goto next_y;

		xmin = Mtx[0] * xl + Mtx[1] * yb + Mtx[2];
		xmax = Mtx[0] * xr + Mtx[1] * yt + Mtx[2];
		ymin = Mtx[3] * xl + Mtx[4] * yb + Mtx[5];
		ymax = Mtx[3] * xr + Mtx[4] * yt + Mtx[5];
		if (xmax < xmin) { z = xmin; xmin = xmax; xmax = z; }
		if (ymax < ymin) { z = ymin; ymin = ymax; ymax = z; }

		/* clip against draw window */
		if (xmin < S_xl) xmin = S_xl;
		if (ymin < S_yb) ymin = S_yb;
		if (xmax > S_xr) xmax = S_xr;
		if (ymax > S_yt) ymax = S_yt;

		if (CHECK_FLAG)
		    to_precheck (act_mask_lay, xmin, xmax, ymin, ymax);
		else
		    paint_box ((float) xmin, (float) xmax, (float) ymin, (float) ymax);
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

static void paint_sub_bb (INST *ip) /* bbox at lowest level */
{
    Coor xl, xr, yb, yt;
    register int i, j;

    xl = ip -> bbxl;
    xr = ip -> bbxr;
    for (i = 0;;) {
	if (xl > Q_xr || xr < Q_xl) goto next_x;
	yb = ip -> bbyb;
	yt = ip -> bbyt;
	for (j = 0;;) {
	    if (yb > Q_yt || yt < Q_yb) goto next_y;
	    pict_rect ( (float) (Mtx[0] * xl + Mtx[1] * yb + Mtx[2]),
			(float) (Mtx[0] * xr + Mtx[1] * yt + Mtx[2]),
			(float) (Mtx[3] * xl + Mtx[4] * yb + Mtx[5]),
			(float) (Mtx[3] * xr + Mtx[4] * yt + Mtx[5]));
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
}
