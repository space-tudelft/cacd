/*
 * ISC License
 *
 * Copyright (C) 1997-2018 by
 *	F.V. Fjeld
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

#include <math.h>

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"
#include "src/space/spider/extern.h"
#include "src/space/spider/displace.h"

#ifndef M_SQRT1_2
#define M_SQRT1_2       0.70710678118654752440
#endif

#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880
#endif

extern int *displaceList;
extern coor_t bigbxr;
extern strip_t *stripR, *stripA;

/* map absolute slope to delta x and y */
Private meshCoor_t delta_x[8] = {0, M_SQRT1_2, 1, M_SQRT1_2, 0, -M_SQRT1_2, -1, -M_SQRT1_2};
Private meshCoor_t delta_y[8] = {1, M_SQRT1_2, 0, -M_SQRT1_2, -1, -M_SQRT1_2, 0, M_SQRT1_2};
Private int slope_coeff[8] = {INF,1,0,-1,INF,1,0,-1}; /* map slope to coefficient */

#define add_slopes(x,y)  ((x+y)&0x07)
#define opposite_dir(x)  (x^DEG_180) /* add_slopes(x,DEG_180)*/

typedef enum {
    ENDPOINT = 100,   /* endpoint of a chain of displacements, <sp1> is endpoint, <sp2> 1st middlepoint */
    MIDDLEPOINT = 101 /* middlepoint in chain. <sp2> is middlepoint, <sp1> prev. and <sp3> next point */
} dkind_t;

typedef struct spiderDisplace {
    meshCoor_t distance; /* to displace */
    spider_t *spider; /* spider to displace */
    spider_t *osp1;   /* 1st opposite spider */
    spider_t *osp2;   /* 2nd opposite spider (for middlepoints) */
    slope_t direction;/* for middlepoints, this is relative to osp1 -> spider -> osp2 */
    dkind_t kind;     /* endpoint or middlepoint */
} spiderDisplace_t;

/* there is one array for each level */
Private int sDnum_levels = 0;
Private int *sDnum_elements[2] = {NULL, NULL};
Private int *sDtable_size[2] = {NULL, NULL};
Private spiderDisplace_t **sDtable[2] = {NULL, NULL};

Private meshCoor_t stripRX;
Private displ_t dtype;

void spiderDelayedDisplaceInit ()
{
    int t, l;

    if (nrOfSpiderLevels > sDnum_levels) {
	for (t = 0; t < 2; t++) {
	    sDnum_elements[t]  = GROW (int, sDnum_elements[t], sDnum_levels, nrOfSpiderLevels);
	    sDtable_size[t]    = GROW (int, sDtable_size[t],   sDnum_levels, nrOfSpiderLevels);
	    sDtable[t] = GROW (spiderDisplace_t *, sDtable[t], sDnum_levels, nrOfSpiderLevels);
	    for (l = sDnum_levels; l < nrOfSpiderLevels; l++) {
		sDtable[t][l] = NULL;
		sDtable_size[t][l] = 0;
	    }
	}
	sDnum_levels = nrOfSpiderLevels;
    }
    for (t = 0; t < 2; t++) {
	for (l = 0; l < sDnum_levels; l++) sDnum_elements[t][l] = 0;
    }
}

/* insert an element (spider point) in sDtable */
Private void spiderDisplaceInsertEntry (displ_t type, int level,
	spider_t *spider, spider_t *osp1, meshCoor_t distance, slope_t direction)
{
    spiderDisplace_t *d;

    ASSERT (level < sDnum_levels);

    if (sDnum_elements[type][level] >= sDtable_size[type][level]) { /* grow array */
	int old_size = sDtable_size[type][level];
	int new_size = old_size == 0 ? 256 : old_size * 2;
	sDtable[type][level] = GROW (spiderDisplace_t, sDtable[type][level], old_size, new_size);
	ASSERT (sDnum_elements[type][level] < new_size);
	sDtable_size[type][level] = new_size;
    }

    d = &sDtable[type][level][sDnum_elements[type][level]++];
    d -> spider = spider;
    d -> osp1 = osp1;
    d -> osp2 = NULL;
    d -> distance = distance;
    d -> direction = direction;
    d -> kind = ENDPOINT;
#define displaceBit(type) (type == EDGESHAPEDISPL ? E_DISPL : C_DISPL)
    spider -> flags |= displaceBit(type);
}

