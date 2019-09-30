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

extern int  yy1lineno;
extern int  yy1parse (void);
static void calcdeplpar (DEPLPARAMS *dptel, DEPLSPECIFS *spec);
static void calcenhpar (ENHPARAMS *eptel, ENHSPECIFS *spec);
static float fvallw (SPECIF *par, float l, float w);
static float fvalx (FTAB_EL ftab[], float x);
static void miserror (void);

extern DM_PROJECT * dmproject;
extern FILE * yy1in;

int     nbrcalc;

ENHSPECIFS nenhspec;
ENHSPECIFS penhspec;
DEPLSPECIFS deplspec;

int     currttype;

void getproc ()
{
    int     missingdim;
    int     cnt;
    float   l, w;
    TRANSISTOR * t;

    ENHPARAMS * eptsearch;
    ENHPARAMS * eptprev;
    ENHPARAMS * eptnew;
    ENHPARAMS * eptel;
    ENHPARAMS * epthelp;
    DEPLPARAMS * dptsearch;
    DEPLPARAMS * dptprev;
    DEPLPARAMS * dptnew;
    DEPLPARAMS * dptel;
    DEPLPARAMS * dpthelp;

    ENHPARAMS * nenhpartab;
    ENHPARAMS * penhpartab;
    DEPLPARAMS * deplpartab;

    char *fn1;
    char *fn2;
    char fn3[200];

    if (fn_proc == NULL) {
	if ((yy1in = fopen ("slsmod", "r")) == NULL) {
	    if ((fn1 = (char *)dmGetMetaDesignData (
				 PROCPATH, dmproject, "slsmod")) == NULL) {
		die (1);
	    }
	    OPENR (yy1in, fn1);
	}
	fn_proc = "slsmod"; /* for error messages */
    }
    else {
	if ((yy1in = fopen (fn_proc, "r")) == NULL) {
	    if ((fn2 = (char *)dmGetMetaDesignData (
				 PROCPATH, dmproject, "slsmod")) == NULL) {
		die (1);
	    }
	    strcpy (fn3, fn2);
	    strcpy (fn3 + strlen (fn3) - strlen ("slsmod"), fn_proc);
	    if ((yy1in = fopen (fn3, "r")) == NULL) {
		OPENR (yy1in, fn_proc); /* forces an error message */
	    }
	}
    }

    yy1lineno = 1;
    yy1parse ();

    CLOSE (yy1in);

    nenhpartab = NULL;
    penhpartab = NULL;
    deplpartab = NULL;

    missingdim = FALSE;

    for (cnt = 0; cnt < T_cnt; cnt++) {
	t = &T[cnt];

	if (t -> type == Res) continue;

	if (t -> length <= 0 || t -> width <= 0) {
	    t -> length = 4e-6;
	    t -> width = 4e-6;
	    if (!missingdim) {
                slserror (NULL, 0, WARNING, "network contains transistors without correct dimensions\n", "(w=l=4u assumed)");
		missingdim = TRUE;
	    }
	}

	l = t -> length;
	w = t -> width;

	if (t -> type == Nenh || t -> type == Penh) {
	    switch (t -> type) {
		case Nenh:
		    eptsearch = nenhpartab;
		    break;
		case Penh:
		    eptsearch = penhpartab;
		    break;
		default:
		    eptsearch = NULL;
		    ERROR_EXIT(1);
	    }
	    eptprev = NULL;

	    while (eptsearch != NULL && eptsearch -> width < w) {
		eptprev = eptsearch;
		eptsearch = eptsearch -> nextw;
	    }

	    if (eptsearch == NULL || eptsearch -> width > w) {
		PALLOC (eptnew, 1, ENHPARAMS);
		eptnew -> length = l;
		eptnew -> width = w;
		eptnew -> nextw = eptsearch;
		eptnew -> nextl = NULL;
		if (eptprev == NULL) {
		    switch (t -> type) {
			case Nenh:
			    nenhpartab = eptnew;
			    break;
			case Penh:
			    penhpartab = eptnew;
			    break;
		    }
		}
		else
		    eptprev -> nextw = eptnew;
	    }
	    else {
		eptprev = NULL;
		while (eptsearch != NULL
			&& eptsearch -> length < l) {
		    eptprev = eptsearch;
		    eptsearch = eptsearch -> nextl;
		}

		if (eptsearch == NULL) {
		    PALLOC (eptnew, 1, ENHPARAMS);
		    eptnew -> length = l;
		    eptnew -> width = w;
		    eptnew -> nextl = NULL;
		    eptprev -> nextl = eptnew;
		}
		else
		    if (eptsearch -> length > l) {
			PALLOC (eptnew, 1, ENHPARAMS);
			eptnew -> length = eptsearch -> length;
			eptnew -> width = eptsearch -> width;
			eptnew -> nextl = eptsearch -> nextl;
			eptsearch -> length = l;
			eptsearch -> width = w;
			eptsearch -> nextl = eptnew;
		    }
	    }
	}
	else
	    if (t -> type == Depl) {
		dptsearch = deplpartab;
		dptprev = NULL;

		while (dptsearch != NULL && dptsearch -> width < w) {
		    dptprev = dptsearch;
		    dptsearch = dptsearch -> nextw;
		}

		if (dptsearch == NULL || dptsearch -> width > w) {
		    PALLOC (dptnew, 1, DEPLPARAMS);
		    dptnew -> length = l;
		    dptnew -> width = w;
		    dptnew -> nextw = dptsearch;
		    dptnew -> nextl = NULL;
		    if (dptprev == NULL) {
			deplpartab = dptnew;
		    }
		    else
			dptprev -> nextw = dptnew;
		}
		else {
		    dptprev = NULL;
		    while (dptsearch != NULL
			    && dptsearch -> length < l) {
			dptprev = dptsearch;
			dptsearch = dptsearch -> nextl;
		    }

		    if (dptsearch == NULL) {
			PALLOC (dptnew, 1, DEPLPARAMS);
			dptnew -> length = l;
			dptnew -> width = w;
			dptnew -> nextl = NULL;
			dptprev -> nextl = dptnew;
		    }
		    else
			if (dptsearch -> length > l) {
			    PALLOC (dptnew, 1, DEPLPARAMS);
			    dptnew -> length = dptsearch -> length;
			    dptnew -> width = dptsearch -> width;
			    dptnew -> nextl = dptsearch -> nextl;
			    dptsearch -> length = l;
			    dptsearch -> width = w;
			    dptsearch -> nextl = dptnew;
			}
		}
	    }
    }

    for (cnt = 0; cnt < T_cnt; cnt++) {
	t = &T[cnt];

	if (t -> type == Res)
	    continue;

	l = t -> length;
	w = t -> width;

	if (t -> type == Nenh) {
	    eptel = nenhpartab;
	    while (w > eptel -> width)
		eptel = eptel -> nextw;
	    while (l > eptel -> length)
		eptel = eptel -> nextl;
	    t -> attr = (int *) eptel;
	}
	else
	    if (t -> type == Penh) {
		eptel = penhpartab;
		while (w > eptel -> width)
		    eptel = eptel -> nextw;
		while (l > eptel -> length)
		    eptel = eptel -> nextl;
		t -> attr = (int *) eptel;
	    }
	    else
		if (t -> type == Depl) {
		    dptel = deplpartab;
		    while (w > dptel -> width)
			dptel = dptel -> nextw;
		    while (l > dptel -> length)
			dptel = dptel -> nextl;
		    t -> attr = (int *) dptel;
		}
    }

    nbrcalc = 0;

    currttype = Nenh;
    eptel = nenhpartab;
    while (eptel != NULL) {
	epthelp = eptel;
	while (eptel != NULL) {
	    calcenhpar (eptel, &nenhspec);
	    eptel = eptel -> nextl;
	}
	eptel = epthelp -> nextw;
    }

    currttype = Penh;
    eptel = penhpartab;
    while (eptel != NULL) {
	epthelp = eptel;
	while (eptel != NULL) {
	    calcenhpar (eptel, &penhspec);
	    eptel = eptel -> nextl;
	}
	eptel = epthelp -> nextw;
    }

    currttype = Depl;
    dptel = deplpartab;
    while (dptel != NULL) {
	dpthelp = dptel;
	while (dptel != NULL) {
	    calcdeplpar (dptel, &deplspec);
	    dptel = dptel -> nextl;
	}
	dptel = dpthelp -> nextw;
    }

    if (debugsim)
	fprintf (debug, "nbr of different transistor sizes = %d\n\n", nbrcalc);
}

