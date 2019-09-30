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
#include <math.h>
#include <stdlib.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/green.h"
#include "src/space/green/export.h"

#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern strip_t *stripL, *stripR;
extern int new_via_mode;
extern int new_refine;
extern bool_t optEstimate3D;
extern int connect_ground;
extern int nrOfCondStd;
extern subnode_t *subnSUB;
extern void findCenterSpiderInit (void);
extern nodePoint_t * findCenterSpider (int sidewall, double x, double y, int cx, tile_t **tile);

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void refineTriangle (face_t *face);
Private void meshAddExtraSpider (face_t *face);
Private void refineSidewall (face_t *face);
Private void refineTrapezoid (face_t *face);
Private spider_t *meshSplitPath (face_t *face, spider_t *sp1, spider_t *sp2);
Private int getDirection (spider_t *sp1, spider_t *sp2, spider_t *sp3, int vside);
#ifdef __cplusplus
  }
#endif

#define SMALL_INC  1.000000001
        /* This value is used as a multiplication factor
	   to assure that length and area comparisons
	   give results as expected.  It was noticed that
	   under Linux, compiled with -O, a comparison a > b,
	   where a == b, gave TRUE as result. So now we use
	   a > b * SMALL_INC as comparison.
	*/

Private double distRatio = 0.5;

extern bool_t prePass1;

extern int numFaces;
extern int crossingIndex;
extern double crossing_z;
extern double crossing_z09;
extern double crossing_z11;
extern double maxFeRatio;

#define SIMPLE_TRIANGLE(f) (meshFindEdge (f->corners[0], f->corners[1]) \
			 && meshFindEdge (f->corners[1], f->corners[2]) \
			 && meshFindEdge (f->corners[2], f->corners[0]))

#define SP2(k) f2 -> corners[k]

Private void removeEdge (spiderEdge_t *oh)
{
    spiderEdge_t *eh;
    spider_t *sp = oh -> sp;

    if ((eh = sp -> edge) == oh) sp -> edge = oh -> nb;
    else {
	while (eh && eh -> nb != oh) eh = eh -> nb;
	ASSERT (eh);
	eh -> nb = oh -> nb;
    }
    oh -> sp = NULL;
    oh -> face = NULL;

    if ((eh = sp -> edge) && (oh = eh -> nb) && !oh -> nb) {
	eh -> sp = oh -> sp = NULL;
	sp -> edge = NULL;
	eh -> oh -> oh = oh -> oh;
	oh -> oh -> oh = eh -> oh;
	if (FeModePwl) stripRemoveSpider (sp);
    }
}

Private void removeFace2 (face_t *f2, face_t *face, spiderEdge_t *oh)
{
    spiderEdge_t *eh;
    spider_t *sp, *first = SP2(0);
    sp = first;
    do {
	ASSERT (sp);
	for (eh = sp -> edge; eh && eh -> face != f2; eh = NEXT_EDGE (sp, eh));
	ASSERT (eh);
	eh -> face = face;
    } while ((sp = OTHER_SPIDER (sp, eh)) != first);
    disposeFace (f2); numFaces--;
    removeEdge (oh);
    removeEdge (oh -> oh);
}

Private spider_t *splitPath (face_t *face, spider_t *sp1, spider_t *sp2, meshCoor_t nom_v)
{
    meshCoor_t act_x, act_y, nom_x, nom_y;
    meshCoor_t dx, dy, sign, d3, d4;
    spider_t *sp, *sp3, *sp4;

    dx = sp2 -> nom_x - sp1 -> nom_x;
    dy = sp2 -> nom_y - sp1 -> nom_y;
    ASSERT (sp2 -> nom_z == sp1 -> nom_z);

    // split hor.edge
    act_x = nom_x = nom_v;
    nom_y = sp1 -> nom_y;
    act_y = sp1 -> act_y;
    if (dy) { // 45 degree only
	d3 = nom_x - sp1 -> nom_x;
	if ((dy < 0 && d3 > 0) || (dy > 0 && d3 < 0)) d3 = -d3;
	nom_y += d3;
	act_y += d3;
    }

    sp3 = sp4 = NULL;
    for (sp = sp1; sp != sp2; sp = sp4) {
        sign = (nom_x - sp -> nom_x) * dx + (nom_y - sp -> nom_y) * dy;
	if (sign <= 0) break;
	sp3 = sp;
	sp4 = ccwa (sp, face);
    }
    ASSERT (sp3 && sp4);

    if ((d3 = nom_x - sp3 -> nom_x) < 0) d3 = -d3;
    if ((d4 = nom_x - sp4 -> nom_x) < 0) d4 = -d4;

    if (d3 < d4 && d3 < 4) {
	ASSERT (sp3 != sp1);
	return (sp3);
    }
    else if (d4 < 4) {
	ASSERT (sp4 != sp2);
	return (sp4);
    }
    sp = meshSplitEdge (sp3, sp4, act_x, act_y, sp1 -> act_z, nom_x, nom_y, sp1 -> nom_z);
    return (sp);
}

Private void splitFace (face_t *face, meshCoor_t nom_v)
{
    face_t *newf;
    spider_t *sp1, *sp2, *sp3, *sp4, *sp5, *sp6;

    // split hor.edge
    if (SP(0) -> nom_x != SP(1) -> nom_x) { // hor.edge
	sp1 = SP(0); sp2 = SP(1); sp3 = SP(2); sp4 = SP(3);
    } else {
	sp1 = SP(1); sp2 = SP(2); sp3 = SP(3); sp4 = SP(0);
	ASSERT (sp1 -> nom_x != sp2 -> nom_x);
    }
    sp5 = splitPath (face, sp1, sp2, nom_v);
    sp6 = splitPath (face, sp3, sp4, nom_v);
    newf = meshRenameFace (sp5, sp6, face);
    setLastFace ();
    meshMakeEdge (sp5, sp6, INTERNALEDGE);
    meshSetFaces (sp5, sp6, face, newf);
    meshSetCorners (newf, sp6, sp5, sp2, sp3);
    meshSetCorners (face, sp4, sp1, sp5, sp6);
}

Private void reconstructFaceI (face_t *face)
{
    face_t *f2;
    spiderEdge_t *eh;
    spider_t *sp, *sp2;
    int i, j, k;

    if (!(face -> type & (FACE_BOT+FACE_TOP))) return;
    if (!SP(3)) return; /* triangle */
    i = 0;
    sp2 = SP(i);
    do {
	sp = sp2;
	for (eh = sp -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp, eh));
	ASSERT (eh);
	sp2 = OTHER_SPIDER (sp, eh);
	for (j = 0; j < 4; j++) if (SP(j) == sp2) break;
	if (i < 4 || j < 4) { // corner
	    if (sp2 -> nom_x != sp -> nom_x) { // hor.edge
		f2 = eh -> oh -> face;
#if 0
		if (f2 && (f2 -> type & (FACE_BOT+FACE_TOP)) && SP2(3)) { /* trapezoid */
#else
		if (f2  && f2 -> type == face -> type && SP2(3)) { /* trapezoid */
#endif
		    if (i < 4) { // sp = corner spider
			for (k = 0; k < 4; k++) if (SP2(k) == sp) break;
			if (k >= 4) { // sp not corner spider of f2
			    splitFace (f2, sp -> nom_x);
			    if (j < 4) f2 = eh -> oh -> face;
			}
		    }
		    if (j < 4) { // sp2 = corner spider
			for (k = 0; k < 4; k++) if (SP2(k) == sp2) break;
			if (k >= 4) { // sp2 not corner spider of f2
			    splitFace (f2, sp2 -> nom_x);
			}
		    }
		}
	    }
	}
    } while ((i = j) != 0);
}

