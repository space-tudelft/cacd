/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* load the voltage source structure with those pointers needed later
 * for fast matrix loading
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ccvsdefs.h"
#include "util.h"
#include "sperror.h"

int CCVSsetup(SMPmatrix *matrix, GENmodel *inModel, CKTcircuit *ckt, int *states)
{
    register CCVSmodel *model = (CCVSmodel*)inModel;
    register CCVSinstance *here;
    int error;
    CKTnode *tmp;

    /* loop through all the voltage source models */
    for ( ; model; model = model->CCVSnextModel) {

        /* loop through all the instances of the model */
        for (here = model->CCVSinstances; here; here = here->CCVSnextInstance) {

            if (here->CCVSbranch == 0) {
                error = CKTmkCur(ckt, &tmp, here->CCVSname, "branch");
                if (error) return(error);
                here->CCVSbranch = tmp->number;
            }
            here->CCVScontBranch = CKTfndBranch(ckt, here->CCVScontName);
            if (here->CCVScontBranch == 0) {
                IFuid namarray[2];
                namarray[0] = here->CCVSname;
                namarray[1] = here->CCVScontName;
                (*(SPfrontEnd->IFerror))(ERR_FATAL, "%s: unknown controlling source %s", namarray);
                return(E_BADPARM);
            }

/* macro to make elements with built in test for out of memory */
#define TSTALLOC(ptr, first, second) \
if (!(here->ptr = SMPmakeElt(matrix, here->first, here->second))) return(E_NOMEM);

            TSTALLOC(CCVSposIbrptr, CCVSposNode, CCVSbranch)
            TSTALLOC(CCVSnegIbrptr, CCVSnegNode, CCVSbranch)
            TSTALLOC(CCVSibrNegptr, CCVSbranch, CCVSnegNode)
            TSTALLOC(CCVSibrPosptr, CCVSbranch, CCVSposNode)
            TSTALLOC(CCVSibrContBrptr, CCVSbranch, CCVScontBranch)
        }
    }
    return(OK);
}

int CCVSunsetup(GENmodel *inModel, CKTcircuit *ckt)
{
#ifndef HAS_BATCHSIM
    CCVSmodel *model;
    CCVSinstance *here;

    for (model = (CCVSmodel *)inModel; model; model = model->CCVSnextModel)
    {
        for (here = model->CCVSinstances; here; here = here->CCVSnextInstance)
	{
	    if (here->CCVSbranch) {
		CKTdltNNum(ckt, here->CCVSbranch);
		here->CCVSbranch = 0;
	    }
	}
    }
#endif
    return OK;
}
