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
extern int  NR_lay;
extern int  Textnr;
extern int  cmd_nbr, new_cmd;
extern int  erase_text;
extern int *term_lay;
extern int *vis_arr;
extern int  v_sterm, v_inst;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, ybc; /* cursor parameters */
extern char *ImageName; /* inst_name of image (Sea-of-Gates) */

void rot_inst ()
{
    INST *ip;
    char mess_str[MAXCHAR];
    Coor ddx, ddy, ll, rr, bb, tt;
    register int lay;

    if (!present_inst ()) return;

    ptext ("Select an instance to be rotated!");
again:
    do {
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    /* Sea-of-Gates: the image can't be rotated! */
    if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) {
	btext ("The image cannot be rotated!");
	return;
    }

    inst_window (ip, &ll, &rr, &bb, &tt);

    ddx = ip -> tr[2];
    ddy = ip -> tr[5];
    ip -> tr[2] = (Trans) (ip -> bbxl - ddy + ip -> bbyt);
    ip -> tr[5] = (Trans) (ip -> bbyb + ddx - ip -> bbxl);

    ddx = ip -> bbxr - ip -> bbxl;
    ddy = ip -> bbyt - ip -> bbyb;
    ip -> bbxr = ip -> bbxl + ddy;
    ip -> bbyt = ip -> bbyb + ddx;

    /* update the matrix (rotate CCW) */
    if (ip -> tr[0]) {
	ip -> tr[1] = -ip -> tr[4];
	ip -> tr[3] = ip -> tr[0];
	ip -> tr[0] = ip -> tr[4] = 0;
    }
    else {
	ip -> tr[0] = -ip -> tr[3];
	ip -> tr[4] = ip -> tr[1];
	ip -> tr[1] = ip -> tr[3] = 0;
    }

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (ip -> level > 1 && vis_arr[v_inst]) { /* inst. visible */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_lay[lay]) pict_arr[lay] = ERAS_DR;
    }
    inst_window (ip, &piwl, &piwr, &piwb, &piwt);
    if (rr > piwr) piwr = rr;
    if (tt > piwt) piwt = tt;
    sprintf (mess_str, "Instance %s(%s) rotated, select another?",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);
    picture ();
    goto again;
}

void mov_inst ()
{
    INST *ip;
    char mess_str[MAXCHAR];
    Coor ddx, ddy, ll, rr, bb, tt;
    register int lay;

    if (!present_inst ()) return;

    ptext ("Select an instance to be moved!");
again:
    do {
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    /* Sea-of-Gates: the image can't be moved! */
    if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) {
	btext ("The image cannot be moved!");
	return;
    }

    inst_window (ip, &ll, &rr, &bb, &tt);
    set_bbox_loc (ll, rr, bb, tt);

    sprintf (mess_str, "Instance %s(%s), enter new position!",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);
    erase_text = 1;
    if ((new_cmd = get_cursor (6, 1, SNAP)) != -1) return;

    ddx = ll - xlc;
    ddy = bb - ybc;
    if (ddx == 0 && ddy == 0) goto again;
    ip -> bbxl -= ddx;
    ip -> bbxr -= ddx;
    ip -> bbyb -= ddy;
    ip -> bbyt -= ddy;
    ip -> tr[2] -= (Trans) ddx;
    ip -> tr[5] -= (Trans) ddy;

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (ip -> level > 1 && vis_arr[v_inst]) { /* inst. visible */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_lay[lay]) pict_arr[lay] = ERAS_DR;
    }
    inst_window (ip, &piwl, &piwr, &piwb, &piwt);
    if (ll < piwl) piwl = ll;
    if (rr > piwr) piwr = rr;
    if (bb < piwb) piwb = bb;
    if (tt > piwt) piwt = tt;

    sprintf (mess_str, "Instance %s(%s) at (%ld, %ld), select another?",
	ip -> inst_name, ip -> templ -> cell_name,
	(long) (xlc / QUAD_LAMBDA), (long) (ybc / QUAD_LAMBDA));
    ptext (mess_str);
    picture ();
    goto again;
}

