/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Patrick Groeneveld
 *	Simon de Graaf
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

#include <signal.h>
#include "src/ocean/seadali/header.h"

extern struct Disp_wdw *c_wdw;
extern struct Disp_wdw *p_wdw;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern int *dom_arr;
extern int *drawing_order;
extern int *eIds;
extern int *pict_arr;
extern int *vis_arr;
extern int  nr_planes;
extern int  NR_lay;
extern int  NR_all;
extern int  NR_dom;
extern int  DRC_nr;
extern int  Gridnr;
extern int  Textnr;

extern short CLIP_FLAG;
/*
** The CLIP_FLAG signals whether clipping has
** to be performed or not.
** Default value: TRUE.  FALSE when complete.
** PICT-viewport has to be redrawn. TRUE in other cases
** to save contact holes outside drawing window.
*/

int Draw_dominant = FALSE;
int interrupt_flag = FALSE;
int maxDisplayStrLen = 0;

static void pict_all_whites (int mode);

void interrupt_display (int sig)
{
 /* Re-install signal handler. Not absolutely crucial, as flag can only be set once. */
    if (signal (SIGINT, SIG_IGN) == SIG_ERR) fprintf (stderr, "CANNOT re-set signal handler\n");
    /* signal (SIGINT, interrupt_display); */
    interrupt_flag = TRUE;
}

