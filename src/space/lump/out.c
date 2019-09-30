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
#ifdef PLOT_CIR_MODE
#include "src/space/auxil/plot.h"
#endif
#include "src/space/lump/define.h"
#include "src/space/lump/extern.h"
#include "src/space/scan/export.h"
#ifdef DISPLAY
#include "src/space/X11/export.h"
#endif
#include "src/space/bipolar/define.h"
#include "src/space/bipolar/export.h"
#include "src/space/extract/define.h"

DM_STREAM *dmsNet = NULL;
DM_STREAM *dmsMc  = NULL;
DM_STREAM *dmsCon = NULL;
#ifdef DMS_TERM_POS
DM_STREAM *dmsTermpos = NULL;
#endif
DM_STREAM *dmsDevgeo  = NULL;

int res_cnt;
int cap_cnt;
int gndCap_cnt;
int substrCap_cnt;
int substrRes_cnt;
int substrToGnd_cnt;
int node_cnt;
int tor_cnt;
int incrConduc;
int outConduc;
int act_dev_cnt;

int *GCP = NULL; /* not necessary to do explicity, but it MUST be NULL */
int GCP_cnt;
int *SCP = NULL; /* not necessary to do explicity, but it MUST be NULL */
int SCP_cnt;
char * defGndCapPol;
char * defGndCapOtherPol;
char * defSubCapPol;
char * defSubCapOtherPol;

static DM_CELL* saveCellCirKey;
static DM_CELL* saveCellLayKey;
static int isSubstrNode;

static int group_out_node_cnt;
static char * grp_node_sep;
static bool_t makeMaskXYName;
static char * maskXYNamePrefix;

static char groupName[1024];
static terminal_t *tmGrp;
static node_t *new_nodes = NULL;

static int *gndCapFlag = NULL;
static int *substrCapFlag = NULL;
static int dsWarning = 0;
int dsWarning0 = 0;
int dsWarning1 = 0;
static int maskXYnote = 0;

#ifdef DEBUG_NODES
static int check_cnt, leftBehind, hasPrinted;
FILE *fp_dbn;
#endif

#ifndef NOPACK
#define dmPUT _dmPack
#else
#define dmPUT _dmDoput
#endif

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private void outNode (node_t *n);
Private void fill_cnet_term (terminal_t *term);
Private void print_cnet (FILE *fp);
Private void outResistor  (node_t *n, element_r *el, int number, int sort);
Private void outCapacitor (node_t *n, element_c *el, int number, int sort);
Private int markAreaPerimElement (node_t *n, element_c *el, int number, int sort);
Private void outAreaPerimElement (node_t *n, element_c *el, int number, int sort);
Private void addNetEq (node_t *n, int type, int term, int nr, char *instName);
Private void outTileConnectivity (group_t *grp);
Private void setGndCapPol (void);
Private void setSubCapPol (void);
#ifdef DEBUG_NODES
Private void debug_nodes_init (void);
#endif
#ifdef MOMENTS
Private void outputSeriesRC (node_t *node);
Private void outputSeriesL  (node_t *node);
#endif
#ifdef __cplusplus
  }
#endif

extern bool_t compact_attr_vals;
extern double MIN_CONDUCTANCE;

void initOut (DM_CELL *cellCirKey, DM_CELL *cellLayKey)
{
    struct stat buf;
    static int cnetInit = 0;

    if (!grp_node_sep) {
	grp_node_sep = paramLookupS ("net_node_sep", "_");
	makeMaskXYName = paramLookupB ("node_pos_name", "off");
	maskXYNamePrefix = paramLookupS ("pos_name_prefix", "");
    }
    dsWarning = dsWarning0 = dsWarning1 = 0;

    if (extrPass) {
	group_out_node_cnt = 0;
	dmsNet = dmOpenStream (cellCirKey, "net", "w");
	dmsMc  = dmOpenStream (cellCirKey, "mc", "w");

	if (optBackInfo > 1) {
#ifdef DMS_TERM_POS
	    dmsTermpos = dmOpenStream (cellCirKey, "termpos", "w");
#endif
	    dmsDevgeo  = dmOpenStream (cellCirKey, "devgeo", "w");
	    fprintf (dmsDevgeo -> dmfp, "#1\n"); /* version */
	}
    }

    if (optBackInfo > 1) { /* extrPass */
	dmsCon = dmOpenStream (cellCirKey, "congeo", "w");
	fprintf (dmsCon -> dmfp, "#1\n"); /* version */
    }

    if (prePass1 != 2) {
#define dmUNLINK(x) {if (!dmStat (cellCirKey, x, &buf)) dmUnlink (cellCirKey, x);}
	if (!dmsCon)     dmUNLINK ("congeo");
#ifdef DMS_TERM_POS
	if (!dmsTermpos) dmUNLINK ("termpos");
#endif
	if (!dmsDevgeo)  dmUNLINK ("devgeo");
    }

    res_cnt = 0;
    cap_cnt = 0;
    gndCap_cnt = 0;
    substrCap_cnt = 0;
    substrRes_cnt = 0;
    substrToGnd_cnt = 0;
    isSubstrNode = 0;
    node_cnt = 0;
    tor_cnt = 0;
    outConduc = 0;

    act_dev_cnt = 0;

    saveCellCirKey = cellCirKey;
    saveCellLayKey = cellLayKey;

    GCP_cnt = 0;
    SCP_cnt = 0;
    defGndCapPol = NULL;
    defSubCapPol = NULL;

    if (!cnetInit) {
	cnet.net_dim = 0;
	cnet.ref_dim = 0;
	cnet.inst_dim = 0;
	cnet.net_upper  = cnet.net_lower  = NEW (long, 2);
	cnet.inst_upper = cnet.inst_lower = NEW (long, 2);
	cnetInit = 1;

	gndCapFlag = NEW (int, capSortTabSize);
	substrCapFlag = NEW (int, capSortTabSize);
    }

#ifdef DEBUG_NODES
    debug_nodes_init ();
#endif
}

void endOut ()
{
#define dmCLOSE(x) if (x) dmCloseStream (x, COMPLETE), x = NULL

    if (extrPass) {
	dmCLOSE (dmsNet);
	dmCLOSE (dmsMc);
#ifdef DMS_TERM_POS
	dmCLOSE (dmsTermpos);
#endif
	dmCLOSE (dmsDevgeo);
    }
    dmCLOSE (dmsCon);

    outSubTerm = substrRes_cnt;
}

