/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	R. Paulussen
 *	S. de Graaf
 *	A.J. van Genderen
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
 * Program CIG (Convert ICD-data to GDS II-data)
 *
 * This program reads data form the ICD database and put
 * it into a GDS II-formatted file.
 *
 * CIG input files are: info, term, box, mc, nor
 *
 * The output file(s): <maincell>.gds[.<vol_nr>] (binary)
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"
#include "src/gds2/defs.h"

#define TOOLVERSION "4.54 12-Apr-2011"

#define PUTBYTE(c) putc ((char)(c), fpdata)

#define ALLOC(ptr,type) { \
    if(!(ptr=(struct type *)malloc(sizeof(struct type)))) \
	pr_exit (A, 14, 0); }

#define ALLOCN(ptr,type,nr) {\
    if(!(ptr=(type *)malloc(sizeof(type)*(nr))))\
	pr_exit (A, 14, 0); }

struct c_elmt {
    char          *c_name;
    char          *r_name;
    DM_CELL       *c_key;
    DM_PROJECT    *p_key;
    struct c_elmt *c_hash;
    struct c_elmt *c_next;
};

#define HASHSIZE 100
struct c_elmt *hashlist[HASHSIZE];
struct c_elmt *gen_list;
struct c_elmt *gll;

char    file_name[BUFLEN],	/* file name    */
       *libname,		/* libname      */
       *maincell,		/* name of the top cell */
       *cellname,		/* name of the current cell */
       *ref_name,		/* ref name of current ref. */
      **maskname,
        buf[BUFLEN],
        buf1[128],
        buf2[128],
        buf3[128],
        buf4[128];

int     byte_cnt = 0;
int     take_cnt = 0;
int     mf = 1000;		/* multiplication factor */
int     min_long;		/* minimum value (before mf) */
int     max_long;		/* maximum value (before mf) */
int     maskcode;		/* layer number */
int     maskdtype;		/* layer datatype */
int     maskttype;		/* layer texttype */

int    *masknr;		/* gds number for mask polygon */
int    *termnr;		/* gds number for mask terminal */
int    *labelnr;	/* gds number for mask label */
int    *tmasknr;	/* terminal mask numbers */
int    *maskdatatype;	/* gds data type for mask polygon */
int    *termdatatype;	/* gds data type for mask terminal */
int    *labeldatatype;	/* gds data type for mask label */
int    *tmaskdatatype;	/* terminal mask numbers */

int     uppercase = 0;		/* strings to uppercase */
char   *bmlist = 0;
int     verbose = 0;
int     aflag = 0;		/* if true, no arefs */
int     iflag = 0;		/* if true, process imp. cells */
int     lflag = 0;		/* if true, set user units to lambda */
int     rflag = 0;		/* if true, use remote names */
int     tflag = 0;              /* if true, store terminals in TEXT records */
int     Tflag = 0;              /* if true, then only the top cell must be done */

char   *argv0 = "cig";		/* Program Name */
char   *use_msg =		/* Command Line */
"\nUsage: %s maincell [-ailrTuv] [-F uu] [-f mf] [-L libname] [-m mlist]\n\n";

int     angle,
	check_flag = 0,		/* value checking needed   */
	magnify,		/* magnification factor    */
        refl,			/* rotation and reflection */
        file_len,		/* length of GDS file      */
        label_len,		/* length of GDS label     */
        vol_nr = 0,		/* volume number           */
        tape_len = 2400,
        density = 1600,		/* tape parameters         */
        year, mon, day, hour, min, sec,	/* time specification  */
        TAPE_LEN_VL = 0,	/* tapelength specified flag   */
        TAPE_DEN_VL = 0,	/* tape density specified flag */
        TAPE_MUL_VL = 0;	/* split file in volumes if nessasary */

struct tm  *timeptr;

long    cell_nrmc,		/* number of cell-calls in a cell */
        t_code[3];		/* unique tape code */
time_t  timeval;

double  poly_coor[MAX_COOR * 2 + 2];
double  gds_unit;		/* GDS unit for GDS file */
double  uu = 0;			/* user unit for GDS file */

DM_PROJECT *default_project;	/* project of top cell */
DM_PROJECT *project;		/* current project */
DM_PROCDATA *process;
DM_CELL *mod_key;

FILE *fpdata;

#define PROP_TERMSIDE   59
#define PROP_TERMLAY    60
#define PROP_INSTANCE   61
#define PROP_TERMINAL   62
#define PROP_LABELSIDE  -1
#define PROP_LABELLAY   -1
#define PROP_LABEL      -1

/****
Properties PROP_TERMSIDE PROP_TERMLAY PROP_LABELSIDE PROP_LABELLAY
   are followed by an integer
Properties PROP_INSTANCE PROP_TERMINAL PROP_LABEL are followed by a string

The terminal/label direction can be:
****/

#define BOTTOM  0
#define RIGHT   1
#define TOP     2
#define LEFT    3
#define ANY_DIRECTION   99

int prop_termside;
int prop_termlay;
int prop_instance;
int prop_term;
int prop_labelside;
int prop_labellay;
int prop_label;

void usage (void);
void add_cell (char *name);
int StrCmp (char *S1, char *S2);
void get_term (void);
void get_label (void);
void OLDterm_processing (char *name);
void term_processing (char *name);
void label_processing (char *name);
void OLDlabel_processing (char *name);
void get_box (void);
void get_nor (void);
void get_mc (void);
void box_processing (void);
void poly_processing (int nr);
void circle_processing (void);
long l_round (double real);
int fxy (double x0, double y0, double x1, double y1, double x, double y);
int fx0_min_xa (double x0, double xa);
double fy (double xa, double ya, double xb, double yb, double x0);
double fx (double x0, double y0, double x1, double y1, double xa, double ya, double xb, double yb);
void mc_processing (void);
void write_box (long xl, long xr, long yb, long yt);
void write_header (void);
void write_nodata (int type);
void write_short (int type, int value);
void write_string (char *str, int type);
void write_rec_header (int len, int type);
void write_bgn (int type);
void write_bound (double *poly_co, int nr);
void write_path (double *poly_co, int nr, long width);
void write_sref (long tx, long ty);
void write_aref (void);
void write_label (void);
void next_volume (void);
void open_write (void);
void close_write (void);
void print_double (double ln);
void print_short (int ln);
void print_long (long ln);
void check_elmt_bbox (char *cell);
void cell_error (char *s, char *cell, long value);
void ref_error (char *s, long value);
void exceeds_msg (void);
void sig_handler (int sig);
void pr_exit (int mode, int err_no, char *cs);
void die (void);

