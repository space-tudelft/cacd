/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * CKTpName()
 *  Take a parameter by Name and set it on the specified device
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

int CKTpName(char *parm, IFvalue *val, CKTcircuit *ckt, int dev, char *name, GENinstance **fast)
	/* parm - the name of the parameter to set */
	/* val  - the parameter union containing the value to set */
	/* ckt  - the circuit this device is a member of */
	/* dev  - the device type code to the device being parsed */
	/* name - the name of the device being parsed */
	/* fast - direct pointer to device being parsed */
{
    int error;  /* int to store evaluate error return codes in */
    int i;

    for (i = 0; i < *DEVices[dev]->DEVpublic.numInstanceParms; i++) {
	if (eq(parm, DEVices[dev]->DEVpublic.instanceParms[i].keyword)) {
	    error = CKTparam((GENERIC*)ckt, (GENERIC *)*fast,
		     DEVices[dev]->DEVpublic.instanceParms[i].id, val, (IFvalue *)NULL);
	    return(error);
	}
    }
    return(E_BADPARM);
}
