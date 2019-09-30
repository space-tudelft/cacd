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

extern void getsignal (NODE *n, simtime_t t, SIGNALEVENT *sigev);
extern void wr_path (FILE *fp, PATH_SPEC *path);

static void dump_state (simtime_t t);
static char *nrl2int (NODE_REF_LIST *nrl, int fmt, int sign_bit, int two_compl);
static void outresults (void);
static void prcenter (char *s);
static void prequals (void);
static void prtime (simtime_t t);
static void signamesresults (void);
static void lineresults (void);

FILE * simout;
FILE * simres = NULL;
FILE * simtst = NULL;

int ia_ou_fac;
int ia_ou_max;
int outwidth;
simtime_t tlastwritten;

RES_FILE * rf_simres;

int *abstractl_len;
char *abstractl_adj;

void initintrup ()
{
    RES_PATH * rp;
    int i, nbr;
    simtime_t tdmp;

    OPENW (simres, fn_res);
    if (debugsim) setbuf (simres, (char *) NULL);

    fprintf (simres, "%e", outtimeaccur);
    for (rp = rp_begin; rp; rp = rp -> next) {
	wr_path (simres, rp -> path);
    }
    fprintf (simres, "\n");

    if (tester_output) {
	OPENW (simtst, fn_tst);
	fprintf (simtst, "%e", outtimeaccur);
	for (rp = rp_begin; rp; rp = rp -> next) {
	    wr_path (simtst, rp -> path);
	}
	fprintf (simtst, "\n");
    }

    tbreak = tsimduration + 1;
    nbr = arr_size ((char *)tdumps);
    for (i = 0; i < nbr; i++) {
        tdmp = D_ROUND (tdumps[i] * sigtoint);
        if (tdmp < tbreak && tdmp > tcurr) tbreak = tdmp;
    }

    tlastwritten = -1;

    intrupm.on = FALSE;
    intrupm.timebreak = FALSE;
    intrupm.outputchange = FALSE;
    intrupm.signalbreak = FALSE;

    fflush (simres);
}

