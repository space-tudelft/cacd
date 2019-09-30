/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <math.h>
#include <ctype.h>
#include <stdarg.h>

#include "src/space/auxil/auxil.h"

/* The maximum number of parameters
 * to be handled.
 */
#define MAXPARAMETERS 250

static bool_t verbose_flag = 0;
static bool_t psaid[MAXPARAMETERS];
static bool_t osaid[MAXPARAMETERS];
static char *pkeys[MAXPARAMETERS], *pvals[MAXPARAMETERS];
static char *okeys[MAXPARAMETERS], *ovals[MAXPARAMETERS];
static int paramIndex = 0;
static int optionIndex = 0;

/* Report error for parameter <param>
 * Should be called as
 *    paramError (param, formatstring, arg1, ... );
 */
void paramError (char *param, const char *format, ...)
{
    va_list args;
    va_start (args, format);

    fprintf (stderr, "error for parameter '%s': ", param);
    vfprintf (stderr, format, args);
    fprintf (stderr, "\n");
    va_end (args);
}

/* turn on/off verbose mode */
void paramSetVerbose (int level)
{
    verbose_flag = level > 0 ? level : 0;
}

/*
 * read <paramfile>
 */
void paramReadFile (char *paramfile)
{
    int line, len;
    FILE *fp;
    char *key, *val, *s, *PreStr;
    char linebuf[200], keybuf[200];

#ifndef R_OK
    if (!(fp = fopen (paramfile, "r"))) {
	say ("parameter file %s is unreadable", paramfile);
	return;
    }
#else
    if (access (paramfile, R_OK) != 0) {
	say ("parameter file %s is unreadable", paramfile);
	return;
    }
    fp = cfopen (paramfile, "r");
#endif /* not R_OK */
    ASSERT (fp);

    Debug (fprintf (stderr, "reading %s\n", paramfile));

    /* clear all parameters that were read */
    while (paramIndex) {
	--paramIndex;
	DISPOSE (pkeys[paramIndex], strlen(pkeys[paramIndex]) + 1);
	DISPOSE (pvals[paramIndex], strlen(pvals[paramIndex]) + 1);
    }

    PreStr = 0;
    line = 0;

    while (fgets (linebuf, sizeof (linebuf) - 1, fp)) {
	line++;

	if ((s = strchr (linebuf, '#'))) *s = '\0';

	s = linebuf;
	while (isspace ((int)*s)) ++s;
	if (!*s) continue;
	key = s;

	do ++s; while (*s && !isspace ((int)*s));
	len = s - key;

	/* strip leading & trailing white space */
	if (*s) {
	    *s = '\0';
	    do ++s; while (isspace ((int)*s));
	}
	val = s;

	if (*s) {
	    while (*++s);
	    do --s; while (isspace ((int)*s));
	    *++s = '\0';
	}

	Debug (fprintf (stderr, "line %d: key <%s> val <%s>\n", line, key, val));

	if (len == 5) {
	    s = key;
	    if ((*key == 'b' || *s == 'B') &&
		(*++s == 'e' || *s == 'E') &&
		(*++s == 'g' || *s == 'G') &&
		(*++s == 'i' || *s == 'I') &&
		(*++s == 'n' || *s == 'N')) {

		s = val;
		while (*s && !isspace ((int)*s)) ++s;
		*s = '\0';

		if (PreStr) {
		    *PreStr = '\0';
		    say ("BEGIN %s at line %d while still in section %s",
			val, line, keybuf);
		    die ();
		}
		if (*val) {
		    strcpy (keybuf, val);
		    PreStr = keybuf + (s - val);
		    *PreStr = '.';
		}
		continue;
	    }
	}
	if (len == 3) {
	    s = key;
	    if ((*key == 'e' || *s == 'E') &&
		(*++s == 'n' || *s == 'N') &&
		(*++s == 'd' || *s == 'D')) {

		PreStr = 0;
		continue;
	    }
	}

	if (PreStr) {
	    strcpy (PreStr+1, key);
	    key = keybuf;
	}

	if (paramIndex == MAXPARAMETERS) {
	    paramError (key, "too many parameters");
	    break;
	}
	else { /* set parameter <key> to <value> */
	    pkeys[paramIndex] = strsave (key);
	    pvals[paramIndex] = strsave (val);
	    psaid[paramIndex++] = FALSE;
	}
    }

    fclose (fp);
}

/*
 * Set an option. Can be used to set parameters
 * from the command line, see the test driver.
 * <option> should be of the form "name" (boolean option)
 * or "name=value"
 * White space around name and value is stripped.
 */
