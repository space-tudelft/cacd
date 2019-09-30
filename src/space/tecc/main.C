/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <unistd.h>

#include "src/space/libmin/mindefs.h"
#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

extern char *argv0;
char *use_msg = (char*)"\nUsage: %s [-censtvV] [-m maskdatafile] [-p process] file\n\n";

mask_t cNull;

int silent = 0;
int t_flag = 0;
int v_flag = 0;

int docompress = 1;
int notFatal = 0;
int printCondList = 0;
int printNonCoded = 0;
int prVerbose = 0;

struct contact *cons = NULL;
struct transistor *tors = NULL;
struct capacitance *caps = NULL;
struct resistance *ress = NULL;
struct vdimension *vdms = NULL;
struct shape *shps = NULL;
struct bipoTor *bjts = NULL;
struct junction *juns = NULL;
struct connect *cnts = NULL;
struct dielectric *diels = NULL;
struct substrate *subcaps = NULL;
struct substrate *substrs = NULL;
struct selfsubdata *selfs = NULL;
struct mutualsubdata *muts = NULL;
struct newsubdata *newmsks = NULL;
struct subcont *subconts = NULL;
struct resizedata *resizes = NULL;
struct waferdata *wafers = NULL;
struct w_list *w_list_p, *w_list_n;
int bem_depth = 0;
int bem_type = 0;

double *diel_zq_values = NULL;
double *diel_zp_values = NULL;
double *diel_r_values = NULL;
int diel_grid_count = 80;
int diel_num_annealing_iterations = 10000;
double diel_max_determinant_binning_error = 0.0;
double diel_max_adjoint_binning_error = 0.0;
double diel_max_annealed_inverse_matrix_binning_error = 0.0;
double diel_max_preprocessed_annealing_matrices_binning_error = 0.0;
double diel_max_reduce_error = 0.0;
double diel_max_annealing_error = -4;
double diel_r_switch = -1;

double *subs_zq_values = NULL;
double *subs_zp_values = NULL;
double *subs_r_values = NULL;
int subs_grid_count = 80;
int subs_num_annealing_iterations = 10000;
double subs_neumann_simulation_ratio = 100.0;
double subs_max_determinant_binning_error = 0.0;
double subs_max_adjoint_binning_error = 0.0;
double subs_max_annealed_inverse_matrix_binning_error = 0.0;
double subs_max_preprocessed_annealing_matrices_binning_error = 0.0;
double subs_max_reduce_error = 0.0;
double subs_max_annealing_error = -4;
double subs_r_switch = -1;

int con_cnt = 0;
int tor_cnt = 0;
int cap_cnt = 0;
int res_cnt = 0;
int vdm_cnt = 0;
int waf_cnt = 0;
int fem_cnt = 0;
int fem_res_cnt = 0;
int shp_cnt = 0;
int bjt_cnt = 0;
int jun_cnt = 0;
int cnt_cnt = 0;
int diel_cnt = 0;
int diel_zq_values_cnt = 0;
int diel_zp_values_cnt = 0;
int diel_r_values_cnt  = 0;
int subcap_cnt = 0;
int substr_cnt = 0;
int subs_zq_values_cnt = 0;
int subs_zp_values_cnt = 0;
int subs_r_values_cnt  = 0;
int self_cnt = 0;
int mut_cnt = 0;
int new_cnt = 0;
int sbc_cnt = 0;
int resize_cnt = 0;

DM_PROCDATA *procdata;
struct submasks *subdata;

char ** maskdrawcolor = NULL;
char ** masknewcolor = NULL;
char *  masksubcolor = (char*)"glass";

int conducCnt;
int conducCntStd;
int conducCntPos;
int *conducTransf;
int *maskTransf;

int sSlotCnt, sSlotCnt2, eSlotCnt, oSlotCnt;
int maxNbrKeys, maxNbrKeys2, maxEdgeKeys;
int maxKeys, maxKeys2;
int maxprocmasks;
int nbrKeySlots, nbrKeySlots2;
int nbrLays;

struct elemRef **keytab, **keytab2;
struct layerRef *Keylist;
struct layerRef *keylist, *keylist2;

char mdata[512];
char *maskdata = NULL;
char *process = NULL;
char *icdprocess = NULL;
char *tclExportPath = NULL;

