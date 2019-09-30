/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
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

#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "src/space/include/config.h"
#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makedela/makedela.h"
#include "src/space/auxil/proto.h"
#include "src/space/makedela/dproto.h"
#include "src/space/include/extract.h"

#ifdef __cplusplus
  extern "C" {
#endif

double findDist (contactBoundary_t *bdrA_start, contactBoundary_t *bdrB_start);
Private void  initSubData (void);
Private double addval (int i, int j, double val);
Private double interpolateRdir (double area1, double area2, double dist, double *pdecr);
Private double interpolateRsub (double area, double perim, double *rest);

#ifdef __cplusplus
  }
#endif

extern int decrease_sub_res;
extern int eliminateSubstrNode;
extern int sub_res_interp_method;

static DM_STREAM * inputStream = NULL;
static DM_STREAM * outputStream = NULL;

void openOutput (DM_CELL *cellKey, char *mask)
{
    outputStream = dmOpenStream (cellKey, "subres", "w");
    inputStream = dmOpenStream (cellKey, "cont_pos", "r");
    initSubData ();
}

void closeOutput ()
{
    dmCloseStream (outputStream, COMPLETE); outputStream = NULL;
    dmCloseStream (inputStream, COMPLETE); inputStream = NULL;
}

#define initval(n) addval(-1,n,0.0)

