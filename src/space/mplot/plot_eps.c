/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Nick van der Meijs
 *	Arjan van Genderen
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

#include "src/space/mplot/config.h"
#include <string.h>
#include <errno.h>
#include <math.h>

#define StartPolygon(x,y) fprintf (fpPlot, "%d %d P\n", x, y)
#define LineTo(x,y)       fprintf (fpPlot, "%d %d T\n", x, y)
#define LastPoint(x,y)    fprintf (fpPlot, "%d %d L\n", x, y)
#define PlotRect(xl, yb, dx, dy)   fprintf (fpPlot, "%d %d %d %d B\n", xl, yb, dx, dy)

#define BeginRect(x0, y0, dx, dy)  fprintf (fpPlot, "%d %d %d %d R\n", x0, y0, dx, dy)
#define ContRect(dx, dy)  fprintf (fpPlot, "%d %d X\n", dx, dy)
#define EndRect(dx, dy)   fprintf (fpPlot, "%d %d C\n", dx, dy)

extern long bxl, bxr, byb, byt;
static int scale = 1;
static int busy_layer = 0;

static FILE * fpPlot;
static FILE * fpTech;

/* A4 dimensions, in points (= 1/72 inch) */
static double BbLeft   = 28,
              BbBot    = 28,
              BbRight  = 566,
              BbTop    = 807;

static bool_t doRotate = FALSE;

void plotSetRotation ()
{
    doRotate = TRUE;
}

static double lambdaScale = 0.0;

/* explicitely set scaling, in lambda/cm */
void plotSetLambda (char *s)
{
    lambdaScale = 2.54 * 72.0 / atof (s);
}

/* set width of drawing region, in cm */
void plotSetDrawWidth (char *s)
{
    double w = atof (s) / 2.54 * 72.0;
    double v = BbRight - BbLeft;
    BbRight -= (v - w)/2;
    BbLeft  += (v - w)/2;
}

/* set heigth of drawing region, in cm */
void plotSetDrawHeight (char *s)
{
    double h = atof (s) / 2.54 * 72.0;
    double v = BbTop - BbBot;
    BbTop -= (v - h)/2;
    BbBot += (v - h)/2;
}

void mplotInit (DM_CELL *key)
{
    DM_STREAM *info;
    double PicHeight, PicWidth;
    double BbWidth, BbHeight;
    double ScaleFactor;

    fpPlot = cfopen (mprintf ("%s.eps", key -> cell), "w");

    info = dmOpenStream (key, "info3", "r");
    dmGetDesignData (info, GEO_INFO3);
    dmCloseStream (info, COMPLETE);

    if (ginfo3.nr_samples != 0)
	scale = ginfo3.nr_samples;
    else
	scale = SCALE;

    bxl = Scale (ginfo3.bxl);
    bxr = Scale (ginfo3.bxr);
    byb = Scale (ginfo3.byb);
    byt = Scale (ginfo3.byt);

    PicHeight = byt - byb;
    PicWidth  = bxr - bxl;

    if (doRotate) {
	double t = PicHeight; PicHeight = PicWidth; PicWidth = t;
    }

    BbLeft   = ceil (BbLeft + 0.5);
    BbBot    = ceil (BbBot + 0.5);
    BbRight  = ceil (BbRight + 0.5);
    BbTop    = ceil (BbTop + 0.5);
    BbWidth  = BbRight - BbLeft;
    BbHeight = BbTop - BbBot;

    if (lambdaScale == 0) {
	double WidthScale, HeightScale;
	WidthScale  = BbWidth  / PicWidth;
	HeightScale = BbHeight / PicHeight;
	ScaleFactor = WidthScale < HeightScale ? WidthScale : HeightScale;
    }
    else {
	ScaleFactor = lambdaScale / scale;
    }

    /* now move to center of paper */
    BbLeft  += (BbWidth  - PicWidth * ScaleFactor) / 2.0;
    BbWidth  = PicWidth * ScaleFactor;
    BbRight  = BbLeft + BbWidth;
    BbBot   += (BbHeight - PicHeight * ScaleFactor) / 2.0;
    BbHeight = PicHeight * ScaleFactor;
    BbTop    = BbBot + BbHeight;

    /* make prolog */

    doEpsProlog ((long) BbLeft, (long) BbBot, (long) (BbRight), (long) (BbTop));

    if (doRotate) fprintf (fpPlot, "/Rotate true def\n");

    fprintf (fpPlot, "[%ld %ld %ld %ld %ld %ld %ld %ld %d] BeginLayout\n",
	(long) BbLeft, (long) BbBot, (long) (BbRight), (long) (BbTop),
	bxl, byb, bxr, byt,
	Scale (1)); /* Resolution */
}