void paramSetOption (char *option)
{
    char key[42], value[82], *s, *v;
    int i;

    if (!option) return;

    while (isspace ((int)*option)) ++option;
    s = option;
    while (*s && *s != '=') ++s;

    if ((v = s) == option) return; /* no key */

    i = 0;
    --s; while (isspace ((int)*s)) --s;
    ++s;
    if ((s - option) > 40) v = 0, s = option + 20;
    while (option < s) key[i++] = *option++;
    if (!v) {
	key[i++] = '*'; key[i] = '\0';
	paramError (key, "too long parameter");
	return;
    }
    key[i] = '\0';

    i = 0;
    if (*v == '=') {
	++v; while (isspace ((int)*v)) ++v;
	if (*(s = v)) {
	    while (*++s);
	    --s; while (isspace ((int)*s)) --s;
	    ++s;
	    if ((s - v) > 80) {
		paramError (key, "too long value");
		return;
	    }
	    while (v < s) value[i++] = *v++;
	}
    }
    value[i] = '\0';

    paramSetOption2 (key, value);
}

int paramGetOptionCount ()
{
    return optionIndex;
}

char *paramGetOptionKey (int Index)
{
    return Index < optionIndex ? okeys[Index] : "";
}

char *paramGetOptionValue (int Index)
{
    return Index < optionIndex ? ovals[Index] : "";
}

/*
 * Set an option. Can be used to set parameters
 * from the command line, see the test driver.
 * <option> should be of the form "name" (boolean option)
 * or "name=value"
 * White space around name and value is stripped.
 */
void paramSetOption2 (char *name, char *value)
{
    if (optionIndex == MAXPARAMETERS) {
	paramError (name, "too many parameters");
    }
    else {
	okeys[optionIndex] = strsave (name);
	ovals[optionIndex] = strsave (value);
	osaid[optionIndex++] = FALSE;
    }
}

char *paramLookup (const char *key)
{
    int i;

    /* search the tables backwards, so that last setting
     * in file overrules earlier settings.
     */
    for (i = optionIndex - 1; i >= 0; i--) {
	if (strsame (key, okeys[i])) { osaid[i] = TRUE; return (ovals[i]); }
    }
    for (i = paramIndex - 1; i >= 0; i--) {
	if (strsame (key, pkeys[i])) { psaid[i] = TRUE; return (pvals[i]); }
    }
    return (NULL);
}

/*
 * Lookup string value of <key>, but return <dflt> if <key> not found.
 */
char *paramLookupS (const char *key, const char *dflt)
{
    char *s = paramLookup (key);

    if (s) {
	if (verbose_flag) {
	    if (verbose_flag == 2) swapVerboseStreams ();
	    message ("parameter(s) %s = '%s' (set)", key, s);
	    if (verbose_flag == 2) swapVerboseStreams ();
	}
    } else {
	s = (char*)dflt;
	if (verbose_flag) {
	    if (verbose_flag == 2) swapVerboseStreams ();
	    if (s) message ("parameter(s) %s = '%s' (default)", key, s);
	    else   message ("parameter(s) %s = NULL (default)", key);
	    if (verbose_flag == 2) swapVerboseStreams ();
	}
    }
    return (s);
}

void printUnusedParams ()
{
    int i, j = 0;

    for (i = 0; i < optionIndex; ++i) {
	if (!osaid[i]) {
	    if (j++ == 0) message ("unused parameters found:\n");
	    message ("\tparameter %s = '%s'\n", okeys[i], ovals[i]);
	}
    }
    for (i = 0; i < paramIndex; ++i) {
	if (!psaid[i]) {
	    if (j++ == 0) message ("unused parameters found:\n");
	    message ("\tparameter %s = '%s'\n", pkeys[i], pvals[i]);
	}
    }
    if (j) message ("\n");
}

double strtod2 (char *s, char **p)
{
    int min = 0;
    *p = s;
    while (isspace(*s)) ++s;
    if (*s == '-') { ++s; ++min; }
    else if (*s == '+') ++s;
    if (s[0] == 'i' || s[0] == 'I')
    if (s[1] == 'n' || s[1] == 'N')
    if (s[2] == 'f' || s[2] == 'F') {
	*p = s+3;
	return min ? -HUGE_VAL : HUGE_VAL;
    }
    return strtod (*p, p);
}

/*
 * Lookup <key> as a double value, and return <dflt> as double if not found.
 */
