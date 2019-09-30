/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * INPaName()
 *
 *  Take a parameter by Name and ask for the specified value
 * *dev is -1 if type unknown, otherwise, device type
 * **fast is a device, and will be set if possible.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "cpdefs.h"
#include "fteext.h"
#include "ifsim.h"
#include "iferrmsg.h"

int INPaName(char *parm, IFvalue *val, GENERIC *ckt, int *dev, char *devnam,
	GENERIC **fast, IFsimulator *sim, int *dataType, IFvalue *selector)
	/* parm - the name of the parameter to set */
	/* val  - the parameter union containing the value to set */
	/* ckt  - the circuit this device is a member of */
	/* dev  - the device type code to the device being parsed */
	/* devnam - the name of the device */
	/* fast - direct pointer to device being parsed */
	/* sim  - the simulator data structure */
	/* dataType - the datatype of the returned value structure */
	/* selector - data sub-selector for questions */
{
    int error; /* int to store evaluate error return codes in */
    int i;
    IFdevice *device;

    /* find the instance - don't know about model, so use null there,
     * otherwise pass on as much info as we have about the device
     * (name, type, direct pointer) - the type and direct pointer
     * WILL be set on return unless error is not OK
     */
    error = (*(sim->findInstance))(ckt, dev, fast, devnam, (GENERIC *)NULL, (char *)NULL);
    if (error) return(error);

    /* now find the parameter - hunt through the parameter tables for
     * this device type and look for a name match of an 'ask'able parameter.
     */
    device = sim->devices[*dev];
    for (i = 0; i < *device->numInstanceParms; i++) {
        if (eq(parm, device->instanceParms[i].keyword) &&
		     device->instanceParms[i].dataType & IF_ASK) {
            /* found it, so we ask the question using the device info we got
             * above and put the results in the IFvalue structure our caller
             * gave us originally
             */
            error = (*(sim->askInstanceQuest))(ckt, *fast,
                    device->instanceParms[i].id, val, selector);
            if (dataType) *dataType = device->instanceParms[i].dataType;
            return(error);
        }
    }
    return(E_BADPARM);
}
