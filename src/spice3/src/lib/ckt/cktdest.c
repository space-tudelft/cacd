/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* CKTdestroy(ckt)
 * this is a driver program to iterate through all the various
 * destroy functions provided for the circuit elements in the
 * given circuit
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "devdefs.h"
#include "util.h"
#include "ifsim.h"
#include "sperror.h"

extern SPICEdev *DEVices[];

int CKTdestroy(GENERIC *inCkt)
{
    register CKTcircuit *ckt = (CKTcircuit *)inCkt;
    register int i;
    register CKTnode *node;
    register CKTnode *nnode;

#ifdef HAS_SENSE2
    if (ckt->CKTsenInfo) {
         if (ckt->CKTrhsOp)   FREE(ckt->CKTrhsOp);
         if (ckt->CKTsenRhs)  FREE(ckt->CKTsenRhs);
         if (ckt->CKTseniRhs) FREE(ckt->CKTseniRhs);
         SENdestroy(ckt->CKTsenInfo);
    }
#endif

    for (i = 0; i < DEVmaxnum; i++) {
        if (DEVices[i]->DEVdestroy && ckt->CKThead[i]) {
            (*(DEVices[i]->DEVdestroy))(&ckt->CKThead[i]);
        }
    }
    for (i = 0; i <= ckt->CKTmaxOrder+1; i++) {
        FREE(ckt->CKTstates[i]);
    }
    if (ckt->CKTmatrix) SMPdestroy(ckt->CKTmatrix);
    if (ckt->CKTbreaks) FREE(ckt->CKTbreaks);
    for (node = ckt->CKTnodes; node; node = nnode) {
        nnode = node->next;
        FREE(node);
    }
    ckt->CKTnodes = (CKTnode *)NULL;
    ckt->CKTlastNode = (CKTnode *)NULL;
    FREE(ckt);
    return(OK);
}
