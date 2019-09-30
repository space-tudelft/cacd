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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/phil/sea.h"

/* this one returns a pointer to a static identity matrix A and a null vector B */
short* mtxidentity ()
{
  static short identity[6] = { 0, 0, 0, 0, 0, 0 };

  if (identity[A11] == 0) {
  /* cannot initialize compile time without knowing symbolic constants (write time) */
    identity[A11] = identity[A22] = 1;
    identity[A12] = identity[A21] = 0;
    identity[B1]  = identity[B2]  = 0;
  }
  return (identity);
}

/* Translate the operation mtx[] in the space (x,y) to the equivalent
 * operation in the space (x',y'), where x'=x+p and y'=y+q. (I.e. we
 * move the origin from (0,0) to coord (-p,-q)).
 * Suppose operation is Av + b then new operation is Av' - Ar + r + b,
 * where v=(x,y) and r=(p,q).
 */
void mtxtranslate (short* mtx, int p, int q)
{
  mtx[B1] += (p - mtx[A11] * p - mtx[A12] * q);
  mtx[B2] += (q - mtx[A21] * p - mtx[A22] * q);
}

/* copy scrmtx to dstmtx */
void mtxcopy (short* dstmtx, short* srcmtx)
{
  dstmtx[A11] = srcmtx[A11];
  dstmtx[A12] = srcmtx[A12];
  dstmtx[A21] = srcmtx[A21];
  dstmtx[A22] = srcmtx[A22];
  dstmtx[B1]  = srcmtx[B1];
  dstmtx[B2]  = srcmtx[B2];
}

/* apply mtx to vector "in" giving vector "out" */
void mtxapply (int* out, int* in, short* mtx)
{
  out[HOR] = mtx[A11] * in[HOR] + mtx[A12] * in[VER] + mtx[B1];
  out[VER] = mtx[A21] * in[HOR] + mtx[A22] * in[VER] + mtx[B2];
}

void mtxaddvec (short* mtx, int p, int q)
{
  mtx[B1] += p;
  mtx[B2] += q;
}

/* apply mtx to a crd[XL,XR,YB,YT] vector */
void mtxapplytocrd (short* out, short* in, short* mtx)
{
  out[XL] = mtx[A11] * in[XL] + mtx[A12] * in[YB] + mtx[B1];
  out[YB] = mtx[A21] * in[XL] + mtx[A22] * in[YB] + mtx[B2];
  out[XR] = mtx[A11] * in[XR] + mtx[A12] * in[YT] + mtx[B1];
  out[YT] = mtx[A21] * in[XR] + mtx[A22] * in[YT] + mtx[B2];
}

/* Return the 'chained' operation mtxa followed by mtxb.
 * Suppose mtxa="y=Ax+b", mtxb="z=Py+q"
 * then return the operation "z=PAx+(Pb+q)"
 */
short* mtxchain (short* mtxb, short* mtxa)
{
  static short out[6];
  out[A11] = mtxb[A11] * mtxa[A11] + mtxb[A12] * mtxa[A21];
  out[A12] = mtxb[A11] * mtxa[A12] + mtxb[A12] * mtxa[A22];
  out[A21] = mtxb[A21] * mtxa[A11] + mtxb[A22] * mtxa[A21];
  out[A22] = mtxb[A21] * mtxa[A12] + mtxb[A22] * mtxa[A22];
  out[B1]  = mtxb[A11] * mtxa[B1]  + mtxb[A12] * mtxa[B2] + mtxb[B1];
  out[B2]  = mtxb[A21] * mtxa[B1]  + mtxb[A22] * mtxa[B2] + mtxb[B2];
  return (out);
}