static void calcenhpar (ENHPARAMS *eptel, ENHSPECIFS *spec)
{
    float l = eptel -> length;
    float w = eptel -> width;

    if (!spec -> tspecdef) miserror ();

    eptel -> general.rstat = fvallw (&spec -> general.rstat, l, w);
    eptel -> general.rsatu = fvallw (&spec -> general.rsatu, l, w);
    eptel -> general.cgstat = fvallw (&spec -> general.cgstat, l, w);
    eptel -> general.cgrise = fvallw (&spec -> general.cgrise, l, w);
    eptel -> general.cgfall = fvallw (&spec -> general.cgfall, l, w);
    eptel -> general.cestat = fvallw (&spec -> general.cestat, l, w);
    eptel -> general.cerise = fvallw (&spec -> general.cerise, l, w);
    eptel -> general.cefall = fvallw (&spec -> general.cefall, l, w);
    eptel -> pullup.rdyn = fvallw (&spec -> pullup.rdyn, l, w);
    eptel -> pullup.cch = fvallw (&spec -> pullup.cch, l, w);
    eptel -> pulldown.rdyn = fvallw (&spec -> pulldown.rdyn, l, w);
    eptel -> pulldown.cch = fvallw (&spec -> pulldown.cch, l, w);
    eptel -> passup.rdyn = fvallw (&spec -> passup.rdyn, l, w);
    eptel -> passup.cch = fvallw (&spec -> passup.cch, l, w);
    eptel -> passdown.rdyn = fvallw (&spec -> passdown.rdyn, l, w);
    eptel -> passdown.cch = fvallw (&spec -> passdown.cch, l, w);

    nbrcalc++;
}