void makeOutput ()
{
    FILE *fpin, *fpout;
    vertex_t *ver, *v;
    meshedge_t *e;
    contact_t *ctA, *ctB;
    contactBoundary_t *b;
    double *area, *remaining, perim, d1, d2, val, decr, rest, min, m;
    int sc, nc, i, next_nr, next_grp, getval, last_nr, nr_contacts;
    long next_xl, next_yb;

    nr_contacts = 0;
    for (ver = Begin_vertices; ver; ver = ver -> next) {
	if (ver -> contbound -> cont -> nr < 0)
	    ver -> contbound -> cont -> nr = ++nr_contacts;
    }
    if (!nr_contacts) return;

    fpin = inputStream -> dmfp;

    initval (nr_contacts);
    area      = NEW (double, nr_contacts);
    remaining = NEW (double, nr_contacts);
    m = meters / 1e-6;

    last_nr = 0;
    for (ver = Begin_vertices; ver; ver = ver -> next)
    if (ver -> contbound -> cont -> nr > last_nr) {
	++last_nr;
	ASSERT (ver -> contbound -> cont -> nr == last_nr);

	i = fscanf (fpin, "%d %d %ld %ld %le %le",
		&next_nr, &next_grp, &next_xl, &next_yb, &val, &perim);
	ASSERT (i == 6);

	ASSERT (next_nr == last_nr);
	ASSERT (next_xl == ver -> x);
	ASSERT (next_yb == ver -> y);
	sc = last_nr - 1;
	area[sc] = val / 1e-12;
	val = interpolateRsub (area[sc], perim / 1e-6, &rest);
	if (val < 0) { last_nr = 0; goto skip; }
	remaining[sc] = val * (1 - rest);
	addval (sc, sc, val);

	for (b = ver -> contbound -> cont -> boundaries; b; b = b -> bound_next) {
	    for (v = b -> vertices; v; v = v -> contbound_next) {
		e = v -> edges;
		do {
		    if (!e -> fixed &&
			e -> vA -> contbound -> cont != e -> vB -> contbound -> cont) {

			if (v == e -> vA) {
			    ctA = e -> vA -> contbound -> cont;
			    ctB = e -> vB -> contbound -> cont;
			}
			else {
			    ctA = e -> vB -> contbound -> cont;
			    ctB = e -> vA -> contbound -> cont;
			}
			if (ctB -> nr < ctA -> nr && ctB -> neighbors != ctA) {
			    ctB -> neighbors = ctA;
			    d1 = findDist (ctA -> boundaries, ctB -> boundaries);
			    d2 = findDist (ctB -> boundaries, ctA -> boundaries);
			    if (d2 < d1) d1 = d2;
			    nc = ctB -> nr;
			    ASSERT (nc > 0); --nc;
			    val = interpolateRdir (area[sc], area[nc], d1 * m, &decr);
			    if (val == 0) { last_nr = 0; goto skip; }
			    ASSERT (nc < sc);
			    addval (nc, sc, val);
			    if (decrease_sub_res) {
				min = val * decr;
				if (min > remaining[sc]) min = remaining[sc];
				if (min > 0) { addval (sc, sc, -min); remaining[sc] -= min; }
				min = val * decr;
				if (min > remaining[nc]) min = remaining[nc];
				if (min > 0) { addval (nc, nc, -min); remaining[nc] -= min; }
			    }
			}
		    }
		    e = NEXT_E (e, v);
		} while (e != v -> edges);
	    }
	}
    }

    ASSERT (last_nr == nr_contacts);

    if (eliminateSubstrNode) { // Try to eliminate substrate terminal nodes.
	double totconval = 0.0;

	for (sc = 0; sc < nr_contacts; ++sc) totconval += addval (sc, sc, 0.0);

	if (totconval != 0.0) { // Eliminate it!
	    for (i = 1; i < nr_contacts; ++i) {
		sc = i - 1;
		if ((val = addval (sc, sc, 0.0)) != 0.0) {
		    val /= totconval;
		    for (nc = i; nc < nr_contacts; ++nc) {
			if ((m = addval (nc, nc, 0.0)) != 0.0)
			    addval (sc, nc, val * m);
		    }
		}
	    }
	}
    }

skip:
    fpout = outputStream -> dmfp;
    getval = last_nr && !eliminateSubstrNode;
    rewind (fpin);
    sc = 0;
    while (fscanf (fpin, "%d %d %ld %ld %le %le",
	    &next_nr, &next_grp, &next_xl, &next_yb, &m, &perim) > 5) {
	if (getval)
	    fprintf (fpout, "c %d %d xl %ld yb %ld g %le\n",
		next_nr, next_grp, next_xl, next_yb, addval (sc, sc, 0.0));
	else
	    fprintf (fpout, "c %d %d xl %ld yb %ld g 0\n",
		next_nr, next_grp, next_xl, next_yb);
	nc = i = 0;
	if (last_nr) {
	    while (nc < sc) {
		m = addval (nc++, sc, 0.0);
		if (m != 0) { fprintf (fpout, "nc %d g %le\n", nc, m); ++i; }
	    }
	    // nc == sc
	    while (++nc < nr_contacts) {
		if (addval (sc, nc, 0.0) != 0) ++i;
	    }
	}
	fprintf (fpout, "nr_neigh %d\n", i);
	++sc;
    }
    ASSERT (sc == nr_contacts);

    DISPOSE (remaining, sizeof(double) * nr_contacts);
    DISPOSE (area, sizeof(double) * nr_contacts);
    initval (0);
}

