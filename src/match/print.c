static char *rcsid = "$Id: print.c,v 1.1 2018/04/30 12:17:44 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
/*
 * Contains functions to print all kinds of information
 * about the datastructure.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*      FUNCTIONS:
 */
Private void print_edif_block (FILE *fp, block *b, long cnt);
Private void print_anno (partition *p);
Private void print_anno_block (DM_STREAM *dmfp_anno, DM_STREAM *dmfp_tor, block *ThiS);
#if 0
Private char *fvalPar (char *val);
#endif

/*      VARIABLES:
*/
Import network * nom_netw;
Import network * act_netw;
Import boolean b_opt, r_opt;
Import boolean edif_opt;
Import boolean par_opt;
Import boolean anno_opt;

Import string string_cap, string_res;
Import string string_x, string_y, string_v;
Import string act_netw_name;
Import DM_PROJECT *dmproject;

DM_PROCDATA *procdata;

Private char * spaces = "                                                  ";
Private char * indent;
Private int ind_ind = 0;

#define INDENT		indent -= 2; ind_ind++; Assert (indent >= spaces);
#define INITINDENT	indent = spaces + strlen (spaces);
#define DEINDENT	indent += 2; ind_ind--; Assert (ind_ind >= 0);
#define PRINDENT(fp)	fprintf (fp, "%s", indent);

/*
 * Prints the network in a standard format
 * on the specified stream.
 */
Public void print_netw (FILE *fp, network *netw)
{
    object *obj, *tmp;
    link_type * lnk;
    bucket * par;
    boolean net_stmt, net_printed;
    string  cellname;

    if (!netw) return;

    fprintf (fp, "\n");
    if (Get_field (netw, TYPE) == PRIMITIVE)
	fprintf (fp, "primitive ");
    if ((obj = netw -> thead)) {
	fprintf (fp, "network %s (terminal %s", netw -> name, obj -> name);
	while ((obj = obj -> next_obj)) {
	    fprintf (fp, ", %s", obj -> name);
	}
	fprintf (fp, ")\n{\n");
    }
    else
	fprintf (fp, "network %s ()\n{\n", netw -> name);


 /* First, print the net statements */

    net_stmt = False;
    for (obj = netw -> thead; obj; obj = obj -> next_obj) {
	if (obj -> equiv == obj) {
	    net_printed = False;
	    for (tmp = netw -> thead; tmp; tmp = tmp -> next_obj) {
		if (tmp -> equiv == obj && tmp != obj) {
		    if (!net_printed) {
			fprintf (fp, "\tnet\t{ %s", obj -> name);
			net_printed = True;
		    }
		    fprintf (fp, ", %s", tmp -> name);
		}
	    }
	    for (tmp = netw -> nhead; tmp; tmp = tmp -> next_obj) {
		if (tmp -> equiv == obj && tmp != obj) {
		    if (!net_printed) {
			fprintf (fp, "\tnet\t{ %s", obj -> name);
			net_printed = True;
		    }
		    fprintf (fp, ", %s", tmp -> name);
		}
	    }
	    if (net_printed) { fprintf (fp, " };\n"); net_stmt = True; }
	}
    }
    for (obj = netw -> nhead; obj; obj = obj -> next_obj) {
	if (obj -> equiv == obj) {
	    net_printed = False;
	    for (tmp = netw -> thead; tmp; tmp = tmp -> next_obj) {
		if (tmp -> equiv == obj && tmp != obj) {
		    if (!net_printed) {
			fprintf (fp, "\tnet\t{ %s", obj -> name);
			net_printed = True;
		    }
		    fprintf (fp, ", %s", tmp -> name);
		}
	    }
	    for (tmp = netw -> nhead; tmp; tmp = tmp -> next_obj) {
		if (tmp -> equiv == obj && tmp != obj) {
		    if (!net_printed) {
			fprintf (fp, "\tnet\t{ %s", obj -> name);
			net_printed = True;
		    }
		    fprintf (fp, ", %s", tmp -> name);
		}
	    }
	    if (net_printed) { fprintf (fp, " };\n"); net_stmt = True; }
	}
    }

    obj = netw -> dhead;
    if (obj && net_stmt) fprintf (fp, "\n");

    /* print device list */
    for (; obj; obj = obj -> next_obj) {
	cellname = obj -> call -> name;
	if (strcmp (obj -> name, "."))
	    fprintf (fp, "{%s}\t%s\t", obj -> name, cellname);
	else
	    fprintf (fp, "\t%s\t", cellname);

	if (!strcmp (cellname, string_res) || !strcmp (cellname, string_cap)) {
	    fprintf (fp, "%s\t", (char*)obj -> par_list -> data);
	}
	else {
	    for (par = obj -> par_list; par; par = par -> next)
		fprintf (fp, "%s=%s ", par -> key, (char*)par -> data);
	}

	lnk = obj -> head;
	fprintf (fp, "\t(");
	while (lnk) {
	    fprintf (fp, "%s", lnk -> net? lnk -> net -> name : "");
	    lnk = lnk -> next_down;
	    if (lnk) fprintf (fp, ", ");
	}
	fprintf (fp, ");\n");
    }
    fprintf (fp, "}\n");
}

