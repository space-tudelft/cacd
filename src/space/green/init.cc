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

/*
 * Extensions by Theo Smedes for substrate resistances 1995.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/green/green.h"
#include "src/space/green/extern.h"
#include "src/space/extract/export.h"
#include "src/space/spider/export.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

int      maxGreenTerms = 0;	/* number of terms to do from infinite series */

green_t  collocationEps = 0.01;
green_t  greenEps = 0.01;

double subcontZposition        = 0;
real_t pointGreenDist          = 0;
real_t collocationGreenDist    = 0;
real_t asymCollocationGreenDist = 0;

int numPointGreen = 0;
int numCollocationGreen = 0;
int numGalerkinGreen = 0;
int numImages = 0;
int numImageGroups = 0;
int numMultipoleImages = 0;
int numMultipoleImagesTraditional = 0;

long green_cnt;
long noconverge_cnt;

bool_t offDiagonal = FALSE;		/* more conservative */

static struct greenData {
    int nlayers[2]; /* 0 vacuum, 1 = single dielectr. above ground, etc. */
    real_t    bounds[2][GREEN_MAX_LAYERS + 1]; /* position[0] unused */
    char *  material[2][GREEN_MAX_LAYERS + 1];
    green_t matconst[2][GREEN_MAX_LAYERS + 1];
    bool_t  useLowestMedium[2];
    bool_t  useOldImages[2];
} greenData;

real_t greenMeters[2];

bool_t edgeEffects = FALSE;
coor_t sawDist, edgeDist;

bool_t hasSimpleGreen[2];

/* Switches for use/enforcement of analytical and adaptive integration (U.G.)
 *
 * useAdptvIntgrtn: use adaptive integration when Stroud's formulas failed
 *                  to achieve the desired precision, in conventional
 *                  calculation of Green function  (galerkin.c)
 * forceAdptvIntgrtn: enforce use of adaptive integration instead of Stroud's
 *                    formulas, in conventional calculation of Green function
 *                  to achieve the desired precision  (galerkin.c)
 * forceMpAnaInt: when TRUE, enforce use of analytical formulas for inner
 *                (collocation) integral in  multipole routine for  Green function (mpgreen.c)
 * forceMpExInt: when TRUE, enforce use of analytical  a n d   adaptive numerical
 *               integration in multipole routine for  Green function (mpgreen.c)
 */
bool_t useAdptvIntgrtn = FALSE, forceAdptvIntgrtn = FALSE, forceMpAnaInt = TRUE, forceMpExInt = FALSE;

/*  Multipole variables (U.G.) */
real_t multipolesMindist;
int    MaxMpOrder;
int    MaxMpNr;
bool_t printGreenTerms = FALSE;
bool_t printGreenGTerms = FALSE;
bool_t useLowestMedium = FALSE;
bool_t useMeanGreenValues = FALSE;
bool_t useMultiPoles = FALSE;
bool_t useOldImages = FALSE;
bool_t mergedImages = FALSE;
bool_t testMultiPoles = FALSE;  /* makes comparison between multipole-based and
                                   traditional calculation of Green function */

/*
 * Initialize the greens function evaluation module, reads
 * parameters, etc.
 *
 * The argument specifies the geometric scale (how many meters per unit)
 * of the green module.
 *
 * A scale factor, including the effects of the dielectric constants
 * as well as of the geometric scale, for the green's function values
 * is returned.
 * This scale factor should be applied after matrix inversion,
 * to keep numerical values around unity for easier interpretation
 * and smaller round-off errors.
 */
