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

#ifndef DM_MAXNAME /* bloody nelsis kids don't protect against mutiply inclusions */
#include <stdio.h> /* ... and they expect stdio.h to be already included ... */
#include "src/libddm/dmincl.h"
#endif

#include "src/ocean/layflat/layflat.h"

void flatten_mc (char *newcellname, char *oldcellname, DM_PROJECT *projectkey);
void err (int exitstatus, char *mesg);
void dumpcore (void);
MTXELM *mtxchain (MTXELM *mtxb, MTXELM *mtxa);
MTXELM *mtxidentity (void);
void mtxtranslate (MTXELM *mtx, int p, int q);
void mtxcopy (MTXELM *dstmtx, MTXELM *srcmtx);
void mtxapply (int *out, int *in, MTXELM *mtx);
void mtxapplyfloat (double *x, double *y, MTXELM *mtx);
void mtxaddvec (MTXELM *mtx, int p, int q);
void mtxaddvec2 (MTXELM *outmtx, MTXELM *inmtx, int p, int q);
void mtxapplytocrd (MTXELM *out, MTXELM *in, MTXELM *mtx);
void store_term (void);
void cleanup_terms (void);
void output_terms (DM_CELL *dstkey);
void update_bbx (MTXELM localbbx[4]);
void output_bbx (DM_CELL *dstkey);
void put_all_this_stuff (DM_CELL *dstkey, DM_CELL *srckey, MTXELM *mtx, int level);

