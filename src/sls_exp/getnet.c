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

#include "src/sls_exp/extern.h"

extern int findhashedname (char *name);
extern int findnodes (PATH_SPEC *path, MODELCALLTABLE *mcall, int hashed, NODE_REF_LIST **return_list, int permanent);
extern void join_node (int nxh, int nxj);
extern void resetfindnodes (void);

char fn_net[DM_MAXNAME+14];
int item;
int fc_getnet = TRUE;   /* first call to getnet must still be done */

PATH_SPEC path_sp[2];

/* get nets (node connections) of model m */
void getnet (DM_CELL *m)
{
    DM_STREAM * dsp;
    long lower[10], lower1[10], lower2[10];
    long upper[10], upper1[10], upper2[10];
    int i, new, q, k, fnv, nx, ntx, ntx2, nbr, xx;
    PATH_SPEC * path;
    NODE_REF_LIST * ref_list;
    NODE_REF_LIST * eqv_ref_list;
    char attribute_string[256];
    char buf[DM_MAXPATHLEN];
    char net_name[DM_MAXNAME + 1];
    int net_dim;
    int net_neqv;
    struct cir_net *net_eqv;

    if (fc_getnet) {
        path_sp[0].next = NULL;
        path_sp[0].also = NULL;
        path_sp[1].next = NULL;
        path_sp[1].also = NULL;
	fc_getnet = FALSE;
    }

    dm_get_do_not_alloc = 1;
    cnet.net_attribute = attribute_string;
    cnet.net_lower  = lower;
    cnet.net_upper  = upper;
    cnet.inst_lower = lower1;
    cnet.inst_upper = upper1;
    cnet.ref_lower  = lower2;
    cnet.ref_upper  = upper2;

    sprintf (fn_net, "circuit/%s/net", m -> cell);

    dsp = dmOpenStream (m, "net", "r");

    /* first the net records are parsed to allocate the nodes in the
       current level of the hierarchy (net nodes without instance name).
       only these nodes are assumed to be the 'head' net.
    */

    item = 0;
    while (dmGetDesignData (dsp, CIR_NET_ATOM) > 0) {
	net_neqv = cnet.net_neqv;
	item++;

        resetfindnodes ();

        strcpy (path_sp[0].name, cnet.net_name);
        path_sp[0].xarray[0][0] = cnet.net_dim;
        for (k = 0; k < cnet.net_dim; k++) {
            path_sp[0].xarray[k+1][0] = cnet.net_lower[k];
            path_sp[0].xarray[k+1][1] = cnet.net_upper[k];
        }
        path_sp[0].next = NULL;
        path = &(path_sp[0]);

        fnv = findnodes (path, NULL, TRUE, &ref_list, FALSE);

        if (fnv == NAMENEG) {      /* node doesn't exist yet */

            ntx = newname (path -> name);
	    NT[ntx].sort = Node;
	    nbr = 1;

	    if (path -> xarray[0][0] > 0) {
		new = NT[ntx].xtx = newxt ();
		XT[ new ] = path -> xarray[0][0];
		for (i = 1; i <= path -> xarray[0][0]; i++) {
		    if (path -> xarray[i][0] <= path -> xarray[i][1]) {
			new = newxt ();
			XT[ new ] = path -> xarray[i][0];
			new = newxt ();
			XT[ new ] = path -> xarray[i][1];
			nbr = nbr *
			      (path -> xarray[i][1] - path -> xarray[i][0] + 1);
		    }
		    else {
			new = newxt ();
			XT[ new ] = path -> xarray[i][1];
			new = newxt ();
			XT[ new ] = path -> xarray[i][0];
			nbr = nbr * (path -> xarray[i][0] - path -> xarray[i][1] + 1);
		    }
		}
	    }

	    if (path -> xarray[0][0] <= 0) {
	        nx = newnode ();
	        NT[ ntx ].x = nx;
	        N[ nx ].ntx = ntx;
	    }
	    else {
	        xx = newxx (nbr) - nbr + 1;
	        NT[ ntx ].x = xx;
	        while (nbr-- > 0) {
		    nx = newnode ();
		    XX[ xx++ ] = nx;
	            N[ nx ].ntx = ntx;
	        }
	    }
        }
	else if (fnv == REFIMIS || fnv == REFINEG || fnv == REFIERR) {
	    dberror (fn_net, item, "inconsistent index for net", cnet.net_name);
	}
        else if (fnv < 0) {
	    dberror (fn_net, item, "error for net", cnet.net_name);
        }

        for (q = 0; q < net_neqv; q++) {
	    dmGetDesignData (dsp, CIR_NET_ATOM);
	}
    }

    dmSeek (dsp, (long)0, 0);  /* rewind */

    /* now the actual net coalescing will be done */

    item = 0;
    while (dmGetDesignData (dsp, CIR_NET_ATOM) > 0) {
	item++;

	net_neqv = cnet.net_neqv;
	strcpy (net_name, cnet.net_name);
	net_dim = cnet.net_dim;

        for (q = 0; q < net_neqv; q++) {
	    dmGetDesignData (dsp, CIR_NET_ATOM);
	    net_eqv = &cnet;

            resetfindnodes ();

            strcpy (path_sp[0].name, net_name);
            path_sp[0].xarray[0][0] = net_dim;
            for (k = 0; k < net_dim; k++) {
                path_sp[0].xarray[k+1][0] = net_eqv -> ref_lower[k];
                path_sp[0].xarray[k+1][1] = net_eqv -> ref_upper[k];
            }
            path_sp[0].next = NULL;
            path = &(path_sp[0]);

            fnv = findnodes (path, NULL, TRUE, &ref_list, FALSE);

            if (fnv < 0)
	        dberror (fn_net, item, "error for net", net_name);

            strcpy (path_sp[0].name, net_eqv -> inst_name);
            path_sp[0].xarray[0][0] = net_eqv -> inst_dim;
            for (k = 0; k < net_eqv -> inst_dim; k++) {
                path_sp[0].xarray[k+1][0] = net_eqv -> inst_lower[k];
                path_sp[0].xarray[k+1][1] = net_eqv -> inst_upper[k];
            }
            strcpy (path_sp[1].name, net_eqv -> net_name);
            path_sp[1].xarray[0][0] = net_eqv -> net_dim;
            for (k = 0; k < net_eqv -> net_dim; k++) {
                path_sp[1].xarray[k+1][0] = net_eqv -> net_lower[k];
                path_sp[1].xarray[k+1][1] = net_eqv -> net_upper[k];
            }
            if (net_eqv -> inst_name[0] == '\0') {
                path = &(path_sp[1]);
            }
            else {
                path = &(path_sp[0]);
                path_sp[0].next = &(path_sp[1]);
            }

            fnv = findnodes (path, NULL, TRUE, &eqv_ref_list, FALSE);

            if (fnv == NAMENEG
                && path -> next == NULL
                && path -> xarray[0][0] == 0)  {
                           /* nodes that are not a terminal of a modelcall   */
                           /* and that are also not an array, can be defined */
                           /* by being used as an equivalence */

                ntx = newname (path -> name);
	        NT[ntx].sort = Node;

	        nx = newnode ();
	        NT[ ntx ].x = nx;
	        N[ nx ].ntx = ntx;
                fnv = findnodes (path, NULL, TRUE, &eqv_ref_list, FALSE);
	    }

	    if (fnv == NAMENEG) {
		if (path -> next != NULL) {
		    ntx2 = findhashedname (net_eqv -> inst_name);
		    if (ntx2 >= 0 && NT[ntx2].sort == Modelcall)
			sprintf (buf, "reference to unknown terminal %s\n   of cell %s (instance %s)",
			     net_eqv -> net_name, MT[ MCT[ NT[ntx2].x ].mtx ].name, net_eqv -> inst_name);
		    else if (!strncmp (net_eqv -> inst_name, "_CG",  3)) goto skip_dberror;
		    else if (!strncmp (net_eqv -> inst_name, "_CSG", 4)) goto skip_dberror;
		    else
			sprintf (buf, "reference to unknown terminal %s of instance %s",
			     net_eqv -> net_name, net_eqv -> inst_name);
		}
		else {
		    sprintf (buf, "reference to unknown terminal %s", net_eqv -> net_name);
		}
		dberror (fn_net, item, buf, NULL);
skip_dberror:;
	    }
	    else if (fnv == REFIMIS || fnv == REFINEG || fnv == REFIERR) {
		if (path -> next != NULL) {
		    ntx2 = findhashedname (net_eqv -> inst_name);
		    if (ntx2 >= 0 && NT[ntx2].sort == Modelcall)
			sprintf (buf, "inconsistent index usage for terminal %s\n   of cell %s (instance %s)",
			     net_eqv -> net_name, MT[ MCT[ NT[ntx2].x ].mtx ].name, net_eqv -> inst_name);
		    else
			sprintf (buf, "inconsistent index usage for terminal %s of instance %s",
			     net_eqv -> net_name, net_eqv -> inst_name);
		}
		else {
		    sprintf (buf, "inconsistent index usage for terminal %s", net_eqv -> net_name);
		}
	        dberror (fn_net, item, buf, NULL);
	    }
	    else if (fnv == NOTRELEVANT) {
		/* do nothing with it */
	    }
            else if (fnv < 0) {
		if (path -> next != NULL) {
		    sprintf (buf, "error for terminal %s of instance %s",
			     net_eqv -> net_name, net_eqv -> inst_name);
		}
		else {
		    sprintf (buf, "error for terminal %s", net_eqv -> net_name);
		}
	        dberror (fn_net, item, buf, NULL);
	    }

	    if (fnv >= 0) {
		while (ref_list != NULL) {
		    if (eqv_ref_list == NULL)
			dberror (fn_net, item, "too less nodes in equivalence", NULL);
		    if (eqv_ref_list -> nx < 0) {
			*(eqv_ref_list -> xptr) = ref_list -> nx;
		    }
		    else
			join_node (ref_list -> nx, eqv_ref_list -> nx);
		    ref_list = ref_list -> next;
		    eqv_ref_list = eqv_ref_list -> next;
		}
		if (eqv_ref_list != NULL)
		    dberror (fn_net, item, "too many nodes in equivalence", NULL);
	    }
        }
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp, COMPLETE);
}

