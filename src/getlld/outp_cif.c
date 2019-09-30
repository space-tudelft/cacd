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

static int Lay_no = -1;
static int Symbol =  0;

static void cif_term_copy (void);
static void cif_label_copy (void);
static void cif_box_copy (void);
static void cif_nor_copy (char *);
static void cif_mc_copy (void);
static void GenBox (int, int, int, int, int, int);
static int LS (int);
static int LsD (double);

void outp_cif (struct mo_elmt *p)
{
    p -> snr = ++Symbol;
    PF "DS%d 1 1;(%s);\n", Symbol, p -> nname); /* Definition Start */
    PF "9 %s;\n", p -> nname);
    Lay_no = -1;
    cif_term_copy ();
    cif_label_copy ();
    cif_box_copy ();
    cif_nor_copy (p -> oname);
    if (p -> mcalls) cif_mc_copy ();
    PF "DF;\n");		/* Definition Finish */
    if (p -> level == 1) PF "C%d;\n", Symbol);
}

static void cif_term_copy ()
{
    int xl, xr, yb, yt, w, h;
    register int i, j;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "term", "r");

    while (dmGetDesignData (stream, GEO_TERM) > 0) {
	if (gterm.layer_no != Lay_no) {
	    Lay_no = gterm.layer_no;
	    PF "L%s;\n", lay[Lay_no]);
	}
	w = LS ((int)(gterm.xr - gterm.xl));
	h = LS ((int)(gterm.yt - gterm.yb));
	for (i = 0; i <= gterm.nx; ++i) {
	    for (j = 0; j <= gterm.ny; ++j) {
		xl = gterm.xl + i * gterm.dx;
		xr = gterm.xr + i * gterm.dx;
		yb = gterm.yb + j * gterm.dy;
		yt = gterm.yt + j * gterm.dy;
		/* GenBox (xl, xr, yb, yt, 1, 0); */
		if (gterm.nx == 0 && gterm.ny == 0)
		    PF "95 %s %d %d %d %d %s;\n", gterm.term_name, w, h,
			LS (xr + xl) / 2, LS (yt + yb) / 2, lay[Lay_no]);
		else
		    PF "95 %s%d%d %d %d %d %d %s;\n", gterm.term_name, i, j, w, h,
			LS (xr + xl) / 2, LS (yt + yb) / 2, lay[Lay_no]);
	    }
	}
    }
    dmCloseStream (stream, COMPLETE);
}

static void cif_label_copy ()
{
    register DM_STREAM *stream;
    struct stat statbuf;

    if (dmStat (ckey, "annotations", &statbuf) == 0) {
        stream = dmOpenStream (ckey, "annotations", "r");
        while (dmGetDesignData (stream, GEO_ANNOTATE) > 0) {
            if (ganno.type == GA_LABEL) {
		if (ganno.o.Label.maskno != Lay_no) {
		    Lay_no = ganno.o.Label.maskno;
		    PF "L%s;\n", lay[Lay_no]);
		}
		PF "94 %s %d %d %s;\n", ganno.o.Label.name,
		    LS ((int)ganno.o.Label.x), LS ((int)ganno.o.Label.y), lay[Lay_no]);
            }
        }
        dmCloseStream (stream, COMPLETE);
    }
}

static void cif_box_copy ()
{
    int xl, xr, yb, yt;
    register int i, j;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "box", "r");

    while (dmGetDesignData (stream, GEO_BOX) > 0) {
	if (gbox.layer_no != Lay_no) {
	    Lay_no = gbox.layer_no;
	    PF "L%s;\n", lay[Lay_no]);
	}
	for (i = 0; i <= gbox.nx; ++i) {
	    for (j = 0; j <= gbox.ny; ++j) {
		xl = gbox.xl + i * gbox.dx;
		xr = gbox.xr + i * gbox.dx;
		yb = gbox.yb + j * gbox.dy;
		yt = gbox.yt + j * gbox.dy;
		GenBox (xl, xr, yb, yt, 1, 0);
	    }
	}
    }
    dmCloseStream (stream, COMPLETE);
}

