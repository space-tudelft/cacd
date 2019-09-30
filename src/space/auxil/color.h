/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#ifndef COLOR_H_INCLUDED
#define COLOR_H_INCLUDED

#ifndef ZCOLOR
#define BASE_TYPE int
typedef BASE_TYPE mask_base_t;
typedef unsigned BASE_TYPE umask_base_t;

#define NSIZE 32 /* Size in bits of mask_base_t */
#define NCOL  64 /* Size of color bitmap array for masks */
#endif

typedef struct { mask_base_t color[NCOL]; } mask_t;

extern int Ncol;

#define COLOR_INT(x)  (x)->color[0]
#define COLORINIT(x)   x = cNull
#define COLORCOPY(x,y) x = y

#define COLORINITALLBITS(x) { int _n = 0; do x.color[_n] = ~0; while (++_n < Ncol); }
#define COLORINITBITS(x,y) {\
int _n = 0, _i = y;\
do {\
    if (_i < NSIZE) { if (_i <= 0) x.color[_n] = 0; else x.color[_n] = ~((mask_base_t)~0 << _i); }\
    else x.color[_n] = ~0;\
    _i -= NSIZE;\
} while (++_n < Ncol); }

#define COLORINITINDEX(x,y) {\
int _n = 0, _i = y;\
do {\
    if (_i < NSIZE && _i >= 0) x.color[_n] = (mask_base_t)1 << _i; else x.color[_n] = 0;\
    _i -= NSIZE;\
} while (++_n < Ncol); }

#define COLOR_ADD(x,y) { int _n = 0; do x.color[_n] |= y.color[_n]; while (++_n < Ncol); }
#define COLOR_XOR(x,y) { int _n = 0; do x.color[_n] ^= y.color[_n]; while (++_n < Ncol); }
#define COLOR_AND(x,y) { int _n = 0; do x.color[_n] &= y.color[_n]; while (++_n < Ncol); }

#define COLOR_ADDINDEX(x,y) {\
int _n, _i = y;\
for (_n = 0; _i >= NSIZE; ++_n) _i -= NSIZE;\
if (_n < Ncol) x.color[_n] |= (mask_base_t)1 << _i; }

#define COLOR_SLEFT(x) {\
int _n = Ncol;\
while (--_n > 0) {\
    x.color[_n] <<= 1;\
    if (x.color[_n-1] & ((mask_base_t)1 << (NSIZE-1))) x.color[_n] |= 1;\
}\
x.color[0] <<= 1; }
#define COLOR_SLEFT1(x) {\
int _n = Ncol;\
while (--_n > 0) {\
    x.color[_n] <<= 1;\
    if (x.color[_n-1] & ((mask_base_t)1 << (NSIZE-1))) x.color[_n] |= 1;\
}\
x.color[0] <<= 1; x.color[0] |= 1; }
#define COLOR_HASH(x) (x.color[0] + x.color[1])
#define COLOR_EQ_COLOR(x,y) isCOLOR_EQ_COLOR (x,y)
#define COLOR_PRESENT(x,y)  isCOLOR_PRESENT (x,y)
#define COLOR_ABSENT(x,y)   isCOLOR_ABSENT (x,y)
#define IS_COLOR(x) isCOLOR (x)

#endif /* COLOR_H_INCLUDED */
