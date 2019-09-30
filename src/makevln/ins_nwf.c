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

extern int term_layer;
extern struct event_rec *act_event;

/* Insert a new field in the stateruler after the field pointed
** to by c_sr.  On return let c_sr point to the new field.
*/
void ins_nwf (long bot, long top, struct sr_field **pc_sr, long *p_next_stop_pos)
{
    struct sr_field *new_field;

    ALLOCPTR (new_field, sr_field);
    new_field -> yt = top;
    new_field -> yb = bot;
    new_field -> duration = act_event -> xr;
    new_field -> ol_dur = MIN_INTEGER;
    if (term_layer)
	new_field -> checktype = 0;
    else
	new_field -> checktype = act_event -> attr_no;
    if (*p_next_stop_pos > act_event -> xr)
	*p_next_stop_pos = act_event -> xr;
    new_field -> flag.start = 1;
    new_field -> flag.overlap = 0;
    new_field -> next = (*pc_sr) -> next;
    new_field -> prev = *pc_sr;
    (*pc_sr) -> next = new_field;
    new_field -> next -> prev = new_field;
    set_group (new_field);
    *pc_sr = new_field;
}
