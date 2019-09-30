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
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "src/cacdcmap/cacdcmap.h"
#include "src/cacdcmap/cmaperror.h"
#include "src/libddm/dmincl.h"

/* #define DEBUG */

/* External Declarations */
extern	int	cacdcmaperrno;

/* Local Declarations */
static	Display			*_Dpy;
static	int			_S_nr;
static	int			_Initialized = 0;
static	IdElem			*_Idtable = NULL;
static	int			*_pixused = NULL;
static	int			_NIds = 0;
static	int			_tabmax = 0;
static	int			_tabsize = 0;
static	Colormap		_cmap;
static	unsigned long		_basep;
static	int			_basesh;
static	int			_planes;
static	int			_contig;
static	int			_ncolors;
static	int			_used_colors;
static	unsigned long		_planemask;
static	XStandardColormap	_scmap;
static	unsigned long		(*_convint2pix)();
static	unsigned long		(*_convpix2int)();

static int _CoupleIdToColor (int Id, char *color);
static int _CoupleIdToRGB (int Id, float R, float G, float B);
static int _DecoupleId (int Id);
static int _ResizeIdTable (int Id);
static int _GetPixelFromIds (int *Ids, int n, unsigned long *pixel);
static unsigned long _ContigIntToPixel (unsigned long ind);
static unsigned long _ContigPixelToInt (unsigned long pixel);
static unsigned long _NonContigIntToPixel (register unsigned long ind);
static unsigned long _NonContigPixelToInt (register unsigned long pixel);

/*********************************************************/

int InitCacdCmap (Display *dpy, int s_nr)
{
    register int	i;
    register IdElem	*id;
    Atom		atom;
    int			first = 1;
  //char		cmd[256];

    /* if initialized free resources */
    if(_Initialized) {
	if(dpy == _Dpy && s_nr == _S_nr) {
	    return(1);
	}
	else {
	    if(_Idtable) (void) free((char *) _Idtable);
	    if(_pixused) (void) free((char *) _pixused);
	    _Dpy = NULL;
	    _S_nr = 0;
	    _NIds = 0;
	    _tabmax = 0;
	    _tabsize = 0;
	    _Initialized = 0;
	}
    }

Again:
    /* check if the 'RGB_CACD_MAP' atom exists */
    if ((atom = XInternAtom (dpy, RGB_CACD_MAP, True))) {

	/* atom exists: try to get the standard cacd colormap */
	if(XGetStandardColormap(dpy, RootWindow(dpy, s_nr), &_scmap, atom)) {

	    /* the standard cacd cmap exists ! */
	    /* set static vars */
	    _cmap = _scmap.colormap;
	    _basep = _scmap.base_pixel;
	    _basesh = _scmap.red_mult;
	    _contig = _scmap.red_max;
	    _planes = _scmap.blue_mult;
	    _planemask = _scmap.blue_max;
	    _ncolors = 1 << _planes;
	    _used_colors = _scmap.green_max;
	    if (_used_colors < 8) _used_colors = 32;

#ifdef DEBUG
PE "planes=%d, mask=0x%x, ncols=%d, basep=%u, basesh=%d, contig=%d, ucols=%d\n",
	_planes, _planemask, _ncolors, _basep, _basesh, _contig, _used_colors);
#endif

	    /* set the virtual to real colormap conversion functions */
	    if(_contig) {
		_convint2pix = _ContigIntToPixel;
		_convpix2int = _ContigPixelToInt;
	    }
	    else {
		_convint2pix = _NonContigIntToPixel;
		_convpix2int = _NonContigPixelToInt;
	    }

	    /* set float color table */
	    if(_planes != 3 && _planes != 5 && _planes != 7) {
		cacdcmaperrno = BADCMAP;
		return(0);
	    }

	    /* create Id mapping array */
	    if((_Idtable = Calloc(TABSIZE, IdElem)) == NULL) {
		cacdcmaperrno = NOCORE;
		return(0);
	    }
	    /* init Id mapping array */
	    _tabsize = TABSIZE;
	    for(i = 0, id = _Idtable; i < _tabsize; ++i, ++id) {
		id->state = NOTUSED;
	    }

	    /* create _pixused array */
	    if((_pixused = Calloc(_ncolors, int)) == NULL) {
		cacdcmaperrno = NOCORE;
		return(0);
	    }
	    /* init _pixused array */
	    for(i = 0; i < _ncolors; ++i) {
		_pixused[i] = 0;
	    }

	    _Dpy = dpy;
	    _S_nr = s_nr;
	    _Initialized = 1;
	    return(1);
	}
    }

    /* it did not exist */
    if(!_Initialized) {
	/* if this was the first try -> try to create the cacdcmap */
	if(first) {
	    first = 0;

	    /* now try to create the cacdcmap */
	    //sprintf(cmd, "setcmap -s -f -display %s", DisplayString(dpy));
	    //system(cmd);
	    _dmRun ("setcmap", "-s", "-f", "-display", DisplayString(dpy), (char *) NULL);

	    /* retry */
	    goto Again;
	}
	else {	/* second try: creating the cacdcmap failed */
	    cacdcmaperrno = NOCMAP;
	    /* return(0); */
	}
    }

    if(!_Initialized) { /* Try to use the default colormap */

	_planes = DisplayPlanes (dpy, s_nr);
	if (_planes < 3) {
	    cacdcmaperrno = BADCMAP;
	    return (0);
	}

	/* get the colormap to allocate from */
	_cmap = DefaultColormap (dpy, s_nr);
	if (!_cmap) {
	    cacdcmaperrno = BADCMAP;
	    return (0);
	}

	/* set static vars */
	_basep = 0;
	_basesh = 0;
	_ncolors = 1 << _planes;
	_planemask = _ncolors - 1;
	_used_colors = _ncolors > 256 ? 256 : _ncolors;

#ifdef DEBUG
PE "planes=%d, mask=0x%x, ncols=%d, basep=%u, basesh=%d, ucols=%d\n",
    _planes, _planemask, _ncolors, _basep, _basesh, _used_colors);
