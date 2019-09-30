
/*
 * ISC License
 *
 * Copyright (C) 1997-2016 by
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

FILE *subfp;
int  nodeCounter;

extern int mask_info;
extern int onlySubckt;
extern int use_spef;
extern int coordinates;
extern char *nameGND, *nNET, *pNET;
extern struct net_el *fnetSUB;
extern struct net_el *fnetGND;
extern struct node_info *nodetab;

#ifdef __cplusplus
  extern "C" {
#endif
static void readNet (struct model_info *m, struct net_ref **anets);
static void cpCnet2Netel (int subnet, struct net_el *nel);
static void delNetel (struct net_ref *nets);
static void resetIndexSpace (void);
static void resetStringSpace (void);
#ifdef __cplusplus
  }
#endif

struct net_el *free_el_list;

void xnetwork (char *ntwname, DM_PROJECT *proj, int imported, char *orig_name, int submod)
{
    struct model_info *ntw;
    struct net_ref *nets;

    outPos ();

    ntw = newNetw (ntwname, proj, imported, orig_name, submod);

    hashInit ();

    readNet (ntw, &nets);

    if (use_spef) {
	prHead (ntw, (onlySubckt ? 1 : submod));
    }

    prInst1 (ntw, nets);

    if (!use_spef) {
	prHead (ntw, (onlySubckt ? 1 : submod));
    }

    prNets (ntw, nets);

    if (!use_spef) {
	prInst3 (ntw, nets);
	prFoot (ntw, (onlySubckt ? 1 : submod));
    }

    if (submod) {
	delNetel (nets);
	resetStringSpace ();
	resetIndexSpace ();
	deleteNodeTab();
	nameNbrReset ();
    }
}

/* read the net file of model m */
static void readNet (struct model_info *m, struct net_ref **anets)
{
    struct stat statbuf;
    DM_STREAM *dsp;
    DM_STREAM *dsp2;
    struct term_ref *tref;
    struct net_ref *nref, *nreflast, *prev;
    struct net_ref *nref0, *nreflast0;
    struct net_el *n1, *n, *np;
    char *s, *val, buf[32];
    char attribute_string[256];
    long lower[10], lower1[10], lower2[10];
    long upper[10], upper1[10], upper2[10];
    int i, Neqv, nx, inst_found;
    int cirfmt;

    cnet.net_attribute = attribute_string;
    cnet.net_lower  = lower;
    cnet.net_upper  = upper;
    cnet.inst_lower = lower1;
    cnet.inst_upper = upper1;
    cnet.ref_lower  = lower2;
    cnet.ref_upper  = upper2;

    if (dmStat (m -> dkey, "netsub", &statbuf) == 0) {
	dsp = dmOpenStream (m -> dkey, "nethead", "r");
	dsp2 = dmOpenStream (m -> dkey, "netsub", "r");
	subfp = dsp2 -> dmfp;
	cirfmt = CIR_NET_HEAD;
    }
    else {
	dsp2 = dsp = dmOpenStream (m -> dkey, "net", "r");
	subfp = NULL;
	cirfmt = CIR_NET_ATOM;
    }

    nodeCounter = 0;

    *anets = nreflast = NULL;
    nref0 = nreflast0 = NULL;

    while (dmGetDesignData (dsp, cirfmt) > 0) {

	if (++nodeCounter == INT_MAX) fatalErr ("error:", "nodeCounter == INT_MAX");

	PALLOC (nref, 1, struct net_ref);

	if ((n1 = free_el_list)) free_el_list = free_el_list -> net_eqv;
	else PALLOC (n1, 1, struct net_el);

	nref -> n = n1;
	nref -> done = 0;

if (cirfmt == CIR_NET_HEAD) {
	nref -> cd = cnethead.cd_nr;
	n1 -> lay = cnethead.lay_nr;
	n1 -> x = cnethead.node_x;
	n1 -> y = cnethead.node_y;
	Neqv = cnethead.net_neqv;
	if (cnethead.node_nr != nodeCounter) fatalErr ("nethead node_nr !=", "nodeCounter");
	n1 -> nx = nodeCounter;
	if (Neqv > 0) fseeko (subfp, (off_t)cnethead.offset, SEEK_SET);

	sprintf (buf, "%d", n1 -> nx);
	n1 -> net_name = newStringSpace (buf);
	n1 -> net_dim  = 0;
	n1 -> inst_name = NULL;
	n1 -> inst_dim  = 0;
}
else {
	nref -> cd = -1;
	if ((val = getAttrValue (cnet.net_attribute, "cd"))) sscanf (val, "%d", &nref -> cd);
	if (nref -> cd < 0) fatalErr ("net_attribute cd not found in circuit:", m -> name);

	if (mask_info) {
	    if (!(val = getAttrValue (cnet.net_attribute, "lay"))) {
		fatalErr ("net_attribute lay not found in circuit:", m -> name);
	    }
	    n1 -> lay = atoi (val);
	}

	if (coordinates) {
	    if (!(val = getAttrValue (cnet.net_attribute, "x"))) {
		if (strcmp (cnet.net_name, nameGND) == 0 && nreflast) {
		    n1 -> x = nreflast -> n -> x;
		    n1 -> y = nreflast -> n -> y;
		    goto cpCnet;
		}
		fatalErr ("net_attribute x not found in circuit:", m -> name);
	    }
	    sscanf (val, "%d", &n1 -> x);
	    if (!(val = getAttrValue (cnet.net_attribute, "y")))
		fatalErr ("net_attribute y not found in circuit:", m -> name);
	    sscanf (val, "%d", &n1 -> y);
	}
cpCnet:
	cpCnet2Netel (0, n1);
	Neqv = cnet.net_neqv;
	if (Neqv > 0) n1 -> nx = nodeCounter;
	else n1 -> nx = 0;
}

	inst_found = 0;
	np = n1;
	for (i = 0; i < Neqv; ++i) {
	    if (dmGetDesignData (dsp2, CIR_NET_ATOM) <= 0) fatalErr ("net read error:", m -> name);

	    if ((n = free_el_list)) free_el_list = free_el_list -> net_eqv;
	    else PALLOC (n, 1, struct net_el);
	    np -> net_eqv = n; np = n;
	    n -> v.val = 0;

	    if (coordinates) {
		if (!(val = getAttrValue (cnet.net_attribute, "x"))) {
		    n -> x = n1 -> x;
		    n -> y = n1 -> y;
		}
		else {
		    sscanf (val, "%d", &n -> x);
		    if (!(val = getAttrValue (cnet.net_attribute, "y")))
			fatalErr ("net_attribute y not found in circuit:", m -> name);
		    sscanf (val, "%d", &n -> y);
		}
	    }

	    cpCnet2Netel (1, n);

	    if (inst_found) {
		if (!n -> inst_name) fatalErr ("subnet error:", "has no inst_name");
	    }
	    else if (n -> inst_name) inst_found = 1;

	    n -> nx = n1 -> nx;
	    if (i == 0 && cirfmt == CIR_NET_HEAD) {
		if (cnet.net_neqv != n1 -> nx) fatalErr ("netsub net_neqv !=", "nethead node_nr");
		if (!n -> inst_name) {
		    n1 -> net_name  = n -> net_name;
		    n1 -> net_dim   = n -> net_dim ;
		    n1 -> net_lower = n -> net_lower;
		    np = n1;
		    n -> net_eqv = free_el_list; free_el_list = n;
		}
	    }
	}
	np -> net_eqv = NULL;
	nref -> nl = np;

	if (nref -> cd == 0) {
	    if (!nref0) nref0 = nref;
	    else nreflast0 -> next = nref;
	    nreflast0 = nref;
	}
	else if (!*anets) *anets = nreflast = nref;
	else {
	    if (nref -> cd < nreflast -> cd) {
		prev = nreflast -> prev;
		while (prev && nref -> cd < prev -> cd) prev = prev -> prev;
		if (!prev) {
		    nref -> next = *anets;
		    *anets = nref;
		}
		else { /* nref -> cd >= prev -> cd */
		    nref -> prev = prev;
		    nref -> next = prev -> next;
		    prev -> next = nref;
		}
		nref -> next -> prev = nref;
	    }
	    else { /* nref -> cd >= nreflast -> cd */
		nref -> prev = nreflast;
		nreflast -> next = nref;
		nreflast = nref;
	    }
	}
    }

    if (nreflast && !strcmp (nreflast -> n -> net_name, nameGND)) fnetGND = nreflast -> n;
    else fnetGND = NULL;

    /* cd=0 must be last net, but before GND */
    if (nref0) {
	fnetSUB = nreflast0 -> n;
	if (nreflast) {
	    if (fnetGND) {
		nreflast0 -> next = nreflast;
		if (nreflast -> prev)
		    nreflast -> prev -> next = nref0;
		else
		    *anets = nref0;
	    }
	    else { /* nreflast != GND */
		nreflast -> next = nref0;
	    }
	}
	else *anets = nref0;
    }
    else fnetSUB = NULL;

    dmCloseStream (dsp, COMPLETE);
if (cirfmt == CIR_NET_HEAD)
    dmCloseStream (dsp2, COMPLETE);

    createNodeTab (nodeCounter + 1);

    for (nref = *anets; nref; nref = nref -> next) {
	n = nref -> n;
	if ((nx = n -> nx) <= 0) continue;
	nodetab[nx].netref = nref;
	if (n -> inst_name) fatalErr ("net error:", "has inst_name");

	/* check for terminal net */
	do {
	    s = n -> net_name;
	    if (!isdigit (*s))
	    for (tref = m -> terms; tref; tref = tref -> next) {
		if (strcmp (tref -> t -> term_name, s) == 0) {
		    nodetab[nx].isTerm = 1; nx = 0; break;
		}
	    }
	} while (nx && (n = n -> net_eqv) && !n -> inst_name);
    }
}

