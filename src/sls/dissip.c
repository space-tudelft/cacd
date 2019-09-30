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

static void dis_node (NODE *n);
static int  disnbr (NODE *n);

STRING_REF * disnames = NULL;
STRING_REF * end_disnames;

double ** disvalues = NULL;
int nbr_disvalues;
int nbr_dissteps;

NODE ** nsources;

simtime_t interval;

void dis_addname (PATH_SPEC *path) /* stores node name */
{
    STRING_REF * str_ref;

    str_ref = names_from_path (0, path);

    if (disnames) {
	end_disnames -> next = str_ref;
    }
    else {
	disnames = end_disnames = str_ref;
    }

    while (end_disnames -> next != NULL) {
        end_disnames = end_disnames -> next;
    }
}

void init_dis ()
{
    NODE_REF_LIST * nrl;
    int i;

    if (simperiod <= 0) {
	slserror (NULL, 0, WARNING, "no appropriate simperiod specified for dissipation statistics", NULL);
	dissip = 0;
    }

    if (! proclogic) {
	slserror (NULL, 0, WARNING, "dissipation statistics not valuable for simulation at level 1", NULL);
	dissip = 0;
    }
    if (!dissip) return;

    nbr_disvalues = 0;
    for (nrl = disl_begin; nrl; nrl = nrl -> next) nbr_disvalues++;

    if (disperiod > 0 && disperiod < simperiod) {
	nbr_dissteps = simperiod / disperiod;
	interval = sigtoint * disperiod * nbr_dissteps;
	if (interval) {
	    unsigned long long max = 0xffffffffffffffffULL;
	    max /= interval;
	    if (max > 10000) max = 10000;
	    if (nbr_dissteps > max) {
		fprintf (stderr, "-- nbr_dissteps=%d (max=%llu)\n", nbr_dissteps, max);
		slserror (NULL, 0, WARNING, "too many dissipation steps", NULL);
		dissip = 0;
	    }
	}
    }
    else {
	nbr_dissteps = 0;
	interval = sigtoint * simperiod;
    }
    if (!interval) {
	slserror (NULL, 0, WARNING, "no appropriate interval for dissipation statistics", NULL);
	dissip = 0;
    }
    if (!dissip) return;

    PALLOC (disvalues, nbr_disvalues + 1, double *);
    for (i = 0; i < nbr_disvalues + 1; i++) {
	PALLOC (disvalues[i], nbr_dissteps + 1, double);
    }

    PPALLOC (nsources, N_cnt, NODE)
}

void vicin_nsource_dis () /* finds source node(s) for vicinity */
{
    int cnt;
    int lsn;
    NODE ** nn;
    NODE * n;
    NODE * source_H;
    NODE * source_L;

    source_H = NULL;
    source_L = NULL;

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) {
	    lsn = LSTATE (n);
	    if (!source_H && lsn == H_state) { source_H = n; if (source_L) break; }
	    else
	    if (!source_L && lsn == L_state) { source_L = n; if (source_H) break; }
	}
    }

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

        nsources [n - N] = NULL;

	if ((unsigned)(n -> svmin + n -> svmax) > (unsigned)(n -> ivmin + n -> ivmax)) {
	    if (source_H && source_H -> dissip)
		nsources [n - N] = source_H;
	}
	else if ((unsigned)(n -> svmin + n -> svmax) < (unsigned)(n -> ivmin + n -> ivmax)) {
	    if (source_L && source_L -> dissip)
		nsources [n - N] = source_L;
	}
    }
}

void vicin_dis ()
/* computes for each node in the vicinity the */
/* dissipation due to the last voltage change */
{
    NODE **nn, *n;
    int cnt;

    nn = vicin.nodes;
    for (cnt = vicin.node_cnt; cnt > 0; cnt--) {
	n = *nn++;

	if (n -> type == Forced) continue;
	dis_node (n);
    }
}

void forced_dis (NODE *n, int old, int new)
/* computes dissipation for a forced */
/* node that changes logic state */
{
    static int nbr;
    double vstep, v;

    if (old == new) return;

    vstep = vHtmp;

    if (old == X_state || new == X_state) vstep /= 2;

    if (tcurr < interval) {

	v = 0.5 * n -> statcap * vstep * vstep;
	disvalues [nbr_disvalues][nbr_dissteps] += v;

	if (nbr_dissteps) {
	    nbr = (unsigned long long)tcurr * nbr_dissteps / interval;
	    disvalues [nbr_disvalues][nbr] += v;
	}

	if (n -> dissip) {
	    int dn = disnbr (n);
	    disvalues [dn][nbr_dissteps] += v;
	    if (nbr_dissteps) disvalues [dn][nbr] += v;
	}
    }
}