double findDist (contactBoundary_t *bdrA_start, contactBoundary_t *bdrB_start)
{
    vertex_t *vA, *vB, *vB_other, *vmin, *vmax;
    double d, dist;
    contactBoundary_t *bdrA, *bdrB;

    /* Compute the minimum distance between the line segments given
       by vA and the vertices given by vB.
    */
    dist = INF;
    bdrA = bdrA_start;
    while (bdrA) {
	vA = bdrA -> vertices;
	while (vA) {
	    bdrB = bdrB_start;
	    while (bdrB) {
		vB = bdrB -> vertices;
		while (vB) {
		    if (vB -> contbound_next)
			vB_other = vB -> contbound_next;
		    else
			vB_other = vB;
		    /* must be orthogonal */

		    ASSERT (vB -> x == vB_other -> x
			    || vB -> y == vB_other -> y);
		    if (vB -> x == vB_other -> x) {
			if (vB -> y > vB_other -> y) {
			    vmax = vB;
			    vmin = vB_other;
			}
			else {
			    vmax = vB_other;
			    vmin = vB;
			}

			if (vA -> y > vmax -> y)
			    d = sqrt ((double)
				      (Dsqr (vA -> x - vmax -> x)
				      + Dsqr (vA -> y - vmax -> y)));
			else if (vA -> y < vmin -> y)
			    d = sqrt ((double)
				      (Dsqr (vA -> x - vmin -> x)
				      + Dsqr (vA -> y - vmin -> y)));
			else
			    d = Abs (vA -> x - vmin -> x);
		    }
		    else {
			if (vB -> x > vB_other -> x) {
			    vmax = vB;
			    vmin = vB_other;
			}
			else {
			    vmax = vB_other;
			    vmin = vB;
			}

			if (vA -> x > vmax -> x)
			    d = sqrt ((double)
				      (Dsqr (vA -> x - vmax -> x)
				      + Dsqr (vA -> y - vmax -> y)));
			else if (vA -> x < vmin -> x)
			    d = sqrt ((double)
				      (Dsqr (vA -> x - vmin -> x)
				      + Dsqr (vA -> y - vmin -> y)));
			else
			    d = Abs (vA -> y - vmin -> y);
		    }

		    if (d < dist)
			dist = d;

		    vB = vB -> contbound_next;
		}

		bdrB = bdrB -> bound_next;
	    }

	    if (vA -> contbound_next) {
		/* must be orthogonal */
		ASSERT (vA -> x == vA -> contbound_next -> x
			|| vA -> y == vA -> contbound_next -> y);
	    }
	    vA = vA -> contbound_next;
	}
	bdrA = bdrA -> bound_next;
    }

    ASSERT (dist < INF);

    return (dist);
}

void outputCleanUp ()
{
}

void outputPrintInfo (FILE *fp)
{
}

//------------------------------------------------------------------------------
extern selfsubdata_t * selfs;
extern mutualsubdata_t *muts;
extern int self_cnt, mut_cnt;

#ifdef DEBUG
Private void printSubResData ()
{
    int i;
    fprintf (stderr, "self_cnt=%d\n", self_cnt);
    for (i = 0; i < self_cnt; ++i) {
      fprintf (stderr, "%2d: a=%e p=%e r=%e rest=%e\n", i+1,
	selfs[i].area, selfs[i].perim, selfs[i].val, selfs[i].rest);
    }
    fprintf (stderr, "mut_cnt=%d\n", mut_cnt);
    for (i = 0; i < mut_cnt; ++i) {
      fprintf (stderr, "%2d: a1=%e a2=%e d=%e r=%e decr=%e\n", i+1,
	muts[i].area1, muts[i].area2, muts[i].dist, muts[i].val, muts[i].decr);
    }
}
#endif /* DEBUG */

static double *rsubdevtab = NULL;
static double *rdirdevtab = NULL;

static int minvalsub_i = 0;
static int maxperimsub_i = 0;
static int maxareasub_i = 0;
static int minvaldir_i = 0;
static int maxarea1dir_i = 0;
static int maxarea2dir_i = 0;
static int mindistdir_i = 0;

//------------------------------------------------------------------------------
// compute               | a1 b1 c1 |
//           determinant | a2 b2 c2 |
//                       | a3 b3 c3 |
//
#define Det(a1,a2,a3,b1,b2,b3,c1,c2,c3)  ((a1) * ((b2) * (c3) - (b3) * (c2)) - \
					  (a2) * ((b1) * (c3) - (b3) * (c1)) + \
					  (a3) * ((b1) * (c2) - (b2) * (c1)))
//------------------------------------------------------------------------------

#define DELTA           0.000001
/* The following only works for d1 > 0 && d2 > 0 */
#define EqualD(d1,d2)         ((d1 < d2 * (1+DELTA)) && (d1 > d2 * (1-DELTA)))
#define LargerD(d1,d2)         (d1 > d2 * (1+DELTA))
#define LargerOrEqualD(d1,d2)  (d1 > d2 * (1-DELTA))
#define SmallerD(d1,d2)        (d1 < d2 * (1-DELTA))
#define SmallerOrEqualD(d1,d2) (d1 < d2 * (1+DELTA))

