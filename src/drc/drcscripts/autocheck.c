/*
 * ISC License
 *
 * Copyright (C) 1989-2018 by
 *	T.G.R. van Leuken
 *	S. de Graaf
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

char *argv0 = "autocheck";
char *usage = "\nUsage: %s [-f] [-g] [-t] [-v] [cell_name]\n\n";

int main (int argc, char *argv[])
{
	char	cmd[512], *p;
	int	i, fflag = 0, gflag = 0, tflag = 0, vflag = 0;

	for (i = 1; i < argc && argv[i][0] == '-'; ++i) {
	    for (p = &argv[i][1]; *p; ++p) {
		switch (*p) {
		case 'f':
			fflag = 1;
			break;
		case 'g':
			gflag = 1;
			break;
		case 't':
			tflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			fprintf(stderr, "\nunknown option: %c", *p);
			fprintf(stderr, usage, argv0);
			return (1);
		}
	    }
	}

	if (i < argc) {
	    p = argv[i];
	    if (++i < argc) {
		if (argv[i][0] == '-')
		    fprintf(stderr, "\ncell_name must be last argument");
		else
		    fprintf(stderr, "\ntoo many arguments");
		fprintf(stderr, usage, argv0);
		return (1);
	    }
	}
	else p = NULL;

	sprintf(cmd, "%s", "dimcheck -a");
	if (fflag) strcat(cmd, " -f");
	if (gflag) strcat(cmd, " -g");
	if (tflag) strcat(cmd, " -t");
	if (p) { strcat(cmd, " "); strcat(cmd, p); }
	if (vflag) fprintf(stdout, "======= %s =========\n", cmd);
	if (system(cmd)) return (1);

	return (0);
}
