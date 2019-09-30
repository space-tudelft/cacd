/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/extern.h"

static int first_mc = 1;

void mc_warning ()
{
	first_mc = 0;

  if (h_x_mode == -1) {
    h_x_mode = 0;
    if (x_mode == -1) {
	x_mode = 0;
	P_E "%s: WARNING: Using no-origin mode (= new default)!\n", argv0);
	P_E "%s: WARNING: Header contains no info about mode!\n", argv0);
	P_E "%s: WARNING: Use option -x for origin mode!\n", argv0);
    }
  }
  else if (h_x_mode != x_mode) {
    if (x_mode <= 0) {
      if (h_x_mode == 1) {
	P_E "%s: WARNING: Using no-origin mode%s!\n", argv0,
	    x_mode == -1 ? " (= new default)" : "");
	P_E "%s: WARNING: Header says to use origin mode!\n", argv0);
	P_E "%s: WARNING: You must use option -x!\n", argv0);
      }
	x_mode = 0;
    }
    else {
	P_E "%s: WARNING: Using origin mode!\n", argv0);
	P_E "%s: WARNING: Header says to use no-origin mode!\n", argv0);
	P_E "%s: WARNING: You must not use option -x!\n", argv0);
    }
  }
}

void proc_mc (int inst, int mir, int rot)
{
    IMPCELL * ic;
    DM_PROJECT * proj;
    DM_CELL * mc_key;
    DM_STREAM * fp_mc_info;
    long b_xl, b_xr, b_yb, b_yt, tmp;
    register long a, b, c, d;

    if (first_mc) mc_warning ();

    if (!check_tree (mc_name, mod_tree)) {
	pr_exit (014, 10, mc_name);/* not found */
	return;
    }
    if (tree_ptr -> errflag == 1) {
	pr_exit (014, 52, mc_name);/* model def. error */
	return;
    }

    ic = tree_ptr -> impcell;

    if (tree_ptr -> bbox) {
	b_xl = tree_ptr -> bbox -> xl;
	b_xr = tree_ptr -> bbox -> xr;
	b_yb = tree_ptr -> bbox -> yb;
	b_yt = tree_ptr -> bbox -> yt;
    }
    else {
	if (ic) {
	    proj = dmOpenProject (ic -> dmpath, PROJ_READ);
	    mc_key = dmCheckOut (proj, ic -> cellname,
		    ACTUAL, DONTCARE, LAYOUT, READONLY);
	}
	else {
	    mc_key = dmCheckOut (dmproject, mc_name,
		    ACTUAL, DONTCARE, LAYOUT, READONLY);
	}

	fp_mc_info = dmOpenStream (mc_key, "info", "r");

	if (dmGetDesignData (fp_mc_info, GEO_INFO) <= 0)
	    pr_exit (0137, 28, mc_name);

	ALLOC (tree_ptr -> bbox, mod_bbox);
	tree_ptr -> bbox -> xl = b_xl = ginfo.bxl;
	tree_ptr -> bbox -> xr = b_xr = ginfo.bxr;
	tree_ptr -> bbox -> yb = b_yb = ginfo.byb;
	tree_ptr -> bbox -> yt = b_yt = ginfo.byt;

	dmCloseStream (fp_mc_info, COMPLETE);
	dmCheckIn (mc_key, COMPLETE);
    }

    gmc.imported = ic ? IMPORTED : LOCAL;

    if (!inst) strcpy (instance, ".");

    a = b = c = d = 0;

    switch (rot) {		/* rotate (degrees) */
	case 0:
	    ++a; ++d; break;
	case 90:
	    --b; ++c; break;
	case 180:
	    --a; --d; break;
	case 270:
	    ++b; --c; break;
    }

    switch (mir) {		/* mirror */
	case 1:
	    b = -b; d = -d; break;
	case 2:
	    a = -a; c = -c; break;
    }

    if (sfx > 1) { b_xl *= sfx; b_xr *= sfx; }
    if (sfy > 1) { b_yb *= sfy; b_yt *= sfy; }

    tmp = b_xl;
    b_xl = a * tmp + b * b_yb;
    b_yb = c * tmp + d * b_yb;
    tmp = b_xr;
    b_xr = a * tmp + b * b_yt;
    b_yt = c * tmp + d * b_yt;
    if (b_xl > b_xr) {
	tmp = b_xl;
	b_xl = b_xr;
	b_xr = tmp;
    }
    if (b_yb > b_yt) {
	tmp = b_yb;
	b_yb = b_yt;
	b_yt = tmp;
    }

 /* translate and copy r_box */
 /* this version normally uses a modified modelcall def */

    if (x_mode) { /* origin mode */
	tx -= b_xl;
	ty -= b_yb;
    }

    b_xl += tx + (dx < 0 ? (dx * nx) : 0);
    b_xr += tx + (dx > 0 ? (dx * nx) : 0);
    b_yb += ty + (dy < 0 ? (dy * ny) : 0);
    b_yt += ty + (dy > 0 ? (dy * ny) : 0);

    strcpy (gmc.cell_name, mc_name);
    strcpy (gmc.inst_name, instance);

    gmc.mtx[0] = sfx * a;
    gmc.mtx[1] = sfy * b;
    gmc.mtx[2] = tx;
    gmc.mtx[3] = sfx * c;
    gmc.mtx[4] = sfy * d;
    gmc.mtx[5] = ty;
    gmc.bxl = b_xl;
    gmc.bxr = b_xr;
    gmc.byb = b_yb;
    gmc.byt = b_yt;
    gmc.dx = dx;
    gmc.nx = nx;
    gmc.dy = dy;
    gmc.ny = ny;

    dmPutDesignData (fp_mc, GEO_MC);

    if (ini_mcbbox) {		/* update modelcall bounding box */
	mcbb_xl = b_xl;
	mcbb_xr = b_xr;
	mcbb_yb = b_yb;
	mcbb_yt = b_yt;
	ini_mcbbox = 0;
    }
    else {
	if (mcbb_xl > b_xl) mcbb_xl = b_xl;
	if (mcbb_xr < b_xr) mcbb_xr = b_xr;
	if (mcbb_yb > b_yb) mcbb_yb = b_yb;
	if (mcbb_yt < b_yt) mcbb_yt = b_yt;
    }
}