void mplotEnd ()
{
    endLayer ();
    fprintf (fpPlot, "EndLayout\n");

    fclose (fpPlot), fpPlot = NULL;
}

void plotBbox ()
{
    fprintf (fpPlot, "PlotBBox\n");
}

void plotPass (int pass)
{
    endLayer ();
    fprintf (fpPlot, "%d plotPass\n", pass);
}

void endLayer ()
{
    if (busy_layer) fprintf (fpPlot, "EndLayer\n");
    busy_layer = 0;
}

void setLayer (char *layer)
{
    fprintf (fpPlot, "(%s) SetLayer\n", layer);
    busy_layer = 1;
}

void plotMask (char *name, int color)
{
    endLayer ();
    setLayer (name);
}

/* Postscript: xl xr yb yt dx dy termname plotTerminal */

void plotTerminal (long xl, long xr, long yb, long yt, char *name)
{
    fprintf (fpPlot, "%ld %ld %ld %ld 0 0 (%s) plotTerminal\n",
	Scale (xl), Scale (xr), Scale (yb), Scale (yt), name);
}

void mplotLine (long x1, long y1, long x2, long y2, int mode)
{
    fprintf (fpPlot, "%ld %ld %ld %ld %d plotArrow\n",
	Scale (x1), Scale (y1), Scale (x2), Scale (y2), mode);
}

void plotText (long x, long y, int orient, char *text)
{
    fprintf (fpPlot, "%ld %ld %d (%s) plotText\n",
	Scale (x), Scale (y), orient, text);
}

static void plotRect (edge_t *edge)
{
    edge_t * e;
    int x0 = edge -> xr, y0 = edge -> yr;
    int x = edge -> xl, y = edge -> link -> yl;

    if (edge -> link -> link == NULL) { /* a rectangle */
	PlotRect (x, y, x0-x, y0-y);
	return;
    }

    BeginRect (x0, y0, x - x0, y - y0);

    e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    while ((e = edge -> link)) {
	int ox = x, oy = y;
	x = (edge -> xl == x) ? edge -> xr : edge -> xl;
	y = e -> yl;
	if (edge -> link -> link)
	    ContRect (x - ox, y - oy);
	else
	    EndRect (x - ox, y - oy);
	DISPOSE (edge, sizeof(edge_t)); edge = e;
    }

    DISPOSE (edge, sizeof(edge_t));
}

static void plotPolygon (edge_t *edge)
{
    edge_t * e;
    int x0 = edge -> xr, y0 = edge -> yr;
    int x  = edge -> xl, y  = edge -> yl;

    StartPolygon (x0, y0);
    LineTo (x, y);

    e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    while (edge) {
        if (edge -> xl == x) {
	    if (edge -> yl != y) {	/* vert */
		x = edge -> xl, y = edge -> yl;
		LineTo (x, y);
	    }
	    x = edge -> xr, y = edge -> yr;
	    if (edge -> link)	/* do not close contour */
		LineTo (x, y);
	    else
		LastPoint (x, y);
	}
	else {
	    ASSERT (edge -> xr == x);
	    if (edge -> yr != y) {	/* vert */
		x = edge -> xr, y = edge -> yr;
		LineTo (x, y);
	    }
	    x = edge -> xl, y = edge -> yl;
	    if (edge -> link)	/* do not close contour */
		LineTo (x, y);
	    else
		LastPoint (x, y);
	}
	e = edge -> link; DISPOSE (edge, sizeof(edge_t)); edge = e;
    }
}

void plotContour (edge_t *edge)
{
    edge_t * e;

    for (e = edge; e; e = e -> link) {
	if (e -> yl != e -> yr) {
	    plotPolygon (edge);
	    return;
	}
    }
    plotRect (edge);
}

void printLinks (edge_t *edge) /* print contour (use when in debugger) */
{
    while (edge) {
        printf ("%d %d %d %d\n", edge -> xl, edge -> xr, edge -> yl, edge -> yr);
	edge = edge -> link;
    }
}

/*
 * Handle file inclusion directive
 * syntax: %%Include <filename>
 *     or: %%Include "filename"
 * First form only seeks in icdpath/share/lib, second first locally.
 * Analogous to cpp convention.
 */
