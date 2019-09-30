/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
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

#include <ctype.h>
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

/* A picture-flag for each mask, for text 'Textnr',
** grid 'Gridnr', and the design rule errors 'DRC_nr'.
*/
#define NR_VIS  11

#if DM_MAXNAME >= 100
#define MAXCHAR DM_MAXNAME+1
#define MAXSTR  DM_MAXNAME+1
#else
#define MAXCHAR 100
#define MAXSTR  100
#endif

#define LIST_LENGTH 20
#define MaxPoints  10000

#define NO_SNAP     0
#define SNAP        1

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

#define FILL_SOLID     0
#define FILL_HASHED    1
#define FILL_HASHED25  2
#define FILL_HASHED50  3
#define FILL_HOLLOW    4
#define FILL_HASHED12B FILL_HASHED   + FILL_HOLLOW
#define FILL_HASHED25B FILL_HASHED25 + FILL_HOLLOW
#define FILL_HASHED50B FILL_HASHED50 + FILL_HOLLOW
#define FILL_CROSS     8
#define FILL_STIPPLE1 16
#define FILL_STIPPLE5 32

#define LINE_SOLID  0
#define LINE_DOTTED 1
#define LINE_DOUBLE 2

#define READ_TERM  0
#define READ_ALL   1

#define TRUE    1
#define FALSE   0

#define MENU    2
#define PICT    3
#define TEXT    5
#define LAYS    6

#define PE fprintf(stderr,

#define Abs(a)   ((a) < 0 ? -(a) : (a))
#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define Round(x) (((x) > 0) ? ((x)+0.5) : ((x)-0.5))
#define LowerRound(x) (((x) < 0.0) ? ((x) - 1.0) : (x))
#define UpperRound(x) (((x) > 0.0) ? ((x) + 1.0) : (x))

#define MALLOC(ptr,type) { if (!(ptr = (type *) malloc (sizeof (type)))) pr_nomem (); }

#define MALLOCN(ptr,type,num) { if (!(ptr = (type *) malloc ((unsigned)(num) * sizeof (type)))) pr_nomem (); }

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
    short  sides;	/* slope of the left&right side */
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
    struct qtree *Uleft,	/* pointers to the 4 child nodes */
		*Uright, *Lleft, *Lright;
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
    int    tmlen;
    Coor   dx, dy;
    int    nx, ny;
    struct term *nxttm;
} TERM;

typedef struct termref {
    int    lay;
    struct term *tp;
    struct termref *next;
} TERMREF;

typedef struct inst {
    Coor   bbxl, bbxr, bbyb, bbyt;
    char  *inst_name;
    int    imported;
    Trans  tr[6];
    Coor   dx, dy;
    int    nx, ny;
    int    level;	/* expansie level */
    struct templ *templ;
    struct inst  *next;
} INST;

