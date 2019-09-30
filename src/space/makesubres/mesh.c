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

#define SameDirection(p,q) ((p).x * (q).y == (p).y * (q).x \
			 && (p).y * (q).z == (p).z * (q).y \
			 && (p).z * (q).x == (p).x * (q).z \
			 && (p).x * (q).x >= 0.0 \
			 && (p).y * (q).y >= 0.0 \
			 && (p).z * (q).z >= 0.0)

/* Find next spider in ccw order around face.
 * Set face to newface if newface non-null.
 */
spider_t *meshCcwAdjacent (spider_t *sp, face_t *face, face_t *newface)
{
    register spiderEdge_t *eh;

    /* Too verbose.
    Debug ((fprintf (stderr, "face: %p, spider:", face), psp (sp)));
    */

    /* Search for the edgehalf eh going out of sp and which has
     * eh -> face as the face to be followed.
     * The other spider is then in eh -> oh -> sp.
     */
    for (eh = sp -> edge; eh && eh -> face != face; eh = eh -> nb);

    if (eh) {
	ASSERT (eh -> sp == sp);	 /* Firewalls */
	ASSERT (eh -> oh -> oh == eh);

	/* Debug ((fprintf (stderr, "edge: "), pse (eh))); */

	if (newface) eh -> face = newface;

	return (eh -> oh -> sp);
    }
    say ("Internal error %s:%d", __FILE__, __LINE__); die ();
    return (NULL);
}

/* Find next spider in cw order around face.
 */
spider_t *meshCwAdjacent (spider_t *sp, face_t *face)
{
    register spiderEdge_t *eh;

    for (eh = sp -> edge; eh && eh -> oh -> face != face; eh = eh -> nb);

    /* When a face on the current right boundary of the total mesh is being
     * refined, its right neigbor face may not exist.  Thus, eh may be NULL.
     */
    return (eh ? eh -> oh -> sp : NULL);
}

/* Find the edge that links spiders sp1 and sp2.
 * Return NULL if not found.
 */
spiderEdge_t *meshFindEdge (spider_t *sp1, spider_t *sp2)
{
    register spiderEdge_t *eh;

    /* pre */
    ASSERT (sp1 && sp2 && sp1 != sp2);

    for (eh = sp1 -> edge; eh && eh -> oh -> sp != sp2; eh = eh -> nb);

#ifdef DEBUG
    /* post */
    ASSERT (!eh || (eh -> sp == sp1 && eh -> oh -> sp == sp2));
#endif
    return (eh);
}

/* Create an edge that links spiders sp1 and sp2.
 * Do nothing if such an edge already exists.
 * The newly created edge is added to the strip datastructure
 * so that it can be deleted when its time has come.
 */
void meshMakeEdge (spider_t *sp1, spider_t *sp2, edgeType_t type)
{
    register spiderEdge_t *edge;

    /* pre */
    ASSERT (sp1 && sp2 && sp1 != sp2);
    ASSERT (sp1 -> act_x != sp2 -> act_x || sp1 -> act_y != sp2 -> act_y);

    /* The next assertion is not true, because of sequencing aspects.
    * ASSERT (sp1 -> subnode -> node == sp2 -> subnode -> node);
    */

    if ((edge = meshFindEdge (sp1, sp2)) != NULL) {
	/* this must be a vertical link ! */
	ASSERT (meshFindEdge (sp2, sp1) == edge -> oh);
	return;					/* already linked */
    }

    edge = newSpiderEdgePair ();

    edge -> face = edge -> oh -> face = NULL;

    edge -> sp = sp1;
    edge -> nb = sp1 -> edge;
    sp1 -> edge = edge;
    edge -> type = type;

    edge = edge -> oh;

    edge -> sp = sp2;
    edge -> nb = sp2 -> edge;
    sp2 -> edge = edge;
    edge -> type = type;

    stripAddEdge (edge);
    stripAddEdge (edge -> oh);

#ifdef DEBUG
    /* post */
    ASSERT (meshFindEdge (sp1, sp2) == edge -> oh);
    ASSERT (meshFindEdge (sp2, sp1) == edge);
#endif
}

/* Split an edge, return the new spider.
 * The edge is split by creating a new spider at (x,y,z).
 * The new edges are added to the strip datastructure.
 * Calls newSpider (), stripAddEdge ();
 */
