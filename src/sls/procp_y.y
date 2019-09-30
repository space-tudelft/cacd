%{
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

extern void yy1error (char *s);
extern int  yy1lex   (void);
extern int  yy1parse (void);
static void ftabstore (SPECIF *sp, char o, double fval);

#undef MAXINT
#undef MAXLONG

extern ENHSPECIFS nenhspec;
extern ENHSPECIFS penhspec;
extern DEPLSPECIFS deplspec;

float c_l;
float c_lmin;
float c_loffset;
float c_w;
float c_wmin;
float c_woffset;
int lftab_ind;
int wftab_ind;
GENSPECIFS * c_genspblock;
MODESPECIFS * c_modespblock;
ENHSPECIFS * c_enhspblock;
DEPLSPECIFS * c_deplspblock;

char * proc_err_list[] = {
	"too much different values for w or l now",
	"w and l not consistant with wmin and lmin"
	};

#ifdef YYBISON
extern int yy1lineno;  /* exported from LEX output */
#endif

%}

%union
{
	char * sval;
	int ival;
	double fval;
}

%token <sval> INT
%token <fval> FLO

%token VH VSWITCH VMINH VMAXL KRISE KFALL NENH PENH NDEP END
%token PULLUP PULLDOWN PASSUP PASSDOWN LOAD
%token SUPERLOAD RSTAT RSATU CGSTAT CGRISE CGFALL
%token CESTAT CERISE CEFALL RDYN CCH
%token LOFFSET WOFFSET L W EQUAL COLON

/*
%type <ival> integer
*/
%type <fval> real

%start proc_desc

%%

proc_desc       : lvolt_desc k_desc nenh_desc penh_desc depl_desc
		;

lvolt_desc      : VH EQUAL real
		  vswitch_descr
		  VMINH EQUAL real
		  VMAXL EQUAL real
		{
			if (vHtmp < 0) vHtmp = $3;
			if (vminH < 0) vminH = $7;
			if (vmaxL < 0) vmaxL = $10;
		}
		;

vswitch_descr	: VSWITCH EQUAL real
		{
			vswitch = $3;
		}
		| /* empty */
		;

k_desc		: KRISE EQUAL real
		  KFALL EQUAL real
		{
			krise = $3;
			kfall = $6;
		}
		;

nenh_desc	: /* empty */
		{
			nenhspec.tspecdef = FALSE;
		}
		| NENH
		{
			nenhspec.tspecdef = TRUE;
			c_enhspblock = &nenhspec;
		}
		  enhspecifs END
		;

penh_desc	: /* empty */
		{
			penhspec.tspecdef = FALSE;
		}
		| PENH
		{
			penhspec.tspecdef = TRUE;
			c_enhspblock = &penhspec;
		}
		  enhspecifs END
		;

depl_desc       :/* empty */
		{
			deplspec.tspecdef = FALSE;
		}
		| NDEP
		{
			deplspec.tspecdef = TRUE;
			c_deplspblock = &deplspec;
		}
		  deplspecifs END
		;

enhspecifs	:
		{
			c_genspblock = &c_enhspblock->general;
		}
		  genspecs PULLUP
		{
			c_modespblock = &c_enhspblock->pullup;
		}
		  modespecs END PULLDOWN
		{
			c_modespblock = &c_enhspblock->pulldown;
		}
		  modespecs END PASSUP
		{
			c_modespblock = &c_enhspblock->passup;
		}
		  modespecs END PASSDOWN
		{
			c_modespblock = &c_enhspblock->passdown;
		}
		  modespecs END
		;

deplspecifs	:
		{
			c_genspblock = &c_deplspblock->general;
		}
		  genspecs LOAD
		{
			c_modespblock = &c_deplspblock->load;
		}
		  modespecs END SUPERLOAD
		{
			c_modespblock = &c_deplspblock->superload;
		}
		  modespecs END
		;

