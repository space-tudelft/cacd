/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	P. Bingley
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
#include <X11/Xlib.h>
#include "src/cacdcmap/cacdcmap.h"

#define VERSION "4.2 3-Mar-2008"

/* External Declarations */
extern int set_cacd_cmap (Display *dpy, int s_nr, int *planes, int nplanes, int print, int remove, int force);
extern int _nr_colors;
extern char *_colors[];

/* Global Declarations */
char	*rgb_name[128];
int	rgb_red[128];
int	rgb_green[128];
int	rgb_blue[128];
int	nr_rgb = 0;
int	rmv = 0;
int	print = 0;
int	force = 0;
int	silent = 0;
int	verbose = 0;
char	*dispname = NULL;
char	*colorfile = NULL;
char	*rgbfile = NULL;

char *argv0 = "setcmap";

void pr_missing (char *opt)
{
    PE "%s: missing argument\n", opt);
}

void fail ()
{
    if (!silent) PE "%s: Cannot alloc memory!\n", argv0);
    exit (1);
}

int main (int argc, char **argv)
{
    Display	*dpy;
    FILE	*fp;
    char	line[256];
    char	color[80];
    int		s_nr;
    int		planes[3];
    int		nplanes = 0;
    int		error = 0;
    int		i, R, G, B;

    /* parse options */
    for (i = 1; i < argc; ++i) {
	if (strcmp ("-help", argv[i]) == 0) {
	    error = 1;
	    continue;
	}
	else if (strcmp ("-3", argv[i]) == 0) {
	    planes[0] = 3;
	    nplanes = 1;
	    continue;
	}
	else if (strcmp ("-5", argv[i]) == 0) {
	    planes[0] = 5;
	    nplanes = 1;
	    continue;
	}
	else if (strcmp ("-7", argv[i]) == 0) {
	    planes[0] = 7;
	    nplanes = 1;
	    continue;
	}
	else if (strcmp ("-p", argv[i]) == 0) {
	    print = 1;
	    continue;
	}
	else if (strcmp ("-pd", argv[i]) == 0) {
	    print = 2;
	    continue;
	}
	else if (strcmp ("-ps", argv[i]) == 0) {
	    print = 3;
	    continue;
	}
	else if (strcmp ("-r", argv[i]) == 0) {
	    rmv = 1;
	    continue;
	}
	else if (strcmp ("-f", argv[i]) == 0) {
	    force = 1;
	    continue;
	}
	else if (strcmp ("-v", argv[i]) == 0) {
	    verbose = 1;
	    continue;
	}
	else if (strcmp ("-s", argv[i]) == 0) {
	    silent = 1;
	    continue;
	}
	else if (strcmp ("-display", argv[i]) == 0) {
	    if (++i >= argc) {
		if (!silent) pr_missing (argv[i]);
		error = 1;
	    }
	    else {
		dispname = argv[i];
	    }
	    continue;
	}
	else if (strcmp ("-cf", argv[i]) == 0) {
	    if (++i >= argc) {
		if (!silent) pr_missing (argv[i]);
		error = 1;
	    }
	    else {
		colorfile = argv[i];
	    }
	    continue;
	}
	else if (strcmp ("-rgb", argv[i]) == 0) {
	    if (++i >= argc) {
		if (!silent) pr_missing (argv[i]);
		error = 1;
	    }
	    else {
		rgbfile = argv[i];
	    }
	    continue;
	}
	else {
	    if (!silent) PE "%s: illegal option\n", argv[i]);
	    error = 1;
	}
    }

    if (error) {
	if (!silent) {
	    char *s = argv0;

	    PE "\n%s %s\n\n", argv0, VERSION);
	    PE "Usage: %s [-help] [-display host:dpy]\n", argv0);
	    while (*s) *s++ = ' ';
	    PE "       %s [-3|-5|-7] [-p[d|s]] [-r] [-f] [-s] [-v]\n", argv0);
	    PE "       %s [-cf colorfile] [-rgb rgbfile]\n\n", argv0);
	}
	exit (1);
    }

    /* force overrides remove */
    if (force) rmv = 0;

    /* silent overrides verbose */
    if (silent) verbose = 0;

    if (!dispname) dispname = getenv("DISPLAY");

    if (!(dpy = XOpenDisplay(dispname))) {
	PE "%s: Cannot open display %s\n", argv0, dispname ? dispname : "");
	exit (1);
    }
    s_nr = DefaultScreen (dpy);

    if (!rmv) {
	/* set up # of planes array that should be tried */
	if (!nplanes) {
	    if (7 < DisplayPlanes (dpy, s_nr)) {
		planes[nplanes++] = 7;
	    }
	    if (5 < DisplayPlanes (dpy, s_nr)) {
		planes[nplanes++] = 5;
	    }
	    if (3 < DisplayPlanes (dpy, s_nr)) {
		planes[nplanes++] = 3;
	    }
	    if (nplanes == 0) {
		if (!silent) {
		    PE "%s: Not enough bitplanes in your machine\n", argv0);
		}
		exit (1);
	    }
	}
    }

    if (colorfile) {
	if (!(fp = fopen (colorfile, "r"))) {
	    if (!silent)
		PE "%s: Cannot open colorfile \"%s\"\n", argv0, colorfile);
	    exit (1);
	}
	i = 0;
	while (fgets (line, 256, fp)) {
	    if (sscanf (line, "%s", color) != 1) {
		if (!silent) PE "%s: Read error in colorfile!\n", argv0);
		exit (1);
	    }
	    if (!(_colors[i] = malloc (strlen (color) + 1))) fail ();
	    strcpy (_colors[i], color);
	    if (++i == 128) break;
	}
	if (i < 8) {
	    if (!silent) PE "%s: Not enough colors in colorfile!\n", argv0);
	    exit (1);
	}
	_nr_colors = i;
	fclose (fp);
    }

    if (rgbfile) {
	if (!(fp = fopen (rgbfile, "r"))) {
	    if (!silent)
		PE "%s: Cannot open rgbfile \"%s\"\n", argv0, rgbfile);
	    exit (1);
	}
	while (fgets (line, 256, fp)) {
	    if (sscanf (line, "%d%d%d%s", &R, &G, &B, color) != 4) {
		if (!silent) PE "%s: Read error in rgbfile!\n", argv0);
		exit (1);
	    }
	    if (R < 0 || R > 255 || G < 0 || G > 255 || B < 0 || B > 255) {
		if (!silent)
		    PE "%s: Illegal RGB value in rgbfile at line %d\n",
			argv0, nr_rgb + 1);
		exit (1);
	    }
	    if (!(rgb_name[nr_rgb] = malloc (strlen (color) + 1))) fail ();
	    strcpy (rgb_name[nr_rgb], color);
	    rgb_red  [nr_rgb] = R;
	    rgb_green[nr_rgb] = G;
	    rgb_blue [nr_rgb] = B;
	    if (++nr_rgb == 128) break;
	}
	fclose (fp);
    }

    /* set cacd colormap */
    if (!set_cacd_cmap (dpy, s_nr, planes, nplanes, print, rmv, force)) {
	exit (1); /* not ok */
    }
    exit (0);
    return (0);
}

#ifdef STATIC
/* libX11.a fix for static linking */
void *dlopen (const char *filename, int flag)
{
    return NULL;
}
#endif
