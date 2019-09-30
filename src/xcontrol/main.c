/*
 * ISC License
 *
 * Copyright (C) 1999-2018 by
 *	M. Grueter
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
#include <signal.h>
#include <time.h>

char *argv0 = "xcontrol";
char *use_msg = "\n\
Usage: xcontrol [-macro | -regular | -device | -library | -remote]\n\
                [-strict | -free | -freemasks[:mask,..]] [-v] cell...\n\
       xcontrol -cleanup [-v] [cell...]\n\
       xcontrol -default [-v] [cell...]\n\
       xcontrol -liste   [ct] [it] [cell... | -cl | -icl | -obs | -new | -old]\n\
       xcontrol -list    [ct] [it] [cell... | -cl | -icl]\n\
       xcontrol -convert [-v]\n\n";

static DM_PROJECT *dmproject;
static DM_PROCDATA *mdata;
static DM_XDATA *newxdata;
static char msks[512];
static char *devicestream = "devmod";
static char *macrostream = "is_macro";
static FILE *fpxdata = NULL;

#define ERROR fprintf(stderr,
#define PRINT printf(
#define MAX_NR_MASKS 64

void sig_handler (int sig);
void printimpc (IMPCELL *ic, int ct, int it);
void printdata (DM_XDATA *data, char *path);

static void (* sigstatus) (int);
static int do_cleanup = 0;
static int setdefault = 0;
static int verbose = 0;
static int nomasks = 0;
static int lookRem = 0;
static int err_no = 0;
int   lc_no, ic_no;

static int max_lc = 0;
char **cellbuf = NULL;
int   *lc_stat = NULL;

static int max_ic = 0;
IMPCELL **icbuf = NULL;
int    *ic_stat = NULL;

int cmp_cl (const void *va, const void *vb)
{
    char **a = (char **)va;
    char **b = (char **)vb;
    return (strcmp (*a, *b));
}

int cmp_il (const void *va, const void *vb)
{
    IMPCELL **a = (IMPCELL **)va;
    IMPCELL **b = (IMPCELL **)vb;
    return (strcmp ((*a)->alias, (*b)->alias));
}

char **getCelllist ()
{
    char **cl, **cl1, **cl2;
    int i, j, n, rv;

    cl = cl1 = (char **) dmGetMetaDesignData(CELLLIST, dmproject, LAYOUT);
    i = 0; while (*cl++) ++i;
    if (i > 1) qsort (cl1, i, sizeof (char *), cmp_cl);
    cl = cl2 = (char **) dmGetMetaDesignData(CELLLIST, dmproject, CIRCUIT);
    i = 0; while (*cl++) ++i;
    if (i > 1) qsort (cl2, i, sizeof (char *), cmp_cl);

    i = j = n = 0;
    while (cl1[i] && cl2[j]) {
	rv = strcmp (cl1[i], cl2[j]);
	if (n == max_lc) { max_lc += 2048;
	    lc_stat = realloc (lc_stat, sizeof (int) * max_lc);
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	if (rv <= 0) {
	    if (rv == 0) { ++j; lc_stat[n] = 3; }
	    else lc_stat[n] = 1;
	    cellbuf[n++] = cl1[i++];
	}
	else {
	    lc_stat[n] = 2;
	    cellbuf[n++] = cl2[j++];
	}
    }
    while (cl1[i]) {
	if (n == max_lc) { max_lc += 2048;
	    lc_stat = realloc (lc_stat, sizeof (int) * max_lc);
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	lc_stat[n] = 1;
	cellbuf[n++] = cl1[i++];
    }
    while (cl2[j]) {
	if (n == max_lc) { max_lc += 2048;
	    lc_stat = realloc (lc_stat, sizeof (int) * max_lc);
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	lc_stat[n] = 2;
	cellbuf[n++] = cl2[j++];
    }
    lc_no = n;
    return cellbuf;
}

int search_stat;
int alias_in_cl;

int search_cl (char *name, int msg)
{
    int i, rv;
    for (i = 0; i < lc_no; ++i) {
	if (!(rv = strcmp (name, cellbuf[i]))) {
	    search_stat = lc_stat[i];
	    alias_in_cl = 1;
	    if (msg) ERROR "Warning: %s: alias also found in celllist!\n", name);
	    return 1;
	}
	if (rv < 0) break;
    }
    search_stat = 0;
    return 0;
}

IMPCELL *search_icl (char *name)
{
    int i, rv;
    for (i = 0; i < ic_no; ++i) {
	if (!(rv = strcmp (name, icbuf[i]->alias))) {
	    search_stat = ic_stat[i];
	    return icbuf[i];
	}
	if (rv < 0) break;
    }
    search_stat = 0;
    return NULL;
}

IMPCELL **getImpCelllist ()
{
    IMPCELL **il, **il1, **il2;
    int i, j, n, rv;

    il = il1 = (IMPCELL **) dmGetMetaDesignData(IMPORTEDCELLLIST, dmproject, LAYOUT);
    i = 0; while (*il++) ++i;
    if (i > 1) qsort (il1, i, sizeof (IMPCELL *), cmp_il);
    il = il2 = (IMPCELL **) dmGetMetaDesignData(IMPORTEDCELLLIST, dmproject, CIRCUIT);
    i = 0; while (*il++) ++i;
    if (i > 1) qsort (il2, i, sizeof (IMPCELL *), cmp_il);

    i = j = n = 0;
    while (il1[i] && il2[j]) {
	rv = strcmp (il1[i]->alias, il2[j]->alias);
	if (n == max_ic) { max_ic += 2048;
	    ic_stat = realloc (ic_stat, sizeof (int) * max_ic);
	    icbuf = realloc (icbuf, sizeof (IMPCELL *) * max_ic);
	}
	if (rv <= 0) {
	    if (rv == 0) {
		rv = strcmp (il1[i]->cellname, il2[j]->cellname);
		if (!rv) rv = strcmp (il1[i]->dmpath, il2[j]->dmpath);
		ic_stat[n] = rv ? 1 : 3;
		icbuf[n++] = il1[i];
		if (rv) {
		    ERROR "Warning: %s: imported celllists mismatch!\n", il2[j]->alias);
		    if (n == max_ic) { max_ic += 2048;
			ic_stat = realloc (ic_stat, sizeof (int) * max_ic);
			icbuf = realloc (icbuf, sizeof (IMPCELL *) * max_ic);
		    }
		    ic_stat[n] = 2;
		    icbuf[n++] = il2[j];
		}
		j++;
	    }
	    else {
		ic_stat[n] = 1;
		icbuf[n++] = il1[i];
	    }
	    (void) search_cl (il1[i]->alias, 1);
	    i++;
	}
	else {
	    (void) search_cl (il2[j]->alias, 1);
	    ic_stat[n] = 2;
	    icbuf[n++] = il2[j++];
	}
    }
    while (il1[i]) {
	(void) search_cl (il1[i]->alias, 1);
	if (n == max_ic) { max_ic += 2048;
	    ic_stat = realloc (ic_stat, sizeof (int) * max_ic);
	    icbuf = realloc (icbuf, sizeof (IMPCELL *) * max_ic);
	}
	ic_stat[n] = 1;
	icbuf[n++] = il1[i++];
    }
    while (il2[j]) {
	(void) search_cl (il2[j]->alias, 1);
	if (n == max_ic) { max_ic += 2048;
	    ic_stat = realloc (ic_stat, sizeof (int) * max_ic);
	    icbuf = realloc (icbuf, sizeof (IMPCELL *) * max_ic);
	}
	ic_stat[n] = 2;
	icbuf[n++] = il2[j++];
    }
    ic_no = n;
    return icbuf;
}

char **makeCelllist (int mode)
{
    char    *s, *t;
    char    **cl, **cl1, **cl2;
    IMPCELL **il, **il1, **il2;
    int n, rv, new_t;
    int it, jt, at, bt;
    int i, j, a, b;

    cl1 = cl2 = 0; /* suppres uninitialized warning */
    il1 = il2 = 0; /* suppres uninitialized warning */

    it = jt = at = bt = 0;
    if (mode & 1) {
	cl = cl1 = (char **) dmGetMetaDesignData(CELLLIST, dmproject, LAYOUT);
	while (*cl++) ++it;
	if (it > 1) qsort (cl1, it, sizeof (char *), cmp_cl);
	cl = cl2 = (char **) dmGetMetaDesignData(CELLLIST, dmproject, CIRCUIT);
	while (*cl++) ++jt;
	if (jt > 1) qsort (cl2, jt, sizeof (char *), cmp_cl);
    }
    if (mode & 2) {
	il = il1 = (IMPCELL **) dmGetMetaDesignData(IMPORTEDCELLLIST, dmproject, LAYOUT);
	while (*il++) ++at;
	if (at > 1) qsort (il1, at, sizeof (IMPCELL *), cmp_il);
	il = il2 = (IMPCELL **) dmGetMetaDesignData(IMPORTEDCELLLIST, dmproject, CIRCUIT);
	while (*il++) ++bt;
	if (bt > 1) qsort (il2, bt, sizeof (IMPCELL *), cmp_il);
    }

    i = j = a = b = n = 0;
    new_t = 1;
