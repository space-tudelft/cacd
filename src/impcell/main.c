/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
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

#include "src/libddm/dmstd.h"
#include "signal.h"

#define PE fprintf(stderr,
#define BUFLEN DM_MAXNAME+DM_MAXNAME+MAXLINE+20

void build_celllist (void);
int impcell (char *loc_name, char *rem_name, char *lib_path, int view_entry);
int append_tree (char *name, int nr);
void is_legal_lib (char *lib_path);
void usage (void);
void die (int status);

struct impcell_tree  {
    char   a_name[DM_MAXNAME + 1];
    struct impcell_tree *ht_next;
};

struct stat st_buf;

DM_PROCDATA *mdata;
DM_XDATA xdata;
int oldproj;
int write_xdata;

#define HASHSIZE 128
struct impcell_tree *Hashtab[DM_NOVIEWS][HASHSIZE];
FILE *icl_fp[DM_NOVIEWS];

DM_PROJECT *cp_key;
DM_PROJECT *lib_key;
char *projlist = "./projlist";
int view_flag[DM_NOVIEWS] = { 0, 0, 0 };
char *view_buf[DM_NOVIEWS] = { NULL, NULL, NULL };
char *argumentlist[3] = { NULL, NULL, NULL };
char ** viewlist = NULL;
char  Buf0[BUFLEN];
char  Buf1[BUFLEN];
char  Buf2[BUFLEN];
char  Buf3[BUFLEN];
int   vflag;
int   A_opt = 0;
int   I_opt = 0;
int   R_opt = 0;
char *prefix = NULL;

char *argv0 = "impcell";	/* program name */
char *use_msg1 =			/* command line */
"\nUsage: %s -c|f|l -a|i|r [-p prefix] project";
char *use_msg2 =			/* command line */
"\n   or: %s -c|f|l project remote [local]\n\n";

int main (int argc, char *argv[])
{
    char *lib_path = 0;
    char *loc_name = 0;
    char *rem_name = 0;
    char *cp;
    int  arg;
    int  ok = 0;
    char *view;
    char **clp;
    int  view_entry;

    if (argc < 2) {
	PE "%s: illegal number of arguments specified\n", argv0);
	usage ();
    }

    for (arg = 1; arg < argc; arg++) {
	if (argv[arg][0] == '-') {
	    for (cp = &argv[arg][1]; *cp; ++cp) {
		switch (*cp) {
		case 'a':
		    A_opt = 1;
		    break;
		case 'i':
		    A_opt = I_opt = 1;
		    break;
		case 'r':
		    A_opt = R_opt = 1;
		    break;
		case 'p':
		    if (arg+1 < argc) {
			if (dmTestname (prefix = argv[arg+1])) {
			    PE "%s: prefix '%s' not allowed\n", argv0, prefix);
			    exit (1);
			}
			A_opt = 1;
		    }
		    break;
		case 'l':
		    view_buf[0] = LAYOUT;
		    ok = 1;
		    break;
		case 'c':
		    view_buf[1] = CIRCUIT;
		    ok = 1;
		    break;
		case 'f':
		    view_buf[2] = FLOORPLAN;
		    ok = 1;
		    break;
		default:
		    usage ();
		}
	    }
	}
    }

    if (!ok) {
	PE "%s: no view specified\n", argv0);
	usage ();
    }


    ok = 0;
    for (arg = 1; arg < argc; arg++) {
	if (argv[arg][0] != '-') {
	    if (A_opt) {
		if (!prefix || prefix != argv[arg]) {
		    if (ok != 2) {
			lib_path = argv[arg];
			ok = 2;
		    }
		    else {
			PE "%s: warning: extra argument '%s' ignored\n",
			    argv0, argv[arg]);
		    }
		}
	    }
	    else if (argc != 2) {
		switch (ok) {
		case 0:
		    lib_path = argv[arg];
		    ok = 1;
		    break;
		case 1:
		    loc_name = rem_name = argv[arg];
		    ok = 2;
		    break;
		case 2:
		    loc_name = argv[arg];
		    ok = 3;
		    break;
		default:
		    PE "%s: warning: extra argument '%s' ignored\n",
			argv0, argv[arg]);
		}
	    }
	}
    }

    if (ok < 2) {
	if (ok == 1) PE "%s: no remote cell name specified\n", argv0);
	else PE "%s: no library project specified\n", argv0);
	usage ();
    }

    /* ignore all signals */
