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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#ifdef PLOT_CIR_MODE
#include "src/space/auxil/plot.h"
#endif
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/lump/define.h"
#include "src/space/lump/export.h"
#include "src/space/substr/export.h"
#include "src/space/bipolar/define.h"

contRef_t *selResTiles, *selResTilesLast;
extern int hasBipoELEM;
int hasBipoElem = 0;
extern unsigned int nodePointSize;

int msgDoubleJuncCaps;
int condWarnCnt;
int tileCnt;
int tileConCnt;   /* number of tiles with at least one conductor */
int doTileXY;

char **prickName;
int   *prickUsed;
int prickNameCnt;
static int prickNameNew;

tileBoundary_t boundary, *bdr;

/* The following is used by the supply short message
   to print the position where things are connected. */
int joiningCon = -1;
coor_t joiningX;
coor_t joiningY;

bool_t mergeNeighborSubContacts = 1;
bool_t *sep_on_res = NULL; /* Specifies whether derived substrate terminals
	must be separate when resistances are extracted for this conductor. */

typedef struct xyInfo {
    coor_t x, y;
    int cx;
    char *name;
    struct xyInfo *prev;
    struct xyInfo *next;
} xyInfo_t;

static xyInfo_t *xyListBegin;
static xyInfo_t *xyListPos;

int nrOfXYItems;

static int nextX;
static int nextY;
static int nextCon;
static char *nextName;
int *helpArray;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void readXYFile (FILE *fp, int type);
Private void freeXYItem (void);
Private void getXYItem  (void);
Private void skipXYItem (void);
Private void sortXYItems (void);
Private int compareXY (const void * e1, const void * e2);
#ifdef __cplusplus
  }
#endif

void initExtract (DM_CELL *circuitKey, DM_CELL *layoutKey)
{
    struct stat sbuf;
    DM_STREAM *dmsTorname;
    FILE *fpPrick;
    char buf[128];
    int i;

    bdr = &boundary;

    if (!helpArray) helpArray = NEW (int, nrOfCondStd);

    if (!nodePointSize) {
	nodePointSize = sizeof (nodePoint_t)
		+ nrOfCondStd * sizeof (subnode_t *)
		+ nrOfCondStd * sizeof (subnode_t);
    }

    if ((optSubResSave || optSubRes) && !sep_on_res) {
	sep_on_res = NEW (int, nrOfCondStd);
	for (i = 0; i < nrOfCondStd; ++i) {
	    sprintf (buf, "sub_term_distr_%s", masktable[conductorMask[i]].name);
	    sep_on_res[i] = paramLookupB (buf, "off");
	}
	mergeNeighborSubContacts = !paramLookupB ("sep_sub_term", "off");
    }

#ifdef PLOT_CIR_MODE
    if (optPlotCir && extrPass) plotInit ("cir");
#endif

    hasBipoElem = extrPass ? hasBipoELEM : 0;

    initLump (circuitKey, layoutKey);

    if (prePass == 1) initMeshEdge (layoutKey);

    if (substrRes) initSubstr (layoutKey);
    if (prePass1 && optSimpleSubRes) initContEdge (layoutKey);

    xyListBegin = NULL;
    nrOfXYItems = 0;

    msgDoubleJuncCaps = 0;
    condWarnCnt = 0;
    tileCnt = 0;   /* first tile will have cnt = 1 */
    tileConCnt = 0;

    if (optPrick && extrPass) {
	if (paramLookupB ("debug.selective_res", "off")) {
	    contRef_t *c;
	    fprintf (stderr, "selected high-res tiles are:\n");
	    if (selResTiles) fprintf (stderr, "%8s  con    xl    bl\n", "tile_nr");
	    for (c = selResTiles; c; c = c -> next) {
		fprintf (stderr, "%8d %4d %5d %5d\n", c->nr, c->cx, c->xl/4, c->bl/4);
	    }
	    for (i = 0; i < prickNameCnt; ++i) {
		fprintf (stderr, " netname '%s' used %d time%s\n", prickName[i],
		    prickUsed[i], prickUsed[i] > 1 ? "s" : (prickUsed[i] > 0 ? "" : "!"));
		prickUsed[i] = 1;
	    }
	}
	if (prickNameNew) {
	    for (i = 0; i < prickNameCnt; ++i) {
		if (!prickUsed[i]) say ("warning: file 'sel_con', netname '%s' not used", prickName[i]);
		DISPOSE (prickName[i], strlen(prickName[i]) + 1);
	    }
	    DISPOSE (prickName, prickNameNew*sizeof(char*));
	    DISPOSE (prickUsed, prickNameNew*sizeof(int));
	    prickNameCnt = prickNameNew = 0;
	}
    }

    if (prePass == 2) {
	prickNameCnt = prickNameNew = 0;
	selResTiles = selResTilesLast = NULL;
	fpPrick = fopen ("sel_con", "r");
	if (!fpPrick) {
	    char *fn = dmGetMetaDesignData (PROCPATH, circuitKey -> dmproject, "sel_con");
	    fpPrick = fopen (fn, "r");
	}
	if (!fpPrick) {
	    say ("error: cannot open file 'sel_con'");
	    die ();
	}
	readXYFile (fpPrick, 1);
	fclose (fpPrick);
    }

    if (useAnnotations && extrPass) {
	if (dmStat (layoutKey, "torname", &sbuf) == 0) {
	    dmsTorname = dmOpenStream (layoutKey, "torname", "r");
	    readXYFile (dmsTorname -> dmfp, 2);
	    dmCloseStream (dmsTorname, COMPLETE);
	}
    }

    if (xyListBegin) {
	if (nrOfXYItems > 1) sortXYItems ();
	xyListPos = xyListBegin;
	getXYItem ();
	doTileXY = 1;
    }
    else
	doTileXY = 0;
}

