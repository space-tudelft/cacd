/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void prMaskinfo (void);
Private void prBitmasks (void);
Private void prElems (void);
Private int howManyCond (struct layCondRef *cond);
Private void prKeytab (void);
Private void prKeytab2 (void);
Private void prOtherInfo (void);
Private void prResizeInfo (void);
Private void itocc (int i, int *c0, int *c1);
Private char *itostr (int i);
#ifdef __cplusplus
  }
#endif

#define P_E fprintf(stderr,

#define MajorProtNr  2
#define MinorProtNr  4

extern int maskWidth;
extern mask_t filterBitmask;
extern mask_t subBitmask;
extern mask_t *newMaskColor;

static int layC_cnt;
static int elR_cnt;

static FILE *fp_out;

static int ElemMaxNbr = 0;
static int ElemMaxNbr2 = 0;
static int elemCnt;
static int condCnt;
static int CondNbr[5];
static int CondMaxNbr;

static char *resSortTab[MAXRESSORT];
int resSortTabSize;
static char *capSortTab[MAXCAPSORT];
int capSortTabSize;
static short capJuncTab[MAXCAPSORT];

static int printMask (int e, char *name, int j)
{
    j += strlen (name);
    if (e) ++j;
    if (++j > 60) { j = 0; P_E "\n        "); }
    P_E " %s%s", e ? "-" : "", name);
    return j;
}

void prTabs (char *outfile)
{
    int i, j, n;

    for (i = 0; i < 5; i++) CondNbr[i] = 0;

    fp_out = outfile ? cfopen (outfile, "w") : stdout;

    if (prVerbose) {
	fprintf (fp_out, "# %d 0\n", MajorProtNr);
	fprintf (fp_out, "(code_nr)\n%d\n", printNonCoded ? 0 : 2);
    }
    else {
	fprintf (fp_out, "# %d %d\n", MajorProtNr, MinorProtNr);
	fprintf (fp_out, "%d\n", printNonCoded ? 0 : 2);
    }

    prMaskinfo ();

    prBitmasks ();

    prResizeInfo ();

    prElems ();

    prKeytab ();

    prKeytab2 ();

    prOtherInfo ();

    if (outfile) fclose (fp_out);

    if (!silent) {
	struct layerRef *k;
	double d;

	n = j = 0;
	P_E "\n-- keys:");
	for (k = keylist; k; k = k -> next) {
	    ++n;
	    j = printMask (0, maskname (k -> lay -> mask), j);
	}

	/* separate EDGE keylist */
	j = 0;
	P_E "\n-- keys2:");
	for (k = keylist2; k; k = k -> next) {
	    if (k -> lay -> occurrence == EDGE)
		j = printMask (1, maskname (k -> lay -> mask), j);
	    else
		j = printMask (0, maskname (k -> lay -> mask), j);
	}

	if (n < maxKeys) {
	    j = 0;
	    P_E "\n-- non-keys:");
	    for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) {
		if (maskTransf[i] >= n && maskTransf[i] < maxKeys) j = printMask (0, maskname (i), j);
	    }
	}

	P_E "\n-- number of keys: %d + %d (%d)\n", sSlotCnt, oSlotCnt, maxKeys);
	P_E "-- number of keys2: %d + %d (%d)\n", sSlotCnt2, eSlotCnt, maxKeys2);
	P_E "-- number of key slots: %d (%d)\n", nbrKeySlots, nbrKeySlots2);
	P_E "-- maximum number of elements per key slot: %d (%d)\n", ElemMaxNbr, ElemMaxNbr2);
	P_E "-- maximum number of additional conditions per element: %d\n", CondMaxNbr);
	d = condCnt; if (elemCnt) d /= elemCnt;
	P_E "-- average number of additional conditions per element: %.3f\n\n", d);
	P_E "-- add. cond.  :");
	for (i = 0; i <= CondMaxNbr; i++) {
	    if (CondNbr[i] > 99) P_E " %3d", i);
	    else P_E " %2d", i);
	}
	P_E "\n   no. of elem.:");
	for (j = i = 0; i <= CondMaxNbr; i++) {
	    j += CondNbr[i];
	    P_E " %2d", CondNbr[i]);
	}
	P_E " (%d)\n\n", j);
    }
}

Private void prResizeInfo ()
{
    char *fmt1, *fmt2;
    int i, j;
    struct resizeCond *resCond;

    if (prVerbose) fprintf (fp_out, "(resize_cnt)\n");
    fprintf (fp_out, "%d\n", resize_cnt);
    if (prVerbose && resize_cnt)
	fprintf (fp_out, "(r#  name     m#   value      cNr  cPres cAbs ...)\n");
    for (i = 0; i < resize_cnt; i++) {
	if (prVerbose)
	    fprintf (fp_out, "(%2d) %-6s %4d  %+.3e %3d", i, resizes[i].newmaskname,
		resizes[i].id, resizes[i].val, resizes[i].condcnt);
	else
	    fprintf (fp_out, "%s %d %g %d", resizes[i].newmaskname,
		resizes[i].id, resizes[i].val, resizes[i].condcnt);

	if (prVerbose) { fmt1 = " %7s"; fmt2 = " %4s"; }
	else fmt1 = fmt2 = " %s";

	resCond = resizes[i].cond;
	for (j = 0; j < resizes[i].condcnt; j++) {
	    fprintf (fp_out, fmt1, colorIntStr (&resCond -> present));
	    fprintf (fp_out, fmt2, colorIntStr (&resCond -> absent));
	    resCond = resCond -> next;
	}
	fprintf (fp_out, "\n");
    }
}

