/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <ctype.h>
#include "inpdefs.h"

void INPcaseFix(register char *string)
{
    while (*string) {
        if (isupper(*string)) *string = tolower(*string);
        string++;
    }
}
