/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/libddm/dmstd.h"

static char *projbuf[DM_MAXPROJECTS];

char *dmSubstituteEnvironmentVars (char *path);
char *_dmExtractEnvironentVariable (char **str);

char **_dmProjectlist (DM_PROJECT *dmproject)
{
    FILE * fp;
    char    path[MAXLINE];
    int     i = 0;

    _dmSprintf (path, "%s/projlist", dmproject -> dmpath);

    (void) dmSubstituteEnvironmentVars (path);

    if (!(fp = fopen (path, "r"))) {
	dmerrno = DME_NOPRJL;
	dmError ("_dmProjectlist: no projlist");
	return (NULL);
    }

    while (fscanf (fp, "%s", path) != EOF) {
	if (*path == '#') continue; /* skip comment lines! */
	projbuf[i++] = _dmStrSave (path);
	if (i == DM_MAXPROJECTS)
	    _dmFatal ("_dmProjectlist: too many projects", "", "");
    }
    projbuf[i] = NULL;

#ifdef DM_DEBUG
    IFDEBUG {
	fprintf (stderr, "_dmProjectlist: ");
	for (i = 0; projbuf[i] != NULL; i++)
	    fprintf (stderr, "%s%s", projbuf[i], projbuf[i + 1] ? ", " : "\n");
    }
#endif /* DM_DEBUG */

    fclose (fp);
    return (projbuf);
}

/* This function substitutes all occurrences of $SOMETHING in PATH by the value
 * of the environment variable SOMETHING. It is an error if SOMETHING is not
 * set in the environment. The expanded PATH is returned as a pointer to a
 * static character array. It will be overwritten by the next call to
 * dmSubstituteEnvironmentVars().
 */
char *dmSubstituteEnvironmentVars (char *path)
{
   static char epath[1 + MAXLINE];
   int c, i = 0;
   char *p = path;
   if (path == NULL) return NULL; /* nothing to do */
   while ((c = *p++)) {
      if (c != '$') {
	 epath[i++] = c; /* no special char, copy it to epath */
      }
      else if (*p == '$') { /* the special construct "$$" is just replaced by a singe '$': */
	 epath[i++] = c;    /* copy a single '$', then proceed normally */
	 p++;
      }
      else { /* OK, found a real environment var... which one is it? */
	 char *env = _dmExtractEnvironentVariable (&p);
	 char *value_of_env = getenv(env);
	 if (value_of_env == NULL) {
	    char s[MAXLINE];
	    sprintf (s, "_dmProjectlist: The environment variable \"%s\" is not set!", env);
	    dmError (s);
	 }
	 while ((c = *value_of_env++)) { /* copy env var to epath */
	    epath[i++] = c;
	 }
      }
   }
   epath[i] = '\0';
   return epath;
}

/* Extract the environment variable from *str and return it. Leave str pointing
 * just past the end of the environment variable name. Env. names may contain
 * alphas, digits and underscores. They may be enclosed in curly braces {}, in
 * which case these braces are not returned as part of the env. name.
 *
 * Examples:
 * str = "SOMETHING/seadif" returns "SOMETHING" and leaves str pointing
 *       to "/seadif".
 *
 * str = "{SOMETHING}/seadif" also returns "SOMETHING" and also leaves str
 *       pointing to "/seadif".
 */
char *_dmExtractEnvironentVariable (char **str)
{
   static char name[MAXLINE];
   char *p = *str;
   int c, i = 0;
   int inCurlyBraces = (*p == '{');
   if (inCurlyBraces) p++;
   while ((c = *p) != '\0') {
      if (inCurlyBraces && c == '}') {
	 p++;
	 break;
      }
      if (!inCurlyBraces && !isalnum (c) && c != '_') break;
      name[i++] = c;
      p++;
   }
   name[i] = '\0';
   *str = p;
   return name;
}