Private void prMaskinfo ()
{
    mask_t color;
    char *s;
    int gln, i, j, k, l, w = 8;

    i = maxprocmasks;
    if (IS_COLOR (&subBitmask)) ++i;
    k = (31 + i) / 32;
    if (prVerbose) fprintf (fp_out, "(needed ncol32)\n%d\n", k);
    else fprintf (fp_out, "%d\n", k);

    if (prVerbose) fprintf (fp_out, "(nomasks)\n");
    fprintf (fp_out, "%d\n", procdata -> nomasks + subdata -> nomasks + 1);
    /* Note: @sub always added to the mask-list */

    if (prVerbose) {
	for (k = i = 0; i < procdata -> nomasks; ++i) if (maskTransf[i] > k) k = maskTransf[i];
	COLORINITINDEX (color, k);
	if ((w = strlen (colorIntStr (&color))) < 8) w = 8;
	fprintf (fp_out, "(  m#  %-*s ct %*s cd# gln type drawColor)\n", maskWidth-1, "name", w, "color#");
    }
    for (i = 0; i < procdata -> nomasks; i++) {
	gln = 1;
	k = conducTransf[i] < 0 ? 0 : 1;
	l = procdata -> mask_type[i];
	COLORINITINDEX (color, maskTransf[i]);
	if (!(s = maskdrawcolor[i])) s = "glass";
	if (prVerbose)
	    fprintf (fp_out, "(%4d) %-*s %d %*s %3d  %d %3d   %s\n", i, maskWidth,
		procdata -> mask_name[i], k, w, colorIntStr (&color), conducTransf[i], gln, l, s);
	else
	    fprintf (fp_out, "%s %d %s %d %d %d %s\n",
		procdata -> mask_name[i], k, colorIntStr (&color), conducTransf[i], gln, l, s);
    }

    for (j = 0; j < subdata -> nomasks; i++, j++) {
	k = conducTransf[i] < 0 ? 0 : (conducTransf[i] < conducCntStd ? 1 : 2);
	if (k == 2 && conducTransf[i] >= conducCntPos) ++k;
	l = resizemask (i);
	if (l >= 0) { // resize of new mask (real mask)
	    gln = 1;
	    COLORINITINDEX (color, maskTransf[i]);
	    if (!IS_COLOR (&subBitmask)) { // look for negative mask
		struct resizeCond *rc;
		for (rc = resizes[l].cond; rc; rc = rc -> next) {
		    if (!IS_COLOR (&rc -> present)) {
			ASSERT (IS_COLOR (&rc -> absent));
			COLOR_ADD (subBitmask, color);
			break;
		    }
		}
	    }
	}
	else { gln = 0; color = newMaskColor[j]; }
	if (!(s = masknewcolor[j])) s = "glass";
	if (prVerbose)
	    fprintf (fp_out, "(%4d) %-*s %d %*s %3d %2d   0   %s\n", i, maskWidth,
		subdata -> mask_name[j], k, w, colorIntStr (&color), conducTransf[i], gln, s);
	else
	    fprintf (fp_out, "%s %d %s %d %d 0 %s\n",
		subdata -> mask_name[j], k, colorIntStr (&color), conducTransf[i], gln, s);
    }

    /* Note: @sub always added to the mask-list */
    {
	if (prVerbose)
	    fprintf (fp_out, "(%4d) %-*s 4 %*s %3d  0   0   %s\n", i, maskWidth,
		"@sub", w, "0", conducCnt++, masksubcolor);
	else
	    fprintf (fp_out, "@sub 4 0 %d 0 0 %s\n", conducCnt++, masksubcolor);
    }

    if (prVerbose) fprintf (fp_out, "(conducCnt)\n");
    fprintf (fp_out, "%d\n", conducCnt);
}

Private void prBitmasks ()
{
    if (prVerbose) fprintf (fp_out, "(keylist: maxKeys/sKeys/oKeys)\n");
    fprintf (fp_out, "%d %d %d\n", maxKeys, sSlotCnt, oSlotCnt);
    if (prVerbose) fprintf (fp_out, "(keylist2: maxKeys/sKeys/eKeys)\n");
    fprintf (fp_out, "%d %d %d\n", maxKeys2, sSlotCnt2, eSlotCnt);

    if (prVerbose) fprintf (fp_out, "(filterBitmask/subBitmask)\n");
    fprintf (fp_out, "%s\n", colorIntStr (&filterBitmask));
    fprintf (fp_out, "%s\n", colorIntStr (&subBitmask));
}

