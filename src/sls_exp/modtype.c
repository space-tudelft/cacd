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

extern int newfd (void);
extern int newfv (void);
extern char *next_attr (char **p, char **v, char *a);
extern int strcmp_quick (char *s1, char *s2);

int devtype (char *name)
{
    int hval = name[0] + 100 * name[1];

    switch (hval) {
	case 'n' + 100 * 'e' :
	    if (strcmp_quick (name, "nenh") == 0)
		return (D_NENH);
	    else
		return (-1);
	case 'p' + 100 * 'e' :
	    if (strcmp_quick (name, "penh") == 0)
		return (D_PENH);
	    else
		return (-1);
	case 'n' + 100 * 'd' :
	    if (strcmp_quick (name, "ndep") == 0)
		return (D_DEPL);
	    else
		return (-1);
	case 'r' + 100 * 'e' :
	    if (strcmp_quick (name, "res") == 0)
		return (D_RES);
	    else
		return (-1);
	case 'c' + 100 * 'a' :
	    if (strcmp_quick (name, "cap") == 0)
		return (D_CAP);
	    else
		return (-1);
	default :
	    return (-1);
    }
}

int stdfunctype (char *name)
{
    int hval = name[0] + 100 * name[1];

    switch (hval) {
	case 'o' + 100 * 'r' :
	    if (strcmp_quick (name, "or") == 0)
		return (F_OR);
	    else
		return (-1);
	case 'a' + 100 * 'n' :
	    if (strcmp_quick (name, "and") == 0)
		return (F_AND);
	    else
		return (-1);
	case 'n' + 100 * 'o' :
	    if (strcmp_quick (name, "nor") == 0)
		return (F_NOR);
	    else
		return (-1);
	case 'n' + 100 * 'a' :
	    if (strcmp_quick (name, "nand") == 0)
		return (F_NAND);
	    else
		return (-1);
	case 'e' + 100 * 'x' :
	    if (strcmp_quick (name, "exor") == 0)
		return (F_EXOR);
	    else
		return (-1);
	case 'i' + 100 * 'n' :
	    if (strcmp_quick (name, "invert") == 0)
		return (F_INVERT);
	    else
		return (-1);
	default :
	    return (-1);
    }
}

/* returns index of function description in FD[]
 * (reads in in FD[] when not present in FD[])
 * in t the last modification time of the function will be put
 */