/*
 * Prints some statistics about the specified partition.
 */
Public void p_part_s (FILE *fp, partition *p)
{
    fprintf (fp, "Total number of elements:   %6ld\n", p -> n_elemts);
    fprintf (fp, "Number of iterations:       %6ld\n", p -> n_iter);
    fprintf (fp, "Number of bound  blocks:    %6ld\n\n", p -> n_bound);

    fprintf (fp, "        Active  blocks:     %6ld\n", p -> n_active);
    fprintf (fp, "        Passive blocks:     %6ld\n", p -> n_passive);
    fprintf (fp, "        Invalid blocks:     %6ld\n", p -> n_invalid);
    fprintf (fp, "%36s\n", "------ +");
    fprintf (fp, "%23s %10ld\n\n", "Total:", p -> n_blcks);
}

/*
 * Prints some statistics about the specified network.
 */
Public void p_netw_s (FILE *fp, network *netw)
{
    object *net;
    int n_term = netw -> n_terms;
    int n_nets = 0;
    int n_devs = 0;

    for (net = netw -> nhead; net; net = net -> next_obj) {
	if (!net -> equiv || net -> equiv == net) ++n_nets;
    }
    for (net = netw -> dhead; net; net = net -> next_obj) ++n_devs;

    fprintf (fp, "Network: %s\n", netw -> name);
    fprintf (fp, "                        %10u Terminals\n", n_term);
    fprintf (fp, "                        %10u Nets\n", n_nets);
    fprintf (fp, "                        %10u Devices\n", n_devs);
    fprintf (fp, "%36s\n", "------ +");
    fprintf (fp, "%23s %10u\n\n", "Total:", n_term + n_nets + n_devs);
}

Private void print_edif_header (FILE *fp, partition *p)
{
    PRINDENT (fp);
    fprintf (fp, "(matchOutput %s %s\n", nom_netw -> name, act_netw -> name);
    INDENT;
    PRINDENT (fp);
    fprintf (fp, "(matchFormat 1 1)\n");
    PRINDENT (fp);
    if (p -> n_invalid > 0) {
	fprintf (fp, "(matchResult failed)\n");
    }
    else {
	fprintf (fp, "(matchResult succeeded)\n");
    }
}

Private void print_edif_footer (FILE *fp)
{
    DEINDENT;
    PRINDENT (fp);
    fprintf (fp, ")\n");
}

Private void print_edif_group_start (FILE *fp, char *str)
{
    PRINDENT (fp);
    fprintf (fp, "(%s\n", str);
    INDENT;
}

Private void print_edif_group_end (FILE *fp)
{
    DEINDENT;
    PRINDENT (fp);
    fprintf (fp, ")\n");
}

Private void print_group_head (FILE *fp, char *str)
{
    fprintf (fp, "+---------------+\n");
    fprintf (fp, "| %-13s |\n", str);
    fprintf (fp, "+-------+-------+-----------------------------+-------------------------------------+\n");
    fprintf (fp, "|   P   | Network %-22.22s (Nom)| Network %-22.22s (Act)|\n", nom_netw -> name, act_netw -> name);
}

Private void print_group_tail (FILE *fp)
{
    fprintf (fp, "+-------+-------------------------------------+-------------------------------------+\n");
}

