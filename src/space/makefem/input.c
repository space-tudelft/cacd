/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makefem/define.h"
#include "src/space/makefem/extern.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern int substr_cnt;
extern substrate_t *substrs;
extern subcont_t *subcont_list_begin;
extern subcont_t *subcont_list_end;
extern subcont_t *subcont_list_begin2;
extern subcont_t *subcont_list_end2;

static int   geofmt = GEO_BOXLAY;
static char *geofile = "cont_bln";
static DM_STREAM *inStream;
static DM_STREAM *outStream1;
FILE *outgeo_fp;

edge_t * fetchEdge ()
{
    edge_t *edge;
    mask_t color;
    int k = dmGetDesignData (inStream, geofmt);
    if (k > 0) {
	if (geofmt == GEO_BOXLAY) {
	    if (gboxlay.chk_type & 0x100) /* interior edge */
		color = cNull;
	    else {
		k = gboxlay.chk_type & 0xff; /* conductor_nr */
		ASSERT (k >= 0 && k < 64);
		COLORINITINDEX (color, k);
		ASSERT (IS_COLOR (&color));
	    }
	    NEW_EDGE (edge, (coor_t) gboxlay.xl, (coor_t) gboxlay.yb,
			    (coor_t) gboxlay.xr, (coor_t) gboxlay.yt, color);
	    edge -> cc = gboxlay.chk_type;
	}
	else { // GEO_GLN
	    COLORINITINDEX (color, 0);
	    if (ggln.xl == -INF) {
		coor_t ext = outScale * cs_ext;
		ASSERT (ggln.xr == INF);
		if (cs_bxr > cs_bxl) {
		    ggln.xl = outScale * cs_bxl;
		    ggln.xr = outScale * cs_bxr;
		}
		else {
		    ggln.xl = bbxl - ext;
		    ggln.xr = bbxr + ext;
		}
		if (ggln.yl == -INF) {
		    if (cs_bxr > cs_bxl)
			ggln.yl = ggln.yr = outScale * cs_byb;
		    else
			ggln.yl = ggln.yr = bbyb - ext;
		}
		else {
		    ASSERT (ggln.yl == INF);
		    if (cs_bxr > cs_bxl)
			ggln.yl = ggln.yr = outScale * cs_byt;
		    else
			ggln.yl = ggln.yr = bbyt + ext;
		}
	    }
	    NEW_EDGE (edge, (coor_t) ggln.xl, (coor_t) ggln.yl,
			    (coor_t) ggln.xr, (coor_t) ggln.yr, color);
	}
    }
    else {
	if (k < 0) say ("%s read error", geofile), die ();
	NEW_EDGE (edge, INF, INF, INF, INF, cNull); /* EOF */
    }
    return edge;
}

void swapInput ()
{
    subcont_t *subcont_list_begin1, *subcont_list_end1;
    int tileConCnt1;

    tileConCnt1 = tileConCnt;
    subcont_list_begin1 = subcont_list_begin;
    subcont_list_end1   = subcont_list_end;

    dmCloseStream (inStream, COMPLETE);

    geofmt = GEO_GLN;
    geofile = mprintf ("%s_gln", csMASK);
    inStream = dmOpenStream (cellKey, geofile, "r");

    subcont_list_begin = subcont_list_end = 0;
    tileConCnt = 0;
    scan (fetchEdge ());

    tileConCnt2 = tileConCnt;
    subcont_list_begin2 = subcont_list_begin;
    subcont_list_end2   = subcont_list_end;

    tileConCnt = tileConCnt1;
    subcont_list_begin = subcont_list_begin1;
    subcont_list_end   = subcont_list_end1;
}

edge_t * openInput ()
{
    inStream = dmOpenStream (cellKey, geofile, "r");
    outStream1 = dmOpenStream (cellKey, "subres", "w");

    initSubstr ();

    outgeo_fp = cfopen ("step1_geometry.m", "w");
 // fprintf (outgeo_fp, "%% FEMLAB Model M-file\n");
 // fprintf (outgeo_fp, "flclear fem\n");
    fprintf (outgeo_fp, "%% New geometry 1\n");
    fprintf (outgeo_fp, "fem.sdim={'x','y','z'};\n\n");
    fprintf (outgeo_fp, "%% Geometry of substrate contacts (in meters)\n");
    fprintf (outgeo_fp, "wp=[0 1 0;0 0 1;0 0 0];\n");

    return fetchEdge ();
}

