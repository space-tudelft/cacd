/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	R. Paulussen
 *	S. de Graaf
 *	A.J. van Genderen
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "src/gds2/defs.h"
#include "src/libddm/dmincl.h"

#define MAXPATH   50           /* max # of chars in a pathname */
#define NOINTS    (2*MAX_COOR) /* max # of ints  of a boundary */

#define CALLOC(ptr,type,nr) {\
if(!(ptr=(type *)calloc((size_t)(nr),sizeof(type))))\
pr_exit (A, 17, 0); }

#define ALLOC(ptr,type) {\
if(!(ptr=(struct type *)malloc(sizeof(struct type))))\
pr_exit (A, 17, 0); }

#define FREE(ptr) free((void *)(ptr))

#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))

#define NEXT_RECORD next_rec ()

struct mod_bbox { /* cell bounding box co-ordinates */
    long   xl, xr, yb, yt;
};

struct name_tree {
    char             *name;     /* name of cell */
    IMPCELL          *impcell;  /* imported cell (if set) */
    struct mod_bbox  *bbox;               /* bbox struct  */
    int               status;             /* status cell  */
    struct name_tree *rchild;             /* tree childs  */
    struct name_tree *lchild;
};

struct stat stat_buf; /* file status info */

int	lay_code;	/* layer code (of process table) */
int 	data_type;	/* data type of layer */
int     ini_bbbox = 0;	/* bounding box flags, set when a ms statement */
int     ini_mcbbox = 0;	/* is encountered, reset when the
			   corresponding bounding box gets a value */

char    ms_name[DM_MAXNAME+1] = "??";/* name of current cell def. */
char    mc_name[DM_MAXNAME+1];	/* name of cell called */
char    instance[DM_MAXNAME+1];	/* name of cell inst. */

/* Binary sorted name trees */

struct name_tree   *tree_ptr = NULL;
struct name_tree   *mod_tree = NULL;
                   /* tree containing names of cells
		      defined in the database */

DM_PROJECT  *dmproject = NULL;          /* ptr to project      */
DM_PROCDATA *process;                   /* ptr to process info */
DM_CELL     *mod_key = NULL;            /* ptr to cell         */
DM_STREAM   *fp_info, *fp_mc,
            *fp_box, *fp_term, *fp_nor;

/* Element Parameters: */

int     int_val[NOINTS];	/* int  number buffer */
long    bbnd_xl, bbnd_xr;       /* box bounding box */
long    bbnd_yb, bbnd_yt;
long    mcbb_xl, mcbb_xr;       /* cell reference bbox */
long    mcbb_yb, mcbb_yt;

