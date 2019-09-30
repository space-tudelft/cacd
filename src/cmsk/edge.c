/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
 *	S. de Graaf
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

#include "src/cmsk/extern.h"

void give_edge_struct ()
{
    if (!fr_edge_structs) {
	ALLOC_STRUCT (lastedge, edge);
    }
    else {
	lastedge = fr_edge_structs;
	fr_edge_structs = fr_edge_structs -> list;
    }
    lastedge -> lnext = NULL;
    lastedge -> rnext = NULL;
    lastedge -> uneighbour = NULL;
    lastedge -> dneighbour = NULL;
    lastedge -> list = NULL;
}

void ins_edge (int x, int ybottom, int ytop, int bodybit) /* insert edge */
{
    give_edge_struct ();
    lastedge -> x = x;
    lastedge -> ybottom = ybottom;
    lastedge -> ytop = ytop;
    lastedge -> bodybit = bodybit;

    if (!edgeptr) {
	/*
	** we are at the most rightside element
	*/
	if (mostright)
	    mostright -> rnext = lastedge;
	else
	    mostleft = lastedge;
	lastedge -> lnext = mostright;
	mostright = lastedge;
    }
    else {
	if (!edgeptr -> lnext)
	/* we are at the most left edge-element */
	    mostleft = lastedge;
	else
	    (edgeptr -> lnext) -> rnext = lastedge;
	lastedge -> rnext = edgeptr;
	lastedge -> lnext = edgeptr -> lnext;
	edgeptr -> lnext = lastedge;
    }
    edgeptr = lastedge;
}

void free_edge (struct edge *ptr)
{
    if (!ptr -> lnext)
	mostleft = ptr -> rnext;
    else
	(ptr -> lnext) -> rnext = ptr -> rnext;

    if (!ptr -> rnext)
	mostright = ptr -> lnext;
    else
	(ptr -> rnext) -> lnext = ptr -> lnext;

    ptr -> list = fr_edge_structs;
    fr_edge_structs = ptr;
}

void ins_scan (int ybottom, int ytop, int state) /* insert scan */
{
    if (!fr_scan_structs) {
	ALLOC_STRUCT (lastscan, scan);
    }
    else {
	lastscan = fr_scan_structs;
	fr_scan_structs = fr_scan_structs -> list;
    }

    lastscan -> usrc = NULL;
    lastscan -> dsrc = NULL;
    lastscan -> list = NULL;
    lastscan -> ybottom = ybottom;
    lastscan -> ytop = ytop;
    lastscan -> state = state;

    if (!scanptr) {
	/*
	** we are at the upmost element
	*/
	if (!mostup)
	    mostdown = lastscan;
	else
	    mostup -> unext = lastscan;
	lastscan -> dnext = mostup;
	lastscan -> unext = NULL;
	mostup = lastscan;
    }
    else {
	if (!scanptr -> dnext)
	/* we are at the downmost element */
	    mostdown = lastscan;
	else
	    (scanptr -> dnext) -> unext = lastscan;
	lastscan -> unext = scanptr;
	lastscan -> dnext = scanptr -> dnext;
	scanptr -> dnext = lastscan;
    }
    scanptr = lastscan;
}

void free_scan (struct scan *ptr)
{
    if (!ptr -> dnext)
	mostdown = ptr -> unext;
    else
	ptr -> dnext -> unext = ptr -> unext;

    if (!ptr -> unext)
	mostup = ptr -> dnext;
    else
	ptr -> unext -> dnext = ptr -> dnext;

    ptr -> list = fr_scan_structs;
    fr_scan_structs = ptr;
}

