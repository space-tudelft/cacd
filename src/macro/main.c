/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

char *argv0 = "macro";
char *use_msg = "\nUsage: %s [-amask | -dmask | -f | -s[mask] | -u | -t] [cell ...]\n\n";

#define MAX_NR_MASKS 64
/* #define DEBUG */
#define PE fprintf (stderr,
#define PO fprintf (stdout,

int full_macro = 0;
int tog_set = 0;
int tog_full = 0;
int tog_unset = 0;
int set_val = 0;
int set_all = 0;
int noErrMes = 0;
int notok_no = 0;
int xcontrol = 0;

int *a_mask;
int *c_mask;
int *d_mask;

struct stat buf;
char *masks;
char oldmasks[1024];
char newmasks[1024];
char *setmasks;
char *addmasks;
char *delmasks;
char *proj_notok[20];

DM_PROJECT *dmproject = NULL;
DM_PROCDATA *process = NULL;
DM_CELL *cellKey;
DM_STREAM *dsp;
DM_XDATA xdata;

int  setMasks (void);
void int_hdl (int sig);
void initIntrup (void);
void die (void);

DM_PROJECT *getprojkey (char *path)
{
    int i;
    DM_PROJECT *proj = 0;

    if (notok_no) {
	for (i = 0; i < notok_no; ++i) {
	    if (path == proj_notok[i] ||
		strcmp (path, proj_notok[i]) == 0) return proj;
	}
    }
    noErrMes = 1;
    proj = dmOpenProject (path, PROJ_READ);
    noErrMes = 0;
    if (!proj) proj_notok[notok_no++] = path;
    return proj;
}

void do_cell (DM_PROJECT *proj, char *cell, IMPCELL *ic)
{
    int i, isMacro = 0, isNr = 0, type = ' ';

    masks = oldmasks; *masks = 0;

    if (xcontrol) {
	xdata.name = cell;
	i = dmGetCellStatus (proj, &xdata);
	if (ic) {
	    if (i || xdata.celltype == DM_CT_IMPORT) {
		if (!(proj = getprojkey (ic -> dmpath))) return;
read_xrem:
		xdata.name = ic -> cellname;
		i = dmGetCellStatus (proj, &xdata);
	    }
	}
	switch (xdata.celltype) {
	case DM_CT_DEVICE:  type = 'D';
	case DM_CT_LIBRARY: if (type == ' ') type = 'L';
	    if (xdata.interfacetype == DM_IF_STRICT) break;
	case DM_CT_MACRO:
	    isMacro = 1;
	    if (xdata.interfacetype == DM_IF_FREEMASKS) { masks = xdata.masks; isNr = 1; }
	}
    }
    else {
	if (ic) {
	    if (!(proj = getprojkey (ic->dmpath))) return;
	    if (dmStatXData (proj, &buf) == 0) goto read_xrem;
	}
	noErrMes = 1; /* SdeG4.18 */
	cellKey = dmCheckOut (proj, ic? ic->cellname : cell, WORKING, DONTCARE, LAYOUT, READONLY);
	noErrMes = 0;
	if (!cellKey) {
	    PO "cannot read cell: %s/%s/%s\n", proj->dmpath, LAYOUT, ic? ic->cellname : cell);
	    return;
	}
	if (dmStat (cellKey, "is_macro", &buf) == 0 &&
	    (dsp = dmOpenStream (cellKey, "is_macro", "r"))) {
	    fscanf (dsp -> dmfp, "%d%s", &isMacro, masks);
	    dmCloseStream (dsp, COMPLETE);
	}
	dmCheckIn (cellKey, COMPLETE);
    }

    if (isMacro) {
	PO "%c%c %s", type, ic? '*' : ' ', cell);
	if (*masks) {
	    char *m;
	    PO " (only for masks:");
	    while ((m = strtok (masks, " +"))) {
		masks = NULL;
		if (isNr) {
		    if ((i = atoi (m)) < process -> nomasks)
			PO " %s", process -> mask_name[i]);
		    else PO " %s??", m);
		}
		else PO " %s", m);
	    }
	    PO ")\n");
	}
	else PO "\n");
    }
}