void outGroup (node_t **qn, int n_cnt)
{
    int i;
    group_t *grp = Grp (qn[0]);

    if (qn[0] == nSUB) {
	ASSERT (isSubstrNode == 0);
	isSubstrNode = 1;
	ASSERT (n_cnt == 1);
    } else {
	ASSERT (n_cnt >= 1);
    }

    if ((tmGrp = grp -> name)) {
	ASSERT (!tmGrp -> instName);
	sprintf (groupName, "%s%s", tmGrp -> termName,
		makeIndex (tmGrp -> termX >= 0 ? 1 : -1,
			tmGrp -> termY >= 0 ? 1 : -1,
			tmGrp -> termX, tmGrp -> termY));
    }
    else groupName[0] = '\0';

    outGrp++;

#ifdef DEBUG_NODES
    fprintf (fp_dbn, "Outputting group#%d grp=%p name=%s qn[0]=%p n_cnt=%d\n",
	outGrp, grp, groupName, qn[0], n_cnt);
#endif

    if (dmsCon) fprintf (dmsCon -> dmfp, "-%d ( ", outGrp);

    if (optReduc2 && currIntCap > 0) reducGroupII (qn, n_cnt);

if (isSubstrNode) {
    outNode (nSUB);
}
else {
    incrConduc = 0;

    for (i = 0; i < n_cnt; i++) {
#ifdef MOMENTS
	if (optRes) {
	    if (doOutRC) outputSeriesRC (qn[i]);
	    if (doOutL)  outputSeriesL  (qn[i]);
	}
#endif
	outNode (qn[i]);
    }

    /* also output new nodes that have been created to model higher-order moments */
    while (new_nodes) {
	outNode (new_nodes);
	new_nodes = new_nodes -> gnext;
    }

    if (incrConduc) outConduc++;
}

    if (dmsCon) outTileConnectivity (grp);

    groupDel (grp);
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

Private void outNode (node_t *n)
{
    int checkTermName, i, net_neqv, new_net_neqv;
    register element_c *cap;
    register element_r *con;
    terminal_t *tm, *tmHead;
    netEquiv_t *ne, *ne2, *next_ne;
    char *s, buf[132], name_buf[1024];
    char maskbuf[DM_MAXLAY + 6];
    char net_name[DM_MAXNAME + 1];

    if (!extrPass) {
	if (substrRes)
	for (con = n -> con[0]; con; con = NEXT (con, n)) {
	    elemDelRes (con);
	}
	goto ret;
    }

    ASSERT (!n -> delayed);

    for (i = 0; i < capSortTabSize; i++) {
	gndCapFlag[i] = 0;
	substrCapFlag[i] = 0;
    }

    /* First we are going to count how many subnets (equivalences)
       we have, and if we should really output the net.
       The following code must be an exact reflection of what is
       done later on when we write the subnets !!!!!!!!
    */

    /* For comment, see also the second part. */

    net_neqv = n -> n_n_cnt;

    if (!isSubstrNode) {

	if (n -> res_cnt > 0)
	    for (i = 0; i < resSortTabSize; i++)
	    for (con = n -> con[i]; con; con = NEXT (con, n)) {
		if (Abs (con -> val) >= MIN_CONDUCTANCE
#ifdef MOMENTS
		&& !(con -> val < 0 && !printMoments && !doOutNegR)
#endif
			) net_neqv++;
		else con -> val = 0;
	    }

	if (n -> cap_cnt > 0)
	    for (i = 0; i < capSortTabSize; i++)
	    for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
		if (capAreaPerimEnable[i] == 1) {
		    if (!cap -> flagEl) markAreaPerimElement (n, cap, 2, i);
		}
#ifdef MOMENTS
		else if (printMoments) ;
#endif
		else if (cap -> val <= 0) {
		    if (cap -> val == 0) cap -> flagEl = 1;
#ifdef MOMENTS
		    else if (!doOutNegC && !doOutL) cap -> flagEl = 1;
#endif
		}
		if (!cap -> flagEl) net_neqv++;
	    }

	if (!net_neqv && !n -> names && n -> term != 2 && !n -> res_cnt && !n -> cap_cnt) goto ret;

	for (i = 0; i < capSortTabSize; i++) {

	    if (n -> gndCap[i] != 0) {
		if (capAreaPerimEnable[i] == 1) {
		    if (gndCapFlag[i] == 0 && markAreaPerimElement (n, NULL, 1, i)) net_neqv++;
		}
#ifdef MOMENTS
		else if (n -> gndCap[i] < 0 && !printMoments && !doOutNegC && !doOutL) gndCapFlag[i] = 1;
#endif
		else if (min_ground_cap <= 0 || Abs (n -> gndCap[i]) >= min_ground_cap) net_neqv++;
		else gndCapFlag[i] = 1;
	    }
	    else gndCapFlag[i] = 1; /* skip */

	    if (n -> substrCap[i] != 0) {
		if (capAreaPerimEnable[i] == 1) {
		    if (substrCapFlag[i] == 0 && markAreaPerimElement (n, NULL, -1, i)) net_neqv++;
		}
#ifdef MOMENTS
		else if (n -> substrCap[i] < 0 && !printMoments && !doOutNegC && !doOutL) substrCapFlag[i] = 1;
#endif
		else if (min_ground_cap <= 0 || Abs (n -> substrCap[i]) >= min_ground_cap) net_neqv++;
		else substrCapFlag[i] = 1;
	    }
	    else substrCapFlag[i] = 1; /* skip */
	}

	if (optRes) {
	    for (i = 0; i < resSortTabSize; i++)
		if (Abs (n -> substrCon[i]) >= MIN_CONDUCTANCE
#ifdef MOMENTS
		&& !(n -> substrCon[i] < 0 && !printMoments && !doOutNegR)
#endif
			) net_neqv++;
		else n -> substrCon[i] = 0; /* skip */
	}
    }
    else {
	ASSERT (n -> res_cnt == 0);
	ASSERT (n -> cap_cnt == 0);

	if (nameGND != nameSUBSTR) {
	    for (i = 0; i < capSortTabSize; i++) {
		if (n -> gndCap[i] != 0) {
		    if (n -> gndCap[i] < 0 && no_neg_cap) gndCapFlag[i] = 1;
		    else if (!optCoupCap && i == 0) gndCapFlag[i] = 1;
		    else if (capAreaPerimEnable[i] == 1) {
			if (gndCapFlag[i] == 0) {
			    markAreaPerimElement (NULL, NULL, -2, i);
			    substrToGnd_cnt++;
			}
		    }
#ifdef MOMENTS
		    else if (n -> gndCap[i] < 0 && !printMoments && !doOutNegC && !doOutL) gndCapFlag[i] = 1;
#endif
		    else substrToGnd_cnt++;
		}
		else gndCapFlag[i] = 1; /* skip */
	    }
	    net_neqv += substrRes_cnt + substrCap_cnt + substrToGnd_cnt;
	}
	else net_neqv += substrRes_cnt + substrCap_cnt + gndCap_cnt;

	if (!net_neqv && !n -> names) goto ret; /* unconnected */
    }

    /* Now, we do the real writing and memory de-allocation */

    tm = tmHead = NULL;
    checkTermName = 0;

    if (isSubstrNode) {
	strcpy (cnet.net_name, nameSUBSTR);
	/* SdeG: if nameSUBSTR == terminal-name, don't output subnet */
	for (tm = n -> names; tm; tm = tm -> next) {
	    if (!tm -> instName && tm -> termX < 0 && tm -> termY < 0) {
		if (strcmp (tm -> termName, nameSUBSTR) == 0) {
		    tmHead = tm;
		    if (tm -> type == tTerminal) break;
		}
	    }
	}
	tm = tmHead;
    }
    else if (makeMaskXYName) {
	if (n -> mask == -1)
	    strcpy (maskbuf, "@sub");
	else if (n -> mask >= 0 && n -> mask < nrOfMasks)
	    strcpy (maskbuf, masktable[n -> mask].name);
	else
	    sprintf (maskbuf, "#%d", n -> mask);
	sprintf (name_buf, "%s%s_%s_%s", maskXYNamePrefix, maskbuf, strCoor (n -> node_x), strCoor (n -> node_y));
	strcpy (cnet.net_name, truncDmName (name_buf));
	checkTermName = 1;
    }
    else if (tmGrp) {
	for (tm = n -> names; tm; tm = tm -> next)
	    if (tm == tmGrp) break;
	if (tm) { /* tmGrp found */
	    s = groupName;
	    if (tm -> termX < 0 && tm -> termY < 0) tmHead = tm;
	    /* Don't set tmHead if it has an array form, it must also
	       be output as an array in an equivalence (e.g. for sls). */
	} else {
	    sprintf (name_buf, "%s%s%d", groupName, grp_node_sep, ++group_out_node_cnt);
	    s = name_buf;
	    checkTermName = 1;
	}
	strcpy (cnet.net_name, truncDmName (s));
    }
    else {
	/* SdeG: We don't need to search for a label, because
		 if there is a label, then tmGrp is set!
		 We also don't need to search for a terminal,
		 because if there is one without instName,
		 then this terminal is on first position.
	*/
	if ((tm = n -> names) && !tm -> instName) {
	    s = tm -> termName;
	    if (tm -> termX < 0 && tm -> termY < 0) tmHead = tm;
	    else { /* SdeG: Make array name for terminal. */
		sprintf (name_buf, "%s%s", s,
		    makeIndex (tm -> termX >= 0 ? 1 : -1,
			tm -> termY >= 0 ? 1 : -1, tm -> termX, tm -> termY));
		s = name_buf;
	    }
	    strcpy (cnet.net_name, truncDmName (s));
	} else {
	    tm = NULL;
	    sprintf (cnet.net_name, "%d", ++node_cnt + maxTermNumber);
	    /* Since maxTermNumber is the maximum value for a number
	       that is already a terminal or a label, we do not have to
	       check if the name that is generated here already exists.
	    */
	}
    }

    sprintf (net_name, cnet.net_name);

    if (checkTermName) {
	while (findTerminal (cnet.net_name)) {
	    checkTermName = 0;
	    sprintf (name_buf, "_%s", cnet.net_name);
	    strcpy (cnet.net_name, truncDmName (name_buf));
	}
	if (!checkTermName) { /* found and changed */
	    say ("%s '%s', but %s,\n\t%s '%s' %s\n",
		"tried to create net name", net_name, "it already exists",
		"generated net name", cnet.net_name, "instead.");
	    if (makeMaskXYName && (++maskXYnote % 4) == 1) {
		say ("%s\n\t%s\n",
		    "note that you can specify a net name prefix",
		    "using the parameter 'pos_name_prefix'");
	    }
	    sprintf (net_name, cnet.net_name);
	}
    }

    cnet.inst_name[0] = '\0';
    cnet.net_attribute = buf;

    if (isSubstrNode || n -> substr) i = 0;
    else { i = outConduc + 1; incrConduc = 1; }

    if (tmHead && tmHead -> type != tTerminal)
	sprintf (buf, "cd=%d;lay=%d;type=label", i, n -> mask);
    else
	sprintf (buf, "cd=%d;lay=%d", i, n -> mask);

    if (optTorPos) {
	if (tm)
	    addCoorXY (buf, tm -> x, tm -> y);
	else
	    addCoorXY (buf, n -> node_x, n -> node_y);
#if 0
	if (n -> area == 1 && n -> term == 2 && n -> node_w) {
	    sprintf (buf + strlen (buf), ";w=%d", n -> node_w);
	}
#endif
    }

    if (tmHead) {
	net_neqv--;
#ifdef DMS_TERM_POS
	if (dmsTermpos)
	    fprintf (dmsTermpos -> dmfp, "$ %d %d %s %d %d %d %d\n",
		tm -> instX, tm -> instY, cnet.net_name,
		tm -> termX, tm -> termY, tm -> x / inScale, tm -> y / inScale);
#endif
    }
    cnet.net_neqv = net_neqv;
    dmPutDesignData (dmsNet, CIR_NET_ATOM);
    cnet.net_neqv = 0;
    if (dmsCon) fprintf (dmsCon -> dmfp, "%s ", cnet.net_name);

    new_net_neqv = 0;

    if (n -> names) {

	/* if (tmHead) ASSERT (!tmHead -> instName);
	   Normally tmHead is skipped, if it is the net name. */

	for (tm = n -> names; tm; tm = tm -> next) {
	    if (tm == tmHead) continue;
	    fill_cnet_term (tm);

#ifdef DMS_TERM_POS
	    if (dmsTermpos)
		fprintf (dmsTermpos -> dmfp, "%s %d %d %s %d %d %d %d\n",
		    tm -> instName ? tm -> instName : "$",
		    tm -> instX, tm -> instY,
		    cnet.net_name, tm -> termX, tm -> termY,
		    tm -> x / inScale, tm -> y / inScale);
#endif
	    if (dmsCon && !tm -> instName) {
		print_cnet (dmsCon -> dmfp);
	    }
	    dmPutDesignData (dmsNet, CIR_NET_ATOM);
	    new_net_neqv++;
	}
	cnet.net_dim = cnet.inst_dim = 0; /* reset */
    }

    cnet.net_attribute = NULL; /* reset */
    cnet.net_name[1] = '\0';

    if (!isSubstrNode) {
	if (n -> res_cnt > 0) {
	    cnet.net_name[0] = 'p';
	    for (i = 0; i < resSortTabSize; i++) {
		for (con = n -> con[i]; con; con = NEXT (con, n)) {
		    if (con -> val != 0.0) {
			sprintf (cnet.inst_name, "_R%d", ++res_cnt);
			dmPutDesignData (dmsNet, CIR_NET_ATOM);
			outResistor (n, con, res_cnt, i);
			new_net_neqv++;
		    }
		    elemDelRes (con);
		}
	    }
	}

	if (n -> cap_cnt > 0)
	for (i = 0; i < capSortTabSize; i++)
	for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
	    if (!cap -> flagEl) {
		sprintf (cnet.inst_name, "_C%d", ++cap_cnt);
		cnet.net_name[0] = (capPolarityTab[i] != 'n')? 'p' : 'n';
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
		if (capAreaPerimEnable[i] == 1)
		    outAreaPerimElement (n, cap, cap_cnt, i);
		else outCapacitor (n, cap, cap_cnt, i);
		new_net_neqv++;
	    }
	    elemDelCap (cap);
	}
    }

    for (ne = n -> netEq; ne; ne = next_ne) {
	next_ne = ne -> next;

	switch (ne -> instType) {
	    case CAP :
		sprintf (cnet.inst_name, "_C%d", ne -> number);
		break;
	    case RES :
		sprintf (cnet.inst_name, "_R%d", ne -> number);
		break;
	    case BJT :
		if (ne -> instName)
		    strcpy (cnet.inst_name, ne -> instName);
		else
		    sprintf (cnet.inst_name, "_BJT%d", ne -> number);
		break;
	    case TOR :
		if (ne -> instName)
		    strcpy (cnet.inst_name, ne -> instName);
		else
		    sprintf (cnet.inst_name, "_T%d", ne -> number);
		break;

	    default:
		ASSERT (ne -> instType == CAP);
	}

	cnet.net_name[0] = ne -> term;
	dmPutDesignData (dmsNet, CIR_NET_ATOM);
	new_net_neqv++;

	DISPOSE (ne, sizeof(netEquiv_t));
	currNeqv--;
    }

    if (!isSubstrNode) {

        for (i = 0; i < capSortTabSize; i++) {

	    if (gndCapFlag[i] == 0) {
		if (capPolarityTab[i] != 'n') {
		    cnet.net_name[0] = 'p';
		    if (!defGndCapPol) { defGndCapPol = "n"; defGndCapOtherPol = "p"; }
		} else {
		    cnet.net_name[0] = 'n';
		    if (!defGndCapPol) { defGndCapPol = "p"; defGndCapOtherPol = "n"; }
                }
		sprintf (cnet.inst_name, "_CG%d", ++gndCap_cnt);
		if (defGndCapOtherPol[0] != cnet.net_name[0]) setGndCapPol ();
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
		if (capAreaPerimEnable[i] == 1)
		    outAreaPerimElement (n, NULL, gndCap_cnt, i);
		else
		    outCapacitor (n, NULL, gndCap_cnt, i);
		new_net_neqv++;
	    }

	    if (substrCapFlag[i] == 0) {
		if (capPolarityTab[i] != 'n') {
		    cnet.net_name[0] = 'p';
		    if (!defSubCapPol) { defSubCapPol = "n"; defSubCapOtherPol = "p"; }
		} else {
		    cnet.net_name[0] = 'n';
		    if (!defSubCapPol) { defSubCapPol = "p"; defSubCapOtherPol = "n"; }
                }
		sprintf (cnet.inst_name, "_CS%d", ++substrCap_cnt);
		if (defSubCapOtherPol[0] != cnet.net_name[0]) setSubCapPol ();
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
		if (capAreaPerimEnable[i] == 1)
		    outAreaPerimElement (n, NULL, -substrCap_cnt, i);
		else
		    outCapacitor (n, NULL, -substrCap_cnt, i);
		new_net_neqv++;
	    }
	}

	if (optRes) {
	    cnet.net_name[0] = 'p';
	    for (i = 0; i < resSortTabSize; i++)
	    if (n -> substrCon[i] != 0.0) {
		sprintf (cnet.inst_name, "_RS%d", ++substrRes_cnt);
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
		outResistor (n, NULL, substrRes_cnt, i);
		new_net_neqv++;
	    }
	}
    }

    if (isSubstrNode) {
	int cnt;

	cnet.net_name[0] = 'n';
	for (cnt = 0; cnt < substrRes_cnt;) {
	    sprintf (cnet.inst_name, "_RS%d", ++cnt);
	    dmPutDesignData (dmsNet, CIR_NET_ATOM);
	}
	new_net_neqv += substrRes_cnt;

	for (i = cnt = 0; cnt < substrCap_cnt;) {
	    sprintf (cnet.inst_name, "_CS%d", ++cnt);
	    if (i < SCP_cnt && SCP[i] == cnt) {
		cnet.net_name[0] = *defSubCapOtherPol; ++i; }
	    else
		cnet.net_name[0] = *defSubCapPol;
	    dmPutDesignData (dmsNet, CIR_NET_ATOM);
	}
	new_net_neqv += substrCap_cnt;

	if (substrToGnd_cnt) {
	    for (cnt = i = 0; i < capSortTabSize; i++) {
		if (gndCapFlag[i] == 0) {
		    sprintf (cnet.inst_name, "_CSG%d", ++cnt);
		    cnet.net_name[0] = (capPolarityTab[i] != 'n')? 'p' : 'n';
		    dmPutDesignData (dmsNet, CIR_NET_ATOM);
		    if (capAreaPerimEnable[i] == 1)
			outAreaPerimElement (NULL, NULL, cnt, i);
		    else
			outCapacitor (NULL, NULL, cnt, i);
		}
	    }
	    ASSERT (cnt == substrToGnd_cnt);
	    new_net_neqv += cnt;
	}
	if (nameGND == nameSUBSTR && gndCap_cnt) {
	    for (i = cnt = 0; cnt < gndCap_cnt;) {
		sprintf (cnet.inst_name, "_CG%d", ++cnt);
		if (i < GCP_cnt && GCP[i] == cnt) {
		    cnet.net_name[0] = *defGndCapOtherPol; ++i; }
		else
		    cnet.net_name[0] = *defGndCapPol;
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
	    }
	    new_net_neqv += gndCap_cnt;
	    gndCap_cnt = 0;
	}
    }

    ASSERT (new_net_neqv == net_neqv);

    outNod++;
