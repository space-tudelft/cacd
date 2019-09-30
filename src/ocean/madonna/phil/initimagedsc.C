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

#include "src/ocean/libseadif/sea_func.h"
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/phil/image.h"
#include "src/ocean/madonna/phil/plaza.h"
#include "src/ocean/madonna/phil/sea.h"
#include "src/ocean/madonna/phil/im.h"
#include <stdlib.h>

void initimagedesc (IMAGEDESCPTR imagedesc)
{
  int dblsize[2], *columnarray, *columnptr, **stamp;

  /*readmirroraxis (imagedesc);*/
  dblsize[HOR] = 2 * imagedesc->size[HOR];
  dblsize[VER] = 2 * imagedesc->size[VER];

  /* allocate a two dimensional int array sized (dblsize[HOR], dblsize[VER]) */
  if (!(imagedesc->stamp = stamp = (int**)malloc (dblsize[HOR] * sizeof(int*))))
     err (5, (char*)"initimagedesc: cannot allocate memory");
  if (!(columnarray = (int*)malloc (dblsize[HOR] * dblsize[VER] * sizeof(int))))
     err (5, (char*)"initimagedesc: cannot allocate memory");

  columnptr = columnarray;
  for (int j = 0; j < dblsize[HOR]; columnptr += dblsize[VER], ++j)
  {
    stamp[j] = columnptr;
    for (int i = dblsize[VER] - 1; i >= 0; --i) columnptr[i] = 0;
  }

  /* this one assigns to imagedesc->{numaxis, stamp, mirror} */
  drawmirroraxis (dblsize, imagedesc);

  /* and this one assigns to imagedesc->{numsector, sectorlist, stamp} */
  labelallsectors (dblsize, imagedesc);

  /* this sets up for each axis the lists of mirror equivalent sectors */
  /* mkequivalencelist (dblsize, imagedesc); */
  /* the matrices are now for double coords. Change this to normal coords.*/
  mtxdoubletonormal (imagedesc -> mirroraxis);
}

/* draw mirror axis in image matrix */
void drawmirroraxis (int dblsize[], IMAGEDESCPTR imagedesc)
{
  MIRRORPTR mirror;
  int axisid, numaxis, *crd, dx, dy, h2, v2, x, y, **stamp;

  stamp = imagedesc->stamp;
  axisid = 1; numaxis = 0;

  h2 = dblsize[HOR];
  v2 = dblsize[VER];

  for (mirror = imagedesc->mirroraxis; mirror; mirror = mirror->next)
  {
    mirror->axisid = axisid;   /* assign id */
    mirror->logaxisid = numaxis;
    ++numaxis;
    if (axisid < 0) { /* overflow */
      fprintf (stderr, "drawmirroraxis: too many mirror axis defined, ignore all but first %d\n", numaxis);
      break;
    }

    crd = mirror->axis;
    if (crd[X1] < 0 || crd[X1] >= h2 || crd[X2] < 0 || crd[X2] >= h2 ||
	crd[Y1] < 0 || crd[Y1] >= v2 || crd[Y2] < 0 || crd[Y2] >= v2)
	err (5, (char*)"drawmirroraxis: axis coordinates out of range");

    dx = dy = 0;
    if (crd[X1] < crd[X2]) dx= 1; else if (crd[X1] > crd[X2]) dx= -1;
    if (crd[Y1] < crd[Y2]) dy= 1; else if (crd[Y1] > crd[Y2]) dy= -1;

    /* normalize direction, that is, point to right/upper: */
    if (dx == -1 || (dx == 0 && dy == -1)) { dx *= -1; dy *= -1; }

    mirror -> delta[HOR] = dx;
    mirror -> delta[VER] = dy;
    computetransrotmtx (mirror);

    /* draw axis in one direction */
    x = crd[X1]; y = crd[Y1];
    while (x < h2 && x >= 0 && y < v2 && y >= 0) { stamp[x][y] |= axisid; x += dx; y += dy; }

    /* draw axis in other direction */
    x = crd[X1]; y = crd[Y1];
    while (x < h2 && x >= 0 && y < v2 && y >= 0) { stamp[x][y] |= axisid; x -= dx; y -= dy; }

    axisid <<= 1;
  }
  imagedesc->numaxis = numaxis;
}

