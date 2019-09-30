
/*
 * ISC License
 *
 * Copyright (C) 1997-2017 by
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

extern int *Nil;
extern int incl_model;
extern int choosebulk;
extern int tog_nnod;
extern int tog_pnod;
extern int xtree;
extern int alsoImport;

extern char *cannot_handle_aliases;

extern DM_PROJECT *dmproject;

extern FILE *fp_out;

extern struct model_info *preFuncs; /* predefined functions */
extern struct model_info *Funcs; /* other functions */
struct model_info *Devs;
struct model_info *mlast;

extern struct cell_par_info *cpi_head;
extern struct cell_prefix_info *pfx_head;
extern struct lib_model *lm_head;

static FILE *fp_spicemod;
static char *fn_spicemod;

static struct cir *NoDevs;

int longPrefix = 0;
int nmos_bulk_defined = 0;
int pmos_bulk_defined = 0;
int ndep_bulk_defined = 0;
int npn_bulk_defined = 0;
int pnp_bulk_defined = 0;
int vpbulkdefined = 0;
int vnbulkdefined = 0;
int nvolt_in_lib = 0;
int pvolt_in_lib = 0;

float nmos_bulk;
float pmos_bulk;
float ndep_bulk;
float npn_bulk;
float pnp_bulk;
float vpbulk;
float vnbulk;

int lineno;
int spicemod;

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
static int getword (FILE *fp, char *buf, int onSameLine, int onNewLine);
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
    if (cpi_head) {
	struct cell_par_info *cpi;
	cpi = findCellParam (name);
	if (!cpi && m -> type_name)
	    if (strcmp (m -> type_name, name))
		cpi = findCellParam (m -> type_name);
	if (cpi) m -> pars = cpi -> pars;
    }

    if (pfx_head) {
	struct cell_prefix_info *p;
	p = findCellPrefix (name);
	if (!p && m -> type_name)
	    if (strcmp (m -> type_name, name))
		p = findCellPrefix (m -> type_name);
	if (p) {
	    if (!longPrefix) p -> prefix[1] = 0;
	    strcpy (m -> prefix, p -> prefix);
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
    DM_CELL *dkey;
    DM_STREAM *dsp;
    char keyword[256];
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
		m -> name = icp -> alias;
		m -> orig_name = name;
	    }
	    else
		m -> orig_name = m -> name = strsave (name);
	    m -> out_name = m -> name;
	    m -> imported = imported;
	    m -> proj = proj;
	    m -> dw = m -> dl = 0;
	    m -> spec_dev = 1;

	    if (mlast) mlast -> next = m;
	    else Devs = m;
	    mlast = m;
	}

	if (dmStat (dkey, "devmod", &statbuf) == 0 &&
		(dsp = dmOpenStream (dkey, "devmod", "r"))) {

	    spicemod = 0;
	    lineno = 1;

	    while (getword (dsp -> dmfp, keyword, 0, 1) > 0) {
		if (spicemod) {
		    if (strcmp (keyword, "prefix") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) {
			    buf[7] = 0;
			    strcpy (m -> prefix, buf);
			}
		    }
		    else if (strcmp (keyword, "rename") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0) {
			    m -> alias_s = strsave (buf);
			    if (getword (dsp -> dmfp, buf, 1, 0) > 0) {
				m -> alias_t = strsave (buf);
				if (strcmp (m -> alias_t, "nbulk") == 0) tog_nnod = 1;
				if (strcmp (m -> alias_t, "pbulk") == 0) tog_pnod = 1;
			    }
			}
		    }
		    else if (strcmp (keyword, "dw") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0)
			    sscanf (buf, "%e", &m -> dw);
		    }
		    else if (strcmp (keyword, "dl") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0)
			    sscanf (buf, "%e", &m -> dl);
		    }
		    else if (strcmp (keyword, "bulk") == 0) {
			if (getword (dsp -> dmfp, buf, 1, 0) > 0)
			    sscanf (buf, "%e", &m -> vbulk);
			else
			    m -> vbulk = 0.0;
			m -> vbulkdefined = 1;
		    }
		    else if (strcmp (keyword, "end") == 0) spicemod = 0;
		}
		else if (strcmp (keyword, "begin") == 0) {
		    if (getword (dsp -> dmfp, keyword, 1, 0) > 0) {
			if (strcmp (keyword, "spicemod") == 0) {
			    spicemod = 1;
			    if (m -> spec_dev) m -> spec_dev = 2; /* "begin spicemod" */
			}
		    }
		    else {
			sprintf (buf, "%d", lineno);
			fatalErr ("stream devmod: syntax error at line", buf);
		    }
		}
	    }

	    dmCloseStream (dsp, COMPLETE);
	}

	/* overrul terminals by term stream */
	dsp = dmOpenStream (dkey, "term", "r");
	termRun (m, dsp);
	dmCloseStream (dsp, COMPLETE);

	if (m -> spec_dev) {
	    if (!m -> prefix[0]) {
		m -> prefix[0] = 'x'; /* default */
		m -> type_name = NULL;
	    }
	    else {
		int p1 = tolower (m -> prefix[1]);
		switch (tolower (m -> prefix[0])) {
		case 'm':
		    m -> type_name = (p1 == 'p')? PMOS : NMOS;
		    break;
		case 'q':
		    m -> type_name = (p1 == 'p')? PNP : NPN;
		    break;
		case 'c':
		    m -> type_name = CAP;
		    break;
		case 'd':
		    m -> type_name = DIO;
		    break;
		case 'r':
		    m -> type_name = RES;
		    break;
		case 'k':
		case 'l':
		    m -> type_name = IND;
		    break;
		}
		if (!longPrefix) m -> prefix[1] = 0;
	    }
	}
    }

    if (dkey) dmCheckIn (dkey, COMPLETE);

    return (m);
}

