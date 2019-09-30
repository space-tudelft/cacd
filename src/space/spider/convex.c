/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Frederik Beeftink
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
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/export.h"

#include "src/space/spider/define.h"
#include "src/space/spider/recog.h"

#include "src/space/green/green.h"
#include "src/space/green/gputil.h"

#include "src/space/spider/extern.h"

#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void algorithmJoeSimpson (face_t *face, spider_t *ksp);
Private void meshSetFaceCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *corners[]);
#ifdef __cplusplus
  }
#endif

extern int *displaceList;
extern int new_convex;
extern int nrOfCondStd;

#define R3ActSet(v, sp) R3Assign (v, sp -> act_x, sp -> act_y, sp -> act_z)
#define R3NomSet(v, sp) R3Assign (v, sp -> nom_x, sp -> nom_y, sp -> nom_z)

#define R3ActVector(sp1, sp2, v) {\
    v.x = sp1 -> act_x - sp2 -> act_x; \
    v.y = sp1 -> act_y - sp2 -> act_y; \
    v.z = sp1 -> act_z - sp2 -> act_z; \
}

#define R3NomVector(sp1, sp2, v) {\
    v.x = sp1 -> nom_x - sp2 -> nom_x; \
    v.y = sp1 -> nom_y - sp2 -> nom_y; \
    v.z = sp1 -> nom_z - sp2 -> nom_z; \
}

Private double Acos (double v)
{
    if (ceil (v) == -1.0) return M_PI;
    if (floor (v) == 1.0) return  0.0;
    return acos (v);
}

#define MAX_VERTICES 100
#define New_vertex(i) if (i >= MAX_VERTICES) too_many_vertices (i)

Private void too_many_vertices (int i)
{
    say ("error: number of vertices for one face exceeds maximum (%d)", i);
    die ();
}

Private int equal_act_x (spider_t *sp1, spider_t *sp2)
{
    spiderEdge_t *e;
    spider_t *spe;
    int ok, j, d1, d2;

    if (sp1 -> act_x == sp2 -> act_x) return (1);

    j = ok = d1 = 0;
    for (e = sp1 -> edge; e; e = e -> nb) { spe = e -> oh -> sp;
	if (spe != sp2 && spe -> nom_z == sp2 -> nom_z) {
	    if (++j == 3) break;
	    if (spe -> nom_y == sp1 -> nom_y) d2 = 0;
	    else if (spe -> nom_x == sp1 -> nom_x) d2 = 2;
	    else if (spe -> nom_x > sp1 -> nom_x)
		 d2 = (spe -> nom_y > sp1 -> nom_y)? 1 : -1;
	    else d2 = (spe -> nom_y < sp1 -> nom_y)? 1 : -1;
	    if (!ok) { d1 = d2; ++ok; } else if (d2 == d1) ++ok;
	}
    }
    if (j == 2 && ok == 2) {
	sp1 -> act_x = sp2 -> act_x;
	if (d1) sp1 -> act_y += sp1 -> act_y - sp1 -> nom_y;
	return (1);
    }
    return (0);
}

