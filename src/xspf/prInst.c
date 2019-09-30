
/*
 * ISC License
 *
 * Copyright (C) 1994-2016 by
 *	Frederik Beeftink
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

#include "src/xspf/incl.h"
#include "math.h"

extern int driver;
extern int mask_info;
extern int tog_prExt;
extern int gnd_node;
extern int groundVnet;
extern int spef_print, rm_gnd_node;
extern char *model_sf;
extern char *nameGND, *nNET, *pNET;
extern char spef_dnet[];
extern FILE *subfp;

extern struct net_el *fnetGND;
extern struct conduct *conducts;
extern struct contact *contacts;
extern int nr_of_conducts;
extern int nr_of_contacts;

extern int *Nil;
extern int out_indent;
extern int tog_pnod;
extern int tog_nnod;
extern int tog_use0;
extern int tog_use0_save;
extern int useDBinames;
extern int use_spef;
extern int cd_cnt;
extern int rctype;

extern float vnbulk;
extern float vpbulk;
extern int vnbulkdefined;
extern int vpbulkdefined;

extern char *c_unit;
extern char *snbulk, *spbulk;
extern char *CAP3, *RES3;
extern struct node_info *nodetab;

static struct cell_par *mod_pars;
static int  uncon_cnt;
static long r_cnt;
long c_cnt;
static long x_cnt;
static long m_cnt;
static long q_cnt;
static long d_cnt;

static int mtype;
static int is_caps;

static char fv_ret[32];
static char fv_tmp[32];

#ifdef __cplusplus
  extern "C" {
#endif
static void prAP (int dN_nx, int sN_nx, char *ia);
static void prPar (struct model_info *mod, int mode);
static void prPar2 (struct model_info *mod, int mode);
static double getVal (char *par);
static double getAreaVal (void);
static double getPerimVal (void);
static void prInNm (struct model_info *mod, int *lower, int *upper);
static void cutTrail0s (char *str);
#ifdef __cplusplus
  }
#endif

static struct net_el tnetspace;
static int xv[10], xvt[10];

void prInst1 (struct model_info *ntw, struct net_ref *nets)
{
    DM_STREAM *dsp;
    char buf[512];
    struct net_el *tnet, *fnet;
    struct model_info *mod;
    int i, dim, imported;

    out_indent = 0;
    uncon_cnt = 0;

    r_cnt = 0;
    c_cnt = 0;
    cd_cnt = -1;

    tnet = &tnetspace;
    tnet -> net_dim = 0;
    tnet -> inst_dim = 0;
    tnet -> net_lower = xvt;
    tnet -> inst_lower = xv;
    tnet -> inst_name = cmc.inst_name;

    is_caps = 0;
    *buf = 0;

    dsp = dmOpenStream (ntw -> dkey, "mc", "r");

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	if (!*cmc.inst_name) fatalErr ("internal error", "no inst_name");

	if (is_ap ()) { is_caps = 1;
	    if (cmc.inst_dim != 0) fatalErr ("internal error", "on inst_dim");
	    tnet -> net_name = "n";
	    if ((fnet = findNet (tnet))) {
		char *torname = cmc.cell_name + 1;
		torname[strlen (torname) - 3] = '\0';
		addAP (fnet->nx, torname, getAreaVal (), getPerimVal ());
	    }
	    continue;
	}

	if (cmc.inst_name[0] == '_' && cmc.inst_name[1] == 'T') {
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

	imported = cmc.imported;

	if (is_func ()) { /* Function */
	    mod = findFunc (cmc.cell_name, imported, ntw -> proj, 1);
	    mod -> imported = (ntw -> imported || imported);
	}
	else if ((mod = findDev (cmc.cell_name, imported, ntw -> proj))) {
	    /* Device */
	    if (mod -> proj) mod -> imported = (ntw -> imported || imported);

	    if (strcmp (cmc.cell_name, CAP3) == 0
	     || strcmp (cmc.cell_name, RES3) == 0) {
		if (cmc.inst_dim != 0) fatalErr ("internal error", "on inst_dim");
		findNetRC (tnet, getVal ("v"));
		continue;
	    }
	}
	else { /* Network */
	    mod = findNetw (cmc.cell_name, imported, ntw -> proj, 1);
	    mod -> imported = (ntw -> imported || imported);
	}

	if (driver && (fnet = findInst (cmc.inst_name))) {
	    fnet -> v.name = mod -> name;
	    fnet -> valid = 2;
	}

	if (!tog_prExt) continue;

	if (cmc.inst_dim > 0) {
	    dim = cmc.inst_dim;
	    for (i = 0; i < dim; ++i) {
		xv[i] = cmc.inst_lower[i];
		xvt[i] = cmc.inst_upper[i];
	    }
	}
	else dim = 0;

	if (*buf && strcmp (cmc.cell_name, buf) == 0) oprint (1, " ");
	else {
	    if (*buf) { oprint (1, " \""); oprint (1, buf); oprint (1, "\""); }
	    strcpy (buf, cmc.cell_name);
	    oprint (0, use_spef ? "\n*DEFINE " : "\n**DEFINE ");
	}
	nmprint (1, cmc.inst_name, dim, xv, xvt, 1);
    }

    if (*buf) { oprint (1, " \""); oprint (1, buf); oprint (1, "\"\n"); }

    dmCloseStream (dsp, COMPLETE);
}

