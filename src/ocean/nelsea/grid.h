/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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
 * Include def.h before this file.
 */

#ifndef __GRID_H
#define __GRID_H

/*********************************************************
 *                G R I D  D E F I N E S                 *
 *********************************************************/
/*
 * for grid memory storage
 */
#define COREUNIT        char     /* unit in which the image is stored */
#define GRIDPERUNIT     1        /* number of gridpoints stored per coreunit */
#define GRIDADRESSUNIT  long     /* unit of index */

/*
 * definition of a gridpoint:
 */
#define BITSPERGRID     8

/*
 * Meaning of the individual bits in the grid.
 * The first 6 bits code the connection pattern of the
 * wire-element. the relative ordering of these bits
 * should not be changed.
 * The next two bits code the state:
 *    STATE0     STATE1
 *      0          0        grid point is free (unoccupied)
 *      0          1        grid point belongs to first expansion front (source)
 *      1          0        grid point belongs to second expandsion front (destination)
 *      1          1        grid point is occupied
 *
 * defines used from def.h:
 *
 *       L        0      connects to left
 *       R        1      connects to right
 *       B        2      connects to bottom
 *       T        3      connects to top
 *       D        4      connects downward (a via)
 *       U        5      connects upward (a via)
 */
#define STATE0   6     /* state of the grid */
#define STATE1   7     /* state of the grid */

#define PATTERNMASK  0x003F    /* 00111111  masks of the pattern */
#define STATEMASK    0x00C0    /* 11000000  masks of the state */
#define FRONT1       0x0040    /* 01000000  mask of front1 */
#define FRONT2       0x0080    /* 10000000  mask of front2 */

#define HERE     U+1

/*********************************************************
 *                G R I D  T Y P E D E F S               *
 *********************************************************/

/*
 * to store bounding boxes conveniently
 */
typedef struct _BOX {
    GRIDADRESSUNIT xl, xr, yb, yt, zd, zu;
} BOX, *BOXPTR;

/*
 * This structure is useed as wire pointer and to
 * temporarily store wires
 */
typedef struct _GRIDWIRE {

GRIDADRESSUNIT x, y, z; /* grid coordinates of point */

COREUNIT
   pattern;         /* wiring patterns in point */

long
   cost,            /* cost of offset, or cumulative cost of wire */
   direction;       /* if offset: LBRTUD indicates oridinary offset,
                       negative value indicates tunnel through image */

struct _GRIDWIRE
   *next,
   *next_block,
   *prev_block;

} GRIDWIRE, *GRIDWIREPTR;

/*********************************************************
 *                G R I D   G L O B A L S                *
 *********************************************************/

extern BOX Image_bbx;
extern COREUNIT ***Grid;
extern COREUNIT Pat_mask[HERE+1];
extern GRIDADRESSUNIT Xoff[HERE+1], Yoff[HERE+1], Zoff[HERE+1];

/*********************************************************
 *                M A C R O   T O O L K I T              *
 *********************************************************/

/*
 * converts a general grid coorinate to the
 * corresponding coordinate in the core image
 */
#define to_core(crd,ori) crd % GridRepitition[ori]

#define map_to_layout_coord(crd,ori) \
(((crd) / GridRepitition[ori]) * LayoutRepitition[ori]) + GridMapping[ori][(crd) % GridRepitition[ori]]

/*
 * step though each of the 6 possible offsets
 */
#define for_all_offsets(offset) for(offset = L; offset != HERE; offset++)

/*********************************************************
 *           sample/retrieve operations on the grid
 *********************************************************/

#define is_destination(p)\
(((Grid[p->z][p->y][p->x] ^ p->pattern) & STATEMASK) == STATEMASK)

/*
 * retrieve state bits (= is_not_free)
 * used in lee()
 */
#define get_grid_state(p) (Grid[p->z][p->y][p->x] & STATEMASK)

/*
 * retrieve state bits (= is_not_free)
 * used in lee()
 */