bool_t convexFace (face_t *face)
{
    int d1, d2, i, level;
    pointR3_t v1, v2;    /* Vectors of edges */
    pointR3_t cp1, cp2;  /* Crossproducts */
    double ncp1, ncp2;
    spider_t *sp[MAX_VERTICES];
    spider_t *sp0, *sp1, *sp2, *sp3;

    sp0 = SP(0);
    if ((level = sp0 -> conductor) >= nrOfCondStd) return (TRUE);

    if (face -> type & FACE_BOT)
	level = 2 * level;
    else {
	ASSERT (face -> type & FACE_TOP);
	level = 2 * level + 1;
    }
    if (!displaceList[level]) return (TRUE);

    /* First thing to do is to find the spiders that belong
     * to the face that cause a concave corner in the face.
     */

    /* Calculate reference crossproduct. */

    R3NomVector (SP(1), sp0, v1);
    R3NomVector (SP(2), SP(1), v2);

    R3Cross (v1, v2, cp1);
    ncp1 = R3Norm (v1) * R3Norm (v2);
    if (ncp1 != 0) { cp1.x /= ncp1; cp1.y /= ncp1; cp1.z /= ncp1; }
    /* The length of cp1 is now equal to sin (angle(v1,v2)) */

    /* It would be possible to get the correct normal vector
     * by using the actual coordinates and check the inproduct
     * between that crossproduct and the nominal one, but
     * really shouldn't matter for our application: the angle
     * between the normal vectors of the actual and nominal
     * surface will never be larger than 90 degrees.
     */
    sp1 = sp0;
    sp2 = ccwa (sp1, face);
    sp3 = ccwa (sp2, face);

    i = 0;
    do {
	if (new_convex && sp1 -> nom_x == sp2 -> nom_x && sp3 -> nom_x == sp2 -> nom_x) {
	    d1 = equal_act_x (sp1, sp2);
	    d2 = equal_act_x (sp3, sp2);
	    if (d1 && d2) goto next_sp1; /* not concave */
	}

	/* Now determine crossproduct of two vectors */
	R3ActVector (sp2, sp1, v1);
	R3ActVector (sp3, sp2, v2);

	R3Cross (v1, v2, cp2);
        ncp2 = R3Norm (v1) * R3Norm (v2);
	if (ncp2 != 0) { cp2.x /= ncp2; cp2.y /= ncp2; cp2.z /= ncp2; }
        /* The length of cp2 is now equal to sin (angle(v1,v2)) */

	if (R3Dot (cp1, cp2) < (-1 * SP_EPS)) {
	    New_vertex (i); sp[i++] = sp2;
	}
next_sp1:
	sp1 = sp2; sp2 = sp3; sp3 = ccwa (sp3, face);
    }
    while (sp1 != sp0);

    if (i == 0) return (TRUE); /* No concave spiders in face! */

    /* Here we solve the problem by dividing the concave faces
     * into several convex faces and put those back into the
     * recursion list. We finish with returning false.
     */

    Debug (fprintf (stderr, "-> %d concave spider(s) in face <-\n", i));

    /* Untill now, I found two algorithms that divide
     * concave/irregular faces into convex faces.
     */
    algorithmJoeSimpson (face, sp[0]);
    return (FALSE);
}

Private bool_t pointR3OnEdge (pointR3_t *p, spider_t *sp1, spider_t *sp2)
{
    double alpha;
    pointR3_t sv1, sv2, v1, v2, v3;

    if (p -> x == sp1 -> act_x && p -> y == sp1 -> act_y && p -> z == sp1 -> act_z) return (TRUE);
    if (p -> x == sp2 -> act_x && p -> y == sp2 -> act_y && p -> z == sp2 -> act_z) return (TRUE);

    R3Assign (v1, p -> x, p -> y, p -> z);

    R3ActSet (v2, sp1);
    R3ActSet (v3, sp2);

    R3Subtract (v2, v1, sv1);
    R3Subtract (v3, v1, sv2);

    alpha = Acos (R3Dot (sv1, sv2) / (R3Norm (sv1) * R3Norm (sv2)));

    if ((Abs (alpha - M_PI)) < SP_EPS) return (TRUE);

    return (FALSE);
}

Private spider_t * spiderAdd (spider_t *sp1, spider_t *sp2, pointR3_t *p)
{
    double ratio;
    spider_t *sp;

    if (sp1 -> act_x == p -> x && sp1 -> act_y == p -> y && sp1 -> act_z == p -> z) return (sp1);
    if (sp2 -> act_x == p -> x && sp2 -> act_y == p -> y && sp2 -> act_z == p -> z) return (sp2);

    sp = NEW (spider_t, 1);
    sp -> act_x = p -> x;
    sp -> act_y = p -> y;
    sp -> act_z = p -> z;

    ratio = spiderDist (sp, sp1) / spiderDist (sp1, sp2);

    sp -> nom_x = sp1 -> nom_x + ratio * (sp2 -> nom_x - sp1 -> nom_x);
    sp -> nom_y = sp1 -> nom_y + ratio * (sp2 -> nom_y - sp1 -> nom_y);
    sp -> nom_z = sp1 -> nom_z + ratio * (sp2 -> nom_z - sp1 -> nom_z);

    sp -> subnode = NULL; /* mark as virtual spider */

    return (sp);
}

/* Remove the virtual spiders, and if the selected spider
 * is such, create proper spider and edges.
 */
