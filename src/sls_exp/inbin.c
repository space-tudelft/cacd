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

extern int functype (char *name, int imported, time_t *t, char *otherproject);
extern void inint (DM_STREAM *dsp, int *var);
static void intab (DM_STREAM *dsp, char *tabref, int *cnt, int type_size);

void inbin (CHILD_LIST *child)
{
    char *other_name;
    DM_PROJECT *new_projkey;
    int cnt;
    int offset;
    int prev_ST_cnt;
    int prev_NT_cnt;
    int prev_MT_cnt;
    int prev_CTT_cnt;
    int prev_MCT_cnt;
    int prev_XT_cnt;
    int prev_XX_cnt;
    int prev_N_cnt;
    int prev_DS_cnt;
    int prev_C_cnt;
    int prev_T_cnt;
    int prev_F_cnt;
    int prev_I_cnt;
    int prev_FI_cnt;
    int prev_FR_cnt;
    int prev_FO_cnt;
    int prev_FS_cnt;
    int mx;
    int nbr;
    int inst_nbr;
    int i;
    int k;
    int obj_mct_cnt;
    CALL_LIST * call;
    DM_CELL * key_child;
    DM_STREAM * dsp_bin;
    FUNCDESCR FDT;
    OLD_FUNCDESCR OLD_FDT;
    OLD_MODELTABLE OLD_MT;
    OLD2_FUNCDESCR OLD2_FDT;
    OLD2_MODELTABLE OLD2_MT;
    int FDT_cnt;
    int size;
    time_t dummy;
    int * newtype;
    int oldbinformat = 0;
    int mark;
    long dmTell ();

    newtype = NULL;

    if (child -> imported) {
	new_projkey = dmFindProjKey (IMPORTED, child -> object,
				     dmproject, &other_name, CIRCUIT);
	key_child = dmCheckOut (new_projkey, other_name, ACTUAL,
				DONTCARE, CIRCUIT, READONLY);
    }
    else {
	key_child = dmCheckOut (dmproject, child -> object, ACTUAL,
				DONTCARE, CIRCUIT, READONLY);
    }

    dsp_bin = dmOpenStream (key_child, sls_fn, "r");

    dmSeek (dsp_bin, -(long) (sizeof (BINREF) + sizeof (int)), 2);
    inint (dsp_bin, &mark);
    if (mark == MARK_OLD2_NAMESIZE) {
        oldbinformat = 2;
    }
    else if (mark != MARK_NEW_NAMESIZE) {
        oldbinformat = 1;
    }

    dmSeek (dsp_bin, (long)0, 0);  /* rewind */

    for (k = 1; k <= 6; k++) {    /* read 6 arrays */
	inint (dsp_bin, &size);
        dmSeek (dsp_bin, (long)size, 1);
    }
    inint (dsp_bin, &size);
    obj_mct_cnt = size / sizeof (MODELCALLTABLE);

    dmSeek (dsp_bin, (long)0, 0);  /* rewind */

    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1); /* old information */

    inint (dsp_bin, &size);
    if (oldbinformat == 2)
        FDT_cnt = size / sizeof (OLD2_FUNCDESCR);
    else if (oldbinformat == 1)
        FDT_cnt = size / sizeof (OLD_FUNCDESCR);
    else
        FDT_cnt = size / sizeof (FUNCDESCR);
    if (FDT_cnt > 0)
	PALLOC (newtype, FDT_cnt, int);
    for (i = 0; i < FDT_cnt; i++) {
        if (oldbinformat == 2) {
            fread ((char *)&OLD2_FDT, sizeof (OLD2_FUNCDESCR), 1, dsp_bin -> dmfp);
            strcpy (FDT.name, OLD2_FDT.name);
            strcpy (FDT.dmpath, OLD2_FDT.dmpath);
            FDT.help = OLD2_FDT.help;
            FDT.fvx = OLD2_FDT.fvx;
            FDT.fvx_cnt = OLD2_FDT.fvx_cnt;
            FDT.offsx = OLD2_FDT.offsx;

        }
        else if (oldbinformat == 1) {
            fread ((char *)&OLD_FDT, sizeof (OLD_FUNCDESCR), 1, dsp_bin -> dmfp);
            strcpy (FDT.name, OLD_FDT.name);
            strcpy (FDT.dmpath, OLD_FDT.dmpath);
            FDT.help = OLD_FDT.help;
            FDT.fvx = OLD_FDT.fvx;
            FDT.fvx_cnt = OLD_FDT.fvx_cnt;
            FDT.offsx = OLD_FDT.offsx;

        }
        else
            fread ((char *)&FDT, sizeof (FUNCDESCR), 1, dsp_bin -> dmfp);
        newtype[i] = functype (FDT.name, 0, &dummy, FDT.dmpath);
	FD[ newtype[i] ].help = 1;  /* set flag */
    }

    prev_ST_cnt = ST_cnt;
    intab (dsp_bin, ST + ST_cnt, &cnt, sizeof (char));
    ST_cnt += cnt;

    prev_MT_cnt = MT_cnt;
    if (oldbinformat == 2) {
        inint (dsp_bin, &size);
        cnt = size / sizeof (OLD2_MODELTABLE);
        for (i = 0; i < cnt; i++) {
            fread ((char *)&OLD2_MT, sizeof (OLD2_MODELTABLE), 1, dsp_bin -> dmfp);
            strcpy (MT[MT_cnt + i].name, OLD2_MT.name);
            MT[MT_cnt + i].nt_cnt = OLD2_MT.nt_cnt;
	}
    }
    else if (oldbinformat == 1) {
        inint (dsp_bin, &size);
        cnt = size / sizeof (OLD_MODELTABLE);
        for (i = 0; i < cnt; i++) {
            fread ((char *)&OLD_MT, sizeof (OLD_MODELTABLE), 1, dsp_bin -> dmfp);
            strcpy (MT[MT_cnt + i].name, OLD_MT.name);
            MT[MT_cnt + i].nt_cnt = OLD_MT.nt_cnt;
        }
    }
    else
	intab (dsp_bin, (char *)(MT + MT_cnt), &cnt, sizeof (MODELTABLE));
    MT_cnt += cnt;
    mx = MT_cnt - 1;

    offset = (int) dmTell (dsp_bin);

    for (call = child -> calls; call != NULL; call = call -> next) {

        call -> mcx = MCT_cnt + obj_mct_cnt * call -> number;
        for (inst_nbr = 1; inst_nbr <= call -> number; inst_nbr++) {

            prev_NT_cnt = NT_cnt;
            prev_CTT_cnt = CTT_cnt;
            prev_MCT_cnt = MCT_cnt;
            prev_XT_cnt = XT_cnt;
            prev_XX_cnt = XX_cnt;
            prev_N_cnt = N_cnt;
            prev_DS_cnt = DS_cnt;
            prev_C_cnt = C_cnt;
            prev_T_cnt = T_cnt;
            prev_F_cnt = F_cnt;
            prev_I_cnt = I_cnt;
            prev_FI_cnt = FI_cnt;
            prev_FR_cnt = FR_cnt;
            prev_FO_cnt = FO_cnt;
            prev_FS_cnt = FS_cnt;

	    dmSeek (dsp_bin, (long)offset, 0);
            /* move pointer to begin of arrays */

            intab (dsp_bin, (char *)(NT + NT_cnt), &cnt, sizeof (NAMETABLE));
            NT_cnt += cnt;
            for (i = prev_NT_cnt; i < NT_cnt; i++) {
	        NT[i].name += prev_ST_cnt;
	        if (NT[i].xtx >= 0)
	            NT[i].xtx += prev_XT_cnt;
	        switch ( NT[i].sort) {
		    case Node_t :
                        NT[i].sort = Node;
		    case Node :
		        if (NT[i].xtx >= 0)
			    NT[i].x += prev_XX_cnt;
		        else
		            NT[i].x += prev_N_cnt;
		        break;
		    case Modelcall :
		        NT[i].x += prev_MCT_cnt;
		        break;
		    case Transistor :
		        NT[i].x += prev_T_cnt;
		        break;
		    case Intercap :
		        NT[i].x += prev_I_cnt;
		        break;
		    case Functional :
		        NT[i].x += prev_F_cnt;
		        break;
	        }
	    }

            intab (dsp_bin, (char *)(CTT + CTT_cnt), &cnt,
                   sizeof (CONTEXTTABLE));
            CTT_cnt += cnt;
            for (i = prev_CTT_cnt; i < CTT_cnt; i++) {
	        CTT[i].ceiling += prev_N_cnt;
	        if (CTT[i].mctx >= 0 )
	            CTT[i].mctx += prev_MCT_cnt;
	        else
		    CTT[i].mctx = call -> mcx + inst_nbr - 1;
	    }

            intab (dsp_bin, (char *)(MCT + MCT_cnt), &cnt,
                   sizeof (MODELCALLTABLE));
            MCT_cnt += cnt;
	    for (i = prev_MCT_cnt; i < MCT_cnt; i++) {
	        if (MCT[i].parent >= 0)
	            MCT[i].parent += prev_MCT_cnt;
	        else
		    MCT[i].parent = call -> mcx + inst_nbr - 1;
	        if (MCT[i].mtx >= 0)
	            MCT[i].mtx += prev_MT_cnt;
	        if (MCT[i].ntx >= 0)
	            MCT[i].ntx += prev_NT_cnt;
	        if (MCT[i].n_ntx >= 0)
	            MCT[i].n_ntx += prev_NT_cnt;
	    }

            intab (dsp_bin, (char *)(XT + XT_cnt), &cnt, sizeof (int));
            XT_cnt += cnt;

            intab (dsp_bin, (char *)(XX + XX_cnt), &cnt, sizeof (int));
            XX_cnt += cnt;
	    for (i = prev_XX_cnt; i < XX_cnt; i++) {
	        XX[i] += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(N + N_cnt), &cnt, sizeof (NODE));
            N_cnt += cnt;
	    for (i = prev_N_cnt; i < N_cnt; i++) {
                if (N[i].ntx >= 0)
	            N[i].ntx += prev_NT_cnt;
	        if (N[i].dsx >= 0)
	            N[i].dsx += prev_DS_cnt;
	        if (N[i].cx >= 0) {
		    if (N[i].redirect)
	                N[i].cx += prev_N_cnt;
		    else
	                N[i].cx += prev_C_cnt;
	        }
	    }

            intab (dsp_bin, (char *)(DS + DS_cnt), &cnt, sizeof (int));
            DS_cnt += cnt;
	    for (i = prev_DS_cnt; i < DS_cnt; i++) {
	        nbr = DS[i];
	        while (nbr-- > 0)
	            DS[++i] += prev_T_cnt;
	    }

            intab (dsp_bin, (char *)(C + C_cnt), &cnt, sizeof (CONTROL));
            C_cnt += cnt;
	    for (i = prev_C_cnt; i < C_cnt; i++) {
	        nbr = C[i].c;
	        while (nbr-- > 0) {
	            switch ( C[++i].sort ) {
		        case Transistor :
	                    C[i].c += prev_T_cnt;
		            break;
		        case Functional :
	                    C[i].c += prev_F_cnt;
		            break;
		        case Intercap :
	                    C[i].c += prev_I_cnt;
		            break;
		    }
	        }
	    }

            intab (dsp_bin, (char *)(T + T_cnt), &cnt,sizeof (TRANSISTOR));
            T_cnt += cnt;
	    for (i = prev_T_cnt; i < T_cnt; i++) {
	        if (T[i].gate >= 0)
	            T[i].gate += prev_N_cnt;
	        T[i].source += prev_N_cnt;
	        T[i].drain += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(F + F_cnt),  &cnt, sizeof (FUNCTION));
            F_cnt += cnt;
	    for (i = prev_F_cnt; i < F_cnt; i++) {
	        if (F[i].fix >= 0) F[i].fix += prev_FI_cnt;
	        if (F[i].frx >= 0) F[i].frx += prev_FR_cnt;
	        if (F[i].fox >= 0) F[i].fox += prev_FO_cnt;
	        if (F[i].fsx >= 0) F[i].fsx += prev_FS_cnt;
                if (F[i].type < 1000)
                    F[i].type = newtype[ F[i].type ];
                    /* that's the index in FD */
	    }

            intab (dsp_bin, (char *)(I + I_cnt),  &cnt, sizeof (INTERCAP));
            I_cnt += cnt;
	    for (i = prev_I_cnt; i < I_cnt; i++) {
	        I[i].con1 += prev_N_cnt;
	        I[i].con2 += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(FI + FI_cnt),  &cnt, sizeof (int));
            FI_cnt += cnt;
	    for (i = prev_FI_cnt; i < FI_cnt; i++) {
	        nbr = FI[i];
	        while (nbr-- > 0)
	            FI[++i] += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(FR + FR_cnt),  &cnt, sizeof (int));
            FR_cnt += cnt;
	    for (i = prev_FR_cnt; i < FR_cnt; i++) {
	        nbr = FR[i];
	        while (nbr-- > 0)
	            FR[++i] += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(FO + FO_cnt), &cnt, sizeof (FUNCOUT));
            FO_cnt += cnt;
	    for (i = prev_FO_cnt; i < FO_cnt; i++) {
	        nbr = FO[i].x;
	        while (nbr-- > 0)
	            FO[++i].x += prev_N_cnt;
	    }

            intab (dsp_bin, (char *)(FS + FS_cnt), &cnt, sizeof (char));
            FS_cnt += cnt;

            /* create a new model call in the MCT table */
	    MCT[ call -> mcx + inst_nbr - 1 ].n_ntx = NT_cnt - MT[mx].nt_cnt;
	    MCT[ call -> mcx + inst_nbr - 1 ].parent = -1;
	    MCT[ call -> mcx + inst_nbr - 1 ].mtx = mx;
        }
        MCT_cnt += call -> number;
    }

    if (newtype) CFREE (newtype);

    dmCloseStream (dsp_bin, COMPLETE);
    dmCheckIn (key_child, QUIT);
}

static void intab (DM_STREAM *dsp, char *tabref, int *cnt, int type_size) /* read an array into tabref */
{
    int size;
    fread ((char *)&size, sizeof (int), 1, dsp -> dmfp);
    fread ((char *)tabref, size, 1, dsp -> dmfp);
    *cnt = size / type_size;
}

void inint (DM_STREAM *dsp, int *var) /* read an integer into var */
{
    fread ((char *)var, sizeof (int), 1, dsp -> dmfp);
}
