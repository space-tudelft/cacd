/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "src/libddm/dmincl.h"
#include <math.h>
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/scan/export.h"
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"
#include "src/space/extract/define.h"

#define SET_TERM(node) node -> term |= 1+4
#define KEEP(node) (node -> term & 2)

DM_STREAM *dmsNet = NULL;
#ifdef CIR_NET_HEAD
DM_STREAM *dmsNethead = NULL;
#endif
DM_STREAM *dmsNetCD = NULL;
DM_STREAM *dmsMc  = NULL;

#define Ready(n) !n -> subs

int maxNeqv;
int gndCap_cnt;
int substrCap_cnt;
int substrRes_cnt;
int substrToGnd_cnt;
int tor_cnt;
int outConduc;

static char netattr[132];
static int isSubstrNode;

static int ready_grp = 0;
static int dsWarning = 0;
int dsWarning0 = 0;
int dsWarning1 = 0;

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
extern void nqElimGrp (group_t *grp);
extern void readyGrp (group_t *grp);
extern void removeNodeFromCluster (node_t *n, int dispose);
extern void outNode (node_t *n);
Private void fill_cnet_term (terminal_t *tm);
extern  void outResistor  (int ne_nr, int nr, int sort, double val);
extern  void outCapacitor (node_t *n, netEquiv_t *ne, int nr, int sort, double val);
Private void addNetEq (node_t *n, int type, int pin, int nr);
extern  void addNetEqCAP (node_t *n, int nr, int sort, double val);
extern  void addNetEqRES (node_t *n, int nr, int sort, double val);
extern  netEquiv_t *putNetEqCAP (node_t *n, netEquiv_t *ne, netEquiv_t *p);
extern  netEquiv_t *putNetEqRES (node_t *n, netEquiv_t *ne, netEquiv_t *p);
#ifdef __cplusplus
  }
#endif

extern double MIN_CONDUCTANCE;
static int addREScount, minREScount;

void initOut (DM_CELL *cellCirKey, DM_CELL *cellLayKey)
{
    static int cnetInit = 0;

    dsWarning = dsWarning0 = dsWarning1 = 0;
    addREScount = minREScount = 0;

    ASSERT (extrPass);

#ifdef CIR_NET_HEAD
    dmsNethead = dmOpenStream (cellCirKey, "nethead", "w");
    dmsNet = dmOpenStream (cellCirKey, "netsub", "w");
#else
    dmsNet = dmOpenStream (cellCirKey, "net", "w");
#endif
    dmsNetCD = dmOpenStream (cellCirKey, "netcd", "w");
    fprintf (dmsNetCD -> dmfp, "%24s\n", "");
    dmsMc = dmOpenStream (cellCirKey, "mc", "w");

    maxNeqv = 0;
    gndCap_cnt = 0;
    substrCap_cnt = 0;
    substrRes_cnt = 0;
    substrToGnd_cnt = 0;
    isSubstrNode = 0;
    tor_cnt = 0;
    outConduc = 0;

    if (!cnetInit) {
	cnet.net_attribute = netattr;
	cnet.net_dim = 0;
	cnet.ref_dim = 0;
	cnet.inst_dim = 0;
	cnet.net_upper  = cnet.net_lower  = NEW (long, 2);
	cnet.inst_upper = cnet.inst_lower = NEW (long, 2);
	cnetInit = 1;
    }
}

#define dmCLOSE(x) if (x) dmCloseStream (x, COMPLETE), x = NULL

void endOut ()
{
    dmCLOSE (dmsNet);
#ifdef CIR_NET_HEAD
    dmCLOSE (dmsNethead);
#endif
    if (dmsNetCD) {
	rewind  (dmsNetCD -> dmfp);
	fprintf (dmsNetCD -> dmfp, "%d\n", outConduc);
	fprintf (dmsNetCD -> dmfp, "%d\n", outNod);
	dmCLOSE (dmsNetCD);
    }
    dmCLOSE (dmsMc);
    outSubTerm = substrRes_cnt;

    if (addREScount > 0) say ("warning: addNetEqRES: %d values < MIN_CONDUCTANCE", addREScount);
    if (minREScount > 0) say ("warning: outResistor: %d values < MIN_CONDUCTANCE", minREScount);
}

