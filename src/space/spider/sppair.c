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
#include <math.h>               /* contains ceil */
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/spider/define.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

#include "src/space/scan/export.h"

extern tileBoundary_t *bdr;
extern strip_t *stripA, *stripAA;
extern bool_t *diffusionConductor;
extern subnode_t *subnSUB;

extern int connect_ground;
extern int nrOfCondStd;
extern int numVertices;
extern int numJoins;
static int vside;
static int isGate;
static tile_t *tile_below;
static strip_t *the_strip;

#ifdef __cplusplus
  extern "C" {
#endif
Private void tryFace (tile_t *tile, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4,
			bool_t coarse, bool_t gate);
Private void spiderFindNew (tile_t *tile, int side, meshCoor_t x1, meshCoor_t y1, meshCoor_t x2, meshCoor_t y2,
	meshCoor_t z, int level, int conductor, int equal, spider_t **sp1, spider_t **sp2);
Private spider_t *spiderNew (meshCoor_t x, meshCoor_t y, meshCoor_t z, int conductor, tile_t *tile);
Private face_t *spiderFindFace (tile_t *tile, int level, bool_t core, bool_t coarse, int type);
#ifdef __cplusplus
  }
#endif

/*
** tile_l is left tile and tile_r is right tile when traversing
** their common boundary from x1,y1 to x2,y2.
*/
void spiderPair (tile_t *tile_l, tile_t *tile_r, int orientation)
{
    edgeType_t etype;
    spider_t *bl1, *bl2, *br1, *br2;
    spider_t *tl1, *tl2, *tr1, *tr2;
    spider_t *b1, *b2, *t1, *t2;
    meshDef_t **mesh, *m;
    meshCoor_t x_1, y_1, x_2, y_2;
    int i, j, conductor, coarse, blevel, tlevel, g1, g2, solid, equal;
    spiderEdge_t *e = NULL;

    the_strip = (bdr -> x1 < stripA -> xr || stripA -> xr >= bigbxr)? stripA : stripAA;
    x_1 = bdr -> x1;
    y_1 = bdr -> y1;
    x_2 = bdr -> x2;
    y_2 = bdr -> y2;

    vside = (orientation == 'v');
    if (orientation != 'v') Swap (tile_t *, tile_l, tile_r);

    Debug ((void)fprintf (stderr, "spiderPair (%ld,%ld) (%ld,%ld), o: %c\n",
	(long)bdr -> x1, (long)bdr -> y1, (long)bdr -> x2, (long)bdr -> y2, orientation));
    Debug ((void)fprintf (stderr, "spider enum pair %g %g %g %g tiles %p, %p\n",
	(double) x_1, (double) y_1, (double) x_2, (double) y_2, tile_l, tile_r));

    if (prePass1) g1 = g2 = -1;
    else {
	g1 = tile_l -> tor ? tile_l -> tor -> type -> s.tor.gCon : -1;
	g2 = tile_r -> tor ? tile_r -> tor -> type -> s.tor.gCon : -1;
    }

    /* find the mesh between tile_l and tile_r */
    mesh = recogMesh (tile_l, tile_r, &i);

    for (; (m = mesh[i]); i++) if (m -> solid_l || m -> solid_r) {

	solid = (m -> solid_l && m -> solid_r);

	bl1 = br1 = bl2 = br2 = NULL;
	tl1 = tr1 = tl2 = tr2 = NULL;
	isGate = 0;

	if (prePass1) {
	    conductor = nrOfCondStd;
	    blevel = 0;
	}
	else {
	    conductor = i; /* >= 0 */
	    blevel = 2 * conductor;
	    if (g1 == conductor || g2 == conductor) isGate |= 1 << 31;
	    else if (diffusionConductor[conductor]) isGate |= 1 << 30;
	}

	coarse = FALSE;

	if ((m -> solid_l && m -> ybl < m -> ytl)
	||  (m -> solid_r && m -> ybr < m -> ytr))
	    tlevel = blevel + 1;
	else {
	    /*
	    ** We want to do only the top level, not the bottom level,
	    ** This makes it easier to generate a hgf file.
	    */
	    tlevel = blevel;
	    blevel = -100;
	    if (spiderControl.maxCoarseFeArea > 0) coarse = TRUE;
	}

	/* No kidding below, see spiderFindNew
	** FindLFT means left tile, and the spiders are located along
	** its right and bottom edges. Mutatis mutandis for FindRGT.
	*/
#define FindLFT(z, l, c, s1, s2, eq) spiderFindNew (tile_l, RGT_BOT, x_1, y_1, x_2, y_2, z, l, c, eq, s1, s2)
#define FindRGT(z, l, c, s1, s2, eq) spiderFindNew (tile_r, LFT_TOP, x_1, y_1, x_2, y_2, z, l, c, eq, s1, s2)

	/* FVF:
	** We must have spiders that bound the face between the two tiles.
	** The #1 and #2 refer to the two points that defines the edge between
	** the two tiles. Left and right refer to the left and right tile, and
	** top and bottom refer to the top and bottom of this metal layer.
	*/

	/* Depending on the boundary orientation, we do first LFT or first
	   RGT, to be sure that a newly created spider is always added
	   to 'tile' when there is a choice between 'tile' and 'newerTile'.
	   This property is used later on in meshAddExtraSpider().
	*/
	if (blevel >= 0) {
	equal = (solid && m -> ybl == m -> ybr);
	if (orientation == 'v') {
	    tile_below = NULL;
	    if (m -> solid_l) FindLFT (m -> ybl, blevel, conductor, &bl1, &bl2, 0);
	    if (m -> solid_r) FindRGT (m -> ybr, blevel, conductor, &br1, &br2, equal);
	}
	else { /* 'h' */
	    tile_below = equal ? tile_l : NULL;
	    if (m -> solid_r) FindRGT (m -> ybr, blevel, conductor, &br1, &br2, 0);
	    if (m -> solid_l) FindLFT (m -> ybl, blevel, conductor, &bl1, &bl2, equal);
	}
	}

	equal = (solid && m -> ytl == m -> ytr);
	if (orientation == 'v') {
	    tile_below = NULL;
	    if (m -> solid_l) FindLFT (m -> ytl, tlevel, conductor, &tl1, &tl2, 0);
	    if (m -> solid_r) FindRGT (m -> ytr, tlevel, conductor, &tr1, &tr2, equal);
	}
	else { /* 'h' */
	    tile_below = equal ? tile_l : NULL;
	    if (m -> solid_r) FindRGT (m -> ytr, tlevel, conductor, &tr1, &tr2, 0);
	    if (m -> solid_l) FindLFT (m -> ytl, tlevel, conductor, &tl1, &tl2, equal);
	}

	/* Create necessary links along the tile boundaries.
	*/
	if (blevel >= 0 && !(m -> contb_l && m -> contb_r)) { /* not "inside" contact */
	    if (m -> contb_l || m -> contb_r) etype = CONTACTEDGE_H;
	    else if (bl1 && br1) etype = bl1 != br1 ? CROSSOVEREDGE : INTERNALEDGE;
	    else etype = CONDUCTOREDGE;
	    if (bl1) {
		e = meshMakeEdge (bl1, bl2, etype);
		if (m -> contb_l) m -> contb_l = e;
	    }
	    if (br1) {
		if (br1 != bl1) e = meshMakeEdge (br1, br2, etype);
		if (m -> contb_r) m -> contb_r = e;
	    }
	}

	if (!(m -> conta_l && m -> conta_r)) { /* not "inside" contact */
	    if (m -> conta_l || m -> conta_r) etype = CONTACTEDGE_H;
	    else if (tl1 && tr1) etype = tl1 != tr1 ? CROSSOVEREDGE : INTERNALEDGE;
	    else etype = CONDUCTOREDGE;
	    if (tl1) {
		e = meshMakeEdge (tl1, tl2, etype);
		if (m -> conta_l) m -> conta_l = e;
	    }
	    if (tr1) {
		if (tr1 != tl1) e = meshMakeEdge (tr1, tr2, etype);
		if (m -> conta_r) m -> conta_r = e;
	    }
	}

#define Face(sp1, sp2, tile, level, core, type) \
	meshSetFace (sp1, sp2, spiderFindFace (tile, level, core, coarse, type))

	/* Set top and bottom faces.
	** Be careful with the orientation of the faces.
	*/
	if (bl1 && !m -> contb_l) {
	    int type = FACE_BOT;
	    if (g1 == conductor) type += FACE_GATE;
	    Face (bl2, bl1, tile_l, blevel, m -> coreb_l, type);
	}
	if (br1 && !m -> contb_r) {
	    int type = FACE_BOT;
	    if (g2 == conductor) type += FACE_GATE;
	    Face (br1, br2, tile_r, blevel, m -> coreb_r, type);
	}
	if (tl1 && !m -> conta_l) {
	    int type = (blevel < 0)? FACE_BOT + FACE_TOP : FACE_TOP;
	    if (m -> corea_l) { type += FACE_CAP2D;
		tl1 -> isGate |= 1 << 29;
		tl2 -> isGate |= 1 << 29;
	    }
	    if (g1 == conductor) type += FACE_GATE;
	    Face (tl1, tl2, tile_l, tlevel, FALSE, type);
	}
	if (tr1 && !m -> conta_r) {
	    int type = (blevel < 0)? FACE_BOT + FACE_TOP : FACE_TOP;
	    if (m -> corea_r) { type += FACE_CAP2D;
		tr1 -> isGate |= 1 << 29;
		tr2 -> isGate |= 1 << 29;
	    }
	    if (g2 == conductor) type += FACE_GATE;
	    Face (tr2, tr1, tile_r, tlevel, FALSE, type);
	}

	/* Check for contact faces. But not before we know that the "other"
	** part of the contact is created.
	*/

	j = m -> contb_oconductor;

	/* check first for contact below us */
	if (m -> contb_l && !m -> contb_r /* && j < i */) {
	    if (!(bl1 && bl2)) {
		if (j < i) say ("Missing bottom spider (flat conductor?) -- contact lost\n");
		goto end_below;
	    }
	    if (solid && m->esbr) {
		spiderDelayedDisplace (EDGESHAPEDISPL, bl1, bl2, m->esbr, blevel, LEFT);
	    }
	    if (j > i) goto end_below;

	    if (j < 0) {
		FindLFT (0, nrOfSpiderLevels, nrOfCondStd, &t1, &t2, 0);
		meshMakeEdge (t1, t2, CONTACTEDGE_H);
	    }
	    else {
		e = mesh[j] -> conta_l;
		if (e && e != (spiderEdge_t*)TRUE) { t1 = e -> sp; t2 = e -> oh -> sp; }
		else t1 = t2 = NULL;
	    }

	    if (!(t1 && t2)) {
		say ("Missing contact spider (illegal layout?) -- contact lost\n");
	    }
	    else if (bl1 != t1) {
		meshMakeEdge (bl1, t1, CONTACTEDGE);
		meshMakeEdge (t2, bl2, CONTACTEDGE);
		tryFace (tile_l, bl1, t1, t2, bl2, FALSE, FALSE);
	    }
	}
	if (m -> contb_r && !m -> contb_l /* && j < i */) {
	    if (!(br1 && br2)) {
		if (j < i) say ("Missing bottom spider (flat conductor?) -- contact lost\n");
		goto end_below;
	    }
	    if (solid && m->esbl) {
		spiderDelayedDisplace (EDGESHAPEDISPL, br1, br2, m->esbl, blevel, RIGHT);
	    }
	    if (j > i) goto end_below;

	    if (j < 0) {
		FindRGT (0, nrOfSpiderLevels, nrOfCondStd, &t1, &t2, 0);
		meshMakeEdge (t1, t2, CONTACTEDGE_H);
	    }
	    else {
		e = mesh[j] -> conta_r;
		if (e && e != (spiderEdge_t*)TRUE) { t1 = e -> sp; t2 = e -> oh -> sp; }
		else t1 = t2 = NULL;
	    }

	    if (!(t1 && t2)) {
		say ("Missing contact spider (illegal layout?) -- contact lost\n");
	    }
	    else if (br1 != t1) {
		meshMakeEdge (t1, br1, CONTACTEDGE);
		meshMakeEdge (br2, t2, CONTACTEDGE);
		tryFace (tile_r, br1, br2, t2, t1, FALSE, FALSE);
	    }
	}
end_below:
	j = m -> conta_oconductor;

	/* then check for contact above us */
	if (m -> conta_l && !m -> conta_r /* && j < i */) {
	    if (solid && m->estr) {
		spiderDelayedDisplace (EDGESHAPEDISPL, tl1, tl2, m->estr, tlevel, LEFT);
	    }
	    if (j > i) goto end_above;
	    e = mesh[j] -> contb_l;
	    if (e && e != (spiderEdge_t*)TRUE) { b1 = e -> sp; b2 = e -> oh -> sp; }
	    else b1 = b2 = NULL;

	    if (!(b1 && b2)) {
		say ("Missing bottom spider (flat conductor?) -- contact lost\n");
	    }
	    else if (b1 != tl1) {
		meshMakeEdge (b1, tl1, CONTACTEDGE);
		meshMakeEdge (tl2, b2, CONTACTEDGE);
		tryFace (tile_l, b1, tl1, tl2, b2, FALSE, FALSE);
	    }
	}
	if (m -> conta_r && !m -> conta_l /* && j < i */) {
	    if (solid && m->estl) {
		spiderDelayedDisplace (EDGESHAPEDISPL, tr1, tr2, m->estl, tlevel, RIGHT);
	    }
	    if (j > i) goto end_above;
	    e = mesh[j] -> contb_r;
	    if (e && e != (spiderEdge_t*)TRUE) { b1 = e -> sp; b2 = e -> oh -> sp; }
	    else b1 = b2 = NULL;

	    if (!(b1 && b2)) {
		say ("Missing bottom spider (flat conductor?) -- contact lost\n");
	    }
	    else if (b1 != tr1) {
		meshMakeEdge (tr1, b1, CONTACTEDGE);
		meshMakeEdge (b2, tr2, CONTACTEDGE);
		tryFace (tile_r, b1, b2, tr2, tr1, FALSE, FALSE);
	    }
	}
end_above:
	/* ---- contact faces done ----- */

	if (solid) {
	    /* 1) Create vertical crossover links (edges).
	     * 2) Make crossover sidewall face with tryFace.
	     */
	    if (tl1 != tr1) {
		meshMakeEdge (tl1, tr1, CROSSOVEREDGE);
		meshMakeEdge (tr2, tl2, CROSSOVEREDGE);
		spiderDelayedDisplace (SLOPEDISPL, tl1, tl2, m->cstl, tlevel, LEFT);
		spiderDelayedDisplace (SLOPEDISPL, tr1, tr2, m->cstr, tlevel, RIGHT);
		if (!(m -> conta_l && m -> conta_r)) { /* not "inside" contact */
		    tryFace (tile_l, tl2, tl1, tr1, tr2, coarse, FALSE);
		}
	    }
	    if (blevel >= 0 && bl1 != br1) {
		meshMakeEdge (br1, bl1, CROSSOVEREDGE);
		meshMakeEdge (bl2, br2, CROSSOVEREDGE);
		spiderDelayedDisplace (SLOPEDISPL, bl1, bl2, m->csbl, blevel, LEFT);
		spiderDelayedDisplace (SLOPEDISPL, br1, br2, m->csbr, blevel, RIGHT);
		if (!(m -> contb_l && m -> contb_r)) { /* not "inside" contact */
		    tryFace (tile_l, bl1, bl2, br2, br1, coarse, FALSE);
		}
	    }
	}
	else if (blevel >= 0) {
	    /* 1) Create vertical links for conductor sidewall.
	     * 2) Make conductor sidewall face with tryFace.
	     */
	    if (m -> solid_l) {
		meshMakeEdge (tl1, bl1, VERTICALEDGE);
		meshMakeEdge (bl2, tl2, VERTICALEDGE);
		spiderDelayedDisplace (EDGESHAPEDISPL, bl1, bl2, m->esbl, blevel, RIGHT);
		spiderDelayedDisplace (EDGESHAPEDISPL, tl1, tl2, m->estl, tlevel, RIGHT);
		tryFace (tile_l, tl2, tl1, bl1, bl2, coarse, g1 == conductor);
	    }
	    else { /* solid_r */
		meshMakeEdge (br1, tr1, VERTICALEDGE);
		meshMakeEdge (tr2, br2, VERTICALEDGE);
		spiderDelayedDisplace (EDGESHAPEDISPL, br1, br2, m->esbr, blevel, LEFT);
		spiderDelayedDisplace (EDGESHAPEDISPL, tr1, tr2, m->estr, tlevel, LEFT);
		tryFace (tile_r, tr1, tr2, br2, br1, coarse, g2 == conductor);
	    }
	}
    }
}

