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
extern short CHECK_FLAG;
extern short CLIP_FLAG;
extern int  NR_lay;
extern int  act_mask_lay;
extern int *fillst;
extern int *vis_arr;
extern int  interrupt_flag;

/* PATRICK */
extern char ImageInstName[];   /* instance name which is not drawn */
extern int  MaxDrawImage; /* maximum number of images on the screen */
int Draw_hashed;
extern int  ImageMode;
/* END PATRICK */

Trans *Mtx;	/* pointer to transformation matrix.
		External used by "sub_to_checker" */

static struct obj_node *srch;
static Coor srch_wndw[4];

static void disp_exp_term (TERM *term_p);
static void instance_draw (INST *p, int level, Trans mtx[], DM_PROJECT *key);
static void paint_sub_bb  (INST *p);

/*
** Draw a specified part of the window.
** INPUT: the window coordinates.
*/
void draw_inst_window (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    INST *inst_p;
    Trans matrix[6];
    struct obj_node srch_struct;

    if (!vis_arr[NR_lay + 2]) return; /* instances not visible */
    if (lay == -1) {
	if (!vis_arr[NR_lay + 3]) return; /* bounding boxes not visible */
    }
    else if (!vis_arr[lay]) return; /* layer not visible */

    srch = &srch_struct; /* will be used later in recursive procedure */

    srch_wndw[0] = xmin;
    srch_wndw[1] = ymin;
    srch_wndw[2] = xmax;
    srch_wndw[3] = ymax;

    matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
    matrix[3] = 0; matrix[4] = 1; matrix[5] = 0;

    act_mask_lay = lay;

    if (act_mask_lay != -1) {
	ggSetColor (lay);
	if (Draw_hashed == TRUE) { /* draw instances in a lower intensity */
	    if (fillst[lay] != FILL_HOLLOW)
		d_fillst (FILL_HASHED25);
	    else
		d_fillst (fillst[lay]); /* keep (hollow) */
	}
	else
	    d_fillst (fillst[lay]);
    }

    for (inst_p = inst_root; inst_p; inst_p = inst_p -> next) {
	/* does instance intersect the window at its father level */
	if (!inst_outside_window (inst_p, xmin, xmax, ymin, ymax)) {
	    instance_draw (inst_p, inst_p -> level, matrix, dmproject);
	}
    }
}