double greenInit (double meters)
{
    static int initDIEL, initSUBS;
    static real_t microns = 0;
    double d, greenFactor;
    char *val;
    int i;
    bool_t printInit = paramLookupB ("debug.print_green_init", "off");

    greenMeters[greenCase] = meters;

    printGreenTerms  = paramLookupB ("debug.print_green_terms", "off");
    printGreenGTerms = paramLookupB ("debug.print_green_gterms", "off");

    if (greenCase == DIEL) {
	if (!initDIEL) {

	    hasSimpleGreen[DIEL] = (diel_cnt > GREEN_MAX_DIELECTRICS_SIMPLE)? FALSE : TRUE;

	    if (!hasSimpleGreen[DIEL]) {
		fprintf (stderr, "error: Can only handle %d dielectric layers.", GREEN_MAX_DIELECTRICS_SIMPLE);
		die ();
	    }
	}
    }
    else {
	if (!initSUBS) {

	    hasSimpleGreen[SUBS] = TRUE;
	    if (substr_cnt > GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE) {
		if (!(substr_cnt == GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE+1 &&
		    strsame (substrs[GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE].name, "metalization"))) {
		    hasSimpleGreen[SUBS] = FALSE;
		}
	    }

	    if (!hasSimpleGreen[SUBS]) {
		fprintf (stderr, "error: Can only handle %d substrate layers.", GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE);
		die ();
	    }
	}
    }

    /*
     * Interpret the dimensions in spiderControl and greenData as microns
     * and convert them into internal layout units.
     * Variable meters specifies the number of meters per layout unit.
     * The dielectric layer bottoms are defined as microns.
     */
    if (microns == 0)  microns = meters * 1e6;
    else if (microns / (real_t)(meters * 1e6) > 1.0000000001
	  || microns / (real_t)(meters * 1e6) < 0.9999999999) {
	say ("error: layout unit changed during program execution (from %le to %le)!",
		microns, (real_t)(meters * 1e6));
	die ();
    }

    green_cnt = 0;
    noconverge_cnt = 0;

    useAdptvIntgrtn = paramLookupB ("use_adptv_intgrtn", "off");
    forceAdptvIntgrtn = paramLookupB ("force_adptv_intgrtn", "off");
    forceMpAnaInt = paramLookupB ("force_mp_anaint", "on");
    forceMpExInt = paramLookupB ("force_mp_exint", "off");

    useMeanGreenValues = (greenCase == DIEL
		? paramLookupB ("cap3d.use_mean_green_values", "off")
		: paramLookupB ("sub3d.use_mean_green_values", "off"));
    mergedImages = (greenCase == DIEL
		? paramLookupB ("cap3d.merge_images", "on")
		: paramLookupB ("sub3d.merge_images", "on"));

    useMultiPoles = paramLookupB ("use_multipoles", "on");
    testMultiPoles = paramLookupB ("test_multipoles", "off");

    multipolesMindist = (greenCase == DIEL
		? paramLookupD ("cap3d.mp_min_dist", "2.0")
		: paramLookupD ("sub3d.mp_min_dist", "2.0"));
    if (multipolesMindist < 1) multipolesMindist = 1;

    MaxMpOrder = (greenCase == DIEL
		? paramLookupI ("cap3d.mp_max_order", "2")
		: paramLookupI ("sub3d.mp_max_order", "2"));
    MaxMpNr = 1;
    if (MaxMpOrder >= 1) MaxMpNr += 2;
    if (MaxMpOrder >= 2) MaxMpNr += 9;
    if (MaxMpOrder >= 3) MaxMpNr += 27;

    edgeEffects = FALSE;

    if (greenCase == SUBS) {
        d = paramLookupD ("sub3d.saw_dist", "infinity") / microns;
        sawDist = (coor_t) Max (-INF, Min (INF, d));

        d = paramLookupD ("sub3d.edge_dist", "0.0") / microns;
        edgeDist = (coor_t) Max (-INF, Min (INF, d));

        if (edgeDist > sawDist) {
            if (useMultiPoles) {
		say ("Warning: in the current version sidewall effects %s",
		    "are only approximated when parameter 'use_multipoles' is 'off'!");
            }
            else {
                edgeEffects = TRUE;
                say ("Sidewall effects will be approximated for elements close to edge!");
            }
	}
    }

    if (printInit)
	fprintf (stderr, "greenInit: FeModeGalerkin=%d FeModePwl=%d\n", FeModeGalerkin, FeModePwl);

    if (greenCase == DIEL && !initDIEL) {
	initDIEL = 1;

	greenData.useLowestMedium[0] = paramLookupB ("cap3d.use_lowest_medium", "on");
	greenData.useOldImages[0] = paramLookupB ("cap3d.use_old_images", "off");

	if (printInit)
	    fprintf (stderr, "greenInit: reading dielectric specification\n");

	for (i = 1; i <= diel_cnt; i++) {
	    char *m = diels[i-1].name;

	    if (printInit)
		fprintf (stderr, "greenInit: (%d) name='%s' e=%g b=%g\n", i,
		    m, diels[i-1].permit, diels[i-1].bottom);

	    if (i > GREEN_MAX_LAYERS)
		paramError (mprintf ("dielectric %s", m),
		    "can only handle %d layers", GREEN_MAX_LAYERS), die ();
	    if (diels[i-1].permit <= 0)
		paramError (mprintf ("dielectric %s", m), "epsilon must be > 0"), die ();
	    if (i == 1 && diels[i-1].bottom != 0)
		paramError (mprintf ("dielectric %s", m), "bottom must be == 0"), die ();
	    if (i != 1 && diels[i-1].bottom <= diels[i-2].bottom)
		paramError (mprintf ("dielectric %s", m), "bottom must be > previous bottom"), die ();

	    greenData.material[0][i] = strsave (m);
	    greenData.matconst[0][i] = diels[i-1].permit;
	    greenData.bounds[0][i] = diels[i-1].bottom / microns;
	    greenData.nlayers[0] = i;
	}

	if (greenData.nlayers[0] == 0) { /* uniform dielectric */
	    greenData.material[0][1] = strsave ("vacuum");
	    if (printInit)
		fprintf (stderr, "greenInit: (0) name='vacuum' e=1 b=0\n");
	}
    }

    if (greenCase == DIEL) {
	useLowestMedium = greenData.useLowestMedium[0];
	useOldImages = greenData.useOldImages[0];

	cap3dSettings (greenData.nlayers[0],
	    greenData.matconst[0]+1, greenData.bounds[0]+1);
    }

    if (greenCase == SUBS && !initSUBS) {
	initSUBS = 1;

	greenData.useLowestMedium[1] = paramLookupB ("sub3d.use_lowest_medium", "on");
	greenData.useOldImages[1] = paramLookupB ("sub3d.use_old_images", "off");
	subcontZposition = 0;

	if (printInit)
	    fprintf (stderr, "greenInit: reading substrate specification\n");

	for (i = 1; i <= substr_cnt; i++) {
	    char *m = substrs[i-1].name;

	    if (printInit)
		fprintf (stderr, "greenInit: (%d) name='%s' s=%g t=%g\n", i,
		    m, substrs[i-1].conduc, substrs[i-1].top);

	    if (strsame (m, "metalization")) {
		if (i != substr_cnt)
		    paramError (mprintf ("sublayer %s", m), "this layer must be last!"), die ();
	    }
	    else if (i > GREEN_MAX_LAYERS)
		paramError (mprintf ("sublayer %s", m),
		    "can only handle %d layers", GREEN_MAX_LAYERS), die ();
	    if (substrs[i-1].conduc <= 0)
		paramError (mprintf ("sublayer %s", m), "sigma must be > 0"), die ();
	    if (i == 1 && substrs[i-1].top != 0)
		paramError (mprintf ("sublayer %s", m), "top must be == 0"), die ();
	    if (i != 1 && substrs[i-1].top >= substrs[i-2].top)
		paramError (mprintf ("sublayer %s", m), "top must be < previous top"), die ();

	    greenData.material[1][i] = strsave (m);
	    greenData.matconst[1][i] = substrs[i-1].conduc;
	    greenData.bounds[1][i] = substrs[i-1].top;
	    greenData.nlayers[1] = i;

	    if (i == substr_cnt && strsame (m, "metalization")) { /* grounded substrate */
		char *param = (char*)"sub3d.neumann_simulation_ratio";
		if ((val = paramLookupS (param, 0))) {
		    d = paramLookupD (param, val);
		    if (d <= 0) paramError (param, "must be > 0"), die ();
		}
		else d = 100; // default
		if (printInit) fprintf (stderr, "greenInit: %s=%g\n", param+6, d);
		if (i == 1) say ("error: no substrate layer specified (only metalization)"), die ();
		initSUBS = 2;
		greenData.matconst[1][i] = greenData.matconst[1][1] / d;
		greenData.bounds[1][i] = -greenData.bounds[1][i];
		subcontZposition = greenData.bounds[1][i] / microns;
		for (i = 2; i < substr_cnt; ++i) {
		    greenData.bounds[1][i] += greenData.bounds[1][substr_cnt];
		}
		for (i = 1; i+1 < substr_cnt-i; ++i) {
		    d = greenData.bounds[1][i+1];
		    greenData.bounds[1][i+1] = greenData.bounds[1][substr_cnt-i];
		    greenData.bounds[1][substr_cnt-i] = d;
		}
		for (i = 1; i < substr_cnt-i; ++i) {
		    d = greenData.matconst[1][i];
		    greenData.matconst[1][i] = greenData.matconst[1][substr_cnt-i];
		    greenData.matconst[1][substr_cnt-i] = d;
		}
		if (printInit)
		for (i = 1; i <= substr_cnt; ++i) {
		    fprintf (stderr, "greenInit: (%d) e=%g b=%g\n", i,
			greenData.matconst[1][i], greenData.bounds[1][i]);
		}
		break;
	    }
	}

	if (greenData.nlayers[1] == 0)
	    say ("error: no substrate layer specified!"), die ();
	for (i = 1; i <= greenData.nlayers[1]; i++)
	    greenData.bounds[1][i] /= microns;
    }

    if (greenCase == SUBS) {
	if (initSUBS > 1) greenType = 3; /* grounded substrate */
	useLowestMedium = greenData.useLowestMedium[1];
	useOldImages = greenData.useOldImages[1];

	sub3dSettings (greenData.nlayers[1],
	    greenData.matconst[1]+1, greenData.bounds[1]+1);
    }

    pointGreenDist = (greenCase == DIEL
	? paramLookupD ("cap3d.point_green", "infinity")
	: paramLookupD ("sub3d.point_green", "infinity"));
    collocationGreenDist = (greenCase == DIEL
	? paramLookupD ("cap3d.collocation_green", (char*) (FeModeGalerkin ? "infinity" : "0"))
	: paramLookupD ("sub3d.collocation_green", (char*) (FeModeGalerkin ? "infinity" : "0")));
    asymCollocationGreenDist = (greenCase == DIEL
	? paramLookupD ("cap3d.asym_collocation_green", "infinity")
	: paramLookupD ("sub3d.asym_collocation_green", "infinity"));

    if (greenData.nlayers[greenCase] < 2)
	maxGreenTerms = 1;
    else {
	maxGreenTerms = (greenCase == DIEL
		? paramLookupI ("cap3d.max_green_terms", mprintf ("%d", GREEN_MAX_TERMS))
		: paramLookupI ("sub3d.max_green_terms", mprintf ("%d", GREEN_MAX_TERMS)));
	if (maxGreenTerms > GREEN_MAX_TERMS) {
	    maxGreenTerms = GREEN_MAX_TERMS;
	    say ("max_green_terms set to absolute maximum %d", maxGreenTerms);
	}
	else if (maxGreenTerms < 1) {
	    maxGreenTerms = 1;
	    say ("max_green_terms set to absolute minimum %d", maxGreenTerms);
	}
    }

    greenEps = (greenCase == DIEL
		? paramLookupD ("cap3d.green_eps", "0.001")
		: paramLookupD ("sub3d.green_eps", "0.001"));

    /* When doing numerical integration, the integrand evaluations
     * are performed using higher accuaracy then the required
     * accuracy of the integral.
     * greenEps       = general accuracy of influence matrix entries, whether
     *                  Galerkin or collocation.
     * collocationEps = accuracy of collocation, equal to greenEps
     *                  in collocation mode, or equal to
     *                  collocationRelativeEps * greenEps in Galerkin mode.
     */
    if (FeModeGalerkin == FALSE) {
	collocationEps = greenEps;
    }
    else {
	d = (greenCase == DIEL
		? paramLookupD ("cap3d.col_rel_eps", "0.2")
		: paramLookupD ("sub3d.col_rel_eps", "0.2"));
	collocationEps = d * greenEps;
    }

    if (printInit) {
	fprintf (stderr, "greenInit: use_multipoles=%d test_multipoles=%d\n",
	    useMultiPoles, testMultiPoles);
	fprintf (stderr, "greenInit: use_lowest_medium=%d use_old_images=%d\n",
	    useLowestMedium, useOldImages);
	fprintf (stderr, "greenInit: use_mean_green_values=%d merge_images=%d\n",
	    useMeanGreenValues, mergedImages);
	fprintf (stderr, "greenInit: greenType=%d nLayers=%d maxGreenTerms=%d collocationEps=%g\n",
	    greenType, greenData.nlayers[greenCase], maxGreenTerms, collocationEps);
    }

    /* greenFactor is defined here.
     * It is used to scale the values of green's function
     * computations to physical dimensions, the green's function
     * computations are carried out on internal units.
     * It is equivalent to divide these values by greenFactor
     * and to multiply the values after matrix inversion
     * by greenFactor.
     * We presently implement this latter strategy, since
     * this results in numbers around unity, which should
     * give fewer round-off errors in the matrix inversion
     * and easier interpretation of these values during debugging.
     *
     * Note that we also delay the division by 4*pi.
     * T.S.: multiplication by Epsilon0 moved to cap3d.c
     */

    greenFactor = meters * 4.0 * M_PI;

    return (greenFactor);
}