next_s:
    if (i < it) {
	if (j < jt) {
	    rv = strcmp (cl1[i], cl2[j]);
	    if (rv <= 0) {
		if (rv == 0) ++j;
		s = cl1[i++];
	    }
	    else
		s = cl2[j++];
	}
	else s = cl1[i++];
    }
    else if (j < jt) s = cl2[j++];
    else s = 0;

    t = 0;
    if (new_t) {
next_t:
	if (a < at) {
	    if (b < bt) {
		rv = strcmp (il1[a]->alias, il2[b]->alias);
		if (rv <= 0) {
		    if (rv == 0) ++b;
		    t = il1[a++]->alias;
		}
		else
		    t = il2[b++]->alias;
	    }
	    else t = il1[a++]->alias;
	}
	else if (b < bt) t = il2[b++]->alias;
    }

    if (s && t) {
	if (n == max_lc) { max_lc += 2048;
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	if ((rv = strcmp (s, t)) <= 0) {
	    new_t = (rv == 0);
	    cellbuf[n++] = s;
	    goto next_s;
	}
	cellbuf[n++] = t;
	goto next_t;
    }
    else if (s) {
	if (n == max_lc) { max_lc += 2048;
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	new_t = 0;
	cellbuf[n++] = s;
	goto next_s;
    }
    else if (t) {
	if (n == max_lc) { max_lc += 2048;
	    cellbuf = realloc (cellbuf, sizeof (char *) * max_lc);
	}
	cellbuf[n++] = t;
	goto next_t;
    }
    lc_no = n;
    return cellbuf;
}

char *pbn (DM_PROJECT *proj)
{
    char * t = strrchr (proj->dmpath, '/');
    return t? t+1 : proj->dmpath;
}

void unlinkMacro (DM_PROJECT *proj, char *cell)
{
    char path[MAXLINE];
    char *pp = proj->dmpath;
    _dmSprintf (path, "%s/%s/%s/%s", pp, LAYOUT, cell, macrostream);
    if (unlink (path)) {
	char *t = strrchr (pp, '/');
	if (t) ++t; else t = pp;
	ERROR "Warning: %s: cannot unlink!\n", path + (t - pp));
    }
}