void dis_end () /* ends dissipation book-keeping and writes file */
{
    int i, j, k;
    NODE_REF_LIST *nrl, *nrl_pg;
    STRING_REF *srl, *srl_pg;
    char str[20];
    int cnt;
    FILE * fp;
    char fn[16];
    double val;
    int dn;

    sprintf (fn, "%s.dis", netwname);
    OPENW (fp, fn);

    for (cnt = 0; cnt < N_cnt; cnt++) {
	if (N[cnt].type != Forced) dis_node (N + cnt);
    }

    fprintf (fp, "\nDynamic Dissipation [in Watts]\n\n");

    if (nbr_dissteps)
	val = disperiod * nbr_dissteps * sigtimeunit;
    else
	val = simperiod * sigtimeunit;
    fprintf (fp, "--- time interval [0, %e)\n\n", val);

    j = 0;
    nrl_pg = disl_begin;
    srl_pg = disnames;
    while (j < nbr_disvalues + 1) {

        if (nbr_dissteps)
	    fprintf (fp, "time [in sec]  ");
	else
	    fprintf (fp, "               ");

	i = 0;
        nrl = nrl_pg;
	srl = srl_pg;
	while (i + j <= nbr_disvalues && i < 8) {
	    if (nrl == NULL)
		fprintf (fp, "<total netw.> ");
	    else {
		while (nrl -> nx < 0) {
		    nrl = nrl -> next;
		}
		fprintf (fp, "%-13.13s ", srl -> str);
		nrl = nrl -> next;
		srl = srl -> next;
	    }
	    i++;
	}
	fprintf (fp, "\n");

	for (k = 0; k <= nbr_dissteps; k++) {

            if (k == nbr_dissteps)
		fprintf (fp, "average        ");
	    else {
		sprintf (str, "%e", k * disperiod * sigtimeunit);
		fprintf (fp, "%-13.13s  ", str);
	    }

	    i = 0;
	    nrl = nrl_pg;
	    while (i + j <= nbr_disvalues && i < 8) {
		if (nrl == NULL)
		    val = disvalues[nbr_disvalues][k];
		else {
		    while (nrl -> nx < 0) {
			nrl = nrl -> next;
		    }
		    dn = disnbr (N + nrl -> nx);
		    val = disvalues[ dn ][k];
		    nrl = nrl -> next;
		}
		if (k == nbr_dissteps)
		    val = val / (interval * outtimeaccur);
		else
		    val = val / (disperiod * sigtoint * outtimeaccur);
		sprintf (str, "%.5e", val);
		fprintf (fp, "%-13.13s ", str);
		i++;
	    }
	    fprintf (fp, "\n");
	}
	fprintf (fp, "\n");
	j += i;
	nrl_pg = nrl;
	srl_pg = srl;
    }

    CLOSE (fp);
}

static void dis_node (NODE *n)
/* computes dissipation for one node and */
/* finds the source node for that node */
{
    static int nbr;
    double consumed, minpart, maxpart, vstep;
    simtime_t tbegin, tend, tmiddle;

    if (tcurr >= n -> tstabmin)
	minpart = 1;
    else
	minpart = (double)(tcurr - n -> tstabmin + n -> Ttmin) / n -> Ttmin;

    if (tcurr >= n -> tstabmax)
	maxpart = 1;
    else
	maxpart = (double)(tcurr - n -> tstabmax + n -> Ttmax) / n -> Ttmax;

    minpart *= ((int)(n -> svmin) - (int)(n -> ivmin));
    maxpart *= ((int)(n -> svmax) - (int)(n -> ivmax));

    tbegin = (n -> tstabmin - n -> Ttmin
	    + n -> tstabmax - n -> Ttmax) / 2;
    if (tbegin < 0) tbegin = 0;

    tend = (n -> tstabmin + n -> tstabmax) / 2;
    if (tend < 0) tend = 0;
    if (tend > tcurr) tend = tcurr;

    tmiddle = (tend + tbegin) / 2;

    if (tmiddle < interval) {
	vstep = (minpart + maxpart) / 2;
	vstep = vHtmp * vstep / vH;

	consumed = 0.5 * n -> statcap * vstep * vstep;
	disvalues [nbr_disvalues][nbr_dissteps] += consumed;

	if (nbr_dissteps) {
	    nbr = (unsigned long long)tmiddle * nbr_dissteps / interval;
	    disvalues [nbr_disvalues][nbr] += consumed;
	}

	if (nsources [n - N] != NULL) {
	    int dn = disnbr (nsources [n - N]);
	    disvalues [dn][nbr_dissteps] += consumed;
	    if (nbr_dissteps) disvalues [dn][nbr] += consumed;
	}
    }
}

static int disnbr (NODE *n) /* returns the node list number */
{
    NODE_REF_LIST * nrl;
    int cnt, nx;

    cnt = 0; nx = n - N;
    for (nrl = disl_begin; nrl; nrl = nrl -> next) {
	if (nrl -> nx == nx) return (cnt);
	cnt++;
    }
    slserror (NULL, 0, ERROR1, "node number not found", NULL);
    return (cnt);
}
