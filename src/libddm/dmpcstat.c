/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
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

#include "src/libddm/dmstd.h"
#include <time.h>

int dmPutXData (FILE *fp, DM_XDATA *xdata)
{
    char *fmt, *ml;

    if ((ml = xdata->masks))
	while (*ml == ' ') ++ml;
    if (!ml || !*ml)
	fmt = "%s %010ld %d %d\n";
    else
	fmt = "%s %010ld %d %d %s\n";
    if (fprintf (fp, fmt,
	xdata->name,
	xdata->timestamp,
	xdata->celltype,
	xdata->interfacetype, ml) > 0) return 0;
    dmerrno = DME_PUT;
    dmError ("dmPutXData");
    return -1;
}

int dmPutCellStatus (DM_PROJECT *project, DM_XDATA *xdata)
{
    char   buf[DM_MAXLINE];
    char   buf2[DM_MAXLINE];
    char   *name, *m, *s, *t;
    FILE   *fp;
    long   pos, pos2;
    DM_PROCDATA *mdata;
    int ct, it, i, j, no, len, len2, mlen, c;
    int ml[DM_MAXNOMASKS];

    if (!project || !xdata) {
	dmerrno = !project ? DME_BADPR : DME_NOCELL;
	goto err;
    }

    name = xdata->name;
    if (!name || !*name) {
	dmerrno = DME_NOCELL;
	goto err;
    }

    ct = xdata->celltype;
    if (ct != DM_CT_REGULAR && ct != DM_CT_MACRO   &&
	ct != DM_CT_DEVICE  && ct != DM_CT_LIBRARY &&
	ct != DM_CT_IMPORT) {
	dmerrno = DME_NOCELLTYPE;
	goto err;
    }

    mlen = 0;
    no = 0; /* initialize to suppress warning */
    it = xdata->interfacetype;
    if (it == DM_IF_FREEMASKS) {

	mdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);
	if (!mdata) goto err1;
	if ((no = mdata->nomasks) > DM_MAXNOMASKS) goto err1;

	for (i = 0; i < no; ++i) ml[i] = 0;

	m = xdata->masks;
again:
	while (*m == ' ') ++m;
	if (isdigit ((int)*m)) {
	    i = *m - '0';
	    while (isdigit ((int)*++m)) { i *= 10; i += *m - '0'; } /* SdeG3.2 */
	    if (i >= no) goto err1;
	    if (!ml[i]) {
		ml[i] = 1;
		if (i < 10) mlen += 2;
		else if (i < 100) mlen += 3;
		else if (i < 1000) mlen += 4;
		else mlen += 5;
	    }
	    if (*m) {
		if (*m++ != ' ') goto err1;
		goto again;
	    }
	}
	else goto err1;
    }
    else if (it != DM_IF_STRICT && it != DM_IF_FREE) {
	dmerrno = DME_NOINTERFACETYPE;
	goto err;
    }

    if (!(fp = dmOpenXData (project, "r+"))) {
	dmerrno = DME_FOPEN;
	goto err;
    }

    xdata->timestamp = time (0);

    while (fgets (buf, DM_MAXLINE, fp)) {
	s = buf;
	t = name;
	while (*s == *t) { ++s; ++t; };
	if (*s > *t) {
	    if (!*t) {
		if (*s == ' ') goto found;
	    }
	    goto notfound;
	}
    }
    if (!feof (fp)) {
	dmerrno = DME_GET;
	goto err;
    }
    len = 0;
    pos2 = 0; /* initialize to suppress warning */
doit:
    fprintf (fp, "%s %010ld %d %d", name, xdata->timestamp, ct, it);
    if (mlen) {
	for (i = 0; i < no; ++i) {
	    if (ml[i]) fprintf (fp, " %d", i);
	}
    }
    fprintf (fp, "\n");
    pos = ftell (fp);

    while (len) {
	len2 = 0;
	if (len == DM_MAXLINE) {
	    fseek (fp, pos2, 0);
	    while (len2 < DM_MAXLINE) {
		if ((c = fgetc (fp)) == EOF) break;
		buf2[len2++] = c;
	    }
	    pos2 += len2;
	}
	fseek (fp, pos, 0);
	for (j = 0; j < len; ++j) fputc (buf[j], fp);
	pos += len;

	len = 0;
	if (len2 == DM_MAXLINE) {
	    fseek (fp, pos2, 0);
	    while (len < DM_MAXLINE) {
		if ((c = fgetc (fp)) == EOF) break;
		buf[len++] = c;
	    }
	    pos2 += len;
	}
	fseek (fp, pos, 0);
	for (j = 0; j < len2; ++j) fputc (buf2[j], fp);
	pos += len2;
    }
    fclose (fp);
    return 0;

notfound:
    len = strlen (buf);
    pos = ftell (fp) - len;
dofill:
    while (len < DM_MAXLINE) {
	if ((c = fgetc (fp)) == EOF) break;
	buf[len++] = c;
    }
    pos2 = ftell (fp);
    fseek (fp, pos, 0);
    goto doit;

found:
    len = strlen (++s);
    pos = ftell (fp) - len;

    if (len >= mlen + 15) {
	fseek (fp, pos, 0);
	fprintf (fp, "%010ld %d %d", xdata->timestamp, ct, it);
	if (it == DM_IF_FREEMASKS) {
	    for (i = 0; i < no; ++i) {
		if (ml[i]) fprintf (fp, " %d", i);
	    }
	    for (j = mlen + 15; j < len; ++j) fputc (' ', fp);
	}
    }
    else {
	--pos;
	name = "";
	len = 0;
	goto dofill;
    }
    fclose (fp);
    return 0;
err1:
    dmerrno = DME_NOMASK;
err:
    dmError ("dmPutCellStatus");
    return -1;
}
