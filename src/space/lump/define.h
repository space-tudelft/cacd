/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#define PREV(el,nA)  ((el)->parentA == nA ? ((el)->prevA) : ((el)->prevB))
#define NEXT(el,nA)  ((el)->parentA == nA ? ((el)->nextA) : ((el)->nextB))
#define APREV(el,nA) ((el)->parentA == nA ?(&(el)->prevA) :(&(el)->prevB))
#define ANEXT(el,nA) ((el)->parentA == nA ?(&(el)->nextA) :(&(el)->nextB))
#define OTHER(el,nA) ((el)->parentA == nA ? (el)->parentB : (el)->parentA)

#define Grp(node) ((node) -> grp)

#define CAP  1
#define RES  2
#define TOR  3

#define JUN  4
#define GJUN 5
#define BJT  6

#ifdef MOMENTS
#define IND  7
#endif

#define C_LINEAR          1
#define C_NON_LINEAR      2
#define C_AREA            3
#define C_AREA_PERIMETER  4
#define C_SEPARATE        5

