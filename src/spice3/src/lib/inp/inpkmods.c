/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"

extern INPmodel *modtab;

void INPkillMods()
{
    INPmodel *modtmp;

    while ((modtmp = modtab)) {
	modtab = modtmp->INPnextModel;
	FREE(modtmp);
    }
}
