/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

#define P_E fprintf(stderr,
#define RAD_DEG 0.01745329252

#define MAXPATH 50	/* max # of chars in a pathname */
#define NOINTS 600	/* max # of ints  of a poly */

struct mod_bbox {
    long    xl, xr, yb, yt;
};

struct name_tree {
    char    name[DM_MAXNAME+1];
    int     errflag;
    struct mod_bbox    *bbox;
    IMPCELL            *impcell;
    struct name_tree   *rchild;
    struct name_tree   *lchild;
};

/* Macro which can allocate a structure */

#define ALLOC(ptr,type) {\
if(!(ptr=(struct type *)malloc(sizeof(struct type))))\
pr_exit (07, 27, 0); }

#define FREE(ptr) free((char *)(ptr))

/* Macro which compares two integers */

#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))

/* Macro for rounding */

#define ROUND(x) (x < 0)?(x - 0.5001):(x + 0.5001)

#include "src/cldm/proto.h"