/*
 * Prints the unmatched parts of the partition
 * on the specified stream.
 */
Public void print_bindings (FILE *fp, partition *p)
{
    block *that, *ref;
    long   cnt;

    if (p == NULL) return;

    validate_list (&(p -> passive));

    if (anno_opt) print_anno (p);

    if (!b_opt) return;

    if (edif_opt) {
	INITINDENT;
	print_edif_header (fp, p);
    }

    ref = NULL;
    cnt = 0;
    for (that = p -> passive; that && that != ref; that = that -> next) {
	if (Get_flag (that, BOUNDED)) {
	    if (cnt == 0) {
		if (edif_opt) {
		    print_edif_group_start (fp, "matching");
		}
		else {
		    print_group_head (fp, "MATCHING");
		    print_group_tail (fp);
		}
	    }
	    cnt++;
	    if (edif_opt) {
		print_edif_block (fp, that, cnt);
	    }
	    else {
		print_block (fp, that, cnt, False);
	    }
	}

	ref = p -> passive;
    }
    if (cnt > 0) {
	if (edif_opt) {
	    print_edif_group_end (fp);
	}
	else {
	    print_group_tail (fp);
	}
    }

    ref = NULL;
    cnt = 0;
    for (that = p -> passive; that && that != ref; that = that -> next) {
	if (!Get_flag (that, BOUNDED)) {
	    if (cnt == 0) {
		if (edif_opt) {
		    print_edif_group_start (fp, "inconclusive");
		}
		else {
		    print_group_head (fp, "INCONCLUSIVE");
		}
	    }
	    cnt++;
	    if (edif_opt) {
		print_edif_block (fp, that, cnt);
	    }
	    else {
		print_block (fp, that, cnt, True);
	    }
	}

	ref = p -> passive;
    }

    if (cnt > 0) {
	if (edif_opt) {
	    print_edif_group_end (fp);
	}
	else {
	    print_group_tail (fp);
	}
    }


    ref = NULL;
    for (that = p -> invalid; that && that != ref; that = that -> next) {
	if (that -> parent) {
	    that -> parent -> level = 1;
	}

	ref = p -> invalid;
    }

    cnt = 0;
    ref = NULL;
    for (that = p -> invalid; that && that != ref; that = that -> next) {
	if (cnt == 0) {
	    if (edif_opt) {
		print_edif_group_start (fp, "deficient");
	    }
	    else {
		print_group_head (fp, "DEFICIENT");
	    }
	}
	if (!(that -> parent)) {
	    cnt++;
	    if (edif_opt) {
		print_edif_block (fp, that, cnt);
	    }
	    else {
		print_block (fp, that, cnt, True);
	    }
	}
	else if (that -> parent -> level == 1) {
	    long i;
	    block * ThiS;
	    cnt++;
	    that -> parent -> level = 0;
	    ThiS = that -> parent;
	    for (i = 0; i < ThiS -> n_childs; i++) {
		if (Get_field (ThiS -> childs[i], STATE) != INVALID) {
					/* matching or inconclusive block */
		    continue;
		}
		if (ThiS -> childs[i] -> n_childs != 0) { /* Not a leaf block */
		    continue;
		}
		if (edif_opt) {
		    print_edif_block (fp, ThiS -> childs[i], cnt);
		}
		else {
		    print_block (fp, ThiS -> childs[i], cnt, True);
		}
	    }
	}

	ref = p -> invalid;
    }

    if (cnt > 0) {
	if (edif_opt) {
	    print_edif_group_end (fp);
	}
	else {
	    print_group_tail (fp);
	}
    }

    if (edif_opt) {
	print_edif_footer (fp);
    }
}

