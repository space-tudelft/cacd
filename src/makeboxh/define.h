/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

#define P_E   fprintf(stderr,
#define TRUE		1
#define FALSE		0

struct wdw { /* wdw-list element structure */
    long   wdw[4];	    /* window-coordinates */
    struct wdw *next; 	    /* link to next element */
};

struct ctree { /* cell-tree node structure */
    char   name[DM_MAXNAME+1];	/* cell name */
    struct ctree *lchild;	/* left child link */
    struct ctree *rchild;	/* right child link */
};

struct cptrl { /* cell-ptr-list element structure */
    struct ctree *cell;	    /* ptr to cell in cell-tree */
    struct cptrl *next;	    /* link to next element */
};

struct mc_elmt { /* mc-tree element structure */
    char   name[DM_MAXNAME+1];	 /* cell name */
    char   inst_name[DM_MAXNAME+1]; /* instance name */
    long   imported;
    long   bbox[4];		 /* mc-bounding box */
    long   mtx[6];		 /* transf. matrix */
    long   dx, nx;		 /* x-copies */
    long   dy, ny;		 /* y-copies */
    struct wdw *act_regl;	 /* ptr to act-region-list */
    struct mc_elmt *sibling;	 /* link to sibling node */
    struct mc_elmt *child;	 /* link to child node */
    struct mc_elmt *parent;	 /* link to parent node */
};

#define ALLOCPTR(ptr,name) {\
if(!(ptr=(struct name *)malloc(sizeof(struct name))))\
errexit(6,"");}

#define ALLOCARR(ptr,el,name) {\
if(!(ptr=(name *)malloc((unsigned)((el)*sizeof(name)))))\
errexit(6,"");}

#define FREE(ptr) free((char *)(ptr))
