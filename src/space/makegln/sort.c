/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include "src/space/makegln/config.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

#define STATE_INIT 0
#define STATE_SORT 1
#define STATE_FETCH 2
static int state = STATE_INIT;

#ifdef DEBUG
#define SB_INITIAL_BUFS 1
#define BT_INITIAL_SIZE 1
#define EDGEBUF_SIZE    512
#else
#define SB_INITIAL_BUFS 10
#define BT_INITIAL_SIZE 100
#define EDGEBUF_SIZE    512
#endif /* DEBUG */

/*
 * Algorithm: 1) sort subfiles in core
 *            2) write each subfile to tmp file
 *            3) merge subfiles using priority queue algorithm
 *
 * Note: the storage for the intial sort of each block
 * and that for the merge phase is COMMON.
 * This is effected by the operation makeBtBufs.
 */

/* the next items are for the subfile sort phase
 */
static _edge_t ** sortBuf = NULL;
static ecnt_t      sbSize = 0;
static ecnt_t      sbMaxSize = 0; /* capacity of subfile buffer (# edges) */
static ecnt_t      sbCnt = 0;     /* occupancy of subfile buffer */

/* the next structure describes a block during the merge phase
 */
typedef struct {
    long start;		/* ftell position in tmp file of this block */
    ecnt_t  size;	/* total # edges of this block (incl. those on disk */
    ecnt_t cnt;
    _edge_t * bufp;	/* base of edges of this block */
    _edge_t * p;	/* running pointer into *bufp */
    ecnt_t   bufs;	/* capacity (# edges) of *bufp */
    coor_t cursor_x;
    coor_t cursor_y;
} block_t;

static block_t * blocktable;	/* admin of all partition blocks */
static btnum_t btCnt = 0;		/* total number of partition blocks */

static _edge_t ** edgebuftable = NULL;
static btnum_t ebtCnt;
static size_t ebtSize;

static FILE * fp_tmp;
static char * tmpname;

static long infoMaxTempFileSize = 0;
static ecnt_t infoNumEdges = 0;

btnum_t infoNumPartitions;
double infoPartitionBalance;

#ifdef DRIVER
static bool_t optPrint = FALSE;
#endif /* DRIVER */

/* local operations */
Private void writeBlock (_edge_t **base, ecnt_t N);
Private void writeEdge (FILE *fp, _edge_t *e, coor_t *cursor_x, coor_t *cursor_y);
Private void readEdge (FILE *fp, _edge_t *e, coor_t *cursor_x, coor_t *cursor_y);
Private void makeBtBufs (void);
Private _edge_t *fetch (btnum_t b);
Private void fillBuf (btnum_t b);
#ifdef DRIVER
Private void pe (_edge_t *e);
#endif

/* entrance of sort machinery: call this one for each edge */
void sortEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign)
{
    static char buf[256];
    _edge_t * e;

    ASSERT (sign == START || sign == STOP);

    infoNumEdges++;

    if (state != STATE_SORT) {
	sbCnt = 0;
	btCnt = 0;
	if (state != STATE_INIT) {
	    rewind (fp_tmp);
	}
	else {
	    ebtCnt        = 0;
	    ebtSize       = optMaxMemory / (EDGEBUF_SIZE * sizeof (_edge_t));
	    if (ebtSize == 0) ebtSize = 1;
	    edgebuftable  = NEW (_edge_t *, ebtSize);

	    sbSize        = SB_INITIAL_BUFS * EDGEBUF_SIZE;
	    sbMaxSize     = ebtSize * EDGEBUF_SIZE;
	    sortBuf       = NEW (_edge_t *, sbSize);

	    tmpname       = tempname ("mkgln", buf, 256);
	    fp_tmp        = cfopen (tmpname, "wb+");
	    unlink (tmpname);
#ifdef DEBUG
            if (DEBUG) fprintf (stderr, "sbMaxSize %ld\n", (long) sbMaxSize);
#endif /* DEBUG */
	}
	state = STATE_SORT;
    }

    if (sbCnt >= sbSize) {
	unsigned to = Min (2 * sbSize, sbMaxSize);
	sortBuf = GROW (_edge_t *, sortBuf, sbSize, to);
	sbSize = to;
    }

    if (sbCnt >= ebtCnt * EDGEBUF_SIZE) {
	ecnt_t i;
	_edge_t * eb;
	eb = NEW (_edge_t, EDGEBUF_SIZE);
	edgebuftable[ebtCnt++] = eb;
	for (i = 0; i < Min (EDGEBUF_SIZE, sbSize - sbCnt); i++) {
	    ASSERT (sbCnt + i < sbSize);
	    sortBuf[sbCnt + i] = eb + i;
	}
    }

    e = sortBuf [sbCnt++];

    e -> xl = xl, e -> xr = xr;
    e -> yl = yl, e -> yr = yr;
    e -> slope = slope;
    e -> sign = sign;

#ifdef DRIVER
    if (optPrint) {fprintf (stderr, "sortedge: "); pe (e);}
#endif /* DRIVER */

    if (sbCnt == sbMaxSize) {
	sortBlock (sortBuf, sbCnt);
	writeBlock (sortBuf, sbCnt);
	sbCnt = 0;
    }
}