void unlinkDevice (DM_PROJECT *proj, char *cell)
{
    char path[MAXLINE];
    char *pp = proj->dmpath;
    _dmSprintf (path, "%s/%s/%s/%s", pp, CIRCUIT, cell, devicestream);
    if (unlink (path)) {
	char *t = strrchr (pp, '/');
	if (t) ++t; else t = pp;
	ERROR "Warning: %s: cannot unlink!\n", path + (t - pp));
    }
}

int getOldCellStatus (DM_PROJECT *proj, DM_XDATA *xdata, int used)
{
    struct stat sbuf;
    int mkno[MAX_NR_MASKS];
    DM_CELL *ck;
    DM_STREAM *sp;
    char *arg, *name;
    int rv = 0;

    name = xdata->name;
    xdata->timestamp = 0;
    xdata->celltype = DM_CT_REGULAR;
    xdata->interfacetype = DM_IF_STRICT;
    xdata->masks = msks;
    *msks = 0;

    ck = dmCheckOut (proj, name, WORKING, DONTCARE, LAYOUT, READONLY);
    if (ck) {
	if (dmStat (ck, macrostream, &sbuf) == 0) {
	    int i = 0;
	    rv += 4;
	    xdata->timestamp = sbuf.st_mtime;
	    if (!(sp = dmOpenStream (ck, macrostream, "r"))) {
		ERROR "%s/%s/%s/%s: cannot open file!\n", pbn (proj), LAYOUT, name, macrostream);
		err_no = 16;
		dmError(0);
	    }
	    (void) fscanf (sp->dmfp, "%d %s", &i, msks);
	    dmCloseStream (sp, COMPLETE);

	    if (i) {
		xdata->celltype = DM_CT_MACRO;
		xdata->interfacetype = DM_IF_FREE;
	    }
	    if (*msks) {
		int n = 0;
		for (i = 0; i < MAX_NR_MASKS; ++i) mkno[i] = 0;
		arg = strtok (msks, "+");
		while (arg) {
		    for (i = 0; i < nomasks; ++i) {
			if (strcmp (arg, mdata->mask_name[i]) == 0) break;
		    }
		    if (i == nomasks) {
			ERROR "%s/%s/%s/%s: Unknown mask '%s'! SKIPPED\n", pbn (proj), LAYOUT, name, macrostream, arg);
		    } else if (i >= MAX_NR_MASKS) {
			ERROR "%s/%s/%s/%s: Mask-nr of '%s' too big! SKIPPED\n", pbn (proj), LAYOUT, name, macrostream, arg);
		    } else if (!mkno[i]) {
			mkno[i] = 1;
			++n;
		    }
		    arg = strtok (NULL, "+");
		}

		if (n == nomasks) {
		    xdata->interfacetype = DM_IF_FREE;
		    *msks = 0;
		}
		else {
		    xdata->interfacetype = DM_IF_FREEMASKS;
		    arg = msks;
		    for (i = 0; i < MAX_NR_MASKS; ++i) if (mkno[i]) {
			sprintf (arg, " %d", i);
			arg += strlen (arg);
		    }
		    *arg = 0;
		}
	    }
	}
	dmCheckIn (ck, COMPLETE);
    }
    else if (used & 1) {
	ERROR "%s/%s/%s: cannot checkout cell!\n", pbn (proj), LAYOUT, name);
	err_no = 16;
	dmError(0);
    }

    ck = dmCheckOut (proj, name, WORKING, DONTCARE, CIRCUIT, READONLY);
    if (ck) {
	if (dmStat (ck, "devmod", &sbuf) == 0) {
	    rv += 2;
	    if (sbuf.st_size == 0) rv += 8;
	    if (xdata->timestamp < sbuf.st_mtime)
		xdata->timestamp = sbuf.st_mtime;
	    xdata->celltype = DM_CT_DEVICE;
	}
	dmCheckIn (ck, COMPLETE);
    }
    else if (used & 2) {
	ERROR "%s/%s/%s: cannot checkout cell!\n", pbn (proj), CIRCUIT, name);
	err_no = 16;
	dmError(0);
    }
    return rv;
}

void putCellStatus ()
{
#ifdef SIGINT
    if (sigstatus != (void (*)(int)) SIG_IGN) signal (SIGINT, SIG_IGN);
#endif
    (void) dmPutCellStatus (dmproject, newxdata);
    if (verbose) {
	(void) dmGetCellStatus (dmproject, newxdata);
	printdata (newxdata, 0);
    }
#ifdef SIGINT
    if (sigstatus != (void (*)(int)) SIG_IGN) signal (SIGINT, sig_handler);
#endif
}