ret:
    disposeNode (n);
}

Private void fill_cnet_term (terminal_t *term)
{
    static char buf[60];

    strcpy (cnet.net_name, truncDmName (term -> termName));
    if (term -> termX < 0 && term -> termY < 0) {
	cnet.net_dim = 0;
    }
    else if (term -> termX >= 0 && term -> termY >= 0) {
	cnet.net_dim = 2;
	cnet.net_lower[0] = term -> termX;
	cnet.net_lower[1] = term -> termY;
    }
    else if (term -> termX >= 0) {
	cnet.net_dim = 1;
	cnet.net_lower[0] = term -> termX;
    }
    else {
	cnet.net_dim = 1;
	cnet.net_lower[0] = term -> termY;
    }

    if (term -> instName) {
	strcpy (cnet.inst_name, term -> instName);
	if (term -> instX < 0 && term -> instY < 0) {
	    cnet.inst_dim = 0;
	}
	else if (term -> instX >= 0 && term -> instY >= 0) {
	    cnet.inst_dim = 2;
	    cnet.inst_lower[0] = term -> instX;
	    cnet.inst_lower[1] = term -> instY;
	}
	else if (term -> instX >= 0) {
	    cnet.inst_dim = 1;
	    cnet.inst_lower[0] = term -> instX;
	}
	else {
	    cnet.inst_dim = 1;
	    cnet.inst_lower[0] = term -> instY;
	}
    }
    else {
	cnet.inst_name[0] = '\0';
	cnet.inst_dim = 0;
    }

    if (term -> type != tTerminal) {
	sprintf (buf, "type=label");
	cnet.net_attribute = buf;
    }
    else
	cnet.net_attribute = 0;

    if (optTorPos) {
	if (!cnet.net_attribute) {
	    cnet.net_attribute = buf;
	    *buf = '\0';
	}
	addCoorXY (buf, term -> x, term -> y);
    }
}

