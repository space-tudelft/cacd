/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "tskdefs.h"
#include "jobdefs.h"
#include "ifsim.h"
#include "util.h"
#include "iferrmsg.h"

int CKTdelTask(GENERIC *ckt, GENERIC *task)
{
    JOB *job, *old;

    for (job = ((TSKtask*)task)->jobs; job; ) {
	old = job;
	job = job->JOBnextJob;
	FREE(old);
    }
    FREE(task);
    return(OK);
}