Private void print_edif_block (FILE *fp, block *ThiS, long cnt)
{
    object *act, *nom;
    int first_item = True;
    int no_items;

    if (!ThiS -> head) return;

    PRINDENT (fp);
    switch (Get_field (ThiS -> head, TYPE)) {
	case NET:
	    fprintf (fp, "(nets %ld\n", cnt);
	    INDENT;
	    break;
	case TERMINAL:
	    fprintf (fp, "(ports %ld\n", cnt);
	    INDENT;
	    break;
	case DEVICE:
	    fprintf (fp, "(instances %ld\n", cnt);
	    INDENT;
	    break;
    }

    first_item = True;
    no_items = 0;
    PRINDENT (fp);
    fprintf (fp, "(");
    for (nom = ThiS -> head; nom; nom = nom -> next_elm) {
	if (nom -> netw == nom_netw) {
	    if (first_item) {
		fprintf (fp, "names");
		first_item = False;
	    }

	    if (no_items >= 10) {
		no_items = 0;
		fprintf (fp, "\n");
		PRINDENT (fp);
	    }

	    no_items++;
	    fprintf (fp, " %s", nom -> name);
	}
    }
    fprintf (fp, ")\n");

    first_item = True;
    no_items = 0;
    PRINDENT (fp);
    fprintf (fp, "(");
    for (act = ThiS -> head; act; act = act -> next_elm) {
	if (act -> netw == act_netw) {
	    if (first_item) {
		fprintf (fp, "names");
		first_item = False;
	    }

	    if (no_items >= 10) {
		no_items = 0;
		fprintf (fp, "\n");
		PRINDENT (fp);
	    }

	    no_items++;
	    fprintf (fp, " %s", act -> name);
	}
    }
    fprintf (fp, ")\n");

    DEINDENT;
    PRINDENT (fp);
    fprintf (fp, ")\n");
}

Public void print_block (FILE *fp, block *ThiS, long cnt, int new_block)
{
    bucket *buck;
    object *act, *nom;
    char *t, nom_name[40], nom_p[40], act_name[40], act_p[40];
    int i, j, first_line = True;

    nom = ThiS -> head;
    act = ThiS -> head;

    while (nom || act) {
	for (; nom; nom = nom -> next_elm) {
	    if (nom -> netw == nom_netw) break;
	}
	for (; act; act = act -> next_elm) {
	    if (act -> netw == act_netw) break;
	}

	if (!nom && !act) break;

	nom_name[0] = '\0';
	nom_p[0] = '\0';
	if (nom) {
	    switch (Get_field (nom, TYPE)) {
		case TERMINAL:
		    t = "terminal";
		    break;
		case DEVICE:
		    t = nom -> call -> name;
		    break;
		default:
		    t = "net";
	    }
	    i = strlen (nom -> name);
	    j = strlen (t);
	    if (i+j > 33) {
		if (j > 9) j = (i > 24)? 9 : 33 - i;
		i = 33 - j;
	    }
	    if (i < 10 && j < 24) i = 10;
	    sprintf (nom_name, "%-*.*s (%*.*s)", i, i, nom -> name, j, j, t);

	    if (par_opt) {
		i = 0;
		for (buck = nom -> par_list; buck && i < 36; buck = buck -> next) {
		    t = buck -> key;
		    while (i < 36 && *t) nom_p[i++] = *t++;
		    if (i < 36) nom_p[i++] = '=';
		    t = (char*)buck -> data;
		    while (i < 36 && *t) nom_p[i++] = *t++;
		    if (i < 36) nom_p[i++] = ';';
		    if (i < 36) nom_p[i++] = ' ';
		}
		nom_p[i] = '\0';
	    }

	    nom = nom -> next_elm;
	}

	act_name[0] = '\0';
	act_p[0] = '\0';
	if (act) {
	    switch (Get_field (act, TYPE)) {
		case TERMINAL:
		    t = "terminal";
		    break;
		case DEVICE:
		    t = act -> call -> name;
		    break;
		default:
		    t = "net";
	    }
	    i = strlen (act -> name);
	    j = strlen (t);
	    if (i+j > 33) {
		if (j > 9) j = (i > 24)? 9 : 33 - i;
		i = 33 - j;
	    }
	    if (i < 10 && j < 24) i = 10;
	    sprintf (act_name, "%-*.*s (%*.*s)", i, i, act -> name, j, j, t);

	    if (par_opt) {
		i = 0;
		for (buck = act -> par_list; buck && i < 36; buck = buck -> next) {
		    t = buck -> key;
		    while (i < 36 && *t) act_p[i++] = *t++;
		    if (i < 36) act_p[i++] = '=';
		    t = (char*)buck -> data;
		    while (i < 36 && *t) act_p[i++] = *t++;
		    if (i < 36) act_p[i++] = ';';
		    if (i < 36) act_p[i++] = ' ';
		}
		act_p[i] = '\0';
	    }

	    act = act -> next_elm;
	}

	if (new_block && first_line) {
	    print_group_tail (fp);
	}
	if (first_line) {
	    fprintf (fp, "|%6ld ", cnt);
	    first_line = False;
	}
	else {
	    fprintf (fp, "|       ");
	}
	fprintf (fp, "| %-36s| %-36s|\n", nom_name, act_name);

	if (*nom_p || *act_p) {
	    fprintf (fp, "|       | %36s| %36s|\n", nom_p, act_p);
	}
	first_line = False;
    }
}

