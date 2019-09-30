
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

#include "src/xnle/incl.h"

extern DM_PROJECT *dmproject;

extern struct model_info *preFuncs; /* predefined functions */
extern struct model_info *Funcs; /* other functions */
extern struct model_info *allDevs;
struct model_info *Devs;
struct model_info *mlast;

extern struct cell_prefix_info *pfx_head;
extern struct lib_model *lm_head;

static struct cir *NoDevs;

double tox = 2.5e-8; /* default value */

int NLEnbulkdefined = 0;
int NLEpbulkdefined = 0;

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
static int getword (FILE *fp, char *buf, int onNewLine);
#ifdef __cplusplus
  }
#endif

static struct cell_prefix_info *findCellPrefix (char *name)
{
    struct cell_prefix_info *p;
    for (p = pfx_head; p; p = p -> next)
	if (strcmp (p -> cell_name, name) == 0) break;
    return (p);
}

char *findDevType (char *type)
{
    int i;
    for (i = 0; i < dev_type_cnt; ++i) {
	if (strcmp (type, dev_type_tbl[i]) == 0) return (dev_type_tbl[i]);
    }
    P_E "Warning: unknown predef. model type %s\n", type);
    type = strsave (type);
    if (dev_type_cnt < 40) dev_type_tbl[dev_type_cnt++] = type;
    return (type);
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

    struct lib_model *lm;
    struct model_info *m;
    struct cir *c;

    /* first, check it from the existing Devs list */

    for (m = Devs; m; m = m -> next)
	if (strcmp (m -> name, name) == 0) return (m); /* found */

    /* also, check if it has not been examined before */

    for (c = NoDevs; c; c = c -> next)
	if (strcmp (c -> name, name) == 0) return (m); /* not a device */

    if (allDevs) {
	struct model_info *mp = NULL;
	for (m = allDevs; m; m = m -> next) {
	    if (strcmp (m -> name, name) == 0) {
		if (mp) mp -> next = m -> next;
		else allDevs = m -> next;
		if (mlast) mlast -> next = m;
		else Devs = m;
		m -> next = NULL;
		return (mlast = m); /* found */
	    }
	    mp = m;
	}
    }

    /* second, check the library models */

    for (lm = lm_head; lm; lm = lm -> next) {
	if (strcmp (name, lm -> orig_name) == 0) {
	    if ((m = check_predef_dev (lm -> type_name))) {
		m -> orig_name = m -> name = lm -> orig_name;
		break;
	    }
	}
    }

    /* third, look if the device is specified in the database */
    /* possibly add information from database (devmod stream) */

    /* check_spec_dev fails on names starting with '$' */
    if (name[0] != DSCAP_PREFIX)
	m = check_spec_dev (name, imported, proj, m);

    /* fourth, look at predefined devices */

    if (m || (m = check_predef_dev (name))) {
	if (pfx_head) {
	    struct cell_prefix_info *p = findCellPrefix (name);
	    if (!p && lm) /* library model */
		if (strcmp (lm -> type_name, name))
		    p = findCellPrefix (lm -> type_name);
	    if (p) strcpy (m -> prefix, p -> prefix);
	}
	return (m);
    }

    /* it is not a device */

    PALLOC (c, 1, struct cir);
    c -> name = strsave (name);
    c -> next = NoDevs;
    NoDevs = c;
    return (0);
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
    char buf[256];
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
	    m -> imported = imported;
	    m -> proj = proj;

	    if (mlast) mlast -> next = m;
	    else Devs = m;
	    mlast = m;

	    if (dmStat (dkey, "devmod", &statbuf) == 0 &&
		    (dsp = dmOpenStream (dkey, "devmod", "r"))) {
		while (getword (dsp -> dmfp, buf, 1) > 0) {
		    if (strcmp (buf, "prefix") == 0) {
			if (getword (dsp -> dmfp, buf, 0) > 0) {
			    buf[7] = 0;
			    strcpy (m -> prefix, buf);
			    break;
			}
		    }
		}
		dmCloseStream (dsp, COMPLETE);
	    }
	    if (m -> prefix[0]) {
		int p1 = tolower (m -> prefix[1]);
		switch (tolower (m -> prefix[0])) {
		case 'm':
		    m -> prefix[0] = 'T';
		    m -> prefix[1] = (p1 == 'p')? 'p' : 'e';
		    m -> prefix[2] = 0;
		    break;
		case 'q':
		    m -> prefix[0] = 'T';
		    m -> prefix[1] = (p1 == 'p')? 'i' : 'h';
		    m -> prefix[2] = 0;
		    break;
		case 'c':
		    m -> prefix[0] = 'C';
		    m -> prefix[1] = '0';
		    m -> prefix[2] = 0;
		    break;
		case 'd':
		    m -> prefix[0] = 'D';
		    m -> prefix[1] = 0;
		    break;
		case 'r':
		    m -> prefix[0] = 'R';
		    m -> prefix[1] = 0;
		    break;
		case 'k':
		case 'l':
		    m -> prefix[0] = 'L';
		    m -> prefix[1] = 0;
		    break;
		}
	    }
	}

	/* overrul terminals by term stream */
	m -> terms = NULL;
	dsp = dmOpenStream (dkey, "term", "r");
	termRun (m, dsp, 0);
	dmCloseStream (dsp, COMPLETE);
    }

    if (dkey) dmCheckIn (dkey, COMPLETE);

    return (m);
}

