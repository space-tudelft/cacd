
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

#include "src/xspice/incl.h"

extern long *Nil;
extern int add_comma;
extern int add_paren;
extern int dialect;
extern int incl_model;
extern int choosebulk;
extern int tog_nnod;
extern int tog_pnod;
extern int tog_nodnbr;
extern int xtree;
extern int alsoImport;
extern int use_Pmodels;
extern int use_Qmodels;

extern char *argv0;
extern char *cannot_handle_aliases;

extern DM_PROJECT *dmproject;

extern FILE *fp_out;

extern struct model_info *preFuncs; /* predefined functions */
extern struct model_info *Funcs; /* other functions */
struct model_info *Devs;
struct model_info *mlast;

extern struct cell_par_info *cpi_head;
extern struct cell_bulk_info *bulk_head;
extern struct cell_prefix_info *no_bulks;
extern struct cell_prefix_info *pfx_head;
extern struct cell_prefix_info *rnm_head;
extern struct lib_model *lm_head;
extern struct lib_model *md_head;

static FILE *fp_spicemod;
static char *fn_spicemod;

static struct cir *NoDevs;

int longPrefix = 0;
int bulkdefined = 0;

#define UNDEF 1024

float rnbulk = 100e6; /* default */
float rpbulk = 100e6; /* default */
float vmbulk = 2.5; /* default */
float vnbulk = 0.0; /* default */
float vpbulk = 5.0; /* default */

static int c_gw, lineno, spicemod;

int   dev_type_cnt = 0;
char *dev_type_tbl[40];

char *CAP  = "c";
char *DIO  = "d";
char *RES  = "r";
char *IND  = "l";
char *CAP3 = "cap";
char *RES3 = "res";
char *IND3 = "ind";
char *NPN  = "npn";
char *PNP  = "pnp";
char *NJF  = "njf";
char *PJF  = "pjf";
char *NDEP = "ndep";
char *NENH = "nenh";
char *NMOS = "nmos";
char *PENH = "penh";
char *PMOS = "pmos";
char *NBULK = "nbulk";
char *PBULK = "pbulk";
char *FN = "spicemod";

struct spicemod_data {
    char *i_name;
    char *o_name;
    char *type;
    float dw, dl;
    int   nr;
    struct spicemod_data *next;
};
static struct spicemod_data *spicemod_list;

#ifdef __cplusplus
  extern "C" {
#endif
static struct model_info *check_spec_dev (char *name, int imported, DM_PROJECT *proj, struct model_info *mexist);
static struct model_info *check_predef_dev (char *name, char *type, int save_it);
static int getword (FILE *fp, char *buf, int onSameLine, int onNewLine);
static void read_spicemod (void);
#ifdef __cplusplus
  }
#endif

static struct cell_par_info *findCellParam (char *cell_name)
{
    struct cell_par_info *p;
    for (p = cpi_head; p; p = p -> next)
	if (strcmp (p -> cell_name, cell_name) == 0) break;
    return (p);
}

static struct cell_prefix_info *findCellPrefix (char *cell_name)
{
    struct cell_prefix_info *p;
    for (p = pfx_head; p; p = p -> next)
	if (strcmp (p -> cell_name, cell_name) == 0) break;
    return (p);
}

char *renameDev (char *name)
{
    struct cell_prefix_info *p;
    for (p = rnm_head; p; p = p -> next)
	if (strcmp (p -> cell_name, name) == 0) return (p -> prefix);
    return (name);
}

static void warning (char *msg, char *s1, char *s2, char *s3)
{
    P_E "%s: Warning: ", argv0);
    P_E msg, s1, s2, s3);
    P_E "\n");
}

char *findDevType (char *type, char *fname)
{
    int i;
    for (i = 0; i < dev_type_cnt; ++i) {
	if (strcmp (type, dev_type_tbl[i]) == 0) return (dev_type_tbl[i]);
    }
    type = strsave (type);
    if (fname) {
	warning ("unknown model type \"%s\" in %s", type, fname, "");
	if (dev_type_cnt < 40) dev_type_tbl[dev_type_cnt++] = type;
    }
    return (type);
}

char *getDevType (char *type)
{
#ifdef XSPECTRE
    if (type == NMOS) return "mos0 type=n";
    if (type == PMOS) return "mos0 type=p";
    if (type == NPN) return "bjt type=npn";
    if (type == PNP) return "bjt type=pnp";
    if (type == NJF) return "jfet type=n";
    if (type == PJF) return "jfet type=p";
    if (type == CAP) return "capacitor";
    if (type == RES) return "resistor";
    if (type == DIO) return "diode";
    if (type == IND) return "inductor";
#else
    if (dialect == PSPICE) {
	if (type == CAP) return CAP3;
	if (type == RES) return RES3;
    }
#endif
    return type;
}