int main (int argc, char *argv[])
{
    char *s, *m;
    char **cellNames;
    char **celllist;
    char *cell;
    int cell_cnt;
    int wasMacro;
    int isMacro;
    int i, mode;
    int usage = 0;

    cellNames = (char **) calloc (argc, sizeof (char *));

    cell_cnt = 0;
    while (--argc > 0) {
        if ((*++argv)[0] == '-') {
	    for (s = *argv + 1; *s; ++s) {
	        switch (*s) {
		    case 's':
			tog_set = 1;
			if (*++s) {
			    if (setmasks) {
				PE "%s: option -s: already specified\n", argv0);
				usage = 1;
			    }
			    setmasks = s;
			}
			goto nextarg;
		    case 'a':
			if (*++s) {
			    if (addmasks) {
				PE "%s: option -a: already specified\n", argv0);
				usage = 1;
			    }
			    addmasks = s;
			}
			goto nextarg;
		    case 'd':
			if (*++s) {
			    if (delmasks) {
				PE "%s: option -d: already specified\n", argv0);
				usage = 1;
			    }
			    delmasks = s;
			}
			goto nextarg;
		    case 'f':
			tog_full = 1;
			break;
		    case 't':
			set_all = 1;
			break;
		    case 'u':
			tog_unset = 1;
			break;
		    default:
			PE "%s: option -%c: unknown option\n", argv0, *s);
			usage = 1;
	        }
	    }
	}
	else
	    cellNames[cell_cnt++] = *argv;