int main (int argc, char *argv[])
{
    DM_STREAM *fp;
    FILE   *fp_bml;
    int     gds_laynr;
    int     gds_datatype;
    int     nolays;		/* max. number of mask codes */
    register struct c_elmt *clp;
    register int    i, j, l;
    char *s;
    int istext;
    struct stat statBuf;

    if (argc <= 1) usage ();

    /*
    ** check/read options:
    */
    for (j = 1; j < argc; ++j) {

	if (argv[j][0] != '-') {
	    if (maincell) {
		PE "%s: error: too many maincells specified\n", argv0);
		usage ();
	    }
	    maincell = argv[j];
	    continue;
	}

        s = argv[j] + 1;
        while (s && *s) {
	    switch (*s++) {
		case 'T': 		/* do only the top cell */
		    Tflag = 1;
		    break;
		case 'a': 		/* don't generate arefs */
		    aflag = 1;
		    break;
		case 'l': 		/* set user unit to lambda */
		    lflag = 1;
		    break;
		case 'L': 		/* use this libname string */
                    if (*s) libname = s;
                    else {
			if (++j >= argc) usage ();
			libname = argv[j];
                    }
                    s = NULL;
		    break;
		case 'F': 		/* Fixed user unit specified */
                    if (*s) {
		        uu = atof (s);
                    }
                    else {
			if (++j >= argc) usage ();
			uu = atof (argv[j]);
                    }
                    s = NULL;
		    break;
		case 'f': 		/* multipl. factor specified */
                    if (*s) {
		        mf = atoi (s);
                    }
                    else {
			if (++j >= argc) usage ();
			mf = atoi (argv[j]);
                    }
                    s = NULL;
		    break;
		case 'i': 		/* processing of imported cells */
		    iflag = 1;
		    break;
		case 'r': 		/* use remote names */
		    rflag = 1;
		    break;
		case 's': 		/* split in tape volumes */
		    if (TAPE_MUL_VL) usage ();
		    TAPE_MUL_VL = 1;
		    break;
		case 't': 		/* tape length specified */
		    if (TAPE_LEN_VL) usage ();
		    if (!TAPE_MUL_VL) break;
		    TAPE_LEN_VL = 1;
                    if (*s) {
		        tape_len = atoi (s);
                    }
                    else {
			if (++j >= argc) usage ();
			tape_len = atoi (argv[j]);
                    }
                    s = NULL;
		    break;
		case 'd': 		/* tape density specified */
		    if (TAPE_DEN_VL) usage ();
		    if (!TAPE_MUL_VL) break;
		    TAPE_DEN_VL = 1;
                    if (*s) {
		        density = atoi (s);
                    }
                    else {
			if (++j >= argc) usage ();
			density = atoi (argv[j]);
                    }
                    s = NULL;
		    break;
		case 'm':
                    if (*s) {
		        bmlist = s;
                    }
                    else {
			if (++j >= argc) usage ();
			bmlist = argv[j];
                    }
                    s = NULL;
		    break;
		case 'u':
		    ++uppercase;
		    break;
		case 'v':
		    ++verbose;
		    break;
		case 'x':
		    ++tflag;
                    PE "%s: -x: obsolete option\n", argv0);
		    break;
		default:
		    PE "%s: -%c: unknown option\n", argv0, *(s - 1));
		    usage ();
            }
	}
    }

    if (!maincell || !*maincell) {
	PE "%s: error: no maincell specified\n", argv0);
	usage ();
    }

    if (strlen (maincell) > DM_MAXNAME) {
	PE "%s: error: too long maincell name (max. %d characters)\n", argv0, DM_MAXNAME);
	exit (1);
    }

    if (!libname) libname = maincell;

    if (verbose) PE "%s: TOOLVERSION %s\n", argv0, TOOLVERSION);
    if (bmlist) PE "%s: using '%s' as basic masklist\n", argv0, bmlist);
    if (aflag) PE "%s: no arefs (expanded to srefs)\n", argv0);
    if (iflag) PE "%s: processing of imported cells\n", argv0);
    if (rflag) PE "%s: try to use remote cell names\n", argv0);
    if (uppercase) PE "%s: uppercase mode\n", argv0);
    if (tflag) PE "%s: converting terminals to text records\n", argv0);
    if (Tflag) PE "%s: processing only the top cell\n", argv0);

    if (TAPE_MUL_VL) {
	PE "%s: splitting mode\n", argv0);

	if (density != 800 && density != 1600) {
	    density = 1600;
	    PE "%s: \7ILLEGAL density specified\n", argv0);
	}

	PE "%s: density set to %d bpi\n", argv0, density);

	if (tape_len != 600 && tape_len != 1200 && tape_len != 2400) {
	    tape_len = 2400;
	    PE "%s: \7ILLEGAL tape length specified\n", argv0);
	}

	PE "%s: tape length set to %d ft\n", argv0, tape_len);

	switch (tape_len) {
	    case 600:
		if (density == 800)
		    file_len = NR_BLOCKS_L * BLOCKSIZE;
		else
		    file_len = NR_BLOCKS_H * BLOCKSIZE;

	    case 1200:
		if (density == 800)
		    file_len = NR_BLOCKS_L * BLOCKSIZE;
		else
		    file_len = NR_BLOCKS_H * BLOCKSIZE;
		file_len *= 2;

	    case 2400:
		if (density == 800)
		    file_len = NR_BLOCKS_L * BLOCKSIZE;
		else
		    file_len = NR_BLOCKS_H * BLOCKSIZE;
		file_len *= 4;
	}
	label_len = 3 * BASE_LEN + SHORT + 3 * LONG + strlen (maincell);
	if (label_len % 2) ++label_len;
	file_len -= label_len;
    }
    else {
	if (verbose) PE "%s: no splitting mode\n", argv0);
	density = 0;
	tape_len = 0;
    }

    if ((i = sizeof (int)) != LONG) pr_exit (A, 13, 0);
    if (mf < 1) {
	mf = 1;
	PE "%s: illegal multipl. factor (set to 1)\n", argv0);
    }
    min_long = MIN32 / mf;
    max_long = MAX32 / mf;

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);	/* ignore hangup signal */
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);
    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);
    gds_unit = project -> lambda / (double) mf;
    if (lflag && uu <= 0) uu = 1.0 / (double) mf;

    if (verbose) {
	PE "%s: multipl. factor = %d\n", argv0, mf);
	if (uu > 0)
	    PE "%s: 1 DBU = %.3E UU\n", argv0, uu);
	else
	    PE "%s: 1 DBU = %.3E UU\n", argv0, gds_unit);
	PE "%s: 1 DBU = %.3E micron (%g DBU/micron)\n", argv0, gds_unit, 1 / gds_unit);
	if (uu > 0)
	    PE "%s: 1  UU = %.3E micron\n", argv0, gds_unit / uu);
	else
	    PE "%s: 1  UU = %.3E micron\n", argv0, 1.0);
    }

    default_project = project;
    maskname = process -> mask_name;
    nolays = process -> nomasks;

    if (!bmlist)
	bmlist = (char *) dmGetMetaDesignData (PROCPATH, project, "bmlist.gds");

    if (!(fp_bml = fopen (bmlist, "r"))) pr_exit (A, 1, bmlist);

    prop_termside = PROP_TERMSIDE;
    prop_termlay = PROP_TERMLAY;
    prop_instance = PROP_INSTANCE;
    prop_term = PROP_TERMINAL;
    prop_labelside = PROP_LABELSIDE;
    prop_labellay = PROP_LABELLAY;
    prop_label = PROP_LABEL;

    ALLOCN (masknr, int, nolays);
    ALLOCN (labelnr, int, nolays);
    ALLOCN (termnr, int, nolays);
    ALLOCN (tmasknr, int, nolays);
    ALLOCN (maskdatatype, int, nolays);
    ALLOCN (labeldatatype, int, nolays);
    ALLOCN (termdatatype, int, nolays);
    ALLOCN (tmaskdatatype, int, nolays);

    for (j = 0; j < nolays; ++j) masknr[j] = -1;
    for (j = 0; j < nolays; ++j) labelnr[j] = -1;
    for (j = 0; j < nolays; ++j) termnr[j] = -1;
    for (j = 0; j < nolays; ++j) tmasknr[j] = -1;
    for (j = 0; j < nolays; ++j) maskdatatype[j] = -1;
    for (j = 0; j < nolays; ++j) labeldatatype[j] = -1;
    for (j = 0; j < nolays; ++j) termdatatype[j] = -1;
    for (j = 0; j < nolays; ++j) tmaskdatatype[j] = -1;

    while (fgets (buf, BUFLEN, fp_bml)) {
        if (sscanf (buf, "%s", buf1) <= 0 || buf1[0] == '#')
            continue;  /* empty line or comment line. */
        istext = 0;
        buf1[0] = buf2[0] = buf3[0] = '\0';
        if (sscanf (buf, "%s%s%s", buf1, buf2, buf3) < 2)
            pr_exit (A, 2, bmlist);

        if (strcmp (buf1, "prop_termside") == 0) {
            if (sscanf (buf2, "%d", &prop_termside) < 1) {
                pr_exit (A, 16, "prop_termside");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_termlay") == 0) {
            if (sscanf (buf2, "%d", &prop_termlay) < 1) {
                pr_exit (A, 16, "prop_termlay");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_instance") == 0) {
            if (sscanf (buf2, "%d", &prop_instance) < 1) {
                pr_exit (A, 16, "prop_instance");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_term") == 0) {
            if (sscanf (buf2, "%d", &prop_term) < 1 || prop_term < 0) {
                pr_exit (W, 16, "prop_term");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_labelside") == 0) {
            if (sscanf (buf2, "%d", &prop_labelside) < 1) {
                pr_exit (A, 16, "prop_labelside");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_labellay") == 0) {
            if (sscanf (buf2, "%d", &prop_labellay) < 1) {
                pr_exit (A, 16, "prop_labellay");
            }
            continue;
        }
        else if (strcmp (buf1, "prop_label") == 0) {
            if (sscanf (buf2, "%d", &prop_label) < 1 || prop_label < 0) {
                pr_exit (W, 16, "prop_label");
            }
            continue;
        }

        l = strlen (buf1);
        if (strncmp (buf1, "anno_", 5) == 0) {
            /* This one is for backward compatibility
               with an earlier version */
            istext = 1;
            strcpy (buf4, buf1 + 5);
            strcpy (buf1, buf4);
        }
        else if (l > 6 && strcmp (&buf1[l-6], ":label") == 0) {
            istext = 1;
            buf1[l-6] = '\0';
        }
        else if (l > 5 && strcmp (&buf1[l-5], ":term") == 0) {
            istext = 2;
            buf1[l-5] = '\0';
        }
	for (i = 0; i < nolays; ++i)
	    if (strcmp (buf1, maskname[i]) == 0) break;
	if (i >= nolays) pr_exit (A, 3, buf1);
	gds_laynr = atoi (buf2);
	if ((gds_laynr >= 0 && gds_laynr > MAX_GDS_LAYNR)
            || (gds_laynr < 0 && -gds_laynr > MAX_GDS_LAYNR))
	    pr_exit (A, 5, buf2);
        if (buf3[0] >= '0' && buf3[0] <= '9') {
            gds_datatype = atoi (buf3);
            sprintf (buf4, "with datatype %d ", gds_datatype);
        }
        else {
            gds_datatype = 1;   /* default value for datatype */
            strcpy (buf4, buf2);
        }
	if (gds_laynr >= 0) {
            /* We allow more freedom (so that e.g. different gds numbers
               can be mapped to the same name) and do not do these checks
               anymore.
	    if (masknr[i] >= 0) pr_exit (A, 4, buf1);
	    for (j = 0; j < nolays; ++j)
		if (masknr[j] == gds_laynr
                    && maskdatatype[j] == gds_datatype) pr_exit (W, 6, buf4);
            */
            if (istext == 1) {
	        labelnr[i] = gds_laynr;
	        labeldatatype[i] = gds_datatype;
            }
            else if (istext == 2) {
	        termnr[i] = gds_laynr;
	        termdatatype[i] = gds_datatype;
            }
            else {
	        masknr[i] = gds_laynr;
	        maskdatatype[i] = gds_datatype;
            }
	}
	else {     /* This is old stuff.   We should remove it once. */
	    if (tmasknr[i] >= 0) pr_exit (A, 4, buf1);
	    for (j = 0; j < nolays; ++j)
		if (tmasknr[j] == -gds_laynr
                    && tmaskdatatype[j] == gds_datatype) pr_exit (W, 6, buf4);
	    tmasknr[i] = -gds_laynr;
	    tmaskdatatype[i] = gds_datatype;
	}
    }

    fclose (fp_bml);

    if (verbose) {
	for (j = 0; j < nolays; ++j) {
            if (masknr[j] >= 0)
		PE "%s: using GDS layer# %2d with datatype# %2d for mask '%s'\n",
		argv0, masknr[j], maskdatatype[j], maskname[j]);
            if (termnr[j] >= 0)
		PE "%s: using GDS layer# %2d with datatype# %2d for terminal mask '%s'\n",
		argv0, termnr[j], termdatatype[j], maskname[j]);
            if (labelnr[j] >= 0)
		PE "%s: using GDS layer# %2d with datatype# %2d for label mask '%s'\n",
		argv0, labelnr[j], labeldatatype[j], maskname[j]);
        }
	for (j = 0; j < nolays; ++j) {
            if (tmasknr[j] >= 0)
		PE "%s: using GDS layer# %2d with datatype# %2d for mask '%s' (has terminals)\n",
		argv0, tmasknr[j], tmaskdatatype[j], maskname[j]);
        }
    }

    /* open first volume */
    open_write ();
    write_header ();

    mod_key = dmCheckOut (project, maincell, WORKING, DONTCARE, LAYOUT, READONLY);
    gmc.imported = 0;
    add_cell (maincell);
    gen_list -> c_key = mod_key;

    fp = dmOpenStream (mod_key, "info", "r");
    if (dmGetDesignData (fp, GEO_INFO) <= 0) pr_exit (A, 10, maincell);
    dmCloseStream (fp, COMPLETE);
    if (ginfo.bxl < min_long || ginfo.byb < min_long
	|| ginfo.bxr > max_long || ginfo.byt > max_long) ++check_flag;

    /* init hashlist with local cell names */
    if (!Tflag)
    for (clp = gen_list;;) {
	if (check_flag) check_elmt_bbox (clp -> c_name);
	fp = dmOpenStream (mod_key, "mc", "r");
	while (dmGetDesignData (fp, GEO_MC) > 0)
	    if (!gmc.imported) add_cell (gmc.cell_name);
	dmCloseStream (fp, COMPLETE);
	if ((clp = clp -> c_next)) {
	    mod_key = dmCheckOut (project, clp -> c_name,
			    ACTUAL, DONTCARE, LAYOUT, READONLY);
	    clp -> c_key = mod_key;
	}
	else break;
    }

    for (clp = gen_list; clp; clp = clp -> c_next) {
	project  = clp -> p_key;
	cellname = clp -> r_name;
	if (verbose) PE "%s: *** processing cell '%s'\n", argv0, cellname);
	if (!(mod_key = clp -> c_key)) {
	    mod_key = dmCheckOut (project, clp -> c_name,
			    ACTUAL, DONTCARE, LAYOUT, READONLY);
	    if (check_flag) check_elmt_bbox (cellname);
	}
	write_bgn (BGNSTR);
	write_string (cellname, STRNAME);
	get_term ();
        if (dmStat (mod_key, "annotations", &statBuf) != -1) {
	    get_label ();
        }
	get_box ();
	get_nor ();
	get_mc ();
	write_nodata (ENDSTR);
	dmCheckIn (mod_key, COMPLETE);
    }

    write_nodata (ENDLIB);
    close_write ();

    dmQuit ();
    PE "%s: -- program finished --\n", argv0);
    exit (0);
    return (0);
}

