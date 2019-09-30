/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
 *	J. Liedorp
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
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include "src/libddm/dmincl.h"

#define GRP_BUFLEN	1024
#define MODELSIZE	10
#define LAYERSIZE	3
#define MAXGAPTABLE	8
#define MAXLINE		80
#define MAXINT		0x3FFFFFFF
#define MAX_GROUP	1000

#define START_EDGE	1
#define STOP_EDGE	2
#define CHANGE_EDGE	4
#define START_CHG_EDGE	START_EDGE + CHANGE_EDGE
#define STOP_CHG_EDGE 	STOP_EDGE + CHANGE_EDGE
#define START_OV    	8
#define STOP_OV     	16

#define TRUE		1
#define FALSE		0

#define UP          	1
#define DOWN        	2

#define ON	    	1
#define OFF         	0

#define FIRST       	0
#define SECOND      	1
#define THIRD		2

#define LEFT		1
#define RIGHT		2
#define BOTTOM		4
#define TOP		8

#define NOT_PRESENT 	0
#define CHG_TO_PRESENT	1
#define CHG_TO_NOTPRESENT 2
#define PRESENT	 	3

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define ABS(a)   ((a)<0 ? -(a) : (a))

#define EXPDATA		"exp_dat"

#define ALLOC(ptr,name) {\
if(!(ptr = (struct name *) calloc (1, sizeof (struct name))))\
perror ("calloc"); }

#define NALLOC(ptr,num,name) {\
if(!(ptr = (name *) calloc ((unsigned) (num), sizeof (name))))\
perror ("calloc"); }

#define FREE(ptr) free((char*)(ptr))

#define OPEN(ptr,name,mode) {\
if(!(ptr = fopen (name, mode))) { perror (name); die (1); }}

#define CLOSE(ptr) {\
if((fclose (ptr)) == EOF) { perror ("fclose"); die (1); }}

DM_STREAM * pvln[3];
FILE * pexp;
FILE * pdat;
FILE * pout;
FILE * bufil;

char    c;
char    line[MAXLINE];
char    conn_dir[MAX_GROUP];

struct {
    int     e_xi;
    int     e_occ;
    int     e_yb;
    int     e_yt;
    int     e_conn;
    int     e_group;
    int     e_ctype;
} event[3];

struct chk_err {
    char err_type[10];
    int x1, x2, y1, y2;
    struct chk_err *next;
} *head_errlist;

int     zero_flag;
struct {
   int x1;
   int x2;
   int y1;
   int y2;
   int group1;
   int group2;
} buff;

int	kind;
int     exgap;
int     exlength;
int     exgapflag;
int	gapflag;
int     gap;
int     overlap;
int     overlapflag;
int     Errno;
int     MAXINFLUENCE;
int     grow_fact;

int	nr_samples;

struct group {
    struct eq *eq_pntr;
};

struct group *group_arr[GRP_BUFLEN];

struct eq {
   int eq_group;
   struct eq *next;
};

struct err_pos {
    int x1;
    int y1;
    int x2;
    int y2;
    int group1;
    int group2;
    struct err_pos *next;
} *head_err;

struct sr_field {
    int     xstart[2];
    int     yb;
    int     yt;
    int     lay_status[2];
    int     group[2];
    int     chk_type[2];
    int	    helplay_status;
    int	    helplay_xstart;
    struct sr_field *next;
    struct sr_field *prev;
} *h_sr, head_sr;

struct table {
    int     dis[MAXGAPTABLE + 1];
} *ptable;

/* checks.c */
void add_err (int x1, int y1, int x2, int y2, int group1, int group2);
void check_g_circle (int x_sr, struct sr_field *c_sr, int direction, int i);
void check_xgap (int x_sr, struct sr_field *c_sr, int i);
void check_ygap (int x_sr, struct sr_field *c_sr, int direction, int i);

/* dig_circle.c */
int dig_circle (int length, int dx);

/* extr_ovlp.c */
void extr_overlap  (int sr_pos);
void extr_overlap1 (int sr_pos);
void extr_overlap2 (int sr_pos);
void extr_overlap3 (int sr_pos);
void extr_overlap6 (int sr_pos);

/* extr_prof.c */
void det_conn_ver (int sr_pos);
void det_conn_hor (int sr_pos);
void extr_profile (int sr_pos);

/* fltr_err.c */
void error_meas (char *str, int x1, int y1, int x2, int y2);
void filter_err (void);

/* free_sr.c */
void free_state_ruler (void);

/* fr_errstr.c */
void free_errs (struct chk_err *err_head);
void rmv_errstr (void);

/* get_vln.c */
int get_vln (DM_STREAM *fp, int nbr_file);

/* Iedge.c */
void insert_edge (struct sr_field **r_srp, int yb, int yt,
	char edge_type, int group_no, int chk_type, int nbr_file);

/* ini.c */
void ini (void);

/* main.c */
void addstring (char *main_string, char *sub_string);
void die (int status);
void ERROR (char *str, int p1, int p2, int p3, int p4);
void error (char *s);

/* main_check.c */
void main_check (void);

/* print_err.c */
void print_err (int kind);

/* print_sr.c */
void print_sr (void);

/* update_sr.c */
void update_sr (int x_sr, int xnew);
