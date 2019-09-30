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

extern TERM **term_root;
extern TERM **TBuf;
extern TERMREF *termlist, *termlistlast;
extern int  NR_lay;
extern int  Textnr;
extern int  new_cmd;
extern int  erase_text;
extern int *non_edit;
extern int *pict_arr;
extern Coor piwl, piwr, piwb, piwt;
extern Coor xlc, xrc, ybc, ytc; /* cursor parameters */

void fill_tbuf ()
{
    register int lay;
    register TERM *pt, *tp, *next_p, *prev;
    Coor  ll, rr, bb, tt, b_x1, b_y1, b_x2, b_y2;
    char  err_str[MAXCHAR];
    int   count = 0;

    if (!present_term ()) return;

    b_x1 = b_y1 = b_x2 = b_y2 = 0; // init, to suppress compiler warning
    ptext ("Select area with terminals to delete!");
    erase_text = 1;
    if ((new_cmd = get_cursor (5, 2, SNAP)) != -1) return;

    for (lay = 0; lay < NR_lay; ++lay) {
	for (pt = TBuf[lay]; pt; pt = next_p) {
	    next_p = pt -> nxttm;
	    FREE (pt -> tmname);
	    FREE (pt);
	}
	if (non_edit[lay]) continue;

	prev = NULL; /* previous tp */
	for (tp = term_root[lay]; tp; tp = next_p) {
	    next_p = tp -> nxttm;
	    term_win (tp, &ll, &rr, &bb, &tt, 1);
	    /* terminal complete in delete area? */
	    if (ll >= xlc && rr <= xrc && bb >= ybc && tt <= ytc) {
		TERMREF *t, *prevt;
		prevt = 0;
		for (t = termlist; t && t -> tp != tp; t = t -> next) prevt = t;
		if (t) {
		    if (prevt) prevt -> next = t -> next;
		    else termlist = t -> next;
		    if (t == termlistlast) termlistlast = prevt;
		    FREE (t);
		}

		if (!prev) term_root[lay] = next_p;
		else prev -> nxttm = next_p;
		tp -> nxttm = pt;
		pt = tp;
		if (++count == 1) {
		    b_x1 = ll;
		    b_y1 = bb;
		    b_x2 = rr;
		    b_y2 = tt;
		}
		else {
		    if (ll < b_x1) b_x1 = ll;
		    if (bb < b_y1) b_y1 = bb;
		    if (rr > b_x2) b_x2 = rr;
		    if (tt > b_y2) b_y2 = tt;
		}
	    }
	    else prev = tp;
	}

	if ((TBuf[lay] = pt)) pict_arr[lay] = ERAS_DR;
    }
    if (count) {
	pict_arr[Textnr] = ERAS_DR;
	piwl = b_x1; piwb = b_y1;
	piwr = b_x2; piwt = b_y2;
    }
    sprintf (err_str, "Buffer filled with %d deleted terminals!", count);
    ptext (err_str);
}

void yank_tbuf ()
{
    register int lay;
    register TERM *tp, *next_p;
    Coor  ll, rr, bb, tt;
    char  err_str[MAXCHAR];
    int   count = 0;

    if (!present_term ()) return;

    ptext ("Select area with terminals to yank!");
    erase_text = 1;
    if ((new_cmd = get_cursor (5, 2, SNAP)) != -1) return;

    for (lay = 0; lay < NR_lay; ++lay) {
	for (tp = TBuf[lay]; tp; tp = next_p) {
	    next_p = tp -> nxttm;
	    FREE (tp -> tmname);
	    FREE (tp);
	}
	TBuf[lay] = NULL;
	if (non_edit[lay]) continue;

	for (tp = term_root[lay]; tp; tp = tp -> nxttm) {
	    term_win (tp, &ll, &rr, &bb, &tt, 0);
	    /* terminal complete in yank area? */
	    if (ll >= xlc && rr <= xrc && bb >= ybc && tt <= ytc) {
		if (create_term (&TBuf[lay],
		    tp -> xl, tp -> xr, tp -> yb, tp -> yt,
		    tp -> tmname, tp -> tmlen,
		    tp -> dx, tp -> nx, tp -> dy, tp -> ny)) ++count;
	    }
	}
    }
    sprintf (err_str, "Buffer filled with %d terminals!", count);
    ptext (err_str);
}