Private void print_cnet (FILE *fp)
{
    if (cnet.net_dim == 0)
	fprintf (fp, "%s ", cnet.net_name);
    else if (cnet.net_dim == 1)
	fprintf (fp, "%s[%ld] ", cnet.net_name, cnet.net_lower[0]);
    else /* cnet.net_dim == 2 */
	fprintf (fp, "%s[%ld,%ld] ", cnet.net_name, cnet.net_lower[0], cnet.net_lower[1]);
}

void outGndNode ()
{
    int cnt, i;
    char buf[32];

    if (!extrPass) return;
    if ((cnet.net_neqv = gndCap_cnt + substrToGnd_cnt) == 0) return;

    cnet.net_attribute = buf;
    sprintf (buf, "cd=%d;lay=-1", outConduc + 1);
    cnet.inst_name[0] = '\0';
    strcpy (cnet.net_name, nameGND);
    dmPutDesignData (dmsNet, CIR_NET_ATOM);

    cnet.net_attribute = NULL;
    cnet.net_name[1] = '\0';
    cnet.net_neqv = 0;

    cnt = i = 0;
    while (cnt < gndCap_cnt) {
	sprintf (cnet.inst_name, "_CG%d", ++cnt);
	if (i < GCP_cnt && GCP[i] == cnt) {
	    cnet.net_name[0] = *defGndCapOtherPol; ++i; }
	else
	    cnet.net_name[0] = *defGndCapPol;
	dmPutDesignData (dmsNet, CIR_NET_ATOM);
    }

    if (substrToGnd_cnt) {
	for (cnt = i = 0; i < capSortTabSize; i++) {
	    if (gndCapFlag[i] == 0) {
		cnet.net_name[0] = (capPolarityTab[i] != 'n')? 'n' : 'p';
		sprintf (cnet.inst_name, "_CSG%d", ++cnt);
		dmPutDesignData (dmsNet, CIR_NET_ATOM);
	    }
	}
	ASSERT (cnt == substrToGnd_cnt);
    }

    outNod++;
    outConduc++;
}

