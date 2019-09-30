
/*
 * ISC License
 *
 * Copyright (C) 1994-2013 by
 *	Frederik Beeftink
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

#ifdef __cplusplus
  extern "C" {
#endif
static int  getWord (FILE *fp, char buf[]);
#ifdef __cplusplus
  }
#endif

#define L_BRACE '{'
#define R_BRACE '}'
#define L_PAREN '('
#define R_PAREN ')'

char *parFileName;

struct cell_par_info    *cpi_head = NULL;
struct cell_prefix_info *pfx_head = NULL;

static FILE *fp_Ctrl;

extern char *excl_lib[];
extern int excl_lib_cnt;
extern int longPrefix;
extern int incl_model;

extern char *NPN, *PNP, *NDEP, *NMOS, *PMOS;
extern char *divider, *delimiter, *nameGND;
extern char *controlFile;
extern DM_PROJECT *dmproject;
extern struct lib_model *lm_head;

extern int nmos_bulk_defined;
extern int pmos_bulk_defined;
extern int ndep_bulk_defined;
extern int npn_bulk_defined;
extern int pnp_bulk_defined;

extern int onlySubckt;

extern float nmos_bulk;
extern float pmos_bulk;
extern float ndep_bulk;
extern float npn_bulk;
extern float pnp_bulk;

#define MAX_INC_LIBS 10
#define MAX_MAP_LIBS 10

static FILE *libName_fp[MAX_INC_LIBS];
static char *libNameTab[MAX_INC_LIBS];
static int libName_cnt = 0;

static char *libMapNmTab[MAX_MAP_LIBS];
static char *libTagNmTab[MAX_MAP_LIBS];
static int libMap_cnt = 0;
static int libTag_cnt = 0;
static int line_nr = 0;
static int sharp_ok = 0;

static char *homepath;

static void openLibrary (char *name)
{
    char buf[1024];
    FILE *fp;

    if (libName_cnt == MAX_INC_LIBS) ctrlFileErr ("Too many", "include_libraries!");

    parFileName = name;
    if (!(fp = fopen (parFileName, "r")) && parFileName[0] != '/') {
	if (!homepath) homepath = dmGetMetaDesignData (PROCPATH, dmproject, "");
	sprintf (buf, "%s/%s", homepath, parFileName);
	parFileName = buf;
	fp = fopen (parFileName, "r");
    }

    if (!fp) ctrlFileErr ("Cannot open library file", name);

    libName_fp[libName_cnt] = fp;
    libNameTab[libName_cnt] = strsave (parFileName);
    ++libName_cnt;
}

static void readLibrary ()
{
    int i;

    for (i = 0; i < libName_cnt; i++) {
	parFileName = libNameTab[i];
	scanrestart (libName_fp[i], i);
    }
}

static int readRange (struct lib_model *lm)
{
    char *t;
    int buf_cnt;
    char buf[128];
    char bf1[128];
    char bf2[128];
    int c, index = 0, k;
    int parentheses = 0;
    int range = 0;

    while ((k = getWord (fp_Ctrl, buf)) > 0) {
        if (k <= 2) {
	    if (*buf == R_PAREN) return (range);
	    t = bf2;
	    for (k = 0; buf[k]; ++k) *t++ = tolower (buf[k]);
	    *t = '\0';
	         if (strcmp (bf2, "ae") == 0) index = AE;
	    else if (strcmp (bf2, "pe") == 0) index = PE;
	    else if (strcmp (bf2, "wb") == 0) index = WB;
	    else if (strcmp (bf2, "w" ) == 0) index = WI;
	    else if (strcmp (bf2, "l" ) == 0) index = LE;
	    else if (strcmp (bf2, "v" ) == 0) index = VL;
	    else k = 3;
        }
        if (k > 2) ctrlFileErr ("model: unknown parameter", buf);

        buf[0] = '\0';
        bf1[0] = '\0';
        bf2[0] = '\0';

        while (!bf2[0]) {
	    if (buf[0]) {
	        if (!bf1[0]) strcpy (bf1, buf);
	        else         strcpy (bf2, buf);
	        buf[0] = '\0';
	    }

	    while (iscntrl ((c = getc (fp_Ctrl))) || c == ' ')
	        if (c == '\n') ++line_nr;

	    if (c == L_PAREN) {
	        ++parentheses;
	        buf_cnt = 0;
	        buf[ buf_cnt++ ] = c;
	        while (parentheses) {
		    if ((c = getc (fp_Ctrl)) == L_PAREN) ++parentheses;
		    else if (c == R_PAREN) --parentheses;
		    else if (c == '\n') ++line_nr;
		    else if (c == EOF) goto err;
		    if (!isspace (c)) buf[ buf_cnt++ ] = c;
	        }
	        buf[ buf_cnt ] = '\0';
	    }
	    else if (isdigit (c) || c == '.' || c == '$') {
	        ungetc (c, fp_Ctrl);
	        (void) getWord (fp_Ctrl, buf);
	    }
	    else {
	        if (!bf2[0]) {
		    if (!bf1[0]) goto err;
		    strcpy (bf2, bf1);
		    bf1[0] = '\0';
	        }
	        ungetc (c, fp_Ctrl);
	    }
        }

        /*
	    (1) bf2		-> typ
	    (2) bf1 bf2	-> min, max
	    (3) bf1 bf2 buf -> min, typ, max
        */
        if (buf[0]) lm -> upper[index] = strsave (buf);
        if (bf1[0]) { lm -> lower[index] = strsave (bf1); range |= 2; }
        if (!buf[0] && bf1[0]) lm -> upper[index] = strsave (bf2);
        else { lm -> typical[index] = strsave (bf2); range |= 1; }
    }
