/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

#ifdef __cplusplus
  extern "C" {
#endif

/* local operations */
Private void faceRecur (face_t *face);
Private void meshAddExtraSpider (face_t *face);
Private void refineTrapezoid (face_t *face);
Private void refineTriangle  (face_t *face);
Private void triangulate (face_t *face);

#ifdef __cplusplus
  }
#endif

#define DEVIATION_FACTOR 0.8
     /* For controlling the direction of mesh splitting. */

#define SMALL_INC  1.000000001
    /*  This value is used as a multiplication factor
	to assure that length and area comparisons
	give results as expected.  It was noticed that
	under Linux, compiled with -O, a comparison a > b,
	where a == b, gave TRUE as result.
	So now we use a > b * SMALL_INC as comparison.
    */

// A simple triangle does not have extra spiders on the edges.
#define SIMPLE_TRIANGLE(f) (meshFindEdge (f->corners[0], f->corners[1]) \
			 && meshFindEdge (f->corners[1], f->corners[2]) \
			 && meshFindEdge (f->corners[2], f->corners[0]))

/* Refine the mesh in stripR
 */
void meshRefine ()
{
    face_t *face;

    Debug (fprintf (stderr, "refine mesh %s\n", sprintfStripRcoor ()));
    // Insert ready faces, to be refined, into the face priority queue.
    faceInitEnumerate ();
    if (!(face = faceEnumerate ())) return; // no faces found
    do {
	// Debug (meshPrintFace (face));
	faceRecur (face);
    } while ((face = faceEnumerate ()));

    Debug (fprintf (stderr, "split faces for refinement %s\n", sprintfStripRcoor ()));
    // Retrieve faces in largest first order and split the faces
    // until they are small enough for triangularization.
    while ((face = pqDeleteHead ())) {
	// Debug (meshPrintFace (face));
	if (face -> mark == FACE_TRAPEZOID) refineTrapezoid (face);
	else {
	    ASSERT (face -> mark == FACE_TRIANGLE);
	    if (SIMPLE_TRIANGLE (face)) refineTriangle (face);
	    else triangulate (face);
	}
    }

    if (FeShape == 3) { /* FeModePwl */
	Debug (fprintf (stderr, "make simple triangles\n"));

	// For all quadrilateral faces, triangulate them.
	// And put too big faces into pq.
	faceInitEnumerate ();
	while ((face = faceEnumerate ())) {
	    if (face -> mark == FACE_TRAPEZOID || !SIMPLE_TRIANGLE (face)) triangulate (face);
	    else faceRecur (face);
	}
	// Retrieve triangles in largest first order and refine them.
	FeShape = 0;
	while ((face = pqDeleteHead ())) {
	    if (SIMPLE_TRIANGLE (face)) refineTriangle (face);
	    else triangulate (face);
	}
	FeShape = 3;
    }

    /*	Now follows a trick that is used for collocation (see notes)
	with a constant charge on each face.
	For each finite element, a new spider is created that points to the face.
	These new spiders are then used in traversing the strips.
	This can't be skipped with order-1 finite elements, since the faces
	would not be deallocated when they were not necessary anymore.
	This would give problems when extracting multiple cells.
    */
    Debug (fprintf (stderr, "add extra spider\n"));
    // For all ready faces, add extra spider.
    faceInitEnumerate ();
    while ((face = faceEnumerate ())) meshAddExtraSpider (face);

    Debug (fprintf (stderr, "end of refine mesh\n"));
}

/* Rename the face which is found by ccw traversal from sp1 to sp2.
 * The spider pointer of the new face is set to sp1.
 * Return the new face.
 * NOTE: MeshSetCorners should be performed by caller.
 */
Private face_t *meshRenameFace (spider_t *sp1, spider_t *sp2, face_t *face)
{
    face_t *newf;

    newf = newFace (); newf -> sc_subn = face -> sc_subn;
    while ((sp1 = meshCcwAdjacent (sp1, face, newf)) != sp2);
    return (newf);
}