/* key is project key of father template
*/
static void instance_draw (INST *p, int level, Trans mtx[], DM_PROJECT *key)
{
    int     nx, ny;
    int     nx1, nx2, ny1, ny2;
    float   f_n1, f_n2, f_nh;
    Coor    bbxl, bbyb, bbxr, bbyt;
    Coor    local_wndw[4];
    Trans   determinant;
    Trans   new_mtx[6];
    Trans   inv_mtx[4];
    INST   *inst_p;

    if (level == 1) return; /* no more levels to draw */

    if (!p -> templ -> quad_trees[0]) { /* template is empty: read it */
	exp_templ (p -> templ, key, p -> imported, READ_ALL);
    }

    nx1 = nx2 = 0;
    ny1 = ny2 = 0;

    if (p -> nx != 0 || p -> ny != 0) {
	/*
	 * Repetition: first determine lower- and
	 * upperbounds for repetition numbers.
	 */

/* PATRICK: do not draw image if there are too many on the screen */
	if (ImageMode == TRUE && strcmp (p->inst_name, ImageInstName) == 0) {
	 /* d_fillst (fillst[act_mask_lay]); */

	    if (Abs (((srch_wndw[2] - srch_wndw[0]) * (srch_wndw[3] - srch_wndw[1])) /
		((p->bbxr - p->bbxl) * (p->bbyt - p->bbyb))) > MaxDrawImage) {
	    /*
		fprintf (stderr, "INSTANCE skipped: %d %d %d %d / %d %d %d %d  ratio = %f\n",
		    (int) srch_wndw[0], (int) srch_wndw[2], (int) srch_wndw[1], (int) srch_wndw[3],
		    (int) p->bbxl, (int) p->bbxr, (int) p->bbyb, (int) p->bbyt,
		    (float) (Abs (srch_wndw[0] - srch_wndw[2]) * Abs (srch_wndw[1] - srch_wndw[3])) /
			    (Abs (p->bbxl - p->bbxr) * Abs (p->bbyt - p->bbyb)));
	    */
		return;
	    }
	}
/* END PATRICK */

	/* calculate inverse transformation matrix */
	determinant = mtx[0] * mtx[4] - mtx[1] * mtx[3];

	ASSERT (determinant == 1 || determinant == -1);

	inv_mtx[0] = determinant * mtx[4];
	inv_mtx[1] = -determinant * mtx[1];
	inv_mtx[2] = -determinant * mtx[3];
	inv_mtx[3] = determinant * mtx[0];

	/* calculate search window at this instance level */
	bbxl = inv_mtx[0] * (srch_wndw[0] - mtx[2]) + inv_mtx[1] * (srch_wndw[1] - mtx[5]);
	bbyb = inv_mtx[2] * (srch_wndw[0] - mtx[2]) + inv_mtx[3] * (srch_wndw[1] - mtx[5]);
	bbxr = inv_mtx[0] * (srch_wndw[2] - mtx[2]) + inv_mtx[1] * (srch_wndw[3] - mtx[5]);
	bbyt = inv_mtx[2] * (srch_wndw[2] - mtx[2]) + inv_mtx[3] * (srch_wndw[3] - mtx[5]);

	local_wndw[0] = Min (bbxl, bbxr);
	local_wndw[1] = Min (bbyb, bbyt);
	local_wndw[2] = Max (bbxl, bbxr);
	local_wndw[3] = Max (bbyb, bbyt);

	/*
	 * The following must hold:
	 * p -> bbxr + nx1 * dx > local_wndw[0]
	 * p -> bbyt + ny1 * dy > local_wndw[1]
	 * p -> bbxl + nx2 * dx < local_wndw[2]
	 * p -> bbyb + ny2 * dy < local_wndw[3]
	 */
	if (p -> nx != 0) {
	    f_n1 = (float) (local_wndw[0] - p -> bbxr) / (float) p -> dx;
	    f_n2 = (float) (local_wndw[2] - p -> bbxl) / (float) p -> dx;

	    if (p -> dx < 0) {
		/* Swap boundaries. */
		f_nh = f_n1;
		f_n1 = f_n2;
		f_n2 = f_nh;
	    }
	    ASSERT (f_n1 < f_n2);

	    nx1 = Max (0, (int) UpperRound (f_n1));
	    nx2 = Min (p -> nx, (int) LowerRound (f_n2));
	}
	if (p -> ny != 0) {
	    f_n1 = (float) (local_wndw[1] - p -> bbyt) / (float) p -> dy;
	    f_n2 = (float) (local_wndw[3] - p -> bbyb) / (float) p -> dy;

	    if (p -> dy < 0) {
		/* Swap boundaries. */
		f_nh = f_n1;
		f_n1 = f_n2;
		f_n2 = f_nh;
	    }
	    ASSERT (f_n1 < f_n2);

	    ny1 = Max (0, (int) UpperRound (f_n1));
	    ny2 = Min (p -> ny, (int) LowerRound (f_n2));
	}

	ASSERT ((nx1 <= nx2 && ny1 <= ny2) || inst_outside_window (p, srch_wndw[0], srch_wndw[2], srch_wndw[1], srch_wndw[3]));
    }

    new_mtx[0] = mtx[0] * p -> tr[0] + mtx[1] * p -> tr[3];
    new_mtx[1] = mtx[0] * p -> tr[1] + mtx[1] * p -> tr[4];
    new_mtx[3] = mtx[3] * p -> tr[0] + mtx[4] * p -> tr[3];
    new_mtx[4] = mtx[3] * p -> tr[1] + mtx[4] * p -> tr[4];

    /* calculate inverse transformation matrix */
    determinant = new_mtx[0] * new_mtx[4] - new_mtx[1] * new_mtx[3];

    ASSERT (determinant == 1 || determinant == -1);

    inv_mtx[0] =  determinant * new_mtx[4];
    inv_mtx[1] = -determinant * new_mtx[1];
    inv_mtx[2] = -determinant * new_mtx[3];
    inv_mtx[3] =  determinant * new_mtx[0];

    for (nx = nx1; nx <= nx2; ++nx)
    for (ny = ny1; ny <= ny2; ++ny) {
	Mtx = new_mtx;
	new_mtx[2] = mtx[2] + (p -> tr[2] + nx * p -> dx) * mtx[0] + (p -> tr[5] + ny * p -> dy) * mtx[1];
	new_mtx[5] = mtx[5] + (p -> tr[2] + nx * p -> dx) * mtx[3] + (p -> tr[5] + ny * p -> dy) * mtx[4];

	/* calculate search window at this level */
	bbxl = inv_mtx[0] * (srch_wndw[0] - new_mtx[2]) + inv_mtx[1] * (srch_wndw[1] - new_mtx[5]);
	bbyb = inv_mtx[2] * (srch_wndw[0] - new_mtx[2]) + inv_mtx[3] * (srch_wndw[1] - new_mtx[5]);
	bbxr = inv_mtx[0] * (srch_wndw[2] - new_mtx[2]) + inv_mtx[1] * (srch_wndw[3] - new_mtx[5]);
	bbyt = inv_mtx[2] * (srch_wndw[2] - new_mtx[2]) + inv_mtx[3] * (srch_wndw[3] - new_mtx[5]);

	local_wndw[0] = srch -> ll_x1 = Min (bbxl, bbxr);
	local_wndw[1] = srch -> ll_y1 = Min (bbyb, bbyt);
	local_wndw[2] = srch -> ll_x2 = Max (bbxl, bbxr);
	local_wndw[3] = srch -> ll_y2 = Max (bbyb, bbyt);

	if (act_mask_lay != -1) {
	    if (CHECK_FLAG == FALSE) {
		quad_search (p -> templ -> quad_trees[act_mask_lay], srch, paint_sub_trap);
	    }
	    else {
		quad_search (p -> templ -> quad_trees[act_mask_lay], srch, sub_to_checker);
	    }
	    disp_exp_term (p -> templ -> term_p[act_mask_lay]);
	}

	/*
	** Now we will dive into the recursion:
	** this will affect our global information
	** which we should not use anymore.
	** srch_wndw[] is only read and not updated
	** in this (recursive) routine.
	** Mtx and srch are always set before (re)use,
	** either in this call itself within the
	** surrounding for-loop and in all lower ones.
	*/
	ASSERT (level >= 1);

	for (inst_p = p -> templ -> inst; inst_p; inst_p = inst_p -> next) {

	    /* patrick: do not paint the images under sub-cells */
	    if (ImageMode == TRUE && strcmp (inst_p->inst_name, ImageInstName) == 0) continue;

	    /* Does instance intersect the window at its father level?
	    */
	    if (!inst_outside_window (inst_p, local_wndw[0], local_wndw[2], local_wndw[1], local_wndw[3])) {
		if (level - 1 > 1) {
		    instance_draw (inst_p, level - 1, new_mtx, p -> templ -> projkey);
		}
		else if (act_mask_lay == -1 && CHECK_FLAG == FALSE) {
		    paint_sub_bb (inst_p);
		}
	    }
	}
    }
}

