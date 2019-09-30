/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
 *	Patrick Groeneveld
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
#include "src/libddm/dmincl.h"
#include "src/ocean/layflat/layflat.h"
#include "src/ocean/layflat/prototypes.h"

static MTXELM identity[6];

/* return pointer of identity matrix */
MTXELM *mtxidentity ()
{
    if (identity[A11] == 0) identity[A11] = identity[A22] = 1;
    return identity;
}

/* Translate the operation mtx[] in the space (x,y) to the equivalent
 * operation in the space (x',y'), where x'=x+p and y'=y+q. (I.e. we
 * move the origin from (0,0) to coord (-p,-q)).
 * Suppose operation is Av + b then new operation is Av' - Ar + r + b,
 * where v=(x,y) and r=(p,q).
 */
void mtxtranslate (MTXELM *mtx, int p, int q)
{
    mtx[B1] += (p - mtx[A11] * p - mtx[A12] * q);
    mtx[B2] += (q - mtx[A21] * p - mtx[A22] * q);
}

/* copy scrmtx to dstmtx */
void mtxcopy (MTXELM *dstmtx, MTXELM *srcmtx)
{
    dstmtx[A11] = srcmtx[A11];
    dstmtx[A12] = srcmtx[A12];
    dstmtx[A21] = srcmtx[A21];
    dstmtx[A22] = srcmtx[A22];
    dstmtx[B1]  = srcmtx[B1];
    dstmtx[B2]  = srcmtx[B2];
}

/* apply mtx to vector "in" giving vector "out" */
void mtxapply (int *out, int *in, MTXELM *mtx)
{
    out[HOR] = mtx[A11] * in[HOR] + mtx[A12] * in[VER] + mtx[B1];
    out[VER] = mtx[A21] * in[HOR] + mtx[A22] * in[VER] + mtx[B2];
}

void mtxapplyfloat (double *x, double *y, MTXELM *mtx)
{
    double inx = *x, iny = *y;
   *x = mtx[A11] * inx + mtx[A12] * iny + mtx[B1];
   *y = mtx[A21] * inx + mtx[A22] * iny + mtx[B2];
}

void mtxaddvec (MTXELM *mtx, int p, int q)
{
    mtx[B1] += p;
    mtx[B2] += q;
}

void mtxaddvec2 (MTXELM *outmtx, MTXELM *inmtx, int p, int q)
{
    outmtx[A11] = inmtx[A11];
    outmtx[A12] = inmtx[A12];
    outmtx[A21] = inmtx[A21];
    outmtx[A22] = inmtx[A22];

    /* patrick: rotate the repetition vector, if necessary */
    if (inmtx[A11] == 0 && inmtx[A22] == 0) {
	int tmp = p; p = q; q = tmp; // swap
    }
    outmtx[B1] = inmtx[B1] + p;
    outmtx[B2] = inmtx[B2] + q;
}

/* apply mtx to a crd[XL,XR,YB,YT] vector */
void mtxapplytocrd (MTXELM *out, MTXELM *in, MTXELM *mtx)
{
    out[XL] = mtx[A11] * in[XL] + mtx[A12] * in[YB] + mtx[B1];
    out[YB] = mtx[A21] * in[XL] + mtx[A22] * in[YB] + mtx[B2];
    out[XR] = mtx[A11] * in[XR] + mtx[A12] * in[YT] + mtx[B1];
    out[YT] = mtx[A21] * in[XR] + mtx[A22] * in[YT] + mtx[B2];

    if (out[XL] > out[XR]) { MTXELM tmp = out[XL]; out[XL] = out[XR]; out[XR] = tmp; } // swap
    if (out[YB] > out[YT]) { MTXELM tmp = out[YB]; out[YB] = out[YT]; out[YT] = tmp; } // swap
}

/* Return the 'chained' operation mtxa followed by mtxb.
 * Suppose:  mtxa = "y=Ax+b", mtxb="z=Py+q"
 * then return the operation: "z=PAx+(Pb+q)".
 */
MTXELM *mtxchain (MTXELM *mtxb, MTXELM *mtxa)
{
    static MTXELM out[6];

    out[A11] = mtxb[A11] * mtxa[A11] + mtxb[A12] * mtxa[A21];
    out[A12] = mtxb[A11] * mtxa[A12] + mtxb[A12] * mtxa[A22];
    out[A21] = mtxb[A21] * mtxa[A11] + mtxb[A22] * mtxa[A21];
    out[A22] = mtxb[A21] * mtxa[A12] + mtxb[A22] * mtxa[A22];
    out[B1]  = mtxb[A11] * mtxa[B1]  + mtxb[A12] * mtxa[B2] + mtxb[B1];
    out[B2]  = mtxb[A21] * mtxa[B1]  + mtxb[A22] * mtxa[B2] + mtxb[B2];

    return out;
}
