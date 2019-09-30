/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

typedef long long simtime_t;

typedef struct node {
    int ntx;                 /* index in NT[] */
    int dsx;                 /* index in DS[] */
    int cx;                  /* index in C[] */
    unsigned funcoutp : 2;   /* functional block output ? */
    unsigned linked : 2;     /* node already linked to transistors ? */
			     /* if yes, then cx gives the index of that node */
    unsigned redirect : 2;   /* has node been redirected to another node ? */
    unsigned flag : 2;       /* general purpose flag */
    /* the above members are common with PRE_NODE */
    unsigned type : 2;       /* Forced or Normal */
    unsigned inp : 2;        /* network input ? */
    unsigned outp : 2;       /* network output ? */
    unsigned breaksig : 2;   /* break signal valid for this node ? */
    unsigned essential : 2;  /* essential node ? */
    unsigned thisvicin : 2;  /* must the vicinity of this node be evaluated ? */
    unsigned dissip : 2;     /* dissipation for node is calculated */
    unsigned plotevent : 2;
    unsigned evalflag : 2;   /* set when vicinity (still) has to be evaluated */
    unsigned state : 2;      /* current logic state \                         */
    unsigned nextstate : 2;  /* next logic state     > only valid when node   */
    unsigned stabstate : 2;  /* stable logic state  /            is essential */
    unsigned svmin : 8;
    unsigned svmax : 8;
    unsigned ivmin : 8;
    unsigned ivmax : 8;
    short plot;
    short print;
    simtime_t Ttmin, Ttmax;
    simtime_t tstabmin, tstabmax;
    float dyncap;            /* dynamical capacitance */
    float statcap;           /* static capacitance */
    struct forcedsignal * forcedinfo;  /* info about the forced signal when
					  the node is Forced */
    struct nevalinfo * ei;   /* extra evaluation info */
} NODE;

typedef struct transistor {
    int gate;
    int source;
    int drain;
    float width;
    float length;            /* or resistances when type is Res */
    unsigned type     : 16;  /* Nenh, Penh, Depl or Res */
    unsigned flag     : 16;  /* general purpose flag    */
    /* the above members are common with PRE_TRANSISTOR */
    unsigned state    : 16;
    unsigned premode  : 16;
    int * attr;              /* transistor process parameters */
    struct tevalinfo * ei;   /* extra evaluation info */
} TRANSISTOR ;

typedef struct node_ref_list {
    int nx;
    int * xptr;
    struct node_ref_list * next;
} NODE_REF_LIST;

typedef struct path_spec
{
    char name[DM_MAXNAME + 1];
    int xarray[MAXDIM+1][2];
    struct path_spec * next;
    struct path_spec * also;
} PATH_SPEC;

typedef struct res_path
{
    PATH_SPEC * path;
    int totnum;
    struct res_path * next;
} RES_PATH;

typedef struct res_file
{
    char name[DM_MAXNAME + 1];
    RES_PATH * signals;
    int sig_cnt;
    long offset;
    double scaling;
    FILE * fp;
    struct res_file * next;
} RES_FILE;

typedef struct string_ref
{
    char * str;
    struct string_ref * next;
} STRING_REF;

typedef struct forcedsignal {
    struct signalelement * insignal;
    short initfstate;                /* the initial logical state */
    short nextfstate;                /* the next    logical state */
    short stabfstate;                /* the stable  logical state */
    float sigmult;
    simtime_t tswitch;
    simtime_t tswitch_stab;
    int fox;          	     	   /* the accompanying function output */
    struct forcedsignal *next;     /* for a node the forcedinfo's form a list */
} FORCEDSIGNAL;

typedef struct signalelement
{
    int val;
    simtime_t len;
    struct signalelement *sibling;
    struct signalelement *child;
} SIGNALELEMENT;

typedef struct signalevent
{
    int val;
    simtime_t time;
} SIGNALEVENT;

typedef struct history_list
{
    simtime_t t;
    int ntx;  /* moet nattuurlijk nx heten */
    struct history_list * next;
} HISTORY_LIST ;

typedef struct group
{
    unsigned state       : 4;    /* isolated group state */
    unsigned strength    : 4;    /* Strong or Weak */
    unsigned unprotected : 8;
    unsigned agglomdone  : 16;   /* agglomeration of this group is done */
    float qmin;
    float qmax;
    float cap;
    float svmin1;
    float svmax1;
    float svmin2;
    float svmax2;
    NODE ** nodes;     /* nodes in group */
    NODE ** nbs;       /* neighbor nodes of group */
    int node_cnt;
    int nb_cnt;
} GROUP ;