Private void reconstructFaceY (face_t *face)
{
    face_t *f2;
    spiderEdge_t *eh;
    spider_t *sp, *sp2, *sp3;
    int i, j, k, l, m, n, dy, dy2;

    if (!(face -> type & (FACE_BOT+FACE_TOP))) return;
    n = SP(3)? 3 : 2;
again:
    i = 0;
    sp2 = SP(i);
    do {
	sp = sp2;
	for (eh = sp2 -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp2, eh));
	ASSERT (eh);
	sp2 = OTHER_SPIDER (sp2, eh);
	for (j = 0; j < 4; j++) if (SP(j) == sp2) break;
	if (j < 4 && i < 4 && sp -> nom_x != sp2 -> nom_x) { // corners && hor.edge
	    f2 = eh -> oh -> face;
	    if (f2  && f2 -> type == face -> type) {
		m = SP2(3)? 3 : 2;
		for (k = 0; k <= m; k++) if (SP2(k) == sp) {
		    l = k; if (--l < 0) l = m;
		    if (SP2(l) == sp2) { // match
			if (++k > m) k = 0;
			if (--l < 0) l = m;
			m = 0;
			if (n == 2) /* face is triangle */
			     sp3 = (j == 2)? SP(0) : SP(j+1);
			else sp3 = NULL;
			if (SP2(k) -> nom_x == sp -> nom_x) { SP(i) = SP2(k);
			    if (sp3 && sp3 -> nom_x != sp -> nom_x) m = 1;
			}
			if (SP2(l) -> nom_x == sp2-> nom_x) { SP(j) = SP2(l);
			    if (sp3 && sp3 -> nom_x == sp -> nom_x) m = 1;
			}
			if (m) { // make face a trapezoid
			    if (sp3 -> nom_x != sp -> nom_x) sp2 = SP(j);
			    else { sp = SP(i); i = j; }
			    SP(0) = SP(i);
			    SP(1) = sp2;
			    SP(2) = sp3;
			    SP(3) = sp;
			    n = 3; /* trapezoid */
			}
			removeFace2 (f2, face, eh);
			goto again;
		    }
		    break;
		}
	    }
	}
    } while ((i = j) != 0);
}

Private void reconstructFaceX (face_t *face)
{
    face_t *f2;
    spiderEdge_t *eh;
    spider_t *sp, *sp2, *sp3;
    int i, j, k, l, m, n, dy, dy2;

    if (!(face -> type & (FACE_BOT+FACE_TOP))) return;
    n = SP(3)? 3 : 2;
again:
    i = 0;
    sp2 = SP(i);
    do {
	sp = sp2;
	for (eh = sp2 -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp2, eh));
	ASSERT (eh);
	sp2 = OTHER_SPIDER (sp2, eh);
	for (j = 0; j < 4; j++) if (SP(j) == sp2) break;
	if (j < 4 && i < 4) { // corners
	    f2 = eh -> oh -> face;
	    if (f2  && f2 -> type == face -> type) {
		m = SP2(3)? 3 : 2;
		if (sp -> nom_x == sp2 -> nom_x) { // ver.edge
		    for (k = 0; k <= m; k++) if (SP2(k) == sp) {
			l = k; if (--l < 0) l = m;
			if (SP2(l) == sp2) { // match
			    if (++k > m) k = 0;
			    if (--l < 0) l = m;

			    sp3 = (i > 0)? SP(i-1) : SP(n);
			    if (sp3 -> nom_y == sp -> nom_y) dy = 0;
			    else dy = (sp3 -> nom_y > sp -> nom_y)? 1 : -1;
			    sp3 = SP2(k);
			    if (sp -> nom_y == sp3 -> nom_y) dy2 = 0;
			    else dy2 = (sp -> nom_y > sp3 -> nom_y)? 1 : -1;
			    if (dy2 != dy) break;

			    sp = (j < n)? SP(j+1) : SP(0);
			    if (sp -> nom_y == sp2 -> nom_y) dy = 0;
			    else dy = (sp -> nom_y > sp2 -> nom_y)? 1 : -1;
			    sp3 = SP2(l);
			    if (sp2 -> nom_y == sp3 -> nom_y) dy2 = 0;
			    else dy2 = (sp2 -> nom_y > sp3 -> nom_y)? 1 : -1;
			    if (dy2 != dy) break;

			    if (SP2(k) == sp3) {
				ASSERT (m == 2); /* f2 is triangle */
				if (--i < 0) i = n;
				SP(0) = SP(i);
				SP(1) = sp3;
				SP(2) = sp; // j+1
				SP(3) = NULL;
				n = 2; /* triangle */
			    }
			    else {
				SP(i) = SP2(k);
				SP(j) = sp3;
			    }
			    removeFace2 (f2, face, eh);
			    goto again;
			}
			break;
		    }
		}
	    }
	}
    } while ((i = j) != 0);
}

Private void reconstructFaceS (face_t *face) /* remove vertical edges from sidewall faces */
{
    face_t *f2;
    spiderEdge_t *eh;
    spider_t *sp, *sp2, *spk;
    int i, j, k, l, dx, dx2;

    if (face -> type & (FACE_BOT+FACE_TOP+FACE_CSIDE)) return;

    if (!(spk = SP(3))) return; /* face is triangle */

    i = 0; sp  = SP(i);
    j = 1; sp2 = SP(j);
    if (sp2 -> nom_z == sp -> nom_z) { // hor.edge
	spk = sp;
	i = 1; sp  = sp2;
	j = 2; sp2 = SP(j);
	ASSERT (sp2 -> nom_z != sp -> nom_z);
    }
    // sp <-> sp2 is vertical edge

    // face orientation
    if (sp -> nom_x != spk -> nom_x) {
	if (sp -> nom_y != spk -> nom_y) {
	    if (sp -> nom_x < spk -> nom_x)
		 dx = (sp -> nom_y > spk -> nom_y)? 2 : 3;
	    else dx = (sp -> nom_y > spk -> nom_y)? 3 : 2;
	}
	else dx = 1;
    }
    else dx = 0;

again:
    for (eh = sp -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp, eh));
    ASSERT (eh);
    f2 = eh -> oh -> face;
    if (f2  && f2 -> type == face -> type && SP2(3)) { /* f2 is trapezoid */
	ASSERT (OTHER_SPIDER (sp, eh) == sp2); // no extra spider on ver.edge

	for (k = 0; k < 4; k++) if (SP2(k) == sp) break;
	ASSERT (k < 4); // SdeG -- f2 moet corner op dezelfde z hebben
	if (k < 4) {
	    l = k; if (--l < 0) l = 3;
	    ASSERT (SP2(l) == sp2); // SdeG -- f2 corner moet kloppen
	    if (SP2(l) == sp2) { // match
		if (++k > 3) k = 0;
		spk = SP2(k);
		if (spk -> nom_x != sp -> nom_x) {
		    if (spk -> nom_y != sp -> nom_y) {
			if (spk -> nom_x < sp -> nom_x)
			     dx2 = (spk -> nom_y > sp -> nom_y)? 2 : 3;
			else dx2 = (spk -> nom_y > sp -> nom_y)? 3 : 2;
		    }
		    else dx2 = 1;
		}
		else dx2 = 0;

		if (dx2 == dx) {
		    if (--l < 0) l = 3;
		    SP(i) = sp  = spk;
		    SP(j) = sp2 = SP2(l);
		    removeFace2 (f2, face, eh);
		    goto again;
		}
	    }
	}
    }
}