#endif
	/* create Id mapping array */
	if ((_Idtable = Calloc (TABSIZE, IdElem)) == NULL) {
	    cacdcmaperrno = NOCORE;
	    return (0);
	}
	_tabsize = TABSIZE;

	_Dpy = dpy;
	_S_nr = s_nr;
	_Initialized = 2;
	return (1);
    }
    /* should not get here */

    return(0);
}

int CreateGCforCmap (Drawable d, GC *pgc)
{
    GC gc;

    /* if not initialized */
    if (!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    /* create graphics context */
    if ((gc = XCreateGC (_Dpy, d, 0L, NULL))) {
	/* set plane mask */
	XSetPlaneMask (_Dpy, gc, _planemask);
	*pgc = gc;
	return(1);
    }
    else {	/* gc creation failed */
	cacdcmaperrno = NOGC;
	return(0);
    }
}

int CoupleIdsToColors (int *Ids, char **colors, int n)
{
    register int i;

    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    /* for all Id's */
    for(i = 0; i < n; ++i) {
	if(!_CoupleIdToColor(Ids[i], colors[i])) return(0);
    }

    return(1);
}

int CoupleIdToColor (int Id, char *color)
{
    /* if not initialized */
    if (!_Initialized) {
	cacdcmaperrno = NOINIT;
	return (0);
    }
    return (_CoupleIdToColor (Id, color));
}

static int _CoupleIdToColor (int Id, char *color)
{
    XColor def;

    if (XParseColor (_Dpy, _cmap, color, &def)) {
	return (_CoupleIdToRGB (Id, (float) (((double) def.red) / 65535.0),
				(float) (((double) def.green) / 65535.0),
				(float) (((double) def.blue) / 65535.0)));
    }
    else {	/* color does not exist */
	cacdcmaperrno = BADCOLOR;
	return (0);
    }
}

int CoupleIdsToRGBs (int *Ids, float *Rs, float *Gs, float *Bs, int n)
{
    register int i;

    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    /* for all Id's */
    for(i = 0; i < n; ++i) {
	if(!_CoupleIdToRGB(Ids[i], Rs[i], Gs[i], Bs[i])) return(0);
    }

    return(1);
}

int CoupleIdToRGB (int Id, float R, float G, float B)
{
    /* if not initialized */
    if (!_Initialized) {
	cacdcmaperrno = NOINIT;
	return (0);
    }
    return (_CoupleIdToRGB (Id, R, G, B));
}

static int _CoupleIdToRGB (int Id, float R, float G, float B)
{
    register unsigned long	i, j;
    register float		d, diff;
    float			mindiff = 4.0;
    unsigned long		bestpix = 0;
    int				cnt, nr, first_i = 0;
    int				minbits = 6;
    int				minused = MAXINT;
    XColor			col;

    /* if legal color */
    if (R < 0.0 || R > 1.0 ||
	G < 0.0 || G > 1.0 ||
	B < 0.0 || B > 1.0) {
	cacdcmaperrno = BADCOLOR;
	return(0);
    }

    /* do work: first check Id */
    if(Id < 0) {
	cacdcmaperrno = BADID;
	return(0);
    }

    /* if Id does not fit in current array: resize it */
    if(Id >= _tabsize) {
	if(!_ResizeIdTable(Id)) return(0);
    }

    if(_Initialized == 2) {
	col.flags = (char) (DoRed | DoGreen | DoBlue);
	col.red   = R * 65535;
	col.green = G * 65535;
	col.blue  = B * 65535;
	XAllocColor (_Dpy, _cmap, &col);
	bestpix = col.pixel;

	if (Id > _tabmax) _tabmax = Id;
	_Idtable[Id].state = USED;
	_Idtable[Id].pixel = bestpix;
	goto ret;
    }

    /* get best pixel value (GetClosestDisjunctPixel(R,G,B)) */

    if ((nr = _ncolors) > _used_colors) nr = _used_colors;

    /* for _used_colors or less different colors */
    for(i = 0; i < nr; ++i) {

	col.pixel = (*_convint2pix) (i);
	XQueryColor (_Dpy, _cmap, &col);

	/* calculate difference */
	d = R - (float) (col.red   / 65535.0);
	diff = d * d;
	d = G - (float) (col.green / 65535.0);
	diff += (d * d);
	d = B - (float) (col.blue  / 65535.0);
	diff += (d * d);

	/* first criteria: minimize color difference */
	if(diff < mindiff) {
	    bestpix = first_i = i;
	    mindiff = diff;
	    minbits = 0; /* count number of one bits in 'i' */
	    for(j = 1; j < _used_colors; j <<= 1) if(i & j) ++minbits;
	    minused = _pixused[i];
#ifdef DEBUG
PE "bestpix=%3u, minbits=%d, minused=%d, mindiff=%f (1st)\n",
	bestpix, minbits, minused, mindiff);
#endif
	}
	/* second criteria: minimize pixel dependancies */
	else if(diff == mindiff) {
	    cnt = 0; /* count number of one bits in 'i' */
	    for(j = 1; j < _used_colors; j <<= 1) if(i & j) ++cnt;
	    if(cnt < minbits) {
		bestpix = i;
		minbits = cnt;
		minused = _pixused[i];
#ifdef DEBUG
PE "bestpix=%3u, minbits=%d, minused=%d, mindiff=%f (2nd)\n",
	bestpix, minbits, minused, mindiff);
#endif
	    }
	    /* third criteria: minimize usage of same pixel */
	    else if(cnt == minbits && _pixused[i] < minused) {
		bestpix = i;
		minused = _pixused[i];
#ifdef DEBUG
PE "bestpix=%3u, minbits=%d, minused=%d, mindiff=%f (3rd)\n",
	bestpix, minbits, minused, mindiff);
#endif
	    }
	}
    }

    if (_ncolors > _used_colors && first_i == 7) {
	/*
	** If "white" try other planes (third criteria)
	*/
	j = 8; /* start by first possible "white" plane */
	while (j < i) j <<= 1;
	if (j < _ncolors) {
	    if (minbits == 1) j >>= 1;
	    else {
		bestpix = j;
		minused = _pixused[j];
#ifdef DEBUG
PE "bestpix=%3u, minbits=1, minused=%d, mindiff=%f (whites)\n",
	bestpix, minused, mindiff);
#endif
	    }
	    while ((j <<= 1) < _ncolors) {
		if (_pixused[j] < minused) {
		    bestpix = j;
		    minused = _pixused[j];
#ifdef DEBUG
PE "bestpix=%3u, minbits=1, minused=%d, mindiff=%f (whites)\n",
	bestpix, minused, mindiff);
#endif
		}
	    }
	}
    }

    /* set _tabmax */
    if(Id > _tabmax) _tabmax = Id;

    /* couple Id to best pixel */

    /* if Id was already used */
    if(_Idtable[Id].state == USED) {
	_pixused[ (*_convpix2int) (_Idtable[Id].pixel) ]--;
    }
    else {
	++_NIds;
	_Idtable[Id].state = USED;
    }
    _Idtable[Id].pixel = (*_convint2pix) (bestpix);
    _pixused[bestpix]++;

