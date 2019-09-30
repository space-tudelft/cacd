/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
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

#include "src/flatten/define.h"

extern struct clist *celllist;
extern struct tmtx  *tm_p, *tm_s;
extern struct name_tree *tnam_tree;

extern DM_CELL     *cellkey;
extern DM_PROCDATA *process;
extern DM_PROJECT  *project;
extern DM_STREAM   *fp_box, *fp_info, *fp_mc, *fp_nor, *fp_term, *fp_anno;
extern FILE        *fp_ldm;
extern double       rad01, rad90;

extern char  *argv0;
extern char  *ldmfile;

extern long   elbb_xl, elbb_xr, elbb_yb, elbb_yt;
extern long   mcbb_xl, mcbb_xr, mcbb_yb, mcbb_yt;

extern int    dmErrorOFF;
extern int    exp_depth;
extern int    level;
extern int    Lflag;
extern int    pflag;
extern int    tflag;
extern int    verbose;

/* flat_cell.c */
void flat_cell (char *cname, int Vnr);
int append_tree (char *name, struct name_tree **head);

/* read_box.c */
void read_box (void);
void read_term (void);

/* read_mc.c */
void read_mc (void);
void exp_mc (void);

/* read_nor.c */
void newpxpy (int size);
void read_nor (void);

/* services.c */
void pr_err (int err_no, char *s);
void sig_handler (int sig);
void die (void);
#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr);
void pr_clist (struct clist *clp);
#endif

/* tra_mctree.c */
void trav_mctree (struct clist *pcl);