/*
 * Refine the mesh in stripR
 */
void meshRefine ()
{
    face_t *face;

    Debug (fprintf (stderr, "refine mesh %s\n", sprintfStripRcoor ()));

    if (!faceInitEnumerateR ()) return;

    if (new_refine && !FeModePwl && !prePass1) {
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) reconstructFaceI (face);
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) reconstructFaceY (face);
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) reconstructFaceX (face);
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) reconstructFaceS (face);
	findCenterSpiderInit ();
    }

    faceInitEnumerate (); /* Insert faces to be refined into pq */
    while ((face = faceEnumerate ())) {
	/* Debug (meshPrintFace (face)); */
	if (face -> type & FACE_CORE) removeFace (face);
	else {
	    face -> pqIndex = 0; /* init pq index */
	    if (face -> type & FACE_SIDE) {
		/* Split faces that cross a dielectric boundary */
		/* ASSERT (greenCase == DIEL); */
		refineSidewall (face);
	    }
	    else if (!SP(3) || convexFace (face)) faceRecur (face);
	}
    }

    /* Retrieve faces in largest first order and split quadrilateral
     * faces until they are small enough for triangularization. */
    while ((face = pqDeleteHead ())) {
	if (SP(3)) refineTrapezoid (face);
	else {
	    if (SIMPLE_TRIANGLE (face))
		refineTriangle (face);
	    else
		triangulate (face);
	}
    }

    if (FeShape == 3) { /* FeModePwl */
	/* Make simple triangular faces.
	 * Too large simple triangles put into pq.
	 */
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) {
	    if (SP(3) || !SIMPLE_TRIANGLE (face)) triangulate (face);
	}

	/* Refine too large simple triangles.
	 */
	FeShape = 0;
	while ((face = pqDeleteHead ())) {
	    if (SIMPLE_TRIANGLE (face))
		refineTriangle (face);
	    else
		triangulate (face);
	}
	FeShape = 3;
    }

    /*	Now follows a trick that is used for collocation (see notes)
	with a constant charge on each face. For each finite element,
	a new spider is created that points to the face.
	These new spiders are then used in traversing the strips.
	This can not be skipped with order-1 finite elements, since the
	faces would not be deallocated when they were not necessary anymore.
	This would give problems when extracting multiple cells.
    */
    faceInitEnumerate ();
    while ((face = faceEnumerate ())) meshAddExtraSpider (face);
}

/* Rename the face which is found by ccw traversal from sp1 to sp2.
 * The spider pointer of the new face is set to sp1.
 * Return the new face.
 * NOTE: MeshSetCorners should be performed by caller.
 */
face_t *meshRenameFace (spider_t *sp1, spider_t *sp2, face_t *fc)
{
    face_t *newfc = newFace ();
    newfc -> sc_subn = fc -> sc_subn;
    newfc -> type = fc -> type;
    while ((sp1 = meshCcwAdjacent (sp1, fc, newfc)) != sp2);
    stripAddFace (newfc, stripR);
    return (newfc);
}

Private void splitTriangle (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *spN)
{
    face_t *newf;

    newf = newFace ();
    newf -> sc_subn = face -> sc_subn;
    newf -> type = face -> type;
    meshMakeEdge (sp3, spN, INTERNALEDGE);
    meshSetFaces (sp3, spN, face, newf);
    meshSetFace (sp3, sp1, newf);
    meshSetFace (sp1, spN, newf);
    meshSetCorners (face, spN, sp2, sp3, NULL);
    meshSetCorners (newf, spN, sp3, sp1, NULL);
    if (isKnown (face)) { /* already refined */
	feSize (face);
	feSize (newf); stripAddFace (newf, stripL);
    } else {
	faceRecur (face);
	faceRecur (newf); stripAddFace (newf, stripR);
    }
}

/* Refine a triangle face that is too large.
 * The algorithm is to split the longest edge,
 * leading to this triangle being split in two
 * but if there is another triangle bordered by the same edge,
 * this one is also split.
 */
Private void refineTriangle (face_t *face)
{
    face_t *f1, *f2;
    spider_t *sp, *sp1, *sp2, *sp3, *sp4;
    spiderEdge_t *edge;
    meshCoor_t d1, d2, d3, maxFeArea, maxLength;
    meshCoor_t x, y, z, nom_x, nom_y, nom_z;

    /* check if face was refined while it was in the queue
     */
    if (face -> type & FACE_COARSE) {
	maxFeArea = spiderControl.maxCoarseFeArea;
	maxLength = spiderControl.maxCoarseSpiderLength;
    }
    else if (isEdgeFace (face)) {
	maxFeArea = spiderControl.maxEdgeFeArea;
	maxLength = spiderControl.maxSpiderLength;
    }
    else {
	maxFeArea = spiderControl.maxFeArea;
	maxLength = spiderControl.maxSpiderLength;
    }

    if (face -> len  <= maxLength * SMALL_INC
    &&  face -> area <= maxFeArea * SMALL_INC) return; /* stop refinement */

    sp1 = face -> corners[0];
    sp2 = face -> corners[1];
    sp3 = face -> corners[2];

    /* Can save these, by storing longest edge in face
     */
    d1 = spiderDist (sp1, sp2);
    d2 = spiderDist (sp2, sp3);
    d3 = spiderDist (sp3, sp1);

    if (d1 >= d2 && d1 >= d3) {
	edge = meshFindEdge (sp1, sp2);
    }
    else if (d2 >= d1 && d2 >= d3) {
	edge = meshFindEdge (sp2, sp3);
	sp = sp1; sp1 = sp2; sp2 = sp3; sp3 = sp;
    }
    else {
	edge = meshFindEdge (sp3, sp1);
	sp = sp1; sp1 = sp3; sp3 = sp2; sp2 = sp;
    }
    ASSERT (edge);

    /* meshRefineEdge:
     * The input is an edge that is part of 1 or 2 triangles.
     * Split the edge halfway, and make from the newly created
     * spider extra edges in the triangles at both sides.
     * (at least one such triangle must exist)
     * Thus, as a result there are 2 or 4 triangles.
     */
    f1 = face;
    ASSERT (f1 == edge -> face);
    ASSERT (ccwa (sp2, f1) == sp3);
    ASSERT (ccwa (sp3, f1) == sp1);
    Debug2 (meshCheckTriangle (sp1, sp2, sp3, f1));

    /* Make the coordinate of the new spider the average
     * of the coordinates of sp1 and sp2.
     */
    x = (sp1 -> act_x + sp2 -> act_x) / 2;
    y = (sp1 -> act_y + sp2 -> act_y) / 2;
    z = (sp1 -> act_z + sp2 -> act_z) / 2;

    nom_x = (sp1 -> nom_x + sp2 -> nom_x) / 2;
    nom_y = (sp1 -> nom_y + sp2 -> nom_y) / 2;
    nom_z = (sp1 -> nom_z + sp2 -> nom_z) / 2;

    /* Mesh Smoothing
     * If both triangles (left and right of edge) exist, and
     * if they are in the same plane, make the coordinates of
     * the new spider equal to the average of all 4 spiders.
     * Note: This does not capture the side walls of
     * the conductors, since the test for same plane
     * actually tests on z-coordinates only.
     *
     * A possibility is to take the inproduct of 2 normal vectors.
     */
    sp = meshSplitEdge (sp1, sp2, x, y, z, nom_x, nom_y, nom_z);

    splitTriangle (f1, sp1, sp2, sp3, sp);

    if (FeModePwl && (f2 = edge -> oh -> face)) {
	if (f2 -> area > 0 && (sp4 = cwa (sp2, f2)) && cwa (sp4, f2) == sp1) {
	    Debug2 (meshCheckTriangle (sp4, sp2, sp1, f2));
	    if (isKnown (f2)) splitTriangle (f2, sp2, sp1, sp4, sp);
	    else if (isRight (f2) && !FeShape && !f2 -> pqIndex) pqInsert (f2);
	}
    }
}