#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, SIG_IGN);
    signal (SIGINT, SIG_IGN);

    if (dmInit (argv0)) exit (1);

    if ((cp_key = dmOpenProject (".", PROJ_WRITE)) == NULL) {
	PE "%s: the cwd is no valid project\n", argv0);
	die (1);
    }

    if (!view_buf[0] && !view_buf[1])
	oldproj = 1;
    else
	oldproj = dmStatXData (cp_key, &st_buf);
    if (!oldproj) {
	mdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, cp_key);
	if (!mdata) die (1);
    }

    build_celllist ();

    if (!lib_path && !A_opt) {
	if (isatty (0)) {
	    ++vflag;
	    PE "%s: -- reading from stdin --\n", argv0);
	}
	/* get lines in impcelllist format: alias, cellname, project-path */
	while (fgets (Buf0, BUFLEN, stdin)) {
	    if (sscanf (Buf0, "%s%s%s", Buf1, Buf2, Buf3) != 3) {
		PE "%s: stdin: line contains incorrect number of strings\n",
		    argv0);
		die (1);
	    }
	    for (view_entry = 0; view_entry < DM_NOVIEWS; ++view_entry) {
		if (!view_buf[view_entry]) continue;

		impcell (Buf1, Buf2, Buf3, view_entry);
	    }
	}
    }
    else if (A_opt) {	/* import all cells from library */
	if (I_opt) ++vflag;

	dmerrno = 0;

	if ((lib_key = dmOpenProject (lib_path, PROJ_READ)) == NULL) {
	    PE "%s: path '%s' is no valid project\n", argv0, lib_path);
	    die (1);
	}

	if (lib_key == cp_key) {
	    PE "%s: path '%s' is the current project\n", argv0, lib_path);
	    die (1);
	}

	lib_path = lib_key -> dmpath;

	if (dmerrno != DME_PRLOCK) {
	    is_legal_lib (lib_path); /* (exits if not legal) */
	}

	write_xdata = oldproj? 0 : dmStatXData (lib_key, &st_buf);

	for (view_entry = 0; view_entry < DM_NOVIEWS; ++view_entry) {
	    if (!(view = view_buf[view_entry])) continue;

	    if (I_opt) PE "view: %s\n", view);

	    /* read library celllist */
	    if (!(clp = lib_key -> celllist[view_entry] = (char **) dmGetMetaDesignData (CELLLIST, lib_key, view))) {
		PE "%s: error: cannot read file '%s/%s/celllist'\n", argv0, lib_path, view);
		die (1);
	    }

	    /* for all entries in the library celllist */
	    for (; *clp != NULL; clp++) {
		if (R_opt && (int) _dmCellIsRoot2 (lib_key, *clp, view, lib_key -> celllist[view_entry]) != 1) continue;

		_dmSprintf (Buf0, "%s%s", (prefix ? prefix : ""), *clp);
		loc_name = Buf0;

		if (!I_opt) {
		    impcell (loc_name, *clp, lib_path, view_entry);
		}
		else {	/* I_opt: interactive */
		    ok = 0;
		    while (!ok) {
			/* import this cell ? */
			PE "%s [n]? ", *clp);
			if (fgets (Buf1, BUFLEN, stdin)) {
			    switch (*Buf1) {
			    case 'y':
				ok = 2;
				break;
			    case '\n':
			    case 'n':
				ok = 1;
				break;
			    case 'q':
				die (0);
				break;
			    default:
				Buf1[strlen (Buf1) - 1] = '\0';
				PE "%s: answer '%s' incorrect, type 'y', 'n' or 'q'\n", argv0, Buf1);
				break;
			    }
			}
			else die (1);

			/* get alias name */
			while (ok == 2) {
			    PE "local name [%s]: ", loc_name);
			    if (fgets (Buf2, BUFLEN, stdin)) {
				Buf2[strlen (Buf2) - 1] = '\0';
				if (*Buf2) {
				    if (dmTestname (Buf2)) *loc_name = '\0';
				    else loc_name = Buf2;
				}
				if (*loc_name && impcell (loc_name, *clp, lib_path, view_entry)) {
				    /* cell imported */
				    ok = 3;
				}
				else { /* impcell failed */
				    if (*Buf0 || *Buf2) {
					*Buf0 = '\0';
					loc_name = Buf0;
				    }
				    else { /* skip this cell after all */
					PE "%s: warning: cell '%s' skipped\n", argv0, *clp);
					ok = 4;
				    }
				}
			    }
			    else die (1);
			}
		    }
		}
	    }
	}
    }
    else {
	for (view_entry = 0; view_entry < DM_NOVIEWS; ++view_entry) {
	    if (!view_buf[view_entry]) continue;

	    impcell (loc_name, rem_name, lib_path, view_entry);
	}
    }

    die (0);
    return (0);
}

