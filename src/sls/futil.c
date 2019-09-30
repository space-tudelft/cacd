/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#include "src/sls/extern.h"

static void cap_vclose  (NODE *n, int min_max);
static void cap_vsearch (NODE *n, int min_max);

#define	OutpTerm	1   /* these defines must be equal to the */
#define	InoTerm		2   /* the defines used in cfun      */
#define	InpTerm		3
#define	InrTerm		4

#define	MIN_CAP		5   /* these defines must be equal to the */
#define	MAX_CAP		6   /* the defines used in cfun      */


float cap_tot;              /* used to determine the total        */
			    /* capacitance of a vicinity          */

/* delay() : the rise or fall or 'both' delay time of the output  */
/*         : terminal of the function in which the routine is     */
/*         : used is changed; the new value is val;               */
/*         : the terminal is the 'outnmb-th' terminal of          */
/*         : the function block                                   */

void delay (char type, float val, int outnmb)
{
    int fox;

    fox = currf -> fox + 1 + outnmb;

    switch (type) {
        case 'r' :
            FO[fox].trise = val;
            break;
        case 'f' :
            FO[fox].tfall = val;
            break;
        case 'b' :
        default :
            FO[fox].trise = val;
            FO[fox].tfall = val;
            break;
    }
}

/* cap_add() : to the capacitance of the node to which a terminal of  */
/*           : the function, in which the routine is called, is       */
/*           : connected is increased with value val;                 */
/*           : the type of the terminal can be input or inread        */
/*           : or inout                                               */

void cap_add (float val, int type, int innmb)
{
    int firx;

    if ( type == InpTerm ) {
    	firx = currf -> fix + 1 + innmb;
	N[ FI[firx] ].statcap +=  val;
	N[ FI[firx] ].dyncap +=  val;
    }
    else if ( type == InrTerm ) {
    	firx = currf -> frx + 1 + innmb;
	N[ FR[firx] ].statcap +=  val;
	N[ FR[firx] ].dyncap +=  val;
    }
    else if ( type == InoTerm ) {
    	firx = currf -> frx + 1 + innmb;
	N[ FO[firx].x ].statcap +=  val;
	N[ FO[firx].x ].dyncap +=  val;
    }
}

/* cap_val() : the capacitance of the node to which a terminal        */
/*           : of the function, in which the routine is called, is    */
/*           : connected is returned;                                 */
/*           : the type of the terminal can be input or inread        */
/*           : or inout or output                                     */
/*           : when vicin is 1 the routine cap_vsearch is called to   */
/*           : find the total capacitance of the node including the   */
/*           : capacitances of all the nodes that are connected to    */
/*           : the given node through resistors or transistors;       */

float cap_val (int type, int vicin, int min_max, int term_nmb)
{
    int foirx;
    cap_tot = 0.0;

    if ( type == InpTerm ) {
    	foirx = currf -> fix + 1 + term_nmb;
	if ( vicin) {
	    cap_vsearch( &N[ FI[foirx] ], min_max);
	    cap_vclose ( &N[ FI[foirx] ], min_max);
	    return( cap_tot);
	}
	else
	    return ( N[ FI[foirx] ].statcap );
    }
    else if ( type == InrTerm ) {
    	foirx = currf -> frx + 1 + term_nmb;
	if ( vicin) {
	    cap_vsearch( &N[ FR[foirx] ], min_max);
	    cap_vclose ( &N[ FR[foirx] ], min_max);
	    return( cap_tot);
	}
	else
	    return ( N[ FR[foirx] ].statcap );
    }
    else if ( type == OutpTerm || type == InoTerm ) {
    	foirx = currf -> fox + 1 + term_nmb;
	if ( vicin) {
	    cap_vsearch( &N[ FO[foirx].x ], min_max);
	    cap_vclose ( &N[ FO[foirx].x ], min_max);
	    return( cap_tot);
	}
	else
	    return ( N[ FO[foirx].x ].statcap );
    }
    else
	return(-1.0);
}

