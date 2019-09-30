/**********
Copyright 1990 Regents of the University of California. All rights reserved.
**********/

/*
 * String functions
 */

#include "spice.h"
#include <stdio.h>
#include "misc.h"

int prefix (register char *p, register char *s)
{
    while (*p && *p == *s) p++, s++;
    if (!*p) return (true);
    return (false);
}

/* Create a copy of a string. */

char * copy (char *str)
{
    char *p;
    if ((p = tmalloc(strlen(str) + 1))) strcpy(p, str);
    return(p);
}

/* Determine whether sub is a substring of str. */
/* Like strstr() XXX */

int substring (register char *sub, register char *str)
{
    char *s, *t;

    while (*str) {
        if (*str == *sub) {
	    t = str;
            for (s = sub; *s; s++) {
                if (!*t || (*s != *t++)) break;
            }
            if (!*s) return (true);
        }
        str++;
    }
    return (false);
}

/* Try to identify an integer that begins a string. Stop when a non-
 * numeric character is reached.
 */
/* Like atoi() XXX */

int scannum (char *s)
{
    int i = 0;

    while (isdigit(*s)) { i = i * 10 + (*s - '0'); s++; }
    return(i);
}

/* Case insensitive str eq. */
/* Like strcasecmp() XXX */

int cieq (register char *p, register char *s)
{
    while (*p) {
        if ((isupper(*p) ? tolower(*p) : *p) !=
            (isupper(*s) ? tolower(*s) : *s))
            return(false);
        p++;
        s++;
    }
    return (*s ? false : true);
}

/* Case insensitive prefix. */

int ciprefix (register char *p, register char *s)
{
    while (*p) {
        if ((isupper(*p) ? tolower(*p) : *p) !=
            (isupper(*s) ? tolower(*s) : *s))
            return(false);
        p++;
        s++;
    }
    return (true);
}

void strtolower (register char *str)
{
    if (str)
	while (*str) {
	    *str = tolower(*str);
	    str++;
	}
}

char * gettok (char **s)
{
    char buf[BSIZE_SP];
    int c;
    int i = 0;
    int paren = 0;

    while (isspace(**s)) (*s)++;
    if (!**s) return (NULL);
    while ((c = **s) && !isspace(c)) {
	     if (c == '(') ++paren;
	else if (c == ')') --paren;
	else if (c == ',' && paren < 1) break;
        buf[i++] = c; (*s)++;
    }
    buf[i] = '\0';
    while (isspace(c) || c == ',') { (*s)++; c = **s; }
    return (copy(buf));
}
