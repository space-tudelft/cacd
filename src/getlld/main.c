/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	S. de Graaf
 *	T.G.R. van Leuken
 *	P. Kist
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
#include "src/getlld/incl.h"

#define BUFLEN	80
#define HASHSIZE 128

struct mo_elmt *Hashold[HASHSIZE];
struct mo_elmt *Hashnew[HASHSIZE];

struct stat stat_buf;		/* file status buffer */
struct mo_elmt *molist = 0;
struct ic_elmt *iclist = 0;
IMPCELL    **IClist;		/* Imported Cell list */
FILE        *fp_file;
DM_CELL     *ckey;
DM_PROJECT  *dmproject;
char    outputfile[DM_MAXNAME + 6];
char   *bmlist;
char    buf[BUFLEN];
char    buf1[DM_MAXLAY + 1];
char    buf2[DM_MAXLAY + 1];
int     layset[DM_MAXNOMASKS];
char   *lay[DM_MAXNOMASKS];
char  **ldmlay;

static int addmode = 1;
static int checkout = 0;
static int level  = 1; /* current level */

static void get_alias (char *);
static void pr_aliases (struct ic_elmt *);
static void trav_mctree (struct mo_elmt *);
void sig_handler (int sig);

int	new_names = 0;
int     nolays;
int     i_mode = 0;
int     x_mode = 0;
int     r_mode = 0;
int     t_mode = 0;
int     u_mode = 0;
int     ut_mode = 0;
int     v_mode = 0;
int     usage = 0;
int     Pmode = LDM;
double  resol;

char   *skipCellName = "Error_Marker";

char   *begC = ":: "; /* begin of comment line */
char   *endC = "\n";  /*  end  of comment line */

#ifdef XLDM
char   *argv0 = "xldm"; /* program name */
#else
#ifdef XCIF
char   *argv0 = "xcif"; /* program name */
#else
#ifdef XCMK
char   *argv0 = "xcmk"; /* program name */
#else
char   *argv0 = "getlld"; /* program name */
#endif
#endif
#endif

char   *pVnr  = "4.19";	/* program version number */
char   *pDate = "17 Mar 2008";	/* program date */

int main (int argc, char *argv[])
{
    struct tm *ptm;
    register int    i, j, iarg;
    register char  *s;
    register struct mo_elmt *p;
    DM_PROCDATA *process;	/* ptr to process info */
    FILE   *fp_bml;
    time_t  tloc;
    int     c;
    int     f_mode = 0;
    int     help = 0;
    char   *suffix;
    char   *topcell;

#ifdef XLDM
	Pmode = LDM;
#else
#ifdef XCIF
	Pmode = CIF;
	begC = "(";
	endC = ");\n";
#else
#ifdef XCMK
	Pmode = CMK;
#else
    if ((argv0 = strrchr (argv[0], '/'))) ++argv0;
    else argv0 = argv[0];
#ifdef WIN32
    if ((s = strrchr (argv0, '\\'))) argv0 = s+1;
    if ((s = strchr (argv0, '.'))) *s = 0;
#endif

    if (strcmp (argv0, "xldm") == 0) { /* process as xldm */
	Pmode = LDM;
    }
    else if (strcmp (argv0, "xcif") == 0) { /* process as xcif */
	Pmode = CIF;
	begC = "(";
	endC = ");\n";
    }
    else if (strcmp (argv0, "xcmk") == 0) { /* process as xcmk */
	Pmode = CMK;
    }
    else {
	PE "To use this program, the program must be installed\n");
	PE "as: \"xldm\" or \"xcmk\" or \"xcif\".\n");
	exit (1);
    }