Private void addCoorXY (char *buf, coor_t x, coor_t y)
{
    double xd = x / (double)outScale;
    double yd = y / (double)outScale;

    if (*buf) {
	buf += strlen (buf);
	*buf++ = ';';
    }
    if (xd == (double)((long)xd))
	sprintf (buf, "x=%ld", (long)xd);
    else
	sprintf (buf, "x=%.2f", xd);

    buf += strlen (buf);
    if (yd == (double)((long)yd))
	sprintf (buf, ";y=%ld", (long)yd);
    else
	sprintf (buf, ";y=%.2f", yd);
}

void readyGroup (node_t *n)
{
//fprintf (stderr, "\nwarning: readyGroup!!!  n=%d,%d a=%d t=%d\n", n->node_x/4, n->node_y/4, n->area, n->term);
    readyGrp (Grp (n));
}

void readyGrp (group_t *grp)
{
    node_t *n;

    ASSERT (grp -> notReady == 0);
    ready_grp = 1;

    if (grp -> nodes == nSUB) {
	outNode (nSUB);
    }
    else {
	nqElimGrp (grp);
	while ((n = grp -> nodes)) outNode (n);
    }

    ASSERT (grp -> nod_cnt == 0);
    ASSERT (grp -> nodes == NULL);

    if (grp -> grp_nr || isSubstrNode == 2) outGrp++;
    groupDel (grp);
    ready_grp = 0;
}

Private void dmPutNet ()
{
    dmPutDesignData (dmsNet, CIR_NET_ATOM);
    cnet.net_neqv = 0;
    *netattr = '\0'; /* reset */
}