double paramLookupD (const char *key, const char *dflt)
{
    char *p, *s = paramLookup (key);
    double d;
    int t = 0, swapped = 0;

    if (!dflt || !*dflt) dflt = "0";
    if (!s) { s = (char*)dflt; t = 1; }
    else if (!*s) { d = 0; goto set_to; }

    d = strtod2 (s, &p);
    if (p == s) { /* no conversion */
	d = strtod2 ((char*)dflt, &p);
	message ("parameter(r) %s = '%s' (substituting default = '%g')", key, s, d);
	return (d);
    }

    if (verbose_flag) {
	if (verbose_flag == 2) { swapVerboseStreams (); swapped = 1; }
set_to:
	if (t)
	    message ("parameter(r) %s = '%s' (default = '%g')", key, s, d);
	else
	    message ("parameter(r) %s = '%s' (set to '%g')", key, s, d);
	if (swapped) swapVerboseStreams ();
    }
    return (d);
}

int paramLookupI (const char *key, const char *dflt)
{
    char *p, *s = paramLookup (key);
    double d;
    int i, t = 0, swapped = 0;

    if (!dflt || !*dflt) dflt = "0";
    if (!s) { s = (char*)dflt; t = 1; }
    else if (!*s) { i = 1; goto set_to; }

    d = strtod2 (s, &p);
    if (p == s) { /* no conversion */
	d = strtod2 ((char*)dflt, &p);
	     if (d < -INT_MAX) i = -INT_MAX;
	else if (d >  INT_MAX) i =  INT_MAX;
	else i = (int) d;
	message ("parameter(i) %s = '%s' (substituting default = '%d')", key, s, i);
	return (i);
    }

	 if (d < -INT_MAX) { i = -INT_MAX; if (s[1] != 'i' && s[1] != 'I') goto set_to; }
    else if (d >  INT_MAX) { i =  INT_MAX;
	if (*s == '+') { if (s[1] != 'i' && s[1] != 'I') goto set_to; }
	else if (s[0] != 'i' && s[0] != 'I') goto set_to;
    }
    else { i = (int) d; if (i != d) goto set_to; }

    if (verbose_flag) {
	if (verbose_flag == 2) { swapVerboseStreams (); swapped = 1; }
set_to:
	if (t)
	    message ("parameter(i) %s = '%s' (default = '%d')", key, s, i);
	else
	    message ("parameter(i) %s = '%s' (set to '%d')", key, s, i);
	if (swapped) swapVerboseStreams ();
    }
    return (i);
}

bool_t paramLookupB (const char *key, const char *dflt)
{
    char *p, *s = paramLookup (key);
    int i, t, swapped = 0;

    if (!s) { t = 0; s = (char*)dflt; if (!s || !*s) s = "on"; }
    else if (*s) t = 1;
    else { i = t = 2; goto ret; }

    if (strsame (s, "on" ) || strsame (s, "1")) { i = 1; goto ret; }
    if (strsame (s, "off") || strsame (s, "0")) { i = 0; goto ret; }

    i = (int) strtod2 (s, &p);
    if (p == s) { /* no conversion */
	i = 1;
	if (dflt && *dflt && !strsame (dflt, "on")) i = 0;
	message ("parameter(b) %s = '%s' (substituting default = '%s')", key, s, i ? "on" : "off");
	return (i == 0 ? FALSE : TRUE);
    }
    t = 3;
ret:
    if (verbose_flag || t > 2) {
	if (verbose_flag == 2) { swapVerboseStreams (); swapped = 1; }
	if (t == 0)
	    message ("parameter(b) %s = '%s' (default)", key, s);
	else if (t == 1)
	    message ("parameter(b) %s = '%s' (set)", key, s);
	else
	    message ("parameter(b) %s = '%s' (set to '%s')", key, s, i == 0 ? "off" : "on");
	if (swapped) swapVerboseStreams ();
    }
    return (i == 0 ? FALSE : TRUE);
}

#ifdef DRIVER
/* test driver and example */
int main (int argc, char *argv[])
{
    extern int optind;
    extern char *optarg;
    int i, c;

    if (argc == 1) {
	system ("param -Spietje=puk pietje jantje");
    }

    while ((c = xgetopt (argc, argv, "S:")) != EOF) {
	switch (c) {
	    case 'S': paramSetOption (optarg); break;
	    case '?': fprintf (stderr, "bad option %c\n", c);
	}
    }
    paramReadFile ("paramtest.p");

    for (i = optind; i < argc; i++) {
	fprintf (stderr, "param: <%s> value: <%s>\n",
	    argv[i], paramLookupS (argv[i], "supposed default"));
    }
    return (0);
}
#endif /* DRIVER */
/* EOF */
