/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	S. de Graaf
 *	T.G.R. van Leuken
 *	P. Kist
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
#include "src/getlld/incl.h"
#include "src/getlld/extern.h"

static char *formatCX = "cx %s,%d";
static char *formatCY = "%scy %s,%d";
static char *formatRA = "ra(%s,%s,%s,%s,%s)";
static char *formatRS = "rs(%s,%s,%s,%s,%s)";
static char *formatRR = ",r,%s,%s";
static char *formatAA = ",a,%s,%s";
static char *formatMX = " mx";
static char *formatRO = "%sr %d";
static char *formatSX = "%ssx %d";
static char *formatSY = "%ssy %d";
static char *formatPA = "path(%s,%s,%s";
static char *formatPO = "polygon(%s,%s,%s";
static char *formatMS = "pattern %s\n";
static char *formatME = "end %s\n";
static char *strCI = "circle";
static char *strCP = "cpeel";

static void cmk_term_copy (void);
static void cmk_box_copy (void);
static void cmk_nor_copy (char *);
static void cmk_mc_copy (void);

void outp_cmk (struct mo_elmt *p)
{
    static int firsttime = 1;
    char newname[DM_MAXNAME + 1];
    register char *c, *s;

    c = p -> nname;
    s = newname;
    while ((*s = *c++))
	if (*s++ == '_') *(s - 1) = '-';

    if (u_mode && firsttime) {
	firsttime = 0;
	formatCX = "CX %s,%d";
	formatCY = "%sCY %s,%d";
	formatRA = "RA(%s,%s,%s,%s,%s)";
	formatRS = "RS(%s,%s,%s,%s,%s)";
	formatRR = ",R,%s,%s";
	formatAA = ",A,%s,%s";
	formatMX = " MX";
	formatRO = "%sR %d";
	formatSX = "%sSX %d";
	formatSY = "%sSY %d";
	formatPA = "PATH(%s,%s,%s";
	formatPO = "POLYGON(%s,%s,%s";
	formatMS = "PATTERN %s\n";
	formatME = "END %s\n";
	strCI = "CIRCLE";
	strCP = "CPEEL";
    }
    PF formatMS, newname);
    cmk_term_copy ();
    cmk_box_copy ();
    cmk_nor_copy (p -> oname);
    if (p -> mcalls) cmk_mc_copy ();
    PF formatME, newname);
}

static void cmk_term_copy ()
{
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "term", "r");

    while (dmGetDesignData (stream, GEO_TERM) > 0) {
	PF formatRA, lay[gterm.layer_no],
	    d2a (resol * gterm.xl), d2a (resol * gterm.xr),
	    d2a (resol * gterm.yb), d2a (resol * gterm.yt));
	if (gterm.nx)
	    PF formatCX, d2a (resol * gterm.dx), gterm.nx);
	if (gterm.ny)
	    PF formatCY, (gterm.nx ? "," : ""), d2a (resol * gterm.dy), gterm.ny);
	PF " : term %s\n", gterm.term_name);
    }
    dmCloseStream (stream, COMPLETE);
}

static void cmk_box_copy ()
{
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "box", "r");

    while (dmGetDesignData (stream, GEO_BOX) > 0) {
	PF formatRA, lay[gbox.layer_no],
	    d2a (resol * gbox.xl), d2a (resol * gbox.xr),
	    d2a (resol * gbox.yb), d2a (resol * gbox.yt));
	if (gbox.nx)
	    PF formatCX, d2a (resol * gbox.dx), gbox.nx);
	if (gbox.ny)
	    PF formatCY, (gbox.nx ? "," : ""), d2a (resol * gbox.dy), gbox.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}

