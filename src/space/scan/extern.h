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

#ifdef __cplusplus
  extern "C" {
#endif

/* ../space/date.c */
void compileDate (char *s);

/* ../extract/enumpair.c */
void missingTermCon (terminal_t *term);

/* update.c */
void tileInsertEdge  (edge_t *edge);
void tileDeleteEdge  (edge_t *edge);
void tileCrossEdge   (edge_t *edge, int split);
void tileAddTerm     (edge_t *edge, coor_t termY);
void tileAdvanceScan (edge_t *edge);
void tileStopScan    (edge_t *head);

/* edge.c */
edge_t *createEdge (coor_t xl, coor_t yl, coor_t xr, coor_t yr);
void disposeEdge (edge_t *edge);
void printEdge (char *s, edge_t *edge);
void edgeStatistics (FILE *fp);

/* input.c */
#ifdef DM_MAXNAME
void openInput (DM_CELL *cellKey);
#endif
void closeInput (void);
edge_t *fetchEdge (void);
terminal_t *fetchTerm (void);

/* main.c */
int main (int argc, char *argv[]);
void die (void);
void quit (void);
void convertHierName (char *name);
char *giveICD (char *filepath);

/* tile.c */
tile_t *createTile (coor_t xl, coor_t bl, coor_t xr, coor_t br, tile_t *stb, tile_t *stl, int cc);
void disposeTile (tile_t *tile);
void printTile (char *s, tile_t *tile);
void tileStatistics (FILE *fp);

/* scan.c */
void setContext (char *c, coor_t l, coor_t r);
void catchAlarm (void);
void scan (void);

/* slant.c */
coor_t calcY (edge_t *e, coor_t x);
void testIntersection (edge_t *e1, edge_t *e2);
void split (edge_t *e1);

/* getparam.c */
#ifdef DM_MAXNAME
char * getParameters (DM_PROJECT *dmproject, char *libFile, char *userFile);
#endif
void lookupParameters (void);
void reGetParameters (void);

/* hier.c */
#ifdef DM_MAXNAME
void readTid (DM_CELL *layoutKey, DM_CELL *circuitKey);
#endif
void disposeTid (void);
void newTerminal (termtype_t type, int conductor, coor_t x, coor_t y,
    char *tName, char *iName, int ix, int iy, int tx, int ty);
void addTerminalNames (void);
int compareByName (const void * e1, const void * t2);

/* info.c */
void scanPrintInfo (FILE *fp);

/* determ.c */
#ifdef DM_MAXNAME
bool_t existDmStream (DM_CELL *key, char *streamName);
#endif

/* From other places !!! */
extern char * argv0;
extern bool_t paramCapitalize;
extern double max_tan_slice_y;
extern double physBandWidth;
extern double equiBandWidth;
extern coor_t baseWindow;
extern double max_lat_base;
extern bool_t parallelMerge;
extern int hasSubmask;
extern int nrOfCondStd;
extern int nrOfMasks;
extern int nrOfTerminals;
extern int doTileXY;
extern mask_t cNull;
extern mask_t resBitmask;
extern maskinfo_t * masktable;
extern terminal_t ** TERM;
extern terminal_t ** TERMBYNAME;
extern char ** LABELNAME;
extern int nrOfLabelNames;
extern int nrOfTermNames;

#ifdef __cplusplus
  }
#endif
