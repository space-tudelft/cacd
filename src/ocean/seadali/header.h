/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	H.T. Fassotte
 *	Simon de Graaf
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

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

#undef MALLOC
#undef MALLOCN
#undef FREE

#define ULEFT  1
#define URIGHT 2
#define LLEFT  3
#define LRIGHT 4

#define LEFT   0
#define RIGHT  1
#define CENTER 2

#if DM_MAXNAME >= 100
#define MAXCHAR DM_MAXNAME+1
#define MAXSTR  DM_MAXNAME+1
#else
#define MAXCHAR 100
#define MAXSTR  100
#endif

#define LIST_LENGTH 20

#define MaxPolyPoints 10000

#define NO_SNAP	0
#define SNAP	1

#ifdef DELETE
#undef DELETE
#endif
#define ADD         0
#define DELETE      1

#define SKIP        0
#define DRAW        1
#define ERAS_DR     2

#define DOMINANT    0
#define TRANSPARENT 1
#define ERASE       2
#define COMPLEMENT  3

#define QUAD_LAMBDA 4

#define FILL_SOLID  0
#define FILL_HASHED 1
#define FILL_HOLLOW 2
#define FILL_HASHED12B 3
#define FILL_HASHED25B 4
#define FILL_HASHED50B 5
#define FILL_HASHED25 6
#define FILL_HASHED50 7

#define LINE_SOLID  0
#define LINE_DOTTED 1
#define LINE_DOUBLE 2

#define EXP_LEV    9
#define MAX_LEV   16

#define READ_TERM  0
#define READ_ALL   1

#define SUB_TRAP   0
#define ROOT_TRAP  1

#define TRUE    1
#define FALSE   0

#define MENU    2
#define PICT    3
#define TEXT    5
#define LAYS    6

#define XL c_wdw->wxmin
#define XR c_wdw->wxmax
#define YB c_wdw->wymin
#define YT c_wdw->wymax

#define PE fprintf(stderr,

#define Abs(a)   ((a) < 0 ? -(a) : (a))
#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define Round(x) (((x) > 0) ? ((x)+0.5) : ((x)-0.5))
#define LowerRound(x) (((x) < 0.0) ? ((x) - 1.0) : (x))
#define UpperRound(x) (((x) > 0.0) ? ((x) + 1.0) : (x))

#define MALLOC(ptr,type) { if (!(ptr = (type *) malloc (sizeof (type)))) pr_nomem (); }

#define MALLOCN(ptr,type,num) { if (!(ptr = (type *) malloc ((unsigned) (num) * sizeof (type)))) pr_nomem (); }

#define FREE(ptr) free ((void *) (ptr))

#ifdef ED_DEBUG
#define ASSERT(ex) { if (!(ex)) print_assert (__FILE__, __LINE__); }
#else
#define ASSERT(ex)
#endif

struct Disp_wdw {
    int     w_nr; /* window number */
    float   vxmax, vymax; /* boundary of viewport in virtual coordinate system */
    float   wxmin, wxmax, wymin, wymax; /* real-world coordinates of viewport */
};

typedef long Coor;
typedef long Trans;

/* data structure for linked list */

struct obj_node {
    struct obj_node *next; /* pointer to the next node */
    Coor ll_x1, ll_x2, ll_y1, ll_y2; /* bbox of the trapezoids */
    short  leftside;	/* slope of the left side */
    short  rightside;	/* slope of the right side */
    short  mark;	/* mark flag */
};

/* data structure for linked list of pointers to trapezoids */

struct found_list {
    struct obj_node *ptrap;
    struct found_list *next;
};

struct ref_node {
    struct ref_node *next; /* pointer to the next node */
    struct obj_node *ref;  /* pointer to an object */
};

/* data structure for quad tree nodes */

struct qtree {
    struct qtree *Uleft, *Uright, *Lleft, *Lright; /* pointers to the 4 child nodes */
    struct ref_node *reference; /* pointer to the reference list */
    struct obj_node *object;    /* pointer to the object list */
    short  Ncount;              /* nr of objects in object list */
    Coor   quadrant[4];         /* quadrant of this tree node */
};

typedef struct qtree qtree_t;

struct ylist {
    struct ylist *next;
    Coor yval;
};

