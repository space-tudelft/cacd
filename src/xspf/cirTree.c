
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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

#include "src/xspf/incl.h"

extern DM_PROJECT *dmproject;

extern int alsoImport;
extern int xtree;

extern char *excl_lib[];
extern int excl_lib_cnt;

char *cannot_handle_aliases = "Cannot handle aliases when also retrieving imported cells:";

struct cir *beg_extl = NULL;
struct cir *end_extl = NULL;

static struct cir *beg_cirl;
static struct cir *end_cirl;

#ifdef __cplusplus
  extern "C" {
#endif
static void findChildren (char *name, int isroot, DM_PROJECT *proj, int fatherImported);
static struct cir *locateTopCell (char *name);
static void reportExt (char *name, int imported, DM_PROJECT *father_proj);
#ifdef __cplusplus
  }
#endif

/* cirTree returns a pointer to an array of character-pointers
   pointing to the names of cells that are in the hierarchical
   tree of the cell specified.  The order will be 'depth first',
   and the last pointer will contain 0.
*/
struct cir *cirTree (char *name)
{
    struct cir *top;

    beg_cirl = end_cirl = NULL;

    top = locateTopCell (name);

    if (xtree) findChildren (top -> orig_name, 1, top -> proj, top -> imported);

    /* add root cell */

    if (!beg_cirl)
	beg_cirl = top;
    else {
	end_cirl -> next = top;
    }

    return (beg_cirl);
}

static void findChildren (char *name, int isroot, DM_PROJECT *proj, int fatherImported)
{
    DM_CELL *dkey;
    DM_STREAM *dsp;
    struct cir *beg_childl, *end_childl, *cl, *cl2;
    int imported;

    dkey = dmCheckOut (proj, name, isroot ? WORKING : ACTUAL, DONTCARE, CIRCUIT, READONLY);

    beg_childl = end_childl = NULL;

    dsp = dmOpenStream (dkey, "mc", "r");

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	if (is_ap ()) continue;
	imported = cmc.imported;
	if (is_func ()) {
	    (void) findFunc (cmc.cell_name, imported, proj, 0);
	    continue;
	}
	if (findDev (cmc.cell_name, imported, proj)) continue;
	if (imported == IMPORTED) {
	    char *s, *t;
	    int i;
	    if (alsoImport) {
		if (excl_lib_cnt) {
		    IMPCELL *icp = dmGetImpCell (proj, cmc.cell_name, 1);
		    if (icp)
		    for (i = 0; i < excl_lib_cnt; ++i) {
			s = excl_lib[i];
			t = icp -> dmpath;
			if (*s != '/') {
			    s = ++t;
			    while (*s) if (*s++ == '/') t = s;
			    s = excl_lib[i];
			}
			while (*s && *s == *t) { ++s; ++t; }
			if (!*s) goto excl;
		    }
		}
		goto incl;
	    }
excl:
	    reportExt (cmc.cell_name, imported, proj);
	    continue;
	}
incl:
	for (cl = beg_childl; cl; cl = cl -> next) /* search local list */
	    if (strcmp (cl -> name, cmc.cell_name) == 0) goto nxt;

	/* not yet in local list, so add at the end of the local list */
	PALLOC (cl, 1, struct cir);
	if (!beg_childl) beg_childl = cl;
	else     end_childl -> next = cl;
	end_childl = cl;

	if (imported == IMPORTED) {
	    IMPCELL *icp = dmGetImpCell (proj, cmc.cell_name, 1);
	    if (strcmp (cmc.cell_name, icp -> cellname))
		fatalErr (cannot_handle_aliases, icp -> cellname);
	    cl -> proj = dmOpenProject (icp -> dmpath, PROJ_READ);
	    cl -> name = icp -> alias;
	    cl -> orig_name = icp -> cellname;
	    cl -> imported = IMPORTED;
	}
	else {
	    cl -> proj = proj;
	    cl -> orig_name = cl -> name = strsave (cmc.cell_name);
	    cl -> imported = fatherImported;
	}
	cl -> next = NULL;
nxt:;
    }

    dmCloseStream (dsp, COMPLETE);

    while ((cl = beg_childl)) {

	beg_childl = cl -> next;

	/* search global list */
	for (cl2 = beg_cirl; cl2; cl2 = cl2 -> next) {
	    if (strcmp (cl2 -> name, cl -> name) == 0) {
		if (cl2 -> proj == cl -> proj) break;
		fatalErr ("cell is defined in two different projects:", cl -> name);
	    }
	}
	if (!cl2) { /* not yet in global list */

	    /* handle possible children first */

	    findChildren (cl -> orig_name, 0, cl -> proj, cl -> imported);

	    /* add structure at the end of the global list */

	    if (!beg_cirl) beg_cirl = cl;
	    else   end_cirl -> next = cl;
	    end_cirl = cl;
	    cl -> next = NULL;
	}
	else
	    Free (cl);
    }

    dmCheckIn (dkey, COMPLETE);
}

