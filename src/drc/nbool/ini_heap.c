/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
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

#include "src/drc/nbool/nbool.h"

static int insert_file (char *lfname, unsigned int newmask, int *n_inp, int term_lay);
static int pres_in_heap (int n_inp, char *mask_name);

void ini_heap (char *input)
/* input - contains layernames */
{
/* This function initialises the edge_heap.
*/
    int     i;			/* loop variable	 */
    int     n_inp;		/* nbr of edge in input	 */
    int     i_str;		/* pointer in inputstring */
    int     i_wrd;		/* pointer in word	 */
    char    lfname[MAXLINE];	/* layer_file		 */
    char    t_lfname[MAXLINE];	/* terminal layer_file	 */
    unsigned int    newmask;	/* bolean mask_id	 */
    int	    newmask_flag;	/* newmask used or not	 */

    nf = 0;
    n_inp = 0;
    i_str = 0;
    i_wrd = 0;

/* reserve the first mask_id's for the masks given in	 */
/* the process.						 */

    newmask = 1;
    newmask = newmask << (process -> nomasks - 1);

/* decodation of the input_string			 */

    while (input[i_str] != '\0') {
	switch (input[i_str]) {
	    case '\040':
	    case '\t':
	    case '\n':
		if (i_wrd != 0) {
		    lfname[i_wrd] = '\0';
		    newmask_flag = insert_file (lfname, newmask, &n_inp, FALSE);
		    if ((newmask_flag == NEW_TERMLAY) &&
		       (chk_flag == CHK_HRCHY)) {
			sprintf(t_lfname, "t_%s", lfname);
		        insert_file (t_lfname, newmask, &n_inp, TRUE);
			mask_terms = mask_terms | newmask;
			newmask = newmask << 1;
		    }
		    if (newmask_flag == TRUE) {
			newmask = newmask << 1;
		    }
		    i_wrd = 0;
		}
		i_str++;
		break;
	    default:
		lfname[i_wrd++] = input[i_str++];
		break;
	}
    }

    if (i_wrd != 0) {
	lfname[i_wrd] = '\0';
        newmask_flag = insert_file (lfname, newmask, &n_inp, FALSE);
        if ((newmask_flag == NEW_TERMLAY) && (chk_flag == CHK_HRCHY)) {
	    sprintf(t_lfname, "t_%s", lfname);
	    insert_file (t_lfname, newmask, &n_inp, TRUE);
	    mask_terms = mask_terms | newmask;
	    newmask = newmask << 1;
	}
	if (newmask_flag == TRUE) {
	    newmask = newmask << 1;
        }
	i_wrd = 0;
    }

    if (chk_flag == CHK_HRCHY) {

    /* If the hierarchy is to be checked all masks of the */
    /* process must be read, including the terminal masks. */

	for (i = 1; i < process -> nomasks; ++i) {
	    if (pres_in_heap (n_inp, process -> mask_name[i]) == FALSE) {
		sprintf (lfname, "%s_vln", process -> mask_name[i]);
		newmask_flag = insert_file (lfname, newmask, &n_inp, FALSE);
                if ((newmask_flag == NEW_TERMLAY) && (chk_flag == CHK_HRCHY)) {
	            sprintf(t_lfname, "t_%s", lfname);
	            insert_file (t_lfname, newmask, &n_inp, TRUE);
	            mask_terms = mask_terms | newmask;
	            newmask = newmask << 1;
		}
	    }
	}
    }
    nomasks = n_inp;
}

static int insert_file (char *lfname, unsigned int newmask, int *n_inp, int term_lay)
/* lfname  - layer_file */
/* newmask - bolean mask_id */
/* n_inp   - nbr in input_heap */
/* term_lay - term_layer or not */
{
/* This procedure inserts the file given by mod_key and
 * lfname in the edge_heap. If it is a file of a layer
 * known in the process its corresponding parameters
 * such as mask_id and mask_type are taken from it.
 * If not the mask is taken from newmask and the mask_type
 * is set to BOLEAN or to TERM_MASK if term_lay is TRUE.
 */
    register int i;
    int  newmask_flag = FALSE; /* newmask or not */
    char vln_name[DM_MAXLAY + 7]; /* maskname_vln */

    for (i = 0; i < process -> nomasks; ++i) {
	sprintf (vln_name, "%s_vln", process -> mask_name[i]);
	if (strcmp (lfname, vln_name) == 0) {
	    edges[*n_inp].mask = (1 << process -> mask_no[i]);
	    edges[*n_inp].mask_type = process -> mask_type[i];
	    strcpy (edges[*n_inp].mask_name, lfname);
	    if ((edges[*n_inp].mask_type == CONN_MASK) &&
		(chk_flag == CHK_HRCHY)) {
		    edges[*n_inp].mask_term = newmask;
		    newmask_flag = NEW_TERMLAY;
	    }
	    break;
	}
    }

    if (i >= process -> nomasks) {
	if (newmask <= (1 << (MAX_NOMASKS - 1))) {
	    edges[*n_inp].mask = newmask;
	    if (term_lay == TRUE)
	        edges[*n_inp].mask_type = TERM_MASK;
	    else
	        edges[*n_inp].mask_type = BOLEAN;
	    strcpy (edges[*n_inp].mask_name, lfname);
	    newmask_flag = TRUE;
	}
	else {
	    fprintf (stderr, "\nERROR in NBOOL:Too many layers asked for\n");
	    die (1);
	}
    }

    edges[*n_inp].fp = dmOpenStream (mod_key, lfname, "r");

    if (dmGetDesignData (edges[*n_inp].fp, GEO_VLNLAY) > 0) {
	edges[*n_inp].pos = gvlnlay.x;
	edges[*n_inp].yb = gvlnlay.yb;
	edges[*n_inp].yt = gvlnlay.yt;
	edges[*n_inp].edge_type = gvlnlay.occ_type;
	edges[*n_inp].chk_type = gvlnlay.chk_type;
	edge_heap[nf] = *n_inp;
	(*n_inp)++;
	nf++;
	mk_heap ();
    }
    else {
	dmCloseStream (edges[*n_inp].fp, COMPLETE);
	(*n_inp)++;
    }

    return (newmask_flag);
}

static int pres_in_heap (int n_inp, char *mask_name)
/* n_inp - nbr of edges in heap */
/* mask_name - name of the mask */
{
/* This procedure returns TRUE if a mask is already
 * present in the edge_heap; FALSE otherwise.
 */
    register int i;
    char vln_name[DM_MAXLAY + 7]; /* maskname_vln */

    for (i = 0; i < n_inp; ++i) {
	sprintf (vln_name, "%s_vln", mask_name);
	if (strcmp (edges[i].mask_name, vln_name) == 0)
	    return (TRUE);
    }
    return (FALSE);
}
