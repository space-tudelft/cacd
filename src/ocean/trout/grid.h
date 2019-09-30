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

/* converts a general grid coordinate to corresponding coordinate in the core image */
#define to_core(crd,ori) crd % GridRepitition[ori]

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                             *
 *           sample/retrieve operations on the grid            *
 *                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define grid_neighbour(x,y,z,o) (Grid[z][y][x] & Pat_mask[o])

/* TRUE if the wiring pattern in p connects to a grid (p must lie in the grid) */
#define has_neighbour(p,o) grid_neighbour(p->x,p->y,p->z,o)

/* TRUE if the wiring pattern in p connects to a grid (first offset is added to p) */
#define has_neighbour_o(p,o,n) grid_neighbour(p->x+Xoff[n],p->y+Yoff[n],p->z+Zoff[n],o)
#define has_neighbour_offset(p,o,q) grid_neighbour(p->x+q->x,p->y+q->y,p->z+q->z,o)

/* TRUE if state of gridpoint is state */
#define is_state(x,y,z,state) ((Grid[z][y][x] & STATEMASK) == state)

/* TRUE if the gridpoint is free */
#define is_Free(x,y,z) is_state(x,y,z,0)

/* TRUE if the grid point p is free */
#define is_free(p) is_state(p->x,p->y,p->z,0)

/* TRUE if the state of gridpoint is full occupied */
#define is_occupied(x,y,z) is_state(x,y,z,STATEMASK)

/* TRUE if the state of grid point p is full occupied */
#define grid_is_occupied(p) is_state(p->x,p->y,p->z,STATEMASK)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                             *
 *               set operations on the grid                    *
 *                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* brute-force clear of the entire point */
#define set_grid_clear(p) Grid[p->z][p->y][p->x] = 0

/* sets the state-bits of the gridpoint to occupied (11xxxxxx) */
#define set_occupied(x,y,z) Grid[z][y][x] |= STATEMASK

/* sets the state-bits of grid point p to occupied */
#define set_grid_occupied(p) set_occupied(p->x,p->y,p->z)

/* copy from wire structure to grid, the pattern is or-ed! */
#define restore_grid_pattern(p) Grid[p->z][p->y][p->x] |= p->pattern

/* set the grid statebits to pattern, assuming that the grid point is free: 00xxxxxx */
#define set_grid_pattern(p,pattern) Grid[p->z][p->y][p->x] = pattern

/* step the coordinates of point p into the direction of offset o */
#define step_point(p,o) p->x += Xoff[o]; p->y += Yoff[o]; p->z += Zoff[o]

/* add the grid to a specific neighbour pattern */
#define add_grid_neighbour_c(x,y,z,n) Grid[z][y][x] |= Pat_mask[n]
#define add_grid_neighbour(p,n) Grid[p->z][p->y][p->x] |= Pat_mask[n]

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                             *
 *               get operations on gridpoints                  *
 *                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* has_neighbour, but for wgridpoint structure */
#define wire_has_neighbour(p,o) (p->pattern & Pat_mask[o])

/* make the pattern in structpoint p point to o */
#define add_wire_neighbour(p,o) p->pattern |= Pat_mask[o]

/* set wire occupied */
#define set_wire_occupied(p) p->pattern = STATEMASK