/* Represent each fe by a spider in the center of gravity of the fe.
 * This is used for 0-order fe's, where the faces form the finite
 * elements. They are now represented by the extra spider.
 * This is also used to throw away all faces,
 * with 0 as well as 1-th order finite elements.
 */
Private void meshAddExtraSpider (face_t *face)
{
    int conductor;
    meshCoor_t x, y, z, nom_x, nom_y, nom_z;
    spider_t *sp1 = face -> corners[0];
    spider_t *sp2 = face -> corners[1];
    spider_t *sp3 = face -> corners[2];
    spider_t *sp4 = face -> corners[3];
    spider_t *spe;
    subnode_t *subnode, *subnodeSC = NULL;
    int isGate;

    /* the face must be ready, but not yet be split. */

    if (!sp4) { /* face is triangle */
	/* Center of gravity of triangle
	 */
	x = (sp1 -> act_x + sp2 -> act_x + sp3 -> act_x) / 3.0;
	y = (sp1 -> act_y + sp2 -> act_y + sp3 -> act_y) / 3.0;
	z = (sp1 -> act_z + sp2 -> act_z + sp3 -> act_z) / 3.0;

	nom_x = (sp1 -> nom_x + sp2 -> nom_x + sp3 -> nom_x) / 3.0;
	nom_y = (sp1 -> nom_y + sp2 -> nom_y + sp3 -> nom_y) / 3.0;
	nom_z = (sp1 -> nom_z + sp2 -> nom_z + sp3 -> nom_z) / 3.0;

	isGate = (sp1 -> isGate & sp2 -> isGate & sp3 -> isGate);
    }
    else { /* face is trapezoid */
	x = (sp1 -> act_x + sp2 -> act_x + sp3 -> act_x + sp4 -> act_x) / 4.0;
	y = (sp1 -> act_y + sp2 -> act_y + sp3 -> act_y + sp4 -> act_y) / 4.0;
	z = (sp1 -> act_z + sp2 -> act_z + sp3 -> act_z + sp4 -> act_z) / 4.0;

	nom_x = (sp1 -> nom_x + sp2 -> nom_x + sp3 -> nom_x + sp4 -> nom_x) / 4.0;
	nom_y = (sp1 -> nom_y + sp2 -> nom_y + sp3 -> nom_y + sp4 -> nom_y) / 4.0;
	nom_z = (sp1 -> nom_z + sp2 -> nom_z + sp3 -> nom_z + sp4 -> nom_z) / 4.0;

	isGate = (sp1 -> isGate & sp2 -> isGate & sp3 -> isGate & sp4 -> isGate);
    }

    conductor = sp1 -> conductor;

    /* It doesn't make sense to take the subnode of the closest spider since
       all spiders are equal close.
       Instead we choose the subnode that is in the right upper corner to be
       sure that it belongs to the tile that contains the face (see also the
       way Find () is called in sppair.c).
    */
    if (prePass1) { /* optSubRes */
	/* For substrate, the node cannot always correctly be found by
	   edge spiders (this can happen by triangles).
	   Thus the face is used to find the substrate contact subnode.
	*/
	subnode = face -> sc_subn;
	ASSERT (subnode);
    }
    else
    if (!FeModePwl) { /* Only for Pwc */
	if (sp3 -> conductor != conductor && new_via_mode) { // via
	    if (sp3 -> nom_z > sp1 -> nom_z) { sp1 = sp3; conductor = sp3 -> conductor; }
	    else ASSERT (sp1 -> nom_z > sp3 -> nom_z);
	}
	if (new_refine && conductor != nrOfCondStd && !optEstimate3D) {
	    tile_t *tile;
	    nodePoint_t *np = findCenterSpider ((int)(face -> type & FACE_SIDE),
				    (double)nom_x, (double)nom_y, conductor, &tile);
	    subnode = np ? np -> cons[conductor] : tile -> cons[conductor];
	    if (connect_ground) {
		subnodeSC = subnSUB;
		if (connect_ground == 2 && tile -> subcont) { // distributed
		    subnodeSC = tile -> subcont -> subn;
		    ASSERT (subnodeSC);
		}
	    }
	}
	else {
#define MORE_XY(sp) if (((new_via_mode && sp -> conductor == conductor) || !new_via_mode) && (\
			sp -> nom_x > sp1 -> nom_x || (sp -> nom_x == sp1 -> nom_x &&\
			sp -> nom_y > sp1 -> nom_y))) sp1 = sp;
	    MORE_XY (sp2);
	    MORE_XY (sp3);
	    if (sp4) MORE_XY (sp4);
	    subnode = sp1 -> subnode;
	    subnodeSC = face -> sc_subn;
	}
	ASSERT (subnode);
    }
    else subnode = NULL;

    /* Make a new spider in the center of gravity of triangle or trap.
     * Mark the spider as being special, it in fact represents the entire face.
     */
    spe = newSpider (nom_x, nom_y, nom_z, x, y, z, -1, subnode, subnodeSC, conductor, isGate);
    spe -> face = face;
    stripAddSpider (spe);
    face -> type |= FACE_KNOWN; /* face is represented by a spider */
}

/* Mesh refinement recursion by entering refined face
 * into priority queue if necessary and if it is not already there.
 */
