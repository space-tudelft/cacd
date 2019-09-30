/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Jaijeet S Roychowdhury
**********/

#include "spice.h"
#include "cktdefs.h"
#include "sperror.h"
#include "distodef.h"

int DkerProc(int type, double *rPtr, double *iPtr, int size, DISTOAN *job)
{
    int i;

    switch (type) {
    case D_F1:
    case D_F2:
    case D_TWOF1:
    case D_THRF1:
	for (i = 1; i <= size; i++) {
	    rPtr[i] *= 2.0;
	    iPtr[i] *= 2.0; /* convert to sinusoid amplitude */
	}
	break;

    case D_F1PF2:
    case D_F1MF2:
	for (i = 1; i <= size; i++) {
	    rPtr[i] *= 4.0;
	    iPtr[i] *= 4.0;
	}
	break;

    case D_2F1MF2:
	for (i = 1; i <= size; i++) {
	    rPtr[i] *= 6.0;
	    iPtr[i] *= 6.0;
	}
	break;

    default:
	return(E_BADPARM);
    }

    return(OK);
}
