
/*
 * ISC License
 *
 * Copyright (C) 1987-2016 by
 *	Frederik Beeftink
 *	Peter Elias
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Edwin Matthijssen (EM)
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

#include "src/xspice/incl.h"

extern DM_PROJECT *dmproject;

extern char *NBULK, *PBULK;
extern long *Nil;
extern int add_comma;
extern int add_paren;
extern int out_indent;
extern int dialect;
extern int sameline;
extern int tanner;
extern int tog_pnod;
extern int tog_nnod;
extern int tog_nodnbr;
extern int tog_use0;
extern int tog_use0_save;
extern int noUnconnect;
extern int useDBinames;
extern int use_Pmodels;
extern int use_Qmodels;
extern int saveInOutBuf;

extern float  vmbulk;
extern struct node_info *nodetab;

static struct cell_par *mod_pars;

static long uncon_cnt;
static long r_cnt;
static long c_cnt;
static long x_cnt;
static long m_cnt;
static long q_cnt;
static long f_cnt;
static long d_cnt;

#ifdef XPSTAR
static int addcomma;
#endif
static int llmodel;
static int mtype;
static int prefix;

static char fv_ret[32];
static char fv_tmp[32];
char *model_sf;

#ifdef __cplusplus
  extern "C" {
#endif
static void prAP (long dN_nx, long sN_nx, char *ia);
static void prPar (struct model_info *mod, int mode);
static void prPar2 (struct model_info *mod, int mode);
static void prTanner (struct net_el *fnet);
static double getVal (char *par);
static double getAreaVal (void);
static double getPerimVal (void);
static void prInNm (struct model_info *mod, long *lower, long *upper);
static void cutTrail0s (char *str);
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
    int firstinode;
    int ready;
    char str[32];
    char buf[512];
    char attribute_string[512];
    char *actual_name;
    struct net_el tnetspace;
    struct net_el *tnet, *fnet;
    struct term_ref *tref;
    struct model_info *mod;
    long dN_nx, sN_nx;
    int is_ap_mos;
    int is_caps;
    int prPnod;
    int prNnod;
    int has_connect;
    int valPrinted;
    int imported;
    int bulkDone;

    uncon_cnt= 0;
    r_cnt= 0;
    c_cnt= 0;
    x_cnt= 0;
    m_cnt= 0;
    q_cnt= 0;
    f_cnt= 0;
    d_cnt= 0;

    tnet = &tnetspace;
    tnet -> net_dim = 0;
    tnet -> inst_dim = 0;
    tnet -> net_lower = xvt;
    tnet -> inst_lower = xv;
    tnet -> inst_name = cmc.inst_name;

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    is_caps = 0;

    /* EM: always ap_info, for nodetab see also xnetwork.c
    */
    if ((dsp = dmOpenStream (ntw -> dkey, "mc", "r"))) {

	while (dmGetDesignData (dsp, CIR_MC) > 0) {
	    if (!*cmc.inst_name) fatalErr ("internal error", "no inst_name");
	    if (is_ap ()) {
		is_caps = 1;
		if (cmc.inst_dim != 0) fatalErr ("internal error", "on inst_dim");
                tnet -> net_name = "n";
		if ((fnet = findNet (tnet))) {
		    char *torname = buf;
		    strcpy (torname, cmc.cell_name + 1);
		    torname[strlen (cmc.cell_name) - 4] = '\0';
		    addAP (fnet->nx, torname, getAreaVal (), getPerimVal ());
		}
	    }
	    else if (cmc.inst_name[0] == '_' && cmc.inst_name[1] == 'T') {
		double gatewidth = getVal ("w");
		if (cmc.inst_dim != 0) fatalErr ("internal error", "on inst_dim");
		tnet -> net_name = "s";
		if ((fnet = findNet (tnet))) {
		    addCntWidth (fnet->nx, cmc.cell_name, gatewidth);
		}
		tnet -> net_name = "d";
		if ((fnet = findNet (tnet))) {
		    addCntWidth (fnet->nx, cmc.cell_name, gatewidth);
		}
	    }
	}
        dmCloseStream (dsp, COMPLETE);
    }

    dsp = dmOpenStream (ntw -> dkey, "mc", "r");

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_caps) {
	    if (is_ap ()) continue;
	    is_ap_mos = (cmc.inst_name[0] == '_' && cmc.inst_name[1] == 'T');
	}
	else is_ap_mos = 0;

	dN_nx = sN_nx = 0;

	imported = cmc.imported;
	has_connect = 0;
	if (noUnconnect) saveInOutBuf = 1;

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

	if (mtype == 'd') {
	    model_sf = NULL;
	    mod_pars = mod -> pars; /* default model params */
	    if (mod -> createName) {
		struct lib_model *lm = createModel (mod);
		if (lm && lm -> pars) mod_pars = lm -> pars;
	    }
	}
	else mod_pars = NULL;

        ready = 0;
        while (!ready) {

	    firstinode = 1;
#ifdef XPSTAR
	    out_indent = 2;
#else
	    out_indent = 0;
#endif
	    prInNm (mod, xv, xv); /* => prefix */

	    if ((out_indent = outPos ()) > 24) out_indent = 24;

	    if (mtype == 'n' && (tog_pnod || tog_nnod)) {

		prPnod = tog_pnod;
		prNnod = tog_nnod;

		for (tref = mod -> terms; tref; tref = tref -> next) {
		    if (strcmp (tref -> t -> term_name, PBULK) == 0) {
			if (!prNnod) goto nxt;
			prPnod = 0;
		    }
		    if (strcmp (tref -> t -> term_name, NBULK) == 0) {
			if (!prPnod) goto nxt;
			prNnod = 0;
		    }
		}

		if (prPnod) {
		    firstinode = 0;
		    nmprint (0, PBULK, 0L, Nil, Nil, 0, tog_nodnbr);
		}
		if (prNnod) {
		    if (firstinode) firstinode = 0;
		    else {
			if (add_comma) oprint (1, ",");
			oprint (0, " ");
		    }
		    nmprint (0, NBULK, 0L, Nil, Nil, 0, tog_nodnbr);
		}
	    }
nxt:
	    bulkDone = 0;

	    for (tref = mod -> terms; tref; tref = tref -> next) {

		tnet -> net_name = tref -> t -> term_name;
		tnet -> net_dim  = tref -> t -> term_dim;
		for (j = tnet -> net_dim; --j >= 0;) xvt[j] = tref -> t -> term_lower[j];

		if (tnet -> net_name[0] == mod -> lookForBulk) {
		    ASSERT (tnet -> net_dim == 0);
		    ASSERT (tref -> next == NULL);
		    bulkDone = 1;
		}

		do {
		    if (firstinode) firstinode = 0;
		    else {
			if (add_comma) oprint (1, ",");
			oprint (0, " ");
		    }

		    fnet = findNet (tnet);

		    if (!fnet) {
		      if (!bulkDone) {
			sprintf (str, "xxx_%ld", ++uncon_cnt);
			tog_use0 = 0;
			nmprint (0, str, 0L, Nil, Nil, 0, tog_nodnbr);
			tog_use0 = tog_use0_save;
		      }
		    }
		    else {
			if (noUnconnect) {
			    if (fnet -> net_neqv > 1) has_connect = 1;
			    else
			    if (nodetab[fnet->nx].isTerm) has_connect = 1;
			    else
			    if (fnet -> inst_name) has_connect = 1; // ??
			}

		    if (fnet -> inst_name) { /* first net is pin of instance */
			/* fnet -> net_name is pin name */
			/* use node number */
			sprintf (str, "%ld", fnet->nx);
			tog_use0 = 0;
			nmprint (0, str, 0L, Nil, Nil, 0, tog_nodnbr);
			tog_use0 = tog_use0_save;
		    }
		    else {
			if (is_ap_mos && !tnet -> net_name[1]) {
			    if (tnet -> net_name[0] == 'd') dN_nx = fnet->nx;
			    else
			    if (tnet -> net_name[0] == 's') sN_nx = fnet->nx;
			}

			if (fnet -> net_dim == 0 && mod -> alias_s &&
			    strcmp (fnet -> net_name, mod -> alias_s) == 0) {

			    /* An alias should be used for this actual
			       terminal name. For example GND => pbulk
			       for a p diffusion capacitor */

			    actual_name = mod -> alias_t;
			}
			else
			    actual_name = fnet -> net_name;

			nmprint (0, actual_name, fnet -> net_dim, fnet -> net_lower, Nil, 0, tog_nodnbr);
			if (tanner > 1 && mtype == 'd') prTanner (fnet);
		    }
		    }

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

	    if (mod -> lookForBulk && !bulkDone) {
		str[0] = mod -> lookForBulk; str[1] = 0;
		tnet -> net_name = str;
		tnet -> net_dim = 0;
		fnet = findNet (tnet);
	    }
	    else fnet = NULL;

	    if (fnet) { /* bulk found */
		if (add_comma) oprint (1, ",");
		oprint (1, " ");
		nmprint (0, fnet -> net_name, fnet -> net_dim, fnet -> net_lower, Nil, 0, tog_nodnbr);
		if (tanner > 1 && mtype == 'd') prTanner (fnet);
	    }
	    else if (mod -> vbulkdefined) { /* mtype == 'd' */
		if (add_comma) oprint (1, ",");
		oprint (1, " ");
		nmprint (0, mod -> vbulk < vmbulk ? NBULK : PBULK, 0L, Nil, Nil, 0, tog_nodnbr);
	    }

	    if (add_paren) oprint (0, ")");

	    switch (mtype) {
		case 'f':
		case 'n':
#ifndef XPSTAR
		    prPar (mod, 0);
#endif
		/*  prPar (mod, 1);
		    prPar (mod, 2);
		    prPar (mod, 3); */
		    break;

		case 'd':

		/* Note: If there is NO out_name it must be a predef. device whereof
		 * there no out_name needs to exist (CAP, RES or IND).
		 * This decision is made in function findDev() */

		    valPrinted = 0;

		    if (mod -> out_name && prefix != 'v') { /* print model name */
#ifndef XSPECTRE
			if ((prefix == 'c' || prefix == 'r') && dialect != HSPICE && dialect != PSPICE) {
			    /*
				SPICE requires that for cap & res first the
				value is printed and then the model name.
			    */
			    prPar (mod, 1);
			    valPrinted = 1;
			}
#endif
#ifndef XPSTAR
			prPar (mod, 0);
#endif
			if (model_sf && prefix != 'x') { /* scale factor */
			    if (atof (model_sf) != 1.0) {
				struct cell_par *cp;
				for (cp = mod_pars; cp; cp = cp -> next)
				    if (cp -> val && strcmp (cp -> val, "msf") == 0) break;
				if (!cp) {
#ifdef XSPECTRE
				    char *s = "m=";
#else
				    char *s = "";
#ifdef XPSTAR
				    if (addcomma) oprint (0, ",");
#endif
				    if (dialect == HSPICE || prefix == 'm') s = "m=";
#endif
				    sprintf (buf, " %s%s", s, model_sf);
				    oprint (0, buf);
#ifdef XPSTAR
				    addcomma = 1;
#endif
				}
			    }
			    valPrinted = 1;
			}
		    }
#ifdef XSPECTRE
		    else {
			     if (prefix == 'c') oprint (0, " capacitor");
			else if (prefix == 'r') oprint (0, " resistor");
			else if (prefix == 'l') oprint (0, " inductor");
			else if (prefix == 'v') oprint (0, " vsource");
		    }
#endif
		if (prefix != 'x') {
		    if (!valPrinted) prPar (mod, 1);
		    if (is_ap_mos) {
			char *ia = cmc.inst_attribute;
			ia += strlen (ia);
			prAP (dN_nx, sN_nx, ia);
			prPar (mod, 2);
			prPar (mod, 3);
			*ia = 0;
		    }
		    else {
			prPar (mod, 2);
			prPar (mod, 3);
		    }
		}
	    }

#ifdef XPSTAR
	    addcomma = 0;
#endif
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

#ifdef XPSTAR
	    /* if use_Qmodels, no "s=1" definition (P.Elias) */
	    oprint (1, llmodel == 1 ? ", s=1;\n" : ";\n");
	    outPos ();
#else
	    oprint (0, "\n");
#endif
	    sameline = 0;
	}

	if (noUnconnect) {
	    saveInOutBuf = 0;
	    if (!has_connect)
		destroyOutBuf ();
	    else
		flushOutBuf ();
	}
    }

    out_indent = 0;

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

static int com_nr;
static char *com_ptr[20];
static char *cbufp;
static char cbuf[1024];
static int a_nr, a_nr_v, a_arr_x[5], a_arr_y[5], a_valid[5];

static void prPar (struct model_info *mod, int mode)
/* prPar has three modes:
	1. print value parameter (v=val)
	   (if not specified in the cell parameter list).
	2. print other parameters (par=val)
	3. print comment */
{
    char buf[128];
    char *s, *t, *p, *v;
    struct cell_par *cp;
    double w, W;
    int comment, i;

    if (mode == 0) { /* !PSTAR */
	for (cp = mod_pars; cp; cp = cp -> next)
	    if (cp -> val && strcmp (cp -> val, "mname") == 0) return;
	oprint (0, " ");
	oprint (0, mod -> out_name);
	return;
    }

    if (mode == 2) {
	com_nr = 0;
	cbufp = cbuf;

	for (cp = mod_pars; cp; cp = cp -> next) {
	    if ((p = cp -> par) && *p == '!') {
		if (*++p != '!') continue;
		++p;
		comment = 1;
	    }
	    else comment = 0;

	    v = NULL;
	    if (cp -> val) {
		if (*cp -> val == 'm') {
		    if (strcmp (cp -> val, "mname") == 0) {
			if (!mod -> out_name) continue;
#ifdef XPSTAR
			if (addcomma) oprint (0, ", ");
			else { oprint (0, " "); addcomma = 1; }
#else
			oprint (0, " ");
#endif
			oprint (0, mod -> out_name);
			continue;
		    }
		    if (strcmp (cp -> val, "msf") == 0) {
			v = model_sf; if (!v) v = "1";
		    }
		    else if (strcmp (cp -> val, "mdl") == 0) {
			sprintf (buf, "%e", mod -> dl); v = buf;
		    }
		    else if (strcmp (cp -> val, "mdw") == 0) {
			sprintf (buf, "%e", mod -> dw); v = buf;
		    }
		}
		if (!v) {
		    v = getAttrValue (cmc.inst_attribute, cp -> val);
		    if (!v) continue; /* attr not found */
		}
		W = atof (v);
		v = cp -> val;
	    }
	    else W = 0;

	    if ((s = cp -> par_stat)) {
		int at, m, c = *s;
		if (!v && c != '@') { c = '+'; --s; }
		while (c == '+' || c == '-' || c == '*' || c == '/') {
		    m = 0;
		    t = s++;
		    if (*s == '-') { ++s; m = 1; }
		    else if (*s == '+') ++s;
		    if (*s == '$') ++s;

		    w = 0;
		    if (isalpha ((int)*s)) {
			t = s++;
			while (isalnum ((int)*s) || *s == '_') ++s;
			at = *s; *s = 0;
			i = 0;
			if (*t == 'm') {
			    if (strcmp (t, "msf") == 0) {
				w = model_sf ? atof (model_sf) : 1;
				i = 1;
			    }
			    else if (strcmp (t, "mdl") == 0) {
				w = mod -> dl; i = 1;
			    }
			    else if (strcmp (t, "mdw") == 0) {
				w = mod -> dw; i = 1;
			    }
			}
			if (!i) {
			    t = getAttrValue (cmc.inst_attribute, t);
			    if (t) w = atof (t);
			}
			if (w == 0) t = NULL;
			*s = at;
		    }
		    else if (isdigit ((int)*s) || *s == '.') {
			w = strtod (s, &s);
		    }
		    else {
			if ((s = t) < cp -> par_stat) ++s;
			break;
		    }
		    if (t) { /* if 'w' */
			if (m) w = -w;
			if (!v) { v = "yes"; W = w; }
			else
			switch (c) {
			    case '+': W += w; break;
			    case '-': W -= w; break;
			    case '*': W *= w; break;
			    case '/': if (w) W /= w;
			}
		    }
		    c = *s;
		}
		if (c == '@') ++s;
	    }

	    if (comment) {
		if (com_nr < 20) {
		    com_ptr[com_nr++] = cbufp;
		    if (p && *p) {
			strcpy (cbufp, p);
			cbufp += strlen (cbufp);
		    }
		    if (v) {
			sprintf (buf, "%e", W);
			strcpy (cbufp, fvalPar (v, buf));
			cbufp += strlen (cbufp);
		    }
		    if (s && *s) {
			strcpy (cbufp, s);
			cbufp += strlen (s);
		    }
		    ++cbufp;
		}
	    }
	    else {
#ifdef XPSTAR
		if (addcomma) oprint (0, ", ");
		else { oprint (0, " "); addcomma = 1; }
#else
		oprint (0, " ");
#endif
		if (p) oprint (0, p);
		if (v) {
		    sprintf (buf, "%e", W);
		    oprint (0, fvalPar (v, buf));
		}
		if (s && *s) oprint (1, s);
	    }
	}
    }
    else if (mode == 3) { /* print comment */
	if (com_nr == 0 && a_nr == 0) return;

#ifdef XPSTAR
	if ((out_indent = outPos ()) > 70) out_indent = 7;
	oprint (0, " /*");
#else
	if (tanner) {
	    if (!a_nr_v) { a_nr = 0; return; }
	    sameline = 1;
	}
	else {
	    oprint (0, "\n");
	    startComment ();
	}
#endif

	if ((cbufp - cbuf) > 1020) fatalErr ("internal error:", "too small comment buf");

	for (comment = 0; comment < com_nr; ++comment) {
#ifdef XPSTAR
	    if (addcomma) oprint (1, ","); else addcomma = 1;
	    if (out_indent != 7 && (outPos() + strlen(com_ptr[comment]) > 70)) out_indent = 7;
#endif
	    sprintf (buf, " %s", com_ptr[comment]);
	    oprint (0, buf);
	}

	if (a_nr) { /* tanner > 1 */
	    for (i = a_nr_v; i <= a_nr; ++i) {
		if (a_valid[i]) {
		    sprintf (buf, " x%d=%g", i, dmproject -> lambda * a_arr_x[i]);
		    oprint (0, buf);
		    sprintf (buf, " y%d=%g", i, dmproject -> lambda * a_arr_y[i]);
		    oprint (0, buf);
		}
	    }
	    a_nr_v = a_nr = 0;
	}
#ifdef XPSTAR
        oprint (1, " */");
#endif
	return;
    }

    if (!cmc.inst_attribute || !*cmc.inst_attribute) return;

    prPar2 (mod, mode);
}

static void prPar2 (struct model_info *mod, int mode)
{
    int pr_par, pr_val, pr_com, pr_sf;
    char *attr, *par, *v, *val;
    char buf[32];

    attr = cmc.inst_attribute;

    pr_sf = 0;

    while (attr) {
	attr = nextAttr (&par, &v, attr);
	val = v;

	if (par) {

	    pr_par = 0;
	    pr_val = 0;
	    pr_com = 0;

	    switch (mtype) {

		case 'f':
		    if (*par == 'f' && !par[1]) goto nxt_attr;

		    if (*par == 't' && (par[1] == 'r' || par[1] == 'f') && !par[2]) {
			/* tr, tf */
			pr_par = pr_val = 1;
		    }
		    else pr_com = 1;
		    break;

		case 'd':
		    if (!*par)
			pr_com = 1;
		    else if (!par[1]) {
			if (*par == 'w' || *par == 'l') {
			    double vval = (*par == 'w')? mod -> dw : mod -> dl;
			    if (vval != 0) {
				vval = atof (val) - vval;
				if (vval <= 0) fatalErr ("Non-positive effective transistor dimension", 0);
				sprintf (val = buf, "%e", vval);
			    }
			    pr_par = pr_val = 1;
			}
			else if (*par == 'v') pr_val = 1;
			else if (*par == 'g') { /* convert conductance */
			    if (mode == 1) { sprintf (buf, "%e", 1 / atof (val)); val = buf; }
			    pr_val = 1;
			}
			else pr_com = 1;
		    }
		    else {
			if ((!par[2] && (*par == 'a' || *par == 'p')
			    && (par[1] == 'd' || par[1] == 's'))
			    || (*par == 'n' && par[1] == 'r'
				&& (par[2] == 'd' || par[2] == 's') && !par[3])) {
			    /* ad, as, pd, ps, nrd, nrs */
			    pr_par = pr_val = 1;
			}
			else if (!par[2] && ((*par == 'w' && par[1] == 'b') ||
			    ((*par == 'a' || *par == 'p') && par[1] == 'e'))) {
			    /* wb, ae, pe */
			    /* pr_par = pr_val = 1; */
			}
			else if (strcmp (par, "scalef") == 0) {
			    if (mode == 2) pr_sf = pr_val = 1;
			    else pr_com = 1;
			}
			else pr_com = 1;
		    }
		    break;

		case 'n':
		    if (*par == 'f' && !par[1]) goto nxt_attr;
		    pr_com = 1;
	    }

	    if ((pr_com || pr_par || pr_val) && mod_pars) {
		struct cell_par *cp;
		/*
		* We do not need to additionally print parameters that are already
		* explicitly specified in (and printed by) the cell parameter list.
		*/
		for (cp = mod_pars; cp; cp = cp -> next)
		    if (cp -> val && strcmp (cp -> val, par) == 0) goto nxt_attr;
	    }

	    if (pr_sf) {
		if (prefix == 'm' || (dialect == HSPICE && prefix != 'v')) {
		    par = "m";
		    pr_par = 1;
		}
		pr_sf = 0;
	    }

	    if (pr_par && mode == 2) {
#ifdef XPSTAR
		if (addcomma) oprint (0, ", ");
		else { oprint (0, " "); addcomma = 1; }
#else
		oprint (0, " ");
#endif
		if (llmodel != 2) oprint (0, par);
	    }

	    if (pr_val && ((pr_par && mode == 2) || (!pr_par && mode == 1))) {
		if (val) {
		    int i = 0;
		    if (pr_par) { if (llmodel != 2) oprint (++i, "="); }
#ifdef XPSTAR
		    else if (addcomma) oprint (0, ", ");
#endif
		    else {
			oprint (0, " ");
#ifdef XSPECTRE
			if (*par == 'v') {
				 if (prefix == 'c') oprint (0, "c=");
			    else if (prefix == 'r') oprint (0, "r=");
			    else if (prefix == 'l') oprint (0, "l=");
			    else if (prefix == 'v') oprint (0, "dc=");
			}
#endif
#ifdef XPSTAR
			addcomma = 1;
#endif
		    }
		    oprint (i, fvalPar (par, val));
		}
	    }

            if (pr_com && mode == 2 && com_nr < 20) {
		com_ptr[com_nr++] = cbufp;
		strcpy (cbufp, par);
		cbufp += strlen (cbufp);
		if (val) {
		    *cbufp++ = '=';
		    if (*par == 'x' || *par == 'y') {
			if (tanner && !par[1])
			    sprintf (cbufp, "%g", dmproject -> lambda * atof (val));
			else
			    strcpy (cbufp, val);
		    }
		    else
			strcpy (cbufp, fvalPar (par, val));
		    cbufp += strlen (cbufp);
		}
		++cbufp;
	    }
	}
nxt_attr:
	if (v) *(v - 1) = '=';
	if (attr) *(attr - 1) = ';';
    }
}

static void prTanner (struct net_el *fnet)
{
    if (++a_nr > 4) fatalErr ("internal error:", "incorrect number of device nodes");
    if (fnet -> x < INF_COORD) {
	a_arr_x[a_nr] = fnet -> x;
	a_arr_y[a_nr] = fnet -> y;
	a_valid[a_nr] = 1;
	if (!a_nr_v) a_nr_v = a_nr;
    }
    else
	a_valid[a_nr] = 0;
}

static double getVal (char *par)
{
    return (atof (getAttrValue (cmc.inst_attribute, par)));
}

static double getAreaVal ()
{
    return (getAttrFValues (cmc.inst_attribute, "area"));
}

static double getPerimVal ()
{
    return (getAttrFValues (cmc.inst_attribute, "perim"));
}

static void prAP (long dN_nx, long sN_nx, char *ia)
{
    long cd, cs;
    double ad, pd, wd, as, ps, ws;
    double nrs = 0.0, nrd = 0.0;
    double gatewidth;

    gatewidth = getVal ("w");

    if (dN_nx && getAP (dN_nx, cmc.cell_name, &ad, &pd, &cd, &wd)) {
	/* calculate values for this transistor */
	pd = ((2.0 * ad + gatewidth * wd) / (wd * wd + 2.0 * cd * ad)) * pd;
	nrd = ad / (wd * gatewidth);
	ad = gatewidth * ad / wd;
    }
    else dN_nx = 0;

    if (sN_nx && getAP (sN_nx, cmc.cell_name, &as, &ps, &cs, &ws)) {
	/* calculate values for this transistor */
	ps = ((2.0 * as + gatewidth * ws) / (ws * ws + 2.0 * cs * as)) * ps;
	nrs = as / (ws * gatewidth);
	as = gatewidth * as / ws;
    }
    else sN_nx = 0;

    if (dN_nx && ad  > 0.0) { sprintf (ia, ";ad=%g",  ad);  ia += strlen (ia); }
    if (sN_nx && as  > 0.0) { sprintf (ia, ";as=%g",  as);  ia += strlen (ia); }
    if (dN_nx && pd  > 0.0) { sprintf (ia, ";pd=%g",  pd);  ia += strlen (ia); }
    if (sN_nx && ps  > 0.0) { sprintf (ia, ";ps=%g",  ps);  ia += strlen (ia); }
    if (sN_nx && nrs > 0.0) { sprintf (ia, ";nrs=%g", nrs); ia += strlen (ia); }
    if (dN_nx && nrd > 0.0) { sprintf (ia, ";nrd=%g", nrd); ia += strlen (ia); }
}

static void prInNm (struct model_info *mod, long *lower, long *upper)
{
    long *acnt;
    char buf[512];
    char *val;

    tog_use0 = 0;
    llmodel = 0;

    prefix = tolower (*(mod -> prefix));

    switch (prefix) {
	case 'c': acnt = &c_cnt; break; /* capacitors */
	case 'r': acnt = &r_cnt;	/* resistors */
	    val = getAttrValue (cmc.inst_attribute, "v");
	    if (val && atof (val) == 0) prefix = 'v';
	    break;
	case 'q': acnt = &q_cnt; break; /* BJT's */
	case 'x': acnt = &x_cnt; break; /* sub-circuits */
	case 'd': acnt = &d_cnt;	/* diodes */
#ifdef XPSTAR
		if (use_Pmodels) llmodel = 3;
		else
		if (use_Qmodels) llmodel = 2;
#endif
		break;
	case 'm':			/* MOS-tors */
#ifdef XPSTAR
		if (use_Pmodels) llmodel = 1;
		else
		if (use_Qmodels) llmodel = 2;
#endif
	default:  acnt = &m_cnt;
    }

#ifdef XPSTAR
    if (prefix == 'v') sprintf (buf, "e_%ld", ++(*acnt));
    else if (useDBinames) {
	if (*cmc.inst_name == '_')
	    sprintf (buf, "%s%s", mod -> out_name, cmc.inst_name);
	else
	    sprintf (buf, "%s_%s", mod -> out_name, cmc.inst_name);
    }
    else sprintf (buf, "%s_%ld", mod -> out_name, ++(*acnt));
#else
    if (prefix == 'v') sprintf (buf, "v%ld", ++(*acnt));
    else if (useDBinames) {
	int len = strlen (mod -> prefix);
	char *name = cmc.inst_name;

	if (*name == '_') ++name;
	if (len == 1 && mtype == 'd') {
	    if (prefix == *name || toupper(prefix) == *name) len = 0;
	}
	if (len) len = strncmp (name, mod -> prefix, len);
	if (len) sprintf (buf, "%s%s", mod -> prefix, name);
	else sprintf (buf, "%s", name);
    }
    else sprintf (buf, "%s%ld", mod -> prefix, ++(*acnt));
#endif
    nmprint (0, buf, useDBinames? cmc.inst_dim : 0L, lower, upper, 0, 0);

    if (add_paren)
	oprint (0, " (");
    else
	oprint (0, " ");

    tog_use0 = tog_use0_save;
}

/* fvalPar prints a parameters value by using an S.I.
 * scale factor if possible
 */
/* static */
char *fvalPar (char *par, char *val)
{
    char *pv, *pr, *pt;
    int ncnt, mcnt, pcnt, exp, r, i, negative;

    pv = val;
    pt = fv_tmp;

    if (*pv == '-') {
	negative = 1;
	*pt++ = *pv++;
    }
    else negative = 0;

    ncnt = -1; /* number of digits after '.' */
    pcnt = 0;  /* precision (total number of digits) */
    while ((*pv >= '0' && *pv <= '9') || *pv == '.') {
	if (*pv == '.') ncnt = 0;
	else {
	    pcnt++;
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

    if (dialect == ESPICE || llmodel) {
	switch (*par) {
	case 'l':
	case 'w':
	    if (*++par) break;
	case 'p':
	    if (*par) {
		if (*++par == 's' || *par == 'd') { /* "ps" or "pd" */
		    if (*++par) break;
		}
		else if (*par == 'e') { /* "peri" */
		    if (*++par != 'r') break;
		    if (*++par != 'i') break;
		}
		else break;
	    }
	    exp += 6;	/* output is in micron (u) */
	    break;
	case 'a':
	    if (*++par == 's' || *par == 'd') { /* "as" or "ad" */
		if (*++par) break;
	    }
	    else if (*par == 'r') { /* "area" */
		if (*++par != 'e') break;
		if (*++par != 'a') break;
	    }
	    else break;
	    exp += 12;	/* output is in square micron (u^2) */
	}
    }

    exp = exp + (pcnt - 1) - ncnt;
       /* exp + new ncnt - old ncnt */
    ncnt = pcnt - 1;
    mcnt = 1;  /* number of digits before '.' */

    /* the requirement is: exp % 3 == 0
			   and no zero before '.' (i.e. mcnt > 0) ! */

    if (exp > 0) {
	r = exp % 3;
	if (r != 0) {
	    if (mcnt - (3 - r) <= 0) {
		exp = exp - r;
		mcnt = mcnt + r;
	    }
	    else {
		exp = exp + (3 - r);
		mcnt = mcnt - (3 - r);
	    }
	}
    }
    else if (exp < 0) {
	r = -exp % 3;
	if (r != 0) {
	    if (mcnt - r <= 0) {
		exp = exp - (3 - r);
		mcnt = mcnt + (3 - r);
	    }
	    else {
		exp = exp + r;
		mcnt = mcnt - r;
	    }
	}
    }

    pr = fv_ret;
    pt = fv_tmp;

    if (negative) *pr++ = *pt++;

    for (i = 0; i < pcnt; i++) {
	if (i == mcnt) *pr++ = '.';
	*pr++ = *pt++;
    }
    while (i < mcnt) {
	*pr++ = '0';    /* trailing zero's */
	i++;
    }

    switch (exp) {
	case -18:
#if defined XPSTAR || defined XSPECTRE
	    sprintf (pr, "a");
#else
	    sprintf (pr, "e%d", exp); /* not always understood by spice */
#endif
	    break;
	case -15:
	    sprintf (pr, "f");
	    break;
	case -12:
	    sprintf (pr, "p");
	    break;
	case -9:
	    sprintf (pr, "n");
	    break;
	case -6:
	    sprintf (pr, "u");
	    break;
	case -3:
#ifdef XPSTAR
	    sprintf (pr, "ml");
#else
	    sprintf (pr, "m");
#endif
	    break;
	case 0:
	    *pr = 0;
	    break;
	case 3:
	    sprintf (pr, "k");
	    break;
	case 6:
#ifdef XSPECTRE
	    sprintf (pr, "M");
#elif defined XPSTAR
	    sprintf (pr, "mg");
#else
	    sprintf (pr, "meg");
#endif
	    break;
	case 9:
#ifdef XSPECTRE
	    sprintf (pr, "G");
#else
	    sprintf (pr, "g");
#endif
	    break;
	case 12:
#ifdef XSPECTRE
	    sprintf (pr, "T");
#else
	    sprintf (pr, "t");
#endif
	    break;
#ifdef XSPECTRE
	case 15:
	    sprintf (pr, "P");
	    break;
#endif
	default:
	    sprintf (pr, "e%d", exp);
	    break;
    }

    cutTrail0s (fv_ret);

    return (fv_ret);
}

static void cutTrail0s (char *str)
{
    char *p, *last;
    int commaPassed;

    if (!*str) return;

    commaPassed = 0;
    if (*str == '-') ++str;
    p = str;
    last = str;
    while ((*p >= '0' && *p <= '9') || *p == '.') {
	if (*p == '.') commaPassed = 1;
	if (*p != '0' || !commaPassed) last = p;
	p++;
    }
    if (*last == '.') last--;

    if (p != last) {
	while (*p) *++last = *p++;
	*++last = '\0';
    }
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
