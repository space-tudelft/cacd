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
#include <string.h>
#include <unistd.h>	/* for access */
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/extract/define.h"
#include "src/space/extract/extern.h"
#include "src/space/bipolar/define.h"
#include <ctype.h>

#define MajorProtNr 2

/* Next defines must be < 0.
 */
#define ELEM_END   -1
#define ELEM_IDEM  -2
#define ELEM_ERROR -3

#ifdef DRIVER
extern char *argv0;
bool_t optCap3D = FALSE;
bool_t prVerbose = FALSE;
bool_t prTable = FALSE;
#endif

int nrOfMasks;
int nrOfConductors;
int nrOfCondStd;
int nrOfCondPos;
int nrOfCondNeg;
int nrOfSpiderLevels;
int *displaceList;

int hasSubmask = 0;
int hasBipoELEM = 0;
int hasBipoSub = 0;
int hasEdgeConnect = 0;
int hasSurfConnect = 0;
int hasEdgeCaps = 0;
int hasSurfCaps = 0;
int hasShapeElems = 0;

/* These variables are, for example, used for displaying purposes,
 * to adjust the transformation dependend on the z-dimension. (XSPACE)
 * They are also used to determine the z-extend for rounding purposes. (CAP3D)
 */
int vdimCount = 0;
int vdimIndex = 0;
double vdimZtop = 0;

maskinfo_t * masktable;

int        * keyTab;	/* array containing pointers into elemTab */
int        * keyTab2;	/* array    ,,   for edge elements (SdeG) */
elemDef_t ** elemTab;	/* array containing pointers to elemDef_t's */
elemDef_t ** elemTab2;	/* array    ,,   for edge elements (SdeG) */
elemDef_t ** elemListSpaceE;	/* see "recog.c" */
elemDef_t ** elemListSpaceS;	/* see "recog.c" */
elemDef_t ** otherElemSpace;	/* see "enumpair.c" */
elemDef_t ** otherElemSpace2;	/* see "latcap.c" */

elemDef_t * elemDefTab;	/* contains the techn. elements */
int elemDefTabSize;
int keyTabSize = 0;
int keyTabSize2 = 0;

/*============================================================================*/
/*! @brief Resistance "sort" table.

    Each resistance "sort" defined in the tecc file has its own index in this
    table. The table contains the names of the resistance sorts. For example,
    "res" is the common name used for ordinary resistances, and is always
    available, usually at index 0 in the table, but this is NOT guaranteed.

    You can specify a resistance sort by specifying a name after the
    keyword "conductors" in the tecc file. If you leave out the name, then
    the name "res" will be used.

    Resistances of different sorts are manipulated separately by Space.
    Ordinary resistances and p-diffusion resistances, for example, will
    never be collapsed into a single resistance in the final circuit model.

    The variable <code>resSortTabSize</code> contains the size of the table.
*//*==========================================================================*/

char **resSortTab;
int resSortTabSize = 0;

/*============================================================================*/
/*! @brief Capacitance "sort" table.

    This table contains the names of the capacitance "sorts" defined in the
    tecc file. Basically, this table works in a similar fashion as
    <code>resSortTab</code>.

    You can specify a capacitance sort by specifying a name after the keyword
    "capacitances" in the tecc file. If you omit the name, then the default
    name "cap" will be used.

    The size of the capacitance sort table is kept in the variable
    <code>capSortTabSize</code>.

    @see resSortTab
*//*==========================================================================*/

char **capSortTab;
int capSortTabSize = 0;

/*============================================================================*/
/*! @brief Capacitance "polarity" table.

    This table contains the polarities of the capacitance sorts defined in the
    array <code>capSortTab</code>. For each entry in <code>capSortTab</code>
    there is an entry in <code>capPolarityTab</code> that describes the
    polarity.

    Polarities are encoded as character literals. For example, the character
    'p' is used for p-type polarity. Similarly, 'n' is used for n-type polarity.
    The character 'x' is used for ordinary (linear) capacitances. The
    character '$' (through the define DSCAP_PREFIX) is used for drain/source
    capacitance.

    @see capSortTab
*//*==========================================================================*/

char *capPolarityTab;

dielectric_t * diels;
substrate_t * subcaps;
substrate_t * substrs;
selfsubdata_t * selfs;
mutualsubdata_t * muts;
int diel_cnt;
int subcap_cnt;
int substr_cnt;
int self_cnt;
int mut_cnt;
char ** maskdrawcolor;

int sBitmask, eBitmask, oBitmask;
int sBitmask2, sKeys, sKeys2;
int sBitmask2MAX;
int coded;

mask_t cNull;
mask_t filterBitmask;
mask_t resBitmask;
mask_t subBitmask;

double * conVal;
int * conNums;
int * conSort;
int * conductorMask;		/* maps conductor number to mask index */
bool_t * diffusionConductor;
bool_t * has_polnode;
bool_t extractNon3dAllways = FALSE;
bool_t extractDiffusionCap3d = FALSE;
bool_t omit_subcont = FALSE;
bool_t omit_ds_caps = FALSE;

int nrOfResizes;
resizeData_t * resizes;

char * usedTechFile = NULL;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void readTechFile (FILE *fp_tech);
Private int getnum (FILE *fp);
Private int getElemNumber (FILE *fp);
#ifdef __cplusplus
  }
#endif

#ifdef DRIVER
Private char *elTypeName (elemDef_t *el)
{
    switch (el->type) {
    case CONTELEM:	return ("CONTACT");
    case TORELEM:	return ("TORELEM");
    case VBJTELEM:	return ("VBJT_EL");
    case LBJTELEM:	return ("LBJT_EL");
    case LJUNELEM:	return ("LJUN_EL");
    case VJUNELEM:	return ("VJUN_EL");
    case PNCONELEM:	return ("CONNECT");
    case SURFCAP3DELEM: return ("SURFCAP3D");
    case EDGECAP3DELEM: return ("EDGECAP3D");
    case LATCAP3DELEM:  return ("LATCAP3D");
    case SURFCAPELEM:	return ("SURFCAP");
    case EDGECAPELEM:	return ("EDGECAP");
    case LATCAPELEM:	return ("LAT_CAP");
    case RESELEM:	return ("COND_EL");
    case VDIMELEM:	return ("VDIM_EL");
    case ESHAPEELEM:	return ("ESHAPE");
    case CSHAPEELEM:	return ("CSHAPE");
    case SUBCONTELEM:	return ("SUBCONT");
    case NEWMASKELEM:	return ("NEWMASK");
    }
    return ("UNKNOWN");
}
#endif

void techReadError (int n)
{
    say ("error in reading technology file (n = %d)", n);
    die ();
}

#ifndef MAKEDELA
void techCondError (int i, int cx)
{
    say ("error in technology file: element %s, incorrect conductor %d!",
	elemDefTab[i].name, cx);
    die ();
}

void techCondError2 (int i, int cx)
{
    say ("error in technology file: element %s, incorrect %s!",
	elemDefTab[i].name, cx == 0 ? "occurrence type" : "mask number");
    die ();
}
#endif /* MAKEDELA */

#ifndef DRIVER

void getTechnology (DM_PROJECT *dmproject, char *techDef, char *techFile)
{
    FILE *fp_tech;

    if (!usedTechFile) getTechFile (dmproject, techDef, techFile);
    fp_tech = cfopen (usedTechFile, "r");
    readTechFile (fp_tech);
    CLOSE (fp_tech);
}

#endif

#if !defined(MAKEDELA) && !defined(MAKESIZE)
char *keep_nodes = (char*)"";
char *keep_conductors = (char*)"";

Private int findname (char* name, int node)
{
    register char *p, *q;

    p = keep_nodes;
    if (!*p) return 0;
    do {
	while (*p == ' ' || *p == ',') ++p;
	q = name;
	while (*p == *q && *q) { ++p; ++q; }
	if (!*q && (!*p || *p == ' ' || *p == ',' || (*p == '.' && *(p+1) == node))) {
#ifdef DRIVER
	    fprintf (stderr, "keep_node '%c' of element '%s'!\n", node, name);
#endif
	    return 1;
	}
	while (*p && *p != ' ' && *p != ',') ++p;
    } while (*p);
    return 0;
}

Private int doLookup (char* name)
{
    register char *p, *q;

    p = keep_conductors;
    if (!*p) return 0;
    do {
	while (*p == ' ' || *p == ',') ++p;
	q = name;
	while (*p == *q && *q) { ++p; ++q; }
	if (!*q && (!*p || *p == ' ' || *p == ',')) {
#ifndef DRIVER
	    if (optVerbose)
#endif
	    fprintf (stderr, "keep_conductor element '%s'!\n", name);
	    return 1;
	}
	while (*p && *p != ' ' && *p != ',') ++p;
    } while (*p);
    return 0;
}
#endif /* !MAKEDELA && !MAKESIZE */

