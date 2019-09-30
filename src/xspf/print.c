
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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

#include "src/xspf/incl.h"

extern int spef_print;
extern FILE *fp_out;
extern int maxLL;
extern int tog_nobrack;
extern int tog_use0;
extern int tog_vss0;
extern int tog_gnd0;
extern char node0[];
extern char *nameGND;

extern int currTermType;
extern int use_spef;

int rm_gnd_node = 0;
int in_prHead   = 0; /* see prHead */
int out_indent  = 0; /* output indent */
int out_tabstep = 8; /* output tab step, don't make it zero ! */

char spef_dnet[DM_MAXNAME + 118];
static char *buf = NULL;
static int bufpos = 0;
static int cpos = 0;
static int cpos_save = 0;
static int comment = 0;

struct nametable {
    long nr;
    char *name;
};

static struct nametable *ntab = NULL;
static long ntabNr = 0;
static int ntabN = 0;
static int ntabSize = 0;

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
*/
void nmprint (int concat, char *name, int dim, int *lower, int *upper, int pr_range)
{
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *ps;
    int i;

    if (pr_range && dim > 0) {
	int xv[25];

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

	    if (!use_spef) oprint (0, " ");
	}

        /* This line will never be reached. */
    }

    ps = buf1;
    if (spef_print) {
	/* SPEF: add backslash in front of D_NET name or add D_NET name to NODE (SdeG) */
	if (isdigit ((int)*name)) {
	    if (spef_print == 1) *ps++ = '\\';
	    else {
		char *s = spef_dnet;
		while (*s) *ps++ = *s++;
		*ps++ = ':';
	    }
	}
    }
    *ps = *name;
    if (spef_print == 1) {
	/* SPEF: skip part of D_NET name, starting from ':' (SdeG) */
	while (*ps) { *++ps = *++name; if (*ps == ':') *ps = 0; }
    }
    else {
	while (*ps) *++ps = *++name;
    }

    if (dim > 0) {
	*ps++ = '[';
	for (i = 0;;) {
	    if (pr_range)
	        sprintf (ps, "%d..%d", lower[i], upper[i]);
	    else
	        sprintf (ps, "%d", lower[i]);
	    while (*++ps) ;
	    if (++i >= dim) break;
	    *ps++ = ',';
	}
	*ps++ = ']';
	*ps = 0;
    }

    if (tog_use0 && test0 (buf1) == 0) {
	long nr;
	if ((nr = testNameNbr (buf1)) < 0) {
	    if (!in_prHead) nr = 0;
	    assignNameNbr (buf1, nr);
	}
	if (nr == 0) {
	    if (rm_gnd_node) { spef_print = 0; return; }
	    strcpy (buf1, "0");
	    goto ret;
	}
    }

    if (tog_nobrack) {
	for (ps = buf1; *ps; ps++) {
	    if (*ps == '[' || *ps == ']' || *ps == ',') *ps = '_';
	}
    }

    if (use_spef) {
	ps = buf1;
	if (*ps == '\\' && spef_print) ++ps;
	for (; *ps; ps++) {
	    if (!isalnum ((int)*ps) && *ps != '_' && *ps != '[' && *ps != ']' && (*ps != ':' || !spef_print)) {
		int a, b;
		char *q = ps;
		a = *q++;
		for (;;) {
		    b = *q;
		    *q++ = a;
		    if (!a) break;
		    a = b;
		}
		*ps++ = '\\';
	    }
	}
	if (spef_print == 1) strcpy (spef_dnet, buf1); /* save D_NET name (SdeG) */
	spef_print = 0;
    }
ret:
    oprint (concat, buf1);

    if (use_spef && in_prHead) {
	switch (currTermType) {
	case INPUT:
	case INREAD:
	    oprint (1, " I\n"); break;
	case OUTPUT:
	case OUTPUT3:
	    oprint (1, " O\n"); break;
	case INOUT:
	default:
	    oprint (1, " B\n"); break;
	}
    }
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
    int i;
    for (i = 0; i < ntabN; i++)
	if (strcmp (ntab[i].name, name) == 0) return (ntab[i].nr);
    return (-1L);
}

int testNbr (long nr)
{
    int i;
    for (i = 0; i < ntabN; i++)
	if (ntab[i].nr == nr) return (1);
    return (0);
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
	    if (ntabN == INT_MAX) fatalErr ("error:", "ntabN == INT_MAX");
	    ntabSize = 2 * ntabSize;
	    if (ntabSize < 0) ntabSize = INT_MAX;
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
	int c;

	putc ('\n', fp_out);

	cpos = 2;
	if (comment) {
	    c = '*'; /* comment line continuation */
	}
	else {
	    c = '+'; /* general line continuation */
	}

	putc (c, fp_out);
	if (cpos == 2) putc (' ', fp_out);

	if (cpos < out_indent) { /* add space */
	    do {
		putc (' ', fp_out);
	    } while (++cpos < out_indent);
	}

	/* contains buf only space? */
	ps = s;
	while (*ps == ' ') ps++;
	if (!*ps) pr = 0; /* don't print */
    }
    else if (cpos_old < cpos) { /* add space */
	do {
	    putc (' ', fp_out);
	} while (++cpos_old < cpos);
    }

    if (endnewline) comment = 0;
    else if (cpos == 0 && s[0] == '*') comment = 1;

    if (pr) {
	if (endnewline)
	    cpos = 0;
	else {
	    cpos += len;
	    if (maxLL && len == 1 && s[0] == ' ' &&
		cpos + new_len >= maxLL) goto save;
	}

	fprintf (fp_out, "%s", s);
    }

save:
    cpos_save = cpos;

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
