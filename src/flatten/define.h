/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "src/libddm/dmincl.h"

#define PE fprintf (stderr,
#define PO fprintf (fp_ldm,

struct mc_elmt { /* mc-tree node structure */
    struct clist *parent;	/* link to parent cell node */
    long   mtx[6];		/* transf. matrix */
    long   dx, nx;		/* x-copies */
    long   dy, ny;		/* y-copies */
    struct mc_elmt *mc_next;	/* link to next element */
};

struct clist { /* cell-list element structure */
    DM_CELL        *ckey;	/* cell access key */
    int		    imps;	/* contains imported cells */
    struct mc_elmt *mc_p;	/* link to mc-node */
    struct clist   *cl_next;	/* link to next element */
    struct clist   *ht_next;	/* link to next hashtable element */
};

struct tmtx { /* transf. matrix element structure */
    long   mtx[6];		/* transf. matrix */
    struct tmtx *tm_next;	/* link to next element */
};

struct name_tree {
    char  name[DM_MAXNAME+1];
    struct name_tree *rchild;
    struct name_tree *lchild;
};

#define ALLOCPTR(ptr,name) {\
if(!(ptr=(struct name *)malloc(sizeof(struct name)))) pr_err (6, ""); }

#define ALLOCARR(ptr,elmts,name) {\
if(!(ptr=(name *)malloc(elmts * sizeof(name)))) pr_err (6, ""); }

#define FREE(ptr) free((char *)(ptr))
