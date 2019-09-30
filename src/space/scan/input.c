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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "src/libddm/dmincl.h"

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"

extern int nrOfMasksExt;
extern int * conductorMask;
extern mask_t subBitmask;
extern bool_t term_use_center;

#ifdef DEBUG
extern coor_t bbxl, bbxr, bbyb, bbyt; /* global bounding box */
#endif

struct e {
    int mask_no;
    edge_t    * edge;
    DM_STREAM * stream;
    struct e  * next;
};

static struct e * edges = NULL;
static int TERM_index = -1;
static int TERM_look_index = 0;
static int TERM_used_index = 0;

static edge_t	* subEdge1;
static edge_t	* subEdge2;
#ifdef CONFIG_SPACE3D
static coor_t	ySub;
#endif

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private edge_t *doFetch (DM_STREAM *stream, int mask_no);
Private void sortTerminals (terminal_t **base, int nelem);
Private void sortTerminalsByName (terminal_t **base, int nelem);
Private int compare (const void *e1, const void *e2);
Private void readLabels (DM_CELL *cellKey);
Private void readHierNames (DM_CELL *cellKey);
#ifdef CONFIG_SPACE3D
extern coor_t findStripForSubNextY (coor_t);
#endif

#ifdef __cplusplus
  }
#endif

void openInput (DM_CELL *cellKey)
{
    int i;
    edge_t * edge;
    terminal_t *t, *t2;
    DM_STREAM *bxxStream, *glnStream;
    char buf[264];
    char buf2[264];
    char * labelName;
    char help_name[1024];
    double x, y;
    static int gave_dupl_warning = 0;
    static int duplicate_list = -1;
    int maxTermOffset = nrOfTerminals;

    edges = NULL;

#ifdef CONFIG_SPACE3D
    if (prePass1 == 2) {
	TERM_index = -1; /* re-init needed for Xspace */
	nrOfTerminals = 0;
	i = 0; /* cont_bln mask must be a conductor mask */

	glnStream = dmOpenStream (cellKey, "cont_bln", "r");
	if (glnStream && (edge = doFetch (glnStream, i))) {
	    edges = NEW (struct e, 1);
	    edges -> mask_no = i;
	    edges -> stream = glnStream;
	    edges -> edge = edge;
	    edges -> next = NULL;
	}
	return;
    }
#else
    ASSERT (prePass1 != 2);
#endif

    if (duplicate_list < 0) duplicate_list = paramLookupB ("duplicate_list", "off");

    /* open the individual streams per mask */

    for (i = 0; i < nrOfMasksExt; i++) {
	if (!masktable[i].gln) continue;
	    glnStream = dmOpenStream (cellKey, mprintf ("%s_gln", masktable[i].name), "r");
	    if (glnStream && (edge = doFetch (glnStream, i))) {
		struct e *e, *f, *n;

		if (edge -> xl == -INF) { /* skip negative mask edges */
		    ASSERT (hasSubmask);
		    COLOR_ADD (subBitmask, edge -> color);
		    edge = doFetch (glnStream, i);
		    ASSERT (edge && edge -> xl == -INF);
		    if (!(edge = doFetch (glnStream, i))) goto skip;
		}

		n = NEW (struct e, 1);
		n -> mask_no = i;
		n -> stream = glnStream;
		n -> edge = edge;
		for (f = 0, e = edges; e; e = (f = e) -> next) {
		    if (e -> edge -> xl > edge -> xl) break;
		    if (e -> edge -> xl < edge -> xl) continue;
		    if (e -> edge -> yl > edge -> yl) break;
		    if (e -> edge -> yl < edge -> yl) continue;
		    if (compareSlope (e -> edge, <, edge)) continue;
		    break;
		}
		if (f) {
		    n -> next = f -> next;
		    f -> next = n;
		}
		else {
		    n -> next = edges;
		    edges = n;
		}
	    }
skip:	;

	if (masktable[i].terminal && masktable[i].conductor >= 0) {

	    /* Read (possible) terminals that are defined for this mask */

	    bxxStream = dmOpenStream (cellKey, mprintf ("t_%s_bxx", masktable[i].name), "r");
            if (bxxStream) {
		while (dmGetDesignData (bxxStream, GEO_BOXLAY) > 0) {
		    ASSERT (gboxlay.chk_type < maxTermOffset);
		    t = TERM[gboxlay.chk_type];
		    t -> conductor = masktable[i].conductor;

		    if (term_use_center) { /* center coord. (tile split) */
			x = ((double)gboxlay.xl + gboxlay.xr) / 2;
			y = ((double)gboxlay.yb + gboxlay.yt) / 2;
		    }
		    else { /* terminal on left edge (gives nicer BE-mesh) */
			x = gboxlay.xl;
			y = ((double)gboxlay.yb + gboxlay.yt) / 2;
		    }
		    t -> x = x * inScale;
		    t -> y = y * inScale;

		    if (useLeafTerminals && t -> instName) {
			labelName = LABELNAME[gboxlay.chk_type];
			if (*labelName == ' ') {
			    sprintf (help_name, "%s_%s_%s", labelName + 1, strCoor (t -> x), strCoor (t -> y));
			    DISPOSE (labelName, strlen(labelName) + 1);
			    labelName = strsave (help_name);
			}

			/* The truncation of the name is done later on - in
			   outNode () - since we want to remember the complete
			   label name.  The latter is desired since label names
			   may be used to generate other node names (e.g.
			   'inst1.labelA_1', 'inst1.labelA_2' for label
			   'inst1.labelA') that may again be truncated.
			*/
			newTerminal (tLabel2, t -> conductor, t -> x, t -> y, labelName, NULL, -1, -1, -1, -1);
                    }
		}

		dmCloseStream (bxxStream, COMPLETE);
	    }
	}
    }

    if (hasSubmask) {
	NEW_EDGE (subEdge1, bigbxl, bigbyb, bigbxr, bigbyb, subBitmask);
	NEW_EDGE (subEdge2, bigbxl, bigbyt, bigbxr, bigbyt, subBitmask);
    }

    if (useAnnotations) readLabels (cellKey);
    if (useHierAnnotations || useHierTerminals) readHierNames (cellKey);

    nrOfTermNames = nrOfTerminals - nrOfTermNames; /* !instance terminals */

    addTerminalNames ();
    sortTerminals (TERM, nrOfTerminals);
    if (nrOfTerminals < 2) return;
    sortTerminalsByName (TERMBYNAME, nrOfTerminals);

    /* Check on duplicate terminal/label names.
       If both names have an array form, the duplicity is not detected,
       since that requires a more extended test.
    */
    t2 = TERMBYNAME[0];
    for (i = 1; i < nrOfTerminals; i++) {
	t = t2;
	t2 = TERMBYNAME[i];

	if (((t -> termX < 0 && t -> termY < 0) || (t2 -> termX < 0 && t2 -> termY < 0))

	    && compareByName ((const void *)&t, (const void *)&t2) == 0

	    && (!t -> instName
		|| (t  -> instX < 0 && t  -> instY < 0)
		|| (t2 -> instX < 0 && t2 -> instY < 0))) {

	    if (!gave_dupl_warning) {
		gave_dupl_warning = 1;
		say ("warning: duplicate terminal/label names were detected.");
		say ("\tAs a result - if the terminals/labels are on different nets -");
		say ("\tthere may be different nets with the same name.");
		if (!duplicate_list)
		say ("\tSet parameter 'duplicate_list' to obtain a list of duplicate names.");
	    }
	    if (duplicate_list) {
		if (t -> instName)
		    sprintf (buf, "'%s' for instance\n   '%s' ", t -> termName, t -> instName);
		else
		    sprintf (buf, "'%s'\n   ", t -> termName);
		strcpy (buf2, strCoorBrackets (t -> x, t -> y));
		say ("warning: duplicate terminal/label name %sat position %s and position %s",
		    buf, buf2, strCoorBrackets (t2 -> x, t2 -> y));
	    }
	}
    }
}

