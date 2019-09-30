/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/makeboxh/define.h"

extern struct ctree   *celltree;
extern struct cptrl   *cptrhead;
extern struct cptrl   *cptrlast;
extern struct cptrl   *cptr;
extern struct wdw     *mc_bboxl;
extern struct wdw     **arl_ptr;
extern DM_PROCDATA *process;
extern DM_STREAM  **fp_bxx;
extern DM_STREAM   *fp_tid;
extern DM_CELL     *cellkey;
extern DM_CELL     *top_key;
extern DM_PROJECT  *project;

extern char   fn_exp[];
extern char  *argv0;
extern char   fname[];

extern long  *no_bxx;
extern long   exp_reg[4];
extern int    part_exp;
extern long   exp_depth;
extern long   usr_chlev;
extern long   term_no;
extern long   chtype;
extern long   fchtype;
extern long   maxol;
extern int    mask_no;
extern int    level;

/* aux.c */
void olap (long xl, long xr, long yb, long yt, long *wdw2, long *owdw);
#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr);
void pr_ctree (struct ctree *ptr);
void pr_cptrl (struct cptrl *ptr);
void pr_wdwl (struct wdw *ptr, char *str);
void pr_wdw (struct wdw *ptr);
#endif

/* errexit.c */
void errexit (int err_no, char *cs);
void die (void);

/* exp_box.c */
void exp_box (struct mc_elmt *pmc, long xl, long xr, long yb, long yt);

/* exp_model.c */
void exp_cell (struct ctree *cell, int partial);

/* mak_actreg.c */
void make_actreg (struct mc_elmt *pmc);
void exp_wdw (struct mc_elmt *pmc, long xl, long xr, long yb, long yt);

/* read_box.c */
void read_box (struct mc_elmt *pmc);

/* read_mc.c */
void read_mc (struct mc_elmt *pmc);
int exp_mcwdw (struct mc_elmt *pmc, long xl, long xr, long yb, long yt);
void add_mc_olap (void);
void add_arl (long xl, long xr, long yb, long yt);

/* read_term.c */
void read_term (struct mc_elmt *pmc);

/* tra_mctree.c */
void trav_mctree (struct mc_elmt *pmc, DM_PROJECT *pkey);

/* upd_motree.c */
void upd_ctree (struct ctree *cell, char *name);
struct ctree *upd_cptrl (char *name);
