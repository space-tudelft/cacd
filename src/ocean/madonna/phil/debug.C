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
#include "src/ocean/madonna/phil/im.h"

/* this one's for debugging */
void printstamp (IMAGEDESCPTR imagedesc)
{
  int x, y, **stamp = imagedesc->stamp;

  fprintf (stderr, "\n");
  for (y = (imagedesc->size[VER] << 1) - 1; y >= 0; --y)
  {
    for (x = 0; x < (imagedesc->size[HOR] << 1); ++x)
      fprintf (stderr, "%3d", stamp[x][y]);
    fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
}

/* this one's for debugging */
void printequiv (IMAGEDESCPTR imagedesc)
{
  MIRRORPTR    mirrax;
  EQUIVLISTPTR equiv;

  for (mirrax = imagedesc->mirroraxis; mirrax; mirrax = mirrax->next)
  {
    fprintf (stderr, "(Mirrax %d", mirrax->axisid);
    for (equiv = mirrax->equivalence; equiv; equiv = equiv->next)
      fprintf (stderr, " (%d %d)", equiv->a, equiv->b);
    fprintf (stderr, ")\n");
  }
}

/* this one's for debugging */
void printpivot (PIVOTPTR pivot)
{
  fprintf (stderr, "\n(Pivot (%d %d) (%d %d)", pivot->x, pivot->y, pivot->lx, pivot->ly);
  printchild (pivot);
  fprintf (stderr, ")");
}

/* this one's for debugging */
void printchild (PIVOTPTR pivot)
{
  PLAZGLAFPTR glaf = pivot->children;
  LAYINSTPTR  li;
  LAYOUTPTR   lay;

  fprintf (stderr, "\n (Children");
  for (; glaf; glaf = glaf->next)
  {
    lay = glaf->layout;
    fprintf (stderr, "\n  (Layout (%s(%s(%s)))",
      lay->name, lay->circuit->name, lay->circuit->function->name);
    glaf = (PLAZGLAFPTR)lay->flag.p;
    if (glaf->layout != lay) fprintf (stderr, " GLAF_LAYOUT");
    for (li = glaf->layinst; li; li = (LAYINSTPTR)li->flag.p)
      fprintf (stderr, " (%s)", li->name);
    fprintf (stderr, ")");
  }
  fprintf (stderr, ")");
}

/* this one's for debugging */
void printmtx (short* mtx)
{
  fprintf (stderr, "/ %2d %2d \\   / %2d \\\n", (int)mtx[A11], (int)mtx[A12], (int)mtx[B1]);
  fprintf (stderr, "\\ %2d %2d /   \\ %2d /\n", (int)mtx[A21], (int)mtx[A22], (int)mtx[B2]);
}