Private void readTechFile (FILE *fp_tech)
{
    char buf[1024];
    int c, cnt, i, k, t, extraCnd, maxKeys, maxKeys2;
    int maxEdgeEl, maxSurfEl;
    int elemTabSize, elemTabIndex, keyTabIndex;
    int elemTabSize2, elemTabIndex2;
    int majorPrNr, minorPrNr;
#ifdef MAKEDELA
    double tmp;
#else
    xy_list_t *begin_xy_list, *last_xy_list;
    xy_list_t *new_xy_list, *xy_list;
    xy_abp_t *new_xy_el, *last_xy_el;
    resizeCond_t *rCond;
    resElemDef_t *res;
    elemDef_t *el;
    bool_t debug_gettech;
#ifdef DRIVER
    int skipnr;
    int maxRECOG = 64;
    int tooMANY = 0;
#endif
    int stop = 0;
    int extraCount = 0;
    int extraCount2 = 0;
    int lcapELsk = 0;
    int capELsk  = 0;
    int subcELsk = 0;
    int vdimELsk = 0;
    int shapELsk = 0;
    int lcapELtt = 0;
    int capELtt  = 0;
    int resELtt  = 0;
    int resELsk  = 0;
    int subcELtt = 0;
    int vdimELtt = 0;
    int shapELtt = 0;
    int connELtt = 0;
    int connELsk = 0;
    int contELtt = 0;
    int contELsk = 0;
    int juncELtt = 0;
    int bjtELtt  = 0;
    int bjtELsk  = 0;
    int torELtt  = 0;
    int torELbeg = 0;
    int newELsk  = 0;
    int newELtt  = 0;
    int dscaps = 0;
    int do_bipo = 1;
#endif /* MAKEDELA */

    majorPrNr = minorPrNr = 0;

    if ((c = getc (fp_tech)) == '#') {
	fscanf (fp_tech, "%d %d", &majorPrNr, &minorPrNr);
    }
    if (majorPrNr < MajorProtNr) {
	say ("old technology file: use tecc to update your technology file!");
	die ();
    }
    if (majorPrNr > MajorProtNr || minorPrNr < 2 || minorPrNr > 5) {
	say ("incompatible version of technology file");
	die ();
    }

#if !defined(MAKEDELA) && !defined(MAKESIZE)
#ifdef DRIVER
    debug_gettech = 1;
#else
    debug_gettech = paramLookupB ("debug.gettech", "off");
    keep_nodes = paramLookupS ("keep_nodes", "");
    keep_conductors = paramLookupS ("keep_conductors", "");
    do_bipo = paramLookupB ("do_bipo", "on");
#endif
#endif /* !MAKEDELA && !MAKESIZE */

    fscanf (fp_tech, "%d", &coded);
    if (!(coded == 0 || coded == 2)) {
	say ("cannot read this coded technology file");
	die ();
    }
    fscanf (fp_tech, "%d", &t); /* number of color32 needed */
    setNcol (t);
    fscanf (fp_tech, "%d", &nrOfMasks);
#ifdef MAKEDELA
    if (getc (fp_tech) != '\n') techReadError (1);
#else
    /* The +1 below is necessary for addition of the
     * mesh_gln pseudo mask, which is used for the mesh refinement
     * for resistance extraction using the two-pass mechanism
     * or for hierarchical 3D capacitance extraction.
     */
    masktable = NEW (maskinfo_t, nrOfMasks + 1);
#ifndef MAKESIZE
    maskdrawcolor = NEW (char *, nrOfMasks);
#endif
#endif /* MAKEDELA */
    cnt = -1;
    for (i = 0; i < nrOfMasks; i++) {
#ifdef MAKEDELA
	while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
#else
	fscanf (fp_tech, "%s %d", buf, &k);
	if (k) { /* conductor */
	         if (k == 1) ++nrOfCondStd;
	    else if (k == 2) ++nrOfCondPos;
	    else if (k == 3) ++nrOfCondNeg;
	}
	masktable[i].name = strsave (buf);
	fscanf (fp_tech, "%s %d", buf, &k);
	if (k > cnt) cnt = k;
	masktable[i].conductor = k;
	initcolorint (&masktable[i].color, buf);

	fscanf (fp_tech, "%d %d", &k, &t);
	masktable[i].gln = k;
	masktable[i].terminal = (t == DM_INTCON_MASK);

	/* read mask draw color */
	k = 0;
	while ((c = getc (fp_tech)) != '\n') {
	    if (c == EOF) techReadError (1);
	    if (c > ' ' && k < 99) buf[k++] = c;
	}
	buf[k] = '\0';
#ifndef MAKESIZE
	if (!k)
	    maskdrawcolor[i] = strsave ("glass");
	else
	    maskdrawcolor[i] = strsave (buf);
#endif
#endif /* MAKEDELA */
    }
#ifdef DRIVER
if (prTable) {
    mask_t c, sum = cNull;
    fprintf (stderr, "\n=====================================================================\n");
    fprintf (stderr, "%-31s color    cond. color %16s\n", "MASKS:", "first");
    fprintf (stderr, "nr: %-27s name:     nr#  bit-present-mask (bit)\n", "name:");
    for (i = 0; i < nrOfMasks; i++) {
	k = 0;
	if (IS_COLOR (&masktable[i].color)) {
	    while (k < 64) {
		COLORINITINDEX (c, k); ++k;
		if (!COLOR_ABSENT (&masktable[i].color, &c)) break;
	    }
	}
	fprintf (stderr, "%3d %-27s %-10.10s %2d  %s (%2d)\n", i, masktable[i].name, maskdrawcolor[i],
	    masktable[i].conductor, colorHexStr (&masktable[i].color), k);
	COLOR_ADD (sum, masktable[i].color);
    }
    fprintf (stderr, "%46s %s\n", "sum:", colorHexStr (&sum));
}
#endif

    fscanf (fp_tech, "%d", &nrOfConductors);

#if !defined(MAKEDELA) && !defined(MAKESIZE)
    nrOfConductors -= 1;
    if (cnt != nrOfConductors) {
	say ("error in technology file: incorrect conductor count!");
	die ();
    }
    ASSERT (strsame (masktable[nrOfMasks - 1].name, "@sub"));
    ASSERT (masktable[nrOfMasks - 1].conductor == nrOfConductors);

    ASSERT (nrOfCondStd + nrOfCondPos + nrOfCondNeg == nrOfConductors);
    nrOfCondPos += nrOfCondStd;
    if (nrOfConductors > nrOfCondStd) {
	masktable[nrOfMasks - 1].conductor = nrOfCondStd;
    }

#ifdef CAP3D
    /* This is an overestimation, since we can have
     * flat conductors for which we need only one level.
     */
    nrOfSpiderLevels = 2 * nrOfCondStd;
    displaceList = NEW (int, nrOfSpiderLevels);
    for (i = 0; i < nrOfSpiderLevels; ++i) displaceList[i] = 0;
#endif

    /*
     * The array conductorMask maps conductor numbers to mask indices.
     * This is used to find the mask of a conductor when generating
     * a coordinate pair + mask info for a net.
     *
     * The array conductorMask maps also conductor numbers to colors.
     * This is used to find the color of a spider when
     * animating 3d cap extraction.
     * Note: Separate mask and conductor fields in elemdefs would
     *       maybe not be neccessary.
     *
     * The diffusionConductor array will contain TRUE
     * for drain/source conductors.
     */

    conductorMask      = NEW (int   , nrOfCondStd+1);
    diffusionConductor = NEW (bool_t, nrOfCondStd+1);
    has_polnode        = NEW (bool_t, nrOfCondStd+1);

    for (i = 0; i <= nrOfCondStd; ++i) conductorMask[i] = -1;

    for (i = 0; i < nrOfMasks; i++) {
	if ((k = masktable[i].conductor) >= 0 && k <= nrOfCondStd) conductorMask[k] = i;
    }

    for (i = 0; i <= nrOfCondStd; ++i) {
	diffusionConductor[i] = FALSE;
	has_polnode[i] = FALSE;
	if (conductorMask[i] == -1) {
	    say ("error in technology file: incorrect conductor number!");
	    die ();
	}
    }
#endif /* !MAKEDELA && !MAKESIZE */

    /* keylist */
    fscanf (fp_tech, "%d %d %d", &t, &sKeys, &oBitmask);
#if !defined(MAKEDELA) && !defined(MAKESIZE)
    if (t < 0 || sKeys < 0 || oBitmask < 0) techReadError (2);
    maxKeys = sKeys + oBitmask;
    if (maxKeys > 30) techReadError (3); /* too many keys */
    maxKeys = (1 << maxKeys);
#endif

    /* keylist2 */
    fscanf (fp_tech, "%d %d %d", &t, &sKeys2, &eBitmask);
#if !defined(MAKEDELA) && !defined(MAKESIZE)
    if (t < 0 || sKeys2 < 0 || eBitmask < 0) techReadError (4);
    maxKeys2 = sKeys2 + eBitmask;
    if (maxKeys2 > 30) techReadError (5); /* too many keys */
    maxKeys2 = (1 << maxKeys2);

    if ((k = t - sKeys) < 0) techReadError (6);
    if (k == 0 || oBitmask == 0) { /* no separate oBitmask needed */
	sKeys += oBitmask;
	oBitmask = 0;
	sBitmask = ~(~0 << sKeys);
    }
    else { /* k > 0 */
	sBitmask = ~(~0 << sKeys);
	oBitmask = ~(~0 << oBitmask);
	oBitmask <<= sKeys;
	sKeys = k;
    }
    if (t > 31) sBitmask2MAX = ~0;
    else sBitmask2MAX = ~(~0 << t);
    sBitmask2 = ~(~0 << sKeys2);
    if (eBitmask > 0) eBitmask = ~(~0 << eBitmask);
#endif /* !MAKEDELA && !MAKESIZE */

    fscanf (fp_tech, "%s", buf);
#if !defined(MAKEDELA) && !defined(MAKESIZE)
    initcolorint (&filterBitmask, buf);
#endif

    fscanf (fp_tech, "%s", buf);
#if !defined(MAKEDELA) && !defined(MAKESIZE)
    /* If additional substrate-masks are defined,
     * then the variable hasSubmask must be set.
     */
    initcolorint (&subBitmask, buf);
    if (IS_COLOR (&subBitmask)) hasSubmask = 1;
#endif

    /* Read resize information */

    nrOfResizes = getnum (fp_tech);
#ifndef MAKEDELA
    resizes = NEW (resizeData_t, nrOfResizes);
#endif
    for (i = 0; i < nrOfResizes; i++) {
#ifdef MAKEDELA
	while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
#else
	if (fscanf (fp_tech, "%s %d %lg %d",
	    resizes[i].newmaskname, &resizes[i].id,
	    &resizes[i].val, &cnt) != 4) techReadError (7);
	resizes[i].condcnt = cnt;
	resizes[i].cond = NEW (resizeCond_t, 1);
	rCond = resizes[i].cond;
	for (k = 0; k < cnt; k++) {
	    if (fscanf (fp_tech, "%s", buf) != 1) techReadError (8);
	    initcolorint (&rCond -> present, buf);
	    if (fscanf (fp_tech, "%s", buf) != 1) techReadError (9);
	    initcolorint (&rCond -> absent, buf);
	    if (k+1 < cnt) {
		rCond -> next = NEW (resizeCond_t, 1);
		rCond = rCond -> next;
	    } else
		rCond -> next = NULL;
	}
#endif /* MAKEDELA */
    }

#ifndef MAKESIZE

    fscanf (fp_tech, "%d", &resSortTabSize);
    if (resSortTabSize > 0) {
#ifndef MAKEDELA
	resSortTab = NEW (char *, resSortTabSize);
#endif
	for (i = 0; i < resSortTabSize; i++) {
	    fscanf (fp_tech, "%s", buf);
#ifndef MAKEDELA
	    resSortTab[i] = strsave (buf);
#endif
	}
    }
    else techReadError (10);

    fscanf (fp_tech, "%d", &capSortTabSize);
    if (capSortTabSize > 0) {
#ifndef MAKEDELA
	capSortTab = NEW (char *, capSortTabSize);
	capPolarityTab = NEW (char, capSortTabSize);
#endif
	for (i = 0; i < capSortTabSize; i++) {
	    fscanf (fp_tech, "%s", buf);
#ifndef MAKEDELA
	    if (i > 1 && strsame (buf, capSortTab[i-1])) {
		capPolarityTab[i-1] = 'p';
		capPolarityTab[i] = 'n';
		capSortTab[i] = capSortTab[i-1];
	    }
	    else {
		capPolarityTab[i] = 'x';
		capSortTab[i] = strsave (buf);
	    }
#endif /* MAKEDELA */
	}
    }
    else techReadError (11);

#ifndef MAKEDELA
    begin_xy_list = last_xy_list = NULL;
    last_xy_el = NULL;
#endif

    fscanf (fp_tech, "%d", &cnt); /* nr of values */
    while (cnt > 0) {
#ifndef MAKEDELA
	new_xy_list = NEW (xy_list_t, 1);
	new_xy_list -> list = NULL;
	new_xy_list -> next = NULL;
	if (begin_xy_list == NULL)
	    begin_xy_list = new_xy_list;
	else
	    last_xy_list -> next = new_xy_list;
	last_xy_list = new_xy_list;
#endif
	do {
#ifdef MAKEDELA
	    fscanf (fp_tech, "%le %le %le %le %le", &tmp, &tmp, &tmp, &tmp, &tmp);
#else
	    new_xy_el = NEW (xy_abp_t, 1);
	    new_xy_el -> next = NULL;
	    fscanf (fp_tech, "%le %le %le %le %le",
		&new_xy_el -> x, &new_xy_el -> y,
		&new_xy_el -> a, &new_xy_el -> b, &new_xy_el -> p);
	    if (last_xy_list -> list == NULL)
		last_xy_list -> list = new_xy_el;
	    else
		last_xy_el -> next = new_xy_el;
	    last_xy_el = new_xy_el;
#endif /* MAKEDELA */
	} while (--cnt > 0);
	fscanf (fp_tech, "%d", &cnt);
    }

    if (minorPrNr > 4) {
	/* this version cannot use cap/res-filters */
	cnt = getnum (fp_tech);
	while (cnt-- > 0) { /* skip filter line */
	    while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
	}
    }

    elemDefTabSize = getnum (fp_tech);
#ifndef MAKEDELA
    elemDefTab = NEW (elemDef_t, elemDefTabSize);
#endif /* MAKEDELA */

    for (i = 0; i < elemDefTabSize; i++) {
#ifdef MAKEDELA
	while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
#else
	el = &elemDefTab[i];
	fscanf (fp_tech, "%s", buf);
	buf[MAX_ELEM_NAME] = 0;
	strcpy (el -> name, buf);
	fscanf (fp_tech, "%d %d %s", &el -> id, &el -> cond_cnt, buf);
	initcolorint (&el -> sBitPresent, buf);
	fscanf (fp_tech, "%s", buf);
	initcolorint (&el -> sBitAbsent, buf);
	fscanf (fp_tech, "%s", buf);
	initcolorint (&el -> eBitPresent, buf);
	fscanf (fp_tech, "%s %d", buf, &el -> type);
	initcolorint (&el -> eBitAbsent, buf);

	el -> el_recog_cnt = 0;

	switch (el -> type) {
	    case CONTELEM:
	    {
		contElemDef_t *cont = &el -> s.cont;
		++contELtt;
		fscanf (fp_tech, "%d %d %d %le", &cont -> sortNr,
			&cont -> mask1, &cont -> mask2, &cont -> val);
		ASSERT (cont -> sortNr >= 0);
		t = 0;
		if (cont -> mask1 < 0) {
		    t = 1;
		    Swap (int, cont -> mask1, cont -> mask2);
		}

		if (cont -> mask1 >= 0) {
		    cont -> con1 = masktable[cont -> mask1].conductor;
		    if (cont -> con1 < 0) techCondError (i, cont -> con1);
		    if (cont -> con1 >= nrOfCondStd) {
			++contELsk; el -> type = SKIP_ELEM; break;
		    }
		}
		else techCondError (i, cont -> mask1);

		if (cont -> mask2 >= 0) {
		    cont -> con2 = masktable[cont -> mask2].conductor;
		    if (cont -> con2 < 0) techCondError (i, cont -> con2);
		    if (cont -> con2 >= nrOfCondStd) {
			++contELsk; el -> type = SKIP_ELEM; break;
		    }
		}
		else {
		    cont -> con2 = k = cont -> mask2;
		    if (k != -2 && k != -4) techCondError (i, k);
		}

		if (!optIntRes || cont -> val <= low_contact_res) cont -> val = 0; // low res

		cont -> keep1 = findname (el -> name, t? '2' : '1');
		cont -> keep2 = findname (el -> name, t? '1' : '2');
		break;
	    }
	    case TORELEM:
	    {
		torElemDef_t *tor = &el -> s.tor;
		if (++torELtt == 1) torELbeg = i;
		if (strlen (el -> name) > MAX_TORNAME) {
		    el -> name[MAX_TORNAME] = 0;
		    say ("warning: device name truncated to '%s'", el -> name);
		}

		if (minorPrNr > 3) {
		    fscanf (fp_tech, "%d %d %d %d",
			&tor -> gMask, &tor -> dsMask, &tor -> sMask, &tor -> bMask);
		}
		else {
		    fscanf (fp_tech, "%d %d %d",
			&tor -> gMask, &tor -> dsMask, &tor -> bMask);
		    tor -> sMask = tor -> dsMask;
		}
		tor -> gCon = masktable[tor -> gMask].conductor;
		if (tor -> gCon < 0) techCondError (i, tor -> gCon);
		tor -> dsCon = masktable[tor -> dsMask].conductor;
		if (tor -> dsCon < 0) techCondError (i, tor -> dsCon);
		tor -> sCon = masktable[tor -> sMask].conductor;
		if (tor -> sCon < 0) techCondError (i, tor -> sCon);
		if (tor -> bMask >= 0) {
		    tor -> bCon = masktable[tor -> bMask].conductor;
		    if (tor -> bCon < 0) techCondError (i, tor -> bCon);
		}
		else {
		    tor -> bCon = k = tor -> bMask;
		/* If bMask/bCon == -1 : no bulk terminal,
		   if bMask/bCon == -2 || -4 : sub. */
		    if (k == -3 || k < -4) techCondError (i, k);
		}

		tor -> dsCap = 0;
		if (dscaps) {
		    for (k = 1; k < capSortTabSize; ++k) {
			if (*capSortTab[k] == DSCAP_PREFIX && capSortTab[k] != capSortTab[k-1]) {
			    sprintf (buf, "%c%s%s", DSCAP_PREFIX, el -> name, DSCAP_SUFFIX);
			    if (strsame (buf, capSortTab[k])) {
				tor -> dsCap = 1;
				has_polnode[tor -> dsCon] = TRUE;
				has_polnode[tor -> sCon] = TRUE;
				break;
			    }
			}
		    }
		}
		break;
	    }
	    case VBJTELEM:
	    case LBJTELEM:
	    {
		bjtorElemDef_t *bjt = &el -> s.bjt;
		++bjtELtt;
		if (strlen (el -> name) > MAX_TORNAME) {
		    el -> name[MAX_TORNAME] = 0;
		    say ("warning: device name truncated to '%s'", el -> name);
		}
		fscanf (fp_tech, "%d %d %d %d %d %d %d",
			&bjt -> cMask, &bjt -> cOccurrence,
			&bjt -> bMask, &bjt -> bOccurrence,
			&bjt -> eMask, &bjt -> eOccurrence, &bjt -> sMask);
		fscanf (fp_tech, "%s", buf);
		initcolorint (&bjt -> oBitPresent, buf);
		fscanf (fp_tech, "%s", buf);
		initcolorint (&bjt -> oBitAbsent, buf);

		if (!do_bipo) {
		    ++bjtELsk;
		    el -> type = SKIP_ELEM;
		    break;
		}

		bjt -> cCon = masktable[bjt -> cMask].conductor;
		if (bjt -> cCon < 0) techCondError (i, bjt -> cCon);
		has_polnode[bjt -> cCon] = TRUE;
		bjt -> bCon = masktable[bjt -> bMask].conductor;
		if (bjt -> bCon < 0) techCondError (i, bjt -> bCon);
		has_polnode[bjt -> bCon] = TRUE;
		bjt -> eCon = masktable[bjt -> eMask].conductor;
		if (bjt -> eCon < 0) techCondError (i, bjt -> eCon);
		has_polnode[bjt -> eCon] = TRUE;
		if (bjt -> sMask >= 0) {
		    bjt -> sCon = masktable[bjt -> sMask].conductor;
		    if (bjt -> sCon < 0) techCondError (i, bjt -> sCon);
		    has_polnode[bjt -> sCon] = TRUE;
		}
		else {
		    bjt -> sCon = k = bjt -> sMask;
			/* If sCon == -1 : no substrate terminal,
			   if sCon == -2 || -4 : substrate */
		    if (k == -2 || k == -4) hasBipoSub = 1;
		    else if (k != -1) techCondError (i, k);
		}
		if (el -> type == LBJTELEM) hasBipoELEM |= 1;
		else hasBipoELEM |= 2; /* VBJTELEM */
		break;
	    }
	    case LJUNELEM:
	    case VJUNELEM:
	    {
		junElemDef_t *jun = &el -> s.jun;
		++juncELtt;
		fscanf (fp_tech, "%d %d %d %d",
			&jun -> pMask, &jun -> pOccurrence,
			&jun -> nMask, &jun -> nOccurrence);
		fscanf (fp_tech, "%le %le %le",
			&jun -> voltage, &jun -> cap, &jun -> depth);

		el -> type = SKIP_ELEM;
		say ("junction element %s not supported!", el -> name);
		break;
	    }
	    case PNCONELEM:
	    {
		pnconElemDef_t *pnc = &el -> s.pnc;
		++connELtt;
		fscanf (fp_tech, "%d %d %d %d %le",
			&pnc -> mask1, &pnc -> occurrence1,
			&pnc -> mask2, &pnc -> occurrence2, &pnc -> val);

		pnc -> con1 = masktable[pnc -> mask1].conductor;
		if (pnc -> con1 < 0) techCondError (i, pnc -> con1);
		pnc -> con2 = masktable[pnc -> mask2].conductor;
		if (pnc -> con2 < 0) techCondError (i, pnc -> con2);
		if (pnc -> con1 >= nrOfCondStd || pnc -> con2 >= nrOfCondStd) {
		    ++connELsk; el -> type = SKIP_ELEM;
		    break;
		}
		if (pnc -> occurrence1 == SURFACE
		 && pnc -> occurrence2 == SURFACE) hasSurfConnect = 1;
		else { hasEdgeConnect = 1;
		    if (pnc -> occurrence1 != EDGE) {
			Swap (int, pnc -> con1, pnc -> con2);
			Swap (int, pnc -> mask1, pnc -> mask2);
			Swap (int, pnc -> occurrence1, pnc -> occurrence2);
		    }
		    if (pnc -> occurrence1 != EDGE ||
			pnc -> occurrence2 != SURFACE) techCondError2 (i, 0);
		}
		break;
	    }
	    case SURFCAP3DELEM:
	    case EDGECAP3DELEM:
	    case LATCAP3DELEM:
	    case SURFCAPELEM:
	    case EDGECAPELEM:
	    case LATCAPELEM:
	    {
		capElemDef_t *cap = &el -> s.cap;
		int xy_index;
		int swap = 0;

		fscanf (fp_tech, "%d %d %d %d %d", &cap -> sortNr,
			&cap -> pMask, &cap -> pOccurrence,
			&cap -> nMask, &cap -> nOccurrence);
		cap -> sBitPresent = el -> sBitPresent;
		cap -> sBitAbsent  = el -> sBitAbsent;
		ASSERT (cap -> sortNr >= 0);

		if (el -> type == LATCAPELEM || el -> type == LATCAP3DELEM) {
		    fscanf (fp_tech, "%s", buf);
		    initcolorint (&cap -> oBitPresent, buf);
		    fscanf (fp_tech, "%s", buf);
		    initcolorint (&cap -> oBitAbsent, buf);
		    if (cap -> pOccurrence == EDGE && cap -> nOccurrence == OTHEREDGE) /* ok */;
		    else
		    if (cap -> nOccurrence == EDGE && cap -> pOccurrence == OTHEREDGE) swap = 1;
		    else techCondError2 (i, 0);
		    ++lcapELtt;
		}
		else if (el -> type == EDGECAPELEM || el -> type == EDGECAP3DELEM) {
		    if (cap -> pOccurrence == EDGE && cap -> nOccurrence != OTHEREDGE) /* ok */;
		    else
		    if (cap -> nOccurrence == EDGE && cap -> pOccurrence != OTHEREDGE) swap = 1;
		    else techCondError2 (i, 0);
		    ++capELtt;
		}
		else { /* SURFCAPELEM */
		    if (cap -> pOccurrence == SURFACE && cap -> nOccurrence == SURFACE) /* ok */;
		    else techCondError2 (i, 0);
		    if (cap -> pMask < 0) swap = 1;
		    ++capELtt;
		}
		fscanf (fp_tech, "%le %d", &cap -> val, &xy_index);

		if (swap) {
		    Swap (int, cap -> pMask, cap -> nMask);
		    Swap (int, cap -> pOccurrence, cap -> nOccurrence);
		    if (capPolarityTab[cap -> sortNr] == 'p') cap -> sortNr += 1;
		}

		if (cap -> pMask >= 0) {
		    if (cap -> pMask >= nrOfMasks) techCondError2 (i, 1);
		    cap -> pCon = masktable[cap -> pMask].conductor;
		    if (cap -> pCon < 0) techCondError (i, cap -> pCon);
		    cap -> pKeep = findname (el -> name, swap? '2' : '1');
		}
		else techCondError (i, cap -> pMask);

		if (cap -> nMask >= 0) {
		    if (cap -> nMask >= nrOfMasks) techCondError2 (i, 1);
		    cap -> nCon = masktable[cap -> nMask].conductor;
		    if (cap -> nCon < 0) techCondError (i, cap -> nCon);
		    cap -> nKeep = findname (el -> name, swap? '1' : '2');
		}
		else {
		    cap -> nCon = k = cap -> nMask;
		    if (k == -1) cap -> nKeep = 1;
		    else if ((k == -2 && el -> type != SURFCAPELEM) || k == -4)
			cap -> nKeep = findname (el -> name, swap? '1' : '2');
		    else techCondError (i, k);
		}
		cap -> done = 0;
		cap -> dsCap = 0;

		if (xy_index >= 0) {
		    xy_list = begin_xy_list;
		    while (xy_index > 0) {
			xy_list = xy_list -> next;
			xy_index--;
		    }
		    cap -> mval = xy_list -> list;
		    cap -> val = cap -> mval -> a;
		}
		else
		    cap -> mval = NULL;

		if (el -> type == SURFCAP3DELEM) {
		    if (!optCap || (!optCap3D && *el -> name == '$')) goto skip_cap;
		    if (!optCap3D) el -> type = SURFCAPELEM;
		    hasSurfCaps = 1;
		}
		else if (el -> type == EDGECAP3DELEM) {
		    if (!optCap) goto skip_cap;
		    if (!optCap3D) el -> type = EDGECAPELEM;
		    hasEdgeCaps = 1;
		}
		else if (el -> type == LATCAP3DELEM) {
		    if (!optLatCap) goto skip_cap;
		    if (!optCap3D) el -> type = LATCAPELEM;
		    hasEdgeCaps = 1;
		}
		else {
		    if (el -> type == LATCAPELEM && !optLatCap) goto skip_cap;
		    if (*capSortTab[cap -> sortNr] == DSCAP_PREFIX) {
			if (omit_ds_caps) goto skip_cap;
			dscaps = 1;
			cap -> dsCap = 1;
		    }
		    else if (!optCap || cap -> pCon >= nrOfCondStd || cap -> nCon >= nrOfCondStd) {
skip_cap:
			if (el -> type == LATCAPELEM || el -> type == LATCAP3DELEM) ++lcapELsk;
			else ++capELsk;
			el -> type = SKIP_ELEM;
			break;
		    }
		    if (el -> type == SURFCAPELEM) hasSurfCaps = 1;
		    else hasEdgeCaps = 1;
		}
		break;
	    }
	    case RESELEM:
	    {
		double height, thickness;
		++resELtt;
		res = &el -> s.res;

		/* If techfile is suitable for bipolar extraction, also get
		 * the type (t) of resistance ('n' or 'p'), else do not.
		 */
		fscanf (fp_tech, "%d %d %d %le %le %le",
			&res -> sortNr, &res -> mask, &t, &res -> val, &height, &thickness);
		ASSERT (res -> sortNr >= 0);

		res -> con = masktable[res -> mask].conductor;
		if (res -> con < 0) techCondError (i, res -> con);
		if (res -> con >= nrOfCondStd) {
		    ++resELsk; el -> type = SKIP_ELEM; break;
		}
		else {
#ifndef DRIVER
		    if (!optIntRes || res -> val <= low_sheet_res) res -> val = 0; // low res
		    else { res -> val = 1 / res -> val; // high res
			if (optResMesh == 2) {
			    COLOR_ADD (resBitmask, masktable[res -> mask].color);
			}
			else if (optResMesh == 1 && !stop) {
			    if (!IS_COLOR (&el -> sBitPresent)) { resBitmask = filterBitmask; stop = 1; }
			    else COLOR_ADD (resBitmask, masktable[res -> mask].color);
			}
		    }
#endif
		}
		if (t) {
		    for (k = 0; k < torELtt; ++k) {
			if (res -> con == elemDefTab[k+torELbeg].s.tor.gCon) {
say ("warning: conductor %s has incorrect type: is used as gate, cannot be diffusion conductor", el -> name);
			    res -> type = 'a'; t = 0;
			    break;
			}
		    }
		}
		if (t) diffusionConductor[res -> con] = TRUE; /*!!!3D*/
		res -> type = (t == 0 ? 'a' : (t == 1 ? 'n' : 'p'));
		res -> rKeep = doLookup (el -> name);
		break;
	    }
	    case VDIMELEM:
	    {
		vDimElemDef_t *vdim = &el -> s.vdim;
		++vdimELtt;
		fscanf (fp_tech, "%d %le %le", &vdim -> mask, &vdim -> height, &vdim -> thickness);

		vdim -> con = masktable[vdim -> mask].conductor;
		if (vdim -> con < 0) techCondError (i, vdim -> con);
		vdimZtop = Max (vdimZtop, vdim -> height + vdim -> thickness);
		if (++vdimCount == 1) vdimIndex = i;
		if (!optCap3D) {
		    ++vdimELsk; el -> type = SKIP_ELEM;
		}
		break;
	    }
	    case ESHAPEELEM:
	    case CSHAPEELEM:
	    {
		shapeElemDef_t *sh = &el -> s.shape;
		++shapELtt;
		fscanf (fp_tech, "%d %le %le %le %le", &sh -> mask,
			&sh -> xb1, &sh -> xt1, &sh -> xb2, &sh -> xt2);

		sh -> con = masktable[sh -> mask].conductor;
		if (sh -> con < 0) techCondError (i, sh -> con);
		if (!optCap3D) {
		    ++shapELsk; el -> type = SKIP_ELEM;
		}
		else hasShapeElems = 1;
		break;
	    }
	    case SUBCONTELEM:
	    {
		subcontElemDef_t *sc = &el -> s.subc;
		++subcELtt;
		fscanf (fp_tech, "%d %d", &t, &sc -> captype);
		if (t >= 0) { /* mask## */
		    t = masktable[t].conductor; /* cond## */
		    if (t < 0) techCondError (i, t);
		    if (sc -> captype && t < nrOfCondStd && diffusionConductor[t])
			sc -> captype = 2; /* diffusionCap */
		}
		sc -> ccnr = t; /* cond## */
		if (!substrRes || t >= nrOfCondStd) {
		    ++subcELsk;
		    el -> type = SKIP_ELEM;
		}
		else if (sc -> captype && omit_subcont) {
		    if (!optCap || (optCap3D && !extractNon3dAllways
		    && (extractDiffusionCap3d || sc -> captype != 2))) {
			++subcELsk;
			el -> type = SKIP_ELEM;
		    }
		}
		break;
	    }
	    case NEWMASKELEM:
	    {
		newElemDef_t *newMaskElem = &el -> s.newmask;
		++newELtt;
		fscanf (fp_tech, "%d", &newMaskElem -> mask);
		if (newMaskElem -> mask < 0 || newMaskElem -> mask >= nrOfMasks) techReadError (12);
#ifndef CONFIG_XSPACE
		++newELsk;
		el -> type = SKIP_ELEM;
#endif
		break;
	    }
	    default:
		say ("error in technology file: unknown element found!");
		die ();
         }
#endif /* MAKEDELA */
    }
#ifdef CONFIG_XSPACE
    return;
#endif

#ifndef MAKEDELA
    if (optCap3D && (hasSurfCaps || hasEdgeCaps)) {
	hasSurfCaps = hasEdgeCaps = 0;
	for (i = 0; i < elemDefTabSize; i++) {
	    el = &elemDefTab[i];
	    switch (el -> type) {
	    case SURFCAP3DELEM:
		if (*el -> name != '$') el -> type = SURFCAPELEM;
#ifdef DRIVER
fprintf (stderr, "cap3d: NOT SKIPPED el=%s (%s)\n", el->name, el -> type == SURFCAPELEM ? "keep2d" : "omit3d");
#endif
		hasSurfCaps = 1;
		break;
	    case EDGECAP3DELEM:
	    case LATCAP3DELEM:
		el -> type = el -> type == LATCAP3DELEM ? LATCAPELEM : EDGECAPELEM;
#ifdef DRIVER
fprintf (stderr, "cap3d: NOT SKIPPED el=%s (keep2d)\n", el->name);
#endif
		hasEdgeCaps = 1;
		break;
	    case SURFCAPELEM:
	    case EDGECAPELEM:
	    case LATCAPELEM:
		if (!extractNon3dAllways && (extractDiffusionCap3d || !diffusionCap (&el -> s.cap))) {
		    if (el -> type == LATCAPELEM) ++lcapELsk;
		    else ++capELsk;
		    el -> type = SKIP_ELEM;
#ifdef DRIVER
fprintf (stderr, "cap3d: SKIPPING el=%s\n", el->name);
#endif
		}
		else {
		    if (el -> type == SURFCAPELEM) hasSurfCaps = 1;
		    else hasEdgeCaps = 1;
#ifdef DRIVER
fprintf (stderr, "cap3d: NOT SKIPPED el=%s (%s)\n", el->name, extractNon3dAllways ? "allways2d" : "diffusionCap2d");
#endif
		}
	    }
	}
    }
#endif /* MAKEDELA */

#ifdef DRIVER
if (prTable) {
    mask_t c, sum;
    int cnt0, cnt1, cnt2, cnt3, cnt4;

    sum = cNull;
    cnt0 = cnt1 = cnt2 = cnt3 = cnt4 = 0;
    fprintf (stderr, "=====================================================================\n");
    fprintf (stderr, "%-32s element extra %-15s  first\n", "EDGE ELEMENTS:", "edge");
    fprintf (stderr, "nr: %-28s type:   cond# bit-present-mask (bit)\n", "name:");
    for (t = i = 0; i < elemDefTabSize; ++i) {
	el = elemDefTab+i;
	if (el -> type != SKIP_ELEM) {
	    if (IS_COLOR (&el -> eBitPresent) || IS_COLOR (&el -> eBitAbsent)) {
		k = 0;
		if (IS_COLOR (&el -> eBitPresent)) {
		    while (k <= nrOfConductors) {
			COLORINITINDEX (c, k); ++k;
			if (!COLOR_ABSENT (&el -> eBitPresent, &c)) break;
		    }
		}
		extraCnd = 0;
		if (el -> cond_cnt & 1) ++extraCnd;
		if (el -> cond_cnt & 2) ++extraCnd;
		if (el -> cond_cnt & 4) ++extraCnd;
		if (el -> cond_cnt & 8) ++extraCnd;
		switch (extraCnd) {
		case 0: ++cnt0; break;
		case 1: ++cnt1; break;
		case 2: ++cnt2; break;
		case 3: ++cnt3; break;
		case 4: ++cnt4; break;
		}
		fprintf (stderr, "%3d %-28s %-10s %d  %s (%2d)\n", ++t, el -> name, elTypeName (el), extraCnd, colorHexStr (&el -> eBitPresent), k);
		COLOR_ADD (sum, el -> eBitPresent);
		COLOR_ADD (sum, el -> eBitAbsent);
		COLOR_ADD (sum, el -> sBitPresent);
		COLOR_ADD (sum, el -> sBitAbsent);
	    }
	}
    }
    sprintf (buf, "no. of extra cond: 0=%d 1=%d 2=%d 3=%d 4=%d", cnt0, cnt1, cnt2, cnt3, cnt4);
    fprintf (stderr, "%-46s %s\n", buf, colorHexStr (&sum));

    sum = cNull;
    cnt0 = cnt1 = cnt2 = 0;
    fprintf (stderr, "=====================================================================\n");
    fprintf (stderr, "%-32s element extra %-15s  first\n", "SURFACE ELEMENTS:", "surface");
    fprintf (stderr, "nr: %-28s type:   cond# bit-present-mask (bit)\n", "name:");
    for (t = i = 0; i < elemDefTabSize; ++i) {
	el = elemDefTab+i;
	if (el -> type != SKIP_ELEM) {
	    if (!IS_COLOR (&el -> eBitPresent) && !IS_COLOR (&el -> eBitAbsent)) {
		k = 0;
		if (IS_COLOR (&el -> sBitPresent)) {
		    while (k <= nrOfConductors) {
			COLORINITINDEX (c, k); ++k;
			if (!COLOR_ABSENT (&el -> sBitPresent, &c)) break;
		    }
		}
		extraCnd = 0;
		if (el -> cond_cnt & 1) ++extraCnd;
		if (el -> cond_cnt & 2) ++extraCnd;
		switch (extraCnd) {
		case 0: ++cnt0; break;
		case 1: ++cnt1; break;
		case 2: ++cnt2; break;
		}
		if (el -> type == RESELEM)
		    fprintf (stderr, "%3d %-28s %.4s #%2d   %d  %s (%2d)\n", ++t, el -> name, elTypeName (el),
			el -> s.res.con, extraCnd, colorHexStr (&el -> sBitPresent), k);
		else
		    fprintf (stderr, "%3d %-28s %-10s %d  %s (%2d)\n", ++t, el -> name, elTypeName (el), extraCnd, colorHexStr (&el -> sBitPresent), k);
		COLOR_ADD (sum, el -> sBitPresent);
		COLOR_ADD (sum, el -> sBitAbsent);
	    }
	}
    }
    sprintf (buf, "no. of extra cond: 0=%d 1=%d 2=%d", cnt0, cnt1, cnt2);
    fprintf (stderr, "%-46s %s\n", buf, colorHexStr (&sum));
    fprintf (stderr, "=====================================================================\n");
}
#endif

#ifndef MAKEDELA
    conVal = NEW (double, nrOfCondStd);
    conNums = NEW (int, nrOfCondStd);
    conSort = NEW (int, nrOfCondStd);

    maxEdgeEl = maxSurfEl = 1;
#endif /* MAKEDELA */

    /*----------- reading Surface Elements keytab ---------------*/

    keyTabSize = getnum (fp_tech);
#ifndef MAKEDELA
    if (keyTabSize != maxKeys) techReadError (13);
#endif

    elemTabSize = getnum (fp_tech) + 1;
#ifndef MAKEDELA
    keyTab  = NEW (int, keyTabSize);
    elemTab = NEW (elemDef_t *, elemTabSize);
    elemTabIndex = 0;
    elemTab[elemTabIndex++] = NULL;
#endif

    extraCnd = 0;
    for (keyTabIndex = 0; keyTabIndex < keyTabSize; keyTabIndex++) {
#ifdef MAKEDELA
	while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
#else
	k = elemTabIndex;
#ifdef DRIVER
	skipnr = 0;
#endif
	while ((i = getElemNumber (fp_tech)) >= 0) {
	    el = &elemDefTab[i];
	    if (el -> type != SKIP_ELEM) {
		elemTab[elemTabIndex++] = el;
		if (el -> cond_cnt) ++extraCnd;
	    }
#ifdef DRIVER
	    else ++skipnr;
#endif
	}
	if (i == ELEM_IDEM) { /* The elements for this key were already seen
				for another key. Copy the value in keyTab */
	    i = getnum (fp_tech);
	    ASSERT (i >= 0 && i < keyTabIndex);
	    keyTab[keyTabIndex] = keyTab[i];
	    if (keyTab[i] < 0) ++extraCount;
	}
	else if (elemTabIndex == k) { /* no elements in slot */
	    keyTab[keyTabIndex] = 0;
	}
	else {
	    if (extraCnd) {
		extraCnd = 0;
		++extraCount;
		keyTab[keyTabIndex] = -k;
	    }
	    else keyTab[keyTabIndex] = k;

	    elemTab[elemTabIndex++] = NULL;

	    k = elemTabIndex - k;
	    if (k > maxSurfEl) maxSurfEl = k;
#ifdef DRIVER
	    if (--k > maxRECOG) {
		fprintf (stderr, "found %d elements in keyTab slot %d (read=%d skipped=%d)\n",
		    k, keyTabIndex, k + skipnr, skipnr);
		++tooMANY;
	    }
#endif
	}
#endif /* MAKEDELA */
    }

#ifndef MAKEDELA
    /* If the elemTab array is empty for more than 10k,
     * realloc to reclaim the empty space.
     * It can be empty because of elemDefs left out because
     * of option settings (see the loop above),
     * or because of a bug in tecc (date: 930329).
     */
    t = (elemTabSize - elemTabIndex) * sizeof (elemDef_t *);
    if (t > 10 * 1024) {
	elemTab = RESIZE (elemTab, elemDef_t *, elemTabIndex, elemTabSize);
    }
#endif /* MAKEDELA */

    /*----------- reading Edge Elements keytab ---------------*/

    keyTabSize2 = getnum (fp_tech);
#ifndef MAKEDELA
    if (keyTabSize2 != maxKeys2) techReadError (14);
#endif

    elemTabSize2 = getnum (fp_tech) + 1;
#ifndef MAKEDELA
    keyTab2  = NEW (int, keyTabSize2);
    elemTab2 = NEW (elemDef_t *, elemTabSize2);
    elemTabIndex2 = 0;
    elemTab2[elemTabIndex2++] = NULL;
#endif

    extraCnd = 0;
    for (keyTabIndex = 0; keyTabIndex < keyTabSize2; keyTabIndex++) {
#ifdef MAKEDELA
	while ((c = getc (fp_tech)) != '\n') if (c == EOF) techReadError (1);
#else
	k = elemTabIndex2;
#ifdef DRIVER
	skipnr = 0;
#endif
	while ((i = getElemNumber (fp_tech)) >= 0) {
	    el = &elemDefTab[i];
	    if (el -> type != SKIP_ELEM) {
		elemTab2[elemTabIndex2++] = el;
		if (el -> cond_cnt) ++extraCnd;
	    }
#ifdef DRIVER
	    else ++skipnr;
#endif
	}
	if (i == ELEM_IDEM) { /* The elements for this key were already seen
				for another key. Copy the value in keyTab2 */
	    i = getnum (fp_tech);
	    ASSERT (i >= 0 && i < keyTabIndex);
	    keyTab2[keyTabIndex] = keyTab2[i];
	    if (keyTab2[i] < 0) ++extraCount2;
	}
	else if (elemTabIndex2 == k) { /* no elements in slot */
	    keyTab2[keyTabIndex] = 0;
	}
	else {
	    if (extraCnd) {
		extraCnd = 0;
		++extraCount2;
		keyTab2[keyTabIndex] = -k;
	    }
	    else keyTab2[keyTabIndex] = k;

	    elemTab2[elemTabIndex2++] = NULL;

	    k = elemTabIndex2 - k;
	    if (k > maxEdgeEl) maxEdgeEl = k;
#ifdef DRIVER
	    if (--k > maxRECOG) {
		fprintf (stderr, "found %d elements in keyTab2 slot %d (read=%d skipped=%d)\n",
		    k, keyTabIndex, k + skipnr, skipnr);
		++tooMANY;
	    }
#endif
	}
#endif /* MAKEDELA */
    }

#ifndef MAKEDELA
    t = (elemTabSize2 - elemTabIndex2) * sizeof (elemDef_t *);
    if (t > 10 * 1024) {
	elemTab2 = RESIZE (elemTab2, elemDef_t *, elemTabIndex2, elemTabSize2);
    }

#ifdef DRIVER
    if (prVerbose)
    for (i = 0; i < elemDefTabSize; i++) {
	el = &elemDefTab[i];
	if (el -> type == SKIP_ELEM) fprintf (stderr, "skipping element: %s\n", el -> name);
    }
    if (tooMANY)
	fprintf (stderr, "found %d slots with more than %d entries!\n", tooMANY, maxRECOG);
#endif

if (debug_gettech) {
    fprintf (stderr, "-----------------------------------\n");
    fprintf (stderr, "hasSubmask = %d\n", hasSubmask);
    fprintf (stderr, "nrOfCondStd = %d\n", nrOfCondStd);
    fprintf (stderr, "nrOfCondPos = %d\n", nrOfCondPos - nrOfCondStd);
    fprintf (stderr, "nrOfCondNeg = %d\n", nrOfCondNeg);
    fprintf (stderr, "nrOfConductors = %d\n", nrOfConductors);
    fprintf (stderr, "-----------------------------------\n");
    fprintf (stderr, "elemDefTab size = %7d bytes\n", elemDefTabSize * (int)sizeof (elemDef_t));
    fprintf (stderr, "elemTab    size = %7d bytes", elemTabSize  * (int)sizeof (elemDef_t *));
    if ((t = elemTabIndex) < elemTabIndex2) t = elemTabIndex2;
    sprintf (buf, "%d", t  * (int)sizeof (elemDef_t *));
    i = strlen (buf);
    t = (elemTabSize - elemTabIndex) * sizeof (elemDef_t *);
    if (t > 10024) fprintf (stderr, " (%*d after resize)", i, elemTabIndex * (int)sizeof (elemDef_t *));
    fprintf (stderr, "\nelemTab2   size = %7d bytes", elemTabSize2 * (int)sizeof (elemDef_t *));
    t = (elemTabSize2 - elemTabIndex2) * sizeof (elemDef_t *);
    if (t > 10024) fprintf (stderr, " (%*d after resize)", i, elemTabIndex2 * (int)sizeof (elemDef_t *));
    fprintf (stderr, "\nkeyTab     size = %7d bytes\n", keyTabSize  * (int)sizeof (int));
    fprintf (stderr, "keyTab2    size = %7d bytes\n", keyTabSize2 * (int)sizeof (int));
    fprintf (stderr, "-----------------------------------\n");
    fprintf (stderr, "conduct elements = %3d (%3d skipped)\n",  resELtt, resELsk);
    fprintf (stderr, "connect elements = %3d (%3d skipped)\n", connELtt, connELsk);
    fprintf (stderr, "contact elements = %3d (%3d skipped)\n", contELtt, contELsk);
    fprintf (stderr, "fet tor elements = %3d\n",  torELtt);
    fprintf (stderr, "l/v bjt elements = %3d (%3d skipped)\n",  bjtELtt, bjtELsk);
    fprintf (stderr, "l/v jun elements = %3d (%3d skipped)\n", juncELtt, juncELtt);
    fprintf (stderr, "lat cap elements = %3d (%3d skipped)\n", lcapELtt, lcapELsk);
    fprintf (stderr, "s/e cap elements = %3d (%3d skipped)\n",  capELtt,  capELsk);
    fprintf (stderr, "subcont elements = %3d (%3d skipped)\n", subcELtt, subcELsk);
    fprintf (stderr, "vdimens elements = %3d (%3d skipped)\n", vdimELtt, vdimELsk);
    fprintf (stderr, "ecshape elements = %3d (%3d skipped)\n", shapELtt, shapELsk);
    fprintf (stderr, "newmask elements = %3d (%3d skipped)\n",  newELtt,  newELsk);
    fprintf (stderr, "-----------------------------------\n");
    fprintf (stderr, "TOTAL # elements = %3d (%3d skipped)\n", elemDefTabSize,
	lcapELsk + capELsk + subcELsk + vdimELsk + shapELsk + newELsk + contELsk + resELsk + connELsk + bjtELsk);
    fprintf (stderr, "-----------------------------------\n");
    fprintf (stderr, "elemTabcnt = %6d (= %*d bytes)\n", elemTabIndex, i, elemTabIndex * (int)sizeof (elemDef_t *));
    fprintf (stderr, "elemTabcnt2= %6d (= %*d bytes)\n", elemTabIndex2, i, elemTabIndex2 * (int)sizeof (elemDef_t *));
    fprintf (stderr, "keyTabSize = %6d (%d has extra cond)\n", keyTabSize, extraCount);
    fprintf (stderr, "keyTabSize2= %6d (%d has extra cond)\n", keyTabSize2, extraCount2);
    fprintf (stderr, "maxSurfEl# = %6d (in one keyslot)\n", maxSurfEl - 1);
    fprintf (stderr, "maxEdgeEl# = %6d (in one keyslot)\n", maxEdgeEl - 1);
    fprintf (stderr, "-----------------------------------\n");
}

    otherElemSpace  = NEW (elemDef_t *, maxEdgeEl);
    otherElemSpace2 = NEW (elemDef_t *, maxEdgeEl);
    elemListSpaceE  = NEW (elemDef_t *, maxEdgeEl);
    elemListSpaceS  = NEW (elemDef_t *, maxSurfEl);
#endif /* MAKEDELA */

    /* Read dielectric, substrate,
       selfsubdata and mutualsubdata information
       and mask draw color. */

    if ((diel_cnt = getnum (fp_tech)) > 0) {
#ifdef MAKEDELA
#ifdef DRIVER
	fprintf (stderr, "diel_cnt=%d\n", diel_cnt);
#endif
#endif
	diels = NEW (dielectric_t, diel_cnt);
	for (i = 0; i < diel_cnt; i++) {
	    if (fscanf (fp_tech, "%s %le %le", diels[i].name,
		&diels[i].permit, &diels[i].bottom) != 3) techReadError (15);
	    while ((c = getc (fp_tech)) != '\n' && c != EOF);
#ifdef MAKEDELA
#ifdef DRIVER
	    fprintf (stderr, "name=%s val=%e bottom=%e\n",
		diels[i].name, diels[i].permit, diels[i].bottom);
#endif
#endif
	}
    }

    if ((substr_cnt = getnum (fp_tech)) > 0) {
#ifdef MAKEDELA
#ifdef DRIVER
	fprintf (stderr, "substr_cnt=%d\n", substr_cnt);
#endif
#endif
	substrs = NEW (substrate_t, substr_cnt);
	for (i = 0; i < substr_cnt; i++) {
	    if (fscanf (fp_tech, "%s %le %le", substrs[i].name,
		&substrs[i].conduc, &substrs[i].top) != 3) techReadError (16);
	    while ((c = getc (fp_tech)) != '\n' && c != EOF);
#ifdef MAKEDELA
#ifdef DRIVER
	    fprintf (stderr, "name=%s val=%e top=%e\n",
		substrs[i].name, substrs[i].conduc, substrs[i].top);
#endif
#endif
	}
    }

    if (minorPrNr > 2 && (subcap_cnt = getnum (fp_tech)) > 0) {
#ifdef MAKEDELA
#ifdef DRIVER
	fprintf (stderr, "subcap_cnt=%d\n", subcap_cnt);
#endif
#endif
	subcaps = NEW (substrate_t, subcap_cnt);
	for (i = 0; i < subcap_cnt; i++) {
	    if (fscanf (fp_tech, "%s %le %le", subcaps[i].name,
		&subcaps[i].conduc, &subcaps[i].top) != 3) techReadError (16);
	    while ((c = getc (fp_tech)) != '\n' && c != EOF);
#ifdef MAKEDELA
#ifdef DRIVER
	    fprintf (stderr, "name=%s val=%e top=%e\n",
		subcaps[i].name, subcaps[i].conduc, subcaps[i].top);
#endif
#endif
	}
    }

    if ((self_cnt = getnum (fp_tech)) > 0) {
#ifdef MAKEDELA
#ifdef DRIVER
	fprintf (stderr, "self_cnt=%d\n", self_cnt);
#endif
#endif
	selfs = NEW (selfsubdata_t, self_cnt+1);
	for (i = 0; i < self_cnt; i++) {
	    if (fscanf (fp_tech, "%le %le %le %le",
		&selfs[i].area, &selfs[i].perim,
		&selfs[i].val, &selfs[i].rest) != 4) techReadError (17);
	    while ((c = getc (fp_tech)) != '\n' && c != EOF);
#ifdef MAKEDELA
#ifdef DRIVER
	    fprintf (stderr, "a=%e p=%e r=%e rest=%e\n",
		selfs[i].area, selfs[i].perim, selfs[i].val, selfs[i].rest);
#endif
#endif
	}
    }
    else self_cnt = 0;

    if ((mut_cnt = getnum (fp_tech)) > 0) {
#ifdef MAKEDELA
#ifdef DRIVER
	fprintf (stderr, "mut_cnt=%d\n", mut_cnt);
#endif
#endif
	muts = NEW (mutualsubdata_t, mut_cnt+1);
	for (i = 0; i < mut_cnt; i++) {
	    if (fscanf (fp_tech, "%le %le %le %le %le",
		&muts[i].area1, &muts[i].area2, &muts[i].dist,
		&muts[i].val, &muts[i].decr) != 5) techReadError (18);
	    if (muts[i].area1 > muts[i].area2) {
		double tmp_a1 = muts[i].area1;
		muts[i].area1 = muts[i].area2;
		muts[i].area2 = tmp_a1;
	    }
	    while ((c = getc (fp_tech)) != '\n' && c != EOF);
#ifdef MAKEDELA
#ifdef DRIVER
	    fprintf (stderr, "a1=%e a2=%e d=%e r=%e decr=%e\n",
		muts[i].area1, muts[i].area2, muts[i].dist,
		muts[i].val, muts[i].decr);
#endif
#endif
	}
    }
    else mut_cnt = 0;

#endif /* MAKESIZE */
}