#define get_grid_pattern(p) Grid[p->z][p->y][p->x]

/*
 * TRUE if the point is free
 * used in macro lee/is_marked()
 * print_image_layer()
 */
#define is_free(p) (Grid[p->z][p->y][p->x] & STATEMASK) == 0

/*
 * idem, with offset
 * used in lee()
 */
#define is_free_o(p,o) (Grid[p->z+Zoff[o]][p->y+Yoff[o]][p->x+Xoff[o]] & STATEMASK) == 0

/*
 * idem, with variable offset
 * used in lee()
 */
#define is_free_offset(p,o) (Grid[p->z+o->z][p->y+o->y][p->x+o->x] & STATEMASK) == 0

/*
 * FALSE if the point is free
 * used in macro belongs_to_opposite_front
 */
#define is_not_free(p) Grid[p->z][p->y][p->x] & STATEMASK

/*
 * idem, with variable offset, stored in o
 * used in lee/(macro)is_expandable/is_free_in_deep_image()
 */
#define is_not_free_offset(p,o) Grid[p->z + o->z][p->y + o->y][p->x + o->x] & STATEMASK

/*
 * TRUE if the gridpoint is not occupied
 */
#define is_not_occupied(x,y,z) (Grid[z][y][x] & STATEMASK) != STATEMASK

/*
 * TRUE if the gridpoint is not occupied
 */
#define is_occupied(x,y,z) (Grid[z][y][x] & STATEMASK) == STATEMASK

#define grid_is_occupied(p) is_occupied(p->x,p->y,p->z)

/* Returns TRUE if the wiring pattern in 'p' connects to a grid.
 *   WIREPTR p, offset o
 * including the offset the point must lie in the grid.
 * used in lee/save_source(), print_image_layer()
 */
#define has_neighbour(p,o) Grid[p->z][p->y][p->x] & Pat_mask[o]

#define has_neighbour_s(p,o) Grid[p.z][p.y][p.x] & Pat_mask[o]

/* Returns TRUE if the wiring pattern in 'p' connects to a grid.
 * first the offset is added to the point
 * used in is_free_in_deep_image()
 */
#define has_neighbour_offset(p,o,off) \
Grid[p->z+off->z][p->y+off->y][p->x+off->x] & Pat_mask[o]

/* Returns TRUE if the wiring pattern in 'p' connects to a grid.
 * first the offset is added to the point
 * used in is_free_in_deep_image()
 */
#define has_neighbour_o(p,o,ofs) \
Grid[p->z+Zoff[ofs]][p->y+Yoff[ofs]][p->x+Xoff[ofs]] & Pat_mask[o]

/* Sets o to the exclusive pointer to origin.
 * Will set o to HERE if zero or more than 1 paths are found.
 * used in lee/trace_wire(), lee/clear_expansion_ink()
 */
#define exclusive_source(p,o) \
for_all_offsets(o)\
   {\
   if(((Grid[p->z][p->y][p->x] & PATTERNMASK) ^ Pat_mask[o]) == 0x00) break;\
   }\
if(offset == HERE && p->z == 0)\
   {\
   for_all_offsets(o)\
      {\
      if(o != D && ((Grid[p->z][p->y][p->x] & PATTERNMASK) ^ Pat_mask[o]) == Pat_mask[D]) break;\
      }\
   }\

/*
 * same function, but for tunnels
 * sets o to the exclusive pointer to origin, assuming that
 * a via downwards exists.
 * Will set o to HERE if zero or more than 1 paths are found.
 * used in lee/trace_wire(), lee/clear_expansion_ink()
 */
#define exclusive_source_tunnel(p,off,o) \
for_all_offsets(o)\
   {\
   if(offset != D && ((Grid[p->z][p->y+off->y][p->x+off->x] & PATTERNMASK) ^ Pat_mask[o]) == Pat_mask[D])\
      break;\
   }\

/*
 * returns TRUE if the point + the offset points into
 * the negative layers (to poly/diff)
 * used in save_source()
 */
