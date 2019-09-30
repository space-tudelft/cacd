/*
 * ISC License
 *
 * Copyright (C) 1982-2018 by
 *	T.G.R. van Leuken
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

extern DM_PROJECT *dmproject;

static void add_mterm (unsigned int mask, unsigned int not_mask, struct form *frm_pntr);
static void update_mask (unsigned int *mask, unsigned int *not_mask, int *not_flag, char *lay_name);

void mk_formstruct (int file_flag)
{
/* This function makes the structures containing the
 * bolean file_names and masks to be able to determine
 * if a certain combination of masks belongs to a bolean
 * combination. Furthermore buffers to contain edges for
 * the bolean output files are set_up.
 * Globally the structure consists of a linked list of
 * forms each form containing the name of the
 * bolean_file to make and a pointer to a linked list
 * of min_terms of the formula.
 * Each min_term contains two masks: one of the layers
 * that must be present in a mask_combination to be part
 * of that min_term and one mask containing the layers
 * that may not be present in the mask_combination.
 */
#define BUFSIZE 256

    char    lay_name[MAXLINE];	/* name of input_layer	 */
    char    input[BUFSIZE];	/* formula string 	 */
    char    c;
    int     form_nbr;		/* bool_form_number	 */
    int     i;			/* pointer in input	 */
    int     j;			/* pointer in lay_name	 */
    int     not_flag;		/* mask/not_mask flag	 */
    unsigned int    mask;	/* layers to be present	 */
    unsigned int    not_mask;	/* layers to be not pres. */
    FILE * f_nboold;		/* ptr to form inp_file  */
    FILE * f_out;		/* ptr to temp outp_file */
    struct form *frm_pntr;	/* current form pointer	 */
    char   *bool_file;		/* name+path bool_file	 */

/* open the file NBOOLDATA and read its contents line	 */
/* by line: input then contains the formula and form_nbr */
/* the number of the formula. This number is used to	 */
/* form the file_name of the bolean output_file		 */
/* belonging to the formula.				 */

    mask = 0;
    not_mask = 0;
    fp_head = NULL;
    not_flag = FALSE;
    f_nboold = NULL; /* init to suppres compiler warning */

    if (file_flag == ON) {
	if (access ("./booldata", 0) == 0) {
	    OPEN (f_nboold, "booldata", "r");
	}
	else {
	    fprintf (stderr, "\nbooldata not in the WD; i'll take the standard one\n");
	    file_flag = OFF;
	}
    }
    if (file_flag == OFF) {
	bool_file = dmGetMetaDesignData (PROCPATH, dmproject, "booldata");
	OPEN (f_nboold, bool_file, "r");
    }

    while (1) {
	while ((c = getc (f_nboold)) == '\n');
	if (c == '#') {
	/* remove comment string */
	    fgets (input, BUFSIZE - 1, f_nboold);
	    continue;
	}
	else {
	    ungetc (c, f_nboold);
	    break;
	}
    }

    fscanf (f_nboold, "%[^:]%*[^\n]%*c", input);
    ini_heap (input);

    while (1) {
	while ((c = getc (f_nboold)) == '\n');
	if (c == '#') {
	/* remove comment string */
	    fgets (input, BUFSIZE - 1, f_nboold);
	    continue;
	}
	else {
	    ungetc (c, f_nboold);
	}
	if (fscanf (f_nboold, "%[^:]%*s%d%*[^\n]%*c", input, &form_nbr) == EOF)
	    break;
	if (strcmp (input, "\n") == 0)
	    continue;

	sprintf (fr_name, TEMP_ONE, form_nbr, pid);
	OPEN (f_out, fr_name, "w");
	CLOSE (f_out);

/* allocate space for the form_data and its buffers and	 */
/* put the file_name in this structure. Mark the buffers */
/* empty by setting curr_place = -1 in the structure.	 */

	ALLOC (frm_pntr, form);
	frm_pntr -> f_nbr = form_nbr;
	frm_pntr -> curr_place = -1;
	for (i = 0; i < BUFLEN; i++) {
	    ALLOC (b_pntr, buff);
	    frm_pntr -> b_pntr[i] = b_pntr;
	}
	frm_pntr -> next = fp_head;
	fp_head = frm_pntr;

/* decode the formula in te string input. This string	 */
/* may contain names of layers and the logical		 */
/* operators: | (or) & (and) and ! (negation).		 */

	i = 0;
	j = 0;
	while (input[i] != '\0') {
	    switch (input[i]) {
		case '\040':
		case '\t':
		case '\n':
		    i++;
		    break;
		case '!':
		    i++;
		    not_flag = TRUE;
		    break;
		case '&':
		    i++;
		    lay_name[j] = '\0';
		    update_mask (&mask, &not_mask, &not_flag, lay_name);
		    j = 0;
		    break;
		case '|':
		    i++;
		    lay_name[j] = '\0';
		    update_mask (&mask, &not_mask, &not_flag, lay_name);
		    add_mterm (mask, not_mask, frm_pntr);
		    j = 0;
		    mask = 0;
		    not_mask = 0;
		    break;
		default:
		    lay_name[j++] = input[i++];
		    break;
	    }
	}
	if (j != 0) {
	    lay_name[j] = '\0';
	    update_mask (&mask, &not_mask, &not_flag, lay_name);
	    add_mterm (mask, not_mask, frm_pntr);
	    i = 0;
	    j = 0;
	    mask = 0;
	    not_mask = 0;
	}
    }
    CLOSE (f_nboold);
}

static void update_mask (unsigned int *mask, unsigned int *not_mask, int *not_flag, char *lay_name)
/* mask     - layers to be present */
/* not_mask - layers to be not present */
/* not_flag - mask/not_mask flag */
/* lay_name - name of input_layer */
{
/* This function updates mask/not_mask with the layer found in the formula.
 */
    int k;
    for (k = 0; k < nomasks; k++) {
	if (strcmp (lay_name, edges[k].mask_name) == 0) {
	    if (*not_flag == TRUE) {
		*not_mask = *not_mask | edges[k].mask;
		*not_flag = FALSE;
	    }
	    else
		*mask = *mask | edges[k].mask;
	    break;
	}
    }
    if (k >= nomasks) {
	fprintf (stderr, "unknown masktype");
	die (1);
    }
}

static void add_mterm (unsigned int mask, unsigned int not_mask, struct form *frm_pntr)
/* mask     - layers to be present */
/* not_mask - layers to be not present */
/* frm_pntr - current form_pointer */
{
/* This function allocates space for a min_term found
 * in a formula and fills in the mask and not_mask
 * variables. The structure is added to the form_struct
 * and the vulnerability mask of the formula is updated.
 */
    struct min_term *mt_pntr; /* current min_term ptr */

    ALLOC (mt_pntr, min_term);
    mt_pntr -> mask = mask;
    mt_pntr -> not_mask = not_mask;
    mt_pntr -> next = frm_pntr -> mt_pntr;
    frm_pntr -> mt_pntr = mt_pntr;
    frm_pntr -> vuln_mask = frm_pntr -> vuln_mask | mask | not_mask;
}