void usage ()
{
    PE "%s %s\n", argv0, TOOLVERSION);
    PE use_msg, argv0);
    exit (1);
}

/* BEGIN OF FILLING CELL GENERATION LIST SECTION
**
** In uppercase mode, the cell names must be unique!
** A special string compare function StrCmp() is used
** for this purpose.
** Note that in uppercase mode the same hash index is
** used for upper- and lowercase strings (to find them).
*/
void add_cell (char *name)
{
    register char *s;
    register DM_PROJECT *proj;
    register struct c_elmt *hp;
    register int  hv = 0;
    register int  nr = 0;
    char *a = name;

    if (gmc.imported && (iflag || rflag)) {
	proj = dmFindProjKey (IMPORTED, a, project, &name, LAYOUT);
    }
    else { /* local */
	proj = project;
    }

    s = name - 1;
    if (uppercase) {
	while (*++s) {
	    hv += *s;
	    if (islower ((int)*s)) ++nr;
	}
	hv -= nr * 32;
    }
    else {
	while (*++s) hv += *s;
    }
    hv %= HASHSIZE;
    for (hp = hashlist[hv]; hp; hp = hp -> c_hash) {
	if (hp -> p_key == proj && !strcmp (hp -> c_name, name)) {
	    ref_name = hp -> r_name;
	    return; /* found */
	}
    }

    ref_name = name;
    if (*name == 'A' && s - name == 8 && name[1] == '_' && name[7] == '_') {
	goto use_another;
    }
    if (proj != default_project && !rflag) {
	goto use_another;
    }
    if (rflag || nr) /* uppercase mode and lowercase letter(s) found */
    for (hp = hashlist[hv]; hp; hp = hp -> c_hash) {
	if (StrCmp (hp -> c_name, name)) {
use_another:
	    sprintf (buf, "A_%05d_", ++take_cnt);
	    if (proj != default_project)
		PE "%s: using name '%s' for '%s' of '%s'\n",
		    argv0, buf, name, proj -> dmpath);
	    else
		PE "%s: using name '%s' for '%s'\n",
		    argv0, buf, name);
	    ref_name = buf;
	    break;
	}
    }
    ALLOC (hp, c_elmt);
    if (!(s = malloc (++s - name))) pr_exit (A, 14, 0);
    else strcpy (s, name);
    hp -> c_name = s;
    if (ref_name == buf) {
	if (!(s = malloc (10))) pr_exit (A, 14, 0);
	else strcpy (s, ref_name);
    }
    hp -> r_name = s;
    hp -> c_key  = 0;
    hp -> p_key  = proj;
    hp -> c_hash = hashlist[hv];
    hashlist[hv] = hp;

    if (!gmc.imported || iflag) { /* add to gen_list */
	hp -> c_next = 0;
	if (!gll) gen_list = hp;
	else gll -> c_next = hp;
	gll = hp;
    }
    else {
	PE "%s: warning: '%s'", argv0, a);
	if (a != ref_name) PE "(%s)", ref_name);
	PE " is reference of imported cell!\n");
    }
}

