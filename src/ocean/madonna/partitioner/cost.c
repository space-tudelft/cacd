/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/part.h"
#include <stdlib.h>

#define BIG    10000
#define SMALL -10000

int *cvrt_to_row; /* used by Row(i) macro */
int *cvrt_to_clm; /* used by Clm(i) macro */

void init_row_clm_transformation_arrays (TOTALPPTR total)
{
    int j, nx, ny, numparts;

    nx = total->nx;
    ny = total->ny;
    numparts = total->numparts;

    if (!(cvrt_to_row = (int*)malloc ((2*numparts + 2) * sizeof(int)))) {
	fprintf (stderr, "cannot allocate memory for transformation_arrays\n");
	dumpcore();
    }

    cvrt_to_clm = cvrt_to_row + numparts + 1;

    for (j = 1; j <= numparts; ++j) {
	cvrt_to_clm[j] = 1 + (j-1) % nx;
	cvrt_to_row[j] = 1 + (j-1) / nx;
    }
}

void initnetstate (NETSTATEPTR netstate, CFUNC *costfunction)
{
    int     *netdistr;
    register int total, hor;
    register int mh;      /* mh == (1+i) % hor */
    register int v, tmpdist, prev;
    register int *clmnext, *clmprev, *rowprev, *rownext;
    int      i, minh = BIG, minv = BIG, maxh = SMALL, maxv = SMALL, nsubarea = 0;
    int     *clm, *row, total_in_row = 0;

    netdistr = netstate->dist;
    total = netstate->numparts;
    hor = netstate->nx;
    row = netstate->row[LIST];   /* the reduntant column list */
    clm = netstate->clm[LIST];   /* the reduntant row list */

    for (mh = 1, v = 1, i = 1; i <= total; ++i)
    {
	if ((tmpdist = netdistr[i]) > 0) {
	    ++nsubarea;
	    ++total_in_row;
	    ++clm[mh];
	    if (v < minv) minv = v;
	    if (v > maxv) maxv = v;
	    if (mh < minh) minh = mh;
	    if (mh > maxh) maxh = mh;
	}
	else if (tmpdist < 0) {
	    fprintf (stderr, "gencost: netdistr[%d] contains negative number: %d\n", i, netdistr[i]);
	    dumpcore();
	}
	if (++mh > hor) {
	    row[v] = total_in_row;
	    total_in_row = 0;
	    ++v;
	    mh = 1;
	}
    }

    clmprev = netstate->clm[PREV];
    clmnext = netstate->clm[NEXT];
    rowprev = netstate->row[PREV];
    rownext = netstate->row[NEXT];

    for (v = minh, prev = 0; v <= maxh; ++v) {
	if (clm[v] == 0) continue;
	clmnext[prev] = v;
	clmprev[v] = prev;
	prev = v;
    }
    for (v = minv, prev = 0; v <= maxv; ++v) {
	if (row[v] == 0) continue;
	rownext[prev] = v;
	rowprev[v] = prev;
	prev = v;
    }
    netstate->xl = minh;
    netstate->xr = maxh;
    netstate->yb = minv;
    netstate->yt = maxv;
    netstate->deviation = nsubarea;
    netstate->cost = (*costfunction[COSTSTATE])(0, netstate);
}

/* Compute cost of a net-distribution. Based on the size of the enclosing
 * rectangle and the number of sub-areas visited within that rectangle.
 */
int netstatecost (int dummy_id /* unused */, NETSTATEPTR ns)
{
    return ((ns->yt - ns->yb + 1) * (ns->xr - ns->xl + 1) + ns->deviation);
}

int netstatecost2 (int dummy_id /* unused */, int xl, int xr, int yb, int yt, int deviation)
{
    return ((yt - yb + 1) * (xr - xl + 1) + deviation);
}