/*
** sort and insert edge
*/
void sort_ins_edge (int x, int ybottom, int ytop, int bodybit)
{
    int helpint;

    if (ybottom >= ytop) {
	if (ybottom == ytop)
	    return;
	else {
	    helpint = ybottom;
	    ybottom = ytop;
	    ytop = helpint;
	    bodybit = -bodybit;
	}
    }

    while (edgeptr && edgeptr -> x >= x)
	edgeptr = edgeptr -> lnext;

    if (!edgeptr)
	edgeptr = mostleft;
    while (edgeptr && edgeptr -> x < x)
	edgeptr = edgeptr -> rnext;
    while (edgeptr && edgeptr -> x == x && edgeptr -> ytop <= ybottom)
	edgeptr = edgeptr -> rnext;

/* now we found the place the next step is to match the edge with already
** existing edges. If there is overlap (that means: the same x-coordinate
** and ytop1>ybottom2 and ytop2>ybottom1) the overlap area must become an
** edge with a bodybit != 1 or -1. If the bodybit becomes zero (that hap-
** pens when a start-edge has overlap with an end-edge) the edge should
** be deleted.
*/
    for (;;) {
	if (!edgeptr || edgeptr -> x != x) {
	    ins_edge (x, ybottom, ytop, bodybit);
	    break;
	}
	else {
	    if (edgeptr -> ybottom > ybottom) {
		if (edgeptr -> ybottom >= ytop) {
		    ins_edge (x, ybottom, ytop, bodybit);
		    edgeptr = edgeptr -> rnext;
		    break;
		}
		else {
		    ins_edge (x, ybottom, edgeptr -> ybottom, bodybit);
		    edgeptr = edgeptr -> rnext;
		    ybottom = edgeptr -> ybottom;
		}
	    }
	    else {
		if (edgeptr -> ybottom == ybottom) {
		    if (edgeptr -> ytop > ytop) {
			ins_edge (x, ybottom, ytop, edgeptr -> bodybit + bodybit);
			if (!edgeptr -> bodybit)
			    free_edge (edgeptr);
			edgeptr = edgeptr -> rnext;
			edgeptr -> ybottom = ytop;
			break;
		    }
		    else {
			edgeptr -> bodybit = edgeptr -> bodybit + bodybit;
			if (edgeptr -> bodybit == 0)
			    free_edge (edgeptr);
			if (edgeptr -> ytop == ytop) {
			    edgeptr = edgeptr -> rnext;
			    break;
			}
			ybottom = edgeptr -> ytop;
			edgeptr = edgeptr -> rnext;
		    }
		}
		else {
		    ins_edge (x, edgeptr -> ybottom, ybottom, edgeptr -> bodybit);
		    edgeptr = edgeptr -> rnext;
		    edgeptr -> ybottom = ybottom;
		}
	    }
	}
    }
}

void scan_edges (int flag)
{
    scanflag = flag;
/*
** we scan from left to right and take edges one by one
*/
    for (edgeptr = mostleft; edgeptr; edgeptr = edgeptr -> rnext) {
	while (scanptr && scanptr -> ytop > edgeptr -> ybottom)
	    scanptr = scanptr -> dnext;
	if (!scanptr)
	    scanptr = mostdown;
	while (scanptr && scanptr -> ytop <= edgeptr -> ybottom)
	    scanptr = scanptr -> unext;

	/*
	** the scanptr now stands at the lowest scan-element involved
	*/
	for (;;) {
	    if (!scanptr) {
		ins_scan (edgeptr -> ybottom,
		    edgeptr -> ytop, edgeptr -> bodybit);

		if (mk_start_edge ()) {
		    edgeptr -> bodybit = RIGHT;
		    ln_start_edge ();
		}
		else
		    free_edge (edgeptr);
		break;
	    }
	    else {
		if (scanptr -> ybottom > edgeptr -> ybottom) {
		    if (scanptr -> ybottom >= edgeptr -> ytop) {
			ins_scan (edgeptr -> ybottom, edgeptr -> ytop,
				edgeptr -> bodybit);
			if (mk_start_edge ()) {
			    edgeptr -> bodybit = RIGHT;
			    ln_start_edge ();
			}
			else
			    free_edge (edgeptr);
			comp_scans ();
			scanptr = scanptr -> unext;
			break;
		    }
		    else {
			ins_scan (edgeptr -> ybottom, scanptr -> ybottom,
				edgeptr -> bodybit);
			if (mk_start_edge ()) {
			    ins_edge (edgeptr -> x, scanptr -> ybottom,
				    scanptr -> ytop, RIGHT);
			    ln_start_edge ();
			    edgeptr = edgeptr -> rnext;
			}
			comp_scans ();
			scanptr = scanptr -> unext;
			edgeptr -> ybottom = scanptr -> ybottom;
		    }
		}
		else {
		    if (scanptr -> ybottom == edgeptr -> ybottom) {
			if (scanptr -> ytop > edgeptr -> ytop) {
			    ins_scan (edgeptr -> ybottom, edgeptr -> ytop,
				    scanptr -> state + edgeptr -> bodybit);
			    scanptr -> dsrc = scanptr -> unext -> dsrc;
			    if (mk_stop_edge ()) {
				edgeptr -> bodybit = LEFT;
				ln_stop_edge ();
			    }
			    else {
				if (mk_start_edge ()) {
				    edgeptr -> bodybit = RIGHT;
				    ln_start_edge ();
				}
				else
				    free_edge (edgeptr);
			    }
			    comp_scans ();
			    scanptr = scanptr -> unext;
			    scanptr -> ybottom = edgeptr -> ytop;
			    break;
			}
			else {
			    scanptr -> state = scanptr -> state + edgeptr -> bodybit;
			    if (scanptr -> ytop == edgeptr -> ytop) {
				if (mk_stop_edge ()) {
				    edgeptr -> bodybit = LEFT;
				    ln_stop_edge ();
				}
				else {
				    if (mk_start_edge ()) {
					edgeptr -> bodybit = RIGHT;
					ln_start_edge ();
				    }
				    else
					free_edge (edgeptr);
				}
				comp_scans ();
				scanptr = scanptr -> unext;
				break;
			    }
			    else {
				if (mk_stop_edge ()) {
				    ins_edge (edgeptr -> x, scanptr -> ybottom,
					    scanptr -> ytop, LEFT);
				    ln_stop_edge ();
				    edgeptr = edgeptr -> rnext;
				}
				else {
				    if (mk_start_edge ()) {
					ins_edge (edgeptr -> x, scanptr -> ybottom,
						scanptr -> ytop, RIGHT);
					ln_start_edge ();
					edgeptr = edgeptr -> rnext;
				    }
				}
				comp_scans ();
				edgeptr -> ybottom = scanptr -> ytop;
				scanptr = scanptr -> unext;
			    }
			}
		    }
		    else {
			ins_scan (scanptr -> ybottom,
				edgeptr -> ybottom, scanptr -> state);
			scanptr -> dsrc = scanptr -> unext -> dsrc;
			scanptr = scanptr -> unext;
			scanptr -> ybottom = edgeptr -> ybottom;
		    }
		}
	    }
	}
	comp_scans ();
    }
}

