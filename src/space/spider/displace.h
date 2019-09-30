/*
 * ISC License
 *
 * Copyright (C) 1997-2018 by
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

#ifndef spdisplace_h
#define spdisplace_h

typedef enum {
  EDGESHAPEDISPL=0,
  SLOPEDISPL=1
} displ_t;


/* slopes:

       7   0   1
        \  |  /
         \ | /
      6----O----2
         / | \
        /  |  \
       5   4   3

*/

typedef enum {
    UP = 0,
    DOWN = 4, DEG_180 = 4,
    LEFT = 6,
    RIGHT = 2, DEG_90 = 4,
    UPLEFT = 7,
    DOWNLEFT = 5,
    UPRIGHT = 1,
    DOWNRIGHT = 3
} slope_t;

#endif /* spdisplace_h */