Private void spiderRemove (spider_t *u[], face_t *face, spider_t **sp1, spider_t **sp2)
{
    int i;
    pointR3_t p;
    spider_t *sp, *newsp, *spcw, *spccw, *ksp;

#define Spider(sp, psp1, psp2) ((psp1 && sp == *psp1) ? *psp1 : *psp2)

    ksp = face -> corners[0];

    for (i = 0; u[i]; i++) {

	if (!u[i] -> subnode) {

	    if ((sp1 && u[i] == *sp1) || (sp2 && u[i] == *sp2)) {

		sp = Spider (u[i], sp1, sp2);

		R3ActSet (p, sp);

		for (spcw = ksp, spccw = ccwa (ksp, face);
		     !pointR3OnEdge (&p, spcw, spccw);
		     spcw = spccw, spccw = ccwa (spccw, face));

		newsp = meshSplitEdge (spcw, spccw, p.x, p.y, p.z, sp -> nom_x, sp -> nom_y, sp -> nom_z);

		if (sp1 && sp == *sp1) *sp1 = newsp;
		else *sp2 = newsp;
	    }
	    DISPOSE (u[i], sizeof(spider_t));
	}
    }
}

Private double minimumAngle (face_t *face, spider_t *ksp, spider_t *sp1, spider_t *sp2)
{
    double min = M_PI;
    double alpha, beta, gamma;
    spider_t *spcw, *spccw;
    pointR3_t p, v1, v2, v3;

    if (sp1 -> subnode) {
	spcw = cwa (sp1, face);
	spccw = ccwa (sp1, face);
    }
    else {
	R3ActSet (p, sp1);

	for (spcw = ksp, spccw = ccwa (ksp, face);
	     !pointR3OnEdge (&p, spcw, spccw);
	     spcw = spccw, spccw = ccwa (spccw, face));
    }

    R3ActVector (ksp, sp1, v2);

    R3ActVector (spcw, sp1, v1);
    alpha = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
    if (alpha < min) min = alpha;

    R3ActVector (spccw, sp1, v1);
    beta = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
    if (beta < min) min = beta;

    R3ActVector (sp1, ksp, v2);

    R3ActVector (cwa (ksp, face), ksp, v1);
    alpha = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
    if (alpha < min) min = alpha;

    R3ActVector (ccwa (ksp, face), ksp, v1);

    /* If this is the only edge to be checked,
     * we can return here (finished).
     */
    if (!sp2) {
	beta = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
	if (beta < min) min = beta;
	return (min);
    }

    R3ActVector (sp2, ksp, v3);

    beta = Acos (R3Dot (v1, v3) / (R3Norm (v1) * R3Norm (v3)));
    if (beta < min) min = beta;

    /* Determine gamma */
    gamma = 2 * M_PI - alpha - beta - Acos (R3Dot (v2, v3) / (R3Norm (v2) * R3Norm (v3)));

    /* Return if gamma larger than 180 degrees */
    if (gamma > M_PI) return (0);

    if (gamma < min) min = gamma;

    if (sp2 -> subnode) {
	spcw = cwa (sp2, face);
	spccw = ccwa (sp2, face);
    }
    else {
	R3ActSet (p, sp2);

	for (spcw = ksp, spccw = ccwa (ksp, face);
	     !pointR3OnEdge (&p, spcw, spccw);
	     spcw = spccw, spccw = ccwa (spccw, face));
    }

    R3ActVector (ksp, sp2, v2);

    R3ActVector (spcw, sp2, v1);
    alpha = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
    if (alpha < min) min = alpha;

    R3ActVector (spccw, sp2, v1);
    beta = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));
    if (beta < min) min = beta;

    return (min);
}

/* Calculate intersection between two vectors A and B.
 * The endpoints of the vectors are used to construct
 * the line-equations, result is stored into p.
 */