static struct model_info *check_predef_dev (char *name)
{
    struct model_info *m = NULL;
    char *type = NULL;
    int outname = 1;

    if (strcmp (name, NENH) == 0 || strcmp (name, NDEP) == 0) {
	PALLOC (m, 1, struct model_info);
	if (strcmp (name, NENH) == 0) {
	    name = NENH;
	    m -> vbulk = (float)(nmos_bulk_defined ? nmos_bulk : 0.0);
	}
	else {
	    name = NDEP;
	    m -> vbulk = (float)(ndep_bulk_defined ? ndep_bulk : 0.0);
	}
	m -> vbulkdefined = 1;
	m -> prefix[0] = 'm';
	m -> lookForBulk = 'b';
	type = NMOS;
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, PENH) == 0) { name = PENH;
	PALLOC (m, 1, struct model_info);
	m -> vbulk = (float)(pmos_bulk_defined ? pmos_bulk : 5.0);
	m -> vbulkdefined = 1;
	m -> prefix[0] = 'm';
	m -> lookForBulk = 'b';
	type = PMOS;
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, NPN) == 0) { name = NPN;
	PALLOC (m, 1, struct model_info);
	if (npn_bulk_defined) { m -> vbulk = npn_bulk; m -> vbulkdefined = 1; }
	m -> prefix[0] = 'q';
	m -> lookForBulk = 's';
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, PNP) == 0) { name = PNP;
	PALLOC (m, 1, struct model_info);
	if (pnp_bulk_defined) { m -> vbulk = pnp_bulk; m -> vbulkdefined = 1; }
	m -> prefix[0] = 'q';
	m -> lookForBulk = 's';
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, DIO) == 0) { name = DIO;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'd';
        termstore (m, "p", 0, NULL, NULL, INOUT);
        termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, RES3) == 0) { name = RES3;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'r';
	type = RES;
	outname = 0;
        termstore (m, "p", 0, NULL, NULL, INOUT);
        termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, CAP3) == 0) { name = CAP3;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'c';
	type = CAP;
	outname = 0;
        termstore (m, "p", 0, NULL, NULL, INOUT);
        termstore (m, "n", 0, NULL, NULL, INOUT);
    }
    else if (strcmp (name, IND3) == 0) { name = IND3;
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = 'l';
	type = IND;
	outname = 0;
        termstore (m, "p", 0, NULL, NULL, INOUT);
        termstore (m, "n", 0, NULL, NULL, INOUT);
    }

    if (m) {
	m -> orig_name = m -> name = name;
	if (outname) m -> out_name = name;
	m -> type_name = type? type : name;
        m -> dw = m -> dl = 0;

        if (!Devs) Devs = m;
        else mlast -> next = m;
	mlast = m;
    }
    return (m);
}