Private void splitTriangle (face_t *face,
    spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *spN)
{
    face_t *newf;
    int mark = face -> mark;

    newf = newFace (); newf -> sc_subn = face -> sc_subn;

    meshMakeEdge (sp3, spN, INTERNALEDGE);
    meshSetFaces (sp3, spN, face, newf);
    meshSetFace (sp3, sp1, newf);
    meshSetFace (sp1, spN, newf);
    meshSetCorners (face, spN, sp2, sp3, Null(spider_t *));
    meshSetCorners (newf, spN, sp3, sp1, Null(spider_t *));
    if (mark == FACE_KNOWN) { /* already refined */
	face -> mark = FACE_KNOWN;
	newf -> mark = FACE_KNOWN;
	feSize (face);
	feSize (newf);
	/* Note: Face newf doesn't get an extra spider (see meshAddExtraSpider)
		 and can't be deallocated with the sp -> face method. */
    }
    else {
	faceRecur (face);
	faceRecur (newf);
    }
}

/* Refine a triangle face that is too large.
 * The algorithm is to split the longest edge, leading to this triangle
 * being split in two, but if there is another triangle bordered
 * by the same edge, this one is also split.
 */
Private void refineTriangle (face_t *face)
{
    spider_t *sp1, *sp2, *sp3, *sp;
    spiderEdge_t *edge;
    meshCoor_t d1, d2, d3, maxFeArea, maxLength;
    meshCoor_t x, y, z;

    // Debug (meshPrintFace (face));

    /* check if face was refined while it was in the queue
     */

    if (isEdgeFace (face)) {
        maxFeArea = spiderControl.maxEdgeFeArea;
        maxLength = spiderControl.maxSpiderLength;
    }
    else {
        maxFeArea = spiderControl.maxFeArea;
        maxLength = spiderControl.maxSpiderLength;
    }

    ASSERT (face -> area > 0);

    if (face -> len <= maxLength * SMALL_INC
    && face -> area <= maxFeArea * SMALL_INC) return; /* stop refinement */

    sp1 = face -> corners[0];
    sp2 = face -> corners[1];
    sp3 = face -> corners[2];

    d1 = spiderDist (sp1, sp2);
    d2 = spiderDist (sp2, sp3);
    d3 = spiderDist (sp3, sp1);

    if (d1 >= d2 && d1 >= d3)
	edge = meshFindEdge (sp1, sp2);
    else if (d2 >= d1 && d2 >= d3) {
	edge = meshFindEdge (sp2, sp3);
	sp1 = sp2; sp2 = sp3; sp3 = face -> corners[0];
    }
    else {
	edge = meshFindEdge (sp3, sp1);
	sp1 = sp3; sp3 = sp2; sp2 = face -> corners[0];
    }

    ASSERT (edge); // because: SIMPLE_TRIANGLE(face)
    ASSERT (edge -> face == face);

    /* meshRefineEdge
	The input is an edge that is part of one or two triangles.
	Split the edge halfway, and make from the newly created
	spider extra edges in the triangles at both sides.
	(at least one such triangle must exist)
	Thus, as a result there are two or four triangles.
    */

    /* Make the coordinate of the new spider the average
     * of the coordinates of sp1 and sp2.
     */
    x = (sp1 -> act_x + sp2 -> act_x) / 2;
    y = (sp1 -> act_y + sp2 -> act_y) / 2;
    z = sp1 -> act_z;

    sp = meshSplitEdge (sp1, sp2, x, y, z);

    splitTriangle (face, sp1, sp2, sp3, sp);

    if (FeModePwl && (face = edge -> oh -> face) && face -> area > 0) {
	sp3 = ccwa (sp1, face);
	if (ccwa (sp3, face) == sp2) {
	    // ASSERT (SIMPLE_TRIANGLE (face));
	    if (face -> mark == FACE_KNOWN) splitTriangle (face, sp2, sp1, sp3, sp);
	    else if (!FeShape && !face -> pqIndex && !inStripA (face)) pqInsert (face);
	}
    }
}

/* Represent each fe by a spider in the center of gravity of the fe.
 * This is used for 0-order fe's, where the faces form
 * the finite elements. They are now represented by the extra spider.
 * This is also used to throw away all faces,
 * with 0 as well as 1-th order finite elements.
 */
