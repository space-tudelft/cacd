/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

extern int NoErr;
extern int columns;
extern int view_nr;
extern DM_PROJECT *project;

static int maxC = 0;
static int maxF = 0;

static char **celllst;
static char **fcelllst;

static int gmatch (char *s, char *p);
static void print (char **cl, int n);

void lscells (char *cellName, char **cl, IMPCELL **icl)
{
	struct stat buf;
	DM_CELL *dmcell;
	char *view;
	int nentry = 0;
	int fentry = 0;

	switch (view_nr) {
	case 1: view = LAYOUT; break;
	case 2: view = CIRCUIT; break;
	case 3: view = FLOORPLAN; break;
	default: view = 0;
	}

	for (; *cl; ++cl) {
	    if (gmatch (*cl, cellName)) {
		NoErr = 1;
		dmcell = dmCheckOut (project, *cl, ACTUAL, DONTCARE, view, READONLY);
		if (dmcell) {
		    if (view_nr == 2 && dmStat (dmcell, "fterm", &buf) == 0) {
			if (fentry == maxF) {
			    maxF += 2048;
			    fcelllst = realloc (fcelllst, sizeof (char *) * maxF);
			}
			fcelllst[fentry++] = *cl;
			NoErr = 0;
		    }
		    if (dmStat (dmcell, "mc", &buf) == 0) {
			if (nentry == maxC) {
			    maxC += 2048;
			    celllst = realloc (celllst, sizeof (char *) * maxC);
			}
			celllst[nentry++] = *cl;
			NoErr = 0;
		    }
		    dmCheckIn (dmcell, COMPLETE);
		}
		else ++NoErr;
		if (NoErr) {
		    char *s;
		    int len = strlen (*cl);
		    s = (char *) malloc (len + 2);
		    strcpy (s, *cl);
		    s[len++] = (NoErr == 2)? '!' : '?';
		    s[len] = '\0';
		    if (nentry == maxC) {
			maxC += 2048;
			celllst = realloc (celllst, sizeof (char *) * maxC);
		    }
		    celllst[nentry++] = s;
		    NoErr = 0;
		}
	    }
	}
	if (nentry > 0) {
	    print (celllst, nentry);
	}
	if (fentry > 0) {
	    printf ("function:\n");
	    print (fcelllst, fentry);
	}

	nentry = 0;
	for (; *icl; ++icl) {
	    if (gmatch ((*icl) -> alias, cellName)) {
		if (nentry == maxC) {
		    maxC += 2048;
		    celllst = realloc (celllst, sizeof (char *) * maxC);
		}
		celllst[nentry++] = (*icl) -> alias;
	    }
	}
	if (nentry > 0) {
	    printf ("imported:\n");
	    print (celllst, nentry);
	}
}

static int compare (const void *v1, const void *v2)
{
    char **s1 = (char **)v1;
    char **s2 = (char **)v2;
    return (strcmp (*s1, *s2));
}

static void print (char **cl, int nentry)
{
    int lines, i, j, k, max_len;

    qsort (cl, nentry, sizeof (char *), compare);

    max_len = 14;
    for (i = 0; i < nentry; ++i) if ((k = strlen (cl[i])) > max_len) max_len = k;

    lines = (nentry + columns - 1) / columns;

    for (i = 0; i < lines; i++) {
	for (j = 0; j < columns; j++) {
	    k = j * lines + i;
	    if (k + lines >= nentry) {
		printf ("%s\n", cl[k]);
		break;
	    }
	    printf ("%-*s  ", max_len, cl[k]);
	}
    }
}

/* global matcher of names
**
** "*" in params matches r.e ".*"
** "?" in params matches r.e. "."
** "[...]" in params matches character class
** "[...a-z...]" in params matches a through z.
**
*/
static int gmatch (char *s, char *p)
{
	register int scc;
	int ok, lc, c;

	if (!p) return (1);

	if ((scc = *s++)) if ((scc &= 0177) == 0) scc = 0200;

	switch ((c = *p++)) {

	    case '[':
		ok = 0; lc = 077777;
		while ((c = *p++)) {
			if (c == ']') {
			    return (ok ? gmatch (s, p) : 0);
			}
			else if (c == '-') {
			    if (lc <= scc && scc <= (*p++)) ok++;
			}
			else if (scc == (lc = (c & 0177))) ok++;
		}
		break;

	    default:
		if ((c & 0177) != scc) return (0);

	    case '?':
		return (scc ? gmatch (s, p) : 0);

	    case '*':
		if (*p == 0) return (1);
		--s;
		while (*s) if (gmatch (s++, p)) return (1);
		return (0);

	    case 0:
		return (scc == 0);
	}

	return (0);
}
