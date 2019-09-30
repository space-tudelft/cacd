/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher
**********/

/*
 * The main routine for nutmeg.
 */

#include "spice.h"
#include "misc.h"
#include "ifsim.h"
#include "inpdefs.h"
#include "iferrmsg.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedev.h"
#include "ftedebug.h"
#include "const.h"
#include <setjmp.h>

#include <sys/types.h>
#include <unistd.h>

#include <signal.h>

char *ft_rawfile = "rawspice.raw";

char *errRtn;
char *errMsg;
char *cp_program;

bool ft_intrpt = false;     /* Set by the signal handlers. */
bool ft_setflag = false;    /* Don't abort after an interrupt. */
bool ft_nutmeg = true;

struct variable *(*if_getparam)();

jmp_buf jbuf;
static char *usage = "Usage: nutmeg [-] [-n] [datafile ...]\n";

static bool started = false;
IFsimulator *ft_sim = 0;

extern IFsimulator SIMinfo;
static void SIMinit (IFfrontEnd *frontEnd, IFsimulator **simulator);

extern struct comm nutcp_coms[];
struct comm *cp_coms = nutcp_coms;
static IFfrontEnd nutmeginfo;

char *hlp_filelist[] = { "spice", NULL };

int main (int ac, char **av)
{
    char	**tv;
    int		tc;
    char	buf[BSIZE_SP];
    bool	readinit = true;
    bool	istty = true, qflag = false;
    bool	gdata = true, gotone;
    FILE	*fp;

    if (started) {
        fprintf(cp_err, "main: Internal Error: jump to zero\n");
        fatal();
    }
    started = true;

    ivars();

    cp_in  = stdin;
    cp_out = stdout;
    cp_err = stderr;

    istty = (bool) isatty(fileno(stdin));

    init_time();

    SIMinit (&nutmeginfo, &ft_sim);
    cp_program = ft_sim->simulator;

    srandom(getpid());

    tv = av;
    tc = ac;

    /* Pass 1 -- get options. */
    while (--tc > 0) {
        tv++;
        if (**tv == '-')    /* Option argument */
            switch ((*tv)[1]) {

	    case '\0':  /* No raw file */
                gdata = false;
                break;

	    case 'q':   /* No command completion */
	    case 'Q':
                qflag = true;
                break;

	    case 'n':   /* Don't read .spiceinit */
	    case 'N':
                readinit = false;
                break;

	    default:
                fprintf(cp_err, "Error: bad option %s\n", *tv);
                fprintf(cp_err, usage);
                exit(EXIT_BAD);
            }
    }

    if_getparam = nutif_getparam;

    if (!istty || qflag) cp_nocc = true;
    if (!istty) out_moremode = false;

    /* Would like to do this later, but cpinit evals commands */
    init_rlimits( );

    /* Have to initialize cp now */
    ft_cpinit();

    /* To catch interrupts during .spiceinit */
    if (setjmp(jbuf) == 1) {
        fprintf(cp_err, "Warning: error executing .spiceinit\n");
        goto bot;
    }

    /* Set up signal handling */
    (void) signal(SIGINT, ft_sigintr);
    (void) signal(SIGFPE, sigfloat);
#ifdef SIGTSTP
    (void) signal(SIGTSTP, sigstop);
#endif
    /* Set up signal handling for fatal errors */
    (void) signal(SIGILL, sigill);

#ifdef SIGBUS
    (void) signal(SIGBUS, sigbus);
#endif
#ifdef SIGSEGV
    (void) signal(SIGSEGV, sigsegv);
#endif
#ifdef SIGSYS
    (void) signal(SIGSYS, sig_sys);
#endif

    if (readinit) {
	char *home;
        /* Try to source either .spiceinit or ~/.spiceinit */
        if (access(".spiceinit", 0) == 0)
            inp_source(".spiceinit");
        else if ((home = getenv("HOME"))) {
            strcpy(buf, home);
            strcat(buf, "/.spiceinit");
            if (access(buf, 0) == 0) inp_source(buf);
        } else {
	    /* Try to source the file "spice.rc" in the current directory */
	    if ((fp = fopen("spice.rc", "r"))) {
		fclose(fp);
		inp_source("spice.rc");
	    }
	}
    }

    DevInit();
    com_version(NULL);
    if (News_File && *News_File) {
	if ((fp = fopen(News_File, "r"))) {
	    while (fgets(buf, BSIZE_SP, fp)) fputs(buf, stdout);
	    fclose(fp);
	}
    }

bot:

    /* Pass 2 -- get the filenames.
     */

    if (setjmp(jbuf) == 1) goto evl;

    cp_interactive = false;

    gotone = false;
    /* Read in the rawfiles */
    for (av++; *av; av++)
	if (**av != '-') {
	    ft_loadfile(*av);
	    gotone = true;
	}
    if (!gotone && gdata) ft_loadfile(ft_rawfile);

evl:
    /* Nutmeg "main" */
    (void) setjmp(jbuf);
    cp_interactive = true;
    while (cp_evloop((char *) NULL) == 1) ;

    exit(EXIT_NORMAL);
    return(0);
}

/* allocate space for global constants in 'CONST.h' */

double CONSTroot2;
double CONSTvt0;
double CONSTKoverQ;
double CONSTe;
IFfrontEnd *SPfrontEnd = NULL;

static void SIMinit (IFfrontEnd *frontEnd, IFsimulator **simulator)
{
    SPfrontEnd = frontEnd;
    *simulator = &SIMinfo;
    CONSTroot2 = sqrt(2.);
    CONSTvt0 = CONSTboltz * (27 /* degree Celsius */ + CONSTCtoK ) / CHARGE;
    CONSTKoverQ = CONSTboltz / CHARGE;
    CONSTe = exp((double)1.0);
}

int  if_run() { return (0); }
int  if_sens_run() { return (0); }
int  if_option() { return (0); }
void if_cktfree() {}
void if_dump() {}
char *if_inpdeck() { return (NULL); }
void if_setparam() {}
bool if_tranparams() { return (false); }
struct variable *if_getstat() { return (NULL); }

#ifdef STATIC
/* libX11.a fixes for static linking */
void *dlopen (const char *filename, int flag)  { return (NULL); }
void *dlsym (void *handle, const char *symbol) { return (NULL); }
#endif
