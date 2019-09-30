/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include <string.h>
#include "inpdefs.h"
#include "util.h"

char * INPmkTemp (char *string)
{
    char *temp = MALLOC(strlen(string) + 1);
    if (temp) strcpy(temp, string);
    return(temp);
}