static void cmk_nor_copy (char *cell)
{
    double  xs, ys, r1, r2, step, h1, h2;
    int     cpeel, n_edges, no_xy;
    char   *layer;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "nor", "r");

    while (dmGetDesignData (stream, GEO_NOR_INI) > 0) {
	no_xy = gnor_ini.no_xy;
	layer = lay[gnor_ini.layer_no];

	if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
	    error (2, cell);
	xs = resol * gnor_xy.x;
	ys = resol * gnor_xy.y;

	switch (gnor_ini.elmt) {
	    case WIRE_NOR:
		if (--no_xy < 2) error (9, cell);
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		PF formatPA, layer, d2a (resol * gnor_xy.x), d2a (resol * gnor_xy.y));
		--no_xy;
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		PF formatRR, d2a (resol * gnor_xy.x), d2a (resol * gnor_xy.y));
		PF ",%s", d2a (xs));
		while (--no_xy > 0) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		    PF formatRR, d2a (resol * gnor_xy.x), d2a (resol * gnor_xy.y));
		}
		PF ")");
		break;
	    case SBOX_NOR:
		if (no_xy != 2) error (9, cell);
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		PF formatRS, layer, d2a (xs), d2a (ys),
		    d2a (resol * gnor_xy.x), d2a (resol * gnor_xy.y));
		break;
	    case RECT_NOR:
		if (no_xy != 4) error (9, cell);
	    case POLY_NOR:
		if (no_xy < 3) error (9, cell);
		PF formatPO, layer, d2a (xs), d2a (ys));
		while (--no_xy > 0) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		    PF formatAA, d2a (resol * gnor_xy.x), d2a (resol * gnor_xy.y));
		}
		PF formatAA, d2a (xs), d2a (ys));
		PF ")");
		break;
	    case CIRCLE_NOR:
		if (no_xy < 2 || no_xy > 4) error (9, cell);
		cpeel = 0;
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		r2 = resol * gnor_xy.x;
		if ((r1 = resol * gnor_xy.y) > 0.0) ++cpeel;
		step = 0;
		if (no_xy > 2) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		    step = gnor_xy.x;
		}
		h1 = 0;
		h2 = 360;
		if (no_xy > 3) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0) error (2, cell);
		    h1 = gnor_xy.x;
		    h2 = gnor_xy.y;
		    ++cpeel;
		}
		PF "%s(%s,%s,%s", (cpeel ? strCP : strCI), layer, d2a (xs), d2a (ys));
		if (cpeel)
		    PF ",%s,%s,%s,%s", d2a (r1), d2a (r2), d2a (h1), d2a (h2));
		else
		    PF ",%s", d2a (r2));
		n_edges = 32000;
		if (step > 0) {
		    if (step >= 8) {
			n_edges = step;
			if (n_edges % 8) n_edges = 360.0 / step + 0.5;
		    }
		    else n_edges = 360.0 / step + 0.5;
		    n_edges /= 8;
		    n_edges *= 8;
		    if (n_edges < 8) n_edges = 8;
		    if (n_edges > 32000) n_edges = 32000;
		}
		if (n_edges == 32)
		    PF ")");
		else
		    PF ",%d)", n_edges);
		break;
	    default:
		error (6, cell);
	}

	if (gnor_ini.nx)
	    PF formatCX, d2a (resol * gnor_ini.dx), gnor_ini.nx);
	if (gnor_ini.ny)
	    PF formatCY, (gnor_ini.nx ? "," : ""),
		d2a (resol * gnor_ini.dy), gnor_ini.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}

static void cmk_mc_copy ()
{
    char  newname[DM_MAXNAME + 1];
    int   mir, rot, sfx, sfy, comma;
    register char *c, *s;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "mc", "r");

    while (dmGetDesignData (stream, GEO_MC) > 0) {

	if (new_names || (gmc.imported && i_mode)) {
	    struct mo_elmt *q;
	    q = findcell (ckey -> dmproject, gmc.cell_name);
	    c = q -> nname;
	}
	else
	    c = gmc.cell_name;

        if (strcmp (c, skipCellName) == 0)
            continue;

	s = newname;
	while ((*s = *c++))
	    if (*s++ == '_') *(s - 1) = '-';
	PF "%s", newname);

	if (gmc.mtx[0] == 0) {
	    if (gmc.mtx[3] > 0) {
		if (gmc.mtx[1] > 0) {/* MX+R90 */
		    mir = 1;
		    rot = 90;
		    sfx = gmc.mtx[3];
		    sfy = gmc.mtx[1];
		}
		else {		/* R90 */
		    mir = 0;
		    rot = 90;
		    sfx = gmc.mtx[3];
		    sfy = -gmc.mtx[1];
		}
	    }
	    else {
		if (gmc.mtx[1] > 0) {/* R270 */
		    mir = 0;
		    rot = 270;
		    sfx = -gmc.mtx[3];
		    sfy = gmc.mtx[1];
		}
		else {		/* MX+R270 */
		    mir = 1;
		    rot = 270;
		    sfx = -gmc.mtx[3];
		    sfy = -gmc.mtx[1];
		}
	    }
	}
	else {
	    if (gmc.mtx[0] > 0) {
		if (gmc.mtx[4] > 0) {/* R0 */
		    mir = 0;
		    rot = 0;
		    sfx = gmc.mtx[0];
		    sfy = gmc.mtx[4];
		}
		else {		/* MX+R0 */
		    mir = 1;
		    rot = 0;
		    sfx = gmc.mtx[0];
		    sfy = -gmc.mtx[4];
		}
	    }
	    else {
		if (gmc.mtx[4] > 0) {/* MX+R180 */
		    mir = 1;
		    rot = 180;
		    sfx = -gmc.mtx[0];
		    sfy = gmc.mtx[4];
		}
		else {		/* R180 */
		    mir = 0;
		    rot = 180;
		    sfx = -gmc.mtx[0];
		    sfy = -gmc.mtx[4];
		}
	    }
	}

	comma = 0;
	if (mir) {   PF formatMX); comma++; }
	if (rot)     PF formatRO, (comma++ ? "," : " "), rot);
	if (sfx > 1) PF formatSX, (comma++ ? "," : " "), sfx);
	if (sfy > 1) PF formatSY, (comma++ ? "," : " "), sfy);

	if (gmc.mtx[2] || gmc.mtx[5])
	    PF "%s%s,%s", (comma++ ? "," : " "),
		d2a (resol * gmc.mtx[2]), d2a (resol * gmc.mtx[5]));

	if (gmc.nx) {
	    PF "%s", (comma++ ? "," : " "));
	    PF formatCX, d2a (resol * gmc.dx), gmc.nx);
	}
	if (gmc.ny)
	    PF formatCY, (comma ? "," : " "), d2a (resol * gmc.dy), gmc.ny);

	if (strcmp (gmc.inst_name, ".") != 0)
	    PF " : <%s>\n", gmc.inst_name);
	else
	    PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}