void interrupt ()
{
    NODE *n;
    NODE_REF_LIST *pl, *ref_el;
    ABSTRACT_OUTPUT * ab_el;
    ABSTRACT_VALUE * ab_val;
    char s;
    char stst;
    char *val;
    char *p;
    int sign_bit;
    int two_compl;
    int i, nbr, nbr_inverted;
    simtime_t tdmp;

    if (intrupm.timebreak) {
        dump_state (tcurr);
	tbreak = tsimduration + 1;
        nbr = arr_size ((char *)tdumps);
        for (i = 0; i < nbr; i++) {
            tdmp = D_ROUND (tdumps[i] * sigtoint);
            if (tdmp < tbreak && tdmp > tcurr) tbreak = tdmp;
        }
        intrupm.on = FALSE;
        intrupm.timebreak = FALSE;
        return;
    }

    nbr_inverted = arr_size ((char *)prinvert);
    nbr = 0;

    fprintf (simres, "%lld", tcurr);
    if (tester_output) fprintf (simtst, "%lld", tcurr);

    for (pl = pl_begin; pl; pl = pl -> next) {
	if (pl -> nx >= 0) {
	    val = "X";
	    if (pl -> nx < N_cnt) {
		n = &N[pl -> nx];
		switch (LSTATE (n)) {
		    case H_state: s = 'h'; break;
		    case L_state: s = 'l'; break;
                    default: s = 'x';
		}
	    }
	    else {
		ab_el = abstractl_begin;
		i = pl -> nx - N_cnt;
		while (i > 0) {
		    ab_el = ab_el -> next;
		    i--;
		}
                ab_val = ab_el -> vals;
                while (ab_val) {
		    ref_el = ab_el -> inputs;
                    i = 0;
                    while (ref_el) {
                        if (ab_val -> in_values[i] != Dontcare) {
			    n = &N[ref_el -> nx];
			    if (ab_val -> in_values[i] != LSTATE (n)) break;
                        }
                        ref_el = ref_el -> next;
                        i++;
                    }
                    if (ref_el)
                        /* it did not match, try another min-term */
			ab_val = ab_val -> next;
                    else {
                        /* we found matching inputs ! */
			val = ab_val -> out_value;
			if (val[0] == '$') {
			    sign_bit = 0;
			    two_compl = 0;
			    if (val[1] == 's') {
				sign_bit = 1;
				p = val + 2;
			    }
			    else if (val[1] == 't') {
				two_compl = 1;
				p = val + 2;
			    }
			    else {
				p = val + 1;
			    }
			    if (strcmp (p, "dec") == 0) {
				val = nrl2int (ab_el -> inputs, 10, sign_bit, two_compl);
			    }
			    else if (strcmp (p, "oct") == 0) {
				val = nrl2int (ab_el -> inputs, 8, sign_bit, two_compl);
			    }
			    else if (strcmp (p, "hex") == 0) {
				val = nrl2int (ab_el -> inputs, 16, sign_bit, two_compl);
			    }
			    else if (strcmp (p, "bin") == 0) {
				val = nrl2int (ab_el -> inputs, 2, sign_bit, two_compl);
			    }
			}
                        ab_val = NULL;
                    }
                }
		s = 0;
	    }

            if (nbr < nbr_inverted && prinvert[ nbr ] == pl) {
                switch (s) {
                    case 'h': s = 'l'; break;
                    case 'l': s = 'h'; break;
                }
                nbr++;
            }

	    if (pl -> nx < N_cnt)
		putc (s, simres);
	    else
		fprintf (simres, "(%s)", val);

	    if (tester_output) {
		if (pl -> nx < N_cnt) {
		    n = &N[pl -> nx];
		    if (n -> inp && n -> forcedinfo -> initfstate != F_state)
		    /*
		    tswitch > tcurr && n -> forcedinfo -> initfstate == Forced)
			|| (n -> forcedinfo -> tswitch <= tcurr
			    && n -> forcedinfo -> tswitch_stab > tcurr
			    && n -> forcedinfo -> nextfstate == Forced)
			|| (n -> forcedinfo -> tswitch_stab <= tcurr
			    && n -> forcedinfo -> nextfstate == Forced)))
		    */
			stst = s - 'a' + 'A';
		    else
			stst = s;
		    putc (stst, simtst);
		}
		else
		    fprintf (simtst, "x");
		    /* cannot print abstract value in tst output */
	    }
	}
    }

    putc ('\n', simres);
    if (tester_output) putc ('\n', simtst);

    tlastwritten = tcurr;

    intrupm.on = FALSE;
    intrupm.timebreak = FALSE;
    intrupm.outputchange = FALSE;
    intrupm.signalbreak = FALSE;

    fflush (simres);
}

