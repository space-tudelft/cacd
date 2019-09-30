
/*
 * ISC License
 *
 * Copyright (C) 1994-2013 by
 *	Frederik Beeftink
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

extern char *argv0;
extern char *DIO, *NPN, *PNP;
extern int incl_model;

char *model_sf;
char *errorNameTab[MAX_IN_TAB];
int errorName_cnt = 0;

struct lib_model *md_head = NULL;
struct lib_model *lm_head = NULL;
struct lib_model *lm_curr;

static struct lib_model *md_last;

static int findModel (struct model_info *mod, double geom[])
{
    double min, max;
    int i;
    char *name = lm_curr -> orig_name;

    do {
	if (lm_curr -> model_type < 4)
	for (i = 0; i < RANGE_NUMBER; i++) {
	    if (lm_curr -> lower[i]) {
		if (geom[i] == 0.0) goto next;
		min = atof (fvalEqn (lm_curr -> lower[i], geom));
		max = atof (fvalEqn (lm_curr -> upper[i], geom));
		if (geom[i] < min || geom[i] > max) goto next;
	    }
	    else if (lm_curr -> typical[i]) {
		if (geom[i] == 0.0) goto next;
		if (geom[i] != atof (fvalEqn (lm_curr -> typical[i], geom)))
		    goto next;
	    }
	}
	return (lm_curr -> model_type);
next:
	lm_curr = lm_curr -> next;
    } while (lm_curr && lm_curr -> orig_name == name);

    lm_curr = NULL;
    return (0);
}

struct numberInfo {
    char *name;
    int size;
    int cnt;
    double *geom;
    struct numberInfo *next;
};

static int numberModel (char *name, double geom[])
{
    int i, j, flag;
    struct numberInfo *ni;
    static struct numberInfo *niHead = NULL;
    static struct numberInfo *niTail = NULL;

    ni = niHead;
    while (ni && strcmp (ni -> name, name) != 0) {
	ni = ni -> next;
    }

    if (!ni) {
	PALLOC (ni, 1, struct numberInfo);
	if (niTail) {
	    niTail -> next = ni;
	    niTail = ni;
	}
	else
	    niHead = niTail = ni;
	ni -> name = strsave (name);
	ni -> size = 3;
	PALLOC (ni -> geom, RANGE_NUMBER * ni -> size, double);
	ni -> cnt = 0;
	ni -> next = NULL;
    }

    for (j = 0; j < ni -> cnt; j++) {
	flag = 1;
	for (i = 0; i < RANGE_NUMBER; i++) {
	    if (*(ni -> geom + RANGE_NUMBER * j + i) != geom[i]) flag = 0;
	}
	if (flag) break;
    }

    if (j < ni -> cnt) return (j + 1);

    if (ni -> size == ni -> cnt) {
	ni -> size = (int)(1.6 * ni -> size);
	REPALLOC (ni -> geom, RANGE_NUMBER * ni -> size, double);
    }

    for (i = 0; i < RANGE_NUMBER; i++) {
	*(ni -> geom + RANGE_NUMBER * j + i) = geom[i];
    }

    ni -> cnt++;

    return (j + 1);
}

static void add2ModelList (struct model_info *mod, double geom[])
{
    struct model_par *pm, *mp, *mp_lp;
    struct lib_model *md;
    char buf[256];
    char *name;

    if (lm_curr -> var_in_par) {
	sprintf (buf, "%s%d", lm_curr -> name, numberModel (lm_curr -> name, geom));
	mod -> out_name = strsave (buf);
    }
    else {
	mod -> out_name = lm_curr -> name;
    }

    name = mod -> out_name;

    for (md = md_head; md; md = md -> next)
	if (strcmp (md -> name, name) == 0) return;

    PALLOC (md, 1, struct lib_model);
    md -> name = name;
    md -> specified = lm_curr -> specified;
    md -> type_name = lm_curr -> type_name;
    md -> par_list = NULL;
    md -> next = NULL;

    mp_lp = NULL;
    for (pm = lm_curr -> par_list; pm; pm = pm -> next) {
	PALLOC (mp, 1, struct model_par);
	mp -> name = pm -> name;
	if (pm -> var_in_val)
	    mp -> value = strsave (fvalEqn (pm -> value, geom));
	else
	    mp -> value = pm -> value;
        mp -> separator = pm -> separator;
	mp -> next = NULL;

	if (mp_lp) mp_lp -> next = mp;
	else md -> par_list = mp;
	mp_lp = mp;
    }

    if (!md_head) md_head = md;
    else md_last -> next = md;
    md_last = md;
}

void fvalPar2Buf (char *bf, char *par, double val)
{
    char buf[32];
    sprintf (buf, "%g", val);
    sprintf (bf + strlen(bf), " %s=%s", par, fvalPar (par, buf));
}

struct lib_model *createModel (struct model_info *mod)
{
    int i, len;
    char *attr, *par, *val;
    static char buf[32];
    double geom[10];
    double value;

    for (i = 0; i < RANGE_NUMBER; i++) geom[i] = 0.0;

    lm_curr = mod -> createName;
    if (lm_curr -> model_type < 4 && (attr = cmc.inst_attribute) && *attr) {

	for (;;) {
	    attr = nextAttr (&par, &val, attr);
	    len = 0;
	    if (par) {
		if (par[len]) { par[len] = tolower (par[len]); len++; }
		if (par[len]) { par[len] = tolower (par[len]); len++; }
		if (par[len]) len++;
	    }
	    i = -1;
	    if (len == 1) {
		switch (*par) {
		case 'w': i = WI; break;
		case 'l': i = LE; break;
		case 'v': i = VL; break;
		}
	    }
	    else if (len == 2) {
		switch (*par) {
		case 'a': if (par[1] == 'e') i = AE; break;
		case 'p': if (par[1] == 'e') i = PE; break;
		case 'w': if (par[1] == 'b') i = WB; break;
		}
	    }
	    if (i >= 0) {
		/* all attributes are put in the 'geom' array */
		geom[i] = atof (val);
		if (incl_model && geom[i] == 0.0) {
		    P_E "%s: Sorry, cannot handle parameter '%s=%s' when computing a model for '%s'\n",
			argv0, par, val, cmc.cell_name);
		}
	    }
	    if (val) *(val - 1) = '=';
	    if (!attr) break;
	    *(attr - 1) = ';';
	}
    }

    /* In fact, after modifications have been made (11 oct 1995),
       and model parameters are computed with each model that
       has variables in the parameter list, it is now possible
       to combine exactModel, scalableModel and substitutionModel
       into one function. The only difference is that in the current
       situation, in case of mulitiple model matches, exactModel
       has a priority above scalableModel, and scalableModel has
       a priority above substitutionModel (AvG).
    */

    if (lm_curr -> model_type < 4 && findModel (mod, geom) == 2) { /* scalable model */
	i = -1;
	if (mod -> type_name == NPN || mod -> type_name == PNP) i = AE;
	else if (mod -> type_name == DIO) i = VL;
	buf[0] = '1';
	buf[1] = 0;
	if (i >= 0) {
	    if (lm_curr -> typical[i]) {
		value = atof (fvalEqn (lm_curr -> typical[i], geom));
		if (value != 0) value = geom[i] / value;
		if (value == 0) P_E "%s: Warning: scaling factor == 0 for model %s\n", argv0, lm_curr -> name);
		sprintf (buf, "%g", value);
	    }
	    else {
		P_E "%s: Must specify typical value for parameter '%s' for model '%s' in control file\n",
		    argv0, i == AE ? "ae" : (i == PE ? "pe" : "v"), lm_curr -> name);
	    }
	}
	model_sf = buf;
    }

    if (lm_curr) {
	if (incl_model)
	    add2ModelList (mod, geom);
	else
	    mod -> out_name = lm_curr -> name;
    }
    else { /* there is no default model specified (SdeG) */
	char bf[512];
	/* nothing else possible: output model cell_name and warn? */

	sprintf (bf, "%s with", mod -> orig_name);
	if (geom[AE]) fvalPar2Buf (bf, "ae", geom[AE]);
	if (geom[PE]) fvalPar2Buf (bf, "pe", geom[PE]);
	if (geom[WB]) fvalPar2Buf (bf, "wb", geom[WB]);
	if (geom[LE]) fvalPar2Buf (bf, "l" , geom[LE]);
	if (geom[WI]) fvalPar2Buf (bf, "w" , geom[WI]);
	if (geom[VL]) fvalPar2Buf (bf, "v" , geom[VL]);
	for (i = 0; i < errorName_cnt; i++)
	    if (strcmp (bf, errorNameTab[i]) == 0) break;
	if (i == errorName_cnt && i < MAX_IN_TAB) {
	    errorNameTab[errorName_cnt++] = strsave (bf);
	    P_E "%s: Warning: no library model for %s\n", argv0, bf);
	}
	mod -> out_name = mod -> name;
    }

    return (lm_curr);
}

void printModels ()
{
    char buf[512];
    char bf[4], *s;
    struct model_par *mp;
    struct lib_model *lm;

    bf[2] = '\0';
    if (!(lm = md_head)) return;
    do {
	if (!lm -> specified) {
	    sprintf (buf, "*.model %s %s\n", lm -> name, lm -> type_name);
	    oprint (0, buf);
	    continue;
	}
	sprintf (buf, ".model %s %s(", lm -> name, lm -> type_name);
	oprint (0, buf);

	if (lm -> par_list) {
	    bf[1] = ' ';
	    for (mp = lm -> par_list; mp; mp = mp -> next) {
		s = bf;
		if (mp -> separator) bf[0] = mp -> separator; else ++s;
		if (!mp -> next) bf[1] = ')';
		if (!mp -> value) sprintf (buf, "%s%s", mp -> name, s);
		else sprintf (buf, "%s=%s%s", mp -> name, fvalPar (mp -> name, mp -> value), s);
		oprint (0, buf);
	    }
	}
	else oprint (0, ")");
	oprint (0, "\n");
    } while ((lm = lm -> next));

    oprint (0, "\n");
}