void ln_start_edge () /* link start edge */
{
    if (edgeptr -> lnext && edgeptr -> lnext -> x == edgeptr -> x &&
	    edgeptr -> lnext -> ytop == edgeptr -> ybottom &&
	    edgeptr -> lnext -> bodybit == RIGHT) {
	edgeptr -> lnext -> ytop = edgeptr -> ytop;
	free_edge (edgeptr);
	edgeptr = edgeptr -> lnext;
    }
    else {
	if (scanptr -> dnext && scanptr -> dnext -> ytop >= scanptr -> ybottom
		&& src_edge_exists (scanptr -> dnext)) {
	    edgeptr -> dneighbour = scanptr -> dnext -> usrc;
	    if (edgeptr -> dneighbour -> bodybit == RIGHT)
		edgeptr -> dneighbour -> uneighbour = edgeptr;
	    else
		edgeptr -> dneighbour -> dneighbour = edgeptr;
	}
	else
	    scanptr -> dsrc = edgeptr;
    }
    if (scanptr -> unext && scanptr -> unext -> ybottom <= scanptr -> ytop
	    && src_edge_exists (scanptr -> unext)) {
	edgeptr -> uneighbour = scanptr -> unext -> dsrc;
	if (edgeptr -> uneighbour -> bodybit == RIGHT)
	    edgeptr -> uneighbour -> dneighbour = edgeptr;
	else
	    edgeptr -> uneighbour -> uneighbour = edgeptr;
    }
    else
	scanptr -> usrc = edgeptr;
}

void ln_stop_edge () /* link stop edge */
{
    if (edgeptr -> lnext && edgeptr -> lnext -> x == edgeptr -> x &&
	    edgeptr -> lnext -> ytop == edgeptr -> ybottom &&
	    edgeptr -> lnext -> bodybit == LEFT) {
	edgeptr -> lnext -> ytop = edgeptr -> ytop;
	free_edge (edgeptr);
	edgeptr = edgeptr -> lnext;
    }
    else {
	if (scanptr -> dnext && scanptr -> dnext -> ytop >= scanptr -> ybottom
		&& src_edge_exists (scanptr -> dnext))
	    scanptr -> dnext -> usrc = edgeptr;
	else {
	    edgeptr -> dneighbour = scanptr -> dsrc;
	    if (edgeptr -> dneighbour -> bodybit == LEFT)
		edgeptr -> dneighbour -> uneighbour = edgeptr;
	    else
		edgeptr -> dneighbour -> dneighbour = edgeptr;
	}
    }
    if (scanptr -> unext && scanptr -> unext -> ybottom <= scanptr -> ytop
	    && src_edge_exists (scanptr -> unext))
	scanptr -> unext -> dsrc = edgeptr;
    else {
	edgeptr -> uneighbour = scanptr -> usrc;
	if (edgeptr -> uneighbour -> bodybit == LEFT)
	    edgeptr -> uneighbour -> dneighbour = edgeptr;
	else
	    edgeptr -> uneighbour -> uneighbour = edgeptr;
    }
}