void initDevs ()
{
    Devs = mlast = NULL;
    fp_spicemod = NULL;
    NoDevs = NULL;

    dev_type_cnt = 0;
    dev_type_tbl[dev_type_cnt++] = NMOS;
    dev_type_tbl[dev_type_cnt++] = PMOS;
    dev_type_tbl[dev_type_cnt++] = CAP;
    dev_type_tbl[dev_type_cnt++] = RES;
    dev_type_tbl[dev_type_cnt++] = IND;
    dev_type_tbl[dev_type_cnt++] = DIO;
    dev_type_tbl[dev_type_cnt++] = NPN;
    dev_type_tbl[dev_type_cnt++] = PNP;
    dev_type_tbl[dev_type_cnt++] = NJF;
    dev_type_tbl[dev_type_cnt++] = PJF;
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
	if (strcmp (c -> name, name) == 0) return (m); /* no device */

    /* second, check the library-models */
    for (lm = lm_head; lm; lm = lm -> next)
	if (strcmp (lm -> orig_name, name) == 0) {
	    m = check_predef_dev (lm -> orig_name, lm -> type_name, 0);
	    if (m) m -> createName = lm;
	    break;
	}

    /* third, look if the device is specified in the database */
    /* possibly add information from database (devmod stream) */
    if ((m = check_spec_dev (name, imported, proj, m)))
	goto found; /* device specified or library-model */

    /* look at spicemod devices */
    if (incl_model == 2) { read_spicemod (); incl_model = 1; }
    if (spicemod_list) {
	struct spicemod_data *p;
	for (p = spicemod_list; p; p = p -> next)
	    if (strcmp (name, p -> i_name) == 0) break;
	if (p) {
	    m = check_predef_dev (p -> i_name, p -> type, 0);
	    if (m) {
#ifndef XPSTAR
		m -> out_name = p -> o_name;
#endif
		m -> dl = p -> dl;
		m -> dw = p -> dw;
		if (p -> nr) {
		    m -> spec_dev = incl_model = 3;
		    if (p -> nr < 0) p -> nr = -p -> nr; /* positive */
		}
		goto found;
	    }
	}
    }

    /* fourth, look at predefined devices */
    if ((m = check_predef_dev (name, NULL, 1))) goto found;

    /* it is not a device */
    PALLOC (c, 1, struct cir);
    c -> name = strsave (name);
    c -> next = NoDevs;
    NoDevs = c;
    return (m);

found:
    if (!Devs) Devs = m;
    else mlast -> next = m;
    mlast = m;

    if (cpi_head) {
	struct cell_par_info *cpi;
	cpi = findCellParam (name);
	if (!cpi && m -> type_name)
	    if (strcmp (m -> type_name, name))
		cpi = findCellParam (m -> type_name);
	if (cpi) m -> pars = cpi -> pars;
    }

#ifndef XPSTAR
    if (pfx_head) {
	struct cell_prefix_info *p;
	p = findCellPrefix (name);
	if (!p && m -> type_name)
	    if (strcmp (m -> type_name, name))
		p = findCellPrefix (m -> type_name);
	if (p) {
	    int i, n;
	    char *s, *t;
	    s = p -> prefix;
	    t = m -> prefix;
	    n = longPrefix? 7 : 1;
	    for (i = 0; i < n && s[i]; ++i) t[i] = s[i];
	    t[i] = 0;
	}
    }
#endif
    if (bulk_head) {
	struct cell_bulk_info *p;
	for (p = bulk_head; p; p = p -> next)
	    if (strcmp (p -> type, name) == 0) break;
	if (!p)
	for (p = bulk_head; p; p = p -> next)
	    if (p -> type == m -> type_name) break;
	if (p) {
	    m -> vbulk = p -> volt;
	    m -> vbulkdefined = 1;
	    m -> lookForBulk = (m -> type_name == NPN || m -> type_name == PNP)? 's' : 'b';
	}
    }
    if (no_bulks) {
	struct cell_prefix_info *p;
	for (p = no_bulks; p; p = p -> next)
	    if (strcmp (p -> cell_name, name) == 0) break;
	if (!p && (name = m -> type_name))
	for (p = no_bulks; p; p = p -> next)
	    if (strcmp (p -> cell_name, name) == 0) break;
	if (p) {
	    m -> vbulkdefined = 0;
	    m -> lookForBulk = 0;
	}
    }
    return (m);
}

/* With XCONTROL the cell must be of celltype DM_CT_DEVICE to be
*  possible a special device cell.  Note that only in that case
*  is looked for the existance of a "devmod" file.
*  Without XCONTROL, there must always be looked for "devmod".
*/
static struct model_info *check_spec_dev (char *name, int imported, DM_PROJECT *proj, struct model_info *m)
{
    static float bv, dl, dw;
    struct stat statbuf;
    DM_XDATA xdata;
    DM_CELL *dkey;
    DM_STREAM *dsp;
    char buf[256], keyword[256];
    char prefix[8], *type, *alias_s, *alias_t;
    IMPCELL *icp = NULL;
    int Device = 0;

    /* check_spec_dev fails on names starting with '$' */
    if (name[0] == DSCAP_PREFIX) return (m);