void outNode (node_t *n)
{
    node_t *on;
    int i, j, net_neqv, new_net_neqv;
    register element_c *cap, *el;
    register element_r *con;
    terminal_t *tm;
    netEquiv_t *ne, *ne2;
    group_t *grp = Grp (n);

    if (n == nSUB) {
	ASSERT (grp -> notReady == 0);
	ASSERT (isSubstrNode == 0);
	isSubstrNode = 1;
    }

    net_neqv = n -> n_n_cnt;

    if (!isSubstrNode) {
	if (!net_neqv && !KEEP(n) && !n -> res_cnt && !n -> cap_cnt) {
	    ASSERT (!n -> names);
	    ASSERT (!n -> netEq);
	    ASSERT (!n -> netEq2);
	    goto ret;
	}
    }
    else { /* isSubstrNode */
	ASSERT (n -> res_cnt == 0); /* no RES netEq's attached */
	ASSERT (n -> cap_cnt == 0);
	ASSERT (!n -> netEq2);

	for (i = 0; i < capSortTabSize; i++) {
	    ASSERT (n -> cap[i] == NULL);
	    if (n -> gndCap[i]) { substrToGnd_cnt++; break; }
	}

	if (net_neqv + substrRes_cnt + substrCap_cnt + substrToGnd_cnt == 0) { /* unconnected */
	    ASSERT (!n -> names);
	    ASSERT (!n -> netEq);
	    goto ret;
	}
	isSubstrNode = 2;
    }

    /* Now, we do the real writing and memory de-allocation */
    outNod++;

    if (isSubstrNode) strcpy (cnet.net_name, nameSUBSTR);
    else sprintf (cnet.net_name, "%d", outNod);

    if (isSubstrNode || n -> substr) i = 0;
    else if (!(i = grp -> grp_nr)) grp -> grp_nr = i = ++outConduc;

    cnet.inst_name[0] = '\0';

    new_net_neqv = 0;

#ifdef CIR_NET_HEAD
    cnethead.cd_nr  = i;
    cnethead.node_nr= outNod;
    cnethead.node_x = n -> node_x;
    cnethead.node_y = n -> node_y;

    cnethead.offset = ftello (dmsNet -> dmfp);
    cnethead.lay_nr = n -> mask;
    cnethead.area   = n -> area;
    cnethead.term   = n -> term;

    cnet.net_neqv = outNod;
    if (isSubstrNode) {
	dmPutNet ();
	new_net_neqv++;
    }
#else
    sprintf (netattr, "cd=%d;lay=%d", i, n -> mask);
    if (n -> area) sprintf (netattr + strlen (netattr), ";a=%d", n -> area);
    if (n -> term) sprintf (netattr + strlen (netattr), ";t=%d", n -> term);
    if (optTorPos) addCoorXY (netattr, n -> node_x, n -> node_y);
    cnet.net_neqv = net_neqv;
    dmPutNet ();
#endif

    if (n -> names) {
	for (tm = n -> names; tm; tm = tm -> next) {
	    fill_cnet_term (tm);
	    dmPutNet ();
	    new_net_neqv++;
	}
	cnet.net_dim = cnet.inst_dim = 0; /* reset */
    }

    cnet.net_name[1] = '\0';

    while ((ne = n -> netEq)) {
	n -> netEq = ne -> next;

	switch (ne -> instType) {
	    case RES :
		outResistor (ne -> number, outNod, ne -> sort, ne -> val);
		break;
	    case BJT :
		sprintf (cnet.inst_name, "_BJT%d", ne -> number);
		cnet.net_name[0] = ne -> term;
		dmPutNet ();
		new_net_neqv++;
		break;
	    case TOR :
		sprintf (cnet.inst_name, "_T%d", ne -> number);
		cnet.net_name[0] = ne -> term;
		dmPutNet ();
		new_net_neqv++;
		break;
	    default:
		ASSERT (ne -> instType == TOR);
	}

	DISPOSE (ne, sizeof(netEquiv_t)); currNeqv--;
    }

    if (!isSubstrNode) {

	for (i = 0; i < resSortTabSize; i++) {
	    while ((con = n -> con[i])) {
		on = OTHER (con, n);
		addNetEqRES (on, outNod, i, con -> val);
		elemDelRes (con);
	    }
	    if (n -> substrCon[i]) outResistor (0, outNod, i, n -> substrCon[i]);
	}

	while ((ne = n -> netEq2)) {
	    n -> netEq2 = ne -> next;
	    outCapacitor (n, ne, outNod, ne -> sort, ne -> val);
	    DISPOSE (ne, sizeof(netEquiv_t)); currNeqv--;
	}

	for (i = 0; i < capSortTabSize; i++) {
	    while ((cap = n -> cap[i])) {
		if (cap -> val != 0) {
		    on = OTHER (cap, n);
		    addNetEqCAP (on, outNod, i, cap -> val);
		}
		elemDelCap (cap);
	    }
	    if (n -> gndCap[i]) outCapacitor (n, NULL, outNod, i, n -> gndCap[i]);
	    if (n -> substrCap[i]) outCapacitor (n, NULL, -outNod, i, n -> substrCap[i]);
	}
    }
    else { /* isSubstrNode */
	if (substrToGnd_cnt) {
	    for (i = 0; i < capSortTabSize; i++) {
		if (n -> gndCap[i]) outCapacitor (NULL, NULL, outNod, i, n -> gndCap[i]);
	    }
	}
    }

#ifdef CIR_NET_HEAD
    cnethead.net_neqv = new_net_neqv;
    if (new_net_neqv == 0) cnethead.offset = 0;
    dmPutDesignData (dmsNethead, CIR_NET_HEAD);
#else
    if (new_net_neqv != net_neqv) {
fprintf (stderr, "-- outNode: net_neqv=%d new_net_neqv=%d\n", net_neqv, new_net_neqv);
     // ASSERT (new_net_neqv == net_neqv);
    }
#endif
ret:
    nodeDel (n);
}

Private void fill_cnet_term (terminal_t *tm)
{
    strcpy (cnet.net_name, truncDmName (tm -> termName));
    if (tm -> termX < 0 && tm -> termY < 0) {
	cnet.net_dim = 0;
    }
    else if (tm -> termX >= 0 && tm -> termY >= 0) {
	cnet.net_dim = 2;
	cnet.net_lower[0] = tm -> termX;
	cnet.net_lower[1] = tm -> termY;
    }
    else if (tm -> termX >= 0) {
	cnet.net_dim = 1;
	cnet.net_lower[0] = tm -> termX;
    }
    else {
	cnet.net_dim = 1;
	cnet.net_lower[0] = tm -> termY;
    }

    if (tm -> instName) {
	strcpy (cnet.inst_name, tm -> instName);
	if (tm -> instX < 0 && tm -> instY < 0) {
	    cnet.inst_dim = 0;
	}
	else if (tm -> instX >= 0 && tm -> instY >= 0) {
	    cnet.inst_dim = 2;
	    cnet.inst_lower[0] = tm -> instX;
	    cnet.inst_lower[1] = tm -> instY;
	}
	else if (tm -> instX >= 0) {
	    cnet.inst_dim = 1;
	    cnet.inst_lower[0] = tm -> instX;
	}
	else {
	    cnet.inst_dim = 1;
	    cnet.inst_lower[0] = tm -> instY;
	}
    }
    else {
	cnet.inst_name[0] = '\0';
	cnet.inst_dim = 0;
    }

    if (tm -> type != tTerminal) {
	int i = strlen (netattr);
	if (i) netattr[i++] = ';';
	sprintf (netattr + i, "type=label");
    }

    if (optTorPos) addCoorXY (netattr, tm -> x, tm -> y);
}

