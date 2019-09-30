/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
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
 * default includes for included in all sources
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* standard booolean defines */
#define ON           1
#define OFF          0
#define YES          1
#define NO           0
#define TRUE         1
#define FALSE        0
#define PROCESSED    1
#define WRONG        0
#define OK           1

/*
 * for indices
 */
#define X            0
#define Y            1
#define Z            2

#define HORIZONTAL   0
#define VERTICAL     1

/*
 * define for rnet->type
 */
#define SIGNAL 0
#define CLOCK  1
#define POWER  2
#define VSS    3
#define VDD    4


/*
 * for error routine
 */
#define FATAL_ERROR  0
#define ERROR        1
#define WARNING      2

/*
 * standard direction defines (see also coredef.h)
 */
#define  L        0     /* connects to left */
#define  R        1     /* connects to right */
#define  B        2     /* connects to bottom */
#define  T        3     /* connects to top */
#define  D        4     /* connects downward (a via) */
#define  U        5     /* connects upward (a via) */

#define  HERE     U+1

/*
 * for grid memory storage
 */
#define  COREUNIT        unsigned char     /* unit in which the image is stored */
#define  GRIDADRESSUNIT  long     /* unit of index */

/*
 * Meaning of the individual bits in the grid.
 * The first 6 bits code the connection pattern of the wire-element.
 * The relative ordering of these bits should not be changed.
 * See above the defines of L, R, B, T, D and U.
 * Bits 7 and 8 code the state:
 *  Bit 8 (FRONT2) Bit 7 (FRONT1)
 *      0          0        grid point is free (unoccupied)
 *      0          1        grid point belongs to first expansion front (source)
 *      1          0        grid point belongs to second expandsion front (destination)
 *      1          1        grid point is occupied
 */
#define  PATTERNMASK  0x3F    /* 00111111  masks of the pattern */
#define  STATEMASK    0xC0    /* 11000000  masks of the state */
#define  FRONT1       0x40    /* 01000000  mask of front1 */
#define  FRONT2       0x80    /* 10000000  mask of front2 */

/*
 * macro: step though each of the 6 possible offsets
 */
#define for_all_offsets(offset) for(offset = L; offset != HERE; ++offset)

/* defines used to make routines direction invaviant, and for rotation */
#define x_sign(a,b)  ((a) == L || (a) == B ? -(b) : (b))
#define y_sign(a,b)  ((a) == L || (a) == T ? -(b) : (b))

/* anticlockwise rotation */
#define nxt(a)   ((a) == L ? (B) : (a) == R ? (T) : (a) == B ? (R) : (L))
/* clockwise rotation */
#define prev(a)  ((a) == L ? (T) : (a) == R ? (B) : (a) == B ? (L) : (R))
/* opposite side */
#define opposite(a) (a==R ? L : a == L ? R : a == B ? T : a == T ? B : a == U ? D: U)
/* convert wheter horizontal or vertical */
#define to_hv(a)  ((a) <= R ? (HORIZONTAL) : (VERTICAL))
/* convert 4 sides to two side index */
#define to_hor(a) ((a) == L || (a) == B ? (L) : (R))

#define BIGNUMBER       1000000000
#define BIGFLOAT        10e20

#define ABS(a)   ((a) < 0 ? -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN_UPDATE(a,b) if (a > b) a = b
#define MAX_UPDATE(a,b) if (a < b) a = b

#ifdef MALLOC
#undef MALLOC
#endif
#define MALLOC(ptr,type) { ptr = (type *)malloc(sizeof(type));\
if(!ptr) error(FATAL_ERROR,"malloc");\
}

#define STRINGSAVE(ptr, str) { ptr = (char *)malloc(strlen(str)+1);\
if(!ptr) error(FATAL_ERROR,"malloc_stringsave");\
strcpy(ptr, str);\
}

#ifdef CALLOC
#undef CALLOC
#endif
#define CALLOC(ptr,type,no) { ptr = (type *)calloc((unsigned)no, sizeof(type));\
if(!ptr) error(FATAL_ERROR,"calloc");\
}

#ifdef REALLOC
#undef REALLOC
#endif
#define REALLOC(ptr,type, no) { ptr = (type *)realloc(ptr, (no * sizeof(type)));\
if(!ptr) error(FATAL_ERROR,"realloc");\
}
