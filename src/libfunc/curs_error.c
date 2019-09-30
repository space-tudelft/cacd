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

/* curs_error() : can be called in the behavior part of function */
/*              : of function description when curses is used    */
/*              : message is given, simulation can be stopped    */

#ifndef NO_CURSES
void curs_error ()
{
    int tmpc;

    move (23, 0);
    printw ("an error has been detected; do you want to continu? (y/n)");
    refresh ();
    savetty ();
    cbreak ();
    noecho ();

    tmpc = getch ();
    resetty ();
    if (tmpc != 'y') {
	clear ();
	(void)mvcur (0, 79, 23, 0);
	refresh ();
	endwin ();
	die (0);
    }
    else {
	clear ();
	refresh ();
    }
}
#endif