typedef struct term {
    Coor   xl, xr, yb, yt;
    char  *tmname;
    Coor   dx, dy;
    int    nx, ny;
    struct term *nxttm;
} TERM;

typedef struct inst {
    Coor   bbxl, bbxr, bbyb, bbyt;
    char  *inst_name;
    int    imported;
    Trans  tr[6];
    Coor   dx, dy;
    int    nx, ny;
    int    level;	/* expansie level */
    int    t_draw;	/* draw terminals of instance */
    struct templ *templ;
    struct inst  *next;
} INST;

typedef struct templ {
    char  *cell_name;
    DM_PROJECT *projkey;
    qtree_t **quad_trees;
    int    t_flag;	/* 1 -> terminals gelezen */
    TERM **term_p;
    Coor   bbxl, bbxr, bbyb, bbyt;  /* patrick: added bbx crd of inst */
    struct templ *next;
    struct inst  *inst;
} TEMPL;

struct err_pos {
    int    p_nr;
    char  *p_str;
    float  x1_plot, y1_plot, x2_plot, y2_plot;
    Coor   x1, y1, x2, y2;
    struct err_pos *next;
};

struct edges {
    int   *start_edges;
    int   *stop_edges;
    int    d_start;
    int    d_stop;
    struct edges *next;
    struct edges *prev;
};

/* data structures for interactive checker */

struct drc {
    int overlap;
    int gap;
    int exgap;
    int exlength;
};

/* data structures for x-scan (used by "make_true_edges") */
/* x_lst is a double linked list */

struct x_lst {
    struct x_lst *next;	/* pointer to next element in list */
    struct x_lst *back;	/* pointer to previous element in list */
    Coor   xs;		/* x start value of edge */
    Coor   ys;		/* y start value of edge */
    Coor   xe;		/* x end value of edge */
    Coor   ye;		/* y end value of edge */
    short  dir;		/* direction of edge */
    short  side; 	/* edge orientation */
    short  lower;	/* lower edges */
    int    layer;	/* mask layer of edge */
};

struct f_edge {
    float  xs;		/* x start value of edge */
    float  ys;		/* y start value of edge */
    float  xe;		/* x end value of edge */
    float  ye;		/* y end value of edge */
    short  dir;		/* direction of edge */
    short  side; 	/* edge orientation */
};

/* data structure for binary tree */

struct bintree {
    struct bintree *left;  /* pointer to left part */
    struct bintree *right; /* pointer to right part */
    struct bintree *link;  /* link pointer for linked list */
    Coor   value;
};

/* data structure for binary edge tree */

struct edgetree {
    struct edgetree *left;  /* pointer to left part */
    struct edgetree *right; /* pointer to right part */
    struct x_lst *edge;     /* pointer to list of edges */
};

struct p_to_edge {
    struct p_to_edge *next;
    struct x_lst *edge;
};

/* add_poly.c */
int add_del_poly (short mode);
void draw_poly_line (Coor line[], int min_index, int max_index, int mode);

/* add_quad.c */
void addel_cur (Coor xl, Coor xr, Coor yb, Coor yt, int ad_mode);
void add_new_trap (int lay, Coor xl, Coor yb, Coor xr, Coor yt, short lside, short rside);
void add_quad (qtree_t *quad_array[], struct obj_node *p, int lay);

/* add_wire.c */
void add_wire (Coor stack_x[], Coor stack_y[], int npoints, Coor width, int non_orth, int wire_ext);

/* animate.c */
void load_rc_files (void);

/* array.c */
int ask_par (char ***arr_p_p);
int nx_ny_par (int *nx_ny, Coor *dx_dy, Coor dd, char *obj_name);
int dx_dy_par (Coor *d_p, int n_p, Coor margin, int comm_par);
void arr_par (void);

/* bintree.c */
struct bintree *link_bintree (struct bintree *p);
struct x_lst   *link_edgetree (struct edgetree *p);
struct bintree *mk_bintree (Coor value);
struct edgetree *mk_edgetree (struct x_lst *edge);
void to_bintree (struct bintree *p, Coor val);
void to_edgetree (struct edgetree *p, struct x_lst *edge);
void cl_edgetree (struct edgetree *p);

