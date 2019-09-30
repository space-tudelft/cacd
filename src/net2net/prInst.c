/*
 * ISC License
 *
 * Copyright (C) 2016 by
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

#include "src/net2net/incl.h"

extern char *argv0;
extern char *noNAME;
extern char *sorts[];
extern int coordinates;
extern int inst_change;
extern int no_lays;
extern int nr_cds;
extern int nrtimes;
extern int nxGND, nxSUB;
extern int verbose;
extern struct net_ref **nreftab;
extern DM_CELL *dkey;

static DM_STREAM *dspmc2;
static DM_STREAM *dspnet2;

static char *Val (double val);

int wr_nr[20];
int cntR  = 0;
int cntRS = 0;
int cntC  = 0;
int cntCS = 0;
int cntCG = 0;
int nodeCnt = 0;

void prInst (struct net_ref *nets)
{
    char *attr, *s;
    char *inst, *t;
    DM_STREAM *dsp;
    double val;

    dsp = dmOpenStream (dkey, "mc", "r");
    dspmc2 = dmOpenStream (dkey, "mc2", "w");
    dspnet2 = dmOpenStream (dkey, "net2", "w");

    attr = cmc.inst_attribute;
    inst = cmc.inst_name; t = inst + 1;

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	if (*inst == '_' && (*t == 'C' || *t == 'R')) {
	    if (cmc.inst_dim != 0) fatalErr ("internal error", "on inst_dim");
	    if (!(s = getAttrValue (attr, "v"))) {
		warning2 ("instance '%s', attr. 'v' not found", inst);
		val = 0;
	    }
	    else val = atof (s);
	    findNetRC ((*t == 'C' || val == 0)? val : 1/val);
	}
	else {
	    if (!coordinates && (s = getAttrValue (attr, "x"))) { /* strip */
		if (*--s == '=')
		if (*--s == 'x')
		if (*--s == ';') *s = 0;
	    }
	    dmPutDesignData (dspmc2, CIR_MC);
	}
    }

    dmCloseStream (dsp, COMPLETE);
}

static void convert_dim (char *s, long lower[], int dim)
{
    int i, j;

    if (!s || *s == '[' || !*s) fatalErr ("convert_dim error:", "incorrect string start");
    ++s; while (*s != '[' && *s) ++s;
    for (i = 0; i < dim; ++i) {
	if (*s != '[') fatalErr ("convert_dim error:", "item '[' not found");
	*s++ = 0;
	j = 0; while (isdigit (*s)) j = j * 10 + (*s++ - '0');
	if (*s != ']') fatalErr ("convert_dim error:", "item ']' not found");
	lower[i] = j;
	++s;
    }
    if (*s) fatalErr ("convert_dim error:", "EOS not found");
}

