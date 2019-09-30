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

extern void copystruct (char *struc1, char *struc2, int size);

/* remove all names from the nametable */
/* that are no terminal names */
void rmnoterm ()
{
    int cnt;
    int freecnt;
    int * NTmoved;
    int * STmoved;
    int * XTmoved;
    int * XXmoved;
    int * MTnt_cnt;
    char * st;
    int new_nt_cnt;
    int new_n_ntx;
    int i;
    int xtxes;
    int nbr;
    int range;

    PALLOC (NTmoved, NT_cnt, int);
    PALLOC (XTmoved, XT_cnt, int);
    PALLOC (XXmoved, XX_cnt, int);

    for (cnt = 0; cnt < XT_cnt; cnt++) {
        XTmoved[cnt] = 0;
    }

    for (cnt = 0; cnt < XX_cnt; cnt++) {
        XXmoved[cnt] = 0;
    }

    freecnt = 0;
    for (cnt = 0; cnt < NT_cnt; cnt++) {

	if (NT[cnt].sort != Node_t) {

            NTmoved[cnt] = -1;

            st = ST + NT[cnt].name;
            while (*st != '\0' && *st != '\t') {
                *st++ = '\t';
            }
            *st = '\t';

            if (NT[cnt].xtx >= 0) {
                nbr = 1;
                i = NT[cnt].xtx;
                for (xtxes = XT[i], XTmoved[i] = -1, i++; xtxes > 0; xtxes--) {
                    if (XT[i] > XT[i+1])
                        range = XT[i] - XT[i+1] + 1;
                    else
                        range = XT[i+1] - XT[i] + 1;
                    nbr = nbr * range;
                    XTmoved[i] = -1;
                    XTmoved[i+1] = -1;
                    i = i + 2;
                }
                if (NT[cnt].sort == Node) {
                    for (i = NT[cnt].x; nbr-- > 0; i++) {
                        XXmoved[i] = -1;
                    }
                }
            }

            NT[cnt].name = -1;
        }
        else {
            NTmoved[cnt] = freecnt++;
        }
    }

    freecnt = 0;
    for (cnt = 0; cnt < XT_cnt; cnt++) {
        if (XTmoved[cnt] == 0) {
            XTmoved[cnt] = freecnt;
            XT[freecnt++] = XT[cnt];
        }
        else {
            XTmoved[cnt] = -1;
        }
    }
    XT_cnt = freecnt;

    freecnt = 0;
    for (cnt = 0; cnt < XX_cnt; cnt++) {
        if (XXmoved[cnt] == 0) {
            XXmoved[cnt] = freecnt;
            XX[freecnt++] = XX[cnt];
        }
        else {
            XXmoved[cnt] = -1;
        }
    }
    XX_cnt = freecnt;

    for (cnt = 0; cnt < N_cnt; cnt++) {
        if (N[cnt].ntx >= 0)
            N[cnt].ntx = NTmoved[ N[cnt].ntx ];
    }

    PALLOC (MTnt_cnt, MT_cnt, int);

    for (cnt = 0; cnt < MCT_cnt; cnt++) {
        if (MCT[cnt].ntx >= 0) {
            MCT[cnt].ntx = NTmoved[ MCT[cnt].ntx ];
            new_nt_cnt = 0;
            new_n_ntx = -1;
            i = MCT[cnt].n_ntx;
            while (i - MCT[cnt].n_ntx < MT[ MCT[cnt].mtx ].nt_cnt) {
                if (NTmoved[i] >= 0) {
                    if (new_n_ntx < 0)
                        new_n_ntx = i;
                    new_nt_cnt++;
                }
                i++;
            }
            MCT[cnt].n_ntx = new_n_ntx;
            MTnt_cnt[ MCT[cnt].mtx ] = new_nt_cnt;
        }
    }

    new_nt_cnt = 0;
    i = NT_cnt - MT[ MT_cnt - 1 ].nt_cnt;
    while (i < NT_cnt) {
        if (NTmoved[i] >= 0) {
            new_nt_cnt++;
        }
        i++;
    }
    MTnt_cnt[ MT_cnt - 1 ] = new_nt_cnt;

    for (cnt = 0; cnt < MT_cnt; cnt++) {
        MT[cnt].nt_cnt = MTnt_cnt[cnt];
    }

    CFREE (MTnt_cnt);

    PALLOC (STmoved, ST_cnt, int);

    freecnt = 0;
    for (cnt = 0; cnt < ST_cnt; cnt++) {
        if (ST[cnt] == '\t') {
            STmoved[cnt] = -1;
        }
        else {
            ST[freecnt] = ST[cnt];
            STmoved[cnt] = freecnt++;
        }
    }
    ST_cnt = freecnt;

    freecnt = 0;
    for (cnt = 0; cnt < NT_cnt; cnt++) {

        if (NTmoved[cnt] >= 0) {
            if ( NT[cnt].name >= 0 ) {
                NT[cnt].name = STmoved[ NT[cnt].name ];
            }
            if ( NT[cnt].xtx >= 0) {
                NT[cnt].xtx = XTmoved[ NT[cnt].xtx ];
                if ( NT[cnt].x >= 0
                     && (NT[cnt].sort == Node_t || NT[cnt].sort == Node))
                     NT[cnt].x = XXmoved[ NT[cnt].x ];
            }
            if (cnt != freecnt)
                copystruct ((char *)(NT + freecnt), (char *)(NT + cnt), sizeof (NAMETABLE));
            freecnt++;
        }

    }
    NT_cnt = freecnt;

    CFREE (XTmoved);
    CFREE (XXmoved);
    CFREE (NTmoved);
    CFREE (STmoved);
}
