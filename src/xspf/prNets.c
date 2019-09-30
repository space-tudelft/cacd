
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

extern char *nameGND;
extern char spef_dnet[];
extern int searchInst;
extern int useInst;
extern int usePort;
extern int omitres;
extern int coordinates;
extern int driver;
int spef_print = 0;
int spef_doRES = 0;

struct vnet {
    int  net_cnt;
    char *a_name;
    char *b_name;
    struct vnet *next;
};

static struct vnet *vn_free = NULL;
static struct vnet *vn_beg, *vn_end;

#define NEW_VNET(ptr) { \
    if (vn_free) { \
	ptr = vn_free; \
	vn_free = ptr -> next; \
	ptr -> next = NULL; \
    } \
    else { \
	PALLOC (ptr, 1, struct vnet); \
    } \
    if (!vn_beg) vn_beg = ptr; \
    else vn_end -> next = ptr; \
    vn_end = ptr; \
}

#define FREE_VNET() { \
    vn_end -> next = vn_free; \
    vn_free = vn_beg; \
}

extern struct node_info *nodetab;
extern char *delimiter;
extern int  *Nil;
extern long c_cnt;
extern int out_indent;
extern int tog_nobrack;
extern int tog_use0;
extern int tog_use0_save;
extern int groundVnet;
extern int use_spef;
extern char *snbulk, *spbulk;

int  cd_cnt;
int  rctype;

#ifdef __cplusplus
  extern "C" {
#endif
static char *nameToStr (int nr, char *name, int dim, int *lower);
static char *nameToSpf (char *name, int dim, int *lower);
static int  unequalNet (struct net_el *n1, struct net_el *n2);
static int  unequalTermNet (struct cirterm *t, struct net_el *n, int tb);
static void do_vnets (void);
#ifdef __cplusplus
  }
#endif

static char *end_str;

static void do_vnets ()
{
    char buf[300];
    struct vnet *v, *w;

    if (use_spef) {
      if (spef_doRES) {
	v = vn_beg;
	do {
	    sprintf (buf, "%ld ", ++c_cnt);
	    oprint (0, buf);
	    if (isdigit ((int)*(v -> a_name))) { oprint (1, spef_dnet); oprint (1, ":"); }
	    oprint (1, v -> a_name);
	    oprint (1, " ");
	    if (isdigit ((int)*(v -> b_name))) { oprint (1, spef_dnet); oprint (1, ":"); }
	    oprint (1, v -> b_name);
	    oprint (1, " 0\n");
	} while ((v = v -> next));
      }
    }
    else {
	if (groundVnet) {
	    v = vn_beg;
	    do {
		if (strcmp (v -> a_name, "0") && strcmp (v -> b_name, "0")) {
		    sprintf (buf, "rnet%da ", v -> net_cnt);
		    oprint (0, buf);
		    oprint (1, v -> a_name);
		    oprint (1, " 0 100g\n");
		    w = vn_beg;
		    while (w != v) {
			if (strcmp (w -> b_name, v -> b_name) == 0) break;
			w = w -> next;
		    }
		    if (w == v) {
			sprintf (buf, "rnet%db ", v -> net_cnt);
			oprint (0, buf);
			oprint (1, v -> b_name);
			oprint (1, " 0 100g\n");
		    }
		}
	    } while ((v = v -> next));
	}
	v = vn_beg;
	do {
	    sprintf (buf, "vnet%d %s", v -> net_cnt, v -> a_name);
	    oprint (0, buf);
	    sprintf (buf, " %s 0\n", v -> b_name);
	    oprint (0, buf);
	} while ((v = v -> next));
    }
    FREE_VNET ();
    vn_beg = NULL;
}