void prNets (struct net_ref *nets)
{
    struct net_ref *nref, *ref2;
    struct net_el *n, *nn;
    struct net_el2 *m, *nm;
    char *attr;
    int   cd, cd_nr, cnt, nr, type;
    long  net_lower[10], inst_lower[10];

    cnet.net_attribute = attr = cmc.inst_attribute;
    cnet.inst_dim = 0;
    cnet.ref_dim = 0;
    cnet.net_eqv = NULL;
    cnet.net_lower  = cnet.net_upper  = net_lower;
    cnet.inst_lower = cnet.inst_upper = inst_lower;

    cmc.imported = 0;
    cmc.inst_dim = 0;

    cd_nr = -1;
    cd = 0;
    for (nref = nets; nref; nref = nref -> next) {

	n = nref -> n; /* start by head net-struct */
		       /* and do subnets (only these have instances) */

	nn = n -> nexte;
	nm = nref -> nR;
	if (!nm) {
	    if ((nm = nref -> nC)) nref -> nC = NULL;
	    else { nm = nref -> nl; nref -> nl = NULL; }
	}
	if (!nn && !nm) { /* no subnets */
	    fprintf (stderr, "%s: found no subnets for net %d\n", argv0, nref -> nx);
	    continue;
	}

	if (nref -> cd != cd_nr) {
	    cd_nr = nref -> cd;
	    if (cd_nr) ++cd;
	    if (nr_cds) {
		if (n -> net_name == noNAME)
		    fprintf (stderr, "%s: found new cd number cd=%d net=%d\n", argv0, cd_nr, nref -> nx);
		else
		    fprintf (stderr, "%s: found new cd number cd=%d net=%s\n", argv0, cd_nr, n -> net_name);
	    }
	}

	if (no_lays)
	    sprintf (attr, "cd=%d", cd_nr ? cd : 0);
	else
	    sprintf (attr, "cd=%d;lay=%d", cd_nr ? cd : 0, nref -> lay);

	++nodeCnt;

	if (n -> net_name == noNAME) {
	    sprintf (cnet.net_name, "%d", inst_change ? nodeCnt : nref -> nx);
	} else {
	    if (n -> net_dim) convert_dim (n -> net_name, net_lower, n -> net_dim);
	    strcpy (cnet.net_name, n -> net_name);
	}

	cnet.net_dim = n -> net_dim;
	cnet.inst_name[0] = 0;

	if (coordinates) sprintf (attr + strlen(attr), ";x=%d;y=%d", n -> x, n -> y);
	cnet.net_neqv = nref -> netcnt;
	dmPutDesignData (dspnet2, CIR_NET_ATOM);
	cnet.net_neqv = 0;

	cnt = 0;
	while ((n = nn)) { /* for all general subnets */
	    nn = n -> nexte;
	    ++cnt;
	    if (n -> net_dim) convert_dim (n -> net_name, net_lower, n -> net_dim);
	    strcpy (cnet.net_name, n -> net_name);
	    cnet.net_dim = n -> net_dim;

	    attr[0] = 0;
	    if (coordinates && n -> x != INT_MAX) sprintf (attr, "x=%d;y=%d", n -> x, n -> y);
	    if (!n -> inst_name) {
		cnet.inst_name[0] = 0;
		dmPutDesignData (dspnet2, CIR_NET_ATOM);
	    }
	    else {
		if (n -> inst_dim > 0) {
		    convert_dim (n -> inst_name, inst_lower, n -> inst_dim);
		    cnet.inst_dim = n -> inst_dim;
		}
		strcpy (cnet.inst_name, n -> inst_name);
		dmPutDesignData (dspnet2, CIR_NET_ATOM);
		cnet.inst_dim = 0;
	    }
	}

	cnet.net_dim = 0;
	cnet.net_name[1] = 0;

	while ((m = nm)) { /* for all RC subnets */
	    nm = m -> nexte;
	    if (!nm) {
		if ((nm = nref -> nC)) nref -> nC = NULL;
		else { nm = nref -> nl; nref -> nl = NULL; }
	    }
	    ++cnt;

	    nr   = m -> inst_nr;
	    type = m -> inst_type;

	    if (m -> times > 0) { /* 1st subnet of RC instance */
		cnet.net_name[0] = 'p';
		switch (type) {
		case typCS: sprintf (cnet.inst_name, "_CS%d", nr); ref2 = nreftab[nxSUB]; nr = ++cntCS; break;
		case typRS: sprintf (cnet.inst_name, "_RS%d", nr); ref2 = nreftab[nxSUB]; nr = ++cntRS; break;
		case typCSG:sprintf (cnet.inst_name, "_CSG%d",nr); ref2 = nreftab[nxGND]; break;
		case typCG: sprintf (cnet.inst_name, "_CG%d", nr); ref2 = nreftab[nxGND]; nr = ++cntCG; break;
		default:
		    if (nref -> nx < nr)
			sprintf (cnet.inst_name, "_%c%d_%d", type, nref -> nx, nr);
		    else
			sprintf (cnet.inst_name, "_%c%d_%d", type, nr, nref -> nx);
		    ref2 = nreftab[nr];
		    if (type == 'R') nr = ++cntR; else nr = ++cntC;
		    m -> inst_nr = nref -> nx;
		}
		if (m -> times > 1 && nrtimes)
		    fprintf (stderr, "%s: found %d times instance %s\n", argv0, m -> times, cnet.inst_name);
		if (m -> val == 0) {
		    warning2 ("instance '%s' skipped, val=0", cnet.inst_name);
		    goto put_subnet;
		}
		++wr_nr[m -> sort];
		if (inst_change && type != typCSG) { /* use nr */
		    switch (type) {
		    case typCS: sprintf (cnet.inst_name, "_CS%d", nr); break;
		    case typCG: sprintf (cnet.inst_name, "_CG%d", nr); break;
		    case typRS: sprintf (cnet.inst_name, "_RS%d", nr); break;
		    default: sprintf (cnet.inst_name, "_%c%d", type, nr);
		    }
		    m -> inst_nr = nr;
		}
		strcpy (cmc.cell_name, sorts[m -> sort]);
		strcpy (cmc.inst_name, cnet.inst_name);
		if (type < 'R')
		    sprintf (attr, "v=%s", Val (m -> val)); // C-type
		else
		    sprintf (attr, "v=%s", Val (1 / m -> val)); // R-type
		dmPutDesignData (dspmc2, CIR_MC);

		/* add subnet to 2nd pin net-list */
		if (!ref2) {
		    warning2 ("instance '%s' 2nd pin not found", cnet.inst_name);
		    goto put_subnet;
		}
		++ref2 -> netcnt;
		     if (type == 'C') { m -> nexte = ref2 -> nC; ref2 -> nC = m; }
		else if (type == 'R') { m -> nexte = ref2 -> nR; ref2 -> nR = m; }
		else { m -> nexte = ref2 -> nl; ref2 -> nl = m; }
		m -> times = 0;
	    }
	    else {
		cnet.net_name[0] = 'n';
		switch (type) {
		case typCS: sprintf (cnet.inst_name, "_CS%d", nr); break;
		case typCSG:sprintf (cnet.inst_name, "_CSG%d",nr); break;
		case typCG: sprintf (cnet.inst_name, "_CG%d", nr); break;
		case typRS: sprintf (cnet.inst_name, "_RS%d", nr); break;
		default:
		    if (inst_change)
			sprintf (cnet.inst_name, "_%c%d", type, nr);
		    else if (nref -> nx < nr)
			sprintf (cnet.inst_name, "_%c%d_%d", type, nref -> nx, nr);
		    else
			sprintf (cnet.inst_name, "_%c%d_%d", type, nr, nref -> nx);
		}
	    }
put_subnet:
	    attr[0] = 0;
	    dmPutDesignData (dspnet2, CIR_NET_ATOM);
	}

	if (cnt != nref -> netcnt) warning ("incorrect netcnt!");
    }

    dmCloseStream (dspmc2, COMPLETE);
    dmCloseStream (dspnet2, COMPLETE);
}

