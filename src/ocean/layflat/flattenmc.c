/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
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

#include <stdio.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

extern char *argv0; /* the name of this program */
extern char *FilterLib; /* the name of the lib to be filtered out */

static MTXELM out[6];

static DM_PROJECT *topprojectkey;
static int instcount = 1; /* the instance counter, to prevent multiple names */

static void flatten_mc1 (DM_CELL *dstkey, char *oldcellname, MTXELM *oldmtx,
			DM_PROJECT *projectkey, int level, DM_STREAM *mcfp);
static int is_library_primitive (char *cellname);
static int is_a_child (char *topcellname, char *childcellname, DM_PROJECT *projectkey);

void flatten_mc (char *newcellname, char *oldcellname, DM_PROJECT *projectkey)
{
    MTXELM mtxid[6];
    DM_CELL *dstkey;
    DM_STREAM *fp, *mcfp;
    int rows, columns;

    topprojectkey = projectkey;

    /* Check that newcellname is not equal to oldcellname or any of its children */
    if (is_a_child (oldcellname, newcellname, projectkey)) {
	fprintf (stderr, "\n%s: WATCH OUT -- cell %s appears in the hierarchy of cell %s...",
		argv0, newcellname, oldcellname);
	err (5, "    ...seems wiser not to overwrite this cell, bye!");
    }

    if (!(dstkey = dmCheckOut (projectkey, newcellname, WORKING, DONTCARE, LAYOUT, UPDATE)))
	err (5, "Cannot get model key for new cell");

    /* First, create or truncate these four "primary" streams: */
    if ((fp = dmOpenStream (dstkey, "nor" , "w"))) dmCloseStream (fp, COMPLETE);
    if ((fp = dmOpenStream (dstkey, "term", "w"))) dmCloseStream (fp, COMPLETE);
    if ((fp = dmOpenStream (dstkey, "box" , "w"))) dmCloseStream (fp, COMPLETE);

    /* And now for the real work: traverse oldcellname-tree and copy to dstkey-stream */
    mtxcopy (mtxid, mtxidentity());	  /* set mtxid initially to the identity matrix. */

    /* open mc-key also, and propagate it */
    if (!(mcfp = dmOpenStream (dstkey, "mc", "w"))) err (5, "Cannot open mc");

    /* now call the function that recursively flattens this thing: */
    flatten_mc1 (dstkey, oldcellname, mtxid, projectkey, TOPLEVEL, mcfp);

    /* Actually, we did not copy the term stream, only stored it. First
     * cleanup_terms() assures that every term has a unique name, then
     * output_terms() writes the term stream.
     */
    dmCloseStream (mcfp, COMPLETE);
    cleanup_terms ();
    output_terms (dstkey);

    /* At this point we know the bounding box of the new cell. Write to the database: */
    output_bbx (dstkey);

    dmCheckIn (dstkey, COMPLETE);
}