IMPCELL *dmGetImpCell (DM_PROJECT *proj, char *name, int msg)
{
    static int view_entry = -1;
    IMPCELL **icl;

    if (view_entry < 0) view_entry = _dmValidView (CIRCUIT);

    if (!(icl = proj -> impcelllist[view_entry]))
	icl = (IMPCELL **) dmGetMetaDesignData (IMPORTEDCELLLIST, proj, CIRCUIT);
    while (*icl) {
	if (strcmp ((*icl) -> alias, name) == 0) return (*icl);
	++icl;
    }
    if (msg) fatalErr ("Cannot find imported cell:", name);
    return (*icl);
}

static struct cir *locateTopCell (char *name)
{
    IMPCELL *icp;
    struct cir *new;

    PALLOC (new, 1, struct cir);

    if ((icp = dmGetImpCell (dmproject, name, 0))) {
	new -> proj = dmOpenProject (icp -> dmpath, PROJ_READ);
	new -> name = icp -> alias;
	new -> orig_name = icp -> cellname;
	new -> imported = IMPORTED;
    }
    else {
	new -> proj = dmproject;
	new -> orig_name = new -> name = strsave (name);
	new -> imported = LOCAL;
    }
    new -> next = NULL;
    return (new);
}

void scanInst (char *name, DM_PROJECT *proj)
{
/* This routine scans the instances in order to see which functions
 * and devices are present and reads their description.
 * Also, the sub-networks are stored in beg_extl.
 * This routine is only used in `root' mode!
 * Note: Routines dmCheckOut() and dmOpenStream() don't return
 *       if there is an error, routine dmError() is called!
 */
    DM_CELL *dkey;
    DM_STREAM *dsp;
    int imported;

    dkey = dmCheckOut (proj, name, WORKING, DONTCARE, CIRCUIT, READONLY);
    dsp  = dmOpenStream (dkey, "mc", "r");

    while (dmGetDesignData (dsp, CIR_MC) > 0) {
	if (is_ap ()) continue;
	imported = cmc.imported;
	if (is_func ()) {
	    (void) findFunc (cmc.cell_name, imported, proj, 0);
	}
	else if (!findDev (cmc.cell_name, imported, proj)) {
	    reportExt (cmc.cell_name, imported, proj);
	}
    }

    dmCloseStream (dsp, COMPLETE);
    dmCheckIn (dkey, COMPLETE);
}

int is_func () /* is cmc a function? */
{
    char *a;

    if (!(a = cmc.inst_attribute)) return (0); /* no */

    if (*a == 'f') {
	if (*++a == ';' || !*a) return (1); /* yes */
	else if (*a++ == '=') return (*a != '0');
    }
    while (*a) if (*a++ == ';' && *a == 'f') {
	if (*++a == ';' || !*a) return (1); /* yes */
	else if (*a++ == '=') return (*a != '0');
    }
    return (0);
}

static void reportExt (char *name, int imported, DM_PROJECT *father_proj)
{
    struct cir *extl;

    for (extl = beg_extl; extl; extl = extl -> next) {
	if (strcmp (extl -> name, name) == 0) return;
    }

    PALLOC (extl, 1, struct cir);
    extl -> imported = imported;

    if (imported == IMPORTED) {
	IMPCELL *icp = dmGetImpCell (father_proj, name, 1);
	extl -> proj = dmOpenProject (icp -> dmpath, PROJ_READ);
	extl -> name = icp -> alias;
	extl -> orig_name = icp -> cellname;
    }
    else {
	extl -> proj = father_proj;
	extl -> orig_name = extl -> name = strsave (name);
    }
    extl -> next = NULL;

    if (end_extl)
	end_extl -> next = extl;
    else
	beg_extl = extl;
    end_extl = extl;
}
