/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/*
 * provide the error message appropriate for the given error code
 */

#include "spice.h"
#include <stdio.h>
#include <string.h>
#include "ifsim.h"
#include "iferrmsg.h"
#include "sperror.h"
#include "util.h"

char * INPerror(int code)
{
    char *val;
    char ebuf[513];

    val = SPerror(code);

    if (!val) return(val);

    if (errRtn)
	sprintf(ebuf, "%s detected in routine \"%s\"\n", val, errRtn);
    else
	sprintf(ebuf, "%s\n", val);

    val = MALLOC(strlen(ebuf) + 1);
    if (val) strcpy(val, ebuf);

    return(val);
}
