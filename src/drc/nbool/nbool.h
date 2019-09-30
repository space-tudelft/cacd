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
#include <signal.h>
#include "src/libddm/dmincl.h"

#define MAXLINE	    80
#define MAXINT  0x3FFFFFFF
#define BUFLEN      50
#define MAX_NOMASKS 30

#define START_EDGE   1
#define STOP_EDGE    2
#define CHANGE_EDGE  4
#define START_OV     8
#define STOP_OV     16

#define NOT_PRESENT  0
#define PAST_PRESENT 1
#define FUT_PRESENT  2
#define PRESENT      PAST_PRESENT + FUT_PRESENT

#define CONN_MASK   DM_INTCON_MASK
#define BOLEAN      3
#define TERM_MASK   4

#define CHK_HRCHY   1
#define NO_CHK_HRCHY 0

#define INITIAL    -1
#define DIFF_CT    -2

#define TRUE        1
#define FALSE       0
#define NEW_TERMLAY 2

#define	OFF	    0
#define ON	    1

#define EXPDATA	    "exp_dat"
#define TEMP_ONE    "/tmp/drc1.%d.%05d"
#define TEMP_TWO    "/tmp/drc2.%d.%05d"

#define ALLOC(ptr,name) {\
if(!(ptr = (struct name *) calloc (1, sizeof (struct name))))\
perror ("calloc"); }

#define FREE(ptr) free ((char *)(ptr))

#define OPEN(ptr,name,mode) {\
if(!(ptr = fopen (name, mode))) { perror (name); die (1); }}

#define CLOSE(ptr) {\
if((fclose (ptr)) == EOF) { perror ("fclose"); die (1); }}

FILE   *pexp;
FILE   *pout;
char    fr_name[24];	/* temp. filename */
char    fi_name[24];	/* temp. filename */
char    filename[256];
int     nomasks;
int     chk_flag;
int     pid;		/* program process_id */

struct grp_sr {
   int yb;
   int yt;
   struct group_tree *group;
   struct grp_sr *prev;
   struct grp_sr *next;
} grp_sr_head;

struct group_tree {
   union {
      int count;
      int name;
   } tree;
   struct group_tree *parent;
   struct group_tree *next;
} *group_ptr;

struct group_list {
    struct group_tree  *group;
    struct group_list  *next;
};

struct buff {
   int x;
   char edge;
   int yb;
   int yt;
   char conn;
   struct group_tree *grp;
   int ct;
   } *b_pntr;


unsigned int mask_terms;

struct sr_field {
    int     yb;
    int     yt;
    int     chk_type;
    struct check *p_chk;
    struct checktype *p_chg_ct;
    unsigned int mask_past;
    unsigned int mask_fut;
    unsigned int ov_mask;
    struct sr_field *next;
    struct sr_field *prev;
}
                   *h_sr, head_sr;

struct line_segment {
   int pos;
   int yb, yt;
   unsigned char edge_type;
   int chk_type;
   unsigned mask;
   unsigned mask_term;
   unsigned mask_type;
   char mask_name[DM_MAXLAY+7];
   DM_STREAM *fp;
};

struct check {
   int chk_arr[MAX_NOMASKS];
};

struct checktype {
   unsigned int mask;
   int chk_type;
   struct checktype *next;
};

struct min_term {
   unsigned int mask;
   unsigned int not_mask;
   struct min_term *next;
};

struct form {
   int f_nbr; /* bool_form_number */
   int curr_place;
   unsigned int vuln_mask;
   struct buff *b_pntr[BUFLEN];
   struct min_term *mt_pntr;
   struct form *next;
} *fp_head;

DM_PROCDATA *process;    /* ptr to process_descriptor */
DM_CELL *mod_key;

struct line_segment edges[MAX_NOMASKS];
int    edge_heap[MAX_NOMASKS];
int    nf;

/* add_grpnbr.c */
void add_grpnbr (int form_nbr);

/* chk_hrchy.c */
void check_hierarchy (int sr_pos);

/* extr_prof.c */
void extr_profile (int sr_pos);
void write_buf (struct form *frm_pntr, int nbr);

/* free_sr.c */
void free_state_ruler (void);

/* fr_frmstr.c */
void free_formstruct (void);

/* Iedge.c */
void insert_edge (struct sr_field **r_srp, int yb, int yt, char edge_type, int chk_type, unsigned int mask);

/* ini_heap.c */
void ini_heap (char *input);

/* main_bool.c */
void main_bool (void);

/* main.c */
void die (int status);

/* mk_frmstr.c */
void mk_formstruct (int file_flag);

/* reheap.c */
void reheap (void);
void mk_heap (void);

/* sel_edge.c */
int select_edge (int *pos_p, int *yb_p, int *yt_p, char *edge_type_p, int *chk_type_p, unsigned int *mask_p);

/* update_sr.c */
void update_sr (void);