Private void meshAddExtraSpider (face_t *face)
{
    meshCoor_t x, y;
    spider_t * sp1 = face -> corners[0];
    spider_t * sp2 = face -> corners[1];
    spider_t * sp3 = face -> corners[2];
    spider_t * sp4 = face -> corners[3];

    if (!sp4) {
	ASSERT (face -> mark == FACE_TRIANGLE);
	x = (sp1 -> act_x + sp2 -> act_x + sp3 -> act_x) / 3.0;
	y = (sp1 -> act_y + sp2 -> act_y + sp3 -> act_y) / 3.0;
    }
    else {
	ASSERT (face -> mark == FACE_TRAPEZOID);
	x = (sp1 -> act_x + sp2 -> act_x + sp3 -> act_x + sp4 -> act_x) / 4.0;
	y = (sp1 -> act_y + sp2 -> act_y + sp3 -> act_y + sp4 -> act_y) / 4.0;
    }

    /* Make a new spider in the center of gravity of triangle or trapezoid.
     * Mark the spider as being special, it in fact represents the entire face.
     */
    sp2 = newSpider (x, y, sp1 -> act_z, face -> sc_subn, face);
    face -> mark = FACE_KNOWN;
}

/* Mesh refinement recursion by entering refined face
 * into priority queue if necessary and if it is not already there.
 */
Private void faceRecur (face_t *face)
{
    meshCoor_t maxFeArea, maxLength;

    ASSERT (face -> mark == FACE_TRAPEZOID || face -> mark == FACE_TRIANGLE);

    feSize (face);

    /* insert face in pq, if it is too big */

    maxFeArea = isEdgeFace(face) ? spiderControl.maxEdgeFeArea : spiderControl.maxFeArea;
    maxLength = spiderControl.maxSpiderLength;

    if (FeShape == 3 && face -> corners[3]) { /* FeModePwl */
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
    ||  face -> area > maxFeArea * SMALL_INC) pqInsert (face);
}

/* Return the area and length of longest edge of a finite element.
 * It is possible to save the square root by comparing with squared areas.
 */
void feSize (face_t *face)
{
    meshCoor_t s;		/* semiperimeter */
    meshCoor_t a, b, c, d;	/* length of sides of face */
    meshCoor_t e;		/* diagonal in case of quadrilateral */
    meshCoor_t l_max, l_min;
    spider_t *sp1, *sp2, *sp3, *sp4;

    sp1 = face -> corners[0];
    sp2 = face -> corners[1];
    sp3 = face -> corners[2];
    sp4 = face -> corners[3];

    a = spiderDist (sp1, sp2);
    b = spiderDist (sp2, sp3);
    l_min = Min (a, b);

    if (!sp4) { // triangle
	// compute area cf. schaum, s = semiperimeter
	c = spiderDist (sp3, sp1);
	if (c < l_min) l_min = c;
	s = (a + b + c) / 2;
	face -> area = sqrt (D(s * (s - a) * (s - b) * (s - c)));
    }
    else { // quadrilateral
	// compute area as if it are two triangles: 1 2 3 and 3 4 1.
	c = spiderDist (sp3, sp4);
	if (c < l_min) l_min = c;
	d = spiderDist (sp4, sp1);
	if (d < l_min) l_min = d;
	e = spiderDist (sp1, sp3);	/* diagonal */
	s = (a + b + e) / 2;
	face -> area = sqrt (D(s * (s - a) * (s - b) * (s - e)));
	s = (c + d + e) / 2;
	face -> area += sqrt (D(s * (s - c) * (s - d) * (s - e)));
	if (d > c) c = d;
    }

    if (b > a) a = b;
    l_max = Max (a, c); // length of face
    ASSERT (l_max > 0);

    face -> len  = l_max;
    face -> len2 = l_min;
}

/* Return TRUE if sp1, sp2, sp3 are on a straight line.
 * If so, a == s || b == s || c == s;
 * Also see the implementation of feSize.
 */
Private bool_t spLinear (spider_t *sp1, spider_t *sp2, spider_t *sp3)
{
    meshCoor_t a, b, c, s;

    a = spiderDist (sp1, sp2);
    b = spiderDist (sp2, sp3);
    c = spiderDist (sp3, sp1);
    s = (a + b + c) / 2;

    // To remedy the fact that due to truncation we may not find that
    // 3 spiders are on a straight line, we make s somewhat smaller.
    s *= 0.999999999;

    return ((a >= s || b >= s || c >= s) ? TRUE : FALSE);
}