void mir_inst ()
{
    static char *alt_arr[] = { /* static for redraw */
    /* 0 */ "-cancel-",
    /* 1 */ "x",
    /* 2 */ "y",
    };
    INST *ip;
    char mess_str[MAXCHAR];
    int axis;
    register int lay;

    if (!present_inst ()) return;

    ptext ("Select an instance to be mirrored!");
again:
    do {
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    /* Sea-of-Gates: the image can't be mirrored! */
    if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) {
	btext ("The image cannot be mirrored!");
	return;
    }

    sprintf (mess_str, "Instance %s(%s), select mirror axis!",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);
    erase_text = 1;
    save_menu ();
    if ((axis = ask (3, alt_arr, -1)) <= 0) return;
    prev_menu ();

    if (axis == 2) { /* around y-as */
	ip -> tr[2] = (Trans) (ip -> bbxl + ip -> bbxr - (Coor) ip -> tr[2]);
	ip -> tr[0] = -ip -> tr[0];
	ip -> tr[1] = -ip -> tr[1];
    }
    else if (axis == 1) { /* around x-as */
	ip -> tr[5] = (Trans) (ip -> bbyb + ip -> bbyt - (Coor) ip -> tr[5]);
	ip -> tr[3] = -ip -> tr[3];
	ip -> tr[4] = -ip -> tr[4];
    }

    if (ip -> level > 1 && vis_arr[v_inst]) { /* inst. visible */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
	pict_arr[Textnr] = ERAS_DR; /* names */
    }
    else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_lay[lay]) pict_arr[lay] = ERAS_DR;
	pict_arr[Textnr] = ERAS_DR; /* term-names */
    }
    inst_window (ip, &piwl, &piwr, &piwb, &piwt);
    sprintf (mess_str, "Instance %s(%s) mirrored, select another?",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);
    picture ();
    goto again;
}

void set_actinst (int mode)
{
    INST *ip;
    char mess_str[MAXCHAR];

    if (!present_inst ()) return;

    if (mode)
	ptext ("Select instance point to show tree on 'stdout'!");
    else
	ptext ("Select an instance to show and set name!");
again:
    if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	erase_text = 1;
	return;
    }
    if (mode)
	ip = cell_tree (xlc, ybc);
    else
	ip = search_inst (xlc, ybc);

    if (ip)
	sprintf (mess_str, "Instance is %s(%s), select another?", ip -> inst_name, ip -> templ -> cell_name);
    else
	sprintf (mess_str, "No instance selected, select one?");
    ptext (mess_str);
    goto again;
}

void set_inst_name ()
{
    INST *ip;

    if (!present_inst ()) return;

    do {
	ptext ("Select an instance to be renamed!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    /* Sea-of-Gates: the image can't be renamed! */
    if (ImageName && strcmp (ip -> inst_name, ImageName) == 0) {
	btext ("The image cannot be renamed!");
	return;
    }
    ask_inst (ip);
    new_cmd = cmd_nbr;
}

void upd_inst_bbox ()
{
    INST *ip;
    DM_PROJECT *projkey;
    char  mess_str[MAXCHAR];
    char *cellname;
    Coor  arr[4];
    int   i;

    if (!present_inst ()) return;

    ptext ("Select an instance to update bbox!");
again:
    do {
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    projkey = dmFindProjKey (ip -> imported, ip -> templ -> cell_name, dmproject, &cellname, LAYOUT);
    if (!projkey) return;

    if (read_bound_box (projkey, cellname, &arr[0], &arr[1], &arr[2], &arr[3]) == -1) return;

    inst_window (ip, &piwl, &piwr, &piwb, &piwt);

    trans_box (arr, arr, ip, 0, 0);
    i = 0;
    if (ip -> bbxl != arr[0]) { ++i; ip -> bbxl = arr[0]; }
    if (ip -> bbxr != arr[1]) { ++i; ip -> bbxr = arr[1]; }
    if (ip -> bbyb != arr[2]) { ++i; ip -> bbyb = arr[2]; }
    if (ip -> bbyt != arr[3]) { ++i; ip -> bbyt = arr[3]; }
    if (i) {
	inst_window (ip, &arr[0], &arr[1], &arr[2], &arr[3]);
	if (arr[0] < piwl) piwl = arr[0];
	if (arr[1] > piwr) piwr = arr[1];
	if (arr[2] < piwb) piwb = arr[2];
	if (arr[3] > piwt) piwt = arr[3];
	pict_arr[Textnr] = ERAS_DR;
	sprintf (mess_str, "Bounding box updated by %s(%s)!", ip -> inst_name, ip -> templ -> cell_name);
    }
    else {
	sprintf (mess_str, "No update needed by %s(%s)!", ip -> inst_name, ip -> templ -> cell_name);
    }
    ptext (mess_str);
    picture ();
    goto again;
}
