/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

#ifdef MAXVOLT
#undef MAXVOLT
#endif
#ifdef MAXTIME
#undef MAXTIME
#endif

#define MAXVOLT 2147483647
#define MAXTIME 9223372036854775807LL

#define TRUE   1
#define FALSE  0

#define LOWER ('a' - 'A')

#define MAXNAME DM_MAXNAME+20

#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Abs(a)   ((a)>=0?(a):(-(a)))
#define strsame(s1,s2)  (!strcmp(s1,s2))

#define NEW(ptr, nel, type) \
    if (!(ptr = (type *)calloc ((unsigned)(nel), sizeof (type)))) die_alloc ();

#define NEWSVAL(ptr) \
if (free_svals) { ptr = free_svals; free_svals = free_svals -> next; } \
else if (!(ptr = (struct sig_value *)malloc (sizeof (struct sig_value)))) die_alloc ();

#define ENLARGE(ptr, nel, type) \
    if (!(ptr = (type *)realloc (ptr, (unsigned)(nel * sizeof (type))))) die_alloc ();

#define DELETE(ptr) free ((void *) ptr)

#define DELETESVAL(ptr) ptr -> next = free_svals; free_svals = ptr

#define ZOOMIN      1
#define ZOOMOUT     2
#define MEASURE     3
#define MOVESIG     4
#define RENAME      5
#define DELSIG     13
#define COPYSIG    14
#define CHANGE     15
#define YANK       16
#define PUT        17

/* defines for intermediate reading */
#define END_INTER  -1
#define NON_INTER   0
#define  IN_INTER   1
#define BEG_INTER   2