void proc_cif_mc (int inst, int x0, int x1, int x2, int y0, int y1, int y2)
{
    IMPCELL * ic;
    DM_PROJECT * proj;
    DM_CELL * mc_key;
    DM_STREAM * fp_mc_info;
    long b_xl, b_xr, b_yb, b_yt, tmp;
    long a, b, c, d;

    a = x0;
    b = y0;
    c = x1;
    d = y1;
    tx = x2;
    ty = y2;

    if (first_mc) mc_warning ();

    if (!check_tree (mc_name, mod_tree)) {
	pr_exit (014, 10, mc_name);/* not found */
	return;
    }
    if (tree_ptr -> errflag == 1) {
	pr_exit (014, 52, mc_name);/* model def. error */
	return;
    }

    ic = tree_ptr -> impcell;

    if (tree_ptr -> bbox) {
	b_xl = tree_ptr -> bbox -> xl;
	b_xr = tree_ptr -> bbox -> xr;
	b_yb = tree_ptr -> bbox -> yb;
	b_yt = tree_ptr -> bbox -> yt;
    }
    else {
	if (ic) {
	    proj = dmOpenProject (ic -> dmpath, PROJ_READ);
	    mc_key = dmCheckOut (proj, ic -> cellname,
		    ACTUAL, DONTCARE, LAYOUT, READONLY);
	}
	else {
	    mc_key = dmCheckOut (dmproject, mc_name,
		    ACTUAL, DONTCARE, LAYOUT, READONLY);
	}

	fp_mc_info = dmOpenStream (mc_key, "info", "r");

	if (dmGetDesignData (fp_mc_info, GEO_INFO) <= 0)
	    pr_exit (0137, 28, mc_name);

	ALLOC (tree_ptr -> bbox, mod_bbox);
	tree_ptr -> bbox -> xl = b_xl = ginfo.bxl;
	tree_ptr -> bbox -> xr = b_xr = ginfo.bxr;
	tree_ptr -> bbox -> yb = b_yb = ginfo.byb;
	tree_ptr -> bbox -> yt = b_yt = ginfo.byt;

	dmCloseStream (fp_mc_info, COMPLETE);
	dmCheckIn (mc_key, COMPLETE);
    }

    gmc.imported = ic ? IMPORTED : LOCAL;

    if (!inst) strcpy (instance, ".");

    tmp = b_xl;
    b_xl = a * tmp + b * b_yb;
    b_yb = c * tmp + d * b_yb;
    tmp = b_xr;
    b_xr = a * tmp + b * b_yt;
    b_yt = c * tmp + d * b_yt;
    if (b_xl > b_xr) {
	tmp = b_xl;
	b_xl = b_xr;
	b_xr = tmp;
    }
    if (b_yb > b_yt) {
	tmp = b_yb;
	b_yb = b_yt;
	b_yt = tmp;
    }

 /* translate and copy r_box */
 /* this version normally uses a modified modelcall def */

    if (x_mode) { /* origin mode */
	tx -= b_xl;
	ty -= b_yb;
    }

    b_xl += tx + (dx < 0 ? (dx * nx) : 0);
    b_xr += tx + (dx > 0 ? (dx * nx) : 0);
    b_yb += ty + (dy < 0 ? (dy * ny) : 0);
    b_yt += ty + (dy > 0 ? (dy * ny) : 0);

    strcpy (gmc.cell_name, mc_name);
    strcpy (gmc.inst_name, instance);

    gmc.mtx[0] = a;
    gmc.mtx[1] = b;
    gmc.mtx[2] = tx;
    gmc.mtx[3] = c;
    gmc.mtx[4] = d;
    gmc.mtx[5] = ty;
    gmc.bxl = b_xl;
    gmc.bxr = b_xr;
    gmc.byb = b_yb;
    gmc.byt = b_yt;
    gmc.dx = dx;
    gmc.nx = nx;
    gmc.dy = dy;
    gmc.ny = ny;

    dmPutDesignData (fp_mc, GEO_MC);

    if (ini_mcbbox) {		/* update modelcall bounding box */
	mcbb_xl = b_xl;
	mcbb_xr = b_xr;
	mcbb_yb = b_yb;
	mcbb_yt = b_yt;
	ini_mcbbox = 0;
    }
    else {
	if (mcbb_xl > b_xl) mcbb_xl = b_xl;
	if (mcbb_xr < b_xr) mcbb_xr = b_xr;
	if (mcbb_yb > b_yb) mcbb_yb = b_yb;
	if (mcbb_yt < b_yt) mcbb_yt = b_yt;
    }
}