#endif
#endif
#endif

    suffix = argv0 + 1;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	j = iarg;
	for (i = 1; argv[j][i] != '\0'; ++i) {
	    switch (argv[j][i]) {
		case 'f':
		    f_mode = ++iarg;
		    break;
		case 'h':
		    ++help;
		    break;
		case 'i':
		    ++i_mode;
		    break;
		case 'm':
		    if (Pmode <= CIF) goto unknown;
		    bmlist = argv[++iarg];
		    break;
		case 'x':
		    if (Pmode >= CMK) goto unknown;
		    ++x_mode;
		    break;
		case 'r':
		    ++r_mode;
		    break;
		case 't':
		    if (Pmode <= CIF) goto unknown;
		    t_mode = 0;
		    while ((c = argv[j][++i])) {
			if (c >= '0' && c <= '9') {
			    t_mode = 10 * t_mode + (c - '0');
			}
			else break;
		    }
		    if (!t_mode) ++t_mode;
		    ++ut_mode;
		    --i;
		    break;
		case 'u':
		    if (Pmode <= CIF) goto unknown;
		    ++u_mode;
		    ++ut_mode;
		    break;
		case 'v':
		    ++v_mode;
		    break;
		default:
unknown:
		    ++usage;
		    PE "%s: -%c: unknown option\n", argv0, argv[j][i]);
	    }
	}
    }

    if (help) goto helpme;

    if (f_mode) {
	if (*argv[f_mode])
	    PE "%s: -f: output to file: %s\n", argv0, argv[f_mode]);
	else
	    PE "%s: -f: output to stdout\n", argv0);
    }
    if (bmlist) {
	if (*bmlist)
	    PE "%s: -m: using basic mask list: %s\n", argv0, bmlist);
	else
	    PE "%s: -m: using no basic mask list\n", argv0);
    }
    if (r_mode) {
	PE "%s: -r: only root of cell\n", argv0);
	i_mode = 0;
    }
    if (i_mode) {
	PE "%s: -i: extract imported cells\n", argv0);
	++ut_mode;
    }

    if (t_mode) {
	if (t_mode > 3) {
	    PE "%s: -t: cell names are truncated to %d chars\n",
		argv0, t_mode);
	}
	else {
	    ++usage;
	    PE "%s: -tN: no truncation (N must be >= 4)\n", argv0);
	}
    }
    if (u_mode) PE "%s: -u: uppercase mode\n", argv0);
    if (v_mode) PE "%s: -v: verbose mode\n", argv0);
    if (x_mode) PE "%s: -x: cell origin mode\n", argv0);

    if (argc <= iarg) {
	PE "%s: no cell name given\n", argv0);
	++usage;
    }
    if (argc > iarg + 1) {
	PE "%s: too many arguments given\n", argv0);
	++usage;
    }
    if (usage) {
helpme:
        PE "\n%s %s (%s)\n", argv0, pVnr, pDate);
        PE "\nUsage: %s [-%s] [-f ofile] cell\n\n", argv0,
	    (Pmode <= CIF) ? "hirvx" : "m mlist] [-hiurv] [-tN");
	if (help) {
	    PE "-h   this help menu\n");
	    PE "-f   use next arg. as output filename\n");
	    PE "-i   extract also the imported cells\n");
	  if (Pmode > CIF) {
	    PE "-m   use next arg. as masklist file\n");
	    PE "-tN  truncate cell names to N (>= 4)\n");
	    PE "-u   uppercase mode\n");
	  }
	    PE "-r   only the root of the cell\n");
	    PE "-v   verbose mode\n");
	  if (Pmode <= CIF)
	    PE "-x   cell origin mode\n");
	}
	else {
	    PE "      (use option -h for help)\n");
	}
	PE "\n");
	exit (1);
    }

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN);	/* ignore hangup signal */
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
    signal (SIGTERM, sig_handler);

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);

    topcell = argv[iarg];
    if (f_mode) {
	strcpy (outputfile, argv[f_mode]);
    }
    else {
	sprintf (outputfile, "%s.%s", topcell, suffix);
    }

    if (*outputfile)
	if (stat (outputfile, &stat_buf) != -1)
	    error (5, outputfile);

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);
    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    ldmlay = process -> mask_name;
    nolays = process -> nomasks;

    gmc.imported = 0;
    p = findcell (dmproject, topcell);
    p -> key = dmCheckOut (dmproject, topcell, WORKING, DONTCARE, LAYOUT, READONLY);
    p -> prev = (struct mo_elmt *) 0;
    p -> next = (struct mo_elmt *) 0;
    p -> mcalls = 1;
    molist = p;

    if (Pmode >= CMK) { /* CMK */
	if (!bmlist) {
	    if (Pmode == CMK) {
		bmlist = (char *) dmGetMetaDesignData (PROCPATH,
		    dmproject, "bmlist.cmk");
		if (stat (bmlist, &stat_buf) == -1) {
		    PE "%s: warning: cannot read file: %s\n",
			argv0, bmlist);
		    bmlist = "";
		}
	    }
	    else bmlist = "";
	}

	if (*bmlist) {
	    if (!(fp_bml = fopen (bmlist, "r"))) error (11, bmlist);

	    while (fgets (buf, BUFLEN, fp_bml)) {
		if (*buf == '#') continue;
		if (sscanf (buf, "%s%s", buf1, buf2) != 2)
		    error (14, bmlist);
		for (i = 0; i < nolays; ++i) {
		    if (strcmp (buf2, ldmlay[i]) == 0) break;
		}
		if (i >= nolays) error (15, buf2);
		if (layset[i]) error (16, buf2);
		for (j = 0; j < nolays; ++j) {
		    if (layset[j])
			if (strcmp (buf1, lay[j]) == 0) error (16, buf1);
		}
		layset[i] = 1;
		lay[i] = strsave (buf1, strlen (buf1));
	    }

	    fclose (fp_bml);
	}

	for (i = 0; i < nolays; ++i) {
	    if (!layset[i]) {
		for (j = 0; j < nolays; ++j) {
		    if (layset[j])
			if (strcmp (ldmlay[i], lay[j]) == 0)
			    error (13, ldmlay[i]);
		}
		layset[i] = 1;
		lay[i] = strsave (ldmlay[i], strlen (ldmlay[i]));
	    }
	}
    }

    if (*outputfile) {
	if (!(fp_file = fopen (outputfile, "w")))
	    error (0, outputfile);
    }
    else
	fp_file = stdout;

    resol = dmproject -> lambda;

    switch (Pmode) {
    case LDM:
	PF "%sDelft Layout Description Modified, LDM V3.2%s", begC, endC);
	break;
    case CIF:
	PF "%sCaltech Intermediate Form, CIF V2.0%s", begC, endC);
	break;
    case CMK:
	PF "%sPhilips CIRCUITMASK (CMK) 1978 AUG%s", begC, endC);
	break;
    }

    tloc = time (NULL);
    ptm = localtime (&tloc);
    PF "%sGenerated with the program x%s V%s%s", begC, suffix, pVnr, endC);
    PF "%sat %.24s%s", begC, asctime (ptm), endC);
    PF "%sby the Delft NELSIS IC Design System Release 3%s", begC, endC);

    if (Pmode <= CIF) { /* LDM || CIF */
	PF "%s%sorigin mode %s%s",
		begC,
		x_mode ? "" : "no-",
		Pmode == CIF ? "CIF" : "LDM",
		endC);
    }

    PF "%sproject = %s%s", begC, dmproject -> dmpath, endC);
    PF "%sprocess = %s%s", begC, process -> pr_name, endC);

    if (Pmode <= CIF) /* LDM || CIF */
	PF "%slambda = %g micron%s", begC, dmproject -> lambda, endC);
    if (Pmode == CIF) {
	PF "%sthe parameter unit is in centi micron%s", begC, endC);
	for (i = 0; i < nolays; ++i) {
	    lay[i] = strsave (ldmlay[i], strlen (ldmlay[i]));
	    for (s = lay[i]; *s; ++s)
		if (islower ((int)*s)) *s -= 32; /* UPPER */
	}
    }

    if (Pmode == CMK) {
	i = 0;
	PF "%s   %s", begC, ldmlay[i]);
	while (++i < nolays) PF ",%s", ldmlay[i]);
	PF "    converted to ...%s", endC);
	i = -1;
	suffix = u_mode ? "MASKS " : "masks ";
	while (++i < nolays) {
	    if (u_mode)
		for (s = lay[i]; *s; ++s)
		    if (islower ((int)*s)) *s -= 32; /* UPPER */
	    PF "%s%s", (i == 0 ? suffix : ","), lay[i]);
	}
	PF "\n");
    }

    PF "%sextraction of ", begC);
    if (r_mode) {
	PF "only the root of cell: %s%s", topcell, endC);
    }
    else {
	PF "all related cells of cell: %s%s", topcell, endC);
	checkout = 1;
    }
    for (p = molist; p; p = p -> prev)
	if (p -> mcalls) trav_mctree (p);
    addmode = 0;

    if (t_mode)
	PF "%scell names are truncated to %d chars%s", begC, t_mode, endC);
    if (u_mode)
	PF "%suppercase mode is used%s", begC, endC);

    outp_lld ();

    if (new_names) {
	PF "%slist of changed cell names%s", begC, endC);
	PF "%s%-14s old_name:%s", begC, "new_name:", endC);
	for (j = 0; j < HASHSIZE; ++j) {
	    for (p = Hashold[j]; p; p = p -> onext)
	    if (p -> nname != p -> oname) {
		PF "%s%-14s %s", begC, p -> nname, p -> oname);
		if (p -> pkey == dmproject)
		    PF "%s", endC);
		else
		    PF " %s%s", p -> pkey -> dmpath, endC);
	    }
	}
    }

    if (iclist) {
	IClist = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
	PF "%slist of imported cells%s", begC, endC);
	PF "%s%-14s %-14s lib_project_path:%s", begC, "alias:", "rem_name:", endC);
	pr_aliases (iclist);
    }

    if (Pmode == CIF)
	PF "E\n");
    else
	PF "%seof%s", begC, endC);

    if (*outputfile) fclose (fp_file);
    die (0);
    return (0);
}