genspecs	: LOFFSET EQUAL real
		  WOFFSET EQUAL real
		{
			c_loffset = $3;
			c_woffset = $6;
		}
		  gstats
		{
			c_genspblock->rstat.lftab[lftab_ind].x = -1;
			c_genspblock->rstat.wftab[wftab_ind].x = -1;
			c_genspblock->rsatu.lftab[lftab_ind].x = -1;
			c_genspblock->rsatu.wftab[wftab_ind].x = -1;
			c_genspblock->cgstat.lftab[lftab_ind].x = -1;
			c_genspblock->cgstat.wftab[wftab_ind].x = -1;
			c_genspblock->cgrise.lftab[lftab_ind].x = -1;
			c_genspblock->cgrise.wftab[wftab_ind].x = -1;
			c_genspblock->cgfall.lftab[lftab_ind].x = -1;
			c_genspblock->cgfall.wftab[wftab_ind].x = -1;
			c_genspblock->cestat.lftab[lftab_ind].x = -1;
			c_genspblock->cestat.wftab[wftab_ind].x = -1;
			c_genspblock->cerise.lftab[lftab_ind].x = -1;
			c_genspblock->cerise.wftab[wftab_ind].x = -1;
			c_genspblock->cefall.lftab[lftab_ind].x = -1;
			c_genspblock->cefall.wftab[wftab_ind].x = -1;
		}
		;

gstats		: /* empty */
		{
			lftab_ind = 0;
			wftab_ind = 0;
			c_genspblock->rstat.ldepend = LINEAIR;
			c_genspblock->rstat.wdepend = INVERSE;
			c_genspblock->rstat.loffset = c_loffset;
			c_genspblock->rstat.woffset = c_woffset;
			c_genspblock->rsatu.ldepend = LINEAIR;
			c_genspblock->rsatu.wdepend = INVERSE;
			c_genspblock->rsatu.loffset = c_loffset;
			c_genspblock->rsatu.woffset = c_woffset;
			c_genspblock->cgstat.ldepend = LINEAIR;
			c_genspblock->cgstat.wdepend = LINEAIR;
			c_genspblock->cgstat.loffset = c_loffset;
			c_genspblock->cgstat.woffset = c_woffset;
			c_genspblock->cgrise.ldepend = LINEAIR;
			c_genspblock->cgrise.wdepend = LINEAIR;
			c_genspblock->cgrise.loffset = c_loffset;
			c_genspblock->cgrise.woffset = c_woffset;
			c_genspblock->cgfall.ldepend = LINEAIR;
			c_genspblock->cgfall.wdepend = LINEAIR;
			c_genspblock->cgfall.loffset = c_loffset;
			c_genspblock->cgfall.woffset = c_woffset;
			c_genspblock->cestat.ldepend = NOT;
			c_genspblock->cestat.wdepend = LINEAIR;
			c_genspblock->cestat.loffset = c_loffset;
			c_genspblock->cestat.woffset = c_woffset;
			c_genspblock->cerise.ldepend = NOT;
			c_genspblock->cerise.wdepend = LINEAIR;
			c_genspblock->cerise.loffset = c_loffset;
			c_genspblock->cerise.woffset = c_woffset;
			c_genspblock->cefall.ldepend = NOT;
			c_genspblock->cefall.wdepend = LINEAIR;
			c_genspblock->cefall.loffset = c_loffset;
			c_genspblock->cefall.woffset = c_woffset;
		}
		| gstats gstat
		;