/*
** Put a trapezoid of a child cell on the screen
** if it intersects the search range "srch".
** INPUT: a pointer to the trapezoid, the accumulated
** transformation matrix of the cell and the display window.
*/
void paint_sub_trap (struct obj_node *p)
{
    Coor line_x[4], line_y[4];
    Coor x0, x1, x2, x3, y1, y2;
    Coor xmin, xmax, ymin, ymax;

    if (stop_drawing () == TRUE) return;

    if (p -> leftside == 0 && p -> rightside == 0) {

	x1 = Mtx[0] * p -> ll_x1 + Mtx[1] * p -> ll_y1 + Mtx[2];
	y1 = Mtx[3] * p -> ll_x1 + Mtx[4] * p -> ll_y1 + Mtx[5];
	x2 = Mtx[0] * p -> ll_x2 + Mtx[1] * p -> ll_y2 + Mtx[2];
	y2 = Mtx[3] * p -> ll_x2 + Mtx[4] * p -> ll_y2 + Mtx[5];

	xmin = Min (x1, x2);
	xmax = Max (x1, x2);
	ymin = Min (y1, y2);
	ymax = Max (y1, y2);

	/* clip against draw window */

	xmin = Max (xmin, srch_wndw[0]);
	xmax = Min (xmax, srch_wndw[2]);
	ymin = Max (ymin, srch_wndw[1]);
	ymax = Min (ymax, srch_wndw[3]);

	paint_box ((float) xmin, (float) xmax, (float) ymin, (float) ymax);
	return;
    }

    if (CLIP_FLAG) {
	clip_paint (p, srch -> ll_x1, srch -> ll_x2, srch -> ll_y1, srch -> ll_y2, Mtx, SUB_TRAP);
	return;
    }

    /*
    ** Calculate the corner points of the trapezoid:
    */
    y1 = p -> ll_y1;
    y2 = p -> ll_y2;
    ymin = y2 - y1;

    switch (p -> leftside) {
    case 0:
	x0 = x3 = p -> ll_x1;
	break;
    case 1:
	x0 = p -> ll_x1;
	x3 = x0 + ymin;
	break;
    case -1:
	x3 = p -> ll_x1;
	x0 = x3 + ymin;
	break;
    default:
	ptext ("Illegal trapezoid!");
	return;
    }

    switch (p -> rightside) {
    case 0:
	x1 = x2 = p -> ll_x2;
	break;
    case 1:
	x2 = p -> ll_x2;
	x1 = x2 - ymin;
	break;
    case -1:
	x1 = p -> ll_x2;
	x2 = x1 - ymin;
	break;
    default:
	ptext ("Illegal trapezoid!");
	return;
    }

    /*
    ** Multiply the trapezoid with
    ** the accumulated transformation matrix
    */
    line_x[0] = x0 * Mtx[0] + y1 * Mtx[1] + Mtx[2];
    line_y[0] = x0 * Mtx[3] + y1 * Mtx[4] + Mtx[5];
    line_x[1] = x1 * Mtx[0] + y1 * Mtx[1] + Mtx[2];
    line_y[1] = x1 * Mtx[3] + y1 * Mtx[4] + Mtx[5];
    line_x[2] = x2 * Mtx[0] + y2 * Mtx[1] + Mtx[2];
    line_y[2] = x2 * Mtx[3] + y2 * Mtx[4] + Mtx[5];
    line_x[3] = x3 * Mtx[0] + y2 * Mtx[1] + Mtx[2];
    line_y[3] = x3 * Mtx[3] + y2 * Mtx[4] + Mtx[5];

    /* paint the trapezoid on the screen */
    draw_trap (line_x, line_y);
}

