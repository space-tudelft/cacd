/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"
#include "ifsim.h"
#include "cpstd.h"
#include "fteext.h"

extern INPmodel *modtab;

char * INPgetMod(GENERIC *ckt, char *name, INPmodel **model, INPtables *tab)
{
    INPmodel *modtmp;
    IFvalue *val;
    IFdevice *device;
    register int j;
    char *line;
    char *parm;
    char *err = NULL;
    char *temp;
    int error;

    for (modtmp = modtab; modtmp; modtmp = modtmp->INPnextModel) {
	if (eq(modtmp->INPmodName, name)) {
	    /* found the model in question - now instantiate if necessary */
	    /* and return an appropriate pointer to it */
	    if (modtmp->INPmodType < 0) {
		/* illegal device type, so can't handle */
		*model = (INPmodel *)NULL;
		err = MALLOC(34 + strlen(name));
		sprintf(err, "Unknown device type for model %s\n", name);
		return(err);
	    }
	    if (!modtmp->INPmodUsed) {
		/* not already defined, so create & give parameters */
		error = (*(ft_sim->newModel))(ckt, modtmp->INPmodType,
			&modtmp->INPmodfast, modtmp->INPmodName);
		if (error) return(INPerror(error));
		    /* parameter isolation, identification, binding */
		line = modtmp->INPmodLine->line;
		INPgetTok(&line, &parm, 1); txfree(parm); /* throw away '.model' */
		INPgetTok(&line, &parm, 1); txfree(parm); /* throw away 'modname' */
		while (*line) {
		    INPgetTok(&line, &parm, 1);
		    if (*parm) {
			device = ft_sim->devices[modtmp->INPmodType];
			for (j = 0; j < *device->numModelParms; j++) {
			    if (eq(parm, device->modelParms[j].keyword)) {
				val = INPgetValue(ckt, &line,
					device->modelParms[j].dataType, tab);
				error = (*(ft_sim->setModelParm))(ckt, modtmp->INPmodfast,
					device->modelParms[j].id, val, (IFvalue*)NULL);
				if (error) return(INPerror(error));
				break;
			    }
			}
			if (eq(parm, "level")) {
			    /* just grab the level number and throw away */
			    /* since we already have that info from pass1 */
			    val = INPgetValue(ckt, &line, IF_REAL, tab);
			} else if (j >= *device->numModelParms) {
			    temp = MALLOC(40 + strlen(parm));
			    sprintf(temp, "unrecognized parameter (%s) - ignored\n", parm);
			    err = INPerrCat(err, temp);
			}
		    }
		    txfree(parm);
		}
		modtmp->INPmodUsed = 1;
		modtmp->INPmodLine->error = err;
	    }
	    *model = modtmp;
	    return(NULL);
	}
    }
    /* didn't find model - ERROR - return model */
    *model = (INPmodel *)NULL;
    err = MALLOC(56 + strlen(name));
    sprintf(err, "unable to find definition of model %s - default assumed\n", name);
    return(err);
}
