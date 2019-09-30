/*
 * ISC License
 *
 * Copyright (C) 2016 by
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

#include "src/net2net/incl.h"

extern int coordinates;
extern char *nameGND, *nameSUB;
extern char *argv0;
extern char *sorts[];
extern int fd_nr[], wr_nr[];
extern int sort_cnt, cntR, cntRS, cntC, cntCS, cntCG, nodeCnt;
extern int verbose;

#ifdef __cplusplus
  extern "C" {
#endif
static void readCds (void);
static void readNets (void);
static void cpCnet2Netel (struct net_el *n);
#ifdef __cplusplus
  }
#endif

DM_CELL *dkey;
char *ntwname;
char *noNAME = "0";
struct net_ref **nreftab;
struct net_ref *nets;
int nodeCounter = 0, nxGND = 0, nxSUB = 0;
int gCnt, subCnt;

int *cdprev = NULL;
int cdcnt = 0;
int nodes = 0;

void xnetwork (char *name, DM_PROJECT *proj)
{
    int i;

    verbose3 ("converting %s into %s", "nethead/netsub/mc", "net2/mc2");

    ntwname = name;
    dkey = dmCheckOut (proj, name, WORKING, DONTCARE, CIRCUIT, ATTACH);

    readCds ();
    verbose3 ("reading %s and %s", "nethead", "netsub");
    readNets ();
    if (nets) {
	findNetInit (nets);
	verbose3 ("reading %s", "mc", NULL);
	verbose3 ("writing %s", "mc2", NULL);
	prInst (nets);
	verbose3 ("writing %s", "net2", NULL);
	prNets (nets);
    }

    for (i = 0; i < sort_cnt; ++i) {
	fprintf (stderr, "%s: %s: %d instances found, %d written\n", argv0, sorts[i], fd_nr[i], wr_nr[i]);
    }
    if (cntC ) fprintf (stderr, "%s: %d capacitors\n", argv0, cntC);
    if (cntCS) fprintf (stderr, "%s: %d capacitors to net SUB\n", argv0, cntCS);
    if (cntCG) fprintf (stderr, "%s: %d capacitors to net GND\n", argv0, cntCG);
    if (cntR ) fprintf (stderr, "%s: %d resistors\n", argv0, cntR);
    if (cntRS) fprintf (stderr, "%s: %d resistors to net SUB\n", argv0, cntRS);
    fprintf (stderr, "%s: %d gates found\n", argv0, gCnt);
    fprintf (stderr, "%s: %d nodes found, %d written\n", argv0, nodeCounter, nodeCnt);
    if (subCnt > 1) fprintf (stderr, "%s: %d subnodes found\n", argv0, subCnt);

    verbose3 ("ready", NULL, NULL);
}

static void readCds ()
{
    DM_STREAM *dsp;
    FILE *fp;
    char *s, buf[256];
    int i, j;

    dsp = dmOpenStream (dkey, "netcd", "r");
    fp = dsp -> dmfp;

    s = buf; *s = 0; fgets (buf, 256, fp); // read 1st line
    while (isdigit (*s)) cdcnt = 10 * cdcnt + (*s++ - '0');
    if (cdcnt <= 0) fatalErr ("netcd:", "incorrect cdcnt (<= 0)");

    s = buf; *s = 0; fgets (buf, 256, fp); // read 2nd line
    while (isdigit (*s)) nodes = 10 * nodes + (*s++ - '0');
    if (nodes <= 0) fatalErr ("netcd:", "incorrect nodes (<= 0)");

    s = buf; *s = 0; fgets (buf, 256, fp); // read 3rd line
    if (*s != ' ') fatalErr ("netcd: incorrect input:", "line 3");

    if (cdcnt > nodes) fatalErr ("netcd:", "incorrect cdcnt (> nodes)");

    while (fgets (buf, 256, fp)) {
	s = buf;
	i = 0; while (isdigit (*s)) i = 10 * i + (*s++ - '0');
	if (i <= 0) fatalErr ("netcd:", "incorrect i (<= 0)");
	s++;
	j = 0; while (isdigit (*s)) j = 10 * j + (*s++ - '0');
	if (!(j > i)) fatalErr ("netcd:", "incorrect j (<= i)");
	if (!cdprev) PALLOC (cdprev, cdcnt + 1, int);
	if (j > cdcnt) fatalErr ("netcd:", "incorrect j (> cdcnt)");
	if (cdprev[j]) fatalErr ("netcd:", "incorrect j (already cdprev)");
	cdprev[j] = i;
    }

    dmCloseStream (dsp, COMPLETE);

    if (cdprev)
    for (j = 1; j <= cdcnt; ++j) {
	if (!(i = cdprev[j])) cdprev[j] = j;
	else cdprev[j] = cdprev[i];
    }
}

static void readNets ()
{
    DM_STREAM *dsp, *dsp2;
    FILE *subfp;
    struct net_ref *nref, *nreflast, *last;
    struct net_ref *nets0, *nets0last;
    struct net_el *n1, *n, *np;
    char *val, *attr, attribute_buf[256];
    long lower[10], lower1[10], upper[10];
    int  cd, i, Neqv;

    cnet.net_attribute = attr = attribute_buf;
    cnet.net_lower  = lower;
    cnet.inst_lower = lower1;
    cnet.net_upper  = upper;
    cnet.inst_upper = upper;
    cnet.ref_lower  = upper;
    cnet.ref_upper  = upper;

    if ((i = nodes + 1) < 4) i = 4;
    PALLOC (nreftab, i, struct net_ref *);

    dsp = dmOpenStream (dkey, "nethead", "r");
    dsp2 = dmOpenStream (dkey, "netsub", "r");
    subfp = dsp2 -> dmfp;

    nets0 = nets0last = NULL;
    nets = NULL;

    while (dmGetDesignData (dsp, CIR_NET_HEAD) > 0) {

	++nodeCounter;
	if (cnethead.node_nr != nodeCounter) fatalErr ("nethead node_nr !=", "nodeCounter");

	Neqv = cnethead.net_neqv;
	if (Neqv > 0) fseeko (subfp, (off_t)cnethead.offset, SEEK_SET);

	n1 = np = NULL;
	for (i = 0; i < Neqv; ++i) {
	    if (dmGetDesignData (dsp2, CIR_NET_ATOM) <= 0) fatalErr ("netsub read error:", ntwname);

	    PALLOC (n, 1, struct net_el);
	    cpCnet2Netel (n);

	    if (coordinates) {
		if (!(val = getAttrValue (attr, "x"))) n -> x = INT_MAX;
		else {
		    sscanf (val, "%d", &n -> x);
		    if (!(val = getAttrValue (attr, "y")))
			fatalErr ("net_attribute y not found in circuit:", ntwname);
		    sscanf (val, "%d", &n -> y);
		}
	    }

	    if (np) np -> nexte = n;
	    else {
		if (cnet.net_neqv != nodeCounter) fatalErr ("netsub net_neqv !=", "nethead node_nr");
		n1 = n;
	    }
	    np = n;
	}

	if (n1 && !n1 -> inst_name) --Neqv;
	else {
	    PALLOC (n, 1, struct net_el);
	    n -> net_name = noNAME;
	    n -> nexte = n1;
	    n1 = n;
	}
	n1 -> x = cnethead.node_x;
	n1 -> y = cnethead.node_y;

	PALLOC (nref, 1, struct net_ref);
	nref -> nx  = nodeCounter;
	nref -> lay = cnethead.lay_nr;
	nref -> netcnt = Neqv;
	nref -> n  = n1;

	cd = cnethead.cd_nr;
	if (cd < 0 || cd > cdcnt) fatalErr ("error:", "incorrect cd number");
	if (cdprev) cd = cdprev[cd];
	nref -> cd = cd;

	if (cd == 0) { ++subCnt;
	    if (!nets0) nets0 = nref;
	    else nets0last -> next = nref;
	    nets0last = nref;
	}
	else if (!nets) {
	    if (cd != 1) fatalErr ("error:", "first cd number != 1");
	    nreftab[cd] = nets = nref;
	    nref -> prev = nref; /* first */
	}
	else {
	    if ((last = nreftab[cd])) {
		last -> next = nref;
		nref -> prev = last -> prev;
	    }
	    else nref -> prev = nref; /* first */
	    nreftab[cd] = nref; /* last */
	}
    }

    dmCloseStream (dsp, COMPLETE);
    dmCloseStream (dsp2, COMPLETE);

    if (!nets) fatalErr ("error:", "no nets found!");
    if (nodeCounter != nodes) fatalErr ("error:", "unexpected # nodes!");

    nreflast = nreftab[1];
    for (cd = 2; cd < cdcnt; ++cd) {
	if ((last = nreftab[cd])) {
	    nreflast -> next = last -> prev; /* first */
	    nreflast = last;
	}
    }
    last = nreftab[cd];

    if (last) { /* GND? */
	if (strcmp (last -> n -> net_name, nameGND) == 0) {
	    nxGND = last -> nx;
	    if (nxGND != nodeCounter) fatalErr ("error:", "net GND != last node");
	    if (last -> prev != last) fatalErr ("error:", "net GND prev != last");
	}
	else { /* last not GND */
	    nreflast -> next = last -> prev; /* first */
	    nreflast = last; last = NULL;
	}
    }
    if (nreflast == last) fatalErr ("error:", "nreflast == last");

    /* cd=0 must be last net, but before GND */

    if (nets0) { // cd=0 found
	nreflast -> next = nets0;
	nreflast = nets0last;
	if (strcmp (nreflast -> n -> net_name, nameSUB) == 0) {
	    nxSUB = nreflast -> nx;
	    if (nxGND) {
		if (nxSUB != nxGND - 1) fatalErr ("error:", "net SUB != GND - 1");
	    }
	    else if (nxSUB != nodeCounter) fatalErr ("error:", "net SUB != last node");
	}
    }
    nreflast -> next = last; /* add GND */

    if (!nxSUB) warning ("net SUB not found");
    if (!nxGND) warning ("net GND not found");
}

