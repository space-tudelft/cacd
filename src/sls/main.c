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

#include "src/sls/extern.h"
#include <signal.h>
#include <unistd.h>

extern void binfil (void);
extern void deb_all (void);
extern void deb_mem (void);
extern void endintrup (void);
extern void getcommands (void);
extern void getproc (void);
extern void initfunctionals (int round);
extern void initintrup (void);
extern void initmode (void);
extern void initsimul (void);
extern void plot_scale (void);
extern void simulate (void);
extern float deplpar (TRANSISTOR *trans, int mode, int par);
extern float enhpar (TRANSISTOR *trans, int mode, int par);
extern float respar (TRANSISTOR *trans, int mode, int par);
extern float torpar (TRANSISTOR *trans, int mode, int par);

static double fl_to_pt (double);
static void initnodes (void);
static void inittors (void);
static void int_hdl (int sig);

#ifdef FUNCSLS
char * argv0 = "funcsls";
#else
char * argv0 = "sls";
#endif

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
int SIZE_PTR_INT;

FILE * debug;

char * netwname = NULL;
char * fn_cmd;
char * fn_proc;
char * fn_res;
char * fn_tst;
char * fn_plt;
char * fn_out;
char * fn_init;

int debugdata;
int debugsim;
int debugmem;
int force_exp = FALSE;
int monitoring;
int verbose;
int no_expand;
int make_output;

int delaysim;
int outonchange;
int proclogic;
int printraces;
int printdevices;
int printstatis;
int maxpagewidth;
int printremain;
int dissip;
int random_initialization;
int random_td_initialization;
int tester_output;

double mindevtime;
double maxdevtime;
simtime_t simperiod;
simtime_t disperiod;
simtime_t sig_toffset;

double sigtimeunit;  /* signal time unit */
double outtimeunit;  /* output time unit */
double outtimeaccur; /* output time accuracy */
double sigtoint;     /* sig time to internal time conversion factor */

int vH;
double vHtmp;
double vswitch;
double vminH;
double vmaxL;
double krise;
double kfall;

int maxevents;
int maxnvicin;
int maxtvicin;
int logic_depth;
int res_cnt;
int nodes_cnt;
int essnodes_cnt;
int inpnodes_cnt;
int funcoutpnodes_cnt;
int pnodes_cnt;
int plotnodes_cnt;
int events_cnt;
int simstep_cnt;
int timepoint_cnt;
int act_maxnvicin;
int act_maxtvicin;
int sigendless;
NODE_REF_LIST ** prinvert;

simtime_t * tdumps;
simtime_t tcurr;
simtime_t tbreak;
simtime_t tsimduration;

int stopsim;
int stepdelay;

int fatalerror;

VICINITY vicin;
AGGLOMERATION agglom;
INTERRUPTMASK intrupm;

NODE_REF_LIST * pl_begin = NULL;
NODE_REF_LIST * pl_end;
NODE_REF_LIST * plotl_begin = NULL;
NODE_REF_LIST * plotl_end;
NODE_REF_LIST * disl_begin = NULL;
NODE_REF_LIST * disl_end;
ABSTRACT_OUTPUT * abstractl_begin = NULL;
ABSTRACT_OUTPUT * abstractl_end;
RES_PATH * rp_begin = NULL;
RES_PATH * rp_end;
HISTORY_LIST * ol_begin;
HISTORY_LIST * ol_end;

int * fsnulls;
FUNCTION * currf;

int dbinitialized = FALSE;
DM_PROJECT * dmproject = NULL;