char *gx (double val)
{
    static char buf[40];
    char *e, *f, *s, *p;

    e = buf;
    if (val == 0) { *e++ = '0'; *e = 0; return buf; }
    sprintf (buf, "%e", val);
    p = 0; while (*e && *e != 'e') { if (*e == '.') p = e; ++e; }
    ASSERT (p && *e == 'e');
    s = e++; while (*--s == '0') ;
    ASSERT (*e == '-');
    f = ++e; ++e;
    ASSERT (*f >= '0' && *e >= '1');
    while (p != s) { *p = *(p+1); ++p; if (++*e > '9') { ++*f; *e = '0'; }}
    *p++ = 'e';
    *p++ = '-';
    if (*f > '0') *p++ = *f;
    *p++ = *e;
    *p = 0;
    return buf;
}

#define MAX_SC 200
double M[2][MAX_SC+1][MAX_SC+1];
#define G M[0]
#define C M[1]

char *infile = "femsub.out";

Private int convertPart (FILE *fpi, char *part)
{
    double d, total;
    char buf[40];
    int i, j, k, nr;

    i = 0;
    while (fscanf (fpi, "%s", buf) > 0)
	if (strsame (buf, part)
	    && fscanf (fpi, "%s", buf) > 0
	    && strsame (buf, "part")) { ++i; break; }
    if (!i) { say ("%s: %s part not found!", infile, part); return 1; }

    k = (*part == 'i')? 1 : 0;

    nr = tileConCnt;
    if (backcontact) ++nr;

    for (i = 0; i < nr; ++i) {
	for (j = 0; j < nr; ++j) {
	    if (fscanf (fpi, "%s", buf) <= 0) {
		say ("%s: read error, too less contact values!", infile); return 1; }
	    M[k][i][j] = atof (buf);
	}
	if (fgetc (fpi) != '\n') {
		say ("%s: read error, too many contact values!", infile); return 1; }
    }

    // Calculate the mean value of the two values in the array
    // and make the result a positive value!!!
    // And correct the diagonal values for the subres file.
    for (i = 0; i < tileConCnt; ++i)
	for (j = i+1; j < nr; ++j) {
	    M[k][i][j] = -(M[k][i][j] + M[k][j][i]) / 2;
	    M[k][i][i] -= M[k][i][j];
	    M[k][j][j] -= M[k][i][j];
	}

if (backcontact) {
    // Gaussian elimination of the backcontact node (nr == tileConCnt+1)
    total = M[k][tileConCnt][tileConCnt];
    for (i = 0; i < tileConCnt; ++i) total += M[k][i][tileConCnt];
    for (i = 0; i < tileConCnt; ++i) {
	d = M[k][i][tileConCnt] / total;
	M[k][i][i] += d * M[k][tileConCnt][tileConCnt];
	for (j = i+1; j < tileConCnt; ++j) M[k][i][j] += d * M[k][j][tileConCnt];
    }
}

    // Eliminate conductances to the substrate node.
    if (eliminateSubstrNode) {
	total = 0;
	for (i = 0; i < tileConCnt; ++i) total += M[k][i][i];
	if (total != 0)
	for (i = 0; i < tileConCnt; ++i) {
	    for (j = i+1; j < tileConCnt; ++j) M[k][i][j] += M[k][j][j] * M[k][i][i] / total;
	    M[k][i][i] = 0;
	}
    }

    return 0;
}

Private void convertFemOutput ()
{
    double cf;
    FILE *fpi, *fpo;
    subcont_t *sc;
    int cnt, i, j;

    fpi = cfopen (infile, "r");
    if (convertPart (fpi, "real")) goto ret;
    if (convertPart (fpi, "image")) goto ret;

    fpo = outStream1 -> dmfp;

    cf = 1 / (2 * M_PI); // capacitance factor

    // Output the array in subres format
    j = 0;
    for (sc = subcont_list_begin; sc; sc = sc -> next) {
	fprintf (fpo, "c %d %d xl %ld yb %ld g %le c %le\n",
	    j+1, sc -> group, (long)sc -> xl, (long)sc -> yb, G[j][j], cf * C[j][j]);
	cnt = 0;
	for (i = 0; i < j; ++i) {
	    if (G[i][j]) {
		fprintf (fpo, "nc %d g %le c %le\n", i+1, G[i][j], cf * C[i][j]);
		++cnt;
	    }
	}
	while (++i < tileConCnt) if (G[j][i]) ++cnt;
	fprintf (fpo, "nr_neigh %d\n", cnt);
	++j;
    }
    ASSERT (j == tileConCnt);
ret:
    fclose (fpi);
}

