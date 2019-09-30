/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
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

#include <signal.h>
#include <setjmp.h>
#include "src/dali/header.h"

extern struct Disp_wdw *p_wdw;
extern float XL, XR, YB, YT;
extern float c_sX, c_sY; /* current window scaling */
extern float c_cW, c_cH; /* current char. width/height */
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern int *black_arr;
extern int *dom_arr;
extern int *drawing_order;
extern int *ggeIds;
extern int *pict_arr;
extern int  NR_all;
extern int  NR_dom;
extern int  NR_lay;
extern int  CLIP_FLAG;
extern int  DRC_nr;
extern int  Draw_dominant;
extern int  Erase_hollow;
extern int  Gridnr;
extern int  Textnr;

int erase_text = 0;
int maxDisplayStrLen = 0;

static jmp_buf env;

static void pict_all_blacks (int mode);
static void pict_all_dominants (int mode);
static void pict_all_whites (int mode);

void interrupt_display ()
{
    longjmp (env, 1);
}

void Rpicture (int x, int y, int w, int h) /* redraw picture */
{
    float xmin, ymin, xmax, ymax;
    pict_all (DRAW);
    set_c_wdw (PICT);
    xmin = XL   + (float) x / c_sX;
    xmax = xmin + (float) w / c_sX;
    ymax = YT   - (float) y / c_sY;
    ymin = ymax - (float) h / c_sY;
    piwl = (Coor) LowerRound (xmin);
    piwr = (Coor) UpperRound (xmax);
    piwb = (Coor) LowerRound (ymin);
    piwt = (Coor) UpperRound (ymax);
    picture ();
}