char *layerName (int i)
{
    if (greenCase == DIEL)
	return (greenData.material[greenCase][i+1]);
    return (greenData.material[greenCase][1]);
}

/*
 * Print statistics to stream indicated by fp.
 */
void greenStatistics (FILE *fp)
{
    fprintf (fp, "overall green statistics:\n");
    fprintf (fp, "\tpoint greens         : %d\n", numPointGreen);
    fprintf (fp, "\tcollocation greens   : %d\n", numCollocationGreen);
    fprintf (fp, "\tgalerkin greens      : %d\n", numGalerkinGreen);
    fprintf (fp, "\timages evaluated     : %d\n", numImages);
    fprintf (fp, "\timagegroups evaluated: %d\n", numImageGroups);
    fprintf (fp, "\tmp images            : %d\n", numMultipoleImages);
    fprintf (fp, "\tmp images traditional: %d\n", numMultipoleImagesTraditional);
}

void greenNoConvergence (int i, int j, int terms)
{
    if (noconverge_cnt++ == 0) {
	say ("%s %d green_terms,\n   %s (layers are '%s' and '%s').",
	"Computation of Greens function truncated after", terms,
	"error specified by green_eps not reached", layerName (i), layerName (j));
    }
}

void printIfNoconverge ()
{
    if (noconverge_cnt > 0) {
	say ("Warning: maximum error not reached for %.1f%% of the Greens functions.",
		noconverge_cnt * 100.0 / green_cnt);
    }
}
