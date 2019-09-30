/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#define NO	0
#define YES	1

typedef int (* IFP) ();

typedef struct {
    char *      optionName;
    int         optionArg;
    int         (* function) ();
    void *	optionVariablePtr;
    char *	description;
} OptionSpec;

int eseOptionHandler (int argc, char *argv[], OptionSpec *optionSpecs, int noFixed, char *arguments[]);
int eseHelp    (OptionSpec *optionSpecs);
int eseHelpAll (OptionSpec *optionSpecs);
int eseToggle (int *argument);
int eseListArguments (char ***variable, char *arguments);
int eseAssignArgument (char **variable, char *argument);
int eseAssignIntArgument (int *variable, char *argument);
int esePrintString (char *argument);
int eseTurnOn  (int *argument);
int eseTurnOff (int *argument);

/* Example specification of options */
#ifdef LEAVE_THIS_BUT_DO_NOT_USE
OptionSpec optionSpecs[] = {
/* "usage" must always be on top, *
 * the rest (need not be) in alphabetical order. *
 * The description string is mandatory *
 * if in combination with a '-help' entry *
 * on the sanction of a possible core-dump. *
*/
    { "usage", NO, esePrintString, (void *)
            "Usage:             testOption [options] arguments",
            "Usage:     testOption [options] arguments. Options (may be abbreviated) are:"},
    { "help", NO, eseHelp, optionSpecs,
            "    -help:                 print this list" },
    { "verbose", NO, printVerbose, (void *) & verboseMode,
            "    -verbose:              print run-time information" },
    { "view", YES, printView, (void *) viewList,
            "    -view viewName:        do it for this view, too"  },
    { (char *) 0, 0, (IFP) 0, (void *) 0, (char *) 0 },
};
#endif

