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

#ifndef TECC_TYPE_H
#define TECC_TYPE_H

/*============================================================================*/
/*! @brief Representation of a layer reference.

    This structure may represent, for example 'cpg' or '=cpg' or '-cpg' in
    an expression.
*//*==========================================================================*/

struct layer {
    /*------------------------------------------------------------------------*/
    /*! @brief The mask number.
     *//*---------------------------------------------------------------------*/

    int mask;

    /*------------------------------------------------------------------------*/
    /*! @brief The `type' of occurence of the layer in the expression.
     *
     *  May be: @c EDGE, @c OTHEREDGE, or @c SURFACE.
     *
     *  If a mask name is used with a leading '-', then we have an @c EDGE,
     *  if the mask name is used with a leading '=', then we have an
     *  @c OTHEREDGE. Otherwise, we have a @c SURFACE.
     *//*---------------------------------------------------------------------*/

    int occurrence;
};

struct layerRef {
    int present;
    struct layer *lay;
    struct layerRef *next;
};

struct layerCond {
    int present;
    struct layer *lay;
};

struct layCondRef {
    struct layerCond *layC;
    struct layCondRef *next;
};

struct condListRef {
    struct layCondRef *cond_list;
    struct condListRef *next;
};

struct newMaskRef {
    char *name;
    int id;
    int mask;
    int resized;
    struct condListRef *cond_lists;
    struct newMaskRef *next;
};

struct elemRef {
    int elno;
    struct elemRef *next;
};

struct contact {
    char *name;
    int id;
    char *sort;
    int sortNr;
    int mask1;
    int mask2;
    double val;
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
};

struct transistor {
    char *name;
    int id;
    int g;
    int ds;
    int s;
    int b;
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
    int dscap;	/* EM: to indicate whether a drain/source capacitance
    		       has been specified in the same extraction rule
    		       as the transistor. Used for generation of an
    		       inconsistency warning */
};

struct capacitance {
    char *name;
    int id;
    char *sort;
    int sortNr;
    int junc;
    int cap3d;
    int eltype; /* SURFCAPELEM, EDGECAPELEM, LATCAPELEM */
    struct layer *pLay;
    struct layer *nLay;
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    mask_t oBitPresent, oBitAbsent;
    double val, z1, z2;
    struct x_y *x_y_vals;
    int x_y_index;
    struct layCondRef *cond;
};

struct resistance {
    char *name;
    int id;
    char *sort;
    int sortNr;
    int mask;
    int type; /* m, n, p */
    double val;
    double height;
    double thickness;
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
};

struct vdimension {
    char *name;
    int id;
    int mask;
    double height;
    double thickness;
    double spacing;
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
};

struct shape {
    char *name;
    int id;
    int type;
    int mask;
    double xb1, xt1, xb2, xt2;
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    struct layCondRef *cond;
};

struct subcont {
    char *name;
    int id;
    int ccnr;	/* number used for assigning to causing_con (cc), to
                   prevent joining of different types of substrate contacts */
    int captype; /* if true, subcont of capacitance element */
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
};

struct newsubdata {
    char *name;
    int id;
    int mask;
    mask_t sBitPresent, sBitAbsent;
    struct layCondRef *cond;
};

struct w_list {
    int begin;
    int depth;
    int mskno;
    struct w_list *next;
};

struct waferdata {
    int depth, tn;
    int lays;
    int type;
    struct condListRef *cref;
};

struct x_y {
    double x, y;
    struct x_y *next;
};

struct submasks {
    int nomasks;
    char **mask_name;
};

struct junction {
    char *name;
    int id;
    struct layer *pins[2];
    double vbr;
    double cap;
    double dep;
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    struct layCondRef *cond;
};

struct bipoTor {
    char *name;
    int id;
    int type;
    struct layer *pins[4];
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    mask_t oBitPresent, oBitAbsent;
    struct layCondRef *cond;
};

struct connect {
    char *name;
    int id;
    double val;
    struct layer *cons[2];
    mask_t sBitPresent, sBitAbsent;
    mask_t eBitPresent, eBitAbsent;
    struct layCondRef *cond;
};

struct dielectric {
    char *name;
    double permit;
    double bottom;
};

struct substrate {
    char *name;
    double conduc;
    double top;
};

struct selfsubdata {
    double area;
    double perim;
    double val;
    double rest;
};

struct mutualsubdata {
    double area1;
    double area2;
    double dist;
    double val;
    double decr;
};

struct resizeCond {
    mask_t present;	/* bitvector for masks which MUST be present */
    mask_t absent;	/* bitvector for masks which MUST be absent */
    struct resizeCond *next;	/* for OR possibility */
};

struct resizedata {
    int id;
    char *newmaskname;
    double val;
    int condcnt;	/* number of conditions, for OR possibility */
    struct resizeCond *cond;
};

#endif /* TECC_TYPE_H */