int main (int argc, char *argv[])
{
#ifdef FUNCSLS
    char *use_msg = "\nUsage: %s [-mov] cell commandfile\n\n";
#else
    char *use_msg = "\nUsage: %s [-fmnov] cell commandfile\n\n";
#endif
    char *s;
    int netwname_len;

#ifdef DEBUG
    /*  Note: "sls" executes another instance of "sls" when it needs to
        simulate compiled functions. In the "called" case, argv0 will be set to
        "funcsls". See also the "Func_mkdb user's manual", or see the tests in
        test/tests/sls/runtest.tcl.

        Here we print just what version of sls is invoked right now, and with
        what arguments.
    */
    if (DEBUG) {
	int i;
	fprintf (stderr, "now running: argv0=%s\n", argv0);
	for (i = 0; i < argc; i++)
	fprintf (stderr, "now running: argv[%d]=%s\n", i, argv[i]);
    }
#endif /* DEBUG */

    fatalerror = FALSE;

    debugdata = FALSE;
    debugsim = FALSE;
    debugmem = FALSE;
    monitoring = FALSE;
    verbose = FALSE;
    no_expand = FALSE;
    make_output = FALSE;

    fn_cmd = NULL;

    while (--argc > 0) {
        if ((*++argv)[0] == '-') {
	    for (s = *argv + 1; *s != '\0'; s++) {
	        switch (*s) {
#ifndef FUNCSLS
		    case 'f':
		        force_exp = TRUE;
		        break;
#endif
		    case 'm':
		        monitoring = TRUE;
		        break;
#ifndef FUNCSLS
		    case 'n':
			no_expand = TRUE;
			break;
#endif
		    case 'o':
			make_output = TRUE;
			break;
		    case 'v':
			verbose = TRUE;
			break;
		    case '%' :
			switch (*++s) {
		            case 'd':
			        debugdata = TRUE;
			        break;
		            case 'b':
			        debugsim = TRUE;
			        break;
		            case 'm':
			        debugmem = TRUE;
			        break;
			    case '\0':
				break;
			    default :
		                fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
		                die (1);
			}
			break;
		    default:
		        fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
                        fprintf (stderr, use_msg, argv0);
		        die (1);
	        }
	    }
	}
	else {
            if (netwname == NULL) netwname = *argv;
	    else {
                if (fn_cmd == NULL) fn_cmd = *argv;
                else {
		    fprintf (stderr, "%s: too many arguments\n", argv0);
                    fprintf (stderr, use_msg, argv0);
		    die (1);
                }
	    }
	}
    }

    if (fn_cmd == NULL || netwname == NULL) {
	if (netwname == NULL) fprintf (stderr, "%s: missing cell name\n", argv0);
	if (fn_cmd == NULL) fprintf (stderr, "%s: missing commandfile\n", argv0);
	fprintf (stderr, use_msg, argv0);
	die (1);
    }

    netwname_len = strlen (netwname);
    if (netwname_len > DM_MAXNAME) {
	fprintf (stderr, "%s: too long cell name\n", argv0);
	die (1);
    }

#ifndef FUNCSLS
    if (no_expand == FALSE || force_exp == TRUE) {
	struct stat buf;
	char com[DM_MAXNAME + 20];
	char *fsls = "funcsls";

	sprintf (com, "sls_exp");
	if (force_exp) sprintf (com + strlen(com), " -f");
	if (! verbose) sprintf (com + strlen(com), " -s");
	sprintf (com + strlen(com), " %s", netwname);

	if (verbose) fprintf (stderr, "+ %s\n", com);
	if (system (com) != 0) die (1);

	if (stat (fsls, &buf) == 0) {
	    if (verbose) {
		sprintf (com, "%s", make_output ? "-ov": "-v");
		fprintf (stderr, "+ %s %s %s %s\n", fsls, com, netwname, fn_cmd);
		execl (fsls, fsls, com, netwname, fn_cmd, (char *)0);
	    }
	    else if (make_output)
		execl (fsls, fsls, "-o", netwname, fn_cmd, (char *)0);
	    else
		execl (fsls, fsls, netwname, fn_cmd, (char *)0);
	}
    }
#endif

    SIZE_PTR_INT = MAX (sizeof (char *), sizeof (int));

    if (verbose) {
	fprintf (stderr, "Running %s %s\n", argv0, VERSION);
	fprintf (stderr, "%sting %s with %s\n", make_output? "Output": "Simula", netwname, fn_cmd);
    }

    if (debugdata || debugsim || debugmem) {
	OPENW (debug, "deb");
	setbuf (debug, (char *) NULL);
    }

    if (monitoring) {
	startmonitime (argv0);
	monitime ("B main");
    }

    dbinitialized = TRUE;

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

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

    binfil ();

    PALLOC (fn_res, netwname_len + 6, char);
    PALLOC (fn_tst, netwname_len + 6, char);
    PALLOC (fn_plt, netwname_len + 6, char);
    PALLOC (fn_out, netwname_len + 6, char);
    sprintf (fn_res, "%s.res", netwname);
    sprintf (fn_tst, "%s.tst", netwname);
    sprintf (fn_plt, "%s.plt", netwname);
    sprintf (fn_out, "%s.out", netwname);

    if (!make_output) initfunctionals (1); /* execute 'load' part of functions */

    if (debugmem) deb_mem ();

    if (debugdata) deb_all ();

    fn_proc = NULL;
    fn_init = NULL;
    delaysim = FALSE;
    outonchange = FALSE;
    proclogic = FALSE;
    printraces = TRUE;
    printdevices = FALSE;
    printstatis = FALSE;
    maxpagewidth = 132;
    printremain = FALSE;
    dissip = FALSE;
    random_initialization = FALSE;
    random_td_initialization = FALSE;
    tester_output = FALSE;

    mindevtime = -1;
    maxdevtime = -1;
    simperiod = -1;
    sig_toffset = 0;
    disperiod = -1;

    sigtimeunit = -1;
    outtimeunit = -1;
    outtimeaccur = -1;
    vHtmp = -1;
    vswitch = -1;
    vminH = -1;
    vmaxL = -1;

    logic_depth = -1;

    pnodes_cnt = 0;
    plotnodes_cnt = 0;
    sigendless = FALSE;

    getcommands ();

    if (random_td_initialization) random_initialization = TRUE;

    if (sigtimeunit < 0) sigtimeunit = 1;
    if (sigtimeunit < 1e-12) slserror (NULL, 0, ERROR1, "too small sigunit", NULL);
    if (sigtimeunit > 1e+3)  slserror (NULL, 0, ERROR1, "too large sigunit", NULL);

    if (outtimeunit <= 0) {
	if (outtimeunit == 0) slserror (NULL, 0, WARNING, "too small outunit", NULL);
	outtimeunit = fl_to_pt (sigtimeunit);
    }

    if (outtimeaccur <= 0) {
	if (outtimeaccur == 0) slserror (NULL, 0, WARNING, "too small outacc", NULL);
	outtimeaccur = outtimeunit;
    }
    else if (outtimeaccur > outtimeunit)
	slserror (NULL, 0, WARNING, "outacc is more than outunit", NULL);

    if (outtimeaccur > sigtimeunit) {
	slserror (NULL, 0, WARNING, "outacc is more than sigunit", NULL);
	do { outtimeaccur /= 10; } while (outtimeaccur > sigtimeunit);
    }

    sigtoint = doub_conv (sigtimeunit / outtimeaccur);

    if (simperiod < 0) {
	if (sigendless)
	    slserror (NULL, 0, ERROR1, "endless changing input signal", "while no simperiod was specified");
	tsimduration = MAXSIMTIME + 1; /* this value is used later ! */
    }
    else {
	if (simperiod * sigtoint > MAXSIMTIME)
	    slserror (NULL, 0, ERROR1, "time resolution too large:", "reduce 'simperiod' or enlarge 'outacc'");
	tsimduration = simperiod * sigtoint;
    }

    if (mindevtime < 0) mindevtime = 1;
    else if (mindevtime > 1) {
	slserror (NULL, 0, WARNING, "tdevmin > 1 (using 1)", NULL);
	mindevtime = 1;
    }

    if (maxdevtime < 0) maxdevtime = 1;
    else if (maxdevtime < 1) {
	slserror (NULL, 0, WARNING, "tdevmax < 1 (using 1)", NULL);
	maxdevtime = 1;
    }

    if (!make_output && (proclogic || delaysim)) getproc ();

    if (vHtmp <= 0) {
	if (vHtmp == 0) slserror (NULL, 0, WARNING, "vh == 0 (using 5)", NULL);
	vHtmp = 5;
    }
    vH = VOLTMAX_INT;

    if (vswitch >= 0)
	vswitch = vH * (vswitch / vHtmp);
    else
	vswitch = vH / 2;

    if (vswitch > vH) {
	slserror (NULL, 0, ERROR1, "error: vswitch > vh", NULL);
    }

    if (vminH >= 0)
	vminH = vH * (vminH / vHtmp);
    else
        vminH = vH * VMINH_REL_DEF;

    if (vmaxL >= 0)
	vmaxL = vH * (vmaxL / vHtmp);
    else
        vmaxL = vH * VMAXL_REL_DEF;

    if (vminH > vH || vmaxL < 0 || vminH < vmaxL) {
	slserror (NULL, 0, ERROR1, "error on logic threshold voltages", NULL);
    }

    if (vminH < vswitch) {
	slserror (NULL, 0, ERROR1, "error: vminh < vswitch", NULL);
    }

    if (vmaxL > vswitch) {
	slserror (NULL, 0, ERROR1, "error: vmaxl > vswitch", NULL);
    }

    if (debugsim) {
	fprintf (debug, "vH: %12d  vswitch: %20f\n\n", vH, vswitch);
	fprintf (debug, "vminH: %20f  vmaxL: %20f\n\n", vminH, vmaxL);
	fprintf (debug, "sigtimeunit = %e\n", sigtimeunit);
	fprintf (debug, "outtimeunit = %e\n", outtimeunit);
	fprintf (debug, "outtimeaccur = %e\n", outtimeaccur);
	fprintf (debug, "simperiod = %lld\n", simperiod);
	fprintf (debug, "tsimduration = %lld\n", tsimduration);
	fprintf (debug, "\n");
    }

    if (!make_output) {
	initnodes ();
	inittors ();

	if (logic_depth < 0) {
	    logic_depth = essnodes_cnt;
	    if (logic_depth > 100) logic_depth = 100; /* max logic depth when not specified */
	}

	maxevents = essnodes_cnt + inpnodes_cnt + funcoutpnodes_cnt + plotnodes_cnt;

	initsimul ();
	initintrup ();

	plot_scale ();

	if (delaysim) initmode ();

	initfunctionals (2); /* execute 'initial' part of functions */

	simulate ();
    }

    endintrup ();

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    dbinitialized = FALSE;

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

    if (debugdata || debugsim || debugmem) CLOSE (debug);

    if (monitoring) {
	monitime ("E main");
	stopmonitime ();
    }

    return (0);
}

