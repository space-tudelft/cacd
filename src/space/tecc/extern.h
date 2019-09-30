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

extern mask_t cNull;

extern struct contact *cons;
extern struct transistor *tors;
extern struct capacitance *caps;
extern struct resistance *ress;
extern struct vdimension *vdms;
extern struct shape *shps;
extern struct bipoTor *bjts;
extern struct junction *juns;
extern struct connect *cnts;
extern struct dielectric *diels;
extern struct substrate *subcaps;
extern struct substrate *substrs;
extern struct selfsubdata *selfs;
extern struct mutualsubdata *muts;
extern struct newsubdata *newmsks;
extern struct subcont *subconts;
extern struct resizedata *resizes;
extern struct waferdata *wafers;
extern struct w_list *w_list_p;
extern struct w_list *w_list_n;
extern int bem_depth, bem_type;

extern double *diel_zq_values;
extern double *diel_zp_values;
extern double *diel_r_values;

extern double *subs_zq_values;
extern double *subs_zp_values;
extern double *subs_r_values;

extern struct newMaskRef *first_newmask_list;

extern int docompress;
extern int prVerbose;
extern int printNonCoded;
extern int silent;
extern int v_flag;

extern int con_cnt;
extern int tor_cnt;
extern int cap_cnt;
extern int res_cnt;
extern int vdm_cnt;
extern int waf_cnt;
extern int fem_cnt;
extern int fem_res_cnt;
extern int shp_cnt;
extern int bjt_cnt;
extern int jun_cnt;
extern int cnt_cnt;
extern int diel_cnt;

extern int diel_zq_values_cnt;
extern int diel_zp_values_cnt;
extern int diel_r_values_cnt;

extern int subs_zq_values_cnt;
extern int subs_zp_values_cnt;
extern int subs_r_values_cnt;

extern int diel_grid_count;
extern int subs_grid_count;

extern int diel_num_annealing_iterations;
extern int subs_num_annealing_iterations;

extern double subs_neumann_simulation_ratio;

extern double diel_max_determinant_binning_error;
extern double subs_max_determinant_binning_error;

extern double diel_max_adjoint_binning_error;
extern double subs_max_adjoint_binning_error;

extern double diel_max_annealed_inverse_matrix_binning_error;
extern double subs_max_annealed_inverse_matrix_binning_error;

extern double diel_max_preprocessed_annealing_matrices_binning_error;
extern double subs_max_preprocessed_annealing_matrices_binning_error;

extern double diel_max_reduce_error;
extern double subs_max_reduce_error;

extern double diel_max_annealing_error;
extern double subs_max_annealing_error;

extern double diel_r_switch;
extern double subs_r_switch;

extern int subcap_cnt;
extern int substr_cnt;
extern int self_cnt;
extern int mut_cnt;
extern int new_cnt;
extern int sbc_cnt;
extern int resize_cnt;

extern DM_PROCDATA *procdata;
extern struct submasks *subdata;

extern char ** maskdrawcolor;
extern char ** masknewcolor;
extern char *  masksubcolor;

extern int conducCnt;
extern int conducCntStd;
extern int conducCntPos;
extern int *conducTransf;
extern int *maskTransf;

extern int sSlotCnt, sSlotCnt2, eSlotCnt, oSlotCnt;

extern int nbrLays;
extern int maxNbrKeys, maxNbrKeys2, maxEdgeKeys;
extern int maxKeys, maxKeys2;
extern int maxprocmasks;

extern struct elemRef **keytab;
extern struct elemRef **keytab2;
extern int nbrKeySlots;
extern int nbrKeySlots2;

extern struct layerRef *Keylist;
extern struct layerRef *keylist;
extern struct layerRef *keylist2;

/* change.c */
void changeMasks (void);

/* check.c */
void checkConnections (void);

/* findcond.c */
void findConducTransf (void);

/* lex.l */
int yylex (void);
void yyerror (const char *s);

/* main.c */
#if defined (__cplusplus) && defined (YYSTATE)
int main (int argc, char *argv[]);
#endif
void fatalErr   (const char *s1, const char *s2);
void warningMes (const char *s1, const char *s2);
void die (void);

/* mkcapbit.c */
void mkCapBitmasks (void);

/* mkkeys.c */
void mkKeys (void);
int condcmp (struct layCondRef *c1, struct layCondRef *c2);

/* parse.Y */
void doparse (char *filename);
int resizemask (int mask);
void set_profile_subcont (void);

/* prtabs.c */
void prTabs (char *outfile);

/* selkeys.c */
void selectKeys (void);

/* tclexport.C */
char *maskname (int i);
void exportToTcl (FILE *fp);

/* debug.c */
void printcl (struct layCondRef *cond);

#ifdef __cplusplus
  }
#endif
