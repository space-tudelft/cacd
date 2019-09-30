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

extern int devtype (char *name);
extern int functype (char *name, int imported, time_t *t, char *otherproject);
extern int is_ap (void);
extern int is_func (char *a);
extern int newfi (int nbr);
extern int newfo (int nbr);
extern int newfr (int nbr);
extern int newfs (int nbr);
extern int newfunctional (void);
extern int newintercap (void);
extern int newtransistor (void);
extern char *next_attr (char **p, char **v, char *a);
extern int stdfunctype (char *name);

/* gets the devices (transistors etc.) and standard */
/* functions (nands, nors etc.) of model m */
void getdev (DM_CELL *m)
{
    char attribute_string[256];
    long lower[10], upper[10];
    int i, j, k;
    int dev;
    int up;
    int low;
    int new;
    float length, width;
    int x;
    int ntx;
    int fix;
    int frx;
    int fox;
    int fsx;
    int nbr;
    float val;
    int number;
    float trise, tfall;
    int size;
    int fi_space;
    int fr_space;
    int fo_space;
    int fs_space;
    int os_cnt;
    int is_cnt;
    int rs_cnt;
    int ps_cnt;
    int ss_cnt;
    int fvx;
    int osx;
    int isx;
    int rsx;
    int psx;
    int ssx;
    char * pc;
    unsigned first;
    char fn_mc[DM_MAXNAME+12];
    DM_STREAM * dsp;
    int is_f;
    int item;
    time_t dummy;
    char * attribute;
    char * parname;
    char * parval;

    sprintf (fn_mc, "circuit/%s/mc", m -> cell);

    dsp = dmOpenStream (m, "mc", "r");

    dm_get_do_not_alloc = 1;
    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    /* read devices from the modelcall file */

    item = 0;
    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	item++;
	if (is_ap ()) continue;

	is_f = is_func (cmc.inst_attribute);
        dev = devtype (cmc.cell_name);
	if (dev < 0) {
	    if (is_f) {
		if ((dev = stdfunctype (cmc.cell_name)) < 0) {
                    if ((dev = functype (cmc.cell_name, (int)cmc.imported, &dummy, NULL)) < 0)
		        dberror (fn_mc, item, "internal error", NULL);
		}
	    }
	    else
	        /* skip this network call */
	        continue;
	}

        width = -1;
	length = -1;
	val = -1;
	number = -1;
	trise = -1;
	tfall = -1;

        attribute = cmc.inst_attribute;
	while (attribute != NULL) {
	    attribute = next_attr (&parname, &parval, attribute);
            if (parval == NULL) parval = "";
	    /* to prevent core dump when database error */

	    switch (parname[0]) {
		case 'w' :
		    if (parname[1] == '\0') {
		        if (sscanf (parval, "%f", &width) != 1)
			    dberror (fn_mc, item, "integer expected", NULL);
		    }
		    break;
		case 'l' :
		    if (parname[1] == '\0') {
		        if (sscanf (parval, "%f", &length) != 1)
			    dberror (fn_mc, item, "integer expected", NULL);
		    }
		    break;
		case 'v' :
		    if (parname[1] == '\0') {
		        if (sscanf (parval, "%f", &val) != 1)
			    dberror (fn_mc, item, "float expected", NULL);
		    }
		    break;
		case 'n' :
		    if (parname[1] == '\0') {
		        if (sscanf (parval, "%d", &number) != 1)
			    dberror (fn_mc, item, "integer expected", NULL);
		    }
		    break;
		default :
		    if (strcmp (parname, "tr") == 0) {
		        if (sscanf (parval, "%f", &trise) != 1)
			    dberror (fn_mc, item, "float expected", NULL);
		    }
		    else if (strcmp (parname, "tf") == 0) {
		        if (sscanf (parval, "%f", &tfall) != 1)
			    dberror (fn_mc, item, "float expected", NULL);
		    }
		    break;
	    }
	}

	ntx = newname (cmc.inst_name);

	nbr = 1;
        if (cmc.inst_dim > 0) {
	    new = NT[ntx].xtx = newxt ();
	    XT[new] = cmc.inst_dim;
            for (i = 0; i < cmc.inst_dim; i++) {
                low = cmc.inst_lower[i];
                up = cmc.inst_upper[i];
		new = newxt ();
		XT[new] = low;
		new = newxt ();
		XT[new] = up;
		nbr = nbr * (up - low + 1);
	    }
        }

        switch (dev) {

	    case D_NENH :
	    case D_PENH :
	    case D_DEPL :

		NT[ntx].sort = Transistor;

                first = TRUE;
                while (nbr-- > 0) {
		    x = newtransistor ();

		    switch (dev) {
		        case D_NENH :
		            T[x].type = Nenh;
			    break;
		        case D_PENH :
		            T[x].type = Penh;
			    break;
		        case D_DEPL :
		            T[x].type = Depl;
		    	    break;
		    }
		    T[x].width = width;
		    T[x].length = length;

                    if (first == TRUE) {
			NT[ntx].x = x;
			first = FALSE;
		    }
		}

                break;

	    case D_RES :    /* resistor */

		NT[ntx].sort = Transistor;

                if (val < 0) val = 0;

                first = TRUE;
                while (nbr-- > 0) {
		    x = newtransistor ();

		    T[x].type = Res;
		    T[x].length = val;

                    if (first == TRUE) {
			NT[ntx].x = x;
			first = FALSE;
		    }
		}

                break;

	    case D_CAP :  /* capacitor */

		NT[ntx].sort = Intercap;

                if (val < 0) val = 0;

                first = TRUE;
                while (nbr-- > 0) {
		    x = newintercap ();

		    I[x].cap = val;

                    if (first == TRUE) {
			NT[ntx].x = x;
			first = FALSE;
		    }
		}

                break;

            case F_OR :  /* or */
            case F_AND :  /* and */
            case F_NOR :  /* nor */
	    case F_NAND :  /* nand */
	    case F_EXOR :  /* exor */
	    case F_INVERT : /* invert */

		NT[ntx].sort = Functional;

                if (number < 0) number = 1; /* default nr. of inputs */
                if (trise < 0) trise = 0;   /* default rise time */
                if (tfall < 0) tfall = 0;   /* default fall time */

                first = TRUE;
                while (nbr-- > 0) {
		    x = newfunctional ();
		    fix = newfi (number + 1) - number;
                    fox = newfo (1 + 1) - 1;

		    F[x].type = dev;
		    F[x].fix = fix;
		    FI[fix] = number;
		    F[x].fox = fox;
                    FO[fox].x = 1;
                    FO[fox+1].trise = trise;
                    FO[fox+1].tfall = tfall;
		    F[x].fsx = -1;

                    if (first == TRUE) {
			NT[ntx].x = x;
			first = FALSE;
		    }
		}

                break;

            default :   /* defined function */

                FD[dev].help = 1;  /* set flag because it is used */

		NT[ntx].sort = Functional;

                first = TRUE;
                while (nbr-- > 0) {

		    x = newfunctional ();

                    fo_space = 0;
                    fi_space = 0;
                    fr_space = 0;
                    os_cnt = 0;
                    is_cnt = 0;
                    rs_cnt = 0;
                    ps_cnt = 0;
                    ss_cnt = 0;

                    fvx = FD[dev].fvx;
                    for (i = 0; i < FD[dev].fvx_cnt; i++) {
			size = 0;
                        switch (FV[fvx].type) {
                            case INPUT_T :
                                if (FV[fvx].ind[0] == 0) {
                                    fi_space++;
                                    is_cnt++;
                                }
                                else if (FV[fvx].ind[1] == 0) {
                                    fi_space += FV[fvx].ind[0];
                                    is_cnt += FV[fvx].ind[0] + 1;
                                }
                                else {
                                    fi_space += FV[fvx].ind[0] * FV[fvx].ind[1];
                                    is_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
                                    ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
                                }
                                break;
                            case INPUT_R :
                                if (FV[fvx].ind[0] == 0) {
                                    fr_space++;
                                    rs_cnt++;
                                }
                                else if (FV[fvx].ind[1] == 0) {
                                    fr_space += FV[fvx].ind[0];
                                    rs_cnt += FV[fvx].ind[0] + 1;
                                }
                                else {
                                    fr_space += FV[fvx].ind[0] * FV[fvx].ind[1];
                                    rs_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
                                    ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
                                }
                                break;
                            case INOUT :
                            case OUTPUT :
                                if (FV[fvx].ind[0] == 0) {
                                    fo_space++;
                                    os_cnt++;
                                }
                                else if (FV[fvx].ind[1] == 0) {
                                    fo_space += FV[fvx].ind[0];
                                    os_cnt += FV[fvx].ind[0] + 1;
                                }
                                else {
                                    fo_space += FV[fvx].ind[0] * FV[fvx].ind[1];
                                    os_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
                                    ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
                                }
                                break;
                            case CHAR :
                                if (FV[fvx].ind[0] == 0) {
                                    ss_cnt += 1;
                                }
                                else if (FV[fvx].ind[1] == 0) {
                                    ss_cnt += FV[fvx].ind[0] + 1;
                                }
                                else {
                                    ss_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
                                    ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
                                }
                                break;
                            case INTEGER :
                            case FLOAT :
                            case DOUBLE :
                                if (FV[fvx].type == INTEGER) {
				    size = sizeof (int);
				}
                                else if (FV[fvx].type == FLOAT) {
                                    size = sizeof (float);
				}
                                else if (FV[fvx].type == DOUBLE) {
                                    size = sizeof (double);
				}
                                if (FV[fvx].ind[0] == 0) {
                                    ss_cnt += size * 3 - 1;
                                }
                                else if (FV[fvx].ind[1] == 0) {
                                    ss_cnt += size * (2 + FV[fvx].ind[0]) - 1;
                                }
                                else {
                                    ss_cnt += size * FV[fvx].ind[0] * FV[fvx].ind[1] + 2 * size - 1;
                                    ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
                                }
                                break;
                        }
                        fvx++;
                    }

                    if (ps_cnt > 0)
                        ps_cnt += 3 * SIZE_PTR_INT;

                    fs_space = os_cnt + is_cnt + rs_cnt + ps_cnt + ss_cnt;

                    if (fi_space > 0) {
		        fix = newfi (fi_space + 1) - fi_space;
		        F[x].fix = fix;
		        FI[fix] = fi_space;
                    }
                    else {
		        F[x].fix = -1;
                    }

                    if (fr_space > 0) {
		        frx = newfr (fr_space + 1) - fr_space;
		        F[x].frx = frx;
		        FR[frx] = fr_space;
                    }
                    else {
		        F[x].frx = -1;
                    }

                    if (fo_space > 0) {
			int fxj;

                        fox = newfo (fo_space + 1) - fo_space;
		        F[x].fox = fox;
		        FO[fox].x = fo_space;
			fvx = FD[dev].fvx;
			fxj = fox;
                        for (j = 0; j < FO[fox].x; j++) {
			    ++fxj;
			    while (FV[fvx].type != INOUT &&
			           FV[fvx].type != OUTPUT &&
				   FO[fxj].type == NOTYPE) fvx++;
			    if (FO[fxj].type == NOTYPE) {
				if (FV[fvx].ind[0] == 0) {
				    FO[fxj].type = FV[fvx].type;
				}
				else if (FV[fvx].ind[1] == 0) {
				   for (i = 0; i < FV[fvx].ind[0]; ++i) {
				      FO[fxj+i].type = FV[fvx].type;
				   }
				}
				else {
				  for (i = 0; i < FV[fvx].ind[0] * FV[fvx].ind[1]; ++i) {
				      FO[fxj+i].type = FV[fvx].type;
				  }
				}
				fvx++;
			    }

			    FO[fxj].trise = (trise > 0)? trise : 0;
			    FO[fxj].tfall = (tfall > 0)? tfall : 0;
                        }
                    }
                    else {
		        F[x].fox = -1;
                    }

                    if (fs_space > 0) {
                        fsx = newfs (fs_space) - (fs_space - 1);
		        F[x].fsx = fsx;
                        if (FD[dev].offsx != os_cnt + is_cnt + rs_cnt + ps_cnt) {
			    fprintf (stderr, "Check error on FD[].offsx\n");
			    die (1);
			}

                        osx = fsx;
                        isx = osx + os_cnt;
                        rsx = isx + is_cnt;
                        psx = rsx + rs_cnt;
                        ssx = psx + ps_cnt;

                        if (ps_cnt > 0) {
                            FS[psx] = '*';
                            psx += SIZE_PTR_INT;
                            if (psx % SIZE_PTR_INT != 0)
                                psx += SIZE_PTR_INT - psx % SIZE_PTR_INT;
                            /* to be machine independent */
                            pc = FS + psx;
                            *(int *)pc = (ps_cnt - 3 * SIZE_PTR_INT) / SIZE_PTR_INT;
                            psx += SIZE_PTR_INT;
                        }

                        fvx = FD[dev].fvx;
                        for (i = 0; i < FD[dev].fvx_cnt; i++) {
                            switch (FV[fvx].type) {
                                case INPUT_T :
                                    if (FV[fvx].ind[0] == 0) {
                                        FS[isx++] = 'X';
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) FS[isx++] = 'X';
                                        FS[isx++] = '\0';
                                    }
                                    else {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = isx + j * (FV[fvx].ind[1] + 1) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) FS[isx++] = 'X';
                                            FS[isx++] = '\0';
                                        }
                                    }
                                    break;
                                case INPUT_R :
                                    if (FV[fvx].ind[0] == 0) {
                                        FS[rsx++] = 'X';
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) FS[rsx++] = 'X';
                                        FS[rsx++] = '\0';
                                    }
                                    else {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = rsx + j * (FV[fvx].ind[1] + 1) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) FS[rsx++] = 'X';
                                            FS[rsx++] = '\0';
                                        }
                                    }
                                    break;
                                case INOUT :
                                case OUTPUT :
                                    if (FV[fvx].ind[0] == 0) {
                                        FS[osx++] = 'X';
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) FS[osx++] = 'X';
                                        FS[osx++] = '\0';
                                    }
                                    else {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = osx + j * (FV[fvx].ind[1] + 1) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) FS[osx++] = 'X';
                                            FS[osx++] = '\0';
                                        }
                                    }
                                    break;
                                case CHAR :
                                    if (FV[fvx].ind[0] == 0) {
                                        FS[ssx++] = 'X';
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) FS[ssx++] = 'X';
                                        FS[ssx++] = '\0';
                                    }
                                    else {
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = ssx + j * (FV[fvx].ind[1] + 1) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) FS[ssx++] = 'X';
                                            FS[ssx++] = '\0';
                                        }
                                    }
                                    break;
                                case INTEGER :
                                    if (FV[fvx].ind[0] == 0) {
                                        ssx += sizeof (int);
                                        if (ssx % sizeof (int) != 0)
                                            ssx += sizeof (int) - ssx % sizeof (int);
                                        pc = FS + ssx;
                                        *(int *)pc = 0;
                                        ssx += sizeof (int);
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        ssx += sizeof (int);
                                        if (ssx % sizeof (int) != 0)
                                            ssx += sizeof (int) - ssx % sizeof (int);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + ssx;
                                            *(int *)pc = 0;
                                            ssx += sizeof (int);
                                        }
                                    }
                                    else {
                                        ssx += sizeof (int);
                                        if (ssx % sizeof (int) != 0)
                                            ssx += sizeof (int) - ssx % sizeof (int);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = ssx + sizeof (int) + j * (FV[fvx].ind[1] + 1) * sizeof (int) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) {
                                                pc = FS + ssx;
                                                *(int *)pc = 0;
                                                ssx += sizeof (int);
                                            }
                                        }
                                    }
                                    break;
                                case FLOAT :
                                    if (FV[fvx].ind[0] == 0) {
                                        ssx += sizeof (float);
                                        if (ssx % sizeof (float) != 0)
                                            ssx += sizeof (float) - ssx % sizeof (float);
                                        pc = FS + ssx;
                                        *(float *)pc = 0.0;
                                        ssx += sizeof (float);
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        ssx += sizeof (float);
                                        if (ssx % sizeof (float) != 0)
                                            ssx += sizeof (float) - ssx % sizeof (float);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + ssx;
                                            *(float *)pc = 0.0;
                                            ssx += sizeof (float);
                                        }
                                    }
                                    else {
                                        ssx += sizeof (float);
                                        if (ssx % sizeof (float) != 0)
                                            ssx += sizeof (float) - ssx % sizeof (float);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = ssx + sizeof (float) + j * (FV[fvx].ind[1] + 1) * sizeof (float) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++){
                                                pc = FS + ssx;
                                                *(float *)pc = 0.0;
                                                ssx += sizeof (float);
                                            }
                                        }
                                    }
                                    break;
                                case DOUBLE :
                                    if (FV[fvx].ind[0] == 0) {
                                        ssx += sizeof (double);
                                        if (ssx % sizeof (double) != 0)
                                            ssx += sizeof (double) - ssx % sizeof (double);
                                        pc = FS + ssx;
                                        *(double *)pc = 0.0;
                                        ssx += sizeof (double);
                                    }
                                    else if (FV[fvx].ind[1] == 0) {
                                        ssx += sizeof (double);
                                        if (ssx % sizeof (double) != 0)
                                            ssx += sizeof (double) - ssx % sizeof (double);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + ssx;
                                            *(double *)pc = 0.0;
                                            ssx += sizeof (double);
                                        }
                                    }
                                    else {
                                        ssx += sizeof (double);
                                        if (ssx % sizeof (double) != 0)
                                            ssx += sizeof (double) - ssx % sizeof (double);
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            pc = FS + psx;
                                            *(int *)pc = ssx + sizeof (double) + j * (FV[fvx].ind[1] + 1) * sizeof (double) - psx;
                                            psx += SIZE_PTR_INT;
                                        }
                                        for (j = 0; j < FV[fvx].ind[0]; j++) {
                                            for (k = 0; k < FV[fvx].ind[1]; k++) {
                                                pc = FS + ssx;
                                                *(double *)pc = 0.0;
                                                ssx += sizeof (double);
                                            }
                                        }
                                    }
                                    break;
                             }
                             fvx++;
                        }

                    }
                    else {
		        F[x].fsx = -1;
                    }

		    F[x].type = dev;

                    if (first == TRUE) {
			NT[ntx].x = x;
			first = FALSE;
		    }
		}

                break;
        }
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp, COMPLETE);
}
