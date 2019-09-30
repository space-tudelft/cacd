
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/xvhdl/incl.h"

extern FILE *fp_out;
extern int maxLL;

extern int currTermType;
extern int prPortType;

int out_indent  = 0; /* output indent */
int out_tabstep = 8; /* output tab step, don't make it zero ! */

static char *buf = NULL;
static int bufpos = 0;
static int cpos = 0;
static int comment = 0;

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
*/
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range)
{
    long i;
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *ps;

    if (prPortType) {
	char *type = "";
	switch (currTermType) {
	case INPUT:
	    type = " IN"; break;
	case INOUT:
	    type = " INOUT"; break;
	case OUTPUT:
	case OUTPUT3:
	    type = " OUT";
	}
	ps = buf1;
	if (dim > 0) {
	    i = 0;
	    if (dim >= 2) {
		sprintf (ps, "%s:%s IS ARRAY (%ld %s %ld) OF", name, type,
		    lower[i], lower[i] > upper[i] ? "DOWNTO" : "TO", upper[i]);
		if (dim > 2) goto vhdl_err;
		++i;
	    }
	    else {
		sprintf (ps, "%s:%s", name, type);
	    }
	    while (*++ps) ;
	    sprintf (ps, " STD_LOGIC_VECTOR(%ld %s %ld)",
		lower[i], lower[i] > upper[i] ? "DOWNTO" : "TO", upper[i]);
	}
	else
	    sprintf (ps, "%s:%s STD_LOGIC", name, type);
	name = buf1;
    }
    else if (dim > 0) {
	ps = buf1;
	if (pr_range)
	    sprintf (ps, "%s_%ld", name, lower[0]);
	else {
	    sprintf (ps, "%s(%ld)", name, lower[0]);
	    if (dim > 2) goto vhdl_err;
	}
	for (i = 0; ++i < dim;) {
	    while (*++ps) ;
	    if (pr_range)
		sprintf (ps, "_%ld", lower[i]);
	    else
		sprintf (ps, "(%ld)", lower[i]);
	}
	name = buf1;
    }
    oprint (0, name);
    return;
vhdl_err:
    oprint (0, buf1);
    oprint (0, "\n");
    oprint (0, "\n");
    fatalErr ("Error: array dimension > 2 exists in database!", NULL);
}

/* oprint is the lowest level print routine
*/
void oprint (int concat, char *new_s)
{
    static int bufsize = 0;
    int len, endnewline, pr, cpos_old;
    char *ps, *s;
    int new_len = strlen (new_s);

    if (!bufpos || concat) goto fill;

    s = buf; /* first, flush current contents of buffer */

    if (s[0] == '\n') {
	do {
	    putc ('\n', fp_out);
	    bufpos--;
	} while (*++s == '\n');
	cpos = 0;
	comment = 0;
	if (bufpos <= 0) goto save;
    }

    cpos_old = cpos;

    if (s[0] != '*') {
	if (cpos < out_indent) cpos = out_indent;

	if (s[0] == '\t') {
	    len = (cpos - out_indent) % out_tabstep;
	    if (len > 0) cpos += out_tabstep - len;
	    s++;
	    bufpos--;
	}
    }

    len = bufpos;

    if (len > 0 && s[len-1] == '\n') {
	len--;
	endnewline = 1;
    }
    else
	endnewline = 0;


    pr = 1;

    if (maxLL && len > 0 && len + cpos >= maxLL && cpos > out_indent) {
	putc ('\n', fp_out);
	cpos = 0;
	if (cpos < out_indent) { /* add space */
	    do { putc (' ', fp_out); } while (++cpos < out_indent);
	}

	/* contains buf only space? */
	ps = s;
	while (*ps == ' ') ps++;
	if (!*ps) pr = 0; /* don't print */
    }
    else if (cpos_old < cpos) { /* add space */
	do { putc (' ', fp_out); } while (++cpos_old < cpos);
    }

    if (pr) {
	if (endnewline)
	    cpos = 0;
	else {
	    cpos += len;
	    if (maxLL && len == 1 && s[0] == ' ' &&
		cpos + new_len + 1 >= maxLL) goto save;
	}
	fprintf (fp_out, "%s", s);
    }

save:
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