/* replace element (spider point) data */
Private void spiderDisplaceReplaceEntry (spiderDisplace_t *d,
	spider_t *spider, spider_t *osp1, spider_t *osp2, meshCoor_t distance, slope_t direction)
{
    ASSERT (d -> kind == ENDPOINT);
    ASSERT (d -> spider == spider);
    d -> osp1 = osp1;
    d -> osp2 = osp2;
    d -> distance = distance;
    d -> direction = direction;
    d -> kind = MIDDLEPOINT;
}

Private spiderDisplace_t * spiderDisplaceFindElement (displ_t type, spider_t *spider, int level)
{
    int m;
    if (spider -> flags & displaceBit(type))
    for (m = sDnum_elements[type][level]; --m >= 0;) {
	if (sDtable[type][level][m].spider == spider) return &sDtable[type][level][m];
    }
    return NULL;
}

void spiderDisplaceRemove (spider_t *sp1, spider_t *sp2, int level)
{
    spiderDisplace_t *d1, *d2;

    if ((d1 = spiderDisplaceFindElement (EDGESHAPEDISPL, sp1, level))) {
	if ((d2 = spiderDisplaceFindElement (EDGESHAPEDISPL, sp2, level))) {
	    spiderDisplaceReplaceEntry (d2, sp2, d2 -> osp1, d1 -> osp1, d2 -> distance, d1 -> direction);
	    d1 -> spider = NULL;
	}
	else d1 -> spider = sp2;
    }
    if ((d1 = spiderDisplaceFindElement (SLOPEDISPL, sp1, level))) {
	if ((d2 = spiderDisplaceFindElement (SLOPEDISPL, sp2, level))) {
	    spiderDisplaceReplaceEntry (d2, sp2, d2 -> osp1, d1 -> osp1, d2 -> distance, d1 -> direction);
	    d1 -> spider = NULL;
	}
	else d1 -> spider = sp2;
    }
}

int spiderDelayedDisplace (displ_t type, spider_t *sp1, spider_t *sp2,
	meshCoor_t distance, int level, slope_t direction)
{
    spiderDisplace_t *d1, *d2;

    if (Abs (distance) <= SP_EPS) return 0; /* ignore null-displacements */

    displaceList[level] = 1;

    d1 = spiderDisplaceFindElement (type, sp1, level);
    d2 = spiderDisplaceFindElement (type, sp2, level);

    if (d1 && d2) { /* both spiders take part in another displacement */
	spiderDisplaceReplaceEntry (d1, sp1, d1 -> osp1, sp2, distance, direction);
	spiderDisplaceReplaceEntry (d2, sp2, sp1, d2 -> osp1, distance, direction);
    } else if (d1 && !d2) {
	spiderDisplaceReplaceEntry (d1, sp1, d1 -> osp1, sp2, distance, direction);
	spiderDisplaceInsertEntry (type, level, sp2, sp1, distance, opposite_dir (direction));
    } else if (!d1 && d2) {
	spiderDisplaceReplaceEntry (d2, sp2, sp1, d2 -> osp1, distance, direction);
	spiderDisplaceInsertEntry (type, level, sp1, sp2, distance, direction);
    } else {
	spiderDisplaceInsertEntry (type, level, sp1, sp2, distance, direction);
	spiderDisplaceInsertEntry (type, level, sp2, sp1, distance, opposite_dir (direction));
    }
    return 1;
}

/* measure the absolute slope from sp1 to sp2 */
Private slope_t spiderSlope (spider_t *sp1, spider_t *sp2)
{
    int equal_x, equal_y;
    meshCoor_t x1 = sp1 -> nom_x;
    meshCoor_t y1 = sp1 -> nom_y;
    meshCoor_t x2 = sp2 -> nom_x;
    meshCoor_t y2 = sp2 -> nom_y;

    equal_x = Nearby (x2, x1);
    equal_y = Nearby (y2, y1);

    if (equal_x) {
	ASSERT (!equal_y);
	return y2 > y1 ? UP : DOWN;
    }
    if (x2 > x1) {
	if (equal_y) return RIGHT;
	if (y2 > y1) return UPRIGHT;
	return DOWNRIGHT;
    }
    /* (x2 < x1) */
    if (equal_y) return LEFT;
    if (y2 > y1) return UPLEFT;
    return DOWNLEFT;
}

