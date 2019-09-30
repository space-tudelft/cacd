/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/flatten/extern.h"

void read_mc ()
{
    DM_STREAM *fp = dmOpenStream (cellkey, "mc", "r");

    if (dmGetDesignData (fp, GEO_MC) > 0) {
if (ldmfile) {
	if (cellkey -> dmproject != project)
	    PO ":: lib=%s cell=%s\n",
		cellkey -> dmproject -> projectid, cellkey -> cell);
}
	exp_mc ();
	while (dmGetDesignData (fp, GEO_MC) > 0) exp_mc ();
if (ldmfile) {
	if (cellkey -> dmproject != project) PO ":: eof mc's\n");
}
    }

    dmCloseStream (fp, COMPLETE);
}

void exp_mc ()
{
    register long *m, *a;
    register struct tmtx *tm;
    long Dx, Nx, Dy, Ny, Xl, Xr, Yb, Yt;
    long xl, xr, yb, yt, tmp;
    long dx, nx, dy, ny, b[6];
    int  pos, sfx, sfy, second = 0;

    a = gmc.mtx;
    Dx = gmc.dx;
    Nx = gmc.nx;
    Dy = gmc.dy;
    Ny = gmc.ny;

    if (!ldmfile) {
	Xl = gmc.bxl; Xr = gmc.bxr;
	Yb = gmc.byb; Yt = gmc.byt;
    }
    else { Xl = Xr = Yb = Yt = 0; } /* suppres uninitialized warning */

    for (tm = tm_p; tm; tm = tm -> tm_next) {
again:
	if (!second && !gmc.imported) continue;

	m = tm -> mtx;
	b[0] = m[0] * a[0] + m[1] * a[3];
	b[1] = m[0] * a[1] + m[1] * a[4];
	b[2] = m[0] * a[2] + m[1] * a[5] + m[2];
	b[3] = m[3] * a[0] + m[4] * a[3];
	b[4] = m[3] * a[1] + m[4] * a[4];
	b[5] = m[3] * a[2] + m[4] * a[5] + m[5];

	dx = dy = nx = ny = 0;
	if (Nx) {
	    if (m[0]) { nx = Nx; dx = m[0] * Dx; }
	    else      { ny = Nx; dy = m[3] * Dx; }
	}
	if (Ny) {
	    if (m[4]) { ny = Ny; dy = m[4] * Dy; }
	    else      { nx = Ny; dx = m[1] * Dy; }
	}

	if (ldmfile) {
	    if (b[0] == 0) {
		if (b[3] > 0) {
		    if (b[1] > 0) { /* MX+R90 */
			pos = 5;
			sfx = b[3];
			sfy = b[1];
		    }
		    else {          /* R90 */
			pos = 1;
			sfx = b[3];
			sfy = -b[1];
		    }
		}
		else {
		    if (b[1] > 0) { /* R270 */
			pos = 3;
			sfx = -b[3];
			sfy = b[1];
		    }
		    else {          /* MY+R90 */
			pos = 7;
			sfx = -b[3];
			sfy = -b[1];
		    }
		}
	    }
	    else {
		if (b[0] > 0) {
		    if (b[4] > 0) { /* R0 */
			pos = 0;
			sfx = b[0];
			sfy = b[4];
		    }
		    else {          /* MX+R0 */
			pos = 4;
			sfx = b[0];
			sfy = -b[4];
		    }
		}
		else {
		    if (b[4] > 0) { /* MY */
			pos = 6;
			sfx = -b[0];
			sfy = b[4];
		    }
		    else {          /* R180 */
			pos = 2;
			sfx = -b[0];
			sfy = -b[4];
		    }
		}
	    }

	    PO "mc %s", gmc.cell_name);

	    if (sfx > 1) PO " sfx %d", sfx);
	    if (sfy > 1) PO " sfy %d", sfy);

	    switch (pos) {
		case 1: PO " r3"); break;
		case 2: PO " r6"); break;
		case 3: PO " r9"); break;
		case 4: PO " mx"); break;
		case 5: PO " mx r3"); break;
		case 6: PO " my"); break;
		case 7: PO " my r3"); break;
	    }

	    if (b[2] || b[5]) PO " %ld %ld", b[2], b[5]);
	    if (nx) PO " cx %ld %ld", dx, nx);
	    if (ny) PO " cy %ld %ld", dy, ny);
	    PO "\n");
	}
	else {
	    gmc.inst_name[0] = '.';
	    gmc.inst_name[1] = '\0';

	    gmc.mtx[0] = b[0];
	    gmc.mtx[1] = b[1];
	    gmc.mtx[2] = b[2];
	    gmc.mtx[3] = b[3];
	    gmc.mtx[4] = b[4];
	    gmc.mtx[5] = b[5];

	    gmc.dx = dx; gmc.nx = nx;
	    gmc.dy = dy; gmc.ny = ny;

	    xl = m[0] * Xl + m[1] * Yb + m[2];
	    yb = m[3] * Xl + m[4] * Yb + m[5];
	    xr = m[0] * Xr + m[1] * Yt + m[2];
	    yt = m[3] * Xr + m[4] * Yt + m[5];
	    if (xl > xr) { tmp = xl; xl = xr; xr = tmp; }
	    if (yb > yt) { tmp = yb; yb = yt; yt = tmp; }
	    gmc.bxl = xl;
	    gmc.bxr = xr;
	    gmc.byb = yb;
	    gmc.byt = yt;

	    if (xl < mcbb_xl) mcbb_xl = xl;
	    if (xr > mcbb_xr) mcbb_xr = xr;
	    if (yb < mcbb_yb) mcbb_yb = yb;
	    if (yt > mcbb_yt) mcbb_yt = yt;

	    dmPutDesignData (fp_mc, GEO_MC);
	}
    }
    if (!second++ && (tm = tm_s)) goto again;
}
