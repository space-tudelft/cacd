/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
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

#include "src/ocean/seadali/header.h"

#define	MESS_LENGTH 150

extern struct Disp_wdw *p_wdw;
extern DM_PROJECT *dmproject;
extern Coor xltb, xrtb, ybtb, yttb; /* bound box total */
extern char *cellstr;
extern INST *act_inst;
extern TERM *act_term;
extern int ImageMode;

/*
** List process information of top project.
*/
void inform_process ()
{
    char infostr[MESS_LENGTH];
    sprintf (infostr, "the process used is: %s,   lambda is: %.3fu",
	dmproject -> maskdata -> pr_name, dmproject -> lambda);
    ptext (infostr);
}

/*
** List picture window size.
*/
void inform_window ()
{
    char infostr[MESS_LENGTH];
    sprintf (infostr, "window: xl = %.1f, xr = %.1f, yb = %.1f, yt = %.1f",
	p_wdw -> wxmin / QUAD_LAMBDA, p_wdw -> wxmax / QUAD_LAMBDA,
	p_wdw -> wymin / QUAD_LAMBDA, p_wdw -> wymax / QUAD_LAMBDA);
    ptext (infostr);
}

/*
** List active terminal info.
*/
void inform_act_term ()
{
    char infostr[MESS_LENGTH];
    if (!act_term) {
	ptext ("No actual terminal set!");
	return;
    }
    sprintf (infostr, "name: %s   place: %ld, %ld, %ld, %ld    rep.: %d, %ld, %d, %ld",
	act_term -> tmname,
	act_term -> xl / QUAD_LAMBDA, act_term -> xr / QUAD_LAMBDA,
	act_term -> yb / QUAD_LAMBDA, act_term -> yt / QUAD_LAMBDA,
	act_term -> nx, act_term -> dx / QUAD_LAMBDA,
	act_term -> ny, act_term -> dy / QUAD_LAMBDA);
    ptext (infostr);
}

/*
** List active instance info.
*/
void inform_act_inst ()
{
    char infostr[MESS_LENGTH];
    if (!act_inst) {
	ptext ("No actual instance set!");
	return;
    }
    sprintf (infostr, "name: %s   dimensions: %ld, %ld, %ld, %ld   transl.: %d, %d   rep.: %d, %ld, %d, %ld",
	act_inst -> templ -> cell_name,
	act_inst -> bbxl / QUAD_LAMBDA, act_inst -> bbxr / QUAD_LAMBDA,
	act_inst -> bbyb / QUAD_LAMBDA, act_inst -> bbyt / QUAD_LAMBDA,
	(int) act_inst -> tr[2] / QUAD_LAMBDA, (int) act_inst -> tr[5] / QUAD_LAMBDA,
	act_inst -> nx, act_inst -> dx / QUAD_LAMBDA, act_inst -> ny, act_inst -> dy / QUAD_LAMBDA);
    ptext (infostr);
}

/*
** List active cell info.
*/
void inform_cell ()
{
    char infostr[MESS_LENGTH];
    if (!cellstr) {
	ptext ("No cell name!");
	return;
    }
    if (ImageMode == TRUE) {
	inform_cell_image (cellstr, xltb, xrtb, ybtb, yttb);
	return;
    }
    sprintf (infostr, "cellname: %s   bounding_box: %ld, %ld, %ld, %ld", cellstr,
	xltb / QUAD_LAMBDA, xrtb / QUAD_LAMBDA, ybtb / QUAD_LAMBDA, yttb / QUAD_LAMBDA);
    ptext (infostr);
}