static void pr_aliases (struct ic_elmt *p)
{
    if (p) {
	pr_aliases (p -> l);
	get_alias (p -> mo);
	pr_aliases (p -> r);
    }
}

static void get_alias (char *mo)
{
    register IMPCELL **q = IClist;
    register IMPCELL  *ic;

    while ((ic = *q++)) {
	if (strcmp (ic -> alias, mo) == 0) {
	    PF "%s%-14s %-14s %s%s",
		begC, mo, ic -> cellname, ic -> dmpath, endC);
	    return;
	}
    }
    error (1, mo);
}

static void trav_mctree (struct mo_elmt *p)
{
    DM_STREAM *stream;
    struct mo_elmt *q;

    level = p -> level + 1;
    stream = dmOpenStream (p -> key, "mc", "r");
    q = (struct mo_elmt *) 0;

    while (dmGetDesignData (stream, GEO_MC) > 0) {
	q = findcell (p -> pkey, gmc.cell_name);
	if (q -> key && q -> level < level) {
	    if (q == p) error (12, "cell calls itself");
	    if (!q -> next) error (12, "cell calls topcell");
	    if (level > 1000) error (12, "level > 1000");
	    q -> level = level;
	    if (q -> prev) {
		q -> next -> prev = q -> prev;
		q -> prev -> next = q -> next;
		q -> prev = (struct mo_elmt *) 0;
		q -> next = molist;
		molist -> prev = q;
		molist = q;
	    }
	}
    }

    dmCloseStream (stream, COMPLETE);
    if (!q) p -> mcalls = 0;
}

