/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	O. Hol
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

#include <stdio.h>
#ifndef NO_CURSES
#include <curses.h>
#endif

extern void die (int nr);
extern int stopsim;

/* single_curs_step() : can be called in the behavior part of a        */
/*                    : function description when curses is used       */
/*                    : single step is performed when a character      */
/*                    : is typed; 'q' stops the simulation             */
/*                    : immediately, 'x' stops the simulation after    */
/*                    : the current simulation step, 'c' continues     */
/*                    : until the end of simulation.                   */

#ifndef NO_CURSES
void single_curs_step ()
{
    int tmpc;
    static int no_stop = 0;

    if (no_stop == 0) {
	savetty ();
	cbreak ();
	noecho ();

	tmpc = getch ();
	resetty ();
	if (tmpc == 'q'){
	    clear ();
	    (void)mvcur (0, 79, 23, 0);
	    refresh ();
	    endwin ();
	    die (1);
	}
	else if (tmpc == 'x') {
	    clear ();
	    (void)mvcur (0, 79, 23, 0);
	    refresh ();
	    endwin ();
	    stopsim = 1;
	}
	else if (tmpc == 'c')
	    no_stop = 1;
    }
}
#endif
