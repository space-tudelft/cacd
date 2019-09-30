/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include "src/makeboxl/define.h"

extern struct clist *celllist;
extern struct clist *clp_image;
extern struct clist *topcell;
extern struct tmtx  *tm_p, *tm_s;

extern DM_PROCDATA *process;
extern DM_STREAM  **fp_bxx;
extern DM_STREAM  **fp_nxx;
extern DM_STREAM   *fp_tid;
extern DM_STREAM   *fp_spec;
extern DM_STREAM   *fp_tidpos;
extern DM_STREAM   *fp_tidnam;
extern DM_STREAM   *fp_anno_exp;
extern DM_CELL     *cellkey;
extern DM_PROJECT  *project;
extern double      *px, *py;
extern double       rad01, rad90;

extern char   buf[];
extern char  *cellname;
extern char  *argv0;

extern int    no_masks;
extern long  *no_bxx;
extern long   exp_reg[];
extern int    part_exp;
extern int    dflag;
extern long   exp_depth;
extern long   term_no;
extern int    mask_no;
extern int    t_mask_no;
extern int    level;
extern int    s_mode;
extern long   samples;
extern int    pt_size;
extern int    Lflag;
extern int    flat_mode;
extern int    hier_mode;
extern int    pseudo_hier_mode;
extern int    lookatdevmod;
extern int    extraStreams;
extern int    noErrMes;
extern int    Image_done;
extern int    verbose;

extern char * imageName;

/* auxil.c */
#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr);
void pr_clist (struct clist *clp);
#endif

/* errexit.c */
void errexit (int err_no, char *s);
void die (void);

/* expbox.c */
void exp_box (long Xl, long Xr, long Yb, long Yt);

/* expimage.c */
void write_image_bxx (void);
void write_image_nxx (void);

/* expmodel.c */
int  notEmpty (DM_CELL *ck, char *name, int err);
void exp_cell (char *cell);
DM_STREAM * open_bxx (int i, int j);
DM_STREAM * open_nxx (int i);

/* readbox.c */
void read_box (void);

/* readhtm.c */
void read_hier_term (struct clist *clp);

/* readnor.c */
void read_nor (void);
void read_nor2 (void);

/* readterm.c */
void read_term (struct clist *clp);

/* sexp_box.c */
void s_exp_box (long Xl, long Xr, long Yb, long Yt);

/* trmctree.c */
char *strsave (char *s);
void trav_mctree (struct clist *pcl, int image_done);