void outGndNode ()
{
    cnet.net_neqv = gndCap_cnt + substrToGnd_cnt;
    if (cnet.net_neqv == 0) return;

    outNod++;
#ifdef CIR_NET_HEAD
    cnethead.cd_nr  = ++outConduc;
    cnethead.node_nr= outNod;
    cnethead.node_x = 0;
    cnethead.node_y = 0;
    cnethead.net_neqv= 1;
    cnethead.offset = ftello (dmsNet -> dmfp);
    cnethead.lay_nr = -1;
    cnethead.area   = 0;
    cnethead.term   = 0;
    dmPutDesignData (dmsNethead, CIR_NET_HEAD);
    cnet.net_neqv = outNod;
#else
    sprintf (netattr, "cd=%d;lay=-1", ++outConduc);
#endif
    cnet.inst_name[0] = '\0';
    strcpy (cnet.net_name, nameGND);
    dmPutDesignData (dmsNet, CIR_NET_ATOM);
    *netattr = '\0';
}

Private char *Val (double val, int i)
{
    static char buf[60];
    int ii, j, exp;

    if (val == 0) return ("0");
    sprintf (buf, "%.*e", i, val);
    i += 2;
    j = 1;
    if (buf[j] != '.') ++j;
    if (buf[j] == '.')
    if (buf[i] == 'e' || buf[++i] == 'e') {
	ii = i; i += 2; exp = 0;
	while (buf[i] == '0') ++i;
	while (buf[i]) exp = exp*10 + (buf[i++] - '0');
	i = ii - 1;
	while (buf[i] == '0') --i;
	if (i != j) ++i;
	if (exp) {
	    if (buf[ii+1] == '-') {
		if (exp <= 3) {
		    if (i == j) ++i;
		    buf[j] = buf[j-1];
		    buf[j-1] = '.';
		    while (exp-- > 1) {
			for (ii = i - 1; ii >= j; --ii) buf[ii+1] = buf[ii];
			++i;
			buf[j++] = '0';
		    }
		    buf[i] = '\0';
		}
		else sprintf (&buf[i], "e-%d", exp);
	    }
	    else {
		if (i == j && exp <= 2) {
		    while (--exp >= 0) buf[i++] = '0';
		    buf[i] = '\0';
		}
		else if (i > j && exp <= i - j + 1) {
		    while (--exp >= 0) {
			if (++j < i) buf[j-1] = buf[j]; else buf[j-1] = '0';
		    }
		    if (j+1 < i) buf[j] = '.'; else i = j;
		    buf[i] = '\0';
		}
		else sprintf (&buf[i], "e%d", exp);
	    }
	}
	else buf[i] = '\0';
    }
    return (buf);
}

void outResistor (int ne_number, int number, int sort, double val)
{
    char buf[80];

    if (Abs (val) < MIN_CONDUCTANCE) { ++minREScount; return; }

    if (ne_number) {
	if (ne_number < number)
	    sprintf (cmc.inst_name, "_R%d_%d", ne_number, number);
	else
	    sprintf (cmc.inst_name, "_R%d_%d", number, ne_number);
    }
    else {
	++substrRes_cnt;
	sprintf (cmc.inst_name, "_RS%d", number);
    }

    strcpy (cmc.cell_name, resSortTab[sort]);

    sprintf (buf, "v=%s", Val(1 / val, 6));
 // sprintf (buf, "g=%s", Val(val, 6));
    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);
    outRes++;
}