void build_celllist ()
{
    FILE *fp;
    char *view;
    int view_entry;

    for (view_entry = 0; view_entry < DM_NOVIEWS; ++view_entry) {
	if (!(view = view_buf[view_entry])) continue;

	if (!(cp_key -> celllist[view_entry] = (char **) dmGetMetaDesignData (CELLLIST, cp_key, view))) {
	    PE "%s: error: cannot read file './%s/celllist'\n", argv0, view);
	    die (1);
	}

	_dmSprintf (Buf0, "./%s/impcelllist", view);
	if (!(fp = fopen (Buf0, "r+"))) {
	    PE "%s: error: cannot read file '%s'\n", argv0, Buf0);
	    die (1);
	}

	/* format: alias, cellname, project-path */
	while (fscanf (fp, "%s%*s%*s", Buf0) != EOF) {
	    append_tree (Buf0, view_entry);
	}
	icl_fp [view_entry] = fp;
    }
}

void read_lib_status (char *rem_name)
{
    char *msks = 0;
    int macro = 0;

    _dmSprintf (Buf3, "%s/layout/%s/is_macro", lib_key -> dmpath, rem_name);
    if (stat (Buf3, &st_buf) == 0) {
	FILE *fp = fopen (Buf3, "r");
	if (fp) {
	    msks = Buf0;
	    *msks = 0;
	    (void) fscanf (fp, "%d %s", &macro, msks);
	    fclose (fp);
	}
	else
	    PE "%s: warning: can't read '%s'\n", argv0, Buf3);
    }

    if (macro) {
	xdata.celltype = DM_CT_MACRO;
	if (*msks) {
	    char *mask;
	    int i;
	    for (i = 0; i < mdata->nomasks; ++i) mdata->mask_no[i] = 0;

	    while ((mask = strtok (msks, "+"))) {
		msks = NULL;
		for (i = 0; i < mdata->nomasks; ++i) {
		    if (strcmp (mask, mdata->mask_name[i]) == 0) {
			mdata->mask_no[i] = 1;
			break;
		    }
		}
		if (i == mdata->nomasks)
		    PE "%s: warning: skipping mask '%s' in '%s'\n", argv0, mask, Buf3);
	    }

	    xdata.interfacetype = DM_IF_FREEMASKS;
	    xdata.masks = msks = Buf0;
	    *msks = 0;
	    for (i = 0; i < mdata->nomasks; ++i) {
		if (mdata->mask_no[i]) {
		    sprintf (msks, " %d", i);
		    msks += strlen (msks);
		}
	    }
	}
    }

    _dmSprintf (Buf3, "%s/circuit/%s/devmod", lib_key -> dmpath, rem_name);
    if (stat (Buf3, &st_buf) == 0) xdata.celltype = DM_CT_DEVICE;
}

