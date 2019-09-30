/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
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

#include "src/cmsk/extern.h"

static int box_type = 0;

int roundX (double d)
{
    int i = (d < 0) ? (d - 0.5001) : (d + 0.5001);
    return (i);
}

int roundL (double d)
{
    int i = (d < 0) ? (d - 0.9995) : (d + 0.0005);
    return (i);
}

int roundh (double d)
{
    int i = (d < 0) ? (d - 0.0005) : (d + 0.9995);
    return (i);
}

char to_buf[20];

char *d2a (double d)
{
    sprintf (to_buf, "%g", resol * d);
    return (to_buf);
}

void pr_pat_call (char *name) /* print pattern call */
{
    IMPCELL    *ic;
    DM_PROJECT *proj;
    DM_CELL    *key;
    DM_STREAM  *fp;
    register int a, b, c, d;

    if ((ic = tree_ptr -> impcell)) {
	gmc.imported = IMPORTED;
    }
    else {
	gmc.imported = LOCAL;
    }

    if (tree_ptr -> bbox) {
	callbox[0] = tree_ptr -> bbox -> xl;
	callbox[1] = tree_ptr -> bbox -> xr;
	callbox[2] = tree_ptr -> bbox -> yb;
	callbox[3] = tree_ptr -> bbox -> yt;
    }
    else {
	if (ic) {
	    proj = dmOpenProject (ic -> dmpath, PROJ_READ);
	    key = dmCheckOut (proj, ic -> cellname,
			ACTUAL, DONTCARE, LAYOUT, READONLY);
	}
	else {
	    key = dmCheckOut (dmproject, name,
			ACTUAL, DONTCARE, LAYOUT, READONLY);
	}

	fp = dmOpenStream (key, "info", "r");
	dmGetDesignData (fp, GEO_INFO);

	ALLOC_STRUCT (tree_ptr -> bbox, pat_bbox);
	tree_ptr -> bbox -> xl = callbox[0] = ginfo.bxl;
	tree_ptr -> bbox -> xr = callbox[1] = ginfo.bxr;
	tree_ptr -> bbox -> yb = callbox[2] = ginfo.byb;
	tree_ptr -> bbox -> yt = callbox[3] = ginfo.byt;

	dmCloseStream (fp, COMPLETE);
	dmCheckIn (key, COMPLETE);
    }

    a = b = c = d = 0;

    switch (rotateflag) {
	case   0: ++a; ++d; break;
	case  90: --b; ++c; break;
	case 180: --a; --d; break;
	case 270: ++b; --c; break;
    }

    if (mirrorflag) {
	if (mirrorflag == 1) { /* MX */
	    b = -b;
	    d = -d;
	}
	else { /* MY */
	    a = -a;
	    c = -c;
	}
    }

    temp1 = callbox[0];
    callbox[0] = a * temp1 + b * callbox[2];
    callbox[2] = c * temp1 + d * callbox[2];
    temp1 = callbox[1];
    callbox[1] = a * temp1 + b * callbox[3];
    callbox[3] = c * temp1 + d * callbox[3];

    if (callbox[0] > callbox[1]) {
	temp1 = callbox[0];
	callbox[0] = callbox[1];
	callbox[1] = temp1;
    }
    if (callbox[2] > callbox[3]) {
	temp1 = callbox[2];
	callbox[2] = callbox[3];
	callbox[3] = temp1;
    }

    callbox[0] += x_origin;
    callbox[1] += x_origin;
    callbox[2] += y_origin;
    callbox[3] += y_origin;

    gmc.mtx[0] = a;
    gmc.mtx[1] = b;
    gmc.mtx[2] = x_origin;
    gmc.mtx[3] = c;
    gmc.mtx[4] = d;
    gmc.mtx[5] = y_origin;

    if (cxnumber) {
	gmc.nx = cxnumber;
	gmc.dx = cxdistance;
	temp1 = cxnumber * gmc.dx;
	if (temp1 < 0)
	    callbox[0] += temp1;
	else
	    callbox[1] += temp1;
    }
    else {
	gmc.nx = 0;
	gmc.dx = 0;
    }

    if (cynumber) {
	gmc.ny = cynumber;
	gmc.dy = cydistance;
	temp1 = cynumber * gmc.dy;
	if (temp1 < 0)
	    callbox[2] += temp1;
	else
	    callbox[3] += temp1;
    }
    else {
	gmc.ny = 0;
	gmc.dy = 0;
    }

    gmc.bxl = callbox[0];
    gmc.bxr = callbox[1];
    gmc.byb = callbox[2];
    gmc.byt = callbox[3];
    strcpy (gmc.inst_name, ".");
    strcpy (gmc.cell_name, name);
    dmPutDesignData (fpmc, GEO_MC);

    if (callpresent) {
	cmp_fillbb (callbox, totcallbox, totcallbox);
    }
    else {
	++callpresent; /* true */
	fillbb (callbox, totcallbox);
    }
}

void get_pattern (char *name, struct edge **edgearray[3], IMPCELL *impcell)
{
    register int i;
    struct edge **localarray[3];
    struct edge *helpptr;

    CALLOC_ARR (localarray[0], struct edge *, nrofmasks);
    CALLOC_ARR (localarray[1], struct edge *, nrofmasks);
    CALLOC_ARR (localarray[2], struct edge *, nrofmasks);

    get_box_patterns (name, localarray, dmproject, (impcell ? 1 : 0));

    for (i = 0; i < nrofmasks; ++i) {
	setedgeptrto (edgearray, i);
	for (helpptr = localarray[1][i]; helpptr; helpptr = helpptr -> rnext) {
	    sort_ins_edge (helpptr -> x, helpptr -> ybottom, helpptr -> ytop, RIGHT);
	    sort_ins_edge (helpptr -> bodybit, helpptr -> ybottom, helpptr -> ytop, LEFT);
	    helpptr -> list = fr_edge_structs;
	    fr_edge_structs = helpptr;
	}
	scan_edges (-1);
	setarraytoptr (edgearray, i);
    }

    FREE (localarray[2]);
    FREE (localarray[1]);
    FREE (localarray[0]);
}

