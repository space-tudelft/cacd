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

/* a pseudo type for numbering partitions (block tables) */
#define btnum_t int32

#ifdef __cplusplus
  extern "C" {
#endif

/* extern variables */
extern coor_t bxl, byb, bxr, byt;
extern bool_t optVerbose;
extern bool_t optDelete;
extern bool_t optCompress;
extern bool_t optInfo;
extern bool_t optSortOnly;
extern bool_t optNoOutput;
extern bool_t optSpecial;
extern size_t optMaxMemory;
extern int    scale;
extern char * argv0;
extern double infoPartitionBalance;
extern btnum_t infoNumPartitions;

/* library */
double drand48 (void);

/* edge.c */
edge_t *createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign);
void disposeEdge (edge_t *edge);
edge_t *newChain (void);
void printEdge (char *s, edge_t *edge);

/* gln.c */
void glnUpdate (edge_t *edge, coor_t x);

/* input.c */
#ifdef DM_MAXNAME
void readEdges (DM_CELL *cellKey, char *mask, int masktype, int scaleFactor);
#endif

/* main.c */
int  main (int argc, char **argv);
void die (void);

/* output.c */
#ifdef DM_MAXNAME
void openOutput (DM_CELL *cellKey, char *mask);
#endif
void closeOutput (void);
void selectForOutput (edge_t *edge);
void readyForOutput (edge_t *edge);
void scanAdvance (void);
void outputEdge (edge_t *e);
void outputCleanUp (void);
void outputPrintInfo (FILE *fp);

/* pqueue.c */
void pqInit (int32 N);
void pqHead (_edge_t **ep, int32 *np);
void pqInsert (_edge_t *edge, int block);
void pqReplaceHead (_edge_t *edge, int block);

/* qsort.c */
void sortBlock (_edge_t **base, int32 N);

/* scan.c */
void scan (void);
void scanPrintInfo (FILE *fp);

/* slant.c */
void testIntersect (coor_t x, edge_t *e1, edge_t *e2);
edge_t *split (edge_t *e1, coor_t xi, coor_t yi);

/* sort.c */
void sortEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr, slope_t slope, sign_t sign);
edge_t *fetchEdge (void);
void sortPrintInfo (FILE *fp);

#ifdef __cplusplus
  }
#endif