int impcell (char *loc_name, char *rem_name, char *lib_path, int view_entry)
{
    FILE *fp;
    char *view;
    register char **clp;

    if (strlen (lib_path) >= MAXLINE) {
	PE "%s: too long project path name\n", argv0);
	if (vflag) return (0);
	else die (1);
    }

    if (dmTestname (rem_name)) {
	PE "%s: remote cell name '%s' illegal\n", argv0, rem_name);
	if (vflag) return (0);
	else die (1);
    }

    if (dmTestname (loc_name)) {
	PE "%s: local cell name '%s' illegal\n", argv0, loc_name);
	if (vflag) return (0);
	else die (1);
    }

    view = view_buf[view_entry];
    dmerrno = 0;

    if (!A_opt) {
	if ((lib_key = dmOpenProject (lib_path, PROJ_READ)) == NULL) {
	    PE "%s: '%s' is no valid project\n", argv0, lib_path);
	    if (vflag) return (0);
	    else die (1);
	}

	if (lib_key == cp_key) {
	    PE "%s: '%s' is the current project\n", argv0, lib_path);
	    if (vflag) return (0);
	    else die (1);
	}

	lib_path = lib_key -> dmpath;

	if (dmerrno != DME_PRLOCK) {
	    is_legal_lib (lib_path); /* (exits if not legal) */
	}

	if (!(clp = lib_key -> celllist[view_entry]))
	if (!(clp = lib_key -> celllist[view_entry] = (char **) dmGetMetaDesignData (CELLLIST, lib_key, view))) {
	    PE "%s: error: cannot read file '%s/%s/celllist'\n", argv0, lib_path, view);
	    die (1);
	}

	while (*clp && strcmp (*clp, rem_name)) ++clp;
	if (!*clp) {
	    PE "%s: cell '%s' not found in '%s/%s/celllist'\n", argv0, rem_name, lib_path, view);
	    PE "%s: warning: '%s' not added to view '%s'\n", argv0, loc_name, view);
	    return (0);
	}
    }

    clp = cp_key -> celllist[view_entry];

    while (*clp && strcmp (*clp, loc_name)) ++clp;
    if (*clp) {
	PE "%s: cell '%s' found in './%s/celllist'\n", argv0, loc_name, view);
	PE "%s: warning: '%s' not added to view '%s'\n", argv0, loc_name, view);
	return (0);
    }

    if (append_tree (loc_name, view_entry)) {
	PE "%s: cell '%s' already imported in view '%s'\n", argv0, loc_name, view);
	return (0);
    }

    fp = icl_fp[view_entry];
    fseek (fp, 0L, 2);
    fprintf (fp, "%s %s %s\n", loc_name, rem_name, lib_path);

if (view_entry < 2) { /* Not the floorplan view */
    if (!A_opt)
	write_xdata = oldproj? 0 : dmStatXData (lib_key, &st_buf);
    if (write_xdata) {
	xdata.name = loc_name;
	if (dmGetCellStatus (cp_key, &xdata) == 1) { /* not found */
	    read_lib_status (rem_name);
	    (void) dmPutCellStatus (cp_key, &xdata);
	}
    }
}

    return (1);
}

int append_tree (char *name, int nr)
{
    register char *s;
    register struct impcell_tree *ptr;
    register int  hashval;

    s = name;
    hashval = 0;
    while (*s) hashval += *s++;
    hashval %= HASHSIZE;

    for (ptr = Hashtab[nr][hashval]; ptr; ptr = ptr -> ht_next)
	if (strcmp (name, ptr -> a_name) == 0) return (1); /* found */

    /*
    ** allocate new hash-table element
    */
    ptr = (struct impcell_tree *) malloc (sizeof (struct impcell_tree));
    if (!ptr) {
	PE "%s: error: cannot malloc\n", argv0);
	die (1);
    }
    strcpy (ptr -> a_name, name);
    ptr -> ht_next = Hashtab[nr][hashval];
    Hashtab[nr][hashval] = ptr;
    return (0); /* not found */
}

void is_legal_lib (char *lib_path)
{
    FILE *fp;
    int found;

    if (!(fp = fopen (projlist, "r"))) {
	PE "%s: error: cannot read file '%s'\n", argv0, projlist);
	die (1);
    }

    found = 0;
    while (fgets (Buf0, BUFLEN, fp)) {
	Buf0[strlen (Buf0) - 1] = '\0';
	if (strcmp (Buf0, lib_path) == 0) { ++found; break; }
    }
    fclose (fp);

    if (!found) {
	PE "%s: path '%s' not found in 'projlist'\n", argv0, lib_path);
	die (1);
    }
}

void usage ()
{
    PE use_msg1, argv0);
    PE use_msg2, argv0);
    exit (1);
}

void dmError (char *s)
{
    PE "%s: ", argv0);
    dmPerror (s);
}

void die (int status)
{
    dmQuit ();
    if (status) PE "%s: -- program aborted --\n", argv0);
    else if (vflag) PE "%s: -- program finished --\n", argv0);
    exit (status);
}