#ifndef MAKEDELA
void printRecogCnt ()
{
    elemDef_t *el;
    int i;

    fprintf (stderr, "\nElement:   recognize_count:\n");
    for (i = 0; i < elemDefTabSize; i++) {
	el = &elemDefTab[i];
	if (el -> type == SKIP_ELEM) continue;
	fprintf (stderr, "%-18s %8d\n", el -> name, el -> el_recog_cnt);
    }
}

int diffusionCap (capElemDef_t *cap)
{
    int n = cap -> nCon; /* n is Gnd if < 0 */
    return ((n < 0 || diffusionConductor[n]) && diffusionConductor[cap -> pCon]);
}

char * conNr2Name (int n)
{
    if (n >= 0 && n <= nrOfCondStd) return (masktable[conductorMask[n]].name);
    return ((char*)"???");
}

int conName2Nr (char *name)
{
    int i;
    for (i = 0; i < nrOfMasks; ++i)
	if (strsame (masktable[i].name, name)) return (masktable[i].conductor);
    return (-1);
}
#endif /* MAKEDELA */

#ifndef DRIVER
char *giveICD (char *filepath)
{
    int len = strlen (icdpath);
    if (strncmp (icdpath, filepath, len)) return filepath;
    return mprintf ("$ICDPATH%s", filepath + len);
}

