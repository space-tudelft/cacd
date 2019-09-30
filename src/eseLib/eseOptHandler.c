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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "src/eseLib/eseOption.h"

#define MAXAMB  8

static int argvNumber = 0;
static int curFixed = 0;
int errorFlag = 0;

static int optionError (char *string, char *arg1, char *arg2)
{
    fprintf (stderr, "Error: ");
    fprintf (stderr, string, arg1, arg2);
    return 1;
}

static char * getOption (int argc, char *argv[], int noFixed, char *arguments[])
{
    if (++argvNumber >= argc) {
        return NULL;
    }
    else if (argv[argvNumber][0] != '-') {
        if (curFixed >= noFixed) {
            optionError ("Expected an option: '-xxxx', not an argument: '%s'\n", argv[argvNumber], "");
            errorFlag++;
        }
        else {
	    arguments[curFixed++] = argv[argvNumber];
	}
        return getOption (argc, argv, noFixed, arguments);
    }
    else {
	return argv[argvNumber] + 1;
    }
}

static char * getOptionArgument (int argc, char *argv[], int shouldBe, int noFixed, char *arguments[])
{
    if (++argvNumber >= argc) {
        return NULL;
    }
    else if (argv[argvNumber][0] == '-' && !shouldBe) {
	argvNumber--;
	return NULL;
    }
    else if (shouldBe) {
	return argv[argvNumber];
    }
    else if (curFixed >= noFixed) {
	return argv[argvNumber];
    }
    else {
	arguments[curFixed++] = argv[argvNumber];
	return NULL;
    }
}

int eseOptionHandler (int argc, char *argv[], OptionSpec *optionSpecs, int noFixed, char *arguments[])
{
    char * currentOption;
    char * currentArgument = NULL;

    while ((currentOption = getOption (argc, argv, noFixed, arguments)) != NULL) {
	/* find the (unique) entry in optionSpecs */
        IFP currentFunction;
	void * optionVariablePtr;
	int index;
	int foundEntry = -1;
        int    ambOptions[MAXAMB];
        int    currAmbOption = -1;

	for (index = 0; optionSpecs[index].optionName != NULL; index++) {
	    if (strncmp (currentOption, optionSpecs[index].optionName,
                    strlen (currentOption)) == 0) {
		if (optionSpecs[index].optionName[0] == '%' &&
			getenv ("ESEOPTIONS") == NULL) {
		    continue;
		}

	        foundEntry = index;

		if (strlen (currentOption) ==
			strlen (optionSpecs[index].optionName)) {
		    /* strings match exactly, this must be the one */
		    currAmbOption = 0;
                   ambOptions[currAmbOption] = index;
		   break;
                }
		else if (currAmbOption < MAXAMB - 1) {
                        /* retain matching options in case of ambiguities */
                   ambOptions[++currAmbOption] = index;
                }
	    }
	}

	if (foundEntry == -1) {
	    optionError ("Option '-%s' is unknown\n", currentOption, "");
	    getOptionArgument (argc, argv, 0, noFixed, arguments);
				/* actually skip, if there is one */
	    errorFlag++;
	    continue;
	}

	if (currAmbOption > 0) {
	    optionError ("Option '-%s' is ambiguous\n", currentOption, "");
	    getOptionArgument (argc, argv, 0, noFixed, arguments);
				/* actually skip, if there is one */

            fprintf (stderr, "Possible options are:\n");
            do {
                fprintf (stderr, "-%s\n", optionSpecs[ambOptions[currAmbOption]].optionName);
            } while (currAmbOption-- > 0);

	    errorFlag++;
	    continue;
	}

	/* We have found a unique option */
        /* Check on missing, illegal arguments, respectively */
	if (optionSpecs[foundEntry].optionArg == YES &&
		(currentArgument = getOptionArgument (argc, argv, 1, noFixed, arguments)) == NULL) {
	    optionError ("Option '-%s' needs an argument\n", optionSpecs[foundEntry].optionName, "");
	    errorFlag++;
	    continue;
	}

	if (optionSpecs[foundEntry].optionArg == NO &&
		(currentArgument = getOptionArgument (argc, argv, 0, noFixed, arguments)) != NULL) {
	    optionError ("Option '-%s' has an illegal argument: '%s'\n", optionSpecs[foundEntry].optionName, currentArgument);
	    errorFlag++;
	    continue;
	}

	/* So far so good */
        currentFunction = optionSpecs[foundEntry].function;
        optionVariablePtr = optionSpecs[foundEntry].optionVariablePtr;

	if (optionSpecs[foundEntry].optionArg == YES) {
            (*currentFunction) (optionVariablePtr, currentArgument);
	}
	else {
	    (*currentFunction) (optionVariablePtr);
	}
    }
    return (errorFlag);
}

int eseListArguments (char ***variable, char *arguments)
{
    char * endOfArgument;
    char * curArgument;
    int index = 0;
    int noElements = 8; /* at least 4 or more */

    if (!(*variable)) {
        *variable = (char **) malloc (noElements * sizeof (char *));
        (*variable)[0] = NULL;
    }
    while ((*variable)[index] != NULL) {
        index++;
    }

    for (curArgument = arguments; curArgument != NULL; index++) {
	int strl;

	if (index + 2 > noElements) {
	    int i;
	    noElements *= 2;
	    *variable = (char **) realloc (*variable, noElements * sizeof (char *));
	}

	while (curArgument[0] == '\n' ||
		curArgument[0] == ' ' ||
		curArgument[0] == '\t') {
	    curArgument++;

	}

	strl = strlen (curArgument);
	while (curArgument[strl - 1] == '\n' ||
		curArgument[strl - 1] == ' ' ||
		curArgument[strl - 1] == '\t') {
	    curArgument[strl - 1] = '\0';
	    strl--;
	}

	if ((endOfArgument = strchr (curArgument, ',')) != NULL) {
 	    *endOfArgument = '\0';
            (*variable)[index] = curArgument;
	    curArgument = endOfArgument + 1;
	}
	else {
	    (*variable)[index] = curArgument;
	    curArgument = NULL;
	}
    }
    (*variable)[index] = NULL;
    return 0;
}

int eseAssignArgument (char **variable, char *argument)
{
    *variable = argument;
    return 0;
}

int eseAssignIntArgument (int *variable, char *argument)
{
    *variable = atoi (argument);
    return 0;
}

int eseToggle (int *argument)
{
    if (*argument == 1)
        *argument = 0;
    else
        *argument = 1;
    return *argument;
}

int eseTurnOn (int *argument)
{
    *argument = 1;
    return *argument;
}

int eseTurnOff (int *argument)
{
    *argument = 0;
    return *argument;
}

int esePrintString (char *argument)
{
    fprintf (stdout, argument);
    fprintf (stdout, "\n");
    exit (0);
}

int eseHelp (OptionSpec *optionSpecs)
{
    int index;
    fprintf (stdout, "\n");
    for (index = 0; optionSpecs[index].optionName != NULL; index++) {
	if (optionSpecs[index].description &&
		optionSpecs[index].optionName[0] != '%') {
	    fputs (optionSpecs[index].description, stdout);
	    fprintf (stdout, "\n");
	}
    }
    fprintf (stdout, "\n");
    exit (0);
}

int eseHelpAll (OptionSpec *optionSpecs)
{
    int index;
    fprintf (stdout, "\n");
    for (index = 0; optionSpecs[index].optionName != NULL; index++) {
	if (optionSpecs[index].description) {
	    fputs (optionSpecs[index].description, stdout);
	    fprintf (stdout, "\n");
	}
    }
    fprintf (stdout, "\n");
    exit (0);
}

