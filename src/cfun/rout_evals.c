/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
 *	P.E. Menchen
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

#include "src/cfun/func_parsedefs.h"

static int check_ind (FTERM *ptr, char ind0[], char ind1[], int indvar0, int indvar1);

/* Function delay_eval()
** this function evaluates the call of the function delay()
** by checking the arguments; if the arguments are legal,
** this function prints the last part of the delay call
** with converted arguments.
*/
void delay_eval (char term[], char ind0[], char ind1[], int indvar0, int indvar1)
{
    int     found = 0;
    register FTERM * sc_ptr;

    for (sc_ptr = ftrm_list; sc_ptr != NULL && found == 0;
					sc_ptr = sc_ptr -> next) {
	if (strcmp (sc_ptr -> name, term) == 0) {
	    if (sc_ptr -> type != OutpTerm && sc_ptr -> type != InoTerm) {
		fprintf (yyout,
    "\nterminal '%s' in routine delay is not output or inout\n", term);
		die (8, term, "");
	    }

	    found = check_ind (sc_ptr, ind0, ind1, indvar0, indvar1);

	    if (found == 1) {
		pr_rtn_arg ("delay", sc_ptr, ind0, ind1, indvar0, indvar1, 0);
	    }
	    else {
		fprintf (yyout,
		    "\nterminal '%s', illegal call of routine delay\n", term);
		die (9, term, "delay");
	    }
	}
    }
    if (found == 0) {
	fprintf (yyout,
	    "\ncannot find terminal '%s' in routine delay\n", term);
	die (10, term, "delay");
    }
}

/* Function capadd_eval()
** this function evaluates the call of the function capadd()
** by checking the arguments; if the arguments are legal,
** this function prints the last part of the capadd call
** with converted arguments.
*/
void capadd_eval (char term[], char ind0[], char ind1[], int indvar0, int indvar1)
{
    int     found = 0;
    int     min_cnt = 0;
    register FTERM * sc_ptr;

    for (sc_ptr = ftrm_list; sc_ptr != NULL && found == 0;
					sc_ptr = sc_ptr -> next) {
	if (strcmp (sc_ptr -> name, term) == 0) {
	    switch (sc_ptr -> type) {
		case InpTerm:
		    min_cnt = os_cnt;
		    found = 1;
		    break;
		case InrTerm:
		    min_cnt = os_cnt + is_cnt;
		    found = 1;
		    break;
		default:
		    fprintf (yyout,
    "\nterminal '%s' in routine cap_add is not input or inread\n", term);
		    die (11, term, "");
	    }

	    found = check_ind (sc_ptr, ind0, ind1, indvar0, indvar1);

	    if (found == 1) {
		fprintf (yyout, ", %d ", sc_ptr -> type);
		pr_rtn_arg ("cap_add", sc_ptr, ind0, ind1,
				    indvar0, indvar1, min_cnt);
	    }
	    else {
		fprintf (yyout,
    "\nterminal '%s', illegal call of routine cap_add\n", term);
		die (9, term, "cap_add");
	    }
	}
    }
    if (found == 0) {
	fprintf (yyout,
	    "\ncannot find terminal '%s' in routine cap_add\n", term);
	die (10, term, "cap_add");
    }
}

/* Function getcap_eval()
** this function evaluates the call of the functions
** dyncap_val or statcap_val by checking the arguments;
** if the arguments are legal,
** this function prints the the last part of dyncap_val
** or stat_cap call with converted arguments.
*/
void getcap_eval (char rout[], int vicin, int min_max, char term[], char ind0[], char ind1[], int indvar0, int indvar1)
{
    int     found = 0;
    int     min_cnt = 0;
    register FTERM * sc_ptr;

    for (sc_ptr = ftrm_list; sc_ptr != NULL && found == 0;
					sc_ptr = sc_ptr -> next) {
	if (strcmp (sc_ptr -> name, term) == 0) {
	    switch (sc_ptr -> type) {
		case InpTerm:
		    min_cnt = os_cnt;
		    found = 1;
		    break;
		case InrTerm:
		    min_cnt = os_cnt + is_cnt;
		    found = 1;
		    break;
		case OutpTerm:
		case InoTerm:
		    min_cnt = 0;
		    found = 1;
		    break;
		default:
		    fprintf (yyout,
			"\nterminal '%s' in routine %s_val is not a terminal\n",
			term, rout);
		    die (12, term, rout);
	    }

	    found = check_ind (sc_ptr, ind0, ind1, indvar0, indvar1);

	    if (found == 1) {
		fprintf (yyout, "%s_val ( %d, %d, %d ",
			rout, sc_ptr -> type, vicin, min_max);
		pr_rtn_arg (rout, sc_ptr, ind0, ind1,
				    indvar0, indvar1, min_cnt);
	    }
	    else {
		fprintf (yyout,
		    "\nterminal '%s', illegal call of routine %s_val\n",
		    term, rout);
		die (13, term, rout);
	    }
	}
    }
    if (found == 0) {
	fprintf (yyout,
	    "\ncannot find terminal '%s' in routine %s\n", term, rout);
	die (10, term, rout);
    }
}

