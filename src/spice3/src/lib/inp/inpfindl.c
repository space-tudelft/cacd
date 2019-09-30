/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
Modified: 2009 Simon de Graaf (ala Paolo Nenzi, 1999)
**********/

/* INPfindLev(line,level)
 *      find the 'level' parameter on the given line and return its
 *      value (1,2,..,99 for now, 1 default)
 *  NOTE: now limited to 9 for compatibility
 */

#include "spice.h"
#include "misc.h"
#include <stdio.h>
#include "inpdefs.h"
#include "util.h"

char * INPfindLev(char *line, int *level)
{
    char *where;

    where = line;

    while (1) {
        where = strchr(where, 'l');
        if (!where) { /* no 'l' in the line => no 'level' => default */
            *level = 1;
            return(NULL);
        }
        if (strncmp(where, "level", 5)) { /* 'l' not in the word 'level', try again */
            where++;    /* make sure we don't match same char again */
            continue;
        }
        /* found the word level, lets look at the rest of the line */
        where += 5;
        while (*where == ' ' || *where == '\t' || *where == '=' ||
               *where == ',' || *where == '(' || *where == ')' ||
               *where == '+') { /* legal white space - ignore */
            where++;
        }
        /* now the magic number */
        if (*where >= '1' && *where <= '9') {
            *level = *where - '0';
            where++;
            if (*where >= '0' && *where <= '9') {
                *level = (*level * 10) + (*where - '0');
            }
            if (*level >= 1 && *level <= 9) return(NULL);
        }
        *level = 1;
        return(INPmkTemp("illegal argument to level parameter - level=1 assumed"));
    }
}