static struct model_info *check_predef_dev (char *name)
{
    struct model_info *m = NULL;

    /* name NMOS/PMOS only for library models */

    if (name == NMOS || strcmp (name, NENH) == 0 || strcmp (name, NDEP) == 0) {
	name = name[1] == 'e' ? NENH : NDEP;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'T';
	m -> prefix[1] = 'e';
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
	NLEnbulkdefined = 1;
    }
    else if (name == PMOS || strcmp (name, PENH) == 0) { name = PENH;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'T';
	m -> prefix[1] = 'p';
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
	NLEpbulkdefined = 1;
    }
    else if (strcmp (name, NPN) == 0) { name = NPN;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'T';
	m -> prefix[1] = 'h';
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
	NLEnbulkdefined = 1;
    }
    else if (strcmp (name, PNP) == 0) { name = PNP;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'T';
	m -> prefix[1] = 'i';
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
	NLEpbulkdefined = 1;
    }
    else if (strcmp (name, DIO) == 0) { name = DIO;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'D';
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, RES) == 0) { name = RES;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'R';
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, CAP) == 0) { name = CAP;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'C';
	m -> prefix[1] = '0';
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, IND) == 0) { name = IND;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'L';
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

double slstof (char *s, int len)
{
    int c;
    double val = -1;

    c = s[len - 1];
    if (isdigit (c)) val = atof (s);
    else {
	s[len - 1] = '\0';
	switch (c) {
	case 'G': val = 1.0e+9  * atof (s); break;
	case 'M': val = 1.0e+6  * atof (s); break;
	case 'k': val = 1.0e+3  * atof (s); break;
	case 'm': val = 1.0e-3  * atof (s); break;
	case 'u': val = 1.0e-6  * atof (s); break;
	case 'n': val = 1.0e-9  * atof (s); break;
	case 'p': val = 1.0e-12 * atof (s); break;
	case 'f': val = 1.0e-15 * atof (s); break;
	case 'a': val = 1.0e-18 * atof (s); break;
	}
    }
    return (val);
}

char *strsave (char *s)
{
    char *t;
    if (!(t = malloc (strlen (s) + 1))) cannot_die (1, 0);
    return (strcpy (t, s));
}

void read_tox ()
{
    static char *fn_spicemod = NULL;
    FILE *fp_spicemod;
    char buf[256];
    char buf2[20];
    char *s;

    if (fn_spicemod) return;
    fn_spicemod = (char *)dmGetMetaDesignData (PROCPATH, dmproject, "spicemod");
    if (!fn_spicemod) fn_spicemod = "spicemod";

    OPENR (fp_spicemod, fn_spicemod, 0);

    if (fp_spicemod)
    while (fgets (buf, 256, fp_spicemod)) {
	if (*buf == '+') {
	    s = buf;
	    while (*++s && *s != 't' && *s != 'T') ;
	    if (*s == '\n' || !*s) continue;
	    if (!strncmp (s, "tox=", 4) || !strncmp (s, "TOX=", 4)) {
		sscanf (s + 4, "%s", buf2);
		tox = slstof (buf2, strlen (buf2));
		if (tox < 1e-8 || tox > 1e-6) goto r1;
		goto r2;
	    }
	}
    }
r1:
    P_E "Warning: using tox=%.1e to calc. gate-cap of transistors%s\n",
	tox, fp_spicemod ? "" : " (no spicemod)");
r2:
    if (fp_spicemod) CLOSE (fp_spicemod);
}

static int getword (FILE *fp, char *buf, int onNewLine)
{
    int c;

    if (onNewLine) {
        while ((c = getc (fp)) != '\n') if (c == EOF) return (-1);
    }
    c = getc (fp);

    while (c != EOF) {
	while (c == ' ' || c == '\t' || c == '\r') c = getc (fp);
	if (c == '\n') {
	    if (onNewLine) c = getc (fp);
	    else {
	        ungetc (c, fp);
		return (0);
	    }
	}
	else if (c == '*') c = getc (fp); /* skip comment */
	else {
	    ungetc (c, fp);
	    fscanf (fp, "%s", buf);
	    return (1);
	}
    }
    return (-1);
}
