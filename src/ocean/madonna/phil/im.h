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

#ifndef __IM_H
#define __IM_H

#include "src/ocean/madonna/phil/image.h"
#include "src/ocean/madonna/phil/plaza.h"

#ifndef LINKAGE_TYPE
#include "src/ocean/libseadif/libstruct.h"
#endif

#ifdef __cplusplus
extern LINKAGE_TYPE
{
#endif

/* mtx.c */
short *mtxchain (short *mtxb, short *mtxa);
short *mtxidentity (void);
void mtxtranslate (short *mtx, int p, int q);
void mtxcopy (short *dstmtx, short *srcmtx);
void mtxapply (int *out, int *in, short *mtx);
void mtxaddvec (short *mtx, int p, int q);
void mtxapplytocrd (short *out, short *in, short *mtx);

/* initimag.c */
void initimagedesc (IMAGEDESCPTR imagedesc);
void drawmirroraxis (int dblsize[], IMAGEDESCPTR imagedesc);
void computetransrotmtx (MIRRORPTR mirror);
void labelallsectors (int dblsize[], IMAGEDESCPTR imagedesc);
void labelsector (int **stamp, int px, int py, int xsiz, int ysiz, int label);
void labelhalfsector (int **stamp, int dx, int px, int py, int xsiz, int ysiz, int label);
void mkequivalencelist (int dblsize[], IMAGEDESCPTR imagedesc);
void mtxdoubletonormal (MIRRORPTR maxis);
void readmirroraxis (IMAGEDESCPTR imagedescptr);

/* debug.c */
void printstamp (IMAGEDESCPTR imagedesc);
void printequiv (IMAGEDESCPTR imagedesc);
void printpivot (PIVOTPTR pivot);
void printchild (PIVOTPTR pivot);
void printmtx (short *mtx);

#ifdef __cplusplus
}
#endif

#endif
