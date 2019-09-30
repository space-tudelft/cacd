/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
 *	T.G.R. van Leuken
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include "src/libddm/dmincl.h"

#define MAXGAPTABLE      8
#define MAXLINE	        80
#define MAXINT  0x3FFFFFFF

#define START_EDGE  1
#define STOP_EDGE   2
#define CHANGE_EDGE 4
#define START_CHG_EDGE START_EDGE + CHANGE_EDGE
#define STOP_CHG_EDGE  STOP_EDGE + CHANGE_EDGE
#define START_OV    8
#define STOP_OV    16

#define UP          1
#define DOWN        2

#define ON	    1
#define OFF         0

#define TRUE		0
#define FALSE		1

#define NOT_PRESENT	  0
#define CHG_TO_PRESENT	  1
#define CHG_TO_NOTPRESENT 2
#define PRESENT	 	  3

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(a)	 ((a) < 0 ? -(a) : (a))

#define EXPDATA		"exp_dat"

#define ALLOC(ptr,name) {\
if(!(ptr = (struct name *) calloc (1, sizeof (struct name))))\
perror ("calloc"); }

#define NALLOC(ptr,num,name) {\
if(!(ptr = (name *) calloc ((unsigned)(num), sizeof (name))))\
perror ("calloc"); }

#define OPEN(ptr,name,mode) {\
if(!(ptr = fopen (name, mode))) { perror (name); die (1); }}

#define CLOSE(ptr) {\
if(fclose (ptr) == EOF) { perror ("fclose"); die (1); }}

DM_STREAM *pvln[2];
FILE    *pexp;
FILE    *pdat;
FILE    *pout;

char    line[MAXLINE];

struct {
    int     e_xi;
    int     e_occ;
    int     e_yb;
    int     e_yt;
    int     e_conn;
    int     e_group;
    int     e_ctype;
} event[2];

struct buff {
   int x;
   char edge;
   int yb;
   int yt;
   char conn;
   int grp;
   int ct;
} *b_pntr;

struct chk_err {
    char err_type[10];
    int x1, x2, y1, y2;
    int group;
    struct chk_err *next;
} *head_err;

int     zero_flag;
int	group_check;
int     exgap;
int     exlength;
int     exgapflag;
int	gapflag;
int	widthflag;
int	maxwidthflag;
int     gap;
int     width;
int     maxwidth;
int     Errno;
int     MAXINFLUENCE;
int     nr_samples;

struct sr_field {
    int     xstart;
    int     yb;
    int     yt;
    int     lay_status;
    int     helplay_status;
    int     group;
    int     group_old;
    int     chk_type;
    int     chk_type_old;
    struct sr_field *next;
    struct sr_field *prev;
} *h_sr, head_sr;

struct table {
    int     dis[MAXGAPTABLE + 1];
} *ptable;

/* Iedge.c */
void insert_edge (struct sr_field **r_srp, int yb, int yt, char edge_type, int group_no, int chk_type, int file_nbr);
/* checks.c */
void check_stop (int sr_pos, struct sr_field *c_sr);
void check_start (int sr_pos, struct sr_field *c_sr);
void check_left_start (int sr_pos, struct sr_field *c_sr);
void check_right_stop (int sr_pos, struct sr_field *c_sr);
void check_max_ywidth (int x_sr, struct sr_field *c_sr);
void check_max_xwidth (int x_sr, struct sr_field *c_sr);
void check_ywidth (int x_sr, struct sr_field *c_sr, int chk_status, int direction);
void check_xwidth (int x_sr, struct sr_field *c_sr);
void check_ygap (int x_sr, struct sr_field *c_sr, int direction);
void check_w_circle (int x_sr, struct sr_field *c_sr, int direction);
void check_g_circle (int x_sr, struct sr_field *c_sr, int direction);
void check_xgap (int x_sr, struct sr_field *c_sr);
/* dig_circle.c */
int dig_circle (int length, int dx);
/* extr_prof.c */
void extr_profile (int sr_pos);
/* fltr_err.c */
void add_err (char *str, int x1_err, int y1_err, int x2_err, int y2_err, int group);
void filter_err (void);
/* free_sr.c */
void free_errs (struct chk_err *head_errs);
void free_state_ruler (void);
/* ini.c */
void ini (void);
/* main.c */
void die (int status);
void ERROR (char *str, int p1, int p2, int p3, int p4);
/* main_check.c */
void main_check (void);
/* update_sr.c */
void update_sr (int x_sr, int xnew);
