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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"

#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"
#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif
#include "src/space/green/green.h"

extern int new_via_mode;

#define D(x) (double)(x)

/* Find next spider in ccw (counter-clock wise) order around face.
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
    for (eh = sp -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp, eh));

    if (eh) {
	ASSERT (eh -> sp == sp);	/* Firewalls */
	ASSERT (eh -> oh -> oh == eh);

	/* Debug ((fprintf (stderr, "edge: "), pse (eh))); */

	if (newface) eh -> face = newface;

	return (OTHER_SPIDER (sp, eh));
    }
    say ("Internal error %s:%d", __FILE__, __LINE__); die ();
    return (NULL);
}

/* Find next spider in cw (clock wise) order around face.
 */
spider_t *meshCwAdjacent (spider_t *sp, face_t *face)
{
    register spiderEdge_t *eh;

    for (eh = sp -> edge; eh && eh -> oh -> face != face; eh = NEXT_EDGE (sp, eh));

    /* When a face on the current right boundary
     * of the total mesh is being refined, its right neigbor
     * face may not exist. Thus, eh may be NULL.
     */
    return (eh ? OTHER_SPIDER (sp, eh) : NULL);
}

/* Find the edge that links spiders sp1 and sp2.
 * Return NULL if not found.
 */
spiderEdge_t *meshFindEdge (spider_t *sp1, spider_t *sp2)
{
    register spiderEdge_t *edge;

    ASSERT (sp1 && sp2 && sp1 != sp2);

    for (edge = sp1 -> edge; edge; edge = NEXT_EDGE (sp1, edge)) {
	if (OTHER_SPIDER (sp1, edge) == sp2) break;
    }
    return (edge);
}

/* Create an edge that links spiders sp1 and sp2.
 * Do nothing if such an edge already exists.
 * The newly created edge is added to the strip datastructure
 * so that it can be deleted when its time has come.
 */
spiderEdge_t * meshMakeEdge (spider_t *sp1, spider_t *sp2, edgeType_t type)
{
    spiderEdge_t *edge;

    /* pre */
    ASSERT (sp1 && sp2 && sp1 != sp2);

    /* The next assertion is not true, because of sequencing aspects.
    * ASSERT (sp1 -> subnode -> node == sp2 -> subnode -> node);
    */
    if ((edge = meshFindEdge (sp1, sp2)) != NULL) {
	/* this must be a vertical link ! */
#if 0
	ASSERT (sp1 -> nom_x == sp2 -> nom_x);
	ASSERT (sp1 -> nom_y == sp2 -> nom_y);
	ASSERT (sp1 -> nom_z != sp2 -> nom_z);
	ASSERT (meshFindEdge (sp2, sp1) == edge -> oh);
#endif
	return (edge); /* already linked */
    }

    edge = newSpiderEdgePair ();
    edge -> sp = sp1; edge -> nb = sp1 -> edge; sp1 -> edge = edge; edge -> type = type; edge -> face = NULL;
    edge = edge -> oh;
    edge -> sp = sp2; edge -> nb = sp2 -> edge; sp2 -> edge = edge; edge -> type = type; edge -> face = NULL;

    stripAddEdge (edge);
    stripAddEdge (edge -> oh);
    return (edge -> oh);
}

/* Split an edge, return the new spider.
 * The edge is split by creating a new spider at (x,y,z).
 * The new edges are added to the strip datastructure.
 * Calls newSpider (), stripAddEdge ();
 */