    if (dmStatXData (proj, &statbuf) == 0) {
	xdata.name = name;
	dmGetCellStatus (proj, &xdata);
	if (xdata.celltype == DM_CT_DEVICE) Device = 1;
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
	    Device = 1;
	}
    }

    if (_dmExistCell (proj, name, CIRCUIT) == 1) {
	dkey = dmCheckOut (proj, name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
	/* if there is no xcontrol (old situation) existance of devmod gives device status */
	dsp = NULL;
	if (dmStat (dkey, "devmod", &statbuf) == 0) { Device = 1;
	    dsp = dmOpenStream (dkey, "devmod", "r");
	}

    if (Device) { /* database specified device */
	    /* the cell must exist else no device status */

	bv = UNDEF;
	dw = dl = 0;
	alias_s = alias_t = NULL;
	type = NULL;
	prefix[0] = 0;
	spicemod = 0;
	if (dsp) {
	    lineno = 1; c_gw = 0;
	    while (getword (dsp -> dmfp, keyword, 0, 1) > 0) { /* on new line */
		if (spicemod) {
		    if (strcmp (keyword, "prefix") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) {
			    buf[7] = 0; strcpy (prefix, buf);
			}
		    }
		    else if (strcmp (keyword, "rename") == 0) {
			if (getword (dsp -> dmfp, keyword, 1, 0) > 0) {
			    if (getword (dsp -> dmfp, buf, 1, 0) > 0) {
				alias_s = strsave (keyword);
				alias_t = strsave (buf);
			    }
			}
		    }
		    else if (strcmp (keyword, "dw") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) sscanf (buf, "%e", &dw);
		    }
		    else if (strcmp (keyword, "dl") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) sscanf (buf, "%e", &dl);
		    }
		    else if (strcmp (keyword, "bulk") == 0) { bv = 0;
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) sscanf (buf, "%e", &bv);
		    }
		    else if (strcmp (keyword, ".model") == 0 || strcmp (keyword, ".subckt") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) { /* on same line */
			    if (strcmp (name, buf)) warning ("%s/devmod/%s: %s", name, keyword, "incorrect name");
			    if (keyword[1] == 'm') {
				if (getword (dsp -> dmfp, buf, 1, 0) > 0)
				    type = findDevType (buf, "devmod");
				else warning ("%s/devmod/%s: %s", name, keyword, "missing type");
			    }
			}
			else warning ("%s/devmod/%s: %s", name, keyword, "missing name");

			if (keyword[1] == 's' && prefix[0] && prefix[0] != 'x' && prefix[0] != 'X')
			    warning ("%s/devmod/%s: %s", name, keyword, "incorrect prefix");
			spicemod = 2;
			break;
		    }
		    else if (strcmp (keyword, "end") == 0) break;
		}
		else if (strcmp (keyword, "begin") == 0) {
		    getword (dsp -> dmfp, keyword, 1, 0);
		    if (strcmp (keyword, FN) == 0) spicemod = 1;
		    else warning ("%s/devmod skipped, no \"begin %s\" found", name, FN, "");
		}
	    }
	    dmCloseStream (dsp, COMPLETE);
	}

	if (!m) { /* not predefined */
	    if (!type && prefix[0]) { /* set prefix type */
		switch (tolower (prefix[0])) {
		case 'm': type = (tolower (prefix[1]) == 'p')? PMOS : NMOS; break;
		case 'q': type = (tolower (prefix[1]) == 'p')? PNP  : NPN ; break;
		case 'j': type = (tolower (prefix[1]) == 'p')? PJF  : NJF ; break;
		case 'c': type = CAP; break;
		case 'd': type = DIO; break;
		case 'r': type = RES; break;
		case 'k':
		case 'l': type = IND; break;
		}
	    }
	    m = check_predef_dev (name, type, imported != IMPORTED);
	    if (!m) { /* unknown type */
		PALLOC (m, 1, struct model_info);
		if (!prefix[0]) strcpy (m -> prefix, "x"); /* default */
		if (icp) m -> name = icp -> alias;
		else m -> name = name = strsave (name);
		m -> orig_name = name;
		m -> out_name = m -> name;
	    }
	    else if (icp) {
		m -> name = icp -> alias;
#ifndef XPSTAR
		m -> out_name = m -> name;
#endif
	    }
	}
	    m -> imported = imported;
	    m -> proj = proj;
	    m -> spec_dev = spicemod;
	m -> dl = dl;
	m -> dw = dw;

	if (prefix[0]) { /* overrule model prefix */
	    if (!longPrefix) prefix[1] = 0;
	    strcpy (m -> prefix, prefix);
	}
	if (bv != UNDEF) { /* bulk voltage specified */
	    /* don't overrule if specified in control file */
		 if (m -> type_name == NMOS){ m -> vbulk = bv; }
	    else if (m -> type_name == PMOS){ m -> vbulk = bv; }
	    else if (m -> type_name == NPN) { m -> vbulk = bv; m -> vbulkdefined = 1; }
	    else if (m -> type_name == PNP) { m -> vbulk = bv; m -> vbulkdefined = 1; }
	    else if (m -> type_name == NJF) { m -> vbulk = bv; m -> vbulkdefined = 1; m -> lookForBulk = 'b'; }
	    else if (m -> type_name == PJF) { m -> vbulk = bv; m -> vbulkdefined = 1; m -> lookForBulk = 'b'; }
	}
	if (alias_s) {
	    m -> alias_s = alias_s;
	    m -> alias_t = alias_t;
	    if (strcmp (alias_t, NBULK) == 0) tog_nnod = 1;
	    if (strcmp (alias_t, PBULK) == 0) tog_pnod = 1;
	}

	if (dsp) { /* overrule terminals by term stream */
	    m -> terms = NULL;
	    dsp = dmOpenStream (dkey, "term", "r");
	    termRun (m, dsp);
	    dmCloseStream (dsp, COMPLETE);
	}
    }
	dmCheckIn (dkey, COMPLETE);
    }
    return (m);
}

static struct model_info *check_predef_dev (char *name, char *type, int save_it)
{
    static struct term_ref *mos_terms;
    static struct term_ref *bjt_terms;
    static struct term_ref *pn_terms;
    struct model_info *m = NULL;
    char *t2 = NULL;
    int outname = 1;

