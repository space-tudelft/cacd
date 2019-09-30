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
/*
 * USRLIB - Useful stuff.
 */

#ifndef __USRLIB_H
#define __USRLIB_H

//        ERROR CODES

#define   NO_ERROR    0
#define   EDOS        1
#define   ESYS        2
#define   ENOTMEM     3
#define   EWRCFG      4
#define   EINPDAT     5
#define   EFNFND      6
#define   EUNKNOW     7
#define   EWRITE      8
#define   ETASKIMP    9
#define   EINDEX     10

void  usrErr  (char*, int);
void  warning (char*, int);

void** allocArray2(int xsize, int ysize, int itemSize);
void   freeArray2 (int xsize, void** toFree);

#endif
