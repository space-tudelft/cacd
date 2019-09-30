/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* get input token from 'line',
 *  and return a pointer to it in 'token'
 */

/* INPgetTok: node names
   INPgetUTok: numbers and other elements in expressions (called from INPevaluate)
 */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "util.h"
#include "iferrmsg.h"
#include "inpdefs.h"

/* if gobble, eat non-whitespace trash AFTER token */

int INPgetTok(char **line, char **token, int gobble)
{
    char *s, *b, *t;
    int len;

    /* throwing away garbage characters */
    for (s = *line; *s; s++) {
	if (*s == ' ' || *s == '\t' || *s == '=' || *s == '(' || *s == ')' || *s == ',') continue;
	break;
    }
    b = s; /* mark beginning of token */

    /* now find all good characters */
    for ( ; *s; s++) {
	if (*s == ' ' || *s == '\t' || *s == '=' || *s == '(' || *s == ')' || *s == ',') break;
    }
    len = s - b;

    *token = t = MALLOC(len + 1);
    if (t) { strncpy(t, b, len); t[len] = '\0'; }

    /* gobble garbage to next token */
    for ( ; *s; s++) {
	if (*s == ' ' || *s == '\t') continue;
	if (gobble) {
	    if (*s == '=' || *s == ',') continue;
#ifdef notdef
	/* This is the wrong thing to do for expression-valued parameters.
	The parens will get taken out at the beginning, leave them here for
	parse trees */
	    if (*s == '(' || *s == ')') continue;
#endif
	}
	break;
    }
    *line = s;
    /*printf("found token (%s) and rest of line (%s)\n", *token, *line);*/
    return(t ? OK : E_NOMEM);
}

int INPgetUTok(char **line, char **token, int gobble)
{
    char *b, *s, *t, separator;
    int signstate;
    int len;

    /* throwing away garbage characters */
    for (s = *line; *s; s++) {
	if (*s == ' ' || *s == '\t' || *s == '=' || *s == '(' || *s == ')' || *s == ',') continue;
	break;
    }
    if (*s == '"' || *s == '\'') separator = *s++;
    else separator = 0;

    b = s; /* mark beginning of token */

    /* now find all good characters */
    signstate = 0;
    for ( ; *s; s++) {
	if (separator) {
	    if (*s == separator) break;
	    continue;
	}
	if (*s == ' ' || *s == '\t' || *s == '=' || *s == '(' || *s == ')' || *s == ',') break;

	/* This is not complex enough to catch all errors, but it will
	   get the "good" parses */
	if (*s == '+' && (signstate == 1 || signstate == 3)) break;
	if (*s == '-') {
	    if (signstate == 1 || signstate == 3) break;
	    signstate += 1;
	    continue;
	}
	if (*s == '*' || *s == '/' || *s == '^') break;

	if (isdigit(*s) || *s == '.')
	    signstate = signstate > 1 ? 3 : 1;
	else if (tolower(*s) == 'e' && signstate == 1)
	    signstate = 2;
	else
	    signstate = 3;
    }

    if (separator && *s == separator) s--;

    if (s == b && *s) s++; /* Weird items, 1 char */
    len = s - b;

    *token = t = MALLOC(len + 1);
    if (t) { strncpy(t, b, len); t[len] = '\0'; }

    /* gobble garbage to next token */
    for ( ; *s; s++) {
	if (*s == separator || *s == ' ' || *s == '\t') continue;
        if (gobble && (*s == '=' || *s == ',')) continue;
        break;
    }
    *line = s;
    /*printf("found token (%s) and rest of line (%s)\n", *token, *line);*/
    return(t ? OK : E_NOMEM);
}