static struct model_info *check_spice_dev (struct lib_model *lm)
{
    struct model_info *m = NULL;
    char *type = lm -> type_name;

    if (type == NMOS || type == PMOS) {
	PALLOC (m, 1, struct model_info);
	if (type == NMOS) {
	    m -> vbulk = (float)(nmos_bulk_defined ? nmos_bulk : 0.0);
	}
	else {
	    m -> vbulk = (float)(pmos_bulk_defined ? pmos_bulk : 5.0);
	}
	m -> vbulkdefined = 1;
	m -> lookForBulk = 'b';
	m -> prefix[0] = 'm';
	termstore (m, "d", 0, NULL, NULL, INOUT);
	termstore (m, "g", 0, NULL, NULL, INPUT);
	termstore (m, "s", 0, NULL, NULL, INOUT);
    }
    else if (type == NPN || type == PNP) {
	PALLOC (m, 1, struct model_info);
	if (type == NPN) {
	    if (npn_bulk_defined) { m -> vbulk = npn_bulk; m -> vbulkdefined = 1; }
	}
	else {
	    if (pnp_bulk_defined) { m -> vbulk = pnp_bulk; m -> vbulkdefined = 1; }
	}
	m -> lookForBulk = 's';
	m -> prefix[0] = 'q';
	termstore (m, "c", 0, NULL, NULL, INOUT);
	termstore (m, "b", 0, NULL, NULL, INOUT);
	termstore (m, "e", 0, NULL, NULL, INOUT);
    }
    else if (type == DIO || type == CAP || type == RES || type == IND) {
	PALLOC (m, 1, struct model_info);
	m -> prefix[0] = *type;
	termstore (m, "p", 0, NULL, NULL, INOUT);
	termstore (m, "n", 0, NULL, NULL, INOUT);
    }

    if (m) {
	m -> orig_name = m -> name = lm -> orig_name;
	m -> out_name = m -> name;
	m -> type_name = type;
        m -> dw = m -> dl = 0;
	m -> createName = lm;

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
	    if ((m = check_spice_dev (lm))) return (m);
	}
    }
    return (0);
}

void termstore (struct model_info *m, char *name, int dim, int *lower, int *upper, int type)
{
    struct term_ref *end;
    struct term_ref *tref;

    PALLOC (tref, 1, struct term_ref);
    PALLOC (tref -> t, 1, struct cirterm);

    tref -> type = type;
    tref -> t -> term_name = newStringSpace (name);
    if (dim > 0) {
	tref -> t -> term_dim = dim;
	tref -> t -> term_upper = upper;
	tref -> t -> term_lower = lower;
    }

    if ((end = m -> terms) == NULL) m -> terms = tref;
    else {
	while (end -> next) end = end -> next;
	end -> next = tref;
    }
}