int StrCmp (char *S1, char *S2)
{
    register char *s1 = S1;
    register char *s2 = S2;
    while (*s1 && *s2) {
	if (islower ((int)*s1)) {
	    if (islower ((int)*s2)) {
		if (*s1 != *s2) return (0);
	    }
	    else
		if (*s1 - 32 != *s2) return (0);
	}
	else if (islower ((int)*s2)) {
	    if (*s1 != *s2 - 32) return (0);
	}
	else if (*s1 != *s2) return (0);
	++s1; ++s2;
    }
    if (*s1 || *s2) return (0);
    return (1);
}
/* END OF FILLING CELL GENERATION LIST SECTION */

/* BEGIN OF GET INFORMATION SECTION
**
** These procedures extract information out of the
** input_files, one line each time.
** Since the EBPG has a restriction on the size of some
** of its parameters (< 2**15), and ICD not, they are tested.
*/
void get_term ()
{
    DM_STREAM *fpterm;

    fpterm = dmOpenStream (mod_key, "term", "r");
    while (dmGetDesignData (fpterm, GEO_TERM) > 0) {
        if ((maskcode = termnr[gterm.layer_no]) >= 0) {
            maskttype = termdatatype[gterm.layer_no];
	    term_processing (gterm.term_name);
        }
	else if (tflag) {
	    if ((maskcode = tmasknr[gterm.layer_no]) < 0
		&& (maskcode = masknr[gterm.layer_no]) < 0) {
		if (tmasknr[gterm.layer_no] == -1) {
		    tmasknr[gterm.layer_no] = -2;
		    pr_exit (W, 7, maskname[gterm.layer_no]);
		}
		continue;
	    }
            maskttype = 1;
	    term_processing (gterm.term_name);
	}
	else {
	    if ((maskcode = masknr[gterm.layer_no]) < 0) {
		if (maskcode == -1) {
		    masknr[gterm.layer_no] = -2;
		    pr_exit (W, 7, maskname[gterm.layer_no]);
		}
		continue;
	    }
            maskdtype = maskdatatype[gterm.layer_no];
	    OLDterm_processing (gterm.term_name);
	}
    }
    dmCloseStream (fpterm, COMPLETE);
}

void OLDterm_processing (char *name)
{
    register int i, j;
    register long dx, dy, xl, xr, yb, yt;
    char mask[32];
    char new_name[128];

    dx = (gterm.nx > 0) ? gterm.dx * mf : 0;
    dy = (gterm.ny > 0) ? gterm.dy * mf : 0;
    xl = gterm.xl * mf;
    xr = gterm.xr * mf;
    yb = gterm.yb * mf;
    yt = gterm.yt * mf;

    for (i = 0; i <= gterm.nx; ++i) {
	for (j = 0; j <= gterm.ny; ++j) {
	    write_box (xl, xr, yb + j * dy, yt + j * dy);

	if (prop_termlay >= 0) {
	    write_short (PROPATTR, prop_termlay);
	    sprintf (mask, "%d", maskcode);
	    write_string (mask, PROPVALUE);
	}

	if (prop_termside >= 0) {
	    write_short (PROPATTR, prop_termside);
	    sprintf (mask, "%d", ANY_DIRECTION);
	    write_string (mask, PROPVALUE);
	}

	    write_short (PROPATTR, prop_term);
	    if (gterm.nx == 0 && gterm.ny == 0)
		strcpy (new_name, name);
	    else if (gterm.nx == 0)
		sprintf (new_name, "%s_%d", name, j);
	    else if (gterm.ny == 0)
		sprintf (new_name, "%s_%d", name, i);
	    else
		sprintf (new_name, "%s_%d_%d", name, i, j);
	    write_string (new_name, PROPVALUE);

	    write_nodata (ENDEL);
	}
	xl += dx; xr += dx;
    }
}

void term_processing (char *name)
{
    register long   i, j, dx, dy, xl, xr, yb, yt;
    char            new_name[128];

    dx = (gterm.nx > 0) ? gterm.dx * mf : 0;
    dy = (gterm.ny > 0) ? gterm.dy * mf : 0;
    xl = gterm.xl * mf;
    xr = gterm.xr * mf;
    yb = gterm.yb * mf;
    yt = gterm.yt * mf;

    if (xr < xl) xl = xr;
    if (yt < yb) yt = yb;

    for (i = 0; i <= gterm.nx; ++i) {
	for (j = 0; j <= gterm.ny; ++j) {
	    write_nodata(TEXT);
	    write_short(LAYER, maskcode);
	    write_short(TEXTTYPE, maskttype);
	    write_rec_header(2 * LONG, XY);
	    print_long(xl);
	    print_long(yb + j * dy);

	    if (gterm.nx == 0 && gterm.ny == 0)
		strcpy(new_name, name);
	    else if (gterm.nx == 0)
		sprintf(new_name, "%s_%ld", name, j);
	    else if (gterm.ny == 0)
		sprintf(new_name, "%s_%ld", name, i);
	    else
		sprintf(new_name, "%s_%ld_%ld", name, i, j);
	    write_string(new_name, STRING);

	    write_nodata(ENDEL);

            if (xr > xl && yt > yb) {
                /* also write a box for the terminal */
                write_box(xl, xr, yb + j * dy, yt + j * dy);
                write_nodata(ENDEL);
            }
	}
	xl += dx;
        xr += dx;
    }
}

void get_label ()
{
    DM_STREAM *dms_anno;

    dms_anno = dmOpenStream (mod_key, "annotations", "r");
    while (dmGetDesignData (dms_anno, GEO_ANNOTATE) > 0) {
        if (ganno.type == GA_LABEL) {
            if ((maskcode = labelnr[ganno.o.Label.maskno]) >= 0) {
                maskttype = labeldatatype[ganno.o.Label.maskno];
	        label_processing (ganno.o.Label.name);
            }
            else if (prop_label >= 0
                     && (maskcode = masknr[ganno.o.Label.maskno]) >= 0) {
	        OLDlabel_processing (ganno.o.Label.name);
            }
            else if (maskcode == -1) {
		labelnr[ganno.o.Label.maskno] = -2;
		pr_exit (W, 17, maskname[ganno.o.Label.maskno]);
	    }
	}
    }
    dmCloseStream (dms_anno, COMPLETE);
}

void label_processing (char *name)
{
    register long   xl, xr, yb, yt;
    char            new_name[128];

    xl = (long)(ganno.o.Label.x * mf);
    xr = xl;
    yb = (long)(ganno.o.Label.y * mf);
    yt = yb;

    write_nodata(TEXT);
    write_short(LAYER, maskcode);
    write_short(TEXTTYPE, maskttype);
    write_rec_header(2 * LONG, XY);
    print_long(xl);
    print_long(yb);

    strcpy(new_name, name);
    write_string(new_name, STRING);

    write_nodata(ENDEL);
}

void OLDlabel_processing (char *name)
{
    register long   xl, xr, yb, yt;
    char mask[32];
    char new_name[128];

    xl = (long)(ganno.o.Label.x * mf);
    xr = xl;
    yb = (long)(ganno.o.Label.y * mf);
    yt = yb;

    write_box (xl, xr, yb, yt);

    if (prop_labellay >= 0) {
        write_short (PROPATTR, prop_labellay);
        sprintf (mask, "%d", maskcode);
        write_string (mask, PROPVALUE);
    }

    if (prop_labelside >= 0) {
        write_short (PROPATTR, prop_labelside);
        sprintf (mask, "%d", ANY_DIRECTION);
        write_string (mask, PROPVALUE);
    }

    write_short (PROPATTR, prop_label);
    strcpy (new_name, name);
    write_string (new_name, PROPVALUE);

    write_nodata (ENDEL);
}

void get_box ()
{
    DM_STREAM *fpbox;

    fpbox = dmOpenStream (mod_key, "box", "r");
    while (dmGetDesignData (fpbox, GEO_BOX) > 0) {
	if ((maskcode = masknr[gbox.layer_no]) < 0) {
	    if (maskcode == -1) {
		masknr[gbox.layer_no] = -2;
		pr_exit (W, 8, maskname[gbox.layer_no]);
	    }
	    continue;
	}
	maskdtype = maskdatatype[gbox.layer_no];
	box_processing ();
    }
    dmCloseStream (fpbox, COMPLETE);
}