void outCapacitor (node_t *n, netEquiv_t *ne, int number, int sort, double val)
{
    char buf[60];

    if (ne) {
	ASSERT (number > 0);
	if (capPolarityTab[sort] == 'n')
	    sprintf (cmc.inst_name, "_C%d_%d", number, ne -> number);
	else
	    sprintf (cmc.inst_name, "_C%d_%d", ne -> number, number);
    }
    else if (n) {
	if (number > 0) {
	    ++gndCap_cnt;
	    if (capPolarityTab[sort] == 'n')
		sprintf (cmc.inst_name, "_CG_%d", number);
	    else
		sprintf (cmc.inst_name, "_CG%d", number);
	}
	else {
	    ++substrCap_cnt;
	    if (capPolarityTab[sort] == 'n')
		sprintf (cmc.inst_name, "_CS_%d", -number);
	    else
		sprintf (cmc.inst_name, "_CS%d", -number);
	}
    }
    else {
	if (capPolarityTab[sort] == 'n')
	    sprintf (cmc.inst_name, "_CSG_%d", sort);
	else
	    sprintf (cmc.inst_name, "_CSG%d", sort);
    }

    strcpy (cmc.cell_name, capSortTab[sort]);

    if (capAreaPerimEnable[sort] == 2) val /= capOutFac[sort];
        /* Transform value back to area or perimeter
           (for junction capacitance information). */

    sprintf (buf, "v=%s", Val(val,6));
    if (capAreaPerimEnable[sort] == 1) {
	if (capAreaPerimType[sort] == 'a')
	    sprintf (buf + strlen (buf), ";a=%d", sort);
	else
	    sprintf (buf + strlen (buf), ";p=%d", sort);
    }
    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);
    outCap++;
}

void outVBJT (BJT_t *vT)
{
    int i, inst_n;
    char buf[80];

    if (!extrPass) return;

    inst_n = outpVBJT + outpLBJT + 1;

    if (!vT -> n[SU] && vT -> type -> s.bjt.sCon == -4) {
	say ("warning: no substrate area found for bjt '%s' at position %s\n",
	    vT -> type -> name, strCoorBrackets (vT -> pn[EM] -> xl, vT -> pn[EM] -> yb));
    }

    addNetEq (vT -> n[BA], BJT, 'b', inst_n);
    addNetEq (vT -> n[CO], BJT, 'c', inst_n);
    addNetEq (vT -> n[EM], BJT, 'e', inst_n);
    if (vT -> n[SU]) addNetEq (vT -> n[SU], BJT, 's', inst_n);

    strcpy (cmc.cell_name, vT -> type -> name);
    sprintf (cmc.inst_name, "_BJT%d", inst_n);

    sprintf (buf, "ae=%.2e;pe=%.2e", vT -> area, vT -> length);

    if (vT -> scalef > 1) sprintf (buf + strlen (buf), ";scalef=%d", vT -> scalef);

    if (optTorPos) addCoorXY (buf, vT -> pn[EM] -> xl, vT -> pn[EM] -> yb);

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);

    outpVBJT++;
}

void outLBJT (BJT_t *lT)
{
    int i, inst_n;
    char buf[80];
    polnode_t *pnEM;

    if (!extrPass) return;

    inst_n = outpVBJT + outpLBJT + 1;

    pnEM = lT -> pn[EM];

    if (!lT -> n[SU] && lT -> type -> s.bjt.sCon == -4) {
	say ("warning: no substrate area found for bjt '%s' at position %s\n",
	    lT -> type -> name, strCoorBrackets (pnEM -> xl, pnEM -> yb));
    }

    addNetEq (lT -> n[CO], BJT, 'c', inst_n);
    addNetEq (lT -> n[BA], BJT, 'b', inst_n);
    addNetEq (lT -> n[EM], BJT, 'e', inst_n);
    if (lT -> n[SU]) addNetEq (lT -> n[SU], BJT, 's', inst_n);

    strcpy (cmc.cell_name, lT -> type -> name);
    sprintf (cmc.inst_name, "_BJT%d", inst_n);

    sprintf (buf, "ae=%.2e;pe=%.2e;wb=%.2e", pnEM -> area, pnEM -> length, meters * lT -> basew);

    if (lT -> scalef > 1) sprintf (buf + strlen (buf), ";scalef=%d", lT -> scalef);

    if (optTorPos) addCoorXY (buf, pnEM -> xl, pnEM -> yb);

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);

    outpLBJT++;
}