void closeInput ()
{
    ASSERT (!edges);
    while (TERM_used_index < nrOfTerminals) missingTermCon (TERM[TERM_used_index++]);
}

edge_t * fetchEdge ()
{
    edge_t *edge, *edge2;
    struct e *e, *f;

    if (!(e = edges)) {
	if (subEdge1)
	    edge = subEdge1, subEdge1 = NULL;
	else if (subEdge2)
	    edge = subEdge2, subEdge2 = NULL;
	else
	    NEW_EDGE (edge, INF, INF, INF, INF, cNull); /* EOF */
	goto ret;
    }

    edge = e -> edge;

    if (subEdge2) {
	/* When a conducting substrate mask is present, the layout bounding
	   box is surrounded by another (wider) bounding box to guarantee
	   that resistance mesh nodes will not be placed at infinity.
	   The surrounding box is added by inserting two extra edges with
	   color = subBitmask.
	*/
	if (subEdge1) {
	    ASSERT (subEdge1 -> xl <= edge -> xl);
	    ASSERT (subEdge1 -> yl <= edge -> yl);
	    ASSERT (subEdge1 -> yl <= edge -> yr);
	    edge = subEdge1; subEdge1 = NULL;
	    goto ret;
	}
	ASSERT (subEdge2 -> xl <= edge -> xl);
	ASSERT (subEdge2 -> yl >= edge -> yl);
	ASSERT (subEdge2 -> yl >= edge -> yr);
	if (subEdge2 -> xl < edge -> xl) {
	    edge = subEdge2; subEdge2 = NULL;
	    goto ret;
	}
    }

    if ((edge2 = doFetch (e -> stream, e -> mask_no))) {
	e -> edge = edge2;

	for (f = 0; (e = e -> next); f = e) {
	    if (e -> edge -> xl > edge2 -> xl) break;
	    if (e -> edge -> xl < edge2 -> xl) continue;
	    if (e -> edge -> yl > edge2 -> yl) break;
	    if (e -> edge -> yl < edge2 -> yl) continue;
	    if (compareSlope (e -> edge, <, edge2)) continue;
	    break;
	}
	if (f) {
	    edges = (e = edges) -> next;
	    e -> next = f -> next;
	    f -> next = e;
	}
    }
    else {
	edges = e -> next;
	DISPOSE (e, sizeof(struct e));
    }
ret:
    Debug (printEdge ("fetchEdge", edge));
 /* ASSERT (edge -> xl < edge -> xr || (edge -> xl == INF && edge -> xr == INF)); */
    return (edge);
}