/* Try to create a cross-over sidewall face.
*/
Private void cosFace (spider_t *sp1)
{
    spiderEdge_t *edges[6], *e, *o;
    spider_t   *spiders[6], *sp;
    face_t *f1, *f2 = NULL;
    int i, n, type;

    for (e = sp1 -> edge; e; e = NEXT_EDGE (sp1, e))
	if (e -> type == VERTICALEDGE) break;
    if (!e) return; /* no vertical edge */
    o = e -> oh;
         if (e -> face == NULL) { f1 = o -> face; ASSERT (f1); }
    else if (o -> face == NULL) { f1 = e -> face; e = o; sp1 = e -> sp; }
    else return; /* both faces defined */

    /* Lookup all edges in CCW direction from sp1 with a NULL face.
     * The loop must already be finished.
     */
    n = 0;
    spiders[n] = sp = OTHER_SPIDER (sp1, e);
    edges[n++] = e;
    do {
	for (e = sp -> edge; e && e -> face; e = NEXT_EDGE (sp, e));
	ASSERT (e);
	if (e -> type == VERTICALEDGE) { ASSERT(!f2); f2 = e -> oh -> face; ASSERT(f2); }
	else ASSERT (e -> type == CROSSOVEREDGE);
	spiders[n] = sp = OTHER_SPIDER (sp, e);
	ASSERT (sp);
	edges[n++] = e;
    } while (n < 6 && sp != sp1);

#ifdef DEBUG
    /* post */
    ASSERT (((n == 4 || n == 6 || n == 3) && sp == sp1) || sp != sp1);
#endif

    if (n == 4 || n == 3 || (n == 6 && sp == sp1)) { /* create face */
	ASSERT (f2);
	type = FACE_SIDE + FACE_CSIDE;
	if ((f1 -> type & f2 -> type) & FACE_CORE) type += FACE_CORE;
	if ((f1 -> type | f2 -> type) & FACE_VSIDE) type += FACE_VSIDE;

	f1 = newFace ();
	f1 -> sc_subn = sp1 -> subnode2;
	f1 -> type = type;

	for (i = 0; i < n; ++i) edges[i] -> face = f1;

	if (n == 6) { /* create a second face */
	    f2 = newFace ();
	    f2 -> sc_subn = sp1 -> subnode2;
	    f2 -> type = type;

	    e = meshMakeEdge (spiders[1], spiders[4], VERTICALEDGE);
	    e -> face = f1;
	    e -> oh -> face = f2;
	    for (i = 2; i < 5; ++i) edges[i] -> face = f2;
	    meshSetCorners (f1, spiders[0], spiders[1], spiders[4], spiders[5]);
	    meshSetCorners (f2, spiders[1], spiders[2], spiders[3], spiders[4]);

	    if (stripR -> xr < bigbxr)
	    if (!(spiders[1] -> nom_x < stripRX || spiders[4] -> nom_x < stripRX)) {
		if (!(spiders[0] -> nom_x < stripRX || spiders[5] -> nom_x < stripRX)) i = 0;
		if (!(spiders[2] -> nom_x < stripRX || spiders[3] -> nom_x < stripRX)) n = 0;
	    }
	    stripAddFace (f1, i ? stripR : stripA);
	    stripAddFace (f2, n ? stripR : stripA);
	}
	else {
	    if (n == 3) spiders[3] = NULL;
	    meshSetCorners (f1, spiders[0], spiders[1], spiders[2], spiders[3]);
	    i = 0;
	    //if (stripR -> xr < bigbxr)
		//for (; i < n; ++i) if (spiders[i] -> nom_x < stripRX) break;
	    stripAddFace (f1, i < n ? stripR : stripA);
	}
    }
}