void get_nor ()
{
    DM_STREAM *fpnor;
    register int i, nrcoor;
    double  x1, y1, x2, y2, dd;

    fpnor = dmOpenStream (mod_key, "nor", "r");
    while (dmGetDesignData (fpnor, GEO_NOR_INI) > 0) {
	if ((maskcode = masknr[gnor_ini.layer_no]) < 0) {
	    if (maskcode == -1) {
		masknr[gnor_ini.layer_no] = -2;
		pr_exit (W, 9, maskname[gnor_ini.layer_no]);
	    }
	    for (i = 0; i < gnor_ini.no_xy; ++i)
		dmGetDesignData (fpnor, GEO_NOR_XY);
	    continue;
	}
	maskdtype = maskdatatype[gnor_ini.layer_no];

	if (gnor_ini.no_xy > MAX_COOR) {
	    PE "%s: number of co-ordinate pairs in a polygon\n", argv0);
	    PE "of cell '%s' is greater than %d\n", cellname, MAX_COOR);
	    die ();
	}

	nrcoor = 0;
	switch (gnor_ini.elmt) {
	    case SBOX_NOR:
		dmGetDesignData (fpnor, GEO_NOR_XY);
		x1 = gnor_xy.x;
		y1 = gnor_xy.y;
		dmGetDesignData (fpnor, GEO_NOR_XY);
		x2 = gnor_xy.x;
		y2 = gnor_xy.y;
		dd = ((x2 - x1) + (y2 - y1)) / 2;
		poly_coor[nrcoor++] = l_round (x1);
		poly_coor[nrcoor++] = l_round (y1);
		poly_coor[nrcoor++] = l_round (x1 + dd);
		poly_coor[nrcoor++] = l_round (y1 + dd);
		poly_coor[nrcoor++] = l_round (x2);
		poly_coor[nrcoor++] = l_round (y2);
		poly_coor[nrcoor++] = l_round (x2 - dd);
		poly_coor[nrcoor++] = l_round (y2 - dd);
		poly_processing (nrcoor);
		break;
	    case RECT_NOR:
	    case POLY_NOR:
		for (i = 0; i < gnor_ini.no_xy; ++i) {
		    dmGetDesignData (fpnor, GEO_NOR_XY);
		    poly_coor[nrcoor++] = l_round (gnor_xy.x);
		    poly_coor[nrcoor++] = l_round (gnor_xy.y);
		}
		poly_processing (nrcoor);
		break;
	    case WIRE_NOR:
		dmGetDesignData (fpnor, GEO_NOR_XY);
		dd = gnor_xy.x;
		x1 = y1 = 0;
		for (i = 1; i < gnor_ini.no_xy; ++i) {
		    dmGetDesignData (fpnor, GEO_NOR_XY);
		    x1 += gnor_xy.x;
		    y1 += gnor_xy.y;
		    poly_coor[nrcoor++] = l_round (x1);
		    poly_coor[nrcoor++] = l_round (y1);
		}
		write_path (poly_coor, nrcoor, l_round (dd));
		break;
	    case CIRCLE_NOR:
		poly_coor[4] = 0;
		poly_coor[6] = 0;
		poly_coor[7] = 0;
		for (i = 0; i < gnor_ini.no_xy; ++i) {
		    dmGetDesignData (fpnor, GEO_NOR_XY);
		    poly_coor[nrcoor++] = gnor_xy.x;
		    poly_coor[nrcoor++] = gnor_xy.y;
		}
		circle_processing ();
		break;
	    default:
		PE "%s: element of cell '%s' is not a polygon or circle\n",
		    argv0, cellname);
		die ();
	}
    }
    dmCloseStream (fpnor, COMPLETE);
}

void get_mc ()
{
    DM_STREAM *fpmc;

    cell_nrmc = 0;
    fpmc = dmOpenStream (mod_key, "mc", "r");
    while (dmGetDesignData (fpmc, GEO_MC) > 0) mc_processing ();
    dmCloseStream (fpmc, COMPLETE);
}
/* END OF GET INFORMATION SECTION */

/* BEGIN OF PROCESSING SECTION
**
** This section contains the various procedures
** that converts the data obtained in
** the "get information" section.
*/
void box_processing ()
{
    register int i, j;
    register long dx, dy, xl, xr, yb, yt;

    dx = (gbox.nx > 0) ? gbox.dx * mf : 0;
    dy = (gbox.ny > 0) ? gbox.dy * mf : 0;
    xl = gbox.xl * mf;
    xr = gbox.xr * mf;
    yb = gbox.yb * mf;
    yt = gbox.yt * mf;

    for (i = 0; i <= gbox.nx; ++i) {
	for (j = 0; j <= gbox.ny; ++j) {
	    write_box (xl, xr, yb + j * dy, yt + j * dy);
	    write_nodata (ENDEL);
	}
	xl += dx; xr += dx;
    }
}

void poly_processing (int nr)
{
    double  x0, y0, x1, y1, x, y, xa, ya, xb, yb;
    int     x0_xaold, x0_xanew, fxyold, fxynew, richting;
    register int i;

    x0 = poly_coor[0];
    y0 = poly_coor[1];
    x1 = poly_coor[2];
    y1 = poly_coor[3];
    x = poly_coor[4];
    y = poly_coor[5];

    if (x0 < x1)
	richting = 1;
    else
	if (x0 > x1)
	    richting = 2;
	else
	    if (y0 < y1)
		richting = 3;
	    else
		richting = 4;

    if (richting == 3 || richting == 4) {
	x0_xaold = fx0_min_xa (x0, x);
	for (i = 6; i < nr - 1; i += 2) {
	    xa = poly_coor[i];
	    ya = poly_coor[i + 1];
	    x0_xanew = fx0_min_xa (x0, xa);
	    if (x0_xanew != x0_xaold && x0_xanew != 0) {
		x0_xaold = x0_xanew;
		xb = poly_coor[i - 2];
		yb = poly_coor[i - 1];
		y = fy (xa, ya, xb, yb, x0);
	    }
	}
    }
    else {
	fxyold = fxy (x0, y0, x1, y1, x, y);
	for (i = 6; i < nr - 1; i += 2) {
	    xa = poly_coor[i];
	    ya = poly_coor[i + 1];
	    fxynew = fxy (x0, y0, x1, y1, xa, ya);
	    if (fxynew != fxyold && fxynew != 0) {
		fxyold = fxynew;
		xb = poly_coor[i - 2];
		yb = poly_coor[i - 1];
		x = fx (x0, y0, x1, y1, xa, ya, xb, yb);
	    }
	}
    }

    write_bound (poly_coor, nr);
}

