/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * INPdevParse()
 *
 *  parse a given input according to the standard rules - look
 *  for the parameters given in the parmlists, In addition,
 *  an optional leading numeric parameter is handled.
 */

#include "spice.h"
#include <stdio.h>
#include "util.h"
#include "ifsim.h"
#include "inpdefs.h"
#include "iferrmsg.h"
#include "cpdefs.h"
#include "fteext.h"

char * INPdevParse(char **line, GENERIC *ckt, int dev, GENERIC *fast, double *leading, int *waslead, INPtables *tab)
	/* line - the line to parse */
	/* ckt  - the circuit this device is a member of */
	/* dev  - the device type code to the device being parsed */
	/* fast - direct pointer to device being parsed */
	/* leading - the optional leading numeric parameter */
	/* waslead - flag -1 if leading double given, 0 otherwise */
{
    int error; /* int to store evaluate error return codes in */
    char *parm;
    char *errbuf;
    int i;
    IFvalue *val;
    IFdevice *device;

    /* check for leading value */
    *waslead = 0;
    *leading = INPevaluate(line, &error, 1);
    if (error == 0) *waslead = 1; /* found a good leading number */
    else *leading = 0.0;

    while (**line) {
	INPgetTok(line, &parm, 1);
	if (!*parm) continue;
	device = ft_sim->devices[dev];
	for (i = 0; i < *device->numInstanceParms; i++) {
	    if (eq(parm, device->instanceParms[i].keyword)) {
		val = INPgetValue(ckt, line, device->instanceParms[i].dataType, tab);
		if (!val) return (INPerror(E_PARMVAL));
		error = (*(ft_sim->setInstanceParm))(ckt, fast,
			device->instanceParms[i].id, val, (IFvalue*)NULL);
		if (error) return(INPerror(error));
		break;
	    }
	}
	if (i == *device->numInstanceParms) {
	    errbuf = MALLOC(strlen(parm)+25);
	    sprintf(errbuf, "unknown parameter (%s)\n", parm);
	    return(errbuf);
	}
	FREE(parm);
    }
    return((char *)NULL);
}
