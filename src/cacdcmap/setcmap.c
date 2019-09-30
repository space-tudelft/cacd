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
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "src/cacdcmap/cacdcmap.h"

/* External Declarations */
extern char *argv0;

extern char *_colors[];
extern int _nr_colors;

extern char *rgb_name[];
extern int rgb_red[];
extern int rgb_green[];
extern int rgb_blue[];
extern int nr_rgb;

extern int silent;
extern int verbose;

/* Local Declarations */
static int print_cacd_cmap (Display *dpy, XStandardColormap *scmap, int print);
static int remove_cacd_cmap (Display *dpy, int s_nr, XStandardColormap *scmap, Atom atom);
static int create_cacd_cmap (Display *dpy, int s_nr, int *nplanesp, int nplanes, Atom atom);
static int create_apollo_14_cmap (Display *dpy, int s_nr, Atom atom);
static int create_apollo_14_dummies (Display *dpy, Colormap cmap, int pixel);
static int free_apollo_14_dummies (Display *dpy, Colormap cmap);
static int get_apollo_14_color (Display *dpy, Colormap cmap, unsigned long pixel, int red, int green, int blue);

/*********************************************************/

int set_cacd_cmap (Display *dpy, int s_nr, int *planes, int nplanes, int print, int remove, int force)
{
    Atom		atom;
    XStandardColormap	scmap;

    /* if the cacd colormap exists */
    if((atom = XInternAtom(dpy, RGB_CACD_MAP, True)) &&
	XGetStandardColormap(dpy, RootWindow(dpy, s_nr), &scmap, atom)) {

	/* replace ? */
	if(force) {
	    /* first remove old colormap */
	    if(!remove_cacd_cmap (dpy, s_nr, &scmap, atom)) return(0);

	    /* then create new colormap */
	    if(!create_cacd_cmap (dpy, s_nr, planes, nplanes, atom)) {
		return(0);
	    }

	    /* get new scmap ? */
	    if(print || remove) {
		XGetStandardColormap(dpy, RootWindow(dpy, s_nr), &scmap, atom);
	    }
	}

	/* print ? */
	if(print && !print_cacd_cmap (dpy, &scmap, print)) return(0);

	/* remove ? */
	if(remove && !remove_cacd_cmap (dpy, s_nr, &scmap, atom)) {
	    return(0);
	}

	if(verbose && !force && !print && !remove) {
	    PE "%s: Cacd cmap already present\n", argv0);
	}

	/* all went well */
	return(1);
    }
    else {	/* the cacd colormap does not exist */
	if(remove) {
	    if(!silent) PE "%s: No cacd cmap present\n", argv0);
	    return(1);
	}

	/* create atom if necessary */
	if(!atom && (atom = XInternAtom(dpy, RGB_CACD_MAP, False)) == 0) {
	    if(!silent) PE "%s: Cannot create atom\n", argv0);
	    return(0);
	}

	/* create new colormap */
	if(!print || force) {
	    if(!create_cacd_cmap (dpy, s_nr, planes, nplanes, atom)) {
		return(0);
	    }
	}

	/* print ? */
	if(print) {
	    if(force) {
		XGetStandardColormap(dpy, RootWindow(dpy, s_nr), &scmap, atom);
		if(!print_cacd_cmap (dpy, &scmap, print)) return(0);
	    }
	    else {
		if(!silent) PE "%s: No cacd cmap present\n", argv0);
		return(0);
	    }
	}

	/* all went well */
	return(1);
    }
}


/* convert virtual cmap indices ('ind') to real pixels values */
static unsigned long int_to_pixel(ind, mask, base)
register unsigned long	ind;
register unsigned long	mask;
unsigned long		base;
{
    register unsigned long	pixel = 0;
    register unsigned long	real_ind = 1;

    while(ind) {
	while(!(mask & real_ind)) real_ind <<= 1;

	if(ind & 0x1) pixel |= real_ind;

	ind >>= 1;
	real_ind <<= 1;
    }
    return(pixel | base);
}

