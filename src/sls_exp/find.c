/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#ifdef SLS
#include "src/sls/extern.h"
#else
#include "src/sls_exp/extern.h"
#endif /* SLS */

extern char *Bsearch (char *key, char *base, unsigned nel, unsigned width, int (*compar)());
extern int findhashedname (char *name);
extern int vfunc (FUNCTION *f, char *varname, int *xvector, NODE_REF_LIST *ref);

static int chk_bounds (int xtx, int *xr);
static int findname (char *name, NAMETABLE *tabstart, int tabsize);
static NODE_REF_LIST *newrefel (int permanent);

typedef struct ref_el_ptr {     /* datastructure for node_ref_list */
    NODE_REF_LIST * noderefel;  /* elements managing               */
    struct ref_el_ptr * next;
} REF_EL_PTR;

REF_EL_PTR * rep_begin = NULL;   /* first node_ref_list element to  */
				 /* be used after resetfindnodes () */
REF_EL_PTR * rep_last = NULL;    /* last node_ref_list element used */

int findnodes (PATH_SPEC *path, MODELCALLTABLE *mcall, int hashed, NODE_REF_LIST **return_list, int permanent)
/* find node reference list
 * returns NAMENEG if path name doesn't exist
 *         NOPATH if no path is specified
 *         NONODE if path is no node or variab
 *         REFIMIS if real name has indices
 *         REFINEG if real name has no indices
 *         REFIERR indices incorrect
 *         NODETYPE if items are nodes
 *         CHARTYPE if items are chars
 *         INTEGERTYPE if items are integers
 *         FLOATTYPE if items are floats
 *         DOUBLETYPE if items are floats
 *         DIFFTYPE if items are different
 * in node ref list the following is returned
 *  nx   = actual node index in N[]
 *  xptr = pointer to variable, which is the formal
 *         parameter of a device (then nx will be < 0)
 * function arguments:
 *  path   = the path name of the node(s) to be searched
 *  mcall  = modelcall in which path is valid
 *  hashed = the first name of path is in the hash table
 *  return_list = adress of pointer to first element of reference list returned
 *  permanent   = reference list returned will be used permanently
 *        when FALSE, resetfindnodes() will cause a reuse of the reference list
 */
{
    int ntx;
    int xtx;
    int x;
    int i, j, k, loc;
    int ready;
    int xvector[MAXDIM + 1];
    int fval;
    int id1, id2;
    int inner_ready;
    int inner_xvector[MAXDIM + 1];
    PATH_SPEC * inner_path;   /* used when built-in model has been recognized */
    NODE_REF_LIST * new_ref_el;
    NODE_REF_LIST * last_list;   /* last node_ref_list element found */
    int type = EMPTYTYPE;

    /* the path is specified as an hierarchical tree by means of
     * the also pointer (to the right) and the next pointer (downwards).
     * the references are searched by depth-first.
     * first the 'next' is visited and then the 'also'.
     */

    *return_list = NULL;
    last_list = NULL;

    if (path == NULL) {
        if (mcall == NULL)
            return (NOPATH);
        else
            return (NONODE);
    }

    while (path != NULL) {

        /* firstly, find name in NT[] */

        if (hashed) {
    	    ntx = findhashedname (path -> name);
        }
        else {
	    if (mcall == NULL)
	        ntx = findname (path -> name,
			        &NT[ NT_cnt - MT[MT_cnt - 1].nt_cnt],
			        MT[MT_cnt - 1].nt_cnt);
	    else
                ntx = findname (path -> name,
			        &NT[ mcall -> n_ntx ],
			        MT[ mcall -> mtx ].nt_cnt);
        }

        if (ntx < 0)
	    return (NAMENEG);   /* name doesn't exist */

        xtx = NT[ntx].xtx;
        if (xtx >= 0 && path -> xarray[0][0] <= 0)
	    return (REFIMIS);              /* real name has indices */

        if (xtx < 0 && path -> xarray[0][0] > 0)
	    return (REFINEG);           /* real name has no indices */

        /* for each vector of indices in the range given by xarray,
	 * the path is now searched recursively.
	 * xvector contains the current vector of indices.
	 */

        xvector[0] = path -> xarray[0][0];
        for (i = path -> xarray[0][0]; i > 0; i--) {
            xvector[i] = path -> xarray[i][0];
        }

        ready = FALSE;
        do {
	    if (xvector[0] > 0) {

	        if (chk_bounds (xtx, xvector) != 0)
	            return (REFIERR);    /* indices are incorrect */

                /* find offset in NT[] (loc) for the current indices */

	        for (i = xvector[0], j = 1, k = xtx + 2 * XT[xtx] - 1, loc = 0;
	             i > 0;
	             i--, j = j * (XT[k + 1] - XT[k] + 1), k -= 2)
	            loc += j * (xvector[i] - XT[k]);

		if (NT[ntx].sort == Node || NT[ntx].sort == Node_t)
	            x = XX[ NT[ntx].x + loc ];   /* when it is a node with an */
		else                             /* index, the reference is   */
	            x = NT[ntx].x + loc;         /* done via XX               */
	    }
	    else
	        x = NT[ntx].x;

	    switch ( NT[ntx].sort ) {

	        case Node :
	        case Node_t :

	            new_ref_el = newrefel (permanent);

	            while ( N[ x ].redirect )
	                x = N[ x ].cx;

	            new_ref_el -> nx = x;
	            if (*return_list == NULL)
	                *return_list = new_ref_el;
	            else
	                last_list -> next = new_ref_el;
	            last_list = new_ref_el;

                    if (type == EMPTYTYPE)
                        type = NODETYPE;
                    else if (type != NODETYPE)
                        type = DIFFTYPE;

		    break;

	        case Modelcall :

                    if (*return_list == NULL) {
		        if ((fval = findnodes (path -> next, &MCT[ x ], FALSE,
			 	              return_list, permanent)) < 0) {
			    return (fval);
			}
		        last_list = *return_list;
		    }
		    else {
		        if ((fval = findnodes (path -> next, &MCT[ x ], FALSE,
			                      &(last_list -> next), permanent)) < 0) {
			    return (fval);
			}
		    }

		    while ( last_list -> next != NULL )
		        last_list = last_list -> next;

                    if (type == EMPTYTYPE)
                        type = fval;
                    else if (type != fval)
                        type = DIFFTYPE;

		    break;

	        case Transistor :

		    inner_path = path -> next;
		    while (inner_path != NULL) {
	                new_ref_el = newrefel (permanent);
	                if (*return_list == NULL)
	                    *return_list = new_ref_el;
	                else
	                    last_list -> next = new_ref_el;
			if ( inner_path -> name[1] != '\0' )
			    return (NAMENEG);
			switch ( inner_path -> name[0] ) {
			    case 'g' :
		                new_ref_el -> nx = T[x].gate;
		                new_ref_el -> xptr = &(T[x].gate);
			        break;
			    case 'd' :
		                new_ref_el -> nx = T[x].drain;
		                new_ref_el -> xptr = &(T[x].drain);
			        break;
			    case 's' :
		                new_ref_el -> nx = T[x].source;
		                new_ref_el -> xptr = &(T[x].source);
			        break;
			    case 'p' :
		                new_ref_el -> nx = T[x].drain;
		                new_ref_el -> xptr = &(T[x].drain);
			        break;
			    case 'n' :
		                new_ref_el -> nx = T[x].source;
		                new_ref_el -> xptr = &(T[x].source);
				break;
			    case 'b':
				return (NOTRELEVANT);
			    default :
				return (NAMENEG);
			}
	                last_list = new_ref_el;

		        inner_path = inner_path -> also;
		    }

                    if (type == EMPTYTYPE)
                        type = NODETYPE;
                    else if (type != NODETYPE)
                        type = DIFFTYPE;

		    break;

	        case Intercap :

		    inner_path = path -> next;
		    while (inner_path != NULL) {
	                new_ref_el = newrefel (permanent);
	                if (*return_list == NULL)
	                    *return_list = new_ref_el;
	                else
	                    last_list -> next = new_ref_el;
			if (inner_path -> name[1] != '\0')
			    return (NAMENEG);
			switch (inner_path -> name[0]) {
			    case 'p' :
		                new_ref_el -> nx = I[x].con1;
		                new_ref_el -> xptr = &(I[x].con1);
				break;
			    case 'n' :
		                new_ref_el -> nx = I[x].con2;
		                new_ref_el -> xptr = &(I[x].con2);
				break;
			    default :
			        return (NAMENEG);
			}
	                last_list = new_ref_el;

		        inner_path = inner_path -> also;
		    }

                    if (type == EMPTYTYPE)
                        type = NODETYPE;
                    else if (type != NODETYPE)
                        type = DIFFTYPE;

		    break;

		case Functional :

                    if (F[x].type >= 1000) {

		        inner_path = path -> next;
		        while (inner_path != NULL) {

	                    new_ref_el = newrefel (permanent);
	                    if (*return_list == NULL)
	                        *return_list = new_ref_el;
	                    else
	                        last_list -> next = new_ref_el;

			    if (inner_path -> name[1] != '\0')
			        return (NAMENEG);

			    switch (inner_path -> name[0]) {
			        case 'i' :
				    if (inner_path -> xarray[0][0] == 0)
				        return (REFIMIS);
				    else if (inner_path -> xarray[0][0] != 1)
				        return (REFIERR);
				    id1 = inner_path -> xarray[1][0];
				    id2 = inner_path -> xarray[1][1];
				    if (id1 < 0 || id1 >= FI[ F[x].fix ]
				        || id2 < 0 || id2 >= FI[ F[x].fix ])
				        return (REFIERR);
		                    new_ref_el -> nx = FI[ F[x].fix + id1 + 1 ];
		                    new_ref_el -> xptr = &(FI[ F[x].fix + id1 + 1 ]);

                                    if (id1 != id2) {
                                        do {
                                            if (id1 < id2)
				                id1++;
                                            else
                                                id1--;
	                                    new_ref_el -> next = newrefel (permanent);
				            new_ref_el = new_ref_el -> next;
		                            new_ref_el -> nx = FI[ F[x].fix + id1 + 1 ];
		                            new_ref_el -> xptr = &(FI[ F[x].fix + id1 + 1 ]);
                                        }
				        while (id1 != id2);
                                    }

				    break;

			        case 'o' :  /* direct functional ouput */
		                    new_ref_el -> nx = FO[ F[x].fox + 1 ].x;
		                    new_ref_el -> xptr = &(FO[ F[x].fox + 1 ].x);
				    break;

			        default :
			            return (NAMENEG);
			    }
	                    last_list = new_ref_el;

		            inner_path = inner_path -> also;
		        }

                        if (type == EMPTYTYPE)
                            type = NODETYPE;
                        else if (type != NODETYPE)
                            type = DIFFTYPE;
                    }
                    else {

                        inner_path = path -> next;
                        while (inner_path != NULL) {

                            inner_xvector[0] = inner_path -> xarray[0][0];
                            for (i = inner_xvector[0]; i > 0; i--)
                                inner_xvector[i] = inner_path -> xarray[i][0];
                            inner_ready = FALSE;

                            do {
	                        new_ref_el = newrefel (permanent);
	                        if (*return_list == NULL)
	                            *return_list = new_ref_el;
	                        else
	                            last_list -> next = new_ref_el;

                                if ((fval = vfunc (&F[x], inner_path -> name, inner_xvector, new_ref_el)) < 0)
                                    return (fval);
                                if (type == EMPTYTYPE)
                                    type = fval;
                                else if (type != fval)
                                    type = DIFFTYPE;

                                if (inner_xvector[0] > 0) {
                                    for (i = inner_xvector[0]; i > 0; i--) {
                                        if (inner_path -> xarray[i][0] <= inner_path -> xarray[i][1]) {
                                            inner_xvector[i]++;
                                            if (inner_xvector[i] <= inner_path -> xarray[i][1])
                                                break;
                                            else {
                                                inner_xvector[i] = inner_path -> xarray[i][0];
                                                if (i == 1) inner_ready = TRUE;
                                            }
                                        }
                                        else {
                                            inner_xvector[i]--;
                                            if (inner_xvector[i] >= inner_path -> xarray[i][1])
                                                break;
                                            else {
                                                inner_xvector[i] = inner_path -> xarray[i][0];
                                                if (i == 1) inner_ready = TRUE;
                                            }
                                        }
                                    }
                                }
                                else {
                                    inner_ready = TRUE;
                                }
                                last_list = new_ref_el;
                            }
                            while (!inner_ready);

                            last_list = new_ref_el;

                            inner_path = inner_path -> also;
                        }
                    }

		    break;
	    }

	    /* find new indices by in- or decrementing the current vector */

	    if (xvector[0] > 0) {
                for (i = xvector[0]; i > 0; i--) {
		    if (path -> xarray[i][0] <= path -> xarray[i][1]) {
                        xvector[i]++;
	                if (xvector[i] <= path -> xarray[i][1])
		            break;
	                else {
		            xvector[i] = path -> xarray[i][0];
		            if (i == 1) ready = TRUE;
	                }
		    }
		    else {
                        xvector[i]--;
	                if (xvector[i] >= path -> xarray[i][1])
		            break;
	                else {
		            xvector[i] = path -> xarray[i][0];
		            if (i == 1) ready = TRUE;
	                }
		    }
                }
	    }
	    else
	        ready = TRUE;
        }
        while (! ready);

        path = path -> also;
    }

    return (type);
}

