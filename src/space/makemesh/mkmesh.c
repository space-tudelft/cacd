/*
 * ISC License
 *
 * Copyright (C) 2014-2018 by
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

#include "src/space/include/config.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"

typedef int32 coor_t;
#define INF MAX32

typedef struct Edge {
    coor_t xl, yl, xr, yr;
    struct Edge *fwd, *bwd, *nxt;
} edge_t;

void readEdges (void);
void sortEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
void sortEdge2 (edge_t *e);
edge_t *createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
void disposeEdge (edge_t *edge);

DM_CELL    *cellKey;
DM_PROJECT *dmproject;
DM_STREAM  *outputStream;
char *cellName = NULL;
extern char *argv0;
int optA = 0;
int optB = 0;
int optD = 0;
int optV = 0;

/*
 * This version sorts ONLY horizontal lines!
 */
int main (int argc, char **argv)
{
    int i, o, errflg = 0;

    argv0 = "mkmesh";

    if (argc == 2) {
	if (argv[1][0] != '-') cellName = argv[1];
	optD = 1;
    }
    else if (argc == 3) {
	o = 1;
	if (argv[o][0] != '-') o = 2;
	if (argv[o][0] != '-') {
	    say ("Unknown option argument specified\n"); errflg++;
	}
	else {
	    for (i = 1; argv[o][i]; ++i) {
		switch (argv[o][i]) {
		case 'a': optA = 1; break;
		case 'b': optB = 1; break;
		case 'D': optD = 1; break;
		case 'v': optV = 1; break;
		default:
		    say ("Unknown option '%c' specified\n", argv[o][i]); errflg++;
		}
	    }
	}
	cellName = argv[o == 1 ? 2 : 1];
    }

    if (!cellName) { say ("No cellname specified\n"); errflg++; }
    else if (argc > 3) { say ("Too many specified\n"); errflg++; }

    if (errflg) {
	fprintf (stderr, "Usage: %s [-abDv] cell\n", argv0);
	return (1);
    }

    dmInit ("makemesh");
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    cellKey = dmCheckOut (dmproject, cellName, WORKING, DONTCARE, LAYOUT, ATTACH);

 // if (optD) dmUnlink (cellKey, "mesh_gln");
    if (optD) outputStream = dmOpenStream (cellKey, "mesh_gln", "w");
    readEdges ();
    if (optD) dmCloseStream (outputStream, COMPLETE);

    dmCheckIn (cellKey, COMPLETE);
    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    return (0);
}

void dmError (char *s) { dmPerror (s); die (); }

void die () /* clean-up and stop */
{
    static int recursive = 0;
    if (recursive++) fprintf (stderr, "recursive die: Emergency exit\n");
    else dmQuit ();
    exit (1);
}

void extraSay () { }
//==================================================================================
static edge_t *e2, *e1, *e1fwd;

edge_t * fetchEdge ()
{
    static edge_t z;
    edge_t *edge;

    if (e1) {
	edge = e1;
	if (!(e1 = e1 -> nxt) && e1fwd) { e1 = e1fwd; e1fwd = e1 -> fwd; }
    } else {
	edge = &z;
	edge -> xl = INF;
    }
    return edge;
}

void readEdges ()
{
    DM_STREAM *stream;
    edge_t *e;

    stream = dmOpenStream (cellKey, "mesh_aln", "r");
    while (dmGetDesignData (stream, GEO_GLN) > 0) sortEdge (ggln.xl, ggln.yl, ggln.xr, ggln.yr);
    dmCloseStream (stream, COMPLETE);

    if (e1) while (e1 -> bwd) e1 = e1 -> bwd;
    if (e1) e1fwd = e1 -> fwd;

    if (e1 && optV) {
	int i = 0;
	e2 = e1;
	fprintf (stderr, "\n");
	while ((e = fetchEdge()) && e->xl != INF) {
	    fprintf (stderr, "readEdge: %2d edge(%d %d %d %d)\n", ++i, e->xl, e->xr, e->yl, e->yr);
	}
	fprintf (stderr, "\n");
	e1 = e2;
	e1fwd = e1 -> fwd;
    }

    if (e1 && (outputStream || optB || optV)) {
	int i = 0, j = 0;

	e2 = NULL;
	while ((e = fetchEdge()) && e->xl != INF) sortEdge2 (e);
	e1 = e2;
	while (e1 -> bwd) e1 = e1 -> bwd;
	e1fwd = e1 -> fwd;

	while ((e = fetchEdge()) && e->xl != INF) {
	    if (e->xr == e->xl) ++j;
	    else {
		if (optV) {
		    fprintf (stderr, "readEdge: %2d edge(%d %d %d %d)\n", ++i, e->xl, e->xr, e->yl, e->yr);
		}
		if (optB) {
		    fprintf (stdout, "l %d,%d %d,%d\n", e->xl, e->yl, e->xr, e->yr);
		    fprintf (stdout, "v %d,%d\n", e->xl, e->yl);
		    fprintf (stdout, "v %d,%d\n", e->xr, e->yr);
		}
		if (outputStream) {
		    ggln.xl = e -> xl; ggln.xr = e -> xr; ggln.yl = e -> yl; ggln.yr = e -> yr;
		    dmPutDesignData (outputStream, GEO_GLN);
		}
	    }
	}
	if (j) fprintf (stderr, "\nwarning: %d non-horizontal edges skipped\n", j);
    }
}

void sortEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    static int k = 0;
    int point_e2 = 0;

    ASSERT (xr >= xl);
    ASSERT (yr == yl);

    if (xr == xl) { point_e2 = 1; ASSERT (xr > xl); }

    e2 = createEdge (xl, yl, xr, yr); k++;

    if (optV) fprintf (stderr, "sortEdge: %2d edge(%d %d %d %d)\n", k, xl, xr, yl, yr);
    if (optA) {
	if (point_e2) fprintf (stdout, "X %d,%d\n", xl, yl);
	else {
	    fprintf (stdout, "l %d,%d %d,%d\n", xl, yl, xr, yr);
	    fprintf (stdout, "v %d,%d\n", xl, yl);
	    fprintf (stdout, "v %d,%d\n", xr, yr);
	}
    }

    if (e1) { /* sort on yl */
	if (yl < e1->yl) {
	    while (e1 -> bwd) { e1 = e1 -> bwd; if (yl >= e1->yl) break; }
	}
	else if (yl > e1->yl) {
	    while (e1 -> fwd) { e1 = e1 -> fwd; if (yl <= e1->yl) break; }
	}
	if (yl == e1->yl) {
	    edge_t *p = NULL, *q = e1;
	    while (q->xl <= xl) {
		if (q->xr > xl) { /* merge */
		    if (xr > q->xr) q->xr = xr;
		    disposeEdge (e2);
		    return;
		}
		p = q;
		if (!(q = q -> nxt)) break;
	    }
	    /* !q || (q->xl > xl) */
	    if (q && q->xl < xr) { /* merge */
		q->xl = xl;
		if (xr > q->xr) q->xr = xr;
		disposeEdge (e2);
		return;
	    }
	    e2 -> nxt = q;
	    if (p) { p -> nxt = e2; return; }
	    e2 -> bwd = e1 -> bwd;
	    e2 -> fwd = e1 -> fwd;
	}
	else if (yl < e1->yl) {
	    e2 -> bwd = e1 -> bwd;
	    e2 -> fwd = e1;
	}
	else {
	    e2 -> bwd = e1;
	    e2 -> fwd = e1 -> fwd;
	}
	if (e2 -> bwd) e2 -> bwd -> fwd = e2;
	if (e2 -> fwd) e2 -> fwd -> bwd = e2;
    }
    else e2 -> bwd = e2 -> fwd = NULL;
    e1 = e2;
}

void sortEdge2 (edge_t *e)
{
    e -> nxt = NULL;
    if (e2) {
	if (e->xl < e2->xl) {
	    while (e2 -> bwd) { e2 = e2 -> bwd; if (e->xl >= e2->xl) break; }
	}
	else if (e->xl > e2->xl) {
	    while (e2 -> fwd) { e2 = e2 -> fwd; if (e->xl <= e2->xl) break; }
	}
	if (e->xl == e2->xl) {
	    edge_t *p = NULL, *q = e2;
	    while (q && q->yl < e->yl) q = (p = q) -> nxt;
	    ASSERT (!q || q->yl > e->yl);
	    e -> nxt = q;
	    if (p) { p -> nxt = e; return; }
	    e -> bwd = e2 -> bwd;
	    e -> fwd = e2 -> fwd;
	}
	else if (e->xl < e2->xl) {
	    e -> bwd = e2 -> bwd;
	    e -> fwd = e2;
	}
	else {
	    e -> bwd = e2;
	    e -> fwd = e2 -> fwd;
	}
	if (e -> bwd) e -> bwd -> fwd = e;
	if (e -> fwd) e -> fwd -> bwd = e;
    }
    else e -> bwd = e -> fwd  = NULL;
    e2 = e;
}

//==================================================================================
static edge_t *freeList = NULL;

edge_t * createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr)
{
    edge_t *e;
    if (!(e = freeList)) {
	freeList = NEW (edge_t, 1000);
	e = freeList + 1000;
	--e; e -> fwd = NULL;
	while (e != freeList) { --e; e -> fwd = e + 1; }
    }
    freeList = freeList -> fwd;

    e -> xl = xl;
    e -> xr = xr;
    e -> yl = yl;
    e -> yr = yr;
    e -> nxt = NULL;
    return (e);
}

void disposeEdge (edge_t *edge)
{
    edge -> fwd = freeList;
    freeList = edge;
}
//==================================================================================