static int print_cacd_cmap (Display *dpy, XStandardColormap *scmap, int print)
{
    unsigned long	i;
    unsigned long	ncolors, basesh, planes, planemask;
    unsigned long	basep;
    Colormap		cmap;
    XColor		col;

    cmap = scmap->colormap;
    basep = scmap->base_pixel;
    basesh = scmap->red_mult;
    planes = scmap->blue_mult;
    planemask = scmap->blue_max;
    ncolors = 1 << planes;

    /* print these figures */
    printf("#planes = %ld, mask = 0x%lx, #colors = %lu, basepix = %lu\n",
		planes, planemask, ncolors, basep);
    printf("    contig = %ld, baseshift = %lu, Xid = 0x%lx\n",
		scmap->red_max, basesh, scmap->green_mult);
    i = scmap->green_max;
    printf("    nr_of_used_colors = %ld\n", i < 8 ? 32 : i);

    /* retrieve all colors from the colormap */
    for(i = 0; i < ncolors; ++i) {
	col.pixel = int_to_pixel(i, planemask, basep);

	XQueryColor(dpy, cmap, &col);

	switch (print) {
	case 3:
	    printf("pixel = %3lu, red = %3d, green = %3d, blue = %3d\n",
		col.pixel, col.red >> 8, col.green >> 8, col.blue >> 8);
	    break;
	case 2:
	    printf("pixel = %3lu, red = %5d, green = %5d, blue = %5d\n",
		col.pixel, col.red, col.green, col.blue);
	    break;
	default:
	    printf("pixel = %3lu, red = %f, green = %f, blue = %f\n",
		col.pixel, (float) (col.red / 65535.0),
		(float) (col.green / 65535.0), (float) (col.blue / 65535.0));
	}
    }

    return(1);
}

static int remove_cacd_cmap (Display *dpy, int s_nr, XStandardColormap *scmap, Atom atom)
{
/* free colormap and the colors in the colormap */

    /* Would like to do this but we are not allowed to */
    /* since we are not owner of the resources. Therefore */
    /* we created the (unused) Xid so we can 'kill' the */
    /* 'resources' which we are allowed */
    /*
    unsigned long planes = (((1 << scmap->blue_mult) - 1) << scmap->red_mult);
    XFreeColors(dpy, scmap->colormap, &scmap->base_pixel, 1, planes);
    */

/* use XKILLWORKAROUND when you cannot 'kill the resources' */
/* of a client by using the Xid */
#ifdef XKILLWORKAROUND
    XKillClient(dpy, AllTemporary);
#else
    if(scmap->green_mult) {
	XKillClient(dpy, (XID) scmap->green_mult);
    }
    else if(!silent) {
	PE "%s: Warning: Cannot free resources\n", argv0);
    }
#endif

/* now remove the standard cacd colormap */

    /* grab the server to make this an atomair operation */
    XGrabServer(dpy);

    /* remove the standard colormap */
    XDeleteProperty(dpy, RootWindow(dpy, s_nr), atom);

    /* ungrab the server */
    XUngrabServer(dpy);

    /* sync X (to make sure that the cacd colormap is removed) */
    XSync(dpy, 0);

    /* tell user about it */
    if(!silent) PE "%s: cacd colormap removed\n", argv0);

    return(1);
}

static int find_rgb_nr (char *color)
{
    register int j;

    for (j = 0; j < nr_rgb; ++j)
	if (!strcmp (rgb_name[j], color)) return (j);
    return (-1);
}

static int find_color_nr (char *color)
{
    register int j;

    for (j = 0; j < _nr_colors; ++j)
	if (!strcmp (_colors[j], color)) return (j);
    return (-1);
}

void col_err (char *color)
{
    if (!silent) PE "%s: Cannot find color \"%s\"\n", argv0, color);
}

