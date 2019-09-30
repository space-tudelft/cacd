/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * CKTmkNode
 *  Tentatively allocate a new circuit equation structure
 */

#include "spice.h"
#include <stdio.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "smpdefs.h"
#include "cktdefs.h"
#include "util.h"

int CKTmkNode(CKTcircuit *ckt, CKTnode **node)
{
    CKTnode *mynode;

    mynode = (CKTnode *)MALLOC(sizeof(CKTnode));
    if (!mynode) return(E_NOMEM);
    mynode->next = (CKTnode *)NULL;
    mynode->name = (IFuid) 0;

    if (node) *node = mynode;
    return(OK);
}
