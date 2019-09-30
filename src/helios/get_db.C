/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#undef atexit /* Motif header files make this malicious definition */

#include "src/helios/realist.h"
#include "src/helios/externs.h"

/* Sort the string table */
int compare (const void *s1, const void *s2)
{
    return (strcmp (*((char **)s1), *((char **)s2)));
}

void asort (char **list, int n)
{
    qsort (list, n, sizeof (char *), compare);
}

void sort (char **cell_list)
{
    register int n;
    char **string;

    n = 0;
    if (cell_list) {
	string = cell_list;
	while (*string) {
	    ++n;
	    ++string;
	}
        /* Use the quick-sort routine provided by the operating system */
	qsort (cell_list, n, sizeof (char *), compare);
    }
}

/***********************************************************************
*
* Write a device model to a file
*
***********************************************************************/
void WriteDeviceFile (FILE *fp, char *device, char *terminals, char *prefix,
                      char *context, char *description)
{
    fprintf (fp, "device %s\n", device);
    fprintf (fp, "begin spicemod\n");
    fprintf (fp, "* terminals\t%s\n", terminals);
    fprintf (fp, "* prefix\t%s\n", prefix);
    fprintf (fp, "* description\t%s\n", description);

    if (context[strlen (context) - 1] != '\n')
	fprintf (fp, "%s\nend\n", context);    /* can't the trailing '\n' be omitted ? */
    else
	fprintf (fp, "%send\n", context);
}