/* mtx == A11 A12 B1
 *        A21 A22 B2
 * Matrix refers to ``dblsize'' coordinates !!!
 * mtx[A11..A22] vary with the direction of the mirror axis.
 * mtx[B1,B2] must equal mtx[k*xx,k*yy] where xx and yy are the points
 *    where the mirror axis crosses the x-axis and y-axis respectively.
 * Formulas:	xx = x - (y/dy) * dx
 *		yy = y - (x/dx) * dy
 *		k = 2 for axis (1,0) and (0,1)
 *		k = 1 for axis (1,1) and (1,-1)
 */
void computetransrotmtx (MIRRORPTR mirror)
{
  short *mtx = mirror->mtx;
  int   *crd = mirror->axis; /* note: crd[] refers to dblsize coordinates */

  for (int j = 0; j < 6; ++j) mtx[j] = 0;

  if (mirror->delta[HOR] == 0 && mirror->delta[VER] == 1)
  {
    mtx[A11] = -1; mtx[A22] = 1; mtx[B1] = 2 * crd[X1];
  }
  else if (mirror->delta[HOR] == 1 && mirror->delta[VER] == 0)
  {
    mtx[A11] = 1; mtx[A22] = -1; mtx[B2] = 2 * crd[Y1];
  }
  else if (mirror->delta[HOR] == 1 && mirror->delta[VER] == 1)
  {
    mtx[A12] = 1; mtx[A21] = 1;
    mtx[B1] = (crd[X1] - crd[Y1]);
    mtx[B2] = (crd[Y1] - crd[X1]);
    if ((mtx[B1]&1) || (mtx[B2]&1)) err (3, (char*)"invalid diagonal mirror axis defined");
  }
  else if (mirror->delta[HOR] == 1 && mirror->delta[VER] == -1)
  {
    mtx[A12] = -1; mtx[A21] = -1;
    mtx[B1] = (crd[X1] + crd[Y1]);
    mtx[B2] = (crd[X1] + crd[Y1]);
    if ((mtx[B1]&1) || (mtx[B2]&1)) err (3, (char*)"invalid diagonal mirror axis defined");
  }
  else err (5, (char*)"computetransrotmtx: internal error 761");
}

/* count and label the resulting sectors */
void labelallsectors (int dblsize[], IMAGEDESCPTR imagedesc)
{
  SECTORLISTPTR sectorlistp;
  int **stamp = imagedesc -> stamp;
  int sectorcontrol = 1;
  int sectorid = 1;

  for (int px = 0; px < dblsize[HOR]; ++px)
  for (int py = 0; py < dblsize[VER]; ++py)
  if (stamp[px][py] == 0)
  { /* found a new sector, still unlabeled */
    if (!sectorcontrol) err (5, (char*)"labelallsectors: cannot handle more sectors\n");
    labelsector (stamp, px, py, dblsize[HOR], dblsize[VER], 0 - sectorid);
    NewSectorlist (sectorlistp);
    sectorlistp->point[HOR] = px;
    sectorlistp->point[VER] = py;
    sectorlistp->sectorid = sectorid;
    sectorlistp->next = imagedesc->sectorlist;
    imagedesc->sectorlist = sectorlistp;
    ++sectorid;
    sectorcontrol <<= 1; /* only to check for too many sectors */
  }

  imagedesc -> numsector = sectorid - 1;
}

/* fill the empty area bounded by mirror axis with label */
void labelsector (int **stamp, int px, int py, int xsiz, int ysiz, int label)
{
  /* label sector to the left of vertical line "x == px" */
  labelhalfsector (stamp, -1, px, py, xsiz, ysiz, label);
  /* label sector to the right of vertical line "x == px" */
  labelhalfsector (stamp,  1, px, py, xsiz, ysiz, label);
}

void labelhalfsector (int **stamp, int dx, int px, int py, int xsiz, int ysiz, int label)
{
  int y, gofurther;

  for (; px >= 0 && px < xsiz; px += dx)
  {
    gofurther = -1;

    /* label the vertical line "x == px" (upper part) */
    for (y = py; y < ysiz && stamp[px][y] <= 0; ++y) {
      stamp[px][y] = label;
      if (px+dx >= 0 && px+dx < xsiz && stamp[px+dx][y] == 0) gofurther = y;
    }

    /* label the vertical line "x == px" (lower part) */
    for (y = py-1; y >= 0 && stamp[px][y] <= 0; --y) {
      stamp[px][y] = label;
      if (px+dx >= 0 && px+dx < xsiz && stamp[px+dx][y] == 0) gofurther = y;
    }

    /* could we find an entry point to the next vertical line? */
    if ((py = gofurther) == -1) break;
  }
}