void initSubData ()
{
    int i;

    if (!rsubdevtab) { /* init only once! */
#ifdef DEBUG
	printSubResData ();
#endif
	/* The last selfs entry is used to define a value
	   for a terminal that has an area and perimeter that
	   is always larger than the area and perimeter of the
	   actual terminal (see interpolateRsub()).
	*/
	rsubdevtab = NEW (double, self_cnt+1);

	for (i = 1; i < self_cnt; i++) {
	    if (selfs[i].area  > selfs[maxareasub_i].area) maxareasub_i = i;
	    if (selfs[i].perim > selfs[maxperimsub_i].perim) maxperimsub_i = i;
	    if (selfs[i].val   < selfs[minvalsub_i].val) minvalsub_i = i;
	}

	for (i = 1; i < mut_cnt; i++) {
	    if (muts[i].area1 > muts[maxarea1dir_i].area1) maxarea1dir_i = i;
	    if (muts[i].area2 > muts[maxarea2dir_i].area2) maxarea2dir_i = i;
	    if (muts[i].dist  < muts[mindistdir_i].dist) mindistdir_i = i;
	    if (muts[i].val   < muts[minvaldir_i].val) minvaldir_i = i;
	}

	/* The last muts entry is used to define a value
	   for a terminal pair for which areas and distances
	   are always larger than those of the actual terminal
	   (see (see interpolateRdir()).
	*/
	rdirdevtab = NEW (double, mut_cnt+1);
    }
}