Private subnode_t * getConnectGround (tile_t *tile)
{
    if (connect_ground == 2 && tile -> subcont) { // distributed
	return tile -> subcont -> subn;
    }
    return subnSUB;
}

Private void tryFace (tile_t *tile, spider_t *sp1, spider_t *sp2, spider_t *sp3, spider_t *sp4,
	bool_t coarse, bool_t gate)
{
    face_t *face = newFace ();

    if (connect_ground) face -> sc_subn = getConnectGround (tile);
    face -> type = FACE_SIDE;
    if (vside) face -> type += FACE_VSIDE;
    if (gate)  face -> type += FACE_GATE;
    if (coarse)face -> type += FACE_COARSE;

    meshSetFace (sp1, sp2, face);
    meshSetFace (sp2, sp3, face);
    meshSetFace (sp3, sp4, face);
    meshSetFace (sp4, sp1, face);
    meshSetCorners (face, sp1, sp2, sp3, sp4);
    stripAddFace (face, the_strip);
}

Private void spiderJoin (spider_t *sp1, spider_t *sp2, int level)
{
    face_t *face;
    spiderEdge_t *eh, *eh2, *nb, *oh, *e;
    spider_t *sp;
    int i;

 // fprintf (stderr, "spiderJoin: spiders at point (%g %g %g) level=%d\n", sp1->nom_x/4, sp1->nom_y/4, sp1->nom_z/4, level);

    for (eh2 = sp2 -> edge; eh2; eh2 = eh2 -> nb) {
	if (eh2 -> oh -> sp -> nom_z != sp2 -> nom_z) break;
    }
    for (eh = sp1 -> edge; eh; eh = nb) {
	nb = eh -> nb;
	oh = eh -> oh;
	sp = oh -> sp;
	if ((face = eh -> face)) {
	    for (i = 0; i < 4; ++i) if (face -> corners[i] == sp1) { face -> corners[i] = sp2; break; }
	}
	if (sp -> nom_z != sp1 -> nom_z) { /* ver.edge */
	    for (e = eh2; e; e = e -> nb) {
		if (e -> oh -> sp -> nom_z == sp -> nom_z) {
		    if (e -> oh -> sp == sp) {
			if (e -> face) {
			    ASSERT (!face && oh -> face);
			    ASSERT (!e -> oh -> face);
			    e -> oh -> face = oh -> face;
			} else {
			    ASSERT (face && !oh -> face);
			    ASSERT (e -> oh -> face);
			    e -> face = face;
			}
			eh -> sp = oh -> sp = NULL;
			if ((e = sp -> edge) == oh) sp -> edge = oh -> nb;
			else {
			    while (e -> nb != oh) { e = e -> nb; ASSERT (e); }
			    e -> nb = oh -> nb;
			}
			goto skip_eh;
		    }
		    break;
		}
	    }
	}
	eh -> nb = sp2 -> edge;
	sp2 -> edge = eh;
	eh -> sp = sp2;
skip_eh:	;
    }
    sp1 -> edge = NULL;

    if (sp2 -> isGate != sp1 -> isGate) sp2 -> isGate |= sp1 -> isGate;

    if (isDisplace (sp1)) spiderDisplaceRemove (sp1, sp2, level);
    if (sp1 -> strip && FeModePwl) stripRemoveSpider (sp1);
    numVertices--;
    numJoins++;
}

