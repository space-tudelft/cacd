
/*
 * ISC License
 *
 * Copyright (C) 1987-2011 by
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

#include "src/xvhdl/incl.h"

extern int xtree;
extern int alsoImport;
extern char *cannot_handle_aliases;

struct model_info *Funcs = NULL;
struct model_info *preFuncs = NULL; /* predefined functions */
static struct model_info *flast; /* last predefined function */

char *prefuncs[] = {
	"nand",
	"nor",
	"and",
	"or",
	"invert",
	"exor",
};

struct model_info *findFunc (char *name, int imported, DM_PROJECT *father_proj)
{
    struct model_info *fun = NULL;
    int i = -1;

    switch (*name) { /* test for predefined function */
    case 'n':
	if (strcmp (name, prefuncs[0]) == 0) i = 0;
	else
	if (strcmp (name, prefuncs[1]) == 0) i = 1;
	break;
    case 'a':
	if (strcmp (name, prefuncs[2]) == 0) i = 2;
	break;
    case 'o':
	if (strcmp (name, prefuncs[3]) == 0) i = 3;
	break;
    case 'i':
	if (strcmp (name, prefuncs[4]) == 0) i = 4;
	break;
    case 'e':
	if (strcmp (name, prefuncs[5]) == 0) i = 5;
    }

    if (i >= 0) { /* predefined function */
	long *low, *up;
	long upper;
	char *val;
	int ninputs = 1; /* default */

	if ((val = getAttrValue (cmc.inst_attribute, "n")))
	    if (isdigit ((int)*val)) if (!(ninputs = *val - '0')) ninputs = 1;
	upper = ninputs - 1;

	/* first, check it from the existing predef. function list */

	for (fun = preFuncs; fun; fun = fun -> next) {
	    if (fun -> name == prefuncs[i]) { /* found */
		fun -> terms -> t -> term_upper[0] = upper;
		return (fun);
	    }
	}

	PALLOC (fun, 1, struct model_info);
	PALLOC (low, 1, long); low[0] = 0;
	PALLOC (up,  1, long); up[0] = upper;
	termstore (fun, "i", 1, low, up, INPUT);
	termstore (fun, "o", 0, NULL, NULL, OUTPUT);
	fun -> name = prefuncs[i];
	fun -> orig_name = fun -> name;
	fun -> dkey = NULL;

	if (!preFuncs) preFuncs = fun;
	else flast -> next = fun;
	flast = fun;
	goto found;
    }

    for (fun = Funcs; fun; fun = fun -> next)
	if (strcmp (fun -> name, name) == 0) return (fun); /* found */

    /* not found, read it ! */

    PALLOC (fun, 1, struct model_info);
    fun -> next = Funcs;
    Funcs = fun;

    if (imported == IMPORTED) {
	IMPCELL *icp = dmGetImpCell (father_proj, name, 1);
	if (xtree && alsoImport && strcmp (name, icp -> cellname))
	    fatalErr (cannot_handle_aliases, icp -> cellname);
	fun -> imported = imported;
	fun -> proj = dmOpenProject (icp -> dmpath, PROJ_READ);
	fun -> name = icp -> alias;
	fun -> orig_name = name = icp -> cellname;
    }
    else {
	fun -> proj = father_proj;
	fun -> orig_name = fun -> name = strsave (name);
    }

    fun -> dkey = dmCheckOut (fun -> proj, name, ACTUAL, DONTCARE, CIRCUIT, READONLY);
    readTerm (fun, 1);
    dmCheckIn (fun -> dkey, COMPLETE);

found:
    P_E "Warning: cell '%s' is function instance!\n", fun -> name);
    return (fun);
}