void endintrup ()
{
    int     c, i, k, l, nbr, pos;
    char    buf[51];
    char   *nodename;
    double  ct;
    simtime_t tnow, help = 0;
    NODE_REF_LIST *pl;
    ABSTRACT_OUTPUT *ab_el;

    if (simres) CLOSE (simres);
    if (simtst) CLOSE (simtst);

    OPENR (simres, fn_res);
    OPENW (simout, fn_out);

    if (debugsim) setbuf (simout, (char *) NULL);

    nbr = 0;
    for (ab_el = abstractl_begin; ab_el; ab_el = ab_el -> next) ++nbr;

    PALLOC (abstractl_len, nbr + 1, int);
    for (i = 0; i < nbr; i++) abstractl_len[i] = 1;

    PALLOC (abstractl_adj, nbr + 1, char);
    for (i = 0; i < nbr; i++) abstractl_adj[i] = 'l';

    /* go to begin of signal values */
    while ((c = getc (simres)) != '\n' && c != EOF);

    PALLOC (rf_simres, 1, RES_FILE);
    rf_simres -> offset = ftell (simres);
    rf_simres -> signals = rp_begin;
    rf_simres -> sig_cnt = pnodes_cnt;

    /* skip comma's */
    while (pl_begin && pl_begin -> nx < 0) pl_begin = pl_begin -> next;
    if ((pl = pl_begin)) {
	do {
	    pl_end = pl;
	    while ((pl = pl -> next) && pl -> nx < 0);
	} while (pl);
	pl_end -> next = NULL;
    }

    k = 1;
    while (fscanf (simres, "%lld", &help) == 1) {
	++k;
	pl = pl_begin;
	while ((c = getc (simres)) != '\n' && c != EOF) {
	    if (!pl) slserror (fn_res, k, ERROR1, "too many values", NULL);
	    if (c == '(') {
		i = pl -> nx - N_cnt;
		if (i < 0) slserror (fn_res, k, ERROR1, "incorrect '('", NULL);
		l = 0;
		while ((c = getc (simres)) != ')' && c != '\n' && c != EOF) {
		    if (!l && (isdigit(c) || c == '-')) abstractl_adj[i] = 'r';
		    l++;
		}
		if (c != ')') slserror (fn_res, k, ERROR1, "missing ')'", NULL);
		if (l > abstractl_len[i]) abstractl_len[i] = l;
	    }
	    while ((pl = pl -> next) && pl -> nx < 0);
	}
	if (pl) slserror (fn_res, k, ERROR1, "too less values", NULL);
    }

    ia_ou_max = 2; tnow = 100;
    while (tnow > 0 && help >= tnow) { ++ia_ou_max; tnow *= 10; }

    ia_ou_fac = 0;
    ct = outtimeaccur / outtimeunit;
    while (ct < 0.5) { ct *= 10; ++ia_ou_fac; }
    if (ia_ou_fac) {
	if (ia_ou_fac > ia_ou_max) ia_ou_max = ia_ou_fac;
	++ia_ou_max; /* add point */
    }
    else while (ct > 5.) { ct /= 10; --ia_ou_fac; ++ia_ou_max; } /* add zero */

    if (ia_ou_max < 9) ia_ou_max = 9; /* minimum field width */

    pos = ia_ou_max + 2;
    for (pl = pl_begin; pl; pl = pl -> next) {
	if (pl -> nx < N_cnt)
	    pos = pos + 2;
	else
	    pos = pos + abstractl_len[pl -> nx - N_cnt] + 1;
    }

    outwidth = maxpagewidth;
    if (pos <= 80 && outwidth > 80) outwidth = 80;
    if (pos > 80 && outwidth > pos) outwidth = pos;

    prequals ();
    prcenter ("S L S");
    sprintf (buf, "version: %s", VERSION);
    prcenter (buf);
    prcenter ("S I M U L A T I O N   R E S U L T S");
    prequals ();

    outresults ();

    if (printraces && ol_begin) {
	fprintf (simout, "  races occurred during simulation :\n");
	fprintf (simout, "%*s  nodes\n", ia_ou_max, "time");
	while (ol_begin) {
	    tnow = ol_begin -> t;
	    prtime (tnow);
	    fprintf (simout, " ");
	    pos = ia_ou_max + 2;
	    while (ol_begin && ol_begin -> t == tnow) {
		nodename = hiername (ol_begin -> ntx);
		l = strlen (nodename);
		if (pos > ia_ou_max + 2 && pos + l > outwidth) {
		    fprintf (simout, "\n%*s", ia_ou_max+1, "");
		    pos = ia_ou_max + 2;
		}
		fprintf (simout, " %s", nodename);
		pos += l + 1;
		ol_begin = ol_begin -> next;
	    }
	    fprintf (simout, "\n");
	}
	prequals ();
    }

    fprintf (simout, "  network : %s\n", netwname);
    fprintf (simout, "  nodes   : %d\n", N_cnt);
    fprintf (simout, "  pnodes  : %d\n", pnodes_cnt);
    if (printdevices || printstatis) {
        prequals ();
	fprintf (simout, "\n");
    }
    if (printdevices) {
	fprintf (simout, "  transistors : %7d\n", T_cnt - res_cnt);
	fprintf (simout, "  resistors   : %7d\n", res_cnt);
        /* sls_exp has removed all capacitors as separate devices
	fprintf (simout, "  capacitors  : %7d\n", I_cnt);
        */
	fprintf (simout, "  functions   : %7d\n", F_cnt);
	fprintf (simout, "\n");
    }
    if (printstatis) {
	fprintf (simout, "  simulation timepoints        : %7d\n", timepoint_cnt);
	fprintf (simout, "  simulation steps             : %7d\n", simstep_cnt);
	fprintf (simout, "  essential logic events       : %7d\n", events_cnt);
	fprintf (simout, "  max. nodes in vicinity       : %7d\n", act_maxnvicin);
	fprintf (simout, "  max. transistors in vicinity : %7d\n", act_maxtvicin);
	fprintf (simout, "\n");
    }
    prequals ();

    CLOSE (simout);
    CLOSE (simres);
}