Private void outResistor (node_t *n, element_r *el, int number, int sort)
{
    char buf[80];
    double val;

    if (el) {
	node_t *on = OTHER (el, n);
	val = el -> val;
	addNetEq (on, RES, 'n', number, NULL);
	sprintf (cmc.inst_name, "_R%d", number);
#ifdef DISPLAY
	if (goptOutResistor) drawOutResistor (n -> node_x, n -> node_y, on -> node_x, on -> node_y);
#endif
    }
    else {
	val = n -> substrCon[sort];
	sprintf (cmc.inst_name, "_RS%d", number);
    }

    strcpy (cmc.cell_name, resSortTab[sort]);

#ifdef MOMENTS
    if (printMoments) sprintf (buf, "v=%e;m0=%.3g", 1 / val, 1 / val);
    else
#endif
    if (compact_attr_vals)
	sprintf (buf, "v=%s", Val(1 / val, 6));
    else
	sprintf (buf, "v=%e", 1 / val);

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);
    outRes++;
}

Private void outCapacitor (node_t *n, element_c *el, int number, int sort)
/* when the number is negative, it is a substrate capacitance */
{
    char buf[600];
    double *m, val;

    if (el) {
	node_t *on;
#ifdef MOMENTS
	m = el -> moments;
#endif
	val = el -> val;
	on = OTHER (el, n);
	if (capPolarityTab[sort] != 'n')
	    addNetEq (on, CAP, 'n', number, NULL);
	else
	    addNetEq (on, CAP, 'p', number, NULL);
	sprintf (cmc.inst_name, "_C%d", number);

#ifdef DISPLAY
	if (goptOutCapacitor) drawOutCapacitor (n -> node_x, n -> node_y, on -> node_x, on -> node_y);
#endif
    }
    else if (n) {
	if (number > 0) {
#ifdef MOMENTS
	    m = n -> moments;
#endif
	    val = n -> gndCap[sort];
	    sprintf (cmc.inst_name, "_CG%d", number);
	}
	else {
#ifdef MOMENTS
	    m = n -> moments2;
#endif
	    val = n -> substrCap[sort];
	    sprintf (cmc.inst_name, "_CS%d", -number);
	}
    }
    else {
#ifdef MOMENTS
	m = nSUB -> moments;
#endif
	val = nSUB -> gndCap[sort];
	sprintf (cmc.inst_name, "_CSG%d", number);
    }

#ifdef MOMENTS
    if (doOutL && val < 0)
	strcpy (cmc.cell_name, "ind");
    else
#endif
	strcpy (cmc.cell_name, capSortTab[sort]);

    if (capAreaPerimEnable[sort] == 2) val /= capOutFac[sort];
        /* Transform value back to area or perimeter
           (for junction capacitance information). */

#ifdef MOMENTS
    if (maxMoments > 0) {
        if (val >= 0 || doOutNegC) {
            if (printMoments)
                sprintf (buf, "v=%e;m1=%.3g", val, val);
            else
                sprintf (buf, "v=%e", val);
        }
        else {
	    if (doOutL) sprintf (buf, "v=%e", -val);
	    else if (printMoments)
		sprintf (buf, "v=%e;m1=%.3g", 0.0, val);
	    else
                sprintf (buf, "v=%e", val);
	    val = 0;
        }

	if (printMoments && m) {
	    int j;
	    for (j = 0; j < extraMoments; j++) {
		sprintf (buf + strlen (buf), ";m%d=%.3g", j+2, m[j]);
	    }
	}
    }
    else
#endif
    if (compact_attr_vals)
	sprintf (buf, "v=%s", Val(val, 6));
    else
	sprintf (buf, "v=%e", val);

    if (capAreaPerimEnable[sort] == 2)
        totOutCap += val * capOutFac[sort];
    else
        totOutCap += val;

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);
    outCap++;
}

Private int markAreaPerimElement (node_t *n, element_c *el, int kind, int sort)
/* kind: 2 = coup_cap; 1 = gnd_cap; -1 = substr_cap; -2 = gnd_substr_cap */
{
    /* Mark which area-perimeter elements belong to 'el' and hence should
       not be outputted separately.
    */
    int i, doOut = 0;

    for (i = sort; i < capSortTabSize && capSortTab[i] == capSortTab[sort]; i += 2) {
        if (el) {
	    element_c *cap;
         // ASSERT (kind == 2);
            if (i > sort && (cap = findCapElement (n, OTHER(el, n), i))) cap -> flagEl = 1;
            doOut = 1;
        }
        else {
	    if (i > sort) {
		if (kind != -1) gndCapFlag[i] = 1;
		else substrCapFlag[i] = 1;
	    }

	    if (min_ground_area <= 0 || kind == -2) doOut = 1;
	    else if (capAreaPerimType[i] == 'a') {
		if (kind > 0 && Abs (n -> gndCap[i])    >= min_ground_area) doOut = 1;
		if (kind < 0 && Abs (n -> substrCap[i]) >= min_ground_area) doOut = 1;
	    }
        }
    }

    if (!doOut) {
        /* This element will not be outputted.  So also the first sort
           is marked to be not part of the output. */
	if (kind != -1) gndCapFlag[sort] = 1;
	else substrCapFlag[sort] = 1;
    }

    return (doOut);
}

Private void outAreaPerimElement (node_t *n, element_c *el, int number, int sort)
/* when the number is negative, it is a substrate capacitance */
{
    int i, k_a, k_p;
    char buf[600];
    char sval[20];
    double val;
    element_c *cap;
    node_t *on = NULL;

    if (el) on = OTHER (el, n);
    ASSERT (extrPass);

    buf[0] = '\0';
    k_a = k_p = 1;

    for (i = sort; i < capSortTabSize && capSortTab[i] == capSortTab[sort]; i += 2) {

        if (el) {
            cap = (i == sort)? el : findCapElement (n, on, i);
	    val = cap ? cap -> val : 0;
	}
	else if (n) {
	    if (number >= 0)
		val = n -> gndCap[i];
	    else
		val = n -> substrCap[i];
	}
	else
	    val = nSUB -> gndCap[i];

	if (compact_attr_vals)
	    sprintf (sval, "%s", Val(val, 6));
	else
	    sprintf (sval, "%e", val);

        if (jun_caps == C_SEPARATE) {
            if (capAreaPerimType[i] == 'a')
		sprintf (buf + strlen (buf), ";area%d=%s", k_a++, sval);
            else
		sprintf (buf + strlen (buf), ";perim%d=%s", k_p++, sval);
        }
        else {
	    if (capAreaPerimType[i] == 'a')
		sprintf (buf + strlen (buf), ";area=%s", sval);
	    else
		sprintf (buf + strlen (buf), ";perim=%s", sval);
	}
    }

    if (el) {
	if (capPolarityTab[sort] != 'n')
	    addNetEq (on, CAP, 'n', number, NULL);
	else
	    addNetEq (on, CAP, 'p', number, NULL);
	sprintf (cmc.inst_name, "_C%d", number);
    }
    else if (n) {
	if (number > 0)
	    sprintf (cmc.inst_name, "_CG%d", number);
	else
	    sprintf (cmc.inst_name, "_CS%d", -number);
    }
    else sprintf (cmc.inst_name, "_CSG%d", number);

    strcpy (cmc.cell_name, capSortTab[sort]);

    cmc.inst_attribute = *buf == ';' ? buf + 1 : buf;
    dmPutDesignData (dmsMc, CIR_MC);
    outCap++;
}