void faceRecur (face_t *face)
{
    static int mess_done2 = 0;
    meshCoor_t maxFeArea, maxLength;

    feSize (face);

    /* insert face in pq, if it is too big */

    if (face -> area < spiderControl.minFeArea) {
	if (mess_done2++ < 10) {
	    spider_t *sp = face -> corners[0];
	    say ("warning: too small face at x=%g y=%g z=%g micron (area=%g)",
		Microns (sp -> act_x), Microns (sp -> act_y), Microns (sp -> act_z), Microns2 (face -> area));
	}
	removeFace (face);
	return;
    }

    if (face -> type & FACE_COARSE) {
	maxFeArea = spiderControl.maxCoarseFeArea;
	maxLength = spiderControl.maxCoarseSpiderLength;
    }
    else if (spiderControl.maxEdgeFeArea != spiderControl.maxFeArea && isEdgeFace (face)) {
	maxFeArea = spiderControl.maxEdgeFeArea;
	maxLength = spiderControl.maxSpiderLength;
    }
    else {
	maxFeArea = spiderControl.maxFeArea;
	maxLength = spiderControl.maxSpiderLength;
    }

    if (FeShape == 3 && SP(3)) { /* FeModePwl */
       /* This quadrilateral face will ultimatly be split into triangles.
	* However, a better mesh results when it remains a quadrilateral
	* as long as possible.  The criterium to split the face depends on
	* aspect ratio.  If the face is a square, use a factor of 4.
	* This will lead to the face being split into 4 equal triangles.
	* If the face is non-square, let the factor depend on the aspect ratio.
	* This is a configuration parameter, we can avoid sharp triangles by
	* adjusting f (see line XXX), at the cost of creating too small triangles.
	*/
	meshCoor_t f = 4 * face -> len2 / face -> len; /* f <= 4 */
	if (f < 2) f = 2; /* XXX */
	maxFeArea *= f;
    }

    if (face -> len  > maxLength * SMALL_INC
    ||  face -> area > maxFeArea * SMALL_INC) {
	pqInsert (face);
    }
    else if (maxFeRatio > 0 && SP(3) && FeShape != 3) {
	if (face -> len2 > SP_EPS)
	if (face -> len / face -> len2 > maxFeRatio) pqInsert (face);
    }
}

/* Return the area and length of longest edge of a finite element;
 * It is possible to save the square root by comparing with squared areas.
 */
void feSize (face_t *face)
{
    meshCoor_t a, b, c, d, e, s, l, m;
    spider_t *sp1, *sp2, *sp3, *sp4;

    sp1 = SP(0);
    sp2 = SP(1);
    sp3 = SP(2);
    sp4 = SP(3);

    a = spiderDist (sp1, sp2);
    b = spiderDist (sp2, sp3);
    c = spiderDist (sp3, sp1);
    l = Max (a, b);
    m = Min (a, b);

    /* compute triangle area cf. Schaum (s = semiperimeter)
     */
    s = (a + b + c) / 2;
    a = sqrt ((double) (s * (s - a) * (s - b) * (s - c)));

    if (sp4) { /* quadrilateral */
	/* compute area as if it are two triangles
	 */
	d = spiderDist (sp3, sp4);
	e = spiderDist (sp4, sp1);
	if (d < m) m = d;
	if (e < m) m = e;

	s = (c + d + e) / 2;
	a += sqrt ((double) (s * (s - c) * (s - d) * (s - e)));
	c = Max (d, e);
    }
    else if (c < m) m = c;

    l = Max (l, c);

 /* ASSERT (a > 0); */
 /* ASSERT (l > 0); */

    face -> area = a;
    face -> len  = l; /* max length of face */
    face -> len2 = m; /* min length of face */
}

/* FVF: This routine returns the slope between sp1 and sp2 in either
 * the xy, xz or yz plane. The plane used is the one the spiders are
 * "closest" to, meaning the one where the distance between them is
 * the greatest. This is used to chose the perferred diagonal to split
 * faces over in meshSplitFace().
 */
meshCoor_t weirdoSlope (spider_t *sp1, spider_t *sp2)
{
    meshCoor_t dx = sp1 -> act_x - sp2 -> act_x;
    meshCoor_t dy = sp1 -> act_y - sp2 -> act_y;
    meshCoor_t dz = sp1 -> act_z - sp2 -> act_z;

    /* is x1 smaller than x2 and x3? */
#define Smallest(a,b,c) ((Abs(a) <= Abs(b)) && (Abs(a) <= Abs(c)))

    if (Smallest (dz, dx, dy)) {
	return dy/dx;
    } else if (Smallest (dy, dx, dz)) {
	return dx/dz;
    } else {
	ASSERT (Smallest (dx, dy, dz)); /* this is not a vital assertion */
	return dz/dy;
    }
}

//#define DEBUG_CROSSING

Private spider_t *meshSplitPath2 (face_t *face, spider_t *sp1, spider_t *sp2)
{
    meshCoor_t x, y;
    spider_t *sp, *sp3;

 // ASSERT (sp2 -> nom_x - sp1 -> nom_x == 0);
 // ASSERT (sp2 -> nom_y - sp1 -> nom_y == 0);

    sp3 = sp1;
    for (sp = ccwa (sp1, face); sp != sp2; sp = ccwa (sp, face)) {
	if (sp -> act_z >= crossing_z09 && sp -> act_z <= crossing_z11) return (sp);
	if (sp2 -> act_z > sp1 -> act_z) {
	    if (sp -> act_z > crossing_z11) break;
	}
	else if (sp -> act_z < crossing_z09) break;
	sp3 = sp;
    }
    if (sp -> act_z >= crossing_z09 && sp -> act_z <= crossing_z11) return (sp);

#ifdef DEBUG_CROSSING
    fprintf (stderr, "meshSplitPath2: meshSplitEdge\n");
#endif
    distRatio = (crossing_z - sp1 -> act_z) / (sp2 -> act_z - sp1 -> act_z);
    x = sp1 -> act_x + (sp2 -> act_x - sp1 -> act_x) * distRatio;
    y = sp1 -> act_y + (sp2 -> act_y - sp1 -> act_y) * distRatio;
    sp = meshSplitEdge (sp3, sp, x, y, crossing_z, sp1 -> nom_x, sp1 -> nom_y, crossing_z);

    return (sp);
}

#define Ci(n) ((n)%k)
#define Sp(n) corners[Ci(n)]