spider_t *meshSplitEdge (spider_t *sp1, spider_t *sp2,
	meshCoor_t x, meshCoor_t y, meshCoor_t z,
	meshCoor_t nom_x, meshCoor_t nom_y, meshCoor_t nom_z)
{
    spider_t *sp3, *sp;
    spiderEdge_t *oh1, *oh2, *nh1, *nh2;
    subnode_t *subnodeSC;

    oh1 = meshFindEdge (sp1, sp2);
    oh2 = oh1 -> oh;
    nh1 = newSpiderEdgePair ();
    nh2 = nh1 -> oh;

    sp = sp1;
    if (sp1 -> conductor != sp2 -> conductor && new_via_mode) { // via
	if (sp2 -> nom_z > sp1 -> nom_z) sp = sp2;
	else ASSERT (sp1 -> nom_z > sp2 -> nom_z);
    }
    else
    if (sp1 -> subnode -> node != sp2 -> subnode -> node) {
	meshCoor_t d1, d2, t;
	d1 = x - sp1 -> subnode -> node -> node_x; d1 *= d1; t = y - sp1 -> subnode -> node -> node_y; d1 += t * t;
	d2 = x - sp2 -> subnode -> node -> node_x; d2 *= d2; t = y - sp2 -> subnode -> node -> node_y; d2 += t * t;
	if (d2 < d1) sp = sp2;
    }
    subnodeSC = (sp1 -> subnode2 && sp2 -> subnode2)? sp -> subnode2 : NULL;

    if (new_via_mode)
	sp3 = newSpider (nom_x, nom_y, nom_z, x, y, z, -1,
		sp -> subnode, subnodeSC, sp -> conductor, (sp1 -> isGate & sp2 -> isGate));
    else
	sp3 = newSpider (nom_x, nom_y, nom_z, x, y, z, -1,
		sp -> subnode, subnodeSC, sp1 -> conductor, (sp1 -> isGate & sp2 -> isGate));
    stripAddSpider (sp3);

    sp3 -> edge = nh1; nh1 -> nb = nh2; nh2 -> nb = NULL;

    nh1 -> sp = sp3; nh1 -> oh = oh1; nh1 -> face = oh2 -> face; nh1 -> type = oh1 -> type;
    nh2 -> sp = sp3; nh2 -> oh = oh2; nh2 -> face = oh1 -> face; nh2 -> type = oh2 -> type;

    oh1 -> oh = nh1;
    oh2 -> oh = nh2;

    /* oh1 and oh2 already are in the strip data structure.
    */
    stripAddEdge (nh1);
    stripAddEdge (nh2);

    return (sp3);
}

/* sp1 and sp2 are the endpoints of an edge, these determine an edge.
 * edge is directly an edge.
 * left is a face, can be NULL.
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
 * left and right are two faces, can be NULL.
 * 1. The face pointers of the edge from sp1 to sp2 is
 *    set such that 'left' and 'right' become the faces
 *    at the appropriate side when traveling from sp1 to sp2.
 * 2. For each of both faces that is non-NULL,
 *    set its spider pointer to sp1.
 */
void meshSetFaces (spider_t *sp1, spider_t *sp2, face_t *left, face_t *right)
{
    spiderEdge_t *edge = meshFindEdge (sp1, sp2);

    edge -> face = left;
    edge -> oh -> face = right;

    Debug (fprintf (stderr, "edge from %g %g %g to %g %g %g, faces %p %p\n",
	D(sp1 -> act_x), D(sp1 -> act_y), D(sp1 -> act_z),
	D(sp2 -> act_x), D(sp2 -> act_y), D(sp2 -> act_z), left, right));
}

/* Face is ready.
 * In debug mode, check its consistency.
 */
void meshSetCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4)
{
    ASSERT (sp1 && sp2 && sp3);

    SP(0) = sp1;
    SP(1) = sp2;
    SP(2) = sp3;
    SP(3) = sp4;

#ifdef DEBUG
    Debug (meshPrintFace (face));
    meshCheckLoop (face);
#endif /* DEBUG */
}

/* Returns a value > 0 if the face is at an edge of the element mesh.
   Starting at face -> corners[0] and going counter-clock wise the
   return value has the n-th bit set if the n-th side is an edge
   of the element mesh.  Currently this only works for 2D meshes.
*/
int isEdgeFace (face_t *face)
{
    spider_t *sp, *prev_sp, *start;
    spiderEdge_t *e;
    int is = 0;
    int side = 0;

    ASSERT (face);
    start = SP(0);

    for (prev_sp = start;; prev_sp = sp) {
	sp = ccwa (prev_sp, face);
	for (e = sp -> edge; e; e = NEXT_EDGE (sp, e)) {
	    if (e -> type != INTERNALEDGE) {
		is = is | (1 << 4);
		/* corner of face is on the edge of the mesh */
		if (OTHER_SPIDER (sp, e) == prev_sp) {
		    ASSERT (side < 4);
		    is = is | (1 << side);
		    /* edge of face is an edge of the mesh */
		}
	    }
	}
	if (sp == start) break;
	if (sp == SP(1) || sp == SP(2) || sp == SP(3)) side++;
    }
    return (is);
}

#ifdef DEBUG

/* Check well-formedness of a triangle.
 * sp1, sp2 and sp3 are checked to be in ccwa order around face.
 */