ret:
#ifdef DEBUG
PE "coupled Id=%2d to pixel=%3u (R=%f, G=%f, B=%f) bestpix=%2u\n",
	Id, _Idtable[Id].pixel, R, G, B, bestpix);
#endif
    return(1);
}

static int _ResizeIdTable (int Id)
{
    register IdElem	*ntab;
    register IdElem	*last;
    int			newsize = ((Id + (2 * TABSIZE)) & ~(TABSIZE - 1));

    /* resize Id table */
    if((ntab = Realloc(_Idtable, newsize, IdElem)) == NULL) {
	cacdcmaperrno = NOCORE;
	return(0);
    }

    /* init new part of Id array */
    _Idtable = ntab;
    ntab = _Idtable + _tabsize;
    last = _Idtable + newsize;
    do {
	ntab->state = NOTUSED;
    } while(++ntab != last);

    /* set new table size */
    _tabsize = newsize;

    return(1);
}

int DecoupleIds (int *Ids, int n)
{
    register int i;

    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    /* delete Id's from the table */
    for(i = 0; i < n; ++i) {
	if(!_DecoupleId(Ids[i])) return(0);
    }

    return(1);
}

int DecoupleId (int Id)
{
    /* if not initialized */
    if (!_Initialized) {
	cacdcmaperrno = NOINIT;
	return (0);
    }
    return (_DecoupleId(Id));
}