Private void refineSidewall (face_t *face)
{
    static int mess_done1 = 0;
    face_t *new_face;
    spider_t *sp0, *sp1, *sp2, *sp3;
    spider_t *sp5, *sp6, **corners;
    int i, j, imax, k, next;

    corners = face -> corners;
    k = corners[3] ? 4 : 3;

    if (face -> type & FACE_CSIDE) { /* check cross-over sidewall face */
	int j1 = 0, j3 = 0, n = 0, n1 = 0, n3 = 0;

	for (i = (k == 3 ? 3 : 1); i <= k; ++i) {
	    j = getDirection (Sp(i-1), Sp(i), Sp(i+1), (face -> type & FACE_VSIDE));
	    if (j == 0 || j == 2) {
		if (k == 3 || n) { /* too small face area */
		    removeFace (face);
		    return;
		}
		n = i;
	    }
	    else if (j == 1)   { n1 = i; ++j1; }
	    else  /* j == 3 */ { n3 = i; ++j3; }
	}
	if (j1 == j3) {
	    if (mess_done1++ < 10) {
		say ("warning: incorrect cross-over sidewall face at x=%g y=%g z=%g micron",
		  Microns (corners[0] -> act_x), Microns (corners[0] -> act_y), Microns (corners[0] -> act_z));
	    }
	    removeFace (face);
	    return;
	}
	if (k == 4) {
	    if (j1 && j3) n = j3 > j1 ? n1 : n3;
	    if (n) { /* split 3 points on same line or concave corner */
		sp6 = Sp(n);
		sp5 = Sp(n+2);
		new_face = meshRenameFace (sp5, sp6, face);
		meshMakeEdge (sp5, sp6, INTERNALEDGE);
		meshSetFaces (sp5, sp6, face, new_face);
		meshSetCorners (new_face, Sp(n-1), sp6, sp5, NULL);
		meshSetCorners (face, Sp(n+1), sp5, sp6, NULL);
		refineSidewall (face);
		refineSidewall (new_face);
		return;
	    }
	}
    }

    next = 0;
    while (faceCrossesDielBoundary (face, next)) {
	next = 1;

	/* This is a case where we have to split because
	   we cross a dielectric boundary. */

#ifdef DEBUG_CROSSING
fprintf (stderr, "\nrefineSidewall: crossingIndex=%d crossing_z=%g\n", crossingIndex, crossing_z);
fprintf (stderr, "refineSidewall: corners[0]=%g,%g,%g\n", corners[0]->nom_x, corners[0]->nom_y, corners[0]->nom_z);
fprintf (stderr, "refineSidewall: corners[1]=%g,%g,%g\n", corners[1]->nom_x, corners[1]->nom_y, corners[1]->nom_z);
fprintf (stderr, "refineSidewall: corners[2]=%g,%g,%g\n", corners[2]->nom_x, corners[2]->nom_y, corners[2]->nom_z);
if (corners[3])
fprintf (stderr, "refineSidewall: corners[3]=%g,%g,%g\n", corners[3]->nom_x, corners[3]->nom_y, corners[3]->nom_z);
#endif
	imax = crossingIndex;

	if (!corners[3]) { /* triangle */
	    k = 3;
	    ASSERT (imax < k);
	    sp0 = corners[0];
	    sp1 = corners[1];
	    sp2 = corners[2];
	    sp5 = meshSplitPath2 (face, Sp(imax), Sp(imax+1));
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp5=%g,%g,%g\n", sp5->act_x, sp5->act_y, sp5->act_z);
#endif
	    if (imax == 2) {
		sp6 = sp1;
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp6=%g,%g,%g\n", sp6->act_x, sp6->act_y, sp6->act_z);
#endif
		new_face = meshRenameFace (sp5, sp6, face);
		meshMakeEdge (sp5, sp6, INTERNALEDGE);
		meshSetFaces (sp5, sp6, face, new_face);
		meshSetCorners (new_face, sp0, sp6, sp5, NULL);
		meshSetCorners (face,     sp2, sp5, sp6, NULL);
	    }
	    else if (imax == 1) {
		sp6 = meshSplitPath2 (face, sp2, sp0);
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp6=%g,%g,%g\n", sp6->act_x, sp6->act_y, sp6->act_z);
#endif
		new_face = meshRenameFace (sp5, sp6, face);
		meshMakeEdge (sp5, sp6, INTERNALEDGE);
		meshSetFaces (sp5, sp6, face, new_face);
		meshSetCorners (new_face, sp2, sp6, sp5, NULL);
		if (sp6 != sp0)
		    meshSetCorners (face, sp1, sp5, sp6, sp0);
		else
		    meshSetCorners (face, sp1, sp5, sp6, NULL);
	    }
	    else {
		i = 0;
		if (sp0->nom_z > sp1->nom_z) {
		    if (sp2->nom_z <= sp1->nom_z
		     || (sp2->nom_z < sp0->nom_z && crossing_z > sp2->nom_z)) ++i;
		}
		else {
		    if (sp2->nom_z >= sp1->nom_z
		     || (sp2->nom_z > sp0->nom_z && crossing_z < sp2->nom_z)) ++i;
		}
		sp6 = meshSplitPath2 (face, Sp(i+1), Sp(i+2));
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp6=%g,%g,%g\n", sp6->act_x, sp6->act_y, sp6->act_z);
#endif
		new_face = meshRenameFace (sp5, sp6, face);
		meshMakeEdge (sp5, sp6, INTERNALEDGE);
		meshSetFaces (sp5, sp6, face, new_face);
		if (i) {
		    meshSetCorners (new_face, sp1, sp2, sp6, sp5);
		    meshSetCorners (face, sp0, sp5, sp6, NULL);
		}
		else {
		    meshSetCorners (new_face, sp1, sp6, sp5, NULL);
		    if (sp6 != sp2)
			meshSetCorners (face, sp0, sp5, sp6, sp2);
		    else
			meshSetCorners (face, sp0, sp5, sp6, NULL);
		}
	    }
	}
	else {
	    k = 4;
	    ASSERT (imax < k);
	    sp0 = Sp(imax);
	    sp1 = Sp(imax+1);
	    sp2 = Sp(imax+2);
	    sp3 = Sp(imax+3);
	    sp5 = meshSplitPath2 (face, sp0, sp1);
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp5=%g,%g,%g\n", sp5->act_x, sp5->act_y, sp5->act_z);
#endif
	    if (crossing_z < sp0->nom_z) {
		if (crossing_z09 <= sp2->nom_z) {
		    sp6 = meshSplitPath2 (face, sp1, sp2); i = 1; }
		else if (crossing_z09 <= sp3->nom_z) {
		    sp6 = meshSplitPath2 (face, sp2, sp3); i = 0; }
		else {
		    sp6 = meshSplitPath2 (face, sp3, sp0); i = 2; }
	    }
	    else { // crossing_z > imax
		if (crossing_z11 >= sp2->nom_z) {
		    sp6 = meshSplitPath2 (face, sp1, sp2); i = 1; }
		else if (crossing_z11 >= sp3->nom_z) {
		    sp6 = meshSplitPath2 (face, sp2, sp3); i = 0; }
		else {
		    sp6 = meshSplitPath2 (face, sp3, sp0); i = 2; }
	    }
#ifdef DEBUG_CROSSING
fprintf (stderr, "sp6=%g,%g,%g\n", sp6->act_x, sp6->act_y, sp6->act_z);
#endif
	    new_face = meshRenameFace (sp5, sp6, face);
	    meshMakeEdge (sp5, sp6, INTERNALEDGE);
	    meshSetFaces (sp5, sp6, face, new_face);

	    if (i == 1) { /* triangle direction 1 */
		meshSetCorners (new_face, sp6, sp5, sp1, NULL);
		meshSetCorners (face, sp3, sp0, sp5, sp2);
	    }
	    else if (i == 2 || sp6 == sp3) { /* triangle direction 2 */
		if (sp5 != sp1)
		    meshSetCorners (new_face, sp3, sp5, sp1, sp2);
		else
		    meshSetCorners (new_face, sp3, sp5, sp2, NULL);
		meshSetCorners (face, sp0, sp5, sp6, NULL);
	    }
	    else { /* quadrilateral */
		meshSetCorners (new_face, sp6, sp5, sp1, sp2);
		meshSetCorners (face, sp3, sp0, sp5, sp6);
	    }
	}

	if (crossing_z > sp0->nom_z) {
	    faceRecur (face);
	    face = new_face;
	    corners = face -> corners;
	}
	else {
	    faceRecur (new_face);
	}
    }
    faceRecur (face);
}

/* This operation splits a quadrilateral face into 2 others.
 * The direction of splitting is based on the aspect ratio.
 * Eventually, the face is split into two triangles.
 */