static void calcdeplpar (DEPLPARAMS *dptel, DEPLSPECIFS *spec)
{
    float l = dptel -> length;
    float w = dptel -> width;

    if (!spec -> tspecdef) miserror ();

    dptel -> general.rstat = fvallw (&spec -> general.rstat, l, w);
    dptel -> general.rsatu = fvallw (&spec -> general.rsatu, l, w);
    dptel -> general.cgstat = fvallw (&spec -> general.cgstat, l, w);
    dptel -> general.cgrise = fvallw (&spec -> general.cgrise, l, w);
    dptel -> general.cgfall = fvallw (&spec -> general.cgfall, l, w);
    dptel -> general.cestat = fvallw (&spec -> general.cestat, l, w);
    dptel -> general.cerise = fvallw (&spec -> general.cerise, l, w);
    dptel -> general.cefall = fvallw (&spec -> general.cefall, l, w);
    dptel -> load.rdyn = fvallw (&spec -> load.rdyn, l, w);
    dptel -> load.cch = fvallw (&spec -> load.cch, l, w);
    dptel -> superload.rdyn = fvallw (&spec -> superload.rdyn, l, w);
    dptel -> superload.cch = fvallw (&spec -> superload.cch, l, w);

    nbrcalc++;
}

static float fvallw (SPECIF *par, float l, float w)
{
    float val, vall, vallmin;

    if (l == par -> lmin)
	val = fvalx (par -> wftab, w);
    else
	if (w == par -> wmin)
	    val = fvalx (par -> lftab, l);
	else {
	    val = fvalx (par -> wftab, w);
	    vallmin = fvalx (par -> lftab, par -> lmin);
	    vall = fvalx (par -> lftab, l);

	    if (vallmin != 0)
		val = val * (vall / vallmin);
	    else
		val = 0;
	}

    if (l - par -> loffset <= 0)
        slserror (fn_proc, 0, ERROR1, "loffset too large", NULL);

    switch (par -> ldepend) {
	case NOT:
	    break;
	case LINEAIR:
	    val = val * (l - par -> loffset);
	    break;
	case INVERSE:
	    val = val / (l - par -> loffset);
	    break;
    }

    if (w - par -> woffset <= 0)
        slserror (fn_proc, 0, ERROR1, "woffset too large", NULL);

    switch (par -> wdepend) {
	case NOT:
	    break;
	case LINEAIR:
	    val = val * (w - par -> woffset);
	    break;
	case INVERSE:
	    val = val / (w - par -> woffset);
	    break;
    }

    return (val);
}

