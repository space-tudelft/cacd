/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Various things for quoting words. If this is not ascii, quote and
 * strip are no-ops, so '' and \ quoting won't work.
 */

#include "spice.h"
#include "cpdefs.h"

/* Strip all the 8th bits from a string (destructively). */

void cp_wstrip(char *string)
{
    int c, d;
    if (string)
        while ((c = *string)) {
            if ((d = strip(c)) != c) *string = d;
            string++;
        }
}

void cp_striplist(wordlist *wl)
{
    for (; wl; wl = wl->wl_next) cp_wstrip(wl->wl_word);
}

/* Remove the "" from a string 's'. */

char *cp_unquote (char *s)
{
    int l;

    if (s) {
	if (*s == '"') s++;
	s = copy(s);
	l = strlen(s);
	if (l && s[l-1] == '"') s[l-1] = '\0';
    }
    return (s);
}

char *cp_unquote2 (char *s)
{
    int l;

    if (s) {
	if (*s == '"') s++;
	l = strlen(s);
	if (l && s[l-1] == '"') s[l-1] = '\0';
    }
    return (s);
}
