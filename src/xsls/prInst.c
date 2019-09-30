
/*
 * ISC License
 *
 * Copyright (C) 1987-2016 by
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

#include "src/xsls/incl.h"

extern long *Nil;
extern int out_indent;
extern int tog_irange;
extern int noUnconnect;
extern int useDBinames;
extern int saveInOutBuf;

extern char **globNets;
#ifdef ADDGLOBALNETS
extern char **globConA;
extern char **globConB;
#endif
extern int globNets_cnt;

extern struct node_info *nodetab;

static int mtype;

static char fv_ret[32];
static char fv_tmp[32];

#ifdef __cplusplus
  extern "C" {
#endif
static void prPar (struct model_info *mod, int mode);
static void prInNm (struct model_info *mod, long *lower, long *upper);
static void cutTrail0s (char *str);
static char *fvalPar (char *par, char *val);
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
    int firstinst;
    int ready;
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
    int has_connect;
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

    while (dmGetDesignData (dsp, CIR_MC) > 0) {

	if (is_ap ()) continue;

	imported = cmc.imported;
	has_connect = 0;
	if (noUnconnect) saveInOutBuf = 1;

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

        firstinst = 1;
        ready = 0;
        while (!ready) {

	    firstinode = 1;

            if (!tog_irange || firstinst) {

		out_indent = 4;

		if (!tog_irange) {
		    prInNm (mod, xv, xv);
		}
		else {
		    firstinst = 0;
		    prInNm (mod, cmc.inst_lower, cmc.inst_upper);
		}

		if (mtype == 'f') {
		    oprint (0, "@ ");
		    oprint (1, mod -> name);
		}
		else
		    oprint (0, mod -> name);

		prPar (mod, 1);
		prPar (mod, 2);
		prPar (mod, 3);

		oprint (0, " (");
		if (out_indent != 4) out_indent = 22;

		if ((out_indent = outPos ()) > 24) out_indent = 24;
	    }

	    for (tref = mod -> terms; tref; tref = tref -> next) {

		tnet -> net_name = tref -> t -> term_name;
		tnet -> net_dim  = tref -> t -> term_dim;
		for (j = tnet -> net_dim; --j >= 0;) xvt[j] = tref -> t -> term_lower[j];

		do {
		    if (firstinode) firstinode = 0;
		    else { oprint (1, ","); oprint (0, " "); }

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
#ifdef ADDGLOBALNETS
			if (globalTerm) {
			    nmprint (0, tnet -> net_name, 0L, Nil, Nil, 0);
			}
#endif /* ADDGLOBALNETS */
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
			sprintf (buf, "%ld", fnet->nx);
			nmprint (0, buf, 0L, Nil, Nil, 0);
		    }
		    else {
			actual_name = fnet -> net_name;

#ifdef ADDGLOBALNETS
			if (globalTerm && strcmp (tnet -> net_name, actual_name) != 0) {
			    globConA[globCon_cnt] = actual_name;
			    globConB[globCon_cnt] = tnet -> net_name;
			    globCon_cnt++;
			}
#endif /* ADDGLOBALNETS */

			nmprint (0, actual_name, fnet -> net_dim, fnet -> net_lower, Nil, 0);
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

	    if (ready || !tog_irange) oprint (1, ");");
	    else oprint (1, ",");
	    oprint (0, "\n");
	}

#ifdef ADDGLOBALNETS
	for (i = 0; i < globCon_cnt; i++) {

	    /* generate a connection between actual parameter
	       and global net since the formal parameter is a global net
	    */
	    oprint (0, "net {");
	    nmprint (0, globConA[i], 0L, Nil, Nil, 0);
	    oprint (1, ",");
	    oprint (0, " ");
	    nmprint (0, globConB[i], 0L, Nil, Nil, 0);
	    oprint (1, "};\n");
	}
#endif /* ADDGLOBALNETS */

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

static void prPar (struct model_info *mod, int mode)
/* prPar has three modes:
	1. print value parameter (v=val)
	   (if not specified in the cell parameter list).
	2. print other parameters (par=val)
	3. print comment */
{
    char buf[128];
    char *attr, *par, *val;
    int pr_par, pr_val, pr_com;
    int comment;

    if (mode == 2) {
	com_nr = 0;
	cbufp = cbuf;
    }
    else if (mode == 3) { /* print comment */
	if (com_nr == 0) return;

	if ((out_indent = outPos ()) > 70) out_indent = 7;
	oprint (0, " /*");

	if ((cbufp - cbuf) > 1020) fatalErr ("internal error:", "too small comment buf");

	for (comment = 0; comment < com_nr; ++comment) {
	    if (out_indent != 7) {
		if (outPos () + strlen (com_ptr[comment]) > 70) out_indent = 7;
	    }
	    sprintf (buf, " %s", com_ptr[comment]);
	    oprint (0, buf);
	}

	oprint (1, " */");
	return;
    }

    attr = cmc.inst_attribute;

    if (!attr || !*attr) return;

    while (attr) {
	attr = nextAttr (&par, &val, attr);

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
		    if (!*par) pr_com = 1;
		    else if (!par[1]) {
			if (*par == 'w' || *par == 'l') {
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
			    /* pr_par = pr_val = 1; */
			}
			else if (!par[2] && ((*par == 'w' && par[1] == 'b') ||
			((*par == 'a' || *par == 'p') && par[1] == 'e'))) {
			    /* wb, ae, pe */
			    pr_par = pr_val = 1;
			}
			else pr_com = 1;
		    }
		    break;

		case 'n':
		    if (*par == 'f' && !par[1]) goto nxt_attr;
		    pr_com = 1;
	    }

	    if (pr_par && mode == 2) {
		oprint (0, " ");
		oprint (0, par);
	    }

	    if (pr_val && ((pr_par && mode == 2) || (!pr_par && mode == 1))) {
		if (val) {
		    int i = 0;
		    if (pr_par)
			oprint (++i, "=");
		    else
			oprint (0, " ");
		    oprint (i, fvalPar (par, val));
		}
	    }

            if (pr_com && mode == 2 && com_nr < 20) {
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
}

static void prInNm (struct model_info *mod, long *lower, long *upper)
{
    char buf[512];

    if (cmc.inst_name[0] == '_' && cmc.inst_dim == 0 && !useDBinames) return;

    oprint (0, "{");
    if (cmc.inst_name[0] == '_') {
	if (useDBinames) {
	    sprintf (buf, "x%s", cmc.inst_name);
	    nmprint (1, buf, cmc.inst_dim, lower, upper, tog_irange);
	}
	else
	    nmprint (1, ".", cmc.inst_dim, lower, upper, tog_irange);
    }
    else {
	nmprint (1, cmc.inst_name, cmc.inst_dim, lower, upper, tog_irange);
    }
    oprint (1, "}");
    oprint (0, " ");
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

    switch (exp) {
	case -18:
	    sprintf (pr, "a");
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
	    sprintf (pr, "m");
	    break;
	case 0:
	    *pr = 0;
	    break;
	case 3:
	    sprintf (pr, "k");
	    break;
	case 6:
	    sprintf (pr, "M");
	    break;
	case 9:
	    sprintf (pr, "G");
	    break;
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