void getTechFile (DM_PROJECT *dmproject, char *techDef, char *techFile)
{
    if (!techFile) {
	char *name = techDef? mprintf ("space.%s.t", techDef) : "space.def.t";
	techFile = dmGetMetaDesignData (PROCPATH, dmproject, name);
    }
    if (access (techFile, 0) != 0) {
	say ("%s: no such technology file", giveICD (techFile));
	die ();
    }
    usedTechFile = strsave (techFile);
}

#endif

Private int getnum (FILE *fp)
{
    int negative = 0;
    register int number;
    register int c = getc (fp);

    while (c != EOF && isspace (c)) c = getc (fp);

    if (c == '-') { ++negative; c = getc (fp); }

    if (isdigit (c)) {
	number = c - '0';
	for (c = getc (fp); isdigit (c); c = getc (fp))
	    number = 10 * number + c - '0';
        if (c == '\r') c = getc (fp);
        if (c == '\n') return (negative ? -number : number);
    }
    techReadError (19);
    return (-100);
}

#ifndef MAKEDELA
Private int cctoi (int c0, int c1)
{
    int n, m;

    if (c0 <= '9') {
	if (c0 < '0') techReadError (20);
	n = (c0 - '0');
    }
    else if (c0 <= 'Z') {
	if (c0 < 'A') techReadError (21);
	n = (10 + c0 - 'A');
    }
    else if (c0 <= 'z') {
	if (c0 < 'a') techReadError (22);
	n = (36 + c0 - 'a');
    }
    else { n = 0; techReadError (23); }

    if (c1 == ' ') return (n);

    if (c1 <= '9') {
	if (c1 < '0') techReadError (24);
	m = (1 + c1 - '0');
    }
    else if (c1 <= 'Z') {
	if (c1 < 'A') techReadError (25);
	m = (11 + c1 - 'A');
    }
    else if (c1 <= 'z') {
	if (c1 < 'a') techReadError (26);
	m = (37 + c1 - 'a');
    }
    else { m = 0; techReadError (27); }

    return (m * 62 + n);
}

