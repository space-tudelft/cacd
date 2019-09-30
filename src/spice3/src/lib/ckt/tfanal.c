/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Thomas L. Quarles
**********/

/* subroutine to do DC Transfer Function analysis */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "util.h"
#include "sperror.h"
#include "smpdefs.h"
#include "tfdefs.h"

int TFanal(CKTcircuit *ckt, int restart) /* forced restart flag */
{
    int size;
    int insrc, outsrc;
    double outputs[3];
    IFvalue outdata; /* structure for output data vector, will point to outputs vector above */
    IFvalue refval;  /* structure for 'reference' value (not used here) */
    int error;
    int converged;
    register int i;
    GENERIC *plotptr;   /* pointer to out plot */
    GENERIC *ptr = NULL;
    IFuid uids[3];
#define tfuid  (uids[0]) /* unique id for the transfer function output */
#define inuid  (uids[1]) /* unique id for the transfer function input imp. */
#define outuid (uids[2]) /* unique id for the transfer function output imp. */
    int type;
    char *name;
    TFan *job = (TFan*) ckt->CKTcurJob;

    /* first, find the operating point */
    converged = CKTop(ckt,
            (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITJCT,
            (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITFLOAT, ckt->CKTdcMaxIter);

#ifdef notdef
    /* don't need this any more since newconvtest leaves the matrix factored */
    ckt->CKTmode = (ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITFLOAT;
    error = CKTload(ckt);
    if (error) return(error);

    error = SMPluFac(ckt->CKTmatrix, ckt->CKTpivotAbsTol, ckt->CKTdiagGmin);
    if (error) return(error);
#endif /* notdef */

    type = CKTtypelook("Isource");
    if (type != -1) {
        error = CKTfndDev((GENERIC*)ckt, &type, &ptr, job->TFinSrc, (GENERIC *)NULL, (IFuid)NULL);
        if (error == 0) {
            job->TFinIsI = 1;
            job->TFinIsV = 0;
        } else {
            ptr = NULL;
        }
    }

    type = CKTtypelook("Vsource");
    if (type != -1 && !ptr) {
        error = CKTfndDev((GENERIC*)ckt, &type, &ptr, job->TFinSrc, (GENERIC *)NULL, (IFuid)NULL);
        job->TFinIsV = 1;
        job->TFinIsI = 0;
        if (error) {
            (*(SPfrontEnd->IFerror))(ERR_WARNING, "Transfer function source %s not in circuit", &job->TFinSrc);
            job->TFinIsV = 0;
            return(E_NOTFOUND);
        }
    }

    size = SMPmatSize(ckt->CKTmatrix);
    for (i = 0; i <= size; i++) {
        ckt->CKTrhs[i] = 0;
    }

    if (job->TFinIsI) {
        ckt->CKTrhs[((GENinstance*)ptr)->GENnode1] -= 1;
        ckt->CKTrhs[((GENinstance*)ptr)->GENnode2] += 1;
        insrc = 0; /* prevent compiler warning */
    } else {
        insrc = CKTfndBranch(ckt, job->TFinSrc);
        ckt->CKTrhs[insrc] += 1;
    }

    SMPsolve(ckt->CKTmatrix, ckt->CKTrhs, ckt->CKTrhsSpare);
    ckt->CKTrhs[0] = 0;

    /* make a UID for the transfer function output */
    (*(SPfrontEnd->IFnewUid))(ckt, &tfuid, (IFuid)NULL, "Transfer_function",
            UID_OTHER, (GENERIC **)NULL);

    /* make a UID for the input impedance */
    (*(SPfrontEnd->IFnewUid))(ckt, &inuid, job->TFinSrc,
            "Input_impedance", UID_OTHER, (GENERIC **)NULL);

    /* make a UID for the output impedance */
    if (job->TFoutIsI) {
        (*(SPfrontEnd->IFnewUid))(ckt, &outuid, job->TFoutSrc,
		"Output_impedance", UID_OTHER, (GENERIC **)NULL);
    } else {
        name = MALLOC(strlen(job->TFoutName) + 22);
        sprintf(name, "output_impedance_at_%s", job->TFoutName);
        (*(SPfrontEnd->IFnewUid))(ckt, &outuid, (IFuid)NULL,
                name, UID_OTHER, (GENERIC **)NULL);
    }

    error = (*(SPfrontEnd->OUTpBeginPlot))(ckt, (GENERIC *)(ckt->CKTcurJob),
		job->JOBname, (IFuid)NULL, (int)0, 3, uids, IF_REAL, &plotptr);
    if (error) return(error);

    /*find transfer function */
    if (job->TFoutIsV) {
	outputs[0] = ckt->CKTrhs[job->TFoutPos->number] -
		     ckt->CKTrhs[job->TFoutNeg->number];
	outsrc = 0; /* prevent compiler warning */
    } else {
        outsrc = CKTfndBranch(ckt, job->TFoutSrc);
        outputs[0] = ckt->CKTrhs[outsrc];
    }

    /* now for input resistance */
    if (job->TFinIsI) {
	outputs[1] = ckt->CKTrhs[((GENinstance*)ptr)->GENnode2] -
		     ckt->CKTrhs[((GENinstance*)ptr)->GENnode1];
    } else {
        if (FABS(ckt->CKTrhs[insrc]) < 1e-20) {
            outputs[1] = 1e20;
        } else {
            outputs[1] = -1/ckt->CKTrhs[insrc];
        }
    }

    if (job->TFoutIsI && job->TFoutSrc == job->TFinSrc) {
        outputs[2] = outputs[1];
        goto done;
        /* no need to compute output resistance when it is the same as the input */
    }
    /* now for output resistance */
    for (i = 0; i <= size; i++) {
        ckt->CKTrhs[i] = 0;
    }
    if (job->TFoutIsV) {
        ckt->CKTrhs[job->TFoutPos->number] -= 1;
        ckt->CKTrhs[job->TFoutNeg->number] += 1;
    } else {
        ckt->CKTrhs[outsrc] += 1;
    }
    SMPsolve(ckt->CKTmatrix, ckt->CKTrhs, ckt->CKTrhsSpare);
    ckt->CKTrhs[0] = 0;
    if (job->TFoutIsV) {
	outputs[2] = ckt->CKTrhs[job->TFoutNeg->number] -
		     ckt->CKTrhs[job->TFoutPos->number];
    } else {
        outputs[2] = 1/MAX(1e-20, ckt->CKTrhs[outsrc]);
    }
done:
    outdata.v.numValue = 3;
    outdata.v.vec.rVec = outputs;
    refval.rValue = 0;
    (*(SPfrontEnd->OUTpData))(plotptr, &refval, &outdata);
    (*(SPfrontEnd->OUTendPlot))(plotptr);
    return(OK);
}
