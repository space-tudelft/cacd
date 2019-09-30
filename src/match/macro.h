/* rcsid = "$Id: macro.h,v 1.1 2018/04/30 12:17:35 simon Exp $" */
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
/*
 * This file contains some general purpose macro definitions.
 */

#define Fatal(n) p_error (n, rcsid, __LINE__);

#define Assert(E) if(!(E)) assert_error (rcsid, __LINE__, #E);

#ifdef DEBUG
#define Malloc(ptr,amount,object) \
if(!(ptr = (object *) s_malloc((size_t)(amount*sizeof(object))))) \
	{ printf ("ERROR: %s %d (%d)\n", __FILE__, __LINE__, (int)amount); exit (1); }
#define Free(ptr) s_free(ptr)
#else
#define Malloc(ptr,amount,object) \
if(!(ptr = (object *) malloc((size_t)(amount*sizeof(object))))) m_err((int)(amount*sizeof(object)));
#define Free(ptr) if(ptr) free(ptr)
#endif