gstat 		: dimstat COLON
		  RSTAT EQUAL real
		  RSATU EQUAL real
		  CGSTAT EQUAL real
		  CGRISE EQUAL real
		  CGFALL EQUAL real
		  CESTAT EQUAL real
		  CERISE EQUAL real
		  CEFALL EQUAL real
		{
		    if (lftab_ind == 0 && wftab_ind == 0) {
			c_genspblock->rstat.lmin = c_l;
			c_genspblock->rstat.wmin = c_w;
			c_genspblock->rsatu.lmin = c_l;
			c_genspblock->rsatu.wmin = c_w;
			c_genspblock->cgstat.lmin = c_l;
			c_genspblock->cgstat.wmin = c_w;
			c_genspblock->cgrise.lmin = c_l;
			c_genspblock->cgrise.wmin = c_w;
			c_genspblock->cgfall.lmin = c_l;
			c_genspblock->cgfall.wmin = c_w;
			c_genspblock->cestat.lmin = c_l;
			c_genspblock->cestat.wmin = c_w;
			c_genspblock->cerise.lmin = c_l;
			c_genspblock->cerise.wmin = c_w;
			c_genspblock->cefall.lmin = c_l;
			c_genspblock->cefall.wmin = c_w;
			c_lmin = c_l;
			c_wmin = c_w;
			ftabstore (&c_genspblock->rstat, 'l', $5);
			ftabstore (&c_genspblock->rstat, 'w', $5);
			ftabstore (&c_genspblock->rsatu, 'l', $8);
			ftabstore (&c_genspblock->rsatu, 'w', $8);
			ftabstore (&c_genspblock->cgstat, 'l', $11);
			ftabstore (&c_genspblock->cgstat, 'w', $11);
			ftabstore (&c_genspblock->cgrise, 'l', $14);
			ftabstore (&c_genspblock->cgrise, 'w', $14);
			ftabstore (&c_genspblock->cgfall, 'l', $17);
			ftabstore (&c_genspblock->cgfall, 'w', $17);
			ftabstore (&c_genspblock->cestat, 'l', $20);
			ftabstore (&c_genspblock->cestat, 'w', $20);
			ftabstore (&c_genspblock->cerise, 'l', $23);
			ftabstore (&c_genspblock->cerise, 'w', $23);
			ftabstore (&c_genspblock->cefall, 'l', $26);
			ftabstore (&c_genspblock->cefall, 'w', $26);
			lftab_ind++;
			wftab_ind++;
		    }
		    else if (c_l == c_lmin && c_w > c_wmin) {
			if (wftab_ind >= MAXFTAB)
			    slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[0], NULL);
			ftabstore (&c_genspblock->rstat, 'w', $5);
			ftabstore (&c_genspblock->rsatu, 'w', $8);
			ftabstore (&c_genspblock->cgstat, 'w', $11);
			ftabstore (&c_genspblock->cgrise, 'w', $14);
			ftabstore (&c_genspblock->cgfall, 'w', $17);
			ftabstore (&c_genspblock->cestat, 'w', $20);
			ftabstore (&c_genspblock->cerise, 'w', $23);
			ftabstore (&c_genspblock->cefall, 'w', $26);
			wftab_ind++;
		    }
		    else if (c_w == c_wmin && c_l > c_lmin) {
			if (lftab_ind >= MAXFTAB)
			    slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[0], NULL);
			ftabstore (&c_genspblock->rstat, 'l', $5);
			ftabstore (&c_genspblock->rsatu, 'l', $8);
			ftabstore (&c_genspblock->cgstat, 'l', $11);
			ftabstore (&c_genspblock->cgrise, 'l', $14);
			ftabstore (&c_genspblock->cgfall, 'l', $17);
			ftabstore (&c_genspblock->cestat, 'l', $20);
			ftabstore (&c_genspblock->cerise, 'l', $23);
			ftabstore (&c_genspblock->cefall, 'l', $26);
			lftab_ind++;
		    }
		    else {
			slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[1], NULL);
		    }
		}
		;

modespecs	: mstats
		{
			c_modespblock->rdyn.lftab[lftab_ind].x = -1;
			c_modespblock->rdyn.wftab[wftab_ind].x = -1;
			c_modespblock->cch.lftab[lftab_ind].x = -1;
			c_modespblock->cch.wftab[wftab_ind].x = -1;
		}
		;

