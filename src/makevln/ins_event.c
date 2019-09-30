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

extern struct sr_field  *h_sr;
extern struct event_rec *act_event;

void ins_nwf (long bot, long top, struct sr_field **pc_sr, long *p_next_stop_pos);
void ins_olf (long bot, long top, struct sr_field **pc_sr, long *p_next_stop_pos);

/*
** insert an event in the reduced stateruler
*/
void insert_event (struct sr_field **pr_sr, long *p_next_stop_pos)
{
    struct sr_field *c_sr;

    if (*pr_sr == h_sr) {
    /*
    ** add event at end of stateruler
    */
	c_sr = h_sr -> prev;
	if (act_event -> yb < c_sr -> yt)
	    ins_olf (act_event -> yb,
		Min (act_event -> yt, c_sr -> yt),
		&c_sr, p_next_stop_pos);
	if (act_event -> yt > c_sr -> yt)
	    ins_nwf (Max (act_event -> yb, c_sr -> yt),
		act_event -> yt,
		&c_sr, p_next_stop_pos);
	*pr_sr = h_sr -> prev;
    }
    else {
	c_sr = *pr_sr;
	while (act_event -> yb > c_sr -> yt) {
	/*
	** update reduced stateruler
	*/
	    if ((c_sr = c_sr -> next) == h_sr)
		break;
	}
	*pr_sr = c_sr;

	if (c_sr == h_sr) {
	/*
	** add event at end of stateruler
	*/
	    c_sr = h_sr -> prev;
	    if (act_event -> yb < c_sr -> yt)
		ins_olf (act_event -> yb,
		    Min (act_event -> yt, c_sr -> yt),
		    &c_sr, p_next_stop_pos);
	    if (act_event -> yt > c_sr -> yt)
		ins_nwf (Max (act_event -> yb, c_sr -> yt),
		    act_event -> yt,
		    &c_sr, p_next_stop_pos);
	    *pr_sr = h_sr -> prev;
	}
	else {
	    if (act_event -> yb < c_sr -> yb) {
	    /*
	    ** event is positioned before reduced stateruler
	    */
		ins_nwf (act_event -> yb,
		    Min (act_event -> yt, c_sr -> yb), &(c_sr -> prev),
		    p_next_stop_pos);
		*pr_sr = c_sr -> prev;
	    }

	    if (act_event -> yt > c_sr -> yb
		&& act_event -> yb < c_sr -> yt) {
	    /*
	    ** event overlaps current stateruler field
	    */
		ins_olf (Max (act_event -> yb, c_sr -> yb),
		    Min (act_event -> yt, c_sr -> yt),
		    &c_sr, p_next_stop_pos);
	    }

	    while (act_event -> yt > c_sr -> yt) {
	    /*
	    ** event stretches above current stateruler field
	    */
		if (c_sr -> next == h_sr) {
		/*
		** add event at end of stateruler
		*/
		    ins_nwf (Max (c_sr -> yt, act_event -> yb),
			act_event -> yt,
			&c_sr, p_next_stop_pos);
		    break;
		}
		else {
		    if (c_sr -> yt != c_sr -> next -> yb) {
		    /*
		    ** insert a new field between current and next field
		    */
			ins_nwf (Max (c_sr -> yt, act_event -> yb),
			    Min (c_sr -> next -> yb, act_event -> yt),
			    &c_sr, p_next_stop_pos);
		    }

		    if (act_event -> yt > c_sr -> next -> yb) {
			c_sr = c_sr -> next;
			ins_olf (c_sr -> yb,
			    Min (act_event -> yt, c_sr -> yt),
			    &c_sr, p_next_stop_pos);
		    }
		    else
			c_sr = c_sr -> next;
		}
	    }
	}
    }
}
