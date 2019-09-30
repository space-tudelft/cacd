/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#define MC_NETWORK        1   /* model call to a network                */
#define MC_STD_DEVICE     2   /* to a standerd built-in device          */
#define MC_EXT_DEVICE     3   /* to a extra and external defined device */

#define MAX(i,j) (((i)>(j))?(i):(j))
#define MIN(i,j) (((i)<(j))?(i):(j))

#define SIZE_PTR_INT MAX (sizeof (char *), sizeof (int))

#define PALLOC(ptr, nel, type) { \
    if (nel == 0) ptr = NULL; \
    else if (!(ptr = (type *)calloc ((unsigned)(nel), sizeof (type)))) { \
        cannotAlloc (__FILE__, __LINE__, (int)(nel), sizeof (type)); \
    }}

#define PPALLOC(ptr, nel, type) { \
    if (nel == 0) ptr = NULL; \
    else if (!(ptr = (type **)calloc ((unsigned)(nel), sizeof (type *)))) { \
        cannotAlloc (__FILE__, __LINE__, (int)(nel), sizeof (type *)); \
    }}

#define CFREE(ptr) { free ((void *)ptr); }

#define OPENR(fptr, str)     { \
    if (!(fptr = fopen (str, "r"))) { \
        fprintf (stderr, "%s: Cannot open %s\n", argv0, str); \
	die (1); \
    }}

#define OPENW(fptr, str)     { \
    if (!(fptr = fopen (str, "w"))) { \
        fprintf (stderr, "%s: Cannot open %s\n", argv0, str); \
	die (1); \
    }}

#define CLOSE(fptr) { fclose (fptr); }

