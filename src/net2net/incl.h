/*
 * ISC License
 *
 * Copyright (C) 2016 by
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

#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

#define PALLOC(ptr, nel, type) { \
if(!(nel) || !(ptr = (type *)calloc((size_t)(nel), sizeof(type)))) cannot_die(1,0); }

#define typC   'C'
#define typCS  'D'
#define typCSG 'E'
#define typCG  'G'
#define typR   'R'
#define typRS  'S'

struct net_ref {
    struct net_el *n;	/* first el */
    struct net_el2 *nl;	/* last RC el */
    struct net_el2 *nR;	/* first R el */
    struct net_el2 *nC;	/* first C el */
    int  nx;	/* node number */
    int  cd;	/* conductor nr */
    int  lay;	/* layer number */
    int  netcnt; /* net count */
    struct net_ref *prev, *next;
};

struct net_el {
    char  *net_name;	/* net name */
    char  *inst_name;	/* instance name */
    short  net_dim;	/* no. of net_lower */
    short  inst_dim;	/* no. of inst_lower */
    struct net_el *nexte; /* next net_el */
    int    x, y;	/* coordinates */
};

struct net_el2 {
    double val;		/* RC value */
    struct net_el2 *nexte; /* next net_el2 */
    short  sort;	/* RC sort number */
    char   net_type;	/* net name */
    char   inst_type;	/* instance type */
    int    inst_nr;	/* instance nr */
    int    times;	/* RC found count */
};

#ifdef __cplusplus
  extern "C" {
#endif

/* findNet.c */
void findNetInit (struct net_ref *nets);
void findNetRC (double val);

/* main.c */
void warning  (char *s1);
void warning2 (char *s1, char *s2);
void verbose3 (char *s1, char *s2, char *s3);
void fatalErr (char *s1, char *s2);
void cannot_die (int nr, char *fn);
void die (void);

/* prInst.c */
void prInst (struct net_ref *nets);
void prNets (struct net_ref *nets);

/* xnetwork.c */
void xnetwork (char *name, DM_PROJECT *proj);
char *newStringSpace (char *s, int dim);
char *getAttrValue (char *a, char *par);
char *strsave (char *s);

#ifdef __cplusplus
  }
#endif