static int create_cacd_cmap (Display *dpy, int s_nr, int *nplanesp, int nplanes, Atom atom)
{
    register int	i, j, last, basesh = 0, nr;
    Colormap		cmap;
    unsigned long	low, basep, planes[MAXPLANES];
    unsigned long	planemask = 0;
    int			contig = 1;
    Pixmap		pixmap;
    XColor		col;
    XStandardColormap	scmap;
    int			apollo_14 = 0;

    /* get the colormap to allocate from */
    cmap = DefaultColormap(dpy, s_nr);

    /* grab the server to make this an atomair operation */
    XGrabServer(dpy);

    /* now create pixmap as XID to identify colorcells for remove */
    if((pixmap = XCreatePixmap(dpy, RootWindow(dpy, s_nr), (unsigned) 1,
		(unsigned) 1, (unsigned) DefaultDepth(dpy, s_nr))) == None) {
	if(!silent) {
	    PE "%s: Warning: Cannot create pixmap Id\n", argv0);
	}
    }
    else if(verbose) {
	PE "%s: pixmap Id created (0x%lx)\n", argv0, pixmap);
    }

    /* try to allocate the largest amount of planes */
    /* if this is an apollo; forget it, since even trying it */
    /* (which results in failure) causes the Xapollo server */
    /* to go into '8 color' mode */
    if (DisplayCells (dpy, s_nr) != 14) { /* it's not an apollo */
	for (i = 0; i < nplanes; ++i) {
	    if (!XAllocColorCells (dpy, cmap, contig, planes, nplanesp[i], &basep, 1)) {
		if (!silent) {
		    PE "%s: Cannot allocate %d bitplanes\n", argv0, nplanesp[i]);
		}
	    }
	    else {
		if (verbose) {
		    PE "%s: allocated %d planes\n", argv0, nplanesp[i]);
		}
		break;
	    }
	}
    }
    else {	/* it's an apollo (at least something with 14 colors) */
	/* check if we can create 3-bitplane apollo colormap */
	/* and if the caller wants us to */
	if (nplanesp[nplanes-1] < DisplayPlanes (dpy, s_nr) &&
	    create_apollo_14_cmap (dpy, s_nr, atom)) {
	/* set some values for this colormap */
	    i = nplanes - 1;
	    apollo_14 = 1;
	    basep = 0;
	    nplanes = 3;
	    _nr_colors = 8;
	    contig = 0;
	    planemask = 0x0d;
	}
	else {
	    i = nplanes;
	    if (!silent) {
		PE "%s: Cannot allocate %d bitplanes\n", argv0, nplanesp[nplanes-1]);
	    }
	}
    }

    /* if all allocations failed */
    if (i >= nplanes) {
	/* remove pixmap ID */
	if (pixmap) XFreePixmap (dpy, pixmap);

	/* ungrab the server !! */
	XUngrabServer (dpy);

	if (verbose) PE "%s: Removed pixmap Id\n", argv0);
	return (0);
    }

    if (!apollo_14) {
	nplanes = nplanesp[i];

	/* find lowest plane and build planemask */
	for(low = ~0, i = 0; i < nplanes; ++i) {
	    low = ((low < planes[i]) ? low : planes[i]);
	    planemask |= planes[i];
	}

	/* set last color */
	last = 1 << nplanes;
	if (_nr_colors > last) _nr_colors = last;

	if(verbose)
	    PE "%s: planes = %d, planemask = 0x%lx, low = %lu, nr_colors = %d\n",
				argv0, nplanes, planemask, low, last);

	/* calculate shift from low */
	for (basesh = -1; low; ++basesh) low >>= 1;

	if (verbose)
	    PE "%s: baseshift = %d, basepixel = %lu, used_colors = %d\n",
		argv0, basesh, basep, _nr_colors);

	/* set color flags */
	col.flags = (char) (DoRed | DoGreen | DoBlue);

	/*
	 * store all colors in the colormap
	 */
	if ((nr = last) > _nr_colors) nr = _nr_colors;
	for (i = 0; i < nr; ++i) {
	    if (nr_rgb && (j = find_rgb_nr (_colors[i])) >= 0) {
		col.red   = rgb_red  [j] << 8;
		col.green = rgb_green[j] << 8;
		col.blue  = rgb_blue [j] << 8;
	    }
	    else {
		if (!XParseColor (dpy, cmap, _colors[i], &col)) {
		    col_err (_colors[i]);
		    return (0);
		}
	    }
	    col.pixel = (unsigned long) ((i << basesh) | basep);
	    XStoreColor (dpy, cmap, &col);
	}

	if (last > _nr_colors) { /* set all whites */
	    j = -1;
	    if (nr_rgb) {
		j = find_rgb_nr ("White");
		if (j < 0) j = find_rgb_nr ("white");
	    }
	    if (j >= 0) {
		col.red   = rgb_red  [j] << 8;
		col.green = rgb_green[j] << 8;
		col.blue  = rgb_blue [j] << 8;
	    }
	    else {
		j = find_color_nr ("White");
		if (j < 0) j = find_color_nr ("white");
		if (j < 0) {
		    col_err ("White");
		    return (0);
		}
		if (!XParseColor (dpy, cmap, _colors[j], &col)) {
		    col_err (_colors[j]);
		    return (0);
		}
	    }
	    for(; i < last; ++i) {
		col.pixel = (unsigned long) ((i << basesh) | basep);
		XStoreColor (dpy, cmap, &col);
	    }
	}
    }

    /* now set the standard colormap information */
    scmap.colormap   = cmap;
    scmap.red_max    = (unsigned long) contig;
    scmap.red_mult   = (unsigned long) basesh;
    scmap.green_max  = (unsigned long) _nr_colors;
    scmap.green_mult = (unsigned long) pixmap;
    scmap.blue_max   = planemask;
    scmap.blue_mult  = (unsigned long) nplanes;
    scmap.base_pixel = basep;

    /* set the standard colormap */
    XSetStandardColormap(dpy, RootWindow(dpy, s_nr), &scmap, atom);

    if(!silent && verbose) PE "%s: cacd colormap initialized\n", argv0);

    /* do not destroy allocated colormap on exit */
