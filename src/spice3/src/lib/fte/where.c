/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftehelp.h"
#include "hlpdefs.h"

void com_where()
{
    char *msg;

    if (ft_curckt) {
	msg = (*ft_sim->nonconvErr)((GENERIC *) (ft_curckt->ci_ckt), 0);
	fprintf(cp_out, "%s", msg);
    } else {
	fprintf(cp_err, "Error: no circuit loaded.\n");
    }
}