/* pre: all edges have been entered into the sort module
 *      by calling sortEdge for each of them.
 * repeated calls to fetchEdge will return the edges
 * in sorted order, if there are no more edges,
 * a sentinel edge with xl -> INF is returned.
 */
edge_t * fetchEdge ()
{
    edge_t * e;
    _edge_t * edge;
    btnum_t       block;
    static ecnt_t sortBufIndex= -1000;

    if (state == STATE_INIT)		/* this can happen if there are */
	goto ready;		        /* empty streams */

    if (state != STATE_FETCH) {
	state = STATE_FETCH;
#ifdef DRIVER
	if (optPrint) {
	    btnum_t i;
	    fprintf (stderr, "===== before fetch =====\n");
	    for (i = 0; i < sbCnt; i++) pe (sortBuf[i]);
	    fprintf (stderr, "===== end before fetch =====\n");
	}
#endif /* DRIVER */
	if (btCnt > 0) {
	    if (sbCnt > 0) {
		sortBlock (sortBuf, sbCnt);
		writeBlock (sortBuf, sbCnt);
		sbCnt = 0;
	    }
	    fflush (fp_tmp);

	    infoMaxTempFileSize = Max (infoMaxTempFileSize, ftell (fp_tmp));

	    makeBtBufs ();

	    pqInit (btCnt);
	    for (block = 0; block < btCnt; block++) {
		pqInsert (fetch (block), block);
	    }
	}
	else {
	    if (sbCnt > 0) sortBlock (sortBuf, sbCnt);
	    sortBufIndex = 0;
	}
    }

    if (btCnt > 0) {
	pqHead (&edge, &block);
	e = createEdge (edge -> xl, edge -> yl, edge -> xr, edge -> yr,
			edge -> slope, edge -> sign);
	pqReplaceHead (fetch (block), block);
    }
    else {
	/* There is only one block that was sorted entirely
	 * in core. The merging of blocks and tmp file machinery
	 * is unnecessary now.
	 */
	static _edge_t z;
	if (sortBufIndex < sbCnt)
	    edge = sortBuf [sortBufIndex++];
	else {
ready:
	    edge = &z;
	    edge -> xl = INF;
	}
	e = createEdge (edge -> xl, edge -> yl, edge -> xr, edge -> yr,
			edge -> slope, edge -> sign);
    }

    Debug (printEdge ("fetchEdge", e));

    return (e);
}

Private void writeBlock (_edge_t **base, ecnt_t N)
{
    ecnt_t i;
    static ecnt_t btSize =  0;
    coor_t cursor_x = 0;
    coor_t cursor_y = 0;

    if (btCnt >= btSize) {
	unsigned to = btSize == 0 ? BT_INITIAL_SIZE : 2 * btSize;
	blocktable = GROW (block_t, blocktable, btSize, to);
	btSize = to;
    }

    blocktable[btCnt].start = ftell (fp_tmp);
    blocktable[btCnt].size  = N;

    for (i = 0; i < N; i++) {
	writeEdge (fp_tmp, base[i], &cursor_x, &cursor_y);
    }

    infoMaxTempFileSize = Max (infoMaxTempFileSize, ftell (fp_tmp));

    Debug (fprintf (stderr, "writeBlock: block %ld size %ld\n",
	(long) btCnt, (long) blocktable[btCnt].size));

    btCnt++;
}