char *strsave (char *s)
{
    char *p;
    if (!s || !*s) return (NULL);
    if (!(p = (char *) malloc (strlen (s) + 1))) dmError(0);
    return (strcpy (p, s));
}

int main (int argc, char *argv[])
{
    struct stat sbuf;
    FILE *fp;
    char          **celllist = NULL;
    char          **clist;
    char          **cptr;
    char           *mlist = NULL;
    char           *arg;
    int             ccount = 0;
    int             write = 0;
    int             list = 0;
    int             listent = 0;
    int             listcl = 0;
    int             listicl = 0;
    int             listobs = 0;
    int             listold = 0;
    int             listnew = 0;
    int             do_convert = 0;
    int             ct = 0; /* unknown */
    int             it = 0; /* unknown */
    int             err, i;

    clist = (char **) calloc(argc, sizeof(char *));

    newxdata = (DM_XDATA *) calloc(1, sizeof(DM_XDATA));

    while (*++argv) {
	arg = *argv;
	if (*arg == '-') {
	    if (strcmp(++arg, "v") == 0) {
		verbose = 1;
	    } else if (strcmp(arg, "macro") == 0) {
		ct = DM_CT_MACRO;
	    } else if (strcmp(arg, "regular") == 0) {
		ct = DM_CT_REGULAR;
	    } else if (strcmp(arg, "device") == 0) {
		ct = DM_CT_DEVICE;
	    } else if (strcmp(arg, "library") == 0) {
		ct = DM_CT_LIBRARY;
	    } else if (strcmp(arg, "remote") == 0) {
		ct = DM_CT_IMPORT;
	    } else if (strcmp(arg, "strict") == 0) {
		it = DM_IF_STRICT;
	    } else if (strcmp(arg, "free") == 0) {
		it = DM_IF_FREE;
	    } else if (strncmp(arg, "freemask", 8) == 0) {
		it = DM_IF_FREEMASKS;
		mlist = arg + 8;
	    } else if (strcmp(arg, "cleanup") == 0) {
		do_cleanup = 1; setdefault = list = 0;
	    } else if (strcmp(arg, "convert") == 0) {
		do_convert = 1;
	    } else if (strcmp(arg, "default") == 0) {
		setdefault = 1; do_cleanup = list = 0;
	    } else if (strncmp(arg, "list", 4) == 0) {
		list = 1; do_cleanup = setdefault = 0;
		if (arg[4] == 'e') listent = 1;
	    } else if (strcmp(arg, "cl") == 0) {
		list = listcl = 1;
	    } else if (strcmp(arg, "icl") == 0) {
		list = listicl = 1;
	    } else if (strcmp(arg, "obs") == 0) {
		list = listobs = 1;
	    } else if (strcmp(arg, "old") == 0) {
		list = listold = 1;
	    } else if (strcmp(arg, "new") == 0) {
		list = listnew = 1;
	    } else {
		ERROR "%s: Wrong argument: -%s\n%s", argv0, arg, use_msg);
		exit(1);
	    }
	} else {
	    /*
	     * Get the cells
	     */
	    clist[ccount++] = arg;
	}
    }

    if (do_convert) {
	if (list || ct || it || setdefault || do_cleanup || clist[0]) {
	    ERROR "%s: Wrong argument specified!\n%s", argv0, use_msg);
	    exit(1);
	}
    }
    else if (list) {
	list = 1;
    }
    else if (setdefault) {
	if (ct || it)
	ERROR "Warning: Cells are set to default. All type options ignored.\n");
    }
    else if (do_cleanup) {
	if (ct || it)
	ERROR "Warning: Cells are cleaned up. All type options ignored.\n");
    }
    else if (clist[0]) {
	if (ct || it) write = 1;
	else list = 1;
    }
    else {
	ERROR "%s: No cell specified.\n%s", argv0, use_msg);
	exit(1);
    }

#ifdef SIGHUP
    signal (SIGHUP, SIG_IGN); /* ignore hangup signal */
#endif
#ifdef SIGINT
    sigstatus = (void (*)(int)) signal (SIGINT, SIG_IGN);
    if (sigstatus != (void (*)(int)) SIG_IGN) signal (SIGINT, sig_handler);
#endif
#ifdef SIGQUIT
    signal (SIGQUIT, SIG_IGN);
#endif
#ifdef SIGTERM
    signal (SIGTERM, sig_handler);
