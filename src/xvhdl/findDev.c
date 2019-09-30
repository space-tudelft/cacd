
/*
 * ISC License
 *
 * Copyright (C) 1987-2017 by
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

#include "src/xvhdl/incl.h"

extern long *Nil;
extern int xtree;
extern int alsoImport;

extern char *cannot_handle_aliases;

extern DM_PROJECT *dmproject;

extern FILE *fp_out;

extern struct lib_model *lm_head;

struct model_info *Devs;
static struct model_info *mlast;
static struct cir *NoDevs;

int   dev_type_cnt = 0;
char *dev_type_tbl[40];

char *DIO  = "d";
char *CAP  = "cap";
char *RES  = "res";
char *IND  = "ind";
char *NPN  = "npn";
char *PNP  = "pnp";
char *NDEP = "ndep";
char *NENH = "nenh";
char *NMOS = "nmos";
char *PENH = "penh";
char *PMOS = "pmos";

#ifdef __cplusplus
  extern "C" {
#endif
static struct model_info *check_spec_dev (char *name, int imported, DM_PROJECT *proj, struct model_info *mexist);
static struct model_info *check_predef_dev (char *name);
static struct model_info *check_model_dev (char *name);
#ifdef __cplusplus
  }
#endif

char *findDevType (char *type)
{
    int i;
    for (i = 0; i < dev_type_cnt; ++i) {
	if (strcmp (type, dev_type_tbl[i]) == 0) {
	    if (dev_type_tbl[i] == NMOS) return (NENH);
	    if (dev_type_tbl[i] == PMOS) return (PENH);
	    return (dev_type_tbl[i]);
	}
    }
    P_E "Warning: unknown predef. model type %s\n", type);
    type = strsave (type);
    if (dev_type_cnt < 40) dev_type_tbl[dev_type_cnt++] = type;
    return (type);
}

void initDevs ()
{
    Devs = NULL;
    NoDevs = NULL;

    dev_type_cnt = 0;
    dev_type_tbl[dev_type_cnt++] = NDEP;
    dev_type_tbl[dev_type_cnt++] = NENH;
    dev_type_tbl[dev_type_cnt++] = NMOS;
    dev_type_tbl[dev_type_cnt++] = PENH;
    dev_type_tbl[dev_type_cnt++] = PMOS;
    dev_type_tbl[dev_type_cnt++] = CAP;
    dev_type_tbl[dev_type_cnt++] = RES;
    dev_type_tbl[dev_type_cnt++] = IND;
    dev_type_tbl[dev_type_cnt++] = DIO;
    dev_type_tbl[dev_type_cnt++] = NPN;
    dev_type_tbl[dev_type_cnt++] = PNP;
}

struct model_info *findDev (char *name, int imported, DM_PROJECT *proj)
{
    /* look if the cell is a device (Devs list might be extended) */

    struct model_info *m;
    struct cir *c;

    /* first, check it from the existing Devs list */

    for (m = Devs; m; m = m -> next)
	if (strcmp (m -> name, name) == 0) return (m); /* found */

    /* also, check if it has not been examined before */

    for (c = NoDevs; c; c = c -> next)
	if (strcmp (c -> name, name) == 0) return (0); /* not a device */

    /* second, check the library-models */

    m = check_model_dev (name);

    /* third, look if the device is specified in the database */
    /* possibly add information from database (devmod stream) */

    /* check_spec_dev fails on names starting with '$' */
    if (name[0] != DSCAP_PREFIX)
        if ((m = check_spec_dev (name, imported, proj, m))) {
	    return (m); /* device is specified in the database */
			/* or device is defined by the model-definitions */
        }

    /* fourth, look at predefined devices */

    if ((m = check_predef_dev (name))) return (m);

    PALLOC (c, 1, struct cir);
    c -> name = strsave (name);
    c -> next = NoDevs;
    NoDevs = c;
    return (0); /* not a device */
}

/* With XCONTROL the cell must be of celltype DM_CT_DEVICE to be
*  possible a special device cell.  Note that only in that case
*  is looked for the existance of a "devmod" file.
*  Without XCONTROL, there must always be looked for "devmod".
*/
static struct model_info *check_spec_dev (char *name, int imported, DM_PROJECT *proj, struct model_info *m)
{
    struct stat statbuf;
    DM_CELL *dkey;
    DM_STREAM *dsp;
    IMPCELL *icp = NULL;
    int Device = 0;
    DM_XDATA xdata;