void endExtract ()
{
    if (extrPass) {
#ifdef PLOT_CIR_MODE
	if (optPlotCir) plotEnd ();
#endif
	if (optPrick) ASSERT (!selResTiles);
    }
    else {
	if (prePass == 1) endMeshEdge ();
    }
    if (substrRes) endSubstr ();
    if (prePass1 && optSimpleSubRes) endContEdge ();
    endLump ();
}

int peekTileXY ()
{
    return (nextX);
}

Private void say_two_present (int mos)
{
    char *m1, *m2;
    if (mos) {
	m1 = "bipolar and mos transistor";
	m2 = "assigned to mos transistor";
    }
    else {
	m1 = "two bipolar transistors";
	m2 = "assigned to one transistor";
    }
    say ("warning: %s present at position %s\n   instance name '%s' only %s",
	m1, strCoorBrackets (nextX, nextY), nextName, m2);
}

Private void say_two_inst (char *instName)
{
    char *m1 = "two instance names specified for transistor";
    say ("warning: %s:\n   '%s' and '%s', second name not used",
	m1, instName, nextName);
}

void testTileXY (tile_t *tile, tile_t *tile_above, coor_t xr)
{
    int cx, cy;
    subnode_t ** cons;
    int assigned, checkC, checkB;
    coor_t dx, y, br;

    while (nextX <= xr) {
	dx = nextX - tile -> xl;

	y = tile -> bl;
	if (dx > 0 && tile -> br != y) {
	    y += dx * ((double)tile -> br - y) / ((double)tile -> xr - tile -> xl);
	}
	if (dx < 0 || nextY < y) {
	    skipXYItem ();
	}
	else {
	    y = tile -> tl;
	    if (dx > 0 && (br = tile_above -> br) != y) {
		y += dx * ((double)br - y) / ((double)tile_above -> xr - tile -> xl);
	    }
	    if (nextY > y) break;

	    /* point is in tile ! */

	    if (nextCon >= 0) {
		cons = tile -> cons;
		if (cons[nextCon])
		    Grp (cons[nextCon] -> node) -> prick = 1;
		else {
		    if (nextY == y || nextX == xr) {
			xyListPos = xyListPos -> next;
			goto get_next_xy_nofree;
		    }
		    say ("warning: file 'sel_con', no conductor '%s' at position %s",
			conNr2Name (nextCon), strCoorBrackets (nextX, nextY));
		}
	    }
	    else {
		assigned = 0;

		if (tile -> tor) {
		    if (tile -> tor -> instName)
			say_two_inst (tile -> tor -> instName);
		    else
			tile -> tor -> instName = nextName;
		    assigned = 1;
		}

		if (hasBipoElem) {
		    pnTorLink_t *tl;
		    polnode_t **pn;
		    BJT_t *vT;

		    if (!optRes) cons = tile -> cons;
		    else cons = tile -> rbPoints ? tile -> rbPoints -> cons : NULL;

		    if (cons)
		    for (cx = 0; cx < nrOfCondStd; ++cx) {
			if (!cons[cx] || !cons[cx] -> pn) continue;

			for (tl = cons[cx] -> pn -> tors; tl; tl = tl -> next) {
			    if (tl -> type == TORELEM) continue;
			    pn = tl -> tor -> pn;
			    checkC = checkB = 0;
			    for (cy = 0; cy < nrOfCondStd; ++cy) {
				if (cons[cy] && cons[cy] -> pn) {
				    if (cons[cy] -> pn == pn[BA]) checkB = 1;
				    if (cons[cy] -> pn == pn[CO]) checkC |= 1;
				    if (cons[cy] -> pn == pn[EM]) checkC |= 2;
				}
			    }
			    if (checkB)
			    if (checkC == 3 || (checkC && tl -> type == LBJTELEM)) {
				if (assigned) {
				    say_two_present (1); /* bjt && mos */
				    goto get_next_xy;
				}
				assigned = 1;
				vT = tl -> tor;
				if (vT -> instName)
				    say_two_inst (vT -> instName);
				else
				    vT -> instName = nextName;
				goto get_next_xy;
			    }
			}
		    }
		}
get_next_xy:
		if (!assigned) {
		    if (nextY == y || nextX == xr) {
			xyListPos = xyListPos -> next;
			goto get_next_xy_nofree;
		    }
		    say ("warning: stream 'torname', no transistor '%s' at position %s", nextName,
			strCoorBrackets (nextX, nextY));
		}
	    }
	    freeXYItem ();
get_next_xy_nofree:
	    getXYItem ();
	}
    }

    if (xyListPos != xyListBegin) {
	xyListPos = xyListBegin;
	getXYItem ();
    }
}

