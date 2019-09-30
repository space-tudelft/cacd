
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Frederik Beeftink
 *	Peter Elias
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/xnle/incl.h"

extern DM_PROJECT *dmproject;

extern long *Nil;
extern long nodeCounter;

extern double tox;

extern char *snbulk, *spbulk;
extern char **globNets;
#ifdef ADDGLOBALNETS
extern char **globConA;
extern char **globConB;
#endif
extern int globNets_cnt;

#ifdef __cplusplus
  extern "C" {
#endif
static void prPar  (struct model_info *mod);
#ifdef __cplusplus
  }
#endif

void prInst (struct model_info *ntw, struct net_ref *nets)
{
    DM_STREAM *dsp;
    long i, j;
    long lower[10], upper[10];
    long xv[10];
    long xvt[10];
    char buf[512];
    char attribute_string[512];
    char *actual_name;
    struct net_el tnetspace;
    struct net_el *tnet, *fnet, *n;
    struct net_ref *nref;
    struct term_ref *tref;
    struct model_info *mod;
#ifdef ADDGLOBALNETS
    int globCon_cnt;
    int globalTerm;
#endif
    int imported;
    int mtype;
    int ready;
    int inst_index = 0;

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

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_ap ()) continue;

	imported = cmc.imported;

#ifdef ADDGLOBALNETS
	globCon_cnt = 0;
#endif

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

	tnet -> inst_dim = cmc.inst_dim;
	for (i = 0; i < cmc.inst_dim; i++) xv[i] = cmc.inst_lower[i];

        ready = 0;
        while (!ready) {

	    if (mtype == 'd' && *mod -> prefix) {
		sprintf (buf, "I $%s %d * |", mod -> prefix, ++inst_index);
		oprint (0, buf);
		prPar (mod);
	    }
	    else {
		if (mod -> imported == IMPORTED) {
		    oprint (0, "I MDE ");
		    oprint (0, mod -> orig_name);
		    oprint (0, " ");
		    oprint (0, projname (mod -> proj));
		}
		else {
		    oprint (0, "I NLE ");
		    oprint (0, mod -> orig_name);
		    oprint (0, " *");
		}
		sprintf (buf, " * %d", ++inst_index);
		oprint (0, buf);
		nmprint (0, cmc.inst_name, cmc.inst_dim, xv, Nil, 0);
	    }
	    oprint (0, " |");

	    for (tref = mod -> terms; tref; tref = tref -> next) {

		tnet -> net_name = tref -> t -> term_name;
		tnet -> net_dim  = tref -> t -> term_dim;
		for (j = tnet -> net_dim; --j >= 0;) xvt[j] = tref -> t -> term_lower[j];

		do {
#ifdef ADDGLOBALNETS
		    globalTerm = 0;
		    if (tnet -> net_dim == 0) {
			for (i = 0; i < globNets_cnt; i++) {
			    if (strcmp (globNets[i], tnet -> net_name) == 0) {
				globalTerm = 1;
				break;
			    }
			}
		    }
#endif /* ADDGLOBALNETS */

		    fnet = findNet (tnet);

		    if (!fnet) {
			P_E "Warning: %s: node '%s.", mod -> orig_name,
			    makeArrayName (0, cmc.inst_name, cmc.inst_dim, xv));
			P_E "%s' not connected!\n",
			    makeArrayName (1, tnet -> net_name, tnet -> net_dim, tnet -> net_lower));
			i = nodeCounter++;
		    }
		    else i = fnet -> nx;
		    sprintf (buf, " %ld", i);
		    oprint (0, buf);

		    for (j = tnet -> net_dim; --j >= 0;) {
			if (tref -> t -> term_lower[j] <= tref -> t -> term_upper[j]) {
			    if (++xvt[j] <= tref -> t -> term_upper[j]) break;
			}
			else {
			    if (--xvt[j] >= tref -> t -> term_upper[j]) break;
			}
			xvt[j] = tref -> t -> term_lower[j];
		    }
		} while (j >= 0);
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

	    if (mtype == 'd' && mod -> prefix[0] == 'T') {
		i = 0;
		switch (mod -> prefix[1]) {
		case 'e':
		case 'h':
		    if ((i = testNameNbr (snbulk)) < 0) i = 0;
		    break;
		case 'i':
		case 'p':
		    if ((i = testNameNbr (spbulk)) < 0) i = 0;
		    break;
		}
		if (i == 0) {
		    P_E "Warning: %s: node '%s.b' not connected!\n", mod -> orig_name,
			makeArrayName (0, cmc.inst_name, cmc.inst_dim, xv));
		}
		sprintf (buf, " %ld", i);
		oprint (0, buf);
	    }
	    oprint (0, ";\n");
	}

#ifdef ADDGLOBALNETS
	for (i = 0; i < globCon_cnt; i++) {

	    /* generate a connection between actual parameter
	       and global net since the formal parameter is
	       a global net
	    */
	    nmprint (0, globConA[i], 0L, Nil, Nil, 0);
	    oprint (0, " ");
	    nmprint (0, globConB[i], 0L, Nil, Nil, 0);
	    oprint (1, "\n");
	}
#endif /* ADDGLOBALNETS */
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
    double vval, lval, wval, xval, yval;

    vval = 0;
    lval = 0;
    wval = 0;
    xval = 0;
    yval = 0;

    attr = cmc.inst_attribute;

    if (!attr || !*attr) {
	fatalErr ("No device attributes found!", 0);
	return;
    }

    while (attr) {
	attr = nextAttr (&par, &val, attr);

	if (par && *par && !par[1]) {
	    switch (*par) {
		case 'l': lval = atof (val); break;
		case 'v': vval = atof (val); break;
		case 'w': wval = atof (val); break;
		case 'x': xval = atof (val); break;
		case 'y': yval = atof (val);
	    }
	}
nxt_attr:
	if (val) *(val - 1) = '=';
	if (attr) *(attr - 1) = ';';
    }

    switch (mod -> prefix[0]) {
    case 'C':
	sprintf (buf, " %g", vval * 1e12); /* pF */
	break;
    case 'L':
    case 'R':
	sprintf (buf, " %g", vval); /* ohms (henries) */
	break;
    case 'T':
	sprintf (buf, " %g %g",
	    wval * 1e6 / dmproject -> lambda,
	    lval * 1e6 / dmproject -> lambda);
	oprint (0, buf);
	wval *= lval * 3.9 * 8.85 / tox; /* pF */
	sprintf (buf, " %g [%g,%g]", wval, xval, yval);
	break;
    case 'D':
	sprintf (buf, " %g 0", /* lambda-area,-perimeter */
	    vval * 1e6 / dmproject -> lambda);
	break;
    default:
	fatalErr ("Unknown device prefix!", 0);
    }
    oprint (0, buf);
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
    sprintf (buf, "_%s", p);
    return (buf);
}
