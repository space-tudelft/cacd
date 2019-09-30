/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Nick van der Meijs
 *	Arjan van Genderen
 *	Simon de Graaf
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

typedef int coor_t;

typedef struct edge {
    coor_t xl, yl, xr, yr;
    struct edge * fwd, * bwd;
    struct edge * contour, * link;
} edge_t;

#define INF INT_MAX
#define Scale(x) (scale * (x))
#define Y(e,x) (e->yl==e->yr?e->yl:e->yl+((x-e->xl)*(e->yr-e->yl))/(e->xr-e->xl))

#ifdef EPSPLOT
#define LINE            0
#define TXT             1
#endif /* EPSPLOT */

/* input.c */
void openInput (DM_CELL *cellKey, char *mask);
void closeInput (void);
edge_t *fetch (void);
void printEdge (char *s, edge_t *edge);

/* main.c */
void die (void);
int doAnnotations (void);

/* plot.c */
void mplotEnd (void);
void mplotInit (DM_CELL *key);
void plotMask (char *name, int color);
void plotTerminal (long xl, long xr, long yb, long yt, char *name);
void plotContour (edge_t *edge);
void printLinks (edge_t *edge);

/* plot_eps.c */
void plotSetRotation (void);
void plotSetLambda (char *s);
void plotSetDrawWidth (char *s);
void plotSetDrawHeight (char *s);
void mplotEnd (void);
void plotBbox (void);
void plotPass (int pass);
void endLayer (void);
void setLayer (char *layer);
void mplotInit (DM_CELL *key);
void plotMask (char *name, int color);
void plotTerminal (long xl, long xr, long yb, long yt, char *name);
void plotContour (edge_t *edge);
void mplotLine (long int x1, long int y1, long int x2, long int y2, int mode);
void plotText (long int x, long int y, int orient, char *text);
void printLinks (edge_t *edge);
void doEpsProlog (long l, long b, long r, long t);
void doEpsTechnology (DM_PROJECT *project, char *techDef, char *techFile, int *order_return, int *restroke_return);

/* scan.c */
void scan (void);

