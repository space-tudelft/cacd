/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/sls/extern.h"

extern void cmdinits (void);
extern void restart_scanner (FILE *inputfile);
extern int  yy0parse (void);

extern FILE *yy0in;
extern int yy0lineno;
extern int try_sta_file;

void getcommands ()
{
    char fn_stat[40];
    char *fn_save;
    struct stat sbuf;
    int s;

    if (monitoring) monitime ("B getcommands");

    cmdinits ();

    OPENR (yy0in, fn_cmd);
    yy0lineno = 1;
    yy0parse ();
    CLOSE (yy0in);

    if (try_sta_file) {
	sprintf (fn_stat, "%s.sta", netwname);
	/* Look if a file is present that contains state definitions */
	s = stat (fn_stat, &sbuf);
	if (s != 0) {
	    if (fn_stat[0] >= 'A' && fn_stat[0] <= 'Z') {
		/* If the file does not exist, check the same file name
		   but now with a lower case character as the first character.
		*/
		fn_stat[0] = fn_stat[0] - 'A' + 'a';
		s = stat (fn_stat, &sbuf);
	    }
	}
	if (s == 0) {
	    fn_save = fn_cmd;
	    fn_cmd = fn_stat;
	    OPENR (yy0in, fn_cmd);

            /* this function re-initializes the lex scanner: */
            restart_scanner (yy0in);
	    yy0lineno = 1;
	    yy0parse ();
	    CLOSE (yy0in);
	    fn_cmd = fn_save;
	}
    }

    if (fatalerror) die (1);

    if (monitoring) monitime ("E getcommands");
}
