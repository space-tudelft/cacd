/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * CKTnewNode(ckt,node,name)
 *  Allocate a new circuit equation number (returned) in the specified
 *  circuit to contain a new equation or node
 * returns -1 for failure to allocate a node number
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "util.h"

/* should just call CKTnewEQ and set node type afterwards */

int CKTnewNode(GENERIC *inCkt, GENERIC **node, IFuid name)
{
    register CKTcircuit *ckt = (CKTcircuit *)inCkt;
    register CKTnode *n;

    if (!ckt->CKTnodes) { /* starting the list - allocate both ground and 1 */
	n = (CKTnode *) MALLOC(sizeof(CKTnode));
	if (!n) return(E_NOMEM);
	n->name = (char *)NULL;
	n->number = 0;
	n->type = SP_VOLTAGE;
	ckt->CKTnodes = n;
	ckt->CKTlastNode = n;
    }
    n = (CKTnode *) MALLOC(sizeof(CKTnode));
    if (!n) return(E_NOMEM);
    n->name = name;
    n->number = ckt->CKTmaxEqNum++;
    n->type = SP_VOLTAGE;
    n->next = (CKTnode *)NULL;
    ckt->CKTlastNode->next = n;
    ckt->CKTlastNode = n;

    if (node) *node = (GENERIC *)n;
    return(OK);
}
