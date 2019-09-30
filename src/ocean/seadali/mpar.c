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
extern INST *act_inst;
extern INST *inst_root;
extern char ImageInstName[];
extern int  NR_lay;
extern int  Textnr;
extern int *term_bool;
extern int *vis_arr;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern char *ThisImage;             /* name identifier of image */
extern struct Disp_wdw *c_wdw;

void rot_inst ()
{
    Trans hulp0, hulp1;
    register int lay;
    Coor ddx, ddy;
    Coor ll, rr, bb, tt;

    if (!act_inst) {
	ptext ("Set your actual instance first!");
	return;
    }
    inst_window (act_inst, &piwl, &piwr, &piwb, &piwt);

    ddx = act_inst -> bbxr - act_inst -> bbxl;
    ddy = act_inst -> bbyt - act_inst -> bbyb;
    hulp0 = act_inst -> tr[2];
    act_inst -> tr[2] = (Trans) (act_inst -> bbxl + act_inst -> bbyt - (Coor) act_inst -> tr[5]);
    act_inst -> tr[5] = (Trans) (act_inst -> bbyb + (Coor) hulp0 - act_inst -> bbxl);
    act_inst -> bbxr = act_inst -> bbxl + ddy;
    act_inst -> bbyt = act_inst -> bbyb + ddx;

    /* update the matrix */
    hulp0 = act_inst -> tr[0];
    hulp1 = act_inst -> tr[1];
    act_inst -> tr[0] = -act_inst -> tr[3];
    act_inst -> tr[1] = -act_inst -> tr[4];
    act_inst -> tr[3] = hulp0;
    act_inst -> tr[4] = hulp1;

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (act_inst -> level > 1 && vis_arr[NR_lay + 2]) {
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (act_inst -> t_draw == TRUE && vis_arr[NR_lay + 1]) {
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_bool[lay]) pict_arr[lay] = ERAS_DR;
    }
    inst_window (act_inst, &ll, &rr, &bb, &tt);
    piwr = Max (piwr, rr);
    piwt = Max (piwt, tt);
}

void mov_inst (Coor x1, Coor y1)
{
    char  err_str[MAXCHAR];
    register int lay;
    Coor ddx, ddy;
    Coor ll, rr, bb, tt;

    if (!act_inst) {
	ptext ("Set your actual instance first!");
	return;
    }
    inst_window (act_inst, &piwl, &piwr, &piwb, &piwt);
    ddx = act_inst -> bbxl - x1;
    ddy = act_inst -> bbyb - y1;
    act_inst -> bbxl = x1;
    act_inst -> bbxr -= ddx;
    act_inst -> bbyb = y1;
    act_inst -> bbyt -= ddy;
    act_inst -> tr[2] -= (Trans) ddx;
    act_inst -> tr[5] -= (Trans) ddy;

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (act_inst -> level > 1 && vis_arr[NR_lay + 2]) {
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (act_inst -> t_draw == TRUE && vis_arr[NR_lay + 1]) {
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_bool[lay]) pict_arr[lay] = ERAS_DR;
    }
    inst_window (act_inst, &ll, &rr, &bb, &tt);
    piwl = Min (piwl, ll);
    piwr = Max (piwr, rr);
    piwb = Min (piwb, bb);
    piwt = Max (piwt, tt);

    sprintf (err_str, "%s at %ld, %ld", act_inst -> templ -> cell_name,
	(long) (x1 / QUAD_LAMBDA), (long) (y1 / QUAD_LAMBDA));
    ptext (err_str);

    picture ();
    snap_instance_on_image (act_inst);
}

/*
 * Patrick: added x_implicit
 */
void mir_inst (int implicit) /* implicit is TRUE if image has only one way of morroring */
{
    int axis;
    register int lay;
    static char *alt_arr[3]; /* static for redraw */

    alt_arr[0] = "-cancel-";
    alt_arr[1] = "x";
    alt_arr[2] = "y";

    if (!act_inst) {
	ptext ("Set your actual instance first!");
	return;
    }

    axis = -1;
    if (implicit == TRUE && strcmp (ThisImage, "fishbone") == 0) {
       axis = 1; /* fishbone image: always around x-axis */
    }
    if (implicit == TRUE &&
       (strcmp (ThisImage, "gatearray") == 0 || strcmp (ThisImage, "pm25") == 0)) {
       axis = 2; /* gatearray image: always around y-axis */
    }

    if (axis == -1) /* default, not set */
       if ((axis = ask (3, alt_arr, -1)) <= 0) return;

    if (axis == 2) { /* around y-as */
	act_inst -> tr[2] = (Trans) (act_inst -> bbxl +
		act_inst -> bbxr - (Coor) act_inst -> tr[2]);
	act_inst -> tr[0] = -act_inst -> tr[0];
	act_inst -> tr[1] = -act_inst -> tr[1];
    }
    else if (axis == 1) { /* around x-as */
	act_inst -> tr[5] = (Trans) (act_inst -> bbyb +
		act_inst -> bbyt - (Coor) act_inst -> tr[5]);
	act_inst -> tr[3] = -act_inst -> tr[3];
	act_inst -> tr[4] = -act_inst -> tr[4];
    }
    else {
	ptext ("I only accept x or y!");
	return;
    }

    if (act_inst -> level > 1 && vis_arr[NR_lay + 2]) {
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	pict_arr[Textnr] = ERAS_DR; /* sub_sub_bound.b */
    }
    else if (act_inst -> t_draw == TRUE && vis_arr[NR_lay + 1]) {
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_bool[lay]) pict_arr[lay] = ERAS_DR;
	pict_arr[Textnr] = ERAS_DR; /* characters */
    }
    inst_window (act_inst, &piwl, &piwr, &piwb, &piwt);
}

