/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* CKTask
 *  Ask questions about a specified device.
 */

#include "spice.h"
#include <stdio.h>
#include "cktdefs.h"
#include "ifsim.h"
#include "devdefs.h"
#include "sperror.h"
#include "util.h"

extern SPICEdev *DEVices[];

int CKTask(GENERIC *ckt, GENERIC *fast, int which, IFvalue *value, IFvalue *selector)
{
    int type = ((GENinstance *)fast)->GENmodPtr->GENmodType;
    if (DEVices[type]->DEVask) {
        return((*(DEVices[type]->DEVask)) ((CKTcircuit *)ckt,
                (GENinstance *)fast, which, value, selector));
    }
    return(E_BADPARM);
}