#endif

    dmInit(argv0);
    err_no = 1;
    dmproject = dmOpenProject(DEFAULT_PROJECT, DEFAULT_MODE);
    err_no = 2;
    mdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    if (mdata) nomasks = mdata->nomasks;
    err_no = 0;

    if (dmStatXData(dmproject, &sbuf)) {
	if (!do_convert) {
	    ERROR "%s: Cannot open the xdata file.\n", argv0);
	    ERROR "Use xcontrol -convert first!\n");
	    exit(1);
	}
    }
    else if (do_convert) {
	ERROR "%s: Xdata file already found.\n", argv0);
	ERROR "Project '%s' is already converted!\n", dmproject->dmpath);
	exit(1);
    }

    if (do_convert) {
	DM_PROJECT *newproject;
	char *p, *lc, *ic, *cc;
	int lno, ino, old_proj;
	int do_unlink = 0;

	newproject = 0; /* suppres uninitialized warning */
	old_proj = 0; /* suppres uninitialized warning */

	ERROR "Converting cellstate information...\n");

	fpxdata = dmOpenXData (dmproject, "w");
	if (!fpxdata) {
	    err_no = 6;
	    dmError(0);
	}

	err_no = 16;
	(void) getCelllist();
	(void) getImpCelllist();
	if (alias_in_cl) {
	    err_no = 7;
	    dmError(0);
	}

	lno = 0;
	ino = 0;
	lc = lc_no ? cellbuf[lno] : NULL;
	ic = ic_no ? icbuf[ino]->alias : NULL;
	p = NULL;

	while (lc || ic) {
	    if (lc && ic) {
		i = strcmp (lc, ic);
		if (i <= 0) {
		    if (i == 0) { err_no = 7; dmError(0); }
		    cc = lc;
		}
		else cc = ic;
	    }
	    else if (lc) cc = lc;
	    else cc = ic;

	    if (cc == lc) {
		if (verbose) ERROR "-- loc.cell '%s'...", cc);
		err_no = -1;
		newxdata->name = cc;
		err = getOldCellStatus (dmproject, newxdata, lc_stat[lno]);
		if (err) {
		    if (err & 4) { lc_stat[lno] += 4; do_unlink = 1; }
		    if (err & 8) { lc_stat[lno] += 8; do_unlink = 1; }
		    err_no = 4;
		    err = dmPutXData (fpxdata, newxdata);
		    if (verbose) ERROR "done\n");
		}
		else
		    if (verbose) ERROR "skipped\n");
		++lno;
		lc = lno < lc_no ? cellbuf[lno] : NULL;
	    }
	    else {
		if (verbose) ERROR "-- imp.cell '%s'...", cc);

		if (!p || strcmp (p, icbuf[ino]->dmpath)) {
		    p = icbuf[ino]->dmpath;
		    err_no = 5;
		    newproject = dmOpenProject (p, PROJ_READ);
		    old_proj = dmStatXData (newproject, &sbuf);
		}
		if (!old_proj) { /* new project */
		    if (verbose) ERROR "skipped\n");
		    goto skip_alias;
		}
		err_no = -1;
		newxdata->name = icbuf[ino]->cellname;
		err = getOldCellStatus (newproject, newxdata, ic_stat[ino]);

		newxdata->name = cc;
		err_no = 4;
		err = dmPutXData (fpxdata, newxdata);
		if (verbose) ERROR "done\n");