/* writeEdge - put an edge in tmp file
 *
 * Write differences instead of absolute coordinates,
 * to save space.
 * Differences are taken accros record boundaries,
 * this works very well because the edges are in scanline order.
 */
Private void writeEdge (FILE *fp, _edge_t *e, coor_t *cursor_x, coor_t *cursor_y)
{
    long xl, xr, yl, yr;

    e -> xr -= e -> xl;
    e -> yr -= e -> yl;

    /* Encode the sign of the edges in the direction
     * of the edge.
     */
    /* Orginally it was ASSERT (e -> xr > 0);
       However, I noticed that e->xr can also be 0 in recent version
       of makemesh.  At first sight, the output seems ok. */
    ASSERT (e -> xr >= 0);
    if (e -> sign == STOP) e -> xr = -(e -> xr);

    e -> xl -= *cursor_x;
    *cursor_x += e -> xl;
    if (e -> xl != 0) *cursor_y = 0; /* to prevent range overflow */
    e -> yl -= *cursor_y;
    *cursor_y += e -> yl;

    xl = e -> xl;
    xr = e -> xr;
    yl = e -> yl;
    yr = e -> yr;
    if (pack4D (fp, xr, yl, xl, yr) != 0) {
	say ("tmp file write error, size %ld bytes", (long) ftell (fp));
	perror (tmpname);
	die ();
    }
}

/* readEdge - get an edge from tmp file.
 *
 * See the comments under writeEdge for the format.
 * This routine performs an 'inverse' transformation
 * to the coordinates.
 */
Private void readEdge (FILE *fp, _edge_t *e, coor_t *cursor_x, coor_t *cursor_y)
{
    long xl, xr, yl, yr;

    if (unpack4D (fp, &xr, &yl, &xl, &yr) == EOF) {
	say ("tmp file read error, size %ld bytes", (long) ftell (fp));
	perror (tmpname);
	die ();
    }
    e -> xl = xl;
    e -> xr = xr;
    e -> yl = yl;
    e -> yr = yr;

    if (e -> xl != 0) *cursor_y = 0;
    e -> xl += *cursor_x;
    e -> yl += *cursor_y;
    *cursor_x = e -> xl;
    *cursor_y = e -> yl;

    if (e -> xr > 0) {
	e -> sign = START;
    }
    else {
	e -> sign = STOP;
	e -> xr = -(e -> xr);
    }

    e -> xr += e -> xl;
    e -> yr += e -> yl;

    if (e -> yr == e -> yl) e -> slope = 0;
    else if (e -> yr - e -> yl == e -> xr - e -> xl) e -> slope = 1;
    else if (e -> yr - e -> yl == e -> xl - e -> xr) e -> slope = -1;
#ifdef MAKEMESH
    else if (e -> xl == e -> xr) {
	if (e -> yr > e -> yl) e -> slope = 2;  /* use 2 for 90 degrees */
	else                   e -> slope = -2; /* use -2 for -90 degrees */
    }
#endif
}

Private void makeBtBufs ()
{
    btnum_t b, b1, b2, bs;

    for (b = 0; b < btCnt && b < ebtCnt; b++) {
	blocktable[b].bufp = edgebuftable[b];
	blocktable[b].bufs = EDGEBUF_SIZE;
	blocktable[b].cnt  = 0;
	blocktable[b].cursor_x = 0;
	blocktable[b].cursor_y = 0;
    }

    while (b < btCnt) {
	for (b1 = 0, b2 = b; b1 < b && b2 < btCnt; b1++, b2++) {
	    bs = blocktable[b1].bufs / 2;
	    blocktable[b1].bufs -= bs;
	    blocktable[b2].bufs  = bs;
	    blocktable[b2].bufp  = blocktable[b1].bufp + blocktable[b1].bufs;
	    blocktable[b2].cnt  = 0;
	    blocktable[b2].cursor_x = 0;
	    blocktable[b2].cursor_y = 0;
	}
	b = b2;
    }
}

Private _edge_t * fetch (btnum_t b)
{
    if (blocktable[b].cnt <= 0)
	fillBuf (b);

    blocktable[b].cnt--;

    return (blocktable[b].p++);
}