void meshCheckTriangle (spider_t *sp1, spider_t *sp2, spider_t *sp3, face_t *face)
{
#if 0
    if (!meshFindEdge (SP(0), SP(1)))
    if (!meshFindEdge (SP(1), SP(2)))
    if (!meshFindEdge (SP(2), SP(0))) say ("not simple triangle\n");
#endif
    ASSERT (ccwa (sp1, face) == sp2);
    ASSERT (ccwa (sp2, face) == sp3);
    ASSERT (ccwa (sp3, face) == sp1);

    ASSERT (cwa (sp1, face) == sp3);
    ASSERT (cwa (sp3, face) == sp2);
    ASSERT (cwa (sp2, face) == sp1);
#if 0
    ASSERT (meshFindEdge (sp1, sp2) == meshFindEdge (sp2, sp1) -> oh);
    ASSERT (meshFindEdge (sp2, sp3) == meshFindEdge (sp3, sp2) -> oh);
    ASSERT (meshFindEdge (sp3, sp1) == meshFindEdge (sp1, sp3) -> oh);
#endif
    ASSERT (!spActLinear (sp1, sp2, sp3));
}

/* Check that the face loop is consistent.
 * It also checks that one of the spiders in the loop
 * is the spider pointed to by face.
 * This procedure is for debugging only.
 */
void meshCheckLoop (face_t *face)
{
    spider_t *sp, *start;
    int i, j, k, n, found[4];
    static int spiders_size = 0;
    static spider_t **spiders = NULL;

    ASSERT (face);

    if (spiders == NULL) {
        spiders_size = 1000;
        spiders = NEW (spider_t *, spiders_size);
    }

    n = 0;
    spiders[n++] = start = SP(0);

    for (sp = ccwa (start, face); sp != start; sp = ccwa (sp, face)) {
        if (n >= spiders_size) {
	    int old_size = spiders_size;
            spiders_size = spiders_size * 2;
            spiders = RESIZE (spiders, spider_t *, spiders_size, old_size);
        }
	spiders[n++] = sp;
    }

    k = !SP(3) ? 3 : 4;

    if (n < k) say ("triangle or trapezoid with incorrect number of spiders.\n");

    /* see that there are no duplicates */
    for (i = 0; i < n; i++) {
	for (j = 0; j < i; j++) {
	   ASSERT (!Nearby (spiders[i] -> nom_x, spiders[j] -> nom_x)
		|| !Nearby (spiders[i] -> nom_y, spiders[j] -> nom_y)
		|| !Nearby (spiders[i] -> nom_z, spiders[j] -> nom_z));
	}
    }

    /* see that there are no duplicate corners */
    for (i = 0; i < k; i++) {
	for (j = 0; j < i; j++) {
            ASSERT (!Nearby (SP(i) -> nom_x, SP(j) -> nom_x)
		 || !Nearby (SP(i) -> nom_y, SP(j) -> nom_y)
		 || !Nearby (SP(i) -> nom_z, SP(j) -> nom_z));
        }
    }

    for (j = 0; j < 4; j++) found[j] = 0;
    for (i = 0; i < n; i++) {
	for (j = 0; j < k; j++) {
	    if (spiders[i] == SP(j)) {
                ASSERT (found[j] == 0);
		found[j]++;
	    }
	}
    }
    for (j = 0; j < k; j++) ASSERT (found[j] == 1);
}

/* Debugging: print the spider along the face loop
 * from sp_start to sp_stop.
 */
void meshPrintFace (face_t *face)
{
    spider_t *sp_start, *sp_stop, *sp_next;

    sp_next = sp_start = sp_stop = SP(0);

    fprintf (stderr, "Face: %p (type = %d)\n", face, face -> type);
    for (;;) {
	int i;
	ASSERT (sp_next);
	for (i = 0; i < 4; i++) if (SP(i) == sp_next) break;
	if (i < 4) fprintf (stderr, " (%d)  ", i);
	else       fprintf (stderr, "      ");
	psp (sp_next);
	if ((sp_next = ccwa (sp_next, face)) == sp_stop) break;
    }
}

void pstar (spider_t *sp1)
{
    spiderEdge_t *edge;
    spider_t *sp2;

    fprintf (stdout, "spider {xyz} %g %g %g", D(sp1 -> act_x), D(sp1 -> act_y), D(sp1 -> act_z));

    for (edge = sp1 -> edge; edge; edge = NEXT_EDGE (sp1, edge)) {
	sp2 = OTHER_SPIDER (sp1, edge);
	ASSERT (sp2);
	fprintf (stdout, " : %g %g %g", D(sp2 -> act_x), D(sp2 -> act_y), D(sp2 -> act_z));
    }
    fprintf (stdout, "\n");
}
#endif /* DEBUG */