#if 0
/* Copied from xsls */
Private char *fvalPar (char *val)
{
    static char fv_ret[32];
    static char fv_tmp[32];
    char *pv, *pr, *pt;
    int ncnt, mcnt, pcnt, exp, r, i, negative;

    Assert (val);

    if (*val == '-') {
	negative = 1; ++val;
    }
    else negative = 0;

    pv = val;
    pt = fv_tmp;

    ncnt = -1; /* number of digits after '.' */
    pcnt = 0;  /* precision (total number of digits) */
    while ((*pv >= '0' && *pv <= '9') || *pv == '.') {
	if (*pv == '.') {
	    ncnt = 0;
	}
	else {
	    pcnt++;
	    *pt++ = *pv;
	    if (ncnt >= 0) ncnt++;
	}
	pv++;
    }
    *pt = '\0';
    if (ncnt < 0) ncnt = 0;

    exp = 0;
    if (*pv == 'e' || *pv == 'E') {
	if (sscanf (pv + 1, "%d", &exp) != 1) exp = 0;
    }

    exp = exp + (pcnt - 1) - ncnt;
       /* exp + new ncnt - old ncnt */
    ncnt = pcnt - 1;
    mcnt = 1;  /* number of digits before '.' */

    /* the requirement is: exp % 3 == 0
			   and no zero before '.' (i.e. mcnt > 0) ! */

    if (exp > 0) {
	r = exp % 3;
	if (r != 0) {
	    if (mcnt - (3 - r) <= 0) {
		exp = exp - r;
		mcnt = mcnt + r;
	    }
	    else {
		exp = exp + (3 - r);
		mcnt = mcnt - (3 - r);
	    }
	}
    }
    else if (exp < 0) {
	r = -exp % 3;
	if (r != 0) {
	    if (mcnt - r <= 0) {
		exp = exp - (3 - r);
		mcnt = mcnt + (3 - r);
	    }
	    else {
		exp = exp + r;
		mcnt = mcnt - r;
	    }
	}
    }

    if (negative) {
	fv_ret[0] = '-';
	pr = fv_ret + 1;
    }
    else
	pr = fv_ret;

    pt = fv_tmp;
    for (i = 0; i < pcnt; i++) {
	if (i == mcnt) *pr++ = '.';
	*pr++ = *pt++;
    }
    while (i < mcnt) {
	*pr++ = '0';    /* trailing zero's */
	i++;
    }

    switch (exp) {
	case -15:sprintf (pr, "f"); break;
	case -12:sprintf (pr, "p"); break;
	case -9: sprintf (pr, "n"); break;
	case -6: sprintf (pr, "u"); break;
	case -3: sprintf (pr, "m"); break;
	case  0: *pr = '\0'; break;
	case  3: sprintf (pr, "k"); break;
	case  6: sprintf (pr, "M"); break;
	case  9: sprintf (pr, "G"); break;
	default: sprintf (pr, "e%d", exp);
    }

    /* cutTrail0s (fv_ret); */

    return (fv_ret);
}

#endif

Private void print_anno (partition *p)
{
    block *ref, *that;
    DM_CELL *key;
    DM_STREAM *dmfp_anno, *dmfp_tor;

    if (!dmproject) return;

    procdata = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);

    if (!(key = dmCheckOut (dmproject, act_netw_name, ACTUAL, DONTCARE, LAYOUT, ATTACH))) return;

    if (!(dmfp_anno = dmOpenStream (key, "annotations", "w"))) return;
    if (!(dmfp_tor  = dmOpenStream (key, "torname", "w"))) return;

    /* format record first */
    ganno.type = GA_FORMAT;
    ganno.o.format.fmajor = ganno.o.format.fminor = 1;
    dmPutDesignData (dmfp_anno, GEO_ANNOTATE);

    ref = NULL;
    for (that = p -> passive; that && that != ref; that = that -> next) {
	if (Get_flag (that, BOUNDED)) {
	    print_anno_block (dmfp_anno, dmfp_tor, that);
	}
	ref = p -> passive;
    }

    dmCloseStream (dmfp_anno, COMPLETE);
    dmCloseStream (dmfp_tor, COMPLETE);
}