Private void fillBuf (btnum_t b)
{
    ecnt_t nedges = Min (blocktable[b].bufs, blocktable[b].size);
    ecnt_t i;

    if (nedges <= 0) {
	nedges = 1;
	blocktable[b].bufp -> xl = INF;
    }
    else {
	fseek (fp_tmp, (long) blocktable[b].start, 0);
	for (i = 0; i < nedges; i++)
	    readEdge (fp_tmp, blocktable[b].bufp + i,
		      &blocktable[b].cursor_x, &blocktable[b].cursor_y);

    }

    blocktable[b].cnt    = nedges;
    blocktable[b].p      = blocktable[b].bufp;
    blocktable[b].start  = ftell (fp_tmp);
    blocktable[b].size  -= nedges;

    Debug (fprintf (stderr, "fillBuf: block %ld cnt %ld size %ld\n",
	(long) b, (long) blocktable[b].cnt, (long) blocktable[b].size));
}

void sortPrintInfo (FILE *fp)
{
    if (fp_tmp) {
	fclose (fp_tmp);
	unlink (tmpname);
    }
    if (!fp) return;
    fprintf (fp, "\n");
    fprintf (fp, "\t# edges sorted               : %ld\n", (long) infoNumEdges);
    fprintf (fp, "\tsize of tempfile for sorting : %ld\n", (long) infoMaxTempFileSize);
    fprintf (fp, "\t# quicksort partitions       : %ld\n", (long) infoNumPartitions);
    fprintf (fp, "\tpartition balance            : %g\n", (double) infoPartitionBalance/infoNumPartitions);
}

#ifdef DRIVER
DM_CELL          * cellKey;
DM_PROJECT       * dmproject;
bool_t optDelete = FALSE;

/* test the sort module.
 * Usage: test cellname maskname [cnt]
 */

size_t optMaxMemory = 1024 * 1024;

int scale = SCALE;

extern char *optarg;
extern int optind;

int main (int argc, char **argv)
{
    int errflg = 0;
    bool_t  optVerify = TRUE;
    char  * argv0 = "test";
    edge_t * e1, * e2;
    int i, c;

    while ((c = getopt (argc, argv, "n:pv")) != EOF) {
	switch (c) {
	    case '?': errflg++;              break;
	    case 'p': optPrint = TRUE; break;
	    case 'v': optVerify = TRUE; break;
	}
    }

    if (errflg)
        printf ("Usage: %s [-n#] [-ps] cell mask [...]\n", argv0), die ();

    dmInit ("test");

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    cellKey = dmCheckOut (dmproject, argv[optind], ACTUAL, DONTCARE, LAYOUT, READONLY);

    dmGetDesignData (dmOpenStream (cellKey, "info3", "r"), GEO_INFO3);

    for (optind = optind + 1; argv[optind]; optind++) {

	fprintf (stderr, "Mask %s\n", argv[optind]);

	readEdges (cellKey, argv[optind], DM_OTHER_MASK, 4);

	i = 1;
	e1 = fetchEdge ();
	e2 = fetchEdge ();
	if (optPrint) pe ((_edge_t *) e1);

	while (e2 -> xl < INF) {
	    if (optPrint) pe ((_edge_t *) e2);
	    if (optVerify && smaller (e2, e1)) {
		fprintf (stderr, "Edges %ld and %ld out of order\n",
		    (long) i-1, (long) i);
		if (!optPrint) break;
	    }
	    i++;
	    disposeEdge (e1);
	    e1 = e2, e2 = fetchEdge ();
	}

	if (e1 -> xl == INF)
	    fprintf (stderr, "Total %ld edges\n", (long) 0);
	else
	    fprintf (stderr, "Total %ld edges\n", (long) i);
    }

    sortPrintInfo (stdout);

    dmQuit ();
    return (0);
}

Private void pe (_edge_t *e)
{
    fprintf (stderr, "xl=%ld yl=%ld xr=%ld yr=%ld slope=%d sign=%d\n",
	(long) e -> xl, (long) e -> yl, (long) e -> xr, (long) e -> yr,
	(int) e -> slope, (int) e -> sign);
}
#endif /* DRIVER */
