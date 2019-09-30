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

static char *rec_hiername (int nx, int mctx, char *totstr);

char hierstr[DM_MAXPATHLEN];  /* memory space for the name contructed */

char *hiername (int nx) /* gives the hierarchical name of node nx */
{
    if (N[nx].ntx < 0) {
	strcpy (hierstr, "<no name>");
    }
    else {
	(void) rec_hiername (nx, -1, hierstr);
    }
    return (hierstr);
}

static char *rec_hiername (int nx, int mctx, char *totstr)
/* recursive hier. name constructor
 * nx   = index of the node
 * mctx = model call in which this node must hierarchically be searched
 *        nx < 0 or mctx < 0
 *        when nx   < 0 the modelcall mctx must be constructed
 *        when mctx < 0 the node name nx must be constructed
 * totstr = the total name will be constructed in totstr
 * rec_hiername returns as value a pointer to the character
 * after the last character of the name constructed so far
 */
{
    int xtx;
    int ntx;
    int xx;
    int offset;
    int rest;
    int cnt;
    int up;
    int low;
    int middle;
    int nextmctx;
    int dindex;
    char index[16];  /* contains an integer */
    int i;
    char * endstr;
    char * str;

    if (nx >= 0) { /* find node name */
	ntx = N[nx].ntx;
	if (NT[ntx].xtx >= 0) {
	    xx = NT[ntx].x;
	    while (XX[xx++] != nx);
	    offset = xx - 1 - NT[ntx].x;
	}
	else
	    offset = 0;

	/* find the modelcall in which the node is actually called */
	low = 0;
	up = CTT_cnt - 1;
	while (low < up - 1) {
	    middle = (up + low)/2;
	    if ( CTT[middle].ceiling > nx )
		up = middle;
	    else
		low = middle;
	}
	if (CTT[low].ceiling > nx) {   /* it could be possible low is still 0 */
	    /* low = -1;  (is not neccessary) */
	    up = 0;
	}
	i = up;
	nextmctx = CTT[i].mctx;
    }
    else {
	if (mctx < 0) {
	    return (totstr);  /* both nx and mctx are < 0, so ready ! */
	}
	else {
				      /* find modelcall name */
	    ntx = MCT[mctx].ntx;
	    offset = mctx - NT[ ntx ].x;

	    /* find modelcall in which this modelcall is called */
	    nextmctx = MCT[mctx].parent;
	}
    }

    /* firstly, construct name of the higher hierarchy */

    str = rec_hiername (-1, nextmctx, totstr);

    /* then, add the name of this hierarchy */

    xtx = NT[ ntx ].xtx;

    strcpy (str, ST + NT[ntx].name);

    endstr = str + strlen (str);

    if (xtx >= 0) { /* construct the indices with XT and offset information */

        *endstr++ = '[';

        rest = 1;
        i = xtx + 1;
        for (cnt = 1; cnt <= XT[xtx]; cnt++) {
	    rest = rest * (XT[i+1] - XT[i] + 1);
	    i = i + 2;
        }

        i = xtx + 1;
        for (cnt = 1; cnt <= XT[xtx]; cnt++) {
	    rest = rest / (XT[i+1] - XT[i] + 1);
	    dindex = offset / rest;
	    offset = offset - dindex * rest;
	    sprintf (index, "%d", XT[i] + dindex);
	    sprintf (endstr, "%s", index);
            endstr = endstr + strlen (index);
	    if (cnt < XT[xtx])
                *endstr++ = ',';
	    i = i + 2;
        }

        *endstr++ = ']';
    }

    if (nx >= 0)
        *endstr++ = '\0';
    else
	*endstr++ = '.';

    return (endstr);
}