static void initnodes (void)
{
    int cnt, cnt2, fox, i, index;
    NODE * n;
    TRANSISTOR * t;
    float   tp;
    FUNCTION *f;
    FORCEDSIGNAL *fs, *fshelp;

    nodes_cnt = 0;
    essnodes_cnt = 0;
    inpnodes_cnt = 0;
    funcoutpnodes_cnt = 0;

    for (cnt = 0; cnt < FR_cnt; cnt++) {
	N[ FR[cnt] ].essential = TRUE;
    }
    /* nodes that are connected to an inread should
       be essential because otherwise next values
       are used in one simulation step
    */

    for (cnt = 0; cnt < N_cnt; cnt++) {
	n = &N[cnt];

        if (n -> redirect) continue;

	nodes_cnt++;

	if (n -> inp || n -> funcoutp || n -> cx != -1 || n -> outp) {
	    n -> essential = TRUE;
	}
	else {
	    n -> essential = FALSE;
	}

	if (proclogic) {
	    if (n -> dsx != -1) {
		index = n -> dsx;
		for (cnt2 = DS[index]; cnt2 > 0; cnt2--) {
		    index++;
		    t = &T[DS[index]];
		    tp = torpar (t, 0, Cestat);
		    n -> statcap += tp;
		}
	    }

	    if (n -> cx != -1) {
		index = n -> cx;
		for (cnt2 = C[index].c; cnt2 > 0; cnt2--) {
		    index++;
		    if (C[index].sort == Transistor) {
		        t = &T[C[index].c];
			n -> statcap += torpar (t, 0, Cgstat);
		    }
		}
	    }
	}

	if (!proclogic || (proclogic && n -> statcap <= 0)) {
	    n -> statcap = NODECAP_DEF;
	}

	if (n -> essential) essnodes_cnt++;

	if (n -> inp) {
	    if (n -> forcedinfo -> sigmult < 0)
		n -> forcedinfo -> sigmult = 1;
	    else
		n -> forcedinfo -> sigmult /= (outtimeaccur * sigtoint);
	    inpnodes_cnt++;
	}

	n -> thisvicin = FALSE;
	n -> flag = FALSE;
	n -> evalflag = FALSE;
	n -> ei = NULL;
    }

    for (i = 0; i < F_cnt; ++i){
	f = &F[i];
	if ((fox = f->fox) >= 0) {
	    cnt = FO[fox++].x;
	    while (cnt > 0) {
		funcoutpnodes_cnt++;
		n = &N[ FO[fox].x ];
		PALLOC (fs, 1, FORCEDSIGNAL);
		fs->fox = fox;
		fs->initfstate = X_state;
		fshelp = n->forcedinfo;
		if (fshelp) {
		    while (fshelp -> next)
			fshelp = fshelp -> next;
		    fshelp -> next = fs;
		}
		else
		    n->forcedinfo = fs;
		fs->next = NULL;
		cnt--;
		fox++;
	    }
	}
    }
}

