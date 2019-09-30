/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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

#include "src/drc/dubcheck/dubcheck.h"

extern char maskfile1[];
extern char maskfile2[];

static int evcmp (void);

/* This procedure carries out the gap and overlap checks.*/
/* First the stateruler is given its initial value.	 */
/* The events are then read one by one from the vln_files*/
/* specified by the model and layer given by main. 	 */
/* The events of the two files are sorted by evcmp	 */
/* and inserted into the stateruler by the procedure 	 */
/* insert_event. If for a value of x all events		 */
/* have been installed in the stateruler, the latter is  */
/* analysed for errors (extr_profile and extr_overlap)	 */
/* and after that updated (update_sr).			 */
/* It then again is formed for the next x_value etc.	 */
/* until all events have been read from the vln_files.	 */

void main_check ()
{
    struct sr_field *c_sr;	/* pntr to SR_field	 */
    int     x_old;		/* SR position		 */
    int     ret[3];		/* return_val of get_vln */
    int	    event_index;	/* nbr in event array	 */

    x_old = -MAXINT;
    h_sr = &head_sr;
    h_sr -> xstart[0] = -MAXINT;
    h_sr -> xstart[1] = -MAXINT;
    h_sr -> yb = -MAXINT;
    h_sr -> yt = MAXINT;
    h_sr -> helplay_status = NOT_PRESENT;
    h_sr -> helplay_xstart = -MAXINT;
    h_sr -> next = &head_sr;
    h_sr -> prev = &head_sr;
    c_sr = h_sr;

    ret[0] = get_vln (pvln[0], FIRST);
    ret[1] = get_vln (pvln[1], SECOND);
    if(pvln[2] != NULL)
	ret[2] = get_vln(pvln[2],THIRD);
    else {
	ret[2] = 0;
	event[2].e_xi = MAXINT;
    }

    if (ret[0] == 0 || ret[1] == 0)
	return;

    while (ret[0] != 0 || ret[1] != 0 || ret[2] != 0) {
	event_index = evcmp();
	if (event[event_index].e_xi != x_old) {
	    if (gapflag == 1)
		extr_profile (x_old);
	    if ((overlapflag == 1) && (kind == 0))
		extr_overlap (x_old);
	    if ((overlapflag == 1) && (kind == 1))
		extr_overlap1 (x_old);
	    if ((overlapflag == 1) && (kind == 2))
		extr_overlap2 (x_old);
	    if ((overlapflag == 1) && (kind == 3))
		extr_overlap3 (x_old);
	    if (kind == 4)
		det_conn_hor(x_old);
	    if (kind == 5)
		det_conn_ver(x_old);
	    if ((overlapflag == 1) && (kind == 6))
		extr_overlap6 (x_old);
	    update_sr (x_old, event[event_index].e_xi);
	    x_old = event[event_index].e_xi;
	    c_sr = h_sr;
	}
        insert_edge (&c_sr, event[event_index].e_yb,
		     event[event_index].e_yt,
		     event[event_index].e_occ,
		     event[event_index].e_group,
		     event[event_index].e_ctype, event_index);
	if (ret[event_index] != 0) {
	    ret[event_index] = get_vln (pvln[event_index], event_index);
	    if (ret[event_index] == 0)
		event[event_index].e_xi = MAXINT;
	}
    }

    if (gapflag == 1)
	extr_profile (x_old);

    if (overlapflag == 1 && kind == 0)
	extr_overlap (x_old);

    if (overlapflag == 1 && kind == 1)
	extr_overlap1 (x_old);

    if (overlapflag == 1 && kind == 2)
	extr_overlap2 (x_old);

    if (overlapflag == 1 && kind == 3)
	extr_overlap3 (x_old);

    if ((overlapflag == 1) && (kind == 6))
	extr_overlap6 (x_old);
}

static int evcmp ()
{
    if(event[0].e_xi < event[1].e_xi) {
	if(event[2].e_xi < event[0].e_xi)
	    return(2);
	if(event[2].e_xi > event[0].e_xi)
	    return(0);
	if(event[2].e_yb < event[0].e_yb)
	    return(2);
	else
	    return(0);
    }
    if(event[0].e_xi > event[1].e_xi) {
	if(event[2].e_xi < event[1].e_xi)
	    return(2);
	if(event[2].e_xi > event[1].e_xi)
	    return(1);
	if(event[2].e_yb < event[1].e_yb)
	    return(2);
	else
	    return(1);
    }
    if(event[0].e_yb < event[1].e_yb) {
	if(event[2].e_xi < event[0].e_xi)
	    return(2);
	if(event[2].e_xi > event[0].e_xi)
	    return(0);
	if(event[2].e_yb < event[0].e_yb)
	    return(2);
	else
	    return(0);
    }
    else {
	if(event[2].e_xi < event[1].e_xi)
	    return(2);
	if(event[2].e_xi > event[1].e_xi)
	    return(1);
	if(event[2].e_yb < event[1].e_yb)
	    return(2);
	else
	    return(1);
    }
}