int mk_start_edge () /* make start edge */
{
    switch (scanflag) {
    case -1:
	if (scanptr -> state - edgeptr -> bodybit == 0) return (1);
	break;
    case 1:
	if (scanptr -> state > 0
	 && scanptr -> state - edgeptr -> bodybit < 1) return (1);
	break;
    case 2:
	if (scanptr -> state > 1
	 && scanptr -> state - edgeptr -> bodybit < 2) return (1);
    }
    return (0);
}

int mk_stop_edge () /* make stop edge */
{
    switch (scanflag) {
    case -1:
	if (scanptr -> state == 0) return (1);
	break;
    case 1:
	if (scanptr -> state < 1
	 && scanptr -> state - edgeptr -> bodybit > 0) return (1);
	break;
    case 2:
	if (scanptr -> state < 2
	 && scanptr -> state - edgeptr -> bodybit > 1) return (1);
    }
    return (0);
}

int src_edge_exists (struct scan *ptr)
{
    switch (scanflag) {
    case -1: /* there is a scanptr so there is a source-edge */
	return (1);
    case 1:
	if (ptr -> state > 0) return (1);
	break;
    case 2:
	if (ptr -> state > 1) return (1);
    }
    return (0);
}

void comp_scans () /* compres scans */
{
    if (scanptr && scanptr -> state == 0) {
	free_scan (scanptr);
	return;
    }
    if (scanptr && scanptr -> dnext &&
	    scanptr -> dnext -> ytop >= scanptr -> ybottom &&
	    scanptr -> dnext -> state == scanptr -> state) {
	/* we found two scans that can be taken together */
	scanptr -> ybottom = scanptr -> dnext -> ybottom;
	scanptr -> dsrc = scanptr -> dnext -> dsrc;
	free_scan (scanptr -> dnext);
    }
}

