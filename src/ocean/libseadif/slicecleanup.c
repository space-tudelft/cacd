/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
/*
 * The slicing tree decribing the layout instances in the
 * seadif structure contains the structure SLICE in the
 * nodes and the structure LAYINST in the leaves. The
 * function slicecleanup() restructures a sliceing tree in
 * such a way that a minimal tree results.
 */
#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/sea_decl.h"

/* Remove the surplus of SLICE structures,
 * see also _LayInstList in seadif.y
 */
void slicecleanup (SLICEPTR *slice)
{
    SLICEPTR prntslice, chldslice, leftchld;

    if (!slice) return;

    prntslice = *slice;
    chldslice = prntslice->chld.slice;

    while (prntslice->chld_type == SLICE_CHLD && !chldslice->next)
    {
	/* Get rid of slice */
	chldslice->next = prntslice->next;
	FreeSlice (prntslice);
	prntslice = chldslice;
	chldslice = prntslice->chld.slice;
    }

    if (prntslice->chld_type == SLICE_CHLD)
    for (leftchld = NULL; chldslice; chldslice = chldslice->next)
    {
	slicecleanup (&chldslice);   /* Recursive call */
	if (!leftchld)
	    prntslice->chld.slice = chldslice;
	else
	    leftchld->next = chldslice;
	leftchld = chldslice;
    }

    if (prntslice->chld_type == SLICE_CHLD)
    {
	chldslice = prntslice->chld.slice;

	do {
	    for (; chldslice; chldslice = chldslice->next)
		if (chldslice->chld_type == LAYINST_CHLD && chldslice->ordination == CHAOS)
		    break;
	    if (!chldslice) break;
	    /* chldslice points to the start of a run that can be collected. */
	    collectlayinstances (chldslice);
	    chldslice = chldslice->next;
	}
	while (1); /* end of do-while loop */

	if ((chldslice = prntslice->chld.slice)->chld_type == LAYINST_CHLD &&
		chldslice->ordination == CHAOS && !chldslice->next)
	/* We collected all layinstances into a single child, hence we can remove
	* this child and move the list of layinstances to the parent.
	*/
	{
	    prntslice->chld_type = LAYINST_CHLD; /* change child_type of parent */
	    prntslice->chld.layinst = chldslice->chld.layinst; /* Move instance list to parent */
	    FreeSlice (chldslice);   /* Dispose this one */
	}
    }

    *slice = prntslice;
}

void collectlayinstances (SLICEPTR slice)
{
    SLICEPTR   nextslice;
    LAYINSTPTR layinst;

    layinst = slice->chld.layinst;    /* Cannot be NULL */
    while ((nextslice = slice->next))
	if (nextslice->chld_type == LAYINST_CHLD && nextslice->ordination == CHAOS)
	{
	    /* Unlink nextslice */
	    for (; layinst->next; layinst = layinst->next) ;
	    layinst->next = nextslice->chld.layinst; /* Join two layinstance lists */
	    slice->next = nextslice->next;
	    FreeSlice (nextslice);
	}
	else
	    break;
}