/* bound.c */
void upd_boundb (void);

/* buffer.c */
void fill_buffer (Coor x1, Coor x2, Coor y1, Coor y2, short all_mode);
struct obj_node *out (Coor x1, Coor x2, Coor x3, Coor x4, Coor ymin, Coor ymax, struct obj_node *list);
void put_buffer (int termflag);
void q_found (struct obj_node *p);
void quicksort (Coor a[], int l, int r);

/* check.c */
void area_check (void);

/* clip.c */
struct obj_node *clip (struct obj_node *p, struct obj_node *q, struct obj_node *clip_head);

/* command.c */
void ask_quit (int);
void command (void);
void bulb_dominant (void);
void bulb_hashed (void);

/* comment.c */
void c_label (Coor x, Coor y, int orient);
void c_line (Coor x1, Coor y1, Coor x2, Coor y2, int arrow_mode);
void c_text (Coor x, Coor y, int orient);
int  comment_win (Coor *ll, Coor *rr, Coor *bb, Coor *tt);
void del_comment (Coor x, Coor y);
void draw_all_comments (void);
void empty_comments (void);
int  inp_comment  (DM_CELL *ckey);
int  outp_comment (DM_CELL *ckey);
int  no_comments (void);

/* del_quad.c */
void del_box (int lay, Coor xl, Coor yb, Coor xr, Coor yt);
void del_traps (struct obj_node *p, int lay);