void advanceTileXY (coor_t x)
{
    while (nextX < x) skipXYItem ();
}

Private void freeXYItem ()
{
    xyInfo_t *tmp = xyListPos;

    xyListPos = xyListPos -> next;
    if (tmp == xyListBegin) {
	xyListBegin = xyListPos;
	if (xyListPos) xyListPos -> prev = NULL;
    }
    else {
	if (xyListPos) xyListPos -> prev = tmp -> prev;
	tmp -> prev -> next = xyListPos;
    }
 // if (tmp -> name) DISPOSE (tmp -> name);
    DISPOSE (tmp, sizeof(xyInfo_t));
}

Private void skipXYItem ()
{
    if (nextCon >= 0)
	say ("warning: file 'sel_con', no conductor '%s' at position %s",
	    conNr2Name (nextCon), strCoorBrackets (nextX, nextY));
    else
	say ("warning: stream 'torname', no transistor '%s' at position %s",
	    nextName, strCoorBrackets (nextX, nextY));
    freeXYItem ();
    getXYItem ();
}

Private void getXYItem ()
{
    if (xyListPos) {
	nextX = xyListPos -> x;
	nextY = xyListPos -> y;
	nextCon = xyListPos -> cx;
	nextName = xyListPos -> name;
    }
    else {
	nextX = INF;
	if (!xyListBegin) doTileXY = 0;
    }
}