Private void pointR3Intersect (spider_t *spA1, spider_t *spA2, spider_t *spB1, spider_t *spB2, pointR3_t *p)
{
    double k, denom;
    pointR3_t sv1, sv2;		/* Start vectors */
    pointR3_t dv1, dv2;		/* Direction vectors */

    R3ActSet (sv1, spA1);
    R3ActSet (dv1, spA2);
    R3Subtract (dv1, sv1, dv1);

    R3ActSet (sv2, spB1);
    R3ActSet (dv2, spB2);
    R3Subtract (dv2, sv2, dv2);

    /* Calculate intersection for the two lines:
     * sv1 + k * dv1 == sv2 + l * dv2.
     */

    if (dv1.x == 0 && dv2.x == 0) {
	denom = dv1.y * dv2.z - dv1.z * dv2.y;
	k = (dv2.z * (sv2.y - sv1.y) -  dv2.y * (sv2.z - sv1.z)) / denom;
    }
    else if (dv1.y == 0 && dv2.y == 0) {
	denom = dv1.x * dv2.z - dv1.z * dv2.x;
	k = (dv2.z * (sv2.x - sv1.x) - dv2.x * (sv2.z - sv1.z)) / denom;
    }
    else {
	denom = dv1.x * dv2.y - dv1.y * dv2.x;
	k = (dv2.y * (sv2.x - sv1.x) - dv2.x * (sv2.y - sv1.y)) / denom;
    }
    p -> x = sv1.x + k * dv1.x;
    p -> y = sv1.y + k * dv1.y;
    p -> z = sv1.z + k * dv1.z;
}

Private void visibleSpiders (face_t *face, spider_t *ksp, spider_t *vsp[])
{
    int i = 0;
    bool_t right;
    pointR3_t p, v1, v2, cp1, cp2, cp3, rcp;
    spider_t *sp, *newsp, *sp1, *sp2;

    /* Initialize with cwa and ccwa */
    vsp[i++] = sp2 = cwa  (ksp, face);
    vsp[i++] = sp1 = ccwa (ksp, face);

    /* Calculate reference crossproduct. */
    R3NomVector (face -> corners[1], face -> corners[0], v1);
    R3NomVector (face -> corners[2], face -> corners[1], v2);
    R3Cross (v1, v2, rcp);

    sp1 = ccwa (sp1, face);
    while (sp1 != vsp[0]) {

	R3ActVector (vsp[i-1], ksp, v1);
	R3ActVector (sp2, vsp[i-1], v2);
	R3Cross (v1, v2, cp1);

	if ((R3Dot (rcp, cp1)) <= 0) right = TRUE;
	else right = FALSE;

	R3ActVector (sp1, vsp[i-1], v2);
	R3Cross (v1, v2, cp1);

	R3ActVector (vsp[0], ksp, v1);
	R3ActVector (sp1, vsp[0], v2);
	R3Cross (v1, v2, cp2);

	R3ActVector (vsp[i-1], vsp[i-2], v1);
	R3ActVector (sp1, vsp[i-1], v2);
	R3Cross (v1, v2, cp3);

	if (((R3Dot (rcp, cp1) >= 0) && right)
	    || ((R3Dot (rcp, cp2) < 0) && right)
	    || ((R3Dot (rcp, cp2) >= 0) && !right)
	    || ((R3Dot (rcp, cp1) < 0) && !right)) {
	    /* sp1 in R2 => visible from ksp */
	    New_vertex (i); vsp[i++] = sp1;
	}
	else if (((R3Dot (rcp, cp3) < 0) && right)
		 || ((R3Dot (rcp, cp3) >= 0) && !right)) {
	    /* sp1 in R3 => invisible from ksp */
	    R3ActVector (vsp[i-1], ksp, v1);

	    for (sp = sp1, sp1 = ccwa (sp1, face), newsp = NULL;
		 newsp == NULL; sp = sp1, sp1 = ccwa (sp1, face)) {

		/* Find edge that crosses ksp -> vsp[i-1] */
		R3ActVector (sp1, vsp[i-1], v2);
		R3Cross (v1, v2, cp1);
		R3ActVector (sp, vsp[i-1], v2);
		R3Cross (v1, v2, cp2);

		if (((R3Dot (rcp, cp1) >= 0) && (R3Dot (rcp, cp2) < 0) && right)
		 || ((R3Dot (rcp, cp1) < 0) && (R3Dot (rcp, cp2) >= 0) && !right)) {
		    /* sp -> sp1 crosses ksp -> vsp[i-1] */
		    pointR3Intersect (ksp, vsp[i-1], sp, sp1, &p);
		    newsp = spiderAdd (sp1, sp, &p);
		}

                if (sp1 == vsp[0]) break;
	    }
	    /* Remember to set sp2 correctly */
	    sp2 = cwa (sp, face);

	    /* Correction for the for loop */
	    sp1 = cwa (sp1, face);
	    sp2 = cwa (sp2, face);

	    if (newsp && newsp != sp1 && newsp != sp) { New_vertex (i); vsp[i++] = newsp; };
	    New_vertex (i); vsp[i++] = sp1;
	}
	else {
	    /* sp1 in R1 => blocking vsp-points */
	    for (--i, newsp = NULL; newsp != NULL; i--) {
		R3ActVector (sp1, ksp, v1);
		R3ActVector (vsp[i-1], sp1, v2);
		R3Cross (v1, v2, cp1);

		pointR3Intersect (sp1, ccwa (sp1, face), vsp[i], vsp[i-1], &p);
		/* Check the range of the intersection point p */
		if (pointR3OnEdge (&p, vsp[i], vsp[i-1])) {
		    /* The new edge intersects an old one */
		    newsp = spiderAdd (vsp[i-1], vsp[i], &p);
                    ASSERT (newsp != vsp[i-1] && newsp != vsp[i]);
	            New_vertex (i); vsp[i++] = newsp;
		}

		if (((R3Dot (rcp, cp1) < 0) && right)
		    || ((R3Dot (rcp, cp1) >= 0) && !right)) {
		    /* Spider found left of the new spider */
		    pointR3Intersect (ksp, sp1, vsp[i], vsp[i-1], &p);
		    newsp = spiderAdd (vsp[i-1], vsp[i], &p);
                    ASSERT (newsp != vsp[i-1] && newsp != vsp[i]);
		    if (newsp != sp1) { New_vertex (i); vsp[i++] = newsp; }
		    New_vertex (i); vsp[i++] = sp1;
		}
	    }
	}
	sp1 = ccwa (sp1, face);
	sp2 = ccwa (sp2, face);
	if (sp2 == ksp) sp2 = ccwa (sp2, face);
    }
    vsp[i] = NULL;
}