Private int getElemNumber (FILE *fp)
{
    int number, c = getc (fp);

    if (coded) {
	if (c == '\r' && (c = getc (fp)) != '\n') techReadError (28);
	if (c == '\n') return (ELEM_END);
	if (c == '=')  return (ELEM_IDEM);
	return cctoi (getc (fp), c);
    }

    while (c == ' ' || c == '\r') c = getc (fp);
    if (c == '\n') return (ELEM_END);
    if (c == '=')  return (ELEM_IDEM);

    if (isdigit (c)) {
	number = c - '0';
	for (c = getc (fp); isdigit (c); c = getc (fp))
	    number = 10 * number + c - '0';
	if (c == '\n') ungetc (c, fp);
	return (number);
    }
    techReadError (29);
    return (ELEM_ERROR);
}

#ifndef CONFIG_SPACE2
static mask_t filterBitmaskSave;
mask_t * colorsSave;

void pp1SetColors ()
{
    int i, k;

    filterBitmaskSave = filterBitmask;
    filterBitmask = cNull;
    if (!colorsSave) colorsSave = NEW (mask_t, nrOfCondStd+1);

    for (k = 0; k <= nrOfCondStd; ++k) {
	i = conductorMask[k];
	colorsSave[k] = masktable[i].color;
	COLORINITINDEX (masktable[i].color, k);
	COLOR_ADD (filterBitmask, masktable[i].color);
    }
}

