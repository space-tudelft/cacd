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

static void ldm_term_copy (void);
static void ldm_label_copy (void);
static void ldm_box_copy (void);
static void ldm_nor_copy (char *);
static void ldm_mc_copy (void);

void outp_ldm (struct mo_elmt *p)
{
    PF "ms %s\n", p -> nname);
    ldm_term_copy ();
    ldm_label_copy ();
    ldm_box_copy ();
    ldm_nor_copy (p -> oname);
    if (p -> mcalls) ldm_mc_copy ();
    PF "me\n");
}

static void ldm_term_copy ()
{
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "term", "r");
    while (dmGetDesignData (stream, GEO_TERM) > 0) {
	PF "term %s %ld %ld %ld %ld %s", ldmlay[gterm.layer_no],
	    gterm.xl, gterm.xr, gterm.yb, gterm.yt, gterm.term_name);
	if (gterm.nx) PF " cx %ld %ld", gterm.dx, gterm.nx);
	if (gterm.ny) PF " cy %ld %ld", gterm.dy, gterm.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}

static void ldm_label_copy ()
{
    register DM_STREAM *stream;
    struct stat statbuf;

    if (dmStat (ckey, "annotations", &statbuf) == 0) {
        stream = dmOpenStream (ckey, "annotations", "r");
        while (dmGetDesignData (stream, GEO_ANNOTATE) > 0) {
            if (ganno.type == GA_LABEL) {
	        PF "label %s %ld %ld %s", ldmlay[ganno.o.Label.maskno],
	            (long)ganno.o.Label.x, (long)ganno.o.Label.y,
                    ganno.o.Label.name);
	        PF "\n");
            }
        }
        dmCloseStream (stream, COMPLETE);
    }
}

static void ldm_box_copy ()
{
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "box", "r");
    while (dmGetDesignData (stream, GEO_BOX) > 0) {
	PF "box %s %ld %ld %ld %ld", ldmlay[gbox.layer_no],
	    gbox.xl, gbox.xr, gbox.yb, gbox.yt);
	if (gbox.nx) PF " cx %ld %ld", gbox.dx, gbox.nx);
	if (gbox.ny) PF " cy %ld %ld", gbox.dy, gbox.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}

static void ldm_nor_copy (char *cell)
{
    double  xs, ys, r1, r2, step;
    int     n_edges, no_xy;
    char   *layer;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "nor", "r");

    while (dmGetDesignData (stream, GEO_NOR_INI) > 0) {
	layer = ldmlay[gnor_ini.layer_no];
	no_xy = gnor_ini.no_xy;

	if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
	    error (2, cell);
	xs = gnor_xy.x;
	ys = gnor_xy.y;

	switch (gnor_ini.elmt) {
	    case WIRE_NOR:
		if (no_xy < 3) error (9, cell);
		PF "swire %s w %s", layer, d2a (xs));
		while (--no_xy > 0) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			error (2, cell);
		    PF " %s %s", d2a (gnor_xy.x), d2a (gnor_xy.y));
		}
		break;
	    case SBOX_NOR:
		if (no_xy != 2) error (9, cell);
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
		    error (2, cell);
		PF "sbox %s %s %s %s %s", layer,
		    d2a (xs), d2a (ys), d2a (gnor_xy.x), d2a (gnor_xy.y));
		break;
	    case RECT_NOR:
		if (no_xy != 4) error (9, cell);
	    case POLY_NOR:
		if (no_xy < 3) error (9, cell);
		PF "poly %s %s %s", layer, d2a (xs), d2a (ys));
		while (--no_xy > 0) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			error (2, cell);
		    PF " %s %s", d2a (gnor_xy.x), d2a (gnor_xy.y));
		}
		PF " %s %s", d2a (xs), d2a (ys));
		break;
	    case CIRCLE_NOR:
		if (no_xy < 2 || no_xy > 4) error (9, cell);
		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
		    error (2, cell);
		r1 = gnor_xy.x;
		r2 = gnor_xy.y;
		step = 0;
		if (no_xy > 2) {
		    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			error (2, cell);
		    step = gnor_xy.x;
		}
		if (no_xy > 3 || r2 > 0.0) {
		    PF "cpeel %s %s %s %s %s",
			layer, d2a (xs), d2a (ys), d2a (r1), d2a (r2));
		    if (no_xy > 3) {
			if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			    error (2, cell);
			PF " %s %s", d2a (gnor_xy.x), d2a (gnor_xy.y));
		    }
		    else {
			PF " 0 360");
		    }
		}
		else {
		    PF "circle %s %s %s %s", layer, d2a (xs), d2a (ys), d2a (r1));
		}
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
		if (n_edges != 32) PF " %d", n_edges);
		break;
	    default:
		error (6, cell);
	}

	if (gnor_ini.nx)
	    PF " cx %s %ld", d2a (gnor_ini.dx), gnor_ini.nx);
	if (gnor_ini.ny)
	    PF " cy %s %ld", d2a (gnor_ini.dy), gnor_ini.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}

