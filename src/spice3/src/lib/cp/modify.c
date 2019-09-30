/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include "cpdefs.h"

/* Initialize stuff. */

void cp_init()
{
    cp_vset("history", VT_NUM, (char *) &cp_maxhistlength);

    cp_curin  = stdin;
    cp_curout = stdout;
    cp_curerr = stderr;

    cp_ioreset();
}
