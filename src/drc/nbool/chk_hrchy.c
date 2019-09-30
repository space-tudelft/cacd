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

#include "src/drc/nbool/nbool.h"

static void check_bottop (struct sr_field *sr_b, struct sr_field *sr_t, unsigned int mask, int sr_pos);

void check_hierarchy (int sr_pos)
/* sr_pos - current x_value of the sr */
{
/* this function checks the stateruler for hierarchical	 */
/* errors such as overlap without terminal and		 */
/* overlapping layers with different check_types,	 */
/* indicating that they belong to different models.	 */

    int     i;			/* loop variable	 */
    int     chk_type;		/* temp check_type	 */
    unsigned int    chk_msk_botop;/* check_type mask	 */
    struct sr_field *c_sr;	/* current sr_field ptr	 */

    c_sr = h_sr -> next;
    while (c_sr != h_sr -> prev) {

/* first it is determined if in the case of an overlap	 */
/* in a layer, indicated by a bit in the ov_mask, a	 */
/* terminal in the same layer is present.		 */
/* If not so an error_message is generated in the case	 */
/* of a connection mask and a warning in the other cases.*/

	if (c_sr -> ov_mask != 0) {
	    for (i = 0; i < nomasks; i++) {
		if ((c_sr -> ov_mask & edges[i].mask) != 0) {
		    if (edges[i].mask_type == CONN_MASK) {
			if (((c_sr -> mask_past & edges[i].mask_term) == 0) &&
				((c_sr -> mask_fut & edges[i].mask_term) == 0)) {
			    fprintf (pout, "\nERROR:   overlap of %s without terminal",
				    edges[i].mask_name);
			    fprintf (pout, "\n\tat x = %d, from y = %d to %d\n",
				    sr_pos, c_sr -> yb, c_sr -> yt);
			}
			continue;
		    }
		    else {
			fprintf (pout, "\nWARNING: overlap of %s",
				edges[i].mask_name);
			fprintf (pout, "\n\tat x = %d, from y = %d to %d\n",
				sr_pos, c_sr -> yb, c_sr -> yt);
		    }
		}
	    }
	}

/* than the layers present in the stateruler_field are	 */
/* checked to see if they all have the same check_type,	 */
/* indicating they are from the same (sub)model.	 */
/* If not a warning is generated and the layers with	 */
/* their check_types are printed.			 */

	if ((c_sr -> mask_past != c_sr -> mask_fut) &&
		(c_sr -> chk_type == DIFF_CT)) {
	    chk_type = INITIAL;
	    for (i = 0; i < nomasks; i++) {
		if ((c_sr -> p_chk -> chk_arr[i] == INITIAL) ||
			((edges[i].mask_type == CONN_MASK) &&
			    ((c_sr -> mask_fut | c_sr -> mask_past) &
				edges[i].mask_term) != 0))
		    continue;
		if (chk_type == INITIAL)
		    chk_type = c_sr -> p_chk -> chk_arr[i];
		else
		    if (chk_type != c_sr -> p_chk -> chk_arr[i])
			break;
	    }
	    if (i < nomasks) {
		fprintf (pout, "\nWARNING: area with different check_types");
		fprintf (pout, "\n         you may have created an unwanted element.");
		fprintf (pout, "\n\tat x = %d from y = %d to %d\n\tcheck_types:",
			sr_pos, c_sr -> yb, c_sr -> yt);
		for (i = 0; i < nomasks; i++) {
		    if (c_sr -> p_chk -> chk_arr[i] != INITIAL)
			fprintf (pout, "\n\t%s\t%d", edges[i].mask_name,
				c_sr -> p_chk -> chk_arr[i]);
		}
		fprintf (pout, "\n");
	    }
	}
/* at last a check is done to see if layers do not	 */
/* change of check_type  in the y_direction without the	 */
/* presence of a terminal in that layer.		 */

	if ((chk_msk_botop = (~c_sr -> ov_mask &
			c_sr -> mask_fut & ~c_sr -> mask_past)) != 0) {
	    if ((c_sr -> prev -> chk_type != INITIAL) &&
		    ((c_sr -> chk_type == DIFF_CT) ||
			(c_sr -> chk_type != c_sr -> prev -> chk_type)))
		check_bottop (c_sr -> prev, c_sr, chk_msk_botop, sr_pos);
	    if ((c_sr -> next -> chk_type != INITIAL) &&
		    ((c_sr -> chk_type == DIFF_CT) ||
			(c_sr -> chk_type != c_sr -> next -> chk_type)))
		check_bottop (c_sr, c_sr -> next, chk_msk_botop, sr_pos);
	}
	c_sr = c_sr -> next;
    }
}

static void check_bottop (struct sr_field *sr_b, struct sr_field *sr_t, unsigned int mask, int sr_pos)
/* sr_b   - ptr to upper sr_field */
/* sr_t   - ptr to lower sr_field */
/* mask   - layers to check */
/* sr_pos - state_ruler position */
{
/* This procedure checks if the layers specified in the
 * varible mask have different checktypes in two
 * adjecent state_ruler fields. If this is the case and
 * the layer is a connection_layer and no corresponding
 * terminal is present an ERROR is generated stating the
 * layer and place it occurred. If the layer is not a
 * connection_layer and not a terminal layer a WARNING
 * is generated stating the layer and place it occurred.
 */
    int     i;			/* loop variable		 */
    int     chk_type1;		/* chk_type of layer in bot.field */
    int     chk_type2;		/* chk_type of layer in top field */

    for (i = 0; i < nomasks; i++) {
	if ((edges[i].mask & mask) != 0) {
	    if (((sr_b -> mask_fut & edges[i].mask) == 0) ||
		    ((sr_t -> mask_fut & edges[i].mask) == 0))
		continue;
	    if (sr_b -> chk_type != DIFF_CT)
		chk_type1 = sr_b -> chk_type;
	    else
		chk_type1 = sr_b -> p_chk -> chk_arr[i];
	    if (sr_t -> chk_type != DIFF_CT)
		chk_type2 = sr_t -> chk_type;
	    else
		chk_type2 = sr_t -> p_chk -> chk_arr[i];
	    if ((chk_type1 != chk_type2) &&
		    (chk_type1 != INITIAL) && (chk_type2 != INITIAL)) {
		if (edges[i].mask_type == CONN_MASK) {
		    if (((sr_b -> mask_past | sr_b -> mask_fut |
				    sr_t -> mask_past | sr_t -> mask_fut) &
				edges[i].mask_term) == 0) {
			fprintf (pout,"\nERROR:connection in %s without terminal",
				edges[i].mask_name);
			fprintf (pout, "\nfrom x = %d, y = %d to the right\n",
				sr_pos, sr_b -> yt);
		    }
		}
		else
		    if (edges[i].mask_type != TERM_MASK) {
			fprintf (pout,"\nWARNING:connection of %s from different models",
				edges[i].mask_name);
			fprintf (pout, "\nfrom x = %d, y = %d to the right\n",
				sr_pos, sr_b -> yt);
		    }
	    }
	}
    }
}
