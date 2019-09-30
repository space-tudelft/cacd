/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Frederik Beeftink
 *	Peter Elias
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 *	Bastiaan Sneeuw
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

#include "src/xedif/incl.h"
#include "src/xedif/search.h"

extern DM_PROJECT *dmproject;
extern DM_PROJECT *currproj;

extern long *Nil;
extern int dialectCds;
extern int tog_irange;
extern int tog_srange;

struct inst_ref *Inst_list = NULL;
static struct inst_ref *Inst_list_last = NULL;
static struct inst_ref *Inst_list_old = NULL;

static int mtype;
static char fv_ret[32];
static char fv_tmp[32];

#ifdef __cplusplus
  extern "C" {
#endif
static void prPar  (struct model_info *mod);
static void prInNm (struct model_info *mod, long *lower, long *upper);
static char *fvalPar (char *par, char *val);
#ifdef __cplusplus
  }
#endif

void prInst (struct model_info *ntw, struct net_ref *nets)
{
    DM_STREAM *dsp;
    long i;
    long lower[10], upper[10];
    long xv[10], xvt[10];
    char attribute_string[512];
    struct net_el tnetspace;
    struct net_el *tnet;
    struct model_info *mod;
    int firstinst;
    int ready;
    int imported;

    tnet = &tnetspace;
    tnet -> net_dim = 0;
    tnet -> inst_dim = 0;
    tnet -> net_lower = xvt;
    tnet -> inst_lower = xv;
    tnet -> inst_name = cmc.inst_name;

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    dsp = dmOpenStream (ntw -> dkey, "mc", "r");

    oprint (0, "(contents");
    if (dialectCds > 1) oprint (0, "(page SH1"); /* Schematic */

    if (Inst_list) {
	Inst_list_last -> next = Inst_list_old;
	Inst_list_old = Inst_list;
	Inst_list = NULL;
	resetStringSpace (1);
	resetIndexSpace (1);
    }

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_ap ()) continue;

	imported = cmc.imported;

	if (is_func ()) {
	    mtype = 'f';			/* Function */
	    mod = findFunc (cmc.cell_name, imported, ntw -> proj, 1);
	    mod -> imported = (ntw -> imported || imported);
	}
        else if ((mod = findDev (cmc.cell_name, imported, ntw -> proj))) {
	    mtype = 'd';			/* Device */
	    if (mod -> proj)
		mod -> imported = (ntw -> imported || imported);
	}
	else {
	    mtype = 'n';			/* Network */
	    mod = findNetw (cmc.cell_name, imported, ntw -> proj, 1);
	    mod -> imported = (ntw -> imported || imported);
	}

	if (tog_srange) {
	    struct inst_ref *iref;

	    if (Inst_list_old) {
		iref = Inst_list_old;
		Inst_list_old = Inst_list_old -> next;
	    }
	    else {
		PALLOC (iref, 1, struct inst_ref);
	    }
	    iref -> inst_name = newStringSpace (cmc.inst_name, 1);
	    if (cmc.inst_dim > 0) {
		iref -> inst_dim = cmc.inst_dim;
		iref -> inst_lower = newIndexSpace (cmc.inst_dim, 1);
		for (i = 0; i < cmc.inst_dim; i++) {
		    if (cmc.inst_upper[i] < cmc.inst_lower[i])
			iref -> inst_lower[i] = cmc.inst_upper[i];
		    else
			iref -> inst_lower[i] = cmc.inst_lower[i];
		}
	    }
	    else iref -> inst_dim = 0;
	    iref -> terms = mod -> terms;
	    iref -> next  = Inst_list;
	    if (!Inst_list) Inst_list_last = iref;
	    Inst_list = iref;
	}

	tnet -> inst_dim = cmc.inst_dim;
	for (i = 0; i < cmc.inst_dim; i++) xv[i] = cmc.inst_lower[i];

        firstinst = 1;
        ready = 0;
        while (!ready) {

            if (!tog_irange || firstinst) {

		if (!tog_irange) {
		    prInNm (mod, xv, xv);
		}
		else {
		    firstinst = 0;
		    prInNm (mod, cmc.inst_lower, cmc.inst_upper);
		}

		if (dialectCds) {
		    oprint (0, "(viewRef symbol(cellRef ");
		    oprint (0, mod -> out_name);
		    if (mod -> lib_name) {
			oprint (0, "(libraryRef ");
			oprint (0, mod -> lib_name);
			oprint (0, ")");
		    }
		}
		else {
		    oprint (0, "(viewRef VIEWNAMEDEFAULT(cellRef ");
		    oprint (0, mod -> orig_name);
		    if (mtype == 'd' || mtype == 'f') {
			oprint (0, "(libraryRef Primitives)");
		    }
		    else if (mtype == 'n' && mod -> proj != currproj) {
			oprint (0, "(libraryRef ");
			oprint (0, projname (mod -> proj));
			oprint (0, ")");
		    }
		}
		oprint (0, "))");
		prPar (mod);
	    }

	    ready = 1;
	    for (i = cmc.inst_dim - 1; i >= 0; i--) {
		if (cmc.inst_lower[i] <= cmc.inst_upper[i]) {
		    if (++xv[i] <= cmc.inst_upper[i]) {
			ready = 0;
			break;
		    }
		}
		else {
		    if (--xv[i] >= cmc.inst_upper[i]) {
			ready = 0;
			break;
		    }
		}
		xv[i] = cmc.inst_lower[i];
	    }

	    if (ready || !tog_irange) oprint (0, ")");
	}
    }

    dmCloseStream (dsp, COMPLETE);
}

