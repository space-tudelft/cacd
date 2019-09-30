
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

#include "src/xsls/incl.h"

extern FILE *fp_out;
extern int maxLL;
extern int tog_range;
extern int tog_nobrack;

int out_indent  = 0; /* output indent */
int out_tabstep = 8; /* output tab step, don't make it zero ! */

int saveInOutBuf = 0;
static int save2 = 0;

static char *buf = NULL;
static int bufpos = 0;
static int cpos = 0;
static int cpos_save = 0;
static int comment = 0;

/* A second output buffer 'helpBuf' with toggle 'saveInOutBuf'
 * is provided for destroying some last part of the output.
 */
static char *helpBuf = NULL;
static int helpBufLen = 0;
static int helpBufSize = 0;

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
*/
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range)
{
    long i;
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *ps;

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

	    oprint (1, ",");
	    oprint (0, " ");
	}

        /* This line will never be reached. */
    }

    ps = buf1;
    *ps = *name;
    while (*ps) *++ps = *++name;

    if (dim > 0) {
	*ps++ = '[';
	for (i = 0;;) {
	    if (pr_range)
	        sprintf (ps, "%ld..%ld", lower[i], upper[i]);
	    else
	        sprintf (ps, "%ld", lower[i]);
	    while (*++ps) ;
	    if (++i >= dim) break;
	    *ps++ = ',';
	}
	*ps++ = ']';
	*ps = 0;
    }

    if (tog_nobrack) {
	for (ps = buf1; *ps; ps++) {
	    if (*ps == '[' || *ps == ']' || *ps == ',') *ps = '_';
	}
    }

    oprint (concat, buf1);
}

/* oprint is the lowest level print routine
*/
void oprint (int concat, char *new_s)
{
    static int bufsize = 0;
    int len, endnewline, pr, cpos_old;
    char *ps, *s;
    int new_len = strlen (new_s);

    if (!bufpos) goto fill;

    if (concat) {
	if (save2 != saveInOutBuf) fatalErr ("Internal error:", "oprint: incorrect save2");
	goto fill;
    }

    s = buf; /* first, flush current contents of buffer */

    if (save2 && helpBufLen > helpBufSize - 200) {
	helpBufSize += 1000;
	REPALLOC (helpBuf, helpBufSize, char);
    }

    if (s[0] == '\n') {
	do {
	    if (save2) helpBuf[helpBufLen++] = '\n';
	    else putc ('\n', fp_out);
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
	if (save2) helpBuf[helpBufLen++] = '\n';
	else putc ('\n', fp_out);
	cpos = 0;
	if (cpos < out_indent) { /* add space */
	    if (save2) {
		do {
		    helpBuf[helpBufLen++] = ' ';
		} while (++cpos < out_indent);
	    }
	    else {
		do {
		    putc (' ', fp_out);
		} while (++cpos < out_indent);
	    }
	}

	/* contains buf only space? */
	ps = s;
	while (*ps == ' ') ps++;
	if (!*ps) pr = 0; /* don't print */
    }
    else if (cpos_old < cpos) { /* add space */
	if (save2) {
	    do {
		helpBuf[helpBufLen++] = ' ';
	    } while (++cpos_old < cpos);
	}
	else {
	    do {
		putc (' ', fp_out);
	    } while (++cpos_old < cpos);
	}
    }

    if (pr) {
	if (endnewline)
	    cpos = 0;
	else {
	    cpos += len;
	    if (maxLL && len == 1 && s[0] == ' ' &&
		cpos + new_len + 1 >= maxLL) goto save;
	}

	if (save2) {
	    if (helpBufSize <= helpBufLen + bufpos) {
		helpBufSize += 1000;
		REPALLOC (helpBuf, helpBufSize, char);
	    }
	    strcpy (helpBuf + helpBufLen, s);
	    helpBufLen += bufpos;
	}
	else fprintf (fp_out, "%s", s);
    }

save:
    if (!save2) cpos_save = cpos;

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
    save2 = saveInOutBuf;
}

/* outPos returns current position and flushes the buffer
*/
int outPos ()
{
    oprint (0, "");
    return (cpos);
}

void destroyOutBuf ()
{
    if (save2) bufpos = 0;

    helpBufLen = 0;

    cpos = cpos_save;

    save2 = saveInOutBuf;
}

void flushOutBuf ()
{
    oprint (0, "");
    if (helpBufLen) {
	helpBuf[helpBufLen] = 0;
	fprintf (fp_out, "%s", helpBuf);
	helpBufLen = 0;
    }
}
