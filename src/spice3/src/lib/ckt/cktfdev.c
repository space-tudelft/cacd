/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "cktdefs.h"
#include "util.h"
#include "sperror.h"

int CKTfndDev(GENERIC *Ckt, int *type, GENERIC **fast, IFuid name, GENERIC *modfast, IFuid modname)
{
    register CKTcircuit *ckt = (CKTcircuit *)Ckt;
    register GENinstance *here;
    register GENmodel *mods;
    register int i;

    if (fast && *fast) {
        /* already have fast, so nothing much to do */
        /* just get & set type */
        if (type) *type = (*((GENinstance**)fast))->GENmodPtr->GENmodType;
        return(OK);
    }

    if (modfast) {
        /* have model, just need device */
        mods = (GENmodel*)modfast;
        for (here = mods->GENinstances; here; here = here->GENnextInstance) {
            if (here->GENname == name) {
                if (fast) *(GENinstance **)fast = here;
                if (type) *type = mods->GENmodType;
                return(OK);
            }
        }
        return(E_NODEV);
    }

    if (type)
    if (*type >= 0 && *type < DEVmaxnum) {
        /* have device type, need to find model & device */
        /* look through all models */
        for (mods = ckt->CKThead[*type]; mods; mods = mods->GENnextModel) {
            /* and all instances */
            if (!modname || mods->GENmodName == modname) {
                for (here = mods->GENinstances; here; here = here->GENnextInstance) {
                    if (here->GENname == name) {
                        if (fast) *(GENinstance **)fast = here;
                        return(OK);
                    }
                }
                if (mods->GENmodName == modname) return(E_NODEV);
            }
        }
        return(E_NOMOD);
    } else if (*type == -1) {
        /* look through all types (UGH - worst case - take forever) */
        for (i = 0; i < DEVmaxnum; i++) {
            /* need to find model & device */
            /* look through all models */
            for (mods = ckt->CKThead[i]; mods; mods = mods->GENnextModel) {
                /* and all instances */
                if (!modname || mods->GENmodName == modname) {
                    for (here = mods->GENinstances; here; here = here->GENnextInstance) {
                        if (here->GENname == name) {
                            if (fast) *(GENinstance **)fast = here;
			    *type = i;
                            return(OK);
                        }
                    }
                    if (mods->GENmodName == modname) { *type = i; return(E_NODEV); }
                }
            }
        }
        return(E_NODEV);
    }
    return(E_BADPARM);
}