#define into_deep_image(p,o) p->z + Zoff[o] < 0

/*
 * used in lee/macro/is_free_in_deep_image()
 */
#define is_other_than_free_or_source(pnt,off)\
((Grid[pnt->z + off->z][pnt->y + off->y][pnt->x + off->x] & STATEMASK == get_grid_state(pnt)) ? (TRUE) :\
  is_not_free_offset(pnt,off))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *               set operations on the grid                    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * sets the state bits of the point p to free
 * used in macro lee/set_mark()
 */
#define set_grid_free(p) Grid[p->z][p->y][p->x] &= PATTERNMASK

/*
 * brute-force clear of the entire point
 * used in lee/clear_expansion_ink()
 */
#define set_grid_clear(p) Grid[p->z][p->y][p->x] = 0

#define set_grid_clear_c(x,y,z) Grid[z][y][x] = 0

/*
 * sets the state-bits of the point p to occupied : 11xxxxxx
 * used in make_wire()
 */
#define set_grid_occupied(p) Grid[p->z][p->y][p->x] |= STATEMASK

/*
 * set the grid to a specific neighbour pattern,
 * clearing the state bits.
 * used in lee()
 */
#define set_grid_neighbour(p,n) Grid[p->z][p->y][p->x] = Pat_mask[n]

/*
 * add the grid to a specific neighbour pattern,
 * used in make_wire()
 */
#define add_grid_neighbour(p,n) Grid[p->z][p->y][p->x] |= Pat_mask[n]

/*
 * set the grid to a specific neighbour pattern,
 * clearing the state bits.
 * used in lee()
 */
#define set_grid_neighbour(p,n) Grid[p->z][p->y][p->x] = Pat_mask[n]

/*
 * set the grid statebits to pattern, assuming that the
 * grid point is free: 00xxxxxx
 * used in lee(), lee/save_source()
 */
#define set_grid_pattern(p,pattern) Grid[p->z][p->y][p->x] = pattern

/*
 * force the state bits of the grid to pattern
 */
#define set_grid_state(p, pattern)\
Grid[p->z][p->y][p->x] &= PATTERNMASK;\
Grid[p->z][p->y][p->x] |= pattern

/*
 * copy from wire structure to grid
 * Used in  free_and_restore_wire_list()
 * the pattern is or-ed!
 */
#define restore_grid_pattern(p) Grid[p->z][p->y][p->x] |= p->pattern
#define restore_entire_pattern(p) if(p->cost != -1) restore_grid_pattern(p)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *           set operations on the wire structure              *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * set the coordinates of a wire
 * used in macro copy_point_o()
 */
#define set_wire_coordinates(p,xc,yc,zc) p->x = xc; p->y = yc; p->z = zc

/*
 * sets the cost of a wire element
 * used in lee(), lee4/save_source()
 */
#define set_wire_cost(p,val) p->cost = val

/*
 * step to next of type wire
 * used in lee/save_source(), lee/trace_wire(), lee/clear_expansion_ink()
 */
#define step_to_next_wire(p) p = p->next

/*
 * set the pattern of a wire
 * used in macro add_wire_neighbour()
 */
#define set_wire_pattern(p,bitpat) p->pattern = bitpat

/*
 * perform simple or-ing of wire patterns
 * used in: lee/trace_wire()
 */
#define or_wire_patterns(pset,porg) pset->pattern |= porg->pattern

/*
 * sets the state-bits of the point in wire p to occupied
 * used in lee/trace_wire()
 */
#define set_wire_occupied(p) p->pattern |= STATEMASK

/*
 * erase entire qwire pattern
 * used in lee/trace_wire()
 */
#define set_clear_wire_pattern(p) p->pattern &= STATEMASK

/*
 * allocate a new wire element and copy point
 * WIREPTR new_p, p;
 * used in lee/save_source(), lee/mark_as_front(), lee/trace_wire()
 */