static int capwarning = 0;

float statcap_val (int type, int vicin, int min_max, int term_nmb)
{
    if (capwarning == 0) {
	fprintf (stderr, "Use 'cap_val' instead of 'statcap_val' in your function blocks\n");
	capwarning = 1;
    }
    return (cap_val (type, vicin, min_max, term_nmb));
}

float dyncap_val (int type, int vicin, int min_max, int term_nmb)
{
    if (capwarning == 0) {
	fprintf (stderr, "Use 'cap_val' instead of 'dyncap_val' in your function blocks\n");
	capwarning = 1;
    }
    return (cap_val (type, vicin, min_max, term_nmb));
}

/*  cap_vsearch() : the vicinity of the node n is searched recursively     */
/*                : and the capacitances of the found nodes are added      */
/*                : to the variable cap_tot; when min_max is MIN_CAP       */
/*                : the transistor between two nodes must be closed to let */
/*                : the node on the other side of the transistor be a      */
/*                : member of the vicinity; when min_max is MAX_CAP        */
/*                : the transistor can also be undefined                   */

static void cap_vsearch (NODE *n, int min_max)
{
    int cnt, index;
    TRANSISTOR * t;   /* transistor connected to n by drain or source */
    NODE * con;       /* node connected to n by t */

    n -> flag = TRUE;
    cap_tot += n->statcap;
    index = n -> dsx;
    for (cnt = DS[index]; cnt > 0; cnt--) {
	index++;
	t = &T[DS[index]];
	if ( (min_max==MIN_CAP && t -> state == Closed) ||
	     (min_max==MAX_CAP && t -> state != Open  ) ) {
	    if (&N[t -> drain] == n)
		con = &N[t -> source];
	    else
		con = &N[t -> drain];
	    if (!con -> flag)
		cap_vsearch (con,min_max);
	}
    }
}

/*  cap_vclose() : the vicinity of the node n is searched recursively     */
/*               : and the flags that were set in cap_vsearch are unset   */

static void cap_vclose (NODE *n, int min_max)
{
    int cnt, index;
    TRANSISTOR * t;   /* transistor connected to n by drain or source */
    NODE * con;       /* node connected to n by t */

    n -> flag = FALSE;
    index = n -> dsx;
    for (cnt = DS[index]; cnt > 0; cnt--) {
	index++;
	t = &T[DS[index]];
	if ( (min_max==MIN_CAP && t -> state == Closed) ||
	     (min_max==MAX_CAP && t -> state != Open  ) ) {
	    if (&N[t -> drain] == n)
		con = &N[t -> source];
	    else
		con = &N[t -> drain];
	    if (con -> flag)
		cap_vclose (con,min_max);
	}
    }
}

typedef struct BS_point_adm {
    char                  * BS_pointer;
    struct BS_point_adm   * next;
} BS_POINT_LIST;

BS_POINT_LIST  *list = NULL;

char *adm_bsalloc (int p, char mode)
{
    BS_POINT_LIST  *element;
    char * s = NULL;

    if (mode == 'p') {
        if (!(element = (BS_POINT_LIST *) malloc (sizeof(BS_POINT_LIST)))) {
            fprintf (stderr, "ERROR : adm_bsalloc(): Cannot allocate storage\n");
	    exit(1);
        }
        if (!(s = (char *) malloc (p + 1))) {
            fprintf (stderr, "ERROR : adm_bsalloc(): Cannot allocate storage\n");
	    exit(1);
        }
	element->BS_pointer = s;
	element->next       = list;
	list = element;
    }
    else if (mode == 'r') {
	while (list) {
	    element = list;
	    list = list->next;
	    free ( element->BS_pointer );
	    free ( element );
	}
    }
    else {
        fprintf (stderr, "%c is a illegal argument in adm_bsalloc()\n", mode);
	exit(1);
    }
    return(s);
}