static void cif_nor_copy (char *cell)
{
    double  xs, ys, r1, r2;
    int     no_xy, xx, yy;
    register int i, j;
    register DM_STREAM *stream;

    stream = dmOpenStream (ckey, "nor", "r");

    while (dmGetDesignData (stream, GEO_NOR_INI) > 0) {
	if (gnor_ini.layer_no != Lay_no) {
	    Lay_no = gnor_ini.layer_no;
	    PF "L%s;\n", lay[Lay_no]);
	}
	for (i = 0; i <= gnor_ini.nx; ++i) {
	    for (j = 0; j <= gnor_ini.ny; ++j) {
		no_xy = gnor_ini.no_xy;

		if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
		    error (2, cell);
		xs = gnor_xy.x;
		ys = gnor_xy.y;

		switch (gnor_ini.elmt) {
		    case WIRE_NOR:
			if (no_xy < 3) error (9, cell);
			PF "W%d", LsD (xs));
			xx = 0;
			yy = 0;
			while (--no_xy > 0) {
			    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
				error (2, cell);
			    PF " %d %d",
				LsD ((gnor_xy.x + xx) + i * gnor_ini.dx),
				LsD ((gnor_xy.y + yy) + j * gnor_ini.dy));
			    xx = xx + gnor_xy.x;
			    yy = yy + gnor_xy.y;
			}
			break;
		    case SBOX_NOR:
			if (no_xy != 2) error (9, cell);
			if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			    error (2, cell);
			GenBox (LsD (xs + i * gnor_ini.dx),
				LsD (gnor_xy.x + i * gnor_ini.dx),
				LsD (ys + j * gnor_ini.dy),
				LsD (gnor_xy.y + j * gnor_ini.dy),
				1, 1);
			break;
		    case RECT_NOR:
			if (no_xy != 4) error (9, cell);
		    case POLY_NOR:
			if (no_xy < 3) error (9, cell);
			PF "P%d %d",
			    LsD (xs + i * gnor_ini.dx),
			    LsD (ys + j * gnor_ini.dy));
			while (--no_xy > 0) {
			    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
				error (2, cell);
			    PF " %d %d",
				LsD (gnor_xy.x + i * gnor_ini.dx),
				LsD (gnor_xy.y + j * gnor_ini.dy));
			}
			break;
		    case CIRCLE_NOR:
			if (no_xy < 2 || no_xy > 4) error (9, cell);
			if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
			    error (2, cell);
			r1 = gnor_xy.x;
			r2 = gnor_xy.y;
			if (no_xy > 2) {
			    if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
				error (2, cell);
			}
			if (no_xy > 3 || r2 > 0.0) {
			    PF "(cpeel %s %s %s %s %s", ldmlay[Lay_no],
				d2a (xs), d2a (ys), d2a (r1), d2a (r2));
			    if (no_xy > 3) {
				if (dmGetDesignData (stream, GEO_NOR_XY) <= 0)
				    error (2, cell);
				PF " %s %s)", d2a (gnor_xy.x), d2a (gnor_xy.y));
			    }
			    else {
				PF " 0 360)");
			    }
			}
			else {
			    PF "R%d %d %d",
				LsD (xs + i * gnor_ini.dx),
				LsD (ys + j * gnor_ini.dy),
				LsD (2 * r1));
			}
			break;
		    default:
			error (6, cell);
		}
		PF ";\n");
	    }
	}
    }
    dmCloseStream (stream, COMPLETE);
}

static void cif_mc_copy ()
{
    int     tx, ty, pos, sfx, sfy, snr;
    register int    i, j;
    register DM_STREAM *stream;
    char *cellname;

    stream = dmOpenStream (ckey, "mc", "r");

    while (dmGetDesignData (stream, GEO_MC) > 0) {

	if (r_mode || (gmc.imported && !i_mode)) {
	    cellname = gmc.cell_name;
	    snr = 0;
	}
	else {
	    struct mo_elmt *q;
	    q = findcell (ckey -> dmproject, gmc.cell_name);
	    cellname = q -> nname;
	    snr = q -> snr;
	}

        if (strcmp (cellname, skipCellName) == 0)
            continue;

	if (gmc.mtx[0] == 0) {
	    if (gmc.mtx[3] > 0) {
		if (gmc.mtx[1] > 0) {/* MX+R90 */
		    pos = 5;
		    sfx = gmc.mtx[3];
		    sfy = gmc.mtx[1];
		}
		else {	/* R90 */
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
		else {	/* MY+R90 */
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
		else {	/* MX+R0 */
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
		else {	/* R180 */
		    pos = 2;
		    sfx = -gmc.mtx[0];
		    sfy = -gmc.mtx[4];
		}
	    }
	}

	if (sfx > 1 || sfy > 1) {
	    PE "Warning: C%d: cell scaling not possible (sfx/y)\n", snr);
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

	for (i = 0; i <= gmc.nx; ++i) {
	    for (j = 0; j <= gmc.ny; ++j) {

		PF "C%d", snr);

		switch (pos) {
		    case 1:
			PF " R%d %d", LS (0), LS (1));
			break;
		    case 2:
			PF " R%d %d", LS (-1), LS (0));
			break;
		    case 3:
			PF " R%d %d", LS (0), LS (-1));
			break;
		    case 4:
			PF " MY");
			break;
		    case 5:
			PF " MY R%d %d", LS (0), LS (1));
			break;
		    case 6:
			PF " MX");
			break;
		    case 7:
			PF " MX R%d %d", LS (0), LS (1));
			break;
		}

		if (tx || ty)
		    PF " T%d %d", LS (tx + i * gmc.dx), LS (ty + j * gmc.dy));

		PF ";(%s);\n", cellname);
	    }
	}
    }
    dmCloseStream (stream, COMPLETE);
}

static void GenBox (int Xl, int Xr, int Yb, int Yt, int XDir, int YDir)
{
    PF "B%d %d %d %d",
	LS (Xr - Xl), LS (Yt - Yb),/* Length, Width */
	LS (Xr + Xl) / 2, LS (Yt + Yb) / 2);/* X, Y */
    if (XDir != 1 || YDir != 0)
	PF " %d %d", LS (XDir), LS (YDir));
    PF ";\n");
}

#define ROUNDTEST(x) ((100 * ((int)((x)))) != (int)(100.0*(x)))

static int LS (int i) /* Lambda Scale */
{
    double  s, ss;
    ss = resol * 100.0;
    s = i * ss;
    if (ROUNDTEST (s)) {
	PE "Warning: integer truncation (from int): %d %lf\n", (int) s, s);
	/* PE "Warning: %d %lf %lf %lf\n", i, resol, ss, s); */
    }
    return ((int) s);
}

static int LsD (double d) /* Lambda scale Double */
{
    double s, ss;
    ss = resol * 100.0;
    s = d * ss;
    if (ROUNDTEST (s)) {
	PE "Warning: integer truncation (from double): %d %lf\n", (int) s, s);
	/* PE "Warning: %lf %lf %lf %lf\n", d, resol, ss, s); */
    }
    return ((int) s);
}
