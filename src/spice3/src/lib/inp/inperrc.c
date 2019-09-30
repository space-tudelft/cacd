/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include <string.h>
#include "inpdefs.h"
#include "util.h"

char * INPerrCat(char *a, char *b)
{
    if (a && *a) {
	if (b) {
	    char *ab;
	    int addnl = 0;
	    int len = strlen(a);

	    if (a[len-1] != '\n') ++addnl;

	    ab = MALLOC(len + strlen(b) + 1 + addnl);
	    if (ab) {
		strcpy(ab, a);
		if (addnl) ab[len++] = '\n';
		strcpy(ab + len, b);
	    }
	    FREE(a);
	    FREE(b);
	    return(ab);
	}
	return(a);
    }
    return(b);
}