NODE_REF_LIST * pl_begpage;
NODE_REF_LIST * pl_nextpage;
STRING_REF * sr_beg;
STRING_REF * sr_end;

static void outresults ()
{
    int n;
    RES_PATH * rp;
    STRING_REF * sr;

    sr_beg = NULL;
    n = 0;
    for (rp = rf_simres -> signals; rp; rp = rp -> next) {
	sr = names_from_path (0, rp -> path);
	if (!sr_beg) sr_beg = sr;
	else sr_end -> next = sr;
	while (sr) {
	    sr_end = sr; ++n;
	    sr = sr -> next;
	}
    }
    if (n != pnodes_cnt) slserror (fn_res, 0, ERROR1, "incorrect signal count", NULL);

    for (pl_begpage = pl_begin; pl_begpage; pl_begpage = pl_nextpage) {
	signamesresults ();
	lineresults ();
    }
}

int nbr_on_line;

static void signamesresults ()
{
    int     i, j, w, yes;
    int     ready;
    int     currpos;
    double  t;
    NODE_REF_LIST *pl;
    STRING_REF *sr;

    j = 0;
    ready = 0;
    while (!ready) {
	ready = 1;
	if (j == 0) {
	    fprintf (simout, " time in");
	    ready = 0;
	    w = ia_ou_max - 5;
	}
	else if (j == 1) {
	    i = 0;
	    if (outtimeunit > 5.0)
		for (t = outtimeunit; t > 5.0; t /= 10) ++i;
	    else
		for (t = outtimeunit; t < 0.5; t *= 10) ++i;
	    w = ia_ou_max;
	    while (w-- > 9) putc (' ', simout);
	    fprintf (simout, "1e%c%02d sec", outtimeunit < 0.5 ? '-': '+', i);
	    w = 3;
	}
	else {
	    w = ia_ou_max + 3;
	}
	fprintf (simout, "%*s", w, "| ");
	currpos = ia_ou_max + 2;

        nbr_on_line = 0;

        sr = sr_beg;
	pl = pl_begpage;

	yes = 0;
	if (pl -> nx >= N_cnt) {
	    yes = abstractl_len[pl -> nx - N_cnt];
	    currpos += yes + 1;
	}
	else currpos += 2;

	for (i = 0;;) {
	    if (pl -> nx < 0) i += 2;
	    else {
		if (yes && abstractl_adj[pl -> nx - N_cnt] == 'r') { /* right adjust */
		    i += yes - 1;
		    yes = 0;
		}
		w = strlen (sr -> str);
	        if (j < w - 1) ready = 0;
	        if (j < w) {
		    while (i-- > 0) putc (' ', simout);
		    i = (sr -> str)[j];
		    if (i == '[' || i == ']') i = '*';
		    putc (i, simout);
		    i = 1;
	        }
		else {
		    i += 2;
		}
		if (yes) { /* left adjust */
		    i += yes - 1;
		    yes = 0;
		}
		sr = sr -> next;
	    }
	    nbr_on_line++;
	    pl = pl -> next;
	    if (pl) {
		if (pl -> nx >= N_cnt) {
		    yes = abstractl_len[pl -> nx - N_cnt];
		    currpos += yes + 1;
		}
		else currpos += 2;
		if (currpos > outwidth) break;
	    }
	    else break;
	}
	putc ('\n', simout);
	j++;
    }

    while (pl && pl -> nx < 0) pl = pl -> next;
    pl_nextpage = pl;
    sr_beg = sr;

    prequals ();
}