Private void divideFace (face_t *face, spider_t *ksp, spider_t *s[], spider_t *lc[], spider_t *ic[], spider_t *rc[])
{
    int i, j, l, r, where;
    pointR3_t rcp, cp1, cp2, p, v1, v2, v3;
    spider_t *sp1, *sp2, *spcw, *spccw;

    spcw  = cwa  (ksp, face);
    spccw = ccwa (ksp, face);

    /* Calculate reference crossproduct. */
    R3NomVector (face -> corners[1], face -> corners[0], v1);
    R3NomVector (face -> corners[2], face -> corners[1], v2);
    R3Cross (v1, v2, rcp);

    R3ActVector (spccw, ksp, v1);
    R3ActVector (spcw, ksp, v2);

    where = i = l = r = 0;

    for (j = 0, sp2 = spccw; s[j]; j++) {

	sp1 = sp2;
	sp2 = s[j];

	switch (where) {
	case 0:
	    /* we are in the right cone */
	    R3ActVector (sp2, spcw, v3);
	    R3Cross (v2, v3, cp1);
	    if (R3Dot (rcp, cp1) >= 0) rc[r++] = sp2; /* we stay in the right cone */
	    else {
		pointR3Intersect (ksp, spcw, sp2, sp1, &p);
		if (p.x == sp2 -> act_x && p.y == sp2 -> act_y && p.z == sp2 -> act_z) rc[r++] = sp2;
		ic[i++] = sp2;
		where = 1; /* switch to inner cone */
	    }
	    break;

	case 1:
	    /* we are in the inner cone */
	    R3ActVector (sp2, spccw, v3);
	    R3Cross (v1, v3, cp1);
	    R3ActVector (sp2, spcw, v3);
	    R3Cross (v2, v3, cp2);

	    if (R3Dot (rcp, cp1) < 0) {
		/* we go to the left cone */
		pointR3Intersect (ksp, spccw, sp2, sp1, &p);
		if (p.x == sp2 -> act_x && p.y == sp2 -> act_y && p.z == sp2 -> act_z) ic[i++] = sp2;
		lc[l++] = sp2;
		where = 2; /* switch to left cone */
	    }
	    else if (R3Dot (rcp, cp2) >= 0) {
		/* we go back to the right cone */
		pointR3Intersect (ksp, spcw, sp2, sp1, &p);
		if (p.x == sp2 -> act_x && p.y == sp2 -> act_y && p.z == sp2 -> act_z) ic[i++] = sp2;
		rc[r++] = sp2;
		where = 0; /* switch to right cone */
	    }
	    else ic[i++] = sp2; /* we stay in the right cone */
	    break;

	case 2:
	    /* we are in the left cone */
	    R3ActVector (sp2, spccw, v3);
	    R3Cross (v1, v3, cp1);
	    if (R3Dot (rcp, cp1) < 0) lc[l++] = sp2; /* we stay in the left cone */
	    else {
		pointR3Intersect (ksp, spccw, sp2, sp1, &p);
		if (p.x == sp2 -> act_x && p.y == sp2 -> act_y && p.z == sp2 -> act_z) lc[l++] = sp2;
		ic[i++] = sp2;
		where = 1; /* switch to inner cone */
	    }
	    break;
	}
    }
    lc[l] = NULL; ic[i] = NULL; rc[r] = NULL;
}