typedef struct templ {
    char  *cell_name;
    DM_PROJECT *projkey;
    qtree_t **quad_trees;
    int    t_flag;	/* 1 -> terminals gelezen */
    TERM **term_p;
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
void add_del_poly (int mode);
void calc_bbox (int first, struct obj_node *list);
void draw_lines (Coor line_x[], Coor line_y[], int npoints);
void Rpoly (void);
int  test_point (int big, int non_orth, Coor line_x[], Coor line_y[]);

/* add_quad.c */
void add_new_traps (int lay, struct obj_node *traplist);
void add_quad (qtree_t *quad_array[], struct obj_node *nwlist, int lay);
void addel_cur (Coor xl, Coor xr, Coor yb, Coor yt, int mode);

/* add_wire.c */
void add_wire (Coor stack_x[], Coor stack_y[], int npoints, Coor width, int non_orth, int wire_ext);

/* animate.c */
void animate (void);
void inform_SofG (void);
void open_dalirc (void);

/* array.c */
void arr_par ();
int  ask_par (char ***arr_p_p);
int  dx_dy_par (Coor *d_p, int n_p, Coor bbxl, Coor bbyb, int comm_par);
int  nx_ny_par (int *nx_ny, Coor *dx_dy, Coor dd, char *obj_name);

/* bintree.c */
void cl_edgetree (struct edgetree *p);
struct bintree *link_bintree (struct bintree *p);
struct x_lst *link_edgetree (struct edgetree *p);
struct bintree *mk_bintree (Coor value);
struct edgetree *mk_edgetree (struct x_lst *edge);
void to_bintree (struct bintree *p, Coor val);
void to_edgetree (struct edgetree *p, struct x_lst *edge);

/* bound.c */
void upd_boundb (void);

/* buffer.c */
void clearPutBuf (int lay);
void fill_buffer (Coor xl, Coor xr, Coor yb, Coor yt);
void pict_buf (void);
void put_buffer (int termflag);
struct obj_node *yank_traps (struct obj_node *p, struct obj_node *q, struct obj_node *yank_head);

/* check.c */
void area_check (void);

/* clip.c */
struct obj_node *clip (struct obj_node *p, struct obj_node *q, struct obj_node *clip_head);

/* command.c */
void ask_curs_w (void);
void ask_de_zoom (void);
void ask_quit (void);
void command (void);
void main_menu (void);
void set_dominant (void);
void set_hashed (void);

/* comment.c */
void c_label (Coor x, Coor y, int orient);
void c_line (Coor x1, Coor y1, Coor x2, Coor y2, int arrow_mode);
void c_text (Coor x, Coor y, int orient);
int  comment_win (Coor *ll, Coor *rr, Coor *bb, Coor *tt);
void del_comment (Coor x, Coor y);
void dis_label (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void draw_all_comments (void);
void empty_comments (void);
int  inp_comment (DM_CELL *cellKey);
int  no_comments (void);
int  outp_comment (void);

/* dalirun.c */
int  DaliRun (char *path, char *outp_file, char *alist);

/* del_quad.c */
void del_traps (struct obj_node *del_list, int lay);

/* disp_mask.c */
void clip_paint (struct obj_node *p);
void disp_mask_quad (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void mtx_draw_box (Coor x1, Coor x2, Coor y1, Coor y2);
void mtx_draw_trap (Coor x0, Coor x1, Coor x2, Coor x3, Coor y0, Coor y1);

/* display.c */
void dis_s_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void dis_term (int lay, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void mc_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void pict_mc (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void term_char (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void trans_box (Coor arr1[], Coor arr2[], INST *ip, int nnx, int nny);

/* distance.c */
double dis (float x1, float y1, float x2, float y2);
int  distance (struct f_edge *p, struct f_edge *q, struct f_edge *sh_d);

/* dsgn_dat.c */
void r_design_rules (void);

/* edge_t_ch.c */
void sub_to_checker (struct obj_node *p);
void to_precheck (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void trap_to_checker (struct obj_node *p);

/* expand.c */
void def_level (void);
int  exp_templ (TEMPL *te_p, DM_PROJECT *fath_projkey, int imported, int mode);
int  expansion (int new_level);
void indiv_exp (void);

/* ext_check.c */
void check (void);
void chk_file (void);
void disp_next (int inside_window);
void draw_drc_err (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void empty_err (void);
void ind_err (Coor x_cur, Coor y_cur);
void set_flat_expansion (int aVal);
void setOpt (int k, char *str);
void toggle_drc_err (void);

/* get.c */
int  get_all (int echo, Coor *x, Coor *y);
void get_cmd (void);
int  get_com (void);
int  get_cursor (int echo, int num, int snap);
int  get_one (int echo, Coor *x, Coor *y);
int  get_wire_point (int echo, Coor *x, Coor *y, int width, int non_orth);

/* grid.c */
void disp_axis (void);
void display_grids (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
int  grid_on (void);
void no_grid_adjust (void);
void set_gr_spac (void);
void set_grid_values (int no_grd_sp, int *spc_list);
void set_grid_width (int grid_width);
void toggle_grid (void);

/* inform.c */
void inform_cell (void);
void inform_process (void);
void inform_window (void);

/* input.c */
int  eras_worksp (char *c_name);
int  inp_boxnorfl (DM_CELL *key, qtree_t *quads[]);
int  inp_mcfl (DM_CELL *key, INST **inst_pp);
int  inp_mod (char *c_name);
int  inp_term (DM_CELL *key, TERM **term_pp);
int  read_bound_box (DM_PROJECT *proj_key, char *cell_name, Coor *bxl, Coor *bxr, Coor *byb, Coor *byt);

/* insert.c */
struct obj_node *insert (struct obj_node *new_list, int lay, int mode);
struct ylist *yinsert (struct ylist *root, Coor value);

/* inst_draw.c */
void draw_inst_window (int lay, Coor xmin, Coor xmax, Coor ymin, Coor ymax);

/* main.c */
void fatal (int Errno, char *str);
void print_assert (char *file_str, char *line_str);
void stop_show (int exitstatus);

/* mask.c */
void color_menu (void);
void disable_masks (int no_masks, int *mask_array);
void exit_set_coord (void);
void fillst_menu (void);
void give_lay_stack (Coor x, Coor y);
void init_colmenu (void);
void next_colmenu (void);
void prev_colmenu (void);
void init_mskcol (void);
void init_set_coord (void);
void load_settings (char *file);
int  nr_of_selected_lays (void);
void rebulb (int lay);
void Rmsk (void);
void save_settings (void);
void set_fill_style (int lay, int style, int shift);
void set_pict_arr (int tmp_arr[]);
void toggle_lay (int lay);
int  toggle_pos (int nbr);
void toggle_subterm (void);
void Visible (void);

/* memo.c */
void clear_templ (TEMPL *ep);
void empty_mem (void);
void init_mem (void);
int  no_works (void);

/* menu.c */
int  ask (int nr_alt, char *alt_arr[], int old);
void menu (int nr_alt, char *alt_arr[]);
void post_cmd_proc (int nbr);
void pre_cmd_proc (int nbr);
void prev_menu (void);
void Rmenu (void);
void save_menu (void);
void turn_lamp (int nbr, int enable);

/* mod.c */
void add_inst (void);
int  ask_inst (INST *act_ip);
INST *cell_tree (Coor x, Coor y);
INST *create_inst (INST **inst_pp, char *cellname, char *inst_name, int import,
	Trans tr0, Trans tr1, Trans tr3, Trans tr4, Trans tx, Trans ty, Coor dx, int nx, Coor dy, int ny);
void del_inst (void);
int  inst_outside_window (INST *ip, Coor xmin, Coor xmax, Coor ymin, Coor ymax);
void inst_window (INST *ip, Coor *ll, Coor *rr, Coor *bb, Coor *tt);
int  present_inst (void);
INST *search_inst (Coor x1, Coor y1);

/* modl.c */
char *ask_cell (int local_imported);
int  read_modlist (void);
int  read_impclist (void);

/* mpar.c */
void mir_inst (void);
void mov_inst (void);
void rot_inst (void);
void set_actinst (int mode);
void set_inst_name (void);
void upd_inst_bbox (void);

/* output.c */
int  outp_bbox (void);
int  outp_boxfl (void);
int  outp_mcfl (void);
int  outp_term (void);
void store (struct obj_node *p);
void wrte_cell (void);

/* picture.c */
void pic_max (void);
void pict_all (int mode);
void picture (void);
void Rpicture (int x, int y, int w, int h);

/* poly_trap.c */
struct obj_node *poly_trap (Coor line_x[], Coor line_y[], int npoints);
int  trap_to_poly (Coor line[], struct obj_node *p);

/* precheck.c */
void edge_insert (struct x_lst *new);
void make_true_edges (int act_lay);
void y_insert (Coor value, int layer);

/* q_build.c */
qtree_t *qtree_build (void);

/* q_clear.c */
void quad_clear (qtree_t *Q);

/* q_delete.c */
void quad_delete (qtree_t *Q, struct obj_node *p);

/* q_insert.c */
qtree_t *quad_insert (struct obj_node *p, qtree_t *Q);

/* q_search.c */
void quad_search (qtree_t *Q, void (*function)());
struct found_list *quick_search (qtree_t *Q, struct obj_node *srch);

/* screen.c */
void d_circle (float x1, float y1, float x2, float y2);
void d_fillst (int fs);
void d_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp);
void d_line (float x1, float y1, float x2, float y2);
void d_ltype (int lt);
void d_snapgrid (Coor wxl, Coor wxr, Coor wyb, Coor wyt, Coor sp, Coor xoff, Coor yoff);
void d_text (float x, float y, char *str, int strLen);
void disp_mode (int dmode);
void draw_Cross (Coor xCoor, Coor yCoor);
void draw_cross (Coor xCoor, Coor yCoor);
void draw_trap (Coor line_x[], Coor line_y[]);
void fix_loc (float xx, float yy, int echo);
void flush_pict (void);
int  get_disp_mode (void);
void get_gterm (void);
int  get_loc (float *x_p, float *y_p, int echo);
void GetString (char sbuf[], int len, int max);
void ggBell (int percent);
void _ggBell (int percent);
void ggChangeColorCode (int id, int code);
void ggClearWindow (void);
void ggEraseArea (float xl, float xr, float yb, float yt, int mode);
void ggEraseWindow (void);
int  ggGetColor (void);
void ggSetColor (int color_id);
void ggSetColors (int col_ids[], int nr_ids);
int  ggSetErasedAffected (int eIds[], int p_arr[], int nr_all);
void ggSetIdForColorCode (int code);
int  ggTestColor (int color_id, char *color);
void init_graph (void);
void paint_box (float x1, float x2, float y1, float y2);
void pict_cur (Coor x_c, Coor y_c);
void pict_poly (Coor coors[], int nr_c);
void pict_rect (float x1, float x2, float y1, float y2);
void Prompt (void);
void set_bbox_loc (Coor ll, Coor rr, Coor bb, Coor tt);
void set_color_entry (int nbr, char *color);
void set_new_bitmap (int nr, int w, int h, char bm[]);
void set_titlebar (char *ownstring);
void toggle_tracker (void);

/* sngrid.c */
void disp_sn_grid (Coor wxl, Coor wxr, Coor wyb, Coor wyt);
void set_sn_gr_spac (void);
void set_sn_grid_offset (int x_offset, int y_offset);
void set_sn_grid_values (int no_grd_sp, int *spc_list);
void set_snap_grid_width (int grid_width);

/* subr.c */
void ask_coordnts (void);
int  ask_name (char *disp_str, char *namep, int chk_name);
int  ask_string (char *disp_str, char *ret_str);
void btext (char *str);
void err_meas (char *str);
void init_txtwdw (char *str);
void notify (char *str);
void pr_nomem (void);
void ptext (char *str);
void redraw_cross (void);
void Rtext (void);
char *strsave (char *str, int len);

/* term.c */
void add_term (void);
TERM *create_term (TERM **terml_pp, Coor xl, Coor xr, Coor yb, Coor yt, char *name, int len, Coor dx, int nx, Coor dy, int ny);
void del_term (void);
TERM *new_term (Coor xl, Coor xr, Coor yb, Coor yt, char *name, int len, Coor dx, int nx, Coor dy, int ny, int lay);
int  present_term (void);
TERM *search_term (Coor x_c, Coor y_c, int *layP);
void term_win (TERM *tp, Coor *ll, Coor *rr, Coor *bb, Coor *tt, int point_mode);

/* termbuf.c */
void fill_tbuf (void);
void yank_tbuf (void);

/* tlift.c */
void lift_terms (void);

/* tpar.c */
void mov_term (void);
void t_arr_par (void);

/* window.c */
void bound_w (void);
void center_w (Coor xc, Coor yc);
void curs_w (Coor xl, Coor xr, Coor yb, Coor yt);
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
void wire_points (void);
