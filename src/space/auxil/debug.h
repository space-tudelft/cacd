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

#ifdef  DEBUG

#ifdef __cplusplus
extern "C" {
#endif
  extern bool_t _IfDebug;
  bool_t cIfDebug  (char *file, int line);
  void   cSetDebug (char *file);
#ifdef __cplusplus
}
#endif

#undef  DEBUG
#define DEBUG   (_IfDebug && cIfDebug (__FILE__, __LINE__))

#define PRINT(S)	if (DEBUG) fprintf(stderr,"%s\n",S)
#define TRACE     	if (DEBUG) fprintf(stderr,"\n")
#define Debug(s)	if (DEBUG) {s;}
#define Debug2(s)	{s;}

#else

#define PRINT(S)	/* empty */
#define TRACE		/* empty */
#define Debug(s)	/* empty */
#define Debug2(s)	/* empty */

#endif /* DEBUG */