#ifdef XKILLWORKAROUND
    XSetCloseDownMode(dpy, RetainTemporary);
#else
    XSetCloseDownMode(dpy, RetainPermanent);
#endif

    /* sync X (to make sure that the cacd colormap arrives in the server) */
    XSync(dpy, 0);

    /* ungrab the server */
    XUngrabServer(dpy);

    /* all is well */
    return(1);
}

/* X rgb on apollo (4-planes) ranges from 0 to 65535 in 4096 increments */
#define	RGB_MIN	4000
#define RGB_MAX	61000

static unsigned long	app14_pixs[8] = { 0, 1,  4, 5,  8, 9,  12, 13 };

static int create_apollo_14_cmap (Display *dpy, int s_nr, Atom atom)
{
    int		i, j;
    XColor	col;
    Colormap	cmap;
    int		cmask = DoRed | DoGreen | DoBlue;
    FILE	*ps;
    int		mixed_mode = 0;
    char	ps_line[128];	/* should be enough */

    /* Check if this baby is running mixed mode dm-Xapollo.
     * If it is, this program will not fail on the checks it does on
     * the colour table. It will not even try allocating new colours
     * It is all up to Apollo "lcm" command.
     */
    ps = popen("ps -x", "r");
    while (fscanf(ps, "%s", ps_line) != EOF)
	if (strcmp(ps_line, "dm") == 0)  {
	    mixed_mode = 1;
	    if (verbose) {
		PE "%s: running mixed mode dm/Xapollo !!\n", argv0);
		PE "%s: load the colour map with lcm command\n", argv0);
	    }
	}
    if (mixed_mode) return(1);

    /* get the colormap to allocate from */
    cmap = DefaultColormap(dpy, s_nr);

    /* enough planes and colors ? */
    if(DisplayPlanes(dpy, s_nr) != 4 || DisplayCells(dpy, s_nr) != 14)
			return(0);

    /* check pixel value 0 (should be black) */
    col.pixel = (unsigned long) 0;
    XQueryColor(dpy, cmap, &col);
    if(col.red > RGB_MIN || col.green > RGB_MIN ||
	col.blue > RGB_MIN || (col.flags & cmask) != cmask) {

	if(verbose)
	    PE "pixel = %2lu, red = %f, green = %f, blue = %f, flags = 0x%x\n",
		col.pixel, (float) (col.red / 65535.0),
		(float) (col.green / 65535.0),
		(float) (col.blue / 65535.0), col.flags);

	return(0);
    }

    /* check pixel value 1 (should be red) */
    col.pixel = (unsigned long) 1;
    XQueryColor(dpy, cmap, &col);
    if(col.red < RGB_MAX || col.green > RGB_MIN ||
	col.blue > RGB_MIN || (col.flags & cmask) != cmask) {

	if(verbose)
	    PE "pixel = %2lu, red = %f, green = %f, blue = %f, flags = 0x%x\n",
		col.pixel, (float) (col.red / 65535.0),
		(float) (col.green / 65535.0),
		(float) (col.blue / 65535.0), col.flags);

	return(0);
    }

    /* alloc the additional colors (read-only) */
    for (i = 2; i < 8; ++i) {

	if (nr_rgb && (j = find_rgb_nr (_colors[i])) >= 0) {
	    col.red   = rgb_red  [j] << 8;
	    col.green = rgb_green[j] << 8;
	    col.blue  = rgb_blue [j] << 8;
	}
	else {
	    if (!XParseColor (dpy, cmap, _colors[i], &col)) {
		col_err (_colors[i]);
		return (0);
	    }
	}

	if (!get_apollo_14_color (dpy, cmap, app14_pixs[i], col.red, col.green, col.blue)) {
	    if (verbose)
		PE "%s: Cannot allocate pixel %ld (red = %f, green = %f, blue = %f)\n",
		    argv0, app14_pixs[i],
			(float) (col.red / 65535.0),
			(float) (col.green / 65535.0),
			(float) (col.blue  / 65535.0));
	    return (0);
	}
    }
    (void) free_apollo_14_dummies(dpy, cmap);
    return(1);
}