static void disp_exp_term (TERM *term_p)
{
    Coor x1, y1, x2, y2;
    Coor xmin, ymin, xmax, ymax;
    Coor txl, txr, tyb, tyt;
    register int i, j;

    for (; term_p; term_p = term_p -> nxttm) {

	for (i = 0; i <= term_p -> nx; ++i) {
	    for (j = 0; j <= term_p -> ny; ++j) {

		txl = term_p -> xl + i * term_p -> dx;
		txr = term_p -> xr + i * term_p -> dx;
		tyb = term_p -> yb + j * term_p -> dy;
		tyt = term_p -> yt + j * term_p -> dy;

		if (txl > srch -> ll_x2 || tyb > srch -> ll_y2 ||
		    txr < srch -> ll_x1 || tyt < srch -> ll_y1) continue;

		x1 = Mtx[0] * txl + Mtx[1] * tyb + Mtx[2];
		y1 = Mtx[3] * txl + Mtx[4] * tyb + Mtx[5];
		x2 = Mtx[0] * txr + Mtx[1] * tyt + Mtx[2];
		y2 = Mtx[3] * txr + Mtx[4] * tyt + Mtx[5];

		xmin = Min (x1, x2);
		xmax = Max (x1, x2);
		ymin = Min (y1, y2);
		ymax = Max (y1, y2);

		/* clip against draw window */
		xmin = Max (xmin, srch_wndw[0]);
		xmax = Min (xmax, srch_wndw[2]);
		ymin = Max (ymin, srch_wndw[1]);
		ymax = Min (ymax, srch_wndw[3]);

		if (CHECK_FLAG == TRUE) {
		    to_precheck (act_mask_lay, xmin, xmax, ymin, ymax);
		    continue;
		}

		paint_box ((float) xmin, (float) xmax, (float) ymin, (float) ymax);
	    }
	}
    }
}

static void paint_sub_bb (INST *p)
{
    Coor xl, yb, xr, yt;
    Coor x1, x2, y1, y2;
    register int i, j;

/* PATRICK: for sea of gates: no paint of bb of image, that drives you crazy! */
    if (ImageMode == TRUE && strcmp (p->inst_name, ImageInstName) == 0) return;
/* END PATRICK */

    for (i = 0; i <= p -> nx; ++i)
    for (j = 0; j <= p -> ny; ++j) {

	x1 = p -> bbxl + i * p -> dx;
	x2 = p -> bbxr + i * p -> dx;
	y1 = p -> bbyb + j * p -> dy;
	y2 = p -> bbyt + j * p -> dy;

	xl = Mtx[0] * x1 + Mtx[1] * y1 + Mtx[2];
	yb = Mtx[3] * x1 + Mtx[4] * y1 + Mtx[5];
	xr = Mtx[0] * x2 + Mtx[1] * y2 + Mtx[2];
	yt = Mtx[3] * x2 + Mtx[4] * y2 + Mtx[5];

	if (xl <= srch_wndw[2] && xr >= srch_wndw[0] && yb <= srch_wndw[3] && yt >= srch_wndw[1]) {
	    pict_rect ((float) xl, (float) xr, (float) yb, (float) yt);
	}
    }
}

#ifdef OLD
static eprint (struct obj_node *p)
{
    PE "%d %d %d %d %d %d\n", p -> ll_x1, p -> ll_y1, p -> ll_x2, p -> ll_y2, p -> leftside, p -> rightside);
}
#endif /* OLD */