static int other_name (struct model_info *ntw, struct net_el *n)
{
    char buf[DM_MAXNAME + DM_MAXNAME + 52], *s;
    struct net_el *fnet;
    struct term_ref *tref;
    int termNet;

    if ((termNet = nodetab[n->nx].isTerm)) { /* terminal net */
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    if (!unequalTermNet (tref -> t, n, 1)) return 1; /* n is terminal */
	}
    }
    if (strcmp (n -> net_name, nameGND) == 0) return 1; /* n is ground */
    if (strcmp (n -> net_name, "SUBSTR") == 0) return 1; /* n is substrate */

    if (!usePort) termNet = 0;
    if (termNet || useInst) { /* try the subnets */
	fnet = n;
	for (n = fnet -> net_eqv; n; n = n -> net_eqv) { /* for all subnets */
	    if (!n->inst_name) {
		if (termNet)
		for (tref = ntw -> terms; tref; tref = tref -> next) {
		    if (!unequalTermNet (tref -> t, n, 1)) {
			if (n -> net_dim > 0) {
			    s = nameToStr (0, n -> net_name, n -> net_dim, n -> net_lower);
			    nodetab[fnet->nx].spef_name = newStringSpace (s);
			}
			else nodetab[fnet->nx].spef_name = n -> net_name;
			return 1; /* n is terminal */
		    }
		}
	    }
	    else if (!termNet) { /* use instance */
		s = n -> inst_name;
		if (*s != '_' || (s[1] != 'R' && s[1] != 'C')) {
		    s = nameToSpf (s, 0, Nil);
		    strcpy (buf, s);
		    s = buf + (end_str - s);
		    *s = *delimiter;
		    strcpy (s+1, nameToSpf (n -> net_name, 0, Nil));
		    nodetab[fnet->nx].spef_name = newStringSpace (buf);
		    return 1;
		}
	    }
	}
    }
    return 0;
}

static char *other_name2 (struct model_info *ntw, struct net_ref *nref)
{
    char buf[DM_MAXNAME + DM_MAXNAME + 52], *s, *ins, *pin;
    struct net_ref *ref;
    struct net_el *fnet, *n;
    int rv;

    ins = pin = NULL;
    for (ref = nref; ref && ref -> cd == nref -> cd; ref = ref -> next) {
	fnet = ref -> n;
	for (n = fnet -> net_eqv; n; n = n -> net_eqv) { /* for all subnets */
	    s = n -> inst_name;
	    if (!s) fprintf (stderr, "other_name2: unexpected name w/o instance!\n");
	    else if (*s != '_' || (s[1] != 'R' && s[1] != 'C')) { /* only not RC instances */
		if (!ins || (rv = strcmp (s, ins)) < 0
			 || (rv == 0 && strcmp (n -> net_name, pin) < 0)) {
		    ins = s; pin = n -> net_name;
		}
	    }
	}
    }
    if (ins) {
	sprintf (buf, "%s_%s", ins, pin);
	return newStringSpace (buf);
    }
    fprintf (stderr, "other_name2: no instance found!\n");
    return NULL;
}

