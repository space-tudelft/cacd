/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	A.J. Schooneveld
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

int view_nr;

typedef struct _mc { /* describes info for a cell call */
    char cellname[DM_MAXNAME + 1];
    int func;
    long num;
    struct _cell *cell;
    struct _mc *next;
} MC;

typedef struct _cell { /* describes info for cell definition */
    char cellname[DM_MAXNAME + 1];
    long numcellcall;
    int shown, func;
    int imported;
    struct _mc *subcell;
    struct _cell *next, *prev, *findnext;
} CELL;

extern DM_PROJECT *project; /* project key */
extern int dflag;
extern int hflag;
extern int rflag;
extern int oneview;
extern int NoErr;

void die (int nr, char *s);
void lscells (char *cellName, char **cl, IMPCELL **icl);

#define MINNAMEWIDTH 14

static char *dev_list[] = {
    "nenh", "penh", "ndep", "cap", "res",
    "and", "nand", "or", "nor", "exor", "invert",
    NULL
};

static void display (CELL *cell, long numused, int level, int func, int minwidth);

static char **lay_cl;
static CELL *celllist = NULL;
static CELL *findlist = NULL;
static CELL unlinked;  /* used as a mark */

static DM_XDATA xdata;
static int oldproj;

static int in_lay_cl (char *name)
{
    register char **cl;

    if (!lay_cl)
	lay_cl = (char **) dmGetMetaDesignData (CELLLIST, project, LAYOUT);
    for (cl = lay_cl; *cl; ++cl) {
	if (strcmp (*cl, name) == 0) return (1); /* yes */
    }
    return (0); /* no */
}

static int in_dev_list (char *name)
{
    register char **cl;
    for (cl = dev_list; *cl; ++cl) {
	if (strcmp (*cl, name) == 0) return (1); /* yes */
    }
    return (0); /* no */
}

static MC *samelevel (char *name, MC *subcelllist, int func)
{
    register MC *tmp;

    /* cell must have proper type (func or not) and name */
    for (tmp = subcelllist; tmp; tmp = tmp -> next) {
	if (strcmp (name, tmp -> cellname) == 0 && func == tmp -> func) return tmp;
    }
    return (tmp);
}

static void insertcell (CELL *cell)
{
    static CELL *prevcell;

    if (celllist == NULL) {
	celllist = findlist = cell;
    }
    else {
	prevcell -> next = cell;
	prevcell -> findnext = cell;
	cell -> prev = prevcell;
    }
    prevcell = cell;
}

static int find_f_attr (char *src)
{
    while (*src != '\0') {
	if (*src++ == 'f') {
	    if (*src == '\0' || *src == ';')
		return (1);
	}
	while (*src != '\0' && *src != ';') ++src;
	while (*src == ';') ++src;
    }
    return (0);
}

