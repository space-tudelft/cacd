/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 *	Bastiaan Sneeuw
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

#include "src/xedif/incl.h"

extern FILE *fp_out;
extern int dialectCds;
extern int tog_range;
extern int tog_srange;
extern int tog_nobrack;

extern int currTermType;
extern int prPortType;

static char *buf = NULL;
static int bufpos = 0;
static int cpos = 0;

#ifdef __cplusplus
  extern "C" {
#endif
static void opredf (char *);
#ifdef __cplusplus
  }
#endif

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
*/
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range)
{
    long i;
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];

    if (!tog_range && pr_range && dim > 0) {
	long xv[25];

	for (i = 0; i < dim; i++) xv[i] = lower[i];

	for (;;) {
	    nmprint (0, name, dim, xv, xv, 0);

	    for (i = dim - 1;; i--) {
		if (lower[i] <= upper[i]) {
		    if (++xv[i] <= upper[i]) break;
		}
		else {
		    if (--xv[i] >= upper[i]) break;
		}
		if (i == 0) return;
		xv[i] = lower[i];
	    }
	}

        /* This line will never be reached. */
    }

    if (prPortType) oprint (0, "(port ");

    if (dim > 0 && !tog_nobrack)
	oprint (0, pr_range ? "(array " : "(member ");

    if (isdigit ((int)*name) || *name == '_') oprint (0, "&");
    oprint (0, name);

    if (dim > 0) {
	*buf1 = tog_nobrack ? '_' : ' ';
	i = 0;
	do {
	    if (pr_range) {
		long n = upper[i] - lower[i];
		if (n < 0) {
		    if (!tog_srange && upper[i] != 0) goto edif_err;
		    n = -n;
		}
		else
		    if (!tog_srange && lower[i] != 0) goto edif_err;
		sprintf (buf1 + 1, "%ld", n + 1);
	    }
	    else {
		if (tog_nobrack && i + 1 == dim)
		    sprintf (buf1 + 1, "%ld_", lower[i]);
		else
		    sprintf (buf1 + 1, "%ld", lower[i]);
	    }
	    oprint (0, buf1);
	} while (++i < dim);

	if (!tog_nobrack) oprint (0, ")");
    }

    if (prPortType) {
	switch (currTermType) {
	case INPUT:
	case INREAD:
	    /* Added by B.Sneeuw */
	    if (dialectCds)
		oprint (0, "(direction inout)");
	    else
		oprint (0, "(direction input)");
	    break;
	case INOUT:
	    oprint (0, "(direction inout)"); break;
	case OUTPUT:
	case OUTPUT3:
	    oprint (0, "(direction output)");
	}
	oprint (0, ")");
    }
    return;
edif_err:
    fatalErr ("Error: array boundary unequal 0 exists in database!\n",
	"Use option -e (expand indices) or option -s (shift indices)!");
}

/* oprint is the lowest level print routine
*/
void oprint (int concat, char *new_s)
{
    static int bufsize = 0;
    char *s;
    int new_len = strlen (new_s);

    if (!bufpos || concat) goto fill;

    s = buf; /* first, flush current contents of buffer */
    opredf (s);
    bufpos = 0;
fill:
    if (new_len) {
	if (bufpos + new_len >= bufsize) {
	    bufsize += 1000;
	    REPALLOC (buf, bufsize, char);
	}
	strcpy (buf + bufpos, new_s);
	bufpos += new_len;
    }
}

/* outPos returns current position and flushes the buffer
*/
int outPos ()
{
    oprint (0, "");
    return (cpos);
}

static void opredf (char *s)
{
    static int nb = -1, lb = 0;
    int c, i;

    while ((c = *s++)) {
	switch (c) {
	case '\t':
	case ' ':
	    if (lb) putc (' ', fp_out);
	case '\n':
	    continue;
	case '(':
	    ++nb; lb = 1;
	    putc ('\n', fp_out);
	    i = 0;
	    while (i++ < nb) {
		putc (' ', fp_out);
		putc (' ', fp_out);
	    }
	    break;
	case ')':
	    if (--nb < -1) fatalErr ("Internal error:", "opredf: incorrect ) count");
	    lb = 0;
	}
	putc (c, fp_out);
    }
}