Private void outputTransistor (transistor_t *t)
{
    dsBoundary_t *dsBdr;
    polnode_t *pn;
    double w, W, p, a, n;
    char c;
    char buf[256];

    if (t -> tor_nr) { /* valid tor */
	sprintf (cmc.inst_name, "_T%d", t -> tor_nr);
	strcpy (cmc.cell_name, t -> type -> name);

	sprintf (buf, "w=%s", Val(t -> dsPerimeter,4));
	sprintf (buf + strlen(buf), ";l=%s", Val(t -> totPerimeter,4));

	for (dsBdr = t -> boundaries; dsBdr; dsBdr = dsBdr -> next)
	if ((pn = dsBdr -> pn)) {
	    w = t -> dsPerimeter;
	    c = dsBdr -> type;
	    W = pn->ds_wtot;
	    p = pn->ds_peri * ((w * W + 2 * pn->ds_area) / (W * W + 2 * pn->ds_area * pn->ds_tcnt));
	    a = pn->ds_area * (w / W);
	    n = a / (w * w);
	    sprintf (buf + strlen(buf), ";a%c=%s", c, Val(a,4));
	    sprintf (buf + strlen(buf), ";p%c=%s", c, Val(p,4));
	    sprintf (buf + strlen(buf),";nr%c=%s", c, Val(n,4));
	}

	if (optTorPos) addCoorXY (buf, t -> xl, t -> yb);

	cmc.inst_attribute = buf;
	dmPutDesignData (dmsMc, CIR_MC);
	outTor++;
    }
    torDel (t);
}

void outputReadyTransistor (polnode_t *pnR, pnTorLink_t *tl)
{
    pnTorLink_t *tlp, *tln;
    polnode_t *pn, *PN[8];
    dsBoundary_t *dsBdr;
    transistor_t *t;
    int i, j;

    t = (transistor_t*) tl -> tor;
    ASSERT (!t -> subs);
    /* check if all boundary polnodes are ready */
    i = 0;
    for (dsBdr = t -> boundaries; dsBdr; dsBdr = dsBdr -> next) {
	if ((pn = dsBdr -> pn)) {
	    if (pn != pnR) {
		if (pn -> subs) return; /* polnode not ready */
		for (tl = pn -> tors; tl; tl = tl -> next) {
		    if (tl -> type == TORELEM && ((transistor_t*)tl -> tor) -> subs) {
			return; /* gate not ready */
		    }
		}
	    }
	    /* two boundaries can have the same polnode */
	    for (j = 0; j < i && PN[j] != pn; ++j) ;
	    if (j == i) PN[i++] = pn;
	}
	else fprintf (stderr, "outputReadyTransistor: missing polnode\n");
    }

    outputTransistor (t);

    for (j = 0; j < i; ++j) {
	pn = PN[j];
	tlp = NULL;
	for (tl = pn -> tors; tl; tl = (tlp = tl) -> next)
	    if (tl -> tor == (BJT_t*)t) break;
	ASSERT (tl);
	if (tlp) tlp -> next = tl -> next;
	else pn -> tors = tl -> next;
	DISPOSE (tl, sizeof(pnTorLink_t));
	if (pn != pnR && !pn -> tors && !pn -> juncs) polnodeDispose (pn);
    }
}

