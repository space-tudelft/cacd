/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

/*
    CACD-RELEASE-ALLOWED
*/

typedef struct nametable
{
    int name;       /* index in string table ST[] */
    short xtx;      /* if >= 0, index to the indices of this name */
    short sort;     /* Node, Node_t, Modelcall, Transistor, Functional */
                    /* or Intercap */
    int x;          /* index in N[], MCT[], T[], F[], I[] or XX[] */
} NAMETABLE;

typedef struct modeltable {
    char name[NAMESIZE];  /* name of this model */
    int nt_cnt;           /* number of names within this model */
} MODELTABLE;

typedef struct context {
    int ceiling;    /* maximum index of a group of nodes belonging
		       to the same modelcall */
    short mctx;     /* index to the modelcall to which the nodes belong */
} CONTEXTTABLE;

typedef struct modelcall {
    short mtx;      /* index to modeltable element, denoting the model called */
    short parent;   /* index to a possible modelcall where this
		       modelcall was defined */
    int ntx;        /* index to nametable element which gives
		       the name of this modelcall (instance) */
    int n_ntx;      /* start of names within this model */
} MODELCALLTABLE;

#ifdef SLS

typedef struct pre_node {

#else

typedef struct node {

#endif

    int ntx;                 /* index in NT[] */
    int dsx;                 /* index in DS[] */
    int cx;                  /* index in C[] */
    float statcap;           /* virtual capacity */
    unsigned funcoutp : 8;   /* functional block output ? */
    unsigned linked : 8;     /* node already linked to transistors ? */
			     /* if yes, then cx gives the index of that node */
    unsigned redirect : 8;   /* has node been redirected to another node ? */
    unsigned flag : 8;       /* general purpose flag */

#ifdef SLS

} PRE_NODE;

#else

} NODE;

#endif

typedef struct control {
    int c;           /* index in T[], F[] or I[] */
    short sort;      /* Transistor, Functional or Intercap */
} CONTROL;

#ifdef SLS

typedef struct pre_transistor {

#else

typedef struct transistor {

#endif

    int gate;
    int source;
    int drain;
    float width;
    float length;            /* or resistances when type is Res */
    unsigned type     : 16;  /* Nenh, Penh, Depl or Res */
    unsigned flag     : 16;  /* general purpose flag    */

#ifdef SLS

} PRE_TRANSISTOR;

#else

} TRANSISTOR;

#endif

typedef struct functional {
    unsigned type     : 16;  /* if < 1000, index in FD[] */
    unsigned evalflag : 16;
    int fix;                /* index in FI[] */
    int frx;                /* index in FR[] */
    int fox;                /* index in FO[] */
    int fsx;                /* index in FS[] */
} FUNCTION;

typedef struct funcout {
    int x;                  /* index in N[] */
    float trise;            /* rise delay */
    float tfall;            /* fall delay */
    short type;		    /* type is output or inout */
} FUNCOUT;

typedef struct intercap {
    int con1;
    int con2;
    float cap;
} INTERCAP;

typedef struct funcvar {
    char name[NAMESIZE];
    short help;
    short type;
    short ind[2];
} FUNCVAR;

typedef struct funcdescr {
    char name[NAMESIZE];
    char dmpath[MAXDMPATH + 1];
    short help;
    short fvx;
    short fvx_cnt;
    short offsx;     /* offset to find real state variables in FS[] */
} FUNCDESCR;

typedef struct binref
{
    int a;
    int b;
    short c;
    short d;
    short e;
    char f[12];
    float g;
    float h;
    unsigned i : 8;
    unsigned j : 8;
    unsigned k : 8;
    unsigned l : 8;
    int m;
} BINREF;

/* The following 3 structures are for backwards compatibility with
   old finary files in which NAMESIZE was 33. */

typedef struct old2_modeltable {
    char name[33];  /* name of this model */
    int nt_cnt;           /* number of names within this model */
} OLD2_MODELTABLE;

typedef struct old2_funcvar {
    char name[33];
    short help;
    short type;
    short ind[2];
} OLD2_FUNCVAR;

typedef struct old2_funcdescr {
    char name[33];
    char dmpath[MAXDMPATH + 1];
    short help;
    short fvx;
    short fvx_cnt;
    short offsx;     /* offset to find real state variables in FS[] */
} OLD2_FUNCDESCR;

/* The following 3 structures are for backwards compatibility with
   old finary files in which NAMESIZE was 15. */

typedef struct old_modeltable {
    char name[15];  /* name of this model */
    int nt_cnt;           /* number of names within this model */
} OLD_MODELTABLE;

typedef struct old_funcvar {
    char name[15];
    short help;
    short type;
    short ind[2];
} OLD_FUNCVAR;

typedef struct old_funcdescr {
    char name[15];
    char dmpath[MAXDMPATH + 1];
    short help;
    short fvx;
    short fvx_cnt;
    short offsx;     /* offset to find real state variables in FS[] */
} OLD_FUNCDESCR;