/* display.c */
void dis_s_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void dis_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void pict_mc (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void mc_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void term_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void trans_box (Coor arr1[], Coor arr2[], INST *inst_p, int nnx, int nny);

/* disp_mask.c */
void clip_paint (struct obj_node *p, Coor wxmin, Coor wxmax, Coor wymin, Coor wymax, Trans *Mtx, int mode);
void disp_mask_quad (int mask, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void paint_trap (struct obj_node *p);

/* distance.c */
float dis (float x1, float y1, float x2, float y2);
int  distance (struct f_edge *p, struct f_edge *q, struct f_edge *sh_d);

/* dsgn_dat.c */
void r_design_rules (void);

/* edge_t_ch.c */
void sub_to_checker  (struct obj_node *p);
void trap_to_checker (struct obj_node *p);
void to_precheck (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax);

/* expand.c */
void exp_templ (TEMPL *templ_pntr, DM_PROJECT *fath_projkey, int imported, short mode);
void expansion (int level);
void indiv_exp (Coor x1, Coor y1);

/* ext_check.c */
void check (void);
void chk_file (void);
void disp_next (int inside_window);
void draw_drc_err (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void empty_err (void);
void ind_err (Coor x_cur, Coor y_cur);
void set_flat_expansion (int aVal);
void toggle_drc_err (void);

/* fish.c */
void do_autoroute (int verify_only);
void do_fish (char *option);
void do_madonna (int route_also);
void do_print (void);
void start_browser (char *subject);
void xfilealert (int type, char *fname);

/* get.c */
int  get_all (int echo, Coor *x, Coor *y);
void get_cmd (void);
int  get_com (void);
int  get_line_points (int num, int snap);
int  get_one (int echo, Coor *x, Coor *y);
int  get_wire_point (int echo, Coor *x, Coor *y, int width, int non_orth);
int  set_tbltcur (int num, int snap);

/* grid.c */
void disp_axis (void);
void display_grids (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
int  grid_on (void);
void no_grid_adjust (void);
void set_gr_spac (void);
void set_grid_values (int no_grd_sp, int *spc_list);
void set_grid_width (int grid_width);
void switch_grid (int show_it);
void toggle_grid (void);

/* inform.c */
void inform_act_inst (void);
void inform_act_term (void);
void inform_cell (void);
void inform_process (void);
void inform_window (void);

/* input.c */
void eras_worksp (void);
int  inp_boxnorfl (DM_CELL *key, qtree_t *quad_el[]);
void inp_mod  (char *c_name);
int  inp_mcfl (DM_CELL *key, INST **inst_pp);
int  inp_term (DM_CELL *key, TERM **term_pp);
int  read_bound_box (DM_PROJECT *proj_key, char *cell_name, Coor *bxl, Coor *bxr, Coor *byb, Coor *byt);

/* insert.c */
struct obj_node *insert (struct obj_node *new_list, int lay);
int  merge (struct obj_node *a, struct obj_node *b);
int  test_trap (struct obj_node *p);
struct ylist *yinsert (struct ylist *root, Coor value);

/* inst_draw.c */
void draw_inst_window (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void paint_sub_trap (struct obj_node *p);

/* mask.c */
void bulb (int lay, int flag);
void disable_masks (int no_masks, int *mask_array);
void init_colmenu (void);
void next_colmenu (void);
void prev_colmenu (void);
void init_mskcol (void);
void link_masks (int no_masks, int *mask_array);
int  msk_nbr  (char lc[]);
void Rmsk (void);
void Visible (void);
void paint_lay (int nbr);

/* memo.c */
void clear_templ (TEMPL *templ_p);
void empty_mem (void);
void init_mem (void);
int  no_works (void);

/* menu.c */
int  ask  (int nr_alt, char *alt_arr[], int old);
void menu (int nr_alt, char *alt_arr[]);
void Rmenu (void);
void post_cmd_proc (int nbr, char *names[]);
void pre_cmd_proc  (int nbr, char *names[]);

/* mod.c */
int  add_inst (int local_imported);
INST *create_inst (INST **inst_pp, char *cellname, char *inst_name, int imported,
    Trans tr0, Trans tr1, Trans tr3, Trans tr4, Trans tx, Trans ty, Coor dx, int nx, Coor dy, int ny);
void del_inst (Coor x1, Coor y1);
int  inst_outside_window (INST *inst_p, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void inst_window (INST *inst_p, Coor *ll, Coor *rr, Coor *bb, Coor *tt);
INST *search_inst (Coor x1, Coor y1);

/* modl.c */
char *ask_cell (int local_imported);
int  read_impclist (void);
int  read_modlist (void);

/* mpar.c */
void mir_inst (int implicit);
void mov_inst (Coor x1, Coor y1);
void rot_inst (void);
void set_actinst (Coor x1, Coor y1);
void upd_inst_bbox (void);
void mov_insts (double xlmul, double ybmul);

/* output.c */
int  outp_bbox (DM_CELL *key);
int  outp_boxfl (DM_CELL *key);
int  outp_mcfl (DM_CELL *key);
int  outp_term (DM_CELL *key);
void store (struct obj_node *p);
void wrte_cell (void);

/* picture.c */
void pic_max (void);
void pict_all (int mode);
void picture (void);
void set_alarm (int switchon);
int  stop_drawing (void);

/* poly_trap.c */
struct obj_node *poly_trap (Coor line[], int npoints);
int  trap_to_poly (Coor line[], struct obj_node *p);

/* precheck.c */
void edge_insert (struct x_lst *new);
void make_true_edges (int act_lay);
void y_insert (Coor value, int layer);

/* q_build.c */
qtree_t *make (qtree_t *Q, short quadrant);
qtree_t *qtree_build (void);
void quad_build (qtree_t *Q);

/* q_clear.c */
void quad_clear (qtree_t *Q);

/* q_delete.c */
void quad_delete (qtree_t *Q, struct obj_node *srch);

/* q_insert.c */
qtree_t *quad_insert (struct obj_node *p, qtree_t *Q);
void subquad_insert  (struct obj_node *p, qtree_t *Q);

/* q_search.c */
void quad_search (qtree_t *Q, struct obj_node *srch, void (*function)());
struct found_list *quick_search (qtree_t *Q, struct obj_node *srch);

/* real_main.c */
void fatal (int Errno, char *str);
void print_assert (char *file_str, char *line_str);
void stop_show (int exitstatus);

/* screen.c */
void ch_siz (float *awidth, float *aheight);
void clear_curs (void);
void d_circle (float x1, float y1, float x2, float y2);
void d_fillst (int fs);
void d_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp);
void d_line (float x1, float y1, float x2, float y2);
void d_ltype (int lt);
void d_snapgrid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp, Coor xoff, Coor yoff);
void d_text (float x, float y, char *str);
void disp_mode (int dmode);
int  draw_cross (Coor xCoor, Coor yCoor);
void draw_trap (Coor line_x[], Coor line_y[]);
int  event_exists (void);
void exit_graph (void);
void fix_loc (float xx, float yy);
void flush_pict (void);
void get_gterm (void);
int  get_loc (float *x_p, float *y_p, int echo);
void GetString (char sbuf[], int len, int max);
void ggClearArea (float xl, float xr, float yb, float yt);
void ggClearWindow (void);
void ggEraseArea (float xl, float xr, float yb, float yt);
void ggEraseWindow (void);
int  ggGetColor (void);
void ggSetColor (int color_id);
void ggSetColors (int col_ids[], int nr_ids);
int  ggSetErasedAffected (int eIds[], int p_arr[], int nr_all);
void ggSetIdForColorCode (int code);
int  ggTestColor (int color_id, char *color);
void init_graph (void);
void paint_box (float x1, float x2, float y1, float y2);
void pic_cur (Coor x1, Coor x2, Coor y1, Coor y2);
void pict_poly (Coor coors[], int nr_c);
void pict_rect (float x1, float x2, float y1, float y2);
void print_reason (void);
void Prompt (void);
void set_backing_store (int doit);
void set_titlebar (char *ownstring);
void switch_tracker (int show_it);
void toggle_tracker (void);
void v_text (float x, float y, char *str);

/* sngrid.c */
void set_sn_grid_offset (int x_offset, int y_offset);
void set_sn_grid_values (int no_grd_sp, int *spc_list);
void set_snap_grid_width (int grid_width);
void disp_sn_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void set_sn_gr_spac (void);

/* s_o_g.c */
void d_grid_image (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void inform_cell_image (char *cellstr, Coor xl, Coor xr, Coor yb, Coor yt);
void print_image_crd (Coor xc, Coor yc);
void read_SoG_image (void);
void snap_box_to_grid (Coor *xl, Coor *xr, Coor *yb, Coor *yt);
void snap_instance_on_image (INST *inst);
void snap_point_to_grid (Coor *xc, Coor *yc);
int  tracker_to_image (char *mystr, Coor sx, Coor sy, Coor endx, Coor endy);

/* subr.c */
int  ask_name (char *string, char *namep, unsigned chk_name);
void ask_string (char *disp_str, char *ret_str);
void err_meas (char *str, int color_id);
void init_txtwdw (char *str);
void pr_nomem (void);
void ptext (char *str);
void Rtext (void);
char *strsave (char *s);
void track_coord (Coor xx, Coor yy, int erase_only);

/* termbuf.c */
int  empty_tbuf (void);
void fill_tbuf (Coor x1, Coor x2, Coor y1, Coor y2);
void pict_tbuf (Coor x_c, Coor y_c);
void place_tbuf (Coor xl, Coor yb);

/* term.c */
void add_term (Coor x1, Coor x2, Coor y1, Coor y2);
void all_term (void);
TERM *create_term (TERM **terml_pp, Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny);
void del_t_area (Coor x1, Coor x2, Coor y1, Coor y2);
void del_term (Coor x_cur, Coor y_cur);
void lnew_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny, int lay, int it, int jt);
TERM *new_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, Coor dx, int nx, Coor dy, int ny, int lay);
TERM *search_term (Coor x_c, Coor y_c, int *p_lay, int edit_only);
void term_win (TERM *termp, Coor *ll, Coor *rr, Coor *bb, Coor *tt);

/* tlift.c */
void lift_terms (Coor x1, Coor y1);

/* tpar.c */
void mov_term (Coor x_cur, Coor y_cur);
void set_act_term (Coor x_c, Coor y_c);
void t_arr_par (void);

/* window.c */
void bound_w (void);
void center_w (Coor xx, Coor yy);
void curs_w  (Coor xl, Coor xr, Coor yb, Coor yt);
void de_zoom (Coor xl, Coor xr, Coor yb, Coor yt);
void Def_window (int w_nr);
void def_world_win (int w_nr, float Wxmin, float Wxmax, float Wymin, float Wymax);
void initwindow (void);
void prev_w (void);
void save_oldw (void);
void set_c_wdw (int w_nr);
void Set_window (int w_nr);

/* wire.c */
void set_wire_ext (int aVal);
void set_wire_values (int no_wire_widths, int *wire_list);
void set_wire_width (int aWidth);
void wire_points (int non_orth);

