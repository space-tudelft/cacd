/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
 *	S. de Graaf
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include "src/libddm/dmincl.h"

#ifdef SYSV
#define index strchr
#endif

#define RIGHT		 1
#define LEFT		-1
#define MAXCHRPATH	50
#define MAXWORDS	50
#define MAXLISTMASKS    10

/* struct la_elmt gtype codes */
#define LA_SBOX   003
#define LA_SB_P   001
#define LA_SB_N   002
#define LA_G_XL   004
#define LA_G_XR   010
#define LA_G_YB   020
#define LA_G_YT   040

/* constant tangens of 22.5 degrees */
#define TAN225  0.414213562

/* constant square root 2 */
#define SQRT2   1.414213562

/* constant half square root 2 */
#define HSQRT2  0.70710678

#define CALLOC_ARR(ptr,type,nr) { ptr=(type *)calloc((size_t)(nr),sizeof(type)); if(!ptr)cant_alloc(); }
#define ALLOC_STRUCT(ptr,name) {\
ptr=(struct name *)malloc(sizeof(struct name));\
if(!ptr)cant_alloc();}

#define FREE(ptr) free((void *)ptr)

struct pat_bbox
{
    long xl, xr, yb, yt;
};

struct ptree
{
    char name[DM_MAXNAME+1];
    int  correct;
    struct pat_bbox *bbox;
    IMPCELL      *impcell;
    struct ptree *rchild;
    struct ptree *lchild;
};

struct list
{
    char name[DM_MAXNAME+1];
    struct list *next;
    int nrofelements;
    int element[MAXLISTMASKS][2];
};

struct edge
{
    int x, ybottom, ytop, bodybit;
    struct edge *lnext, *rnext;
    struct edge *uneighbour, *dneighbour;
    struct edge *list;
};

struct scan
{
    int ybottom, ytop, state;
    struct scan *unext, *dnext;
    struct edge *usrc, *dsrc;
    struct scan *list;
};

struct la_elmt
{
    double xl, xr, yb, yt;
    int gtype;
    struct la_elmt *next;
};
