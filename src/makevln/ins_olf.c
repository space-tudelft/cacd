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
/**********************************************************
PROCESS DESCRIPTION

This routine inserts (part of) an event, denoted by its
event-number, into an existing sr-field.
If the event does not completely overlap the sr-field,
the sr-field is split, either in two (a top and an overlap
field), or in three (a top, overlap and bottom field).
The bottom and top fields inherit their attributes from
the existing sr-field, except for the values of yb and yt.
The overlap field also inherits these attributes,
EXCEPT when the checktypes of the event and the sr-field
are different.  The duration of an overlap field is the
maximum of the durations of the event and the sr-field.

**********************************************************/
#include "src/makevln/incl.h"

#define UPD_STOP_POS(p_stop_pos, pos) {\
if (*p_stop_pos > pos) *p_stop_pos = pos;}

extern int term_layer;
extern struct event_rec *act_event;

/*
** Insert (part of) an event into current sr_field
** Return pointer to (new) top field
*/
void ins_olf (long bot, long top, struct sr_field **pc_sr, long *p_next_stop_pos)
{
    struct sr_field *new_field1 = NULL;
    struct sr_field *new_field2 = NULL;

    if (top < (*pc_sr) -> yt) {
    /*
    ** insert new top_field
    ** 		-> alloc a field and copy attributes of
    **		   c_sr field into it, except for
    **		   bottom value of field
    **		-> update stateruler links
    */
	ALLOCPTR (new_field1, sr_field);
	new_field1 -> yt            = (*pc_sr) -> yt;
	new_field1 -> yb            = top;
	new_field1 -> group         = (*pc_sr) -> group;
	new_field1 -> duration      = (*pc_sr) -> duration;
	new_field1 -> ol_dur        = (*pc_sr) -> ol_dur;
	new_field1 -> checktype     = (*pc_sr) -> checktype;
	new_field1 -> flag.start    = (*pc_sr) -> flag.start;
	new_field1 -> flag.overlap  = (*pc_sr) -> flag.overlap;
	new_field1 -> flag.incident = (*pc_sr) -> flag.incident;
	new_field1 -> flag.ol_area  = (*pc_sr) -> flag.ol_area;
	new_field1 -> flag.ct_zero  = (*pc_sr) -> flag.ct_zero;
	new_field1 -> next          = (*pc_sr) -> next;
	new_field1 -> prev          = (*pc_sr);
	if (new_field1 -> next)
	    new_field1 -> next -> prev = new_field1;
	(*pc_sr) -> next = new_field1;
	(*pc_sr) -> yt   = top;
    }

    if (bot > (*pc_sr) -> yb) {
    /*
    ** insert new overlap field
    ** 		-> allocate new field and
    **		   determine attributes
    **		-> update yt of c_sr. (c_sr becomes the bottom field)
    */
	ALLOCPTR (new_field2, sr_field);
	new_field2 -> yt = top;
	new_field2 -> yb = bot;

	/* set attributes that can be inherited from c_sr */
	new_field2 -> group      = (*pc_sr) -> group;
	new_field2 -> flag.start = (*pc_sr) -> flag.start;
	new_field2 -> duration =
	    Max ((*pc_sr) -> duration, act_event -> xr);

	if (term_layer
	    || (*pc_sr) -> checktype == act_event -> attr_no) {
	/*
	** All further attributes can be inherited from
	** c_sr as well.
	*/
	    new_field2 -> ol_dur        = (*pc_sr) -> ol_dur;
	    new_field2 -> flag.overlap  = (*pc_sr) -> flag.overlap;
	    new_field2 -> flag.ol_area  = (*pc_sr) -> flag.ol_area;
	    new_field2 -> flag.incident = (*pc_sr) -> flag.incident;
	    new_field2 -> flag.ct_zero  = (*pc_sr) -> flag.ct_zero;
	    new_field2 -> checktype     = (*pc_sr) -> checktype;
	}
	else {
	/*
	** checktypes are different
	**	-> further attributes of overlap field
	**	   can not be inherited from c_sr.
	*/
	    if ((*pc_sr) -> flag.ol_area == TRUE) {
	    /*
	    ** already an overlapfield
	    */
		new_field2 -> flag.ol_area = TRUE;
		new_field2 -> flag.ct_zero = (*pc_sr) -> flag.ct_zero;
		if (new_field2 -> flag.ct_zero == TRUE  /* SDG: 04-Dec-1986 */
		&&  new_field2 -> flag.start   == TRUE) /* SDG: 08-May-1987 */
		    new_field2 -> flag.overlap = TRUE;

		if (act_event -> xr > (*pc_sr) -> duration) {
		/*
		** checktype of field after overlap finishes is
		** checktype of event
		*/
		   new_field2 -> checktype = act_event -> attr_no;
		}
		else {
		/*
		** checktype is inherited from c_sr
		*/
		   new_field2 -> checktype = (*pc_sr) -> checktype;
		}

		/*
		** determine duration of overlap
		*/
		new_field2 -> ol_dur =
		    Max (Min ((*pc_sr) -> duration, act_event -> xr),
			(*pc_sr) -> ol_dur);

		UPD_STOP_POS (p_next_stop_pos, new_field2 -> ol_dur);
	    }
	    else if ((*pc_sr) -> duration == act_event -> xl) {
	    /*
	    ** the stateruler field stops where the event starts
	    ** 		-> set incident flag
	    */
		new_field2 -> flag.incident = TRUE;
		new_field2 -> checktype     = act_event -> attr_no;
		/* set ol_dur to MIN_INTEGER J.Liedorp 29-8-85 */
		new_field2 -> ol_dur = MIN_INTEGER;
	    }
	    else {
	    /*
	    ** a new overlap field
	    */
		new_field2 -> flag.overlap = TRUE;
		new_field2 -> flag.ol_area = TRUE;

		/* if checktype of sr_field
		** is zero, then set the ct_zero flag.
		*/
		if ((*pc_sr) -> checktype == 0)
		    new_field2 -> flag.ct_zero = TRUE;

		if (act_event -> xr > (*pc_sr) -> duration) {
		/*
		** checktype of field after overlap finishes is
		** checktype of event
		*/
		    new_field2 -> checktype = act_event -> attr_no;
		}
		else {
		/*
		** checktype is inherited from c_sr
		*/
		    new_field2 -> checktype = (*pc_sr) -> checktype;
		}

		/*
		** determine duration of overlap
		*/
		new_field2 -> ol_dur =
		    Min ((*pc_sr) -> duration, act_event -> xr);

		UPD_STOP_POS (p_next_stop_pos, new_field2 -> ol_dur);
	    }
	}

	/* link overlap field in stateruler */
	new_field2 -> next = (*pc_sr) -> next;
	new_field2 -> prev = *pc_sr;
	if (new_field2 -> next)
	    new_field2 -> next -> prev = new_field2;
	(*pc_sr) -> next = new_field2;
	(*pc_sr) -> yt   = bot;
	set_group (new_field2);
    }
    else {
    /*
    ** No new bottom field required
    **		-> update attributes the same way as when
    **		   a new overlap field is allocated
    */
	if (!term_layer
	    && (*pc_sr) -> checktype != act_event -> attr_no) {

	    if ((*pc_sr) -> flag.ol_area == TRUE) {
		if (act_event -> xr > (*pc_sr) -> duration)
		    (*pc_sr) -> checktype = act_event -> attr_no;
		(*pc_sr) -> ol_dur =
		    Max (Min ((*pc_sr) -> duration, act_event -> xr),
			(*pc_sr) -> ol_dur);
		if ((*pc_sr) -> ol_dur < (*p_next_stop_pos))
		    (*p_next_stop_pos) = (*pc_sr) -> ol_dur;
	    }
	    else if ((*pc_sr) -> duration == act_event -> xl) {
		(*pc_sr) -> flag.incident = TRUE;
		(*pc_sr) -> checktype     = act_event -> attr_no;
	    }
	    else {
		(*pc_sr) -> flag.overlap = TRUE;
		(*pc_sr) -> flag.ol_area = TRUE;
		/*
		** if checktype of sr_field
		** is zero, then set the ct_zero flag.
		*/
		if ((*pc_sr) -> checktype == 0)
		    (*pc_sr) -> flag.ct_zero = TRUE;
		if (act_event -> xr > (*pc_sr) -> duration)
		    (*pc_sr) -> checktype = act_event -> attr_no;
		(*pc_sr) -> ol_dur =
		    Min ((*pc_sr) -> duration, act_event -> xr);
		if ((*pc_sr) -> ol_dur < (*p_next_stop_pos))
		    (*p_next_stop_pos) = (*pc_sr) -> ol_dur;
	    }
	}

	(*pc_sr) -> duration =
	    Max ((*pc_sr) -> duration, act_event -> xr);
	set_group (*pc_sr);
    }

    if (new_field1) *pc_sr = new_field1;
    else if (new_field2) *pc_sr = new_field2;
}