static char *Val (double val)
{
    static char buf[60];
    int ii, i, j, exp;

    if (val == 0) return ("0");
    i = 6;
    sprintf (buf, "%.*e", i, val);
    i += 2;
    j = 1;
    if (buf[j] != '.') ++j;
    if (buf[j] == '.')
    if (buf[i] == 'e' || buf[++i] == 'e') {
	ii = i; i += 2; exp = 0;
	while (buf[i] == '0') ++i;
	while (buf[i]) exp = exp*10 + (buf[i++] - '0');
	i = ii - 1;
	while (buf[i] == '0') --i;
	if (i != j) ++i;
	if (exp) {
	    if (buf[ii+1] == '-') {
		if (exp <= 3) {
		    if (i == j) ++i;
		    buf[j] = buf[j-1];
		    buf[j-1] = '.';
		    while (exp-- > 1) {
			for (ii = i - 1; ii >= j; --ii) buf[ii+1] = buf[ii];
			++i;
			buf[j++] = '0';
		    }
		    buf[i] = '\0';
		}
		else sprintf (&buf[i], "e-%d", exp);
	    }
	    else {
		if (i == j && exp <= 2) {
		    while (--exp >= 0) buf[i++] = '0';
		    buf[i] = '\0';
		}
		else if (i > j && exp <= i - j + 1) {
		    while (--exp >= 0) {
			if (++j < i) buf[j-1] = buf[j]; else buf[j-1] = '0';
		    }
		    if (j+1 < i) buf[j] = '.'; else i = j;
		    buf[i] = '\0';
		}
		else sprintf (&buf[i], "e%d", exp);
	    }
	}
	else buf[i] = '\0';
    }
    return (buf);
}