void grow (int delta)
{
    if (delta == 0) return;

    for (edgeptr = mostleft; edgeptr; edgeptr = edgeptr -> rnext) {
	if (edgeptr -> bodybit == RIGHT) {
	    if (edgeptr -> uneighbour -> x > edgeptr -> x)
		edgeptr -> ytop += delta;
	    else
		edgeptr -> ytop -= delta;
	    if (edgeptr -> dneighbour -> x > edgeptr -> x)
		edgeptr -> ybottom -= delta;
	    else
		edgeptr -> ybottom += delta;
	}
	else {
	    if (edgeptr -> uneighbour -> x > edgeptr -> x)
		edgeptr -> ytop -= delta;
	    else
		edgeptr -> ytop += delta;
	    if (edgeptr -> dneighbour -> x > edgeptr -> x)
		edgeptr -> ybottom += delta;
	    else
		edgeptr -> ybottom -= delta;
	}
    }
/*
** all edges now have been grown in y-direction. The next step to take
** is to change the x-direction and relocate the edge. When relocating
** the passing of two negative (ybottom > ytop) edges of each other
** must be detected so the overlap can be burned away.
*/
    growptr = mostright;
    mostleft = mostright = edgeptr = NULL;

    for (; growptr; growptr = growptr -> lnext) {
	if (growptr -> bodybit == RIGHT)
	    growptr -> x -= delta;
	else
	    growptr -> x += delta;

	if (growptr -> ybottom > growptr -> ytop) {
	    for (edgeptr = mostleft;; edgeptr = edgeptr -> rnext) {
		if (!edgeptr || edgeptr -> x > growptr -> x) {
		    ins_edge (growptr -> x, growptr -> ybottom,
			growptr -> ytop, growptr -> bodybit);
		    break;
		}

		if (edgeptr -> ybottom > edgeptr -> ytop &&
			edgeptr -> ybottom > growptr -> ytop &&
			edgeptr -> ytop < growptr -> ybottom) {
		    if (edgeptr -> ytop > growptr -> ytop) {
			give_edge_struct ();
			lastedge -> ytop = growptr -> ytop;
			lastedge -> ybottom = edgeptr -> ytop;
			lastedge -> x = growptr -> x + growptr -> bodybit * delta;
			lastedge -> bodybit = growptr -> bodybit;
			lastedge -> lnext = growptr -> lnext;
			growptr -> lnext = lastedge;
			growptr -> ytop = edgeptr -> ytop;
		    }
		    else {
			if (edgeptr -> ytop < growptr -> ytop) {
			    ins_edge (edgeptr -> x, growptr -> ytop,
				edgeptr -> ytop, edgeptr -> bodybit);
			    edgeptr = edgeptr -> rnext;
			    edgeptr -> ytop = growptr -> ytop;
			}
		    }

		    if (edgeptr -> ybottom < growptr -> ybottom) {
			free_edge (edgeptr);
			growptr -> ytop = edgeptr -> ybottom;
		    }
		    else {
			if (edgeptr -> ybottom > growptr -> ybottom)
			    edgeptr -> ytop = growptr -> ybottom;
			else
			    free_edge (edgeptr);
			break;
		    }
		}
		else {
		    if (edgeptr -> x == growptr -> x &&
			    edgeptr -> ybottom >= growptr -> ytop) {
			ins_edge (growptr -> x, growptr -> ybottom,
			    growptr -> ytop, growptr -> bodybit);
			break;
		    }
		}
	    }
	}
	else {
	    edgeptr = mostleft;
	    while (edgeptr && edgeptr -> x < growptr -> x)
		edgeptr = edgeptr -> rnext;
	    while (edgeptr && edgeptr -> x == growptr -> x &&
		    edgeptr -> ybottom < growptr -> ybottom &&
		    edgeptr -> ybottom < growptr -> ytop)
		edgeptr = edgeptr -> rnext;
	    ins_edge (growptr -> x, growptr -> ybottom, growptr -> ytop,
		    growptr -> bodybit);
	}
	growptr -> list = fr_edge_structs;
	fr_edge_structs = growptr;
    }
/*
** Now the negative edges that pass another negative edge are
** updated.  Next step is to look for overlap etcetera.
** This can be done with sort_ins_edge().
** This procedure also makes normal edges of negative ones.
*/
    growptr = mostright;
    edgeptr = mostleft = mostright = NULL;
    for (; growptr; growptr = growptr -> lnext) {
	sort_ins_edge (growptr -> x, growptr -> ybottom,
	    growptr -> ytop, growptr -> bodybit);
	growptr -> list = fr_edge_structs;
	fr_edge_structs = growptr;
    }

/*
** after the re_sort has finished neighbours etcetera are again made
*/
    scan_edges (1);
}

void mk_boxes_of_edges () /* make boxes of edges */
{
    for (edgeptr = mostleft; edgeptr; edgeptr = edgeptr -> rnext) {
	while (scanptr && scanptr -> ytop > edgeptr -> ybottom)
	    scanptr = scanptr -> dnext;
	if (!scanptr)
	    scanptr = mostdown;
	while (scanptr && scanptr -> ytop <= edgeptr -> ybottom)
	    scanptr = scanptr -> unext;
    /*
    ** we are at the right scan-element. because the edge never has
    ** partial overlap (they are reduced to minimum edges) we only
    ** have to look at the one scan-element involved.
    */
	if (!scanptr) {
	    ins_scan (edgeptr -> ybottom, edgeptr -> ytop, edgeptr -> x);
	    comp_pr_scans ();
	}
	else {
	    if (scanptr -> ybottom >= edgeptr -> ytop) {
		ins_scan (edgeptr -> ybottom, edgeptr -> ytop, edgeptr -> x);
		comp_pr_scans ();
	    }
	    else {
		if (scanptr -> ybottom < edgeptr -> ybottom) {
		    ins_scan (scanptr -> ybottom, edgeptr -> ybottom,
			    scanptr -> state);
		    scanptr = scanptr -> unext;
		    scanptr -> ybottom = edgeptr -> ybottom;
		}
		if (scanptr -> state < edgeptr -> x)
		    pr_box (scanptr -> state, edgeptr -> x, edgeptr -> ybottom,
			    edgeptr -> ytop);
		if (scanptr -> ytop == edgeptr -> ytop) {
		    free_scan (scanptr);
		    scanptr = scanptr -> unext;
		}
		else
		    scanptr -> ybottom = edgeptr -> ytop;
	    }
	}
    }
}

