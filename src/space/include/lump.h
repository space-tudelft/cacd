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

typedef struct elementC {
    double val;
    struct Node *parentA, *parentB;
    struct elementC *nextA, *prevA;
    struct elementC *nextB, *prevB;
    struct elementC *nextHash;
#ifdef MOMENTS
    /*------------------------------------------------------------------------*/
    /*! @brief The "extra" moments of the element.
     *
     *  Contains a pointer to an array of "extra" moments. The amount of
     *  moments in this array is determined by the variable <code>extraMoments</code>.
     *
     *  The pointer may be NULL if there are no extra moments associated with the element.
     *
     *  Note that the moments of an element are the coefficients of the
     *  Laplace-domain admittance polynomial corresponding to the element.
     *
     *  Extra moments may be assigned to each element in the SNE reduction algorithm.
     *
     *  Currently, only capacitive elements will have extra moments (resistances will not).
     *
     *  @see momentsElimOrWeight.
     *//*---------------------------------------------------------------------*/
    double *moments;
#endif
    short sort; /* sort of element for parentA */
    char  type;
    char  flagEl;
} element_c;

/* WARNING: be sure that elementR has the same size as elementC */
typedef struct elementR {
    double val;
    struct Node *parentA, *parentB;
    struct elementR *nextA, *prevA;
    struct elementR *nextB, *prevB;
    struct elementR *nextHash;
#ifdef MOMENTS
    double *momentsx; /* not used */
#endif
    short sort; /* sort of element for parentA */
    char  type;
    char  flagx; /* not used */
} element_r;

typedef struct group {
    int notReady;
    int nod_cnt;
    int grp_nr;
    unsigned supply : 8;
    unsigned prick  : 8;
    unsigned flagQG : 8;
    struct Node * nodes;
#ifndef CONFIG_SPACE2
    struct groupTileInfo * tileInfo;
    struct Terminal * name;
#endif
} group_t;

#ifdef CONFIG_SPACE2
typedef struct cluster {
    int notReadyNodes;
    struct Node * nodes;
#ifdef ADD_ALINKS
    struct Alnk *Anodes;
#endif
} cluster_t;

typedef struct Alnk {
    struct Node *Anode;
    struct Alnk *Anext;
} alnk_t;
#endif

typedef struct netEquiv {
#ifdef CONFIG_SPACE2
    double val;
    char sort;
#endif
    char instType;
    char term;
    int number;
#ifndef CONFIG_SPACE2
    char *instName;     /* if defined: overrules number */
#endif
    struct netEquiv * next;
} netEquiv_t;

typedef struct groupTileInfo {
#ifndef CONFIG_SPACE2
    struct tileRef * tiles;
    struct contRef * conts;
#endif
} groupTileInfo_t;

typedef struct tileRef {
    int tile;
    int cx;
    mask_t color;
    coor_t xl, xr;
    coor_t bl, br;
    coor_t tl, tr;
    struct tileRef * next;
} tileRef_t;

typedef struct contRef {
    int nr;
    int cx;
    coor_t xl, bl;
    struct contRef * cnext;
    struct contRef * next;
    struct contRef * prev;
} contRef_t;

