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

static int  checkBinEndmark (DM_STREAM *dsp);
static int  functype (char *name);
static void inelem (DM_STREAM *dsp, char *elem, int type_size);
static void inint (DM_STREAM *dsp, int *var);
static void intab (DM_STREAM *dsp, char **tabref, int *cnt, int type_size);

extern DM_PROJECT *dmproject;

void binfil ()
{
    int cnt, i, j;
    FUNCDESCR FDT;
    OLD_FUNCDESCR OLD_FDT;
    OLD2_FUNCDESCR OLD2_FDT;
    FUNCVAR FVT;
    OLD_FUNCVAR OLD_FVT;
    OLD2_FUNCVAR OLD2_FVT;
    OLD_MODELTABLE OLD_MT;
    OLD2_MODELTABLE OLD2_MT;
    int FDT_cnt;
    int fvx;
    int fvtx;
    int oldbinformat = 0;
    long fremember;
    char * p;
    int intval;
    int mark;
    DM_STREAM * dsp_bin;
    DM_CELL * model_key;
    PRE_NODE ePRE_N;
    PRE_TRANSISTOR ePRE_T;
    int size;
    struct stat buf;
    char sls_fn[128];

    if (monitoring) monitime ("B inbinfile");

    model_key = dmCheckOut (dmproject, netwname, WORKING, DONTCARE, CIRCUIT, ATTACH);

    sprintf (sls_fn, "sls");

    if (dmStat (model_key, sls_fn, &buf) != 0) {
	slserror (NULL, 0, ERROR2, "no binary file for", netwname);
	die (1);
    }
    dsp_bin = dmOpenStream (model_key, sls_fn, "r");

    if (checkBinEndmark (dsp_bin)) {
	slserror (NULL, 0, ERROR2, "Incompatible binary file for", netwname);
	die (1);
    }

    dmSeek (dsp_bin, -(long) (sizeof (BINREF) + sizeof (int)), 2);
    inint (dsp_bin, &mark);
    if (mark == MARK_OLD2_NAMESIZE) {
        oldbinformat = 2;
    }
    else if (mark != MARK_NEW_NAMESIZE) {
        oldbinformat = 1;
    }

    dmSeek (dsp_bin, (long)0, 0);   /* rewind */

    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);

    inint (dsp_bin, &size);
    if (oldbinformat == 2)
        FDT_cnt = size / sizeof (OLD2_FUNCDESCR);
    else if (oldbinformat == 1)
        FDT_cnt = size / sizeof (OLD_FUNCDESCR);
    else
        FDT_cnt = size / sizeof (FUNCDESCR);
    for (i = 0; i < FDT_cnt; i++) {
        if (oldbinformat == 2) {
            fread ((char *)&OLD2_FDT, sizeof (OLD2_FUNCDESCR), 1 ,dsp_bin -> dmfp);
            strcpy (FDT.name, OLD2_FDT.name);
            strcpy (FDT.dmpath, OLD2_FDT.dmpath);
            FDT.help = OLD2_FDT.help;
            FDT.fvx = OLD2_FDT.fvx;
            FDT.fvx_cnt = OLD2_FDT.fvx_cnt;
            FDT.offsx = OLD2_FDT.offsx;
        }
        else if (oldbinformat == 1) {
            fread ((char *)&OLD_FDT, sizeof (OLD_FUNCDESCR), 1 ,dsp_bin -> dmfp);
            strcpy (FDT.name, OLD_FDT.name);
            strcpy (FDT.dmpath, OLD_FDT.dmpath);
            FDT.help = OLD_FDT.help;
            FDT.fvx = OLD_FDT.fvx;
            FDT.fvx_cnt = OLD_FDT.fvx_cnt;
            FDT.offsx = OLD_FDT.offsx;
        }
        else
            fread ((char *)&FDT, sizeof (FUNCDESCR), 1 ,dsp_bin -> dmfp);
        fremember = dmTell (dsp_bin);
        if ((FD[i].help = functype (FDT.name )) < 0)  /* use help as redir */
	    slserror (NULL, 0, ERROR2, FDT.name, "is an undefined function in the simulator");
        if (FD[ FD[i].help ].fvx_cnt != FDT.fvx_cnt)
	    slserror (NULL, 0, ERROR2, FDT.name, "is not equally defined in network and simulator");
        fvx = FD[ FD[i].help ].fvx;
        fvtx = FDT.fvx;
        if (fvtx >= 0) {
            if (oldbinformat == 2)
		dmSeek (dsp_bin, (long) (sizeof (int) + fvtx * sizeof (OLD2_FUNCVAR)), 0);
            else if (oldbinformat == 1)
		dmSeek (dsp_bin, (long) (sizeof (int) + fvtx * sizeof (OLD_FUNCVAR)), 0);
            else
		dmSeek (dsp_bin, (long) (sizeof (int) + fvtx * sizeof (FUNCVAR)), 0);
        }
        for (j = 0; j < FDT.fvx_cnt; j++) {
            if (oldbinformat == 2) {
                fread ((char *)&OLD2_FVT, sizeof (OLD2_FUNCVAR), 1, dsp_bin -> dmfp);
                strcpy (FVT.name, OLD2_FVT.name);
                FVT.help = OLD2_FVT.help;
                FVT.type = OLD2_FVT.type;
                FVT.ind[0] = OLD2_FVT.ind[0];
                FVT.ind[1] = OLD2_FVT.ind[1];
            }
            else if (oldbinformat == 1) {
                fread ((char *)&OLD_FVT, sizeof (OLD_FUNCVAR), 1, dsp_bin -> dmfp);
                strcpy (FVT.name, OLD_FVT.name);
                FVT.help = OLD_FVT.help;
                FVT.type = OLD_FVT.type;
                FVT.ind[0] = OLD_FVT.ind[0];
                FVT.ind[1] = OLD_FVT.ind[1];
            }
            else
                fread ((char *)&FVT, sizeof (FUNCVAR), 1, dsp_bin -> dmfp);
            if (strcmp (FVT.name, FV[ fvx + j ].name) != 0
            || FVT.type != FV[ fvx + j ].type
            || FVT.ind[0] != FV[ fvx + j ].ind[0]
            || FVT.ind[1] != FV[ fvx + j ].ind[1])
	        slserror (NULL, 0, ERROR2, FDT.name, "is not equally defined in network and simulator");
         }
         dmSeek (dsp_bin, fremember, 0);
    }

    intab (dsp_bin, &ST, &ST_cnt, sizeof (char));
    if (oldbinformat == 2) {
        inint (dsp_bin, &size);
        MT_cnt = size / sizeof (OLD2_MODELTABLE);
	PALLOC (MT, MT_cnt, MODELTABLE);
        for (i = 0; i < MT_cnt; i++) {
            fread ((char *)&OLD2_MT, sizeof (OLD2_MODELTABLE), 1 ,dsp_bin -> dmfp);
            strcpy (MT[i].name, OLD2_MT.name);
            MT[i].nt_cnt = OLD2_MT.nt_cnt;
        }
    }
    else if (oldbinformat == 1) {
        inint (dsp_bin, &size);
        MT_cnt = size / sizeof (OLD_MODELTABLE);
	PALLOC (MT, MT_cnt, MODELTABLE);
        for (i = 0; i < MT_cnt; i++) {
            fread ((char *)&OLD_MT, sizeof (OLD_MODELTABLE), 1 ,dsp_bin -> dmfp);
            strcpy (MT[i].name, OLD_MT.name);
            MT[i].nt_cnt = OLD_MT.nt_cnt;
        }
    }
    else
        intab (dsp_bin, (char **)(void *)&MT, &MT_cnt, sizeof (MODELTABLE));
    intab (dsp_bin, (char **)(void *)&NT, &NT_cnt, sizeof (NAMETABLE));
    intab (dsp_bin, (char **)(void *)&CTT, &CTT_cnt, sizeof (CONTEXTTABLE));
    intab (dsp_bin, (char **)(void *)&MCT, &MCT_cnt, sizeof (MODELCALLTABLE));
    intab (dsp_bin, (char **)(void *)&XT, &XT_cnt, sizeof (int));
    intab (dsp_bin, (char **)(void *)&XX, &XX_cnt, sizeof (int));

    inint (dsp_bin, &size);
    N_cnt = size / sizeof (PRE_NODE);
    PALLOC (N, N_cnt, NODE);
    for (cnt = 0; cnt < N_cnt; cnt++) {

        inelem (dsp_bin, (char *)&ePRE_N, sizeof (PRE_NODE));

        N[cnt].ntx = ePRE_N.ntx;
        N[cnt].dsx = ePRE_N.dsx;
        N[cnt].cx = ePRE_N.cx;
        N[cnt].statcap = ePRE_N.statcap;
        N[cnt].dyncap = ePRE_N.statcap;
        N[cnt].funcoutp = ePRE_N.funcoutp;
        N[cnt].redirect = ePRE_N.redirect;

        N[cnt].type = Normal;
        N[cnt].inp = FALSE;
        N[cnt].outp = FALSE;
        N[cnt].breaksig = FALSE;
        N[cnt].linked = FALSE;
        N[cnt].flag = FALSE;
	N[cnt].dissip = FALSE;
        N[cnt].plot = 0;
        N[cnt].print = 0;
        N[cnt].forcedinfo = NULL;
        if (N[cnt].funcoutp)
            N[cnt].type = Forced;
    }

    intab (dsp_bin, (char **)(void *)&DS, &DS_cnt, sizeof (int));
    intab (dsp_bin, (char **)(void *)&C, &C_cnt, sizeof (CONTROL));

    inint (dsp_bin, &size);
    T_cnt = size / sizeof (PRE_TRANSISTOR);
    PALLOC (T, T_cnt, TRANSISTOR);
    for (cnt = 0; cnt < T_cnt; cnt++) {

        inelem (dsp_bin, (char *)&ePRE_T, sizeof (PRE_TRANSISTOR));

        T[cnt].gate = ePRE_T.gate;
        T[cnt].source = ePRE_T.source;
        T[cnt].drain = ePRE_T.drain;
        T[cnt].type = ePRE_T.type;
        T[cnt].width = ePRE_T.width;
        T[cnt].length = ePRE_T.length;
    }

    intab (dsp_bin, (char **)(void *)&F, &F_cnt, sizeof (FUNCTION));
    intab (dsp_bin, (char **)(void *)&I, &I_cnt, sizeof (INTERCAP));
    intab (dsp_bin, (char **)(void *)&FI, &FI_cnt, sizeof (int));
    intab (dsp_bin, (char **)(void *)&FR, &FR_cnt, sizeof (int));
    intab (dsp_bin, (char **)(void *)&FO, &FO_cnt, sizeof (FUNCOUT));
    intab (dsp_bin, (char **)(void *)&FS, &FS_cnt, sizeof (char));

    dmCloseStream (dsp_bin, COMPLETE);
    dmCheckIn (model_key, COMPLETE);

    for (i =0; i < F_cnt; i++) {
        if (F[i].type < 1000)
            F[i].type = FD[ F[i].type ].help;
    }

    for (p = FS; p - FS < FS_cnt; p++) {
        if (*p == '*') {
            p += SIZE_PTR_INT;
            if ((p - FS) % SIZE_PTR_INT != 0)
                p += SIZE_PTR_INT - ((p - FS) % SIZE_PTR_INT);
            cnt = *(int *)p;
            p += SIZE_PTR_INT;
            while (cnt-- > 0) {
                intval = *(int *)p; /* intval gives the offset of the pointer */
                *(char **)p = p + intval;
                p += SIZE_PTR_INT;
            }
        }
    }

    if (monitoring) monitime ("E inbinfile");
}