/* Function check_ind()
** this function checks whether the index of a certain terminal
** that is an argument of a routine, is legal if the index
** is legal, the function returns a 1 and else a 0.
*/
static int check_ind (FTERM *ptr, char ind0[], char ind1[], int indvar0, int indvar1)
{
    if (ptr -> ind[0] == 0) {
	if (indvar0 != FUNNOIND || indvar1 != FUNNOIND)
	    return (0);
    }
    else
	if (ptr -> ind[1] == 0) {
	    if ((indvar0 == FUNNOIND || indvar1 != FUNNOIND) ||
		    (indvar0 == FUNINT &&
			atoi (ind0) > (ptr -> ind[0] - 1)))
		return (0);
	}
	else {
	    if ((indvar0 == FUNNOIND || indvar1 == FUNNOIND) ||
		    (indvar0 == FUNINT &&
			atoi (ind0) > (ptr -> ind[0] - 1)) ||
		    (indvar1 == FUNINT &&
			atoi (ind1) > (ptr -> ind[1] - 1)))
		return (0);
	}
    return (1);
}

/* Function pr_rtn_arg()
** this function is used for the printing of the last
** part of a function call: it converts the arguments.
** the argument is a terminal or an element of a terminal
** array; it is converted to an integer with a value
** that indicates the place of this 'terminal' in the
** appropriate FI, FR or FO array used in SLS.
*/
void pr_rtn_arg (char rout[], FTERM *ptr, char ind0[], char ind1[], int indvar0, int indvar1, int min_cnt)
{
    register FTERM * sc_ptr;
    int     nullsfior = 0;
    int     h;

    for (sc_ptr = ftrm_list; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	if (((sc_ptr -> type == ptr -> type) ||
		    (sc_ptr -> type == InoTerm && ptr -> type == OutpTerm) ||
		    (sc_ptr -> type == OutpTerm && ptr -> type == InoTerm)) &&
		sc_ptr -> arrind < ptr -> arrind && sc_ptr -> ind[0] != 0) {
	    if (sc_ptr -> ind[1] != 0)
		nullsfior += sc_ptr -> ind[0];
	    else
		if (sc_ptr -> ind[1] == 0)
		    nullsfior += 1;
		else {
		    fprintf (yyout,
			"\ninternal error in evaluating routine %s\n", rout);
		    die (14, rout, "");
		}
	}
    }

    if (ptr -> ind[1] == 0)
	h = 1;
    else
	h = 0;

    if (indvar0 != FUNVAR) {
	if (indvar1 != FUNVAR)
	    fprintf (yyout, ", %d )",
		    ptr -> arrind + (ptr -> ind[1] + h) * atoi (ind0) +
		    atoi (ind1) - min_cnt - nullsfior);
	else
	    fprintf (yyout, ", %d + %s )",
		    ptr -> arrind +
		    (ptr -> ind[1] + h) * atoi (ind0) - min_cnt - nullsfior,
		    ind1);
    }
    else
	if (indvar1 != FUNVAR)
	    fprintf (yyout, ", %d + %d*%s )",
		    ptr -> arrind + atoi (ind1) - min_cnt - nullsfior,
		    (ptr -> ind[1] + h), ind0);
	else
	    fprintf (yyout, ", %d + %d*%s + %s )",
		    ptr -> arrind - min_cnt - nullsfior, (ptr -> ind[1] + h),
		    ind0, ind1);
}