static int bs_nmcmp (char *name, NAMETABLE *tabel)
/* compares string name with name from tabel */
{
    return ( strcmp (name, ST + (tabel -> name)) );
}

static int findname (char *name, NAMETABLE *tabstart, int tabsize)
/* finds name in nametable NT[]
 * function arguments:
 * name     = name to be found
 * tabstart = first element of the part of NT[] in which has to be searced
 * tabsize  = length of the part in which has to be searched
 */
{
    NAMETABLE * nt;

    if (!(nt = (NAMETABLE *)Bsearch (name, (char *)tabstart, tabsize, sizeof (NAMETABLE), bs_nmcmp))) {
	return (-1);
    }
    return (nt - NT);
}

static int chk_bounds (int xtx, int *xr)
/* returns 0 if indices are between bounds */
{
    int i, j;
    int * x;
    int errflag = 0;

    if ( *xr != XT[xtx] ) {
	errflag++;
    }
    else {
	for (i = 0, j = xtx + 1, x = xr + 1; i < *xr; i++, j += 2, x++) {
	    if (*x < XT[j] || *x > XT[j + 1]) {
		errflag++;
	    }
	}
    }

    return (errflag);
}

void resetfindnodes ()
{
    rep_last = rep_begin;
}

static NODE_REF_LIST *newrefel (int permanent)
/* gives new ref list element
 * element will be used permanently
 */
{
    NODE_REF_LIST * new_ref_el;

    if (permanent) {
	PALLOC (new_ref_el, 1, NODE_REF_LIST);
    }
    else {
	if (rep_begin == NULL) {
	    PALLOC (rep_begin, 1, REF_EL_PTR);
	    PALLOC (new_ref_el, 1, NODE_REF_LIST);
	    rep_last = rep_begin;
	    rep_last -> noderefel = new_ref_el;
	    rep_last -> next = NULL;
	}
	else {
	    if (rep_last -> next == NULL) {
	        PALLOC (rep_last -> next, 1, REF_EL_PTR);
	        PALLOC (new_ref_el, 1, NODE_REF_LIST);
		rep_last = rep_last -> next;
		rep_last -> noderefel = new_ref_el;
	        rep_last -> next = NULL;
	    }
	    else {
		rep_last = rep_last -> next;
		new_ref_el = rep_last -> noderefel;
	    }
	}
    }

    new_ref_el -> nx = -1;
    new_ref_el -> xptr = NULL;
    new_ref_el -> next = NULL;
    return (new_ref_el);
}

void printrefs (NODE_REF_LIST *refs)
{
    while (refs) {
	fprintf (stderr, "refs=%p  nx=%d  xptr=%p\n", refs, refs -> nx, refs -> xptr);
	refs = refs -> next;
    }
}

void printpath (PATH_SPEC *path)
{
    int i;

    if (!path) return;

    fprintf (stderr, "start also group\n");

    while (path) {
	fprintf (stderr, "path=%p  name=%s  %d", path, path -> name, path -> xarray[0][0]);
	for (i = 1; i <= path -> xarray[0][0]; i++) {
	    fprintf (stderr, " %d %d", path -> xarray[i][0], path -> xarray[i][1]);
	}
	fprintf (stderr, "\n");
	printpath (path -> next);
	path = path -> also;
    }

    fprintf (stderr, "end also group\n");
}
