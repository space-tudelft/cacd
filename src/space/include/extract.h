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

typedef struct nodePoint {
    coor_t x, y;
    struct subnode ** cons; /* pointer to nrOfCondStd pointers */
    struct nodePoint * next;
    struct nodePoint * prev;
    short fixed;
} nodePoint_t;

typedef struct contElemDef {
    int sortNr;
    int mask1;
    int con1;
    int keep1;
    int mask2;
    int con2;
    int keep2;
    double val;
} contElemDef_t;

typedef struct torElemDef {
    int gMask;     /* gate mask */
    int gCon;      /* gate conductor */
    int dsMask;    /* drain-source mask */
    int dsCon;     /* drain-source conductor */
    int dsCap;     /* drain-source capacitance */
    int sMask;     /* source mask */
    int sCon;      /* source conductor */
    int bMask;     /* bulk mask */
    int bCon;      /* bulk conductor */
} torElemDef_t;

typedef struct capElemDef {
    int sortNr;
    int pMask;
    int pCon;
    int pOccurrence;
    int pKeep;
    int nMask;
    int nCon;
    int nOccurrence;
    int nKeep;
    short done;
    short dsCap;   /* skip drain-source capacitance */
    mask_t sBitPresent, sBitAbsent;
    mask_t oBitPresent, oBitAbsent;
    double val;
    struct xy_abp *mval;
} capElemDef_t;

typedef struct resElemDef {
    int rKeep;
    int sortNr;
    int mask;
    int con;
    double val;
    int type;	/* BIPOLAR: 'a', 'n' or 'p' */
} resElemDef_t;

typedef struct newElemDef {
    int mask;
} newElemDef_t;

#ifndef CAP3D
  typedef double meshCoor_t;
#endif

typedef struct vDimElemDef {
    int mask;
    int con;                    /* conductor */
    double height;
    double thickness;
} vDimElemDef_t;

typedef struct shapeElemDef {
    int mask;
    int con;                    /* conductor */
    double xb1;
    double xt1;
    double xb2;
    double xt2;
} shapeElemDef_t;

typedef struct pnconElemDef {
    int mask1;
    int occurrence1;
    int con1;
    int mask2;
    int occurrence2;
    int con2;
    double val;
} pnconElemDef_t;

typedef struct junElemDef {
    int nMask;
    int nCon;
    int nOccurrence;
    int pMask;
    int pCon;
    int pOccurrence;
    double cap;
    double depth;
    double voltage;
} junElemDef_t;

typedef struct bjtorElemDef {
    int bMask;
    int bCon;
    int bOccurrence;
    int eMask;
    int eCon;
    int eOccurrence;
    int cMask;
    int cCon;
    int cOccurrence;
    int sMask;
    int sCon;
    mask_t oBitPresent, oBitAbsent;
} bjtorElemDef_t;

typedef struct subcontElemDef {
    int captype; /* subcont of cap element */
    int ccnr;	/* number used to assign to causing_con */
} subcontElemDef_t;

typedef struct dielectric {
    char name[MAX_ELEM_NAME + 1];
    double permit;
    double bottom;
} dielectric_t;

typedef struct substrate {
    char name[MAX_ELEM_NAME + 1];
    double conduc;
    double top;
} substrate_t;

typedef struct selfsubdata {
    double area;
    double perim;
    double val;
    double rest;
} selfsubdata_t;

typedef struct mutualsubdata {
    double area1;
    double area2;
    double dist;
    double val;
    double decr;
} mutualsubdata_t;

typedef struct elemDef {
    char name[MAX_ELEM_NAME + 1];
    int id;
    int cond_cnt;
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    int type;
    int el_recog_cnt;
    union {
	resElemDef_t res;
	contElemDef_t cont;
	torElemDef_t tor;
	capElemDef_t cap;
	vDimElemDef_t vdim;
	shapeElemDef_t shape;
	pnconElemDef_t pnc;
	junElemDef_t jun;
	bjtorElemDef_t bjt;
        subcontElemDef_t subc;
	newElemDef_t newmask;
    } s;
} elemDef_t;

typedef struct xy_abp {
    double x, y;
    double a, b, p;
    struct xy_abp *next;
} xy_abp_t;

typedef struct xy_list {
    struct xy_abp *list;
    struct xy_list *next;
} xy_list_t;

typedef struct pnEdge {
    int edgeOrien;
    coor_t x1, y1;
    coor_t x2, y2;
    struct pnEdge * next;
} pnEdge_t, tileBoundary_t;

/* part of a substrate terminal that corresponds to a tile */
typedef struct subcontRef {
    struct subnode * subn;
    int causing_con;    /* conductor that defines this substrate
                                                            terminal. */
    short distributed;  /* substrate terminal is modeled as "distributed"
                           with resistances connected to each corner. */
    struct subcont * subcontInfo;
    struct subcontRef * nextRef;
} subcontRef_t;

/* substrate terminal (consisting of one or more subcontRef's) */
typedef struct subcont {
    coor_t xl;
    coor_t yb;
    double area;
    double perimeter;
    double remaining;
    int ready;
    int nr;
    int rest_neigh_cnt;
    struct subnode * subn;
    struct subnode * subnTR;
    struct subnode * subnTL;
    struct subnode * subnBR;
    struct subnode * subnBL;
    struct subcontRef * subcontRefs;
    struct subcontGroup * group; /* pointer to group during prePass1,
				    group id during other passes */
    struct subcont * nextGroup;
    struct subcont * prevGroup;
    struct subcont * next;
    struct subcont * prev;
} subcont_t;

/* set of substrate terminals that are adjacent */
typedef struct subcontGroup {
    int id;
    struct subcont * sc_begin;
    struct subcont * sc_end;
} subcontGroup_t;

typedef struct resizeCond {
    mask_t present;
    mask_t absent;
    struct resizeCond *next;
} resizeCond_t;

typedef struct resizeData {
    int id;
    char newmaskname [513];
    double val;
    int condcnt;
    struct resizeCond *cond;
} resizeData_t;

