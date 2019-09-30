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

#ifndef recog_h_
#define recog_h_

typedef struct {
    meshCoor_t ybl, ybr, ytl, ytr; /* spider z-coordinate */

    meshCoor_t csbl, csbr, cstl, cstr; /* contour-shape displacements */
    meshCoor_t esbl, esbr, estl, estr; /* edge-shape displacements */

    int conta_oconductor;
    int contb_oconductor;
    spiderEdge_t *conta_l, *conta_r; /* contact above? */
    spiderEdge_t *contb_l, *contb_r; /* contact below? */
    bool_t solid_l, solid_r;
    bool_t corea_l, corea_r; /* core above? */
    bool_t coreb_l, coreb_r; /* core below? */

    elemDef_t *vdim_l, *vdim_r, *shape;
} meshDef_t;

#endif
