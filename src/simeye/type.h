/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

/* the following definitions are for resfile */

#define MAXDIM  4

typedef long long simtime_t;

typedef struct path_spec {
    char    name[DM_MAXNAME + 1];
    int     xarray[MAXDIM + 1][2];
    struct path_spec   *next;
    struct path_spec   *also;
} PATH_SPEC;

typedef struct res_path {
    PATH_SPEC *path;
    int     totnum;
    struct res_path *next;
} RES_PATH;

typedef struct res_file {
    char    name[DM_MAXNAME + 1];
    RES_PATH *signals;
    double  scaling;
    int     sig_cnt;
    long    offset;
    FILE   *fp;
    struct res_file *next;
} RES_FILE;

typedef struct string_ref {
    char   *str;
    struct string_ref  *next;
} STRING_REF;

/* do not change this logic values: */
#define F_state    -1
#define L_state     0
#define X_state     1
#define H_state     2
#define NOstate     9

typedef struct signalelement
{
    simtime_t len;
    int val;
    struct signalelement *sibling;
    struct signalelement *child;
} SIGNALELEMENT;

typedef struct node {
    char *name;
    short no_edit;
    struct signalelement *expr;
    struct node *next;
} NODE;

/* the above definitions are for resfile and cmd_y.y */

/* the following definitions are the general types used */

typedef struct sig_value {
    simtime_t time;
    int value;
    struct sig_value *next;
    struct sig_value *prev;
} SIG_VALUE;

typedef struct signal {
    char *name;
    int i;
    short no_edit;
    short endless;
    short stringValue;
    struct signalelement *expr;
    struct signal *next;
    struct signal *orig_next;
    struct signal *prev;
    struct signal *layover2;
    struct signal *layover3;
    struct signal *layover4;
    struct signal *layover5;
    struct signal *layover6;
    struct sig_value *begin_value;
    struct sig_value *end_value;
    struct sig_value *last_value;
    struct sig_value *begin_value_L;
    struct sig_value *end_value_L;
    struct sig_value *last_value_L;
    struct sig_value *begin_value_U;
    struct sig_value *end_value_U;
    struct sig_value *last_value_U;
} SIGNAL;

typedef int Grid;
