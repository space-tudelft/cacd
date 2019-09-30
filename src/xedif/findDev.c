/*
 * ISC License
 *
 * Copyright (C) 1987-2017 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 *	Bastiaan Sneeuw
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

#include "src/xedif/incl.h"

extern int dialectCds;

extern char *cannot_handle_aliases;

extern DM_PROJECT *dmproject;

extern struct model_info *preFuncs; /* predefined functions */
extern struct model_info *Funcs; /* other functions */
struct model_info *Devs;
struct model_info *mlast;

extern struct lib_model *lm_head;

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

void Cds_initDevs ()
{
    char cds_buf[BUFSIZ];
    char Type[BUFSIZ];
    char Nelsis_Name[BUFSIZ];
    char Cadence_Lib[BUFSIZ];
    char Cadence_Name[BUFSIZ];
    char Terminals_Cds[BUFSIZ];
    char Terminals_Nls[BUFSIZ];
    char *cds_port, *nls_port, *p, *q;
    struct model_info *m, *mp;
    struct model_info *newdevs, *lastdev;
    struct term_ref *tref;
    FILE *proc_fp;

    if (!Devs) return;

    /* open the file with the Nelsis to CADENCE device/technology information */

    if (!(proc_fp = fopen (CADENCE_INIT_FILE, "r"))) {
	if (!(proc_fp = fopen ((char *)dmGetMetaDesignData (PROCPATH, dmproject, CADENCE_INIT_FILE), "r"))) {
		fatalErr ("Cannot open file:", CADENCE_INIT_FILE);
	}
    }

    lastdev = newdevs = NULL;

    while (fgets (cds_buf, BUFSIZ, proc_fp)) {

	/* comment starts with the '#' token and ends at the end of line */
	for (p = cds_buf; *p && *p !='#'; ++p);
	if (*p) *p = '\0'; /* strip comment */

	if (!strncmp (cds_buf, "endtable", 8)) break;

	if (sscanf (cds_buf, "%s%s%s%s%s%s", Type, Nelsis_Name, Cadence_Name,
		Cadence_Lib, Terminals_Cds, Terminals_Nls) != 6)
	    continue;

	mp = NULL;
	for (m = Devs; m; m = (mp = m) -> next) {
	    if (strcmp (m -> name, Nelsis_Name) == 0) {
		m -> out_name = strsave (Cadence_Name);
		if (lastdev && strcmp (lastdev -> lib_name, Cadence_Lib) == 0)
		    m -> lib_name = lastdev -> lib_name;
		else
		    m -> lib_name = strsave (Cadence_Lib);

		/* mark first all terminals as unused */
		for (tref = m -> terms; tref; tref = tref -> next) {
		    tref -> alter_name = NULL;
		}

		p = Terminals_Nls;
		q = Terminals_Cds;
		while (*p && *q) {
		    nls_port = p;
		    while (*p && *p != ',' && *p != '.') ++p;
		    if (*p) *p++ = '\0';

		    cds_port = q;
		    while (*q && *q != ',' && *q != '.') ++q;
		    if (*q) *q++ = '\0';

		    for (tref = m -> terms; tref; tref = tref -> next) {
			if (strcmp (tref -> t -> term_name, nls_port) == 0) /* found */
			    tref -> alter_name = strsave (cds_port);
		    }
		}

		if (*p || *q) {
		    sprintf (cds_buf, "%s: number of alias terminal names different from\nnumber of original terminal names for device",
			CADENCE_INIT_FILE);
		    fatalErr (cds_buf, Nelsis_Name);
		}

		/* put 'm' in the newdevs-list,
		   because we want to sort the devices on lib_name!
		   (only possible if CADENCE_INIT_FILE is sorted)
		*/
		if (!newdevs) newdevs = m;
		else  lastdev -> next = m;
		lastdev = m;

		/* remove 'm' from Devs-list */
		if (Devs == m) Devs = m -> next;
		else mp -> next = m -> next;

		break;
	    }
	}
    }

    for (m = Devs; m; m = m -> next) {
	P_E "-- No model specified for device %s\n", m -> name);
	m -> lib_name = "Primitives";
    }
    if (Devs) {
	P_E "This devices are not found in file %s\n", CADENCE_INIT_FILE);
	P_E "They are added to external library Primitives!\n");
    }

    if (newdevs) {
	lastdev -> next = Devs;
	Devs = newdevs;
    }

    fclose (proc_fp);
}