static void ldm_mc_copy ()
{
    int     tx, ty, pos, sfx, sfy;
    register DM_STREAM *stream;
    char *cellname;

    stream = dmOpenStream (ckey, "mc", "r");

    while (dmGetDesignData (stream, GEO_MC) > 0) {

	if (new_names || (gmc.imported && i_mode)) {
	    struct mo_elmt *q;
	    q = findcell (ckey -> dmproject, gmc.cell_name);
	    cellname = q -> nname;
	}
	else
	    cellname = gmc.cell_name;

        if (strcmp (cellname, skipCellName) == 0)
            continue;

	if (strcmp (gmc.inst_name, ".") == 0)
	    PF "mc %s", cellname);
	else
	    PF "mc <%s> %s", gmc.inst_name, cellname);

	if (gmc.mtx[0] == 0) {
	    if (gmc.mtx[3] > 0) {
		if (gmc.mtx[1] > 0) {/* MX+R90 */
		    pos = 5;
		    sfx = gmc.mtx[3];
		    sfy = gmc.mtx[1];
		}
		else {		/* R90 */
		    pos = 1;
		    sfx = gmc.mtx[3];
		    sfy = -gmc.mtx[1];
		}
	    }
	    else {
		if (gmc.mtx[1] > 0) {/* R270 */
		    pos = 3;
		    sfx = -gmc.mtx[3];
		    sfy = gmc.mtx[1];
		}
		else {		/* MY+R90 */
		    pos = 7;
		    sfx = -gmc.mtx[3];
		    sfy = -gmc.mtx[1];
		}
	    }
	}
	else {
	    if (gmc.mtx[0] > 0) {
		if (gmc.mtx[4] > 0) {/* R0 */
		    pos = 0;
		    sfx = gmc.mtx[0];
		    sfy = gmc.mtx[4];
		}
		else {		/* MX+R0 */
		    pos = 4;
		    sfx = gmc.mtx[0];
		    sfy = -gmc.mtx[4];
		}
	    }
	    else {
		if (gmc.mtx[4] > 0) {/* MY */
		    pos = 6;
		    sfx = -gmc.mtx[0];
		    sfy = gmc.mtx[4];
		}
		else {		/* R180 */
		    pos = 2;
		    sfx = -gmc.mtx[0];
		    sfy = -gmc.mtx[4];
		}
	    }
	}

	if (sfx > 1) PF " sfx %d", sfx);
	if (sfy > 1) PF " sfy %d", sfy);

	switch (pos) {
	    case 1: PF " r3"); break;
	    case 2: PF " r6"); break;
	    case 3: PF " r9"); break;
	    case 4: PF " mx"); break;
	    case 5: PF " mx r3"); break;
	    case 6: PF " my"); break;
	    case 7: PF " my r3"); break;
	}

	if (!x_mode) { /* no-origin mode */
	    tx = gmc.mtx[2];
	    ty = gmc.mtx[5];
	}
	else {
	    tx = gmc.bxl;
	    ty = gmc.byb;
	    if (gmc.dx < 0) gmc.dx = -gmc.dx;
	    if (gmc.dy < 0) gmc.dy = -gmc.dy;
	}
	if (tx || ty) PF " %d %d", tx, ty);
	if (gmc.nx) PF " cx %ld %ld", gmc.dx, gmc.nx);
	if (gmc.ny) PF " cy %ld %ld", gmc.dy, gmc.ny);
	PF "\n");
    }
    dmCloseStream (stream, COMPLETE);
}
