/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * CKTpModName()
 *  Take a parameter by Name and set it on the specified model
 */

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "util.h"
#include "ifsim.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "gendefs.h"
#include "sperror.h"

extern SPICEdev *DEVices[];

int CKTpModName(char *parm, IFvalue *val, CKTcircuit *ckt, int type, IFuid name, GENmodel **modfast)
	/* parm - the name of the parameter to set */
	/* val  - the parameter union containing the value to set */
	/* ckt  - the circuit this model is a member of */
	/* type - the device type code to the model being parsed */
	/* name - the name of the model being parsed */
	/* modfast - direct pointer to model being parsed */
{
    int error;  /* int to store evaluate error return codes in */
    int i;

    for (i = 0; i < *DEVices[type]->DEVpublic.numModelParms; i++) {
	if (eq(parm, DEVices[type]->DEVpublic.modelParms[i].keyword)) {
	    error = CKTmodParam((GENERIC *)ckt, (GENERIC*)*modfast,
		DEVices[type]->DEVpublic.modelParms[i].id, val, (IFvalue*)NULL);
	    return(error);
	}
    }
    return(E_BADPARM);
}