Private void refineTrapezoid (face_t *face)
{
    face_t *new_face;
    int i, imax, iedge;
    meshCoor_t max, min, d, edge, maxFeArea;
    spider_t *sp5, *sp6, **corners;
    int iseVal, k;

    Debug2 (meshCheckLoop (face));

    /* save old corners, these are overwritten below,
     * by meshSetCorners.
     */
    corners = face -> corners;
    k = 4;

    if (face -> type & FACE_COARSE) {
	iseVal = 0;
	maxFeArea = spiderControl.maxCoarseFeArea;
    }
    else {
	iseVal = isEdgeFace (face);
	if (iseVal) {
	    maxFeArea = spiderControl.maxEdgeFeArea;
	    iseVal &= 15; /* strip meshCorner bit */
	}
	else
	    maxFeArea = spiderControl.maxFeArea;
    }

    /* We are going to determine the split line of the face.
       Normally, we try to split it perpendicular to its longest side.
       If the face is part of an edge of the element mesh, we split it
       such that the split line is parallel to the edge of the mesh.
       With the latter, we only allow faces for which (long side)
       < spiderControl.maxEdgeFeRatio * (short side) to be split along
       the longest side.
       To see which edge of the face is an edge of the element mesh,
       we use the information that is encoded in the return value of isEdgeFace().
       Since isEdgeFace() traverses the sides counter-clock wise,
       Sp(0), Sp(1), Sp(2) and Sp(3) must also enumerate the corners
       counter-clock wise.
    */

    max = min = spiderDist (Sp(0), Sp(1));
    imax = iedge = 0;
    edge = 0; /* init, else compiler warning */

    for (i = 1; i <= 3; ++i) {
	d = spiderDist (Sp(i), Sp(i+1));
	if (iseVal == (1 << Ci(i+1))) { edge = d; iedge = i; }
	if (d > max) { if ((d - max) > 0.1) { max = d; imax = i; } } // rounding!
	else if (d < min) min = d;
    }

    if (iedge && iedge != imax) {
	if (edge * spiderControl.maxEdgeFeRatio > max) imax = iedge;
    }

    /* However, sometimes irregular meshes appear due to
     * differences between actual and nominal. This can
     * result in splitting the following quadrilateral:
     *
     *    sp1  __               If this is the case, the
     *        |  \_             edge between sp5 and sp6
     *        |    \_ sp4       lies very close to sp4
     * sp5 -> |      \ <- sp6   and can even be crossed
     *        |       \         by sp4 (if nominal and
     *        |        \        actual differ to much).
     *    sp2  --------- sp3
     *
     * As a solution, we can triangulate such face already
     * at this moment (along shortest diagonal) criterium
     * could be formed by using convexity or, as we do now,
     * the length of the shortest diagonal vs. lenght of
     * edge between the newly created sp5 and sp6.
     */

    if (!FeModePwl || face -> area > 2 * maxFeArea || max >= 2 * min
	|| iseVal == (1 << Ci(imax+1)) || iseVal == (1 << Ci(imax+3))) {
	/* Split into two quadrilaterals.
	 * In case of linear elements, they are triangulated in a later stage.
	 */
	distRatio = 0.5;
	if (iseVal && spiderControl.edgeSplitRatio != 0.5 &&
	    (face -> area * (1.0 - spiderControl.edgeSplitRatio) < spiderControl.maxFeArea)) {
	    /* Only one exterior edge? */
	    if (iseVal == (1 << Ci(imax+1)))
		distRatio = 1.0 - spiderControl.edgeSplitRatio;
	    else if (iseVal == (1 << Ci(imax+3)))
		distRatio = spiderControl.edgeSplitRatio;
	}
	sp5 = meshSplitPath (face, Sp(imax+0), Sp(imax+1));
	distRatio = 1.0 - distRatio;
	sp6 = meshSplitPath (face, Sp(imax+2), Sp(imax+3));
    }
    else {
	/* This is last subdivision, into 2 triangles.
	 * Split along shortest diagonal.
	 * Allow DEVIATION_FACTOR deviation to choose a split direction
	 * perpendicular to a corner of the element mesh. */
	/* FVF: Also allow DEVIATION_FACTOR deviation to chose the globally
	 * preffered direction, such that similar opposite faces
	 * will be split the same way. */
	meshCoor_t d1 = spiderDist (Sp(imax+0), Sp(imax+2));
	meshCoor_t d2 = spiderDist (Sp(imax+1), Sp(imax+3));
	meshCoor_t slope1 = weirdoSlope (Sp(imax+0), Sp(imax+2));
	meshCoor_t slope2 = weirdoSlope (Sp(imax+1), Sp(imax+3));
	int corner1 = 0;
	int corner2 = 0;

	if (iseVal) {
	    if ((iseVal & (1 << Ci(imax+0)) && iseVal & (1 << Ci(imax+1))
		&& !(iseVal & (1 << Ci(imax+2))) && !(iseVal & (1 << Ci(imax+3))))
	     || (iseVal & (1 << Ci(imax+2)) && iseVal & (1 << Ci(imax+3))
		&& !(iseVal & (1 << Ci(imax+0))) && !(iseVal & (1 << Ci(imax+1)))))
		corner1 = 1;
	    if ((iseVal & (1 << Ci(imax+1)) && iseVal & (1 << Ci(imax+2))
		&& !(iseVal & (1 << Ci(imax+0))) && !(iseVal & (1 << Ci(imax+3))))
	     || (iseVal & (1 << Ci(imax+0)) && iseVal & (1 << Ci(imax+3))
		&& !(iseVal & (1 << Ci(imax+1))) && !(iseVal & (1 << Ci(imax+2)))))
		corner2 = 1;
	}
	if (d1 < DEVIATION_FACTOR * d2
	    || ((d1 < d2) && (slope1 > slope2) && !corner2)
	    || ((d1 >= DEVIATION_FACTOR * d2 && (d2 >= DEVIATION_FACTOR * d1)) && corner1)
	    || ((slope1 > slope2) && (DEVIATION_FACTOR * d1 < d2) && !corner2)) {
	    /* triangle direction 1 */
	    sp5 = Sp (imax);
	    sp6 = Sp (imax+2);
	}
	else {
	    /* triangle direction 2 */
	    sp5 = Sp (imax+1);
	    sp6 = Sp (imax+3);
	}
    }

    new_face = meshRenameFace (sp5, sp6, face);

    meshMakeEdge (sp5, sp6, INTERNALEDGE);
    meshSetFaces (sp5, sp6, face, new_face);

    if (sp5 == Sp (imax) && sp6 == Sp (imax+2)) {
	/* triangle direction 1 */
	meshSetCorners (new_face, sp6, sp5, Sp(imax+1), NULL);
	meshSetCorners (face,     Sp(imax+3), sp5, sp6, NULL);
    }
    else if (sp5 == Sp (imax+1) && sp6 == Sp (imax+3)) {
	/* triangle direction 2 */
	meshSetCorners (new_face, sp6, sp5, Sp(imax+2), NULL);
	meshSetCorners (face,     Sp(imax),   sp5, sp6, NULL);
    }
    else {
	/* quadrilateral */
	meshSetCorners (new_face, sp6, sp5, Sp(imax+1), Sp(imax+2));
	meshSetCorners (face,     Sp(imax+3), Sp(imax), sp5, sp6);
    }

    /* Debug (meshPrintFace (face)); */
    /* Debug (meshPrintFace (new_face)); */

    /* Now go into recursion by entering both refined faces
     * into priority queue.
     */
    faceRecur (face);
    faceRecur (new_face);
}

/* Create a new spider (approximately) 'distRatio' along the path from sp1 to sp2.
 * If a spider already exist at that position,
 * just return that spider instead of creating a new one.
 */
