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

static void copy_struct (char *p1, char *p2, int size);
static void outtab (DM_STREAM *dsp, char *tabref, int cnt, int type_size);

/* writes out the datastructure of the current */
/* model 'm' in binary format */
void outbin (DM_CELL *m)
{
    DM_STREAM * dsp_bin;
    int i, j;
    int fvx;
    int size;
    int FVT_cnt;
    int FDT_cnt;
    FUNCDESCR FDT;
    BINREF BR;
    int mark;
    long startgspec;

    FVT_cnt = 0;
    FDT_cnt = 0;
    for (i = 0; i < FD_cnt; i++) {
        if (FD[i].help > 0) {
            fvx = FD[i].fvx;
            for (j = 0; j < FD[i].fvx_cnt; j++) {
                FV[fvx + j].help = 1;
                FVT_cnt++;
            }
            FDT_cnt++;
        }
    }

    if ((dsp_bin = dmOpenStream (m, sls_fn, "w")) == NULL)
        die (1);

    /* m -> model should be equal to MT[ MT_cnt - 1 ].name now ! */

    size = FVT_cnt * sizeof (FUNCVAR);
    fwrite ((char *)&size, sizeof (int), 1, dsp_bin -> dmfp);
    FVT_cnt = 0;
    for (i = 0; i < FV_cnt; i++) {
        if (FV[i].help > 0) {
            fwrite ((char *)&FV[i], sizeof (FUNCVAR), 1, dsp_bin -> dmfp);
            FV[i].help = FVT_cnt++;
        }
    }

    size = FDT_cnt * sizeof (FUNCDESCR);
    fwrite ((char *)&size, sizeof (int), 1, dsp_bin -> dmfp);
    FDT_cnt = 0;
    for (i = 0; i < FD_cnt; i++) {
        if (FD[i].help > 0) {
            copy_struct ((char *)&FDT, (char *)&FD[i], sizeof (FUNCDESCR));
            FDT.fvx = FV[ FD[i].fvx ].help;
            fwrite ((char *)&FDT, sizeof (FUNCDESCR), 1, dsp_bin -> dmfp);
            FD[i].help = FDT_cnt++;
        }
    }

    for (i = 0; i < F_cnt; i++) {
        if (F[i].type < 1000)
            F[i].type = FD[ F[i].type ].help;
    }

    outtab (dsp_bin, ST, ST_cnt, sizeof (char));

    outtab (dsp_bin, (char *)MT, MT_cnt, sizeof (MODELTABLE));

    outtab (dsp_bin, (char *)NT, NT_cnt, sizeof (NAMETABLE));

    outtab (dsp_bin, (char *)CTT, CTT_cnt, sizeof (CONTEXTTABLE));

    outtab (dsp_bin, (char *)MCT, MCT_cnt, sizeof (MODELCALLTABLE));

    outtab (dsp_bin, (char *)XT, XT_cnt, sizeof (int));

    outtab (dsp_bin, (char *)XX, XX_cnt, sizeof (int));

    outtab (dsp_bin, (char *)N, N_cnt, sizeof (NODE));

    outtab (dsp_bin, (char *)DS, DS_cnt, sizeof (int));

    outtab (dsp_bin, (char *)C, C_cnt, sizeof (CONTROL));

    outtab (dsp_bin, (char *)T, T_cnt, sizeof (TRANSISTOR));

    outtab (dsp_bin, (char *)F, F_cnt, sizeof (FUNCTION));

    outtab (dsp_bin, (char *)I, I_cnt, sizeof (INTERCAP));

    outtab (dsp_bin, (char *)FI, FI_cnt, sizeof (int));

    outtab (dsp_bin, (char *)FR, FR_cnt, sizeof (int));

    outtab (dsp_bin, (char *)FO, FO_cnt, sizeof (FUNCOUT));

    outtab (dsp_bin, (char *)FS, FS_cnt, sizeof (char));

    /* write global nets that are defined in this binary file */

    fprintf (dsp_bin -> dmfp, " ");
    startgspec = ftell (dsp_bin -> dmfp);
    fprintf (dsp_bin -> dmfp, " %d ", globNets_cnt);
    for (i = 0; i < globNets_cnt; i++) {
	fprintf (dsp_bin -> dmfp, "%s ", globNets[i]);
    }
    fwrite ((char *)&startgspec, sizeof (long), 1, dsp_bin -> dmfp);

    /* Write a special integer value that denotes that for this file,
       a value for NAMESIZE is used that is Min (33, DM_MAXNAME+1).
    */

    mark = MARK_NEW_NAMESIZE;
    fwrite ((char *)&mark, sizeof (int), 1, dsp_bin -> dmfp);

    /* the following is a 'magic structure' that is used to
       check if this binary file (see checkBinEndmark (),
       elsewhere in this program) is readable for an sls_exp
       or sls run that is executed later.
    */

    BR.a = 1234;
    BR.b = -1234;
    BR.c = 86;
    BR.d = 0;
    BR.e = -456;
    strcpy (BR.f, "qwertyuiopa");
    BR.g = 1.0;
    BR.h = -20.02;
    BR.i = 5;
    BR.j = 13;
    BR.k = 18;
    BR.l = 255;
    BR.m = 28282;
    fwrite ((char *)&BR, sizeof (BINREF), 1, dsp_bin -> dmfp);

    dmCloseStream (dsp_bin, COMPLETE);
}

static void outtab (DM_STREAM *dsp, char *tabref, int cnt, int type_size)
{
    int size = cnt * type_size;
    fwrite ((char *)&size, sizeof (int), 1, dsp -> dmfp);
    fwrite ((char *)tabref, size, 1, dsp -> dmfp);
}

static void copy_struct (char *p1, char *p2, int size)
{
    int i;
    for (i = 0; i < size; i++) *p1++ = *p2++;
}