Private void splitLongEdges (face_t *face, spider_t *ksp, spider_t *vsp[], spider_t *asp[])
{
    double alpha;
    int i, i1, j, k, l, n;
    pointR3_t p1, p2, v1, v2;
    spider_t *sp1, *sp2;

#define SIGMA (M_PI/6)

    k = 0; while (vsp[k]) ++k; /* determine entries vsp */

    j = 0;
    sp1 = ccwa (ksp, face);
    for (sp2 = ccwa (sp1, face); sp2 != ksp; sp1 = sp2, sp2 = ccwa (sp2, face)) {
	for (i = 1; i < k; i++) {
	    if ((i1 = i+1) == k) i1 = 0;
	    R3ActSet (p1, vsp[i]);
	    R3ActSet (p2, vsp[i1]);

	    if (pointR3OnEdge (&p1, sp1, sp2) && pointR3OnEdge (&p2, sp1, sp2)) {
		/* Try to subdivide the edge vsp[i] -> vsp[i1] */
		R3ActVector (vsp[i], ksp, v1);
		R3ActVector (vsp[i1], ksp, v2);

		/* Calculate angle alpha */
		alpha = Acos (R3Dot (v1, v2) / (R3Norm (v1) * R3Norm (v2)));

                n = (int) floor (alpha / SIGMA);
		for (l = 1; l < n; l++) {
		    p1.x = ((n - l) * vsp[i] -> act_x + l * vsp[i1] -> act_x) / n;
		    p1.y = ((n - l) * vsp[i] -> act_y + l * vsp[i1] -> act_y) / n;
		    p1.z = ((n - l) * vsp[i] -> act_z + l * vsp[i1] -> act_z) / n;

		    ASSERT (pointR3OnEdge (&p1, vsp[i], vsp[i1]));
		    asp[j++] = spiderAdd (vsp[i], vsp[i1], &p1);
		}
	    }
	}
    }
    asp[j] = NULL;
}

Private void mergeSortSets (face_t *face, spider_t *vsp[], spider_t *asp[], spider_t *u[])
{
    pointR3_t p;
    int i, j, k, l;

    k = 0; while (vsp[k]) ++k; /* determine entries vsp */

    j = l = 0;
    for (i = 2; i <= k; i++) {
	if (asp[j]) {
	    R3ActSet (p, asp[j]);
	    while (pointR3OnEdge (&p, vsp[i-1], vsp[i%k])) {
		u[l++] = asp[j++];
		if (!asp[j]) break;
		R3ActSet (p, asp[j]);
	    }
	}
	if (i < k) u[l++] = vsp[i];
    }
    u[l] = NULL;
}