mstats		: /* empty */
		{
			lftab_ind = 0;
			wftab_ind = 0;
			c_modespblock->rdyn.ldepend = LINEAIR;
			c_modespblock->rdyn.wdepend = INVERSE;
			c_modespblock->rdyn.loffset = c_loffset;
			c_modespblock->rdyn.woffset = c_woffset;
			c_modespblock->cch.ldepend = LINEAIR;
			c_modespblock->cch.wdepend = LINEAIR;
			c_modespblock->cch.loffset = c_loffset;
			c_modespblock->cch.woffset = c_woffset;
		}
		| mstats mstat
		;

mstat 		: dimstat COLON
		  RDYN EQUAL real
		  CCH EQUAL real
		{
		    if (lftab_ind == 0 && wftab_ind == 0) {
			c_modespblock->rdyn.lmin = c_l;
			c_modespblock->rdyn.wmin = c_w;
			c_modespblock->cch.lmin = c_l;
			c_modespblock->cch.wmin = c_w;
			c_lmin = c_l;
			c_wmin = c_w;
			ftabstore (&c_modespblock->rdyn, 'l', $5);
			ftabstore (&c_modespblock->rdyn, 'w', $5);
			ftabstore (&c_modespblock->cch, 'l', $8);
			ftabstore (&c_modespblock->cch, 'w', $8);
			lftab_ind++;
			wftab_ind++;
		    }
		    else if (c_l == c_lmin && c_w > c_wmin) {
			if (wftab_ind >= MAXFTAB)
			    slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[0], NULL);
			ftabstore (&c_modespblock->rdyn, 'w', $5);
			ftabstore (&c_modespblock->cch, 'w', $8);
			wftab_ind++;
		    }
		    else if (c_w == c_wmin  && c_l > c_lmin) {
			if (lftab_ind >= MAXFTAB)
			    slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[0], NULL);
			ftabstore (&c_modespblock->rdyn, 'l', $5);
			ftabstore (&c_modespblock->cch, 'l', $8);
			lftab_ind++;
		    }
		    else {
			slserror (fn_proc, yy1lineno, ERROR1, proc_err_list[1], NULL);
		    }
		}
		;

dimstat		: wstat lstat
		| lstat wstat
		;

wstat		: W EQUAL real
		{
			c_w = $3;
			if (c_w >= 0.1) c_w = c_w * 1e-6;  /* micro unit */
		}
		;

lstat		: L EQUAL real
		{
			c_l = $3;
			if (c_l >= 0.1) c_l = c_l * 1e-6;  /* micro unit */
		}
		;
/*
integer		: INT
		{
			$$ = atoi($1);
		}
		;
*/
real		: INT
		{
			$$ = atoi($1);
		}
		| FLO
		{
			$$ = $1;
		}
		;
%%
#include "procp_l.h"

static void ftabstore (SPECIF *sp, char o, double fval)
{
	FTAB_EL * ftab;
	int ind;
	float dim;
	float sfval = fval;

	if (o == 'l') {
	    ftab = sp->lftab;
	    ind = lftab_ind;
	    dim = c_l;
	}
	else { // 'w'
	    ftab = sp->wftab;
	    ind = wftab_ind;
	    dim = c_w;
	}

	switch (sp->ldepend) {
		case NOT :
			break;
		case LINEAIR :
			sfval = sfval / (c_l - sp->loffset);
			break;
		case INVERSE :
			sfval = sfval * (c_l - sp->loffset);
			break;
	}

	switch (sp->wdepend) {
		case NOT :
			break;
		case LINEAIR :
			sfval = sfval / (c_w - sp->woffset);
			break;
		case INVERSE :
			sfval = sfval * (c_w - sp->woffset);
			break;
	}

	(ftab + ind)->x = dim;
	(ftab + ind)->fx = sfval;
}

void yy1error (char *s)
{
    slserror (fn_proc, yy1lineno, ERROR1, s, NULL);
}

int yy1wrap ()
{
    return (1);
}
