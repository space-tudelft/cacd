/**********
Copyright 1990 Regents of the University of California. All rights reserved.
**********/

/*
 * A more portable version of the standard "mktemp()" function
 */

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include <unistd.h> /* getpid */

char * smktemp (char *id)
{
    char rbuf[80];
    char *nbuf;

    if (!id) id = "sp";
    sprintf(rbuf, "%s/%s%d", TEMPDIR, id, (int)getpid());
#if 0
    nbuf = (char *) malloc(strlen(rbuf) + 1);
    strcpy(nbuf, rbuf);
#else
    nbuf = copy (rbuf);
#endif
    return nbuf;
}