void picture ()
{
    Coor tewl, tewr, tewb, tewt; /* Text Erase Window */
    Coor tdwl, tdwr, tdwb, tdwt; /* Text Draw Window */
    Coor loXL, loYB, upXR, upYT;
    float   strW;
    register int i, j;
    void    (*istat) ();
    int     dflag = 0; /* no different area for text */
    int     interrupt_flag = 0;

    set_c_wdw (PICT);

    loXL = (Coor) LowerRound (XL);
    loYB = (Coor) LowerRound (YB);
    upXR = (Coor) UpperRound (XR);
    upYT = (Coor) UpperRound (YT);

    CLIP_FLAG = 0; /* do not clip trapezoids while painting */
    if (piwl <= loXL) piwl = loXL;
    else CLIP_FLAG  = 8; /* set clipping mode ON */
    if (piwb <= loYB) piwb = loYB;
    else CLIP_FLAG |= 4; /* set clipping mode ON */
    if (piwr >= upXR) piwr = upXR;
    else CLIP_FLAG |= 2; /* set clipping mode ON */
    if (piwt >= upYT) piwt = upYT;
    else CLIP_FLAG |= 1; /* set clipping mode ON */

    if (piwl < piwr && piwb < piwt) {
    /*
    ** A part of the PICT-window has to be redrawn.
    */
	strW = c_cW * maxDisplayStrLen;

	if (strW && pict_arr[Textnr] == ERAS_DR && (CLIP_FLAG & 7)) {
	/*
	** Set a different area for text:
	**   IF  (1) there is text on the picture
	**   AND (2) the text must be erased
	**   AND (3) a different area for text can be set
	** Erase all strings originating from the piw? area.
	** Strings are allowed to extend 0.5 * c_cH below,
	** c_cH above, and strW to the right of their origin.
	*/
	    dflag = 1; /* different area for text */
	    tewl = piwl;
	    tewb = piwb - (Coor) UpperRound (0.5 * c_cH);
	    tewr = piwr + (Coor) UpperRound (strW);
	    tewt = piwt + (Coor) UpperRound (c_cH);
	    if (tewb < loYB) tewb = loYB;
	    if (tewr > upXR) tewr = upXR;
	    if (tewt > upYT) tewt = upYT;
	}
	else {
	/*
	** No different area for text need to be set!
	**   IF (1) there is no text on the picture
	**   OR (2) the text does not have to be erased
	**   OR (3) the text area can not be extended!
	*/
	    tewl = piwl;
	    tewb = piwb;
	    tewr = piwr;
	    tewt = piwt;
	}

	/*
	** Redraw of TEXT, including strings that may
	** have been destroyed in piw? / tew? area.
	*/
	if (CLIP_FLAG) {
	    tdwl = tewl - (Coor) UpperRound (strW);
	    tdwb = tewb - (Coor) UpperRound (c_cH);
	    tdwr = tewr;
	    tdwt = tewt + (Coor) UpperRound (0.5 * c_cH);
	    if (tdwl < loXL) tdwl = loXL;
	    if (tdwb < loYB) tdwb = loYB;
	    if (tdwt > upYT) tdwt = upYT;
	}
	else { /* pict is max. window */
	    tdwl = piwl;
	    tdwb = piwb;
	    tdwr = piwr;
	    tdwt = piwt;
	}

	if ((i = ggSetErasedAffected (ggeIds, pict_arr, NR_all))) {
	/*
	** Something has to be erased.
	*/
	    if (i == NR_all) {
		ggEraseArea ((float) piwl, (float) piwr, (float) piwb, (float) piwt, 1);
		if (dflag) {	/* different area for TEXT */
		    ggSetColor (Textnr);
		    ggEraseArea ((float) tewl, (float) tewr, (float) tewb, (float) tewt, 0);
		    /*
		     * Text-erase may have destructed other
		     * information too.  Restore.
		     * Could be made conditional if we would
		     * explicitly check whether text affects
		     * other info.
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
		ggSetColors (ggeIds, i);
		ggEraseArea ((float) piwl, (float) piwr, (float) piwb, (float) piwt, 0);
		if (dflag) {	/* different area for TEXT */
		    ggSetColor (Textnr);
		    ggEraseArea ((float) tewl, (float) tewr, (float) tewb, (float) tewt, 0);
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
	for (i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;
	    if (black_arr[i] || (Draw_dominant && dom_arr[i])) continue;
	    /*
	     ** Some non-dominant mask will be drawn:
	     **      redraw dominant masks and whites.
	     */
	    if (Draw_dominant)
		pict_all_dominants (DRAW);
	    else
		pict_all_blacks (DRAW);
	    pict_all_whites (DRAW);
	    goto start_drawing;
	}

	for (i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;
	    if (Draw_dominant && dom_arr[i]) {
	    /*
	     ** Some dominant mask will be drawn:
	     ** redraw all higher dominants and whites.
	     */
		for (j = NR_dom - 1; j >= 0; --j) {
		    if (pict_arr[drawing_order[j]] != SKIP) break;
		}
		while (--j >= 0) pict_arr[drawing_order[j]] = DRAW;
		pict_all_whites (DRAW);
		goto start_drawing;
	    }
	    else if (black_arr[i]) {
	    /*
	     ** Some 'black' mask will be drawn: redraw whites.
	     */
		pict_all_whites (DRAW);
		goto start_drawing;
	    }
	}

	goto start_whites;

start_drawing:

	istat = signal (SIGINT, interrupt_display);

	/*
	** From here the actual drawing will start.
	** Install signal handler to permit the
	** display operation to be interrupted.
	*/
	if (setjmp (env)) {
	    interrupt_flag = 1;
	    ptext ("Interrupt!  Picture may not be complete yet!");
	    goto interrupt;
	}

	/*
	** (1) Start drawing the non-dominant layers.
	*/
	disp_mode (TRANSPARENT);

	for (i = 0; i < NR_lay; ++i) {
	    if (pict_arr[i] == SKIP) continue;
	    if (black_arr[i] || (Draw_dominant && dom_arr[i])) continue;
	    disp_mask_quad (i, piwl, piwr, piwb, piwt);
	    dis_term (i, piwl, piwr, piwb, piwt);
	    draw_inst_window (i, piwl, piwr, piwb, piwt);
	    dis_s_term (i, piwl, piwr, piwb, piwt);
	    dis_label (i, piwl, piwr, piwb, piwt);
	    pict_arr[i] = SKIP; /* done */
	}

	/*
	** (2) Start drawing the dominant layers.
	*/
	disp_mode (DOMINANT);

	/*
	** (2a) If draw dominant mode is set, draw the dominant layers
	**      in specified order (start with the lowest layer first).
	*/
	if (Draw_dominant) {
	    for (j = NR_dom - 1; j >= 0; --j) {
		i = drawing_order[j];
		if (pict_arr[i] == SKIP) continue;
		disp_mask_quad (i, piwl, piwr, piwb, piwt);
		dis_term (i, piwl, piwr, piwb, piwt);
		draw_inst_window (i, piwl, piwr, piwb, piwt);
		dis_s_term (i, piwl, piwr, piwb, piwt);
		dis_label (i, piwl, piwr, piwb, piwt);
	    }
	}
	else {
	/*
	** (2b) Draw default dominants (blacks)
	*/
	    for (i = 0; i < NR_lay; ++i) {
		if (pict_arr[i] == SKIP) continue;
		disp_mask_quad (i, piwl, piwr, piwb, piwt);
	    /** dis_term (i, piwl, piwr, piwb, piwt);   **/
		draw_inst_window (i, piwl, piwr, piwb, piwt);
	    /** dis_s_term (i, piwl, piwr, piwb, piwt); **/
	    }
	}

interrupt: /* restore previous signal mode */
	signal (SIGINT, istat);

start_whites:

	disp_mode (TRANSPARENT);
	d_fillst (FILL_SOLID);

	/*
	** (3) Start drawing the 'whites' (text, b-boxes, grid, drc_errors).
	*/
	if (pict_arr[Textnr] != SKIP) {
	    ggSetColor (Textnr);
	    if (!interrupt_flag) {
		/* if drawing complete window, reset text length */
		if (!CLIP_FLAG) maxDisplayStrLen = 0;
		mc_char (tdwl, tdwr, tdwb, tdwt);
		term_char (tdwl, tdwr, tdwb, tdwt);
		/* draw comment text and lines */
		draw_all_comments ();
	    }

	    /* draw bounding boxes of instances */
	    pict_mc (tewl, tewr, tewb, tewt);

	    /* draw bounding boxes at deepest level */
	    if (!interrupt_flag)
		draw_inst_window (-1, tewl, tewr, tewb, tewt);
	}

	if (pict_arr[Gridnr] != SKIP) {
	    disp_axis ();
	    display_grids (tewl, tewr, tewb, tewt);
	}

	if (pict_arr[DRC_nr] != SKIP) {
	    draw_drc_err (piwl, piwr, piwb, piwt);
	}

	ggSetColor (Gridnr);
	d_fillst (FILL_SOLID);
	Erase_hollow = 0;
	disp_mode (TRANSPARENT);
	flush_pict ();
    }
    if (erase_text) ptext ("");
    pict_all (SKIP);
    piwl = loXL; /* set pic_max */
    piwb = loYB;
    piwr = upXR;
    piwt = upYT;
}

void pict_all (int mode)
{
    register int i;
    for (i = 0; i < NR_all; ++i) pict_arr[i] = mode;
}

static void pict_all_blacks (int mode)
{
    register int i;
    for (i = 0; i < NR_lay; ++i) if (black_arr[i]) pict_arr[i] = mode;
}

static void pict_all_dominants (int mode)
{
    register int i;
    for (i = 0; i < NR_lay; ++i) if (dom_arr[i]) pict_arr[i] = mode;
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