void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    PE "\n");
    error (7, "");
}

void dmError (char *s)
{
    PE "%s: ", argv0);
    dmPerror (s);
    error (8, "");
}

char *errlist[] = {
     /* 0 */ "cannot create file: %s",
     /* 1 */ "%s: imported cell name not found",
     /* 2 */ "read error in nor-file of cell: %s",
     /* 3 */ "%s: no more core",
     /* 4 */ "%s: cell name not found",
     /* 5 */ "%s: file does exist (not overwritten)",
     /* 6 */ "unknown element in nor-file of cell: %s",
     /* 7 */ "received interrupt signal",
     /* 8 */ "error in DMI function",
     /* 9 */ "illegal # of xy-pairs in nor-file of cell: %s",
     /* 10 */ "empty celllist",
     /* 11 */ "cannot read file",
     /* 12 */ "cell recursion (%s)",
     /* 13 */ "cannot make cross reference for mask: %s (already used)",
     /* 14 */ "read error in file: %s",
     /* 15 */ "read error in bmlist-file, unknown mask: %s",
     /* 16 */ "read error in bmlist-file, already used mask: %s",
     /* 17 */ "too many xy-pairs in nor-file of cell: %s",
     /* 18 */ "sfx != sfy in mc-file of cell: %s",
     /* 19 */ "unknown error"
};

void error (int nr, char *s)
{
    if (nr < 0 || nr > 19) nr = 19;
    PE "%s: ", argv0);
    PE errlist[nr], s);
    PE "\n");
    die (1);
}

void die (int status)
{
    dmQuit ();
    if (status) {
	if (*outputfile && fp_file) unlink (outputfile);
	PE "\n%s: -- program aborted --\n", argv0);
    }
    exit (status);
}

