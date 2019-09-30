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

extern struct event_rec *events;

struct event_rec *act_event;
struct sr_field head_sr, *h_sr;
long   sr_pos;

/*
** convert the boxes in the sorted event_list to line segments
*/
void convert (long max_event_no)
{
    long   next_stop_pos;
    struct sr_field *r_sr;
    struct event_rec *max_event;

    act_event = events;
    max_event = &events[max_event_no];
    sr_pos = act_event -> xl;
    next_stop_pos = MAX_INTEGER;

    h_sr = &head_sr;
    h_sr -> next = h_sr -> prev = h_sr;
    h_sr -> yt = MIN_INTEGER;
    h_sr -> yb = MAX_INTEGER;
    r_sr = h_sr;

    while (act_event < max_event) {
	if (sr_pos != act_event -> xl) {
	    /* generate occurences */
	    do {
		next_stop_pos = MAX_INTEGER;
		ScanProf (&next_stop_pos);
	    }
	    while ((sr_pos = next_stop_pos) < act_event -> xl);
	    sr_pos = act_event -> xl;
	    r_sr = h_sr -> next;
	}
	insert_event (&r_sr, &next_stop_pos);
	++act_event;
    }

    /* generate final stop occurences */
    do {
	next_stop_pos = MAX_INTEGER;
	ScanProf (&next_stop_pos);
    }
    while ((sr_pos = next_stop_pos) < MAX_INTEGER);
}