Private void spiderFindNew (tile_t *tile, int side, meshCoor_t x1, meshCoor_t y1, meshCoor_t x2, meshCoor_t y2,
	meshCoor_t z, int level, int conductor, int equal, spider_t **Sp1, spider_t **Sp2)
{
    static spider_t *sp1, *sp2;

    if (equal) {
	spider_t *sp3;
	if (!(sp3 = tile -> mesh -> spider[side][level])) {
	    if (vside) {
		sp3 = tile -> stb -> mesh -> spider[LFT_TOP][level];
		if (sp3 && sp3 -> nom_x == x1 && sp3 -> nom_y == y1 && sp3 -> nom_z == z) {
		    if (sp3 != sp1) {
			spiderJoin (sp3, sp1, level); // remove sp3
			tile -> stb -> mesh -> spider[LFT_TOP][level] = sp1;
		    }
		}
	    }
	}
	else if (sp3 != sp1) spiderJoin (sp3, sp1, level); // remove sp3
	goto found;
    }

    sp1 = tile -> mesh -> spider[side][level];
    if (sp1) {
	if (sp1 -> nom_x == x1 && sp1 -> nom_y == y1 && sp1 -> nom_z == z) goto found1;
	/* above if is normally true, below a hack if not */
	sp1 = tile -> stl -> mesh -> spider[side][level];
	if (sp1 && sp1 -> nom_x == x1 && sp1 -> nom_y == y1 && sp1 -> nom_z == z) goto found1;
    }
    if (side == LFT_TOP) {
	sp1 = tile -> stb -> mesh -> spider[LFT_TOP][level];
	if (sp1 && sp1 -> nom_x == x1 && sp1 -> nom_y == y1 && sp1 -> nom_z == z) goto found1;
    }
    if (tile_below && tile != tile_below) { // 'h' && tile_below == tile_l
	sp1 = tile_below -> mesh -> spider[RGT_BOT][level];
	if (sp1 && sp1 -> nom_x == x1 && sp1 -> nom_y == y1 && sp1 -> nom_z == z) goto found1;
    }
    sp1 = spiderHashLookUp (x1, y1, z);
found1:
    Debug ((void)fprintf (stderr, "sp1 at (%g,%g,%g) level %d, %s in tile %p\n",
	x1, y1, z, level, sp1 ? "found" : "not found", tile));

    if (!sp1) sp1 = spiderNew (x1, y1, z, conductor, tile);
    else if (isGate) sp1 -> isGate |= isGate;

    sp2 = tile -> mesh -> spider[!side][level];
    if (sp2 && sp2 -> nom_x == x2 && sp2 -> nom_y == y2) goto found2;
    sp2 = spiderHashLookUp (x2, y2, z);
found2:
    Debug ((void)fprintf (stderr, "sp2 at (%g,%g,%g) level %d, %s in tile %p\n",
	x2, y2, z, level, sp2 ? "found" : "not found", tile));

    if (!sp2) sp2 = spiderNew (x2, y2, z, conductor, tile);
    else if (isGate) sp2 -> isGate |= isGate;
found:
   *Sp1 = sp1;
   *Sp2 = sp2;
    if (!tile -> mesh -> spider[side][level]) tile -> mesh -> spider[!side][level] = sp1;
    tile -> mesh -> spider[side][level] = sp2;
}

