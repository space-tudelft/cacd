/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "inpdefs.h"
#include "util.h"

extern INPmodel *modtab;

int INPlookMod (char *name)
{
    register INPmodel *mod;

    for (mod = modtab; mod; mod = mod->INPnextModel) {
        if (eq(mod->INPmodName, name)) {
            /* found the model in question - return true */
            return(1);
        }
    }
    /* didn't find model - return false */
    return(0);
}
