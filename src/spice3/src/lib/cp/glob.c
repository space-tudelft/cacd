/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Expand global characters.
 */

#include "spice.h"
#include "cpdefs.h"

#include <sys/types.h>
#include <dirent.h>
#ifndef direct
#define direct dirent
#endif

/* For each word, go through two steps: expand the {}'s, and then do ?*[]
 * globbing in them. Sort after the second phase but not the first...
 */

/* MW. Now only tilde is supportef, {}*? don't work */

wordlist * cp_doglob (wordlist *wlist)
{
    wordlist *wl;
    char *s;

    /* Do tilde expansion. */

    for (wl = wlist; wl; wl = wl->wl_next)
	if ((s = wl->wl_word) && *s == '~') {
	    wl->wl_word = tilde_expand(s);
	    if (wl->wl_word != s) free(s);
	}
    return (wlist);
}

/* Say whether the pattern p can match the string s. */

/* MW. Now simply compare strings */

bool cp_globmatch (char *p, char *s)
{
    return(!(strcmp(p, s)));
}

