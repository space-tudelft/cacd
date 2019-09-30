/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stdio.h>
#include <string.h>
#include <math.h>		/* contains ceil () */
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/extract/define.h"
#include "src/space/extract/export.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern tileBoundary_t *bdr;
extern maskinfo_t *masktable;
extern int hasShapeElems;
extern int nrOfMasks;
extern int nrOfCondStd;
extern bool_t contacts_sub;

extern double subcontZposition;
double step_slope = 2;

#ifdef __cplusplus
  extern "C" {
#endif
#ifdef WRONG_MESH_MSG
Private void wrongMesh (int type, tile_t *tile, tile_t *eTile, elemDef_t *el);
#endif
#ifdef __cplusplus
  }
#endif

/*
 * Find which mesh elements are present along
 * boundary of tile_l and tile_r.
 * This routine is in fact a postprocessor for recognizeElements().
 * These operations could also be done by tecc,
 * when a union like meshDef above would be used instead
 * of those define by VDIMELEM and [CE]SHAPEELEM.
 * However, this approach would suffer from a combinatorial
 * explosion in the number of elements to consider.
 */
meshDef_t **recogMesh (tile_t *tile_l, tile_t *tile_r, int *n)
{
    static meshDef_t *meshDefBuf, **meshDef;
    static int *conductorCheck;
    elemDef_t **elem;
    meshDef_t *m;
    int cx, bot_cond, top_cond, i, hasCon_l, hasCon_r;

    /* Initialize
     */
    if (!meshDefBuf) {
	meshDefBuf = NEW (meshDef_t, nrOfCondStd);
	meshDef = NEW (meshDef_t *, nrOfCondStd + 1);
	conductorCheck = NEW (int, nrOfCondStd);
	for (i = 0; i < nrOfCondStd; ++i) conductorCheck[i] = 0;
	meshDef[nrOfCondStd] = NULL;
    }

    Debug ((void)fprintf (stderr, "recogMesh (%p, %p, [(%d, %d)-(%d, %d)])\n",
		tile_l, tile_r, bdr->x1, bdr->y1, bdr->x2, bdr->y2));

    if (prePass1) { /* optSubRes: for substrate contacts */
	*n = 0;
	if (!tile_l -> subcont && !tile_r -> subcont) {
	    meshDef[0] = NULL;
	    return (meshDef);
	}

	/* Re-initialize the meshDef array and the flags.
	 */
	meshDef[0] = m = meshDefBuf;
	m -> solid_l = m -> solid_r = FALSE;
	m -> conta_l = m -> conta_r = NULL;
	m -> contb_l = m -> contb_r = NULL;
	m -> corea_l = m -> corea_r = FALSE;
	m -> coreb_l = m -> coreb_r = FALSE;

	if (tile_l -> subcont) {
	    m -> solid_l = TRUE;
	    /* T.S. Only for contacts on top surface. */
	    m -> ybl = m -> ytl = subcontZposition;
	}
	if (tile_r -> subcont) {
	    m -> solid_r = TRUE;
	    m -> ybr = m -> ytr = subcontZposition;
	}

	meshDef[1] = NULL;
	return (meshDef);
    }

    /* Re-initialize the meshDef array and the solid_[lr] flags.
     */
    for (i = 0; i < nrOfCondStd; ++i) {
	conductorCheck[i] &= 4;
	meshDef[i] = m = meshDefBuf + i;
	m -> solid_l = m -> solid_r = FALSE;
	m -> conta_l = m -> conta_r = NULL;
	m -> contb_l = m -> contb_r = NULL;
	m -> corea_l = m -> corea_r = FALSE;
	m -> coreb_l = m -> coreb_r = FALSE;
	m -> shape = NULL;
    }

    /* See what elements there are in tile_l.
     */
    if ((hasCon_l = HasConduc (tile_l))) {
	elem = recogSurface (tile_l);

	for (i = 0; elem[i]; i++) {
	    switch (elem[i] -> type) {
	    case RESELEM:
		if ((cx = elem[i] -> s.res.con) >= nrOfCondStd) break;
		conductorCheck[cx] |= 1;
		meshDef[cx] -> vdim_l = elem[i];
		break;
	    case VDIMELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.vdim.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		m -> ybl = elem[i] -> s.vdim.height;
		m -> ytl = elem[i] -> s.vdim.height+elem[i] -> s.vdim.thickness;
		/* this must be the only one */
		if (m -> solid_l) {
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> vdim_l -> name, elem[i] -> name);
		}
		else
		    m -> vdim_l = elem[i];
		m -> solid_l = TRUE;
		break;
	    case SURFCAP3DELEM:
		top_cond = elem[i] -> s.cap.pCon;
		bot_cond = elem[i] -> s.cap.nCon;
		if (top_cond < 0 || top_cond >= nrOfCondStd) break;
		if (bot_cond < 0 || bot_cond >= nrOfCondStd) break;
		meshDef[top_cond] -> coreb_l = TRUE;
		meshDef[bot_cond] -> corea_l = TRUE;
		break;
	    case CONTELEM:
		top_cond = elem[i] -> s.cont.con1;
		bot_cond = elem[i] -> s.cont.con2;

		/* Substrate contacts are not supported yet: uses
		 * a invalid entry in the meshDef array (negative
		 * conductor number (FB, NvdM)) !!! See also right
		 * tile recognition below.
		 */
		if (top_cond < 0 || top_cond >= nrOfCondStd) break;
		if (bot_cond < 0 || bot_cond >= nrOfCondStd) {
		    if (!contacts_sub) break;
		    bot_cond = -1;
		}

		/* vdim must be known */
		if (!meshDef[top_cond] -> solid_l) break;
		if (bot_cond >= 0) {
		    if (!meshDef[bot_cond] -> solid_l) break;
		    if (meshDef[top_cond] -> ybl < meshDef[bot_cond] -> ytl) {
			int t = top_cond; top_cond = bot_cond; bot_cond = t; /* swap */
		    }
		    if (meshDef[top_cond] -> ybl <= meshDef[bot_cond] -> ytl) {
			say ("3D geometries of elements '%s' and '%s' coincide at %s",
			    meshDef[top_cond] -> vdim_l -> name,
			    meshDef[bot_cond] -> vdim_l -> name,
			    strCoorBrackets (bdr -> x1, bdr -> y1));
			break;
		    }
		    meshDef[bot_cond] -> conta_l = (spiderEdge_t*)TRUE;
		    meshDef[bot_cond] -> conta_oconductor = top_cond;
		}
		meshDef[top_cond] -> contb_l = (spiderEdge_t*)TRUE;
		meshDef[top_cond] -> contb_oconductor = bot_cond;
		break;
	    }
	}
    }

    /* See what elements there are in tile_r.
     */
    if ((hasCon_r = HasConduc (tile_r))) {
	elem = recogSurface (tile_r);

	for (i = 0; elem[i]; i++) {
	    switch (elem[i] -> type) {
	    case RESELEM:
		if ((cx = elem[i] -> s.res.con) >= nrOfCondStd) break;
		conductorCheck[cx] |= 2;
		meshDef[cx] -> vdim_r = elem[i];
		break;
	    case VDIMELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.vdim.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		m -> ybr = elem[i] -> s.vdim.height;
		m -> ytr = elem[i] -> s.vdim.height+elem[i] -> s.vdim.thickness;
		/* this must be the only one */
		if (m -> solid_r) {
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> vdim_r -> name, elem[i] -> name);
		}
		else
		    m -> vdim_r = elem[i];
		m -> solid_r = TRUE;
		break;
	    case SURFCAP3DELEM:
		top_cond = elem[i] -> s.cap.pCon;
		bot_cond = elem[i] -> s.cap.nCon;
		if (top_cond < 0 || top_cond >= nrOfCondStd) break;
		if (bot_cond < 0 || bot_cond >= nrOfCondStd) break;
		meshDef[top_cond] -> coreb_r = TRUE;
		meshDef[bot_cond] -> corea_r = TRUE;
		break;
	    case CONTELEM:
		top_cond = elem[i] -> s.cont.con1;
		bot_cond = elem[i] -> s.cont.con2;

		/* Substrate contacts are not supported yet: uses
		 * a invalid entry in the meshDef array (negative
		 * conductor number (FB, NvdM)) !!! See also left
		 * tile recognition above.
		 */
		if (top_cond < 0 || top_cond >= nrOfCondStd) break;
		if (bot_cond < 0 || bot_cond >= nrOfCondStd) {
		    if (!contacts_sub) break;
		    bot_cond = -1;
		}

		/* vdim must be known */
		if (!meshDef[top_cond] -> solid_r) break;
		if (bot_cond >= 0) {
		    if (!meshDef[bot_cond] -> solid_r) break;
		    if (meshDef[top_cond] -> ybr < meshDef[bot_cond] -> ytr) {
			int t = top_cond; top_cond = bot_cond; bot_cond = t; /* swap */
		    }
		    if (meshDef[top_cond] -> ybr <= meshDef[bot_cond] -> ytr) {
			say ("3D geometries of elements '%s' and '%s' coincide at %s",
			    meshDef[top_cond] -> vdim_r -> name,
			    meshDef[bot_cond] -> vdim_r -> name,
			    strCoorBrackets (bdr -> x1, bdr -> y1));
			break;
		    }
		    meshDef[bot_cond] -> conta_r = (spiderEdge_t*)TRUE;
		    meshDef[bot_cond] -> conta_oconductor = top_cond;
		}
		meshDef[top_cond] -> contb_r = (spiderEdge_t*)TRUE;
		meshDef[top_cond] -> contb_oconductor = bot_cond;
		break;
	    }
	}
    }

    /* Now, provide defaults for the lateral displacements
     * of the mesh edges (CSHAPE/ESHAPE).
     */
    /* Check whether a conductor is present for each mesh element.
     * This is not just debugging, but a check for the correctness
     * of the technology file.
     */
    *n = nrOfCondStd;
    top_cond = -1;
    for (i = 0; i < nrOfCondStd; ++i) {
	m = meshDef[i];
	if (!m -> solid_l && !m -> solid_r) {
	    if ((conductorCheck[i] & 3) && conductorCheck[i] < 4) {
		elemDef_t *el = (conductorCheck[i] & 1) ? m -> vdim_l : m -> vdim_r;
		say ("warning: missing vdimension for conductor '%s'\n", el -> name);
		conductorCheck[i] |= 4;
	    }
	    continue;
	}
	if (top_cond < 0) *n = i; // first conductor
	top_cond = i;

	if (m -> solid_l && m -> solid_r) {
	    if (!(conductorCheck[i] & 1)) goto mc_l;
	    if (!(conductorCheck[i] & 2)) goto mc_r;

	    /* FVF: if bottom/top of left tile is not equal bottom/top of right tile */
	    if (m -> ybl > m -> ybr) {
		m -> csbr = (m -> ybl - m -> ybr) / step_slope;
		m -> csbr /= meters;
	    }
	    else m -> csbr = 0;

	    if (m -> ybl < m -> ybr) {
		m -> csbl = (m -> ybr - m -> ybl) / step_slope;
		m -> csbl /= meters;
	    }
	    else m -> csbl = 0;

	    if (m -> ytl > m -> ytr) {
		m -> cstr = (m -> ytl - m -> ytr) / step_slope;
		m -> cstr /= meters;
	    }
	    else m -> cstr = 0;

	    if (m -> ytl < m -> ytr) {
		m -> cstl = (m -> ytr - m -> ytl) / step_slope;
		m -> cstl /= meters;
	    }
	    else m -> cstl = 0;

	    m -> ybl /= meters;
	    m -> ytl /= meters;
	    m -> ybr /= meters;
	    m -> ytr /= meters;
	}
	else if (m -> solid_l) {
	    if (!(conductorCheck[i] & 1)) {
mc_l:		missingCon (m -> vdim_l -> s.vdim.mask, SURFACE, tile_l, (tile_t *) NULL,
		    (tile_t *) NULL, m -> vdim_l, bdr -> x1, bdr -> y1);
		die ();
	    }
	    m -> ybl /= meters;
	    m -> ytl /= meters;
	}
	else {
	    if (!(conductorCheck[i] & 2)) {
mc_r:		missingCon (m -> vdim_r -> s.vdim.mask, SURFACE, tile_r, (tile_t *) NULL,
		    (tile_t *) NULL, m -> vdim_r, bdr -> x1, bdr -> y1);
		die ();
	    }
	    m -> ybr /= meters;
	    m -> ytr /= meters;
	}
	m -> esbl = 0;
	m -> estl = 0;
	m -> esbr = 0;
	m -> estr = 0;
    }

    if (top_cond >= 0) {
	meshDef[top_cond + 1] = NULL;

    if (hasShapeElems) {

    /* Now, we look for defined edge elements.
     * If we find one, we simply overwrite the defaults
     * that are set above.
     */
	elem = recogEdge (tile_l, tile_r, NULL);

	for (i = 0; elem[i]; i++) {
	    switch (elem[i] -> type) {
	    case CSHAPEELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.shape.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		if (!m -> solid_l || !m -> solid_r) { /* not a cross-over */
#ifdef WRONG_MESH_MSG
		    wrongMesh (1, tile_l, tile_r, elem[i]);
#endif
		    break;
		}
		if (m -> shape) { /* not the only one */
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> shape -> name, elem[i] -> name);
		    break;
		}
		m -> csbl = elem[i] -> s.shape.xb1 / meters;
		m -> cstl = elem[i] -> s.shape.xt1 / meters;
		m -> csbr = elem[i] -> s.shape.xb2 / meters;
		m -> cstr = elem[i] -> s.shape.xt2 / meters;
		m -> shape = elem[i];
		break;

	    case ESHAPEELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.shape.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		if (m -> shape) { /* not the only one */
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> shape -> name, elem[i] -> name);
		    break;
		}
		if (m -> solid_l) {
		    m -> esbl = elem[i] -> s.shape.xb1 / meters;
		    m -> estl = elem[i] -> s.shape.xt1 / meters;
		}
		else {
		    m -> esbr = elem[i] -> s.shape.xb1 / meters;
		    m -> estr = elem[i] -> s.shape.xt1 / meters;
		}
		m -> shape = elem[i];
		break;
	    }
	}

    /* Now, the symmetric case.
     * tile_l and tile_r can simply be swapped, since the [CE]SHAPE
     * elements will (should) only be there when
     * tile_l -> color != tile_r -> color.
     * (See the assertions above and below.)
     */
	elem = recogEdge (tile_r, tile_l, NULL);

	for (i = 0; elem[i]; i++) {
	    switch (elem[i] -> type) {
	    case CSHAPEELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.shape.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		if (!m -> solid_l || !m -> solid_r) { /* not a cross-over */
#ifdef WRONG_MESH_MSG
		    wrongMesh (1, tile_r, tile_l, elem[i]);
#endif
		    break;
		}
		if (m -> shape) { /* not the only one */
		    if (m -> shape == elem[i]) break;
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> shape -> name, elem[i] -> name);
		    break;
		}
		m -> csbr = elem[i] -> s.shape.xb1 / meters;
		m -> cstr = elem[i] -> s.shape.xt1 / meters;
		m -> csbl = elem[i] -> s.shape.xb2 / meters;
		m -> cstl = elem[i] -> s.shape.xt2 / meters;
		m -> shape = elem[i];
		break;

	    case ESHAPEELEM:
		++elem[i] -> el_recog_cnt;
		if ((cx = elem[i] -> s.shape.con) >= nrOfCondStd) break;
		m = meshDef[cx];
		if (m -> shape) { /* not the only one */
		    if (m -> shape == elem[i]) break;
		    say ("two elements at position %s:\n   '%s' and '%s', last element ignored",
			strCoorBrackets (bdr -> x1, bdr -> y1), m -> shape -> name, elem[i] -> name);
		    break;
		}
		if (m -> solid_r) {
		    m -> esbr = elem[i] -> s.shape.xb1 / meters;
		    m -> estr = elem[i] -> s.shape.xt1 / meters;
		}
		else {
		    m -> esbl = elem[i] -> s.shape.xb1 / meters;
		    m -> estl = elem[i] -> s.shape.xt1 / meters;
		}
		m -> shape = elem[i];
		break;
	    }
	}
    }
    }
    return (meshDef);
}

#ifdef WRONG_MESH_MSG
Private void wrongMesh (int type, tile_t *tile, tile_t *eTile, elemDef_t *el)
{
    char buf[256], *s;
    int i;

    if (type == 1)
	sprintf (buf, "warning: not on both sides a mesh present for cshape");
    else if (type == 2)
	sprintf (buf, "warning: no mesh edge present for eshape");

    s = buf + strlen (buf);
    sprintf (s, " element '%s'\n  at position %s, element skipped\n",
             el -> name, strCoorBrackets (bdr -> x1, bdr -> y1));

    s += strlen (s);
    sprintf (s, "  masks present :");

    if (tile) {
	for (i = 0; i < nrOfMasks; i++) {
	    if (masktable[i].gln && !COLOR_ABSENT (&masktable[i].color, &tile -> color)) {
		s += strlen (s);
		sprintf (s, " %s", masktable[i].name);
	    }
	}
    }

    if (eTile) {
	for (i = 0; i < nrOfMasks; i++) {
	    if (masktable[i].gln && !COLOR_ABSENT (&masktable[i].color, &eTile -> color)) {
		s += strlen (s);
		sprintf (s, " -%s", masktable[i].name);
	    }
	}
    }

    say (buf);
}
#endif