void prNets (struct model_info *ntw, struct net_ref *nets)
{
    struct vnet *v;
    struct cirterm *t;
    struct term_ref *tref;
    struct net_ref *ref, *nref, *spfref;
    struct net_el *fnet, *gnet, *anet, *n, *f;
    char buf[DM_MAXNAME + DM_MAXNAME + 52];
    char *iname, *nname, *sname, *s;
    char *rightName, *leftName;
    long leftNameNbr;
    int  i, j, k, net_cnt, xvector[10];
    int  dim, not, R_cnt, C_cnt, Conn;
    int  term_ground;

    vn_beg = NULL;
    R_cnt = C_cnt = Conn = 0;
    net_cnt = 0;

    if (tog_use0 && !use_spef) {
	for (tref = ntw -> terms; tref; tref = tref -> next) {
	    t = tref -> t;
	    dim = t -> term_dim;
	    for (i = 0; i < dim; ++i) xvector[i] = t -> term_lower[i];
	    do {
		nname = nameToStr (0, t -> term_name, dim, xvector);
		if (test0 (nname) == 0) { /* terminal connected to GND */
		    NEW_VNET (v);
		    v -> net_cnt = ++net_cnt;
		    v -> a_name = newStringSpace (nameToSpf (t -> term_name, dim, xvector));
		    v -> b_name = "0"; /* ground */
		}
		for (i = dim; --i >= 0;) {
		    if (t -> term_lower[i] <= t -> term_upper[i]) {
			if (++xvector[i] <= t -> term_upper[i]) break;
		    }
		    else {
			if (--xvector[i] >= t -> term_upper[i]) break;
		    }
		    xvector[i] = t -> term_lower[i];
		}
	    } while (i >= 0);
	}
    }

    cd_cnt = -1;

    if (use_spef) { /* choice D_NET names */
	int not_ok = 0;
	i = j = 0; n = NULL; ref = NULL;
	sname = NULL;
	for (nref = nets; nref;) {
	    fnet = nref -> n;
	    if (!fnet->nx) { nref = nref -> next; continue; } /* array decl. */
	    nodetab[fnet->nx].spef_name = nname = fnet -> net_name;

	    if (nref -> cd != cd_cnt) { // new conductor
		ref = nref;
		cd_cnt = nref -> cd;
		not_ok = j = 0; n = NULL;
	    }

	    if ((s = strchr (nname, ':'))) { /* (SdeG) try to find another net_name */
		if (!not_ok) {
		    k = s - nname;
		    if (n || j) { /* previous s or !s */
			if (k != i || strncmp (nname, sname, i)) not_ok = 1;
		    }
		    j = 1;
		    if (!not_ok) {
			i = k;
			k = 0; ++s;
			while (isdigit ((int)*s)) { k = 10*k + (*s - '0'); ++s; }
			if (!k || *s) not_ok = 1;
			if (k > j) j = k;
			sname = nname;
		    }
		}
	    }
	    else if (j) { /* previous s */
		if (!not_ok) {
		    if (n || strncmp (nname, sname, i)) not_ok = 1;
		    else if (!n) n = fnet;
		}
	    }
	    else if (n) not_ok = 1;
	    else { n = fnet; i = strlen ((sname = nname)); }

	    nref = nref -> next;
	    if (!nref || nref -> cd != cd_cnt) { /* new conductor */
		if (not_ok || !n || !j) {
		    if (!n) fprintf (stderr, "warning: incorrect net_name: %s\n", ref -> n -> net_name);
		    n = ref -> n;
		    s = n -> net_name;
		    if (searchInst) { /* test for local node */
			while (isdigit ((int)*s)) ++s;
			if (!*s && (s = other_name2 (ntw, ref))) { /* use instance */
			    n -> net_name = s;
			}
			else s = n -> net_name;
		    }
		    i = 0;
		    if (isdigit ((int)*s)) buf[i++] = '\\';
		    while (*s) { if (*s == ':') buf[i++] = '\\'; buf[i++] = *s++; }
		    j = 0;
		    do {
			n = ref -> n;
			if (!other_name (ntw, n)) {
			    sprintf (buf+i, ":%d", ++j);
			    nodetab[n->nx].spef_name = newStringSpace (buf);
			}
		    } while ((ref = ref -> next) != nref);
		}
		else { /* n has the name w/o index; j = last found index */
		    do {
			if (!other_name (ntw, ref->n) && ref->n == n) { /* label */
			    if (isdigit ((int)*(n -> net_name)))
				sprintf (buf, "\\%s:%d", n -> net_name, j+1);
			    else
				sprintf (buf, "%s:%d", n -> net_name, j+1);
			    nodetab[n->nx].spef_name = newStringSpace (buf);
			}
		    } while ((ref = ref -> next) != nref);
		}
	    }
	}
	cd_cnt = -1;
    }

    spfref = ref = NULL;

    for (nref = nets; nref; nref = nref -> next) {

	fnet = n = nref -> n;

	if (!fnet -> nx) continue; /* net decl. of array */

	sname = nname = n -> net_name;

	if (use_spef) {
	    if (!strcmp (nname, nameGND) && !nref -> next) {
		if (!groundVnet) continue; /* skip last net */
		groundVnet = 2;
	    }
	}
	else { /* SPF format */
	    //if (nref -> cd == 0) continue; /* skip SUBSTR net */
	    leftName = nameToStr (0, nname, n -> net_dim, n -> net_lower);
	    if (test0 (leftName) == 0) { nref -> done = 1; continue; } /* skip GND net */
	}

	if (nref -> cd != cd_cnt) { // new conductor
	    if (cd_cnt >= 0) { // not first conductor
		if (ref) { /* SPF format */
		    while (ref -> cd == cd_cnt) {
			if (!ref -> done) {
			    oprint (0, "*|S ("); n = ref -> n;
			    nmprint (1, n -> net_name, n -> net_dim, n -> net_lower, Nil, 0);
			    oprint (1, " 0 0)\n");
			}
			if (!(ref = ref -> next)) break;
		    }
		    ref = NULL;
		}
		if (C_cnt) { C_cnt = 0;
		    if (use_spef) { oprint (0, "*CAP\n"); c_cnt = 0; }
		    rctype = 'C'; prInst (ntw, spfref);
		}
		if (use_spef && (R_cnt || (vn_beg && spef_doRES))) { oprint (0, "*RES\n"); c_cnt = 0; }
		if (R_cnt) { R_cnt = 0;
		    rctype = 'R'; prInst (ntw, spfref);
		    spef_doRES = 1;
		}
		if (vn_beg) do_vnets ();
		if (use_spef) oprint (0, "*END\n");
	    }
	    spef_doRES = !omitres; /* always *RES (if true) */
	    spfref = nref;
	    cd_cnt = nref -> cd;
	    if (use_spef) {
		spef_print = 1;
		oprint (0, "\n*D_NET ");
		Conn = 0;
		tog_use0 = 0;
	    }
	    else {
		//sprintf (buf, "* cd%d\n", cd_cnt);
		//oprint (0, buf);
		oprint (0, "*|NET ");
	    }
	    nmprint (1, sname, n -> net_dim, n -> net_lower, Nil, 0);
	    oprint (1, " 0\n");
	    if (use_spef) tog_use0 = tog_use0_save;
	}

	tref = ntw -> terms;
	while (tref && unequalTermNet (tref -> t, n, 0)) tref = tref -> next;
	if (tref) {
	    nref -> done = 1;
	    if (use_spef) {
		if (!Conn) { ++Conn; oprint (0, "*CONN\n"); }
		oprint (0, "*P ");
	    }
	    else
		oprint (0, "*|P (");
	    nmprint (1, nname, n -> net_dim, n -> net_lower, Nil, 0);
	    if (use_spef) {
		if (coordinates)
		    sprintf (buf, " B *C %d %d\n", n -> x, n -> y);
		else
		    sprintf (buf, " B\n");
		oprint (1, buf);
	    }
	    else
		oprint (1, " X 0 0 0)\n");
	}
	else if (coordinates) {
	    leftName = nodetab[fnet->nx].spef_name;
	    if ((s = strchr (leftName, ':')) && isdigit(*(s+1))) {
		if (!Conn) { ++Conn; oprint (0, "*CONN\n"); }
		oprint (0, "*N ");
		sprintf (buf, "%s *C %d %d\n", leftName, n -> x, n -> y);
		oprint (1, buf);
	    }
	}

	term_ground = 0;
	gnet = NULL;

	leftName = nameToStr (0, fnet -> net_name, fnet -> net_dim, fnet -> net_lower);
	leftNameNbr = testNameNbr (leftName);

	anet = NULL;
	if (nodetab[fnet->nx].isTerm) { /* terminal net */
	    for (tref = ntw -> terms; tref; tref = tref -> next) {
		if (!unequalTermNet (tref -> t, fnet, 1)) { anet = fnet; break; }
	    }
	}
	if (tog_use0 && test0 (leftName) == 0) { /* connected to GND */
	    if (anet) term_ground = 1;
	    gnet = fnet;
	}

	if (!anet) {
	    if (!strcmp (fnet -> net_name, "GND")) anet = fnet;
	    else
	    if (!strcmp (fnet -> net_name, "SUBSTR")) anet = fnet;
	    if (anet) {
		if (leftNameNbr >= 0 && groundVnet != 2) cannot_die (4, "not new number");
		leftNameNbr = assignNameNbr (leftName, leftNameNbr);
	    }
	}

	for (n = fnet -> net_eqv; n; n = n -> net_eqv) { /* for all subnets */

	    sname = n -> net_name;
	    iname = n -> inst_name;

	    if (!iname) {

                /* Find the connections (name equivalences) */

		not = 0;

		/* try to remove useless net statements */

		if (nodetab[fnet->nx].isTerm) { /* terminal net */
		    for (tref = ntw -> terms; tref; tref = tref -> next) {
			if (!unequalTermNet (tref -> t, n, 1)) break;
		    }
		}
		else tref = NULL;

		rightName = nameToStr (1, sname, n -> net_dim, n -> net_lower);
		if (tog_use0 && test0 (rightName) == 0) { /* connected to GND */
		    if (tref) term_ground = 1;
		    gnet = n;
		}

		if (!tref) {
		    not = 1;
		}
		else {
		    nref -> done = 1;
		    if (use_spef) {
			if (!Conn) { ++Conn; oprint (0, "*CONN\n"); }
			oprint (0, "*P ");
			s = nodetab[fnet->nx].spef_name;
			if (strcmp (rightName, s) != 0) {
			    NEW_VNET (v);
			    v -> a_name = newStringSpace (nameToSpf (sname, n -> net_dim, n -> net_lower));
			    v -> b_name = newStringSpace (nameToSpf (s, fnet -> net_dim, fnet -> net_lower));
			}
			not = 1;
		    }
		    else
			oprint (0, "*|P (");
		    nmprint (1, sname, n -> net_dim, n -> net_lower, Nil, 0);
		    if (use_spef) {
			if (coordinates)
			    sprintf (buf, " B *C %d %d\n", n -> x, n -> y);
			else
			    sprintf (buf, " B\n");
			oprint (1, buf);
		    }
		    else
			oprint (1, " X 0 0 0)\n");

		    if (!anet) {
			anet = n; /* use 'b' as node name */
			not = 1;
		    }
		}

		if (!not && unequalNet (anet, n)) { /* unequal terminal net */
		    NEW_VNET (v);
		    v -> net_cnt = ++net_cnt;
		    v -> a_name = newStringSpace (nameToSpf (sname, n -> net_dim, n -> net_lower));
		    if (use_spef) s = nameToSpf (nodetab[fnet->nx].spef_name, fnet -> net_dim, fnet -> net_lower);
		    else s = nameToSpf (anet -> net_name, anet -> net_dim, anet -> net_lower);
		    v -> b_name = newStringSpace (s);
		}

	    }
	    else { /* inst_connect */
		if (*iname == '_' && (iname[1] == 'R' || iname[1] == 'C')) {
		    if (iname[1] == 'R') ++R_cnt;
		    else                 ++C_cnt;
		}
		else {
		    nref -> done = 1;

		    s = nameToSpf (iname, 0, Nil);
		    strcpy (buf, s);
		    s = buf + (end_str - s);
		    *s = *delimiter;
		    strcpy (s+1, nameToSpf (sname, 0, Nil));

		    if (use_spef) sname = nodetab[fnet->nx].spef_name;
		    else sname = fnet -> net_name;
		    sname = nameToSpf (sname, fnet -> net_dim, fnet -> net_lower);
		    if (strcmp (buf, sname) != 0) {
			NEW_VNET (v);
			v -> net_cnt = ++net_cnt;
			v -> a_name = newStringSpace (buf);
			v -> b_name = newStringSpace (sname);
		    }

		    if (use_spef) {
			if (!Conn) { ++Conn; oprint (0, "*CONN\n"); }
			oprint (0, "*I ");
			oprint (1, buf);
			if (coordinates) {
			    s = NULL;
			    if (driver) {
				f = findInst (iname);
				if (f && f -> valid == 2) s = f -> v.name;
				else fprintf (stderr, "warning: no %sdriver found\n", f ? "valid " : "");
			    }
			    if (s)
				sprintf (buf, " B *C %d %d *D %s\n", n -> x, n -> y, s);
			    else
				sprintf (buf, " B *C %d %d\n", n -> x, n -> y);
			}
			else sprintf (buf, " B\n");
			oprint (1, buf);
		    }
		    else {
			oprint (0, "*|I (");
			oprint (1, buf);
			oprint (1, " ");
			*s = ' ';
			oprint (1, buf);
			oprint (1, " X 0 0 0)\n");
		    }
		}
	    }
	}

	if (gnet && !term_ground) { /* label GND connect */
	    if (anet) { /* label net contains a terminal */
		if (!use_spef) {
		    NEW_VNET (v);
		    v -> net_cnt = ++net_cnt;
		    v -> a_name = newStringSpace (nameToSpf (anet -> net_name, anet -> net_dim, anet -> net_lower));
		    v -> b_name = "0"; /* ground */
		}
	    }
	    else anet = gnet;
	}

	if (!use_spef) { /* SPF format */
	    if (!nref -> done && !ref) ref = nref;

	    if (anet && anet != fnet) { /* use anet as first net */
		//nodetab[fnet->nx].spef_name =
		fnet -> net_name  = anet -> net_name;
		fnet -> net_dim   = anet -> net_dim;
		fnet -> net_lower = anet -> net_lower;
	    }
	}
	else if (gnet && groundVnet != 2) {
	    NEW_VNET (v);
	    v -> a_name = nodetab[fnet->nx].spef_name;
	    v -> b_name = nameGND;
	    spef_doRES = 1;
	}
    }

    if (cd_cnt >= 0) { // not first conductor
	for (; ref && ref -> cd == cd_cnt; ref = ref -> next) { /* only SPF */
	    if (!ref -> done) {
		oprint (0, "*|S ("); n = ref -> n;
		nmprint (1, n -> net_name, n -> net_dim, n -> net_lower, Nil, 0);
		oprint (1, " 0 0)\n");
	    }
	}
	if (C_cnt) {
	    if (use_spef) { oprint (0, "*CAP\n"); c_cnt = 0; }
	    rctype = 'C'; prInst (ntw, spfref);
	}
	if (use_spef && (R_cnt || (vn_beg && spef_doRES))) { oprint (0, "*RES\n"); c_cnt = 0; }
	if (R_cnt) {
	    rctype = 'R'; prInst (ntw, spfref);
	    spef_doRES = 1;
	}
	if (vn_beg) do_vnets ();
	oprint (0, use_spef ? "*END\n" : "*\n");
    }

    out_indent = 0;
}