typedef struct vicinity
{
    unsigned type      : 4;   /* Forced when Forced nodes, else Normal */
    unsigned forcedsH  : 4;   /* there are Forced nodes with H_state */
    unsigned forcedsL  : 4;   /* there are Forced nodes with L_state */
    unsigned forcedsX  : 4;   /* there are Forced nodes with X_state */
    unsigned undeftors : 8;   /* there are undefined state transistors */
    unsigned floatpossible : 8;    /* floating nodes are possible */
    float capHtot;              /* capacitance of H stable state groups */
    float capLtot;              /* capacitance of L stable state groups */
    float capXtot;              /* capacitance of X stable state groups */
    NODE ** nodes;         /* nodes which form the vicinity */
    TRANSISTOR ** tors;    /* transistors in the vicinity */
    GROUP * groups;        /* groups in the vicinity with no Forced nodes */
    int node_cnt;
    int tor_cnt;
    int group_cnt;
} VICINITY ;

typedef struct agglomeration
{
    float capHtot;
    float capLtot;
    float capXtot;
    int group_cnt;
    GROUP ** groups;
} AGGLOMERATION ;

typedef struct nevalinfo
{
    unsigned eventpending : 2;  /* Normal event is on event list */
    unsigned lstate : 2;        /* current LSTATE (only for essential) */
    unsigned bcH_done : 2;
    unsigned wcH_done : 2;
    unsigned bcL_done : 2;
    unsigned wcL_done : 2;
    unsigned done : 2;
    unsigned ontrack : 2;
    unsigned undetermined : 2;
    unsigned common : 2;
    unsigned dc : 2;
    unsigned source : 2;
    unsigned posfloat : 2;
    unsigned accumulated : 2;
    unsigned backtraced : 2;
    unsigned leaf : 2;
    float bcH_r;
    float wcH_r;
    float bcL_r;
    float wcL_r;
    float r;
    float umin;        /* current min and max voltages,      */
    float umax;        /* (not valid for Forced nodes)       */
    float svmin;       /* must use float temporarily because */
    float svmax;       /* of abstract model simulation       */
    GROUP * g;       /* group to which node belongs */
    NODE * bcH_path;
    NODE * wcH_path;
    NODE * bcL_path;
    NODE * wcL_path;
    NODE * path;
    NODE * bcH_dir;
    NODE * wcH_dir;
    NODE * bcL_dir;
    NODE * wcL_dir;
    NODE * dir;
    float r_dir;
    float accumcharge;
    float TD;
    int mode;
    float dyncap;
    float min_resist;
    unsigned path_flag;
    unsigned xtorpath;
    NODE * min_path;
} NEVALINFO ;

typedef struct tevalinfo
{
    float resist;
    unsigned mode;
} TEVALINFO ;

typedef struct upair
{
    float umin;
    float umax;
} UPAIR ;

typedef struct abstract_output
{
    char *name;
    struct node_ref_list *inputs;
    int nr_inputs;
    struct abstract_value *vals;
    struct abstract_output *next;
} ABSTRACT_OUTPUT ;

typedef struct abstract_value
{
    int *in_values;
    char *out_value;
    struct abstract_value *next;
} ABSTRACT_VALUE ;

typedef struct ftab_el
{
    float x;
    float fx;
} FTAB_EL ;

typedef struct specif
{
    float lmin;
    float wmin;
    float loffset;
    float woffset;
    FTAB_EL lftab[MAXFTAB + 1];
    FTAB_EL wftab[MAXFTAB + 1];
    int ldepend;
    int wdepend;
} SPECIF ;

typedef struct genspecifs
{
    SPECIF rstat;
    SPECIF rsatu;
    SPECIF cgstat;
    SPECIF cgrise;
    SPECIF cgfall;
    SPECIF cestat;
    SPECIF cerise;
    SPECIF cefall;
} GENSPECIFS ;

typedef struct modespecifs
{
    SPECIF rdyn;
    SPECIF cch;
} MODESPECIFS ;

typedef struct enhspecifs
{
    int tspecdef;
    GENSPECIFS general;
    MODESPECIFS pullup;
    MODESPECIFS pulldown;
    MODESPECIFS passup;
    MODESPECIFS passdown;
} ENHSPECIFS ;

typedef struct deplspecifs
{
    int tspecdef;
    GENSPECIFS general;
    MODESPECIFS load;
    MODESPECIFS superload;
    MODESPECIFS badload;
} DEPLSPECIFS ;

typedef struct genparams
{
    float rstat;
    float rsatu;
    float cgstat;
    float cgrise;
    float cgfall;
    float cestat;
    float cerise;
    float cefall;
} GENPARAMS ;

typedef struct modeparams
{
    float rdyn;
    float cch;
} MODEPARAMS ;

typedef struct enhparams
{
    float length;
    float width;
    GENPARAMS general;
    MODEPARAMS pullup;
    MODEPARAMS pulldown;
    MODEPARAMS passup;
    MODEPARAMS passdown;
    struct enhparams * nextl;
    struct enhparams * nextw;
} ENHPARAMS ;

typedef struct deplparams
{
    float length;
    float width;
    GENPARAMS general;
    MODEPARAMS load;
    MODEPARAMS superload;
    struct deplparams * nextl;
    struct deplparams * nextw;
} DEPLPARAMS ;

typedef struct interruptmask
{
    unsigned on;
    unsigned timebreak;
    unsigned outputchange;
    unsigned signalbreak;
} INTERRUPTMASK ;