/* This function recursively flattens the cell OLDCELLNAME */
static void flatten_mc1 (DM_CELL *dstkey, char *oldcellname, MTXELM *oldmtx,
			DM_PROJECT *projectkey, int level, DM_STREAM *mcfp)
{
    DM_PROJECT *pkey;
    DM_CELL  *srckey;
    DM_STREAM *fp;
    MTXELM newbasicmtx[6], newmtx[6];
    char cell_name[DM_MAXNAME+1], *remotename;
    int  no_flatten, rv;
    long obxl, obxr, obyb, obyt;

    if (!(srckey = dmCheckOut (projectkey, oldcellname, ACTUAL, DONTCARE, LAYOUT, READONLY)))
	err (5, "Cannot get model key of your instance");

    if (!(fp = dmOpenStream (srckey, "mc", "r")))
	err (5, "Cannot open mc stream for reading -- that's a pity!");

    while ((rv = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	int nx, ny, dx, dy, thisny, thisdy, i;

	/* filter to exclude cells from certain libraries (-L commandline option) */
	if (FilterLib && is_library_primitive (gmc.cell_name))
	    no_flatten = 1;
	else
	    no_flatten = 0;

	/* Found a model. Compute new matrix in newbasicmtx. */
	mtxcopy (newbasicmtx, mtxchain (oldmtx, gmc.mtx));

	/* if library: just copy the instance */
	if (no_flatten)
	{
	    for (i = 0; i < 6; i++) gmc.mtx[i] = newbasicmtx[i];

	    /* compute new bbx = operation of old bbx over oldmtx */
	    obxl = gmc.bxl; obxr = gmc.bxr;
	    obyb = gmc.byb; obyt = gmc.byt;
	    gmc.bxl = (oldmtx[0] * obxl) + (oldmtx[1] * obyb) + oldmtx[2];
	    gmc.byb = (oldmtx[3] * obxl) + (oldmtx[4] * obyb) + oldmtx[5];
	    gmc.bxr = (oldmtx[0] * obxr) + (oldmtx[1] * obyt) + oldmtx[2];
	    gmc.byt = (oldmtx[3] * obxr) + (oldmtx[4] * obyt) + oldmtx[5];

	    if (gmc.bxl > gmc.bxr) { obxl = gmc.bxl; gmc.bxl = gmc.bxr; gmc.bxr = obxl; } /* swap */
	    if (gmc.byb > gmc.byt) { obyb = gmc.byb; gmc.byb = gmc.byt; gmc.byt = obyb; } /* swap */

	    /* modify the instance name, to prevent clashes */
	    sprintf (gmc.inst_name, "inst_%d", instcount++);

	    if (dmPutDesignData (mcfp, GEO_MC)) fprintf (stderr, "ERROR: cannot put mc\n");
	    continue;
	}

	/* If this gmc calls a local cell then pkey == projectkey and
	 * remotename == cellname. If gmc calls an imported cell then these two
	 * contain pointers to the remote projectkey and the remote cell name:
	 */
	if (!(pkey = dmFindProjKey (gmc.imported, gmc.cell_name, projectkey, &remotename, LAYOUT)))
	    err (4, "Cannot get new project key for instance");

	/* need a local copy of cell_name because gmc will be overwritten very soon */
	strcpy (cell_name, remotename);
	/* for the same reason: need local copies of gmc.{dy,ny} */
	thisdy = gmc.dy; thisny = gmc.ny;

	/* flatten each repeated instance */
	for (dx = gmc.dx, nx = gmc.nx; nx >= 0; --nx)
	for (dy = thisdy, ny = thisny; ny >= 0; --ny)
	{
	    /* compute newmtx for this particular repetition of the instance */
	    mtxaddvec2 (newmtx, newbasicmtx, nx*dx, ny*dy);
	    flatten_mc1 (dstkey, cell_name, newmtx, pkey, level+1, mcfp);
	}
    }
    if (rv == -1) err (4, "Something strange going on with dmGetDesignData(fp,GEO_MC)");
    dmCloseStream (fp, COMPLETE);

    if (!(fp = dmOpenStream (srckey, "info", "r")))
	err (5, "Cannot open info stream for reading -- that's a pity!");
    dmGetDesignData (fp, GEO_INFO);

    out[XL] = ginfo.bxl;
    out[XR] = ginfo.bxr;
    out[YB] = ginfo.byb;
    out[YT] = ginfo.byt;
    update_bbx (out);

    dmCloseStream (fp, COMPLETE);

    put_all_this_stuff (dstkey, srckey, oldmtx, level);
    dmCheckIn (srckey, COMPLETE);
}

/* Check that childcellname is a child or grandchild or whatsoever of topcellname.
 */
static int is_a_child (char *topcellname, char *childcellname, DM_PROJECT *projectkey)
{
    DM_PROJECT *pkey;
    DM_CELL  *srckey;
    DM_STREAM *fp;
    char cell_name[DM_MAXNAME+1], *remotename;
    int isachild = 0, rv;

    if (strcmp (topcellname, childcellname) == 0) return 1;

    if (!(srckey = dmCheckOut (projectkey, topcellname, ACTUAL, DONTCARE, LAYOUT, READONLY)))
	err (5, "Cannot get model key of your instance");

    if (!(fp = dmOpenStream (srckey, "mc", "r")))
	err (5, "Cannot open mc stream for reading -- that's a pity!");

    while ((rv = dmGetDesignData (fp, GEO_MC)) > 0)
    {
	/* If this gmc calls a local cell then pkey == projectkey and
	 * remotename == cellname. If gmc calls an imported cell then these two
	 * contain pointers to the remote projectkey and the remote cell name:
	 */
	if (!(pkey = dmFindProjKey (gmc.imported, gmc.cell_name, projectkey, &remotename, LAYOUT)))
	    err (4, "Cannot get project key for instance");
	/* need a local copy of child name because gmc will be overwritten very soon */
	strcpy (cell_name, remotename);
	if ((isachild = is_a_child (cell_name, childcellname, pkey))) break;
    }
    if (rv == -1) err (4, "Something strange going on with dmGetDesignData(fp,GEO_MC)");
    dmCloseStream (fp, COMPLETE);
    dmCheckIn (srckey, COMPLETE);
    return isachild;
}

/*
 * This routine finds out whether the indicated son-cell is a library primitive or not.
 */
static int is_library_primitive (char *cellname)
{
    IMPCELL **imp_p;

    /* look locally */
    if (_dmExistCell (topprojectkey, cellname, LAYOUT) == 1) return 0; /* it is local */

    /* look remote */
    if (!(imp_p = topprojectkey->impcelllist[_dmValidView (LAYOUT)]))
      if (!(imp_p = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, topprojectkey, LAYOUT)))
	err (5, "cannot get imported celllist (is_library_primitive)");

    /* scan imported cells */
    for (; imp_p && *imp_p; imp_p++)
	if (strcmp (cellname, (*imp_p)->alias) == 0) { /* found */
	    if (strstr ((*imp_p)->dmpath, FilterLib))
		return 1;
	    else
		return 0;
	}

    /* we should never arrive here.. */
    return 0;
}
