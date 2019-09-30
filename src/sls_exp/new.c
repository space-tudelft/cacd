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

static void *enlargeArray (void *A, int *A_size, int sizeofA, float m);
extern int  hcreate (int size);
extern void hdestroy (void);
extern ENTRY *hsearch (ENTRY item, ACTION action);

int newstring (char *s)
{
    int i;
    ENTRY item;
    int slen = strlen (s) + 1;

    while (ST_cnt + slen > ST_size) {
	ST = enlargeArray (ST, &ST_size, sizeof (char), 1.5);
	hdestroy ();
	hcreate (NT_size - mcs_NT_cnt);
	for (i = mcs_NT_cnt; i < NT_cnt; i++) {
            item.key = ST + NT[ i ].name;
            item.data = (char *)&NT[ i ];
            hsearch (item, ENTER);
	}
    }
    strcpy (ST + ST_cnt, s);
    ST_cnt += slen;
    return (ST_cnt - slen);
}

int newname (char *s)
{
    int i;
    ENTRY item;

    if (NT_cnt >= NT_size) {
	NT = enlargeArray (NT, &NT_size, sizeof (NAMETABLE), 1.5);
	hdestroy ();
	hcreate (NT_size - mcs_NT_cnt);
	for (i = mcs_NT_cnt; i < NT_cnt; i++) {
            item.key = ST + NT[ i ].name;
            item.data = (char *)&NT[ i ];
            hsearch (item, ENTER);
	}
    }

    NT[ NT_cnt ].name = newstring (s);
    NT[ NT_cnt ].xtx = -1;
    NT[ NT_cnt ].x = -1;
    item.key = ST + NT[ NT_cnt ].name;
    item.data = (char *)&NT[ NT_cnt ];
    hsearch (item, ENTER);
    return (NT_cnt++);
}

int newxt ()
{
    if (XT_cnt + 1 > XT_size) {
	XT = enlargeArray (XT, &XT_size, sizeof (int), 1.5);
    }
    return (XT_cnt++);
}

int newxx (int nbr)
{
    while (XX_cnt + nbr > XX_size) {
	XX = enlargeArray (XX, &XX_size, sizeof (int), 1.5);
    }
    XX_cnt += nbr;
    return (XX_cnt - 1);
}

int newnode ()
{
    if (N_cnt >= N_size) {
        N = enlargeArray (N, &N_size, sizeof (NODE), 1.5);
    }

    N[ N_cnt ].ntx = -1;
    N[ N_cnt ].cx = 0;
    N[ N_cnt ].dsx = 0;
    N[ N_cnt ].statcap = 0;
    N[ N_cnt ].funcoutp = FALSE;
    N[ N_cnt ].redirect = FALSE;
    N[ N_cnt ].linked = FALSE;
    N[ N_cnt ].flag = FALSE;
    return (N_cnt++);
}

int newds (int nbr)
{
    while (DS_cnt + nbr > DS_size) {
	DS = enlargeArray (DS, &DS_size, sizeof (int), 1.5);
    }
    DS_cnt += nbr;
    return (DS_cnt - 1);
}

int newc (int nbr)
{
    while (C_cnt + nbr > C_size) {
	C = enlargeArray (C, &C_size, sizeof (CONTROL), 1.5);
    }
    C_cnt += nbr;
    return (C_cnt - 1);
}

int newtransistor ()
{
    if (T_cnt >= T_size) {
        T = enlargeArray (T, &T_size, sizeof (TRANSISTOR), 1.5);
    }
    T[ T_cnt ].flag = FALSE;
    T[ T_cnt ].gate = -1;
    T[ T_cnt ].source = -1;
    T[ T_cnt ].drain = -1;
    return (T_cnt++);
}

int newfunctional ()
{
    if (F_cnt >= F_size) {
        F = enlargeArray (F, &F_size, sizeof (FUNCTION), 1.5);
    }
    return (F_cnt++);
}

int newintercap ()
{
    if (I_cnt >= I_size) {
        I = enlargeArray (I, &I_size, sizeof (INTERCAP), 1.5);
    }
    I[ I_cnt ].con1 = -1;
    I[ I_cnt ].con2 = -1;
    return (I_cnt++);
}

int newfi (int nbr)
{
    while (FI_cnt + nbr > FI_size) {
	FI = enlargeArray (FI, &FI_size, sizeof (int), 1.5);
    }
    while (nbr-- > 0)
	FI[ FI_cnt++ ] = -1;
    return (FI_cnt - 1);
}

int newfr (int nbr)
{
    while (FR_cnt + nbr > FR_size) {
	FR = enlargeArray (FR, &FR_size, sizeof (int), 1.5);
    }
    while (nbr-- > 0)
	FR[ FR_cnt++ ] = -1;
    return (FR_cnt - 1);
}

int newfo (int nbr)
{
    while (FO_cnt + nbr > FO_size) {
	FO = enlargeArray (FO, &FO_size, sizeof (FUNCOUT), 1.5);
    }
    while (nbr-- > 0) {
        FO[ FO_cnt ].trise = 0;
        FO[ FO_cnt ].tfall = 0;
        FO[ FO_cnt ].type = NOTYPE;
	FO[ FO_cnt++ ].x = -1;
    }
    return (FO_cnt - 1);
}

int newfs (int nbr)
{
    int a_nbr;

    if (nbr % SIZE_PTR_INT == 0)
        a_nbr = nbr;
    else
        a_nbr = nbr + SIZE_PTR_INT - (nbr % SIZE_PTR_INT);
        /* to be machine independent */

    while (FS_cnt + a_nbr > FS_size) {
	FS = enlargeArray (FS, &FS_size, sizeof (char), 1.5);
    }
    FS_cnt += a_nbr;
    return (FS_cnt - (a_nbr - nbr) - 1);
}

int newfd ()
{
    if (FD_cnt >= FD_size) {
        FD = enlargeArray (FD, &FD_size, sizeof (FUNCDESCR), 1.9);
    }
    return (FD_cnt++);
}

int newfv ()
{
    if (FV_cnt >= FV_size) {
        FV = enlargeArray (FV, &FV_size, sizeof (FUNCVAR), 1.9);
    }
    return (FV_cnt++);
}

/* enlarges array A by multiplication with m */
static void *enlargeArray (void *A, int *A_size, int sizeofA, float m)
{
    char *oldarr;
    char *newarr;
    int oldtotsize = (*A_size) * sizeofA;
    int newtotsize, newsize, i;

    newsize = (int)((float)(*A_size) * m);
    if (newsize <= *A_size)
	(*A_size)++;
    else
	*A_size = newsize;

    newtotsize = (*A_size) * sizeofA;

    PALLOC (newarr, newtotsize, char);

    oldarr = A;
    for (i = 0; i < oldtotsize; i++) {
	*(newarr + i) = *(oldarr + i);
    }

    CFREE (A);

    return (newarr);
}
