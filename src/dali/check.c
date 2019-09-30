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

#define EXT_WINDOW 12

extern struct drc **drc_data; /* design rule information */
extern int  CHECK_FLAG;
extern int  NR_lay;
extern int  DRC_nr;
extern int  new_cmd;
extern int  erase_text;
extern Coor xlc, xrc, ybc, ytc;
extern Coor piwl, piwr, piwb, piwt;
extern Coor Q_xl, Q_xr, Q_yb, Q_yt; /* quad search window */
extern qtree_t **quad_root;

struct drc *p_drc;
struct edgetree **prelist;
struct bintree  **y_root;

/* temporary list of pointers to slanting edges */
struct p_to_edge   *templist;

struct x_lst **checklist;
struct x_lst **checkp;

static int lay_present;
static int count; /* a count of the number of design rule errors */
static short   S; /* specified tolerance value */

static void do_internal_check (int lay);
static void drc_error (struct f_edge *p);
static void width_check (struct x_lst *p, struct x_lst *q, struct f_edge *fp, struct f_edge *fq, struct f_edge *short_d);

/*
** Check part of the layout for design rule errors.
*/
void area_check ()
{
    register int lay;

    r_design_rules (); /* read drc file for checker */

    if (!drc_data) {
	ptext ("No drc information available!");
	return;
    }
    else {
	ptext ("Put cursor over check area!");
	erase_text = 1;
	if ((new_cmd = get_cursor (5, 2, NO_SNAP)) != -1) return;
    }

    if (!prelist) {
	MALLOCN (prelist, struct edgetree *, NR_lay);
	MALLOCN (y_root , struct bintree  *, NR_lay);
	MALLOCN (checklist, struct x_lst  *, NR_lay);
	MALLOCN (checkp   , struct x_lst  *, NR_lay);
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	prelist[lay] = NULL;
	y_root[lay] = NULL;
	checklist[lay] = NULL;
	checkp[lay] = NULL;
    }
    /*
    ** Find all edges in the area of interest.
    **
    ** Enlarge search window a bit: this ensures
    ** that no false errors will occur on the edges
    ** of the check area. Errors found within the enlarged
    ** window but outside the check area are not reported.
    */
    piwl = xlc - EXT_WINDOW;
    piwr = xrc + EXT_WINDOW;
    piwb = ybc - EXT_WINDOW;
    piwt = ytc + EXT_WINDOW;

    ptext ("*** checking ***");
    count = 0; /* no errors found so far */

    CHECK_FLAG = 1; /* ON */

    /* Call the display routine.
    ** Trapezoids are redirected (check_flag = TRUE)
    */
    for (lay = 0; lay < NR_lay; ++lay) {
	if (!drc_data[lay]) continue;
	templist = NULL;

	disp_mask_quad (lay, piwl, piwr, piwb, piwt);
	dis_term (lay, piwl, piwr, piwb, piwt);
	draw_inst_window (lay, piwl, piwr, piwb, piwt);
	dis_s_term (lay, piwl, piwr, piwb, piwt);

	/* make true edges for this mask layer */
	if (prelist[lay]) make_true_edges (lay);
	if (checklist[lay]) do_internal_check (lay);
    }

    CHECK_FLAG = 0; /* OFF */

    if (count)
	ptext ("Errors found! (single layer check)");
    else
	ptext ("No errors found. (single layer check)");
}

/*
** Check edges in checklist for design rule errors.
*/
static void do_internal_check (int lay)
{
    struct f_edge fp, fq, short_d;
    struct x_lst *search, *pi, *pn, *pw, *worklist;

    p_drc = drc_data[lay];

    /* calculate S: the maximum distance specified
    ** in the design rules for actual mask layer
    */
    S = Max (p_drc -> gap, p_drc -> exgap);
    S = Max (S, p_drc -> overlap);

    worklist = NULL;

    /* first edge in input list into worklist */
    pw = pi = checklist[lay];

    while (pi) {
	pn = pi -> next;

	/* add pi to worklist */
	pi -> back = NULL;
	pi -> next = worklist;
	if (worklist) {
	    worklist -> back = pi;
	    if (pi -> ye < pw -> ye) pw = pi;
	}
	else pw = pi;
	worklist = pi;

	pi = pn; /* new edge from input list */

	while (!pi || pi -> ys >= pw -> ye + S) { /* pi -> ys = minimum y-coordinate into input list */
	    /*
	    ** pw is the vector with the minimum ye into the worklist
	    */
	    for (search = worklist; search; search = search -> next) {
		if (search != pw) width_check (search, pw, &fp, &fq, &short_d);
	    }

	    /* remove current vector from worklist */
	    if (pw -> next) pw -> next -> back = pw -> back;
	    if (pw -> back) pw -> back -> next = pw -> next;
	    else worklist = pw -> next;
	    FREE (pw);

	    if (!worklist) break; /* worklist is empty */

	    /* search for new minimum ye vector */
	    pw = worklist;
	    for (search = worklist -> next; search; search = search -> next) {
		if (search -> ye < pw -> ye) pw = search;
	    }
	}
    }
}

void set_occ (struct obj_node *q)
{
    lay_present = 1; /* YES */
}

static float length (struct x_lst *p)
{
    return (dis ((float) p -> xs, (float) p -> ys, (float) p -> xe, (float) p -> ye));
}