Private void techFileInclude (char *buf, FILE *fp)
{
    char includename [1024];
    char filename [1024];
    FILE * fpInclude;
    int c;
    extern int errno;

    if (sscanf (buf, "%*s \"%[^\"]\"", filename) == 1) {
	if ((fpInclude = fopen (filename, "r")) == NULL) {
	    if (errno == ENOENT && filename [0] != '/') {
		strcpy (includename, filename);
		goto LibInclude;
	    }
	    else {
		say ("inclusion of eps prolog file \"%s\" failed (%s)",
		    filename, strerror(errno));
		return;
	    }
	}
    }
    else if (sscanf (buf, "%*s <%[^>]>", includename) == 1) {
LibInclude:
	sprintf (filename, "%s/share/lib/%s", icdpath, includename);
	if ((fpInclude = fopen (filename, "r")) == NULL) {
	    say ("inclusion of eps prolog file \"%s\" failed (%s)",
		filename, strerror(errno));
	    return;
	}
    }
    else {
	/* chop terminating \n, if present */
	c = strlen (buf) - 1;
	if (buf[c] == '\n') buf[c] = '\0';
	say ("Syntax error in eps prolog include directive: '%s'", buf);
	return;
    }

    ASSERT (fpInclude);

    fprintf (fp, "%% begin inclusion of %s\n", filename);

    while ((c = fgetc (fpInclude)) != EOF) putc (c, fp);

    fprintf (fp, "%% end inclusion of %s\n", filename);

    fclose (fpInclude);
}

void doEpsProlog (long l, long b, long r, long t)
{
    char buf[200];

    fputs ("%!PS-Adobe-1.0\n", fpPlot);
    fprintf (fpPlot, "%%%%BoundingBox: %ld %ld %ld %ld\n", l, b, r, t);
    fputs ("%%Pages: 1\n", fpPlot);
    fputs ("%%EndComments\n", fpPlot);

    rewind (fpTech);

    while (fgets (buf, sizeof (buf), fpTech) == buf) {

	if (strncmp (buf, "%%Include", 9) == 0) {
	    techFileInclude (buf, fpPlot);
	}
	else {
	    fputs (buf, fpPlot);
	}
    }
}

#define TBASENM "epslay"
#define TSUFFIX "def"

Private FILE * openTechFile (DM_PROJECT *dmproject, char *techDef, char *techFile)
{
    FILE * cfopen ();

    if (!techDef) techDef = TSUFFIX;

    if (!techFile) {
	techFile = dmGetMetaDesignData (PROCPATH, dmproject, mprintf ("%s.%s", TBASENM, techDef));

	if (access (techFile, 0) != 0) {
	    say ("%s.%s: no such element definition in %s",
		TBASENM, techDef, dmGetMetaDesignData (PROCPATH, dmproject, ""));
	    die ();
	}
    }

    if (techFile) {
	verbose ("technology file: %s\n", techFile);
	return (cfopen (techFile, "r"));
    }

    say ("can't open technology file");
    die ();

    return (NULL);
}

void doEpsTechnology (DM_PROJECT *project, char *techDef, char *techFile, int *order_return, int *restroke_return)
{
    int i, j;
    char *name, *p;
    char buf[100];
    DM_PROCDATA * process;

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    fpTech = openTechFile (project, techDef, techFile);
    if (!fpTech) {
	fprintf (stderr, "error: Unable to open technology file `%s'.\n", techFile);
	exit (1);
    }

    for (i = 0; i < process -> nomasks; i++) {
	order_return[i] = -1;
	restroke_return[i] = -1;
    }

    while (fgets (buf, sizeof (buf), fpTech) == buf) {
	if (strncmp (buf, "%%Order:", 8) == 0) {
	    p = buf + 8;
	    j = 0;
	    while ((name = strtok (p, " \t\n"))) {
		p = NULL;
		for (i = 0; i < process -> nomasks; i++) {
		    if (strsame (name, process -> mask_name[i])) {
			restroke_return[j] = i;
			order_return[j++] = i;
			break;
		    }
		}
		if (i == process -> nomasks) {
		    say ("Illegal mask name '%s' in %s directive",
			name, "%%Order");
		}
	    }
	}
	else if (strncmp (buf, "%%Filled:", 9) == 0) {
	    p = buf + 9;
	    j = 0;
	    while ((name = strtok (p, " \t\n"))) {
		p = NULL;
		for (i = 0; i < process -> nomasks; i++) {
		    if (strsame (name, process -> mask_name[i])) {
			order_return[j++] = i;
			break;
		    }
		}
		if (i == process -> nomasks) {
		    say ("Illegal mask name '%s' in %s directive",
			name, "%%Filled");
		}
	    }
	}
	else if (strncmp (buf, "%%Restroked:", 12) == 0) {
	    p = buf + 12;
	    j = 0;
	    while ((name = strtok (p, " \t\n"))) {
		p = NULL;
		for (i = 0; i < process -> nomasks; i++) {
		    if (strsame (name, process -> mask_name[i])) {
			restroke_return[j++] = i;
			break;
		    }
		}
		if (i == process -> nomasks) {
		    say ("Illegal mask name '%s' in %s directive",
			name, "%%Stroked");
		}
	    }
	}
    }
}
