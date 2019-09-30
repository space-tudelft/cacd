
/*
 * ISC License
 *
 * Copyright (C) 1987-2013 by
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

#include "src/xspice/incl.h"

extern FILE *fp_out;
extern int dialect;
extern int maxLL;
extern int tog_nobrack;
extern int tog_use0;
extern int tog_vss0;
extern int tog_gnd0;
extern char node0[];
extern char *nameGND;

int in_prHead   = 0; /* see prHead */
int sameline    = 0; /* tanner inline comments */
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

struct nametable {
    long nr;
    char *name;
};

static struct nametable *ntab = NULL;
static long ntabNr = 0;
static long ntabN = 0;
static long ntabSize = 0;

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
**  pr_nbr   -- print number instead of identifier
*/
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range, int pr_nbr)
{
    long i;
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *ps;

    if (pr_range && dim > 0) {
	long xv[25];

	for (i = 0; i < dim; i++) xv[i] = lower[i];

	for (;;) {
	    nmprint (0, name, dim, xv, xv, 0, pr_nbr);

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
#ifdef XPSTAR
	    oprint (1, ",");
#endif
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

    if (pr_nbr) {
	sprintf (buf1, "%ld", nameNbr (buf1));
	goto ret;
    }
    if (tog_use0 && test0 (buf1) == 0) {
	if ((i = testNameNbr (buf1)) < 0) {
	    if (!in_prHead) i = 0;
	    assignNameNbr (buf1, i);
	}
	if (i == 0) {
	    strcpy (buf1, "0");
	    goto ret;
	}
    }

    if (tog_nobrack) {
	for (ps = buf1; *ps; ps++) {
	    if (*ps == '[' || *ps == ']' || *ps == ',') *ps = '_';
	}
    }

ret:
    oprint (concat, buf1);
}

/* nameNbr returns a number for the string 'name'
*/
long nameNbr (char *name)
{
    long nr = testNameNbr (name);
    if (nr < 0) {
	if (tog_use0 && !in_prHead) nr = test0 (name);
	nr = assignNameNbr (name, nr);
    }
    return (nr);
}

long testNameNbr (char *name)
{
    long i;
    for (i = 0; i < ntabN; i++)
	if (strcmp (ntab[i].name, name) == 0) return (ntab[i].nr);
    return (-1L);
}

int test0 (char *name)
{
    if (tog_use0) {
	if (nameGND && strcmp (name, nameGND) == 0) return (0);
	if (tog_vss0) {
	  if ((name[0] == 'v' && name[1] == 's' && name[2] == 's')
	   || (name[0] == 'V' && name[1] == 'S' && name[2] == 'S')) return (0);
	}
	if (tog_gnd0) {
	  if ((name[0] == 'g' && name[1] == 'n' && name[2] == 'd')
	   || (name[0] == 'G' && name[1] == 'N' && name[2] == 'D')) return (0);
	}
	if (node0[0]) {
	    if ((int)strlen (name) >= tog_use0 &&
		strncmp (name, node0, tog_use0) == 0) return (0);
	}
    }
    return (-1);
}

/*
** Function to assign a number to a name.
** name should not exist already!
*/
long assignNameNbr (char *name, long nr)
{
    if (ntabN >= ntabSize) {
	if (ntabSize == 0) {
	    ntabSize = 50;
	    PALLOC (ntab, ntabSize, struct nametable);
	}
	else {
	    ntabSize = 2 * ntabSize;
	    REPALLOC (ntab, ntabSize, struct nametable);
	}
    }
    ntab[ntabN].name = newStringSpace (name);

    if (nr < 0) nr = ++ntabNr;

    ntab[ntabN++].nr = nr;

    return (nr);
}

void nameNbrReset ()
{
    ntabN = 0;
    ntabNr = 0;
}

/* ntabprint prints the name table used by nameNbr ()
*/
void ntabprint ()
{
    long nr;
    long i;
    char buf1[32];

    oprint (0, "*\n");

    for (nr = 0; nr <= ntabNr; nr++) {
	for (i = 0; i < ntabN; i++) {
	    if (ntab[i].nr == nr) goto prt;
	}
    }
    return;

prt:
    oprint (0, "*");	/* start comment */
    out_indent = 2;
    out_tabstep = 15;

    for (nr = 0; nr <= ntabNr; nr++) {
	for (i = 0; i < ntabN; i++) {
	    if (ntab[i].nr == nr) {
		sprintf (buf1, "\t%3ld ", ntab[i].nr);
		oprint (0, buf1);
		oprint (1, ntab[i].name);
	    }
	}
    }
    oprint (0, "\n*\n"); /* end comment */

    out_indent = 0;
    out_tabstep = 8;
}

void startComment ()
{
    oprint (0, "*");
}

/* oprint is the lowest level print routine
*/
void oprint (int concat, char *new_s)
{
    static int bufsize = 0;
    int len, endnewline, pr, cpos_old;
    char *ps, *s;
    int new_len = strlen (new_s);

#ifdef XSPECTRE
    if (sameline == 1) new_len += 3; /* add " //" */
#else
    if (sameline == 1) new_len += 2; /* add " $" */
#endif

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
	int c;
#ifdef XSPECTRE
	if (save2) helpBuf[helpBufLen++] = '\\';
	else putc ('\\', fp_out);
#endif
	if (save2) helpBuf[helpBufLen++] = '\n';
	else putc ('\n', fp_out);
	cpos = 2;
	if (comment) { /* comment line continuation */
#ifdef XSPECTRE
	    c = '/';
	    if (save2) helpBuf[helpBufLen++] = c;
	    else putc (c, fp_out);
#else
	    c = '*';
#endif
	}
	else { /* general line continuation */
#ifdef XSPECTRE
	    c = ' ';
#else
	    c = dialect == PSTAR ? ' ' : '+';
	    if (c == '+' && sameline == 2) { /* comment */
		if (buf[0] == ' ' && buf[1] == '$' && buf[2] == ' ') cpos = 1;
	    }
#endif
	}

	if (save2) {
	    helpBuf[helpBufLen++] = c;
	    if (cpos == 2) helpBuf[helpBufLen++] = ' ';
	}
	else {
	    putc (c, fp_out);
	    if (cpos == 2) putc (' ', fp_out);
	}

	if (c == '+' && sameline == 2) {
	    out_indent = 0;
	    if (cpos == 2) { /* add line comment */
		if (save2) {
		    helpBuf[helpBufLen++] = '$';
		} else {
		    putc ('$', fp_out);
		}
		++cpos;
	    }
	}

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

    if (endnewline) comment = 0;
    else if (cpos == 0 && s[0] == '*') comment = 1;
    else if (comment == 1) comment = 2;

    if (pr) {
	if (endnewline) cpos = 0;
	else {
	    cpos += len;
	    pr = cpos + new_len;
#ifdef XPSTAR
	    ++pr; /* tog_comma */
#endif
	    if (maxLL && len == 1 && s[0] == ' ' && pr >= maxLL) goto save;
	}

	if (save2) {
	    if (helpBufSize <= helpBufLen + bufpos) {
		helpBufSize += 1000;
		REPALLOC (helpBuf, helpBufSize, char);
	    }
#ifdef XSPECTRE
	    if (comment == 1) helpBuf[helpBufLen++] = s[0] = '/';
#endif
	    strcpy (helpBuf + helpBufLen, s);
	    helpBufLen += bufpos;
	}
	else {
#ifdef XSPECTRE
	    if (comment == 1) fprintf (fp_out, "//%s", s+1);
	    else
#endif
	    fprintf (fp_out, "%s", s);
	}
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
	if (sameline == 1) { /* add " $" */
	    sameline = 2;
	    buf[bufpos++] = ' ';
#ifdef XSPECTRE
	    buf[bufpos++] = '/';
	    buf[bufpos++] = '/';
	    new_len -= 3;
#else
	    buf[bufpos++] = '$';
	    new_len -= 2;
#endif
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