skip_alias:
		if (++ino < ic_no) {
		    if (ic_stat[ino] == 2) /* skip double entries */
			if (!strcmp (icbuf[ino]->alias, cc)) goto skip_alias;
		    ic = icbuf[ino]->alias;
		}
		else ic = NULL;
	    }
	}
	fclose (fpxdata); fpxdata = NULL;

	ERROR "Converting cellstate information DONE!\n");

	if (do_unlink) {
	    if (verbose) ERROR "Removing old status info of local cells.\n");
	    for (i = 0; i < lc_no; ++i) {
		if (lc_stat[i] & 4) unlinkMacro (dmproject, cellbuf[i]);
		if (lc_stat[i] & 8) unlinkDevice(dmproject, cellbuf[i]);
	    }
	}
    }
    else if (write) {
	DM_PROJECT *newproject;
	IMPCELL *ic;
	char *s;
	/*
	 * Write cells
	 */
	if (it == DM_IF_FREEMASKS) {
	  if (*mlist == 's') ++mlist;
	  if (*mlist) {
	    if (*mlist != ':' || !(mlist = strtok(++mlist, ", "))) {
		ERROR "%s: Incorrect free masklist!\n", argv0);
		exit(1);
	    }
	    arg = msks;
	    do {
		for (i = 0; i < nomasks; ++i) {
		    if (strcmp (mlist, mdata->mask_name[i]) == 0) {
			if (i >= MAX_NR_MASKS) {
			    ERROR "%s: Too big mask-nr: mask '%s' in masklist!\n", argv0, mlist);
			    exit(1);
			}
			sprintf (arg, " %d", i);
			arg += strlen (arg);
			break;
		    }
		}
		if (i == nomasks) {
		    ERROR "%s: Unknown mask '%s' in masklist!\n", argv0, mlist);
		    exit(1);
		}
	    } while ((mlist = strtok(NULL, ", ")));
	  }
	}

	(void) getCelllist ();
	(void) getImpCelllist ();

	s = (ct == DM_CT_IMPORT)? "Skipping" : "Warning";

	for (cptr = clist; *cptr; ++cptr) {

	    newxdata->name = *cptr;
	    err = dmGetCellStatus (dmproject, newxdata);

	    if (search_cl (*cptr, 0)) {
		if (ct == DM_CT_IMPORT) {
		    PRINT "%s %s: is a local cell!\n", s, *cptr);
		    continue;
		}
	    }
	    else if ((ic = search_icl (*cptr))) {
		if (err || newxdata->celltype == DM_CT_IMPORT || ct == DM_CT_IMPORT) {
		    err_no = -1; /* remote project must have xcontrol */
		    newproject = dmOpenProject (ic->dmpath, PROJ_READ);
		    err_no = 0;
		    if (!newproject) {
			PRINT "%s %s: cannot open project: %s\n", s, *cptr, ic->dmpath);
			if (ct == DM_CT_IMPORT) continue;
		    }
		    else if (dmStatXData (newproject, &sbuf)) {
			PRINT "%s %s: no xdata in project: %s\n", s, *cptr, ic->dmpath);
			if (ct == DM_CT_IMPORT) continue;
		    }
		    newxdata->name = ic->cellname;
		    (void) dmGetCellStatus (newproject, newxdata);
		    newxdata->name = *cptr;
		}
	    }
	    else if (ct == DM_CT_IMPORT) {
		PRINT "%s %s: is not an imported cell!\n", s, *cptr);
		continue;
	    }

	    if (ct) newxdata->celltype = ct;
	    if (it) {
		newxdata->interfacetype = it;
		if (*msks) newxdata->masks = msks;
	    }
	    err_no = -2;
	    putCellStatus ();
	    err_no = 0;
	}
    }
    else if (do_cleanup) {
	DM_XDATA **xlst;
	int do_cln = 0;

	ERROR "Cleaning cellstate information...\n");
	i = (int) (sbuf.st_size / 16);
	if (i == 0) goto cln_done;
	err_no = 16;

	if (!clist[0]) celllist = makeCelllist(3);
	search_stat = 0;
	xlst = (DM_XDATA **) malloc (i * sizeof (DM_XDATA *));
	if (!xlst) { err_no = 8; dmError(0); }
	i = 0;

	if (!(fp = dmOpenXData(dmproject, "r"))) {
	    err_no = 6;
	    dmError(0);
	}
	while (dmGetXData(fp, newxdata) > 0) {
	    arg = newxdata->name;
	    if (clist[0]) {
		cptr = clist - 1;
		while (*++cptr && strcmp (arg, *cptr));
		err = *cptr ? 1 : 0;
	    }
	    else {
		int j;
		err = 1;
		for (j = 0; j < lc_no && err > 0; ++j) err = strcmp (arg, celllist[j]);
	    }
	    if (err) { /* remove this (obsolete) entry */
		if (verbose) printdata(newxdata, 0);
		do_cln = 1;
	    }
	    else {
		DM_XDATA *xdata;
		err_no = 8;
		xlst[i++] = xdata = (DM_XDATA *) malloc (sizeof (DM_XDATA));
		if (!xdata) dmError(0);
		xdata->name = strsave (newxdata->name);
		xdata->timestamp = newxdata->timestamp;
		xdata->celltype = newxdata->celltype;
		xdata->interfacetype = newxdata->interfacetype;
		xdata->masks = strsave (newxdata->masks);
		err_no = 16;
	    }
	}
	fclose(fp);
	if (do_cln) {
#ifdef SIGINT
	if (sigstatus != (void (*)(int)) SIG_IGN) signal (SIGINT, SIG_IGN);
#endif
	    if ((fp = dmOpenXData (dmproject, "w"))) {
		err_no = 4;
		err = i;
		for (i = 0; i < err; ++i) (void) dmPutXData (fp, xlst[i]);
		fclose (fp);
	    }
	    else { err_no = 6; dmError(0); }
	}
cln_done:
	ERROR "Cleaning cellstate information DONE!\n");
    }
    else if (setdefault) {
	DM_XDATA **xlst;
	DM_PROJECT *newproject;
	IMPCELL *ic;

	ERROR "Set default cellstate information...\n");
	err_no = 16;

	if (!clist[0]) {
	    i = (int) (sbuf.st_size / 16);
	    if (i == 0) goto def_done;
	    xlst = (DM_XDATA **) malloc (i * sizeof (DM_XDATA *));
	}
	else
	    xlst = 0; /* suppres uninitialized warning */

	(void) getCelllist();
	(void) getImpCelllist();

	/*
	 * Write defaults to cell records
	 */
	if (clist[0]) {
	    cptr = clist;
	    while (*cptr) {
		if (!search_cl (*cptr, 0) && (ic = search_icl (*cptr))) {
		    err_no = 5;
		    newproject = dmOpenProject(ic->dmpath, PROJ_READ);
		    newxdata->name = ic->cellname;
		    err_no = 16;
		    if (dmStatXData (newproject, &sbuf) == 0) {
			err = dmGetCellStatus(newproject, newxdata);
		    }
		    else {
			err_no = -1;
			err = getOldCellStatus (newproject, newxdata, search_stat);
			err_no = 16;
		    }
		}
		else {
		    newxdata->celltype = DM_CT_REGULAR;
		    newxdata->interfacetype = DM_IF_STRICT;
		}
		newxdata->name = *cptr++;
		putCellStatus ();
	    }
	}
	else {
	    DM_XDATA *xdata;
	    if (!(fp = dmOpenXData(dmproject, "r"))) {
		err_no = 6;
		dmError(0);
	    }
	    i = 0;
	    while (dmGetXData(fp, newxdata) > 0) {
		arg = strsave (newxdata->name);
		if (!search_cl (arg, 0) && (ic = search_icl (arg))) {
		    err_no = 5;
		    newproject = dmOpenProject (ic->dmpath, PROJ_READ);
		    newxdata->name = ic->cellname;
		    if (dmStatXData (newproject, &sbuf) == 0) {
			err_no = 16;
			err = dmGetCellStatus (newproject, newxdata);
		    }
		    else {
			err_no = -1;
			err = getOldCellStatus (newproject, newxdata, search_stat);
		    }
		}
		else {
		    newxdata->celltype = DM_CT_REGULAR;
		    newxdata->interfacetype = DM_IF_STRICT;
		}
		err_no = 8;
		xlst[i++] = xdata = (DM_XDATA *) malloc (sizeof (DM_XDATA));
		if (!xdata) dmError(0);
		xdata->name = arg;
		xdata->celltype = newxdata->celltype;
		xdata->interfacetype = newxdata->interfacetype;
		xdata->masks = strsave (newxdata->masks);
		err_no = 16;
	    }
	    fclose(fp);

#ifdef SIGINT
	if (sigstatus != (void (*)(int)) SIG_IGN) signal (SIGINT, SIG_IGN);
#endif
	    if ((fp = dmOpenXData (dmproject, "w"))) {
		long ts = time (0);
		search_stat = 0;
		err_no = 4;
		err = i;
		for (i = 0; i < err; ++i) {
		    xlst[i]->timestamp = ts;
		    if (verbose) printdata (xlst[i], 0);
		    (void) dmPutXData (fp, xlst[i]);
		}
		fclose (fp);
	    }
	    else {
		err_no = 6;
		dmError(0);
	    }
	}
