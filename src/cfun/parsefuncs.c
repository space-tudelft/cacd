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

int     os_cnt = 0;
int     is_cnt = 0;
int     rs_cnt = 0;
int     ss_cnt = 0;
int     ps_cnt = 0;

static int print_decl_line (FTERM *ptr, int cnt);

/* Function fill_list()
** adds a new element to the list indicated by ptr
** and if necessary creates this list.
*/
FTERM *fill_list (FTERM *ptr, char name[], int ind0, int ind1, int arrind, int type)
{
    FTERM * t_ptr;
    MALLOC (t_ptr, FTERM);

    strcpy (t_ptr -> name, name);
    t_ptr -> ind[0] = ind0;
    t_ptr -> ind[1] = ind1;
    t_ptr -> arrind = arrind;
    t_ptr -> type = type;
    t_ptr -> next = NULL;

    if (ptr == NULL) {
	ptr = t_ptr;
    }
    else
	app_list (ptr, t_ptr);
    return (ptr);
}

/* Function app_list()
** appends the list that pointer ptr2 points at,
** to the end of the list indicated by pointer ptr1.
*/
void app_list (FTERM *ptr1, FTERM *ptr2)
{
    register FTERM * t_ptr;

    t_ptr = ptr1;
    while (t_ptr -> next != NULL) {
	t_ptr = t_ptr -> next;
    }
    t_ptr -> next = ptr2;
}

/* Function add_list()
** changes the types of elements of
** the list ptr2 into 'type' and appends this
** list to the end of list ptr1.
*/
FTERM *add_list (FTERM *ptr1, FTERM *ptr2, int type)
{
    register FTERM * t_ptr;

    t_ptr = ptr2;
    while (t_ptr != NULL) {
	t_ptr -> type = type;
	t_ptr = t_ptr -> next;
    }

    if (ptr1 == NULL) {
	ptr1 = ptr2;
    }
    else
	app_list (ptr1, ptr2);
    return (ptr1);
}

/* Function check_term()
** checks whether the index of a terminal in a
** declaration list is an integer and whether
** the terminal name has not been used before.
*/
void check_term (FTERM *ptr, char s[], int indv0, int indv1)
{
    register FTERM * sc_ptr;

    if (indv0 == FUNVAR || indv1 == FUNVAR) {
	die (6, s, ""); /* index must be an integer */
    }

    for (sc_ptr = ptr; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	if (strcmp (sc_ptr -> name, s) == 0) {
	    die (7, s, ""); /* name already used */
	}
    }

    for (sc_ptr = ftrm_list; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	if (strcmp (sc_ptr -> name, s) == 0) {
	    die (7, s, ""); /* name already used */
	}
    }
}

/* Function print_func_head()
** prints the head of an I or E function and
** builds the array 'inout' when s == 'I'.
*/
void print_func_head (char s)
{
    fprintf (yyout, "\n");
    fprintf (yyout, "void %c%s (char *inout)\n", s, Func_name);
    fprintf (yyout, "{\n");

    if (s == 'I') build_array ();
    print_decl ();
    lineno (yylineno);
}

