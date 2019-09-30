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

/* Memory allocation
*/
#define DISPOSE(obj, cnt) (free_p ((void *) obj, cnt, __FILE__, __LINE__), obj = NULL)

#define NEW(type, cnt) (type *) malloc_p (sizeof (type) * (cnt), __FILE__, __LINE__)

#define MALLOC(cnt) malloc_p (cnt, __FILE__, __LINE__)

#define RESIZE(p, type, cnt, oldcnt) (type *) realloc_p ((p), sizeof(type) * (cnt), sizeof(type) * (oldcnt), __FILE__, __LINE__)

#define GROW(type, buf, from, to) RESIZE (buf, type, to, from)

#define CLEAR(obj, size) memset ((void *) obj, 0, size)

