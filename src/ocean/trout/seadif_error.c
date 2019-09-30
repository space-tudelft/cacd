/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
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

/*
 * maintain seadif error cells
 */

#include "src/ocean/trout/typedef.h"
#include "src/ocean/trout/grid.h"
#include "src/ocean/libseadif/sealibio.h"
#include <time.h>

#define DEFAULT_PROGRAM      "trout"
#define DEFAULT_AUTHOR       "me"
#define DEFAULT_TECHNOLOGY   "default technology"

#define DEFAULT_TYPE         "no type"

extern LAYOUTPTR   thislay;
extern CIRCUITPTR  thiscir;
extern FUNCTIONPTR thisfun;
extern LIBRARYPTR  thislib;

static LAYOUTPTR unconnect; /* pointer to instance which is placed on an unconnect */
static LAYOUTPTR find_sdf_layout_cell (int what, char *layout, char *circuit, char *function, char *library);

/*
 * This routine initializes the unconnect system
 */
void init_unconnect (LAYOUTPTR lay)
{
    char *unconn;

    /* name of unconnected cell */
    unconn = cs ("Error_Marker");
    unconnect = NULL;

    /* does some unconnect cell exist? */
    if (sdfaliastoseadif (unconn, ALIASLAY) == TRUE)
    {
	if (existslay (thislaynam, thiscirnam, thisfunnam, thislibnam) &&
	    sdfreadlay (SDFLAYSTAT, thislaynam, thiscirnam, thisfunnam, thislibnam))
	{
	    unconnect = thislay;
	    return;
	}
    }

    /* add underscore to make different */
    unconn = cs ("Error_Marker_");

    /* does not exists: make it */
    unconnect = find_sdf_layout_cell (SDFLAYBODY, unconn, unconn, unconn, lay->circuit->function->library->name);
    /*
    * fill unconnect
    */
    unconnect->off[X] = unconnect->off[Y] = 0;
    unconnect->bbx[X] = unconnect->bbx[Y] = 0;
    NewWire (unconnect->wire);
    unconnect->wire->crd[L] = unconnect->wire->crd[R] = 0;
    unconnect->wire->crd[B] = unconnect->wire->crd[T] = 0;
    unconnect->wire->layer = 200;   /* dummy layer no */

    sdfwritelay (SDFLAYALL, unconnect);
}

/*
 * This routine adds an 'unconnect error' with name 'name'
 * atthe specified position
 */
void add_error_unconnect (NETPTR net, GRIDADRESSUNIT x, GRIDADRESSUNIT y, GRIDADRESSUNIT z)
/* x, y, z - point */
{
    if (unconnect)
	add_unconnect (((R_CELLPTR) net->circuit->flag.p)->layout, net->name, x, y, z);
}

void add_unconnect (LAYOUTPTR lay, char *name, GRIDADRESSUNIT x, GRIDADRESSUNIT y, GRIDADRESSUNIT z)
/* x, y, z - point */
{
    register LAYINSTPTR linst;
    SLICEPTR slice;
    LAYOUTPTR errorcell;
    R_CELLPTR rcell;
    int numinst;
    char errname[128];

    /* just add errors to layout */
    if (!unconnect) return;

    if ((rcell = (R_CELLPTR) lay->flag.p) == NULL) return;

    /* just add errors to layout */
    errorcell = lay;

    if (x < rcell->cell_bbx.crd[L] ||
	x > rcell->cell_bbx.crd[R] ||
	y < rcell->cell_bbx.crd[B] ||
	y > rcell->cell_bbx.crd[T])
    {
	// fprintf (stderr, "WARNING (add_unconnect): point outside bbx\n");
	return;
    }

    /* instance already there? */
    for (slice = errorcell->slice; slice && slice->chld_type == SLICE_CHLD; slice = slice->chld.slice);

    if (slice == NULL || slice->chld_type != LAYINST_CHLD) {
	error (ERROR, "didn't find a layout instance\n");
	return;
    }

    numinst = 1;
    for (linst = slice->chld.layinst; linst; linst = linst->next)
    {
	if (linst->layout == unconnect && linst->mtx[2] == x && linst->mtx[5] == y) break;
	if (strncmp (name, linst->name, (size_t) strlen (name)) == 0) numinst++;
    }

    if (linst && strncmp (name, linst->name, (size_t) strlen (name)) == 0) return; /* already there */

    /*
    * make instance of unconnect
    */
    NewLayinst (linst);
    /* name sure that name is unique: add number... */
    /* disabled, too problemnatic.. */
    /* sprintf (errname, "%s%d", name, numinst); */
    sprintf (errname, "%s", name);
    linst->name = canonicstring (errname);
    linst->layout = unconnect;
    linst->mtx[0] = 1; linst->mtx[1] = 0; linst->mtx[2] = x;
    linst->mtx[3] = 0; linst->mtx[4] = 1; linst->mtx[5] = y;
    linst->next = slice->chld.layinst;
    slice->chld.layinst = linst;
}