	 if (strcmp (name, NENH) == 0) { name = NENH; t2 = NMOS; }
    else if (strcmp (name, NDEP) == 0) { name = NDEP; t2 = NMOS; }
    else if (strcmp (name, PENH) == 0) { name = PENH; t2 = PMOS; }
    else if (strcmp (name, NPN)  == 0) { name = NPN ; t2 = NPN; }
    else if (strcmp (name, PNP)  == 0) { name = PNP ; t2 = PNP; }
    else if (strcmp (name, NJF)  == 0) { name = NJF ; t2 = NJF; }
    else if (strcmp (name, PJF)  == 0) { name = PJF ; t2 = PJF; }
    else if (strcmp (name, DIO)  == 0) { name = DIO ; t2 = DIO; }
    else if (strcmp (name, CAP3) == 0) { name = CAP3; t2 = CAP; outname = 0; }
    else if (strcmp (name, RES3) == 0) { name = RES3; t2 = RES; outname = 0; }
    else if (strcmp (name, IND3) == 0) { name = IND3; t2 = IND; outname = 0; }

    if (t2) {
	if (type && type != t2) warning ("%s: incorrect model type \"%s\", using \"%s\"", name, type, t2);
	type = t2;
    }

    if (type == NMOS || type == PMOS) {
	PALLOC (m, 1, struct model_info);
	m -> vbulk = type == NMOS ? vnbulk : vpbulk;
	m -> vbulkdefined = 1;
	m -> lookForBulk = 'b';
	m -> prefix[0] = 'm';
	if (mos_terms) m -> terms = mos_terms;
	else {
	    termstore (m, "d", 0, NULL, NULL, INOUT);
	    termstore (m, "g", 0, NULL, NULL, INPUT);
	    termstore (m, "s", 0, NULL, NULL, INOUT);
	    mos_terms = m -> terms;
	}
#ifdef XPSTAR
	     if (use_Pmodels) m -> out_name = type == NMOS ? "llrnd" : "llrpd";
	else if (use_Qmodels) m -> out_name = type == NMOS ? "n" : "p";
	else m -> out_name = type == NMOS ? "mn" : "mp";
#endif
    }
    else if (type == NJF || type == PJF) {
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'j';
	if (mos_terms) m -> terms = mos_terms;
	else {
	    termstore (m, "d", 0, NULL, NULL, INOUT);
	    termstore (m, "g", 0, NULL, NULL, INPUT);
	    termstore (m, "s", 0, NULL, NULL, INOUT);
	    mos_terms = m -> terms;
	}
#ifdef XPSTAR
	m -> out_name = type == NJF ? "jfn" : "jfp";
#endif
    }
    else if (type == NPN || type == PNP) {
	PALLOC (m, 1, struct model_info);
	m -> lookForBulk = 's';
	m -> prefix[0] = 'q';
	if (bjt_terms) m -> terms = bjt_terms;
	else {
	    termstore (m, "c", 0, NULL, NULL, INOUT);
	    termstore (m, "b", 0, NULL, NULL, INOUT);
	    termstore (m, "e", 0, NULL, NULL, INOUT);
	    bjt_terms = m -> terms;
	}
#ifdef XPSTAR
	m -> out_name = type == NPN ? "tn" : "tp";
#endif
    }
    else if (type == DIO || type == CAP || type == RES || type == IND) {
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = *type;
	if (pn_terms) m -> terms = pn_terms;
	else {
	    termstore (m, "p", 0, NULL, NULL, INOUT);
	    termstore (m, "n", 0, NULL, NULL, INOUT);
	    pn_terms = m -> terms;
	}
#ifdef XPSTAR
	m -> out_name = type;
#endif
    }
    if (m) {
	if (save_it && !t2) name = strsave (name);
	m -> orig_name = m -> name = name;
#ifndef XPSTAR
	if (outname) m -> out_name = name;
#endif
	m -> type_name = type;
	m -> dw = m -> dl = 0;
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

static float getf (char *s)
{
    float f = 1;

    *s = tolower(*s); --s;
    while (isalpha (*s)) { *s = tolower(*s); --s; }
    ++s;
    if (strncmp (s, "m", 1) == 0) {
	if (strncmp (s, "meg", 3) == 0) f = 1e6;
	else f = 1e-3;
    }
    else if (strncmp (s, "g", 1) == 0) f = 1e9;
    else if (strncmp (s, "k", 1) == 0) f = 1e3;
    else if (strncmp (s, "u", 1) == 0) f = 1e-6;
    return f;
}

static void read_spicemod ()
{
    char buf[256], bf2[256], *s;
    struct spicemod_data *p, *mod_last = NULL;
    int c;

    fn_spicemod = (char *)dmGetMetaDesignData (PROCPATH, dmproject, FN);
    OPENR (fp_spicemod, fn_spicemod, 0);

    if (fp_spicemod) {
	spicemod = 0; c_gw = '\n';
	*buf = 0;
	while (getword (fp_spicemod, buf, 0, 1) > 0 && *buf != '%') {
	    if (*buf == '#') continue; /* skip comment line */
	    for (p = spicemod_list; p; p = p -> next)
		if (strcmp (buf, p -> i_name) == 0) break;
	    if (p) {
		warning ("%s index %s, double entry skipped!", FN, buf, "");
		continue;
	    }
	    PALLOC (p, 1, struct spicemod_data);
	    if (!spicemod_list) spicemod_list = p; else mod_last -> next = p;
	    mod_last = p;
	    p -> o_name = p -> i_name = strsave (buf);

	    if (getword (fp_spicemod, bf2, 1, 0) <= 0) s = "outname/dw/dl";
	    else { s = NULL;
		if (strcmp (buf, bf2) != 0) p -> o_name = strsave (bf2);
		if (getword (fp_spicemod, bf2, 1, 0) <= 0) s = "dw/dl";
		else {
		    p -> dw = atof(bf2);
		    if (getword (fp_spicemod, bf2, 1, 0) <= 0) s = "dl";
		    else p -> dl = atof(bf2);
		}
	    }
	    if (s) warning ("%s index %s, missing %s", FN, buf, s);
	}

	if (*buf == '%') {
	    int nr = 0;
	    while (getword (fp_spicemod, buf, 0, 1) > 0) { /* on new line */
		c = *buf;
		if (c == '.') {
		    if (strcmp (buf, ".model") == 0) {
			++nr;
			if (getword (fp_spicemod, buf, 1, 0) <= 0) {
			    warning ("%s %s name missing!", FN, buf, "");
			    continue;
			}
			if (getword (fp_spicemod, bf2, 1, 0) <= 0) {
			    warning ("%s .model %s, type missing!", FN, buf, "");
			    s = NULL;
			}
			else s = findDevType (bf2, FN);

			mod_last = NULL;
			for (p = spicemod_list; p; p = p -> next)
			    if (strcmp (buf, p -> o_name) == 0) {
				if (p -> nr) break;
				p -> nr = -nr; /* negative */
				p -> type = s;
				mod_last = p;
			    }
			if (!mod_last) warning ("%s .model %s, %s", FN, buf,
				p? "double skipped!" : "not found in index");
		    }
		    else warning ("%s %s, expecting .model", FN, buf, "");
		}
		else if ((c == 'v' || c == 'r') &&
		    (strcmp (buf+1, PBULK) == 0 || strcmp (buf+1, NBULK) == 0)) {
		    float vb = 0;
		    if (getword (fp_spicemod, bf2, 1, 0) <= 0) {
			warning ("%s: %s: missing node", FN, buf, "");
			continue;
		    }
		    if (getword (fp_spicemod, bf2, 1, 0) <= 0) {
			warning ("%s: %s: missing second node", FN, buf, "");
			continue;
		    }
		    if (getword (fp_spicemod, bf2, 1, 0) <= 0) *bf2 = 0;
		    if (sscanf (bf2, "%e", &vb) < 1 || (vb <= 0 && c == 'r')) {
			s = *bf2? "%s: %s: incorrect %s" : "%s: %s: missing %s";
			warning (s, FN, buf, c == 'v'? "bulk voltage" : "res value");
			continue;
		    }
		    c = strlen (bf2) - 1;
		    if (isalpha (bf2[c])) vb *= getf (bf2+c);
		    if (*buf == 'v') {
			if (buf[1] == 'p') {
			    c = (bulkdefined & 2); bulkdefined |= 2; vpbulk = vb;
			} else {
			    c = (bulkdefined & 1); bulkdefined |= 1; vnbulk = vb;
			}
			if (c) warning ("%s: %s voltage twice specified", FN, buf, "");
		    }
		    else if (buf[1] == 'p') rpbulk = vb; else rnbulk = vb;
		}
	    }
	}
    }

    if (bulkdefined) { /* in spicemod */
	if (vnbulk >= vpbulk) {
	    P_E "%s: %s: Error: nbulk(%g) >= pbulk(%g) voltage\n", argv0, FN, vnbulk, vpbulk);
	    die();
	}
	bulkdefined = 3; /* both bulks defined */
    }
}

void interDevs ()
{
    char buf[256];
    struct model_info *m;
    float vb;

    for (m = Devs; m; m = m -> next) {
	if (m -> vbulkdefined) {
	    vb = m -> vbulk;
	    if (bulkdefined <= 1) {
		if (!bulkdefined) { vnbulk = vpbulk = vb; bulkdefined = 1; }
		else if (vb < vnbulk) { vnbulk = vb; bulkdefined = 2; }
		else if (vb > vpbulk) { vpbulk = vb; bulkdefined = 2; }
	    }
	    else if (vb != vnbulk && vb != vpbulk) {
		if (bulkdefined == 2) {
		    if (vb < vnbulk) { vb = vnbulk; vnbulk = m -> vbulk; }
		    if (vb > vpbulk) { vb = vpbulk; vpbulk = m -> vbulk; }
		}
		sprintf (buf, "(%g <-> %g <-> %g)", vnbulk, vb, vpbulk);
		warning ("more than two bulk voltages found! %s", buf, "", "");
	    }
	}
    }

    if (bulkdefined) {
	if (bulkdefined == 1) { /* which one? */
	    if (vpbulk > 0) { bulkdefined = 2; vmbulk = vpbulk; }
	    else vmbulk = 1;
	}
	else { bulkdefined = 3; /* both defined */
	    vmbulk = (vnbulk + vpbulk) / 2;
	}
	if (choosebulk) {
	    if (bulkdefined & 1) tog_nnod = 1;
	    if (bulkdefined & 2) tog_pnod = 1;
	}
    }
}

char *strsave (char *s)
{
    char *t;
    if (!(t = malloc (strlen (s) + 1))) cannot_die (1, 0);
    return (strcpy (t, s));
}

static void printBulk (char *b)
{
    char buf[256];
#ifdef XPSTAR
    sprintf (buf, "  e_%s (%s, 0) %g;\n", b, b, *b == 'n'? vnbulk : vpbulk);
    oprint (0, buf);
    sprintf (buf, "  r_%s (%s, 0) %g;\n", b, b, *b == 'n'? rnbulk : rpbulk);
#elif defined XSPECTRE
    char *c;
    c = add_comma? "," : "";
    sprintf (buf, "v%s (%s%s 0) vsource dc=%g\n", b, b, c, *b == 'n'? vnbulk : vpbulk);
    oprint (0, buf);
    sprintf (buf, "r%s (%s%s 0) resistor r=%g\n", b, b, c, *b == 'n'? rnbulk : rpbulk);
#else
    char *c;
    c = add_comma? "," : "";
    sprintf (buf, "v%s %s", b, add_paren? "(" : "");
    oprint (0, buf);
    nmprint (0, b, 0L, Nil, Nil, 0, tog_nodnbr);
    sprintf (buf, "%s 0%s %g\n", c, add_paren? ")" : "", *b == 'n'? vnbulk : vpbulk);
    oprint (0, buf);
    sprintf (buf, "r%s %s", b, add_paren? "(" : "");
    oprint (0, buf);
    nmprint (0, b, 0L, Nil, Nil, 0, tog_nodnbr);
    sprintf (buf, "%s 0%s %g\n", c, add_paren? ")" : "", *b == 'n'? rnbulk : rpbulk);
#endif
    oprint (0, buf);
}

static void out_param (char *k)
{
#ifdef XSPECTRE
    char *t;
    int digit = 0;
    /* check extension */
    if ((t = strchr (k, '='))) ++t; else t = k;
    if (*t == '-' || *t == '+') ++t;
    while (isdigit (*t)) { ++t; ++digit; }
    if (*t == '.') ++t;
    while (isdigit (*t)) { ++t; ++digit; }
    if (digit && *t) { /* digit found */
	*t = tolower (*t);
	if (*t == 'm') {
	    if (t[1] == 'i' || t[1] == 'I') goto ill; /* mil? */
	    else
	    if ((t[1] == 'e' || t[1] == 'E') &&
		(t[2] == 'g' || t[2] == 'G')) *t = 'M'; /* meg */
	}
	else if (*t == 'g') *t = 'G';
	else if (*t == 't') *t = 'T';
	else if (*t == 'a') { *t = 'A';
ill:	    warning ("%s: ill. scale factor in '%s'", FN, k, "");
	}
    }
#endif
    fprintf (fp_out, " %s", k);
}

void endDevs ()
{
    DM_STREAM *dsp;
    DM_CELL *dkey;
    struct model_info *m;
    char keyword[256], buf[32], *s, *t, *k, *model, *subck;
    char outbuf[2560];
    int c, n, nr, nout, dmod = 0;

    if (dialect != PSTAR && incl_model) {
#ifdef XSPECTRE
	model = "model";
	subck = "subckt";
	s = "//";
#else
	model = ".model";
	subck = ".subckt";
	s = "*";
#endif

	/* all model definitions are included in the 'md_head' models */

	printModels ();

	/* (1) add model descriptions from library */

	if (fp_spicemod) {
	if (incl_model == 3) {
	    nr = -1;
	    spicemod = c_gw = 0;
	    rewind (fp_spicemod);
	    while (getword (fp_spicemod, keyword, 0, 1) > 0) { /* on new line */
		if (nr < 0 && (*keyword != '%' || ++nr == 0)) continue;
try_model:
		if (strcmp (keyword, ".model") == 0) {
		    struct spicemod_data *p;

		    ++nr;
		    for (p = spicemod_list; p; p = p -> next)
			if (p -> nr == nr) break;
		    if (!p) continue;
		    if (getword (fp_spicemod, keyword, 1, 0) <= 0) continue;
		    if (getword (fp_spicemod, keyword, 1, 0) <= 0) continue;

if (dmod++ == 0) fprintf (fp_out, "%s-----------------------begin spicemod----", s);
		    fprintf (fp_out, "\n%s %s", model, p -> o_name);
		    k = NULL;
		    if ((t = renameDev(p -> type)) == p -> type) {
#ifdef XSPECTRE
			if (t == NMOS || t == PMOS) {
			    n = 0;
			    if (getword (fp_spicemod, keyword, 1, 0) > 0) {
				k = keyword;
				if (*k == '(') { ++n;
				    if (k[1]) ++k;
				    else if (getword (fp_spicemod, k, 1, 0) <= 0) k = NULL;
				}
			    }
			    if (k && strncmp (k, "level=", 6) == 0) {
				sprintf (buf, "mos%s", k+6); k = NULL; }
			    else strcpy (buf, "mos0");
			    fprintf (fp_out, " %s type=%c%s", renameDev(buf), *t, n? " (" : "");
			    t = NULL;
			}
			else
#endif
			t = getDevType (t);
		    }
		    if (t) fprintf (fp_out, " %s", t);
		    p -> type = NULL;
		    if (k) out_param (k);
		    while (getword (fp_spicemod, keyword, 1, 0) > 0) out_param (keyword);
		    while (getword (fp_spicemod, keyword, 0, 1) > 0) { /* on new line */
			if (*keyword == '*') continue;
			if (*keyword != '+') goto try_model;
#ifdef XSPECTRE
			fprintf (fp_out, " \\\n ");
#else
			fprintf (fp_out, "\n+");
#endif
			if (keyword[1]) out_param (keyword+1);
			while (getword (fp_spicemod, keyword, 1, 0) > 0) out_param (keyword);
		    }
		}
	    }
if (dmod) fprintf (fp_out, "\n%s-----------------------end spicemod------\n", s);
	    }
	    CLOSE (fp_spicemod);
	}

	/* (2) add model descriptions from specified devices in the database */

#ifdef XSPECTRE
	FN = "spectremod";
#endif
	dmod = 0;
	for (m = Devs; m; m = m -> next) {
	    if (m -> spec_dev == 2) { /* "begin spicemod" in "devmod" */

		dkey = dmCheckOut (m -> proj, m -> orig_name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
		dsp = dmOpenStream (dkey, "devmod", "r");

		lineno = 1; spicemod = c_gw = 0;

		while (getword (dsp -> dmfp, keyword, 0, 1) > 0) { /* on new line */
		    if (strcmp (keyword, "begin") == 0)
		    if (getword (dsp -> dmfp, keyword, 1, 0) > 0) /* on same line */
		    if (strcmp (keyword, FN) == 0) { spicemod = 1; break; }
		}
		if (!spicemod) {
		    warning ("%s/devmod: no \"begin %s\" found", m -> orig_name, FN, "");
		    m -> spec_dev = 1;
		}
		else {
		    spicemod = 0;
		    while (getword (dsp -> dmfp, keyword, 0, 1) > 0 /* on new line */
				&& strcmp (keyword, "end") != 0) {
			if (strcmp (keyword, model) == 0) {
			    struct lib_model *md;

			    nr = getword (dsp -> dmfp, keyword, 1, 0); /* on same line */
			    n  = getword (dsp -> dmfp, keyword, 1, 0); /* on same line */
			    if (nr <= 0 || n <= 0) {
				P_E "%s: Error: %s/devmod: %s %s missing\n",
				    argv0, m -> orig_name, model, nr <= 0 ? "name" : "type");
				die();
			    }
#ifdef XSPECTRE
			    if (keyword[n-1] == '\\') {
				P_E "%s: Error: %s/devmod: %s type '%s' incorrect\n",
				    argv0, m -> orig_name, model, keyword);
				die();
			    }
#endif
			    k = renameDev (keyword);
			    strcpy (outbuf, k);
			    nout = strlen (outbuf);
#ifdef XSPICE
			    if (k != keyword) { /* rename -- look for level & skip */
				if ((t = strchr (k, '=')))
				if (strncmp (t-5, "level=", 6) == 0)
				if ((n = getword (dsp -> dmfp, keyword, 1, 0)) > 0) {
				    if (strncmp (keyword, "level=", 6) != 0) {
					outbuf[nout++] = ' ';
					strcpy (outbuf+nout, keyword); nout += n;
				    }
				}
			    }
			    else if (dialect == PSPICE) {
				if (strcmp (keyword, "c") == 0) {
				    outbuf[nout++] = 'a';
				    outbuf[nout++] = 'p';
				}
				else
				if (strcmp (keyword, "r") == 0) {
				    outbuf[nout++] = 'e';
				    outbuf[nout++] = 's';
				}
			    }
#endif
			    if (m -> createName) { k = NULL;
				for (md = md_head; md; md = md -> next)
				    if (md -> mod == m && !md -> specified) { k = md -> name; break; }
			    }
			    else { k = m -> out_name; md = NULL; }

			    if (k) {
if (dmod++ == 0)
	fprintf (fp_out, "%s-----------------------begin devmod's----\n", s);
else	fprintf (fp_out, "%s-----------------------------------------\n", s);
			    fprintf (fp_out, "%s device %s\n", s, m -> name);
			    do {
				while ((n = getword (dsp -> dmfp, keyword, 1, 0)) > 0) { /* on same line */
				    outbuf[nout++] = ' ';
				    strcpy (outbuf+nout, keyword); nout += n;
				}
				outbuf[nout++] = '\n';
#ifdef XSPECTRE
				c = (outbuf[nout-2] == '\\');
#endif
				if ((n = getword (dsp -> dmfp, keyword, 0, 1)) > 0) { /* on new line */
#ifndef XSPECTRE
				    c = (keyword[0] == '+');
#endif
				    if (c) {
#ifdef XSPECTRE
					outbuf[nout++] = ' '; outbuf[nout++] = ' ';
#else
					if (n > 1) { outbuf[nout++] = '+'; keyword[0] = ' '; }
#endif
					strcpy (outbuf+nout, keyword); nout += n;
				    }
				} else c = 0;
			    } while (c);
				outbuf[nout] = '\0';
			    }

			    while (k) {
				fprintf (fp_out, "%s %s %s", model, k, outbuf);
				k = NULL;
				while (md && (md = md -> next)) {
				    if (md -> mod == m && !md -> specified) {
					k = md -> name;
					break;
				    }
				}
			    }
			    goto ready;
			}
			if (strcmp (keyword, subck) == 0) {
			    nr = n = 0;
if (dmod++ == 0)
	fprintf (fp_out, "%s-----------------------begin devmod's----\n", s);
else	fprintf (fp_out, "%s-----------------------------------------\n", s);
			    fprintf (fp_out, "%s device %s\n", s, m -> name);
			    do {
				fprintf (fp_out, "%s", keyword);
#ifdef XSPICE
				if (dialect == PSPICE) {
				    if (strcmp (keyword, ".ends") == 0) { --nr; goto endofline; }
				    if (strcmp (keyword, model) == 0) n = 1;
				    else
				    if (strcmp (keyword, subck) == 0 && ++nr > 1)
					warning ("%s/devmod: %s nesting!", m->orig_name, keyword, "");
				}
#endif
				while (getword (dsp -> dmfp, keyword, 1, 0) > 0) { /* on same line */
#ifdef XSPICE
				    if (n && ++n == 3) {
					if (strcmp (keyword, "c") == 0) strcat (keyword, "ap");
					else
					if (strcmp (keyword, "r") == 0) strcat (keyword, "es");
				    }
#endif
				    fprintf (fp_out, " %s", keyword);
				}
endofline:
				fprintf (fp_out, "\n");
			    } while (getword (dsp -> dmfp, keyword, 0, 1) > 0 /* on new line */
				&& strcmp (keyword, "end") != 0);
			    goto ready;
			}
		    }
		}
ready:
		dmCloseStream (dsp, COMPLETE);
		dmCheckIn (dkey, COMPLETE);
	    }
	}
if (dmod) fprintf (fp_out, "%s-----------------------end devmod's------\n", s);

	/* (3) add model descriptions of not specified devices */

	n = 0;
	for (m = Devs; m; m = m -> next) {
	    if (m -> spec_dev < 2 && !m -> createName) {
		if (m -> out_name) {
		    if (!n) { fprintf (fp_out, "\n"); ++n; }
		    if (!m -> type_name) t = "?";
		    else {
			t = renameDev (m -> type_name);
			if (t == m -> type_name) t = getDevType (t);
		    }
		    fprintf (fp_out, "%s%s %s %s\n", s, model, m -> out_name, t);
		}
	    }
	}
    }

