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

static void deb_FV ()
{
    int i;

    fprintf (debug, "FV[ ] **********************************************\n");
    fprintf (debug, "     i       name    type    ind\n");
    i = 0;
    while (i < FV_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%10s ", FV[i].name);
	switch (FV[i].type) {
	    case INPUT_T :
		fprintf (debug, "INPUT_T ");
		break;
	    case INPUT_R :
		fprintf (debug, "INPUT_R ");
		break;
	    case OUTPUT :
		fprintf (debug, "OUTPUT  ");
		break;
	    case INOUT :
		fprintf (debug, "INOUT   ");
		break;
	    case CHAR :
		fprintf (debug, "CHAR    ");
		break;
	    case INTEGER :
		fprintf (debug, "INTEGER ");
		break;
	    case FLOAT :
		fprintf (debug, "FLOAT   ");
		break;
	    case DOUBLE :
		fprintf (debug, "DOUBLE  ");
		break;
            default :
                fprintf (debug, "%2d      ", FV[i].type);
                break;
	}
	fprintf (debug, "%3d %3d", FV[i].ind[0], FV[i].ind[1]);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_FD ()
{
    int i;

    fprintf (debug, "FD[ ] **********************************************\n");
    fprintf (debug, "     i       name  fvx fvx_ct\n");
    i = 0;
    while (i < FD_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%10s ", FD[i].name);
	fprintf (debug, "%4d ", FD[i].fvx);
	fprintf (debug, "%4d ", FD[i].fvx_cnt);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_NT ()
{
    int i;

    fprintf (debug, "NT[ ] **********************************************\n");
    fprintf (debug, "     i   sort      x    xtx  'name'\n");
    i = 0;
    while (i < NT_cnt) {
	fprintf (debug, "%6d ", i);
	switch (NT[i].sort) {
	    case Node :
		fprintf (debug, "Node   ");
		break;
	    case Node_t :
		fprintf (debug, "Node_t ");
		break;
	    case Modelcall :
		fprintf (debug, "Modelc ");
		break;
	    case Transistor :
		fprintf (debug, "Trans  ");
		break;
	    case Functional :
		fprintf (debug, "Funct  ");
		break;
	    case Intercap :
		fprintf (debug, "Interc ");
		break;
	}
	fprintf (debug, "%6d ", NT[i].x);
	fprintf (debug, "%6d ", NT[i].xtx);
	fprintf (debug, " %s ", ST + NT[i].name);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_MT ()
{
    int i;

    fprintf (debug, "MT[ ] **********************************************\n");
    fprintf (debug, "     i nt_cnt  name\n");
    i = 0;
    while (i < MT_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", MT[i].nt_cnt);
	fprintf (debug, " %s ", MT[i].name);
	fprintf (debug, "\n");
        i++;
    }
    fprintf (debug, "\n");
}

static void deb_CTT ()
{
    int i;

    fprintf (debug, "CTT[ ] **********************************************\n");
    fprintf (debug, "     i ceiling  mctx\n");
    i = 0;
    while (i < CTT_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", CTT[i].ceiling);
	fprintf (debug, "%6d ", CTT[i].mctx);
	fprintf (debug, "\n");
        i++;
    }
    fprintf (debug, "\n");
}

static void deb_MCT ()
{
    int i;

    fprintf (debug, "MCT[ ] **********************************************\n");
    fprintf (debug, "     i parent    ntx   n_ntx   mtx\n");
    i = 0;
    while (i < MCT_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", MCT[i].parent);
	fprintf (debug, "%6d ", MCT[i].ntx);
	fprintf (debug, "%6d ", MCT[i].n_ntx);
	fprintf (debug, "%6d ", MCT[i].mtx);
	fprintf (debug, "\n");
        i++;
    }
    fprintf (debug, "\n");
}

static void deb_XT ()
{
    int i, n;

    fprintf (debug, "XT[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < XT_cnt) {
	fprintf (debug, "%6d ", i);
	n = XT[i++];
	fprintf (debug, "%6d\n", n);
	while (i < XT_cnt && n > 0) {
	    fprintf (debug, "       %6d ", XT[i++]);
	    fprintf (debug, "%6d\n", XT[i++]);
	    n = n - 1;
	}
    }
    fprintf (debug, "\n");
}

static void deb_XX ()
{
    int i;

    fprintf (debug, "XX[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < XX_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", XX[i++]);
	while (i % 10 != 0 && i < XX_cnt) {
	    fprintf (debug, "%6d ", XX[i++]);
	}
	fprintf (debug, "\n");
    }
    fprintf (debug, "\n");
}

static void deb_N (int flag)
{
    int i;

    fprintf (debug, "N[ ] ***********************************************\n");
    fprintf (debug,
    "     i      n      d      c          s f r l\n");
    fprintf (debug,
    "            t      s      x          c u e i\n");
    fprintf (debug,
    "            x      x                 a n d n\n");
    fprintf (debug,
    "                                     p c i k\n");

    i = 0;
    while (i < N_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", N[i].ntx);
	fprintf (debug, "%6d ", N[i].dsx);
	fprintf (debug, "%6d ", N[i].cx);
	fprintf (debug, "%9.3e ", N[i].statcap);
	fprintf (debug, "%1d ", N[i].funcoutp);
	fprintf (debug, "%1d ", N[i].redirect);
	fprintf (debug, "%1d ", N[i].linked);
	if (flag == 1) fprintf (debug, " %s ", hiername (i));
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_DS ()
{
    int i, n, m;

    fprintf (debug, "DS[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < DS_cnt) {
	fprintf (debug, "%6d ", i);
	n = DS[i++];
	fprintf (debug, "%6d ", n);
	m = 0;
	if (n == 0)
	    fprintf (debug, "\n");
	while (i < DS_cnt && m < n) {
	    fprintf (debug, "%6d ", DS[i++]);
	    m++;
	    while (i < DS_cnt && m < n && m % 5 > 0) {
		fprintf (debug, "%6d ", DS[i++]);
		m++;
	    }
	    fprintf (debug, "\n");
	    if (m < n)
		fprintf (debug, "              ");
	}
    }
    fprintf (debug, "\n");
}

static char write_c_sort (int sort)
{
    char val;

    switch (sort) {
	case 0 :
	    val = '0';
	    break;
	case Node :
	case Node_t :
	    val = 'N';
	    break;
	case Modelcall :
	    val = 'M';
	    break;
	case Transistor :
	    val = 'T';
	    break;
	case Functional :
	    val = 'F';
	    break;
	case Intercap :
	    val = 'I';
	    break;
	default :
	    val = '?';
	    break;
    }
    return (val);
}

static void deb_C ()
{
    int i, n, m;

    fprintf (debug, "C[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < C_cnt) {
	fprintf (debug, "%6d ", i);
	n = C[i].c;
	fprintf (debug, "%6d (%c)", C[i].c, write_c_sort (C[i].sort));
	i++;
	m = 0;
	if (n == 0)
	    fprintf (debug, "\n");
	while (i < C_cnt && m < n) {
	    fprintf (debug, "%6d (%c)", C[i].c, write_c_sort (C[i].sort));
	    i++;
	    m++;
	    while (i < C_cnt && m < n && m % 5 > 0) {
	        fprintf (debug, "%6d (%c)", C[i].c, write_c_sort (C[i].sort));
		i++;
		m++;
	    }
	    fprintf (debug, "\n");
	    if (m < n)
		fprintf (debug, "                 ");
	}
    }
    fprintf (debug, "\n");
}

static void deb_T ()
{
    int i;

    fprintf (debug, "T[ ] ***********************************************\n");
    fprintf (debug, "     i   type   flag   gate source  drain ");
    fprintf (debug, "    width    length\n");
    i = 0;
    while (i < T_cnt) {
	fprintf (debug, "%6d ", i);
	switch (T[i].type) {
	    case Nenh :
		fprintf (debug, "  nenh ");
		break;
	    case Penh :
		fprintf (debug, "  penh ");
		break;
	    case Depl :
		fprintf (debug, "  depl ");
		break;
	    case Res :
		fprintf (debug, "   res ");
		break;
	    default :
	        fprintf (debug, "%6d ", T[i].type);
		break;
	}
	fprintf (debug, "%6d ", T[i].flag);
	fprintf (debug, "%6d ", T[i].gate);
	fprintf (debug, "%6d ", T[i].source);
	fprintf (debug, "%6d ", T[i].drain);
	fprintf (debug, "%9.3e ", T[i].width);
	fprintf (debug, "%9.3e ", T[i].length);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_F ()
{
    int i;

    fprintf (debug, "F[ ] ***********************************************\n");
    fprintf (debug, "     i   type    fix    frx    fox    fsx  evalflag \n");
    i = 0;
    while (i < F_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", F[i].type);
	fprintf (debug, "%6d ", F[i].fix);
	fprintf (debug, "%6d ", F[i].frx);
	fprintf (debug, "%6d ", F[i].fox);
	fprintf (debug, "%6d ", F[i].fsx);
	fprintf (debug, "%6d ", F[i].evalflag);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_I ()
{
    int i;

    fprintf (debug, "I[ ] ***********************************************\n");
    fprintf (debug, "     i   con1   con2    cap\n");
    i = 0;
    while (i < I_cnt) {
	fprintf (debug, "%6d ", i);
	fprintf (debug, "%6d ", I[i].con1);
	fprintf (debug, "%6d ", I[i].con2);
	fprintf (debug, "%9.3e ", I[i].cap);
	fprintf (debug, "\n");
	i++;
    }
    fprintf (debug, "\n");
}

static void deb_FI ()
{
    int i, n, m;

    fprintf (debug, "FI[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < FI_cnt) {
	fprintf (debug, "%6d ", i);
	n = FI[i++];
	fprintf (debug, "%6d ", n);
	m = 0;
	if (n == 0)
	    fprintf (debug, "\n");
	while (i < FI_cnt && m < n) {
	    fprintf (debug, "%6d ", FI[i++]);
	    m++;
	    while (i < FI_cnt && m < n && m % 5 > 0) {
		fprintf (debug, "%6d ", FI[i++]);
		m++;
	    }
	    fprintf (debug, "\n");
	    if (m < n)
		fprintf (debug, "              ");
	}
    }
    fprintf (debug, "\n");
}

static void deb_FR ()
{
    int i, n, m;

    fprintf (debug, "FR[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < FR_cnt) {
	fprintf (debug, "%6d ", i);
	n = FR[i++];
	fprintf (debug, "%6d ", n);
	m = 0;
	if (n == 0)
	    fprintf (debug, "\n");
	while (i < FR_cnt && m < n) {
	    fprintf (debug, "%6d ", FR[i++]);
	    m++;
	    while (i < FR_cnt && m < n && m % 5 > 0) {
		fprintf (debug, "%6d ", FR[i++]);
		m++;
	    }
	    fprintf (debug, "\n");
	    if (m < n)
		fprintf (debug, "              ");
	}
    }
    fprintf (debug, "\n");
}

static void deb_FO ()
{
    int i, n, m;

    fprintf (debug, "FO[ ] **********************************************\n");
    fprintf (debug, "     i\n");
    i = 0;
    while (i < FO_cnt) {
	fprintf (debug, "%6d ", i);
	n = FO[i++].x;
	fprintf (debug, "%6d ", n);
	m = 0;
	if (n == 0)
	    fprintf (debug, "\n");
	while (i < FO_cnt && m < n) {
	    fprintf (debug, "%6d ", FO[i++].x);
	    m++;
	    while (i < FO_cnt && m < n && m % 5 > 0) {
		fprintf (debug, "%6d ", FO[i++].x);
		m++;
	    }
	    fprintf (debug, "\n");
	    if (m < n)
		fprintf (debug, "              ");
	}
    }
    fprintf (debug, "\n");
}

static void deb_FS ()
{
    int i, x, ints, nextspace;

    fprintf (debug, "FS[ ] **********************************************\n");
    fprintf (debug, "     i");
    i = 0;
    x = 100;
    ints = 0;
    while (i < FS_cnt) {
        if (ints > 0)
            nextspace = 12;
        else
            nextspace = 2;

        if (x + nextspace >= 80) {
	    fprintf (debug, "\n%6d ", i);
            x = 8;
        }

        if (ints > 0) {
            fprintf (debug, "%11d ", *(int *)(FS + i));
            ints--;
            i += SIZE_PTR_INT;
        }
        else {
            if (FS[i] == '\0') {
	        fprintf (debug, "@ ");
                i++;
            }
            else {
	        fprintf (debug, "%c ", FS[i]);
                i++;
            }
            if (FS[i] == '*') {
                fprintf (debug, "* ");
                i += SIZE_PTR_INT;
                if (i % SIZE_PTR_INT != 0)
                    i += SIZE_PTR_INT - (i % SIZE_PTR_INT);
                ints = *(int *)(FS + i) + 1;
            }
        }

        x += nextspace;
    }
    fprintf (debug, "\n\n");
}

void deb_all () /* writes out the datastrutures */
{
    deb_FV ();
    deb_FD ();
    deb_NT ();
    deb_MT ();
    deb_CTT ();
    deb_MCT ();
    deb_XT ();
    deb_XX ();
    deb_N (1);
    deb_DS ();
    deb_C ();
    deb_T ();
    deb_F ();
    deb_I ();
    deb_FI ();
    deb_FR ();
    deb_FO ();
    deb_FS ();
}

void deb_mem ()
{
    int size;

    fprintf (debug, "\n");

#ifdef SLS

    size = sizeof (char);
    fprintf (debug, "ST : %7d x %2d = %7d\n", ST_cnt, size, ST_cnt * size);

    size = sizeof (NAMETABLE);
    fprintf (debug, "NT : %7d x %2d = %7d\n", NT_cnt, size, NT_cnt * size);

    size = sizeof (MODELTABLE);
    fprintf (debug, "MT : %7d x %2d = %7d\n", MT_cnt, size, MT_cnt * size);

    size = sizeof (CONTEXTTABLE);
    fprintf (debug, "CTT: %7d x %2d = %7d\n", CTT_cnt, size, CTT_cnt * size);

    size = sizeof (MODELCALLTABLE);
    fprintf (debug, "MCT: %7d x %2d = %7d\n", MCT_cnt, size, MCT_cnt * size);

    size = sizeof (int);
    fprintf (debug, "XT : %7d x %2d = %7d\n", XT_cnt, size, XT_cnt * size);

    size = sizeof (int);
    fprintf (debug, "XX : %7d x %2d = %7d\n", XX_cnt, size, XX_cnt * size);

    size = sizeof (NODE);
    fprintf (debug, "N  : %7d x %2d = %7d\n", N_cnt, size, N_cnt * size);

    size = sizeof (int);
    fprintf (debug, "DS : %7d x %2d = %7d\n", DS_cnt, size, DS_cnt * size);

    size = sizeof (CONTROL);
    fprintf (debug, "C  : %7d x %2d = %7d\n", C_cnt, size, C_cnt * size);

    size = sizeof (TRANSISTOR);
    fprintf (debug, "T  : %7d x %2d = %7d\n", T_cnt, size, T_cnt * size);

    size = sizeof (FUNCTION);
    fprintf (debug, "F  : %7d x %2d = %7d\n", F_cnt, size, F_cnt * size);

    size = sizeof (INTERCAP);
    fprintf (debug, "I  : %7d x %2d = %7d\n", I_cnt, size, I_cnt * size);

    size = sizeof (int);
    fprintf (debug, "FI : %7d x %2d = %7d\n", FI_cnt, size, FI_cnt * size);

    size = sizeof (int);
    fprintf (debug, "FR : %7d x %2d = %7d\n", FR_cnt, size, FR_cnt * size);

    size = sizeof (FUNCOUT);
    fprintf (debug, "FO : %7d x %2d = %7d\n", FO_cnt, size, FO_cnt * size);

    size = sizeof (char);
    fprintf (debug, "FS : %7d x %2d = %7d\n", FS_cnt, size, FS_cnt * size);
    size = sizeof (char);

#else

    size = sizeof (char);
    fprintf (debug, "ST : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	ST_size, size, ST_size * size, ST_cnt, size, ST_cnt * size, 100 * ST_cnt / ST_size);

    size = sizeof (NAMETABLE);
    fprintf (debug, "NT : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	NT_size, size, NT_size * size, NT_cnt, size, NT_cnt * size, 100 * NT_cnt / NT_size);

    size = sizeof (MODELTABLE);
    fprintf (debug, "MT : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	MT_size, size, MT_size * size, MT_cnt, size, MT_cnt * size, 100 * MT_cnt / MT_size);

    size = sizeof (CONTEXTTABLE);
    fprintf (debug, "CTT: %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	CTT_size, size, CTT_size * size, CTT_cnt, size, CTT_cnt * size, 100 * CTT_cnt / CTT_size);

    size = sizeof (MODELCALLTABLE);
    fprintf (debug, "MCT: %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	MCT_size, size, MCT_size * size, MCT_cnt, size, MCT_cnt * size, 100 * MCT_cnt / MCT_size);

    size = sizeof (int);
    fprintf (debug, "XT : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	XT_size, size, XT_size * size, XT_cnt, size, XT_cnt * size, 100 * XT_cnt / XT_size);

    size = sizeof (int);
    fprintf (debug, "XX : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	XX_size, size, XX_size * size, XX_cnt, size, XX_cnt * size, 100 * XX_cnt / XX_size);

    size = sizeof (NODE);
    fprintf (debug, "N  : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	N_size, size, N_size * size, N_cnt, size, N_cnt * size, 100 * N_cnt / N_size);

    size = sizeof (int);
    fprintf (debug, "DS : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	DS_size, size, DS_size * size, DS_cnt, size, DS_cnt * size, 100 * DS_cnt / DS_size);

    size = sizeof (CONTROL);
    fprintf (debug, "C  : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	C_size, size, C_size * size, C_cnt, size, C_cnt * size, 100 * C_cnt / C_size);

    size = sizeof (TRANSISTOR);
    fprintf (debug, "T  : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	T_size, size, T_size * size, T_cnt, size, T_cnt * size, 100 * T_cnt / T_size);

    size = sizeof (FUNCTION);
    fprintf (debug, "F  : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	F_size, size, F_size * size, F_cnt, size, F_cnt * size, 100 * F_cnt / F_size);

    size = sizeof (INTERCAP);
    fprintf (debug, "I  : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	I_size, size, I_size * size, I_cnt, size, I_cnt * size, 100 * I_cnt / I_size);

    size = sizeof (int);
    fprintf (debug, "FI : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	FI_size, size, FI_size * size, FI_cnt, size, FI_cnt * size, 100 * FI_cnt / FI_size);

    size = sizeof (int);
    fprintf (debug, "FR : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	FR_size, size, FR_size * size, FR_cnt, size, FR_cnt * size, 100 * FR_cnt / FR_size);

    size = sizeof (FUNCOUT);
    fprintf (debug, "FO : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	FO_size, size, FO_size * size, FO_cnt, size, FO_cnt * size, 100 * FO_cnt / FO_size);

    size = sizeof (char);
    fprintf (debug, "FS : %7d x %2d = %7d  used: %7d x %2d = %7d   (%3d%%)\n",
	FS_size, size, FS_size * size, FS_cnt, size, FS_cnt * size, 100 * FS_cnt / FS_size);

#endif

    fprintf (debug, "\n");
}

void opendebug ()
{
    OPENW (debug, "deb");
    setbuf (debug, (char *) NULL);
}