void outTransistor (transistor_t *t)
{
    double w, l;
    int i, j, dsDone, sDone, gDone, term;
    netEquiv_t *ne;
    subnode_t *SN[8];
    node_t *n;
    dsCoor_t *point, *nextPoint;
    dsBoundary_t *dsBdr;
    char buf[64];

    ASSERT (extrPass);

    dsDone = sDone = gDone = 0;
    ++tor_cnt;

    if (!(dsBdr = t -> boundaries)) {
	if (!dsWarning0++) {
	    say ("%s\n       at position %s, %s",
		"transistor with no drain/source terminals detected",
		strCoorBrackets (t -> xl, t -> yb),
		omit_inc_tors ? "transistor skipped!" : "no terminals added");
	}
	if (omit_inc_tors) { --tor_cnt; goto ret; }
    }
    else {

    term = dsBdr -> type;

    do { /* first round for making the equivalences */
	if (term == 'x') dsBdr -> type = dsDone ? 's' : 'd';
	addNetEq (dsBdr -> dsCond -> node, TOR, dsBdr -> type, tor_cnt);
	if (dsBdr -> type == 's') sDone++;
	dsDone++;
	if (dsBdr -> points) gDone++; /* the dsBdr is not a closed ring */
    } while ((dsBdr = dsBdr -> next));

    if (sDone == 0 || dsDone == sDone) { /* only d or s boundaries */
	if (!dsWarning1++) {
	    say ("%s\n       at position %s, %s",
		"transistor with only drain or source terminal detected",
		strCoorBrackets (t -> xl, t -> yb),
		omit_inc_tors ? "transistor skipped!" :
		(optDsConJoin && term == 'x' ? "second terminal added" : "no terminal added"));
	}
	if (omit_inc_tors) {
	    for (dsBdr = t -> boundaries; dsBdr; dsBdr = dsBdr -> next) {
		n = dsBdr -> dsCond -> node;
		n -> n_n_cnt--;
		ne = n -> netEq;
		n -> netEq = ne -> next;
		DISPOSE (ne, sizeof(netEquiv_t)); currNeqv--;
	    }
	    --tor_cnt;
	    goto ret;
	}
	if (optDsConJoin && term == 'x') { /* add second terminal */
	    n = t -> boundaries -> dsCond -> node;
	    addNetEq (n, TOR, 's', tor_cnt);
	}
    }
    else if (dsDone > 2) {
	if (!dsWarning) { dsWarning = 1;
	    say ("%s\n       at position %s",
		"transistor with more than two drain/source terminals detected",
		strCoorBrackets (t -> xl, t -> yb));
	}
    } }

    if (t -> gate) {
	n = t -> gate -> node;
	SET_TERM (n);
	addNetEq (n, TOR, 'g', tor_cnt);
    }
    else ASSERT (t -> gate);

    if (t -> bulk) {
	n = t -> bulk -> node;
	SET_TERM (n);
	addNetEq (n, TOR, 'b', tor_cnt);
    }
    else if (t -> type -> s.tor.bCon == -4) {
	say ("warning: no substrate area found for transistor '%s' at position %s\n",
	    t -> type -> name, strCoorBrackets (t -> xl, t -> yb));
    }

    /* calculate w and l */
    l = t -> totPerimeter - t -> dsPerimeter;
    if (gDone > 0 && l > 0.0) {
	l = l / gDone;
	w = t -> surface / l;
    } else {
	w = t -> totPerimeter / 2;
	l = t -> surface / w;
    }

    t -> totPerimeter = l;
    t -> dsPerimeter  = w;
    t -> tor_nr = tor_cnt;

ret:
    /* if a tor has boundaries, then the tor is ready when all drains are ready
     * note that this is a special case, because normaly the gate is first ready
     * if the tor has no drains, then the tor is ready, no special parameters
     * note that outputTransistor does only output if tor is valid */

    sDone = i = 0;
    dsBdr = t -> boundaries;
    while (dsBdr) {          /* second round (cannot combined with first
				round since tors may be outputted now) */
	point = dsBdr -> points;
	while (point) {
	    nextPoint = point -> next;
	    DISPOSE (point, sizeof(dsCoor_t));
	    point = nextPoint;
	}
	if (dsBdr -> pn) { ++i;
	    dsBdr -> pn -> ds_wtot += t -> dsPerimeter; /* w */
	    dsBdr -> pn -> ds_tcnt++;
	}
	SN[sDone++] = dsBdr -> dsCond;
	dsBdr = dsBdr -> next;
    }

    if (i) {
	if (i != dsDone) fprintf (stderr, "outTransistor: missing polnode for ds-boundary!\n");
    } else {
	outputTransistor (t);
    }

    /* if there are polnodes, subnodeDel can output current transistor */
    for (j = 0; j < sDone; ++j) {
	subnodeDel (SN[j]);
	DISPOSE (SN[j], sizeof(subnode_t));
    }
}

void groupDel (group_t *grp)
{
    DISPOSE (grp, sizeof(group_t));
    currIntGrp--;
}

void nodeDel (node_t *n)
{
    group_t *grp = Grp (n);

    if (n -> clr) {
	if (ready_grp) {
fprintf (stderr, "\nwarning: readyGroup: cluster node found! (t=%d)\n", n->term);
	    removeNodeFromCluster (n, 5);
	}
	ASSERT (!n -> clr);
    }

    grp -> nod_cnt--;

    if (n -> gnext) n -> gnext -> gprev = n -> gprev;
    if (grp -> nodes == n) grp -> nodes = n -> gnext;
    else {
	n -> gprev -> gnext = n -> gnext;
	if (!n -> gnext) grp -> nodes -> gprev = n -> gprev;
    }

    if (n -> pols) {
	while (n -> pols) nodeLinkDel (n -> pols);
	ASSERT (grp -> notReady >= 0);
    }
    disposeNode (n);
}