static void lineresults ()
{
    int *last_vals;
    int i, j, k, l, s, w;
    simtime_t t;
    NODE_REF_LIST *pl;
    char buf[132];

    if (!printremain) last_vals = NULL;
    else PALLOC (last_vals, nbr_on_line, int);

    /* go to begin of signal values */
    fseek (simres, rf_simres -> offset, 0);

    k = 1;
    while (fscanf (simres, "%lld", &t) > 0) {
	++k;
        prtime (t);
        fprintf (simout, " |");

        for (pl = pl_begin; pl != pl_begpage; pl = pl -> next)
	    if (pl -> nx >= 0) {
		s = getc (simres);
		if (s == '(') {
		    while (getc (simres) != ')');
		}
	    }

	l = 0;
	for (i = 0; pl != pl_nextpage; ++i, pl = pl -> next) {

	    if (pl -> nx < 0) l += 2;
	    else {
		while (l > 0) { putc (' ', simout); --l; }
		s = getc (simres);
		if (pl -> nx >= N_cnt) {
		    if (s != '(') slserror (fn_res, k, ERROR1, "missing '('", NULL);
		    putc (' ', simout);
		    w = abstractl_len[pl -> nx - N_cnt];
		    j = 0;
		    while ((s = getc (simres)) != ')' && j < w) buf[j++] = s;
		    if (s != ')') slserror (fn_res, k, ERROR1, "missing ')'", NULL);
		    if (!j) buf[j++] = '?';
		    buf[j] = '\0';
		    if (abstractl_adj[pl -> nx - N_cnt] == 'r') j = w; else l = w - j;
		    fprintf (simout, "%*s", j, buf);
		}
		else {
		    if (last_vals && last_vals[i] == s) s = '.';
		    else {
			if (last_vals) last_vals[i] = s;
			switch (s) {
			    case 'h': s = '1'; break;
			    case 'l': s = '0'; break;
			    case 'x': break;
			    case 'f': break;
			    default:  s = '?';
			}
		    }
		    putc (' ', simout);
		    putc (s, simout);
		}
	    }
        }
	putc ('\n', simout);

	while ((s = getc (simres)) != '\n' && s != EOF);
    }

    if (last_vals) CFREE (last_vals);

    prequals ();
}

static void prtime (simtime_t t)
{
    char buf[80];
    int i, l, n;

    sprintf (buf, "%lld", t);
    l = strlen (buf);

    if (ia_ou_fac > 0) { /* outtimeaccur < outtimeunit, digits after point */
	i = ia_ou_max - 1;
	n = l - ia_ou_fac; /* digits before point */
	if (n > 0) {
	    for (; i > l; --i) putc (' ', simout);
	}
	else if (i-- > ia_ou_fac) {
	    for (; i > ia_ou_fac; --i) putc (' ', simout);
	    putc ('0', simout);
	}
	for (i = 0; i < n; ++i) putc (buf[i], simout);
	putc ('.', simout);
	for (; n < 0; ++n) putc ('0', simout);
	for (; i < l; ++i) putc (buf[i], simout);
    }
    else { /* outtimeunit <= outtimeaccur */
	if (ia_ou_fac < 0 && t) {
	    for (i = 0; i > ia_ou_fac; --i) buf[l++] = '0';
	}
	i = ia_ou_max - l;
	for (; i > 0; --i) putc (' ', simout);
	for (i = 0; i < l; ++i) putc (buf[i], simout);
    }
}

static void prequals ()
{
    int cnt = outwidth < 35 ? 35 : outwidth;
    while (cnt-- > 0) putc ('=', simout);
    putc ('\n', simout);
}

static void prcenter (char *s)
{
    int cnt = outwidth < 35 ? 35 : outwidth;
    int l = strlen (s);

    cnt = (cnt - l) / 2;
    while (cnt-- > 0) putc (' ', simout);
    fprintf (simout, "%s\n", s);
}

void nextsig_update (NODE *n)
{
    simtime_t tswitch;
    SIGNALEVENT nextevent;

    if (n -> forcedinfo -> tswitch == tcurr) {
	n -> forcedinfo -> initfstate = n -> forcedinfo -> nextfstate;
	n -> forcedinfo -> tswitch = -1;
	n -> forcedinfo -> tswitch_stab = -1;
    }

    nextevent.time = tcurr;

    do {
	getsignal (n, nextevent.time, &nextevent);
    }
    while (nextevent.time >= 0 && nextevent.time <= tsimduration
    && nextevent.val == n -> forcedinfo -> nextfstate && tcurr >= 0);

    if (nextevent.time >= 0) {
	tswitch = nextevent.time;
	n -> forcedinfo -> nextfstate = nextevent.val;
        n -> forcedinfo -> stabfstate = nextevent.val;
	n -> forcedinfo -> tswitch = tswitch;
	n -> forcedinfo -> tswitch_stab = tswitch;
    }
}

void checkbreak (NODE *n, unsigned oldlstate, unsigned oldtype)
{
    if (n -> state != oldlstate) {
        if (n -> breaksig) {
	    intrupm.on = TRUE;
	    intrupm.signalbreak = TRUE;
        }

        intrupm.outputchange = TRUE;
        if (outonchange) {
	    intrupm.on = TRUE;
        }
    }
}