static char *nameToStr (int nr, char *name, int dim, int *lower)
{
    static char bufx[DM_MAXNAME + 132];
    static char bufy[DM_MAXNAME + 132];
    char *buf, *s;
    int i;

    buf = nr ? bufy : bufx;

    s = buf;
    *s = *name;
    while (*s) *++s = *++name;

    if (dim > 0) {
	*s++ = '[';
	for (i = 0;;) {
	    sprintf (s, "%d", lower[i]);
	    while (*++s) ;
	    if (++i >= dim) break;
	    *s++ = ',';
	}
	*s++ = ']';
	*s = 0;
    }

    return (buf);
}

static char *nameToSpf (char *name, int dim, int *lower)
{
    static char bufz[DM_MAXNAME + 132];
    char *s;
    int i;

    s = bufz;
 // if (use_spef && isdigit ((int)*name)) *s++ = '\\';

    while (*name) {
	if (tog_nobrack && (*name == '[' || *name == ',' || *name == ']')) {
	    *s++ = '_'; name++;
	}
	else {
	    //if (use_spef && ((s == bufz && isdigit ((int)*name)) || (!isalnum ((int)*name) && *name != '_' && *name != ':'))) *s++ = '\\';
	    *s++ = *name++;
	}
    }

    if (dim > 0) {
	if (tog_nobrack)
	    *s++ = '_';
	else {
	    //if (use_spef) *s++ = '\\';
	    *s++ = '[';
	}
	for (i = 0;;) {
	    sprintf (s, "%d", lower[i]);
	    while (*++s) ;
	    if (++i >= dim) break;
	    if (tog_nobrack)
		*s++ = '_';
	    else {
		//if (use_spef) *s++ = '\\';
		*s++ = ',';
	    }
	}
	if (tog_nobrack)
	    *s++ = '_';
	else {
	    //if (use_spef) *s++ = '\\';
	    *s++ = ']';
	}
    }
    *s = 0;

    end_str = s;
    return (bufz);
}