err:
    ctrlFileErr ("model:", "read error!");
    return (-1);
}

static char *cpbn = NULL;
static int cpmapped = 0;

static void add2map (char *b, char *t)
{
    int i;

    if (!cpbn) cpbn = projname (dmproject);

    if (libMap_cnt == MAX_MAP_LIBS)
	ctrlFileErr ("Too many", "map_library statements!");
    for (i = 0; i < libMap_cnt; i++) {
	if (strcmp (b, libMapNmTab[i]) == 0)
	    ctrlFileErr ("map_library:", "base_name already specified!");
    }
    libMapNmTab[ libMap_cnt++ ] = strsave (b);
    for (i = 0; i < libTag_cnt; i++) {
	if (strcmp (t, libTagNmTab[i]) == 0)
	    ctrlFileErr ("map_library:", "tag_name already specified!");
    }
    libTagNmTab[ libTag_cnt++ ] = strsave (t);

    if (!cpmapped) cpmapped = (strcmp (b, cpbn) == 0);
}

static char *rcFile;

void readControl ()
{
    char rcbuf[1024];
    char buf[256];
    char bf1[256];
    char bf2[256];
    char *s, *t;
    int c, i, k, curr_dialect;

    /* first check controlFile otherwise read default.
     */
    if (controlFile) {
	OPENR (fp_Ctrl, controlFile, 1);
	rcFile = controlFile;
    }
    else {
	rcFile = "xspicerc";
	if (!(fp_Ctrl = fopen (rcFile, "r"))) {
	    homepath = dmGetMetaDesignData (PROCPATH, dmproject, "");
	    sprintf (rcbuf, "%s/%s", homepath, rcFile);
	    rcFile = rcbuf;
	    if (!(fp_Ctrl = fopen (rcFile, "r"))) return;
	}
    }

    curr_dialect = 1;
    ungetc ('\n', fp_Ctrl); /* force line_nr increment */

    while (getWord (fp_Ctrl, buf) > 0) {

new_line:
	if (*buf == '#') { /* rest is comment or #if,#else or #endif */
	    if (sharp_ok) {
		if (strcmp (buf, "#if") == 0) {
		    i = line_nr;
		    curr_dialect = 0;
		    while (!curr_dialect) { /* try new '#if' argument */

			if (getWord (fp_Ctrl, buf) == 0) goto eof;
			if (line_nr > i) goto new_line;

			curr_dialect = isCurrentDialect (buf);
		    }
		}
		else if (strcmp (buf, "#else") == 0) {
		    curr_dialect = !curr_dialect;
		}
		else if (strcmp (buf, "#endif") == 0) {
		    curr_dialect = 1;
		}
	    }
	    while ((c = getc (fp_Ctrl)) != EOF && c != '\n');
	    ungetc ('\n', fp_Ctrl); /* force line_nr increment */
	    continue;
	}

	if (!curr_dialect) { /* skip */
	    while ((c = getc (fp_Ctrl)) != EOF && c != '\n');
	    ungetc ('\n', fp_Ctrl); /* force line_nr increment */
	    continue;
	}

	for (s = buf; *s; ++s) *s = tolower (*s);

	if (strcmp (buf, "include_library") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    if (incl_model) openLibrary (bf1);
	}
	else if (strcmp (buf, "map_library") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0 ||
		getWord (fp_Ctrl, bf2) == 0) goto read_err;
	    add2map (bf1, bf2);
	}
	else if (strcmp (buf, "as_subckt") == 0) {
            onlySubckt = 1;
        }
	else if (strcmp (buf, "hier_name_sep") == 0 ||
	         strcmp (buf, "inst_term_sep") == 0) {
	    while ((c = getc (fp_Ctrl)) == ' ' || c == '\t');
	    if (c == '\n' || c == EOF) goto read_err;
	    if (*buf == 'h') *divider = c;
	    else *delimiter = c;
        }
	else if (strcmp (buf, "name_ground") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    nameGND = strsave (bf1);
	}
	else if (strcmp (buf, "bulk") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0 ||
		getWord (fp_Ctrl, bf2) == 0) goto read_err;

	    for (s = bf1; *s; ++s) *s = tolower (*s);

	    if (strcmp (bf1, NMOS) == 0) {
		k = sscanf (bf2, "%e", &nmos_bulk);
		if (k == 1) nmos_bulk_defined = 1;
	    }
	    else if (strcmp (bf1, PMOS) == 0) {
		k = sscanf (bf2, "%e", &pmos_bulk);
		if (k == 1) pmos_bulk_defined = 1;
	    }
	    else if (strcmp (bf1, NDEP) == 0) {
		k = sscanf (bf2, "%e", &ndep_bulk);
		if (k == 1) ndep_bulk_defined = 1;
	    }
	    else if (strcmp (bf1, NPN) == 0) {
		k = sscanf (bf2, "%e", &npn_bulk);
		if (k == 1) npn_bulk_defined = 1;
	    }
	    else if (strcmp (bf1, PNP) == 0) {
		k = sscanf (bf2, "%e", &pnp_bulk);
		if (k == 1) pnp_bulk_defined = 1;
	    }
	    else {
		k = 0;
		ctrlFileErr ("bulk voltage specification for unknown device", bf1);
	    }
	    if (k != 1)
		ctrlFileErr ("incorrect bulk voltage specification for device", bf1);
	}
	else if (strcmp (buf, "model") == 0) {

	    struct lib_model *lm, *lm_new, *lm_prev;
	    int nr;

	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    PALLOC (lm_new, 1, struct lib_model);
	    lm_new -> name = strsave (bf1);
	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    if (getWord (fp_Ctrl, bf2) == 0) goto read_err;
	    for (t = bf2; *t; ++t) *t = tolower (*t);

	    lm_prev = NULL;
	    for (lm = lm_head; lm; lm = (lm_prev = lm) -> next) {
		if (strcmp (lm -> orig_name, bf1) == 0) {
		    if (strcmp (lm -> type_name, bf2))
			ctrlFileErr ("different model types are used for", bf1);
		    break;
		}
	    }
	    if (!lm) lm_new -> orig_name = strsave (bf1);

	    nr = line_nr;
	    if (getWord (fp_Ctrl, bf1) == 0 || *bf1 != L_PAREN) {
		if (line_nr == nr) goto read_err;
		k = 4;
	    }
	    else {
		k = readRange (lm_new);
		if (k > 1) k = (k == 2)? 3 : 2;
		else if (k < 1) k = 4;
	    }
	    lm_new -> model_type = k;

	    if (!lm) {
		lm_new -> type_name = findDevType (bf2);
		if (!lm_head) lm_head = lm_new;
		else  lm_prev -> next = lm_new;
	    }
	    else if (k == 4 && lm -> model_type == k) {
		P_E "Warning: skipping model statement for %s %s ()\n",
		    lm -> orig_name, lm -> type_name);
	    }
	    else {
		lm_new -> orig_name = lm -> orig_name;
		lm_new -> type_name = lm -> type_name;

		if (k < lm -> model_type) { /* insert before lm */
		    if (lm == lm_head) lm_head = lm_new;
		    else       lm_prev -> next = lm_new;
		    lm_new -> next = lm;
		}
		else { /* insert after lm */
		    while (lm -> next
			&& lm -> next -> orig_name == lm -> orig_name
			&& lm -> next -> model_type <= k) lm = lm -> next;
		    lm_new -> next = lm -> next;
		    lm -> next = lm_new;
		}
	    }
	    if (*bf1 != L_PAREN && *bf1) {
		strcpy (buf, bf1);
		goto new_line;
	    }
	}
	else if (strcmp (buf, "prefix") == 0) {

	    struct cell_prefix_info *p;

	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    if (getWord (fp_Ctrl, bf2) == 0) goto read_err;

	    PALLOC (p, 1, struct cell_prefix_info);
	    p -> cell_name = strsave (bf1);
	    bf2[7] = 0;
	    p -> prefix = strsave (bf2);
	    p -> next = pfx_head;
	    pfx_head = p;
	}
	else if (strcmp (buf, "params") == 0) {

	    struct cell_par_info *cpi;
	    struct cell_par *cp_head, *cp_last;
	    struct lib_model *lm = NULL;

	    if (getWord (fp_Ctrl, buf) == 0) goto err2;
	    if (getWord (fp_Ctrl, bf2) == 0) goto err2;
	    if (*bf2 != L_BRACE) {
		if (getWord (fp_Ctrl, bf1) == 0) goto err2;
		if (*bf1 != L_BRACE) goto err2;

		for (lm = lm_head; lm; lm = lm -> next) {
		    if (strcmp (lm -> name, bf2) == 0) {
			if (strcmp (lm -> orig_name, buf) == 0 ||
			    strcmp (lm -> type_name, buf) == 0) break;
		    }
		}
		if (!lm) {
		    P_E "Warning: skipping params statement; model %s not found!\n", bf2);
		    while (getWord (fp_Ctrl, bf1) > 0 && *bf1 != R_BRACE) ;
		    if (*bf1 != R_BRACE) goto err2;
		}
	    }
	    else *bf1 = L_BRACE;

	    cp_head = cp_last = NULL;

	    if (*bf1 == L_BRACE) {

		while (getWord (fp_Ctrl, bf1) > 0 && *bf1 != R_BRACE) {

		    if (cp_last) {
			PALLOC (cp_last -> next, 1, struct cell_par);
			cp_last = cp_last -> next;
		    }
		    else {
			PALLOC (cp_head, 1, struct cell_par);
			cp_last = cp_head;
		    }
		    cp_last -> par = NULL;
		    cp_last -> val = NULL;
		    cp_last -> par_stat = NULL;
		    cp_last -> next = NULL;

		    for (t = bf1; *t && *t != '=' && *t != '@'; t++);
		    if (*t == '=') {
			s = bf1;
			if (*s == '!' && *++s == '!' && *++s == '=') {
			    cp_last -> par = strsave ("!!=");
			}
			else { *t = 0;
			    cp_last -> par = strsave (bf1);
			}
			if (*++t == '$') ++t;
			s = t;
			while (isalnum ((int)*t) || *t == '_') ++t;
			c = *t; *t = 0;
			if (*s) cp_last -> val = strsave (s);
			if (c) { *t = c; cp_last -> par_stat = strsave (t); }
		    }
		    else {
			t = bf1;
			if (*t == '!' && *++t == '!') ++t;
			if (*bf1 == '!') {
			    c = *t;
			    *t = 0;
			    cp_last -> par = strsave (bf1);
			    *t = c;
			}
			if (*t == '$') ++t;
			s = t;
			while (isalnum ((int)*t) || *t == '_') ++t;
			c = *t; *t = 0;
			if (*s) cp_last -> val = strsave (s);
			if (c) { *t = c; cp_last -> par_stat = strsave (t); }
		    }
/*
		    fprintf (stderr, "par='%s' val='%s' par_stat='%s'\n",
			cp_last -> par ? cp_last -> par : "<nul>",
			cp_last -> val ? cp_last -> val : "<nul>",
			cp_last -> par_stat ? cp_last -> par_stat : "<nul>");
*/
		}
		if (*bf1 != R_BRACE) goto err2;
	    }

	    if (cp_head) {
		if (lm) {
		    lm -> pars = cp_head;
		    while ((lm = lm -> next)) {
			if (strcmp (lm -> name, bf2) == 0) {
			    if (strcmp (lm -> orig_name, buf) == 0 ||
				strcmp (lm -> type_name, buf) == 0)
				lm -> pars = cp_head;
			}
		    }
		}
		else {
		    PALLOC (cpi, 1, struct cell_par_info);
		    cpi -> cell_name = strsave (buf);
		    cpi -> pars = cp_head;
		    cpi -> next = cpi_head;
		    cpi_head = cpi;
		}
	    }
	}
	else if (strcmp (buf, "exclude_project") == 0) {
	    i = line_nr;
	    if (getWord (fp_Ctrl, bf1) == 0 || line_nr > i) goto read_err;
	    if (excl_lib_cnt < 40)
		excl_lib[excl_lib_cnt++] = strsave (bf1);
	}
	else if (strcmp (buf, "long_prefix") == 0) longPrefix = 1;
    }