char *bNAME = "b";
char *cNAME = "c";
char *dNAME = "d";
char *eNAME = "e";
char *gNAME = "g";
char *sNAME = "s";

static void cpCnet2Netel (int subnet, struct net_el *nel)
{
    int i, dim;

    if (!*cnet.net_name) fatalErr ("no net_name:", "zero length");

    if (!cnet.net_name[1]) {
	switch (*cnet.net_name) {
	case 'b': nel -> net_name = bNAME; break;
	case 'c': nel -> net_name = cNAME; break;
	case 'd': nel -> net_name = dNAME; break;
	case 'e': nel -> net_name = eNAME; break;
	case 'g': nel -> net_name = gNAME; break;
	case 'n': nel -> net_name = nNET ; break;
	case 'p': nel -> net_name = pNET ; break;
	case 's': nel -> net_name = sNAME; break;
	default:
	    nel -> net_name = newStringSpace (cnet.net_name);
	}
    }
    else nel -> net_name = newStringSpace (cnet.net_name);

    if (cnet.net_dim > 0) {
	nel -> net_dim = dim = cnet.net_dim;
	nel -> net_lower = newIndexSpace (dim);
	for (i = 0; i < dim; ++i) {
	    nel -> net_lower[i] = cnet.net_lower[i];
	    if (cnet.net_upper[i] != cnet.net_lower[i])
		if (subnet || cnet.net_neqv) fatalErr ("net_upper != net_lower in net:", cnet.net_name);
	}
    }
    else nel -> net_dim = 0;

    nel -> inst_dim = 0;

    if (*cnet.inst_name) {
	struct net_el *n = findInst (cnet.inst_name);
	if (n) nel -> inst_name = n -> inst_name;
	else   nel -> inst_name = newStringSpace (cnet.inst_name);

	hash (nel, n);

	if (cnet.inst_dim > 0) {
	    nel -> inst_dim = dim = cnet.inst_dim;
	    nel -> inst_lower = newIndexSpace (dim);
	    for (i = 0; i < dim; ++i) {
		nel -> inst_lower[i] = cnet.inst_lower[i];
		if (cnet.inst_upper[i] != cnet.inst_lower[i]) fatalErr ("inst_upper != inst_lower in net:", cnet.net_name);
	    }
	}
    }
    else nel -> inst_name = NULL;
}