void get_box_patterns (char *name, struct edge **edgearray[3], DM_PROJECT *proj, int impflag)
{
    DM_CELL   *key;
    DM_STREAM *fp;
    char   *rname;
    int     i, j, k, mir, rot, sfx, sfy, tx, ty;
    int     xl, xr, yb, yt;
    int     cxd, cxn, cyd, cyn;
    long    file_offset;
    struct edge **localarray[3];
    struct edge *helpptr;

    if (strcmp (name, pat_name) == 0 && proj == dmproject) {
	key = pat_key;
    }
    else {
	proj = dmFindProjKey (impflag, name, proj, &rname, LAYOUT);
	key = dmCheckOut (proj, rname, ACTUAL, DONTCARE, LAYOUT, READONLY);
    }

    if (dmStat (key, "nor", &st_buf) == 0) {
	if (st_buf.st_size != 0) {
	    pr_err ("sorry: pattern", mpn (name), "nor-file not empty");
	    killpattern ();
	    goto next_part;
	}
    }

    fp = dmOpenStream (key, "box", "r");

    while (dmGetDesignData (fp, GEO_BOX) > 0) {

	xl  = gbox.xl;
	xr  = gbox.xr;
	yb  = gbox.yb;
	yt  = gbox.yt;
	cxd = gbox.dx;
	cyd = gbox.dy;
	cxn = gbox.nx;
	cyn = gbox.ny;

	setedgeptrto (edgearray, (int)gbox.layer_no);

	for (k = 0; k <= cxn; ++k) {
	    for (j = 0; j <= cyn; ++j) {
		ins_edge (xl, yb + j * cyd, yt + j * cyd, xr);
	    }
	    /*
	    ** it should be noted that this edges represent boxes !!
	    ** it also should be noted that edgeptr points to mostleft
	    */
	    xl += cxd;
	    xr += cxd;
	}
	setarraytoptr (edgearray, (int)gbox.layer_no);
    }
    dmCloseStream (fp, COMPLETE);

next_part:
    file_offset = 0;

    for (;;) {
	fp = dmOpenStream (key, "mc", "r");
	dmSeek (fp, file_offset, 0);

	if (dmGetDesignData (fp, GEO_MC) <= 0) {
	    dmCloseStream (fp, COMPLETE);
	    break;
	}

	file_offset = dmTell (fp);
	dmCloseStream (fp, COMPLETE);

	cxd = gmc.dx;
	cyd = gmc.dy;
	cxn = gmc.nx;
	cyn = gmc.ny;

	tx = gmc.mtx[2];
	ty = gmc.mtx[5];

	if (gmc.mtx[0] == 0) {
	    if (gmc.mtx[3] > 0) {
		if (gmc.mtx[1] > 0) {
		    mir = 1; rot = 90; /* MX+R90 */
		    sfy = gmc.mtx[1];
		}
		else {
		    mir = 0; rot = 90; /* R90 */
		    sfy = -gmc.mtx[1];
		}
		sfx = gmc.mtx[3];
	    }
	    else {
		if (gmc.mtx[1] > 0) {
		    mir = 0; rot = 270; /* R270 */
		    sfy = gmc.mtx[1];
		}
		else {
		    mir = 1; rot = 270; /* MX+R270 */
		    sfy = -gmc.mtx[1];
		}
		sfx = -gmc.mtx[3];
	    }
	}
	else {
	    if (gmc.mtx[0] > 0) {
		if (gmc.mtx[4] > 0) {
		    mir = 0; rot = 0; /* R0 */
		    sfy = gmc.mtx[4];
		}
		else {
		    mir = 1; rot = 0; /* MX+R0 */
		    sfy = -gmc.mtx[4];
		}
		sfx = gmc.mtx[0];
	    }
	    else {
		if (gmc.mtx[4] > 0) {
		    mir = 1; rot = 180; /* MX+R180 */
		    sfy = gmc.mtx[4];
		}
		else {
		    mir = 0; rot = 180; /* R180 */
		    sfy = -gmc.mtx[4];
		}
		sfx = -gmc.mtx[0];
	    }
	}

	if (sfx != 1 || sfy != 1) {
	    pr_err ("error: pattern", mpn (gmc.cell_name),
			"sfx or sfy not equal to 1");
	    killpattern ();
	    continue;
	}

	CALLOC_ARR (localarray[0], struct edge *, nrofmasks);
	CALLOC_ARR (localarray[1], struct edge *, nrofmasks);
	CALLOC_ARR (localarray[2], struct edge *, nrofmasks);

	get_box_patterns (gmc.cell_name, localarray, proj, (int)gmc.imported);

	for (i = 0; i < nrofmasks; ++i) {
	    setedgeptrto (edgearray, i);
	    for (helpptr = localarray[1][i]; helpptr; helpptr = helpptr -> rnext) {
		xl = helpptr -> x;
		xr = helpptr -> bodybit; /* !!! */
		yb = helpptr -> ybottom;
		yt = helpptr -> ytop;

		if (mir) { /* MX */
		    temp1 = yb; yb = -yt; yt = -temp1;
		}

		switch (rot) {
		    case 90:
			temp1 = xl; temp2 = xr;
			xl = -yt; xr = -yb; yb = temp1; yt = temp2;
			break;
		    case 180:
			temp1 = xl; temp2 = yb;
			xl = -xr; xr = -temp1; yb = -yt; yt = -temp2;
			break;
		    case 270:
			temp1 = xl; temp2 = xr;
			xl = yb; xr = yt; yb = -temp2; yt = -temp1;
			break;
		}
		xl += tx;
		xr += tx;
		yb += ty;
		yt += ty;

		for (k = 0; k <= cxn; ++k) {
		    for (j = 0; j <= cyn; ++j)
			ins_edge (xl, yb + j * cyd, yt + j * cyd, xr);
		    xl += cxd;
		    xr += cxd;
		}
		helpptr -> list = fr_edge_structs;
		fr_edge_structs = helpptr;
	    }
	    setarraytoptr (edgearray, i);
	}

	FREE (localarray[2]);
	FREE (localarray[1]);
	FREE (localarray[0]);
    }

    if (key != pat_key) dmCheckIn (key, COMPLETE);
}

void invert_edges (struct edge **edgearray[3])
{
    struct edge *helpptr;
    int j;

    for (j = 0; j < nrofmasks; ++j)
	for (helpptr = edgearray[1][j]; helpptr; helpptr = helpptr -> rnext)
	    helpptr -> bodybit = -helpptr -> bodybit;
}