void prInst (struct model_info *ntw, struct net_ref *nets)
{
    char buf[512];
    char *actual_name, *s;
    struct net_el *tnet, *fnet, *n;
    struct net_el *fnet1, *fnet2;
    struct net_ref *nref;
    struct term_ref *tref;
    struct model_info *mod;
    int i, firstinode, pin;
    int nx1, nx2;

    tnet = &tnetspace;
    tnet -> net_dim = 0;
    tnet -> inst_dim = 0;

    if (!nets || nets -> cd != cd_cnt) {
	fprintf (stderr, "warning: net with cd=%d not found\n", cd_cnt);
	return;
    }
    nref = nets;
    nx1 = nref -> n -> nx;
    while (nref -> next && nref -> next -> cd == cd_cnt) nref = nref -> next;
    nx2 = nref -> n -> nx;

	 if (rctype == 'C') strcpy (cmc.cell_name, CAP3);
    else if (rctype == 'R') strcpy (cmc.cell_name, RES3);
    else fatalErr ("internal error", "on rctype");

    cmc.inst_dim = 0;

    /* start with nref of first net with cd_cnt */

    for (nref = nets; nref && nref -> cd == cd_cnt; nref = nref -> next) {

    n = nref -> n; /* start by head net-struct */
		   /* and do subnets (only these have instances) */
    while ((n = n -> net_eqv)) { /* for all subnets */
	s = n -> inst_name;
	if (!s || *s != '_' || s[1] != rctype) continue;
	strcpy (cmc.inst_name, s);
	tnet -> inst_name = s;
	pin = n -> net_name[0];

	if (pin == 'n') {
	    tnet -> net_name = pNET;
	    fnet = findNetSub (tnet);
	    if (!fnet) {
		fprintf (stderr, "warning: pin 'p' of instance '%s' not found\n", s);
		continue;
	    }
	    if (!use_spef) {
		if (!(fnet -> nx < n -> nx))
		fprintf (stderr, "warning: pin 'p' of instance '%s' not done!\n", s);
		continue;
	    }
	    if (fnet -> nx >= nx1 && fnet -> nx < n -> nx) continue; /* 'p' done */
	}
	else {
	    if (use_spef && subfp) {
		tnet -> net_name = nNET;
		fnet = findNet (tnet);
		if (!fnet) {
		    fprintf (stderr, "warning: pin 'n' of instance '%s' not found\n", s);
		    continue;
		}
		if (fnet -> nx < nx1) {
		    fprintf (stderr, "warning: pin 'n' of instance '%s' in previous dnet?\n", s);
		    continue;
		}
		if (fnet -> nx > nx2) { /* pin 'n' in a next dnet */
		    if (fnet != fnetGND || groundVnet) addNetSub (tnet, fnet->nx);
		}
	    }
	    fnet = n;
	}

	if (fnet -> valid != 1) {
	    fprintf (stderr, "warning: value of instance '%s' not set\n", s);
	    continue;
	}
	sprintf (cmc.inst_attribute, "v=%e", fnet -> v.val);

	mod = findDev (cmc.cell_name, 0, NULL);
	if (!mod) fatalErr ("findDev error: cannot find:", cmc.cell_name);

	if (toupper (mod -> prefix[0]) != rctype) {
	    fprintf (stderr, "warning: incorrect mod->prefix for %c, cell_name = %s\n", rctype, cmc.cell_name);
	    continue;
	}
	mtype = 'd'; /* device of type C/R */

	mod_pars = mod -> pars; /* default model params */
	if (!use_spef && mod -> createName) {
	    struct lib_model *lm = createModel (mod);
	    if (lm && lm -> pars) mod_pars = lm -> pars;
	}

	firstinode = 1;
	out_indent = 0;

	prInNm (mod, xv, xv);

	if ((out_indent = outPos ()) > 24) out_indent = 24;

	fnet1 = fnet2 = NULL;

	if (use_spef && rctype == 'C') {
	    spef_print = 2;
	    fnet = nref->n;
	    actual_name = nodetab[fnet->nx].spef_name;
	    if (isdigit ((int)*actual_name)) {
		sprintf (buf, "%s:%s", spef_dnet, actual_name);
		nodetab[fnet->nx].spef_name = actual_name = newStringSpace (buf);
	    }
	    if (groundVnet && !gnd_node && strcmp (actual_name, nameGND) == 0) ; /* GND dnet, skip first cap node */
	    else {
		tog_use0 = 0;
		nmprint (0, actual_name, fnet -> net_dim, fnet -> net_lower, Nil, 0);
		tog_use0 = tog_use0_save;
		firstinode = 0;
	    }
	}

	for (tref = mod -> terms; tref; tref = tref -> next) {

	    tnet -> net_name = tref -> t -> term_name;
	    if (tref -> t -> term_dim != 0) fatalErr ("internal error", "on term_dim");

	    if (use_spef && rctype == 'C' && pin == tnet->net_name[0]) continue;

	    if (firstinode) firstinode = 0;
	    else oprint (0, " ");

	    fnet = findNet (tnet);

	    if (!fnet) {
		if (++uncon_cnt == INT_MAX) fatalErr ("error:", "uncon_cnt == INT_MAX");
		sprintf (buf, "xxx_%d", uncon_cnt);
		tog_use0 = 0;
		nmprint (0, buf, 0, Nil, Nil, 0);
		tog_use0 = tog_use0_save;
	    }
	    else {

		if (fnet -> net_dim == 0 && mod -> alias_s &&
		    strcmp (fnet -> net_name, mod -> alias_s) == 0) {

		    /* An alias should be used for this actual terminal name.
		       For example GND => pbulk for a p diffusion capacitor */

		    actual_name = mod -> alias_t;
		}
		else
		    actual_name = fnet -> net_name;

		if (use_spef) {
		    spef_print = 2;
		    actual_name = nodetab[fnet->nx].spef_name;
		    if (isdigit ((int)*actual_name)) {
			if (fnet != nref->n)
			    fprintf (stderr, "warning: fnet != nref->n, actual_name = %s\n", actual_name);
			else {
			    sprintf (buf, "%s:%s", spef_dnet, actual_name);
			    nodetab[fnet->nx].spef_name = actual_name = newStringSpace (buf);
			}
		    }
		    if (rctype == 'C' && !gnd_node) rm_gnd_node = 1;
		    else tog_use0 = 0;
		    if (!fnet1) fnet1 = fnet; else fnet2 = fnet;
		}
		nmprint (0, actual_name, fnet -> net_dim, fnet -> net_lower, Nil, 0);
	    }
	}

	if (use_spef) {
	    rm_gnd_node = 0;
	    tog_use0 = tog_use0_save;
	}

	/*
	    SPICE requires that for cap & res first the
	    value is printed and then the model name.
	*/
	prPar (mod, 1); /* print value */
	if (!use_spef) {
	    if (mod -> out_name) prPar (mod, 0); /* print model name */
	    prPar (mod, 2);
	    prPar (mod, 3);
	}
	else if (mask_info && rctype == 'R' && fnet1 && fnet2) {
	    if (fnet1 -> lay == fnet2 -> lay) {
		double l, w;
		l = fnet2 -> x - fnet1 -> x;
		w = fnet2 -> y - fnet1 -> y;
		l = sqrt (l * l + w * w);
		for (i = 0; i < nr_of_conducts; ++i) if (fnet1 -> lay == conducts[i].lay) break;
		if (i < nr_of_conducts) w = l * conducts[i].res / getVal ("v"); else w = 0;
		sprintf (buf, " // $lvl=%d $l=%g $w=%g", (int)fnet1 -> lay, l, w);
	    }
	    else {
		double a;
		for (i = 0; i < nr_of_contacts; ++i) {
		    if (fnet1 -> lay == contacts[i].lay1 && fnet2 -> lay == contacts[i].lay2) break;
		    if (fnet2 -> lay == contacts[i].lay1 && fnet1 -> lay == contacts[i].lay2) break;
		}
		if (i < nr_of_contacts) a = contacts[i].res / getVal ("v"); else a = 0;
		sprintf (buf, " // $lvl=%d $lvl=%d $a=%g", (int)fnet1 -> lay, (int)fnet2 -> lay, a);
	    }
	    oprint (1, buf);
	}

	oprint (0, "\n");
    }
    }

    out_indent = 0;
}