/* Function build_array()
** builds the form of the array 'inout' as is done
** in sls_exp (getdev()) so declarations can be
** printed correctly.
*/
void build_array ()
{
    register FTERM * sc_ptr;
    int     size;

    for (sc_ptr = ftrm_list; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	size = 0;
	switch (sc_ptr -> type) {
	    case OutpTerm:
	    case InoTerm:
		if (sc_ptr -> ind[0] == 0) {
		    os_cnt++;
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			os_cnt += sc_ptr -> ind[0] + 1;
		    }
		    else {
			os_cnt += sc_ptr -> ind[0] *
			    (sc_ptr -> ind[1] + 1);
			ps_cnt += sc_ptr -> ind[0] * SIZE_PTR_INT;
		    }
		break;
	    case InpTerm:
		if (sc_ptr -> ind[0] == 0) {
		    is_cnt++;
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			is_cnt += sc_ptr -> ind[0] + 1;
		    }
		    else {
			is_cnt += sc_ptr -> ind[0] *
			    (sc_ptr -> ind[1] + 1);
			ps_cnt += sc_ptr -> ind[0] * SIZE_PTR_INT;
		    }
		break;
	    case InrTerm:
		if (sc_ptr -> ind[0] == 0) {
		    rs_cnt++;
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			rs_cnt += sc_ptr -> ind[0] + 1;
		    }
		    else {
			rs_cnt += sc_ptr -> ind[0] *
			    (sc_ptr -> ind[1] + 1);
			ps_cnt += sc_ptr -> ind[0] * SIZE_PTR_INT;
		    }
		break;
	    case StateChar:
		if (sc_ptr -> ind[0] == 0) {
		    ss_cnt += 1;
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			ss_cnt += sc_ptr -> ind[0] + 1;
		    }
		    else {
			ss_cnt += sc_ptr -> ind[0] *
			    (sc_ptr -> ind[1] + 1);
			ps_cnt += sc_ptr -> ind[0] * SIZE_PTR_INT;
		    }
		break;
	    case StateInt:
	    case StateFloat:
	    case StateDouble:
	        if (sc_ptr -> type == StateInt) {
		    size = sizeof (int);
		}
		else if (sc_ptr -> type == StateFloat) {
		    size = sizeof (float);
		}
		else if (sc_ptr -> type == StateDouble) {
		    size = sizeof (double);
		}
		if (sc_ptr -> ind[0] == 0) {
		    ss_cnt += size * 3 - 1;
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			ss_cnt += size * (2 + sc_ptr -> ind[0]) - 1;
		    }
		    else {
			ss_cnt += size * sc_ptr -> ind[0] *
			    sc_ptr -> ind[1] + 2 * size - 1;
			ps_cnt += sc_ptr -> ind[0] * SIZE_PTR_INT;
		    }
		break;
	}
    }
    if (ps_cnt > 0)
	ps_cnt += 3 * SIZE_PTR_INT;
}