Private void spiderDisplacePoint (spiderDisplace_t *d)
{
    spider_t *spider = d -> spider;
    meshCoor_t distance = d -> distance;
    slope_t direction = d -> direction;

    if (d -> kind == MIDDLEPOINT) {
	meshCoor_t aCoeff, bCoeff, aX, bX, bY, intersectX, intersectY;
	slope_t edgeSlope1 = spiderSlope (d -> osp1, spider);
	slope_t edgeSlope2 = spiderSlope (spider, d -> osp2);

	aCoeff = slope_coeff[edgeSlope1]; /* line along edge 1 */
	bCoeff = slope_coeff[edgeSlope2]; /* line along edge 2 */

	edgeSlope1 = add_slopes (edgeSlope1, direction);
	intersectX = distance * delta_x[edgeSlope1];
	intersectY = distance * delta_y[edgeSlope1];

	if (aCoeff != bCoeff) { /* not parallel */
	    edgeSlope2 = add_slopes (edgeSlope2, direction);
	    bX = distance * delta_x[edgeSlope2];
	    bY = distance * delta_y[edgeSlope2];

	    /* place spider at the intersection of lines a and b */
	    /* check for vertical lines */
	    if (aCoeff == INF) {
		intersectY = bY + bCoeff*(intersectX-bX);
	    } else if (bCoeff == INF) {
		intersectY += aCoeff*(bX-intersectX);
		intersectX = bX;
	    } else { /* not vertical */
		aX = intersectX;
		intersectX = (bY-bX*bCoeff-intersectY+aX*aCoeff)/(aCoeff-bCoeff);
		intersectY += aCoeff*(intersectX-aX);
	    }
	}
	spider -> act_x += intersectX;
	spider -> act_y += intersectY;
    }
    else {
	spiderEdge_t *edge;
	slope_t edgeSlope, moveSlope, slope;
	meshCoor_t dx, dy;

	/* Note: ENDPOINTs are normally only used by SLOPEDISPL (SdeG).
	 */
	ASSERT (d -> kind == ENDPOINT);
	ASSERT (direction == LEFT || direction == RIGHT);

	/* move <spider> by <distance> in direction <direction> relative to the
	 * line between <spider> and <d->osp1>.
	 * moveSlope is the direction in which the spider should be displaced.
	 */
	edgeSlope = spiderSlope (spider, d -> osp1); /* absolute slope spider -> d->osp1 */
	moveSlope = 0;
	for (edge = spider -> edge; edge; edge = edge -> nb) {
	    ASSERT (edge -> sp == spider);
	    if (edge -> type == CONDUCTOREDGE) {
		slope = spiderSlope (spider, edge -> oh -> sp);
		slope = (slope - edgeSlope)&0x07; /* relative slope */
		if (direction == RIGHT && slope) slope = 8 - slope; /* slopeMirror */
		if (!slope && !moveSlope && edge -> oh -> sp == d -> osp1)
		    moveSlope = LEFT; /* EDGESHAPEDISPL (SdeG) */
		else if (!slope || moveSlope)
		    say ("spiderDisplace: spider (%g %g %g) unexpected %smoveSlope: %d\n",
			spider->act_x/4, spider->act_y/4, spider->act_z/4, moveSlope? "2nd " : "", slope);
		else
		    moveSlope = slope;
	    }
	}
	if (moveSlope != LEFT && moveSlope != DOWNLEFT && moveSlope != UPLEFT) {
	    say ("spiderDisplace: spider (%g %g %g) unexpected moveSlope: %d\n",
		spider->act_x/4, spider->act_y/4, spider->act_z/4, moveSlope);
	    moveSlope = LEFT;
	}
	if (direction == RIGHT) moveSlope = 8 - moveSlope; /* slopeMirror */
	moveSlope = (moveSlope + edgeSlope)&0x07; /* rel. to abs. slope */

	if ((moveSlope&1) != (edgeSlope&1)) distance *= M_SQRT2;
	spider -> act_x += distance * delta_x[moveSlope];
	spider -> act_y += distance * delta_y[moveSlope];
	if (dtype == SLOPEDISPL) cosFace (spider);
    }
}

Private void spiderDisplaceExecute (displ_t type)
{
    int level, i, j, num_elements;
    spiderDisplace_t *d;

    dtype = type;
    for (level = 0; level < sDnum_levels; level++) {
	num_elements = sDnum_elements[type][level];
	j = 0;
	for (i = 0; i < num_elements; i++) {
	    d = &sDtable[type][level][i];
	    if (d -> spider) { /* skip joined spiders */
	    if (d -> spider -> nom_x <= stripRX) { /* only spider in current strip */
		spiderDisplacePoint (d);
	    } else {
		/* if "not executed", reschedule it, insert it into the array again */
		/* what we are writing over is guaranteed to be executed already */
		sDtable[type][level][j++] = *d; /* copy element */
	    }
	    }
	}
	sDnum_elements[type][level] = j;
    }
}

void spiderDisplaceNow ()
{
    stripRX = stripR -> xr;
    spiderDisplaceExecute (EDGESHAPEDISPL);
    spiderDisplaceExecute (SLOPEDISPL);
}
