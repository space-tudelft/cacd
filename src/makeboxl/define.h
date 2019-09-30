/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "src/libddm/dmincl.h"

#define P_E fprintf(stderr,

/*  Next define is used for static functions.
    Since the gprof profiler does not interpret static functions
    very well, this can then be defined empty
    (Provided there are no name clashes.)
    Protected by ifndef to allow -DPrivate="" on command line
*/
#ifndef Private
#define Private static /* or empty */
#endif

struct mc_elmt { /* mc-tree node structure */
    struct clist *parent;	/* link to parent cell node */
    char  *name;		/* cell name or alias */
    char  *inst_name;		/* instance name */
    long   mtx[6];		/* transf. matrix */
    long   dx, nx;		/* x-copies */
    long   dy, ny;		/* y-copies */
    struct mc_elmt *mc_next;	/* link to next element */
};

struct clist { /* cell-list element structure */
    char          *name;	/* local cell name */
    DM_PROJECT    *pkey;	/* local project key */
    DM_CELL       *ckey;	/* cell access key */
    short          hier;	/* 0: flatten
				   1: use as an instance
				   2: flatten + use as an instance */
    short          status;	/* 0: regular cell (SdeG4.11)
				   1: device  ,,
				   2: macro   ,,
				   3: device + macro */
    short     all_allowed;
    long long freemasks_bits;
    struct mc_elmt *mc_p;	/* link to mc-node */
    struct mc_elmt *mc_p_last;	/* link to last mc-node */
    struct clist   *cl_next;	/* link to next element */
    struct clist   *ht_next;	/* link to next hashtable element */
};

struct tmtx { /* transf. matrix element structure */
    struct mc_elmt *mc;         /* corresponding modelcall structure */
    short  x, y;		/* corresponding instance indices */
    short  tid;			/* do tid for instance */
    short  allow_all;
    long long allowmasks;
    long   mtx[6];		/* transf. matrix */
    struct tmtx_mc *tmc;        /* info about model call */
    struct tmtx *tm_next;	/* link to next element */
};

struct tmtx_mc {
    struct mc_elmt *mc;         /* corresponding modelcall structure */
    short  x, y;		/* corresponding instance indices */
    struct tmtx_mc *child;      /* child model call info */
    struct tmtx_mc *next;       /* next in list that contains all structures */
};

#define ALLOCPTR(ptr,name) {\
if(!(ptr=(struct name *)malloc(sizeof(struct name))))\
errexit(6,"");}

#define ALLOCARR(ptr,elmts,name) {\
if(!(ptr=(name *)malloc((elmts) * sizeof(name))))\
errexit(6,"");}

#define REALLOCARR(ptr,elmts,name) {\
if(!(ptr=(name *)realloc(ptr, (elmts) * sizeof(name))))\
errexit(6,"");}

#define FREE(ptr) free((char *)(ptr))

