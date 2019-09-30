/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#define ALLOCPTR(ptr,name) {\
if(!(ptr=(struct name *)malloc(sizeof(struct name))))\
die ();}

#define ALLOCARR(ptr,elmts,name) {\
if(!(ptr=(name *)malloc((elmts) * sizeof(name))))\
die ();}

#define FREE(ptr) free((char *)(ptr))

extern int exp_depth;
extern int flag_L;

struct clist { /* cell-list element structure */
    DM_CELL        *ckey;       /* cell access key */
    int            hier;        /* expand this cell */
    struct clist   *cl_next;    /* link to next element */
    struct clist   *ht_next;    /* link to next hashtable element */
};

#define HASHSIZE 256
struct clist *Hashtab[HASHSIZE];

struct clist *celllist;

int level;
int nrCells;
int noErrMes = 0;

char *cellname;
DM_PROJECT *project;

void die (void);
void trav_mctree (struct clist *pcl);

char **mktree (char *cell)
{
    DM_CELL *ckey;
    static char **namelist;
    int i;
    struct clist *pcl;

    for (i = 0; i < HASHSIZE; i++) {
	Hashtab[i] = NULL;
    }

    project = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);

    ckey = dmCheckOut (project, cell, WORKING, DONTCARE, LAYOUT, READONLY);

    /*
    ** initialize the mc-tree and cell-list
    */
    ALLOCPTR (celllist, clist);
    celllist -> ckey = ckey;
    celllist -> hier = 1;
    celllist -> cl_next = NULL;
    celllist -> ht_next = NULL;
    /* the lop-level cell is not placed in the hashtable */

    nrCells = 1;
    level = 1;

    if (exp_depth > 1) trav_mctree (celllist);

    ALLOCARR (namelist, nrCells + 1, char *);
    i = 0;
    for (pcl = celllist; pcl; pcl = pcl -> cl_next) {
	if (pcl -> hier == 1) {
	    ALLOCARR (namelist[i], (strlen (pcl -> ckey -> cell) + 1), char);
	    strcpy (namelist[i], pcl -> ckey -> cell);
	    i++;
	}
    }
    namelist[i] = NULL;

    if (project) dmCloseProject (project, QUIT);

    return (namelist);
}

/*
** traverse the mc-tree of the cell recursively
*/
void trav_mctree (struct clist *pcl)
{
    register char  *s;
    register struct clist   *clp;
    register struct clist   *c_list = 0;
    DM_PROJECT *proj;
    DM_STREAM  *fp;
    int    hashval;
    struct stat buf;

    ++level;

    fp = dmOpenStream (pcl -> ckey, "mc", "r");

    while (dmGetDesignData (fp, GEO_MC) > 0) {

	if (gmc.imported) {
	    proj = dmFindProjKey (gmc.imported, gmc.cell_name,
			pcl -> ckey -> dmproject, &cellname, LAYOUT);
	}
	else {
	    proj = pcl -> ckey -> dmproject;
	    cellname = gmc.cell_name;
	}

	s = cellname;
	hashval = 0;
	while (*s) hashval += *s++;
	hashval %= HASHSIZE;

	for (clp = Hashtab[hashval]; clp; clp = clp -> ht_next)
	    if (strcmp (cellname, clp -> ckey -> cell) == 0
		    && proj == clp -> ckey -> dmproject)
		break; /* found */

        if (!clp) {
	    /*
	    ** allocate new cell-element
	    ** add it to the hash-table
	    */
	    ALLOCPTR (clp, clist);
	    clp -> ht_next = Hashtab[hashval];
	    Hashtab[hashval] = clp;

	    clp -> ckey = dmCheckOut (proj, cellname, ACTUAL, DONTCARE, LAYOUT, READONLY);
	    if (flag_L && gmc.imported)
		clp -> hier = 0;
	    else
		clp -> hier = 1;

	    if (clp -> hier == 1) {
		if (_dmExistCell (proj, cellname, CIRCUIT)) {
		    DM_CELL *cirkey;

		    noErrMes = 1;
		    if ((cirkey = dmCheckOut (proj, cellname, ACTUAL, DONTCARE, CIRCUIT, READONLY))) {

			if (dmStat (cirkey, "devmod", &buf) == 0)

                        /* the cell is a device, do not expand the cell.
			   also, do no go into hierarchical tree */

                        clp -> hier = -1;
		    }
		    noErrMes = 0;
		}
	    }

	    if (clp -> hier == 1
		&& dmStat (clp -> ckey, "is_macro", &buf) == 0) {
		clp -> hier = 0;
	    }

	    if (clp -> hier >= 0) {
		/* add it to the cell-list
		*/
		clp -> cl_next = c_list;
		c_list = clp;
	    }

	    if (clp -> hier == 1) {
		nrCells++;
	    }
	}
    }

    dmCloseStream (fp, COMPLETE);

    while ((clp = c_list)) {
	c_list = clp -> cl_next;
	clp -> cl_next = celllist;
	celllist = clp;

	if (level < exp_depth) trav_mctree (clp);
    }

    --level;
}

void dmError (char *s)
{
    if (!noErrMes) dmPerror (s);
}

void die ()
{
    if (project) dmCloseProject (project, QUIT);
    dmQuit ();
    exit (1);
}
