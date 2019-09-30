/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * INPpName()
 *
 *  Take a parameter by Name and set it on the specified device
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cpdefs.h"
#include "fteext.h"
#include "ifsim.h"
#include "iferrmsg.h"

int INPpName(char *parm, IFvalue *val, GENERIC *ckt, int dev, GENERIC *fast)
	/* parm - the name of the parameter to set */
	/* val  - the parameter union containing the value to set */
	/* ckt  - the circuit this device is a member of */
	/* dev  - the device type code to the device being parsed */
	/* fast - direct pointer to device being parsed */
{
    int error; /* int to store evaluate error return codes in */
    int i;
    IFdevice *device;

    device = ft_sim->devices[dev];
    for (i = 0; i < *device->numInstanceParms; i++) {
	if (eq(parm, device->instanceParms[i].keyword)) {
	    error = (*(ft_sim->setInstanceParm))(ckt, fast,
		device->instanceParms[i].id, val, (IFvalue*)NULL);
	    return(error);
	}
    }
    return(E_BADPARM);
}