static void dump_state (simtime_t t)
{
    char fn_dump[40];
    FILE * fp_dump;
    NODE * n;
    int cnt;
    UPAIR * up;

    t = D_ROUND (t / sigtoint);
    sprintf (fn_dump, "dump.%lld", t);

    OPENW (fp_dump, fn_dump);

    fprintf (fp_dump, "dumpfile for %s\n\n", netwname);
    fprintf (fp_dump, "%d\n", N_cnt);

    n = &N[0];
    for (cnt = 0; cnt < N_cnt; cnt++) {
        up = uminmax (n);
        fprintf (fp_dump, "%d %d %d\n",
        (int)(up -> umin), (int)(up -> umax), (int)(n -> state));
        n++;
    }

    fprintf (fp_dump, "|");

    for (cnt = 0; cnt < FS_cnt; cnt++) {
        if (FS[cnt] == '*') {
            cnt += sizeof (int);
            cnt += (*(int *)(FS + cnt) + 1) * SIZE_PTR_INT - 1;
        }
        else {
            fwrite (FS + cnt, 1, 1, fp_dump);
        }
    }

    CLOSE (fp_dump);
}

static char *nrl2int (NODE_REF_LIST *nrl, int fmt, int sign_bit, int two_compl)
{
    unsigned long w;
    unsigned long result;
    int i;
    int nr_vbits;
    int max_vbits;
    char sign = '\0';
    NODE *n;
    NODE_REF_LIST *nrl_local;
    NODE_REF_LIST *nrl_el;
    static char buf[132];

    if (sign_bit || two_compl) {
	/* the first bit is a sign bit */
	if (!nrl) {
	    strcpy (buf, "x");
	    goto end_of_nrl2int;
	}
	nrl_local = nrl -> next;
    }
    else {
	nrl_local = nrl;
    }

    max_vbits = sizeof (long) * 8;

    nr_vbits = 0;
    nrl_el = nrl_local;
    while (nrl_el) {
	nr_vbits++;
	nrl_el = nrl_el -> next;
    }

    result = 0;

    i = 0;
    nrl_el = nrl_local;
    while (nrl_el) {
	n = &N[nrl_el -> nx];
	switch (LSTATE (n)) {
	    case H_state:
		if (((fmt == 8 || fmt == 16 || fmt == 2) && nr_vbits - i > max_vbits)
		     || (fmt == 10 && nr_vbits - i > max_vbits - 1)) {
		    strcpy (buf, "<overflow>");
		    goto end_of_nrl2int;
		}
		result = result + (1 << (nr_vbits - i - 1));
		break;
	    case L_state:
		break;
	    case X_state:
		strcpy (buf, "x");
		goto end_of_nrl2int;
	}
	nrl_el = nrl_el -> next;
	i++;
    }

    if (sign_bit) {
	n = &N[nrl -> nx];
	switch (LSTATE (n)) {
	    case H_state:
		sign = '-';
		break;
	    case L_state:
		sign = '+';
		break;
	    case X_state:
		strcpy (buf, "x");
		goto end_of_nrl2int;
	}
    }

    if (two_compl) {
	n = &N[nrl -> nx];
	switch (LSTATE (n)) {
	    case H_state:
		result = result - (1 << nr_vbits);
		sign = '-';
		result = -result;
		break;
	    case L_state:
		break;
	    case X_state:
		strcpy (buf, "x");
		goto end_of_nrl2int;
	}
    }

    if (fmt == 2) {
	w = 1 << (nr_vbits - 1);
        i = 0;
	while (w > 0) {
	    if (result & w)
                buf[i] = '1';
            else
                buf[i] = '0';
            i++;
            w = w >> 1;
	}
        buf[i] = '\0';
    }
    else if (fmt == 8)
	sprintf (buf, "%lo", result);
    else if (fmt == 16)
	sprintf (buf, "%lx", result);
    else
	sprintf (buf, "%ld", result);

    if (sign == '-') {
	i = strlen (buf);
	while (i >= 0) {
	    buf[i + 1] = buf[i];
	    i--;
	}
	buf[0] = sign;
    }

end_of_nrl2int:
    return (buf);
}
