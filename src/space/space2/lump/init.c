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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/export.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"
#include "src/space/extract/define.h"

extern int sub_caps_entry;
extern int max_name_length;
extern int minorPrNr;

extern int hasBipoElem;
extern int hasBipoSub;
extern int hasEdgeCaps;
extern int hasSurfCaps;
extern int dsWarning0, dsWarning1;

int debug_hashtab, debug_hashtab2;
int cap_hashsize, res_hashsize;
int inCap, outCap, currIntCap, maxIntCap;
int inRes, outRes, currIntRes, maxIntRes;
int inNod, outNod, currIntNod, maxIntNod;
int currIntClr, maxIntClr, currIntAln;
int inTor, outTor, currIntTor, maxIntTor;
int inSubnod, currIntSubnod, maxIntSubnod;
int inSubtor, currIntSubtor, maxIntSubtor;
int inGrp, outGrp, currIntGrp, maxIntGrp;
int currNeqv;
int areaNodes, areaNodesTotal;
int equiLines;
int inJun, outJun, currIntJun;
int inLBJT, outpLBJT, currIntLBJT;
int inVBJT, outpVBJT, currIntVBJT;
int inPolnode, outPolnode, currIntPolnode;
int warnConnect;
int warnSubnJoin;
int inSubTerm, outSubTerm;
int eliNod;

subnode_t * subnGND = NULL;
subnode_t * subnSUB = NULL;
node_t * nSUB = NULL;

#ifdef HISTOGRAM
int * histogram = NULL;
int histoMaxVal;
int histoSize;
int histoBucket;
bool_t optHisto = FALSE;
#endif

char *nameGND    = "GND";
char *nameSUBSTR = "SUBSTR";

#define MAXSUPPLYNAMES 24
char *pos_supply[MAXSUPPLYNAMES];
char *neg_supply[MAXSUPPLYNAMES];
int no_pos_supply;
int no_neg_supply;

double *capOutFac;
char *capPolarityTab;
int *capAreaPerimType;
bool_t *capAreaPerimEnable;
bool_t termIsLabel;

int jun_caps;

extern int tileCnt;
extern int tileConCnt;

extern elemDef_t * elemDefTab;
extern int elemDefTabSize;

static char * layoutName = NULL;

