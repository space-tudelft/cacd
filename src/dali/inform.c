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

#include "src/dali/header.h"

#define	MESS_LENGTH 150

extern struct Disp_wdw *p_wdw;
extern DM_PROJECT *dmproject;
extern Coor xltb, xrtb, ybtb, yttb; /* bound box total */
extern char *cellstr;
extern int  exp_level;

/*
** List process information of top project.
*/
void inform_process ()
{
    char infostr[MESS_LENGTH];
    sprintf (infostr, "Process '%s', type '%s', lambda = %.3f micron",
	dmproject -> maskdata -> pr_name,
	dmproject -> maskdata -> pr_type,
	dmproject -> lambda);
    ptext (infostr);
}

/*
** List picture window size.
*/
void inform_window ()
{
    char infostr[MESS_LENGTH];
    sprintf (infostr, "Current window: xl = %.1f, xr = %.1f, yb = %.1f, yt = %.1f",
	p_wdw -> wxmin / QUAD_LAMBDA, p_wdw -> wxmax / QUAD_LAMBDA,
	p_wdw -> wymin / QUAD_LAMBDA, p_wdw -> wymax / QUAD_LAMBDA);
    ptext (infostr);
}

/*
** List active cell info.
*/
void inform_cell ()
{
    char infostr[MESS_LENGTH];
    sprintf (infostr, "Cell '%s'  bbox: %ld, %ld, %ld, %ld  level: %d",
	cellstr ? cellstr : "????",
	xltb / QUAD_LAMBDA, xrtb / QUAD_LAMBDA,
	ybtb / QUAD_LAMBDA, yttb / QUAD_LAMBDA, exp_level);
    ptext (infostr);
}