/*
** Circle_processing converts a circle from ICD
** to one or more polygons.
*/
void circle_processing ()
{
    double *poly_co;
    double  cx, cy, ri, ro, aa, sa, la, st;
    int     xpo, ypo, xpi, ypi, xpo_old, ypo_old, xpi_old, ypi_old;
    int     dxo, dyo, dxi, dyi, dxo_old, dyo_old, dxi_old, dyi_old;
    int     ready = 0;
    register int ti, to;

    cx = poly_coor[0]; /* center x */
    cy = poly_coor[1]; /* center y */
    ro = poly_coor[2]; /* radius outside */
    ri = poly_coor[3]; /* radius inside */
    if (ri < 0 || ro <= ri) {
	PE "%s: in cell '%s', illegal circle radius (ro = %.3f, ri = %.3f)\n",
	    argv0, cellname, ro, ri);
	die ();
    }

    sa = poly_coor[6]; /* start angle */
    if ((la = poly_coor[7]) == 0) la = 360; /* last angle */
    if (sa < 0 || sa >= 360 || la <= 0 || la > 360 || la == sa) {
	PE "%s: in cell '%s', illegal circle angles (sa = %.3f, la = %.3f)\n",
	    argv0, cellname, sa, la);
	die ();
    }
    if (sa >= la) sa -= 360; /* la > sa */

    if ((st = poly_coor[4]) == 0) st = 0.01; /* minimum step angle */
    else if (st > 0) {
	if (st < 8) ti = (int)(360.0 / st);
	else {
	    ti = (int)st;
	    if (ti % 8) ti = (int)(360.0 / st);
	}
	if (ti > 8) {
	    ti /= 8; ti *= 8; /* make it modulo 8 */
	}
	st = 360.0 / ti;
    }
    if (st < 0.01 || st > 45) {
	PE "%s: in cell '%s', illegal circle step angle (st = %.3f)\n",
	    argv0, cellname, st);
	die ();
    }

    for (aa = 270; aa > sa;) aa -= 90;
    while (aa <= sa) aa += st;
    st *= (3.14159265 / 180); /* step angle */
    aa *= (3.14159265 / 180); /* first start angle > sa */
    sa *= (3.14159265 / 180); /* start angle */
    la *= (3.14159265 / 180); /* last angle */

    xpi_old = l_round (cx + ri * cos (sa));
    ypi_old = l_round (cy + ri * sin (sa));
    xpo_old = l_round (cx + ro * cos (sa));
    ypo_old = l_round (cy + ro * sin (sa));

    while (!ready) {
	poly_co = poly_coor;
	poly_coor[0] = xpi_old;
	poly_coor[1] = ypi_old;
	poly_coor[2] = xpo_old;
	poly_coor[3] = ypo_old;
	dxo_old = dyo_old = dxi_old = dyi_old = 0;
	to = 4;     /* first free x position (outside) */
	ti = MAX_COOR * 2 - 1; /* y position (inside)  */

	while (aa < la && ti > to + 2) {
	    xpo = l_round (cx + ro * cos (aa));
	    ypo = l_round (cy + ro * sin (aa));
	    if (xpo != xpo_old || ypo != ypo_old) {
		dxo = xpo_old - xpo;
		dyo = ypo_old - ypo;
		if (dxo == dxo_old && dyo == dyo_old) to -= 2;
		else { dxo_old = dxo; dyo_old = dyo; }
		poly_coor[to++] = xpo_old = xpo;
		poly_coor[to++] = ypo_old = ypo;
	    }
	    if (ri > 0) {
		xpi = l_round (cx + ri * cos (aa));
		ypi = l_round (cy + ri * sin (aa));
		if (xpi != xpi_old || ypi != ypi_old) {
		    dxi = xpi_old - xpi;
		    dyi = ypi_old - ypi;
		    if (dxi == dxi_old && dyi == dyi_old) ti += 2;
		    else { dxi_old = dxi; dyi_old = dyi; }
		    poly_coor[ti--] = ypi_old = ypi;
		    poly_coor[ti--] = xpi_old = xpi;
		}
	    }
	    aa += st;
	}

	if (aa >= la) { /* set last point exact on position */
	    ready = 1;
	    xpo = l_round (cx + ro * cos (la));
	    ypo = l_round (cy + ro * sin (la));
	    if (xpo != xpo_old || ypo != ypo_old) {
		dxo = xpo_old - xpo;
		dyo = ypo_old - ypo;
		if (dxo == dxo_old && dyo == dyo_old) to -= 2;
		poly_coor[to++] = xpo;
		poly_coor[to++] = ypo;
	    }
	    if (ri > 0) {
		xpi = l_round (cx + ri * cos (la));
		ypi = l_round (cy + ri * sin (la));
		if (xpi != xpi_old || ypi != ypi_old) {
		    dxi = xpi_old - xpi;
		    dyi = ypi_old - ypi;
		    if (dxi == dxi_old && dyi == dyi_old) ti += 2;
		    poly_coor[ti--] = ypi;
		    poly_coor[ti--] = xpi;
		}
	    }
	    else {
		if (poly_coor[2] == xpo && poly_coor[3] == ypo) {
		    to -= 4;
		    poly_co += 2;
		    if (poly_co[0] == poly_co[2] &&
			poly_co[0] == poly_co[to - 2]) {
			poly_co += 2;
			to -= 2;
		    }
		}
		else {
		    dxo_old = (int)(poly_coor[2] - poly_coor[0]);
		    dyo_old = (int)(poly_coor[3] - poly_coor[1]);
		    dxo = (int)(poly_coor[0] - xpo);
		    dyo = (int)(poly_coor[1] - ypo);
		    if (dxo == dxo_old && dyo == dyo_old) {
			poly_co += 2;
			to -= 2;
		    }
		}
	    }
	}

	if (to < ti) { /* place ti- after to-points */
	    if (ti + 1 < MAX_COOR * 2)
	    while (++ti < MAX_COOR * 2) { /* x position */
		poly_coor[to++] = poly_coor[ti++]; /* copy_x */
		poly_coor[to++] = poly_coor[ti];   /* copy_y */
	    }
	}
	else to = MAX_COOR * 2;

	write_bound (poly_co, to);
    }
}

long l_round (double real)
{
    return (long) ((real > 0) ? (real * mf + 0.5) : (real * mf - 0.5));
}

int fxy (double x0, double y0, double x1, double y1, double x, double y)
{
/* This function decides of the point (x,y) is above,
** under or on the line, which is described by the
** points (x0,y0) and (x1,y1).
*/
    double  tus_res;
    tus_res = (double) (y0 - y1) / (x0 - x1);
    tus_res = y - y1 - tus_res * (x - x1);
    if (tus_res > 0) return (1);
    if (tus_res < 0) return (-1);
    return (0);
}

int fx0_min_xa (double x0, double xa)
{
/* This function tests for the maximum of two numbers
** and returns 1 when the second variable the largest is.
*/
    if (xa > x0) return (1);
    if (xa < x0) return (-1);
    return (0);
}

double fy (double xa, double ya, double xb, double yb, double x0)
{
/* This function returns the y-value of the intersection
** of the line, which is described by the points (xa,ya)
** and (xb,yb) with the line x = x0.
*/
    double  tus_res;
    tus_res = (double) (ya - yb) / (xa - xb);
    return (x0 * tus_res + yb - xb * tus_res);
}

double fx (double x0, double y0, double x1, double y1, double xa, double ya, double xb, double yb)
{
/* This function returns the x-value of the intersection
** of the line, which is described by the points (x0,y0)
** and (x1,y1) with the line, which is described by the
** points (xa,ya) and (xb,yb).
*/
    double  tus_res1, tus_res2;
    if (xa == xb) return (xa);
    tus_res1 = (double) (y0 - y1) / (x0 - x1);
    tus_res2 = (double) (ya - yb) / (xa - xb);
    return ((y1 - x1 * tus_res1 - yb + xb * tus_res2) / (tus_res2 - tus_res1));
}

void mc_processing ()
{
    long    sfx, sfy;

    if (!Tflag) add_cell (gmc.cell_name);
    else ref_name = gmc.cell_name;

    ++cell_nrmc;
    if (gmc.mtx[0] == 0) {
	if (gmc.mtx[3] > 0) {
	    angle = 90;
	    if (gmc.mtx[1] > 0) {
		refl = 1;	/* MX+R90 */
		sfy = gmc.mtx[1];
	    }
	    else {
		refl = 0;	/* R90 */
		sfy = -gmc.mtx[1];
	    }
	    sfx = gmc.mtx[3];
	}
	else {
	    angle = 270;
	    if (gmc.mtx[1] > 0) {
		refl = 0;	/* R270 */
		sfy = gmc.mtx[1];
	    }
	    else {
		refl = 1;	/* MX+R270 */
		sfy = -gmc.mtx[1];
	    }
	    sfx = -gmc.mtx[3];
	}
    }
    else {
	if (gmc.mtx[0] > 0) {
	    angle = 0;
	    if (gmc.mtx[4] > 0) {
		refl = 0;	/* R0 */
		sfy = gmc.mtx[4];
	    }
	    else {
		refl = 1;	/* MX+R0 */
		sfy = -gmc.mtx[4];
	    }
	    sfx = gmc.mtx[0];
	}
	else {
	    angle = 180;
	    if (gmc.mtx[4] > 0) {
		refl = 1;	/* MX+R180 */
		sfy = gmc.mtx[4];
	    }
	    else {
		refl = 0;	/* R180 */
		sfy = -gmc.mtx[4];
	    }
	    sfx = -gmc.mtx[0];
	}
    }

    if (sfx != 1 || sfy != 1) {
	if (sfx != sfy || sfx < 2) {
	    PE "%s: sfx/sfy error in %ld-th cell-call of cell '%s'\n",
		argv0, cell_nrmc, cellname);
	    die ();
	}
	magnify = sfx;
    }
    else magnify = 0;

    if (gmc.nx == 0 && gmc.ny == 0)
	write_sref (gmc.mtx[2], gmc.mtx[5]);
    else
	write_aref ();
}
/* END OF PROCESSING SECTION */

/* BEGIN OF WRITING SECTION */
/*
** Write box-record to the outputfile.
*/
void write_box (long xl, long xr, long yb, long yt)
{
    write_nodata (BOUNDARY);
    write_short (LAYER, maskcode);
    write_short (DATATYPE, maskdtype);
    write_rec_header (10 * LONG, XY);
    print_long (xl); print_long (yb);
    print_long (xr); print_long (yb);
    print_long (xr); print_long (yt);
    print_long (xl); print_long (yt);
    print_long (xl); print_long (yb);
}