void comp_pr_scans () /* compres and print scans */
{
    if (scanptr -> dnext && scanptr -> dnext -> ytop == scanptr -> ybottom) {
	if (scanptr -> dnext -> state != scanptr -> state)
	    pr_box (scanptr -> dnext -> state, edgeptr -> x,
		    scanptr -> dnext -> ybottom, scanptr -> dnext -> ytop);
	scanptr -> ybottom = scanptr -> dnext -> ybottom;
	free_scan (scanptr -> dnext);
    }

    if (scanptr -> unext && scanptr -> unext -> ybottom == scanptr -> ytop) {
	pr_box (scanptr -> unext -> state, edgeptr -> x,
		scanptr -> unext -> ybottom, scanptr -> unext -> ytop);
	scanptr -> ytop = scanptr -> unext -> ytop;
	free_scan (scanptr -> unext);
    }
}

void pr_box (int xbottom, int xtop, int ybottom, int ytop)
{
    if (mirrorflag) {
	if (mirrorflag == 1) {
	    temp1 = ybottom;
	    ybottom = -ytop;
	    ytop = -temp1;
	}
	else {
	    temp1 = xbottom;
	    xbottom = -xtop;
	    xtop = -temp1;
	}
    }

    switch (rotateflag) {
	case 0:
	    break;
	case 90:
	    temp1 = xbottom;
	    temp2 = xtop;
	    xbottom = -ytop;
	    xtop = -ybottom;
	    ybottom = temp1;
	    ytop = temp2;
	    break;
	case 180:
	    temp1 = xbottom;
	    temp2 = ybottom;
	    xbottom = -xtop;
	    xtop = -temp1;
	    ybottom = -ytop;
	    ytop = -temp2;
	    break;
	case 270:
	    temp1 = xbottom;
	    temp2 = xtop;
	    xbottom = ybottom;
	    xtop = ytop;
	    ybottom = -temp2;
	    ytop = -temp1;
	    break;
    }

    xbottom += x_origin;
    xtop    += x_origin;
    ybottom += y_origin;
    ytop    += y_origin;

    progbox[0] = xbottom;
    progbox[1] = xtop;
    progbox[2] = ybottom;
    progbox[3] = ytop;

    if (cxnumber || cynumber) {
	gbox.dx = gbox.dy = 0;
	if (cxnumber) {
	    gbox.dx = cxdistance;
	    temp1 = cxnumber * gbox.dx;
	    if (cxdistance < 0)
		progbox[0] += temp1;
	    else
		progbox[1] += temp1;
	}
	if (cynumber) {
	    gbox.dy = cydistance;
	    temp1 = cynumber * gbox.dy;
	    if (cydistance < 0)
		progbox[2] += temp1;
	    else
		progbox[3] += temp1;
	}
	gbox.bxl = progbox[0];
	gbox.bxr = progbox[1];
	gbox.byb = progbox[2];
	gbox.byt = progbox[3];
    }

    gbox.layer_no = masknr;
    gbox.xl = xbottom;
    gbox.xr = xtop;
    gbox.yb = ybottom;
    gbox.yt = ytop;
    gbox.nx = cxnumber;
    gbox.ny = cynumber;
    dmPutDesignData (fpbox, GEO_BOX);

    if (progpresent) {
	cmp_fillbb (progbox, totprogbox, totprogbox);
    }
    else {
	++progpresent; /* true */
	fillbb (progbox, totprogbox);
    }
}

void pr_boxes_from_edges ()
{
    register int i;

    scan_edges (-1);

    if (mostdown) {
	pr_err1 ("scan leaves a residue, figure is not complete");
	for (scanptr = mostdown; scanptr; scanptr = scanptr -> unext)
	    free_scan (scanptr);
	killpattern ();
    }
    else {
	for (i = 0; i < nroflistmasks; ++i) {
	    if (mlist[i][1]) grow (-mlist[i][1]);
	    masknr = mlist[i][0];
	    mk_boxes_of_edges ();
	}
    }

    for (edgeptr = mostleft; edgeptr; edgeptr = edgeptr -> rnext)
	free_edge (edgeptr);
}
