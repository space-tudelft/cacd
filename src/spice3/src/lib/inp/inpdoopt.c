/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* INPdoOpts(ckt,option card)
 *  parse the options off of the given option card and add them to
 *  the given circuit
 */

#include "spice.h"
#include <stdio.h>
#include "inpdefs.h"
#include "ifsim.h"
#include "util.h"
#include "cpdefs.h"
#include "fteext.h"

void INPdoOpts (GENERIC *ckt, GENERIC *anal, card *optCard, INPtables *tab)
{
    char *line;
    char *token;
    char *errmsg;
    IFvalue *val;
    int error;
    int i;
    IFanalysis *prm = NULL;

    for (i = 0; i < ft_sim->numAnalyses; i++) {
        prm = ft_sim->analyses[i];
        if (eq(prm->name, "options")) break;
    }
    if (i >= ft_sim->numAnalyses) {
        optCard->error = INPerrCat(optCard->error, INPmkTemp("Error: analysis options table not found\n"));
        return;
    }

    line = optCard->line;
    INPgetTok(&line, &token, 1); txfree(token); /* throw away '.option' */
    while (*line) {
        INPgetTok(&line, &token, 1);
        for (i = 0; i < prm->numParms; i++) {
            if (eq(token, prm->analysisParms[i].keyword)) {
                if (!(prm->analysisParms[i].dataType & IF_UNIMP_MASK)) {
                    errmsg = MALLOC(44 + strlen(token));
                    sprintf(errmsg, "Warning: %s not yet implemented (ignored)\n", token);
                    optCard->error = INPerrCat(optCard->error, errmsg);
                    val = INPgetValue(ckt, &line, prm->analysisParms[i].dataType, tab);
                    break;
                }
                if (prm->analysisParms[i].dataType & IF_SET) {
                    val = INPgetValue(ckt, &line,
                            prm->analysisParms[i].dataType&IF_VARTYPES, tab);
                    error = (*(ft_sim->setAnalysisParm))(ckt, anal,
                            prm->analysisParms[i].id, val, (IFvalue*)NULL);
                    if (error) {
                        errmsg = MALLOC(30 + strlen(token));
                        sprintf(errmsg, "Warning: can't set option %s\n", token);
                        optCard->error = INPerrCat(optCard->error, errmsg);
                    }
                    break;
                }
            }
        }
        if (i >= prm->numParms) {
	    errmsg = MALLOC(35 + strlen(token));
            sprintf(errmsg, "Error: unknown option %s (ignored)\n", token);
            optCard->error = INPerrCat(optCard->error, errmsg);
        }
	txfree(token);
    }
}