int functype (char *name, int imported, time_t *t, char *otherproject)
{
    char attribute_string[256];
    long lower[10], upper[10];
    int i;
    int item;
    int fdx;
    int fvx;
    int nbr;
    int low;
    int up;
    char svar[NAMESIZE + 1];
    int ind0;
    int ind1;
    char stype;
    DM_CELL * key;
    DM_STREAM * dsp;
    char * attribute;
    char * parname;
    char * parval;
    struct stat buf;
    int os_cnt;
    int is_cnt;
    int rs_cnt;
    int ps_cnt;
    DM_PROJECT * real_projkey;
    char * real_name;
    char * ftermname;

    for (i = 0; i < FD_cnt; i++) {
        if (strcmp_quick (FD[i].name, name) == 0)
            break;
    }

    if (i == FD_cnt) {

        if (sizeof (char) != 1) {
            fprintf (stderr, "Sorry, compiler problem, incorrect sizeof char\n");
            die (1);
        }

	real_projkey = dmFindProjKey (imported, name, dmproject, &real_name, viewtype);

        if (_dmExistCell (real_projkey, real_name, viewtype) != 1) {
	    if (otherproject) {
		real_projkey = dmOpenProject (otherproject, PROJ_READ);
		real_name = name;
	    }
	    else
		return (-1);    /* function is not in the database */
	}

        key = dmCheckOut (real_projkey, real_name, ACTUAL, DONTCARE, viewtype, READONLY);

        *t = 0;

        ftermname = "fterm";

        if (dmStat (key, ftermname, &buf) == 0) {
            if (buf.st_mtime > *t) *t = buf.st_mtime;
        }
	else {
            fprintf (stderr, "Sorry, cannot stat %s\n", ftermname);
	    return (-1);   /* apparently the circuit view does only
			      contain a real circuit and no function */
        }

        if (dmStat (key, sls_o_fn, &buf) == 0) {
            if (buf.st_mtime > *t) *t = buf.st_mtime;
        }
	else {
            fprintf (stderr, "Sorry, cannot stat %s\n", sls_o_fn);
	    return (-1);
        }

        if (*t > newest_ftime) newest_ftime = *t;

        dsp = dmOpenStream (key, ftermname, "r");

        fdx = newfd ();
        strncpy (FD[fdx].name, name, NAMESIZE - 1);
        FD[fdx].fvx = -1;
        FD[fdx].fvx_cnt = 0;
        FD[fdx].help = 0;

        if (strlen (real_projkey -> dmpath) > MAXDMPATH) {
            fprintf (stderr, "Sorry, dmpath to function too long\n");
            die (1);
	}
	strcpy (FD[fdx].dmpath, real_projkey -> dmpath);

	dm_get_do_not_alloc = 1;
	cterm.term_attribute = attribute_string;
	cterm.term_lower = lower;
	cterm.term_upper = upper;

        item = 0;
        while (dmGetDesignData (dsp, CIR_TERM) > 0) {
	    item++;

            fvx = newfv ();

            if (FD[fdx].fvx_cnt++ == 0) FD[fdx].fvx = fvx;

            FV[fvx].type = INPUT_T;  /* default */
            FV[fvx].help = 0;

            attribute = cterm.term_attribute;
	    while (attribute) {
	        attribute = next_attr (&parname, &parval, attribute);
                if (parval == NULL) parval = "";
	        /* to prevent core dump when database error */

                if (strcmp_quick (parname, "ftt") == 0) {
		    switch (parval[0]) {
                        case '0': FV[fvx].type = OUTPUT;  break;
                        case '1': FV[fvx].type = INPUT_T; break;
                        case '2': FV[fvx].type = INPUT_R; break;
                        case '3': FV[fvx].type = INOUT;   break;
                        default :
		            dberror (name, item, "unknown function terminal type", NULL);
		    }
	        }
	    }

            strncpy (FV[fvx].name, cterm.term_name, NAMESIZE - 1);

	    nbr = 1;
            if (cterm.term_dim > 0) {
                for (i = 0; i < cterm.term_dim; i++) {
                    if (i >= 2)
		        dberror (name, item, "dimension function terminal more than 2", NULL);
                    low = cterm.term_lower[i];
                    up = cterm.term_upper[i];
                    if (low != 0)
		        dberror (name, item, "function terminal lower index must be 0", NULL);
                    FV[fvx].ind[i] = up + 1;
		    nbr = nbr * (up + 1);
	        }
            }
            else
                FV[fvx].ind[0] = 0;
        }

	dm_get_do_not_alloc = 0;
        dmCloseStream (dsp, COMPLETE);

        if (dmStat (key, "fstate", &buf) == 0) {

            dsp = dmOpenStream (key, "fstate", "r");

            while (fscanf (dsp -> dmfp, "%s %d %d", svar, &ind0, &ind1) == 3) {

		while ((stype = getc (dsp -> dmfp)) == ' ');

                fvx = newfv ();

                if (FD[fdx].fvx_cnt++ == 0) FD[fdx].fvx = fvx;

                strncpy (FV[fvx].name, svar, NAMESIZE - 1);
                FV[fvx].help = 0;

                switch (stype) {
                    case 'c': FV[fvx].type = CHAR;   break;
                    case 'i': FV[fvx].type = INTEGER; break;
                    case 'f': FV[fvx].type = FLOAT;  break;
                    case 'd': FV[fvx].type = DOUBLE; break;
                    default :
		        dberror ("state description", -1, "unknown state type", NULL);
                }
                FV[fvx].ind[0] = ind0;
                FV[fvx].ind[1] = ind1;
            }

            dmCloseStream (dsp, COMPLETE);
        }

	os_cnt = 0;
	is_cnt = 0;
	rs_cnt = 0;
	ps_cnt = 0;

	fvx = FD[fdx].fvx;
	for (i = 0; i < FD[fdx].fvx_cnt; i++) {
	    switch (FV[fvx].type) {
		case INPUT_T :
		    if (FV[fvx].ind[0] == 0) {
			is_cnt++;
		    }
		    else if (FV[fvx].ind[1] == 0) {
			is_cnt += FV[fvx].ind[0] + 1;
		    }
		    else {
			is_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
			ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
		    }
		    break;
		case INPUT_R :
		    if (FV[fvx].ind[0] == 0) {
			rs_cnt++;
		    }
		    else if (FV[fvx].ind[1] == 0) {
			rs_cnt += FV[fvx].ind[0] + 1;
		    }
		    else {
			rs_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
			ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
		    }
		    break;
		case INOUT :
		case OUTPUT :
		    if (FV[fvx].ind[0] == 0) {
			os_cnt++;
		    }
		    else if (FV[fvx].ind[1] == 0) {
			os_cnt += FV[fvx].ind[0] + 1;
		    }
		    else {
			os_cnt += FV[fvx].ind[0] * (FV[fvx].ind[1] + 1);
			ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
		    }
		    break;
		case CHAR :
		    if (FV[fvx].ind[0] == 0) {
		    }
		    else if (FV[fvx].ind[1] == 0) {
		    }
		    else {
			ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
		    }
		    break;
		case INTEGER :
		case FLOAT :
		case DOUBLE :
		    if (FV[fvx].ind[0] == 0) {
		    }
		    else if (FV[fvx].ind[1] == 0) {
		    }
		    else {
			ps_cnt += FV[fvx].ind[0] * SIZE_PTR_INT;
		    }
		    break;
	    }
	    fvx++;
	}

	if (ps_cnt > 0) ps_cnt += 3 * SIZE_PTR_INT;

	FD[fdx].offsx = os_cnt + is_cnt + rs_cnt + ps_cnt;

        /* dmCheckIn is postponed until the end of the program */

        return (fdx);
    }
    else
        return (i);
}

int strcmp_quick (char *s1, char *s2)
{
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) { s1++; s2++; }
    if (*s1 == '\0' && *s2 == '\0')
        return (0);
    else
        return (1);
}