/* This routine returns the slope between sp1 and sp2.
 */
Private meshCoor_t spSlope (spider_t *sp1, spider_t *sp2)
{
    return (sp1 -> act_y - sp2 -> act_y) / (sp1 -> act_x - sp2 -> act_x);
}

/* Create a new spider (approximately) 'distRatio' along the path from sp1 to sp2.
 * If a spider already exist at that position,
 * just return that spider instead of creating a new one.
 */
Private spider_t *meshSplitPath (face_t *face, spider_t *sp1, spider_t *sp2, double distRatio)
{
    meshCoor_t x, y, z, dx, dy, sign, dist, d3, d4;
    spider_t *sp, *sp3, *sp4, *sp5, tmp;
    spiderEdge_t *edge;

    dist = spiderDist (sp1, sp2);
    ASSERT (dist > 0);

    dx = sp2 -> act_x - sp1 -> act_x;
    dy = sp2 -> act_y - sp1 -> act_y;

    x = sp1 -> act_x + dx * distRatio;
    y = sp1 -> act_y + dy * distRatio;
    z = sp1 -> act_z;

    /* A vector pointing along sp1/sp2 axis.
     * This is used to take inproduct with a vector
     * from a certain point sp to to target split point.
     * If two such inproducts have different sign,
     * they are at different sides of split point.
     */
    sp3 = sp4 = 0;
    for (sp = sp1; sp != sp2; sp = sp4) {
        sign = (x - sp -> act_x) * dx + (y - sp -> act_y) * dy;
	if (sign <= 0) break;
	sp3 = sp;
	sp4 = ccwa (sp, face);
    }
    ASSERT (sp3 && sp4);

    /* Now, sp3 and sp4 are at different sides of the target split point.
     * Now, find out wether one of these is close enough to the target split point.
     * What is close, is however difficult to say.
     */
    if ((1.0 - distRatio) < distRatio)
	dist *= (1.0 - distRatio) / 3;
    else
	dist *= distRatio / 3;

    tmp.act_x = x;
    tmp.act_y = y;
    tmp.act_z = z;

    d3 = spiderDist (&tmp, sp3);
    d4 = spiderDist (&tmp, sp4);
    if (d3 < d4) { if (d3 < dist) return (sp3); }
    else         { if (d4 < dist) return (sp4); }

    edge = meshFindEdge (sp3, sp4);
    face = edge -> oh -> face;

    sp = meshSplitEdge (sp3, sp4, x, y, z);

    if (FeModePwl && face && face -> mark == FACE_KNOWN) {
	sp5 = ccwa (sp3, face);
	ASSERT (ccwa (sp5, face) == sp4);
	splitTriangle (face, sp4, sp3, sp5, sp);
    }
    return (sp);
}

/* This operation splits a quadrilateral face into 2 others
 * The direction of splitting is based on the aspect ratio.
 * Eventually, the face is split into two triangles.
 */