static int _DecoupleId (int Id)
{
    /* if good Id */
    if (Id >= 0 && Id <= _tabmax && _Idtable[Id].state == USED) {
	_Idtable[Id].state = NOTUSED;
	if (_Initialized != 2)
	_pixused[ (*_convpix2int) (_Idtable[Id].pixel) ]--;
	--_NIds;
	return (1);
    }
    else {	/* Id no good */
	cacdcmaperrno = BADID;
	return (0);
    }
}

int SetForegroundFromIds (GC gc, int *Ids, int n)
{
    unsigned long pix;

    /* if not initialized */
    if (!_Initialized) {
	cacdcmaperrno = NOINIT;
	return (0);
    }

    if (!_GetPixelFromIds (Ids, n, &pix)) return(0);

#ifdef DEBUG
PE "SetForeground pix=%d (0x%x)\n", pix, pix);
#endif

    XSetForeground (_Dpy, gc, pix);

    return (1);
}

int SetForegroundFromId (GC gc, int Id)
{
    return (SetForegroundFromIds (gc, &Id, 1));
}

int SetBackgroundFromIds (GC gc, int *Ids, int n)
{
    unsigned long pix;

    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    if(!_GetPixelFromIds(Ids, n, &pix)) return(0);

    XSetBackground(_Dpy, gc, pix);

    return(1);
}

