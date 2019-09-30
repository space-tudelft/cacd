
/*
 * ISC License
 *
 * Copyright (C) 1997-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "src/xspf/incl.h"

/* nextAttr reads attribute string a,
   puts in p the next parameter, in v the next
   value, and returns the rest of the attribute
   string 'a' (uses the part of 'a' that has
   been read !)
	if (v) then *(v-1) == '='
	if (a) then *(a-1) == ';'
*/
char *nextAttr (char **p, char **v, char *a)
{
    *v = NULL;
    if (!a) { *p = NULL; return (NULL); }
    while (*a == ';') ++a;
    if (!*a) { *p = NULL; return (NULL); }
    *p = a;
    if (*a != '=') while (*++a && ((*a != ';' && *a != '=') || *(a-1) == '\\'));
    if (*a == '=') {
	*a++ = '\0';
	*v = a;
	while (*a && (*a != ';' || *(a-1) == '\\')) ++a;
    }
    if (*a) *a++ = '\0';
    return (*a ? a : NULL);
}

/* getAttrValue reads the attribute string, searches for parameter par,
   if this par is followed by a '=', a ptr to this value is returned,
   or a null string is returned (if par is found).
   if par is not found, NULL is returned!
*/
char *getAttrValue (char *a, char *par)
{
    char *p;

    if (!a || !par) return (NULL);

    while (*a) {
	while (*a == ';') ++a;
	for (p = par; *a && *p == *a; ++p) ++a;
	if (!*p) {
	    if (*a == '=') p = ++a;
	    if (*a == ';') return ("");
	    if (p == a || !*a) return (a);
	}
	while (*a && (*a != ';' || *(a-1) == '\\')) ++a;
    }
    return (NULL);
}

/* getAttrFValues reads the attribute string, searches for parameters
   with a name "par*" and adds all the values that occur for these
   parameters.   If no matching parameters are found 0.0 is returned.
*/
double getAttrFValues (char *a, char *par)
{
    char *p, *b;
    char buf[128];
    double val = 0.0;

    if (!a || !par) return (val);

    while (*a) {
	while (*a == ';') ++a;
	for (p = par; *a && *p == *a; ++p) ++a;
	if (!*p) {
            /* matched */
            while (*a && *a != '=' && *a != ';') a++;
	    if (*a == '=') {
                a++;
                b = &buf[0];
                while (*a && *a != ';' && b - buf < 128) *b++ = *a++;
                *b = '\0';
                val += atof (buf);
            }
	}
	while (*a && (*a != ';' || *(a-1) == '\\')) ++a;
    }
    return (val);
}
