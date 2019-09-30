/**********
Copyright 1992 Regents of the University of California.  All rights reserved.
**********/

/* CKTdltNod
*/

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "sperror.h"
#include "util.h"
#include "misc.h"

int CKTdltNod(GENERIC *ckt, GENERIC *node)
{
    return CKTdltNNum(ckt, ((CKTnode *) node)->number);
}

int CKTdltNNum(GENERIC *cktp, int num)
{
    CKTcircuit *ckt = (CKTcircuit *) cktp;
    CKTnode *n, *prev, *node, *sprev;
    int	error;

    sprev = prev = NULL;
    node = NULL;
    for (n = ckt->CKTnodes; n; n = n->next) {
	if (n->number == num) {
	    node = n;
	    sprev = prev;
	}
	prev = n;
    }

    if (!node) return OK;

    ckt->CKTmaxEqNum -= 1;

    if (!sprev) {
	ckt->CKTnodes = node->next;
    } else {
	sprev->next = node->next;
    }
    if (node == ckt->CKTlastNode)
	ckt->CKTlastNode = sprev;

    error = (*(SPfrontEnd->IFdelUid))((GENERIC *)ckt, node->name, UID_SIGNAL);
    txfree(node);

    return error;
}
