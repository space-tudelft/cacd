/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
 *	S. de Graaf
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

#include "src/cmsk/incl.h"

extern struct stat st_buf;
extern DM_PROJECT *dmproject;
extern DM_CELL    *pat_key;
extern DM_STREAM  *fpbox, *fpmc, *fpinfo, *fpnor;

extern int makedatabase, makepattern,
    v_mode, wordsindex, int_res,
    mirrorflag, rotateflag, x_origin, y_origin,
    cxnumber, cxdistance, cynumber, cydistance,
    pat_box[4], totcallbox[4], callbox[4],
    totprogbox[4], progbox[4], temp1, temp2,
    start_x, start_y, y_previous,
    callpresent, progpresent,
    nrofmasks, masknr, nroflistmasks,
    mlist[MAXLISTMASKS][2],
    dest_mask, src1_mask, src2_mask,
    scanflag, line_width, dir_flag,
    n_edges, p_incr, cell_no;

extern double p_x[52], p_y[52], x1, x2, x3, x4, g_y1, y2, y3, y4,
    prev_grow, tmp1, tmp2, lw_tan225, lw_hsqrt2,
    d_f, d_i, resol, p_width, rad45;

extern char
    words[MAXWORDS][DM_MAXNAME+1],
    pat_buffer[DM_MAXNAME+1],
    *pat_name,
    *argv0;

extern char **ldmlay;
extern char **cmklay;
extern int   *layset;

extern struct ptree *tree_ptr;

extern struct list *mlistnames;
extern struct list *listptr;
extern struct list *endlistptr;

extern struct edge *mostleft, *mostright, *edgeptr, *growptr, *fmptr;
extern struct edge *lastedge, *fr_edge_structs;
extern struct edge **edges1[3];
extern struct edge **edges2[3];

extern struct scan *mostup, *mostdown, *scanptr;
extern struct scan *lastscan, *fr_scan_structs;

extern struct la_elmt *first_la_elmt, *last_la_elmt;

/* edge.c */
void give_edge_struct (void);
void ins_edge (int x, int ybottom, int ytop, int bodybit);
void free_edge (struct edge *ptr);
void ins_scan (int ybottom, int ytop, int state);
void free_scan (struct scan *ptr);
void sort_ins_edge (int x, int ybottom, int ytop, int bodybit);
void scan_edges (int flag);
void ln_start_edge (void);
void ln_stop_edge (void);
int mk_start_edge (void);
int mk_stop_edge (void);
int src_edge_exists (struct scan *ptr);
void comp_scans (void);
void grow (int delta);
void mk_boxes_of_edges (void);
void comp_pr_scans (void);
void pr_box (int xbottom, int xtop, int ybottom, int ytop);
void pr_boxes_from_edges (void);

/* func.c */
int roundX (double d);
int roundL (double d);
int roundh (double d);
char *d2a (double d);
void pr_pat_call (char *name);
void get_pattern      (char *name, struct edge **edgearray[3], IMPCELL *impcell);
void get_box_patterns (char *name, struct edge **edgearray[3], DM_PROJECT *proj, int impflag);
void mix_and_scan_and_grow (int flag, int growth);
void mk_all_boxes_of_edges (void);
void mk_mask_boxes_of_edges (int nr);
void invert_edges  (struct edge **edgearray[3]);
void setedgeptrto  (struct edge **edgearray[3], int nr);
void setarraytoptr (struct edge **edgearray[3], int nr);
void rm_array      (struct edge **edgearray[3]);
void placepoint (register int i);
void printonebox (void);
void pr_la_pattern (void);
void pr_rs_pattern (void);
void pr_po_pattern (void);
void pr_pa_pattern (void);
void pr_ci_pattern (void);
void pr_cp_pattern (void);
void cmp_fillbb (int ptr1[], int ptr2[], int ptr3[]);
void fillbb (int ptr1[], int ptr2[]);
void cl_boxes_etc (void);
void clean_bb (void);
void write_info (void);
int getmaskname (char *string);
struct list *getlistname (char *string);

/* main.c */
void pr_opt (int opt, char *str);
char *mpn (char *s);
void killpattern (void);
void init_pat_tree (void);
void sig_handler (int sig);
char *strsave (char *s1, char *s2);
void cant_alloc (void);
void die (void);

/* pat.c */
int check_tree (char *name);
int ck_ptree (char *name, struct ptree *root);
void add_pat_tree (void);
void add_ptree (struct ptree **root);
void mk_patdir (void);
void closepatdir (int mode);

/* proc_text.c */
void proc_text (int height, char *string);

/* cmsk_parse.y */
void upd_path_coords (int new_mode);
int  savename (char *s);
void yyerror (char *s);
int  yylex (void);
int  yyparse (void);
void pr_err1 (char *s1);
void pr_err1i (char *s1, int i);
void pr_err1f (char *s1, double d);
void pr_err2 (char *s1, char *s2);
void pr_err (char *s1, char *s2, char *s3);

/* cmsk_lex.l */
void skip_statement (char *s);
char *textval (void);