void mix_and_scan_and_grow (int flag, int growth)
{
    struct edge *helpptr;
    int i;

    for (i = 0; i < nrofmasks; ++i) {
	setedgeptrto (edges1, i);
	for (helpptr = edges2[1][i]; helpptr; helpptr = helpptr -> rnext) {
	    sort_ins_edge (helpptr -> x, helpptr -> ybottom, helpptr -> ytop, helpptr -> bodybit);
	    helpptr -> list = fr_edge_structs;
	    fr_edge_structs = helpptr;
	}
	scan_edges (flag);
	grow (growth);
	setarraytoptr (edges1, i);
    }
}

void mk_all_boxes_of_edges () /* make boxes of all edges */
{
    mirrorflag = rotateflag = 0;
    x_origin = y_origin = 0;
    cxnumber = cynumber = 0;
    for (masknr = 0; masknr < nrofmasks; ++masknr) {
	setedgeptrto (edges1, masknr);
	mk_boxes_of_edges ();
    }
}

void mk_mask_boxes_of_edges (int nr) /* make boxes of edges for each mask */
{
    int j;
    mirrorflag = rotateflag = 0;
    x_origin = y_origin = 0;
    cxnumber = cynumber = 0;
    masknr = nr;
    for (j = 0; j < nrofmasks; ++j) {
	setedgeptrto (edges1, j);
	mk_boxes_of_edges ();
    }
}

void setedgeptrto (struct edge **edgearray[3], int nr)
{
    edgeptr   = edgearray[0][nr];
    mostleft  = edgearray[1][nr];
    mostright = edgearray[2][nr];
}

void setarraytoptr (struct edge **edgearray[3], int nr)
{
    edgearray[0][nr] = edgeptr;
    edgearray[1][nr] = mostleft;
    edgearray[2][nr] = mostright;
}

void rm_array (struct edge **edgearray[3]) /* remove array */
{
    struct edge *helpptr;
    int i;
    for (i = 0; i < nrofmasks; ++i) {
	for (helpptr = edgearray[1][i]; helpptr; helpptr = helpptr -> rnext) {
	    helpptr -> list = fr_edge_structs;
	    fr_edge_structs = helpptr;
	}
	edgearray[0][i] = edgearray[1][i] = edgearray[2][i] = NULL;
    }
}

void placepoint (register int i)
{
    if (mirrorflag) {
	if (mirrorflag == 1) {
	    p_y[i] = -p_y[i];
	}
	else {
	    p_x[i] = -p_x[i];
	}
    }

    switch (rotateflag) {
	case 0:
	    break;
	case 90:
	    tmp1 = p_x[i];
	    p_x[i] = -p_y[i];
	    p_y[i] = tmp1;
	    break;
	case 180:
	    p_x[i] = -p_x[i];
	    p_y[i] = -p_y[i];
	    break;
	case 270:
	    tmp1 = p_x[i];
	    p_x[i] = p_y[i];
	    p_y[i] = -tmp1;
	    break;
    }

    p_x[i] += x_origin;
    p_y[i] += y_origin;
}

void printonebox () /* print ra/rr program pattern */
{
    register int offset, i;
    double off225;

    placepoint (0);
    placepoint (1);

    if (p_x[0] > p_x[1]) {
	tmp1 = p_x[1]; p_x[1] = p_x[0]; p_x[0] = tmp1;
    }
    if (p_y[0] > p_y[1]) {
	tmp1 = p_y[1]; p_y[1] = p_y[0]; p_y[0] = tmp1;
    }

    for (i = 0; i < nroflistmasks; ++i) {
	masknr = mlist[i][0];
	if ((offset = mlist[i][1])) {
	    if (box_type == 0) {
		p_x[0] += offset;
		p_x[1] -= offset;
		p_y[0] += offset;
		p_y[1] -= offset;
	    }
	    else {
		off225 = offset * TAN225;
		p_x[0] += (box_type & LA_G_XL) ? off225 : offset;
		p_x[1] -= (box_type & LA_G_XR) ? off225 : offset;
		p_y[0] += (box_type & LA_G_YB) ? off225 : offset;
		p_y[1] -= (box_type & LA_G_YT) ? off225 : offset;
	    }
	}

	if ((p_x[1] - p_x[0] <= 0) || (p_y[1] - p_y[0] <= 0)) {
	    pr_err2 ("warning: box skipped, fades to dimension zero, mask:",
		cmklay[masknr]);
	    continue;
	}
	else {
	    progbox[0] = roundX (p_x[0]);
	    progbox[1] = roundX (p_x[1]);
	    progbox[2] = roundX (p_y[0]);
	    progbox[3] = roundX (p_y[1]);

	    if (cxnumber || cynumber) {
		gbox.dx = gbox.dy = 0;
		if (cxnumber) {
		    gbox.dx = cxdistance;
		    temp1 = cxnumber * gbox.dx;
		    if (cxdistance < 0)
			progbox[0] += temp1;
		    else
			progbox[1] += temp1;
		}
		if (cynumber) {
		    gbox.dy = cydistance;
		    temp1 = cynumber * gbox.dy;
		    if (cydistance < 0)
			progbox[2] += temp1;
		    else
			progbox[3] += temp1;
		}
		gbox.bxl = progbox[0];
		gbox.bxr = progbox[1];
		gbox.byb = progbox[2];
		gbox.byt = progbox[3];
	    }

	    gbox.layer_no = masknr;
	    gbox.xl = roundX (p_x[0]);
	    gbox.xr = roundX (p_x[1]);
	    gbox.yb = roundX (p_y[0]);
	    gbox.yt = roundX (p_y[1]);
	    gbox.nx = cxnumber;
	    gbox.ny = cynumber;
	    dmPutDesignData (fpbox, GEO_BOX);

	    if (progpresent) {
		cmp_fillbb (progbox, totprogbox, totprogbox);
	    }
	    else {
		++progpresent; /* true */
		fillbb (progbox, totprogbox);
	    }
	}
    }
}