static void inittors (void)
{
    TRANSISTOR * t;
    int cnt, warningdone = 0;

    res_cnt = 0;

    for (cnt = 0; cnt < T_cnt; cnt++) {
	t = &T[cnt];
	if (t -> type == Res) res_cnt++;
        else {
            if ((t -> length > 0.01 || t -> width > 0.01) && !warningdone) {
                slserror (NULL, 0, WARNING,
		    "transistors with dimensions which are more than 1 cm,\n",
		    "that doesn't seem to be a very clever design");
                warningdone = 1;
            }
        }
	t -> flag = FALSE;
	t -> ei = NULL;
    }
}

float torpar (TRANSISTOR *trans, int mode, int par)
{
    float val = 0; // init, to remove compiler warning

    if (!proclogic && (par == Rstat || par == Rsatu)) {
	switch (trans -> type) {
	    case Nenh:
		val = NENH_RSTAT_DEF;
		break;
	    case Penh:
		val = PENH_RSTAT_DEF;
		break;
	    case Depl:
		val = DEPL_RSTAT_DEF;
		break;
	    case Res:
		val = RES_RSTAT_DEF;
		break;
	}
    }
    else {
	switch (trans -> type) {
	    case Nenh:
	    case Penh:
		val = enhpar (trans, mode, par);
		break;
	    case Depl:
		val = deplpar (trans, mode, par);
		break;
	    case Res:
		val = respar (trans, mode, par);
		break;
	}
    }

    return (val);
}

