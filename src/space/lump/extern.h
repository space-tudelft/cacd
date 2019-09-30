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

#include "src/space/lump/export.h"

#ifdef MOMENTS
#define MAXMOMENT 10
extern int maxMoments;
extern int extraMoments;
extern bool_t doElmore;
extern bool_t doOutRC;
extern bool_t doOutL;
extern bool_t doOutNegC;
extern bool_t doOutNegR;
extern bool_t printMoments;
#endif /* MOMENTS */

#ifdef SNE
extern double sneOmega;
extern int sneNorm;
extern int sneErrorFunc;
extern int sneFullGraph;
extern double sneTolerance;
extern int sneResolution;
extern double sneNormedResolution;
extern int storeCount, elimCount;
extern bool_t printElimCount;
#endif /* SNE */
extern int delayCount;
extern int delay_cnt;

extern bool_t omit_inc_tors;
extern bool_t optCap;
extern bool_t optCoupCap;
extern bool_t optRes;
extern bool_t optSubRes;
extern bool_t optFineNtw;
extern bool_t optNoReduc;
extern bool_t optReduc;
extern bool_t optReduc2;
extern bool_t optTorPos;
extern bool_t optInfo;
extern bool_t optVerbose;
extern int    prePass;
extern bool_t prePass1;
extern bool_t optPrick;
extern bool_t optInvertPrick;
extern bool_t optBackInfo;

extern int nrOfMasks;
extern int nrOfCondStd;

extern maskinfo_t * masktable;
extern int * conductorMask;

extern int inCap;
extern int outCap;
extern int currIntCap;
extern int maxIntCap;
extern int inRes;
extern int outRes;
extern int currIntRes;
extern int maxIntRes;
extern int inNod;
extern int outNod;
extern int currIntNod;
extern int maxIntNod;
extern int inTor;
extern int outTor;
extern int currIntTor;
extern int maxIntTor;
extern int inSubnod;
extern int currIntSubnod;
extern int maxIntSubnod;
extern int inSubtor;
extern int currIntSubtor;
extern int maxIntSubtor;
extern int outGrp;
extern int currIntGrp;
extern int maxIntGrp;
extern int outPrePassGrp;
extern int currNeqv;
extern int equiLines;
extern int areaNodes;
extern int areaNodesTotal;
extern int artReduc;

extern int inJun;
extern int outJun;
extern int currIntJun;
extern int inLBJT;
extern int inVBJT;
extern int outpLBJT;
extern int outpVBJT;
extern int currIntLBJT;
extern int currIntVBJT;
extern int inPolnode;
extern int outPolnode;
extern int currIntPolnode;
extern int warnConnect;
extern coor_t baseWindow;
extern bool_t parallelMerge;

extern int inSubTerm;
extern int outSubTerm;

extern bool_t substrRes;

extern int eliNod;

extern double totOutCap;

#ifdef HISTOGRAM
extern bool_t optHisto;
extern int * histogram;
extern int histoMaxVal;
extern int histoSize;
extern int histoBucket;
#endif /* HISTOGRAM */

extern int tileCnt;
extern int maxTermNumber;

extern int min_art_degree;
extern int min_degree;
extern double min_res;
extern double min_sep_res;
extern double max_par_res;
extern double min_coup_cap;
extern double min_coup_area;
extern double frag_coup_cap;
extern double frag_coup_area;
extern double min_ground_cap;
extern double min_ground_area;
extern bool_t no_neg_res;
extern bool_t no_neg_cap;

extern char *nameGND;
extern char *nameSUBSTR;

extern char *pos_supply[];
extern char *neg_supply[];
extern int no_pos_supply;
extern int no_neg_supply;

extern char **resSortTab;
extern int resSortTabSize;
extern char **capSortTab;
extern int capSortTabSize;
extern double *capOutFac;
extern char *capPolarityTab;
extern int *capAreaPerimType;
extern bool_t *capAreaPerimEnable;
extern int *capAreaPerimRelation;

#ifdef DEBUG_NODES
extern FILE *fp_dbn;
#endif

extern FILE * fp_in;
extern node_t *nqDelayList;
extern group_t *elim_grp;

extern subnode_t * subnGND;
extern subnode_t * subnSUB;
extern node_t * nSUB;

extern mask_t cNull;

extern int jun_caps;

#ifdef __cplusplus
  extern "C" {
#endif

/* elem.c */
#ifdef MOMENTS
double * newMoments (double * m);
#endif
element_c * elemAddCap (node_t *nA, node_t *nB, double val, int sortA, double *moments);
element_r * elemAddRes (int type, node_t *nA, node_t *nB, double val, int sortA);
void elemDelCap (element_c *el);
void elemDelRes (element_r *el);
element_c * findCapElement (node_t *nA, node_t *nB, int sortA);
group_t *mergeGrps (group_t *grA, group_t *grB);

/* elim.c */
void elim (node_t *n, int rx);
int testRCelim (node_t *n);
int updateWeight (node_t *n);

/* lump.c */
node_t * nodeJoin (node_t *nA, node_t *nB);
void nodeRelJoin (node_t *nA, node_t *nB);
void readyNode (node_t *n);
void supplyShort (void);

/* node.c */
void nqInit (void);
void nqInsert (node_t *n);
void nqChange (node_t *n);
void nqDelete (node_t *n);
void nqDumpQueue (void);
node_t *nqFirst (void);
node_t *nqNext (void);
void nqSetDegree (node_t *n);
int nqEliminateGroup (node_t **qn, int n_cnt);
int nqEliminateAreaNodes (node_t **qn, int n_cnt);
void nqDelayElim (node_t *n, int i);
void nqDelayNode (node_t *n);
void nqDelayNodes (void);
node_t *createNode (void);
void disposeNode (node_t *n);

/* out.c */
#ifdef DM_MAXNAME
void initOut (DM_CELL *cellCirKey, DM_CELL *cellLayKey);
#endif
void endOut (void);
void outGroup (node_t **qn, int n_cnt);
void outGndNode (void);
void outTransistor (transistor_t *t);
void outputReadyTransistor (polnode_t *pn, pnTorLink_t *tl);
void groupDel (group_t *grp);
void nodeDel (node_t *n);
void torDel (transistor_t *t);
#ifdef DEBUG_NODES
void debug_nodes_check (int i);
void debug_nodes_print (int i);
void printNode (FILE *fp, node_t *n);
#endif

/* read.c */
int readInput (void);
void inputError (void);
int getrec (void);

/* ready.c */
void readyGroup (node_t *n);

/* reduc.c */
int  reducGroupI (node_t **qn, int n_cnt);
void reducGroupII (node_t **qn, int n_cnt);
void reducRmNegCap (node_t **qn, int n_cnt);
#ifdef __cplusplus
  }
#endif