void pr_la_pattern () /* print la program pattern */
{
    register struct la_elmt *z;
    for (z = first_la_elmt; z; z = z -> next) {
	box_type = z -> gtype;
	p_x[0] = z -> xl;
	p_x[1] = z -> xr;
	p_y[0] = z -> yb;
	p_y[1] = z -> yt;
	if (box_type & LA_SBOX) { /* slanting box */
	    pr_rs_pattern ();
	}
	else {
	    printonebox ();
	}
    }
    box_type = 0;
}

void pr_rs_pattern () /* print rs program pattern */
{
    register int offset, i;

    placepoint (0);
    placepoint (1);

    x1 = p_x[0]; g_y1 = p_y[0];
    x2 = p_x[1]; y2 = p_y[1];

    tmp1 = ((x2 - x1) + (y2 - g_y1)) / 2;
    x3 = x1 + tmp1;
    y3 = g_y1 + tmp1;
    x4 = x2 - tmp1;
    y4 = y2 - tmp1;

    if (x3 < x1) {
	tmp1 = x1; x1 = x3; x3 = tmp1;
	tmp1 = g_y1; g_y1 = y3; y3 = tmp1;
	tmp1 = x2; x2 = x4; x4 = tmp1;
	tmp1 = y2; y2 = y4; y4 = tmp1;
    }
    if (x4 > x1) {
	tmp1 = x1; x1 = x4; x4 = tmp1;
	tmp1 = g_y1; g_y1 = y4; y4 = tmp1;
	tmp1 = x2; x2 = x3; x3 = tmp1;
	tmp1 = y2; y2 = y3; y3 = tmp1;
    }

    for (i = 0; i < nroflistmasks; ++i) {
	masknr = mlist[i][0];
	if ((offset = mlist[i][1])) {
	    if (box_type == 0) {
		tmp1 = offset * SQRT2;
		x3 -= tmp1;
		x4 += tmp1;
		g_y1 += tmp1;
		y2 -= tmp1;
	    }
	    else {
		tmp1 = offset;
		x3 -= tmp1;
		x4 += tmp1;
		g_y1 += tmp1;
		y2 -= tmp1;
		tmp1 = offset * TAN225;
		if (box_type & LA_SB_P) {
		    x1 += tmp1;
		    x2 -= tmp1;
		    y3 -= tmp1;
		    y4 += tmp1;
		}
		else {
		    x1 -= tmp1;
		    x2 += tmp1;
		    y3 += tmp1;
		    y4 -= tmp1;
		}
	    }
	}

	if (x4 >= x1 || x3 <= x1) {
	    pr_err2 ("warning: sbox skipped, fades to dimension zero, mask:",
		cmklay[masknr]);
	    continue;
	}
	else {
	    gnor_ini.bxl = progbox[0] = roundL (x4);
	    gnor_ini.bxr = progbox[1] = roundh (x3);
	    gnor_ini.byb = progbox[2] = roundL (g_y1);
	    gnor_ini.byt = progbox[3] = roundh (y2);

	    if (cxnumber || cynumber) {
		gnor_ini.dx = gnor_ini.dy = 0;
		if (cxnumber) {
		    gnor_ini.dx = cxdistance;
		    tmp1 = cxnumber * gnor_ini.dx;
		    if (cxdistance < 0)
			progbox[0] = roundL (x4 + tmp1);
		    else
			progbox[1] = roundh (x3 + tmp1);
		}
		if (cynumber) {
		    gnor_ini.dy = cydistance;
		    tmp1 = cynumber * gnor_ini.dy;
		    if (cydistance < 0)
			progbox[2] = roundL (g_y1 + tmp1);
		    else
			progbox[3] = roundh (y2 + tmp1);
		}
		gnor_ini.r_bxl = progbox[0];
		gnor_ini.r_bxr = progbox[1];
		gnor_ini.r_byb = progbox[2];
		gnor_ini.r_byt = progbox[3];
	    }

	    gnor_ini.layer_no = masknr;
	    gnor_ini.elmt = RECT_NOR;
	    gnor_ini.no_xy = 4;
	    gnor_ini.nx = cxnumber;
	    gnor_ini.ny = cynumber;
	    dmPutDesignData (fpnor, GEO_NOR_INI);

	    gnor_xy.x = x1;
	    gnor_xy.y = g_y1;
	    dmPutDesignData (fpnor, GEO_NOR_XY);
	    gnor_xy.x = x3;
	    gnor_xy.y = y3;
	    dmPutDesignData (fpnor, GEO_NOR_XY);
	    gnor_xy.x = x2;
	    gnor_xy.y = y2;
	    dmPutDesignData (fpnor, GEO_NOR_XY);
	    gnor_xy.x = x4;
	    gnor_xy.y = y4;
	    dmPutDesignData (fpnor, GEO_NOR_XY);

	    if (progpresent) {
		cmp_fillbb (progbox, totprogbox, totprogbox);
	    }
	    else {
		++progpresent; /* true */
		fillbb (progbox, totprogbox);
	    }
	}
    }
}