static double fl_to_pt (double f) /* converts f to the closest power_ten */
{
    double p = 1;

    if (f >= 1) {
	while (f >= 5.) { f /= 10; p *= 10; }
    }
    else {
	while (f < 0.5) { f *= 10; p /= 10; }
    }

    return (p);
}

void arr_init (char **pa, int elsize, int ispace, double incr)
/* pa     = address of pointer to be used as array */
/* elsize = the size of an array element */
/* ispace = initial number of allocated elements */
/* incr   = increase factor when free elements are exhausted */
{
    char * p;
    int roundincr;

    if (ispace <= 0) ispace = 0;

    if ((p = malloc ((unsigned)((4 * sizeof (int)) + (ispace * elsize)))) == NULL) {
        slserror (NULL, 0, ERROR1, "Cannot allocate storage", NULL);
    }

    roundincr = 100 * incr;

    *pa = p + 4 * (int)sizeof (int);
    *(((int *)*pa) - 4) = elsize;
    *(((int *)*pa) - 3) = ispace;  /* space */
    *(((int *)*pa) - 2) = (int)0;  /* used  */
    *(((int *)*pa) - 1) = roundincr;
}

/* returns an index for a new element in the array
** pa is the address of the array pointer
*/
int arr_new (char **pa)
{
    char * p;
    int elsize;
    int space = *(((int *)*pa) - 3);
    int used  = *(((int *)*pa) - 2);
    int roundincr;

    if (used == space) {
        roundincr = *(((int *)*pa) - 1);
        if (space == space * roundincr / 100)
            space++;
        else
            space = space * roundincr / 100;
        *(((int *)*pa) - 3) = space;
        elsize = *(((int *)*pa) - 4);
        p = *pa - 4 * (int)sizeof (int);
        if (!(p = realloc (p, (unsigned)(4 * (int)sizeof (int) + elsize * space)))) {
            slserror (NULL, 0, ERROR1, "Cannot allocate storage", NULL);
        }
        *pa = p + 4 * (int)sizeof (int);
    }
    return ((*(((int *)*pa) - 2) = ++used) - 1);
}