Private void print_anno_block (DM_STREAM *dmfp_anno, DM_STREAM *dmfp_tor, block *ThiS)
{
    object *act, *nom;
    string act_s, nom_s;
    int act_t, nom_t;
    int k, i;
    char *s, *p;
    char mask[64];

    nom = ThiS -> head;
    act = ThiS -> head;

    while (nom || act) {
	for (; nom; nom = nom -> next_elm) {
	    if (nom -> netw == nom_netw) break;
	}
	for (; act; act = act -> next_elm) {
	    if (act -> netw == act_netw) break;
	}

	if (!nom && !act) break;

	nom_s = "";
	nom_t = -1;
	if (nom) {
	    nom_s = nom -> name;
	    nom_t = Get_field (nom, TYPE);
	}

	act_s = "";
        act_t = -1;
	if (act) {
	    act_s = act -> name;
	    act_t = Get_field (act, TYPE);
	}

        if (nom_t == NET && (act_t == NET || act_t == TERMINAL)) {
	    ganno.type = GA_LABEL;
	    strcpy (ganno.o.Label.name, nom_s);
	    ganno.o.Label.Class[0] = '\0';
	    ganno.o.Label.Attributes[0] = '\0';

            s = act_s;
            while (*s == '_') s++;
            i = 0;
            while (*s && *s != '_') mask[i++] = *s++;
            mask[i] = '\0';
            for (k = 0; k < procdata -> nomasks; k++) {
                if (strcmp (mask, procdata -> mask_name[k]) == 0) break;
            }
            if (k == procdata -> nomasks) {
                /* See if maskname has a trailing #number, where number
                   defines a mask number >= procdata -> nomasks.
                   In that case the mask is a new mask (created by Space)
                   and the specified number should be used.
                */
                k = -1;
                p = &mask[0];
                while (*p && *p != '#') p++;
                if (*p == '#' && *(p+1) >= '0' && *(p+1) <= '9')
                    k = atoi (p + 1);
            }
            if (k < 0) {
                fprintf (stderr, "Option -annotate: cannot extract mask name from node name '%s'\n", act_s);
                goto end_of_nom_act;
            }
	    ganno.o.Label.maskno = k;

            while (*s == '_') s++;
            if (*s && sscanf (s, "%le", &(ganno.o.Label.x)) == 1);
            else {
                fprintf (stderr, "Option -annotate: cannot extract x coordinate from node name '%s'\n", act_s);
                goto end_of_nom_act;
            }

            while (*s && *s != '_') s++;
            while (*s == '_') s++;
            if (*s && sscanf (s, "%le", &(ganno.o.Label.y)) == 1);
            else {
                fprintf (stderr, "Option -annotate: cannot extract y coordinate from node name '%s'\n", act_s);
                goto end_of_nom_act;
            }

	    ganno.o.Label.ay = 0;  /* 0=CENTER, -1=LEFT, 1=RIGHT */
	    ganno.o.Label.ax = 0;  /* 0=CENTER, -1=LEFT, 1=RIGHT */
	    dmPutDesignData (dmfp_anno, GEO_ANNOTATE);
	}
        else if (nom_t == DEVICE && act_t == DEVICE && nom_s[0] != '_') {
            /* Names that start with a '_' are not user defined and do
               not have to be printed in stream 'torname'. */
	    char *x_val, *y_val;
	    if ((x_val = get_par (act, string_x)) && (y_val = get_par (act, string_y))) {
                dmPrintf (dmfp_tor, "%s %s %s\n", x_val, y_val, nom_s);
	    }
        }

end_of_nom_act:

        if (nom) nom = nom -> next_elm;
        if (act) act = act -> next_elm;
    }
}
