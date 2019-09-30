/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/makevln/incl.h"

extern struct sr_field *h_sr;

#ifdef DEBUG
void pr_event (struct event_rec *event)
{
    printf ("<- %6d %6d %6d %6d %6d\n",
	event -> xl, event -> xr,
	event -> yb, event -> yt,
	event -> attr_no);
}

void pr_evt_list (struct event_rec *event1, struct event_rec *event2)
{
    struct event_rec *event;

    for (event = event1; event <= event2; ++event)
	pr_event (event);
}

void pr_prof (struct sr_field *c_sr, int scan_mode)
{
    if (!c_sr) {
	if (scan_mode)
	    c_sr = h_sr -> next;
	else
	    c_sr = h_sr -> prev;
    }

    while (c_sr != h_sr) {
	printf ("%11d %11d %11d %11d %3d",
		c_sr -> yb, c_sr -> yt, c_sr -> duration,
		c_sr -> ol_dur, c_sr -> checktype);
	if (c_sr -> flag.start   ) printf (" s");
	if (c_sr -> flag.overlap ) printf (" o");
	if (c_sr -> flag.ol_area ) printf (" a");
	if (c_sr -> flag.incident) printf (" i");
	if (c_sr -> flag.ct_zero ) printf (" c");
	printf ("\n");
	if (scan_mode)
	    c_sr = c_sr -> next;
	else
	    c_sr = c_sr -> prev;
    }
}
#endif /* DEBUG */