/* return the gain for this netstate if we'd move from from_area to to_area */
int gainnetstate (NETSTATEPTR netstate, int to_area, int from_area, CFUNC *costfunction)
{
    int Nhu, Nhv; /* "netstate"="Nh", "from_area"=="u", "to_area"=="v" */
    int xl, xr, yb, yt, deviation;

    xl = netstate->xl;
    xr = netstate->xr;
    yb = netstate->yb;
    yt = netstate->yt;
    deviation = netstate->deviation;

    Nhu = netstate->dist[from_area] - 1;
    if (Nhu == 0) { /* from_area becomes empty, as far as this net is concerned */
	int from_clm = Clm(from_area), Cumodnx;
	int from_row = Row(from_area), Rvdivnx;
	deviation -= 1;
	/* decrement the column total (# non-zero subareas in this column) */
	Cumodnx = netstate->clm[LIST][from_clm] - 1;
	if (Cumodnx == 0) /* column total becomes zero */
	    keep_track_of_bbx_after_decr_fake (&xl, &xr, from_clm, netstate->clm);
	/* decrement the row total (# non-zero subareas in this row) */
	Rvdivnx = netstate->row[LIST][from_row] - 1;
	if (Rvdivnx == 0) /* row total becomes zero */
	    keep_track_of_bbx_after_decr_fake (&yb, &yt, from_row, netstate->row);
    }

    Nhv = netstate->dist[to_area] + 1;
    if (Nhv == 1) { /* to_area becomes non-empty as far as this net is concerend */
	int to_clm = Clm(to_area), Cvmodnx;
	int to_row = Row(to_area), Rudivnx;
	deviation += 1;
	/* increment the column total */
	Cvmodnx = netstate->clm[LIST][to_clm] + 1;
	if (Cvmodnx == 1) /* column total becomes non-zero */
	    keep_track_of_bbx_after_incr_fake (&xl, &xr, to_clm);
	/* increment the row total */
	Rudivnx = netstate->row[LIST][to_row] + 1;
	if (Rudivnx == 1) /* row total becomes non-zero */
	    keep_track_of_bbx_after_incr_fake (&yb, &yt, to_row);
    }
    return (netstate->cost - (*costfunction[COSTVEC])(0, xl, xr, yb, yt, deviation));
}

/* keep track of xl and xr: is this column from_clm on the outside of the bbx?
 * keep track of yb and yt: is this row from_row on the outside of the bbx?
 * in general:
 * keep track of minor and major: is this bar from_bar on the outside of the bbx?
 */
void keep_track_of_bbx_after_decr (int *minor_, int *major_, int from_bar, int *barlist[])
{
    if (*minor_ == *major_) { /* special case */
	if (from_bar != *minor_) err (5, "gainnetstate: inconsistency");
	*minor_ = *major_ = 0;
    }
    else if (from_bar == (*minor_)) {
	*minor_ = barlist[NEXT][from_bar];
	barlist[PREV][*minor_] = 0;
    }
    else if (from_bar == *major_) {
	*major_ = barlist[PREV][from_bar];
	barlist[NEXT][*major_] = 0;
    }
    else {
	int next = barlist[NEXT][from_bar], prev = barlist[PREV][from_bar];
	barlist[NEXT][prev] = next;
	barlist[PREV][next] = prev;
    }
}

void keep_track_of_bbx_after_decr_fake (int *minor_, int *major_, int from_bar, int *barlist[])
{
    if (*minor_ == *major_) { /* special case */
	if (from_bar != *minor_) err (5, "gainnetstate: inconsistency");
	*minor_ = *major_ = 0;
    }
    else if (from_bar == *minor_) *minor_ = barlist[NEXT][from_bar];
    else if (from_bar == *major_) *major_ = barlist[PREV][from_bar];
}

void keep_track_of_bbx_after_incr (int *minor_, int *major_, int to_bar, int *barlist[])
{
    register int bar, min, maj;

    min = *minor_, maj = *major_;
    if (min == 0 || maj == 0) { /* special case */
	*minor_ = *major_ = to_bar;
	barlist[PREV][to_bar] = 0;
	barlist[NEXT][to_bar] = 0;
    }
    else if (to_bar < min) {
	barlist[NEXT][to_bar] = min;
	barlist[PREV][to_bar] = 0;
	barlist[PREV][min] = to_bar;
	*minor_ = to_bar;
    }
    else if (to_bar > maj) {
	barlist[PREV][to_bar] = maj;
	barlist[NEXT][to_bar] = 0;
	barlist[NEXT][maj] = to_bar;
	*major_ = to_bar;
    }
    else {
	int *nextlist = barlist[NEXT], lower;
	for (bar = min; bar; bar = nextlist[bar])
	    if (to_bar <= bar) break;
	if (bar == 0 || to_bar == bar)
	    err (5, "keep_track_of_bbx_after_incr: internal error 75436");
	lower = barlist[PREV][bar];
	barlist[PREV][to_bar] = lower;
	barlist[NEXT][to_bar] = bar;
	barlist[PREV][bar]   = to_bar;
	barlist[NEXT][lower] = to_bar;
    }
}

