/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	S. de Graaf
 *	T.G.R. van Leuken
 *	P. Kist
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
#include "src/getlld/incl.h"
#include "src/getlld/extern.h"

/*
** output layout language description
*/
void outp_lld ()
{
    register char *proj;
    register int llevel;
    register struct mo_elmt *p = molist;

    if (v_mode) PE "\nlevel:   cell:\n");

    if (!p) return;

    llevel = p -> level + 1; /* start at lowest level */

    do {
	if (p -> level < llevel)
	    PF "%slevel %d%s", begC, --llevel, endC);

	if (p -> pkey != dmproject) {
	    proj = p -> pkey -> dmpath;
	    PF "%s%s %s%s", begC, p -> oname, proj, endC);
	    if (v_mode)
		PE "-- %3d   %s %s\n", llevel, p -> oname, proj);
	}
	else if (v_mode) PE "-- %3d   %s\n", llevel, p -> oname);

	ckey = p -> key;

#ifdef XLDM
	outp_ldm (p);
#else
#ifdef XCIF
	outp_cif (p);
#else
#ifdef XCMK
	outp_cmk (p);
#else
	switch (Pmode) {
	    case LDM:
		outp_ldm (p); break;
	    case CIF:
		outp_cif (p); break;
	    case CMK:
		outp_cmk (p); break;
	}
#endif
#endif
#endif
	dmCheckIn (ckey, COMPLETE);
    } while ((p = p -> next));
}

struct ic_elmt *inst_ic_elmt (char *cell)
{
    struct ic_elmt *p;
    ALLOC (p, struct ic_elmt);
    p -> mo = cell;
    p -> l = p -> r = 0;
    return (p);
}

void inst_alias (struct ic_elmt *p, char *cell)
{
    int     i;
    if ((i = strcmp (cell, p -> mo)) == 0) return;
    if (i < 0) {
	if (p -> l)
	    inst_alias (p -> l, cell);
	else
	    p -> l = inst_ic_elmt (cell);
    }
    else {
	if (p -> r)
	    inst_alias (p -> r, cell);
	else
	    p -> r = inst_ic_elmt (cell);
    }
}

char *d2a (double  d)
{
    static char xbuf[80];	/* using 4 times 20 bytes */
    static int  i = 0;
    register char  *s, *c;

    i += 20;
    i %= 80;
    s = &xbuf[i];		/* select next 20 bytes */
    sprintf (s, "%.3f", d);
    while (*++s != '.');
    c = s + 3;
    if (*c == '0')
	if (*--c == '0')
	    if (*--c == '0') --c;
    if (c == s)
	*s = 0;
    else
	*++c = 0;
    return (&xbuf[i]);
}

char *strsave (char *s, int slen)
{
    register char *c, *p;
    if (!(c = malloc (slen + 1))) error (3, "char");
    p = c;
    while ((p - c) < slen) *p++ = *s++;
    *p = 0;
    return (c);
}
