/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#ifndef __DMDEBUG_H
#define __DMDEBUG_H

/* #define DM_DEBUG */		/* debugging is on */

#ifdef DM_DEBUG
#define IFDEBUG if (_dmIfdebug(__FILE__,__LINE__))
#define TRACE fprintf(stderr,"--trace: %s, %d\n",__FILE__,__LINE__)
#define ASSERT(v) {if (!(v)) \
{fprintf(stderr,"assertion failed, file %s, line %d\n",\
__FILE__, __LINE__); _dmFatal("assert");}}

#else /* NOT DM_DEBUG */
#define IFDEBUG if (0)
#define TRACE ;
#define ASSERT(v) ;
#endif /* DM_DEBUG */

#endif /* __DMDEBUG_H */