struct mo_elmt *findcell (DM_PROJECT *proj, char *name)
{
    register char *o, *s;
    register struct mo_elmt *p, *q, *qp;
    char   *newname;
    char   *oldname;
    int     hashold;
    int     hashnew;
    int     oldlen;
    static char *cn = "X00/";

    if (gmc.imported && i_mode) {
	proj = dmFindProjKey (IMPORTED, name, proj, &oldname, LAYOUT);
    }
    else oldname = name;

    s = oldname;
    hashold = 0;
    while (*s) hashold += *s++;
    hashold %= HASHSIZE;

    for (p = Hashold[hashold]; p; p = p -> onext)
	if (p -> pkey == proj && !strcmp (p -> oname, oldname)) return (p);

    if (!addmode) {
	error (gmc.imported ? 1 : 4, name);
	return (p);
    }

    ALLOC (p, struct mo_elmt);
    oldlen = s - oldname;
    p -> oname = oldname = strsave (oldname, oldlen);
    p -> nname = oldname;
    if (gmc.imported && !i_mode) {
	p -> key = (DM_CELL *) 0;
	if (iclist)
	    inst_alias (iclist, oldname);
	else
	    iclist = inst_ic_elmt (oldname);
    }
    else if (checkout) {
	p -> key = dmCheckOut (proj, oldname,
			ACTUAL, DONTCARE, LAYOUT, READONLY);
	p -> prev = (struct mo_elmt *) 0;
	molist -> prev = p;
	p -> next = molist;
	molist = p;
	p -> mcalls = 1;
    }
    else
	p -> key = (DM_CELL *) 0;
    p -> pkey = proj;
    p -> level = level;
    p -> onext = Hashold[hashold];
    Hashold[hashold] = p;

    if (!ut_mode) return (p);

    newname = oldname;
    hashnew = hashold;

    if (t_mode && oldlen > t_mode) {
	if (!(newname = malloc (t_mode + 1))) error (3, "char");
	o = oldname;
	s = newname;
	hashnew = 0;
	while ((s - newname) < t_mode) {
	    *s = *o++;
	    if (u_mode && islower ((int)*s)) *s -= 32; /* UPPER */
	    hashnew += *s++;
	}
	*s = 0;
	hashnew %= HASHSIZE;
    }
    else if (u_mode) {
	s = oldname - 1;
	while (*++s)
	if (islower ((int)*s)) {
	    if (oldlen < 4) oldlen = 4;
	    if (!(newname = malloc (oldlen + 1))) error (3, "char");
	    o = oldname;
	    s = newname;
	    hashnew = 0;
	    while (*o) {
		*s = *o++;
		if (islower ((int)*s)) *s -= 32; /* UPPER */
		hashnew += *s++;
	    }
	    *s = 0;
	    hashnew %= HASHSIZE;
	    break;
	}
    }

    qp = p;
    for (q = Hashnew[hashnew]; q; q = (qp = q) -> nnext)
    if (strcmp (newname, q -> nname) == 0) {
	/*
	** Already found, use another name!
	*/
	if (p -> pkey == dmproject &&
		(q -> pkey != dmproject || newname == oldname)) {
	    if (newname != oldname) p -> nname = q -> nname;
	    p -> nnext = q -> nnext;
	    if (qp != p) {
		qp -> nnext = p;
		qp = p;
	    }
	    else
		Hashnew[hashnew] = p;
	    p = q;
	}
	else qp = p;

	cn = newname = strsave (cn, 4);
again:
	if (++cn[3] > '9') {
	    cn[3] = '0';
	    if (++cn[2] > '9') {
		cn[2] = '0';
		if (++cn[1] > '9') {
		    cn[1] = '0';
		    if (++cn[0] > 'Z') {
			error (3, "name generation");
		    }
		}
	    }
	}
	hashnew = cn[0] + cn[1] + cn[2] + cn[3];
	hashnew %= HASHSIZE;
	for (q = Hashnew[hashnew]; q; q = q -> nnext)
	    if (strcmp (cn, q -> nname) == 0) goto again;
	q = qp; /* use qp for return */
	break;
    }

    if (newname != oldname) {
	new_names = 1;
	p -> nname = newname;
    }
    p -> nnext = Hashnew[hashnew];
    Hashnew[hashnew] = p;
    return (q? qp : p);
}