nextarg:;
    }
    cellNames[cell_cnt] = NULL;

    if (*cellNames == NULL &&
	(addmasks || delmasks || tog_set || tog_unset || set_all)) {
	PE "%s: option given: no cell name(s) specified\n", argv0);
	usage = 1;
    }

    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

    if (setmasks) tog_set = 2;
    if (tog_set) set_val = 1;
    if (tog_unset) set_val = 2;

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
    xcontrol = (dmStatXData (dmproject, &buf) == 0);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    if (!process) die ();

    a_mask = (int *) calloc (process -> nomasks, sizeof (int));
    c_mask = (int *) calloc (process -> nomasks, sizeof (int));
    d_mask = (int *) calloc (process -> nomasks, sizeof (int));

    if (tog_full) {
	set_all = 2;
	for (i = 0; i < process -> nomasks; ++i) a_mask[i] = 1;
    }
    else if (addmasks || setmasks) {
	int c, istart;

	set_all = 2;
	usage = 'a';
	if ((m = addmasks)) {
again:
	istart = -1;
	s = m;
	while (*m) {
	    while (*s && *s != ',' && *s != '-') ++s;
	    if (s > m) {
		c = *s;
		*s = 0;
		for (i = 0; i < process -> nomasks; ++i) {
		    if (strcmp (m, process -> mask_name[i]) == 0) { a_mask[i] = 1; break; }
		}
		if (i >= process -> nomasks) {
		    PE "%s: option -%c: unknown mask: %s\n", argv0, usage, m);
		    die ();
		}
		if (c == '-') { if (istart < 0) istart = i; }
		else if (istart >= 0) {
		    if (istart > i) while (i < istart) a_mask[i++] = 1;
		    else            while (i > istart) a_mask[i--] = 1;
		    istart = -1;
		}
		if (!c) break;
	    }
	    else if (!*s) break;
	    m = ++s;
	}
	}
	if ((m = setmasks) && usage != 's') { usage = 's'; goto again; }
    }

    if (delmasks) {
	int c, istart = -1;

	set_all = 2;

	s = m = delmasks;
	while (*m) {
	    while (*s && *s != ',' && *s != '-') ++s;
	    if (s > m) {
		c = *s;
		*s = 0;
		for (i = 0; i < process -> nomasks; ++i) {
		    if (strcmp (m, process -> mask_name[i]) == 0) { d_mask[i] = 1; break; }
		}
		if (i >= process -> nomasks) {
		    PE "%s: option -d: unknown mask: %s\n", argv0, m);
		    PE "%s: Note: Use option -t to remove unknown masks.\n", argv0);
		    die ();
		}
		if (c == '-') { if (istart < 0) istart = i; }
		else if (istart >= 0) {
		    if (istart > i) while (i < istart) d_mask[i++] = 1;
		    else            while (i > istart) d_mask[i--] = 1;
		    istart = -1;
		}
		if (!c) break;
	    }
	    else if (!*s) break;
	    m = ++s;
	}
    }

    initIntrup ();

    if (*cellNames == NULL) {
	IMPCELL **icl;

	PO "\nmacro cells:\n\n");

	celllist = (char **)dmGetMetaDesignData (CELLLIST, dmproject, LAYOUT);
	while (*celllist) {
	    do_cell (dmproject, *celllist++, 0);
	}

	icl = (IMPCELL **)dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);
	if (icl)
	while (*icl) {
	    do_cell (dmproject, (*icl) -> alias, *icl);
	    ++icl;
	}
	PO "\n");
    }
    else {
	DM_PROJECT *proj;
	IMPCELL **impcelllist, **icl, *ic;
	int type, cannot_ic = 0;

	mode = (set_val || set_all)? ATTACH : READONLY;

	impcelllist = (IMPCELL **)dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, LAYOUT);

	while (*cellNames) {
	    cell = *cellNames++;

	    ic = 0; /* check if the cell is an imported cell */
	    if ((icl = impcelllist))
	    while (*icl) {
		if (strcmp ((*icl) -> alias, cell) == 0) { ic = *icl; break; }
		++icl;
	    }

	cellKey = NULL;
	type = ' ';

	if (xcontrol) {
	    xdata.name = cell;
	    i = dmGetCellStatus (dmproject, &xdata);
	    if (ic && (i || xdata.celltype == DM_CT_IMPORT)) {
		if ((proj = getprojkey (ic -> dmpath))) {
read_xrem:
		    xdata.name = ic->cellname;
		    i = dmGetCellStatus (proj, &xdata);
		}
	    }
	    wasMacro = 0;
	    full_macro = 0;
	    switch (xdata.celltype) {
	    case DM_CT_DEVICE:  type = 'D';
	    case DM_CT_LIBRARY: if (type != 'D') type = 'L';
		if (xdata.interfacetype == DM_IF_STRICT) break;
		wasMacro = 1;
		full_macro = (xdata.interfacetype == DM_IF_FREE);
		break;
	    case DM_CT_MACRO:
		wasMacro = 1;
		full_macro = (xdata.interfacetype != DM_IF_FREEMASKS);
	    }
	    m = masks = xdata.masks;
	    if (!xcontrol) {
		isMacro = wasMacro;
		if (full_macro) *m = 0;
		goto pr_macro;
	    }
	    if (!*m) full_macro = 1;
	}
	else { /* no xcontrol */
	    if (ic) {
		if (mode == ATTACH) cannot_ic = 1;
		if (!(proj = getprojkey (ic->dmpath))) continue;
		if (dmStatXData (proj, &buf) == 0) goto read_xrem;
		s = ic -> cellname;
	    }
	    else { proj = dmproject; s = cell; }

	    noErrMes = 1;
	    cellKey = dmCheckOut (proj, s, WORKING, DONTCARE, LAYOUT, ic? READONLY : mode);
	    noErrMes = 0;
	    if (!cellKey) {
		PO "%s: cannot read %s cell!\n", cell, ic? "remote" : "local");
		continue;
	    }

	    masks = oldmasks; *masks = 0;
	    wasMacro = 0;
	    if (dmStat (cellKey, "is_macro", &buf) == 0) {
		m = newmasks; *m = 0;
		dsp = dmOpenStream (cellKey, "is_macro", "r");
		fscanf (dsp -> dmfp, "%d%s", &wasMacro, m);
		while ((s = strtok (m, "+"))) {
		    for (i = 0; i < process -> nomasks; ++i) {
			if (strcmp (s, process -> mask_name[i]) == 0) break;
		    }
		    if (i == process -> nomasks) PE "%s: Found unknown mask: '%s' removed!\n", argv0, s);
		    else {
			if (masks != oldmasks) *masks++ = ' ';
			sprintf (masks, "%d", i);
			while (*masks) ++masks;
		    }
		    m = NULL;
		}
		dmCloseStream (dsp, COMPLETE);
	    }
	    m = masks = oldmasks;
	    if (ic) {
		isMacro = wasMacro;
		goto pr_macro;
	    }
	    full_macro = !*masks;
	}
#ifdef DEBUG
	PO "-- wasMacro=%d %s\n", wasMacro, masks);
#endif
	    isMacro = (set_val == 0)? wasMacro : (set_val == 1);
	    if (!setMasks ()) isMacro = 0;
	    m = newmasks;
#ifdef DEBUG
	PO "-- isMacro =%d %s\n", isMacro, m);