void addNewMasks ()
{
    static int maxnew = 0;
    struct condListRef *cl;
    struct newMaskRef *nm;

    for (nm = first_newmask_list; nm; nm = nm -> next) {
	if (!nm -> resized)
	for (cl = nm -> cond_lists; cl; cl = cl -> next) {
	    if (new_cnt == maxnew) {
		maxnew += MAXNEWMSK;
		REALLOC (newmsks, maxnew, struct newsubdata, new_cnt);
	    }
	    newmsks[new_cnt].name = nm -> name;
	    newmsks[new_cnt].id   = nm -> id;
	    newmsks[new_cnt].mask = nm -> mask;
	    newmsks[new_cnt].cond = cl -> cond_list;
	    ++new_cnt;
	}
    }
}

char *outfile = NULL;

int main (int argc, char *argv[])
{
    char *s, *techfile = NULL;
    int id, proc_id;
    FILE *fp_proclist = NULL;

    min_init_library();

    argv0 = (char*)"tecc";

    while (--argc > 0) {
        if ((*++argv)[0] == '-' ) {
	    for (s = *argv + 1; *s != '\0'; s++) {
	        switch (*s) {
		    case 'c':
			printNonCoded = 1;
			break;
		    case 'e':
			printCondList = 1;
			break;
		    case 'm':
			if (process) {
			    fprintf (stderr, "%s: illegal option: %c (-p already specified)\n", argv0, *s);
			    exit (1);
			}
			if (!(maskdata = *++argv)) {
			    fprintf (stderr, use_msg, argv0);
			    exit (1);
			}
			argc--;
			*(s+1) = '\0';
			break;
		    case 'p':
			if (maskdata) {
			    fprintf (stderr, "%s: illegal option: %c (-m already specified)\n", argv0, *s);
			    exit (1);
			}
			if (!(process = *++argv)) {
			    fprintf (stderr, use_msg, argv0);
			    exit (1);
			}
			argc--;
			*(s+1) = '\0';
			break;
		    case 'T':
			if (!(tclExportPath = *++argv)) {
			    fprintf (stderr, use_msg, argv0);
			    exit (1);
			}
			argc--;
			*(s+1) = '\0';
			break;
		    case 's':
			silent = 1;
			break;
		    case 't':
			t_flag = 1;
			break;
		    case 'n':
			docompress = 0;
			break;
		    case 'V':
			prVerbose = 1;
                        break;
		    case 'v':
			v_flag = 1;
			break;
		    default:
			fprintf (stderr, "%s: illegal option: %c\n", argv0, *s);
			fprintf (stderr, use_msg, argv0);
			exit (1);
		}
	    }
	}
	else if (!techfile) techfile = *argv;
	else {
	    fprintf (stderr, use_msg, argv0);
	    exit (1);
	}
    }

    if (!techfile) {
	fprintf (stderr, use_msg, argv0);
	exit (1);
    }

    int l = strlen (techfile);
    if (l < 2 || techfile[l-1] != 's' || techfile [l-2] != '.') {
	fprintf (stderr, "%s: illegal inputfile name\n", argv0);
	exit (1);
    }

    dmInit (argv0);

    if (!process && !maskdata) {
	DM_PROJECT *pkey;
	icdprocess = getenv ("ICDPROCESS");
	if ((pkey = dmOpenProject (DEFAULT_PROJECT, PROJ_READ)))
	    procdata = (DM_PROCDATA *)dmGetMetaDesignData (PROCESS, pkey);
	else
	    process = icdprocess, icdprocess = NULL;
    }

    if (process) {
	char str[80];

	if (!*process) {
	    fprintf (stderr, "%s: illegal process name\n", argv0);
	    exit (1);
	}
	if (strchr (process, '/')) goto procpath;

	sprintf (mdata, "%s/share/lib/process/processlist", icdpath);
	if ((fp_proclist = fopen (mdata, "r")) == NULL) {
	    fprintf (stderr, "%s: cannot open %s\n", argv0, mdata);
	    exit (1);
	}

        id = proc_id = -1;
	s = process;
	while (*s && *s >= '0' && *s <= '9') ++s;
	if (!*s) proc_id = atoi (process);

        while (fscanf (fp_proclist, "%s", str) > 0) {

	    if (str[0] != '#') {
		id = atoi (str);
		if (proc_id >= 0) {
		    if (id == proc_id) break;
		}
		else {
		    fscanf (fp_proclist, "%s", str);
		    if (strsame (str, process) && id >= 0) break;
		}
	    }

	    while (getc (fp_proclist) != '\n');
	    id = -1;
	}

	fclose (fp_proclist);

	if (id < 0) {
	    DIR *dp;
	    if ((dp = opendir (process))) {
		/* 'process' is used as a process directory */
		closedir (dp);
procpath:
		sprintf (mdata, "%s/maskdata", process);
		fclose (cfopen (mdata, "r"));
		procdata = (DM_PROCDATA *)_dmDoGetProcessFile (mdata);
	    }
	    else {
		fprintf (stderr, "%s: process %s not found in %s\n", argv0, process, mdata);
		exit (1);
	    }
	}
	else procdata = (DM_PROCDATA *)_dmDoGetProcess (id, NULL);
    }
    else if (maskdata) {
	if (!*maskdata) {
	    fprintf (stderr, "%s: illegal maskdatafile name\n", argv0);
	    exit (1);
	}
	fclose (cfopen (maskdata, "r"));
	procdata = (DM_PROCDATA *)_dmDoGetProcessFile (maskdata);
    }

    if (!procdata) exit (1);

    maxprocmasks = procdata -> nomasks;

    ALLOC (subdata, 1, struct submasks);
    subdata -> nomasks = 0;
    subdata -> mask_name = NULL;

    ALLOC (maskdrawcolor, procdata -> nomasks, char *);

    if (!t_flag) {
	outfile = strsave (techfile);
	outfile[--l] = 't';
	while (--l >= 0) if (outfile[l] == '\\') outfile[l] = '/';
    }

    doparse (techfile);
    set_profile_subcont ();

    if (tclExportPath) {
	FILE *fp = fopen (tclExportPath, "w");
	if (!fp) {
	    fprintf (stderr, "fatal: Unable to open file `%s' for writing.\n", tclExportPath);
	    die ();
	}
	exportToTcl (fp);
	fclose (fp);
	dmQuit ();
	return (0);
    }

    addNewMasks ();

    if (maxNbrKeys == -1) maxNbrKeys = 12; /* use default */

    selectKeys ();

    changeMasks ();

    checkConnections ();

    mkCapBitmasks ();

    findConducTransf ();

    ALLOC (keytab, nbrKeySlots, struct elemRef *);
    ALLOC (keytab2, nbrKeySlots2, struct elemRef *);

    mkKeys ();

    prTabs (outfile);

    if (t_flag) {
	dmQuit ();
	return (0);
    }

    if (diel_cnt > 3) {
	fprintf(stderr, "warning: More than 3 dielectrics found, you should use unigreen (not supported).\n");
    }
    if (substr_cnt >= 2) {
	int cnt = 2;
	if (!strcmp(substrs[substr_cnt-1].name, "metalization")) cnt = 3;
	if (substr_cnt > cnt) {
	    fprintf(stderr, "warning: More than %d sublayers found%s, you should use unigreen (not supported).\n",
		cnt, cnt == 3 ? " (with metalization)" : "");
	}
    }

    if (prVerbose) {
	fprintf (stderr, "%s: outfile is '%s'\n", argv0, outfile);
	warningMes ("outfile cannot be used by space! (because -V option was used)\n", 0);
    }

    dmQuit ();
    return (0);
}

void fatalErr (const char *s1, const char *s2)
{
    if (notFatal) { warningMes (s1, s2); return; }
    fprintf (stderr, "error: ");
    if (s1 && *s1) fprintf (stderr, "%s", s1);
    if (s2 && *s2) fprintf (stderr, " %s", s2);
    fprintf (stderr, "\n");
    die ();
}

void warningMes (const char *s1, const char *s2)
{
    fprintf (stderr, "warning: ");
    if (s1 && *s1) fprintf (stderr, "%s", s1);
    if (s2 && *s2) fprintf (stderr, " %s", s2);
    fprintf (stderr, "\n");
}

void dmError (char *s)
{
    if (!icdprocess) {
	dmPerror (s);
	die ();
    }
}

void die ()
{
    if (outfile) {
	if (v_flag) fprintf (stderr, "%s: removing outfile '%s'\n", argv0, outfile);
	unlink (outfile);
    }
    dmQuit ();
    exit (1);
}

void extraSay ()
{
}