Private void refineTrapezoid (face_t *face)
{
    face_t *new_face;
    int i, j, iedge, k0, k1, k2, k3;
    meshCoor_t maxFeArea, max, min, d, dedge;
    spider_t *sp5, *sp6, *Sp[4];
    int iseVal;
    double minRatio, splitR1, splitR2;

    ASSERT (face -> mark == FACE_TRAPEZOID);

    // Save old corners, these are overwritten below, by meshSetCorners.
    Sp[0] = face -> corners[0];
    Sp[1] = face -> corners[1];
    Sp[2] = face -> corners[2];
    Sp[3] = face -> corners[3];
    ASSERT (Sp[3] != NULL);

    // Debug (meshPrintFace (face));

    if ((iseVal = isEdgeFace (face)))
        maxFeArea = spiderControl.maxEdgeFeArea;
    else
        maxFeArea = spiderControl.maxFeArea;

    iseVal &= 15;

    /*  We are going to determine the split line of the face.
	Normally, we try to split it perpendicular to its longest side.
	If the face is part of an edge of the element mesh, we split it
	such that the split line is parallel to the edge of the mesh.
	With the latter, we only allow faces for which (long side)
	< spiderControl.maxEdgeFeRatio * (short side) to be split along the longest side.
	To see which edge of the face is an edge of the element mesh,
	we use the information that is encoded in the return value of isEdgeFace().
	Since isEdgeFace() traverses the sides counter-clockwise,
	Sp[0..3] must also enumerate the corners counter-clockwise.
    */
    splitR1 = spiderControl.edgeSplitRatio;
    splitR2 = 1.0 - splitR1;
    minRatio = Min (splitR1, splitR2);

    max = min = spiderDist (Sp[0], Sp[1]);
    k0 = iedge = 0;
    dedge = 0; /* init, else compiler warning */

    for (i = 1; i < 4; ++i) {
	if ((j = i + 1) > 3) j = 0;
	d = spiderDist (Sp[i], Sp[j]);
	if (iseVal == (1 << j)) { iedge = i; dedge = d; }
	if (d > max) { max = d; k0 = i; }
	else if (d < min) min = d;
    }

    if (iedge && max / (dedge * minRatio) < spiderControl.maxEdgeFeRatio) k0 = iedge;
    k1 = (k0+1) % 4;
    k2 = (k0+2) % 4;
    k3 = (k0+3) % 4;

    // all faces are convex (don't have concave spiders)

    if (!FeModePwl || face -> area > 2 * maxFeArea || max >= 2 * min
	|| iseVal == (1 << k1) || iseVal == (1 << k3)) {
	/* Split into two quadrilaterals.
	 */
	double distRatio = 0.5;
	if (iseVal && (face -> area * splitR2 < spiderControl.maxFeArea)) {
	    if (iseVal == (1 << k1)) distRatio = splitR2;
	    else if (iseVal == (1 << k3)) distRatio = splitR1;
	}
	sp5 = meshSplitPath (face, Sp[k0], Sp[k1], distRatio);
	sp6 = meshSplitPath (face, Sp[k2], Sp[k3], 1.0 - distRatio);
    }
    else {
	/* Split into two triangles.  Split along shortest diagonal.
	 * Allow DEVIATION_FACTOR deviation to choose a split direction
	 * perpendicular to a corner of the element mesh.
	 * Also allow DEVIATION_FACTOR deviation to chose the globally
	 * preffered direction, such that similar opposite faces
	 * will be split the same way.
	 */
	meshCoor_t d1 = spiderDist (Sp[k0], Sp[k2]);
	meshCoor_t d2 = spiderDist (Sp[k1], Sp[k3]);
	int corner1 = 0;
	int corner2 = 0;

	if (iseVal) {
	    if ((iseVal & (1 << k0) && iseVal & (1 << k1) &&
		!(iseVal & (1 << k2)) && !(iseVal & (1 << k3)))
	     || (iseVal & (1 << k2) && iseVal & (1 << k3) &&
		!(iseVal & (1 << k0)) && !(iseVal & (1 << k1)))) corner1 = 1;
	    if ((iseVal & (1 << k1) && iseVal & (1 << k2) &&
		!(iseVal & (1 << k0)) && !(iseVal & (1 << k3)))
	     || (iseVal & (1 << k0) && iseVal & (1 << k3) &&
		!(iseVal & (1 << k1)) && !(iseVal & (1 << k2)))) corner2 = 1;
	}

	if (d1 < DEVIATION_FACTOR * d2
	|| (corner1 && d2 >= DEVIATION_FACTOR * d1)
	|| (!corner2 && d2 > DEVIATION_FACTOR * d1 &&
	    spSlope (Sp[k0], Sp[k2]) > spSlope (Sp[k1], Sp[k3]))) {
	    sp5 = Sp[k0]; sp6 = Sp[k2]; /* triangle direction 1 */
	}
	else {
	    sp5 = Sp[k1]; sp6 = Sp[k3]; /* triangle direction 2 */
	}
    }

    new_face = meshRenameFace (sp5, sp6, face);

    meshMakeEdge (sp5, sp6, INTERNALEDGE);
    meshSetFaces (sp5, sp6, face, new_face);

    if (sp5 == Sp[k0] && sp6 == Sp[k2]) {
	/* triangle direction 1 */
	meshSetCorners (face,     Sp[k3], sp5, sp6, Null(spider_t *));
	meshSetCorners (new_face, sp6, sp5, Sp[k1], Null(spider_t *));
    }
    else if (sp5 == Sp[k1] && sp6 == Sp[k3]) {
	/* triangle direction 2 */
	meshSetCorners (face,     Sp[k0], sp5, sp6, Null(spider_t *));
	meshSetCorners (new_face, sp6, sp5, Sp[k2], Null(spider_t *));
    }
    else {
	/* quadrilateral */
	meshSetCorners (face,     Sp[k3], Sp[k0], sp5, sp6);
	meshSetCorners (new_face, sp6, sp5, Sp[k1], Sp[k2]);
    }

    // Debug (meshPrintFace (face));
    // Debug (meshPrintFace (new_face));

    // Now go into recursion by entering both refined faces into priority queue.
    faceRecur (face);
    faceRecur (new_face);
}