    if (bulkdefined && choosebulk) {
#ifdef XPSTAR
	oprint (0, "circuit;\n");
#endif
	if (bulkdefined & 1) printBulk (NBULK);
	if (bulkdefined & 2) printBulk (PBULK);
#ifdef XPSTAR
	oprint (0, "end;\n\n");
#else
	oprint (0, "\n");
#endif
	oprint (0, "\n"); /* flush */
    }
}

static int getword (FILE *fp, char *buf, int onSameLine, int onNewLine)
{
    int n = 0;

    if (onNewLine) while (c_gw != '\n' && c_gw != EOF) c_gw = getc (fp);

    while (c_gw != EOF) {
	while (!c_gw || isspace (c_gw)) { /* skip white space */
	    if (c_gw == '\n') {
		if (onSameLine) return (0);
		lineno++; n = spicemod;
	    }
	    c_gw = getc (fp);
	}
	if (!n || c_gw != '*') break;
	n = 0;
	c_gw = getc (fp); /* skip comment */
    }
    if (c_gw == EOF) return (-1);

    n = 0;
    buf[n++] = c_gw;
    while ((c_gw = getc (fp)) != EOF && !isspace (c_gw)) if(c_gw) buf[n++] = c_gw;
    buf[n] = 0;
    return (n);
}

