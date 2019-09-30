
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

#include "src/xnle/incl.h"

#ifdef __cplusplus
  extern "C" {
#endif
static int getWord (FILE *fp, char buf[]);
#ifdef __cplusplus
  }
#endif

#define L_BRACE '{'
#define R_BRACE '}'
#define L_PAREN '('
#define R_PAREN ')'

struct cell_prefix_info *pfx_head = NULL;
static FILE *fp_Ctrl;

extern char *controlFile;
extern DM_PROJECT *dmproject;
extern struct lib_model *lm_head;

#define MAX_MAP_LIBS 10

static char *libMapNmTab[MAX_MAP_LIBS];
static char *libTagNmTab[MAX_MAP_LIBS];
static int libMap_cnt = 0;
static int libTag_cnt = 0;
static int line_nr = 0;
static int sharp_ok = 0;

static char *homepath;

static void add2map (char *b, char *t)
{
    int i;

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
	rcFile = "xslsrc";
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
	    /* only used for language SPICE */
	}
	else if (strcmp (buf, "map_library") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0 ||
		getWord (fp_Ctrl, bf2) == 0) goto read_err;
	    add2map (bf1, bf2);
	}
	else if (strcmp (buf, "hier_name_sep") == 0 ||
		 strcmp (buf, "inst_term_sep") == 0) {
	    while ((c = getc (fp_Ctrl)) == ' ' || c == '\t');
	    if (c == '\n' || c == EOF) goto read_err;
	    /* only used for language SPF/SPEF */
	}
	else if (strcmp (buf, "name_ground") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    /* only used for language SPICE */
	}
	else if (strcmp (buf, "bulk") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0 ||
		getWord (fp_Ctrl, bf2) == 0) goto read_err;
	}
	else if (strcmp (buf, "model") == 0) {

	    struct lib_model *lm, *lm_new, *lm_prev;

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
			P_E "Warning: different model types used for %s\n", bf1);
		    break;
		}
	    }
	    if (!lm) {
		lm_new -> orig_name = strsave (bf1);
		lm_new -> type_name = findDevType (bf2);
		if (!lm_head) lm_head = lm_new;
		else  lm_prev -> next = lm_new;
	    }
	    else {
		lm_new -> orig_name = lm -> orig_name;
		lm_new -> type_name = lm -> type_name;
	    }

	    k = line_nr;
	    if (getWord (fp_Ctrl, bf1) == 0 || *bf1 != L_PAREN) {
		if (line_nr == k) goto read_err;
		if (*bf1) {
		    strcpy (buf, bf1);
		    goto new_line;
		}
	    }
	    else {
		k = 1;
		while (k) {
		    if ((c = getc (fp_Ctrl)) == EOF) goto read_err;
		    if (c == '\n') ++line_nr;
		    else if (c == L_PAREN) ++k;
		    else if (c == R_PAREN) --k;
		}
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
	    if (getWord (fp_Ctrl, buf) == 0) goto err2;
	    if (getWord (fp_Ctrl, bf1) == 0) goto err2;
	    /* only used for language SPICE */

	    if (*bf1 != L_BRACE) {
		if (getWord (fp_Ctrl, bf1) == 0) goto err2;
		if (*bf1 != L_BRACE) goto err2;
	    }
	    while (getWord (fp_Ctrl, bf1) > 0 && *bf1 != R_BRACE) ;
	    if (*bf1 != R_BRACE) goto err2;
	    P_E "Warning: skipping params statement for %s!\n", buf);
	}
	else if (strcmp (buf, "exclude_project") == 0) {
	    if (getWord (fp_Ctrl, bf1) == 0) goto read_err;
	    P_E "Warning: skipping `%s' statement!\n", buf);
	}
    }
eof:
    CLOSE (fp_Ctrl);
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
