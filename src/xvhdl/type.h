
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

struct cir {
    char *name;
    char *orig_name;
    int imported;
    DM_PROJECT *proj;
    struct cir *next;
};

struct net_ref {
    struct net_el *n;
    struct net_ref *next;
};

struct term_ref {
    struct cir_term *t;
    int type;
    struct term_ref *next;
};

struct node_ref {
    char   *node_name;
    long    node_dim;
    long   *node_lower;
    long   *node_upper;
    int     type;
    struct node_ref *next;
};

struct model_info {
    char *name;
    char *orig_name;
    int imported;
    DM_PROJECT *proj;
    DM_CELL *dkey;
    struct term_ref *terms;
    struct model_info *next;
};

struct model_ref {
    struct model_info *m;
    struct model_ref *next;
};

struct net_el {
    char   *net_name;           /* net name */
    long    net_dim;            /* no. lower pairs */
    long   *net_lower;          /* lower range */
    char   *inst_name;          /* instance name */
    long    inst_dim;           /* no. lower pairs */
    long   *inst_lower;         /* lower range */
    long    net_neqv;           /* no. sub net_el struct's */
    struct net_el *net_eqv;	/* sub net_el's */
    struct net_el *next;	/* hashtab: next entry */
};