void pr_po_pattern () /* print polygon program pattern */
{
    register int i, j;
    double d_bxl, d_bxr, d_byb, d_byt;

    placepoint (0);
    d_bxl = d_bxr = p_x[0];
    d_byb = d_byt = p_y[0];
    for (i = 1; i < p_incr; ++i) {
	placepoint (i);
	if (p_x[i] < d_bxl) d_bxl = p_x[i];
	else if (p_x[i] > d_bxr) d_bxr = p_x[i];
	if (p_y[i] < d_byb) d_byb = p_y[i];
	else if (p_y[i] > d_byt) d_byt = p_y[i];
    }

    for (i = 0; i < nroflistmasks; ++i) {
	masknr = mlist[i][0];
	if (mlist[i][1]) {
	    pr_err1 ("polygon: masklist with offset not implemented");
	    killpattern ();
	    break;
	}
	else {
	    gnor_ini.bxl = progbox[0] = roundL (d_bxl);
	    gnor_ini.bxr = progbox[1] = roundh (d_bxr);
	    gnor_ini.byb = progbox[2] = roundL (d_byb);
	    gnor_ini.byt = progbox[3] = roundh (d_byt);

	    if (cxnumber || cynumber) {
		gnor_ini.dx = gnor_ini.dy = 0;
		if (cxnumber) {
		    gnor_ini.dx = cxdistance;
		    tmp1 = cxnumber * gnor_ini.dx;
		    if (cxdistance < 0)
			progbox[0] = roundL (d_bxl + tmp1);
		    else
			progbox[1] = roundh (d_bxr + tmp1);
		}
		if (cynumber) {
		    gnor_ini.dy = cydistance;
		    tmp1 = cynumber * gnor_ini.dy;
		    if (cydistance < 0)
			progbox[2] = roundL (d_byb + tmp1);
		    else
			progbox[3] = roundh (d_byt + tmp1);
		}
		gnor_ini.r_bxl = progbox[0];
		gnor_ini.r_bxr = progbox[1];
		gnor_ini.r_byb = progbox[2];
		gnor_ini.r_byt = progbox[3];
	    }

	    gnor_ini.layer_no = masknr;
	    gnor_ini.elmt = POLY_NOR;
	    gnor_ini.no_xy = p_incr;
	    gnor_ini.nx = cxnumber;
	    gnor_ini.ny = cynumber;
	    dmPutDesignData (fpnor, GEO_NOR_INI);

	    for (j = 0; j < p_incr; ++j) {
		gnor_xy.x = p_x[j];
		gnor_xy.y = p_y[j];
		dmPutDesignData (fpnor, GEO_NOR_XY);
	    }

	    if (progpresent) {
		cmp_fillbb (progbox, totprogbox, totprogbox);
	    }
	    else {
		++progpresent; /* true */
		fillbb (progbox, totprogbox);
	    }
	}
    }
}

void pr_pa_pattern () /* print path program pattern */
{
    register int i, j, k;
    double d_bxl, d_bxr, d_byb, d_byt;
    double d_x, d_y, a1, a2;
    double dx1, dy1, dx2, dy2;
    double cxl, cyl, cxr, cyr, a2xy, b2xy;
    double a1xy, b1xy, aspx, aspy, bspx, bspy;

    d_bxl = d_bxr = d_byb = d_byt = 0; /* init to suppres gcc warning */
    a1 = dx1 = dy1 = a1xy = b1xy = 0; /* init to suppres gcc warning */

    p_width /= 2;

    for (i = 0; i <= p_incr; ++i) placepoint (i);

    for (k = 0; k < nroflistmasks; ++k) {
	masknr = mlist[k][0];
	if (mlist[k][1]) {
	    tmp1 = mlist[k][1];
	    dx2 = p_x[1] - p_x[0];
	    dy2 = p_y[1] - p_y[0];
	    if (dx2 == 0) {
		if (dy2 == 0) {
		    pr_err1 ("path: incr. values are 0");
		    killpattern ();
		    return;
		}
		if (dy2 > 0) a2 = 2 * rad45;
		else a2 = 6 * rad45;
	    }
	    else {
		a2 = atan (dy2 / dx2);
		if (dx2 > 0) {
		    if (dy2 < 0) a2 += 8 * rad45;
		}
		else {
		    a2 += 4 * rad45;
		}
	    }
	    p_x[0] += tmp1 * cos (a2);
	    p_y[0] += tmp1 * sin (a2);

	    dx2 = p_x[p_incr] - p_x[p_incr-1];
	    dy2 = p_y[p_incr] - p_y[p_incr-1];
	    if (dx2 == 0) {
		if (dy2 == 0) {
		    pr_err1 ("path: incr. values are 0");
		    killpattern ();
		    return;
		}
		if (dy2 > 0) a2 = 2 * rad45;
		else a2 = 6 * rad45;
	    }
	    else {
		a2 = atan (dy2 / dx2);
		if (dx2 > 0) {
		    if (dy2 < 0) a2 += 8 * rad45;
		}
		else {
		    a2 += 4 * rad45;
		}
	    }
	    p_x[p_incr] -= tmp1 * cos (a2);
	    p_y[p_incr] -= tmp1 * sin (a2);
	    p_width -= tmp1;
	}

	if (p_width < resol) {
	    pr_err1 ("path: width smaller than 2 * resolution");
	    killpattern ();
	    return;
	}

	for (i = 1;;) {
	    cxl = p_x[i-1];
	    cyl = p_y[i-1];
	    cxr = p_x[i];
	    cyr = p_y[i];
	    dx2 = cxr - cxl;
	    dy2 = cyr - cyl;
	    if (dx2 == 0) {
		if (dy2 == 0) {
		    pr_err1 ("path: incr. values are 0");
		    killpattern ();
		    return;
		}
		if (dy2 > 0) a2 = 2 * rad45;
		else a2 = 6 * rad45;
	    }
	    else {
		a2 = atan (dy2 / dx2);
		if (dx2 > 0) {
		    if (dy2 < 0) a2 += 8 * rad45;
		}
		else {
		    a2 += 4 * rad45;
		}
	    }
	    d_x = p_width * sin (a2);
	    d_y = p_width * cos (a2);
	    a2xy = (cxr - d_x) * (cyl + d_y) - (cxl - d_x) * (cyr + d_y);
	    b2xy = (cxr + d_x) * (cyl - d_y) - (cxl + d_x) * (cyr - d_y);

	    if (i == 1) {
		if (dx2 > 0) {
		    d_bxl = (d_x > 0) ? (cxl - d_x) : (cxl + d_x);
		    d_bxr = (d_x > 0) ? (cxr + d_x) : (cxr - d_x);
		}
		else {
		    d_bxl = (d_x > 0) ? (cxr - d_x) : (cxr + d_x);
		    d_bxr = (d_x > 0) ? (cxl + d_x) : (cxl - d_x);
		}
		if (dy2 > 0) {
		    d_byb = (d_y > 0) ? (cyl - d_y) : (cyl + d_y);
		    d_byt = (d_y > 0) ? (cyr + d_y) : (cyr - d_y);
		}
		else {
		    d_byb = (d_y > 0) ? (cyr - d_y) : (cyr + d_y);
		    d_byt = (d_y > 0) ? (cyl + d_y) : (cyl - d_y);
		}
	    }
	    else {
		tmp1 = (a1 > a2) ? (a1 - a2) : (a2 - a1);
		if (tmp1 > (2 * rad45) && tmp1 < (6 * rad45)) {
		    pr_err1 ("path: too big direction change");
		    killpattern ();
		    return;
		}
		tmp1 = dx2 * dy1 - dx1 * dy2;
		if (tmp1 == 0) {
		    for (j = i; j <= p_incr; ++j) {
			p_x[j-1] = p_x[j];
			p_y[j-1] = p_y[j];
		    }
		    --i;
		    --p_incr;
		}
		else {
		    aspx = (dx1 * a2xy - dx2 * a1xy) / tmp1;
		    aspy = (dy1 * a2xy - dy2 * a1xy) / tmp1;
		    bspx = (dx1 * b2xy - dx2 * b1xy) / tmp1;
		    bspy = (dy1 * b2xy - dy2 * b1xy) / tmp1;
		    if (aspx > bspx) {
			if (bspx < d_bxl) d_bxl = bspx;
			if (aspx > d_bxr) d_bxr = aspx;
		    }
		    else {
			if (aspx < d_bxl) d_bxl = aspx;
			if (bspx > d_bxr) d_bxr = bspx;
		    }
		    if (aspy > bspy) {
			if (bspy < d_byb) d_byb = bspy;
			if (aspy > d_byt) d_byt = aspy;
		    }
		    else {
			if (aspy < d_byb) d_byb = aspy;
			if (bspy > d_byt) d_byt = bspy;
		    }
		}
	    }

	    if (++i > p_incr) break;
	    a1 = a2;
	    dx1 = dx2;
	    dy1 = dy2;
	    a1xy = a2xy;
	    b1xy = b2xy;
	}

	if (dx2 > 0) {
	    tmp1 = (d_x > 0) ? (cxr + d_x) : (cxr - d_x);
	    if (tmp1 > d_bxr) d_bxr = tmp1;
	}
	else {
	    tmp1 = (d_x > 0) ? (cxr - d_x) : (cxr + d_x);
	    if (tmp1 < d_bxl) d_bxl = tmp1;
	}
	if (dy2 > 0) {
	    tmp1 = (d_y > 0) ? (cyr + d_y) : (cyr - d_y);
	    if (tmp1 > d_byt) d_byt = tmp1;
	}
	else {
	    tmp1 = (d_y > 0) ? (cyr - d_y) : (cyr + d_y);
	    if (tmp1 < d_byb) d_byb = tmp1;
	}

	gnor_ini.bxl = progbox[0] = roundL (d_bxl);
	gnor_ini.bxr = progbox[1] = roundh (d_bxr);
	gnor_ini.byb = progbox[2] = roundL (d_byb);
	gnor_ini.byt = progbox[3] = roundh (d_byt);

	if (cxnumber || cynumber) {
	    gnor_ini.dx = gnor_ini.dy = 0;
	    if (cxnumber) {
		gnor_ini.dx = cxdistance;
		tmp1 = cxnumber * gnor_ini.dx;
		if (cxdistance < 0)
		    progbox[0] = roundL (d_bxl + tmp1);
		else
		    progbox[1] = roundh (d_bxr + tmp1);
	    }
	    if (cynumber) {
		gnor_ini.dy = cydistance;
		tmp1 = cynumber * gnor_ini.dy;
		if (cydistance < 0)
		    progbox[2] = roundL (d_byb + tmp1);
		else
		    progbox[3] = roundh (d_byt + tmp1);
	    }
	    gnor_ini.r_bxl = progbox[0];
	    gnor_ini.r_bxr = progbox[1];
	    gnor_ini.r_byb = progbox[2];
	    gnor_ini.r_byt = progbox[3];
	}

	gnor_ini.layer_no = masknr;
	gnor_ini.elmt = WIRE_NOR;
	gnor_ini.no_xy = p_incr + 2;
	gnor_ini.nx = cxnumber;
	gnor_ini.ny = cynumber;
	dmPutDesignData (fpnor, GEO_NOR_INI);

	gnor_xy.x = 2 * p_width;
	gnor_xy.y = 0;
	dmPutDesignData (fpnor, GEO_NOR_XY);

	gnor_xy.x = roundX (p_x[0]);
	gnor_xy.y = roundX (p_y[0]);
	dmPutDesignData (fpnor, GEO_NOR_XY);

	for (i = 1; i <= p_incr; ++i) {
	    gnor_xy.x = roundX (p_x[i] - p_x[i-1]);
	    gnor_xy.y = roundX (p_y[i] - p_y[i-1]);
	    dmPutDesignData (fpnor, GEO_NOR_XY);
	}

	if (progpresent) {
	    cmp_fillbb (progbox, totprogbox, totprogbox);
	}
	else {
	    ++progpresent; /* true */
	    fillbb (progbox, totprogbox);
	}
    }
}