Private void errXY (char *s, int type, int nr)
{
    if (type == 1)
	say ("error: file 'sel_con' at line %d: %s", nr, s);
    else
	say ("error: stream 'torname' at line %d: %s", nr, s);
    die ();
}

Private void readXYFile (FILE *fp, int type)
{
    xyInfo_t *xyNew, *xyListEnd;
    int nr, x, y;
    char name[512];
    char buf[1024];

    xyListEnd = NULL;

    nr = 0;
    while (fgets (buf, sizeof (buf), fp)) {
	++nr;
	if (*buf == '#') ; /* comment line */
	else if (type == 1 && *buf == '=') ++prickNameCnt;
	else if (sscanf (buf, "%d %d", &x, &y) == 2) {
	    if (sscanf (buf, "%d %d %s", &x, &y, name) == 3) {
		xyNew = NEW (xyInfo_t, 1);
		xyNew -> x = x * outScale;
		xyNew -> y = y * outScale;
		if (type == 1) {
		    if ((xyNew -> cx = conName2Nr (name)) < 0 || xyNew -> cx >= nrOfCondStd) {
			sprintf (buf, "mask '%s' is not a legal conductor", name);
			errXY (buf, type, nr);
		    }
		    xyNew -> name = NULL;
		}
		else {
		    xyNew -> name = strsave (name);
		    xyNew -> cx = -1;
		}
		if (xyListBegin) {
		    xyListEnd -> next = xyNew;
		    xyNew -> prev = xyListEnd;
		}
		else {
		    xyListBegin = xyNew;
		    xyNew -> prev = NULL;
		}
		xyNew -> next = NULL;
		xyListEnd = xyNew;
		nrOfXYItems++;
	    }
	    else if (type == 1)
		errXY ("maskname not found", type, nr);
	    else
		errXY ("torname not found", type, nr);
	}
	else errXY ("x,y point not found", type, nr);
    }

    if (prickNameCnt) {
	rewind (fp);
	prickNameNew = prickNameCnt;
	prickName = NEW (char*, prickNameNew);
	prickUsed = NEW (int  , prickNameNew);
	prickNameCnt = 0;
	while (fgets (buf, sizeof (buf), fp)) if (*buf == '=') {
	    if (sscanf (buf+1, "%s", name) == 1) {
		prickName[prickNameCnt] = strsave (name);
		prickUsed[prickNameCnt] = 0;
		if (++prickNameCnt == prickNameNew) break;
	    }
	}
    }
}

Private void sortXYItems ()
{
    xyInfo_t *xyI, **xyItems;
    int i;

    xyItems = NEW (xyInfo_t *, nrOfXYItems);

    xyI = xyListBegin;
    for (i = 0; i < nrOfXYItems; i++) {
        xyItems[i] = xyI;
        xyI = xyI -> next;
    }

    qsort ((char *)xyItems, nrOfXYItems, sizeof (xyInfo_t *), compareXY);

    xyListBegin = xyItems[0];
    xyListBegin -> prev = NULL;
    for (i = 1; i < nrOfXYItems; i++) {
        xyItems[i - 1] -> next = xyItems[i];
        xyItems[i] -> prev = xyItems[i - 1];
    }
    xyItems[i - 1] -> next = NULL;

    DISPOSE (xyItems, sizeof(xyInfo_t *) * nrOfXYItems);
}

Private int compareXY (const void *e1, const void *e2) /* compare two xy items on x and then y */
{
    xyInfo_t ** xy1 = (xyInfo_t **) e1;
    xyInfo_t ** xy2 = (xyInfo_t **) e2;
    /* smallest first */
    if ((*xy2) -> x > (*xy1) -> x) return (-1);
    if ((*xy2) -> x < (*xy1) -> x) return ( 1);
    if ((*xy2) -> y > (*xy1) -> y) return (-1);
    if ((*xy2) -> y < (*xy1) -> y) return ( 1);
    return (0);
}
