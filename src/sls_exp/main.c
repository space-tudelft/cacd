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

#include "src/sls_exp/extern.h"
#include <signal.h>

extern void compact (void);
extern void deb_all (void);
extern void deb_mem (void);
extern int devtype (char *name);
extern int findnodes (PATH_SPEC *path, MODELCALLTABLE *mcall, int hashed, NODE_REF_LIST **return_list, int permanent);
extern int functype (char *name, int imported, time_t *t, char *otherproject);
extern void getdev (DM_CELL *m);
extern void getnet (DM_CELL *m);
extern void getterm (DM_CELL *m);
extern int  hcreate (int size);
extern void hdestroy (void);
extern void inbin (CHILD_LIST *child);
extern int  is_ap (void);
extern int  is_func (char *a);
extern void inint (DM_STREAM *dsp, int *var);
extern void join_node (int nxh, int nxj);
extern void linkntw (char *model);
extern int  mksim (void);
extern void monitime (char *str);
extern void outbin (DM_CELL *m);
extern void resetfindnodes (void);
extern void rmnoterm (void);
extern void sort (void);
extern int  stdfunctype (char *name);
extern void startmonitime (char *pname);
extern void stopmonitime (void);

#ifdef OOPS
static void int_hdl (int sig);
#endif
static int checkBinEndmark (DM_STREAM *dsp);
#if 0
static int checkGlobNets (DM_STREAM *dsp);
static void readGlobalNets (void);
#endif
static void createGlobNodes (void);
static time_t expand (char *father, int isroot);
static void getexpsizes (CHILD_LIST *child);
static int isGlobalNet (char *s);
static time_t isupdate (DM_CELL *key_m, char *m_name);
static CHILD_LIST *make_child_list (DM_CELL *key_father, char *father, time_t *t);
static CALL_LIST *append_call (CHILD_LIST **begin_child, char *object, int imported);
static void predictsizes (DM_CELL *m);

FUNCVAR * FV;
FUNCDESCR * FD;
char * ST;
NAMETABLE * NT;
MODELTABLE * MT;
CONTEXTTABLE * CTT;
MODELCALLTABLE * MCT;
int * XT;
int * XX;
NODE * N;
int * DS;
CONTROL * C;
INTERCAP * I;
TRANSISTOR * T;
FUNCTION * F;
int * FI;
int * FR;
FUNCOUT * FO;
char * FS;

int FV_cnt;
int FD_cnt;
int ST_cnt;
int NT_cnt;
int MT_cnt;
int CTT_cnt;
int MCT_cnt;
int XT_cnt;
int XX_cnt;
int N_cnt;
int DS_cnt;
int C_cnt;
int T_cnt;
int F_cnt;
int I_cnt;
int FI_cnt;
int FR_cnt;
int FO_cnt;
int FS_cnt;

int FV_size;
int FD_size;
int ST_size;
int NT_size;
int MT_size;
int CTT_size;
int MCT_size;
int XT_size;
int XX_size;
int N_size;
int DS_size;
int C_size;
int T_size;
int F_size;
int I_size;
int FI_size;
int FR_size;
int FO_size;
int FS_size;

int mcs_NT_cnt;
int mcs_T_cnt;
int mcs_F_cnt;
int mcs_I_cnt;

FILE * debug;

char **globNets;
int *globNetsNx;
int *globNetsCheck;
int globNets_cnt;

#define GLOBNETFILE "global_nets"

char *argv0 = "sls_exp";
char *use_msg = "\nUsage: %s [-fgmst] cell\n\n";

DM_PROJECT *dmproject;

int debugdata = FALSE;
int debugmem = FALSE;
int monitoring = FALSE;

int debugcomp = FALSE;

int force_exp = FALSE;
int silent = FALSE;
int rm_noterm = FALSE;
int capcoupling = FALSE;
int nocompaction = FALSE;
int cirflag = FALSE;
char viewtype[BUFSIZ];

char sls_fn[BUFSIZ];
char sls_o_fn[BUFSIZ];

time_t newest_ftime;
time_t time_first_bin = -1;   /* only used when force_exp == TRUE */

