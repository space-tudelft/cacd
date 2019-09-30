/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
 *	Simon de Graaf
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

/*
 * sample/retrieve operations on the grid
 */

#define is_destination(p) (((Grid[p->z][p->y][p->x] ^ p->pattern) & STATEMASK) == STATEMASK)

/* retrieve state bits */
#define get_grid_state(p) (Grid[p->z][p->y][p->x] & STATEMASK)

/* retrieve grid pattern of point p */
#define get_grid_pattern(p) Grid[p->z][p->y][p->x]

/* TRUE if the grid point p with offset o is free */
#define is_free_o(p,o) ((Grid[p->z+Zoff[o]][p->y+Yoff[o]][p->x+Xoff[o]] & STATEMASK) == 0)

/* sets o to the exclusive pointer to origin.
 * Will set o to HERE if zero or more than 1 paths are found.
 */
#define exclusive_source(p,o) \
for_all_offsets(o) {\
   if(((Grid[p->z][p->y][p->x] & PATTERNMASK) ^ Pat_mask[o]) == 0) break;\
}\
if(o == HERE && p->z == 0) {\
   for_all_offsets(o) {\
      if(o != D && ((Grid[0][p->y][p->x] & PATTERNMASK) ^ Pat_mask[o]) == Pat_mask[D]) break;\
   }\
}

/* same function, but for tunnels
 * sets o to the exclusive pointer to origin, assuming that a via downwards exists.
 * Will set o to HERE if zero or more than 1 paths are found.
 */
#define exclusive_source_tunnel(p,off,o) \
for_all_offsets(o) {\
if(o != D && (((Grid[p->z][p->y+off->y][p->x+off->x] & PATTERNMASK) ^ Pat_mask[o]) == Pat_mask[D])) break;\
}

/* TRUE if the point + offset points into the negative layers (to poly/diff) */
#define into_deep_image(p,o) p->z + Zoff[o] < 0

/*****************************
 * set operations on the grid
 */

/* sets the state bits of the grid point p to free */
#define set_grid_free(p) Grid[p->z][p->y][p->x] &= PATTERNMASK

/* brute-force clear of the entire grid point */
#define set_grid_clear_c(x,y,z) Grid[z][y][x] = 0

/* set the state bits of the grid point p to pattern */
#define set_grid_state(p,pattern)\
Grid[p->z][p->y][p->x] &= PATTERNMASK;\
Grid[p->z][p->y][p->x] |= pattern

/***************************************
 * set operations on the wire structure
 */

/* set the pattern of a wire */
#define set_wire_pattern(p,bitpat) p -> pattern = bitpat

/* sets the state-bits of the point in wire p to occupied */
#define add_wire_occupied(p) p -> pattern |= STATEMASK

/* allocate a new wire element and copy point */
#define copy_to_new_wire(new_p,p) \
{ new_p = new_gridpoint (p->x, p->y, p->z); \
  set_wire_pattern (new_p, Grid[p->z][p->y][p->x]); }

/********************************************
 * get/misc operations on the wire structure
 */

/* step along all possible expansion of p, including the tunnels */
#define for_all_offsets_of_point(p,off)\
for(off = OffsetTable[to_core(p->x,X)][to_core(p->y,Y)][p->z]; off; off = off->next)

/* get the pattern of a wire */
#define get_wire_pattern(p) p -> pattern

/* link in wire list */
#define link_in_wire_list(new_p,list) new_p -> next = list; list = new_p

/* wind to end of wire list */
#define wind_wire_to_end(p) while(p -> next) p = p -> next;

/* step along all tunnel ends at the end of an image */
#define for_all_tunnels(wpnt,p)\
for(wpnt = CoreFeed[to_core(p->x,X)][to_core(p->y,Y)]; wpnt; wpnt = wpnt->next)

#define for_all_tunnels2(wpnt,x,y)\
for(wpnt = CoreFeed[to_core(x,X)][to_core(y,Y)]; wpnt; wpnt = wpnt->next)

/* step along all tunnel ends at the end of an image */
#define for_all_restricted_tunnels(wpnt,p)\
for(wpnt = RestrictedCoreFeed[to_core(p->x,X)][to_core(p->y,Y)]; wpnt; wpnt = wpnt->next)

/**************************
 * general test operations
 */

/* this macro will return TRUE if the point is outside the region */
#define outside_bbx(x,y,z,bbx) \
z < bbx->crd[D] || z > bbx->crd[U] ||\
y < bbx->crd[B] || y > bbx->crd[T] ||\
x < bbx->crd[L] || x > bbx->crd[R]

/* this macro will return TRUE if the point is outside the region */
#define outside_bbx_o(p,o,bbx) outside_bbx(p->x+Xoff[o],p->y+Yoff[o],p->z+Zoff[o],bbx)

/* this macro will return TRUE if the point is outside the image array */
#define outside_image(pnt,o) \
pnt->z + Zoff[o] < 0 || pnt->z + Zoff[o] > Bbx->crd[U] ||\
pnt->y + Yoff[o] < 0 || pnt->y + Yoff[o] > Bbx->crd[T] ||\
pnt->x + Xoff[o] < 0 || pnt->x + Xoff[o] > Bbx->crd[R]

/* update the boundingbox bbx with point pnt */
#define update_bbx(bbx,pnt) \
if (pnt->x < bbx->crd[L]) bbx->crd[L] = pnt->x;\
if (pnt->y < bbx->crd[B]) bbx->crd[B] = pnt->y;\
if (pnt->z < bbx->crd[D]) bbx->crd[D] = pnt->z;\
if (pnt->x > bbx->crd[R]) bbx->crd[R] = pnt->x;\
if (pnt->y > bbx->crd[T]) bbx->crd[T] = pnt->y;\
if (pnt->z > bbx->crd[U]) bbx->crd[U] = pnt->z