void join_node (int nxh, int nxj) /* joins node nxj to node nxh */
{
    NODE *nh, *nj;
    int j, nbr, index;

    while (N[nxh].redirect) nxh = N[nxh].cx;

    while (N[nxj].redirect) nxj = N[nxj].cx;

    if (nxh == nxj) return;

    nh = &N[nxh];
    nj = &N[nxj];

    nh -> statcap += nj -> statcap;

    if (nh -> linked) {
	if (nh -> dsx >= 0) {
	    j = nh -> dsx;
	    nh -> dsx = nbr = DS[j++];
	    while (nbr-- > 0) {
	        DS[j++] = 0;
	    }
	}
	else
	    nh -> dsx = 0;

	if (nh -> cx >= 0) {
	    j = nh -> cx;
	    nh -> cx = nbr = C[j++].c;
	    while (nbr-- > 0) {
	        C[j++].c = 0;
	    }
	}
	else
	    nh -> cx = 0;

	nh -> linked = FALSE;
    }

    /* the number of ds or c connections of nj is added to nh.      */
    /* these connections are originating from nodes already linked, */
    /* but when nj is not linked it can have them because of a      */
    /* possible previous joining */

    if (! nj -> linked) {
        nh -> dsx += nj -> dsx;
        nh -> cx += nj -> cx;
	nj -> dsx = 0;
	nj -> cx = 0;
    }
    else {
	if (nj -> dsx >= 0) {
	    nh -> dsx += DS[ nj -> dsx];
	    index = nj -> dsx;
	    nbr = DS[ index ];
	    DS[ index ] = 0;
	    while (nbr-- > 0)
		DS[ ++index ] = 0;
	}
	if (nj -> cx >= 0) {
	    nh -> cx += C[ nj -> cx].c;
	    index = nj -> cx;
	    nbr = C[ index ].c;
	    C[ index ].c = 0;
	    while (nbr-- > 0)
		C[ ++index ].c = 0;
        }
    }

    nj -> redirect = TRUE;
    nj -> cx = nxh;
}