void pp1ResetColors ()
{
    int k;

    filterBitmask = filterBitmaskSave;

    for (k = 0; k <= nrOfCondStd; ++k) {
	masktable[conductorMask[k]].color = colorsSave[k];
    }
}
#endif
#endif /* MAKEDELA */

#ifdef DRIVER
extern char *optarg;
extern int optind;

bool_t optLatCap = FALSE;
bool_t optCap    = FALSE;
bool_t optIntRes = FALSE;
bool_t optSubRes = FALSE;
bool_t substrRes = FALSE;

void print_slots_with_no_extra_conditions ()
{
    elemDef_t ** el;
    int i, k, len;

    fprintf (stderr, "\nkeyTab[i] elements:\n");

    for (i = 0; i < keyTabSize; ++i) {
	if ((k = keyTab[i]) < 0) continue;
	fprintf (stderr, "(%5d)", i); len = 0;
	for (el = elemTab + k; *el; el++) {
	    if (len > 80) { fprintf (stderr, "\n%7s", ""); len = 0; }
	    fprintf (stderr, " %s", (*el) -> name);
	    len += strlen ((*el) -> name);
	}
	fprintf (stderr, "\n");
    }

    fprintf (stderr, "\nkeyTab2[i] elements:\n");

    for (i = 0; i < keyTabSize2; ++i) {
	if ((k = keyTab2[i]) < 0) continue;
	fprintf (stderr, "(%5d)", i); len = 0;
	for (el = elemTab2 + k; *el; el++) {
	    if (len > 80) { fprintf (stderr, "\n%7s", ""); len = 0; }
	    fprintf (stderr, " %s", (*el) -> name);
	    len += strlen ((*el) -> name);
	}
	fprintf (stderr, "\n");
    }
}