void initLump (DM_CELL *cellCirKey, DM_CELL *cellLayKey)
{
    static int initInit = 0;
    int i, j, k, totalCnt, cnt, sort, oldSortNr, ofd, type;
    int oldCapSortTabSize, *capTransf, *capSortCnt;
    char *val, *names, *s, **oldCapSortTab, *oldCapPolarityTab;
    capElemDef_t *cap;

    inGrp = outGrp = 0;
    if (prePass) return;

    layoutName = strsave (cellLayKey -> cell);

    inJun = 0;
    outJun = 0;
    currIntJun = 0;
    inVBJT = 0;
    inLBJT = 0;
    outpVBJT = 0;
    outpLBJT = 0;
    currIntVBJT = 0;
    currIntLBJT = 0;
    inPolnode = 0;
    outPolnode = 0;
    currIntPolnode = 0;
    warnConnect = 0;
    warnSubnJoin = 0;

    inCap = outCap = 0;
    inRes = outRes = 0;
    inNod = outNod = 0;
    inTor = outTor = 0;
    currIntCap = maxIntCap = 0;
    currIntRes = maxIntRes = 0;
    currIntNod = maxIntNod = 0;
    currIntTor = maxIntTor = 0;
    currIntClr = maxIntClr = 0; currIntAln = 0;
    inSubnod = 0;
    currIntSubnod = 0;
    maxIntSubnod = 0;
    inSubtor = 0;
    currIntSubtor = 0;
    maxIntSubtor = 0;
    currIntGrp = 0;
    maxIntGrp = 0;
    currNeqv = 0;
    equiLines = 0;
    areaNodes = 0;
    areaNodesTotal = 0;

    eliNod = 0;

    if (initInit == 0) { /* We do this only once when running the program. */
	int debug_caps = paramLookupB ("debug.jun_caps", "off");
	debug_hashtab  = paramLookupB ("debug.hashtab",  "off");
	debug_hashtab2 = paramLookupB ("debug.hashtab2", "off");
	cap_hashsize = paramLookupI ("cap_hashsize", "0");
	res_hashsize = paramLookupI ("res_hashsize", "0");

        val = paramLookupS ("jun_caps", "linear");

        if (strsame (val, "non-linear")) jun_caps = C_NON_LINEAR;
        else if (strsame (val, "area")) jun_caps = C_AREA;
        else if (strsame (val, "area-perimeter")) jun_caps = C_AREA_PERIMETER;
        else if (strsame (val, "separate")) jun_caps = C_SEPARATE;
        else jun_caps = C_LINEAR;

	if (debug_caps) {
	    fprintf (stderr, "-----------------------------------\n");
	    if (jun_caps == C_LINEAR) val = "linear";
	    fprintf (stderr, "parameter jun_caps = %s\n", val);
	    fprintf (stderr, "capSortTabSize = %d\n", capSortTabSize);
	    fprintf (stderr, "-----------------------------------\n");
	}

        capSortCnt = NEW (int, capSortTabSize);

	for (i = 0; i < capSortTabSize; i++) capSortCnt[i] = 0;
	capSortCnt[0] = 1;
	totalCnt = 1;

	if (hasEdgeCaps || hasSurfCaps)
	for (i = 0; i < elemDefTabSize; i++) {
	    type = elemDefTab[i].type;
	    if (type == EDGECAPELEM || type == SURFCAPELEM || type == LATCAPELEM) {
		sort = elemDefTab[i].s.cap.sortNr;
		if (capPolarityTab[sort] == 'x') {
		    if (capSortCnt[sort]) continue;
		    capSortCnt[sort] = 1;
		    totalCnt += 1;
		}
		else if (*capSortTab[sort] == DSCAP_PREFIX) {
		    /* When the sort name starts with a DSCAP_PREFIX,
		       the sort represents MOS D/S capacitance.
		       This sortTab entry must be skipped. */
		    elemDefTab[i].s.cap.sortNr = 0;
		    continue;
		}
		else if (jun_caps == C_AREA_PERIMETER) {
		    if (capSortCnt[sort]) continue;
		    if (capPolarityTab[sort] == 'n') --sort;
		    capSortCnt[sort] = 2;
		    capSortCnt[sort+1] = 2;
		    totalCnt += 4;
		}
		else if (jun_caps == C_SEPARATE) {
		    if (type == LATCAPELEM) continue;
		    if (elemDefTab[i].id != elemDefTab[i+1].id) {
			if (capPolarityTab[sort] == 'n') --sort;
			capSortCnt[sort]++;   /* p */
			capSortCnt[sort+1]++; /* n */
			totalCnt += 2;
		    }
		}
		else if (jun_caps != C_LINEAR) {
		    if (capSortCnt[sort]) continue;
		    if (capPolarityTab[sort] == 'n') --sort;
		    capSortCnt[sort] = 1;
		    capSortCnt[sort+1] = 1;
		    totalCnt += 2;
		}
	    }
	}

        oldCapSortTabSize = capSortTabSize;
        oldCapSortTab     = capSortTab;
        oldCapPolarityTab = capPolarityTab;

        capSortTabSize = totalCnt;
	if (sub_caps_entry) ++capSortTabSize;

        capTransf          = NEW (int, oldCapSortTabSize);
        capSortTab         = NEW (char *, capSortTabSize);
        capPolarityTab     = NEW (char,   capSortTabSize);
	if (jun_caps == C_AREA) {
	    capOutFac      = NEW (double, capSortTabSize);
	}
        capAreaPerimType   = NEW (int,    capSortTabSize);
        capAreaPerimEnable = NEW (bool_t, capSortTabSize);

        for (i = j = k = 0; j < oldCapSortTabSize; j++) {
            capTransf[j] = capSortCnt[j] > 0 ? i : 0;

            for (cnt = 0; cnt < capSortCnt[j]; cnt++) {
                k = i + 2 * cnt;
                capSortTab[k] = oldCapSortTab[j];
                capPolarityTab[k] = oldCapPolarityTab[j];
                capAreaPerimEnable[k] = FALSE;
                if (capPolarityTab[k] != 'x') {
                    if (jun_caps == C_AREA) {
                        capAreaPerimEnable[k] = 2;
			capOutFac[k] = 1.0;
			capAreaPerimType[k] = 0;
                    }
                    else if (jun_caps == C_AREA_PERIMETER) {
                        capAreaPerimEnable[k] = 1;
			capAreaPerimType[k] = (cnt == 0)? 'a' : 'p';
                    }
                    else if (jun_caps == C_SEPARATE) {
                        capAreaPerimEnable[k] = 1;
                    }
                }
            }
	    if (oldCapPolarityTab[j] == 'n')
		i = k + 1;
	    else
		i++;
        }

	if (debug_caps) {
	    fprintf (stderr,  "OLD  capSort  Polarity Transfer Cnt\n");
	    for (j = 0; j < oldCapSortTabSize; j++) {
		fprintf (stderr, "%3d %8s %4c %8d %8d\n", j,
		    oldCapSortTab[j], oldCapPolarityTab[j], capTransf[j], capSortCnt[j]);
	    }
	    fprintf (stderr, "-----------------------------------\n");
	    fprintf (stderr, "totalCnt = %d\n", totalCnt);
	}

	if (sub_caps_entry) {
	    ++k;
	    capSortTab[k] = oldCapSortTab[0];
	    capPolarityTab[k] = 'x';
	    capAreaPerimEnable[k] = FALSE;
	    sub_caps_entry = k;
	}

	ASSERT (k == capSortTabSize - 1);

	if (oldCapSortTabSize > 1 && (hasEdgeCaps || hasSurfCaps))
	for (i = 0; i < elemDefTabSize; i++) {
	    type = elemDefTab[i].type;
	    if (type == EDGECAPELEM || type == SURFCAPELEM || type == LATCAPELEM) {

		cap = &elemDefTab[i].s.cap;
		if ((oldSortNr = cap -> sortNr) == 0) continue;
                cap -> sortNr = sort = capTransf[oldSortNr];

		if (debug_caps) {
		    fprintf (stderr, "type=%s oldSortNr=%d sortNr=%d\n",
			(type == EDGECAPELEM ? "E" : (type == SURFCAPELEM ? "S" : "L")), oldSortNr, sort);
		}

		if (capPolarityTab[sort] == 'x') continue;
		if (capPolarityTab[sort] == 'n') --sort;

		if (jun_caps == C_AREA_PERIMETER) {
		    cap -> val = 1.0;
		    if (type == EDGECAPELEM) cap -> sortNr += 2;
		}
		else if (jun_caps == C_SEPARATE) {
		    cap -> val = 1.0;
		    if (type == SURFCAPELEM) {
			capAreaPerimType[sort] = 'a';
			capAreaPerimType[sort + 1] = 'a';
		    }
		    else { /* EDGECAPELEM */
			ASSERT (type != LATCAPELEM);
			capAreaPerimType[sort] = 'p';
			capAreaPerimType[sort + 1] = 'p';
		    }
		    if (elemDefTab[i+1].id != elemDefTab[i].id)
			capTransf[oldSortNr] += 2;

		    if (debug_caps) {
			fprintf (stderr, "capAreaPerimType[%d,%d]=%c\n",
			    sort, sort+1, capAreaPerimType[sort]);
		    }
		}
		else if (jun_caps == C_AREA) {
		    ofd = capAreaPerimType[sort];
                    if (ofd == 0 || (ofd == EDGECAPELEM && type == SURFCAPELEM)) {
                        capOutFac[sort] = cap -> val;
                        capAreaPerimType[sort] = type;
                        capOutFac[sort + 1] = cap -> val;
                        capAreaPerimType[sort + 1] = type;
                    }
                    else if (ofd == SURFCAPELEM) {
                        if (type == SURFCAPELEM && cap -> val != capOutFac[sort]) {
                            say ("Sorry, can not handle different area values for the same junction");
			    say ("\tcapacitance type (%s) when parameter jun_caps=area", capSortTab[sort]);
			    die ();
                        }
                    }
                    else {
                        ASSERT (ofd == EDGECAPELEM && type == EDGECAPELEM);
                        if (cap -> val != capOutFac[sort]) {
                            /* This is an error when capAreaPerimType[sort]
                               will not be overruled by a SURFCAPELEM. */
                            capOutFac[sort] = 1234321;
                            capOutFac[sort + 1] = 1234321;
                        }
                    }
                }
            }
        }

	if (debug_caps) {
	    double facVal;
	    fprintf (stderr, "capSortTabSize = %d\n", capSortTabSize);
	    fprintf (stderr, "-----------------------------------\n");
	    if (jun_caps == C_AREA)
		fprintf (stderr,  "NEW  capSort  Pol apEn apTyp facEn facTyp facVal\n");
	    else
		fprintf (stderr,  "NEW  capSort  Pol apEn apTyp facEn\n");
	    for (j = 0; j < capSortTabSize; j++) {
		i = capAreaPerimEnable[j] == 1 ? capAreaPerimType[j] : 'x';
		fprintf (stderr, "%3d %8s %4c %3d %4c %6d", j, capSortTab[j],
		    capPolarityTab[j], capAreaPerimEnable[j] == 1, i, capAreaPerimEnable[j] == 2);
		if (jun_caps == C_AREA) {
		    val = "x"; facVal = 0;
		    if (capAreaPerimEnable[j]) {
			if (capAreaPerimType[j] == EDGECAPELEM) val = "E";
			else if (capAreaPerimType[j] == SURFCAPELEM) val = "S";
			facVal = capOutFac[j];
		    }
		    fprintf (stderr, "%6s %8g", val, facVal);
		}
		fprintf (stderr, "\n");
	    }
	    fprintf (stderr, "-----------------------------------\n");
	}

	if (jun_caps == C_AREA && (hasEdgeCaps || hasSurfCaps))
        for (i = 0; i < capSortTabSize; i++) {
	    if (capAreaPerimEnable[i] && capOutFac[i] == 1234321) {
	        say ("Sorry, can not handle different edge values for the same junction");
	        say ("\tcapacitance type (%s) when parameter jun_caps=area.", capSortTab[i]);
	        die ();
            }
        }

	initInit = 1;

	/* We do also next part only once! */

	no_pos_supply = 0;
	if ((names = paramLookupS ("pos_supply", NULL)))
	while ((s = strtok (names, " \t,"))) {
	    names = NULL;
	    if (no_pos_supply >= MAXSUPPLYNAMES) {
		say ("Sorry, can handle only %d names for parameter pos_supply", MAXSUPPLYNAMES);
		break;
	    }
	    pos_supply[no_pos_supply++] = s;
	}

	no_neg_supply = 0;
	if ((names = paramLookupS ("neg_supply", NULL)))
	while ((s = strtok (names, " \t,"))) {
	    names = NULL;
	    if (no_neg_supply >= MAXSUPPLYNAMES) {
		say ("Sorry, can handle only %d names for parameter neg_supply", MAXSUPPLYNAMES);
		break;
	    }
	    neg_supply[no_neg_supply++] = s;
	}

	termIsLabel = paramLookupB ("term_is_netname", "off");
    }

    nqInit ();

    if (!extrPass) return;

#ifdef HISTOGRAM
    if (paramLookupB ("histogram", "off")) {
	optHisto = TRUE;
	histoSize = 1000;
	histoBucket = optCoupCap ? 5 : 1;
	if (!histogram) histogram = NEW (int, histoSize);
	for (i = 0; i < histoSize; i++) histogram[i] = 0;
	histoMaxVal = 0;
    }
    else optHisto = FALSE;
#endif /* HISTOGRAM */

    subnSUB = NEW (subnode_t, 1);
    subnodeNew (subnSUB);
    nSUB = subnSUB -> node;
    nSUB -> term = 2; // KEEPNODE
    nSUB -> node_x = bigbxl;
    nSUB -> node_y = bigbyb;
    nSUB -> mask = -1;
    if (hasBipoElem && hasBipoSub) polnodeAdd (subnSUB, -2, 'a');

    initOut (cellCirKey, cellLayKey);
}