Private double interpolateRsub (double area, double perim, double *rest)
{
 /* double kk, kkk; */
    double mindev, k1, k2, k3, val, n, m, k;
    double perim_big, area_big, denomDet, da, dp;
    int i, min1_i, min2_i, min3_i;
    int found3inArow = 0;

    ASSERT (area  > 0);
    ASSERT (perim > 0);

    if (!self_cnt) {
	say ("Must specify selfsubres data in technology file");
	return (-1);
    }

    /* First we look if the list has 3 entries in a row such that:
        - area 2nd entry == area 1st entry
        - perim 3rd entry = perim 1st entry
    */

    mindev = 0; /* init, else compiler warning */
    min1_i = -1;
    for (i = 0; i < self_cnt; i = i + 3) {
        da = sqrt (selfs[i].area) - sqrt (area);
        dp = selfs[i].perim - perim;
        rsubdevtab[i] = sqrt (da * da + dp * dp);
        if (min1_i < 0 || rsubdevtab[i] < mindev) {
            mindev = rsubdevtab[i];
            min1_i = i;
        }
    }

    min2_i = min1_i + 1;
    min3_i = min1_i + 2;
    if (min2_i < self_cnt && min3_i < self_cnt
        && EqualD (selfs[min1_i].area, selfs[min2_i].area)
        && EqualD (selfs[min1_i].perim, selfs[min3_i].perim)) {
        found3inArow = 1;
    }
    else {
        min1_i = -1;
        min2_i = -1;
        min3_i = -1;
    }

    if (!found3inArow) {

        /* We are going to use an alternative (old) method to select
           3 entries from the 'selfsubres' list. */

        /* The following is to have always a value in the table that is
           (much) smaller than the value that will be computed.
           This way we prevent that due to the interpolation the value
           becomes smaller than zero.
           Note: the smallest value will have the biggest permimeter
           and area.
        */

        area_big = Max (area, selfs[maxareasub_i].area);
        area_big *= 100;
        perim_big = Max (perim, selfs[maxperimsub_i].perim),
        perim_big *= 10;

        da = selfs[minvalsub_i].area / area_big;
        dp = selfs[minvalsub_i].perim / perim_big;

        selfs[self_cnt].perim = perim_big;
        selfs[self_cnt].area = area_big;
        selfs[self_cnt].val  = selfs[minvalsub_i].val * Min (da, dp);
        selfs[self_cnt].rest = selfs[minvalsub_i].rest;

        /* Let min1_i point to the entry that has a {area, perimeter} that
           is closest to the input {area, perimeter}.
        */

        mindev = 1e99;
        min1_i = -1;
        for (i = 0; i <= self_cnt; i++) {
    	    da = selfs[i].area  - area;
    	    dp = selfs[i].perim - perim;
    	    rsubdevtab[i] = sqrt (da * da + dp * dp);
    	    if (min1_i < 0 || rsubdevtab[i] < mindev) {
    	        mindev = rsubdevtab[i];
    	        min1_i = i;
    	    }
        }

        if (min1_i < 0) goto invalid_sub_parameters;

        /* Let min2_i point to the entry that
            - is different from the entry min1_i
            - has a {area} that is closest to the input {area},
              and such that min1_i.area <= area <= min2_i.area
              or min1_i.area >= area >= min2_i.area
        */

        min2_i = -1;
        for (i = 0; i <= self_cnt; i++) {
    	    if (i != min1_i) {
    	        if (min2_i < 0
    		    || (SmallerD (selfs[min1_i].area, area)
                        && LargerOrEqualD (selfs[i].area, area)
    		        && (selfs[min2_i].area < area
    			    || selfs[i].area < selfs[min2_i].area))
    		    || (LargerD (selfs[min1_i].area, area)
                        && SmallerOrEqualD (selfs[i].area, area)
    		        && (selfs[min2_i].area > area
    			    || selfs[i].area > selfs[min2_i].area))
    		    || (EqualD (selfs[min1_i].area, area)
    		        && (Abs (selfs[i].area - area)
    			    < Abs (selfs[min2_i].area - area))))
    		    min2_i = i;
    	    }
        }

        if (min2_i < 0) goto invalid_sub_parameters;

        /* Let min3_i point to the entry that
            - is different from the entry min1_i and min2_i
            - has a {perim} that is closest to the input {perim},
              and such that min1_i.perim < perim < min3_i.perim
              or min1_i.perim > perim > min3_i.perim
        */

        min3_i = -1;
        for (i = 0; i <= self_cnt; i++) {
    	    if (i != min1_i && i != min2_i) {
    	        if (min3_i < 0
    		    || (SmallerD (selfs[min1_i].perim, perim)
                        && LargerOrEqualD (selfs[i].perim, perim)
    		        && (selfs[min3_i].perim < perim
    			    || selfs[i].perim < selfs[min3_i].perim))
    		    || (LargerD (selfs[min1_i].perim, perim)
                        && SmallerOrEqualD (selfs[i].perim, perim)
    		        && (selfs[min3_i].perim > perim
    			    || selfs[i].perim > selfs[min3_i].perim))
    		    || (EqualD (selfs[min1_i].perim, perim
    		        && (Abs (selfs[i].perim - perim)
    			    < Abs (selfs[min3_i].perim - perim)))))
    		    min3_i = i;
    	    }
        }

        if (min3_i < 0) goto invalid_sub_parameters;
    }

    if (sub_res_interp_method == 1 || (sub_res_interp_method == -1 && found3inArow)) {

        if (!found3inArow) {
	    say ("Error: no suitable 'selfsubres' entries for 'sub_res_interp_method 1'");
            die ();
        }

        /* subres_interp_method 1 */

        m = log (selfs[min2_i].val / selfs[min1_i].val)
            / log (selfs[min1_i].perim / selfs[min2_i].perim);

        n = log (selfs[min3_i].val / selfs[min1_i].val)
            / log (selfs[min1_i].area / selfs[min3_i].area);

        k = 1 / (selfs[min1_i].val
                 * pow (selfs[min1_i].area, n)
                 * pow (selfs[min1_i].perim, m));
        /*
        kk = 1 / (selfs[min2_i].val
                  * pow (selfs[min2_i].area, n)
                  * pow (selfs[min2_i].perim, m));
        kkk = 1 / (selfs[min3_i].val
                   * pow (selfs[min3_i].area, n)
                   * pow (selfs[min3_i].perim, m));
        */

        *rest = (selfs[min1_i].rest + selfs[min2_i].rest + selfs[min3_i].rest) / 3;
        if (*rest < 0) *rest = 0;
        if (*rest > 1) *rest = 1;

        val = k * pow (area, n) * pow (perim, m);
    }
    else {
        /* subres_interp_method 0 */

        denomDet = Det (1, 1, 1,
    	  selfs[min1_i].area, selfs[min2_i].area, selfs[min3_i].area,
    	  selfs[min1_i].perim, selfs[min2_i].perim, selfs[min3_i].perim);

        if (denomDet == 0) goto invalid_sub_parameters;

        k1 = Det (1/selfs[min1_i].val, 1/selfs[min2_i].val, 1/selfs[min3_i].val,
    	  selfs[min1_i].area, selfs[min2_i].area, selfs[min3_i].area,
    	  selfs[min1_i].perim, selfs[min2_i].perim, selfs[min3_i].perim)
    	 / denomDet;

        k2 = Det (1, 1, 1,
    	  1/selfs[min1_i].val, 1/selfs[min2_i].val, 1/selfs[min3_i].val,
    	  selfs[min1_i].perim, selfs[min2_i].perim, selfs[min3_i].perim)
    	 / denomDet;

        k3 = Det (1, 1, 1,
    	  selfs[min1_i].area, selfs[min2_i].area, selfs[min3_i].area,
    	  1/selfs[min1_i].val, 1/selfs[min2_i].val, 1/selfs[min3_i].val)
    	 / denomDet;

        if (k1 < 0) k1 = 0;
        if (k2 < 0) k2 = 0;
        if (k3 < 0) k3 = 0;

        *rest = (selfs[min1_i].rest + selfs[min2_i].rest + selfs[min3_i].rest) / 3;
        if (*rest < 0) *rest = 0;
        if (*rest > 1) *rest = 1;

        val = k1 + k2 * area + k3 * perim;
    }
    if (val >= 0) return (val);

invalid_sub_parameters:
    say ("Error: no orthogonal set of typical self substrate resistances available.");
    say ("Cannot perform interpolation for substrate resistances.");
    say ("Please specify more different entries for 'selfsubres' in the element file.");
    return (-1);
}