void keep_track_of_bbx_after_incr_fake (int *minor_, int *major_, int to_bar)
{
    register int min, maj;

    min = *minor_, maj = *major_;
    if (min == 0 || maj == 0) { /* special case */
	*minor_ = *major_ = to_bar;
    }
    else if (to_bar < min) *minor_ = to_bar;
    else if (to_bar > maj) *major_ = to_bar;
}

NETSTATEPTR domove_and_copy_netstate (NETSTATEPTR oldnetstate,
	int *newstatebuf, int to_area, int from_area, CFUNC* dummy)
{
    int numparts = oldnetstate->numparts;
    register int *z1, *z2, *z3;
    register NETSTATEPTR newnetstate;

    /* copy the state of oldnetstate to newnetstate */
    for (z1 = newstatebuf, z2 = startofstatebuf (oldnetstate),
	z3 = z2 + sizeofstatebuf (oldnetstate); z2 < z3; ++z1, ++z2) *z1 = *z2;
    NewNetstate (newnetstate);
    *newnetstate = *oldnetstate;    /* struct assignment */
    assignstatebuf (newnetstate, newstatebuf);

    if (--newnetstate->dist[from_area] == 0)
    { /* from_area becomes empty, as far as this net is concerned */
	int from_clm = Clm (from_area);
	int from_row = Row (from_area);
	--newnetstate->deviation;
	/* decrement the column total (# non-zero subareas in this column) */
	if (--newnetstate->clm[LIST][from_clm] == 0) /* column total becomes zero */
	    keep_track_of_bbx_after_decr (&newnetstate->xl, &newnetstate->xr, from_clm, newnetstate->clm);
	/* decrement the row total (# non-zero subareas in this row) */
	if (--newnetstate->row[LIST][from_row] == 0) /* row total becomes zero */
	    keep_track_of_bbx_after_decr (&newnetstate->yb, &newnetstate->yt, from_row, newnetstate->row);
    }
    if (++newnetstate->dist[to_area] == 1)
    { /* to_area becomes non-empty as far as this net is concerend */
	int to_clm = Clm (to_area);
	int to_row = Row (to_area);
	++newnetstate->deviation;
	/* increment the column total */
	if (++newnetstate->clm[LIST][to_clm] == 1) /* column total becomes non-zero */
	    keep_track_of_bbx_after_incr (&newnetstate->xl, &newnetstate->xr, to_clm, newnetstate->clm);
	/* increment the row total */
	if (++newnetstate->row[LIST][to_row] == 1) /* row total becomes non-zero */
	    keep_track_of_bbx_after_incr (&newnetstate->yb, &newnetstate->yt, to_row, newnetstate->row);
    }
    newnetstate->cost = netstatecost (0, newnetstate);
    return (newnetstate);
}

int *startofstatebuf (NETSTATEPTR netstate)
{
    return (netstate->dist);
}

int sizeofstatebuf (NETSTATEPTR netstate)
{
    return ((netstate->numparts+1) + 3*(netstate->nx+1) + 3*(netstate->ny+1));
}

int sizeofstatebuf2 (TOTALPPTR total)
{
    return ((total->numparts+1) + 3*(total->nx+1) + 3*(total->ny+1));
}

/* This one distributes the storage in statebuf among the various
 * fields of the netstate structure.
 */
void assignstatebuf (NETSTATEPTR netstate, int *statebuf)
{
    int *columnlist, *rowlist, nx, ny;

    netstate->dist = statebuf;
    nx = netstate->nx + 1;
    ny = netstate->ny + 1;
    columnlist = statebuf + (netstate->numparts + 1);
    netstate->clm[LIST] = columnlist;
    netstate->clm[PREV] = columnlist + nx;
    netstate->clm[NEXT] = columnlist + 2 * nx;
    rowlist = columnlist + 3 * nx;
    netstate->row[LIST] = rowlist;
    netstate->row[PREV] = rowlist + ny;
    netstate->row[NEXT] = rowlist + 2 * ny;
}