Private void prElems ()
{
    int i, j, w, nbr, elemtype;
    struct x_y *x_y, *x_y_first;
    int x_y_cnt, x_y_index;
    double a, b, p;
    double xn1, xn2, yn1, yn2, ylast;
    char *fmt;

    a = b = p = 0.0;
    CondMaxNbr = 0;
    elemCnt = 0;
    condCnt = 0;

    layC_cnt = 0;

    resSortTab[0] = "res";
    resSortTabSize = 1;

    for (i = 0; i < res_cnt; i++) {
	for (j = 0; j < resSortTabSize; j++) {
	    if (strsame (ress[i].sort, resSortTab[j])) break;
	}
	ress[i].sortNr = j;
	if (j == resSortTabSize) {
	    if (resSortTabSize >= MAXRESSORT) fatalErr ("too many", "conductor types!");
	    resSortTab[j] = ress[i].sort;
	    resSortTabSize++;
	}
    }

    for (i = 0; i < con_cnt; i++) {
	for (j = 0; j < resSortTabSize; j++) {
	    if (strsame (cons[i].sort, resSortTab[j])) break;
	}
	cons[i].sortNr = j;
	if (j == resSortTabSize) {
	    if (resSortTabSize >= MAXRESSORT) fatalErr ("too many", "conductor types!");
	    resSortTab[j] = cons[i].sort;
	    resSortTabSize++;
	}
    }

    capSortTab[0] = "cap";
    capJuncTab[0] = 0;
    capSortTabSize = 1;

    for (i = 0; i < cap_cnt; i++) {
	for (j = 0; j < capSortTabSize; j++) {
	    if (strsame (caps[i].sort, capSortTab[j])) break;
	}
	caps[i].sortNr = j;
	if (j == capSortTabSize) {
	    capSortTab[j] = caps[i].sort;
	    capJuncTab[j] = caps[i].junc;
	    capSortTabSize++;
	    if (caps[i].junc) {
		capSortTab[j + 1] = caps[i].sort;
		capSortTabSize++;
	    }
	    if (capSortTabSize >= MAXCAPSORT) fatalErr ("too many", "capacitance types!");
	}
	else if (caps[i].junc != capJuncTab[j]) {
	    if (j == 0)
		fatalErr ("must specify a type", "for a junction capacitance");
	    else
		fatalErr ("both specified as a normal capacitance and a junction capacitance:", caps[i].sort);
	}
    }

    if (prVerbose) fprintf (fp_out, "(resSortTabSize/resSorts)\n");
    fprintf (fp_out, "%d\n", resSortTabSize);
    for (i = 0; i < resSortTabSize; i++) {
	fprintf (fp_out, "%s\n", resSortTab[i]);
    }
    if (prVerbose) fprintf (fp_out, "(capSortTabSize/capSorts)\n");
    fprintf (fp_out, "%d\n", capSortTabSize);
    for (i = 0; i < capSortTabSize; i++) {
	fprintf (fp_out, "%s\n", capSortTab[i]);
    }

    /* Compute a, b and p parameters for lateral coupling capacitances
       and for edge capacitances from x_y values and print them.
       Format: Na
	       0   inf a0a b0a p0a
	       x1a y1a a1a b1a p1a
	       x2a y2a a2a b2a p2a
	       ...
	       xNa yNa aNa bNa pNa
	       Nb
	       x1b y1b a1b b1b p1b
	       ...
	       -1
    */

    if (prVerbose) {
	fprintf (fp_out, "(cap xy_values)\n");
	fprintf (fp_out, "(--- x ---)  (--- y ---)  (--- a ---)  (--- b ---)  (--- p ---)\n");
    }
    x_y_index = 0;
    for (i = 0; i < cap_cnt; i++) {

        if (!(x_y = caps[i].x_y_vals)) {
	    caps[i].x_y_index = -1;
	}
	else {
	    if (i && caps[i-1].x_y_vals == x_y) {
		caps[i].x_y_index = caps[i-1].x_y_index;
	    }
	    else { /* previous one not equal to this x_y vals */
		caps[i].x_y_index = x_y_index++;

		if (x_y -> x == 0) fatalErr ("first distance value equal 0 for cap element", caps[i].name);
		x_y_first = x_y;
		x_y_cnt = 1;
		while (x_y -> next) { x_y_cnt++; x_y = x_y -> next; }

		fprintf (fp_out, "%d\n", x_y_cnt + 1);

		if (caps[i].eltype == LATCAPELEM) {
		    x_y = x_y_first;
		    if (!x_y -> next) { /* only one x_y pair */
			a = x_y -> x * x_y -> y;
			b = 0.0;
			p = 1.0;
			fprintf (fp_out, "%e %e %e %e %e\n", 0.0, 9.999e9, a, b, p);
			fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, x_y -> y, a, b, p);
		    }
		    else {
			while (x_y -> next) {
			    xn1 = x_y -> x;
			    yn1 = x_y -> y;
			    xn2 = x_y -> next -> x;
			    yn2 = x_y -> next -> y;
			    if (xn2 <= xn1) {
			        fatalErr ("non-increasing distance values for cap element", caps[i].name);
			    }
			    if (yn1 < 0.0 || yn2 < 0.0) {
			        fatalErr ("negative capacitance values for cap element", caps[i].name);
			    }
			    if (yn2 >= yn1) {
			        fatalErr ("non-decreasing capacitance values for lateral cap element", caps[i].name);
			    }
			    if (yn1 > 0.0 && yn2 > 0.0) {
			        p = log (yn1 / yn2) / log (xn2 / xn1);
			        a = yn1 * pow (xn1, p);
			        b = 0.0;
			    }
			    else p = 0.0;
			    if (p < 1.0) {
			        p = 1.0;
			        a = (yn1 - yn2) / (1 / xn1 - 1 / xn2);
			        b = yn1 - a / xn1;
			    }
			    if (x_y == x_y_first) {
			        fprintf (fp_out, "%e %e %e %e %e\n", 0.0, 9.999e9, a, b, p);
			    }
			    fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, x_y -> y, a, b, p);
			    x_y = x_y -> next;
			}
			if (x_y -> y == 0.0) a = b = 0.0;
			fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, x_y -> y, a, b, p);
		    }
		}
		else { /* EDGECAPELEM */
		    ylast = x_y -> y;
		    x_y = x_y_first;
		    if (!x_y -> next) { /* only one x_y pair */
			b = p = 0.0;
			fprintf (fp_out, "%e %e %e %e %e\n", 0.0, 0.0, ylast, b, p);
			fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, ylast, ylast, b, p);
		    }
		    else {
       			while (x_y -> next) {
			    xn1 = x_y -> x;
			    yn1 = x_y -> y;
			    xn2 = x_y -> next -> x;
			    yn2 = x_y -> next -> y;
			    if (xn2 <= xn1) {
				fatalErr ("non-increasing distance values for cap element", caps[i].name);
			    }
			    if (yn1 < 0.0 || yn2 < 0.0) {
				fatalErr ("negative capacitance values for cap element", caps[i].name);
			    }
			    if (yn2 <= yn1 || ylast < yn2) {
				fatalErr ("non-increasing capacitance values for edge cap element", caps[i].name);
			    }
			    if (x_y == x_y_first) {
				p = - log (1.0 - yn1 / ylast) / xn1;
				fprintf (fp_out, "%e %e %e %e %e\n", 0.0, 0.0, ylast, 1.0, p);
			    }
			    if (yn2 < ylast) { /* yn2 != ylast */
				a = ylast;
				p = log ((ylast - yn1) / (ylast - yn2)) / (xn2 - xn1);
				b = (ylast - yn1) * exp (p * xn1) / ylast;
			    }
			    else {
				/* p equals the previous calculated p */
				b = (yn2 - yn1) / (yn2 * exp (-p * xn1) - yn1 * exp (-p * xn2));
				a = yn2 / (1.0 - b * exp (-p * xn2));
			    }
			    fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, x_y -> y, a, b, p);
			    x_y = x_y -> next;
			}
			fprintf (fp_out, "%e %e %e %e %e\n", x_y -> x, ylast, ylast, 0.0, 0.0);
		    }
		}
	    }
	}
    }
    fprintf (fp_out, "-1\n");  /* end of a,b,p list */

    w = 12;
    if (prVerbose) {
	for (i = 0; i < con_cnt; ++i) if ((j = strlen (cons[i].name)) > w) w = j;
	for (i = 0; i < cap_cnt; ++i) if ((j = strlen (caps[i].name)) > w) w = j;
	for (i = 0; i < res_cnt; ++i) if ((j = strlen (ress[i].name)) > w) w = j;
	for (i = 0; i < tor_cnt; ++i) if ((j = strlen (tors[i].name)) > w) w = j;
	for (i = 0; i < vdm_cnt; ++i) if ((j = strlen (vdms[i].name)) > w) w = j;
	for (i = 0; i < shp_cnt; ++i) if ((j = strlen (shps[i].name)) > w) w = j;
	for (i = 0; i < jun_cnt; ++i) if ((j = strlen (juns[i].name)) > w) w = j;
	for (i = 0; i < bjt_cnt; ++i) if ((j = strlen (bjts[i].name)) > w) w = j;
	for (i = 0; i < cnt_cnt; ++i) if ((j = strlen (cnts[i].name)) > w) w = j;
	for (i = 0; i < sbc_cnt; ++i) if ((j = strlen (subconts[i].name)) > w) w = j;
	for (i = 0; i < new_cnt; ++i) if ((j = strlen (newmsks[i].name)) > w) w = j;
	fprintf (fp_out, "(nbrOfElements)\n");
    }
    fprintf (fp_out, "%d\n", con_cnt + cap_cnt + res_cnt + tor_cnt + vdm_cnt
		+ shp_cnt + jun_cnt + bjt_cnt + cnt_cnt + sbc_cnt + new_cnt);
    j = 0;

    if (prVerbose) fmt = " %4s";
    else fmt = " %s";

    elemtype = LATCAPELEM;