void interDevs ()
{
    int c;
    char buf[256];
    struct model_info *m;

    if (incl_model) {
	fn_spicemod = (char *)dmGetMetaDesignData (PROCPATH, dmproject, "spicemod");

	OPENR (fp_spicemod, fn_spicemod, 0);

	if (fp_spicemod) {

	    /* first read comment(s) at top of file */
	    while ((c = fscanf (fp_spicemod, "%s", buf)) > 0 && *buf == '#')
		while ((c = getc (fp_spicemod)) != '\n' && c != EOF);

	    while (c > 0 && *buf != '%') {

		m = Devs;
		while (m && strcmp (m -> name, buf)) m = m -> next;

		if (m && !m -> spec_dev && !m -> createName) {
		    if (fscanf (fp_spicemod, "%s %e %e", buf, &m -> dw, &m -> dl) != 3) {
			fatalErr ("Syntax error in file", fn_spicemod);
		    }
		    m -> out_name = strsave (buf);
		}

		while ((c = getc (fp_spicemod)) != '\n' && c != EOF);

		c = fscanf (fp_spicemod, "%s", buf);
	    }

	    if (c > 0 && *buf == '%')
		while ((c = getc (fp_spicemod)) != '\n' && c != EOF);

	    if (choosebulk) {
		long fpos_remem = ftell (fp_spicemod);
		while (fscanf (fp_spicemod, "%s", buf) > 0) {
		    if (!strcmp (buf, "vnbulk") || !strcmp (buf, "VNBULK")) {
			fscanf (fp_spicemod, "%*s %*s %e", &vnbulk);
			tog_nnod = nvolt_in_lib = 1;
		    }
		    if (!strcmp (buf, "vpbulk") || !strcmp (buf, "VPBULK")) {
			fscanf (fp_spicemod, "%*s %*s %e", &vpbulk);
			tog_pnod = pvolt_in_lib = 1;
		    }
		    while ((c = getc (fp_spicemod)) != '\n' && c != EOF);
		}
		fseek (fp_spicemod, fpos_remem, 0);
	    }
	}
    }

    if (choosebulk) {

	for (m = Devs; m; m = m -> next) {

            if (m -> vbulkdefined) {

                /* we require that 1. vpbulk > vnbulk
			           2. vpbulk >  0 if !vnbulkdefined
			           3. vnbulk <= 0 if !vpbulkdefined
                */

                if (!vpbulkdefined  && !vnbulkdefined) {
                    if (m -> vbulk > 0.0) {
                        vpbulk = m -> vbulk;
			vpbulkdefined = tog_pnod = 1;
                    }
                    else {
                        vnbulk = m -> vbulk;
                        vnbulkdefined = tog_nnod = 1;
                    }
                }
                else if (!vpbulkdefined && vnbulkdefined) {
                    if (m -> vbulk > vnbulk) {
                        vpbulk = m -> vbulk;
                        vpbulkdefined = tog_pnod = 1;
                    }
                    else if (m -> vbulk < vnbulk) {
                        vpbulk = vnbulk;
                        vnbulk = m -> vbulk;
                        vpbulkdefined = tog_pnod = 1;
                    }
                }
                else if (vpbulkdefined && !vnbulkdefined) {
                    if (m -> vbulk < vpbulk) {
                        vnbulk = m -> vbulk;
                        vnbulkdefined = tog_nnod = 1;
                    }
                    else if (m -> vbulk > vpbulk) {
                        vnbulk = vpbulk;
                        vpbulk = m -> vbulk;
                        vnbulkdefined = tog_nnod = 1;
                    }
                }
                else {
                    if (m -> vbulk != vpbulk && m -> vbulk != vnbulk) {
                        sprintf (buf, "%fv, %fv and %fv", vnbulk, vpbulk, m -> vbulk);
                        fatalErr ("cannot handle more than 2 different bulk voltages;", buf);
                    }
                }
            }
	}
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

void endDevs ()
{
    DM_STREAM *dsp;
    DM_CELL *dkey;
    struct model_info *m;
    char keyword[256];
    char buf[32];
    char c1, c2, c3, c;
    int dmod = 0;

    if (incl_model) {

	/* all model defintion are included in the 'md_head' models */

	printModels ();

	/* everything below is old and must be changed sooner or later */

        /* first add model descriptions from library */

        if (fp_spicemod) {
	    while ((c = getc (fp_spicemod)) != EOF) putc (c, fp_out);
	    CLOSE (fp_spicemod);
        }
        else {
	    putc ('\n', fp_out);
	}

    /* then add model descriptions from specified devices in the database */

	for (m = Devs; m; m = m -> next) {
	    if (m -> spec_dev == 2) { /* "begin spicemod" in "devmod" */

		dkey = dmCheckOut (m -> proj, m -> orig_name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
		dsp = dmOpenStream (dkey, "devmod", "r");

		lineno = 1;

		while (getword (dsp -> dmfp, keyword, 0, 1) > 0) {

		    if (strcmp (keyword, "begin") == 0) {

			if (getword (dsp -> dmfp, keyword, 1, 0) > 0) {

			    if (strcmp (keyword, "spicemod") == 0) {

				while ((c = getc (dsp -> dmfp)) != '\n' && c != EOF);
if (dmod++ == 0)
    fprintf (fp_out, "*-----------------------begin devmod's----\n");
else
    fprintf (fp_out, "*-----------------------------------------\n");
				fprintf (fp_out, "* device %s\n", m -> name);

				c1 = c2 = c3 = '\0';
				while ((c = getc (dsp -> dmfp)) != EOF
				       && !(c1 == '\n' && c2 == 'e' && c3 == 'n' && c == 'd')) {

				    if (!(c1 == '\n' && (c2 == ' ' || c2 == '\t'))) {

					if ((c1 = c2)) putc (c1, fp_out);
				    }
				    c2 = c3;
				    c3 = c;

				}

				if (c == EOF) {
				    if (c1) putc (c1, fp_out);
				    if (c2) putc (c2, fp_out);
				    if (c3) putc (c3, fp_out);
				}
			    }
			}
		    }
		}

		dmCloseStream (dsp, COMPLETE);
		dmCheckIn (dkey, COMPLETE);
	    }
	}
if (dmod)
    fprintf (fp_out, "*-----------------------end devmod's------\n");

    }

    if (choosebulk && !pvolt_in_lib && !nvolt_in_lib) {
	if (vpbulkdefined) {
	    oprint (0, "vpbulk ");
	    nmprint (0, "pbulk", 0, Nil, Nil, 0);
	    sprintf (buf, " 0 %g\n", vpbulk);
	    oprint (0, buf);
	    oprint (0, "rpbulk ");
	    nmprint (0, "pbulk", 0, Nil, Nil, 0);
	    oprint (0, " 0 100meg\n\n");
	}
	if (vnbulkdefined) {
	    oprint (0, "vnbulk ");
	    nmprint (0, "nbulk", 0, Nil, Nil, 0);
	    sprintf (buf, " 0 %g\n", vnbulk);
	    oprint (0, buf);
	    oprint (0, "rnbulk ");
	    nmprint (0, "nbulk", 0, Nil, Nil, 0);
	    oprint (0, " 0 100meg\n\n");
	}
	if (vpbulkdefined || vnbulkdefined) oprint (0, "\n");
    }
}

static int getword (FILE *fp, char *buf, int onSameLine, int onNewLine)
{
    int c = '\0';

    if (onNewLine) {
        while ((c = getc (fp)) != '\n' && c != EOF);
	if (c == '\n') lineno++;
    }

    if (c != EOF) c = getc (fp);

    while (c != EOF) {
	while (c == ' ' || c == '\t' || c == '\r') c = getc (fp);
	if (c == '\n') {
	    if (onSameLine) {
	        ungetc (c, fp);
		return (0);
	    }
	    else {
	        c = getc (fp);
	        lineno++;
	    }
	}
	else if (spicemod && c == '*') {
	    c = getc (fp);                 /* skip comment */
	}
	else {
	    ungetc (c, fp);
	    fscanf (fp, "%s", buf);
	    return (1);
	}
    }

    return (-1);
}

void prImpDev ()
{
    prImpMod (Devs, "devices");
}