void torDel (transistor_t *t)
{
    dsBoundary_t *dsBdr, *nextDsBdr;

    if ((dsBdr = t -> boundaries)) {
	do {
	    nextDsBdr = dsBdr -> next;
	    DISPOSE (dsBdr, sizeof(dsBoundary_t));
	} while ((dsBdr = nextDsBdr));
    }
    currIntTor--;
    DISPOSE (t, sizeof(transistor_t));
}

void addNetEqCAP (node_t *n, int nr, int sort, double val)
{
    netEquiv_t *ne, *e, *p;

    ASSERT (n != nSUB);

    p = NULL;
    e = n -> netEq2;
    while (e && e -> number >  nr) e = (p = e) -> next;
    while (e && e -> number == nr) {
	if (e -> sort == sort) { e -> val += val; return; }
	e = e -> next;
    }
    ne = NEW (netEquiv_t, 1); if (++currNeqv > maxNeqv) maxNeqv = currNeqv;
    ne -> val = val;
    ne -> sort = sort;
 // ne -> instType = CAP;
    ne -> number = nr;
    if (p) {
	ne -> next = p -> next;
	p -> next = ne;
    }
    else {
	ne -> next = n -> netEq2;
	n -> netEq2 = ne;
    }
    n -> cap_cnt++;
}

netEquiv_t *putNetEqCAP (node_t *n, netEquiv_t *ne, netEquiv_t *p)
{
    netEquiv_t *e;

    e = p ? p -> next : n -> netEq2;
    while (e && e -> number >  ne -> number) e = (p = e) -> next;
    while (e && e -> number == ne -> number) {
	if (e -> sort == ne -> sort) {
	    e -> val += ne -> val;
	    DISPOSE (ne, sizeof(netEquiv_t)); currNeqv--;
	    return p;
	}
	e = e -> next;
    }
    if (p) {
	ne -> next = p -> next;
	p -> next = ne;
    }
    else {
	ne -> next = n -> netEq2;
	n -> netEq2 = ne;
    }
    n -> cap_cnt++;
    return p;
}

void addNetEqRES (node_t *n, int number, int sort, double val)
{
    netEquiv_t *ne, *e, *p;

    ASSERT (n != nSUB);

    if (Abs (val) < MIN_CONDUCTANCE) { ++addREScount; return; }

    p = NULL;
    e = n -> netEq;
    while (e && e -> instType !=  RES) e = (p = e) -> next;
    while (e && e -> number >  number) e = (p = e) -> next;
    while (e && e -> number == number) {
	if (e -> sort == sort) { e -> val += val; return; }
	e = e -> next;
    }
    ne = NEW (netEquiv_t, 1); if (++currNeqv > maxNeqv) maxNeqv = currNeqv;
    ne -> val = val;
    ne -> sort = sort;
    ne -> instType = RES;
    ne -> number = number;
    if (p) {
	ne -> next = p -> next;
	p -> next = ne;
    }
    else {
	ne -> next = n -> netEq;
	n -> netEq = ne;
    }
    n -> res_cnt++;
}

netEquiv_t *putNetEqRES (node_t *n, netEquiv_t *ne, netEquiv_t *p)
{
    netEquiv_t *e;

    if (p) e = p -> next;
    else {
	e = n -> netEq;
	while (e && e -> instType != RES) e = (p = e) -> next;
    }
    /* p is last !RES */
    /* e is first RES */
    while (e && e -> number >  ne -> number) e = (p = e) -> next;
    while (e && e -> number == ne -> number) {
	if (e -> sort == ne -> sort) {
	    e -> val += ne -> val;
	    DISPOSE (ne, sizeof(netEquiv_t)); currNeqv--;
	    return p;
	}
	e = e -> next;
    }
    if (p) {
	ne -> next = p -> next;
	p -> next = ne;
    }
    else {
	ne -> next = n -> netEq;
	n -> netEq = ne;
    }
    if (ne -> instType != RES) return ne;
    n -> res_cnt++;
    return p;
}

Private void addNetEq (node_t *n, int instType, int pin, int nr)
{
    netEquiv_t *ne;

    ne = NEW (netEquiv_t, 1); if (++currNeqv > maxNeqv) maxNeqv = currNeqv;
    ne -> instType = instType;
    ne -> term     = pin;
    ne -> number   = nr;
    n -> n_n_cnt++;

    ne -> next = n -> netEq;
    n -> netEq = ne;
}