Private void resolveFace (face_t *face, spider_t *ksp, spider_t **sp1, spider_t **sp2)
{
    double mu, gw, max;
    int i, j, k;
    spider_t *spcw, *spccw, **s, *sp;
    spider_t * u[MAX_VERTICES];   /* Union set V1 U V2 */
    spider_t * v[MAX_VERTICES];   /* Voronoi neighbours */
    spider_t * wI[MAX_VERTICES];  /* Points in inner cone */
    spider_t * wR[MAX_VERTICES];  /* Points in left cone */
    spider_t * wL[MAX_VERTICES];  /* Points in right cone */
    spider_t * vsp[MAX_VERTICES]; /* Visibility set */
    spider_t * asp[MAX_VERTICES]; /* Angle vertices set */

/* Angle constraints */
#define LAMBDA 2*M_PI/9
#define TAU    M_PI/9

    spcw  = cwa  (ksp, face);
    spccw = ccwa (ksp, face);

    /* According to:
     * D.T. Lee, "Visibility of a Simple Polygon",
     * Comp. Vision, Graph. Image Process. 22, pp. 207-221 (1983)
     */
    visibleSpiders (face, ksp, vsp);      /* make: vsp */
    splitLongEdges (face, ksp, vsp, asp); /* make: asp */
    mergeSortSets  (face, vsp, asp, u);   /* make: u */

    v[0] = NULL; /* Not implemented: voronoiNeighbours (face, ksp, u, v); */

    /* Now we know u and v, so we can really begin */

    for (k = 1; k <= 2; k++) {

	if (k == 1) s = v;
	else s = u;

	divideFace (face, ksp, s, wL, wI, wR); /* make: wL, wI, wR */

	*sp1 = NULL; *sp2 = NULL;
	mu = 0.0; max = 0.0;

	for (i = 0; wI[i]; i++) {
	    ASSERT (ksp != wI[i]);
	    gw = minimumAngle (face, ksp, wI[i], (spider_t *) NULL);
	    if (gw > max) { *sp1 = wI[i]; max = gw; }
	}
	if (max >= LAMBDA) {
	    spiderRemove (u, face, sp1, (spider_t **) NULL);
	    return;
	}
	else mu = max;

	sp = *sp1;
	max = 0.0;

	for (i = 0; wR[i]; i++) {
	    for (j = 0; wL[j]; j++) {
		gw = minimumAngle (face, ksp, wR[i], wL[j]);
		if (gw > max) { *sp1 = wR[i]; *sp2 = wL[i]; max = gw; }
	    }
	}
	if (max > mu) mu = max;
	else { *sp1 = sp; *sp2 = NULL; }
	if (mu >= TAU) {
	    spiderRemove (u, face, sp1, sp2);
	    return;
	}
    }
    spiderRemove (u, face, sp1, sp2);
}

/* B. Joe and R.B. Simpson, "Triangular Meshes for Regions
 * of Complicated Shape",
 * Int. J. Numer. Methods Eng., vol. 23, pp. 751-778 (1986).
 *
 * Main part implemented and works.
 */