void initDevs ()
{
    Devs = mlast = NULL;
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
	    goto found; /* device is specified in the database */
			/* or device is defined by the model-definitions */
        }

    /* fourth, look at predefined devices */

    if ((m = check_predef_dev (name))) goto found;

    /* it is not a device */

    PALLOC (c, 1, struct cir);
    c -> name = strsave (name);
    c -> next = NoDevs;
    NoDevs = c;

    return (0);

found:
    return (m);
}

/* With XCONTROL the cell must be of celltype DM_CT_DEVICE to be
*  possible a special device cell.  Note that only in that case
*  is looked for the existance of a "devmod" file.
*  Without XCONTROL, there must always be looked for "devmod".
*/
static struct model_info *check_spec_dev (char *name, int imported, DM_PROJECT *proj, struct model_info *m)
{
    DM_CELL *dkey;
    DM_STREAM *dsp;
    struct stat statbuf;
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
		m -> name = icp -> alias;
		m -> orig_name = name;
	    }
	    else
		m -> orig_name = m -> name = strsave (name);
	    m -> out_name = m -> name;
	    m -> imported = imported;
	    m -> proj = proj;

	    if (mlast) mlast -> next = m;
	    else Devs = m;
	    mlast = m;
	}

	/* overrul terminals by term stream */
	m -> terms = NULL;
	dsp = dmOpenStream (dkey, "term", "r");
	termRun (m, dsp);
	dmCloseStream (dsp, COMPLETE);
    }

    if (dkey) dmCheckIn (dkey, COMPLETE);

    return (m);
}

static struct model_info *check_predef_dev (char *name)
{
    struct model_info *m = NULL;

    if (strcmp (name, NENH) == 0 || strcmp (name, NDEP) == 0 || strcmp (name, PENH) == 0) {
	if (name[0] == 'p') name = PENH;
	else name = name[1] == 'e' ? NENH : NDEP;
	PALLOC (m, 1, struct model_info);
	m -> param = "w l";
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
	termstore (m, "b", 0, NULL, NULL, INPUT);
    }
    else if (strcmp (name, NPN) == 0 || strcmp (name, PNP) == 0) {
	name = name[0] == 'n' ? NPN : PNP;
	PALLOC (m, 1, struct model_info);
	m -> param = "ae pe wb";
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
	m -> param = "v";
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }

    if (m) {
	m -> name      = name;
	m -> orig_name = name;
	m -> out_name  = name;

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
		m -> out_name = m -> name;
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

    /* Initializing Cadence terminals */
    tref -> alter_name = tref -> t -> term_name;

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

void printDevs ()
{
    struct model_info *m;

    if (dialectCds) {
	char *cur_lib = NULL;

	for (m = Devs; m; m = m -> next) {

	    if (cur_lib != m -> lib_name) {
		if (cur_lib) oprint (0, ")");
		oprint (0, "(external ");
		oprint (0, (cur_lib = m -> lib_name));
		oprint (0, "(edifLevel 0)");
		oprint (0, "(technology(numberDefinition))");
	    }
	    prHead (m, 1);
	    oprint (0, "))");
	}
	if (cur_lib) oprint (0, ")");
    }
    else {
	if (Devs || preFuncs || Funcs) {

	    oprint (0, "(external Primitives(edifLevel 0)");
	    oprint (0, "(technology(numberDefinition))");

	    for (m = Devs; m; m = m -> next) {
		prHead (m, 1);
		oprint (0, "))");
	    }
	    for (m = preFuncs; m; m = m -> next) {
		prHead (m, 1);
		oprint (0, "))");
	    }
	    for (m = Funcs; m; m = m -> next) {
		prHead (m, 1);
		oprint (0, "))");
	    }
	    oprint (0, ")");
	}
    }
}
