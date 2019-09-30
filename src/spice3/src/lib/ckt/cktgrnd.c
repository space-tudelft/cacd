/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* CKTground(ckt,node)
 *  specify the node to be the ground node of the given circuit
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "sperror.h"
#include "util.h"

int CKTground(GENERIC *inCkt, GENERIC **node, IFuid name)
{
    register CKTcircuit *ckt = (CKTcircuit *)inCkt;

    if (ckt->CKTnodes) {
        if (ckt->CKTnodes->name) {
            /*already exists - keep old name, but return it */
            if (node) *node = (char *)ckt->CKTnodes;
            return(E_EXISTS);
        }
    } else {
        ckt->CKTnodes = (CKTnode *)MALLOC(sizeof(CKTnode));
        if (!ckt->CKTnodes) return(E_NOMEM);
        ckt->CKTnodes->next = (CKTnode *)NULL;
        ckt->CKTlastNode = ckt->CKTnodes;
    }
    ckt->CKTnodes->name = name;
    ckt->CKTnodes->type = SP_VOLTAGE;
    ckt->CKTnodes->number = 0;

    if (node) *node = (char *)ckt->CKTnodes;
    return(OK);
}