void pr_ci_pattern () /* print circle program pattern */
{
    register int offset;
    register int i;

    placepoint (0);

    for (i = 0; i < nroflistmasks; ++i) {
	masknr = mlist[i][0];
	if ((offset = mlist[i][1])) {
	    p_x[1] -= offset;
	}

	if (p_x[1] <= 0) {
	    pr_err2 ("warning: circle skipped, fades to dimension zero, mask:",
		cmklay[masknr]);
	    continue;
	}
	else {
	    gnor_ini.bxl = progbox[0] = roundL (p_x[0] - p_x[1]);
	    gnor_ini.bxr = progbox[1] = roundh (p_x[0] + p_x[1]);
	    gnor_ini.byb = progbox[2] = roundL (p_y[0] - p_x[1]);
	    gnor_ini.byt = progbox[3] = roundh (p_y[0] + p_x[1]);

	    if (cxnumber || cynumber) {
		gnor_ini.dx = gnor_ini.dy = 0;
		if (cxnumber) {
		    gnor_ini.dx = cxdistance;
		    tmp1 = p_x[0] + cxnumber * gnor_ini.dx;
		    if (cxdistance < 0)
			progbox[0] = roundL (tmp1 - p_x[1]);
		    else
			progbox[1] = roundh (tmp1 + p_x[1]);
		}
		if (cynumber) {
		    gnor_ini.dy = cydistance;
		    tmp1 = p_y[0] + cynumber * gnor_ini.dy;
		    if (cydistance < 0)
			progbox[2] = roundL (tmp1 - p_x[1]);
		    else
			progbox[3] = roundh (tmp1 + p_x[1]);
		}
		gnor_ini.r_bxl = progbox[0];
		gnor_ini.r_bxr = progbox[1];
		gnor_ini.r_byb = progbox[2];
		gnor_ini.r_byt = progbox[3];
	    }

	    gnor_ini.layer_no = masknr;
	    gnor_ini.elmt = CIRCLE_NOR;
	    gnor_ini.no_xy = 3;
	    gnor_ini.nx = cxnumber;
	    gnor_ini.ny = cynumber;
	    dmPutDesignData (fpnor, GEO_NOR_INI);

	    gnor_xy.x = p_x[0];
	    gnor_xy.y = p_y[0];
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 1 */
	    gnor_xy.x = p_x[1];
	    gnor_xy.y = 0;
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 2 */
	    gnor_xy.x = n_edges;
	    gnor_xy.y = 0;
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 3 */

	    if (progpresent) {
		cmp_fillbb (progbox, totprogbox, totprogbox);
	    }
	    else {
		++progpresent; /* true */
		fillbb (progbox, totprogbox);
	    }
	}
    }
}