void endLump ()
{
    int i;
    FILE *fp_info;

    if (!extrPass) return;

    if (dsWarning0 > 1) say ("warning: %d transistors with no drain/source terminals %s", dsWarning0, omit_inc_tors? "skipped" : "detected");
    if (dsWarning1 > 1) say ("warning: %d transistors with only one drain/source terminal %s", dsWarning1, omit_inc_tors? "skipped" : "detected");

    ASSERT (currIntNod == 1);
    subnodeDel (subnSUB); /* finish group of substrate node */

    DISPOSE (subnSUB, sizeof(subnode_t));

    /* We output the substrate node before the ground node because when the
       substrate node is outputted gndCap_cnt may be increased.
    */
    outGndNode ();

    endOut ();

#ifdef HISTOGRAM
    if (optHisto) {
	long totalcost;
	int size;
	if (optCoupCap)
	    message ("\nhistogram of R^2 + R C\n");
	else
	    message ("\nhistogram of R\n");
	message ("       ");
	for (i = 0; i < 10; i++) message ("%5d ", i * histoBucket);
	message ("\n      ");
	message ("------------------------------------------------------------ ");
	totalcost = 0;
	size = histoMaxVal / histoBucket;
	if (++size > histoSize) size = histoSize;
	for (i = 0; i < size; i++) {
	    if (i % 10 == 0) message ("\n%5d| ", i * histoBucket);
	    message ("%5d " , histogram[i]);
	    if (optCoupCap)
	        totalcost = totalcost + i * histogram[i];
	    else
	        totalcost = totalcost + (i * (i + 1)) * histogram[i];
	}
	message ("\n\n");
	message ("max value  : %d\n", histoMaxVal);
	message ("total cost : %ld\n\n", totalcost);
    }
#endif /* HISTOGRAM */

    if (currIntClr > 0) fprintf (stderr, "\nWARNING: %d cluster items left behind in core!\n", currIntClr);
    if (currIntAln > 0) fprintf (stderr, "\nWARNING: %d cluster Alnks left behind in core!\n", currIntAln);
    if (currIntNod > 0) fprintf (stderr, "\nWARNING: %d node items left behind in core!\n", currIntNod);
    if (currIntGrp > 0) fprintf (stderr, "\nWARNING: %d group items left behind in core!\n", currIntGrp);
    if (currIntCap > 0) fprintf (stderr, "\nWARNING: %d cap items left behind in core!\n", currIntCap);
    if (currIntRes > 0) fprintf (stderr, "\nWARNING: %d res items left behind in core!\n", currIntRes);
    if (currIntTor > 0) fprintf (stderr, "\nWARNING: %d tor items left behind in core!\n", currIntTor);

    if (currIntJun > 0 || currIntVBJT > 0 || currIntLBJT > 0 || currIntPolnode > 0) {
	fprintf (stderr, "\nWARNING: Bipolar circuit items left behind in core!\n");
	fprintf (stderr, "\tHint: check the correctness of the condition list for\n");
	fprintf (stderr, "\tbipolar transistors in the element definition file.\n");
    }

    if (currNeqv > 0) fprintf (stderr, "\nWARNING: %d net equivalences left behind in core!\n", currNeqv);

    if (optInfo) {
	int w1, w2, w3, m;
	w1 = w2 = w3 = 6;
	m = inSubnod;
	for (i = 1000000; m >= i; i *= 10) ++w1;
	m = outCap;
	if (outRes > m) m = outRes;
	if (outNod > m) m = outNod;
	for (i = 1000000; m >= i; i *= 10) ++w2;
	m = maxIntCap;
	if (maxIntRes    > m) m = maxIntRes;
	if (maxIntNod    > m) m = maxIntNod;
	if (maxIntSubnod > m) m = maxIntSubnod;
	for (i = 1000000; m >= i; i *= 10) ++w3;
	fp_info = stdout;
	fprintf (fp_info, "\nextraction statistics for layout %s:\n", layoutName);
	fprintf (fp_info, "                %*s %*s %*s %s\n", w1, "in", w2, "out", w3, "maxInt", "remaining");
	fprintf (fp_info, "capacitances  : %*d %*d %*d %3d\n", w1, inCap, w2, outCap, w3, maxIntCap, currIntCap);
	fprintf (fp_info, "resistances   : %*d %*d %*d %3d\n", w1, inRes, w2, outRes, w3, maxIntRes, currIntRes);
	fprintf (fp_info, "node groups   : %*d %*d %*d %3d\n", w1, inGrp, w2, outGrp, w3, maxIntGrp, currIntGrp);
	fprintf (fp_info, "nodes         : %*d %*d %*d %3d\n", w1, inNod, w2, outNod, w3, maxIntNod, currIntNod);
	fprintf (fp_info, "subnodes      : %*d %*s %*d %3d\n", w1, inSubnod, w2, "--", w3, maxIntSubnod, currIntSubnod);
	fprintf (fp_info, "mosfets       : %*d %*d %*d %3d\n", w1, inTor, w2, outTor, w3, maxIntTor, currIntTor);
	fprintf (fp_info, "subtors       : %*d %*s %*d %3d\n", w1, inSubtor, w2, "--", w3, maxIntSubtor, currIntSubtor);
	fprintf (fp_info, "polnodes      : %*d %*d %*s %3d\n", w1, inPolnode, w2, outPolnode, w3, "--", currIntPolnode);
	fprintf (fp_info, "junctions     : %*d %*d %*s %3d\n", w1, inJun, w2, outJun, w3, "--", currIntJun);
	fprintf (fp_info, "bipolar ver.  : %*d %*d %*s %3d\n", w1, inVBJT, w2, outpVBJT, w3, "--", currIntVBJT);
	fprintf (fp_info, "bipolar lat.  : %*d %*d %*s %3d\n", w1, inLBJT, w2, outpLBJT, w3, "--", currIntLBJT);
      if (substrRes)
	fprintf (fp_info, "sub term nodes: %*d %*d\n", w1, inSubTerm, w2, outSubTerm);
	fprintf (fp_info, "tiles         : %*d\n", w1, tileCnt);
	fprintf (fp_info, "conduc tiles  : %*d\n", w1, tileConCnt);
	fprintf (fp_info, "\n");
	fprintf (fp_info, "nbr. of nodes eliminated : %d\n", eliNod);
	fprintf (fp_info, "equi area nodes          : %d\n", areaNodes);
	fprintf (fp_info, "equi line nodes          : %d\n", equiLines);
	fprintf (fp_info, "total ready area nodes   : %d\n", areaNodesTotal);
	fprintf (fp_info, "\n");
    }
    else if (optVerbose) {
	fp_info = stdout;
	fprintf (fp_info, "\nextraction statistics for layout %s:\n", layoutName);
	fprintf (fp_info, "\tcapacitances       : %d\n", outCap);
	fprintf (fp_info, "\tresistances        : %d\n", outRes);
	fprintf (fp_info, "\tnodes              : %d\n", outNod);
	fprintf (fp_info, "\tmos transistors    : %d\n", outTor);
      if (outpVBJT || outpLBJT) {
	fprintf (fp_info, "\tbipolar vertical   : %d\n", outpVBJT);
	fprintf (fp_info, "\tbipolar lateral    : %d\n", outpLBJT);
      }
      if (substrRes)
	fprintf (fp_info, "\tsubstrate terminals: %d\n", inSubTerm);
	fprintf (fp_info, "\tsubstrate nodes    : %d\n", outSubTerm);
	fprintf (fp_info, "\n");
    }
}