static unsigned long	dum_arr[16];
static int		nr_dummies = 0;
static int		first_get_color = 1;
static unsigned long	last_color;

static int get_apollo_14_color (Display *dpy, Colormap cmap, unsigned long pixel, int red, int green, int blue)
{
    unsigned long	pixs, planes;
    XColor		col;

    col.red = red;
    col.green = green;
    col.blue = blue;
    col.flags = DoRed | DoGreen | DoBlue;

    /* get first color writable to see where we are in the color table */
    /* we can then decide to either use it or make it a dummy */
    /* or abort if the color table is filled to far already */
    if(first_get_color) {
	first_get_color = 0;
#ifdef DEBUG
PE "Firstime: color %2u (r = %5d, g = %5d, b = %5d)\n",
	pixel, col.red, col.green, col.blue);
#endif /* DEBUG */

	if(!XAllocColorCells(dpy, cmap, 0, &planes, 0, &pixs, 1)
		|| pixs > pixel) return(0);
#ifdef DEBUG
PE "We got %u\n", pixs);
#endif /* DEBUG */

	last_color = pixs;

	/* Hey! we can use this */
	if(pixel == pixs) {
#ifdef DEBUG
PE "Storing it\n");
#endif /* DEBUG */
	    col.pixel = pixs;
	    XStoreColor(dpy, cmap, &col);
	    return(1);
	}

#ifdef DEBUG
PE "Making it dummy\n");
#endif /* DEBUG */
	/* this will be a dummy */
	dum_arr[nr_dummies] = pixs;
	++nr_dummies;
    }

    /* allocate dummies */
    if (last_color < pixel - 1) {
	create_apollo_14_dummies (dpy, cmap, pixel);
    }
    if (last_color >= pixel) return(0);

#ifdef DEBUG
PE "Allocating color %2u (r = %5d, g = %5d, b = %5d)\n",
	pixel, col.red, col.green, col.blue);
#endif /* DEBUG */
    /* try to get the color read-only */
    if(!XAllocColor(dpy, cmap, &col) || pixel < col.pixel) return(0);

#ifdef DEBUG
PE "Got pixel %u\n", col.pixel);
#endif /* DEBUG */
    /* we got the right one ! */
    if(pixel == col.pixel) {
	last_color = pixel;
	return(1);
    }

    /* else (pixel > col.pixel) we need to allocate writable */
    /* do not free the incorrect pixel here but do it later with the dummies */
#ifdef DEBUG
PE "Making it dummy; trying writable color\n");
#endif /* DEBUG */
    dum_arr[nr_dummies] = col.pixel;
    ++nr_dummies;

    if(!XAllocColorCells(dpy, cmap, 0, &planes, 0, &pixs, 1)
		|| pixel != pixs) return(0);
#ifdef DEBUG
PE "Got writable pixel %u\n", pixs);
#endif /* DEBUG */

    /* pixel == pixs */
    last_color = col.pixel = pixs;
    XStoreColor(dpy, cmap, &col);
    return (1);
}

static int create_apollo_14_dummies (Display *dpy, Colormap cmap, int pixel)
{
    unsigned long pixs, planes;

    while (last_color < pixel - 1) {
#ifdef DEBUG
PE "Getting dummy: ");
#endif /* DEBUG */

	if (!XAllocColorCells (dpy, cmap, 0, &planes, 0, &pixs, 1)) return(0);
#ifdef DEBUG
PE "Got dummy %u\n", pixs);
#endif /* DEBUG */

	last_color = dum_arr[nr_dummies] = pixs;
	++nr_dummies;
    }
    return (1);
}

static int free_apollo_14_dummies (Display *dpy, Colormap cmap)
{
#ifdef DEBUG
    int	i;

    PE "Freeing dummy pixels: ");
    for(i = 0; i < nr_dummies; ++i) PE "%d, ", dum_arr[i]);
    PE "\n");
#endif /* DEBUG */

    XFreeColors (dpy, cmap, dum_arr, nr_dummies, 0);
    return(1);
}