void mkequivalencelist (int dblsize[], IMAGEDESCPTR imagedesc)
{
  MIRRORPTR      mirrax;
  SECTORLISTPTR  sectorp;
  EQUIVLISTPTR   equiv;
  int mpoint[2], **stamp = imagedesc -> stamp;

  for (mirrax = imagedesc->mirroraxis; mirrax; mirrax = mirrax->next)
  for (sectorp = imagedesc->sectorlist; sectorp; sectorp = sectorp->next)
  {
    NewEquivlist (equiv);
    mtxcopy (equiv->mtx, mirrax->mtx);
    mtxapply (mpoint, sectorp->point, equiv->mtx);

    /* If mpoint outside current image block, correct this: */
    if (mpoint[HOR] < 0)    /* move to the right */
	mtxaddvec (equiv->mtx, dblsize[HOR], 0);
    else if (mpoint[HOR] >= dblsize[HOR]) /* move to the left */
	mtxaddvec (equiv->mtx, -dblsize[HOR], 0);

    if (mpoint[VER] < 0)    /* move up */
	mtxaddvec (equiv->mtx, 0, dblsize[VER]);
    else if (mpoint[VER] >= dblsize[VER]) /* move down */
	mtxaddvec (equiv->mtx, 0, -dblsize[VER]);

    /* Now retry... */
    mtxapply (mpoint, sectorp->point, equiv->mtx);
    if (mpoint[HOR] < 0 || mpoint[HOR] >= dblsize[HOR] ||
	mpoint[VER] < 0 || mpoint[VER] >= dblsize[VER])
	    err (5, (char*)"mkequivalencelist: fatal internal error 55");

    /* remember: sectorid = 0 - sectorlabel */
    equiv->b = 0 - stamp[mpoint[HOR]][mpoint[VER]];
    equiv->a = sectorp->sectorid;
    equiv->next = mirrax->equivalence;
    mirrax->equivalence = equiv;

    /* change mtx form dblsize to normal size */
    if ((equiv->mtx[B1])&1) err (5, (char*)"illegal mirror axis specified, goodbye");
    equiv->mtx[B1] >>= 1;
    if ((equiv->mtx[B2])&1) err (5, (char*)"illegal mirror axis specified, goodbye");
    equiv->mtx[B2] >>= 1;
  }
}

/* convert all dblsize transformations in the maxis to normal size */
void mtxdoubletonormal (MIRRORPTR maxis)
{
  int nerrors = 0;
  for (; maxis; maxis = maxis->next)
  {
    short *mtx = maxis->mtx;
    if ((mtx[B1]&1) || (mtx[B2]&1)) {
      ++nerrors;
      fprintf (stderr, "(%d) illegal mirror axis specified, cannot continue\n", nerrors);
    }
    mtx[B1] >>= 1;
    mtx[B2] >>= 1;
  }
  if (nerrors > 0) err (3, (char*)"...please go hacking your image specification !!!");
}

/* this one only used for testing */
void  readmirroraxis (IMAGEDESCPTR imagedescptr)
{
  FILE *fp;
  int x1, x2, y1, y2, j;
  MIRRORPTR mp;

  if (!(fp = fopen ("mirror.axis", "r"))) err (5, (char*)"cannot open mirror.axis");

  j = fscanf (fp, "Size %d %d\n", &(imagedescptr->size[HOR]), &(imagedescptr->size[VER]));
  if (j != 2) err (5, (char*)"readmirroraxis: cannot read Size");
  j = 0;
  while (fscanf (fp, "Axis %d %d %d %d\n", &x1, &x2, &y1, &y2) == 4)
  {
    NewMirror (mp);
    mp->axis[X1] = x1;
    mp->axis[X2] = x2;
    mp->axis[Y1] = y1;
    mp->axis[Y2] = y2;
    mp->next = imagedescptr->mirroraxis;
    imagedescptr->mirroraxis = mp;
    ++j;
  }
  fprintf (stderr, "\nread %d axis form file \"mirror.axis\"\n", j);
}
