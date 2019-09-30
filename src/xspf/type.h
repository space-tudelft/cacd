
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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
    struct net_el *n, *nl;
    int cd;   /* conductor # */
    int done;
    struct net_ref *prev, *next;
};

struct cirterm {
    char *term_name;
    int   term_dim;
    int  *term_lower, *term_upper;
};
struct term_ref {
    struct cirterm *t;
    int type;
    struct term_ref *next;
};

struct model_info {
    char *name;
    char *orig_name;
    char *type_name;
    char *out_name;

    DM_CELL *dkey;
    DM_PROJECT *proj;
    short imported;
    short spec_dev;
    struct term_ref *terms;
    char *alias_s;
    char *alias_t;
    char  prefix[8]; /* for SPF instance name */
    float dw, dl;
    float vbulk;
    short vbulkdefined;
    short lookForBulk;
    struct lib_model *createName;
    struct cell_par *pars;
    struct model_info *next;
};

struct cell_prefix_info {
    char *cell_name;
    char *prefix;
    struct cell_prefix_info *next;
};

struct cell_par_info {
    char *cell_name;
    struct cell_par *pars;
    struct cell_par_info *next;
};

struct cell_par {
    char *par_stat;
    char *par;
    char *val;
    struct cell_par * next;
};

struct net_el {
union {
    double  val;
    char   *name;
} v;
    char   *net_name;           /* net name */
    int     net_dim;            /* no. lower pairs */
    int    *net_lower;          /* lower range */
    char   *inst_name;          /* instance name */
    int     inst_dim;           /* no. lower pairs */
    int    *inst_lower;         /* lower range */
    struct net_el *net_eqv;	/* sub net_el's */
    struct net_el *next;	/* hashtab: next entry */
    short   valid;		/* 1=val, 2=name */
    short   lay;		/* layer number */
    int     nx;                 /* node index in node table */
    int     x, y;		/* coordinates */
};

struct ap_info {
    char *dev_name;		/* device name */
    double area;		/* drain/source area */
    double perim;		/* drain/source perimeter */
    int  cnt;			/* total number of transistors */
    double width;		/* total width of transistors */
};

struct ap_info_ref {
    struct ap_info *ap;
    struct ap_info_ref *next;
};

struct node_info {
    short isTerm;
    struct ap_info_ref *aplist; /* drain/source area/perimeter info */
    struct net_ref *netref;
    char *spef_name; /* (SdeG) */
};

struct conduct {
    int lay;
    double res;
};

struct contact {
    int lay1, lay2;
    double res;
};