def_done:
	ERROR "Set default cellstate information DONE!\n");
    }
    else if (list) {
	if (listent) {
	    if ((fp = dmOpenXData(dmproject, "r"))) {
		long old, new, sec;

		new = 0;
		if (listobs) new = 3;
		else {
		    if (listcl) new = 1;
		    if (listicl) new += 2;
		}
		celllist = new ? makeCelllist(new) : NULL;
		search_stat = 0;

		old = new = sec = 0;
	        PRINT "entries:\n");
begin_get:
		while (dmGetXData(fp, newxdata) > 0) {
		    if (ct && ct != newxdata->celltype) continue;
		    if (it && it != newxdata->interfacetype) continue;

		    i = 0;
		    if (!*clist && !listcl && !listicl && !listobs) i = 1;
		    if (!i && *clist && !listobs) {
			cptr = clist - 1;
			while (*++cptr) if (strcmp (newxdata->name, *cptr) == 0) { i = 1; break; }
		    }
		    if (!i && celllist) {
			int j;
			err = 1;
			for (j = 0; j < lc_no && err > 0; ++j) err = strcmp (newxdata->name, celllist[j]);
			if (err == 0) i = 1;
		    }
		    if (listobs) i = !i;
		    if (i) {
			if (listold) {
			    if (sec) {
				if (newxdata->timestamp == old) goto prt_it;
			    }
			    else {
				if (!old || newxdata->timestamp < old)
				    old = newxdata->timestamp;
			    }
			    i = 0;
			}
			if (listnew) {
			    if (sec) {
				if (newxdata->timestamp == new) goto prt_it;
			    }
			    else {
				if (!new || newxdata->timestamp > new)
				    new = newxdata->timestamp;
			    }
			    i = 0;
			}
			if (i)
prt_it:				printdata(newxdata, 0);
		    }
		}
		if ((listold || listnew) && !sec) {
		    rewind(fp);
		    sec = 1;
		    goto begin_get;
		}
		PRINT "\n");
		fclose(fp);
	    }
	    else
		ERROR "%s: Cannot open xdata file.\n", argv0);
	}
	else if (*clist) {
	    IMPCELL *ic;

	    (void) getCelllist();
	    (void) getImpCelllist();

	    PRINT "cells:\n");
	    while (*clist) {
		newxdata->name = *clist++;
		err = dmGetCellStatus (dmproject, newxdata);
		ic = NULL;
		if (!search_cl (newxdata->name, 0))
		    if ((ic = search_icl (newxdata->name)))
			if (!err && newxdata->celltype != DM_CT_IMPORT) ic = NULL;
		printimpc (ic, ct, it);
	    }
	    PRINT "\n");
	}
	else {
	    IMPCELL *ic = NULL;
	    if (!listcl && !listicl) listcl = listicl = 1;
	    /*
	     * Get all cells from the project if no cells specified yet
	     */
	    if (listcl) {
		(void) getCelllist();
		if (lc_no) {
		    PRINT "local cells:\n");
		    for (i = 0; i < lc_no; ++i) {
			search_stat = lc_stat[i];
			newxdata->name = cellbuf[i];
			(void) dmGetCellStatus (dmproject, newxdata);
			printimpc (ic, ct, it);
		    }
		    PRINT "\n");
		}
	    }
	    /*
	     * Get all imported cells
	     */
	    if (listicl) {
		if (!listcl) (void) getCelllist();
		(void) getImpCelllist();
		if (ic_no) {
		    PRINT "imported cells:\n");
		    for (i = 0; i < ic_no; ++i) {
			search_stat = ic_stat[i];
			ic = icbuf[i];
			newxdata->name = ic->alias;
			err = dmGetCellStatus (dmproject, newxdata);
			if (!err && newxdata->celltype != DM_CT_IMPORT) {
			    if (i+1 < ic_no) {
				if (strcmp (ic->alias, icbuf[i+1]->alias) == 0) {
				    ++i; search_stat += 2;
				}
			    }
			    ic = NULL;
			}
			printimpc (ic, ct, it);
		    }
		    PRINT "\n");
		}
	    }
	}
    }

    dmCloseProject(dmproject, COMPLETE);
    dmQuit();
    exit(0);
    return (0);
}

