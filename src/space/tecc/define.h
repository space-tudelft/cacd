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

#ifndef TECC_DEFINE_H
#define TECC_DEFINE_H

#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/tecc/type.h"
#include "src/space/bipolar/define.h"
#include "src/space/extract/define.h"

#define ALLOC(ptr, nel, typ) { \
if (!(ptr = (typ *)calloc ((unsigned)(nel), sizeof(typ)))) { \
	fprintf (stderr, "Cannot allocate storage\n"); die (); } \
}

#define REALLOC(ptr, nel, typ, old) { \
if (!(ptr = (typ *)realloc ((void *)ptr, (unsigned)(nel) * sizeof(typ)))) { \
	fprintf (stderr, "Cannot allocate storage\n"); die (); } \
if ((nel-old) > 0) memset ((void *)&ptr[old], 0, (unsigned)(nel-old) * sizeof(typ)); \
}

#define FREE(ptr) { if (ptr) { free ((char *)ptr); ptr = NULL; }}

#define M_TYPE 0
#define N_TYPE 1
#define P_TYPE 2

#define MAXCON       80
#define MAXTOR       40
#define MAXCAP     1200
#define MAXRES       40
#define MAXVDIM      40
#define MAXSHAPE     40
#define MAXSUBMASKS  40
#define MAXBJT       40
#define MAXJUN       40
#define MAXCNT       40
#define MAXDIEL      40
#define MAX_DIEL_INTERPOLATION_POINTS 250
#define MAXSUBSTR    40
#define MAX_SUBS_INTERPOLATION_POINTS 250
#define MAXSELFSUB   32
#define MAXNEWMSK    40
#define MAXMUTSUB    32
#define MAXSUBCONTS  40
#define MAXRESIZE    40
#define MAXWAFER     30

#define MAXRESSORT   40
#define MAXCAPSORT   80

#endif /* TECC_DEFINE_H */
