/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
 *	J. Liedorp
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

#include "src/drc/dimcheck/dimcheck.h"

static int evcmp (void);
static int get_vln (DM_STREAM *fp, int nbr_file);

/* This procedure carries out the gap and width checks.	 */
/* First the stateruler is given its initial value.	 */
/* The events are then read one by one from the vln_file */
/* specified by the model and layer given by main. 	 */
/* The events are inserted into the stateruler by the	 */
/* procedure insert_event. If for a value of x all events*/
/* have been installed in the stateruler, the latter is  */
/* analysed for errors (extr_profile) and after that	 */
/* updated (update_sr). It then again is formed for the  */
/* next x_value etc. until all events have been read	 */
/* from the vln_file.					 */

void main_check ()
{
    struct sr_field *c_sr;
    int     x_old = -MAXINT;
    int     ret[2];
    int     event_index;

    h_sr = &head_sr;
    h_sr -> xstart = -MAXINT;
    h_sr -> yb = -MAXINT;
    h_sr -> yt = MAXINT;
    h_sr -> lay_status = NOT_PRESENT;
    h_sr -> helplay_status = NOT_PRESENT;
    h_sr -> next = &head_sr;
    h_sr -> prev = &head_sr;
    c_sr = h_sr;

    ret[0] = get_vln (pvln[0], 0);
    if (pvln[1] != NULL)
	ret[1] = get_vln (pvln[1], 1);
    else {
	ret[1] = 0;
	event[1].e_xi = MAXINT;
    }
    while ((ret[0] != 0) || (ret[1] != 0)) {
	event_index = evcmp ();
	if (event[event_index].e_xi != x_old) {
	    extr_profile (x_old);
	    update_sr (x_old, event[event_index].e_xi);
	    x_old = event[event_index].e_xi;
	    c_sr = h_sr;
	}

	insert_edge (&c_sr, event[event_index].e_yb, event[event_index].e_yt,
		     event[event_index].e_occ, event[event_index].e_group,
		     event[event_index].e_ctype, event_index);
	if (ret[event_index] != 0)
	    ret[event_index] = get_vln (pvln[event_index], event_index);
	if (ret[event_index] == 0)
	    event[event_index].e_xi = MAXINT;
    }

    extr_profile (x_old);
}

static int get_vln (DM_STREAM *fp, int nbr_file)
{
/* This procedure reads a line from a vln_file and
 * places its contents in the structure event.
 * On the end of file EOF is returned.
 */
    int edge;

    do {
	if (dmGetDesignData (fp, GEO_VLNLAY) <= 0) return (0);
	edge = gvlnlay.occ_type & 7;
    }
    while ((edge == 0) || (edge == 3));

    event[nbr_file].e_xi    = gvlnlay.x;
    event[nbr_file].e_yb    = gvlnlay.yb;
    event[nbr_file].e_yt    = gvlnlay.yt;
    event[nbr_file].e_occ   = edge;
    event[nbr_file].e_group = gvlnlay.grp_number;
    event[nbr_file].e_ctype = gvlnlay.chk_type;
    return (1);
}

static int evcmp ()
{
    if (event[0].e_xi < event[1].e_xi) return (0);
    if (event[0].e_xi > event[1].e_xi) return (1);
    if (event[0].e_yb > event[1].e_yb) return (1);
    return (0);
}