int SetBackgroundFromId (GC gc, int Id)
{
    return (SetBackgroundFromIds (gc, &Id, 1));
}

int GetAffectedIds (int *Ids, int nids, int *AfIds, int *pnaffs)
{
    register int	i;
    register int	naffs = 0;
    register IdElem	*id;
    unsigned long	pix;
    int			maxaffs = *pnaffs;

    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    /* retrieve combined pixel value for Id's */
    if(!_GetPixelFromIds(Ids, nids, &pix)) return(0);

    /* remove base pixel from pix (else all pixel would match) */
    pix &= ~_basep;

    /* for all Id's */
    for(i = 0, id = _Idtable; i <= _tabmax; ++i, ++id) {
	/* if this pixel is affected */
	if((id->state == USED) && (id->pixel & pix)) {
	    /* if affected id array is completely filled */
	    if(naffs >= maxaffs) {
		cacdcmaperrno = ARRAYTOSMALL;
		return(0);
	    }
	    AfIds[naffs++] = i;
	}
    }

    *pnaffs = naffs;
    return(1);
}

int GetPixelFromId (int Id, unsigned long *pixel)
{
    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    return(_GetPixelFromIds(&Id, 1, pixel));
}

int GetPixelFromIds (int *Ids, int n, unsigned long *pixel)
{
    /* if not initialized */
    if(!_Initialized) {
	cacdcmaperrno = NOINIT;
	return(0);
    }

    return(_GetPixelFromIds(Ids, n, pixel));
}

static int _GetPixelFromIds (int *Ids, int n, unsigned long *pixel)
{
    register int		i;
    register int		id;
    register unsigned long	pix = 0;

    /* OR pixels associated with Id's together */
    for(i = 0; i < n; ++i) {
	id = Ids[i];
	/* if good id */
	if(id >= 0 && id <= _tabmax && _Idtable[id].state == USED) {
	    pix |= _Idtable[id].pixel;
	}
	else {	/* Id no good */
	    cacdcmaperrno = BADID;
	    return(0);
	}
    }

    *pixel = pix;
    return(1);
}

/* convert virtual cmap index ('ind') to real pixel value */
/* (for contiguous bitplanes) */
static unsigned long _ContigIntToPixel (unsigned long ind)
{
    return ((ind << _basesh) | _basep);
}

/* convert real pixel value to virtual cmap index ('ind') */
/* (for contiguous bitplanes) */
static unsigned long _ContigPixelToInt (unsigned long pixel)
{
    return ((pixel & ~_basep) >> _basesh);
}

/* convert virtual cmap index ('ind') to real pixel value */
/* (for non-contiguous bitplanes) */
static unsigned long _NonContigIntToPixel (register unsigned long ind)
{
    register unsigned long      pixel = 0;
    register unsigned long      real_ind = 1;
    register unsigned long      mask = _planemask;

    while (ind) {
	while (!(mask & real_ind)) real_ind <<= 1;

	if (ind & 0x1) pixel |= real_ind;

	ind >>= 1;
	real_ind <<= 1;
    }
    return (pixel | _basep);
}

/* convert virtual cmap indices ('ind') to real pixels values */
/* convert real pixel value to virtual cmap index ('ind') */
/* (for non-contiguous bitplanes) */
static unsigned long _NonContigPixelToInt (register unsigned long pixel)
{
    register unsigned long      vir_ind = 1;
    register unsigned long      mask = _planemask;
    register unsigned long      ind = 0;

    pixel &= ~_basep;

    while (pixel) {
	while (!(mask & 0x1)) {
	    mask >>= 1;
	    pixel >>= 1;
	}
	if (pixel & 0x1) ind |= vir_ind;

	vir_ind <<= 1;
	mask >>= 1;
	pixel >>= 1;
    }
    return (ind);
}