Private void algorithmJoeSimpson (face_t *face, spider_t *ksp)
{
    int i;
    face_t *new_face, *f;
    spider_t *sp, *sp1, *sp2, *corners[4];

    for (i = 0; i < 4; i++) corners[i] = SP(i);

    /* The important routine that determines
     * the separator(s) is now called.
     */
    resolveFace (face, ksp, &sp1, &sp2);

    if (!sp1) {
	char buf[180];
	sprintf (buf, "For conductor '%s'\n   in the area", conNr2Name (ksp -> conductor));
	if (corners[0]) {
	    strcat (buf, " ");
	    strcat (buf, strCoorBrackets ((coor_t)(corners[0] -> nom_x), (coor_t)(corners[0] -> nom_y)));
	}
	if (corners[1]) {
	    strcat (buf, " ");
	    strcat (buf, strCoorBrackets ((coor_t)(corners[1] -> nom_x), (coor_t)(corners[1] -> nom_y)));
	}
	if (corners[2]) {
	    strcat (buf, " ");
	    strcat (buf, strCoorBrackets ((coor_t)(corners[2] -> nom_x), (coor_t)(corners[2] -> nom_y)));
	}
	if (corners[3]) {
	    strcat (buf, " ");
	    strcat (buf, strCoorBrackets ((coor_t)(corners[3] -> nom_x), (coor_t)(corners[3] -> nom_y)));
	}
	say ("There are problems with 3D mesh generation.  %s.\n%s\n%s", buf,
	"   A solution may be to increase the parameter 'default_step_slope' and/or",
	"   to decrease the 'eshapes' values in the element definition file.");
	die ();
    }

    /* The face is split along ksp -> sp1. */
    new_face = meshRenameFace (ksp, sp1, face);
    meshMakeEdge (ksp, sp1, INTERNALEDGE);
    meshSetFaces (ksp, sp1, face, new_face);
    meshSetFaceCorners (face, ksp, sp1, corners);
    meshSetFaceCorners (new_face, sp1, ksp, corners);

    if (sp2) {
	/* If sp2 belongs to new_face, switch face and new_face. */
	for (sp = ccwa (sp1, new_face); (sp != sp1 && sp != sp2); sp = ccwa (sp, new_face));
	if (sp == sp2) { f = face; face = new_face; new_face = f; }

	Debug (meshCheckLoop (new_face));
	Debug (meshPrintFace (new_face));
	/* Put the new_face already in the recursion queue. */
	if (!new_face -> corners[3] || convexFace (new_face)) faceRecur (new_face);

	/* Reset the face corners. */
	for (i = 0; i < 4; ++i) corners[i] = SP(i);

	/* Face is also split along ksp -> sp2. */
	new_face = meshRenameFace (ksp, sp2, face);
	meshMakeEdge (ksp, sp2, INTERNALEDGE);
	meshSetFaces (ksp, sp2, face, new_face);
	meshSetFaceCorners (face, ksp, sp2, corners);
	meshSetFaceCorners (new_face, sp2, ksp, corners);
    }

    Debug (meshCheckLoop (face));
    Debug (meshCheckLoop (new_face));
    Debug (meshPrintFace (face));
    Debug (meshPrintFace (new_face));

    /* Put both faces in the recursion queue. */
    if (!SP(3) || convexFace (face)) faceRecur (face);
    face = new_face;
    if (!SP(3) || convexFace (face)) faceRecur (face);
}

Private void meshSetFaceCorners (face_t *face, spider_t *sp1, spider_t *sp2, spider_t *corners[])
{
    int i, imin, imax, ireg;
    spider_t *sp, *csp[40];
    meshCoor_t d1, d2;

#define CORNER(sp) (sp == corners[0] || sp == corners[1] || sp == corners[2] || sp == corners[3])

    /* Find the corners of the two faces */
    i = 0;
    csp[i++] = sp1;
    for (sp = ccwa (sp1, face); sp != sp1; sp = ccwa (sp, face)) {
	if (CORNER (sp) || sp == sp2) csp[i++] = sp;
    }

    if (i == 3) {
	/* It's a triangle : so we must check that
	 * there are no spiders on the edges left.
	 * If this is the case, we treat the new face as if
	 * it were a trapezoid: we start with the longest edge.
	 */

#define Sp(i) csp[(i)%3]

	imin = imax = ireg = 0;
	for (i = 1; i < 3; i++) {
		 if (spiderDist (Sp(i), Sp(i+1)) > spiderDist (Sp(imax), Sp(imax+1))) imax = i;
	    else if (spiderDist (Sp(i), Sp(i+1)) < spiderDist (Sp(imin), Sp(imin+1))) imin = i;
	    else ireg = i;
	}

	     if (!meshFindEdge (Sp(imax), Sp(imax+1))) i = imax;
	else if (!meshFindEdge (Sp(ireg), Sp(ireg+1))) i = ireg;
	else if (!meshFindEdge (Sp(imin), Sp(imin+1))) i = imin;
	else sp1 = NULL;

	if (sp1) {
	    d1 = spiderDist (Sp(i), Sp(i+1));
	    for (sp = ccwa (Sp(i), face); sp != Sp(i+1); sp = ccwa (sp, face)) {
		if ((d2 = Abs(spiderDist (Sp(i), sp) - spiderDist (Sp(i+1), sp))) < d1) {
		    d1 = d2; sp1 = sp;
		}
	    }
	}
	meshSetCorners (face, Sp(i+1), Sp(i+2), Sp(i), sp1);
    }
    else {
	if (i > 4) {
            i = 0;
	    csp[i++] = sp1;
	    for (sp = ccwa (sp1, face); sp != sp1; sp = ccwa (sp, face)) {
		if (CORNER (sp)) csp[i++] = sp;
	    }
        }
	ASSERT (i == 4); /* it stayes a trapezoid */
	meshSetCorners (face, csp[0], csp[1], csp[2], csp[3]);
    }
}