void prImpDev ()
{
    prImpMod (Devs, "devices");
}

static int readDevMod (DM_CELL *dkey, char *name)
{
    char keyword[256];
    DM_STREAM *dsp = dmOpenStream (dkey, "devmod", "r");

    spicemod = 0;
    lineno = 1; c_gw = 0;
    while (getword (dsp -> dmfp, keyword, 0, 1) > 0) { /* on new line */
	if (spicemod) {
	    if (strcmp (keyword, ".model") == 0) {
		warning ("%s/devmod: %s", name, "found .model, missing .subckt", "");
		break;
	    }
	    if (strcmp (keyword, ".subckt") == 0) {
		if (getword (dsp -> dmfp, keyword, 1, 0) > 0) { /* on same line */
		    if (strcmp (name, keyword)) warning ("%s/devmod: %s", name, "incorrect .subckt name", "");
		}
		else warning ("%s/devmod: %s", name, "missing .subckt name", "");
		spicemod = 2;
		break;
	    }
	    else if (strcmp (keyword, "end") == 0) break;
	}
	else if (strcmp (keyword, "begin") == 0) {
	    getword (dsp -> dmfp, keyword, 1, 0);
	    if (strcmp (keyword, FN) == 0) spicemod = 1;
	    else warning ("%s/devmod skipped, no \"begin %s\" found", name, FN, "");
	}
    }
    dmCloseStream (dsp, COMPLETE);
    return spicemod;
}

