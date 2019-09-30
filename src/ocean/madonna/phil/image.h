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

#ifndef __IMAGE_H
#define __IMAGE_H

#define X1 XL
#define X2 XR
#define Y1 YB
#define Y2 YT

typedef struct _EQUIVLIST
{
  int               a, b;
  short             mtx[6]; /* transformation to go from sector a to sector b */
  struct _EQUIVLIST *next;
}
EQUIVLIST, *EQUIVLISTPTR;

typedef struct _MIRROR
{
  int	       axisid, logaxisid; /* id of axis and its 2log, e.g. 128 and 7 */
  int 	       axis[4];	    /* (2 x {X1,X2,Y1,Y2}) where axis passes through. */
  EQUIVLISTPTR equivalence; /* list of mirror-equivalent sectors. */
  short        mtx[6];	    /* rotate/translate matrix for this mirror action */
  int 	       delta[2];    /* direction (points left/upper) of mirror axis */
  struct _MIRROR *next;
}
MIRROR, *MIRRORPTR;

typedef struct _SECTORLIST
{
  int 		point[2];  /* a random point (double size) in the sector */
  int		sectorid;  /* id of sector (sector labeled with -sectorid) */
  struct _SECTORLIST *next;
}
SECTORLIST, *SECTORLISTPTR;

typedef struct _POWERLINE
{
  int        orient;	    /* HOR or VER */
  int        row_or_column; /* if orient = HOR then row number else column */
  char      *name;	    /* "vss" or something like that (canonic) */
  int        layer;	    /* layer in the range [0..numlayers-1] */
  struct _POWERLINE *next;  /* pointer to the next powerline structure */
}
POWERLINE, *POWERLINEPTR;   /* corresponds to PowerLine in image.seadif */

typedef struct _IMAGEDESC
{
  int         size[2];	    /* #gridpoints in x- and y direction */
  int         overlap[2];   /* #gridpoints overlap in x and y direction */
  int	    **stamp;	    /* double sized image matrix, see mkmirrorlist() */
  int	      numsector;    /* number of sectors */
  SECTORLISTPTR sectorlist; /* list containing a point in each sector */
  int 	      numaxis;	    /* number of mirror axis */
  MIRRORPTR   mirroraxis;   /* list of mirroraxis */
  int         numlayers;    /* number of routing layers */
  int        *routeorient;  /* routing orientation of each layer (HOR or VER)*/
  int        *estransp;     /* estimated transparency for each layer */
  POWERLINEPTR powerlines;  /* list of power lines that are image-intrinsic */
}
IMAGEDESC, *IMAGEDESCPTR;

#define NewMirror(p)     ((p) = (MIRRORPTR)	mnew(sizeof(MIRROR)))
#define NewSectorlist(p) ((p) = (SECTORLISTPTR)	mnew(sizeof(SECTORLIST)))
#define NewEquivlist(p)  ((p) = (EQUIVLISTPTR)	mnew(sizeof(EQUIVLIST)))
#define NewImagedesc(p)  ((p) = (IMAGEDESCPTR)	mnew(sizeof(IMAGEDESC)))
#define FreeMirror(p)     mfree((char**)(p), sizeof(MIRROR))
#define FreeSectorlist(p) mfree((char**)(p), sizeof(SECTORLIST))
#define FreeEquivlist(p)  mfree((char**)(p), sizeof(EQUIVLIST))
#define FreeImagedesc(p)  mfree((char**)(p), sizeof(IMAGEDESC))

#endif