    if (dmStatXData (proj, &statbuf) == 0) {
	xdata.name = name;
	dmGetCellStatus (proj, &xdata);
	if (xdata.celltype == DM_CT_DEVICE) ++Device;
	else if (imported != IMPORTED) return (m);
	else if (xdata.celltype != DM_CT_IMPORT) return (m);
    }

    if (imported == IMPORTED) {
	icp = dmGetImpCell (proj, name, 1);
	if (xtree && alsoImport && strcmp (name, icp -> cellname))
	    fatalErr (cannot_handle_aliases, icp -> cellname);
	proj = dmOpenProject (icp -> dmpath, PROJ_READ);
	name = icp -> cellname;

	if (!Device && dmStatXData (proj, &statbuf) == 0) {
	    xdata.name = name;
	    dmGetCellStatus (proj, &xdata);
	    if (xdata.celltype != DM_CT_DEVICE) return (m);
	    ++Device;
	}
    }

    dkey = NULL;
    if (_dmExistCell (proj, name, CIRCUIT) == 1) {
	dkey = dmCheckOut (proj, name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
	/* if there is no xcontrol (old situation) existance of devmod gives device status */
	if (!Device) Device = (dmStat (dkey, "devmod", &statbuf) == 0);
    }

    if (Device && dkey) { /* database specified device */
	/* the cell must exist else no device status */

	if (!m) { /* not predefined */
	    PALLOC (m, 1, struct model_info);
	    if (imported == IMPORTED) {
		m -> imported = imported;
		m -> name = icp -> alias;
		m -> orig_name = name;
	    }
	    else
		m -> orig_name = m -> name = strsave (name);

	    if (!Devs) Devs = m;
	    else mlast -> next = m;
	    mlast = m;
	}

	/* overrul terminals by term stream */
	m -> dkey = dkey;
	readTerm (m, 0);
    }

    if (dkey) dmCheckIn (dkey, COMPLETE);

    return (m);
}

static struct model_info *check_predef_dev (char *name)
{
    struct model_info *m = NULL;

    if (strcmp (name, NENH) == 0 || strcmp (name, PENH) == 0 || strcmp (name, NDEP) == 0) {
	if (name[0] == 'p') name = PENH;
	else name = name[1] == 'e' ? NENH : NDEP;
	PALLOC (m, 1, struct model_info);
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, NPN) == 0 || strcmp (name, PNP) == 0) {
	name = name[0] == 'n' ? NPN : PNP;
	PALLOC (m, 1, struct model_info);
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, DIO) == 0 || strcmp (name, CAP) == 0
	  || strcmp (name, RES) == 0 || strcmp (name, IND) == 0) {
	switch (name[0]) {
	    case 'd': name = DIO; break;
	    case 'c': name = CAP; break;
	    case 'r': name = RES; break;
	    case 'i': name = IND; break;
	}
	PALLOC (m, 1, struct model_info);
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }

    if (m) {
	m -> orig_name = m -> name = name;

	if (!Devs) Devs = m;
	else mlast -> next = m;
	mlast = m;
    }
    return (m);
}

static struct model_info *check_model_dev (char *name)
{
    struct model_info *m;
    struct lib_model *lm;

    for (lm = lm_head; lm; lm = lm -> next) {
	if (strcmp (name, lm -> orig_name) == 0) {
	    if ((m = check_predef_dev (lm -> type_name))) {
		m -> orig_name = m -> name = lm -> orig_name;
		return (m);
	    }
	}
    }
    return (0);
}

void termstore (struct model_info *m, char *name, long dim, long *lower, long *upper, int type)
{
    struct term_ref *end;
    struct term_ref *tref;

    PALLOC (tref, 1, struct term_ref);
    PALLOC ((tref -> t), 1, struct cir_term);

    tref -> type = type;
    tref -> next = NULL;
    sprintf (tref -> t -> term_name, "%s", name);
    tref -> t -> term_dim = dim;
    tref -> t -> term_upper = upper;
    tref -> t -> term_lower = lower;

    if ((end = m -> terms) == NULL) m -> terms = tref;
    else {
	while (end -> next) end = end -> next;
	end -> next = tref;
    }
}

char *strsave (char *s)
{
    char *t;
    if (!(t = malloc (strlen (s) + 1))) cannot_die (1, 0);
    return (strcpy (t, s));
}