Private spider_t * spiderNew (meshCoor_t x, meshCoor_t y, meshCoor_t z, int conductor, tile_t *tile)
{
    spider_t  *spider;
    subnode_t *subnode, *subnodeSC = NULL;

    if (prePass1) { /* optSubRes */
	ASSERT (tile -> subcont);
	subnode = tile -> subcont -> subn;
    }
    else if (!optRes) {
	subnode = conductor == nrOfCondStd ? subnSUB : tile -> cons[conductor];
	if (connect_ground) subnodeSC = getConnectGround (tile);
    }
    else {
	/* Actually, we should seek the closest subnode.
	** The tile has one or more nodePoints, when it contains
	** a conductor. If the tile has only one nodePoint, then it
	** contains only low res conductors (this nodePoint is the
	** lower left tile corner). Each nodePoint contains subnode
	** pointers, which are all set for all tile conductors.
	**
	** Note that only for the first !zero edge of a tile spiderNew
	** is done by spiderFindNew (this is normally a vertical edge).
	** We don't need to seek for the closest nodePoint, because
	** we can always use the tlPoints and rbPoints nodePoints. (SdeG)
	*/
	nodePoint_t *closest_np = tile -> rbPoints;
	ASSERT (closest_np);

	if (closest_np != tile -> tlPoints) { /* no low res tile */
	    if ((meshCoor_t)closest_np -> x != x || (meshCoor_t)closest_np -> y != y) {
		closest_np = tile -> tlPoints;
		//ASSERT ((meshCoor_t)closest_np -> x == x && (meshCoor_t)closest_np -> y == y);
	    }
	}
	if (conductor == nrOfCondStd) {
	    if (tile -> subcont) subnode = tile -> subcont -> subn;
	    else subnode = subnSUB;
	}
	else subnode = closest_np -> cons[conductor];

	if (connect_ground) {
	    subnodeSC = subnSUB;
	    if (connect_ground == 2 && tile -> subcont) { // distributed
		subnodeSC = tile -> subcont -> subn;
		ASSERT (subnodeSC);
	    }
	}
    }
    ASSERT (subnode);

    spider = newSpider (x, y, z, x, y, z, 1, subnode, subnodeSC, conductor, isGate);

    Debug ((void) fprintf (stderr, "New spider at %g %g %g, conductor %d created\n",
	(double) x, (double) y, (double) z, conductor));

    return (spider);
}

Private face_t * spiderFindFace (tile_t *tile, int level, bool_t core, bool_t coarse, int type)
{
    face_t *face;
    if (!(face = tile -> mesh -> faces[level])) {
	tile -> mesh -> faces[level] = face = newFace ();
	face -> type = type;
	if (coarse)face -> type += FACE_COARSE;
	if (core)  face -> type += FACE_CORE;
	if (prePass1) { /* optSubRes */
	    ASSERT (tile -> subcont);
	    face -> sc_subn = tile -> subcont -> subn;
	}
	else if (connect_ground) face -> sc_subn = getConnectGround (tile);
	stripAddFace (face, the_strip);
    }
    /* meshSetCorners is performed in spiderEnumTile */
    return (face);
}