/* Function print_decl()
** prints the declarations in the head of the
** I and E function so the terminals and state
** variables are represented on the correct place
** in the array 'inout'.
*/
void print_decl ()
{
    register FTERM * sc_ptr;
    int     osx;
    int     isx;
    int     rsx;
    int     ssx;
    int     psx;

    osx = 0;
    isx = osx + os_cnt;
    rsx = isx + is_cnt;
    psx = rsx + rs_cnt;
    ssx = psx + ps_cnt;

    if (ps_cnt > 0) {
	psx += SIZE_PTR_INT;
	if (psx % SIZE_PTR_INT != 0)
	    psx += SIZE_PTR_INT - psx % SIZE_PTR_INT;
    /* to be machine independent */
	psx += SIZE_PTR_INT;
    }

    for (sc_ptr = ftrm_list; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	switch (sc_ptr -> type) {
	    case OutpTerm:
	    case InoTerm:
		sc_ptr -> arrind = osx;
		if (sc_ptr -> ind[1] == 0) {
		    osx += print_decl_line (sc_ptr, osx);
		}
		else {
		    osx += sc_ptr -> ind[0] * (sc_ptr -> ind[1] + 1);
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	    case InpTerm:
		sc_ptr -> arrind = isx;
		if (sc_ptr -> ind[1] == 0) {
		    isx += print_decl_line (sc_ptr, isx);
		}
		else {
		    isx += sc_ptr -> ind[0] * (sc_ptr -> ind[1] + 1);
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	    case InrTerm:
		sc_ptr -> arrind = rsx;
		if (sc_ptr -> ind[1] == 0) {
		    rsx += print_decl_line (sc_ptr, rsx);
		}
		else {
		    rsx += sc_ptr -> ind[0] * (sc_ptr -> ind[1] + 1);
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	    case StateChar:
		sc_ptr -> arrind = ssx;
		if (sc_ptr -> ind[1] == 0) {
		    ssx += print_decl_line (sc_ptr, ssx);
		}
		else {
		    ssx += sc_ptr -> ind[0] * (sc_ptr -> ind[1] + 1);
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	    case StateInt:
		ssx += sizeof (int);
		if (ssx % sizeof (int) != 0)
		    ssx += sizeof (int) - ssx % sizeof (int);
		sc_ptr -> arrind = ssx;
		if (sc_ptr -> ind[0] == 0) {
		    print_decl_line (sc_ptr, ssx);
		    ssx += sizeof (int);
		}
		else
		    if (sc_ptr -> ind[1] == 0) {
			print_decl_line (sc_ptr, ssx);
			ssx += sc_ptr -> ind[0] * sizeof (int);
		    }
		    else {
			ssx += sizeof (int) * sc_ptr -> ind[0] *
			    sc_ptr -> ind[1] + 2 * sizeof (int) - 1;
			psx += print_decl_line (sc_ptr, psx);
		    }
		break;
	    case StateFloat:
		ssx += sizeof (float);
		if (ssx % sizeof (float) != 0)
		    ssx += sizeof (float) - ssx % sizeof (float);
		sc_ptr -> arrind = ssx;
		if (sc_ptr -> ind[1] == 0) {
		    print_decl_line (sc_ptr, ssx);
		    ssx += sizeof (float) * 3 - 1;
		}
		else {
		    ssx += sizeof (float) * sc_ptr -> ind[0] *
			sc_ptr -> ind[1] + 2 * sizeof (float) - 1;
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	    case StateDouble:
		ssx += sizeof (double);
		if (ssx % sizeof (double) != 0)
		    ssx += sizeof (double) - ssx % sizeof (double);
		sc_ptr -> arrind = ssx;
		if (sc_ptr -> ind[1] == 0) {
		    print_decl_line (sc_ptr, ssx);
		    ssx += sizeof (double) * 3 - 1;
		}
		else {
		    ssx += sizeof (double) * sc_ptr -> ind[0] *
			sc_ptr -> ind[1] + 2 * sizeof (double) - 1;
		    psx += print_decl_line (sc_ptr, psx);
		}
		break;
	}
    }
}

/* Function print_func_foot()
** prints the assignments in the foot of the
** I and E function so the changes of values
** of the zero-dimensional terminals are also made
** in the array 'inout'.
*/
void print_func_foot ()
{
    register FTERM *sc_ptr;
    char *decl_type;

    for (sc_ptr = ftrm_list; sc_ptr != NULL; sc_ptr = sc_ptr -> next) {
	if (sc_ptr -> ind[0] == 0) {
	    switch (sc_ptr -> type) {
		case StateInt:
		    decl_type = "int";
		    break;
		case StateFloat:
		    decl_type = "float";
		    break;
		case StateDouble:
		    decl_type = "double";
		    break;
		default:
		    decl_type = "char";
		    break;
	    }
	    if (sc_ptr -> arrind > 0)
		fprintf (yyout, "*(%s *)(inout+%d) = %s;\n",
		    decl_type, sc_ptr -> arrind, sc_ptr -> name);
	    else
		fprintf (yyout, "*(%s *)(inout) = %s;\n",
		    decl_type, sc_ptr -> name);
	}
    }
    if (adm_bsalloc_flag == 1)
	fprintf (yyout, "adm_bsalloc (0, 'r');\n");
}

/* Function print_decl_line()
** prints the declaration lines that define the
** connection between the terminals and state
** variables and the array 'inout'.
*/
static int print_decl_line (FTERM *ptr, int cnt)
/* cnt is the index in the array 'inout' */
{
    char cntbuf[16];
    char *decl_type;

    switch (ptr -> type) {
	case StateInt:
	    decl_type = "int";
	    break;
	case StateFloat:
	    decl_type = "float";
	    break;
	case StateDouble:
	    decl_type = "double";
	    break;
	default:
	    decl_type = "char";
	    break;
    }

    if (cnt > 0)
	sprintf (cntbuf, "+%d", cnt);
    else
	*cntbuf = 0;
    if (ptr -> ind[0] == 0) {
	fprintf (yyout, "%s %s = *(%s *)(inout%s);\n", decl_type,
		ptr -> name, decl_type, cntbuf);
	return (1);
    }
    if (ptr -> ind[1] == 0) {
	fprintf (yyout, "%s *%s = (%s *)(inout%s);\n", decl_type,
		ptr -> name, decl_type, cntbuf);
	return (ptr -> ind[0] + 1);
    }
    else {
	fprintf (yyout, "%s **%s = (%s **)(inout%s);\n", decl_type,
		ptr -> name, decl_type, cntbuf);
	return (ptr -> ind[0] * SIZE_PTR_INT);
    }
}

char *strsav (char *str)
{
    char *p;
    if (!(p = malloc ((unsigned) (strlen (str) + 1)))) die (1, "", "");
    strcpy (p, str);
    return (p);
}