Private edge_t * doFetch (DM_STREAM *stream, int mask_no)
{
    edge_t *edge;
    int k;

#ifdef CONFIG_SPACE3D
    if (prePass1 == 2) {
	k = dmGetDesignData (stream, GEO_BOXLAY);
	if (k > 0) {
	    mask_t *colorp;
	    if (gboxlay.chk_type & 0x100) /* interior edge */
		colorp = &cNull;
	    else {
		k = gboxlay.chk_type & 0xff; /* conductor_nr */
		ASSERT (k >= 0 && k <= nrOfCondStd);
		colorp = &masktable[conductorMask[k]].color;
		if (!IS_COLOR (colorp)) fprintf (stderr, "doFetch: no color for conductor_nr %d\n", k);
	    }
	    NEW_EDGE (edge, (coor_t) gboxlay.xl, (coor_t) gboxlay.yb,
			    (coor_t) gboxlay.xr, (coor_t) gboxlay.yt, *colorp);
	    edge -> cc = gboxlay.chk_type;
	    return edge;
	}
	if (k < 0) say ("cont_bln read error"), die ();
	dmCloseStream (stream, COMPLETE);
	return NULL;
    }

    if (mask_no < 0) {
	if ((ySub = findStripForSubNextY (ySub)) < bigbyt) {
	    NEW_EDGE (edge, bigbxl, ySub, bigbxr, ySub, cNull);
	    return edge;
	}
	return NULL;
    }
#endif // CONFIG_SPACE3D

    k = dmGetDesignData (stream, GEO_GLN);
    if (k > 0) {
#ifdef DEBUG
	if (ggln.xl < bbxl || ggln.xr > bbxr
	||  ggln.yl < bbyb || ggln.yl > bbyt
	||  ggln.yr < bbyb || ggln.yr > bbyt) say ("edge outside cell");
#endif
	NEW_EDGE (edge, (coor_t) ggln.xl, (coor_t) ggln.yl,
			(coor_t) ggln.xr, (coor_t) ggln.yr, masktable[mask_no].color);
	return edge;
    }
    if (k < 0) say ("gln read error"), die ();

    dmCloseStream (stream, COMPLETE);
    return NULL;
}

terminal_t * fetchTerm ()
{
    static terminal_t TM;
    terminal_t * tm;

    if (++TERM_index < nrOfTerminals) {
	tm = TERM[TERM_index];
    }
    else { /* after term INF no fetch is more done! */
	tm = &TM;
	ASSERT (TERM_index == nrOfTerminals);
	tm -> x = tm -> y = INF;
    }
    Debug (fprintf (stderr, "fetch term %d %d\n", tm -> x, tm -> y));
    return tm;
}

terminal_t * lookTerm ()
{
    if (TERM_look_index < TERM_index) {
	terminal_t * tm = TERM[TERM_look_index++];
	Debug (fprintf (stderr, "look term %d %d\n", tm -> x, tm -> y));
	return tm;
    }
    return 0;
}

int newLookTerm (coor_t x1, coor_t y1)
{
    while (TERM_used_index < TERM_index) {
	terminal_t * tm = TERM[TERM_used_index];
	if (tm -> x == x1 && tm -> y >= y1) {
	    TERM_look_index = TERM_used_index; /* reset */
	    return 1;
	}
	ASSERT (tm -> x <= x1);
	missingTermCon (tm);
	++TERM_used_index;
    }
    return 0;
}

void useLastLookTerm ()
{
    ASSERT (TERM_look_index > TERM_used_index);

    if (TERM_look_index - 1 > TERM_used_index) {
	terminal_t * save = TERM[TERM_look_index - 1];
	TERM[TERM_look_index - 1] = TERM[TERM_used_index];
	TERM[TERM_used_index] = save;
    }
    TERM_used_index++;
}

