/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#ifndef __PLAZA_H
#define __PLAZA_H

#include "src/ocean/libseadif/libstruct.h"

typedef struct _MIR
{
  int   area;	  /* pattern that lower-left corner of layout occupies when...  */
  int   morearea; /*             (pattern that rest of layout occupies when...) */
  short mtx[6];	  /* ...this operation is applied on it.  */
} MIR, *MIRPTR;

typedef struct _PLAZGLAF
{
  MIRPTR	mlist;	 /* array of MIR representing the mirrored area */
  int		more;	 /* number of repetitions of morearea (0 means no rep.) */
  LAYOUTPTR	layout;	 /* pointer to the layout we're talking about */
  LAYINSTPTR    layinst; /* list of all layinstances that refer to this layout */
  FLAG		flag;	 /* support for fillplaza */
  struct _PLAZGLAF *next;/* next in list */
} PLAZGLAF, *PLAZGLAFPTR;/* linked to LAYOUT.flag */

typedef struct _PIVOT
{
  int	x, y;		 /* current pivot position (0<=x<lx && 0<=y<ly) */
  int	lx, ly;		 /* size of the plaza. */
  int	wx, wy, wlx, wly;/* sub-window coordinates and size */
  short wbbx[2];	 /* actual window bounding box */
  int	numsectors;	 /* total number of sectors in entire layout */
  int** plaza;		 /* the plaza matrix, size [lx,ly] */
  PLAZGLAFPTR  children; /* list of all layouts that child layinstances refer to */
  int          maxmore;	 /* maximum of al the "more" fields in children */
  LAYOUTPTR    father;	 /* the father layout */
  IMAGEDESCPTR imagedesc;/* reference to the the image description */
} PIVOT, *PIVOTPTR;

#define EDGE 2		 /* 2 extra columns at the right side of plaza */

#define NewPlazglaf(p) ((p)=(PLAZGLAFPTR)mnew(sizeof(PLAZGLAF)))
#define FreePlazglaf(p)  mfree ((char **)(p), sizeof(PLAZGLAF))

#endif