/* sp1, sp2 and sp3 are the corners of an unfinished triangle.
 * A new face is created for the triangle.
 * The new edge must be created from sp1 to sp2.
 */
Private void mkTriangle (spider_t *sp1, spider_t *sp2, spider_t *sp3, face_t *face)
{
    face_t *newf;

    newf = newFace (); newf -> sc_subn = face -> sc_subn;

    meshMakeEdge (sp1, sp2, INTERNALEDGE);
    meshSetFaces (sp1, sp2, newf, face);
    meshSetFace (sp2, sp3, newf);
    meshSetFace (sp3, sp1, newf);
    meshSetCorners (newf, sp1, sp2, sp3, Null(spider_t *));

#ifdef DEBUG
    /* Well-formedness of the triangle that has been
     * split off is checked in meshSetCorners.
     * Partially check well-formedness of the remaining part here.
     */
    ASSERT ( cwa (sp1, face) == sp2);
    ASSERT (ccwa (sp2, face) == sp1);
#endif /* DEBUG */

    faceRecur (newf);
}

/* The quadrilateral face is a trapezoidal face.
 * This routine will break the face down into triangles.
 * Triangles are marked as such.
 * (do pq recursion, if doRecur == TRUE)
 */
Private void triangulate (face_t *face)
{
    spider_t *br, *brn, *tl, *tln;

    Debug (meshPrintFace (face));

#define AdvanceCcw tl = tln; tln = ccwa (tl, face)
#define AdvanceCw  br = brn; brn =  cwa (br, face)

    /* must start in a corner */
    tl = face -> corners[0]; tln = ccwa (tl, face);
    br = face -> corners[0]; brn =  cwa (br, face);

    if (spiderDist (tln, br) < spiderDist (tl, brn)) { AdvanceCcw; }
    else { AdvanceCw; }

    while (tln != brn) {
	if (spLinear (br, tl, tln)	    /* may happen in first corner */
	||  spLinear (tln, brn, br)) {      /* may happen in last  corner */
	    mkTriangle (tl, brn, br, face);
	    AdvanceCw;
	}
	else if (spLinear (brn, br, tl)     /* may happen in first corner */
	     ||  spLinear (tl, tln, brn)) { /* may happen in last  corner */
	    mkTriangle (tln, br, tl, face);
	    AdvanceCcw;
	}
	else if ((spiderDist (tln, br) < DEVIATION_FACTOR * spiderDist (brn, tl))
	     || ((spSlope (tln, br) > spSlope (brn, tl) &&
		DEVIATION_FACTOR * spiderDist (tln, br) < spiderDist (brn, tl)))) {
	    mkTriangle (tln, br, tl, face);
	    AdvanceCcw;
	}
	else {
	    mkTriangle (tl, brn, br, face);
	    AdvanceCw;
	}
    }

#ifdef DEBUG
    ASSERT (tln == brn);
    ASSERT (ccwa (tln, face) == br);
    ASSERT (ccwa (br,  face) == tl);
    ASSERT (!spLinear (tln, br, tl));
#endif /* DEBUG */

    // The remaining triangle in the left bottom corner.
    meshSetCorners (face, tln, br, tl, Null(spider_t *));
    faceRecur (face);
}