void prInst3 (struct model_info *ntw, struct net_ref *nets) /* laatste (3-de) fase */
{
    DM_STREAM *dsp;
    char buf[512];
    char *actual_name;
    struct net_el *tnet, *fnet;
    struct term_ref *tref;
    struct model_info *mod;
    int dN_nx, sN_nx, i, j, dim;
    int firstinode, ready;
    int is_ap_mos;
    int prPnod, prNnod;
    int valPrinted;
    int imported;
    int bulkDone;

    x_cnt = 0;
    m_cnt = 0;
    q_cnt = 0;
    d_cnt = 0;

    tnet = &tnetspace;
    tnet -> net_dim = 0;
    tnet -> inst_dim = 0;
    tnet -> inst_name = cmc.inst_name;

    dsp = dmOpenStream (ntw -> dkey, "mc", "r");

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_caps) {
	    if (is_ap ()) continue;
	    is_ap_mos = (cmc.inst_name[0] == '_' && cmc.inst_name[1] == 'T');
	}
	else is_ap_mos = 0;

	dN_nx = sN_nx = 0;

	imported = cmc.imported;

	if (is_func ()) {
	    mtype = 'f'; /* Function */
	    mod = findFunc (cmc.cell_name, imported, ntw -> proj, 1);
	}
	else if ((mod = findDev (cmc.cell_name, imported, ntw -> proj))) {
	    if (strcmp (cmc.cell_name, CAP3) == 0
	     || strcmp (cmc.cell_name, RES3) == 0) continue;
	    /* instance is NIET een Cap/Res-device */
	    mtype = 'd'; /* Device */
	}
	else {
	    mtype = 'n'; /* Network */
	    mod = findNetw (cmc.cell_name, imported, ntw -> proj, 1);
	}

	tnet -> inst_dim = dim = cmc.inst_dim;
	for (i = 0; i < dim; ++i) xv[i] = cmc.inst_lower[i];

	mod_pars = mod -> pars; /* default model params */
	model_sf = NULL;
	if (mtype == 'd' && mod -> createName) {
	    struct lib_model *lm = createModel (mod);
	    if (lm && lm -> pars) mod_pars = lm -> pars;
	}

        ready = 0;
        while (!ready) {

	    firstinode = 1;
	    out_indent = 0;

	    prInNm (mod, xv, xv);

	    if ((out_indent = outPos ()) > 24) out_indent = 24;

	    if (mtype == 'n' && (tog_pnod || tog_nnod)) {

		prPnod = tog_pnod;
		prNnod = tog_nnod;

		for (tref = mod -> terms; tref; tref = tref -> next) {
		    if (strcmp (tref -> t -> term_name, "pbulk") == 0) {
			if (!prNnod) goto nxt;
			prPnod = 0;
		    }
		    if (strcmp (tref -> t -> term_name, "nbulk") == 0) {
			if (!prPnod) goto nxt;
			prNnod = 0;
		    }
		}

		if (prPnod) {
		    if (firstinode) firstinode = 0;
		    else oprint (0, " ");
		    nmprint (0, "pbulk", 0, Nil, Nil, 0);
		}
		if (prNnod) {
		    if (firstinode) firstinode = 0;
		    else oprint (0, " ");
		    nmprint (0, "nbulk", 0, Nil, Nil, 0);
		}
	    }
nxt:
	    bulkDone = 0;

	    for (tref = mod -> terms; tref; tref = tref -> next) {

		tnet -> net_name = tref -> t -> term_name;
		tnet -> net_dim  = tref -> t -> term_dim;
		for (j = tnet -> net_dim; --j >= 0;) xvt[j] = tref -> t -> term_lower[j];

		if (tnet -> net_name[0] == mod -> lookForBulk) bulkDone = 1;

		do {
		    if (firstinode) firstinode = 0;
		    else oprint (0, " ");

		    fnet = findNet (tnet);

		    if (!fnet) {
			if (++uncon_cnt == INT_MAX) fatalErr ("error:", "uncon_cnt == INT_MAX");
			sprintf (buf, "xxx_%d", uncon_cnt);
			tog_use0 = 0;
			nmprint (0, buf, 0, Nil, Nil, 0);
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

			nmprint (0, actual_name, fnet -> net_dim, fnet -> net_lower, Nil, 0);
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

	    if (mtype == 'd') {
		if (mod -> lookForBulk && !bulkDone) {
		    tnet -> net_name = mod -> lookForBulk == 'b' ? "b" : "s";
		    tnet -> net_dim = 0;
		    fnet = findNet (tnet);
		}
		else fnet = NULL;

		if (fnet) {
		    oprint (1, " ");
		    nmprint (0, fnet -> net_name, fnet -> net_dim, fnet -> net_lower, Nil, 0);
		}
		else if (mod -> vbulkdefined) {
		    if (tog_pnod) {
			if ((mod -> spec_dev && vpbulkdefined && mod -> vbulk == vpbulk)
				|| (!mod -> spec_dev && mod -> vbulk > 0)) {
			    oprint (1, " ");
			    nmprint (0, "pbulk", 0, Nil, Nil, 0);
			}
		    }
		    if (tog_nnod) {
			if ((mod -> spec_dev && vnbulkdefined && mod -> vbulk == vnbulk)
				|| (!mod -> spec_dev && mod -> vbulk <= 0)) {
			    oprint (1, " ");
			    nmprint (0, "nbulk", 0, Nil, Nil, 0);
			}
		    }
		}
	    }

	    switch (mtype) {

		case 'f':
		case 'n':
		    prPar (mod, 0);
		    prPar (mod, 1);
		    prPar (mod, 2);
		    prPar (mod, 3);
		    break;

		case 'd':

		/* Note: If there is NO out_name it must be a predef. device whereof
		   there no out_name needs to exist (CAP, RES or IND).
		   This decision is made in function findDev() */

		    valPrinted = 0;

		    if (mod -> out_name) { /* print model name */
			int p = tolower (mod -> prefix[0]);
			if (p == 'c' || p == 'r') {
			    /*
				SPICE requires that for cap & res first the
				value is printed and then the model name.
			    */
			    prPar (mod, 1);
			    valPrinted = 1;
			}
			prPar (mod, 0);
			if (model_sf) { /* scale factor */
			    if (atof (model_sf) != 1.0) {
				struct cell_par *cp;
				for (cp = mod_pars; cp; cp = cp -> next)
				    if (cp -> val && strcmp (cp -> val, "msf") == 0) break;
				if (!cp) {
				    char *s = "";
				    if (p == 'm') s = "m=";
				    sprintf (buf, " %s%s", s, model_sf);
				    oprint (0, buf);
				}
			    }
			    valPrinted = 1;
			}
		    }

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
		    break;
	    }

	    ready = 1;
	    for (i = dim - 1; i >= 0; i--) {
		if (cmc.inst_lower[i] <= cmc.inst_upper[i]) {
		    if (++xv[i] <= cmc.inst_upper[i]) { ready = 0; break; }
		}
		else {
		    if (--xv[i] >= cmc.inst_upper[i]) { ready = 0; break; }
		}
		xv[i] = cmc.inst_lower[i];
	    }

	    oprint (0, "\n");
	}
    }

    out_indent = 0;

    dmCloseStream (dsp, COMPLETE);
}

int is_ap ()
{
    /* EM: Check if name is "$'torname'$ds" */
    if ((cmc.cell_name[0] == DSCAP_PREFIX) &&
	(!strcmp (cmc.cell_name + strlen (cmc.cell_name) - 3, DSCAP_SUFFIX))) return (7);
    return (0);
}

static int com_nr;
static char *com_ptr[20];
static char *cbufp;
static char cbuf[1024];

static char *getNextOperator (char *s)
{
    char *t;
    int c, d = 0;

    t = s; c = *s;
    while (isdigit (c)) { c = *++s; ++d; }
    if (c == '.') {
        c = *++s;
        while (isdigit (c)) { c = *++s; ++d; }
    }
    if (!d) return (t);
    if (c == 'e' || c == 'E') {
        t = s; c = *++s;
        if (c == '-' || c == '+') c = *++s;
        d = 0;
        while (isdigit (c)) { c = *++s; ++d; }
        if (!d) return (t);
    }
    return (s);
}

static void prPar (struct model_info *mod, int mode)
/* prPar has three modes:
	1. print value parameter (v=val)
	   (if not specified in the cell parameter list).
	2. print other parameters (par=val)
	3. print comment */
{
    char buf[128];
    char *s, *p, *v;
    struct cell_par *cp;
    int comment, i;

    if (mode == 0) {
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
	    else
		comment = 0;

	    if (cp -> val) {
		v = NULL;
		if (*cp -> val == 'm') {
		    if (strcmp (cp -> val, "mname") == 0) {
			oprint (0, " ");
			if (mod -> out_name)
			    oprint (0, mod -> out_name);
			else
			    oprint (0, mod -> name);
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
		    v = cp -> val;
		    if (isalpha ((int)*v))
			v = getAttrValue (cmc.inst_attribute, v);
		}
		if (v) {
		    if (p || *v) {
			if (comment) {
			    if (p && *p && com_nr < 20) {
				com_ptr[com_nr++] = cbufp;
				strcpy (cbufp, p);
				cbufp += strlen (cbufp);
				if (*v && *p != '=') *cbufp++ = '=';
				else if (!*v) ++cbufp;
			    }
			}
			else {
			    oprint (0, " ");
			    if (p) {
				oprint (0, p);
				if (*v) oprint (0, "=");
			    }
			}
		    }

		    if (*v) {
			int at = 0;

			if ((s = cp -> par_stat)) {
			    int c = *s;
			    while (c == '+' || c == '-' || c == '*' || c == '/') {
				char *t;
				double w = (c == '*')? 1 : 0;
				int m = 1;

				++s;
				if (*s == '-') { ++s; m = -1; }
				else if (*s == '+') ++s;
				if (*s == '$') ++s;
				if (isalpha ((int)*s)) {
				    i = 0;
				    t = s++;
				    while (isalnum ((int)*s) || *s == '_') ++s;
				    at = *s;
				    *s = 0;
				    if (*t == 'm') {
					if (strcmp (t, "msf") == 0) {
					    w = m * atof (model_sf); i = 1;
					}
					else if (strcmp (t, "mdl") == 0) {
					    w = m * mod -> dl; i = 1;
					}
					else if (strcmp (t, "mdw") == 0) {
					    w = m * mod -> dw; i = 1;
					}
				    }
				    if (!i) {
					t = getAttrValue (cmc.inst_attribute, t);
					if (t) w = m * atof (t);
				    }
				    *s = at;
				}
				else {
				    w = m * atof (s);
				    s = getNextOperator (s);
				}

				switch (c) {
				    case '+': w = atof (v) + w; break;
				    case '-': w = atof (v) - w; break;
				    case '*': w = atof (v) * w; break;
				    case '/': if (w) w = atof (v) / w;
					      else   w = atof (v);
				}
				sprintf (v = buf, "%e", w);
				c = *s;
			    }
			    if (c == '@') ++s;
			    at = *s;
			}

			if (comment) {
			    if (!(p && *p) && com_nr < 20) com_ptr[com_nr++] = cbufp;
			    if (com_nr <= 20) {
				strcpy (cbufp, fvalPar (cp -> val, v));
				cbufp += strlen (cbufp);
				if (at) {
				    strcpy (cbufp, s);
				    cbufp += strlen (s);
				}
				++cbufp;
			    }
			}
			else {
			    oprint (0, fvalPar (cp -> val, v));
			    if (at) oprint (1, s);
			}
		    }
		}
	    }
	    else if ((p = cp -> par_stat)) {
		if (*p == '!') {
		    if (*++p == '!' && *++p && com_nr < 20) com_ptr[com_nr++] = p;
		}
		else {
		    oprint (0, " ");
		    if (*p == '@') ++p;
		    oprint (1, p);
		}
	    }
	}
    }
    else if (mode == 3) { /* print comment */
	if (com_nr == 0) return;

	oprint (0, "\n");
	oprint (0, "*");

	if ((cbufp - cbuf) > 1020) fatalErr ("internal error:", "too small comment buf");

	for (comment = 0; comment < com_nr; ++comment) {
	    sprintf (buf, " %s", com_ptr[comment]);
	    oprint (0, buf);
	}
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
		    else
			pr_com = 1;
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
			else if (*par == 'v') {
			    if (strcmp (cmc.cell_name, CAP3) == 0) par = c_unit;
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
			    /* if (XSLS) pr_par = pr_val = 1; */
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
		if (tolower (mod -> prefix[0]) == 'm') {
		    par = "m";
		    pr_par = 1;
		}
		pr_sf = 0;
	    }

	    if (pr_par && mode == 2) {
		oprint (0, " ");
		oprint (0, par);
	    }

	    if (pr_val && ((pr_par && mode == 2) || (!pr_par && mode == 1))) {
		if (val) {
		    int i = 0;
		    if (pr_par) oprint (++i, "=");
		    else oprint (0, " ");
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

static void prAP (int dN_nx, int sN_nx, char *ia)
{
    int cd, cs;
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

static void prInNm (struct model_info *mod, int *lower, int *upper)
{
    long *acnt;
    char *prefix;
    char buf[512];
    int resistor;

    if (use_spef) {
	tog_use0 = 0;
	sprintf (buf, "%ld", ++c_cnt);
	nmprint (0, buf, (int)cmc.inst_dim, lower, upper, 0);
	oprint (0, " ");
	tog_use0 = tog_use0_save;
	return;
    }

    /* SPF */
    tog_use0 = 0;
    resistor = 0;

    prefix = mod -> prefix;
    switch (tolower (*prefix)) {
	case 'c': acnt = &c_cnt; break; /* capacitors */
	case 'r': acnt = &r_cnt; resistor = 1; break; /* resistors */
	case 'q': acnt = &q_cnt; break; /* BJT's */
	case 'x': acnt = &x_cnt; break; /* sub-circuits */
	case 'd': acnt = &d_cnt; break; /* diodes */
	case 'm':			/* MOS-tors */
	default:  acnt = &m_cnt;
    }

    if (useDBinames) {
	int len = strlen (prefix);
	char *name = cmc.inst_name;

	if (*name == '_') ++name;
	if (len == 1 && mtype == 'd') {
	    if (tolower (*prefix) == *name
	     || toupper (*prefix) == *name) len = 0;
	}
	if (len) len = strncmp (name, prefix, len);
	if (len)
	    sprintf (buf, "%s%s", prefix, name);
	else
	    sprintf (buf, "%s", name);
    }
    else if (resistor && getVal("v") == 0)
	sprintf (buf, "v%ld", ++(*acnt));
    else
	sprintf (buf, "%s%ld", prefix, ++(*acnt));
    nmprint (0, buf, (int)cmc.inst_dim, lower, upper, 0);
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
    int ncnt, mcnt, pcnt, exp, r, i, negative, femtofarad;

    pv = val;
    pt = fv_tmp;

    if (*pv == '-') {
	negative = 1;
	++pv;
    }
    else
	negative = 0;

    ncnt = -1; /* number of digits after '.' */
    pcnt = 0;  /* precision (total number of digits) */
    while ((*pv >= '0' && *pv <= '9') || *pv == '.') {
	if (*pv == '.') {
	    ncnt = 0;
	}
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

    femtofarad = 0;
    if (strcmp (par, "FF") == 0) {
	if (!use_spef) ++femtofarad;
	exp += 15;
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

    if (negative) {
	fv_ret[0] = '-';
	pr = fv_ret + 1;
    }
    else
	pr = fv_ret;

    pt = fv_tmp;
    for (i = 0; i < pcnt; i++) {
	if (i == mcnt) *pr++ = '.';
	*pr++ = *pt++;
    }
    while (i < mcnt) {
	*pr++ = '0';    /* trailing zero's */
	i++;
    }

    if (exp == 0) {
	if (femtofarad)
	    sprintf (pr, "f");
	else
	    *pr = 0;
    }
    else {
	if (femtofarad) exp -= 15;
	sprintf (pr, "e%d", exp);
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