int main (int argc, char *argv[])
{
    elemDef_t ** el;
    int i, k, n, len;
    int optErr = 0;
    int pr_points = 0;
    int pr_e_keys = 0;
    int no_keytab = 0;
    int no_extras = 0;

    argv0 = "gettech";

    while ((k = getopt (argc, argv, "rcCl3e2bBoOtvK:R:pEknh")) != EOF) {
	switch (k) {
	    case 'l': optLatCap = TRUE;
	    case 'C':
	    case 'c': optCap = TRUE; break;
	    case '3': optCap3D = optCap = TRUE; break;
	    case '2': extractNon3dAllways = TRUE; break;
	    case 'e': extractDiffusionCap3d = TRUE; break;
	    case 'r': optIntRes = TRUE; break;
	    case 'B': optSubRes = TRUE;
	    case 'b': substrRes = TRUE; break;
	    case 'o': omit_subcont = TRUE; break;
	    case 'O': omit_ds_caps = TRUE; break;
	    case 't': prTable = TRUE; break;
	    case 'v': prVerbose  = TRUE; break;
	    case 'K': keep_nodes = optarg; break;
	    case 'R': keep_conductors = optarg; break;
	    case 'p': pr_points = 1; break;
	    case 'E': pr_e_keys = 1; break;
	    case 'k': no_keytab = 1; break;
	    case 'n': no_extras = 1; break;
	    default: optErr++;
	}
    }

    if (optErr || optind != argc - 1) {
	fprintf (stderr, "\nUsage: %s ", argv0);
	fprintf (stderr, "[-rcCl3e2bBoOtvpEknh] [-K el.node] [-R res_el] file.t\n\n");
	fprintf (stderr, "\t-e   extractDiffusionCap3d\n");
	fprintf (stderr, "\t-2   extractNon3dAllways\n");
	fprintf (stderr, "\t-o   omit_subcont\n");
	fprintf (stderr, "\t-O   omit_ds_caps\n");
	fprintf (stderr, "\t-t   prTable\n");
	fprintf (stderr, "\t-v   prVerbose\n");
	fprintf (stderr, "\t-p   pr_points\n");
	fprintf (stderr, "\t-E   pr_e_keys\n");
	fprintf (stderr, "\t-k   no_keytab\n");
	fprintf (stderr, "\t-n   pr_no_extra_cond slots\n");
	fprintf (stderr, "\t-h   print help\n");
	fprintf (stderr, "\t-K   keep_nodes\n");
	fprintf (stderr, "\t-R   keep_conductors/filters\n\n");
	return (1);
    }

    usedTechFile = argv[optind];

    readTechFile (cfopen (usedTechFile, "r"));

    if (no_keytab) return (0);

    if (no_extras) {
	print_slots_with_no_extra_conditions ();
	return (0);
    }

    if (pr_e_keys) {
	fprintf (stderr, "\nkeyTab2[i] elements:\n");
	keyTabSize = keyTabSize2;
	keyTab = keyTab2;
	elemTab = elemTab2;
    }
    else
	fprintf (stderr, "\nkeyTab[i] elements:\n");

    if (pr_points) {
	for (i = 0; i <= keyTabSize; ) {
	    if (i == keyTabSize) {
		if (keyTabSize <= 2) break;
		--i;
	    }
	    fprintf (stderr, "(%5d)", i); len = 0;
	    if ((k = keyTab[i]) < 0) k = -k;
	    for (el = elemTab + k; *el; el++) {
		if (len > 80) { fprintf (stderr, "\n%7s", ""); len = 0; }
		fprintf (stderr, " %s", (*el) -> name);
		len += strlen ((*el) -> name);
	    }
	    fprintf (stderr, "\n");
	    if (i) i <<= 1;
	    else i = 1;
	}
	return (0);
    }

    n = keyTabSize < 10 ? keyTabSize : 10;

    for (i = 0; i < n; i++) {
	fprintf (stderr, "(%5d)", i); len = 0;
	if ((k = keyTab[i]) < 0) k = -k;
	for (el = elemTab + k; *el; el++) {
	    if (len > 80) { fprintf (stderr, "\n%7s", ""); len = 0; }
	    fprintf (stderr, " %s", (*el) -> name);
	    len += strlen ((*el) -> name);
	}
	fprintf (stderr, "\n");
    }

    if (keyTabSize < 10) return (0);
    if (keyTabSize > 20) {
	fprintf (stderr, "...\n");
	n = keyTabSize - 10;
    }

    for (i = n; i < keyTabSize; i++) {
	fprintf (stderr, "(%5d)", i); len = 0;
	if ((k = keyTab[i]) < 0) k = -k;
	for (el = elemTab + k; *el; el++) {
	    if (len > 80) { fprintf (stderr, "\n%7s", ""); len = 0; }
	    fprintf (stderr, " %s", (*el) -> name);
	    len += strlen ((*el) -> name);
	}
	fprintf (stderr, "\n");
    }
    fprintf (stderr, "\n");

    return (0);
}
#endif /* DRIVER */
