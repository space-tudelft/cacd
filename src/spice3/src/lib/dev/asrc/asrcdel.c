/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1987 Kanwar Jit Singh
**********/

#include "spice.h"
#include <stdio.h>
#include "asrcdefs.h"
#include "util.h"
#include "sperror.h"

int ASRCdelete(GENmodel *model, IFuid name, GENinstance **fast)
{
    ASRCinstance **instPtr = (ASRCinstance**)fast;
    ASRCinstance **prev = NULL;
    ASRCinstance *here;
    ASRCmodel *modPtr = (ASRCmodel*)model;

    for ( ; modPtr; modPtr = modPtr->ASRCnextModel) {
        prev = &modPtr->ASRCinstances;
        for (here = *prev; here; here = *prev) {
            if (here->ASRCname == name || (instPtr && here == *instPtr)) {
                *prev = here->ASRCnextInstance;
                FREE(here);
                return(OK);
            }
            prev = &here->ASRCnextInstance;
        }
    }
    return(E_NODEV);
}