spider_t *meshSplitEdge (spider_t *sp1, spider_t *sp2, meshCoor_t x, meshCoor_t y, meshCoor_t z)
{
    meshCoor_t dist1, dist2, t;
    spider_t *sp3, *sp;
    spiderEdge_t *oh1, *oh2, *nh1, *nh2;

    oh1 = meshFindEdge (sp1, sp2);
    oh2 = oh1 -> oh;
    nh1 = newSpiderEdgePair ();
    nh2 = nh1 -> oh;

#define DIST(s,d) d = x - s -> subnode -> subcontInfo -> xl;\
	  d *= d; t = y - s -> subnode -> subcontInfo -> yb; d += t * t
    DIST (sp1, dist1);
    DIST (sp2, dist2);
    sp = dist1 <= dist2 ? sp1 : sp2;

    sp3 = newSpider (x, y, z, sp -> subnode, NULL);

    ASSERT (oh1 -> sp == sp1);
    ASSERT (oh2 -> sp == sp2);

    sp3 -> edge = nh1;

    nh1 -> nb   = nh2;
    nh1 -> sp   = sp3;
    nh1 -> oh   = oh1;
    nh1 -> face = oh2 -> face;
    nh1 -> type = oh1 -> type;

    nh2 -> nb   = NULL;
    nh2 -> sp   = sp3;
    nh2 -> oh   = oh2;
    nh2 -> face = oh1 -> face;
    nh2 -> type = oh2 -> type;

    oh1 -> oh = nh1;
    oh2 -> oh = nh2;

    /* oh1 and oh2 already are in the strip data structure */
    stripAddEdge (nh1);
    stripAddEdge (nh2);

    return (sp3);
}

/* sp1 and sp2 are the endpoints of an edge, these determine an edge.
 * Left is a face, can be NULL.
 * 1. The face pointer of the edge (from sp1 to sp2) is set such
 *    that 'left' becomes the face seen from the edge from sp1 to sp2.
 * 2. If left is non-NULL set its spider pointer to sp1.
 */
void meshSetFace (spider_t *sp1, spider_t *sp2, face_t *left)
{
    spiderEdge_t *edge = meshFindEdge (sp1, sp2);

    ASSERT (edge);
    edge -> face = left;

    Debug (fprintf (stderr, "edge from %g %g %g to %g %g %g, face %p\n",
	D(sp1 -> act_x), D(sp1 -> act_y), D(sp1 -> act_z),
	D(sp2 -> act_x), D(sp2 -> act_y), D(sp2 -> act_z), left));
}

/* sp1 and sp2 are the endpoints of an edge.
 * Left and right are two faces, can be NULL.
 * 1. The face pointers of the edge from sp1 to sp2 is
 *    set such that 'left' and 'right' become the faces
 *    at the appropriate side when traveling from sp1 to sp2.
 * 2. For each of both faces that is non-NULL,
 *    set its spider pointer to sp1.
 */
void meshSetFaces (spider_t *sp1, spider_t *sp2, face_t *left, face_t *right)
{
    spiderEdge_t *edge = meshFindEdge (sp1, sp2);

    ASSERT (edge);
    edge -> face = left;
    edge -> oh -> face = right;

    Debug (fprintf (stderr, "edge from %g %g %g to %g %g %g, faces %p %p\n",
	D(sp1 -> act_x), D(sp1 -> act_y), D(sp1 -> act_z),
	D(sp2 -> act_x), D(sp2 -> act_y), D(sp2 -> act_z), left, right));
}

/* Face is ready.
 */
void meshSetCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4)
{
    ASSERT (sp1 && sp2 && sp3);
    face -> corners[0] = sp1;
    face -> corners[1] = sp2;
    face -> corners[2] = sp3;
    face -> corners[3] = sp4;
    face -> mark = sp4 ? FACE_TRAPEZOID : FACE_TRIANGLE;

    Debug (meshPrintFace (face));
}

/* Returns a value > 0 if the face is at an edge of the element mesh.
   Starting at face -> corners[0] and going counter-clockwise.  The return
   value has the n-th bit set if the n-th side is an edge of the element mesh.
*/
int isEdgeFace (face_t *face)
{
    spider_t *sp, *prev_sp, *start;
    spiderEdge_t *e;
    int is = 0;
    int side = 0;

    ASSERT (face);

    prev_sp = start = face -> corners[0];
    do {
	sp = ccwa (prev_sp, face);
	for (e = sp -> edge; e; e = e -> nb) {
	    if (e -> type != INTERNALEDGE) {
		if (e -> oh -> sp == prev_sp) is |= 1 << side;
		    /* edge of face is an edge of the mesh */
		is |= 1 << 4; // corner of face is on the edge of the mesh
	    }
	}
	if (sp == face -> corners[1]
	 || sp == face -> corners[2]
	 || sp == face -> corners[3]) { ++side; ASSERT (side < 4); }
    }
    while ((prev_sp = sp) != start);

    return (is);
}

#ifdef DEBUG
/*
 * Debugging: print the spider along the face loop from sp_start to sp_stop.
 */
void meshPrintFace (face_t *face)
{
    spider_t *sp_start, *sp_stop, *sp_next;

    sp_next = sp_start = sp_stop = face -> corners[0];

    fprintf (stderr, "Face: %p (mark = %d)\n", face, face -> mark);
    for (;;) {
	int i;
	ASSERT (sp_next);
	for (i = 0; i < 4; i++) if (face -> corners[i] == sp_next) break;
	if (i < 4) fprintf (stderr, " (%d)  ", i);
	else       fprintf (stderr, "      ");
	psp (sp_next);
	if ((sp_next = ccwa (sp_next, face)) == sp_stop) break;
    }
}
#endif // DEBUG