static void cpCnet2Netel (struct net_el *n)
{
    int dim;

    n -> net_name = newStringSpace (cnet.net_name, (dim = cnet.net_dim));
    if (!n -> net_name) fatalErr ("no net_name:", "zero length");
    if (dim > 0) n -> net_dim = dim;
    n -> inst_name = newStringSpace (cnet.inst_name, (dim = cnet.inst_dim));
    if (dim > 0 && n -> inst_name) n -> inst_dim = dim;
}

char *bNAME = "b";
char *cNAME = "c";
char *dNAME = "d";
char *eNAME = "e";
char *gNAME = "g";
char *sNAME = "s";

char *newStringSpace (char *s, int dim)
{
    static int SS_cnt = 4096;
    static char *SS;
    char buf[80], *t;
    int cnt, i, j, len;

    if (!(len = strlen (s))) return (NULL);
    if (len == 1 && !dim) {
	switch (*s) {
	case 'b': return (bNAME);
	case 'c': return (cNAME);
	case 'd': return (dNAME);
	case 'e': return (eNAME);
	case 'g': ++gCnt; return (gNAME);
	case 's': return (sNAME);
	}
    }
    cnt = len + 1; /* add 0 byte */

    if (dim) {
	long *arr;
	if (dim < 0) fatalErr ("error newStringSpace:", "dim < 0");
	if (s == cnet.inst_name) arr = cnet.inst_lower;
	else {
	    arr = cnet.net_lower;
	    if (s != cnet.net_name) fatalErr ("error newStringSpace:", "no cnet");
	}
	t = buf;
	for (j = i = 0; i < dim; ++i) {
	    sprintf (t + j, "[%ld]", arr[i]);
	    if ((j = strlen (buf)) >= 80) fatalErr ("error newStringSpace:", "too long dim");
	}
	cnt += j;
    }

    i = 4096 - SS_cnt; /* left */
    if (cnt > i) {
	if (i > 10) {
	    if (!(t = malloc (cnt))) cannot_die (1, 0);
	} else {
	    if (!(t = malloc (4096))) cannot_die (1, 0);
	    SS = t; SS_cnt = cnt;
	}
    }
    else {
	t = SS + SS_cnt; SS_cnt += cnt;
    }
    strcpy (t, s);
    if (dim) strcpy (t + len, buf);
    return (t);
}

char *strsave (char *s)
{
    return newStringSpace (s, 0);
}

/* getAttrValue reads the attribute string, searches for parameter par,
   if this par is followed by a '=', a ptr to this value is returned,
   or a null string is returned (if par is found).
   if par is not found, NULL is returned!
*/
char *getAttrValue (char *a, char *par)
{
    char *p;

    while (*a) {
	while (*a == ';') ++a;
	for (p = par; *a && *p == *a; ++p) ++a;
	if (!*p) {
	    if (*a == '=') p = ++a;
	    if (*a == ';') return ("");
	    if (p == a || !*a) return (a);
	}
	while (*a && (*a != ';' || *(a-1) == '\\')) ++a;
    }
    return (NULL);
}