Private double interpolateRdir (double area1, double area2, double dist, double *pdecr)
{
    double area1_big, area2_big, dist_small;
    double mindev, p, K, val, d1, d2, dd, t1, t2;
    int i, min1_i, min2_i;
    int extra_cnt = 0;

    ASSERT (area1 > 0);
    ASSERT (area2 > 0);
    ASSERT (dist  > 0);

    if (!mut_cnt) {
	say ("Must specify coupsubres data in technology file");
	return (0);
    }

    if (area1 > area2) { /* make area1 <= area2 */
        p = area1; area1 = area2; area2 = p;
    }

    if (sub_res_interp_method == 0) {
        /* The following is to have always a value in the table that is
           (much) smaller than the value that will be computed.
           This way we prevent that due to the interpolation the value
           becomes smaller than zero.
        */
        /* However, I doubt if it makes sense considering the way min1_i
           and min2_i are determined.
        */

        area1_big = Max (area1, muts[maxarea1dir_i].area1);
        area2_big = Max (area2, muts[maxarea2dir_i].area2);
        dist_small = Min (dist, muts[mindistdir_i].dist);

        area1_big *= 100;
        area2_big *= 100;
        dist_small /= 10;

        d1 = (sqrt (muts[minvaldir_i].area1) + sqrt (muts[minvaldir_i].area2))
	        / (sqrt (area1_big) + sqrt (area2_big));
        d2 = dist_small / muts[minvaldir_i].dist;

        muts[mut_cnt].area1 = area1_big;
        muts[mut_cnt].area2 = area2_big;
        muts[mut_cnt].dist = dist_small;
        muts[mut_cnt].val  = muts[minvaldir_i].val * Min (d1, d2);
        muts[mut_cnt].decr = muts[minvaldir_i].decr;

        extra_cnt = 1;
   }

    /* Let min_1 point to entry that has an area1, area2 and perim
       that are closest to the input.
    */

    mindev = 1e99;
    min1_i = -1;
    for (i = 0; i < mut_cnt + extra_cnt; i++) {
	d1 = sqrt (muts[i].area1) - sqrt (area1);
	d2 = sqrt (muts[i].area2) - sqrt (area2);
	dd = muts[i].dist - dist;
	rdirdevtab[i] = sqrt (d1 * d1 + d2 * d2 + dd * dd);
	if (min1_i < 0 || rdirdevtab[i] < mindev) {
	    mindev = rdirdevtab[i];
	    min1_i = i;
	}
    }

    if (min1_i < 0) goto invalid_dir_parameters;

    /* Let min_2 point to the entry that is second closest to
       the input and for which holds that dist min_2 != dist min_1
    */

    d1 = muts[min1_i].dist;
    d2 = 0; /* init, else compiler warning */

    min2_i = -1;
    for (i = 0; i < mut_cnt + extra_cnt; i++) {
	dd = muts[i].dist;
	if (i != min1_i && !EqualD (d1, dd)) {
	    if (min2_i < 0 || rdirdevtab[i] < mindev) {
	        mindev = rdirdevtab[i];
	        min2_i = i;
		d2 = dd;
	    }
	}
    }

    if (min2_i < 0 || d2 == 0) goto invalid_dir_parameters;

    dd = d1 / d2;

    if (dd == 1.0 || dd == 0) goto invalid_dir_parameters;

    t1  = sqrt (muts[min1_i].area1) + sqrt (muts[min1_i].area2);
    t1 *= muts[min1_i].val;
    if (t1 <= 0) goto invalid_dir_parameters;

    t2  = sqrt (muts[min2_i].area1) + sqrt (muts[min2_i].area2);
    t2 *= muts[min2_i].val;
    if (t2 <= 0) goto invalid_dir_parameters;

    p = log (t1 / t2) / log (dd);

 /* if (p == 0) goto invalid_dir_parameters; */ /* SdeG: Why is this invalid? */

    K = (t1 * pow (dist, p)) / pow (muts[min1_i].dist, p);
    if (K == 0) goto invalid_dir_parameters;

    val = (sqrt (area1) + sqrt (area2)) / K;

    /* compute weighted average */

    *pdecr = (rdirdevtab[min1_i] * muts[min1_i].decr
	    + rdirdevtab[min2_i] * muts[min2_i].decr)
	   / (rdirdevtab[min1_i] + rdirdevtab[min2_i]);
    if (val != 0) return (val);

invalid_dir_parameters:
    say ("Error: no orthogonal set of typical coupling substrate resistances available.");
    say ("Cannot perform interpolation for substrate resistances.");
    say ("Please specify more different entries for 'coupsubres' in the element file.");
    return (0);
}

Private double addval (int i, int j, double val)
{
    static double *A;
    static int dim, *N;
    int k;

    if (i < 0) { // do init
	if (j < 1) {
	    if (dim > 0) {
		k = dim * (dim + 1) / 2;
		DISPOSE (N, sizeof(int) * dim);
		DISPOSE (A, sizeof(double) * k);
	    }
	    dim = 0;
	}
	else {
	    dim = j;
	    k = dim * (dim + 1) / 2;
#ifdef DEBUG
	    fprintf (stderr, "initval: dim=%d size=%d\n", dim, k);
#endif
	    A = NEW (double, k);
	    N = NEW (int,  dim);
	    for (k = i = 0; i < dim; ++i) {
		N[i] = k - i;
		for (j = i; j < dim; ++j) A[k++] = 0.0;
	    }
	}
    }
    else {
	if (i > j) { k = i; i = j; j = k; }
	// i <= j
	if (j < dim) {
	    k = N[i] + j;
	    return (A[k] += val);
	}
	ASSERT (0); // j >= dim
    }
    return 0;
}