/*
** Write HEADER, BGNLIB, LIBNAME, UNITS.
*/
void write_header ()
{
    timeval = time (0L);
    timeptr = localtime (&timeval);
    year = timeptr -> tm_year;
    mon = 1 + timeptr -> tm_mon;
    day = timeptr -> tm_mday;
    hour = timeptr -> tm_hour;
    min = timeptr -> tm_min;
    sec = timeptr -> tm_sec;

    /* Calculate the tape-code.
    ** This must be an unique value for each library.
    */
    t_code[0] = year + mon + day;
    t_code[1] = day + hour + min;
    t_code[2] = min + sec;

    write_short (HEADER, VERSION);
    write_bgn (BGNLIB);
    write_string (libname, LIBNAME);
    write_rec_header (2 * DOUBLE, UNITS);
    if (uu > 0)
	print_double (uu);
    else
	print_double (gds_unit);
    print_double (gds_unit * 1E-6);
}

/*
** Write a No_Data_Present record to the outputfile.
*/
void write_nodata (int type)
{
    if ((type & 0XFF) != 0) pr_exit (A, 11, "write_nodata");
    write_rec_header (0, type);
}

/*
** Write ONE Two-Byte_Integer record to the outputfile.
*/
void write_short (int type, int value)
{
    int dt = type & 0XFF;
    if (dt != 1 && dt != 2) pr_exit (A, 11, "write_short");
    write_rec_header (SHORT, type);
    print_short (value);
}

/*
** Write a ASCII_String record to the outputfile.
*/
void write_string (char *str, int type)
{
    register int len, c;

    if ((type & 0XFF) != 6) pr_exit (A, 11, "write_string");
    if ((len = strlen (str)) > MAX_STRLEN) pr_exit (A, 12, "512");
    write_rec_header (len + len % 2, type);
    while ((c = *str++)) {
	if (uppercase && islower (c)) c -= 32;
	PUTBYTE (c);
    }
    if (len % 2) PUTBYTE (c);
}

void write_rec_header (int len, int type)
{
    len += BASE_LEN;
    if (TAPE_MUL_VL) if (byte_cnt + len > file_len) next_volume ();
    byte_cnt += len;
    PUTBYTE (len >> 8);
    PUTBYTE (len);
    PUTBYTE (type >> 8);
    PUTBYTE (type);
}

/*
** Write a LIBBGN or STRBGN-record to the outputfile.
*/
void write_bgn (int type)
{
    timeval = time (0L);
    timeptr = localtime (&timeval);

    write_rec_header (12 * SHORT, type);
    print_short (year);
    print_short (mon);
    print_short (day);
    print_short (hour);
    print_short (min);
    print_short (sec);
    print_short (timeptr -> tm_year);
    print_short (timeptr -> tm_mon + 1);
    print_short (timeptr -> tm_mday);
    print_short (timeptr -> tm_hour);
    print_short (timeptr -> tm_min);
    print_short (timeptr -> tm_sec);
}

/*
** Write a BOUNDARY-record to the outputfile.
*/
void write_bound (double *poly_co, int nr)
{
    register int i, j, k;
    double dx, dy;

    dx = (gnor_ini.nx > 0) ? l_round (gnor_ini.dx) : 0;
    dy = (gnor_ini.ny > 0) ? l_round (gnor_ini.dy) : 0;

    poly_co[nr++] = poly_co[0];
    poly_co[nr++] = poly_co[1];

    for (i = 0; i <= gnor_ini.nx; ++i) {
	for (j = 0; j <= gnor_ini.ny; ++j) {
	    write_nodata (BOUNDARY);
	    write_short (LAYER, maskcode);
	    write_short (DATATYPE, maskdtype);
	    write_rec_header (nr * LONG, XY);
	    for (k = 0; k < nr;) {
		print_long ((long) (poly_co[k++] + i * dx));
		print_long ((long) (poly_co[k++] + j * dy));
	    }
	    write_nodata (ENDEL);
	}
    }
}

/*
** Write a PATH-record to the outputfile.
*/
void write_path (double *poly_co, int nr, long width)
{
    register int i, j, k;
    double dx, dy;

    dx = (gnor_ini.nx > 0) ? l_round (gnor_ini.dx) : 0;
    dy = (gnor_ini.ny > 0) ? l_round (gnor_ini.dy) : 0;

    for (i = 0; i <= gnor_ini.nx; ++i) {
	for (j = 0; j <= gnor_ini.ny; ++j) {
	    write_nodata (PATH);
	    write_short (LAYER, maskcode);
	    write_short (DATATYPE, maskdtype);
	    write_rec_header (LONG, WIDTH);
	    print_long (width);
	    write_rec_header (nr * LONG, XY);
	    for (k = 0; k < nr;) {
		print_long ((long) (poly_co[k++] + i * dx));
		print_long ((long) (poly_co[k++] + j * dy));
	    }
	    write_nodata (ENDEL);
	}
    }
}

/*
** Write a Structure Ref. record to the outputfile.
*/
void write_sref (long tx, long ty)
{
    write_nodata (SREF);
    write_string (ref_name, SNAME);
    if (refl || magnify || angle) {
	write_short (STRANS, refl << 15);
	if (magnify) {
	    write_rec_header (DOUBLE, MAG);
	    print_double ((double) magnify);
	}
	if (angle) {
	    write_rec_header (DOUBLE, ANGLE);
	    print_double ((double) angle);
	}
    }
    write_rec_header (2 * LONG, XY);
    if (check_flag) {
	if (tx < min_long || tx > max_long) ref_error ("tx", tx);
	if (ty < min_long || ty > max_long) ref_error ("ty", ty);
    }
    print_long (tx * mf);
    print_long (ty * mf);

    write_short (PROPATTR, prop_instance);
    write_string (gmc.inst_name, PROPVALUE);
    write_nodata (ENDEL);
}

/*
** Write an Array Ref. record to the outputfile.
*/
void write_aref ()
{
    long x1, x2, x3, y1, y2, y3, dx, dy;
    register int col, row;

    x1 = gmc.mtx[2];
    y1 = gmc.mtx[5];
    dx = gmc.dx;
    dy = gmc.dy;

    if (aflag) { /* don't generate arefs */
	for (row = 0; row <= gmc.ny; ++row)
	    for (col = 0; col <= gmc.nx; ++col) {
		write_sref (x1 + col * dx, y1 + row * dy);
	    }
	return;
    }

    write_nodata (AREF);
    write_string (ref_name, SNAME);
    if (refl || magnify || angle) {
	write_short (STRANS, refl << 15);
	if (magnify) {
	    write_rec_header (DOUBLE, MAG);
	    print_double ((double) magnify);
	}
	if (angle) {
	    write_rec_header (DOUBLE, ANGLE);
	    print_double ((double) angle);
	}
    }

    if (refl) { /* MX */
	if (angle == 0 || angle == 180) {
	    col = gmc.nx + 1;
	    row = gmc.ny + 1;
	    if (angle == 0) {
		if (dx < 0) { dx = -dx; x1 -= gmc.nx * dx; }
		if (dy < 0) { dy = -dy; }
		else y1 += gmc.ny * dy;
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) x2 += col * dx;
		if (row > 1) y3 -= row * dy;
	    }
	    else {
		if (dx < 0) { dx = -dx; }
		else x1 += gmc.nx * dx;
		if (dy < 0) { dy = -dy; y1 -= gmc.ny * dy; }
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) x2 -= col * dx;
		if (row > 1) y3 += row * dy;
	    }
	}
	else {
	    col = gmc.ny + 1;
	    row = gmc.nx + 1;
	    if (angle == 90) {
		if (dx < 0) { dx = -dx; x1 -= gmc.nx * dx; }
		if (dy < 0) { dy = -dy; y1 -= gmc.ny * dy; }
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) y2 += col * dy;
		if (row > 1) x3 += row * dx;
	    }
	    else {
		if (dx < 0) { dx = -dx; }
		else x1 += gmc.nx * dx;
		if (dy < 0) { dy = -dy; }
		else y1 += gmc.ny * dy;
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) y2 -= col * dy;
		if (row > 1) x3 -= row * dx;
	    }
	}
    }
    else {
	if (angle == 0 || angle == 180) {
	    col = gmc.nx + 1;
	    row = gmc.ny + 1;
	    if (angle == 0) {
		if (dx < 0) { dx = -dx; x1 -= gmc.nx * dx; }
		if (dy < 0) { dy = -dy; y1 -= gmc.ny * dy; }
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) x2 += col * dx;
		if (row > 1) y3 += row * dy;
	    }
	    else {
		if (dx < 0) { dx = -dx; }
		else x1 += gmc.nx * dx;
		if (dy < 0) { dy = -dy; }
		else y1 += gmc.ny * dy;
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) x2 -= col * dx;
		if (row > 1) y3 -= row * dy;
	    }
	}
	else {
	    col = gmc.ny + 1;
	    row = gmc.nx + 1;
	    if (angle == 90) {
		if (dx < 0) { dx = -dx; }
		else x1 += gmc.nx * dx;
		if (dy < 0) { dy = -dy; y1 -= gmc.ny * dy; }
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) y2 += col * dy;
		if (row > 1) x3 -= row * dx;
	    }
	    else {
		if (dx < 0) { dx = -dx; x1 -= gmc.nx * dx; }
		if (dy < 0) { dy = -dy; }
		else y1 += gmc.ny * dy;
		x3 = x2 = x1;
		y3 = y2 = y1;
		if (col > 1) y2 -= col * dy;
		if (row > 1) x3 += row * dx;
	    }
	}
    }

    if (check_flag) {
	if (x1 < min_long || x1 > max_long) ref_error ("tx", x1);
	if (y1 < min_long || y1 > max_long) ref_error ("ty", y1);
	if (x2 > max_long) ref_error ("tx", x2);
	if (y3 > max_long) ref_error ("ty", y3);
    }

    write_rec_header (2 * SHORT, COLROW);
    print_short (col);
    print_short (row);
    write_rec_header (6 * LONG, XY);
    print_long (x1 * mf);
    print_long (y1 * mf);
    print_long (x2 * mf);
    print_long (y2 * mf);
    print_long (x3 * mf);
    print_long (y3 * mf);
    write_short (PROPATTR, prop_instance);
    write_string (gmc.inst_name, PROPVALUE);
    write_nodata (ENDEL);
}