void dmError (char *s)
{
    if (err_no == -1) return;
    if (err_no == -2) ERROR "%s: ", newxdata->name);
    if (s) dmPerror(s);
    if (err_no < 0) return;
    if (err_no == 3) {
	ERROR "%s: Cannot open remote project.\n", argv0);
	return;
    }
    if (err_no == 1) {
	if (getenv ("CWD"))
	    ERROR "%s: Cannot open project; CWD is not a correct dir. path\n", argv0);
	else
	    ERROR "%s: Cannot open project; cwd is not a project directory\n", argv0);
    }
    else if (err_no == 2) {
	ERROR "%s: Cannot read maskdata.\n", argv0);
    }
    else if (err_no >= 4) {
	if (setdefault)
	    ERROR "%s: Set default aborted!\n", argv0);
	else if (do_cleanup)
	    ERROR "%s: Cleanup NOT successful!\n", argv0);
	else
	    ERROR "%s: Conversion NOT successful!\n", argv0);
	if (err_no == 4)
	    ERROR "%s: Cannot write/put xdata.\n", argv0);
	else if (err_no == 5)
	    ERROR "%s: Cannot open remote project.\n", argv0);
	else if (err_no == 6)
	    ERROR "%s: Cannot open/create xdata file.\n", argv0);
	else if (err_no == 7)
	    ERROR "%s: Alias found in celllist!\n", argv0);
	else if (err_no == 8)
	    ERROR "%s: Cannot alloc enough memory!\n", argv0);
	if (fpxdata) {
	    fclose (fpxdata); fpxdata = NULL;
	    (void) dmUnlinkXData (dmproject);
	}
    }
    dmQuit();
    exit(1);
}

void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    ERROR "program interrupted!\n");
    if (fpxdata || do_cleanup) {
	err_no = 16;
	dmError(0);
    }
    dmQuit();
    exit(1);
}

void printimpc (IMPCELL *ic, int ct, int it)
{
    char *path = 0;

    if (ic) {
	DM_PROJECT *proj;
	lookRem = 1; /* remote project must have xcontrol */
	err_no = -1;
	proj = dmOpenProject (ic->dmpath, PROJ_READ);
	err_no = 0;
	if (!proj) path = ic->dmpath;
	newxdata->name = ic->cellname;
	(void) dmGetCellStatus (proj, newxdata);
	newxdata->name = ic->alias;
    }
    if ((!ct || ct == newxdata->celltype) &&
	(!it || it == newxdata->interfacetype)) printdata (newxdata, path);
    lookRem = 0;
}

void printdata (DM_XDATA *data, char *path)
{
    char *m;
    int i;

    if (data) {
	/*
	 * Name
	 */
	m = data->name;
	if ((i = strlen(m)) >= 15) {
	    PRINT "  %s\n", m); m = "";
	}
	PRINT "  %-15s", m);
	/*
	 * View
	 */
	switch (search_stat) {
	case 1:  PRINT "L "); break;
	case 2:  PRINT "C "); break;
	case 3:  PRINT "LC"); break;
	default: PRINT "  ");
	}
	if (lookRem) PRINT "* ");
	else         PRINT "  ");
	/*
	 * Celltype
	 */
	switch (data->celltype) {
	case DM_CT_REGULAR:
	    PRINT "Regular  ");
	    break;
	case DM_CT_MACRO:
	    PRINT "Macro    ");
	    break;
	case DM_CT_DEVICE:
	    PRINT "Device   ");
	    break;
	case DM_CT_LIBRARY:
	    PRINT "Library  ");
	    break;
	case DM_CT_IMPORT:
	    PRINT "Remote   ");
	    break;
	default:
	    PRINT "Unknown  ");
	}
	/*
	 * Interfacetype
	 */
	switch (data->interfacetype) {
	case DM_IF_STRICT:
	    PRINT "Strict    ");
	    break;
	case DM_IF_FREE:
	    PRINT "Free      ");
	    break;
	case DM_IF_FREEMASKS:
	    PRINT "Freemasks ");
	    break;
	default:
	    PRINT "Unknown   ");
	}

	/*
	 * Time
	 */
	if (data->timestamp != 0) {
	    time_t timestamp = data->timestamp;
	    PRINT "%s", ctime(&timestamp) + 4);
	}
	else {
	    if (path)
		PRINT "Cannot open project %s\n", path);
	    else if (dmerrno == DME_FOPEN)
		PRINT "Cannot open xdata\n");
	    else
		PRINT "No status found\n");
	}
	/*
	 * Masks
	 */
	if (*data->masks || data->interfacetype == DM_IF_FREEMASKS) {
	    PRINT "           Mask(s):");
	    if ((m = strtok (data->masks, " "))) {
		do {
		    if ((i = atoi (m)) < nomasks)
			PRINT " %s", mdata->mask_name[i]);
		    else
			PRINT " ??");
		} while ((m = strtok (NULL, " ")));
	    }
	    PRINT "\n");
	}
    }
}
