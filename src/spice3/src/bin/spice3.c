/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher
**********/

/*
 * The main routine for spice3.
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
bool ft_nutmeg  = false;
bool ft_batchmode = false;

struct variable *(*if_getparam)();

jmp_buf jbuf;
static char *usage = "Usage: spice3 [-b] [-i] [-n] [-o outfile] [-r rawfile] [file ...]\n";

static bool started = false;
IFsimulator *ft_sim = NULL;

extern IFsimulator SIMinfo;
static void SIMinit (IFfrontEnd *frontEnd, IFsimulator **simulator);

extern struct comm spcp_coms[];
struct comm *cp_coms = spcp_coms;
extern int OUTpBeginPlot(), OUTpData(), OUTwBeginPlot(), OUTwReference();
extern int OUTwData(), OUTwEnd(), OUTendPlot(), OUTbeginDomain();
extern int OUTendDomain(), OUTstopnow(), OUTerror(), OUTattributes();

static IFfrontEnd nutmeginfo = {
    IFnewUid,
    IFdelUid,
    OUTstopnow,
    seconds,
    OUTerror,
    OUTpBeginPlot,
    OUTpData,
    OUTwBeginPlot,
    OUTwReference,
    OUTwData,
    OUTwEnd,
    OUTendPlot,
    OUTbeginDomain,
    OUTendDomain,
    OUTattributes
};

char *hlp_filelist[] = { "spice", NULL };

int main (int ac, char **av)
{
    char	**tv;
    int		tc;
    int		err;
    bool	gotone = false;
    int		error2;
    char	buf[BSIZE_SP];
    bool	readinit = true, rflag = false;
    bool	istty = true, iflag = false, qflag = false;
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

	    case 'b':   /* Batch mode */
	    case 'B':
                ft_batchmode = true;
                break;

	    case 'i':   /* Interactive mode */
	    case 'I':
                iflag = true;
                break;

	    case 'q':   /* No command completion */
	    case 'Q':
                qflag = true;
                break;

	    case 'n':   /* Don't read .spiceinit */
	    case 'N':
                readinit = false;
                break;

	    case 'r':   /* The rawfile */
	    case 'R':
                if (tc > 1) {
                    tc--;
                    tv++;
                    cp_vset("rawfile", VT_STRING, *tv);
                    **tv = '-';
                }
                rflag = true;
                break;

	    case 'o':   /* Output file */
	    case 'O':
                if (tc > 1) {
                    tc--;
                    tv++;
                    if (!(freopen(*tv, "w", stdout))) {
                        perror(*tv);
                        exit(EXIT_BAD);
                    }
                    **tv = '-';
                } else {
                    fprintf(cp_err, usage);
                    exit(EXIT_BAD);
                }
                break;

	    default:
                fprintf(cp_err, "Error: bad option %s\n", *tv);
                fprintf(cp_err, usage);
                exit(EXIT_BAD);
            }
    }

    if_getparam = spif_getparam;

    if (!iflag && !istty) ft_batchmode = true;
    if ((iflag && !istty) || qflag) cp_nocc = true;
    if (!istty || ft_batchmode) out_moremode = false;

    /* Would like to do this later, but cpinit evals commands */
    init_rlimits( );

    /* Have to initialize cp now */
    ft_cpinit();

    /* To catch interrupts during .spiceinit */
    if (setjmp(jbuf) == 1) {
        fprintf(cp_err, "Warning: error executing .spiceinit\n");
        if (!ft_batchmode) goto bot;
    }

    /* Set up (void) signal handling */
    if (!ft_batchmode) {
        (void) signal(SIGINT, ft_sigintr);
        (void) signal(SIGFPE, sigfloat);
#ifdef SIGTSTP
        (void) signal(SIGTSTP, sigstop);
#endif
    }
    /* Set up (void) signal handling for fatal errors */
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

    if (!ft_batchmode) {
        DevInit();
	com_version(NULL);
	if (News_File && *News_File) {
	    if ((fp = fopen(News_File, "r"))) {
		while (fgets(buf, BSIZE_SP, fp)) fputs(buf, stdout);
		fclose(fp);
	    }
	}
    }

bot:

    /* Pass 2 -- get the filenames. If we are spice, then this means
     * build a circuit for this file. If this is in server mode, don't
     * process any of these args.
     */

    if (setjmp(jbuf) == 1) goto evl;

    cp_interactive = false;
    err = 0;

    {
        FILE *file = NULL, *tp = NULL;
        char *tempfile = NULL, buf[BSIZE_SP], *smktemp();
	int i;

        for (tv = av + 1, i = 0; *tv; tv++)
            if (**tv != '-') i++;
        if (i == 1) {
            for (tv = av + 1, i = 0; *tv; tv++)
                if (**tv != '-') break;
            if (!(file = fopen(*tv, "r"))) {
                perror(*tv);
                i = 0;
		if (ft_batchmode) exit(EXIT_BAD);
            }
        } else if (i) {
            tempfile = smktemp("sp");
            if (!(file = fopen(tempfile, "w+"))) {
                perror(tempfile);
                exit(EXIT_BAD);
            }
            for (tv = av + 1, i = 0; *tv; tv++)
                if (**tv != '-') {
                    if (!(tp = fopen(*tv, "r"))) {
                        perror(*tv);
			err = 1;
                        break;
                    }
                    while ((i = fread(buf, 1, BSIZE_SP, tp)) > 0)
                        (void) fwrite(buf, i, 1, file);
                    fclose(tp);
                }
            rewind(file);
        }
        if (file && (!err || !ft_batchmode)) {
            inp_spsource(file, false, tempfile ? (char *) NULL : *tv);
            gotone = true;
        }
	if (tempfile) unlink(tempfile);
	if (ft_batchmode && err) exit(EXIT_BAD);
    }

    if (!gotone && ft_batchmode)
        inp_spsource(stdin, false, (char *) NULL);

evl:
    if (ft_batchmode) {
        /* If we get back here in batch mode then something is
         * wrong, so exit.
         */
        bool st = false;

        (void) setjmp(jbuf);

        if (st == true) exit(EXIT_BAD);
        st = true;
        /* If -r is specified, then we don't bother with the dot
         * cards. Otherwise, we use wrd_run, but we are careful
         * not to save too much.
         */
        cp_interactive = false;
        if (rflag) {
            ft_dotsaves();
            error2 = ft_dorun(ft_rawfile);
            if (ft_cktcoms(true) || error2) exit(EXIT_BAD);
        } else {
            if (ft_savedotargs()) {
		error2 = ft_dorun(NULL);
		if (ft_cktcoms(false) || error2) exit(EXIT_BAD);
	    } else {
		fprintf(stderr, "Note: No \".plot\", \".print\" or \".four\" lines; no simulations run\n");
		exit(EXIT_BAD);
	    }
        }
    } else {
        (void) setjmp(jbuf);
        cp_interactive = true;
	while (cp_evloop((char *) NULL) == 1) ;
    }

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
    CONSTvt0 = CONSTboltz * (27 /* degree Celsius */ + CONSTCtoK) / CHARGE;
    CONSTKoverQ = CONSTboltz / CHARGE;
    CONSTe = exp((double)1.0);
}

#ifdef STATIC
/* libX11.a fixes for static linking */
void *dlopen (const char *filename, int flag)  { return (NULL); }
void *dlsym (void *handle, const char *symbol) { return (NULL); }
#endif