#endif
	if (set_all == 1 || isMacro != wasMacro || (full_macro && *m) || strcmp (m, masks)) {
	    if (xcontrol) {
		xdata.masks = m;
		if (isMacro) {
		    if (xdata.celltype == DM_CT_REGULAR) xdata.celltype = DM_CT_MACRO;
		    xdata.interfacetype = *m ? DM_IF_FREEMASKS : DM_IF_FREE;
		} else {
		    if (xdata.celltype == DM_CT_MACRO) xdata.celltype = DM_CT_REGULAR;
		    if (*m && (full_macro || strcmp (m, masks))) {
			xdata.interfacetype = DM_IF_FREEMASKS;
			dmPutCellStatus (dmproject, &xdata);
		    }
		    xdata.interfacetype = DM_IF_STRICT;
		}
		dmPutCellStatus (dmproject, &xdata);
	    }
	    else {
		int c = ' ';
		dsp = dmOpenStream (cellKey, "is_macro", "w");
		fprintf (dsp -> dmfp, "%d", isMacro);
		if (*m) /* not allset */
		for (i = 0; i < process -> nomasks; ++i) {
		    if (c_mask[i]) {
			if (i >= MAX_NR_MASKS) break;
			fprintf (dsp -> dmfp, "%c%s", c, process -> mask_name[i]);
			c = '+';
		    }
		}
		fprintf (dsp -> dmfp, "\n");
		dmCloseStream (dsp, COMPLETE);
	    }
#ifdef DEBUG
    PO "-- %s: WRITTEN!\n", cell);
#endif
	}
	if (xcontrol && !isMacro && !*m) m = masks;
pr_macro:
	    if (cellKey) dmCheckIn (cellKey, COMPLETE);
	    PO "%c%c %s: macro = %s", type, ic?'*':' ', cell, isMacro? "yes" : "no");
	    if (*m) {
		if (isMacro) PO " (only for masks:");
		else         PO " (freemasks are:");
		while ((s = strtok (m, " "))) {
		    if ((i = atoi(s)) < process -> nomasks)
			PO " %s", process -> mask_name[i]);
		    else
			PO " %s??", s);
		    m = NULL;
		}
		PO ")\n");
	    }
	    else PO "\n");
	}

	if (cannot_ic) PO "cannot change status of imported cells.\n");
    }

    if (notok_no) {
	for (i = 0; i < notok_no; ++i) {
	    PO "cannot open project: %s\n", proj_notok[i]);
	}
    }

    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();
    exit (0);
    return (0);
}

int setMasks ()
{
    char *m;
    int i, allset = 1, set = 0;

    if (!full_macro || tog_set == 2) {
	for (i = 0; i < process -> nomasks; ++i) {
	    c_mask[i] = a_mask[i];
	}
	if (tog_set != 2)
	for (m = masks; *m;) {
	    i = (*m - '0');
	    while (*++m && *m != ' ') i = i*10 + (*m - '0');
	    if (i >= process -> nomasks) PE "%s: Found unknown mask-nr %d: removed!\n", argv0, i);
	    else c_mask[i] = 1;
	    if (*m) ++m;
	}
	for (i = 0; i < process -> nomasks; ++i) {
	    if (!c_mask[i]) allset = 0;
	    else if (d_mask[i]) c_mask[i] = allset = 0;
	    else set = 1;
	}
    }
    else { /* full_macro */
	for (i = 0; i < process -> nomasks; ++i) {
	    if (d_mask[i]) c_mask[i] = allset = 0;
	    else c_mask[i] = set = 1;
	}
    }

    m = newmasks;
    if (set && !allset) {
	for (i = 0; i < process -> nomasks; ++i) {
	    if (c_mask[i]) {
		if (i >= MAX_NR_MASKS) {
		    PE "%s: Only the status for the first %d masks can be set!\n", argv0, MAX_NR_MASKS);
		    break;
		}
		if (m != newmasks) *m++ = ' ';
		sprintf (m, "%d", i);
		while (*m) ++m;
	    }
	}
    }
    *m = 0;
    return set;
}

void initIntrup ()
{
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
}

void int_hdl (int sig) /* interrupt handler */
{
    switch (sig) {
        case SIGILL :
            PE "Illegal instruction\n");
            break;
        case SIGFPE :
            PE "Floating point exception\n");
            break;
#ifdef SIGBUS
        case SIGBUS :
            PE "Bus error\n");
            break;
#endif
        case SIGSEGV :
            PE "Segmentation violation\n");
            break;
    }
    die ();
}

void die ()
{
    if (dmproject) dmCloseProject (dmproject, QUIT);
    dmQuit ();
    exit (1);
}

void dmError (char *s)
{
    if (!noErrMes) {
	PE "%s: ", argv0);
	dmPerror (s);
	die ();
    }
}