/*
** Check the edges p and q.
** INPUT: pointers to the edges.
*/
static void width_check (struct x_lst *p, struct x_lst *q, struct f_edge *fp, struct f_edge *fq, struct f_edge *short_d)
{
    Coor pxmin, qxmin, pxmax, qxmax;
    int  d;

    if (p -> side == q -> side) return; /* the edges are from the same side */

    if (Abs (p -> dir - q -> dir) == 2) return; /* the edges are orthogonal */

    pxmin = Min (p -> xs, p -> xe);
    qxmin = Min (q -> xs, q -> xe);
    pxmax = Max (p -> xs, p -> xe);
    qxmax = Max (q -> xs, q -> xe);

    /* check for x and y ranges */
    if (pxmax <= (qxmin - S)) return;
    if (pxmin >= (qxmax + S)) return;
    if (p -> ye <= (q -> ys - S)) return;
    if (p -> ys >= (q -> ye + S)) return;

    /* if the lines are connected: return */
    if ((p -> xs == q -> xs) && (p -> ys == q -> ys)) return;
    if ((p -> xs == q -> xe) && (p -> ys == q -> ye)) return;
    if ((p -> xe == q -> xs) && (p -> ye == q -> ys)) return;
    if ((p -> xe == q -> xe) && (p -> ye == q -> ye)) return;

    /* convert to float */
    fp -> xs = (float) p -> xs;
    fp -> ys = (float) p -> ys;
    fp -> xe = (float) p -> xe;
    fp -> ye = (float) p -> ye;
    fp -> dir = p -> dir;
    fp -> side = p -> side;

    fq -> xs = (float) q -> xs;
    fq -> ys = (float) q -> ys;
    fq -> xe = (float) q -> xe;
    fq -> ye = (float) q -> ye;
    fq -> dir = q -> dir;
    fq -> side = q -> side;

    /* calculate distance between the edges */
    d = distance (fp, fq, short_d);

    /* Is one of the edges horizontal? */
    if (p -> dir == 2 || q -> dir == 2) {

        if (pxmax < qxmin || qxmax < pxmin) return;

	if ((p -> ys < q -> ys && p -> side == RIGHT) ||
	    (q -> ys < p -> ys && q -> side == RIGHT)) {
	    if (d < p_drc -> overlap) drc_error (short_d);
	    return;
	}
	if (length (p) <= p_drc -> exlength ||
	    length (q) <= p_drc -> exlength) {
	    if (d < p_drc -> exgap) drc_error (short_d);
	    return;
	}
	if (d < p_drc -> gap) drc_error (short_d);
	return;
    }
    if ((pxmin < qxmin && p -> side == LEFT) ||
	(qxmin < pxmin && q -> side == LEFT)) {
	if (d < p_drc -> overlap) drc_error (short_d);
	return;
    }

    /*
    ** If one of the following conditions holds, the check to
    ** be carried out can be as well a gap as an width check.
    ** To see which check has to be carried out, the tree is searched
    ** to see if an object is present in the indicated area.
    ** If TRUE it is a width check, which has already been
    ** carried out elsewhere; else the gap_check that
    ** follows is carried out. (J. Liedorp)
    */
    if (pxmax < qxmin) {
	Q_xl = pxmax;
	Q_xr = qxmin;
        if (p -> ys > q -> ye) {
	    Q_yb = q -> ye;
	    Q_yt = p -> ys;
	    lay_present = 0; /* NO */
	    quad_search (quad_root[p -> layer], set_occ);
	    if (lay_present) return;
        }
        if (p -> ye < q -> ys) {
	    Q_yb = p -> ye;
	    Q_yt = q -> ys;
	    lay_present = 0; /* NO */
	    quad_search (quad_root[p -> layer], set_occ);
	    if (lay_present) return;
        }
    }
    if (qxmax < pxmin) {
	Q_xl = qxmax;
	Q_xr = pxmin;
        if (p -> ys > q -> ye) {
	    Q_yb = q -> ye;
	    Q_yt = p -> ys;
	    lay_present = 0; /* NO */
	    quad_search (quad_root[p -> layer], set_occ);
	    if (lay_present) return;
        }
        if (p -> ye < q -> ys) {
	    Q_yb = p -> ye;
	    Q_yt = q -> ys;
	    lay_present = 0; /* NO */
	    quad_search (quad_root[p -> layer], set_occ);
	    if (lay_present) return;
        }
    }

    if (length (p) <= p_drc -> exlength || length (q) <= p_drc -> exlength) {
	if (d < p_drc -> exgap) drc_error (short_d);
	return;
    }
    if (d < p_drc -> gap) drc_error (short_d);
}

/*
** Reflect a design rule error on the screen.
** INPUT: the coordinates of the two conflicting points.
** Alternative would be to let this routine just store
** the error and incorporate the display of the errors
** in the normal picture procedure (extra flag).
** This would permit repeated display of the errors. (PvdW)
*/
static void drc_error (struct f_edge *p)
{
    /*
    ** Does the error intersect the check area?
    */
    if (p -> xs < (float) xlc || p -> xs > (float) xrc) return;
    if (p -> xe < (float) xlc || p -> xe > (float) xrc) return;
    if (p -> ys < (float) ybc || p -> ys > (float) ytc) return;
    if (p -> ye < (float) ybc || p -> ye > (float) ytc) return;

    if (++count == 1) { /* first error to be displayed */
	set_c_wdw (PICT);
    }

    ggSetColor (DRC_nr);
    disp_mode (COMPLEMENT);

    d_ltype (LINE_DOUBLE);
    d_line (p -> xs, p -> ys, p -> xe, p -> ye);
    d_ltype (LINE_SOLID);

    d_fillst (FILL_HOLLOW);
    d_circle (p -> xs, p -> ys, p -> xe, p -> ye);
    d_fillst (FILL_SOLID);

    disp_mode (TRANSPARENT);
}