int is_ap ()
{
    /* EM: Check if name is "$'torname'$ds" */
    if ((cmc.cell_name[0] == DSCAP_PREFIX) &&
	(!strcmp (cmc.cell_name + strlen (cmc.cell_name)
		- strlen (DSCAP_SUFFIX), DSCAP_SUFFIX)))
	return (7);

    return (0);
}

static void prPar (struct model_info *mod)
{
    char buf[128];
    char *attr, *par, *val;
    int pr_val;
    int cds_Use = 0;

    attr = cmc.inst_attribute;

    if (!attr || !*attr) return;

    while (attr) {
	attr = nextAttr (&par, &val, attr);

	if (par && *par) {

	    pr_val = 0;

	    switch (mtype) {
		case 'f':
		    if (*par == 'f' && !par[1]) goto nxt_attr;
		    if (*par == 't' && (par[1] == 'r' || par[1] == 'f') && !par[2]) {
			/* tr, tf */
			pr_val = 1;
		    }
		    break;
		case 'd':
		    if (!par[1]) {
			if (*par == 'w' || *par == 'l' || *par == 'v') pr_val = 1;
		    }
		    else {
			if ((!par[2] && (*par == 'a' || *par == 'p')
			    && (par[1] == 'd' || par[1] == 's'))
			    || (*par == 'n' && par[1] == 'r'
				&& (par[2] == 'd' || par[2] == 's') && !par[3])) {
			    /* ad, as, pd, ps, nrd, nrs */
			    /* pr_val = 1; */
			}
			else if (!par[2] && ((*par == 'w' && par[1] == 'b') ||
			((*par == 'a' || *par == 'p') && par[1] == 'e'))) {
			    /* wb, ae, pe */
			    /* pr_val = 1; */
			}
		    }
		    break;
		case 'n':
		    if (*par == 'f' && !par[1]) goto nxt_attr;
	    }

	    /* (Added by B.Sneeuw) When the -c and the -I options are set it is
	    * necessary to place the instances according to the Schematic format.
	    * By using Space with the -t option the coordinates of the instances
	    * in the layout are stored and can be used to place the instances
	    * in the Schematic. These coordinates are stored using an 'x' and
	    * an 'y' coordinate. Assumed is that the characters x and y are
	    * only used for the coordinates of the place of the instance.
	    *
	    * If the -c and -I options are set and the name of a parameter is 'x'
	    * or 'y' then the special format is used according to Cadence Edif (
	    * the cds_Use bit is then set to 1). Else the usual way of printing
	    * names with values is used using parameterAssign.
	    */
	    if (dialectCds > 1) { /* Schematic */
		if (strcmp (par, "x") == 0) {
		    oprint (0, "(transform (origin (pt ");
		    oprint (0, val);
		    oprint (0, " ");
		    cds_Use = 1;
		}
		if (strcmp (par, "y") == 0) {
		    oprint (0, val);
		    oprint (0, ")))");
		    cds_Use = 1;
		}
	    }
	    if (!cds_Use) {
		oprint (0, "(parameterAssign ");
		oprint (0, par);
		if (val) {
		    oprint (0, "(number");
		    oprint (0, fvalPar (par, val));
		    oprint (0, "))");
		}
		else oprint (0, ")");
		pr_val = 0;
	    }
	    if (pr_val) {
		if (val) {
		    oprint (1, "=");
		    oprint (1, fvalPar (par, val));
		}
	    }
	}
nxt_attr:
	if (val) *(val - 1) = '=';
	if (attr) *(attr - 1) = ';';
    }
}

static void prInNm (struct model_info *mod, long *lower, long *upper)
{
    if (dialectCds) {
	ENTRY item;
	item.key = strsave (cmc.inst_name);
	item.data = (char *)mod;
	hsearch (item, ENTER);
    }
    oprint (0, "(instance ");
    nmprint (0, cmc.inst_name, cmc.inst_dim, lower, upper, tog_irange);
}

/* fvalPar prints a parameters value by using an S.I.
 * scale factor if possible
 */
static char *fvalPar (char *par, char *val)
{
    char *pv, *pt;
    int ncnt, exp, negative;

    pv = val;
    pt = fv_tmp;

    if (*pv == '-') {
	negative = 1;
	++pv;
    }
    else
	negative = 0;

    ncnt = -1; /* number of digits after '.' */
    while ((*pv >= '0' && *pv <= '9') || *pv == '.') {
	if (*pv == '.') {
	    ncnt = 0;
	}
	else {
	    *pt++ = *pv;
	    if (ncnt >= 0) ncnt++;
	}
	pv++;
    }
    *pt = '\0';
    if (ncnt < 0) ncnt = 0;

    exp = 0;
    if (*pv == 'e' || *pv == 'E') {
	if (sscanf (pv + 1, "%d", &exp) != 1) exp = 0;
    }

    if (exp) {
	if (negative)
	    sprintf (fv_ret, "(e -%s %d)", fv_tmp, exp - ncnt);
	else
	    sprintf (fv_ret, "(e %s %d)", fv_tmp, exp - ncnt);
    }
    else {
	    sprintf (fv_ret, " %s", val);
    }
    return (fv_ret);
}

char *projname (DM_PROJECT *proj)
{
    static char buf[164];
    char *s, *p;

    if (!proj) fatalErr ("projname: projkey == NULL", 0);
    s = proj -> dmpath;
    p = s;
    while (*p) if (*p++ == '/') s = p;
    if (!*s) s = "root";

    p = maplibname (s);
    if (isalpha ((int)*p)) return (p);
    sprintf (buf, "&%s", p);
    return (buf);
}
