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
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private bool_t elemPresent (elemDef_t *el);
Private void printColor (tile_t *sTile, tile_t *eTile);
Private void printElems (elemDef_t **el);
Private elemDef_t ** recogSurfaceFullSearch (tile_t *sTile);
Private elemDef_t ** recogEdgeFullSearch (tile_t *sTile, tile_t *eTile, elemDef_t **elemListIn);
#ifdef __cplusplus
  }
#endif

extern int sBitmask2MAX; /* can be used for recogEdge() */
extern elemDef_t ** elemListSpaceE;
extern elemDef_t ** elemListSpaceS;

static elemDef_t **lc_el;
static mask_t *s_Color;
static mask_t *e_Color;

void recogInit (void)
{
}

/*============================================================================*/
/*! @brief Find matching elements based on surface masks.
*//*==========================================================================*/

elemDef_t ** recogSurface (tile_t *sTile)
{
    elemDef_t ** el = recogSurfaceFullSearch (sTile);
    if (optPrintRecog) {
	printColor (sTile, NULL);
	printElems (el);
    }
    return (el);
}

/*============================================================================*/
/*! @brief Find matching elements based on edge masks.
*//*==========================================================================*/

elemDef_t ** recogEdge (tile_t *sTile, tile_t *eTile, elemDef_t **elemListIn)
{
    elemDef_t ** el = recogEdgeFullSearch (sTile, eTile, elemListIn);
    if (optPrintRecog) {
	printColor (sTile, eTile);
	printElems (el);
    }
    return (el);
}

elemDef_t ** recogEdgeFullSearch (tile_t *sTile, tile_t *eTile, elemDef_t **elemListIn)
{
    elemDef_t ** elemList;
    register elemDef_t **el, *El;
    register int key, recog_cnt, last_id;

    s_Color = &sTile -> color;
    e_Color = &eTile -> color;

    key = COLOR_INT (s_Color) & sBitmask2;
    if (eBitmask) key += (COLOR_INT (e_Color) & eBitmask) << sKeys2;

    /* if there are no extra conditions to test, or there are no
     * potential elements for this key, return immediately.
     */

    if (keyTab2[key] < 0) /* extra conditions in this key slot */
	el = elemTab2 - keyTab2[key];
    else {
	el = elemTab2 + keyTab2[key];
	return (el);
    }

    /* If we come here,
     * we know some potential elements for this key,
     * and filter the actual elements out with elemPresent.
     *
     * Idea: we can maintain two keyTabs, in such a way
     * that they individually only use a SUBSET of the
     * keys, but together they use ALL keys.
     * Identifying wich elements are actually present
     * can then be done by taking a crossection of both
     * lists of candidate elements.
     * This crossection can easily be taken since the
     * element lists are sorted.
     */

    elemList = elemListIn ? elemListIn : elemListSpaceE;

    recog_cnt = 0;
    last_id = -1;
    /* For the elements that are recognized:
     * Do not duplicate elements with same id,
     * unless they are lateral capacitances
     * (in that case a single one is selected later on).
     * This is easy since elements with same id
     * are succeeding members in el.
     */

    for (; (El = *el); el++) {
	if (elemPresent (El)) {
	    if (El -> id != last_id || El -> type == LATCAPELEM) {
		last_id = El -> id;
		elemList[recog_cnt++] = El;
	    }
	}
    }

    elemList[recog_cnt] = NULL; /* terminate element list */

    return (elemList);
}

elemDef_t * hasLatCap (tile_t *sTile, tile_t *eTile)
{
    int key;

    s_Color = &sTile -> color;
    e_Color = &eTile -> color;

    key = COLOR_INT (s_Color) & sBitmask2;
    if (eBitmask) key += (COLOR_INT (e_Color) & eBitmask) << sKeys2;

    if ((key = keyTab2[key]) < 0) key = -key;

    lc_el = elemTab2 + key;
    while (*lc_el && (*lc_el) -> type != LATCAPELEM) ++lc_el;
    while (*lc_el) {
	if (elemPresent (*lc_el)) return (*lc_el);
	++lc_el;
    }
    return (NULL);
}

elemDef_t * nextLatCap ()
{
    for (++lc_el; *lc_el; ++lc_el) {
	if (elemPresent (*lc_el)) return (*lc_el);
    }
    return (NULL);
}

Private elemDef_t ** recogSurfaceFullSearch (tile_t *sTile)
{
    elemDef_t ** elemList;
    register elemDef_t **el, *El;
    register int key, recog_cnt, last_id;

    s_Color = &sTile -> color;

    key = COLOR_INT (s_Color) & sBitmask;
    if (oBitmask) key += (COLOR_INT (s_Color) >> sKeys) & oBitmask;

    /* See note recogEdgeFullSearch()
    */
    if (keyTab[key] < 0) /* extra conditions in this key slot */
	el = elemTab - keyTab[key];
    else {
	el = elemTab + keyTab[key];
	return (el);
    }

    elemList = elemListSpaceS;

    recog_cnt = 0;
    last_id = -1; /* See note recogEdgeFullSearch() */

    for (; (El = *el); el++) {
	if (El -> cond_cnt & 1) { /* SURFACE: present bits */
	    if (!COLOR_PRESENT (s_Color, &El -> sBitPresent)) continue;
	}
	if (El -> cond_cnt & 2) { /* SURFACE: absent bits */
	    if (!COLOR_ABSENT (s_Color, &El -> sBitAbsent)) continue;
	}
	if (El -> id != last_id) {
	    last_id = El -> id;
	    elemList[recog_cnt++] = El;
	}
    }

    elemList[recog_cnt] = NULL; /* terminate element list */

    return (elemList);
}

Private bool_t elemPresent (elemDef_t *el)
{
    if (el -> cond_cnt == 0) return (TRUE); /* element is present */
    /* EDGE: present bits */
    if (!COLOR_PRESENT (e_Color, &el -> eBitPresent)) return (FALSE);
    if (el -> cond_cnt & 1) { /* SURFACE: present bits */
	if (!COLOR_PRESENT (s_Color, &el -> sBitPresent)) return (FALSE);
    }
    if (el -> cond_cnt & 2) { /* SURFACE: absent bits */
	if (!COLOR_ABSENT (s_Color, &el -> sBitAbsent)) return (FALSE);
    }
    if (el -> cond_cnt & 8) { /* EDGE: absent bits */
	if (!COLOR_ABSENT (e_Color, &el -> eBitAbsent)) return (FALSE);
    }
    return (TRUE); /* element is present */
}

Private void printColor (tile_t *sTile, tile_t *eTile)
{
    int i;

    message ("\nsColor: %s", colorOctStr (&sTile -> color));
    if (eTile) message ("eColor: %s", colorOctStr (&eTile -> color));

    message ("Masks: ");
    if (IS_COLOR (&sTile -> color)) {
	for (i = 0; i < nrOfMasks; i++)
	    if (masktable[i].gln && !COLOR_ABSENT (&sTile -> color, &masktable[i].color))
		message ("%s ", masktable[i].name);
    }
    if (eTile) if (IS_COLOR (&eTile -> color)) {
	for (i = 0; i < nrOfMasks; i++)
	    if (masktable[i].gln && !COLOR_ABSENT (&eTile -> color, &masktable[i].color))
		message ("-%s ", masktable[i].name);
    }
    message ("\n");
}

Private void printElems (elemDef_t **el)
{
    message ("Elements: ");
    while (*el) message ("%s ", (*el++) -> name);
    message ("\n");
}
