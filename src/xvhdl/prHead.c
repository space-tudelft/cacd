
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
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

extern struct node_ref *Node_list;
extern struct node_ref *Node_list_free;
extern struct node_ref *Node_list_last;

extern FILE *fp_nmp;
extern long *Nil;
extern int out_indent;
extern int verbose;

int prPortType = 0;
int currTermType = 0;

char nmpbuf[1024];
int nmplen, nmpnr;
long mapdim, mapxv[2];

void do_vhdl_warn (char *old, char *new, char *item)
{
    if (verbose) P_E "-- warning: %s name \"%s\" changed into: %s\n", item, old, new);
}

int no_vhdl_name (char *name)
{
    if (!isalpha ((int)*name)) return (1);
    while (isalnum ((int)*++name) || (*name == '_' && *(name-1) != '_')) ;
    if (*name--) return (1);
    if (*name == '_') return (1);
    return (0);
}

void mk_vhdl_name (char *p, char *q)
{
    while (!isalpha ((int)*p)) {
	if (isdigit ((int)*p)) { *q++ = 'U'; break; }
	if (!*p) { *q++ = 'U'; *q = 0; return; }
	++p;
    }
    for (*q = *p;;) {
	while (isalnum ((int)*++p) || (*p == '_' && *q != '_')) *++q = *p;
	if (!*p) break;
	if (*q != '_') *++q = '_';
    }
    if (*q++ == '_') --q;
    *q = 0;
}

char *vhdl_mapping (char **nname)
{
    char *s, *s_u, *t;
    int i;

    s = *nname;
    if (*s == 'n') { /* name mapping? */
	if ((t = find_nmp (s, 1))) {
	    *nname = t;
	    return (0);
	}
    }

    mapdim = 0;
    if (no_vhdl_name (s)) {
	s_u = NULL;
	t = s;
	while (*++t) {
	    if (*t == '_') {
		if (!mapdim) s_u = t;
		if (isdigit ((int)*++t)) {
		    i = (*t - '0');
		    while (isdigit ((int)*++t)) i = i * 10 + (*t - '0');
		    if (*t == '_') {
			if (++mapdim == 1) mapxv[0] = i;
			else mapxv[1] = i;
			if (*++t && *t != '_') mapdim = 0;
		    }
		    else mapdim = 0;
		}
		else mapdim = 0;
		--t;
	    }
	}
	if (mapdim) *s_u = 0;
	else s_u = NULL;

	if (no_vhdl_name (s)) {
	    mk_vhdl_name (s, nmpbuf);
	    *nname = nmpbuf;
	}
	return (s_u);
    }
    return (0);
}

int getnr ()
{
    char *s = nmpbuf;
    int nr = 0;
    if (*s != 'n' || nmpbuf[nmplen] != ' ') return (0);
    while (isdigit ((int)*++s)) nr = nr * 10 + (*s - '0');
    if (*s != '_') return (0);
    return (nr);
}

char *find_nmp (char *s, int dim)
{
    static long *ptr = NULL;
    char *p, *q;
    int j, k, newnr;

    if (!fp_nmp) return (0);

    p = s;
    newnr = 0;
    while (isdigit ((int)*++p)) newnr = newnr * 10 + (*p - '0');
    if (*p != '_' || newnr < 1 || newnr > nmpnr) return (0);

    if (!ptr) {
	ptr = (long *) malloc (nmpnr * sizeof (long));
	if (!ptr) fatalErr ("cannot alloc memory", NULL);
	ptr[k = 0] = 0;
	fseek (fp_nmp, ptr[k], 0); /* rewind */
	while (++k < nmpnr) {
	    while ((j = getc (fp_nmp)) != '\n')
		if (j == EOF) fatalErr ("premature eof in file.nmp", NULL);
	    ptr[k] = ftell (fp_nmp);
	}
    }

    fseek (fp_nmp, ptr[newnr - 1], 0);
    if (!fgets (nmpbuf, 1024, fp_nmp)) fatalErr ("cannot read file.nmp", NULL);
    if (getnr () != newnr) fatalErr ("illegal file.nmp format", NULL);

    p = nmpbuf + nmplen;
    *p = 0;
    if (strcmp (s, nmpbuf)) return (0); /* not equal */
    s = ++p;
    while (*++p); /* goto end of buffer */
    if (*--p != '\n') fatalErr ("too long line in file.nmp", NULL);
    *p = 0;
    if (dim) { /* request to strip possible array indices */
	mapdim = 0;
	while (*--p == '_') {
	    if (isdigit ((int)*--p)) {
		j = (*p - '0');
		k = 10;
		while (isdigit ((int)*--p)) {
		    j += k * (*p - '0');
		    k *= 10;
		}
		if (*p == '_') {
		    *p = 0;
		    if (mapdim) mapxv[1] = mapxv[0];
		    mapxv[0] = j;
		    if (++mapdim < 2) continue;
		}
	    }
	    break;
	}
    }
    /* strip illegal vhdl characters from string */
    p = s;
    while (!isalpha ((int)*p)) {
	if (isdigit ((int)*p)) { s = p; *--s = 'U'; break; }
	if (!*p) { s = --p; *s = 'U'; return (s); }
	++p;
    }
    for (q = p;;) {
	while (isalnum ((int)*++p) || (*p == '_' && *q != '_')) {
	    if (++q != p) *q = *p;
	}
	if (!*p) break;
	if (*q != '_') *++q = '_';
    }
    if (*q++ == '_') --q;
    *q = 0;
    return (s); /* vhdl string */
}

