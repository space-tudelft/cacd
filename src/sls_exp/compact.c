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

void copystruct (char *struc1, char *struc2, int size)
{
    while (size-- > 0) *struc1++ = *struc2++;
}

/* remove the nodes which are redirected and the DS and C */
/* elements which are 0 */
void compact ()
{
    int * Nmoved;
    char * Neliminated;
    int * DSmoved;
    int * Cmoved;
    int cnt;
    int freecnt;
    int nbr;
    int curr_ceiling;
    int prev_new_ceiling;
    int x;
    NAMETABLE * nt;
    NODE * n;
    TRANSISTOR * t;
    INTERCAP * i;
    CONTEXTTABLE * ctt;

    PALLOC (Nmoved, N_cnt, int);
    PALLOC (Neliminated, N_cnt, char);

    freecnt = 0;   /* we are going to make a new array */
    for (cnt = 0; cnt < N_cnt; cnt++) {
	if (N[cnt].redirect) {
	    x = N[cnt].cx;
	    while (N[ x ].redirect) {
		x = N[ x ].cx;
	    }
	    Nmoved[cnt] = x;         /* old index of node directed */
	    Neliminated[cnt] = 'y';
	}
	else {
	    Nmoved[cnt] = freecnt;   /* new index */
	    Neliminated[cnt] = 'n';
	    freecnt++;
	}
    }

    freecnt = 0;
    for (cnt = 0; cnt < N_cnt; cnt++) {
	if (N[cnt].redirect) {
	    Nmoved[cnt] = Nmoved[ Nmoved[cnt] ];
	}                            /* new index of node redirected */
	else {
	    if (cnt != freecnt)
		copystruct ((char *)(N + freecnt), (char *)(N + cnt), sizeof (NODE));
	    freecnt++;
	}
    }
    N_cnt = freecnt;

    for (cnt = 0, nt = NT; cnt < NT_cnt; cnt++, nt++) {
	if ((nt -> sort == Node || nt -> sort == Node_t) && nt -> xtx < 0) {
	    nt -> x = Nmoved[ nt -> x ];
        }
    }

    curr_ceiling = 0;
    prev_new_ceiling = 0;
    freecnt = 0;
    for (cnt = 0, ctt = CTT; cnt < CTT_cnt; cnt++, ctt++) {
	nbr = 0;
	while (curr_ceiling < ctt -> ceiling) {
	    curr_ceiling++;
	    if (Neliminated[ curr_ceiling - 1 ] == 'n')
		nbr++;
	}
	if (nbr > 0) {
	    /* else there are no nodes any longer in the modelcall */
	    ctt -> ceiling = prev_new_ceiling = prev_new_ceiling + nbr;
	    if (freecnt != cnt)
	        copystruct ((char *)(CTT + freecnt), (char *)ctt, sizeof (CONTEXTTABLE));
	    freecnt++;
	}
    }
    CTT_cnt = freecnt;

    for (cnt = 0; cnt < XX_cnt; cnt++) {
	XX[cnt] = Nmoved[ XX[cnt] ];
    }

    for (cnt = 0, t = T; cnt < T_cnt; cnt++, t++) {
	if (t -> gate >= 0) t -> gate = Nmoved[ t -> gate ];
	t -> drain = Nmoved[ t -> drain ];
	t -> source = Nmoved[ t -> source ];
    }

    for (cnt = 0, i = I; cnt < I_cnt; cnt++, i++) {
	i -> con1 = Nmoved[ i -> con1 ];
	i -> con2 = Nmoved[ i -> con2 ];
    }

    for (cnt = 0; cnt < FI_cnt; cnt++) {
	nbr = FI[cnt];
	while (nbr-- > 0) {
	    cnt++;
	    FI[cnt] = Nmoved[ FI[cnt] ];
	}
    }

    for (cnt = 0; cnt < FR_cnt; cnt++) {
	nbr = FR[cnt];
	while (nbr-- > 0) {
	    cnt++;
	    FR[cnt] = Nmoved[ FR[cnt] ];
	}
    }

    for (cnt = 0; cnt < FO_cnt; cnt++) {
	nbr = FO[cnt].x;
	while (nbr-- > 0) {
	    cnt++;
	    FO[cnt].x = Nmoved[ FO[cnt].x ];
	}
    }

    CFREE (Nmoved);
    CFREE (Neliminated);

    PALLOC (DSmoved, DS_cnt, int);
    PALLOC (Cmoved, C_cnt, int);

    /* now, remove the DS and C elements which are 0 */

    freecnt = 0;
    for (cnt = 0; cnt < DS_cnt; cnt++) {
	if ((nbr = DS[cnt]) > 0) {
	    DSmoved[ cnt ] = freecnt;
	    if (cnt != freecnt)
		copystruct ((char *)(DS + freecnt), (char *)(DS + cnt), (nbr + 1) * sizeof (int));
	    freecnt += nbr + 1;
	    cnt += nbr;
	}
    }
    DS_cnt = freecnt;

    freecnt = 0;
    for (cnt = 0; cnt < C_cnt; cnt++) {
	if ((nbr = C[cnt].c) > 0) {
	    Cmoved[ cnt ] = freecnt;
	    if (cnt != freecnt)
		copystruct ((char *)(C + freecnt), (char *)(C + cnt), (nbr + 1) * sizeof (CONTROL));
	    freecnt += nbr + 1;
	    cnt += nbr;
	}
    }
    C_cnt = freecnt;

    for (cnt = 0, n = N; cnt < N_cnt; cnt++, n++) {
	if (n -> dsx > 0) n -> dsx = DSmoved[ n -> dsx ];
	if (n -> cx > 0) n -> cx = Cmoved[ n -> cx ];
    }

    CFREE (DSmoved);
    CFREE (Cmoved);
}