#define copy_to_new_wire(new_p,p) \
{ WIREPTR new_wire(); new_p = new_wire(); \
  set_wire_coordinates(new_p, p->x, p->y, p->z); \
  set_wire_pattern(new_p, Grid[p->z][p->y][p->x]); }

/*
 * step the coordinates of point 'p' into the direction of offset 'o'
 * used in lee/trace_wire(), lee/clear_expansion_ink()
 */
#define step_point(p,o) \
p->x = p->x + Xoff[o];\
p->y = p->y + Yoff[o];\
p->z = p->z + Zoff[o]

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *           get/misc operations on the wire structure         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * step along all possible expansion of p,
 * including the 'tunnels'
 */
#define for_all_offsets_of_point(p, off)\
for(off = OffsetTable[to_core(p->x, X)][to_core(p->y, Y)][p->z]; off; off = off->next)

/*
 * get the pattern of a wire
 * used in macro add_wire_neighbour()
 */
#define get_wire_pattern(p) p->pattern

/*
 * link in wire list
 * used in lee(), lee/mark_as_front()
 * lee/trace_wire()
 */
#define link_in_wire_list(new_p,list) new_p->next = list; list = new_p

/*
 * connect lists
 * used in lee/save_source()
 */
#define connect_wire_lists(endp,beginp) endp->next = beginp

/*
 * wind to end of wire list
 * used in lee/save_source()
 */
#define wind_wire_to_end(p) while (p->next) p = p->next;

/*
 * copy the contents of a point, without the pointer
 * used in lee()
 */
#define copy_point(d,s) set_wire_coordinates(d, s->x, s->y, s->z)

/*
 * copy the contents of a point, without the pointer
 * used in lee()
 */
#define copy_point_o(d,s,o) set_wire_coordinates(d, s->x+Xoff[o], s->y+Yoff[o], s->z+Zoff[o])

/*
 * copy the contents of a point, without the pointer
 * used in lee()
 */
#define copy_point_offset(d,s,o) set_wire_coordinates(d, s->x+o->x, s->y+o->y, MAX(0, s->z+o->z))

/*
 * make the pattern in structpoint 'p' point to 'o'
 * used in lee(), lee/trace_wire()
 */
#define add_wire_neighbour(p,o) set_wire_pattern(p, get_wire_pattern(p) | Pat_mask[o])

/*
 * step along all tunnel ends at the end of an image
 * used in save_source, macro is_expandable
 */
#define for_all_tunnels(wpnt, p)\
for(wpnt = CoreFeed[to_core(p->x,X)][to_core(p->y,Y)]; wpnt; wpnt = wpnt->next)

/*
 * step along all tunnel ends at the end of an image
 * used in save_source
 */
#define for_all_restricted_tunnels(wpnt, p)\
for(wpnt = RestrictedCoreFeed[to_core(p->x,X)][to_core(p->y,Y)]; wpnt; wpnt = wpnt->next)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    general test operations                                  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * This macro will return TRUE if the point is ouside the region.
 * used in lee(), lee/save_source()
 */
#define outside_bbx(pnt,o,bbx) \
pnt->z + o->z < bbx->zd || pnt->z + o->z > bbx->zu ||\
pnt->y + o->y < bbx->yb || pnt->y + o->y > bbx->yt ||\
pnt->x + o->x < bbx->xl || pnt->x + o->x > bbx->xr

/*
 * This macro will return TRUE if the point is ouside the region.
 * used in lee(), lee/save_source()
 */
#define outside_bbx_o(pnt,o,bbx) \
pnt->z + Zoff[o] < bbx->zd || pnt->z + Zoff[o] > bbx->zu ||\
pnt->y + Yoff[o] < bbx->yb || pnt->y + Yoff[o] > bbx->yt ||\
pnt->x + Xoff[o] < bbx->xl || pnt->x + Xoff[o] > bbx->xr

/*
 * This macro will return TRUE if the point is ouside the image array.
 * used in make_wire()
 */