int arr_size (char *p) /* returns number of elements in array p */
{
    return (*(((int *)p) - 2));
}

void arr_reset (char *p) /* resets number of elements used */
{
    *(((int *)p) - 2) = (int)0;
}

void cannotAlloc (char *fn, int lineno, int nel, int sizel)
{
    int allocated = 0;

    allocated += sizeof (FUNCVAR) * FV_cnt;
    allocated += sizeof (FUNCDESCR) * FD_cnt;
    allocated += sizeof (char) * ST_cnt;
    allocated += sizeof (NAMETABLE) * NT_cnt;
    allocated += sizeof (MODELTABLE) * MT_cnt;
    allocated += sizeof (CONTEXTTABLE) * CTT_cnt;
    allocated += sizeof (MODELCALLTABLE) * MCT_cnt;
    allocated += sizeof (int) * XT_cnt;
    allocated += sizeof (int) * XX_cnt;
    allocated += sizeof (NODE) * N_cnt;
    allocated += sizeof (int) * DS_cnt;
    allocated += sizeof (CONTROL) * C_cnt;
    allocated += sizeof (INTERCAP) * I_cnt;
    allocated += sizeof (TRANSISTOR) * T_cnt;
    allocated += sizeof (FUNCTION) * F_cnt;
    allocated += sizeof (int) * FI_cnt;
    allocated += sizeof (int) * FR_cnt;
    allocated += sizeof (FUNCOUT) * FO_cnt;
    allocated += sizeof (char) * FS_cnt;

    if (debugmem) {
        deb_mem ();
        fprintf (debug, "Cannot allocate storage at %s:%d\n", fn, lineno);
        fprintf (debug, "should be allocated : %d\n", allocated);
        fprintf (debug, "current request     : %d * %d = %d\n\n", nel, sizel, nel * sizel);
    }
    fprintf (stderr, "%s: Cannot allocate storage (%d, %d)\n", argv0, allocated, nel * sizel);
    die (1);
}

static void int_hdl (int sig) /* interrupt handler */
{
    switch (sig) {
        case SIGILL:
            fprintf (stderr, "Illegal instruction\n");
            break;
        case SIGFPE:
            fprintf (stderr, "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS:
            fprintf (stderr, "Bus error\n");
            break;
#endif
        case SIGSEGV:
            fprintf (stderr, "Segmentation violation\n");
            break;
    }
    die (sig);
}

void dmError (char *s)
{
    dmPerror (s);
    die (1);
}

void die (int nr)
{
    if (dbinitialized) {
	if (dmproject) dmCloseProject (dmproject, QUIT);
	dmQuit ();
    }
    exit (nr);
}
