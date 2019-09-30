
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

#include "src/xvhdl/incl.h"

extern long *Nil;
extern long mapdim, mapxv[];
extern int out_indent;

extern char *xvhdl_net_prefix;

static long uncon_cnt;
static int mtype;

static char fv_ret[32];
static char fv_tmp[32];

#ifdef __cplusplus
  extern "C" {
#endif
static void prPar (struct model_info *mod);
static void cutTrail0s (char *str);
static char *fvalPar (char *par, char *val);
#ifdef __cplusplus
  }
#endif

void prInst (struct model_info *ntw)
{
    DM_STREAM *dsp;
    long i, j;
    long lower[10], upper[10];
    long *lxv, xv[10], xvt[10];
    char buf[512];
    char attribute_string[512];
    char *actual_name, *s;
    struct net_el tnetspace;
    struct net_el *tnet, *fnet, *n;
    struct net_ref *nref;
    struct term_ref *tref;
    struct model_info *mod;
    int firstinode;
    int imported;
    int ready;

    uncon_cnt= 0;

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

	if (is_func ()) {
	    mtype = 'f';			/* Function */
	    mod = findFunc (cmc.cell_name, imported, ntw -> proj);
	}
        else if ((mod = findDev (cmc.cell_name, imported, ntw -> proj))) {
	    mtype = 'd';			/* Device */
	}
	else {
	    mtype = 'n';			/* Network */
	    mod = findNetw (cmc.cell_name, imported, ntw -> proj);
	}

	tnet -> inst_dim = cmc.inst_dim;
	for (i = 0; i < cmc.inst_dim; i++) xv[i] = cmc.inst_lower[i];

        ready = 0;
        while (!ready) {

	    firstinode = 1;
	    out_indent = 2;

	    s = cmc.inst_name;
	    if (*s == 'n') { /* name mapping? */
		if (!(s = find_nmp (s, 0))) s = cmc.inst_name;
	    }
	    if (s == cmc.inst_name && no_vhdl_name (s)) {
		mk_vhdl_name (s, buf);
		/* do_vhdl_warn (s, buf, "inst"); */
		s = buf;
	    }
	    nmprint (0, s, cmc.inst_dim, xv, Nil, 1);
	    oprint (1, ": ");

	    if (no_vhdl_name (s = mod -> orig_name)) {
		mk_vhdl_name (s, buf);
		do_vhdl_warn (s, buf, "cell");
		s = buf;
	    }
	    oprint (1, s);

	    prPar (mod);

	    oprint (1, " PORT MAP (");

	    if ((out_indent = outPos ()) > 24) out_indent = 24;

	    for (tref = mod -> terms; tref; tref = tref -> next) {

		tnet -> net_name = tref -> t -> term_name;
		tnet -> net_dim  = tref -> t -> term_dim;
		for (j = tnet -> net_dim; --j >= 0;) xvt[j] = tref -> t -> term_lower[j];

		do {
		    if (firstinode) firstinode = 0;
		    else { oprint (1, ","); oprint (0, " "); }

		    fnet = findNet (tnet);

		    if (!fnet) {
			sprintf (buf, "OPEN%ld", ++uncon_cnt);
			nmprint (0, buf, 0L, Nil, Nil, 0);
		    }
		    else {
			actual_name = fnet -> net_name;
			if ((mapdim = fnet -> net_dim) == 0) {
			    s = actual_name;
			    while (isdigit ((int)*s)) ++s;
			    if (!*s) { /* local node */
				s = NULL;
				sprintf (buf, "%s%s", xvhdl_net_prefix, actual_name);
				actual_name = buf;
			    }
			    else {
				s = vhdl_mapping (&actual_name);
			    }
			    lxv = mapxv;
			}
			else {
			    s = NULL;
			    lxv = fnet -> net_lower;
			}
			nmprint (0, actual_name, mapdim, lxv, Nil, 0);
			if (s) *s = '_';
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

	    oprint (1, ");");
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
	(!strcmp (cmc.cell_name + strlen (cmc.cell_name)
		- strlen (DSCAP_SUFFIX), DSCAP_SUFFIX)))
	return (7);

    return (0);
}

static char *com_ptr[20];
static char cbuf[1024];

static void prPar (struct model_info *mod)
{
    char buf[128];
    char *attr, *par, *val;
    char *cbufp;
    int pr_val, pr_com;
    int com_nr, hasParam;

    attr = cmc.inst_attribute;

    if (!attr || !*attr) return;

    hasParam = 0;
    com_nr = 0;
    cbufp = cbuf;

    while (attr) {
	attr = nextAttr (&par, &val, attr);

	if (par) {
	    pr_val = 0;
	    pr_com = 0;

	    switch (mtype) {
		case 'f':
		    if ((*par == 'f' || *par == 'n') && !par[1]) goto nxt_attr;
		    if (*par == 't' && (par[1] == 'r' || par[1] == 'f') && !par[2]) {
			pr_val = 1; /* tr, tf */
		    }
		    else pr_com = 1;
		    break;

		case 'd':
		    if (!*par) pr_com = 1;
		    else if (!par[1]) {
			if (*par == 'w' || *par == 'l' || *par == 'v') pr_val = 1;
			else pr_com = 1;
		    }
		    else {
			if ((!par[2] && (*par == 'a' || *par == 'p')
			    && (par[1] == 'd' || par[1] == 's'))
			    || (*par == 'n' && par[1] == 'r'
				&& (par[2] == 'd' || par[2] == 's') && !par[3])) {
			    pr_val = 1; /* ad, as, pd, ps, nrd, nrs */
			}
			else if (!par[2] && ((*par == 'w' && par[1] == 'b') ||
			((*par == 'a' || *par == 'p') && par[1] == 'e'))) {
			    pr_val = 1; /* wb, ae, pe */
			}
			else pr_com = 1;
		    }
		    break;

		case 'n':
		    if (*par == 'f' && !par[1]) goto nxt_attr;
		    pr_com = 1;
	    }

	    if (!hasParam) {
		oprint (0, " GENERIC");
		oprint (0, " MAP");
		oprint (0, " (");
		hasParam = 1;
	    }
	    else oprint (0, ", ");
	    oprint (0, par);
	    if (pr_val && val) {
		oprint (1, " => ");
		oprint (1, fvalPar (par, val));
	    }

            if (pr_com && com_nr < 20) {
		com_ptr[com_nr++] = cbufp;
		strcpy (cbufp, par);
		cbufp += strlen (cbufp);
		if (val) {
		    *cbufp++ = '=';
		    if (*par == 'x' || *par == 'y')
			strcpy (cbufp, val);
		    else
			strcpy (cbufp, fvalPar (par, val));
		    cbufp += strlen (cbufp);
		}
		++cbufp;
	    }
	}
nxt_attr:
	if (val) *(val - 1) = '=';
	if (attr) *(attr - 1) = ';';
    }

    if (hasParam) oprint (0, ");");

    if (com_nr > 0) { /* print comment */
	int comment;

	oprint (0, "\n");
	oprint (0, "--"); /* startComment */

	if ((cbufp - cbuf) > 1020) fatalErr ("internal error:", "too small comment buf");

	for (comment = 0; comment < com_nr; ++comment) {
	    sprintf (buf, " %s", com_ptr[comment]);
	    oprint (0, buf);
	}
	oprint (0, "\n");
    }
}

/* fvalPar prints a parameters value by using an S.I.
 * scale factor if possible
 */
static char *fvalPar (char *par, char *val)
{
    char *pv, *pr, *pt;
    int ncnt, mcnt, pcnt, exp, r, i, negative;

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

    sprintf (pr, "e%d", exp);

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
