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

extern int newc  (int nbr);
extern int newds (int nbr);

/* link nodes with their dsx and cx to the devices which refer to them */
void linkntw (char *model)
{
    int cnt;
    int DS_i;
    int C_i;
    int cx;
    int dsx;
    int nbr;
    int ficnt;
    int frcnt;
    int focnt;
    int fix;
    int frx;
    int fox;
    unsigned fatalerror = FALSE;
    NODE * n;
    TRANSISTOR * t;
    FUNCTION * f;
    INTERCAP * i;

    for (cnt = mcs_T_cnt, t = &T[cnt]; cnt < T_cnt; cnt++, t++) {
	if (t -> type != Res) {
	    if (t -> gate < 0)
	        t -> gate = newnode ();
	    while (N[t -> gate].redirect)
	        t -> gate = N[t -> gate].cx;
	    N[ t -> gate ].cx++;
	}
	if (t -> drain < 0)
	    t -> drain = newnode ();
	while (N[t -> drain].redirect)
	    t -> drain = N[t -> drain].cx;
	N[ t -> drain ].dsx++;
	if (t -> source < 0)
	    t -> source = newnode ();
	while (N[t -> source].redirect)
	    t -> source = N[t -> source].cx;
	N[ t -> source ].dsx++;
    }

    for (cnt = mcs_F_cnt, f = &F[cnt]; cnt < F_cnt; cnt++, f++) {
	fix = f -> fix;
        if (fix >= 0) {
	    for (ficnt = FI[fix]; ficnt > 0; ficnt--) {
	        fix++;
	        if ( FI[fix] < 0)
		    FI[fix] = newnode ();
	        while ( N[ FI[fix] ].redirect )
		    FI[fix] = N[ FI[fix] ].cx;
                N[ FI[fix] ].cx++;
	    }
        }

	frx = f -> frx;
        if (frx >= 0) {
	    for (frcnt = FR[frx]; frcnt > 0; frcnt--) {
	        frx++;
	        if ( FR[frx] < 0)
		    FR[frx] = newnode ();
	        while ( N[ FR[frx] ].redirect )
		    FR[frx] = N[ FR[frx] ].cx;
	    }
        }

	fox = f -> fox;
        if (fox >= 0) {
	    for (focnt = FO[fox].x; focnt > 0; focnt--) {
	        fox++;
	        if ( FO[fox].x < 0) {
		    FO[fox].x = newnode ();
	        }
	        while ( N[ FO[fox].x ].redirect )
	            FO[fox].x = N[ FO[fox].x ].cx;
	        N[ FO[fox].x ].funcoutp = TRUE;
	    }
        }
    }

    for (cnt = mcs_I_cnt, i = &I[cnt]; cnt < I_cnt; cnt++, i++) {
	if (i -> con1 < 0)
	    i -> con1 = newnode ();
	while ( N[ i -> con1 ].redirect )
	    i -> con1 = N[ i -> con1 ].cx;
	if (capcoupling)
            N[ i -> con1 ].cx++;
        N[ i -> con1 ].statcap += i -> cap;
	if (i -> con2 < 0)
	    i -> con2 = newnode ();
	while ( N[ i -> con2 ].redirect )
	    i -> con2 = N[ i -> con2 ].cx;
	if (capcoupling)
            N[ i -> con2 ].cx++;
        N[ i -> con2 ].statcap += i -> cap;
    }

    if (!capcoupling) {
        I_cnt = 0;       /* the actual freeing will be done in main () */
    }

    /* allocate DS and C space for the nodes */

    DS_i = DS_cnt;
    C_i = C_cnt;
    for (cnt = 0, n = N; cnt < N_cnt; cnt++, n++) {
	if ( ! n -> linked ) {
            if ( n -> redirect ) {
	        n -> dsx = -1;
	        /* cx is refering to the node to which is redirected */
	    }
	    else {
	        if (n -> dsx > 0) {
	            nbr = n -> dsx;
	            newds (nbr + 1);
	            n -> dsx = DS_i;
	            DS[ DS_i ] = 0;
	            DS_i = DS_i + nbr + 1;
	        }
	        else
	            n -> dsx = -1;

	        if (n -> cx > 0) {
	            nbr = n -> cx;
	            newc (nbr + 1);
	            n -> cx = C_i;
	            C[ C_i ].c = 0;
	            C_i = C_i + nbr + 1;
	        }
	        else
	            n -> cx = -1;
	    }
	}
    }

    /* give DS and C their values by passing the devices T, F and I */

    for (cnt = 0, t = &T[cnt]; cnt < T_cnt; cnt++, t++) {

	/* It is neccessary to pass all transistors because a transistor in */
	/* a lower level may be referring to a node in the current level    */
	/* which had just been joined to another node                       */

	while (N[t -> drain].redirect)
	    t -> drain = N[t -> drain].cx;
	if (!N[ t -> drain ].linked) {
	    dsx = N[ t -> drain ].dsx;
	    DS[ dsx ]++;
	    DS[ dsx + DS[ dsx ] ] = cnt;
	}

	while (N[t -> source].redirect)
	    t -> source = N[t -> source].cx;
	if (!N[ t -> source ].linked) {
	    dsx = N[ t -> source ].dsx;
	    DS[ dsx ]++;
	    DS[ dsx + DS[ dsx ] ] = cnt;
	}

        if (t -> gate >= 0) {
	    while (N[t -> gate].redirect)
	        t -> gate = N[t -> gate].cx;
	    if (!N[ t -> gate ].linked) {
	        cx = N[ t -> gate ].cx;
	        C[ cx ].c++;
	        C[ cx + C[ cx ].c ].c = cnt;
	        C[ cx + C[ cx ].c ].sort = Transistor;
	    }
	}
    }

    for (cnt = 0, f = &F[cnt]; cnt < F_cnt; cnt++, f++) {
	fix = f -> fix;
        if (fix >= 0) {
	    for (ficnt = FI[fix]; ficnt > 0; ficnt--) {
	        fix++;
	        while (N[FI[fix]].redirect)
	            FI[fix] = N[FI[fix]].cx;
	        if (!N[ FI[fix] ].linked) {
	            cx = N[ FI[fix] ].cx;
	            C[ cx ].c++;
	            C[ cx + C[ cx ].c ].c = cnt;
	            C[ cx + C[ cx ].c ].sort = Functional;
	        }
            }
        }

	frx = f -> frx;
        if (frx >= 0) {
	    for (frcnt = FR[frx]; frcnt > 0; frcnt--) {
	        frx++;
	        while (N[FR[frx]].redirect)
	            FR[frx] = N[FR[frx]].cx;
            }
        }

	fox = f -> fox;
        if (fox >= 0) {
	    for (focnt = FO[fox].x; focnt > 0; focnt--) {
	        fox++;
	        while (N[FO[fox].x].redirect)
	            FO[fox].x = N[FO[fox].x].cx;
	        if (cnt < mcs_F_cnt && !N[ FO[fox].x ].linked ) {
		        N[FO[fox].x].funcoutp = TRUE;
	        }
            }
	}
    }

    if (fatalerror) {
	die (1);
    }

    if (capcoupling) {

        for (cnt = 0, i = &I[cnt]; cnt < I_cnt; cnt++, i++) {

	    while (N[i -> con1].redirect)
	        i -> con1 = N[i -> con1].cx;
	    if (!N[ i -> con1 ].linked) {
	        cx = N[ i -> con1 ].cx;
	        C[cx].c++;
	        C[ cx + C[ cx ].c ].c = cnt;
	        C[ cx + C[ cx ].c ].sort = Intercap;
	    }

	    while (N[i -> con2].redirect)
	        i -> con2 = N[i -> con2].cx;
	    if (!N[ i -> con2 ].linked) {
	        cx = N[ i -> con2 ].cx;
	        C[cx].c++;
	        C[ cx + C[ cx ].c ].c = cnt;
	        C[ cx + C[ cx ].c ].sort = Intercap;
	    }
        }
    }

    for (cnt = 0, n = N; cnt < N_cnt; cnt++, n++) {
	n -> linked = TRUE;
    }
}
