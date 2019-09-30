/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
 *	Simon de Graaf
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
 * Default includes for included in all sources.
 */

#ifndef __DEF_H
#define __DEF_H

/*
 * the default includes
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* standard boolean defines */
#define ON   1
#define OFF  0
#define YES  1
#define NO   0

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PROCESSED 1
#define WRONG     0
#define OK        1

/*
 * for indices
 */
#define X  0
#define Y  1
#define Z  2

#ifndef HORIZONTAL
#define HORIZONTAL 0
#endif

#ifndef VERTICAL
#define VERTICAL 1
#endif

/*
 * for error routine
 */
#define FATAL_ERROR 0
#define ERROR       1
#define WARNING     2

/*
 * for static arrays
 */
#define MAXLAYERS  10

/*
 * standard direction defines
 */
#define L  0  /* connects to left */
#define R  1  /* connects to right */
#define B  2  /* connects to bottom */
#define T  3  /* connects to top */
#define D  4  /* connects downward (a via) */
#define U  5  /* connects upward (a via) */

#define opposite(a) (a == R ? L : a == L ? R : a == B ? T : a == T ? B : a == U ? D: U)

#define BIGNUMBER 1000000000
#define BIGFLOAT  10e20

#define ABS(a)   ((a) < 0 ? -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN_UPDATE(a,b) if (a > b) a = b
#define MAX_UPDATE(a,b) if (a < b) a = b

#ifdef MALLOC
#undef MALLOC
#endif
#define MALLOC(ptr,type) \
{ \
ptr = (type *)malloc ((unsigned) sizeof(type));\
if (!ptr) error (FATAL_ERROR, "malloc");\
}

#define STRINGSAVE(ptr,str) \
{ \
ptr = (char *)malloc (strlen (str) + 1);\
if (!ptr) error (FATAL_ERROR, "malloc_stringsave");\
strcpy (ptr, str);\
}

#ifdef CALLOC
#undef CALLOC
#endif
#define CALLOC(ptr,type,no) \
{ \
ptr = (type *)calloc ((unsigned)no, (unsigned)sizeof(type));\
if (!ptr) error (FATAL_ERROR, "calloc");\
}

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC(ptr,type,no) \
{ \
ptr = (type *)realloc ((char *)ptr, (unsigned)(no * sizeof(type)));\
if (!ptr) error (FATAL_ERROR, "realloc");\
}

/* common #define's for main.C and ghotiDelete.C */
#define GHOTI_CK_CMOS		 1
#define GHOTI_CK_PASSIVE	 2
#define GHOTI_CK_ISOLATION	 4
#define GHOTI_CK_UNCONNECTED	 8
#define GHOTI_CK_SEMICONNECTED  16

#endif