static int unequalNet (struct net_el *n1, struct net_el *n2)
{
    char *s;
    int i, k;

    if (n1 -> net_dim != n2 -> net_dim) {
	if (n1 -> net_dim == 0 && (s = strchr (n1 -> net_name, '['))) {
	    *s = 0;
	    i = strcmp (n1 -> net_name, n2 -> net_name);
	    *s = '[';
	    if (i) return (1);
	    for (i = 0; i < n2 -> net_dim; ++i) {
		++s;
		if (!isdigit (*s)) return (1);
		k = (*s++ - '0');
		while (isdigit (*s)) k = 10*k + (*s++ - '0');
		if (k != n2 -> net_lower[i]) return (1);
		if (*s != ',') break;
	    }
	    if (*s++ != ']') return (1);
	    if (*s || i+1 != n2 -> net_dim) return (1);
	    return (0);
	}
	return (1);
    }

    if (strcmp (n1 -> net_name, n2 -> net_name)) return (1);

    for (i = n1 -> net_dim; --i >= 0;) {
	if (n1 -> net_lower[i] != n2 -> net_lower[i]) return (1);
    }
    return (0);
}

static int unequalTermNet (struct cirterm *t, struct net_el *n, int tb)
{
    char *s;
    int i, k;

    if (t -> term_dim != n -> net_dim) {
	if (tb && n -> net_dim == 0 && (s = strchr (n -> net_name, '['))) {
	    *s = 0;
	    i = strcmp (t -> term_name, n -> net_name);
	    *s = '[';
	    if (i) return (1);
	    for (i = 0; i < t -> term_dim; ++i) {
		++s;
		if (!isdigit (*s)) return (1);
		k = (*s++ - '0');
		while (isdigit (*s)) k = 10*k + (*s++ - '0');
		if (t -> term_lower[i] <= t -> term_upper[i]) {
		    if (k < t -> term_lower[i] || k > t -> term_upper[i]) return (1);
		}
		else {
		    if (k < t -> term_upper[i] || k > t -> term_lower[i]) return (1);
		}
		if (*s != ',') break;
	    }
	    if (*s++ != ']') return (1);
	    if (*s || i+1 != t -> term_dim) return (1);
	    return (0);
	}
	return (1);
    }

    if (strcmp (t -> term_name, n -> net_name)) return (1);

    for (i = t -> term_dim; --i >= 0;) {
	k = n -> net_lower[i];
	if (t -> term_lower[i] <= t -> term_upper[i]) {
	    if (k < t -> term_lower[i] || k > t -> term_upper[i]) return (1);
	}
	else {
	    if (k < t -> term_upper[i] || k > t -> term_lower[i]) return (1);
	}
    }
    return (0);
}