static float fvalx (FTAB_EL ftab[], float x)
{
    int     n, m, firsttime;
    float   tn;
    float   val = 0;
    float   mintn = 0;
    float   maxtn = 0;

    firsttime = TRUE;
    for (n = 0; ftab[n].x >= 0; n++) {
	tn = ftab[n].fx;
	if (tn < mintn || firsttime) mintn = tn;
	if (tn > maxtn || firsttime) maxtn = tn;
	firsttime = FALSE;

	for (m = 0; ftab[m].x >= 0; m++) {
	    if (m != n)
		tn = tn * (x - ftab[m].x) / (ftab[n].x - ftab[m].x);
	}

	val = val + tn;
    }

    if (val < mintn) val = mintn;
    if (val > maxtn) val = maxtn;

    if (n == 0) miserror ();

    return (val);
}

static void miserror ()
{
    char type[6];

    switch (currttype) {
	case Nenh:
	    sprintf (type, "nenh");
	    break;
	case Penh:
	    sprintf (type, "penh");
	    break;
	case Depl:
	    sprintf (type, "ndep");
	    break;
	default:
	    ERROR_EXIT(1);
    }
    slserror (NULL, 0, ERROR1, "process specification(s) missing for", type);
}

float enhpar (TRANSISTOR *trans, int mode, int par)
{
    ENHPARAMS * enhparblock;
    MODEPARAMS * modeparblock;
    float   val;

    enhparblock = (ENHPARAMS *) (trans -> attr);

    if (mode == 0) {
	switch (par) {
	    case Rstat:
		val = enhparblock -> general.rstat;
		break;
	    case Rsatu:
		val = enhparblock -> general.rsatu;
		break;
	    case Cgstat:
		val = enhparblock -> general.cgstat;
		break;
	    case Cgrise:
		val = enhparblock -> general.cgrise;
		break;
	    case Cgfall:
		val = enhparblock -> general.cgfall;
		break;
	    case Cestat:
		val = enhparblock -> general.cestat;
		break;
	    case Cerise:
		val = enhparblock -> general.cerise;
		break;
	    case Cefall:
		val = enhparblock -> general.cefall;
		break;
	    default:
		val = 0;
		ERROR_EXIT(1);
	}
    }
    else {
	switch (mode) {
	    case Pullup:
		modeparblock = &enhparblock -> pullup;
		break;
	    case Pulldown:
		modeparblock = &enhparblock -> pulldown;
		break;
	    case Passup:
		modeparblock = &enhparblock -> passup;
		break;
	    case Passdown:
		modeparblock = &enhparblock -> passdown;
		break;
	    default:
		modeparblock = 0;
		ERROR_EXIT(1);
	}

	switch (par) {
	    case Rdyn:
		val = modeparblock -> rdyn;
		break;
	    case Cch:
		val = modeparblock -> cch;
		break;
	    default:
		val = 0;
		ERROR_EXIT(1);
	}
    }

    return (val);
}

float deplpar (TRANSISTOR *trans, int mode, int par)
{
    DEPLPARAMS * deplparblock;
    MODEPARAMS * modeparblock;
    float   val;

    deplparblock = (DEPLPARAMS *) (trans -> attr);

    if (mode == 0) {
	switch (par) {
	    case Rstat:
		val = deplparblock -> general.rstat;
		break;
	    case Rsatu:
		val = deplparblock -> general.rsatu;
		break;
	    case Cgstat:
		val = deplparblock -> general.cgstat;
		break;
	    case Cgrise:
		val = deplparblock -> general.cgrise;
		break;
	    case Cgfall:
		val = deplparblock -> general.cgfall;
		break;
	    case Cestat:
		val = deplparblock -> general.cestat;
		break;
	    case Cerise:
		val = deplparblock -> general.cerise;
		break;
	    case Cefall:
		val = deplparblock -> general.cefall;
		break;
	    default:
		val = 0;
		ERROR_EXIT(1);
	}
    }
    else {
	switch (mode) {
	    case Load:
		modeparblock = &deplparblock -> load;
		break;
	    case Superload:
		modeparblock = &deplparblock -> superload;
		break;
	    default:
		modeparblock = 0;
		ERROR_EXIT(1);
	}

	switch (par) {
	    case Rdyn:
		val = modeparblock -> rdyn;
		break;
	    case Cch:
		val = modeparblock -> cch;
		break;
	    default:
		val = 0;
		ERROR_EXIT(1);
	}
    }
    return (val);
}

float respar (TRANSISTOR *trans, int mode, int par)
{
    float val = 0;

    if (mode == 0 && (par == Rdyn || par == Rlin || par == Rstat || par == Rsatu)) {
	val = trans -> length;
    }
    return (val);
}
