
/*
 * ISC License
 *
 * Copyright (C) 1994-2013 by
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
void readTerm (struct model_info *m, int funf, int submod);
void termRun (struct model_info *m, DM_STREAM *dsp);
void prImpNetw (void);
void prImpMod (struct model_info *m, char *str);

/* findDev.c */
void initDevs (void);
struct model_info *findDev (char *name, int imported, DM_PROJECT *proj);
char *findDevType (char *type, char *fname);
char *getDevType (char *type);
void termstore (struct model_info *m, char *name, long dim, long *lower, long *upper, int type);
void endDevs (void);
void prImpDev (void);
void interDevs (void);
char *strsave (char *s);
char *renameDev (char *name);
void checkFunc (DM_PROJECT *proj, struct model_info *fun);

/* findNet.c */
void findNetInit (struct model_info *ntw, struct net_ref *nets);
struct net_el *findNet (struct net_el *eqv);

/* lex.l */
int yylex (void);
int yywrap (void);
void scanrestart (FILE *input_file, int nr);

/* main.c */
int isCurrentDialect (char *buf);
void fatalErr (char *s1, char *s2);
void die (void);
void cannot_die (int nr, char *fn);

/* parse.y */
int yyparse (void);

/* prHead.c */
void prHead (struct model_info *ntw, int submod);

/* prInst.c */
void prInst (struct model_info *ntw, struct net_ref *nets);
char *fvalPar (char *par, char *val);
int is_ap (void);

/* prNets.c */
void prNets (struct model_info *ntw, struct net_ref *nets);

/* print.c */
void nmprint (int concat, char *name, long dim, long *lower, long *upper, int pr_range, int pr_nbr);
long nameNbr (char *name);
long testNameNbr (char *name);
long assignNameNbr (char *name, long nr);
void nameNbrReset (void);
void ntabprint (void);
void startComment (void);
void oprint (int concat, char *s);
int outPos (void);
void destroyOutBuf (void);
void flushOutBuf (void);
char *projname (DM_PROJECT *proj);
int test0 (char *name);

/* xnetwork.c */
void xnetwork (char *ntwname, DM_PROJECT *proj, int imported, char *orig_name, int submod, int listnames);
char *newStringSpace (char *s);
long *newIndexSpace (long cnt);

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
void createNodeTab (long non);
void deleteNodeTab (void);
void addAP (long N_nx, char *dev_name, double area, double perim);
int  getAP (long N_nx, char *dev_name, double *area, double *perim, long *cnt, double *width);
void addCntWidth (long N_nx, char *dev_name, double width);

#ifdef __cplusplus
  }
#endif
