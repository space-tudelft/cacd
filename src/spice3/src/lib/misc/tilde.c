/**********
Copyright 1991 Regents of the University of California. All rights reserved.
**********/

#include "spice.h"
#include "misc.h"
#include <stdio.h>

char * tilde_expand (char *string)
{
    char buf[BSIZE_SP], *k, *s;

    if (!string) return string;
    s = string;
    if (*s != '~') return string;
    s++;
    if (*s && *s != '/') return string;

    if (!(k = getenv ("HOME"))) return string;

    if (*s) {
	strcpy(buf, k);
	strcat(buf, s);
	k = buf;
    }
    return copy(k);
}
