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

static char buf[DM_MAXLINE];

static int _dmFillXData (DM_XDATA *xdata, char *sep)
{
/* record:
    +------+------+----+----+-------+
    | name | time | ct | it | masks |
    +------+------+----+----+-------+
*/
    long ts;
    int n, m;

    if (!sep) {
	sep = buf;
	while (*sep != ' ' && *sep) ++sep;
	if (*sep != ' ' || sep == buf) return 1;
	*sep = 0;
	xdata->name = buf;
    }
    if (!isdigit ((int)*++sep)) return 1;
    ts = *sep - '0';
    while (isdigit ((int)*++sep)) { ts *= 10; ts += *sep - '0'; }
    xdata->timestamp = ts;

    if (*sep != ' ' || !isdigit ((int)*++sep)) return 1;
    xdata->celltype = *sep++ - '0';

    if (*sep != ' ' || !isdigit ((int)*++sep)) return 1;
    xdata->interfacetype = *sep++ - '0';

    if (*sep == '\n') {
	xdata->masks = sep;
	goto ret;
    }
    if (*sep++ != ' ') return 1;
    xdata->masks = sep;
    if (!isdigit ((int)*sep)) {
	if (*sep == ' ') goto ret;
	return 1;
    }
    n = *sep++ - '0';
    if (isdigit ((int)*sep)) { n *= 10; n += *sep++ - '0'; }

    while (*sep != '\n') {
	if (*sep++ != ' ') return 1;
	if (!isdigit ((int)*sep)) {
	    if (*sep == ' ' || *sep == '\n') { --sep; goto ret; }
	    return 1;
	}
	m = *sep++ - '0';
	if (isdigit ((int)*sep)) { m *= 10; m += *sep++ - '0'; }
	if (m <= n) return 1;
	n = m;
    }
ret:
    *sep = 0;
    return 0;
}

int dmGetXData (FILE *fp, DM_XDATA *xdata)
{
    if (!fgets (buf, DM_MAXLINE, fp)) {
	if (feof (fp)) return 0;
	goto err;
    }
    if (_dmFillXData (xdata, 0) == 0) return 1;
err:
    dmerrno = DME_GET;
    dmError ("dmGetXData");
    return -1;
}

int dmGetCellStatus (DM_PROJECT *project, DM_XDATA *xdata)
{
    FILE *fp;
    char *name, *s, *t;

    if (!xdata) {
	dmerrno = DME_GET;
	goto err;
    }

    dmerrno = DME_NOCELL;

    name = xdata->name;
    if (!name || !*name) goto err;

    if (!project) {
	fp = NULL;
	dmerrno = DME_BADPR;
	goto notfound;
    }

    if (!(fp = dmOpenXData (project, "r"))) {
	dmerrno = DME_FOPEN;
	goto notfound;
    }

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
    if (!feof (fp)) dmerrno = DME_GET;
notfound:
    if (fp) fclose (fp);
    xdata->timestamp = 0;
    xdata->celltype = DM_CT_REGULAR;
    xdata->interfacetype = DM_IF_STRICT;
    xdata->masks = buf;
    *buf = 0;
    return 1;
found:
    fclose (fp);
    if (_dmFillXData (xdata, s) == 0) return 0;
    dmerrno = DME_GET;
err:
    dmError ("dmGetCellStatus");
    return -1;
}