void checkFunc (DM_PROJECT *proj, struct model_info *fun)
{
    struct stat statbuf;
    char *name;
    int Device = 0;

    if (!fun -> proj) name = fun -> out_name;
    else {
	proj = fun -> proj;
	name = fun -> orig_name;
    }

    if (dmStatXData (proj, &statbuf) == 0) {
	DM_XDATA xdata;
	xdata.name = name;
	dmGetCellStatus (proj, &xdata);
	if (xdata.celltype == DM_CT_DEVICE) Device = 1;
    }

    if (Device)
    if (_dmExistCell (proj, name, CIRCUIT) == 1) {
	DM_CELL *dkey = dmCheckOut (proj, name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
	if (dmStat (dkey, "devmod", &statbuf) == 0) {
	    if (readDevMod (dkey, name) == 2) { /* .subckt found */
		struct model_info *m;
		PALLOC (m, 1, struct model_info);
		strcpy (m -> prefix, "x"); /* default */
		m -> name = fun -> name;
		m -> orig_name = name;
		m -> out_name = fun -> out_name;
		m -> proj = proj;
		m -> spec_dev = 2;

		if (!Devs) Devs = m;
		else mlast -> next = m;
		mlast = m;
	    }
	}
	dmCheckIn (dkey, COMPLETE);
    }
}
