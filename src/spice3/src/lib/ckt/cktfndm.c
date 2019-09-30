/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "util.h"
#include "sperror.h"

int CKTfndMod(GENERIC *ckt, int *type, GENERIC **modfast, IFuid modname)
{
    register GENmodel *mods;
    register int i;

    if (modfast && *modfast) {
        /* already have modfast, so nothing to do */
        if (type) *type = (*(GENmodel **)modfast)->GENmodType;
        return(OK);
    }

    if (type)
    if (*type >= 0 && *type < DEVmaxnum) {
        /* have device type, need to find model */
        /* look through all models */
        for (mods = ((CKTcircuit *)ckt)->CKThead[*type]; mods; mods = mods->GENnextModel) {
            if (mods->GENmodName == modname) {
                *modfast = (char *)mods;
                return(OK);
            }
        }
        return(E_NOMOD);
    } else if (*type == -1) {
        /* look through all types (UGH - worst case - take forever) */
        for (i = 0; i < DEVmaxnum; i++) {
            /* need to find model & device */
            /* look through all models */
            for (mods = ((CKTcircuit *)ckt)->CKThead[i]; mods; mods = mods->GENnextModel) {
                if (mods->GENmodName == modname) {
		    *type = i;
                    *modfast = (char *)mods;
                    return(OK);
                }
            }
        }
        return(E_NOMOD);
    }
    return(E_BADPARM);
}
