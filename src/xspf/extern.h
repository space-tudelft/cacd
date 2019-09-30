
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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

/* findFunc.c */
struct model_info *findFunc (char *name, int imported, DM_PROJECT *father_proj, int flag);
void prImpFunc (void);

/* nextAttr.c */
char *nextAttr (char **p, char **v, char *a);
char *getAttrValue (char *a, char *par);
double getAttrFValues (char *a, char *par);

/* prFoot.c */
void prFoot (struct model_info *ntw, int submod);

/* cirTree.c */
struct cir *cirTree (char *name);
void scanInst (char *name, DM_PROJECT *proj);
int is_func (void);
IMPCELL *dmGetImpCell (DM_PROJECT *proj, char *name, int msg);

/* findNetw.c */
struct model_info *findNetw (char *name, int imported, DM_PROJECT *father_proj, int submod);
struct model_info *newNetw (char *name, DM_PROJECT *proj, int imported, char *orig_name, int submod);
void termRun (struct model_info *m, DM_STREAM *dsp);
void prImpNetw (void);
void prImpMod (struct model_info *m, char *str);

/* findDev.c */
void initDevs (void);
struct model_info *findDev (char *name, int imported, DM_PROJECT *proj);
char *findDevType (char *type);
void termstore (struct model_info *m, char *name, int dim, int *lower, int *upper, int type);
void endDevs (void);
void prImpDev (void);
void interDevs (void);
double slstof (char *s, int len);
char *strsave (char *s);

/* findNet.c */
void hashInit (void);
void hash (struct net_el *n2, struct net_el *n);
struct net_el *findInst (char *inst_name);
struct net_el *findNet (struct net_el *eqv);
struct net_el *findNetSub (struct net_el *eqv);
void findNetRC (struct net_el *eqv, double val);
void addNetSub (struct net_el *eqv, int nx);

/* lex.l */
int yylex (void);
int yywrap (void);
void scanrestart (FILE *input_file, int nr);
char *textval (void);

/* main.c */
int isCurrentDialect (char *buf);
void fatalErr (char *s1, char *s2);
void die (void);
void cannot_die (int nr, char *fn);
void spefOpenNetwork (char *name);

/* parse.y */
int yyparse (void);

/* prHead.c */
void prHead (struct model_info *ntw, int submod);

/* prInst.c */
void prInst1 (struct model_info *ntw, struct net_ref *nets);
void prInst  (struct model_info *ntw, struct net_ref *nets);
void prInst3 (struct model_info *ntw, struct net_ref *nets);
char *fvalPar (char *par, char *val);
int is_ap (void);

/* prNets.c */
void prNets (struct model_info *ntw, struct net_ref *nets);

/* print.c */
void nmprint (int concat, char *name, int dim, int *lower, int *upper, int pr_range);
long nameNbr (char *name);
long testNameNbr (char *name);
int  testNbr (long nr);
long assignNameNbr (char *name, long nr);
void nameNbrReset (void);
void oprint (int concat, char *s);
int outPos (void);
char *projname (DM_PROJECT *proj);
int test0 (char *name);

/* xnetwork.c */
void xnetwork (char *ntwname, DM_PROJECT *proj, int imported, char *orig_name, int submod);
char *newStringSpace (char *s);
int  *newIndexSpace (int cnt);

/* model.c */
struct lib_model *createModel (struct model_info *mod);
void printModels (void);

/* eqnHand.c */
char * fvalEqn (char * eqn, double geom[]);

/* ctrlFile.c */
void readControl (void);
char *maplibname (char *s);
void ctrlFileErr (char *s1, char *s2);

/* nodetab.c */
void createNodeTab (int non);
void deleteNodeTab (void);
void addAP (int N_nx, char *dev_name, double area, double perim);
int  getAP (int N_nx, char *dev_name, double *area, double *perim, int *cnt, double *width);
void addCntWidth (int N_nx, char *dev_name, double width);

#ifdef __cplusplus
  }
#endif