Private spider_t *meshSplitPath (face_t *face, spider_t *sp1, spider_t *sp2)
{
    meshCoor_t x, y, z, nom_x, nom_y, nom_z;
    meshCoor_t nom_vx, nom_vy, nom_vz, sign, d12, d3, d4, nearby;
    spider_t *sp, *sp3, *sp4, tmp;

    d12 = spiderDist (sp1, sp2);
    ASSERT (d12 > 0);

    /* If part of the edge between sp1 and sp2 has been
     * corrected for the dimensions on silicon, we have
     * a problem:
     *            sp1 _____  (*)  _____ sp2
     *                     |     |
     *                     |_____|
     *
     * In this case, the new coordinates that are calculated
     * will be on the place of '(*)' and if a new spider is
     * placed there the mesh-generation will go wrong.
     * The following is a solution for this particular case:
     *
     * if ((sp = ccwa (sp1, face)) != sp2 && cwa (sp2, face) != sp) {
     *     sp1 = ccwa (sp1, face);
     *     sp2 = cwa (sp2, face);
     * }
     *
     * However, this can be solved much nicer by using the
     * nominal coordinates to find the two spiders closest
     * to the (nominal) splitting point and then use the
     * proportion between the nominal distances from these
     * two spiders to calculate the actual coordinates.
     */

    nom_vx = sp2 -> nom_x - sp1 -> nom_x;
    nom_vy = sp2 -> nom_y - sp1 -> nom_y;
    nom_vz = sp2 -> nom_z - sp1 -> nom_z;

    nom_x = sp1 -> nom_x + nom_vx * distRatio;
    nom_y = sp1 -> nom_y + nom_vy * distRatio;
    nom_z = sp1 -> nom_z + nom_vz * distRatio;

    /* A vector pointing along sp1/sp2 axis
     * This is used to take inproduct with a vector
     * from a certain point sp to to target split point.
     * If two such inproducts have different sign,
     * they are at different sides of split point.
     */

    sp3 = sp4 = NULL;
    for (sp = sp1; sp != sp2; sp = sp4) {
        sign = (nom_x - sp -> nom_x) * nom_vx
             + (nom_y - sp -> nom_y) * nom_vy
             + (nom_z - sp -> nom_z) * nom_vz;

	if (sign <= 0) break;
	sp3 = sp;
	sp4 = ccwa (sp, face);
    }
    ASSERT (sp3 && sp4);

    /* Now, sp3 and sp4 are at different sides of the target split point.
     * Now, find out wether one of these is close enough to the target split point.
     * What is close, is however difficult to say.
     * A good start is perhaps 'Min (distRatio, (1.0 - distRatio)) / 3'
     * of the distance between sp1 and sp2 (d12).
     */
    nearby = d12 * Min (distRatio, (1.0 - distRatio)) / 3;
    if (nearby > 4 && !FeModePwl) nearby = 4; /* SdeG: chose very nearby, else possible NAN problems */

    tmp.act_x = x = sp1 -> act_x + (sp2 -> act_x - sp1 -> act_x) * distRatio;
    tmp.act_y = y = sp1 -> act_y + (sp2 -> act_y - sp1 -> act_y) * distRatio;
    tmp.act_z = z = sp1 -> act_z + (sp2 -> act_z - sp1 -> act_z) * distRatio;

    /* Situations like (in actual coordinates):
     *
     *   sp1 \
     *        \
     *    (sp) \_____ sp2
     *
     * will give an incorrect results when using actual
     * coordinates. This particular situation could be
     * solved as follows:
     *
     * if (!sp && ccwa (sp1, face) == cwa (sp2, face)) {
     *     sp = ccwa (sp1, face);
     * }
     *
     * However, since we now use nominal coordinates, we
     * will find sp3 or sp4 to be assigned to (sp) and
     * therefore close enough to the target splitting point.
     */

    d3 = spiderDist (&tmp, sp3);
    d4 = spiderDist (&tmp, sp4);
    if (d3 < d4) {
	if (d3 < nearby) return (sp3);
    }
    else {
	if (d4 < nearby) return (sp4);
    }

    /* Spiders sp3 & sp4 are not close enough, so split the edge between them.
     * However, if the other side of the edge is a triangle, the edge can't be split.
     * The other side can be a triangle if this face is
     * at the boundary of two consequtive strips.
     * It is not very clean to do this, since the triangle already
     * was small enough. A better way would be to work with 5 strips.
     */
    {
	spiderEdge_t *edge = meshFindEdge (sp3, sp4);
	ASSERT (edge);

	sp = meshSplitEdge (sp3, sp4, x, y, z, nom_x, nom_y, nom_z);

	if (FeModePwl) {
	    ASSERT (edge -> face == face);
	    face = edge -> oh -> face;
	    if (face && isKnown (face)) { /* split triangle */
		spider_t *sp5 = ccwa (sp3, face);
		ASSERT (ccwa (sp5, face) == sp4);
		splitTriangle (face, sp4, sp3, sp5, sp);
	    }
	}
    }
    return (sp);
}

//  NW   N   NO          ---> ---> 0
//   y2  y2  y2               ^
//    \  |  /                 | 1
//     \ | /              --->|
// W y2 -y1 --- y2 O
//      / \               <--- 2    --->|
//     /   \              --->          |
// ZW    Z  ZO                        3 V

Private int getDirection (spider_t *sp1, spider_t *sp2, spider_t *sp3, int vside)
{
    double x1, x2, x3, y1, y2, y3;
    double t1, t2, d1, d2;

    if (vside) { // vertical face
	x1 = sp1 -> act_y;
	x2 = sp2 -> act_y;
	x3 = sp3 -> act_y;
    }
    else { // non vertical face
	x1 = sp1 -> act_x;
	x2 = sp2 -> act_x;
	x3 = sp3 -> act_x;
    }
    y1 = sp1 -> act_z;
    y2 = sp2 -> act_z;
    y3 = sp3 -> act_z;

    d1 = x2 - x1;
    d2 = x3 - x2;
    if (d1 > 0.0001) {
	if (d2 < 0.0001 && d2 > -0.0001) return (y3 > y2 ? 1 : 3);
	else { // NO && O && ZO
	    t1 = (y2 - y1) / d1;
	    t2 = (y3 - y2) / d2;
	    t2 -= t1;
	    if (d2 > 0) { // x3 > x2
		if (t2 >  0.0001) return (1);
		if (t2 < -0.0001) return (3);
		return (0);
	    }
	    else {
		if (t2 >  0.0001) return (3);
		if (t2 < -0.0001) return (1);
		return (2);
	    }
	}
    }
    else if (d1 < -0.0001) {
	if (d2 < 0.0001 && d2 > -0.0001) return (y3 > y2 ? 3 : 1);
	else { // NW && W && ZW
	    t1 = (y2 - y1) / d1;
	    t2 = (y3 - y2) / d2;
	    t2 -= t1;
	    if (d2 > 0) { // x3 > x2
		if (t2 >  0.0001) return (3);
		if (t2 < -0.0001) return (1);
		return (2);
	    }
	    else {
		if (t2 >  0.0001) return (1);
		if (t2 < -0.0001) return (3);
		return (0);
	    }
	}
    }
    else { // x2 == x1
	if (y2 > y1) { // N
	    if (d2 < -0.0001) return (1);
	    else if (d2 > 0.0001) return (3);
	    else if (y3 > y2) return (0);
	}
	else { // Z
	    if (d2 < -0.0001) return (3);
	    else if (d2 > 0.0001) return (1);
	    else if (y3 < y2) return (0);
	}
	return (2);
    }
}
