
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

#include "src/xnle/incl.h"

extern FILE *fp_out;
extern char **globNets;
extern int globNets_cnt;

extern struct term_ref *NLEportrefs;

extern int currTermType;
extern int NLENodeType;
extern long NLENodeIndex;
extern int NLEMapStatement;

static char *buf = NULL;
static int bufpos = 0;
static int cpos = 0;

struct nametable {
    long nr;
    char *name;
};

static struct nametable *ntab = NULL;
static long ntabNr = 0;
static long ntabN = 0;
static long ntabSize = 0;

char *makeArrayName (int node, char *name, long dim, long *lower)
{
    static char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *s;
    int j;

    if (dim <= 0) return (name);

    if (node) --dim;
    s = buf1;
    *s = *name;
    while (*s) *++s = *++name;
    j = 0;
    while (j < dim) {
	sprintf (s, "_%ld", lower[j++]);
	while (*++s) ;
    }
    if (node) sprintf (s, "[%ld]", lower[j]);
    else { *s++ = '_'; *s = 0; }

    return (buf1);
}

/* Argument:
**  concat   -- concat this name to the previous one
**  pr_range -- print range
*/
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range)
{
    long i;
    char buf1[DM_MAXNAME + DM_MAXNAME + 118];
    char *orig_name;
    char *ps;
    int j;

    if (pr_range && dim > 0) {
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

    orig_name = name;

    if (dim > 0) {
	name = makeArrayName (NLENodeIndex || NLEMapStatement, name, dim, lower);
    }
    else if (NLENodeType == 'X' && !currTermType && NLEportrefs) {
	struct term_ref *tref = NLEportrefs;
	do {
	    if (strcmp (tref -> t -> term_name, name) == 0) {
		if (tref -> type == GLOBAL) return;
		currTermType = tref -> type;
		break;
	    }
	} while ((tref = tref -> next));
    }

    if (NLENodeIndex) {
	sprintf (buf1, "%c %ld", NLENodeType, NLENodeIndex);
	oprint (0, buf1);
    }

    if (isdigit ((int)*name))
	oprint (0, " _");
    else
	oprint (0, " ");

    ps = orig_name;

    if (dim == 0 && (NLENodeType || NLEMapStatement)) {
	for (j = 0; j < globNets_cnt; ++j) {
	    if (strcmp (globNets[j], name) == 0) { /* found */
		ps = buf1;
		--name;
		while (*++name) {
		    if (islower ((int)*name)) *ps++ = toupper (*name);
		    else *ps++ = *name;
		}
		*ps = 0;
		name = buf1;
		goto prt;
	    }
	}
	if (NLENodeType != 'G') {
	    j = strlen (name) - 1;
	    if (name[j] == '_') {
		ps = name + j - 1;
		if (isdigit ((int)*ps)) {
		    --ps;
		    while (isdigit ((int)*ps)) --ps;
		    if (*ps == '_') {
			strcpy (buf1, name);
			buf1[j] = ']';
			j = ps - name;
			buf1[j] = '[';
			ps = buf1 + j;
			name = buf1;
		    }
		}
	    }
	}
    }
prt:
    oprint (0, name);

    if (NLENodeType) {
	if (NLENodeType == 'X') {
	    if (!currTermType && NLEportrefs) {
		struct term_ref *tref = NLEportrefs;

		if (*ps == '[') {
		    *ps = 0;
		    orig_name = name;
		}
		do {
		    if (strcmp (tref -> t -> term_name, orig_name) == 0) {
			currTermType = tref -> type;
			break;
		    }
		} while ((tref = tref -> next));
		if (!tref) P_E "Warning: port \"%s\" not found in orig. circuit!\n", orig_name);
	    }

	    switch (currTermType) {
	    case INPUT:
	    case INREAD:
		oprint (0, " i"); break;
	    case INOUT:
		oprint (0, " b"); break;
	    case OUTPUT:
		oprint (0, " o"); break;
	    case OUTPUT3:
		oprint (0, " ot"); break;
	    }
	}
	oprint (0, ";\n");
    }
}

long testNameNbr (char *name)
{
    long i;
    if (name)
    for (i = 0; i < ntabN; i++)
	if (strcmp (ntab[i].name, name) == 0) return (ntab[i].nr);
    return (-1L);
}

int testNbr (long nr)
{
    long i;
    for (i = 0; i < ntabN; i++)
	if (ntab[i].nr == nr) return (1);
    return (0);
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

/* oprint is the lowest level print routine
*/
void oprint (int concat, char *new_s)
{
    static int bufsize = 0;
    int new_len = strlen (new_s);

    if (!bufpos || concat) goto fill;

    /* first, flush current contents of buffer */
    fprintf (fp_out, "%s", buf);
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