int main (int argc, char *argv[])
{
    char * network = NULL;
    char * f;
    int expanded;

    strcpy (viewtype, CIRCUIT);

    while (--argc > 0) {
	if ((*++argv)[0] == '-') {
	    for (f = (*argv) + 1; *f != '\0'; f++) {
		switch (*f) {
		    case 'f' :
			force_exp = TRUE;
			break;
		    case 'g' :
			debugcomp = TRUE;
			break;
		    case 's' :
			silent = TRUE;
			break;
		    case 't' :
			rm_noterm = TRUE;
			break;
		    case 'm' :
			monitoring = TRUE;
			break;
		    case '%' :
			switch (*++f) {
			    case 'l' :
				capcoupling = TRUE;
				break;
		            case 'd' :
			        debugdata = TRUE;
			        break;
		            case 'm' :
			        debugmem = TRUE;
			        break;
		            case 'n' :
			        nocompaction = TRUE;
			        break;
			    default :
				break;
			}
			break;
		    default :
			fprintf (stderr, "Illegal option: %c\n", *f);
			exit (1);
			break;
		}
	    }
	}
	else {
	    network = *argv;
	}
    }

    if (network == NULL) {
        fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    if (monitoring) startmonitime (argv0);

#ifdef OOPS
    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
        signal (SIGINT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        signal (SIGQUIT, int_hdl);
        /* only when value was not SIG_IGN, a jump must be done to int_hdl */
#endif
    signal (SIGTERM, int_hdl);
    signal (SIGILL, int_hdl);
    signal (SIGFPE, int_hdl);
#ifdef SIGBUS
    signal (SIGBUS, int_hdl);
#endif
    signal (SIGSEGV, int_hdl);
#endif

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    /* from now on the program must always exit via die () */

    sprintf (sls_fn, "sls");   /* name of sls binary file */
    sprintf (sls_o_fn, "sls.o");  /* name of cell sls object file */

    if (debugdata || debugmem) {
	OPENW (debug, "deb");
	setbuf (debug, (char *)NULL);
    }

    FD_size = 10;
    FV_size = 25;
    PALLOC (FD, FD_size, FUNCDESCR);
    PALLOC (FV, FV_size, FUNCVAR);
    FD_cnt = 0;
    FV_cnt = 0;

    newest_ftime = 0;

    globNets_cnt = 0;
    /* readGlobalNets ();
       This is not the current strategy.  Instead, global
       nets are handled by database input tools (e.g. sls_mkdb).
       See also call to "checkGlobNets"
    */

    if (expand (network, 1) >= 0)
	expanded = FALSE;
    else
	expanded = TRUE;

    mksim ();

    if (debugdata || debugmem) {
	CLOSE (debug);
    }

    if (dmproject) dmCloseProject (dmproject, COMPLETE);

    dmQuit ();  /* all models that are still checked out, are now checked in */

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
        signal (SIGINT, SIG_DFL);
#ifdef SIGQUIT
    if (signal (SIGQUIT, SIG_IGN) != SIG_IGN)
        signal (SIGQUIT, SIG_DFL);
#endif
    signal (SIGTERM, SIG_DFL);
    signal (SIGILL, SIG_DFL);
    signal (SIGFPE, SIG_DFL);
#ifdef SIGBUS
    signal (SIGBUS, SIG_DFL);
#endif
    signal (SIGSEGV, SIG_DFL);

    if (monitoring) stopmonitime ();

    return (0);
}

#ifdef OOPS
static void int_hdl (int sig) /* interrupt handler */
{
    switch (sig) {
        case SIGILL :
            fprintf (stderr, "Illegal instruction\n");
            break;
        case SIGFPE :
            fprintf (stderr, "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS :
            fprintf (stderr, "Bus error\n");
            break;
#endif
        case SIGSEGV :
            fprintf (stderr, "Segmentation violation\n");
            break;
    }

    die (sig);
}
#endif

void die (int nr)
{
    if (dmproject) dmCloseProject (dmproject, COMPLETE);

    dmQuit ();

    exit (nr);
}

/* expands the network father recursively
 * first the models in the lower hierarchy are expanded (if neccessary)
 * if a new binary file is created -1 is returned, else it returns
 * the last modification time of the binary file.
 */
static time_t expand (char *father, int isroot)
{
    int ntx;
    int i;
    int new;
    int cnt;
    int uptodate;
    DM_CELL * key_father;
    DM_PROJECT * new_projkey;
    DM_CELL * impcell_key;
    DM_STREAM * dsp;
    char * other_name;
    struct stat buf;
    struct stat buf_bin;
    CHILD_LIST * beg_child_list;
    CHILD_LIST * child_follow;
    CALL_LIST * follow;
    char mstr[35];
    time_t child_time;
    time_t newest_time;
    time_t father_time;

    if (isroot)
	key_father = dmCheckOut (dmproject, father, WORKING, DONTCARE, CIRCUIT, ATTACH);
    else
	key_father = dmCheckOut (dmproject, father, ACTUAL, DONTCARE, CIRCUIT, ATTACH);

    newest_time = 0;
    beg_child_list = make_child_list (key_father, father, &newest_time);

    uptodate = TRUE;
    child_follow = beg_child_list;
    while (child_follow) {
	if (child_follow -> imported) {
	    new_projkey = dmFindProjKey (IMPORTED, child_follow -> object, dmproject, &other_name, CIRCUIT);
	    impcell_key = dmCheckOut (new_projkey, other_name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
            if (dmStat (impcell_key, sls_fn, &buf) != 0) {
		dberror (NULL, -1, "No binary file present for imported cell", child_follow -> object);
	    }
	    else {
		if ((dsp = dmOpenStream (impcell_key, sls_fn, "r")) == NULL)
		    die (1);
		if (checkBinEndmark (dsp)) {
		    dberror (NULL, -1, "Incompatible binary file for imported cell", child_follow -> object);
		}
		dmCloseStream (dsp, COMPLETE);

		if (buf.st_mtime > newest_time) newest_time = buf.st_mtime;
	    }
	    dmCheckIn (impcell_key, COMPLETE);
	}
	else {

	    /* first expand each child ! */

	    if ((child_time = expand (child_follow -> object, 0)) < 0)
		uptodate = FALSE;
	    else if (child_time > newest_time)
		newest_time = child_time;
	}
	child_follow = child_follow -> next;
    }

    if ( uptodate ) {
        father_time = isupdate (key_father, father);
        if (father_time < 0 || father_time < newest_time
        || (force_exp && (time_first_bin < 0 || father_time < time_first_bin)))
            uptodate = FALSE;
    }

    if ( uptodate ) {
	dmCheckIn (key_father, COMPLETE);
	return (father_time);
        /* the current binary file for 'father' is up to date */
    }

    if (monitoring) {
	sprintf (mstr, "B expand %s", father);
	monitime (mstr);
    }

    if (!silent)
        fprintf (stderr, "Expanding %s\n", father);

    if (debugdata || debugmem)
        fprintf (debug, "\n##### Expanding %s #####\n\n", father);

    for (i = 0; i < FV_cnt; i++) FV[i].help = 0;
    for (i = 0; i < FD_cnt; i++) FD[i].help = 0;

    ST_size = 1;
    NT_size = 1;
    MT_size = 1;  /* sufficient */
    CTT_size = 1;  /* sufficient */
    MCT_size = 1;
    XT_size = 1;
    XX_size = 1;
    N_size = 1;
    DS_size = 1;
    C_size = 1;
    T_size = 1;
    F_size = 1;
    I_size = 1;
    FI_size = 1;
    FR_size = 1;
    FO_size = 1;
    FS_size = 1;

    predictsizes (key_father);

    NT_size = MAX (NT_size, 100);
    ST_size = 8 * NT_size;
    N_size = MAX (N_size, 100);
    DS_size = MAX (100, 4 * T_size);
			     /* beware of connections within submodels ! */
    C_size = MAX (100, 2 * (T_size + F_size + I_size));  /* idem */

    hcreate (NT_size);

    child_follow = beg_child_list;
    while (child_follow) {
	getexpsizes (child_follow);
	child_follow = child_follow -> next;
    }

    PALLOC (ST, ST_size, char);
    PALLOC (NT, NT_size, NAMETABLE);
    PALLOC (MT, MT_size, MODELTABLE);
    PALLOC (CTT, CTT_size, CONTEXTTABLE);
    PALLOC (MCT, MCT_size, MODELCALLTABLE);
    PALLOC (XT, XT_size, int);
    PALLOC (XX, XX_size, int);
    PALLOC (N, N_size, NODE);
    PALLOC (DS, DS_size, int);
    PALLOC (C, C_size, CONTROL);
    PALLOC (T, T_size, TRANSISTOR);
    PALLOC (F, F_size, FUNCTION);
    PALLOC (I, I_size, INTERCAP);
    PALLOC (FI, FI_size, int);
    PALLOC (FR, FR_size, int);
    PALLOC (FO, FO_size, FUNCOUT);
    PALLOC (FS, FS_size, char);

    ST_cnt = 0;
    NT_cnt = 0;
    MT_cnt = 0;
    CTT_cnt = 0;
    MCT_cnt = 0;
    XT_cnt = 0;
    XX_cnt = 0;
    N_cnt = 0;
    DS_cnt = 0;
    C_cnt = 0;
    T_cnt = 0;
    F_cnt = 0;
    I_cnt = 0;
    FI_cnt = 0;
    FR_cnt = 0;
    FO_cnt = 0;
    FS_cnt = 0;

    if (debugmem) {
        fprintf (debug, "Initial memory usage:\n");
        deb_mem ();
    }

    child_follow = beg_child_list;
    while (child_follow) {
	inbin (child_follow);
	child_follow = child_follow -> next;
    }

    mcs_NT_cnt = NT_cnt;
    mcs_T_cnt = T_cnt;
    mcs_F_cnt = F_cnt;
    mcs_I_cnt = I_cnt;

    child_follow = beg_child_list;
    while (child_follow) {

	follow = child_follow -> calls;
        while (follow) {

	    ntx = newname (follow -> inst_name);
	    NT[ntx].sort = Modelcall;
	    NT[ntx].x = follow -> mcx;
            if ( follow -> xarray[0][0] > 0 ) {
		new = NT[ntx].xtx = newxt ();
	        cnt = XT[ new ] = follow -> xarray[0][0];
	        for (i = 1; i <= cnt; i++) {
		    new = newxt ();
		    XT[ new ] = follow -> xarray[i][0];
		    new = newxt ();
		    XT[ new ] = follow -> xarray[i][1];
	        }
	    }
	    else
	        NT[ntx].xtx = -1;

	    follow = follow -> next;
        }

	child_follow = child_follow -> next;
    }

    getterm (key_father);

    getdev (key_father);

    getnet (key_father);

    if (globNets_cnt > 0) createGlobNodes ();

    sort ();

    strncpy (MT[ MT_cnt ].name, father, NAMESIZE - 1);
    MT[ MT_cnt ].nt_cnt = NT_cnt - mcs_NT_cnt;
    MT_cnt++;

    CTT[ CTT_cnt ].ceiling = N_cnt;
    CTT[ CTT_cnt ].mctx = -1;
    CTT_cnt++;

    linkntw (father);

    if (!nocompaction) compact ();

    if (rm_noterm) rmnoterm ();

    if (debugmem) {
        fprintf (debug, "Final memory usage:\n");
        deb_mem ();
    }

    if (debugdata) deb_all ();

    outbin (key_father);

    if (force_exp) {
        if (time_first_bin < 0) {    /* no binary has been made yet */
            if (dmStat (key_father, sls_fn, &buf_bin) == 0) {
	        time_first_bin = buf_bin.st_mtime;
            }
        }
    }

    hdestroy ();

    CFREE (ST);
    CFREE (NT);
    CFREE (MT);
    CFREE (CTT);
    CFREE (MCT);
    CFREE (XT);
    CFREE (XX);
    CFREE (N);
    CFREE (DS);
    CFREE (C);
    CFREE (T);
    CFREE (F);
    CFREE (I);
    CFREE (FI);
    CFREE (FR);
    CFREE (FO);
    CFREE (FS);

    dmCheckIn (key_father, COMPLETE);

    if (monitoring) {
	sprintf (mstr, "E expand %s", father);
	monitime (mstr);
    }

    return (-1);  /* because this model was not up to date */
}

/* makes a child list of model calls in model 'father'
 * and returns the first element of the list
 * in t the time of the newest function will be put
 */
static CHILD_LIST *make_child_list (DM_CELL *key_father, char *father, time_t *t)
{
    char attribute_string[256];
    long lower[10], upper[10];
    int i;
    time_t t_tmp;
    DM_STREAM * dsp_mc;
    CHILD_LIST * begin_list = NULL;
    CALL_LIST * call;
    struct stat buf;

    *t = 0;

    if (dmStat (key_father, "net", &buf) != 0) {
	dberror (NULL, -1, "Cannot read circuit view for", father);
    }

    dsp_mc = dmOpenStream (key_father, "mc", "r");

    dm_get_do_not_alloc = 1;
    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    while (dmGetDesignData (dsp_mc, CIR_MC) > 0) {
        if (is_ap ()) continue;

        if (devtype (cmc.cell_name) >= 0) continue;

        if (is_func (cmc.inst_attribute)) {
	    if (stdfunctype (cmc.cell_name) < 0) {
                if (functype (cmc.cell_name, (int)cmc.imported, &t_tmp, NULL) < 0) {
		    dberror (father, -1, "call to unknown function", cmc.cell_name);
                }
                if (t_tmp > *t) *t = t_tmp;
	    }
            continue;
	}

        call = append_call (&begin_list, cmc.cell_name, (int)cmc.imported);

	strcpy (call -> inst_name, cmc.inst_name);

        call -> xarray[0][0] = 0;

        if (cmc.inst_dim > 0) {
            call -> xarray[0][0] = cmc.inst_dim;
            for (i = 0; i < cmc.inst_dim; i++) {
                call -> xarray[i+1][0] = cmc.inst_lower[i];
                call -> xarray[i+1][1] = cmc.inst_upper[i];
            }
        }

	call -> number = 1;
	for (i = 1; i <= call -> xarray[0][0]; i++) {
	    call -> number = call -> number * (call -> xarray[i][1] - call -> xarray[i][0] + 1);
	}
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp_mc, COMPLETE);

    return (begin_list);
}

/* appends a call of model 'object' to the child list 'begin_child'
 * and returns the element
 */
static CALL_LIST *append_call (CHILD_LIST **begin_child, char *object, int imported)
{
    CHILD_LIST *prev_child, *child;
    CALL_LIST *call, *follow;

    prev_child = NULL;
    child = *begin_child;
    while (child && strcmp (child -> object, object) != 0) {
	prev_child = child;
	child = child -> next;
    }

    if (child == NULL) {
	PALLOC (child, 1, CHILD_LIST);
	child -> next = NULL;
	child -> calls = NULL;
	if (imported == IMPORTED)
	    child -> imported = TRUE;
	else
	    child -> imported = FALSE;
        strcpy (child -> object, object);

        if (*begin_child == NULL)
	    *begin_child = child;
	else
	    prev_child -> next = child;
    }
    else {
	if (imported == IMPORTED)
	    child -> imported = TRUE;
    }

    PALLOC (call, 1, CALL_LIST);
    call -> next = NULL;
    if (child -> calls == NULL)
	child -> calls = call;
    else {
	follow = child -> calls;
	while (follow -> next) {
	    follow = follow -> next;
	}
	follow -> next = call;
    }

    return (call);
}

/* predicts the incrementions of the datastructure sizes, due to the model 'm'
 * this procedure is quite heuristic
 */
static void predictsizes (DM_CELL *m)
{
    char attribute_string[256];
    long lower[10], upper[10];
    int nbr, i, dev;
    DM_STREAM * dsp;

    dsp = dmOpenStream (m, "mc", "r");

    dm_get_do_not_alloc = 1;
    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
        if (is_ap ()) continue;

        NT_cnt++;        /* for instance name */

        nbr = 1;
	for (i = 0; i < cmc.inst_dim; ++i) {
	    nbr += cmc.inst_upper[i] - cmc.inst_lower[i];
	}

        if ((dev = devtype (cmc.cell_name)) >= 0) {
	    switch (dev) {
                case D_NENH :
                case D_PENH :
                case D_DEPL :
                case D_RES :
		    T_size += nbr;
		    break;
	        case D_CAP :
		    I_size += nbr;
		    break;
	    }
	}

	if (is_func (cmc.inst_attribute)) {
	    F_size += nbr;
	    FI_size += (nbr * 5);
	    FR_size += (nbr * 3);
	    FO_size += (nbr * 2);
	    FS_size += (nbr * 12);
	}
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp, COMPLETE);

    N_size += 2 * (T_size + F_size + I_size);  /* for the net and term file */
    NT_size += 2 * (T_size + F_size + I_size); /* for the net and term file */
    XX_size += 20; /* for the net and term file */
}

/* increments the datastructure sizes with the values
 * required for when the expanded model calls 'child' are read
 */
static void getexpsizes (CHILD_LIST *child)
{
    int size;
    int nbr;
    CALL_LIST * follow;
    DM_CELL * key_child;
    DM_STREAM * dsp_bin;
    DM_PROJECT * new_projkey;
    char * other_name;
    int mark;
    int oldbinformat = 0;

    nbr = 0;      /* calculate number of real calls for model 'child' */
    for (follow = child -> calls; follow; follow = follow -> next) {
        nbr += follow -> number;
    }

    if (child -> imported) {
	new_projkey = dmFindProjKey (IMPORTED, child -> object,
				     dmproject, &other_name, CIRCUIT);
	key_child = dmCheckOut (new_projkey, other_name, ACTUAL,
				DONTCARE, CIRCUIT, READONLY);
    }
    else {
	key_child = dmCheckOut (dmproject, child -> object, ACTUAL,
				DONTCARE, CIRCUIT, READONLY);
    }

    dsp_bin = dmOpenStream (key_child, sls_fn, "r");

    dmSeek (dsp_bin, -(long) (sizeof (BINREF) + sizeof (int)), 2);
    inint (dsp_bin, &mark);
    if (mark == MARK_OLD2_NAMESIZE) {
        oldbinformat = 2;
    }
    else if (mark != MARK_NEW_NAMESIZE) {
        oldbinformat = 1;
    }

    dmSeek (dsp_bin, (long)0, 0);  /* rewind */

    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    ST_size += size;
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    if (oldbinformat == 2)
        MT_size += size / sizeof (OLD2_MODELTABLE);
    else if (oldbinformat == 1)
        MT_size += size / sizeof (OLD_MODELTABLE);
    else
        MT_size += size / sizeof (MODELTABLE);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    NT_size += nbr * size / sizeof (NAMETABLE);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    CTT_size += nbr * size / sizeof (CONTEXTTABLE);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    MCT_size += nbr + nbr * size / sizeof (MODELCALLTABLE);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    XT_size += nbr * size / sizeof (int);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    XX_size += nbr * size / sizeof (int);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    N_size += nbr * size / sizeof (NODE);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    DS_size += nbr * size / sizeof (int);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    C_size += nbr * size / sizeof (CONTROL);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    T_size += nbr * size / sizeof (TRANSISTOR);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    F_size += nbr * size / sizeof (FUNCTION);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    I_size += nbr * size / sizeof (INTERCAP);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    FI_size += nbr * size / sizeof (int);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    FR_size += nbr * size / sizeof (int);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    FO_size += nbr * size / sizeof (FUNCOUT);
    inint (dsp_bin, &size);
    dmSeek (dsp_bin, (long)size, 1);
    FS_size += nbr * size;

    dmCloseStream (dsp_bin, COMPLETE);
    dmCheckIn (key_child, COMPLETE);
}

/* returns last modification date of m if 'm' is update
 * that is, if the files describing 'm' in the database
 * are older than the current binary file and the
 * endmark is present in the binary file else -1 is returned
 */
static time_t isupdate (DM_CELL *key_m, char *m_name)
{
    DM_STREAM * dsp;
    int is = 1;
    struct stat buf;
    struct stat buf_bin;

    if (dmStat (key_m, sls_fn, &buf_bin) != 0) {
	is = 0;
	goto complete;
    }

    if ((dsp = dmOpenStream (key_m, sls_fn, "r")) == NULL) die (1);
    if (checkBinEndmark (dsp)) is = 0;
    /* see call to "readGlobalNets"
    if (is && checkGlobNets (dsp)) {
	is = 0;
    }
    */
    dmCloseStream (dsp, COMPLETE);

    if (is && (dmStat (key_m, "term", &buf) != 0 || buf_bin.st_mtime < buf.st_mtime)) is = 0;
    if (is && (dmStat (key_m, "net" , &buf) != 0 || buf_bin.st_mtime < buf.st_mtime)) is = 0;

complete:
    if (is) return (buf_bin.st_mtime);
    return (-1);
}

int is_ap ()
{
    /* check for special area/perimeter entries: "$torname$ds" */
    return (cmc.cell_name[0] == '$' && !strcmp (cmc.cell_name + strlen (cmc.cell_name) - 3, "$ds"));
}

/* looks in the attribute string whether it is a function */
int is_func (char *a)
{
    int is = FALSE;
    int colon = TRUE;

    if (a == (char *) 0) return (FALSE);

    while (*a != '\0' && ! is) {
	if (colon) {
	    if (*a == 'f' && (*(a + 1) == ';' || *(a + 1) == '\0'
                              || (*(a + 1) == '=' && *(a + 2) != '0')))
		is = TRUE;
	}
	if (*a == ';')
	    colon = TRUE;
	else
	    colon = FALSE;
	a++;
    }

    return (is);
}

/* puts in p the next parameter, in v the next value,
 * and returns the rest of the attribute string 'a'
 * (uses the part of 'a' that has been read !)
 */
char *next_attr (char **p, char **v, char *a)
{
    char * ret;
    char pc;

    *p = a;
    *v = *p + 1;
    pc = 'a';
    while (**v != '\0' && ((**v != '=' && **v != ';') || pc == '\\'))
	pc = *((*v)++);
    if (**v == '\0') {
	ret = NULL;
	*v = NULL;
    }
    else if (**v == ';') {
	**v = '\0';
	ret = *v + 1;
	*v = NULL;
    }
    else {  /* **v == '=' */
	**v = '\0';
        ret = ++(*v);
	pc = 'a';
        while (*ret != '\0' && (*ret != ';' || pc == '\\'))
	    pc = *(ret++);
	if (*ret == '\0')
	    ret = NULL;
	else {
	    *ret = '\0';
	    ret++;
	}
    }

    return (ret);
}

static int checkBinEndmark (DM_STREAM *dsp)
{
    BINREF BR;

    dmSeek (dsp, -(long) sizeof (BINREF), 2);
    fread ((char *)&BR, sizeof (BINREF), 1, dsp -> dmfp);

    if (BR.a != 1234
    || BR.b != -1234
    || BR.c != 86
    || BR.d != 0
    || BR.e != -456
    || strcmp (BR.f, "qwertyuiopa") != 0
    || BR.g < 0.99999 || BR.g > 1.00001
    || BR.h < -20.02010 || BR.h > -20.01990
    || BR.i != 5
    || BR.j != 13
    || BR.k != 18
    || BR.l != 255
    || BR.m != 28282)
        return (1);
    else
        return (0);
}

#if 0
static void readGlobalNets ()
{
    FILE *fp;
    int cnt;
    int i;
    char *fn;
    char buf[128];

    fp = fopen (GLOBNETFILE, "r");
    if (fp == NULL) {
        fn = (char *)dmGetMetaDesignData (PROCPATH, dmproject, GLOBNETFILE);
	fp = fopen (fn, "r");
    }

    if (fp) {

	cnt = 0;
	while (fscanf (fp, "%s", buf) > 0) {
	    cnt++;
	}
	rewind (fp);

	PPALLOC (globNets, cnt, char);
	PALLOC (globNetsNx, cnt, int);
	PALLOC (globNetsCheck, cnt, int);

	cnt = 0;
	while (fscanf (fp, "%s", buf) > 0) {
	    for (i = 0; i < globNets_cnt; i++) {
		if (strcmp (globNets[i], buf) == 0)
		    break;   /* double specification of this global net */
	    }
	    if (i == globNets_cnt) {
		PALLOC (globNets[cnt], strlen (buf) + 1, char);
		strcpy (globNets[cnt], buf);
		cnt++;
	    }
	}
	globNets_cnt = cnt;

	fclose (fp);
    }
    else {
	globNets_cnt = 0;
    }
}
#endif

static void createGlobNodes ()
{
    int i;
    int ntx, nx;
    PATH_SPEC path;
    NODE_REF_LIST *ref_list;
    int fnv;

    /* First, allocate global nodes that are not yet present */

    for (i = 0; i < globNets_cnt; i++) {

	resetfindnodes ();

	strcpy (path.name, globNets[i]);
	path.xarray[0][0] = 0;
	path.next = NULL;
	path.also = NULL;

	fnv = findnodes (&path, NULL, TRUE, &ref_list, FALSE);

	if (fnv == NAMENEG) {      /* node doesn't exist yet */

	    ntx = newname (globNets[i]);
	    NT[ ntx ].sort = Node;

	    nx = newnode ();
	    NT[ ntx ].x = nx;
	    N[ nx ].ntx = ntx;

	    globNetsNx[i] = nx;
	}
	else if (fnv == NODETYPE) {

	    globNetsNx[i] = ref_list -> nx;
	}
	else {
	    dberror (NULL, -1, "Name error for global net", globNets[i]);
	}
    }

    /* Second, join global nodes that should be joint */

    for (i = 0; i < N_cnt; i++) {
	ntx = N[i].ntx;
	if (ntx >= 0) {
	    if (NT[ntx].xtx < 0
	    && (nx = isGlobalNet (ST + NT[ntx].name)) >= 0) {
		if (nx != NT[ntx].x)
		    join_node (nx, NT[ntx].x);
	    }
	}
    }
}

static int isGlobalNet (char *s)
{
    int i;

    for (i = 0; i < globNets_cnt; i++) {
	if (strcmp (s, globNets[i]) == 0) {
	    return (globNetsNx[i]);
	}
    }

    return (-1);
}

#if 0
static int checkGlobNets (DM_STREAM *dsp)
{
    int i;
    int nr;
    long startgspec;
    char buf[128];

    /* check if all global net names in globNet[i]
       are present in the binary file global net list and vice verse */

    for (i = 0; i < globNets_cnt; i++) {
	globNetsCheck[i] = 0;
    }

    dmSeek (dsp, -(long) (sizeof (BINREF) + sizeof (long)), 2);
    fread ((char *)&startgspec, sizeof (long), 1, dsp -> dmfp);
    dmSeek (dsp, (long) startgspec, 0);
    nr = 0;
    fscanf (dsp -> dmfp, "%d", &nr);
    while (nr > 0) {
	fscanf (dsp -> dmfp, "%s", buf);

	for (i = 0; i < globNets_cnt; i++) {
	    if (strcmp (globNets[i], buf) == 0) {
		globNetsCheck[i] = 1;
		break;
	    }
	}

	if (i == globNets_cnt)
	    break;               /* buf not found in globNets list */

	nr--;
    }

    if (nr > 0)
	return (1);

    for (i = 0; i < globNets_cnt; i++) {
	if (globNetsCheck[i] == 0)
	    return (1);             /* globNets[i] not found in bin. file */
    }

    return (0);    /* OK */
}
#endif

void cannotAlloc (char *fn, int lineno, int nel, int sizel)
{
    int allocated;

    allocated = 0;
    allocated += sizeof (FUNCVAR) * FV_size;
    allocated += sizeof (FUNCDESCR) * FD_size;
    allocated += sizeof (char) * ST_size;
    allocated += sizeof (NAMETABLE) * NT_size;
    allocated += sizeof (MODELTABLE) * MT_size;
    allocated += sizeof (CONTEXTTABLE) * CTT_size;
    allocated += sizeof (MODELCALLTABLE) * MCT_size;
    allocated += sizeof (int) * XT_size;
    allocated += sizeof (int) * XX_size;
    allocated += sizeof (NODE) * N_size;
    allocated += sizeof (int) * DS_size;
    allocated += sizeof (CONTROL) * C_size;
    allocated += sizeof (INTERCAP) * I_size;
    allocated += sizeof (TRANSISTOR) * T_size;
    allocated += sizeof (FUNCTION) * F_size;
    allocated += sizeof (int) * FI_size;
    allocated += sizeof (int) * FR_size;
    allocated += sizeof (FUNCOUT) * FO_size;
    allocated += sizeof (char) * FS_size;

    if (debugmem) {
	deb_mem ();
	fprintf (debug, "Cannot allocate storage at %s:%d\n", fn, lineno);
	fprintf (debug, "allocated       : %d\n", allocated);
	fprintf (debug, "current request : %d * %d = %d\n\n", nel, sizel, nel * sizel);
    }
    fprintf (stderr, "%s: Cannot allocate storage (%d, %d)\n", argv0, allocated, nel * sizel);
    die (1);
}

void dmError (char *s)
{
    dmPerror (s);
    die (1);
}