docap:
    if (prVerbose) {
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el s# m# o# m# o#", w, "capname");
	if (elemtype == LATCAPELEM) fprintf (fp_out, " oPre oAbs");
	fprintf (fp_out, " value     xyidx)\n");
    }
    for (i = 0; i < cap_cnt; i++) {
	if (caps[i].eltype != elemtype) continue;

	nbr = howManyCond (caps[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;

	if (prVerbose) {
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		caps[i].name, caps[i].id, layC_cnt);
	}
	else {
	    fprintf (fp_out, "%s %d %d",
		caps[i].name, caps[i].id, layC_cnt);
	}
	fprintf (fp_out, fmt, colorIntStr (&caps[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&caps[i].sBitAbsent));
	fprintf (fp_out, fmt, colorIntStr (&caps[i].eBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&caps[i].eBitAbsent));

	if (prVerbose) {
	    fprintf (fp_out, " %2d %2d %2d %2d %2d %2d",
		caps[i].cap3d ? elemtype+13 : elemtype, caps[i].sortNr,
		caps[i].pLay -> mask, caps[i].pLay -> occurrence,
		caps[i].nLay -> mask, caps[i].nLay -> occurrence);
	}
	else {
	    fprintf (fp_out, " %d %d %d %d %d %d",
		caps[i].cap3d ? elemtype+13 : elemtype, caps[i].sortNr, // SURFCAP3DELEM ?
		caps[i].pLay -> mask, caps[i].pLay -> occurrence,
		caps[i].nLay -> mask, caps[i].nLay -> occurrence);
	}
	if (elemtype == LATCAPELEM) {
	    fprintf (fp_out, fmt, colorIntStr (&caps[i].oBitPresent));
	    fprintf (fp_out, fmt, colorIntStr (&caps[i].oBitAbsent));
	}
	fprintf (fp_out, " %e %d\n", caps[i].val, caps[i].x_y_index);
    }
    if (elemtype == LATCAPELEM ) { elemtype = EDGECAPELEM; goto docap; }
    if (elemtype == EDGECAPELEM) { elemtype = SURFCAPELEM; goto docap; }

    if (prVerbose && con_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el s# m# m# value)\n", w, "contname");
    for (i = 0; i < con_cnt; i++) {
	nbr = howManyCond (cons[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		cons[i].name, cons[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		cons[i].name, cons[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&cons[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&cons[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d",
		CONTELEM, cons[i].sortNr, cons[i].mask1, cons[i].mask2);
	else
	    fprintf (fp_out, " %d %d %d %d",
		CONTELEM, cons[i].sortNr, cons[i].mask1, cons[i].mask2);

	fprintf (fp_out, " %e\n", cons[i].val);
    }

    if (prVerbose && tor_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el g# d# s# b#)\n", w, "torname");
    for (i = 0; i < tor_cnt; i++) {
	nbr = howManyCond (tors[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		tors[i].name, tors[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		tors[i].name, tors[i].id, layC_cnt);
	fprintf (fp_out, fmt, colorIntStr (&tors[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&tors[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");
	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d %2d\n",
		TORELEM, tors[i].g, tors[i].ds, tors[i].s, tors[i].b);
	else
	    fprintf (fp_out, " %d %d %d %d %d\n",
		TORELEM, tors[i].g, tors[i].ds, tors[i].s, tors[i].b);
    }

    if (prVerbose && bjt_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el c# o# b# o# e# o# s# oPre oAbs)\n", w, "bjtname");
    for (i = 0; i < bjt_cnt; i++) {
	nbr = howManyCond (bjts[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		bjts[i].name, bjts[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		bjts[i].name, bjts[i].id, layC_cnt);
	fprintf (fp_out, fmt, colorIntStr (&bjts[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&bjts[i].sBitAbsent));
	fprintf (fp_out, fmt, colorIntStr (&bjts[i].eBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&bjts[i].eBitAbsent));

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d %2d %2d %2d %2d", bjts[i].type,
		bjts[i].pins[CO] -> mask, bjts[i].pins[CO] -> occurrence,
		bjts[i].pins[BA] -> mask, bjts[i].pins[BA] -> occurrence,
		bjts[i].pins[EM] -> mask, bjts[i].pins[EM] -> occurrence,
		bjts[i].pins[SU] -> mask);
	else
	    fprintf (fp_out, " %d %d %d %d %d %d %d %d", bjts[i].type,
		bjts[i].pins[CO] -> mask, bjts[i].pins[CO] -> occurrence,
		bjts[i].pins[BA] -> mask, bjts[i].pins[BA] -> occurrence,
		bjts[i].pins[EM] -> mask, bjts[i].pins[EM] -> occurrence,
		bjts[i].pins[SU] -> mask);

	fprintf (fp_out, fmt, colorIntStr (&bjts[i].oBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&bjts[i].oBitAbsent));
	fprintf (fp_out, "\n");
    }

    if (prVerbose && jun_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el a# o# c# o# vbr cap dep)\n", w, "junname");
    for (i = 0; i < jun_cnt; i++) {
	nbr = howManyCond (juns[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (juns[i].pins[CA] -> occurrence == SURFACE
	    && juns[i].pins[AN] -> occurrence == SURFACE)
	    elemtype = VJUNELEM;
	else
	    elemtype = LJUNELEM;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		juns[i].name, juns[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		juns[i].name, juns[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&juns[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&juns[i].sBitAbsent));
	fprintf (fp_out, fmt, colorIntStr (&juns[i].eBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&juns[i].eBitAbsent));

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d %2d", elemtype,
		juns[i].pins[AN] -> mask, juns[i].pins[AN] -> occurrence,
		juns[i].pins[CA] -> mask, juns[i].pins[CA] -> occurrence);
	else
	    fprintf (fp_out, " %d %d %d %d %d", elemtype,
		juns[i].pins[AN] -> mask, juns[i].pins[AN] -> occurrence,
		juns[i].pins[CA] -> mask, juns[i].pins[CA] -> occurrence);

	fprintf (fp_out, " %e %e %e\n", juns[i].vbr, juns[i].cap, juns[i].dep);
    }

    if (prVerbose && cnt_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el m# o# m# o# value)\n", w, "connname");
    for (i = 0; i < cnt_cnt; i++) {
	nbr = howManyCond (cnts[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		cnts[i].name, cnts[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		cnts[i].name, cnts[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&cnts[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&cnts[i].sBitAbsent));
	fprintf (fp_out, fmt, colorIntStr (&cnts[i].eBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&cnts[i].eBitAbsent));

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d %2d 0\n", PNCONELEM,
		cnts[i].cons[0] -> mask, cnts[i].cons[0] -> occurrence,
		cnts[i].cons[1] -> mask, cnts[i].cons[1] -> occurrence);
	else
	    fprintf (fp_out, " %d %d %d %d %d 0\n", PNCONELEM,
		cnts[i].cons[0] -> mask, cnts[i].cons[0] -> occurrence,
		cnts[i].cons[1] -> mask, cnts[i].cons[1] -> occurrence);
    }

    if (prVerbose && res_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el s# m# type value height thickness)\n", w, "condname");
    for (i = 0; i < res_cnt; i++) {
	nbr = howManyCond (ress[i].cond);
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		ress[i].name, ress[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		ress[i].name, ress[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&ress[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&ress[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d %2d %2d", RESELEM,
		ress[i].sortNr, ress[i].mask, ress[i].type);
	else
	    fprintf (fp_out, " %d %d %d %d", RESELEM,
		ress[i].sortNr, ress[i].mask, ress[i].type);

	fprintf (fp_out, " %e %e %e\n", ress[i].val, ress[i].height, ress[i].thickness);
    }

    if (prVerbose && vdm_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el m# height thickness)\n", w, "vdimname");
    for (i = 0; i < vdm_cnt; i++) {
	nbr = howManyCond (vdms[i].cond);
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		vdms[i].name, vdms[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		vdms[i].name, vdms[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&vdms[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&vdms[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d", VDIMELEM, vdms[i].mask);
	else
	    fprintf (fp_out, " %d %d", VDIMELEM, vdms[i].mask);

	fprintf (fp_out, " %e %e\n", vdms[i].height, vdms[i].thickness);
    }

    if (prVerbose && shp_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el m# xb1 xt1 xb2 xt2)\n", w, "shapename");
    for (i = 0; i < shp_cnt; i++) {
	nbr = howManyCond (shps[i].cond);
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		shps[i].name, shps[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		shps[i].name, shps[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&shps[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&shps[i].sBitAbsent));
	fprintf (fp_out, fmt, colorIntStr (&shps[i].eBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&shps[i].eBitAbsent));

	if (prVerbose)
	    fprintf (fp_out, " %2d %2d", shps[i].type, shps[i].mask);
	else
	    fprintf (fp_out, " %d %d", shps[i].type, shps[i].mask);

	fprintf (fp_out, " %e %e %e %e\n",
	    shps[i].xb1, shps[i].xt1, shps[i].xb2, shps[i].xt2);
    }

    if (prVerbose && sbc_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el cc_m# captype)\n", w, "subcname");
    for (i = 0; i < sbc_cnt; i++) {
        nbr = howManyCond (subconts[i].cond);
        if (nbr > CondMaxNbr) CondMaxNbr = nbr;
        CondNbr[nbr]++;
        elemCnt++;
        condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		subconts[i].name, subconts[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		subconts[i].name, subconts[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&subconts[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&subconts[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");

	if (prVerbose)
	    fprintf (fp_out, " %2d %5d %2d\n",
		SUBCONTELEM, subconts[i].ccnr, subconts[i].captype);
	else
	    fprintf (fp_out, " %d %d %d\n",
		SUBCONTELEM, subconts[i].ccnr, subconts[i].captype);
    }

    if (prVerbose && new_cnt)
	fprintf (fp_out, "(-----%-*s  id e# sPre sAbs ePre eAbs el nm_m#)\n", w, "newname");
    for (i = 0; i < new_cnt; i++) {
        nbr = howManyCond (newmsks[i].cond);
	if (nbr > CondMaxNbr) CondMaxNbr = nbr;
	CondNbr[nbr]++;
	elemCnt++;
	condCnt += nbr;
	if (prVerbose)
	    fprintf (fp_out, "(%s) %-*s %3d %2d", itostr (j++), w,
		newmsks[i].name, newmsks[i].id, layC_cnt);
	else
	    fprintf (fp_out, "%s %d %d",
		newmsks[i].name, newmsks[i].id, layC_cnt);

	fprintf (fp_out, fmt, colorIntStr (&newmsks[i].sBitPresent));
	fprintf (fp_out, fmt, colorIntStr (&newmsks[i].sBitAbsent));
	fprintf (fp_out, fmt, "0");
	fprintf (fp_out, fmt, "0");

	if (prVerbose)
	    fprintf (fp_out, " %2d %5d\n", NEWMASKELEM, newmsks[i].mask);
	else
	    fprintf (fp_out, " %d %d\n", NEWMASKELEM, newmsks[i].mask);
    }
}

Private int howManyCond (struct layCondRef *cond)
{
    for (layC_cnt = 0; cond; cond = cond -> next) {
	if (cond -> layC -> lay -> occurrence == SURFACE) {
	    if (cond -> layC -> present) layC_cnt |= 1;
	    else layC_cnt |= 2;
	}
	else if (cond -> layC -> lay -> occurrence == EDGE) {
	    if (cond -> layC -> present) layC_cnt |= 4;
	    else layC_cnt |= 8;
	}
	else fatalErr ("incorrect", "extra condition!");
    }

    if (layC_cnt) {
	int cnt = 0;
	if (layC_cnt & 1) ++cnt;
	if (layC_cnt & 2) ++cnt;
	if (layC_cnt & 4) ++cnt;
	if (layC_cnt & 8) ++cnt;
	return (cnt);
    }
    return (0);
}

struct hashRef {
    int i;
    struct hashRef *next;
};

static int *keyref;
static struct hashRef **hashtab;

Private void prKeytab ()
{
    register int i, cnt, nbr;
    register struct elemRef *elref, *elref_i, *elref_j;
    struct hashRef *eh;

    if (prVerbose) fprintf (fp_out, "(nbrKeySlots/elR_cnt)\n");
    fprintf (fp_out, "%d\n", nbrKeySlots);

    elR_cnt = 0;
    i = nbrKeySlots;
    if (nbrKeySlots2 > i) i = nbrKeySlots2;
    ALLOC (keyref, i, int);

    if (docompress) {
	ALLOC (hashtab, 397, struct hashRef *);
	for (i = 0; i < nbrKeySlots; i++) {
	    keyref[i] = -1;
	    if (!(elref_i = keytab[i])) continue;
	    cnt = elR_cnt++;
	    nbr = 0;
	    for (elref = elref_i; elref; elref = elref -> next) {
		nbr += elref -> elno + 16;
		elR_cnt++;
	    }
	    nbr = nbr % 397;
	    for (eh = hashtab[nbr]; eh; eh = eh -> next) {
		elref = elref_i;
		elref_j = keytab[eh -> i];
check:		if (elref -> elno != elref_j -> elno) continue;
		elref = elref -> next;
		elref_j = elref_j -> next;
		if (elref && elref_j) goto check;
		if (!elref && !elref_j) { /* keytab[i] and keytab[j] are equal */
		    keyref[i] = eh -> i;
		    elR_cnt = cnt; /* don't count this entry */
		    goto next_i;
		}
	    }
	    ALLOC (eh, 1, struct hashRef);
	    eh -> i = i;
	    eh -> next = hashtab[nbr];
	    hashtab[nbr] = eh;
next_i: ;
	}
    }
    else {
	for (i = 0; i < nbrKeySlots; i++) {
	    keyref[i] = -1;
	    for (elref = keytab[i]; elref; elref = elref -> next) elR_cnt++;
	    elR_cnt++; /* for terminating NULL pointer */
	}
    }

    fprintf (fp_out, "%d\n", elR_cnt);

    if (prVerbose) fprintf (fp_out, "(slot# element#...)\n");
    for (i = 0; i < nbrKeySlots; i++) {
	if (prVerbose) fprintf (fp_out, "(%4d) ", i);
	if (keyref[i] >= 0) {
	    fprintf (fp_out, "=%d\n", keyref[i]);
	}
	else {
	    if ((elref_i = keytab[i])) {
		if (printNonCoded) {
		    nbr = 1;
		    fprintf (fp_out, "%d", elref_i -> elno);
		    while ((elref_i = elref_i -> next)) {
			nbr++;
			fprintf (fp_out, " %d", elref_i -> elno);
		    }
		}
		else { /* coded */
		    int c0, c1;
		    nbr = 0;
		    do {
			++nbr;
			itocc (elref_i -> elno, &c0, &c1);
			fprintf (fp_out, "%c%c", c1, c0);
		    } while ((elref_i = elref_i -> next));
		}
		if (nbr > ElemMaxNbr) ElemMaxNbr = nbr;
	    }
	    fprintf (fp_out, "\n");
	}
    }
}

Private void prKeytab2 ()
{
    register int i, cnt, nbr;
    register struct elemRef *elref, *elref_i, *elref_j;
    struct hashRef *eh, *beg_eh_list, *end_eh_list;

    if (prVerbose) fprintf (fp_out, "(edge: nbrKeySlots/elR_cnt)\n");
    fprintf (fp_out, "%d\n", nbrKeySlots2);

    elR_cnt = 0;

    if (docompress) {
	beg_eh_list = 0;
	end_eh_list = 0;
	for (i = 0; i < 397; i++) {
	    if ((eh = hashtab[i])) {
		hashtab[i] = 0; /* init */
		if (end_eh_list) end_eh_list -> next = eh;
		else beg_eh_list = eh;
		while (eh -> next) eh = eh -> next;
		end_eh_list = eh;
	    }
	}
	for (i = 0; i < nbrKeySlots2; i++) {
	    keyref[i] = -1;
	    if (!(elref_i = keytab2[i])) continue;
	    cnt = elR_cnt++;
	    nbr = 0;
	    for (elref = elref_i; elref; elref = elref -> next) {
		nbr += elref -> elno + 16;
		elR_cnt++;
	    }
	    nbr = nbr % 397;
	    for (eh = hashtab[nbr]; eh; eh = eh -> next) {
		elref = elref_i;
		elref_j = keytab2[eh -> i];
check:		if (elref -> elno != elref_j -> elno) continue;
		elref = elref -> next;
		elref_j = elref_j -> next;
		if (elref && elref_j) goto check;
		if (!elref && !elref_j) { /* keytab2[i] and keytab2[j] are equal */
		    keyref[i] = eh -> i;
		    elR_cnt = cnt; /* don't count this entry */
		    goto next_i;
		}
	    }
	    if ((eh = beg_eh_list)) {
		beg_eh_list = beg_eh_list -> next;
	    }
	    else {
		ALLOC (eh, 1, struct hashRef);
	    }
	    eh -> i = i;
	    eh -> next = hashtab[nbr];
	    hashtab[nbr] = eh;
next_i: ;
	}
    }
    else {
	for (i = 0; i < nbrKeySlots2; i++) {
	    keyref[i] = -1;
	    for (elref = keytab2[i]; elref; elref = elref -> next) elR_cnt++;
	    elR_cnt++; /* for terminating NULL pointer */
	}
    }

    fprintf (fp_out, "%d\n", elR_cnt);

    if (prVerbose) fprintf (fp_out, "(slot# element#...)\n");
    for (i = 0; i < nbrKeySlots2; i++) {
	if (prVerbose) fprintf (fp_out, "(%4d) ", i);
	if (keyref[i] >= 0) {
	    fprintf (fp_out, "=%d\n", keyref[i]);
	}
	else {
	    if ((elref_i = keytab2[i])) {
		if (printNonCoded) {
		    nbr = 1;
		    fprintf (fp_out, "%d", elref_i -> elno);
		    while ((elref_i = elref_i -> next)) {
			nbr++;
			fprintf (fp_out, " %d", elref_i -> elno);
		    }
		}
		else { /* coded */
		    int c0, c1;
		    nbr = 0;
		    do {
			++nbr;
			itocc (elref_i -> elno, &c0, &c1);
			fprintf (fp_out, "%c%c", c1, c0);
		    } while ((elref_i = elref_i -> next));
		}
		if (nbr > ElemMaxNbr2) ElemMaxNbr2 = nbr;
	    }
	    fprintf (fp_out, "\n");
	}
    }
}

Private void itocc (int i, int *c0, int *c1)
{
    int n;

    n = i % 62;
	 if (n < 10) *c0 = '0' +  n;
    else if (n < 36) *c0 = 'A' + (n - 10);
    else             *c0 = 'a' + (n - 36);

    n = i / 62;
    if (!n) { *c1 = ' '; return; }
    --n;
	 if (n < 10) *c1 = '0' +  n;
    else if (n < 36) *c1 = 'A' + (n - 10);
    else             *c1 = 'a' + (n - 36);
}

Private char *itostr (int i)
{
    static char buf[16];

    if (printNonCoded) {
	sprintf (buf, "%3d", i);
    }
    else {
	int c0, c1;
	itocc (i, &c0, &c1);
	sprintf (buf, " %c%c", c1, c0);
    }
    return buf;
}

Private void prOtherInfo ()
{
    int i;

    if (prVerbose) fprintf (fp_out, "(diel_cnt)\n");
    fprintf (fp_out, "%d\n", diel_cnt);
    for (i = 0; i < diel_cnt; i++) {
	if (prVerbose) fprintf (fp_out, "(%2d) ", i);
	fprintf (fp_out, "%s %e %e\n",
		diels[i].name, diels[i].permit, diels[i].bottom);
    }

    if (prVerbose) fprintf (fp_out, "(substr_cnt)\n");
    fprintf (fp_out, "%d\n", substr_cnt);
    for (i = 0; i < substr_cnt; i++) {
	if (prVerbose) fprintf (fp_out, "(%2d) ", i);
	fprintf (fp_out, "%s %e %e\n",
		substrs[i].name, substrs[i].conduc, substrs[i].top);
    }

    if (prVerbose) fprintf (fp_out, "(subcap_cnt)\n");
    fprintf (fp_out, "%d\n", subcap_cnt);
    for (i = 0; i < subcap_cnt; i++) {
	if (prVerbose) fprintf (fp_out, "(%2d) ", i);
	fprintf (fp_out, "%s %e %e\n",
		subcaps[i].name, subcaps[i].conduc, subcaps[i].top);
    }

    if (prVerbose) fprintf (fp_out, "(self_cnt)\n");
    fprintf (fp_out, "%d\n", self_cnt);
    for (i = 0; i < self_cnt; i++) {
	if (prVerbose) fprintf (fp_out, "(%2d) ", i);
	fprintf (fp_out, "%e %e %e %e\n",
		selfs[i].area, selfs[i].perim, selfs[i].val, selfs[i].rest);
    }

    if (prVerbose) fprintf (fp_out, "(mut_cnt)\n");
    fprintf (fp_out, "%d\n", mut_cnt);
    for (i = 0; i < mut_cnt; i++) {
	if (prVerbose) fprintf (fp_out, "(%2d) ", i);
	fprintf (fp_out, "%e %e %e %e %e\n",
		muts[i].area1, muts[i].area2, muts[i].dist, muts[i].val, muts[i].decr);
    }
}
