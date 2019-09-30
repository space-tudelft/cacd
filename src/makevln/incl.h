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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

/* General definitions: */
#define MAX_INTEGER     2000000000
#define MIN_INTEGER    -2000000000
#define TRUE		1
#define FALSE		0

/* Edge Type Definitions: */
#define START 	001
#define STOP	002
#define ChgCT 	004
#define StartOl	010
#define StopOl	020

/* Connect Type Definitions: */
#define NULL_CONN       0
#define DOWN_CONN	1
#define UP_CONN		2
#define TWO_CONN	(DOWN_CONN + UP_CONN)

/* Change Edge Type Definitions: */
#define OfCt		01
#define OfOl		02
#define ToZero		04

/* Data Structure Definitions: */
struct sr_field {
   long yb, yt;
   long duration;
   long ol_dur;
   long checktype;
   struct group_tree *group;
   struct {
      unsigned start: 1;
      unsigned overlap: 1;
      unsigned ol_area: 1;
      unsigned incident: 1;
      unsigned ct_zero: 1;
   } flag;
   struct sr_field *prev, *next;
};

struct group_tree {
    int    tree_count;
    struct group_tree  *parent;
    struct group_tree  *next;
};

struct event_rec {
    long    xl, xr, yb, yt;
    long    attr_no;
};

struct S_vln {
    long    x, yb, yt;
    char    occ, con;
    struct  group_tree *grp;
    long    cht;
};

struct S_teq {
    long    tnr;
    struct  group_tree *grp;
};

/* Memory Allocation/Release Macro's: */

#define ALLOCPTR(ptr,name) {if((\
ptr=(struct name *)calloc(1,sizeof(struct name))\
)==NULL)errexit(3,"");}

#define ALLOCARR(ptr,elmts,name) {if((\
ptr=(name *)calloc((unsigned)(elmts),sizeof(name))\
)==NULL)errexit(3,"");}

#define FREE(ptr) free((char *)ptr)

/* General Purpose Macro's: */

#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))

/* Connect Type Macro: */

#define CONNECTED(c1_sr,c2_sr) \
(c1_sr != h_sr && c2_sr != h_sr && c1_sr->yt == c2_sr->yb)

#define CONN_SGRP(c1_sr,c2_sr) \
(CONNECTED(c1_sr,c2_sr) && fdgrp_ptr(c1_sr->group) == fdgrp_ptr(c2_sr->group))

/* debug.c */
#ifdef DEBUG
void pr_event (struct event_rec *event);
void pr_evt_list (struct event_rec *event1, struct event_rec *event2);
void pr_prof (struct sr_field *c_sr, int scan_mode);
#endif /* DEBUG */

/* errexit.c */
void errexit (int err_no, char *s);
void die (int status);

/* find.c */
struct group_tree *fdgrp_ptr (struct group_tree *group);
int fdgrp_name (struct group_tree *group);

/* ins_event.c */
void insert_event (struct sr_field **pr_sr, long *p_next_stop_pos);

/* ScanProf.c */
void ScanProf (long *p_stop_pos);

/* set_group.c */
void set_group (struct sr_field *c_sr);