static void delNetel (struct net_ref *nets)
{
    struct net_ref *nref;

    while ((nref = nets)) {
	nets = nref -> next;
	nref -> nl -> net_eqv = free_el_list;
	free_el_list = nref -> n;
	Free (nref);
    }
}

static char **SS;
static int  **IS;
static int SS_y_size, SS_y_cnt, SS_x_cnt;
static int IS_y_size, IS_y_cnt, IS_x_cnt;

static void resetStringSpace ()
{
    SS_y_cnt = SS_x_cnt = 0;
}

static void resetIndexSpace ()
{
    IS_y_cnt = IS_x_cnt = 0;
}

char *newStringSpace (char *s)
{
    int i, cnt;

    cnt = 1;
    if (s) cnt += strlen (s);
    if (cnt == 1) return (NULL);

    if (SS_x_cnt + cnt > 4096) { ++SS_y_cnt; SS_x_cnt = 0; }
    SS_x_cnt += cnt;

    if (SS_y_cnt >= SS_y_size) {
	i = SS_y_size; /* old_size */
	SS_y_size += 3;
	REPALLOC (SS, SS_y_size, char *);
	for (; i < SS_y_size; ++i) PALLOC (SS[i], 4096, char);
    }
    return (strcpy (&SS[SS_y_cnt][SS_x_cnt - cnt], s));
}

int *newIndexSpace (int cnt)
{
    int i;

    if (cnt <= 0) return (NULL);

    if (IS_x_cnt + cnt > 4096) { ++IS_y_cnt; IS_x_cnt = 0; }
    IS_x_cnt += cnt;

    if (IS_y_cnt >= IS_y_size) {
	i = IS_y_size; /* old_size */
	IS_y_size += 2;
	REPALLOC (IS, IS_y_size, int *);
	for (; i < IS_y_size; ++i) PALLOC (IS[i], 4096, int);
    }
    return (&IS[IS_y_cnt][IS_x_cnt - cnt]);
}