void set_actinst (Coor x1, Coor y1)
{
    act_inst = search_inst (x1, y1);
}

void upd_inst_bbox ()
{
    DM_PROJECT *projkey;
    char *cellname;
    Coor tmp_arr[4];

    if (!act_inst) {
	ptext ("Set your actual instance first!");
	return;
    }

    projkey = dmFindProjKey (act_inst -> imported, act_inst -> templ -> cell_name, dmproject, &cellname, LAYOUT);
    if (!projkey) return;

    if (read_bound_box (projkey, cellname, tmp_arr, tmp_arr + 1, tmp_arr + 2, tmp_arr + 3) == -1) return;

    inst_window (act_inst, &piwl, &piwr, &piwb, &piwt);

    trans_box (tmp_arr, tmp_arr, act_inst, 0, 0);
    act_inst -> bbxl = tmp_arr[0];
    act_inst -> bbxr = tmp_arr[1];
    act_inst -> bbyb = tmp_arr[2];
    act_inst -> bbyt = tmp_arr[3];
    inst_window (act_inst, &(tmp_arr[0]), &(tmp_arr[1]), &(tmp_arr[2]), &(tmp_arr[3]));
    piwl = Min (piwl, tmp_arr[0]);
    piwr = Max (piwr, tmp_arr[1]);
    piwb = Min (piwb, tmp_arr[2]);
    piwt = Max (piwt, tmp_arr[3]);
    pict_arr[Textnr] = ERAS_DR;
}

void mov_insts (double xlmul, double ybmul)
{
    register int lay;
    INST *inst;
    Coor ddx;
    Coor ddy;
    Coor tmp_arr[4];
    int first = 1;

    tmp_arr[0] = piwl; /* init to suppress compile warning */
    tmp_arr[1] = piwr;
    tmp_arr[2] = piwb;
    tmp_arr[3] = piwt;

    for (inst = inst_root; inst; inst = inst -> next) {
        if (strcmp (inst -> inst_name, ImageInstName) != 0) {
            inst_window (inst, &piwl, &piwr, &piwb, &piwt);
            if (first) {
	        tmp_arr[0] = piwl;
	        tmp_arr[1] = piwr;
	        tmp_arr[2] = piwb;
	        tmp_arr[3] = piwt;
                first = 0;
            }
            else {
	        tmp_arr[0] = Min (piwl, tmp_arr[0]);
	        tmp_arr[1] = Max (piwr, tmp_arr[1]);
	        tmp_arr[2] = Min (piwb, tmp_arr[2]);
	        tmp_arr[3] = Max (piwt, tmp_arr[3]);
            }

            ddx = (Coor) (inst -> tr[2] * xlmul) - (Coor) (inst -> tr[2]);
            ddy = (Coor) (inst -> tr[5] * ybmul) - (Coor) (inst -> tr[5]);
	    inst -> bbxl += ddx;
	    inst -> bbxr += ddx;
	    inst -> bbyb += ddy;
	    inst -> bbyt += ddy;
	    inst -> tr[2] += (Trans) ddx;
	    inst -> tr[5] += (Trans) ddy;

            snap_instance_on_image (inst);
	    tmp_arr[0] = Min (piwl, tmp_arr[0]);
	    tmp_arr[1] = Max (piwr, tmp_arr[1]);
	    tmp_arr[2] = Min (piwb, tmp_arr[2]);
	    tmp_arr[3] = Max (piwt, tmp_arr[3]);
        }
    }

    pict_arr[Textnr] = ERAS_DR;	/* redraw instances */
    if (vis_arr[NR_lay + 2]) {
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[NR_lay + 1]) {
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_bool[lay]) pict_arr[lay] = ERAS_DR;
    }

    piwl = tmp_arr[0];
    piwr = tmp_arr[1];
    piwb = tmp_arr[2];
    piwt = tmp_arr[3];
    picture ();
}