void pr_cp_pattern () /* print cpeel program pattern */
{
    double r1, r2, a1, a2, rad_a1, rad_a2;
    double d_bxl, d_bxr, d_byb, d_byt;
    register int i, offset;
    int xy_pairs = 4;

    r1 = p_x[1];
    r2 = p_y[1];
    a1 = p_x[2];
    a2 = p_y[2];

    if (a1 == 3600) a1 = 0;
    if (a2 == 0) a2 = 3600;
    if (a1 == 0 && a2 == 3600) --xy_pairs;

    placepoint (0);

    if (xy_pairs == 4) {
	if (mirrorflag) {
	    if (mirrorflag == 1) {
		tmp1 = a1;
		a1 = 3600 - a2;
		a2 = 3600 - tmp1;
	    }
	    else {
		tmp1 = a1;
		a1 = 1800 - a2;
		a2 = 1800 - tmp1;
		if (a1 < 0) a1 += 3600;
		if (a2 <= 0) a2 += 3600;
	    }
	}

	if (rotateflag) {
	    a1 += 10 * rotateflag;
	    a2 += 10 * rotateflag;
	    if (a1 >= 3600) a1 -= 3600;
	    if (a2 > 3600) a2 -= 3600;
	}
    }

    for (i = 0; i < nroflistmasks; ++i) {
	masknr = mlist[i][0];
	if ((offset = mlist[i][1])) {
	    r2 += offset;
	    r1 -= offset;
	}

	if (r1 <= r2) {
	    pr_err2 ("warning: cpeel skipped, fades to dimension zero, mask:",
		cmklay[masknr]);
	    continue;
	}
	else {
	    if (xy_pairs == 3) {
		d_bxl = p_x[0] - r1;
		d_bxr = p_x[0] + r1;
		d_byb = p_y[0] - r1;
		d_byt = p_y[0] + r1;
	    }
	    else {
		rad_a1 = a1 * rad45 / 450;
		rad_a2 = a2 * rad45 / 450;

		tmp1 = p_x[0] + r1 * cos (rad_a1);
		tmp2 = p_x[0] + r1 * cos (rad_a2);
		if (tmp1 < tmp2) {
		    d_bxl = tmp1;
		    d_bxr = tmp2;
		}
		else {
		    d_bxl = tmp2;
		    d_bxr = tmp1;
		}

		if (a1 <= 1800 && (a2 >= 1800
			    || a2 < a1)) d_bxl = p_x[0] - r1;
		else if (a2 >= 1800
			    && a2 < a1) d_bxl = p_x[0] - r1;
		if (a1 == 0 || a2 == 3600) d_bxr = p_x[0] + r1;
		else if (a1 > a2) d_bxr = p_x[0] + r1;

		tmp1 = p_y[0] + r1 * sin (rad_a1);
		tmp2 = p_y[0] + r1 * sin (rad_a2);
		if (tmp1 < tmp2) {
		    d_byb = tmp1;
		    d_byt = tmp2;
		}
		else {
		    d_byb = tmp2;
		    d_byt = tmp1;
		}

		if (a1 <= 2700 && (a2 < a1
			    || a2 >= 2700)) d_byb = p_y[0] - r1;
		if (a2 >= 900 && (a1 > a2
			    || a1 <= 900)) d_byt = p_y[0] + r1;

		if (r2 != 0) {
		    tmp1 = p_x[0] + r2 * cos (rad_a1);
		    tmp2 = p_x[0] + r2 * cos (rad_a2);
		    if (tmp1 < d_bxl) d_bxl = tmp1;
		    if (tmp2 < d_bxl) d_bxl = tmp2;
		    if (tmp1 > d_bxr) d_bxr = tmp1;
		    if (tmp2 > d_bxr) d_bxr = tmp2;
		    tmp1 = p_y[0] + r2 * sin (rad_a1);
		    tmp2 = p_y[0] + r2 * sin (rad_a2);
		    if (tmp1 < d_byb) d_byb = tmp1;
		    if (tmp2 < d_byb) d_byb = tmp2;
		    if (tmp1 > d_byt) d_byt = tmp1;
		    if (tmp2 > d_byt) d_byt = tmp2;
		}
		else {
		    if (a1 >= 2700 && (a2 > a1
				    || a2 <= 900)) d_bxl = p_x[0];
		    else if (a1 <= 900 && a2 <= 900
				    && a2 > a1) d_bxl = p_x[0];
		    if (a1 >= 900 && a2 <= 2700
				    && a2 > a1) d_bxr = p_x[0];
		    if (a2 <= 1800 && a2 > a1) d_byb = p_y[0];
		    if (a1 >= 1800 && a2 > a1) d_byt = p_y[0];
		}
	    }

	    gnor_ini.bxl = progbox[0] = roundL (d_bxl);
	    gnor_ini.bxr = progbox[1] = roundh (d_bxr);
	    gnor_ini.byb = progbox[2] = roundL (d_byb);
	    gnor_ini.byt = progbox[3] = roundh (d_byt);

	    if (cxnumber || cynumber) {
		gnor_ini.dx = gnor_ini.dy = 0;
		if (cxnumber) {
		    gnor_ini.dx = cxdistance;
		    tmp1 = cxnumber * gnor_ini.dx;
		    if (cxdistance < 0)
			progbox[0] = roundL (d_bxl + tmp1);
		    else
			progbox[1] = roundh (d_bxr + tmp1);
		}
		if (cynumber) {
		    gnor_ini.dy = cydistance;
		    tmp1 = cynumber * gnor_ini.dy;
		    if (cydistance < 0)
			progbox[2] = roundL (d_byb + tmp1);
		    else
			progbox[3] = roundh (d_byt + tmp1);
		}
		gnor_ini.r_bxl = progbox[0];
		gnor_ini.r_bxr = progbox[1];
		gnor_ini.r_byb = progbox[2];
		gnor_ini.r_byt = progbox[3];
	    }

	    gnor_ini.layer_no = masknr;
	    gnor_ini.elmt = CIRCLE_NOR;
	    gnor_ini.no_xy = xy_pairs;
	    gnor_ini.nx = cxnumber;
	    gnor_ini.ny = cynumber;
	    dmPutDesignData (fpnor, GEO_NOR_INI);

	    gnor_xy.x = p_x[0];
	    gnor_xy.y = p_y[0];
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 1 */
	    gnor_xy.x = r1;
	    gnor_xy.y = r2;
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 2 */
	    gnor_xy.x = n_edges;
	    gnor_xy.y = 0;
	    dmPutDesignData (fpnor, GEO_NOR_XY); /* 3 */
	    if (xy_pairs == 4) {
		gnor_xy.x = a1 / 10.0;
		gnor_xy.y = a2 / 10.0;
		dmPutDesignData (fpnor, GEO_NOR_XY); /* 4 */
	    }

	    if (progpresent) {
		cmp_fillbb (progbox, totprogbox, totprogbox);
	    }
	    else {
		++progpresent; /* true */
		fillbb (progbox, totprogbox);
	    }
	}
    }
}

