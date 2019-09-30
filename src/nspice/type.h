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

#define MAXLEN 15   /* for resfile */
#define MAXDIM  4

typedef struct path_spec {
    char   name[MAXLEN + 1];
    int    xarray[MAXDIM + 1][2];
    struct path_spec *next;
    struct path_spec *also;
} PATH_SPEC;

typedef struct string_ref {
    char  *str;
    struct string_ref *next;
} STRING_REF;

/* the following definitions are for cmd_y.y */

#define Free_state  00
#define L_state     01
#define X_state     02
#define H_state     03

typedef struct signalelement
{
    int val;
    int len;
    struct signalelement *sibling;
    struct signalelement *child;
} SIGNALELEMENT;

typedef struct sig_value {
    long time;
    int value;
    struct sig_value *next;
} SIG_VALUE;

typedef struct node {
    char *name;
    int nodenr;
    struct signalelement *expr;
    struct sig_value *begin_value;
    struct node *next;
} NODE;

typedef struct node_ref {
    char *name;
    int nodenr;
    struct node_ref *next;
} NODE_REF;