static void readcell (char *name, char *remoteName, char *view, DM_PROJECT *dmpr)
{
    DM_CELL *dmcell;
    DM_STREAM *dmstream;
    register MC *tmp, *tmpmc;
    register CELL *tmpcell;
    int i, isfunc = 0;
    long num;
    struct stat buf;

    if (!(tmpcell = (CELL *) calloc (1, sizeof (CELL)))) {
	die (3, name);
    }

    strcpy (tmpcell -> cellname, name);
    tmpcell -> func = 0;

    if (!remoteName) {
	NoErr = 1;
	dmcell = dmCheckOut (dmpr, name, ACTUAL, DONTCARE, view, READONLY);
	NoErr = 0;
	if (!dmcell) {
	    tmpcell -> func = 128;
	    goto insert;
	}

	if (view_nr == 1) { /* layout */
	    if (!oldproj) {
		xdata.name = name;
		(void) dmGetCellStatus (project, &xdata);
		switch (xdata.celltype) {
		case DM_CT_MACRO:
		    tmpcell -> func += 8;
		case DM_CT_DEVICE:
		    tmpcell -> func += 16;
		case DM_CT_LIBRARY:
		    tmpcell -> func += 32;
		}
	    }
	    else
	    if (dmStat (dmcell, "is_macro", &buf) == 0) {
		DM_STREAM *dsp;
		int flag;

		dsp = dmOpenStream (dmcell, "is_macro", "r");
		if (fscanf (dsp -> dmfp, "%d", &flag) > 0 && flag == 1)
		    tmpcell -> func += 8;
		dmCloseStream (dsp, COMPLETE);
	    }
	}
	else if (view_nr == 2) { /* circuit */
	    if (dmStat (dmcell, "fterm", &buf) == 0) {
		tmpcell -> func += 2;
	    }
	    if (dmStat (dmcell, "devmod", &buf) == 0) {
		tmpcell -> func += 4;
	    }
	}

	if (dmStat (dmcell, "mc", &buf)) goto chkin;
	++(tmpcell -> func);
	dmstream = dmOpenStream (dmcell, "mc", "r");
	tmpmc = NULL; /* no cell calls yet */

	if (view_nr == 1) { /* layout */

	    while (dmGetDesignData (dmstream, GEO_MC) > 0) {

		/* increment cell call count */
		(tmpcell -> numcellcall)++;

		/* check if cell already called here */
		if (!(tmp = (MC *) samelevel (gmc.cell_name, tmpcell -> subcell, 0))) {

		    /* is not found in the subcelllist, insert it in the list */
		    if (!(tmp = (MC *) calloc (1, sizeof (MC)))) {
			die (3, gmc.cell_name);
		    }
		    if (tmpmc == NULL)
			tmpcell -> subcell = tmp;
		    else
			tmpmc -> next = tmp;
		    tmpmc = tmp;

		    strcpy (tmp -> cellname, gmc.cell_name);
		}

		/* record number of called cells */
		tmp -> num += (gmc.nx + 1) * (gmc.ny + 1);
	    }
	}
	else if (view_nr == 2) { /* circuit */

	    while (dmGetDesignData (dmstream, CIR_MC) > 0) {

		/* increment cell call count */
		(tmpcell -> numcellcall)++;

		/* inst_attribute used to check functional
						type of subcell */
		isfunc = find_f_attr (cmc.inst_attribute);

		/* check if cell already called here */
		if (!(tmp = (MC *) samelevel (cmc.cell_name, tmpcell -> subcell, isfunc))) {

		    /* is not found in the subcelllist,
		       insert it in the list */
		    if (!(tmp = (MC *) calloc (1, sizeof (MC)))) {
			die (3, cmc.cell_name);
		    }
		    if (tmpmc == NULL)
			tmpcell -> subcell = tmp;
		    else
			tmpmc -> next = tmp;
		    tmpmc = tmp;

		    strcpy (tmp -> cellname, cmc.cell_name);

		    /* inst_attribute used to flag functional
		    ** in type of subcell
		    */
		    tmp -> func = isfunc;
		}

		/* record number of called cells */
		if (cmc.inst_dim != 0) {
		    num = 0; /* counter for number of instances */
		    for (i = 0; i < cmc.inst_dim; ++i)
			num += (cmc.inst_upper[i] - cmc.inst_lower[i] + 1);
		    tmp -> num += num;
		}
		else
		    tmp -> num += 1; /* one instance */
	    }
	}
	else if (view_nr == 3) { /* floorplan */

	    while (dmGetDesignData (dmstream, FLP_MC) > 0) {

		/* increment cell call count */
		(tmpcell -> numcellcall)++;

		/* check if cell already called here */
		if (!(tmp = (MC *) samelevel (fmc.cell_name, tmpcell -> subcell, 0))) {

		    /* is not found in the subcelllist,
		       insert it in the list */
		    if (!(tmp = (MC *) calloc (1, sizeof (MC)))) {
			die (3, fmc.cell_name);
		    }
		    if (tmpmc == NULL)
			tmpcell -> subcell = tmp;
		    else
			tmpmc -> next = tmp;
		    tmpmc = tmp;

		    strcpy (tmp -> cellname, fmc.cell_name);
		}

		/* record number of called cells */
		tmp -> num += (fmc.nx + 1) * (fmc.ny + 1);
	    }
	}
	dmCloseStream (dmstream, QUIT);
chkin:
	dmCheckIn (dmcell, COMPLETE);
    }
    else
	tmpcell -> imported = 1;

insert:
    /* insert cell in list of cell descriptions */
    insertcell (tmpcell);
}

static CELL *findcell (char *name)
{
    register CELL *tmp;

    for (tmp = findlist; tmp; tmp = tmp -> findnext) {
	if (strcmp (name, tmp -> cellname) == 0) return tmp;
    }
    return tmp;
}

static void updatecall (CELL *cell)
{
    register CELL *tmpc;
    register MC *tmpm;

    for (tmpm = cell -> subcell; tmpm; tmpm = tmpm -> next) {
	if ((tmpc = findcell (tmpm -> cellname)) == NULL) {
	    /* cannot find cell */
	    continue;
	}
	if (tmpc -> next != &unlinked) {
	    /* cell is still in celllist, do an unlink */
	    if (tmpc != celllist) {
		if (tmpc -> prev)
		    tmpc -> prev -> next = tmpc -> next;
		if (tmpc -> next)
		    tmpc -> next -> prev = tmpc -> prev;
	    }
	    else if (tmpc -> next) {
		celllist = tmpc -> next;
		celllist -> prev = NULL;
	    }
	    tmpc -> next = &unlinked; /* mark as unlinked */
	    tmpc -> prev = NULL;
	}
	tmpm -> cell = tmpc;
    }
}

static void printtree (char *cellName)
{
    register CELL *tmpc;

    if (cellName == NULL) { /* display all */
	for (tmpc = celllist; tmpc; tmpc = tmpc -> next) {
	    display (tmpc, 0L, 1, 0, MINNAMEWIDTH);
	}
    }
    else {
	if ((tmpc = findcell (cellName))) {
	    display (tmpc, 0L, 1, 0, MINNAMEWIDTH);
	}
    }
}