int test_nmp ()
{
    long offset, pos;
    char *s;

    /* read 1st line */
    if (!fgets (nmpbuf, 1024, fp_nmp)) return (1);
    s = nmpbuf;
    nmpnr = 0;
    if (*s != 'n') return (1);
    while (isdigit ((int)*++s)) nmpnr = nmpnr * 10 + (*s - '0');
    if (*s != '_' || nmpnr != 1) return (1);
    while (*++s != ' ') if (!*s || *s == '\n') return (1);
    nmplen = s - nmpbuf;

    /* read 2nd line */
    if (!fgets (nmpbuf, 1024, fp_nmp)) return (0);
    if ((nmpnr = getnr ()) != 2) return (1);
    pos = ftell (fp_nmp);
    offset = -pos;
again:
    fseek (fp_nmp, offset, 2); /* end of file */
    if (ftell (fp_nmp) <= 0) return (0);
    if (fgets (nmpbuf, 1024, fp_nmp)) {
	if (fgets (nmpbuf, 1024, fp_nmp)) {
	    while (fgets (nmpbuf, 1024, fp_nmp)) ; /* read last line */
	    if ((nmpnr = getnr ()) < 2) return (1);
	}
	else {
	    offset -= pos;
	    goto again;
	}
    }
    return (0);
}

void prHead (struct model_info *ntw, int ext)
{
    struct cir_term *t;
    struct term_ref *tref;
    struct node_ref *ptr;
    int firstinode;
    char par[256];
    char *ntw_name;
    char *s, *s_u;

    ntw_name = ntw -> name;

    if (no_vhdl_name (ntw_name)) {
	mk_vhdl_name (ntw_name, par);
	do_vhdl_warn (ntw_name, par, "entity");
	ntw_name = newStringSpace (par);
    }
    if (ext) oprint (0, "\nCOMPONENT ");
    else oprint (0, "\nENTITY ");
    oprint (1, ntw_name);
    if (ext) oprint (1, "\n");
    else oprint (1, " IS\n");
    if (!ntw -> terms) goto ret1;

    oprint (0, "  PORT (");

    if ((out_indent = outPos ()) > 24) out_indent = 8;

    prPortType = 1;
    currTermType = 0;

    firstinode = 1;

    for (tref = ntw -> terms; tref; tref = tref -> next) {
	t = tref -> t;

	if (!(currTermType = tref -> type)) currTermType = INOUT;

	s = t -> term_name;
	if ((mapdim = t -> term_dim) == 0) {

	    s_u = vhdl_mapping (&s);

	    for (ptr = Node_list; ptr; ptr = ptr -> next)
		if (strcmp (ptr -> node_name, s) == 0) break;
	    if (ptr) {
		if (ptr -> node_dim != mapdim)
		    fatalErr ("internal error on term_dim:", s);
	    }
	    else {
		if (s != t -> term_name)
		    do_vhdl_warn (t -> term_name, s, "port");
		NEW_NODE (ptr);
		ptr -> node_name = newStringSpace (s);
		ptr -> node_dim  = 0;
		ptr -> type = currTermType;
	    }
	    if (s_u) *s_u = '_';
	}
	else {
	    NEW_NODE (ptr);
	    if (no_vhdl_name (s)) {
		mk_vhdl_name (s, par);
		do_vhdl_warn (s, par, "port");
		s = newStringSpace (par);
	    }
	    ptr -> node_name  = s;
	    ptr -> node_dim   = mapdim;
	    ptr -> node_lower = t -> term_lower;
	    ptr -> node_upper = t -> term_upper;
	    ptr -> type = currTermType;
	}

	if (!tref -> next) {
	    firstinode = 1;
	    for (ptr = Node_list;;) {
		currTermType = ptr -> type;
		nmprint (firstinode, ptr -> node_name, ptr -> node_dim, ptr -> node_lower, ptr -> node_upper, 0);
		if (firstinode) firstinode = 0;
		if (!(ptr = ptr -> next)) break;
		oprint (1, ";\n");
	    }
	    oprint (1, ");\n");
	    FREE_NODES ();
	    goto ret1;
	}
    }

    oprint (0, "\n");

ret1:
    prPortType = 0;
    outPos ();
    if (ext) {
	out_indent = 2;
	oprint (0, "END COMPONENT;\n");
    }
    else {
	out_indent = 0;
	oprint (0, "END ");
	oprint (1, ntw_name);
	oprint (1, ";\n");
	oprint (0, "\nARCHITECTURE structural OF ");
	oprint (1, ntw_name);
	oprint (1, " IS\n");
    }
    outPos ();
}