void cmp_fillbb (int ptr1[], int ptr2[], int ptr3[])
{
    ptr3[0] = (ptr1[0] < ptr2[0]) ? ptr1[0] : ptr2[0];
    ptr3[1] = (ptr1[1] > ptr2[1]) ? ptr1[1] : ptr2[1];
    ptr3[2] = (ptr1[2] < ptr2[2]) ? ptr1[2] : ptr2[2];
    ptr3[3] = (ptr1[3] > ptr2[3]) ? ptr1[3] : ptr2[3];
}

void fillbb (int ptr1[], int ptr2[])
{
    ptr2[0] = ptr1[0];
    ptr2[1] = ptr1[1];
    ptr2[2] = ptr1[2];
    ptr2[3] = ptr1[3];
}

void cl_boxes_etc ()
{
    clean_bb ();

    if (pat_key) {
	fprintf (stderr, "%s: pat_key OPEN, pattern '%s' not placed!\n", argv0, pat_name);
	closepatdir (QUIT);
    }

    makepattern = makedatabase;
}

void clean_bb ()
{
    int i;

    for (i = 0; i < 4; ++i)
	pat_box[i] = totcallbox[i] = totprogbox[i] = 0;

    progpresent = callpresent = 0;
}

void write_info ()
{
    if (callpresent && progpresent)
	cmp_fillbb (totcallbox, totprogbox, pat_box);
    else
	if (callpresent)
	    fillbb (totcallbox, pat_box);
	else
	    fillbb (totprogbox, pat_box);

    if (pat_box[0] == pat_box[1]) {
	pr_err ("pattern", mpn (pat_name), "is empty");
	killpattern ();
	return;
    }

    add_pat_tree ();
    ALLOC_STRUCT (tree_ptr -> bbox, pat_bbox);
    tree_ptr -> bbox -> xl = pat_box[0];
    tree_ptr -> bbox -> xr = pat_box[1];
    tree_ptr -> bbox -> yb = pat_box[2];
    tree_ptr -> bbox -> yt = pat_box[3];

    ginfo.bxl = pat_box[0];
    ginfo.bxr = pat_box[1];
    ginfo.byb = pat_box[2];
    ginfo.byt = pat_box[3];
    dmPutDesignData (fpinfo, GEO_INFO);

    ginfo.bxl = totcallbox[0];
    ginfo.bxr = totcallbox[1];
    ginfo.byb = totcallbox[2];
    ginfo.byt = totcallbox[3];
    dmPutDesignData (fpinfo, GEO_INFO);

    ginfo.bxl = totprogbox[0];
    ginfo.bxr = totprogbox[1];
    ginfo.byb = totprogbox[2];
    ginfo.byt = totprogbox[3];
    dmPutDesignData (fpinfo, GEO_INFO);

    closepatdir (COMPLETE);
}

int getmaskname (char *string)
{
    for (masknr = 0; masknr < nrofmasks; ++masknr) {
	if (strcmp (string, cmklay[masknr]) == 0)
	    return (masknr); /* found */
    }
    return (-1); /* not found */
}

struct list *getlistname (char *string)
{
    for (listptr = mlistnames; listptr; listptr = listptr -> next) {
	if (strcmp (string, listptr -> name) == 0)
	    break; /* found */
    }
    return (listptr);
}

/*
** following routines are for debug purposes
*/
#ifdef DEBUG
void printarray (struct edge **edgearray[3])
{
    struct edge *helpptr;
    int i;

    for (i = 0; i < nrofmasks; ++i) {
	printf ("lijst %d\n", i);
	printf ("array[0][i]:"); printstruct (edgearray[0][i]);
	printf ("array[1][i]:"); printstruct (edgearray[1][i]);
	printf ("array[2][i]:"); printstruct (edgearray[2][i]);
	printf ("lijst zelf:\n");
	for (helpptr = edgearray[1][i]; helpptr; helpptr = helpptr -> rnext)
	    printstruct (helpptr);
    }
}

void printedgeptr ()
{
    printf ("edgeptr:");   printstruct (edgeptr);
    printf ("mostleft:");  printstruct (mostleft);
    printf ("mostright:"); printstruct (mostright);
}

void printstruct (struct edge *ptr)
{
    if (ptr)
	printf (" x: %d, yb: %d, yt: %d, bb: %d\n",
	    ptr -> x, ptr -> ybottom, ptr -> ytop, ptr -> bodybit);
    else
	printf (" pointer is null\n");
}
#endif /* DEBUG */
