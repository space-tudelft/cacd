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

extern TERM **term_root;
extern int  NR_lay;
extern int  Textnr;
extern int *edit_arr;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt;

static TERM **TBuf;
static int TBuf_width, TBuf_height;

static TERM *term_buf (int lay, Coor xl, Coor xr, Coor yb, Coor yt);

void fill_tbuf (Coor x1, Coor x2, Coor y1, Coor y2)
{
    register int lay;
    TERM *pt, *next_p;
    int new = !TBuf;

    if (new) MALLOCN (TBuf, TERM *, NR_lay);

    for (lay = 0; lay < NR_lay; ++lay) {
	if (!new)
	for (pt = TBuf[lay]; pt; pt = next_p) {
	    next_p = pt -> nxttm;
	    FREE (pt -> tmname);
	    FREE (pt);
	}
	if (!edit_arr[lay]) {
	    TBuf[lay] = NULL;
	    continue;
	}
	TBuf[lay] = term_buf (lay, x1, x2, y1, y2);
	for (pt = TBuf[lay]; pt; pt = pt -> nxttm) {
	    pt -> xl -= x1;
	    pt -> xr -= x1;
	    pt -> yb -= y1;
	    pt -> yt -= y1;
	}
    }
    TBuf_width = x2 - x1;
    TBuf_height = y2 - y1;
}

static TERM * term_buf (int lay, Coor xl, Coor xr, Coor yb, Coor yt)
{
    register TERM *tp, *new_f;
    TERM *first_p = NULL;
    Coor ll, rr, bb, tt;

    for (tp = term_root[lay]; tp; tp = tp -> nxttm) {
	term_win (tp, &ll, &rr, &bb, &tt);
	if (ll >= xl && rr <= xr && bb >= yb && tt <= yt) {
	    MALLOC (new_f, TERM);
	    if (!new_f) break;
	    new_f -> nxttm = first_p;
	    first_p = new_f;
	    new_f -> xl = tp -> xl;
	    new_f -> xr = tp -> xr;
	    new_f -> yb = tp -> yb;
	    new_f -> yt = tp -> yt;
	    new_f -> tmname = strsave (tp -> tmname);
	    new_f -> dx = tp -> dx;
	    new_f -> nx = tp -> nx;
	    new_f -> dy = tp -> dy;
	    new_f -> ny = tp -> ny;
	}
    }
    return (first_p);
}

int empty_tbuf ()
{
    register int lay;
    if (TBuf)
    for (lay = 0; lay < NR_lay; ++lay) {
	if (TBuf[lay]) return (FALSE);
    }
    return (TRUE);
}

void pict_tbuf (Coor x_c, Coor y_c)
{
    register TERM *tp;
    register int lay, it, jt;

    pict_rect ((float) x_c, (float) (x_c + TBuf_width), (float) y_c, (float) (y_c + TBuf_height));
    for (lay = 0; lay < NR_lay; ++lay) {
	for (tp = TBuf[lay]; tp; tp = tp -> nxttm) {
	    for (it = 0; it <= tp -> ny; ++it)
	    for (jt = 0; jt <= tp -> nx; ++jt) {
		pict_rect ((float) (x_c + tp -> xl + jt * tp -> dx),
			(float) (x_c + tp -> xr + jt * tp -> dx),
			(float) (y_c + tp -> yb + it * tp -> dy),
			(float) (y_c + tp -> yt + it * tp -> dy));
	    }
	}
    }
}

void place_tbuf (Coor xl, Coor yb)
{
    register int lay;
    register TERM *tp;

    for (lay = 0; lay < NR_lay; ++lay) {
	for (tp = TBuf[lay]; tp; tp = tp -> nxttm) {
	    if (!new_term (tp -> xl + xl, tp -> xr + xl, tp -> yb + yb, tp -> yt + yb,
			tp -> tmname, tp -> dx, tp -> nx, tp -> dy, tp -> ny, lay)) break;
	}
	if (TBuf[lay]) pict_arr[lay] = DRAW;
    }
    pict_arr[Textnr] = DRAW;
    piwl = xl;
    piwr = xl + TBuf_width;
    piwb = yb;
    piwt = yb + TBuf_height;
}