/*
** Write TAPENUM-, TAPECODE- and LIBNAME-record.
*/
void write_label ()
{
    write_short (TAPENUM, vol_nr);
    write_rec_header (3 * LONG, TAPECODE);
    print_long (t_code[0]);
    print_long (t_code[1]);
    print_long (t_code[2]);
    write_string (libname, LIBNAME);
}

/*
** Go to the next volume.
*/
void next_volume ()
{
    TAPE_MUL_VL = 0;
    if (vol_nr == 1) {
	write_label ();
	file_len += label_len - 3 * SHORT;
    }
    else {
	write_short (TAPENUM, vol_nr);
    }
    close_write ();
    open_write ();
    write_label ();
    TAPE_MUL_VL = 1;
}

/*
** Open GDS output file.
*/
void open_write ()
{
    if (++vol_nr > 1)
	sprintf (file_name, "%s.gds.%d", maincell, vol_nr);
    else
	sprintf (file_name, "%s.gds", maincell);
    fpdata = fopen (file_name, "wb");
    if (!fpdata) pr_exit (A, 0, file_name);
    byte_cnt = 0;
}

/*
** Close GDS output file.
*/
void close_write ()
{
#if 0
    register int len, j;

    /* fill the last block with 0000 */
    if ((len = byte_cnt % BLOCKSIZE) > 0) {
	len = BLOCKSIZE - len;
	for (j = 0; j < len; ++j) PUTBYTE (0);
    }
#endif
    fclose (fpdata);
}

/*
** Convert a double to eight-byte real number in GDS format.
*/
void print_double (double ln)
{
    double d, xx;
    unsigned long m_r, m_l, m_h;
    int e_h, expo;

    if (ln != 0) {
	xx = (ln < 0 ? -ln : ln);
	xx = frexp (xx, &expo);
	xx = ldexp (xx, 28 + expo % 4);
	m_l = (unsigned long) xx;
        d = ldexp (xx - m_l, 32);
        m_r = (unsigned long) d;
	if (m_l & 0XF0000000) {
	    e_h = 65 + expo / 4;
	    m_h = (m_l >> 8);
	    m_l = (m_l << 24) | (m_r >> 8);
	}
	else {
	    e_h = 64 + expo / 4;
	    m_h = (m_l >> 4);
	    m_l = (m_l << 28) | (m_r >> 4);
	}
	if (e_h > 127) { /* overflow */
	    pr_exit (A, 15, 0);
	    e_h = 127;
	    m_h = 0X00FFFFFF;
	    m_l = 0XFFFFFFFF;
	}
	else if (e_h < 0) { /* underflow */
	    goto zero;
	}
	if (ln < 0) e_h |= 0X80;
	m_h |= (e_h << 24);
    }
    else { /* zero value */
zero:
	m_l = m_h = 0;
    }
    print_long (m_h);
    print_long (m_l);
}

/*
** Convert a int to two-byte integer number in GDS format.
*/
void print_short (int ln)
{
    PUTBYTE (ln >> 8);
    PUTBYTE (ln);
}

/*
** Convert a int to four-byte integer number in GDS format.
*/
void print_long (long ln)
{
    PUTBYTE (ln >> 24);
    PUTBYTE (ln >> 16);
    PUTBYTE (ln >>  8);
    PUTBYTE (ln);
}
/* END OF WRITING SECTION */

/* BEGIN OF MISCELLANEOUS SECTION */
void check_elmt_bbox (char *cell)
{
    DM_STREAM *fp;
    fp = dmOpenStream (mod_key, "info", "r");
    if (dmGetDesignData (fp, GEO_INFO) <= 0) pr_exit (A, 10, cell);
    if (dmGetDesignData (fp, GEO_INFO) <= 0) pr_exit (A, 10, cell);
    if (dmGetDesignData (fp, GEO_INFO) <= 0) pr_exit (A, 10, cell);
    dmCloseStream (fp, COMPLETE);
    if (ginfo.bxl < min_long) cell_error ("xl", cell, ginfo.bxl);
    if (ginfo.byb < min_long) cell_error ("yb", cell, ginfo.byb);
    if (ginfo.bxr > max_long) cell_error ("xr", cell, ginfo.bxr);
    if (ginfo.byt > max_long) cell_error ("yt", cell, ginfo.byt);
}

void cell_error (char *s, char *cell, long value)
{
    PE "%s: element b%s of cell '%s' out of limit (= %ld)\n",
	argv0, s, cell, value);
    exceeds_msg ();
}

void ref_error (char *s, long value)
{
    PE "%s: %s out of limit in %ld-th cell-call of cell '%s' (= %ld)\n",
	argv0, s, cell_nrmc, cellname, value);
    exceeds_msg ();
}

void exceeds_msg ()
{
    PE "%s: exceeds min/max value after multip. with %d\n", argv0, mf);
    PE "%s: minimum and maximum values are: %d, %d\n",
	argv0, min_long, max_long);
    PE "%s: choice a smaller multiplication factor (option -f)\n", argv0);
    die ();
}

void dmError (char *s)
{
    dmPerror (s);
    PE "%s: error in DMI function\n", argv0);
    die ();
}

void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN);	/* ignore signal */
    PE "%s: recieved signal: %d\n", argv0, sig);
    die ();
}

static char *err_list[] = /* error messages */
{
     /*  0 */ "cannot create file '%s'",
     /*  1 */ "cannot read file '%s'",
     /*  2 */ "after read of file '%s'",
     /*  3 */ "unknown mask '%s' in bmlist-file",
     /*  4 */ "already used mask '%s' in bmlist-file",
     /*  5 */ "illegal GDS masknr '%s' in bmlist-file",
     /*  6 */ "already used GDS masknr '%s' in bmlist-file",
     /*  7 */ "terminal with layer '%s' skipped",
     /*  8 */ "box layer '%s' skipped",
     /*  9 */ "nor layer '%s' skipped",
     /* 10 */ "cannot read bounding box of cell '%s'",
     /* 11 */ "%s: illegal data type",
     /* 12 */ "too long string (> %s)",
     /* 13 */ "sizeof int not equal to 4 bytes",
     /* 14 */ "cannot alloc",
     /* 15 */ "floating point overflow",
     /* 16 */ "no correct value for '%s' in bmlist file.",
     /* 17 */ "label with layer '%s' skipped",
};

/*
** Give a error message (mode=A(bort)) or a warning (mode=W(arning))
** When the mode is E(xit), the program exits without error message.
** When the mode is W or I(nfo), the program does not an exit.
*/
void pr_exit (int mode, int err_no, char *cs)
{
    PE "%s: ", argv0);
    if (mode == W) PE "warning: ");
    if (mode == A) PE "error: ");

    if (err_no < 0 || err_no >= sizeof (err_list) / sizeof (char *)) {
	PE "due to number '%d'", err_no);
	if (cs && *cs) PE ", %s", cs);
    }
    else
	PE err_list[err_no], cs);
    PE "\n");

    if (mode == A) die ();
}

void die ()
{
    if (fpdata) unlink (file_name);
    dmQuit ();
    PE "%s: -- program aborted --\n", argv0);
    exit (1);
}
/* END OF MISCELLANEOUS SECTION */