#define outside_image(pnt,o) \
pnt->z + Zoff[o] < 0 || pnt->z + Zoff[o] > Image_bbx.zu ||\
pnt->y + Yoff[o] < 0 || pnt->y + Yoff[o] > Image_bbx.yt ||\
pnt->x + Xoff[o] < 0 || pnt->x + Xoff[o] > Image_bbx.xr

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    operations for lee()                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Return TRUE if the wire is expandable
 * in the specified direction
 * stacked vias are prevented.
 * off->direction >= 0 indicates an ordinary offset,
 * off->direction < 0 indicates an offset caused by tunneling
 */
#define is_expandable(pnt,off) \
(off->direction < 0 ? is_free_in_deep_image(pnt,off) :\
 off->z == 0 ? \
   ((Grid[pnt->z][pnt->y+off->y][pnt->x+off->x] & STATEMASK) != STATEMASK &&\
    (Grid[pnt->z][pnt->y+off->y][pnt->x+off->x] & STATEMASK) != (pnt->pattern & STATEMASK)) :\
 via_allowed(pnt, off))

/*
 * get the lowest cost element from the front
 */
#define get_lowest_from_list(best)\
{\
best = front_begin; front_begin = front_begin->next;\
if (front_begin)\
   {\
   if (best->prev_block != front_begin)\
      {\
      front_begin->prev_block = best->prev_block; front_begin->direction = best->direction;\
      front_begin->direction--; front_begin->prev_block->next_block = front_begin;\
      }\
   front_begin->next_block = NULL;\
   front_end->direction--;\
   if (front_end->direction > 4 && front_end->direction <= Prevn)\
      {\
      Logn--; Nextn = (Logn + 1)*(Logn + 1);\
      Prevn = (Logn - 1)*(Logn - 1);\
      }\
   }\
else\
   front_end = NULL;\
}\

/*
 * TRUE if the expansion crosses the tearline and is in a vertical layer
 * NOTE some offsets in the images are not caught
 */
#define crosses_tearline(p,off)\
((LayerOrient[p->z] == VERTICAL || off->y == 0) ? (FALSE) :\
 ((p->y <= TearLine && p->y + off->y > TearLine) || (p->y + off->y <= TearLine && p->y > TearLine)))

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    operations for lee/save_source()                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * test for mark == is_free
 */
#define is_marked(p) is_free(p)

/*
 * mark point == set free
 */
#define set_mark(p) set_grid_free(p)

/*
 * Copies the coordinates of the source point 's'
 * to the destination point 'd' with offset 'offset'
 * WIRE     d
 * WIREPTR  s
 * offset   o
 */
#define copy_point_quick(d,s,o) \
d.x = s->x + Xoff[o];\
d.y = s->y + Yoff[o];\
d.z = s->z + Zoff[o]

/*
 * Copies the coordinates of the source point 's'
 * to the destination point 'd' with offset found in the
 * structure which was derived from the core feeds
 */
#define copy_to_point_tunnel(d,s,t)\
d.x = s->x + t->x;\
d.y = s->y + t->y;\
d.z = 0

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    operations for lee/mark_as_front()                       *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * Return TRUE if the point already belongs to the opposite camp.
 * We mis-use the fact that the the points were marked
 * the point should not be 'free' for this statement to be TRUE
 * the frontID is therefore ignored.
 */
#define belongs_to_opposite_front(p,fr_id) is_not_free(p)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    misc
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 * used in make_wire() to find nearest step
 */
#define find_step(p1,p2) \
(p1->z > p2->z ? (D) : p1->z < p2->z ? (U) : \
 p1->y > p2->y ? (B) : p1->y < p2->y ? (T) : \
 p1->x > p2->x ? (L) : p1->x < p2->x ? (R) : (HERE))

/*
 * make the pattern in point 'p' point to 'o'
 * used in: make_wire()
 */
#define set_grid_mirror_neighbour(p,o) \
Grid[p->z + Zoff[o]][p->y + Yoff[o]][p->x + Xoff[o]] |= Pat_mask[opposite(o)]

#endif
