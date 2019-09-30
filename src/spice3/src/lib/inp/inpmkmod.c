/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "util.h"
#include "inpdefs.h"
#include "iferrmsg.h"

INPmodel *modtab;

/* create/lookup a 'model' entry */

int INPmakeMod (char *name, int type, card *line)
{
    register INPmodel *mod, *prev;

    prev = NULL;
    for (mod = modtab; mod; mod = mod->INPnextModel) {
        if (eq(mod->INPmodName, name)) return(OK);
	prev = mod;
    }
    mod = (INPmodel *)MALLOC(sizeof(INPmodel));
    if (!mod) return(E_NOMEM);
    mod->INPmodName = name;
    mod->INPmodType = type;
    mod->INPnextModel = NULL;
    mod->INPmodUsed = 0;
    mod->INPmodLine = line;
    mod->INPmodfast = NULL;
    if (prev) prev->INPnextModel = mod;
    else modtab = mod;
    return(OK);
}