void outVBJT (BJT_t *vT)
{
    int i, inst_n;
    char buf[80];

    ASSERT (!prePass1);
    if (!extrPass) return;

    inst_n = outpVBJT + outpLBJT + 1;

    if (!vT -> n[SU] && vT -> type -> s.bjt.sCon == -4) {
	say ("warning: no substrate area found for bjt '%s' at position %s\n",
	    vT -> type -> name, strCoorBrackets (vT -> pn[EM] -> xl, vT -> pn[EM] -> yb));
    }

    addNetEq (vT -> n[BA], BJT, 'b', inst_n, vT -> instName);
    addNetEq (vT -> n[CO], BJT, 'c', inst_n, vT -> instName);
    addNetEq (vT -> n[EM], BJT, 'e', inst_n, vT -> instName);
    if (vT -> n[SU]) addNetEq (vT -> n[SU], BJT, 's', inst_n, vT -> instName);

    strcpy (cmc.cell_name, vT -> type -> name);
    if (vT -> instName)
	sprintf (cmc.inst_name, vT -> instName);
    else
	sprintf (cmc.inst_name, "_BJT%d", inst_n);

    if (compact_attr_vals) {
	sprintf (buf, "ae=%s;", Val(vT -> area, 2));
	sprintf (buf + strlen (buf), "pe=%s", Val(vT -> length, 2));
    }
    else
	sprintf (buf, "ae=%.2e;pe=%.2e", vT -> area, vT -> length);

    if (vT -> scalef > 1) sprintf (buf + strlen (buf), ";scalef=%d", vT -> scalef);

    if (optTorPos) addCoorXY (buf, vT -> pn[EM] -> xl, vT -> pn[EM] -> yb);

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);

    if (dmsDevgeo && vT -> tiles) {
	mask_t mcolor;
	tileRef_t *tR;
	FILE *fp = dmsDevgeo -> dmfp;
	fprintf (fp, "-%d ( %s )\n", ++act_dev_cnt, cmc.inst_name);
	for (tR = vT -> tiles; tR; tR = tR -> next) {
	    mcolor = cNull;
            for (i = 0; i < nrOfMasks; i++) {
		if (!COLOR_ABSENT (&tR -> color, &masktable[i].color))
		    COLOR_ADDINDEX (mcolor, i);
            }
	    dmPUT (fp, "DDDSDDDDDD\n", (long)tR -> tile, (long)tR -> cx,
		(long)(tR -> cx >= 0 ? conductorMask[tR -> cx] : -1),
		colorIntStr (&mcolor),
		(long)(tR -> xl), (long)(tR -> xr), (long)(tR -> bl),
		(long)(tR -> br), (long)(tR -> tl), (long)(tR -> tr));
        }
	dmPUT (fp, "DDDSDDDDDD\n", 0L, 0L, 0L, "0", 0L, 0L, 0L, 0L, 0L, 0L);
    }

    outpVBJT++;
}