static void display (CELL *cell, long numused, int level, int func, int minwidth)
{
    int len, max_len;
    register MC *tmpm;

    printf ("%*d - %-*s ",
	(level - 1) * 4 + 1, level,
	minwidth, cell -> cellname);

    if (level == 1) {
	if (cell -> imported)
	    printf (" (imported)\n");
	else if (cell -> func == 128)
	    printf (" (nocell)\n");
	else {
	    if (cell -> func & 1)
		printf (" (%ld)", cell -> numcellcall);
	    if (cell -> func & 4)
		printf (" (device model)");
	    if (cell -> func & 8)
		printf (" (macro)");
	    else if (cell -> func & 16)
		printf (" (device)");
	    else if (cell -> func & 32)
		printf (" (library)");
	    if (!(cell -> func & 1)) {
		if (cell -> func & 2)
		    printf (" (function)\n");
		else
		    printf (" (nomc)\n");
		return;
	    }
	    printf ("\n");
	}
	if (rflag) return;
    }
    else {
	printf ("%4ld ", numused);
	if (func) {
	    printf ("(function)\n");
	    return;
	}
	else {
	    if (cell -> imported)
		printf ("(imported)");
	    else if (cell -> func == 128)
		printf ("(nocell)");
	    else if (!(cell -> func & 1))
		printf ("(nomc)");
	    else
		printf ("(%ld)", cell -> numcellcall);
	}
	if (cell -> func & 4)
	    printf (" (device model)");
	if (cell -> func & 8)
	    printf (" (macro)");
	else if (cell -> func & 16)
	    printf (" (device)");
	else if (cell -> func & 32)
	    printf (" (library)");
	printf ("\n");
    }

    if (!cell -> shown) {
    cell -> shown = 1;
    max_len = MINNAMEWIDTH;
    for (tmpm = cell -> subcell; tmpm; tmpm = tmpm -> next) {
	if (tmpm -> cell) len = strlen (tmpm -> cell -> cellname);
	else len = strlen (tmpm -> cellname);
	if (len > max_len) max_len = len;
    }
    for (tmpm = cell -> subcell; tmpm; tmpm = tmpm -> next) {
	if (tmpm -> cell) {
	    display (tmpm -> cell, tmpm -> num, level + 1,
			tmpm -> func, max_len);
	}
	else {
	    switch (view_nr) {
	    case 1: /* layout */
		printf ("%*d - %-*s %4ld (nocell)\n",
		    level * 4 + 1, level + 1,
		    max_len, tmpm -> cellname, tmpm -> num);
		break;
	    case 2: /* circuit */
		if (in_dev_list (tmpm -> cellname)) {
		    if (dflag) {
			printf ("%*d - %-*s %4ld (device)%s",
			    level * 4 + 1, level + 1,
			    max_len, tmpm -> cellname, tmpm -> num,
			    (tmpm -> func) ? " (function)\n" : "\n");
		    }
		}
		else {
		    printf ("%*d - %-*s %4ld (nocell)\n",
			level * 4 + 1, level + 1,
			max_len, tmpm -> cellname, tmpm -> num);
		}
		break;
	    case 3: /* floorplan */
		printf ("%*d - %-*s %4ld ",
		    level * 4 + 1, level + 1,
		    max_len, tmpm -> cellname, tmpm -> num);
		if (in_lay_cl (tmpm -> cellname))
		    printf ("(layout)\n");
		else
		    printf ("(nocell)\n");
		break;
	    }
	}
    }
    }
}

static void freelist ()
{
    register MC   *tmpm;
    register CELL *tmpc;

    while (findlist) {
	while (findlist -> subcell) {
	    tmpm = findlist -> subcell -> next;
	    free ((char *) findlist -> subcell);
	    findlist -> subcell = tmpm;
	}
	tmpc = findlist -> findnext;
	free ((char *) findlist);
	findlist = tmpc;
    }
    celllist = NULL;
}

void lview (char *cellName, char *view, int nr)
{
    struct stat buf;
    register CELL *tmp;
    register char **celllst;
    register IMPCELL **impcelllst;

    freelist ();

    view_nr = nr;

    if (!oneview)
	printf ("\n%s:\n", view);

    celllst = (char **) dmGetMetaDesignData (CELLLIST, project, view);
    if (view_nr == 1) { /* layout */
	lay_cl = celllst;
    }

    impcelllst = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, project, view);

    if (!hflag) {
	lscells (cellName, celllst, impcelllst);
	return;
    }

    if (view_nr == 1) { /* layout */
	oldproj = dmStatXData (project, &buf);
    }

    for (; *celllst; ++celllst) {
	readcell (*celllst, NULL, view, project);
    }

    for (; *impcelllst; ++impcelllst) {
	readcell ((*impcelllst) -> alias, (*impcelllst) -> cellname, view, NULL);
    }

    /* remove the non-roots from celllist */
    for (tmp = celllist; tmp; tmp = tmp -> findnext) {
	updatecall (tmp);
    }

    printtree (cellName);
}
