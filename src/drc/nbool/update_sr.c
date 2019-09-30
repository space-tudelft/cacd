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

#include "src/drc/nbool/nbool.h"

void update_sr ()
{
/* This function updates the stateruler after it has
 * been analysed for a certain x_value.
 * first the ov_mask is cleared, mask_past is set to mask_fut
 * and the check_types are updated in the case of a stop_edge.
 */
    int     chk_type;		/* check_type after update */
    int     i;			/* loop variable	 */
    int     different;		/* same/not same chk_type */
    unsigned int    stop_mask;	/* masks that stopped.	 */
    struct sr_field *c_sr;	/* current sr_pointer	 */
    struct sr_field *rm_pntr;	/* ptr to field to remove */
    struct check   *p_chk;	/* ptr to chktype_struct */
    struct checktype *p_chg_ct; /* ptr to old chk_types */

    c_sr = h_sr -> next;
    while (c_sr != h_sr) {
	stop_mask = c_sr -> mask_past & ~c_sr -> mask_fut;
	c_sr -> ov_mask = 0;
	c_sr -> mask_past = c_sr -> mask_fut;
	if (stop_mask != 0) {
	    if (c_sr -> mask_fut == 0) {
		c_sr -> chk_type = INITIAL;
		if (c_sr -> p_chk != NULL) {
		    FREE (c_sr -> p_chk);
		    c_sr -> p_chk = NULL;
		}
	    }
	    else
		if (c_sr -> chk_type == DIFF_CT) {
		    p_chk = c_sr -> p_chk;
		    different = 0;
		    chk_type = INITIAL;
		    for (i = 1; i < nomasks; i++) {
			if ((stop_mask & edges[i].mask) != 0)
			    p_chk -> chk_arr[i] = INITIAL;
			if (p_chk -> chk_arr[i] != INITIAL) {
			    if (chk_type == INITIAL)
				chk_type = p_chk -> chk_arr[i];
			    else
				if (chk_type != p_chk -> chk_arr[i])
				    different = 1;
			}
		    }
		    if (different == 0) {
			c_sr -> chk_type = chk_type;
			FREE (c_sr -> p_chk);
			c_sr -> p_chk = NULL;
		    }
		}
	}

/* Now the old check_types are deleted.			 */

	p_chg_ct = c_sr -> p_chg_ct;
	while (p_chg_ct != NULL) {
	    FREE (p_chg_ct);
	    p_chg_ct = p_chg_ct -> next;
	}
	c_sr -> p_chg_ct = NULL;

/* after that it is determined if there are adjecent	 */
/* fields in the state_ruler containing the same layers	 */
/* and having the same check_types. If this is so the	 */
/* two fields are combined to one and the other field	 */
/* is removed from the state_ruler.			 */

	if (c_sr -> mask_fut == c_sr -> prev -> mask_fut) {
	    if (c_sr -> chk_type != DIFF_CT) {
		if (c_sr -> chk_type == c_sr -> prev -> chk_type) {
		    c_sr -> prev -> yt = c_sr -> yt;
		    rm_pntr = c_sr;
		    c_sr -> prev -> next = c_sr -> next;
		    c_sr -> next -> prev = c_sr -> prev;
		    c_sr = c_sr -> prev;
		    FREE (rm_pntr);
		}
	    }
	    else
		if (c_sr -> prev -> chk_type == DIFF_CT) {
		    chk_type = INITIAL;
		    different = 0;
		    for (i = 0; i < MAX_NOMASKS; i++) {
			if ((chk_type == INITIAL) &&
				(c_sr -> p_chk -> chk_arr[1] != INITIAL))
			    chk_type = c_sr -> p_chk -> chk_arr[i];
			if (c_sr -> p_chk -> chk_arr[i] !=
				c_sr -> prev -> p_chk -> chk_arr[i]) {
			    different = 1;
			    break;
			}
		    }
		    if (different == 0) {
			FREE (c_sr -> p_chk);
			c_sr -> prev -> yt = c_sr -> yt;
			rm_pntr = c_sr;
			c_sr -> prev -> next = c_sr -> next;
			c_sr -> next -> prev = c_sr -> prev;
			c_sr = c_sr -> prev;
			c_sr -> chk_type = chk_type;
			FREE (rm_pntr);
		    }
		}
	}
	c_sr = c_sr -> next;
    }
}