void outLBJT (BJT_t *lT)
{
    int i, inst_n;
    char buf[80];
    polnode_t *pnEM;

    ASSERT (!prePass1);
    if (!extrPass) return;

    inst_n = outpVBJT + outpLBJT + 1;

    pnEM = lT -> pn[EM];

    if (!lT -> n[SU] && lT -> type -> s.bjt.sCon == -4) {
	say ("warning: no substrate area found for bjt '%s' at position %s\n",
	    lT -> type -> name, strCoorBrackets (pnEM -> xl, pnEM -> yb));
    }

    addNetEq (lT -> n[CO], BJT, 'c', inst_n, lT -> instName);
    addNetEq (lT -> n[BA], BJT, 'b', inst_n, lT -> instName);
    addNetEq (lT -> n[EM], BJT, 'e', inst_n, lT -> instName);
    if (lT -> n[SU]) addNetEq (lT -> n[SU], BJT, 's', inst_n, lT -> instName);

    strcpy (cmc.cell_name, lT -> type -> name);
    if (lT -> instName)
	sprintf (cmc.inst_name, lT -> instName);
    else
	sprintf (cmc.inst_name, "_BJT%d", inst_n);

    if (compact_attr_vals) {
	sprintf (buf, "ae=%s;", Val(pnEM -> area, 2));
	sprintf (buf + strlen (buf), "pe=%s;", Val(pnEM -> length, 2));
	sprintf (buf + strlen (buf), "wb=%s", Val(meters * lT -> basew, 2));
    }
    else
	sprintf (buf, "ae=%.2e;pe=%.2e;wb=%.2e", pnEM -> area, pnEM -> length, meters * lT -> basew);

    if (lT -> scalef > 1) sprintf (buf + strlen (buf), ";scalef=%d", lT -> scalef);

    if (optTorPos) addCoorXY (buf, pnEM -> xl, pnEM -> yb);

    cmc.inst_attribute = buf;
    dmPutDesignData (dmsMc, CIR_MC);

    if (dmsDevgeo && pnEM -> tiles) {
	mask_t mcolor;
	tileRef_t *tR;
	FILE *fp = dmsDevgeo -> dmfp;
	fprintf (fp, "-%d ( %s )\n", ++act_dev_cnt, cmc.inst_name);
	for (tR = pnEM -> tiles; tR; tR = tR -> next) {
	    mcolor = cNull;
            for (i = 0; i < nrOfMasks; i++) {
		if (!COLOR_ABSENT (&tR -> color, &masktable[i].color))
		    COLOR_ADDINDEX (mcolor, i);
            }
	    dmPUT (fp, "DDDSDDDDDD\n", (long)tR -> tile, (long)tR -> cx,
		(long)(tR -> cx >= 0 ? conductorMask[tR -> cx] : -1),
		colorIntStr (&mcolor),
		(long)(tR -> xl), (long)(tR -> xr), (long)(tR -> bl),
		(long)(tR -> br), (long)(tR -> tl), (long)(tR -> tr));
        }
	dmPUT (fp, "DDDSDDDDDD\n", 0L, 0L, 0L, "0", 0L, 0L, 0L, 0L, 0L, 0L);
    }

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
	if (t -> instName)
	    strcpy (cmc.inst_name, t -> instName);
	else
	    sprintf (cmc.inst_name, "_T%d", t -> tor_nr);
	strcpy (cmc.cell_name, t -> type -> name);

    if (compact_attr_vals) {
	sprintf (buf, "w=%s", Val(t -> dsPerimeter, 4));
	sprintf (buf + strlen(buf), ";l=%s", Val(t -> totPerimeter, 4));
    }
    else
	sprintf (buf, "w=%.4e;l=%.4e", t -> dsPerimeter, t -> totPerimeter);

	for (dsBdr = t -> boundaries; dsBdr; dsBdr = dsBdr -> next)
	if ((pn = dsBdr -> pn)) {
	    w = t -> dsPerimeter;
	    c = dsBdr -> type;
	    W = pn->ds_wtot;
	    p = pn->ds_peri * ((w * W + 2 * pn->ds_area) /
				(W * W + 2 * pn->ds_area * pn->ds_tcnt));
	    a = pn->ds_area * (w / W);
	    n = a / (w * w);
	    if (compact_attr_vals) {
		sprintf (buf + strlen(buf), ";a%c=%s", c, Val(a,4));
		sprintf (buf + strlen(buf), ";p%c=%s", c, Val(p,4));
		sprintf (buf + strlen(buf), ";nr%c=%s",c, Val(n,4));
	    }
	    else {
		sprintf (buf + strlen(buf), ";a%c=%.4e", c, a);
		sprintf (buf + strlen(buf), ";p%c=%.4e", c, p);
		sprintf (buf + strlen(buf), ";nr%c=%.4e",c, n);
	    }
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
	addNetEq (dsBdr -> dsCond -> node, TOR, dsBdr -> type, tor_cnt, t -> instName);
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
		DISPOSE (ne, sizeof(netEquiv_t));
		currNeqv--;
	    }
	    --tor_cnt;
	    goto ret;
	}
	if (optDsConJoin && term == 'x') { /* add second terminal */
	    n = t -> boundaries -> dsCond -> node;
	    addNetEq (n, TOR, 's', tor_cnt, t -> instName);
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
	if (!n -> term) n -> term = 1;
	addNetEq (n, TOR, 'g', tor_cnt, t -> instName);
    }
    else ASSERT (t -> gate);

    if (t -> bulk) {
	n = t -> bulk -> node;
	if (!n -> term) n -> term = 1;
	addNetEq (n, TOR, 'b', tor_cnt, t -> instName);
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

#ifdef PLOT_CIR_MODE
    if (optPlotCir) {
	sprintf (buf, "%d %s", tor_cnt, t -> type -> name);
	plotString (6, BELOW, buf, (int)(t -> xl), (int)(t -> yb));
    }
#endif

    if (dmsDevgeo) {
	mask_t mcolor;
	tileRef_t *tileref, *tnext;
	FILE *fp = dmsDevgeo -> dmfp;
	fprintf (fp, "-%d ( %s )\n", ++act_dev_cnt, cmc.inst_name);
	tnext = t -> tileInfo -> tiles;
	while ((tileref = tnext)) {
	    tnext = tileref -> next;

	    mcolor = cNull;
	    for (i = 0; i < nrOfMasks; i++) {
		if (!COLOR_ABSENT (&tileref -> color, &masktable[i].color))
		    COLOR_ADDINDEX (mcolor, i);
	    }

	    dmPUT (fp, "DDDSDDDDDD\n",
		(long)tileref -> tile, (long)tileref -> cx,
		(long)(tileref -> cx >= 0 ? conductorMask[tileref -> cx] : -1),
		colorIntStr (&mcolor),
		(long)(tileref -> xl), (long)(tileref -> xr),
		(long)(tileref -> bl), (long)(tileref -> br),
		(long)(tileref -> tl), (long)(tileref -> tr));

	    DISPOSE (tileref, sizeof(tileRef_t));
	}
	DISPOSE (t -> tileInfo, sizeof(transTileInfo_t));

	dmPUT (fp, "DDDSDDDDDD\n", 0L, 0L, 0L, "0", 0L, 0L, 0L, 0L, 0L, 0L);
    }

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

void backInfoInstance (char *instName, coor_t dx, int nx, coor_t dy, int ny,
	coor_t xl, coor_t yb, coor_t xr, coor_t yt)
{
    int x, y;
    FILE *fp = dmsDevgeo -> dmfp;

    for (x = 0; x <= nx; x++) {
	for (y = 0; y <= ny; y++) {
	    act_dev_cnt++;
	    if (nx == 0 && ny == 0)
		fprintf (fp, "-%d ( %s )\n", act_dev_cnt, instName);
	    else if (nx == 0)
		fprintf (fp, "-%d ( %s[%d] )\n", act_dev_cnt, instName, y);
	    else if (ny == 0)
		fprintf (fp, "-%d ( %s[%d] )\n", act_dev_cnt, instName, x);
	    else
		fprintf (fp, "-%d ( %s[%d,%d] )\n", act_dev_cnt, instName, x, y);

	    dmPUT (fp, "DDDSDDDDDD\n", 0L, -1L, -1L, "0",
		(long)(xl + x * dx), (long)(xr + x * dx),
		(long)(yb + y * dy), (long)(yb + y * dy),
		(long)(yt + y * dy), (long)(yt + y * dy));
	}
    }
}

void groupDel (group_t *grp)
{
    if (grp -> tileInfo) {
	if (grp -> tileInfo -> tiles) {
	    tileRef_t *tileref, *tiles;
	    tiles = grp -> tileInfo -> tiles;
	    while ((tileref = tiles)) {
		tiles = tileref -> next;
		DISPOSE (tileref, sizeof(tileRef_t));
	    }
	}
	DISPOSE (grp -> tileInfo, sizeof(groupTileInfo_t));
    }
    DISPOSE (grp, sizeof(group_t));
    currIntGrp--;
}

void nodeDel (node_t *n)
{
    group_t *grp = Grp (n);

    grp -> nod_cnt--;

    if (n -> gnext) n -> gnext -> gprev = n -> gprev;
    if (grp -> nodes == n) grp -> nodes = n -> gnext;
    else {
	n -> gprev -> gnext = n -> gnext;
	if (!n -> gnext) grp -> nodes -> gprev = n -> gprev;
    }

    if (n -> pols) {
	while (n -> pols) nodeLinkDel (n -> pols);
	ASSERT (grp -> notReady > 0);
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

Private void addNetEq (node_t *n, int instType, int term, int number, char *instName)
{
    netEquiv_t *ne = NEW (netEquiv_t, 1);

    currNeqv++;

    ne -> instType = instType;
    ne -> term = term;
    ne -> number = number;
    ne -> instName = instName;
    ne -> next = n -> netEq;
    n -> netEq = ne;
    n -> n_n_cnt++;
}

Private void outTileConnectivity (group_t *grp)
{
    int i;
    tileRef_t *tileref, *tiles;
    mask_t mcolor;
    FILE *fp = dmsCon -> dmfp;

    fprintf (fp, ")\n");
    tiles = grp -> tileInfo -> tiles;

    while ((tileref = tiles)) {
	tiles = tileref -> next;
	mcolor = cNull;
	for (i = 0; i < nrOfMasks; i++) {
	    if (!COLOR_ABSENT (&tileref -> color, &masktable[i].color))
		COLOR_ADDINDEX (mcolor, i);
	}
	dmPUT (fp, "DDDSDDDDDD\n", (long)tileref -> tile, (long)tileref -> cx,
	    (long)conductorMask[tileref -> cx], colorIntStr (&mcolor),
	    (long)(tileref -> xl), (long)(tileref -> xr),
	    (long)(tileref -> bl), (long)(tileref -> br),
	    (long)(tileref -> tl), (long)(tileref -> tr));
	DISPOSE (tileref, sizeof(tileRef_t));
    }
    DISPOSE (grp -> tileInfo, sizeof(groupTileInfo_t));
    grp -> tileInfo = 0;
    dmPUT (fp, "DDDSDDDDDD\n", 0L, 0L, 0L, "0", 0L, 0L, 0L, 0L, 0L, 0L);
}

Private void setGndCapPol ()
{
    static int GCP_size = 0;

    if (GCP_cnt >= GCP_size) {
	int old_size = GCP_size;
        GCP_size = (GCP_size == 0 ? 50 : ((int) (GCP_size * 1.5)));
        GCP = RESIZE (GCP, int, GCP_size, old_size);
    }
    GCP[GCP_cnt++] = gndCap_cnt;
}

Private void setSubCapPol ()
{
    static int SCP_size = 0;

    if (SCP_cnt >= SCP_size) {
	int old_size = SCP_size;
        SCP_size = (SCP_size == 0 ? 50 : ((int) (SCP_size * 1.5)));
        SCP = RESIZE (SCP, int, SCP_size, old_size);
    }
    SCP[SCP_cnt++] = substrCap_cnt;
}

#ifdef DEBUG_NODES

Private void debug_nodes_init ()
{
    if (fp_dbn) fclose (fp_dbn);
    if (!(fp_dbn = fopen ("debnod", "w"))) {
	fprintf (stderr, "Cannot open debnod\n");
	die ();
    }
    check_cnt = leftBehind = hasPrinted = 0;
}

void debug_nodes_check (int i)
{
#define DEBUG_NODES_NOCHECK /* no check, print everything */
#ifndef DEBUG_NODES_NOCHECK
    int j;
    group_t *g, *og;
    node_t *n, *m;
    register element_c *cap;
    register element_r *con;

    check_cnt++;

    for (m = nqFirst (); m; m = nqNext ()) m -> flag = 0;

    for (m = nqFirst (); m; m = nqNext ()) {
	if (!m -> flag && (g = Grp (m)) -> notReady == 0) {

	    for (n = g -> nodes; n; n = n -> gnext) n -> flag = 1;

	    if (currIntCap > 0)
	    for (n = g -> nodes; n; n = n -> gnext) {
		for (j = 0; j < capSortTabSize; j++)
		for (cap = n -> cap[j]; cap; cap = NEXT (cap, n)) {
		    og = Grp (OTHER (cap, n));
		    if (og != g && og -> notReady > 0) goto skip;
		}
	    }
	    if (currIntRes > 0)
	    for (n = g -> nodes; n; n = n -> gnext) {
		for (j = 0; j < resSortTabSize; j++)
		for (con = n -> con[j]; con; con = NEXT (con, n)) {
		    if (con -> type == 'S') {
			og = Grp (OTHER (con, n));
			if (og != g && og -> notReady > 0) goto skip;
		    }
		}
	    }
	    leftBehind = 1;
	}
skip:	;
    }

    if (hasPrinted) {
	for (m = nqFirst (); m; m = nqNext ()) m -> flag = 0;
	return;
    }

	/* print only before and after an error has occured ! */

    if (!leftBehind)
	rewind (fp_dbn); /* this might be before an error occurs */
    else
	hasPrinted = 1;
#else
    check_cnt++;
#endif /* DEBUG_NODES_NOCHECK */

    debug_nodes_print (1);
}

void debug_nodes_print (int i)
{
    group_t *g;
    node_t *n;

    if (i == 1)
	fprintf (fp_dbn, "\n==============debug_nodes_check============");
    else
	fprintf (fp_dbn, "\n==============endLump======================");

    fprintf (fp_dbn, "\ncheck_cnt: %2d outNodes: %d\n", check_cnt, outNod);

    if (leftBehind)
	fprintf (fp_dbn, "\n!!!!!!  Nodes are left alone !!!!!!\n");

    for (n = nqFirst (); n; n = nqNext ()) n -> flag = 0;

    for (n = nqFirst (); n; n = nqNext ()) {
	if (n -> flag) continue; /* node printed */

	g = Grp (n);
	fprintf (fp_dbn, "\nGrp: %p notReady: %d%s\n", g, g -> notReady,
	    g -> notReady == 0 ? "  This group is left alone !!!" : "");
	fprintf (fp_dbn, "    nod_cnt=%d\n", g -> nod_cnt);

	for (n = g -> nodes; n; n = n -> gnext) {
	    n -> flag = 1;
	    printNode (fp_dbn, n);
	}
	if (i)
	    fprintf (fp_dbn, "-------------------------------------------\n");
    }

    for (n = nqFirst (); n; n = nqNext ()) n -> flag = 0;
}

void printNode (FILE *fp, node_t *n)
{
    int i;
    register element_c *cap;
    register element_r *con;
    group_t *g, *og;
    node_t *on;

    g = Grp (n);
    fprintf (fp, "-------------------------------------------\n");
    fprintf (fp, "Node: %p at %d,%d subs: %d grp: %p notReady: %d\n",
	n, n -> node_x, n -> node_y, n -> subs? 1 : 0, g, g -> notReady);

    fprintf (fp, "    term=%d substr=%d area=%d flag=%d keep=%d delayed=%d\n",
	(int)n->term, (int)n->substr, (int)n->area, (int)n->flag, (int)n->keep, (int)n->delayed);

    fprintf (fp, "res_cnt: %d\n", n -> res_cnt);
    if (n -> res_cnt > 0)
    for (i = 0; i < resSortTabSize; i++) {
	for (con = n -> con[i]; con; con = NEXT (con, n)) {
	    on = OTHER (con, n);
	    fprintf (fp, "\tcon %e %p %p %c\n", con -> val, n, on, con -> type);
	}
    }

    fprintf (fp, "cap_cnt: %d\n", n -> cap_cnt);
    if (n -> cap_cnt > 0) {
	for (i = 0; i < capSortTabSize; i++) {
	    for (cap = n -> cap[i]; cap; cap = NEXT (cap, n)) {
		on = OTHER (cap, n);
		og = Grp (on);
		fprintf (fp, "\tcap %e %p %p ogrp: %p", cap -> val, n, on, og);
		if (og != g) fprintf (fp, " otherNotReady: %d", og -> notReady);
		fprintf (fp, "\n");
	    }
	}
    }
}

#endif /* DEBUG_NODES */

#ifdef MOMENTS
Private node_t * createGroupNode (node_t *node)
{
    node_t *nnew = createNode ();
    nnew -> grp    = node -> grp;
    nnew -> mask   = node -> mask;
    nnew -> node_x = node -> node_x;
    nnew -> node_y = node -> node_y;

    /* make nnew the last node of new_nodes */
    if (!new_nodes) new_nodes = nnew;
    else new_nodes -> gprev -> gnext = nnew;
    new_nodes -> gprev = nnew;
    nnew -> gnext = NULL;

    return (nnew);
}

Private void outputSeriesRC (node_t *node)
{
    int cx, mflag;
    node_t *nnew;
    register element_c *cap;
    double min_ground, v, *m;

    mflag = node -> moments && node -> moments[0] < 0;

    for (cx = 0; cx < capSortTabSize; cx++) {
	if (mflag) {
	    min_ground = capAreaPerimEnable[cx] ? min_ground_area : min_ground_cap;

	    if ((v = node -> gndCap[cx]) > 0 && v >= min_ground) {
		mflag = 0;
		m = node -> moments;
		nnew = createGroupNode (node);
		elemAddRes ('G', node, nnew, v * v / -m[0], 0);
		nnew -> gndCap[cx] = v;
		node -> gndCap[cx] = 0;
		nnew -> moments = m;
		node -> moments = 0;
	    }
	}

	for (cap = node -> cap[cx]; cap; cap = NEXT (cap, node)) {
	    if ((m = cap -> moments) && (v = cap -> val) > 0 && m[0] < 0) {
		nnew = createGroupNode (node);
		elemAddRes ('G', node, nnew, v * v / -m[0], 0);
		elemAddCap (nnew, OTHER (cap, node), v, cx, m);
		elemDelCap (cap);
	    }
	}
    }
}

Private void outputSeriesL (node_t *node)
{
    int cx, rx;
    node_t *nnew, *on;
    register element_c *cap;
    element_r *res;
    double v;

    if (optCoupCap && node -> cap_cnt > 0) {
        for (cx = 0; cx < capSortTabSize; cx++) {
            for (cap = node -> cap[cx]; cap; cap = NEXT (cap, node)) {
	        if (cap -> val < 0) {
		    res = NULL; /* init, else compiler warning */
	            v = 0;
		    on = OTHER (cap, node);
                    for (rx = 0; rx < resSortTabSize; rx++) {
  	                for (res = node -> con[rx]; res; res = NEXT (res, node)) {
			    if (OTHER (res, node) == on) {
				v = res -> val;
				goto found_res;
			    }
                        }
                    }
found_res:
		    if (v != 0) {
			nnew = createGroupNode (node);
			elemAddRes ('G', node, nnew, v, rx);
			elemAddCap (nnew, on, cap -> val/(v * v), cx, NULL);
			elemDelCap (cap);
			elemDelRes (res);
		    }
		    else cap -> val = 0;
		}
	    }
	}
    }
}
#endif /* MOMENTS */