eof:
    CLOSE (fp_Ctrl);

    if (incl_model) readLibrary ();
    return;
read_err:
    *s++ = ':';
    *s = '\0';
    ctrlFileErr (buf, "read error!");
err2:
    ctrlFileErr ("params:", "read error!");
}

#define BRACES (c == L_PAREN || c == R_PAREN || c == L_BRACE || c == R_BRACE)

static int getWord (FILE *fp, char buf[])
{
    int c, i = 0;

    sharp_ok = 0;
    c = getc (fp);
    while (iscntrl (c) || c == ' ')
	if (c == '\n') {
	    ++line_nr;
	    if ((c = getc (fp)) == '#') sharp_ok = 1;
	}
	else c = getc (fp);

    if (c != EOF) {
	buf[i++] = c;
	if (!BRACES) {
	    while ((c = getc (fp)) > ' ' && c < 127 && !BRACES) {
		buf[i++] = c;
	    }
	    if (c == '\n' || BRACES) ungetc (c, fp);
	}
    }

    buf[i] = '\0';
    return (i);
}

char *maplibname (char *s)
{
    int i;
    for (i = 0; i < libMap_cnt; i++) {
	if (strcmp (s, libMapNmTab[i]) == 0) return (libTagNmTab[i]);
    }
    return (s);
}

void ctrlFileErr (char *s1, char *s2)
{
    if (rcFile) {
	char ns1[2000];
	sprintf (ns1, "file '%s', line %d:\n   %s", rcFile, line_nr, s1);
	fatalErr (ns1, s2);
    }
    else fatalErr (s1, s2);
}