scCoor_t *swappoints (scCoor_t *pt) // point first
{
    scCoor_t *pn; // point next
    scCoor_t *pp; // point prev
    pp = 0;
    while ((pn = pt -> next)) { pt -> next = pp; pp = pt; pt = pn; }
    pt -> next = pp;
    return pt;
}

void closeInput ()
{
    FILE *tech_fp;
    subcont_t *sc;
    int i, cnt, cstop;
    double z, dz;
#define MAX_PTS 200
    coor_t X[MAX_PTS];
    coor_t Y[MAX_PTS];
    int j, k, l, nr = 1;
    int blocks;

    dmCloseStream (inStream, COMPLETE);

#if 0
    fprintf (outgeo_fp, "\n%% Substrate conductivity\n");
    for (i = 0; i < substr_cnt; ++i) {
	fprintf (outgeo_fp, "%% name=%s conduc=%g top=%g\n",
	    substrs[i].name, substrs[i].conduc, substrs[i].top);
    }
#endif
    tech_fp = cfopen ("femtech.in", "w");
    fprintf (tech_fp, "#contacts\n");
    fprintf (tech_fp, "contacts %d\n", tileConCnt);
    fprintf (tech_fp, "backcontact %d\n", backcontact ? 1 : 0);
    fprintf (tech_fp, "#subdomain\n");
    blocks = tileConCnt2 + substr_cnt;
    fprintf (tech_fp, "layers %d\n", blocks);
    fprintf (tech_fp, "epsilon");
    for (i = 0; i < blocks; ++i) fprintf (tech_fp, " 11.7");
    fprintf (tech_fp, "\n");
    fprintf (tech_fp, "sigma");
    if (csSIGMA <= 0) csSIGMA = substrs[0].conduc * 100;
    for (i = 0; i < tileConCnt2; ++i) fprintf (tech_fp, " %g", csSIGMA);
    for (i = 0; i < substr_cnt; ++i) fprintf (tech_fp, " %g", substrs[i].conduc);
    fprintf (tech_fp, "\n");
    fprintf (tech_fp, "#frequence\n");
    fprintf (tech_fp, "frequence 1\n");
    fprintf (tech_fp, "omega 1e6\n");
    fclose (tech_fp);

    cstop = 0;
    sc = subcont_list_begin;
    cnt = tileConCnt;
start1:
    for (i = 1; i <= cnt; ++i) {
	scBound_t *bct, *bcn, *bcp;
	scCoor_t *pp, *pt, *pb, *pn;

	ASSERT (sc);
	bct = sc -> boundaries;
	ASSERT (bct);

	j = nr;

    do {
#ifdef DEBUG
	if (bct == sc -> boundaries)
	    fprintf (stderr, "--beg outside boundary-------\n");
	else
	    fprintf (stderr, "--beg in_side boundary-------\n");
#endif
	k = 0;

	// first bct is outside boundary
	pp = pt = swappoints (bct -> pt);
	do {
#ifdef DEBUG
	    fprintf (stderr, "pt: %d %d\n", pt -> x/4, pt -> y/4);
#endif
	    X[k] = pt -> x;
	    Y[k] = pt -> y;
	    ++k;
	} while ((pt = pt -> next));

	pt = bct -> pt;
	pb = bct -> pb;
	while (pb -> x != pt -> x || pb -> y != pt -> y) {
	    // search for matching point
	    for (bcp = bct; (bcn = bcp -> next); bcp = bcn) {
		pb = bcn -> pb;
		if (pb -> x == pt -> x && pb -> y == pt -> y) { pt = bcn -> pt; break; }
		pb = bcn -> pt;
		if (pb -> x == pt -> x && pb -> y == pt -> y) { pt = bcn -> pb; break; }
	    }
	    ASSERT (bcn);
	    // remove boundary bcn from boundary-list
	    bcp -> next = bcn -> next;

	    while (pb -> next) {
		pb = pb -> next;
#ifdef DEBUG
		fprintf (stderr, "pb: %d %d\n", pb -> x/4, pb -> y/4);
#endif
		X[k] = pb -> x;
		Y[k] = pb -> y;
		++k;
	    }
	    pn = swappoints (pt);
	    if (pb -> y == pn -> y) pn = pn -> next;
	    while (pn) {
#ifdef DEBUG
		fprintf (stderr, "pt: %d %d\n", pn -> x/4, pn -> y/4);
#endif
		X[k] = pn -> x;
		Y[k] = pn -> y;
		++k;
		pn = pn -> next;
	    }
	    pb = bct -> pb;
	}

	pb = pb -> next;
	ASSERT (pb);
	while (pb -> next) {
#ifdef DEBUG
	    fprintf (stderr, "pb: %d %d\n", pb -> x/4, pb -> y/4);
#endif
	    X[k] = pb -> x;
	    Y[k] = pb -> y;
	    ++k;
	    pb = pb -> next;
	}
	if (pb -> y != pp -> y) {
#ifdef DEBUG
	    fprintf (stderr, "pb: %d %d\n", pb -> x/4, pb -> y/4);
#endif
	    X[k] = pb -> x;
	    Y[k] = pb -> y;
	    ++k;
	}

#ifdef DEBUG
	if (bct == sc -> boundaries)
	    fprintf (stderr, "--end outside boundary-------\n");
	else
	    fprintf (stderr, "--end in_side boundary-------\n");
#endif
	if (k > MAX_PTS) {
	    say ("sorry, too many points!"), die ();
	}
	l = 0;
	fprintf (outgeo_fp, "s%d=poly2([%s", nr++, gx(meters*X[l]));
	while (++l < k) {
	    if (!(l % 10) && l+1 < k) fprintf (outgeo_fp, " ...\n");
	    fprintf (outgeo_fp, " %s", gx(meters*X[l]));
	}
	l = 0;
	if (k > 5)
	    fprintf (outgeo_fp, "],...\n[%s", gx(meters*Y[l]));
	else
	    fprintf (outgeo_fp, "],[%s", gx(meters*Y[l]));
	while (++l < k) {
	    if (!(l % 10) && l+1 < k) fprintf (outgeo_fp, " ...\n");
	    fprintf (outgeo_fp, " %s", gx(meters*Y[l]));
	}
	fprintf (outgeo_fp, "]);\n");

    } while ((bct = bct -> next));

	ASSERT (nr > j);
	if (cstop)
	    fprintf (outgeo_fp, "b%d=extrude(s%d", i, j);
	else
	    fprintf (outgeo_fp, "c%d=embed(s%d", i, j);
	while (++j < nr) fprintf (outgeo_fp, "-s%d", j);
	if (cstop)
	    fprintf (outgeo_fp, ",'Distance',-cs_thickness,'Wrkpln',wp);\n");
	else
	    fprintf (outgeo_fp, ",wp);\n");

	sc = sc -> next;
    }

    if (!cstop && tileConCnt2 > 0) {
	fprintf (outgeo_fp, "\n%% Geometry of channelstop blocks (in meters)\n");
	fprintf (outgeo_fp, "cs_thickness=%s;\n", gx(csTN*1e-6));
	sc = subcont_list_begin2;
	cstop = 1;
	cnt = tileConCnt2;
	goto start1;
    }

    fprintf (outgeo_fp, "\n%% Geometry of substrate blocks (in meters)\n");
    fprintf (outgeo_fp, "%% cell bbox: %s", gx(meters*bbxl));
    fprintf (outgeo_fp, " %s",   gx(meters*bbxr));
    fprintf (outgeo_fp, " %s",   gx(meters*bbyb));
    fprintf (outgeo_fp, " %s\n", gx(meters*bbyt));
    fprintf (outgeo_fp, "%% subc bbox: %s", gx(meters*c_xl));
    fprintf (outgeo_fp, " %s",   gx(meters*c_xr));
    fprintf (outgeo_fp, " %s",   gx(meters*c_yb));
    fprintf (outgeo_fp, " %s\n", gx(meters*c_yt));
    fprintf (outgeo_fp, "xl=%s;\n", gx(meters*c_xl));
    fprintf (outgeo_fp, "xr=%s;\n", gx(meters*c_xr));
    fprintf (outgeo_fp, "yb=%s;\n", gx(meters*c_yb));
    fprintf (outgeo_fp, "yt=%s;\n", gx(meters*c_yt));
    fprintf (outgeo_fp, "x=%s; %% delta x overlap\n", gx(femX*1e-6));
    fprintf (outgeo_fp, "y=%s; %% delta y overlap\n", gx(femY*1e-6));
    fprintf (outgeo_fp, "z=%s; %% delta z to z-bottom\n\n", gx(femZ*1e-6));

    nr = tileConCnt;
if (backcontact) {
    ++nr;
    fprintf (outgeo_fp, "%% bottom plane also has to be defined as separate contact\n");
    fprintf (outgeo_fp, "c%d=embed(rect2(xl-x,xr+x,yb-y,yt+y),[0 1 0;0 0 1;-z -z -z]);\n\n", nr);
}
    z = 0;
for (i = 1; i < substr_cnt; ++i) {
    z = substrs[i].top;
    ASSERT (z < 0);
    dz = substrs[i-1].top - z;
    ASSERT (dz > 0);
    fprintf (outgeo_fp, "b%d=block3(xr-xl+2*x,yt-yb+2*y,%s", i+tileConCnt2, gx(dz*1e-6));
    fprintf (outgeo_fp, ",'corner',[xl-x yb-y %s]);\n", gx(z*1e-6));
}
    i += tileConCnt2;
    if (z == 0)
	fprintf (outgeo_fp, "b%d=block3(xr-xl+2*x,yt-yb+2*y,z,'corner',[xl-x yb-y -z]);\n", i);
    else {
	ASSERT (z < 0);
	dz = femZ + z;
	if (dz <= 0) say ("error: too small z-bottom value (must be > %g micron)!", -z);
	fprintf (outgeo_fp, "b%d=block3(xr-xl+2*x,yt-yb+2*y,z%s,'corner',[xl-x yb-y -z]);\n", i, gx(z*1e-6));
    }

    fprintf (outgeo_fp, "\nclear s f c p\n");
    fprintf (outgeo_fp, "s.objs={b1");
    for (i = 2; i <= blocks; ++i)
	fprintf (outgeo_fp, ",b%d", i);
    fprintf (outgeo_fp, "};\n");
    fprintf (outgeo_fp, "s.name={'B1'");
    for (i = 2; i <= blocks; ++i)
	fprintf (outgeo_fp, ",'B%d'", i);
    fprintf (outgeo_fp, "};\n");

    fprintf (outgeo_fp, "f.objs={");
    for (i = 1; i < nr; ++i) {
	if (i > 1 && (i % 10) == 1) fprintf (outgeo_fp, "...\n        ");
	fprintf (outgeo_fp, "c%d,", i);
    }
    fprintf (outgeo_fp, "c%d};\n", i);

    fprintf (outgeo_fp, "f.name={");
    for (i = 1; i < nr; ++i) {
	if (i > 1 && (i % 10) == 1) fprintf (outgeo_fp, "...\n        ");
	fprintf (outgeo_fp, "'C%d',", i);
    }
    fprintf (outgeo_fp, "'C%d'};\n", i);

    fprintf (outgeo_fp, "c.objs={};\n");
    fprintf (outgeo_fp, "c.name={};\n");
    fprintf (outgeo_fp, "p.objs={};\n");
    fprintf (outgeo_fp, "p.name={};\n");
    fprintf (outgeo_fp, "drawstruct=struct('s',s,'f',f,'c',c,'p',p);\n");
    fprintf (outgeo_fp, "fem.draw=drawstruct;\n\n");
    fprintf (outgeo_fp, "%% This is for visual feedback:\n");
    fprintf (outgeo_fp, "fem.geom=geomcsg(fem);\n");
    fclose (outgeo_fp);

    if (tileConCnt > MAX_SC) say ("too many contacts (%d > %d)!", tileConCnt, MAX_SC);
    else if (tileConCnt < 1) say ("no substrate contacts found!");
    else if (optRunFem) {
	unlink ("femsub.out");
	_dmRun ("runfemlab", (char *) NULL);
	convertFemOutput ();
    }

    dmCloseStream (outStream1, COMPLETE);
    endSubstr ();
}