static void intab (DM_STREAM *dsp, char **tabref, int *cnt, int type_size)
{
    int size;

    fread ((char *)&size, sizeof (int), 1, dsp -> dmfp);
    PALLOC (*tabref, size, char);
    fread ((char *)(*tabref), size, 1, dsp -> dmfp);
    *cnt = size / type_size;
}

static void inint (DM_STREAM *dsp, int *var)
{
    fread ((char *)var, sizeof (int), 1, dsp -> dmfp);
}

static void inelem (DM_STREAM *dsp, char *elem, int type_size)
{
    fread (elem, type_size, 1, dsp -> dmfp);
}

static int functype (char *name)
{
    int i;

    for (i = 0; i < FD_cnt; i++) {
        if (strcmp (name, FD[i].name) == 0)
            break;
    }

    if (i < FD_cnt)
        return (i);
    else
        return (-1);
}

static int checkBinEndmark (DM_STREAM *dsp)
{
    BINREF BR;

    dmSeek (dsp, -(long) sizeof (BINREF), 2);
    fread ((char *)&BR, sizeof (BINREF), 1, dsp -> dmfp);

    if (BR.a != 1234
    || BR.b != -1234
    || BR.c != 86
    || BR.d != 0
    || BR.e != -456
    || strcmp (BR.f, "qwertyuiopa") != 0
    || BR.g < 0.99999 || BR.g > 1.00001
    || BR.h < -20.02010 || BR.h > -20.01990
    || BR.i != 5
    || BR.j != 13
    || BR.k != 18
    || BR.l != 255
    || BR.m != 28282)
        return (1);
    else
        return (0);
}