Private void sortTerminals (terminal_t **base, int nelem)
{
    if (nelem > 1) qsort (base, nelem, sizeof (terminal_t *), compare);
    TERM_index = -1;
    TERM_look_index = 0;
    TERM_used_index = 0;
}

Private int compare (const void *e1, const void *e2) /* compare two terminals on x and then y */
{
    terminal_t ** t1 = (terminal_t **) e1;
    terminal_t ** t2 = (terminal_t **) e2;
    /* smallest first */
    if ((*t2) -> x > (*t1) -> x) return (-1);
    if ((*t2) -> x < (*t1) -> x) return ( 1);
    if ((*t2) -> y > (*t1) -> y) return (-1);
    if ((*t2) -> y < (*t1) -> y) return ( 1);
    return (0);
}

Private void sortTerminalsByName (terminal_t **base, int nelem)
{
    qsort (base, nelem, sizeof (terminal_t *), compareByName);
}

Private void readLabels (DM_CELL *cellKey)
{
    struct stat statBuf;
    DM_STREAM *dmfp;
    double x, y;
    int m, conductor;
    char *s, *tName;

    if (dmStat (cellKey, "annotations", &statBuf) == -1) return;
    if (!(dmfp = dmOpenStream (cellKey, "annotations", "r"))) return;

    while (dmGetDesignData (dmfp, GEO_ANNOTATE) > 0) {
	if (ganno.type == GA_LABEL) {
	    m = ganno.o.Label.maskno;
	    s = ganno.o.Label.name;
	    x = (double)ganno.o.Label.x * outScale;
	    y = (double)ganno.o.Label.y * outScale;
	    if (m < 0 || m >= nrOfMasks || (conductor = masktable[m].conductor) < 0) {
		say ("warning: annotations: no conductor defined for label '%s' (skipped, msk# %d)", s, m);
		continue;
	    }
	    tName = strsave (s);
	    newTerminal (tLabel, conductor, (coor_t)x, (coor_t)y, tName, NULL, -1, -1, -1, -1);
	}
    }
    dmCloseStream (dmfp, COMPLETE);
}

Private void readHierNames (DM_CELL *cellKey)
{
    static int wnr2;
    struct stat statBuf;
    DM_STREAM *dmfp;
    double x, y;
    char name[1024];
    char help_name[1024];
    char cell_name[DM_MAXNAME + 1];
    char simpleName[DM_MAXNAME + 1];
    char *l, *s;
    int m, conductor, isLabel;

    if (dmStat (cellKey, "anno_exp", &statBuf) == -1) return;
    if (!(dmfp = dmOpenStream (cellKey, "anno_exp", "r"))) return;

    name[0] = cell_name[0] = '\0';
    while (dmScanf (dmfp, "%s", help_name) == 1) {
	if (help_name[0] == '/' && !help_name[1])
            dmScanf (dmfp, "%d\n", &isLabel);
        else {
            strcpy (name, help_name);
            dmScanf (dmfp, "%s %d\n", cell_name, &isLabel);
        }

	if (isLabel) {
	    if (dmGetDesignData (dmfp, GEO_ANNOTATE) <= 0) break;
	    if (!useHierAnnotations) continue;

	    ASSERT (ganno.type == GA_LABEL);
	    s = ganno.o.Label.name;
	    m = ganno.o.Label.maskno;
	    x = ganno.o.Label.x;
	    y = ganno.o.Label.y;
	}
	else {
	    s = simpleName;
	    if (dmScanf (dmfp, "%s %d %le %le\n", s, &m, &x, &y) != 4) break;
	    if (!useHierTerminals) continue;
	}
	x *= inScale;
	y *= inScale;

	l = help_name;
	if (useCellNames || (name[0] == '.' && !name[1])) {
	    sprintf (l, "%s_%s_%s_%s", cell_name, s, strCoor ((coor_t)x), strCoor ((coor_t)y));
	} else {
	    strcpy (l, name);
	    convertHierName (l);
	    sprintf (l + strlen (l), "%s%s", inst_term_sep, s);
	}
	/* The truncation of the name is done later on - in
	   outNode () - since we want to remember the complete
	   label name.  The latter is desired since label names
	   may be used to generate other node names (e.g.
	   'inst1.labelA_1', 'inst1.labelA_2' for label
           'inst1.labelA') that may again be truncated.
	*/

	if (m < 0 || m >= nrOfMasks || (conductor = masktable[m].conductor) < 0) {
	    if (!wnr2++)
	    say ("warning: anno_exp: no conductor defined for label '%s' (skipped, msk# %d)", s, m);
	    continue;
	}
	l = strsave (l);
	newTerminal (tLabel2, conductor, (coor_t)x, (coor_t)y, l, NULL, -1, -1, -1, -1);
    }
    dmCloseStream (dmfp, COMPLETE);
}