/*
 * This routine creates/finds an seadif cell.
 * NOLY cell fun cir lay (if it does not yet exist)
 */
static LAYOUTPTR find_sdf_layout_cell (int what, char *layout, char *circuit, char *function, char *library)
/* what - what is to be read */
/* other args - names (canonicstringed) */
{
    LIBRARYPTR lib;
    register FUNCTIONPTR func;
    register CIRCUITPTR cir;
    register LAYOUTPTR lay;

    /*
     * look in lib for cell
     */
    if (existslib (library))
    {
	if (!sdfreadlib (SDFLIBSTAT, library))
	    error (FATAL_ERROR, "WARNING (find_sdf_layout_cell): lib is there, but cannot read.");
	lib = thislib;
    }
    else
    { /* not found */
	NewLibrary (lib);
	lib->name = canonicstring (library);
	lib->technology = canonicstring (DEFAULT_TECHNOLOGY);
	/* no functions */
	NewStatus (lib->status);
	lib->status->timestamp = time(0);
	lib->status->program = canonicstring (DEFAULT_PROGRAM);
	lib->status->author = canonicstring (DEFAULT_AUTHOR);
	sdfwritelib (SDFLIBSTAT, lib);
    }

    if (existsfun (function, library))
    {
	if (!sdfreadfun (SDFFUNSTAT, function, library))
	    error (FATAL_ERROR, "WARNING (find_sdf_layout_cell): func is there, but cannot read.");
	func = thisfun;
    }
    else
    { /* not found */
	NewFunction (func);
	func->name = canonicstring (function);
	func->type = canonicstring (DEFAULT_TYPE);
	/* no circuits yet */
	NewStatus (func->status);
	func->status->timestamp = time(0);
	func->status->program = canonicstring (DEFAULT_PROGRAM);
	func->status->author = canonicstring (DEFAULT_AUTHOR);
	func->library = lib;
	func->next = lib->function;
	lib->function = func;
	sdfwritefun (SDFFUNSTAT, func);
    }

    /*
     * look for this circuit
     */
    if (existscir (circuit, function, library))
    {
	if (!sdfreadcir (SDFCIRSTAT, circuit, function, library))
	    error (FATAL_ERROR, "WARNING (find_sdf_layout_cell): cir is there, but cannot read.");
	cir = thiscir;
    }
    else
    { /* empty */
	NewCircuit (cir);
	cir->name = canonicstring (circuit);
	/* no lists yet */
	NewStatus (cir->status);
	cir->status->timestamp = time(0);
	cir->status->program = canonicstring (DEFAULT_PROGRAM);
	cir->status->author = canonicstring (DEFAULT_AUTHOR);
	cir->function = func;
	cir->next = func->circuit;
	func->circuit = cir;
	sdfwritecir (SDFCIRSTAT, cir);
    }

    if (existslay (layout, circuit, function, library))
    {
	if (!sdfreadlay (what, layout, circuit, function, library))
	    error (FATAL_ERROR, "WARNING (find_sdf_layout_cell): lay is there, but cannot read.");
	lay = thislay;
    }
    else
    {
	NewLayout (lay);
	lay->name = canonicstring (layout);
	NewStatus (lay->status);
	lay->status->timestamp = time(0);
	lay->status->program = canonicstring (DEFAULT_PROGRAM);
	lay->status->author = canonicstring (DEFAULT_AUTHOR);
	lay->circuit = cir;
	lay->next = cir->layout;
	cir->layout = lay;
	sdfwritelay (SDFLAYSTAT, lay);
    }

    return (lay);
}