void picture ()
{
    register int i, j;
    Coor tewl, tewr, tewb, tewt;/* Text Erase Window */
    Coor tdwl, tdwr, tdwb, tdwt;/* Text Draw Window */
    float   ch_w, ch_h;
    int     dflag = 0;

    set_c_wdw (PICT);

    if ((float) piwl < XR && (float) piwr > XL &&
	(float) piwb < YT && (float) piwt > YB) {
     /*
     ** A part of the PICT-window has to be redrawn.
     */
	piwl = Max (piwl, (Coor) LowerRound (XL));
	piwr = Min (piwr, (Coor) UpperRound (XR));
	piwb = Max (piwb, (Coor) LowerRound (YB));
	piwt = Min (piwt, (Coor) UpperRound (YT));

	ch_siz (&ch_w, &ch_h);

	if (pict_arr[Textnr] == ERAS_DR) {
	    /* Erase all strings originating from the piw? area.
	    ** Strings are allowed to extend 0.5 * ch_h below, ch_h above,
	    ** and maxDisplayStrLen * ch_w to the right of their origin.
	    */
	    tewl = piwl;
	    tewr = Min (piwr + (Coor) UpperRound (maxDisplayStrLen * ch_w), (Coor) UpperRound (XR));
	    tewb = Max (piwb - (Coor) UpperRound (0.5 * ch_h), (Coor) LowerRound (YB));
	    tewt = Min (piwt + (Coor) UpperRound (ch_h), (Coor) UpperRound (YT));
	    if (tewr > piwr) ++dflag;
	    if (tewb < piwb) ++dflag;
	    if (tewt > piwt) ++dflag;
	}
	else {
	    /* TEXT does not have to be erased because of
	    ** changes in text itself. (At most because of
	    ** an erase of other elements.) --> Erase area does NOT
	    ** have to be extended to the right, above and below.
	    */
	    tewl = piwl;
	    tewr = piwr;
	    tewb = piwb;
	    tewt = piwt;
	}

	/* Redraw of TEXT, including strings that may
	** have been destroyed in piw? / tew? area.
	*/
	tdwl = Max (tewl - (Coor) UpperRound (maxDisplayStrLen * ch_w), (Coor) LowerRound (XL));
	tdwr = tewr;
	tdwb = Max (tewb - (Coor) UpperRound (ch_h), (Coor) LowerRound (YB));
	tdwt = Min (tewt + (Coor) UpperRound (0.5 * ch_h), (Coor) UpperRound (YT));

	/*
	** From here the actual drawing will start.
	** Install signal handler to permit the display operation to be interrupted.
	*/
	if (signal (SIGINT, interrupt_display) == SIG_ERR) fprintf (stderr, "Shit: Cannot set drawing interrupt!\n");

        set_alarm (TRUE); /* switch on timer alarm */

	if ((i = ggSetErasedAffected (eIds, pict_arr, NR_all))) {
	/*
	 ** Something has to be erased.
	 */
	    if (i == NR_all) {
		ggClearArea ((float) piwl, (float) piwr, (float) piwb, (float) piwt);
		if (dflag) {	/* different area for TEXT */
		    ggSetColor (Textnr);
		    ggEraseArea ((float) tewl, (float) tewr, (float) tewb, (float) tewt);
		    /*
		     * Text-erase may have destructed other information too.
		     * Restore could be made conditional if we would
		     * explicitly check whether text affects other info.
		     * We could distinguish between (1) piwl area
		     * and (2) tewl area outside piwl area
		     * and handle these differently, since in
		     * second case only text is erased and less
		     * or no mask info may be affected.
		     */
		    piwl = tewl;
		    piwr = tewr;
		    piwb = tewb;
		    piwt = tewt;
		}
		goto start_drawing;
	    }
	    else {
		ggSetColors (eIds, i);
		ggEraseArea ((float) piwl, (float) piwr, (float) piwb, (float) piwt);
		if (dflag) {	/* different area for TEXT */
		    ggSetColor (Textnr);
		    ggEraseArea ((float) tewl, (float) tewr, (float) tewb, (float) tewt);
		    piwl = tewl;
		    piwr = tewr;
		    piwb = tewb;
		    piwt = tewt;
		}
	    }
	}

     /*
     ** The sequence of drawing is as follows:
     ** (1) the transparent (non-background) layers,
     ** (2) the dominant (background) layers,
     ** (3) the 'whites' (text, b-boxes, grid, drc_errors).
     **
     ** The non-dominant layers may overwrite the
     ** dominant layers. If so, also draw the dominant
     ** layers. The dominant layers, on their turn, may
     ** overwrite the whites. If so, also draw the whites.
     */
	for (j = i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;
	    if (dom_arr[i] && (Draw_dominant || dom_arr[i] != 2)) { j = 1; continue; }
	    /*
	    ** Some non-dominant mask will be drawn:
	    **      redraw dominant masks and whites.
	    */
	    for (i = 0; i < NR_lay; ++i)
		if (dom_arr[i] && (Draw_dominant || dom_arr[i] != 2)) pict_arr[i] = DRAW;
	    pict_all_whites (DRAW);
	    goto start_drawing;
	}

	if (j) {
	    if (Draw_dominant) {
		j = NR_dom;
		while (--j >= 0) if (pict_arr[drawing_order[j]] != SKIP) break;
		while (--j >= 0) pict_arr[drawing_order[j]] = DRAW;
	    }
	    /* Some dominant mask will be drawn: redraw whites.
	    */
	    pict_all_whites (DRAW);
	    goto start_drawing2;
	}

	goto start_whites;

start_drawing:
	/*
	** (1) Start drawing the non-dominant layers.
	*/
	disp_mode (TRANSPARENT);

	/*
	 * draw non-dominant layers
	 */
	for (i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;
	    if (dom_arr[i] && (Draw_dominant || dom_arr[i] != 2)) continue; /* draw dominant later on */

	    if (stop_drawing () == TRUE) goto interrupt;
	    disp_mask_quad (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    dis_term (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    draw_inst_window (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    dis_s_term (i, piwl, piwr, piwb, piwt);

	    pict_arr[i] = SKIP; // done
	}

start_drawing2:
	/*
	** (2) Start drawing the dominant layers.
	*/
	disp_mode (DOMINANT);

	/*
	 * draw dominant layers in order
	 */
	if (Draw_dominant)
	for (j = NR_dom; --j >= 0;) {
	    i = drawing_order[j];
	    if (pict_arr[i] == SKIP) continue;

	    if (stop_drawing () == TRUE) goto interrupt;
	    disp_mask_quad (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    dis_term (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    draw_inst_window (i, piwl, piwr, piwb, piwt);

	    if (stop_drawing () == TRUE) goto interrupt;
	    dis_s_term (i, piwl, piwr, piwb, piwt);

	    pict_arr[i] = SKIP; // done
	}

	/*
	 * draw dominant black layers
	 */
	for (i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;

	    if (stop_drawing () == TRUE) goto interrupt;
	    disp_mask_quad (i, piwl, piwr, piwb, piwt);

	    /* if (stop_drawing () == TRUE) goto interrupt;
	    dis_term (i, piwl, piwr, piwb, piwt); */

	    if (stop_drawing () == TRUE) goto interrupt;
	    draw_inst_window (i, piwl, piwr, piwb, piwt);

	    /* if (stop_drawing () == TRUE) goto interrupt;
	    dis_s_term (i, piwl, piwr, piwb, piwt); */
	}

start_whites:
	/*
	** (3) Start drawing the 'whites'
	**     (text, b-boxes, grid, drc_errors).
	*/
	disp_mode (TRANSPARENT);
	d_fillst (FILL_SOLID);

	if (pict_arr[Textnr] != SKIP) {
	    if (stop_drawing () == TRUE) goto interrupt;
	    ggSetColor (Textnr);
	    mc_char (tdwl, tdwr, tdwb, tdwt);
	    if (stop_drawing () == TRUE) goto interrupt;

	    term_char (tdwl, tdwr, tdwb, tdwt);
	    if (stop_drawing () == TRUE) goto interrupt;

	    /* draw comment text and lines */
	    if (vis_arr[NR_lay + 7]) draw_all_comments ();

	    /* draw bounding boxes of instances */
	    pict_mc (tewl, tewr, tewb, tewt);
	    if (stop_drawing () == TRUE) goto interrupt;

	    /* draw bounding boxes at deepest level */
	    draw_inst_window (-1, tewl, tewr, tewb, tewt);
	}

	if (pict_arr[Gridnr] != SKIP) {
	    if (stop_drawing () == TRUE) goto interrupt;
	    disp_axis ();
	    display_grids (tewl, tewr, tewb, tewt);
	}

	if (pict_arr[DRC_nr] != SKIP) {
	    if (stop_drawing () == TRUE) goto interrupt;
	    draw_drc_err (piwl, piwr, piwb, piwt);
	}

interrupt:
	/* restore previous signal mode */

	set_alarm (FALSE); /* switch alarm off */
	if (interrupt_flag == TRUE) {
	   interrupt_flag = FALSE;
	   print_reason ();
	}

	ggSetColor (Gridnr);
	d_fillst (FILL_SOLID);
	disp_mode (TRANSPARENT);
	flush_pict ();
    }
    pict_all (SKIP);
    pic_max ();
    CLIP_FLAG = TRUE;
}

void pict_all (int mode)
{
    register int i;
    for (i = 0; i < NR_all; ++i) pict_arr[i] = mode;
}

static void pict_all_whites (int mode)
{
    pict_arr[Textnr] = mode;
    pict_arr[Gridnr] = mode;
    pict_arr[DRC_nr] = mode;
}

void pic_max ()
{
    piwl = (Coor) LowerRound (p_wdw -> wxmin);
    piwr = (Coor) UpperRound (p_wdw -> wxmax);
    piwb = (Coor) LowerRound (p_wdw -> wymin);
    piwt = (Coor) UpperRound (p_wdw -> wymax);
}

#define ALARM_INTERVAL 1

/*
 * this routine is called by the alarm handler
 * every ALARM_INTERVAL secs during drawing
 * it just sets interrupt_flag to 2, which causes
 * stop drwaing the interrupt in the right time.
 * this was necessary because some x-routines
 * didn't like to be interrupted.
 */
void drawing_interrupt_handler ()
{
    interrupt_flag = 2;
}

/*
 * dummy, in case the time is stil running
 */
void dummy_interrupt_handler () {}

/*
 * this is called by the drawing routines if interrupt_flag >= TRUE
 * it returns TRUE if the drawing is to be interrupted
 */
int stop_drawing ()
{
    if (interrupt_flag == 0) return (FALSE);

    if (interrupt_flag == 2) { /* interrupt is pending for processing */
    /*
     * find out if there was an event in the mean time....
     */
	if (event_exists () == TRUE) { /* stop drawing */
	    interrupt_flag = TRUE; /* stops drawing */
	    set_alarm (FALSE);
	}
	else { /* continue drawing */
	    /* switch it on again.... */
	    set_alarm (TRUE);
	    interrupt_flag = FALSE; /* continue drawing, nothing the matter */
	    return (FALSE);
	}
    }
    return (TRUE);
}

/*
 * switches on or off alarm timer, depending on switchon
 */
void set_alarm (int switchon)
{
    if (switchon == TRUE) {
	if (signal (SIGALRM, drawing_interrupt_handler) == SIG_ERR) fprintf (stderr, "Cannot set SIGALRM\n");
	/* set alarm for ALARM_INTERVAL seconds */
	alarm ((unsigned long) ALARM_INTERVAL);
	interrupt_flag = FALSE;
    }
    else { /* switch off by setting to dummy alarm handler */
	if (signal (SIGALRM, dummy_interrupt_handler) == SIG_ERR) fprintf (stderr, "Cannot set dummy SIGALRM\n");
    }
}
