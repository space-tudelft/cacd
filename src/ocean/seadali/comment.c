/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Alfred van der Hoeven
 *	Pieter van der Wolf
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

#include "src/ocean/seadali/header.h"
#include <sys/stat.h>

typedef struct {
    Coor x1, y1, x2, y2;
    int arrow_mode;
} Line;

typedef struct {
    Coor x, y;
    char text[MAXCHAR];
    int orient;
} Text;

typedef struct {
    Coor x, y;
    int maskno;
    char name[MAXCHAR];
    char class[MAXCHAR];
    int orient;
} Label;

typedef struct comment {
    int type;
    union { /* to be extended with other elements */
	Line  line;
	Text  text;
	Label label;
    } object;
    struct comment *next;
    struct comment *prev;
} Comment;

#define LINEOBJ  (element -> object.line)
#define TEXTOBJ  (element -> object.text)
#define LABELOBJ (element -> object.label)

extern char **lay_names;
extern int   NR_lay;
extern int  *pict_arr;
extern int   Textnr;
extern float c_cW, c_cH;

static Comment *firstElement = NULL;
static Comment *lastElement = NULL;

static void add_comment (Comment *element);
static void show_line (float x1, float y1, float x2, float y2, int arrow_mode);
static void show_text (float x, float y, char *text, int mode);

void c_line (Coor x1, Coor y1, Coor x2, Coor y2, int arrow_mode)
{
    Comment *element;

    MALLOC (element, Comment);
    element -> type = GA_LINE;
    LINEOBJ.arrow_mode = arrow_mode;
    LINEOBJ.x1 = x1;
    LINEOBJ.y1 = y1;
    LINEOBJ.x2 = x2;
    LINEOBJ.y2 = y2;
    add_comment (element);
    pict_arr[Textnr] = DRAW;
}

void c_text (Coor x, Coor y, int orient)
{
    Comment *element;

    MALLOC (element, Comment);
    element -> type = GA_TEXT;
    ask_string ("enter_text: ", TEXTOBJ.text);

    if (strchr (TEXTOBJ.text, '~')) {
	ptext ("text contains illegal character '~'");
	FREE (element);
	return;
    }
    TEXTOBJ.orient = orient;
    TEXTOBJ.x = x;
    TEXTOBJ.y = y;
    add_comment (element);
    pict_arr[Textnr] = DRAW;
}

void c_label (Coor x, Coor y, int orient)
{
    Comment *element;
    int lay, maskno;
    char *p1, *p2, *p3, *p4;
    char buf[MAXCHAR];

    ask_string ("enter_label: ", buf);

    p1 = buf;
    p2 = p1;
    while (*p2 && *p2 != ':') p2++;
    if (*p2) *p2 = '\0', p2++;
    p3 = p2;
    while (*p3 && *p3 != ':') p3++;
    if (*p3) *p3 = '\0', p3++;
    p4 = p3;
    while (*p4 && *p4 != ':') p4++;
    if (*p4) *p4 = '\0', p4++;

    if (!(*p1 && *p3)) {
	ptext ("enter label as \"name:class:mask\"");
	return;
    }

    maskno = -1;
    if (p3[0] == '#') {
	/* In some special cases, space may generate labels
	   for new masks.  In that case, the mask number is used.
	*/
	sscanf (p3 + 1, "%d", &maskno);
    }
    else {
	for (lay = 0; lay < NR_lay; ++lay) {
	    if (strcmp (lay_names [lay], p3) == 0) {
		maskno = lay;
		break;
	    }
	}
    }
    if (maskno < 0) {
	ptext ("illegal mask in \"name:class:mask\"");
	return;
    }

    MALLOC (element, Comment);
    element -> type = GA_LABEL;
    strcpy (LABELOBJ.name, p1);
    strcpy (LABELOBJ.class, p2);
    LABELOBJ.maskno = maskno;
    LABELOBJ.orient = orient;
    LABELOBJ.x = x;
    LABELOBJ.y = y;
    add_comment (element);
    pict_arr[Textnr] = DRAW;
}

static float comm_dist (Coor x, Coor y, Comment *element)
{
    float d1, d2;

    switch (element -> type) {
    case GA_LINE:
	d1 = sqrt ((double) ((LINEOBJ.x1 - x) * (LINEOBJ.x1 - x) + (LINEOBJ.y1 - y) * (LINEOBJ.y1 - y)));
	d2 = sqrt ((double) ((LINEOBJ.x2 - x) * (LINEOBJ.x2 - x) + (LINEOBJ.y2 - y) * (LINEOBJ.y2 - y)));
	return Min (d1, d2);
    case GA_TEXT:
	d1 = sqrt ((double) ((TEXTOBJ.x - x) * (TEXTOBJ.x - x) + (TEXTOBJ.y - y) * (TEXTOBJ.y - y)));
	return d1;
    case GA_LABEL:
	d1 = sqrt ((double) ((LABELOBJ.x - x) * (LABELOBJ.x - x) + (LABELOBJ.y - y) * (LABELOBJ.y - y)));
	return d1;
    }
    return 20;
}

static void del_element (Comment *element)
{
    if (element) {
	if (firstElement == element) firstElement = element -> next;
	if (lastElement  == element) lastElement  = element -> prev;
	if (element -> next) element -> next -> prev = element -> prev;
	if (element -> prev) element -> prev -> next = element -> next;
	FREE (element);
	pict_arr[Textnr] = ERAS_DR;
    }
}

void del_comment (Coor x, Coor y)
{
    float dist;
    Comment *element;

    for (element = firstElement; element; element = element -> next) {
	dist = comm_dist (x, y, element);
	if (dist < c_cW + c_cH) {
	    del_element (element);
	    return;
	}
    }
}

void draw_all_comments ()
{
    Comment *element;
    char buf[MAXCHAR+MAXCHAR+MAXCHAR];
    int maskno;

    for (element = firstElement; element; element = element -> next) {
	switch (element -> type) {
	case GA_LINE:
	    show_line ((float) LINEOBJ.x1, (float) LINEOBJ.y1,
		(float) LINEOBJ.x2, (float) LINEOBJ.y2, LINEOBJ.arrow_mode);
	    break;
	case GA_TEXT:
	    show_text ((float) TEXTOBJ.x, (float) TEXTOBJ.y, TEXTOBJ.text, TEXTOBJ.orient);
	    break;
	case GA_LABEL:
	    maskno = LABELOBJ.maskno;
	    if (maskno >= 0 && maskno < NR_lay) {
		sprintf (buf, "%s:%s:%s", LABELOBJ.name, LABELOBJ.class, lay_names[maskno]);
	    }
	    else {
		/* In some special cases, space may generate labels
		   for new masks.  In that case, use the mask number.
		*/
		sprintf (buf, "%s:%s:#%d", LABELOBJ.name, LABELOBJ.class, maskno);
	    }
	    show_text ((float) LABELOBJ.x, (float) LABELOBJ.y, buf, LABELOBJ.orient);
	    break;
	}
    }
}

static void add_comment (Comment *element)
{
    element -> prev = lastElement;
    element -> next = NULL;
    if (firstElement) lastElement -> next = element;
    else firstElement = element;
    lastElement = element;
}

static void show_text (float x, float y, char *text, int mode)
{
    int strLen = strlen (text);

    switch (mode) {
    case GA_LEFT:
	break;
    case GA_RIGHT:
	x -= (strLen * c_cW);
	break;
    case GA_CENTER:
	x -= (strLen * c_cW) / 2;
	break;
    }
    d_text (x, y, text);
}

static void rotate (float xin, float yin, double angle, float *xout, float *yout)
{
    *xout =  cos (angle) * xin + sin (angle) * yin;
    *yout = -sin (angle) * xin + cos (angle) * yin;
}

static void show_line (float x1, float y1, float x2, float y2, int arrow_mode)
{
    float width, length, dx, dy, newdx, newdy;

    width = 3 * c_cW; /* three times current character width */

    dx = x2 - x1;
    dy = y2 - y1;
    length = sqrt (dx * dx + dy * dy);

    dx = (dx / length) * width;
    dy = (dy / length) * width;

    d_line (x1, y1, x2, y2);

#define ARRANGLE 0.523599

    if (arrow_mode & GA_FW_ARROW) {
	rotate (-dx, -dy, ARRANGLE, &newdx, &newdy);
	d_line (x2, y2, x2 + newdx, y2 + newdy);
	rotate (-dx, -dy, -ARRANGLE, &newdx, &newdy);
	d_line (x2, y2, x2 + newdx, y2 + newdy);
    }
    if (arrow_mode & GA_BW_ARROW) {
	rotate (dx, dy, ARRANGLE, &newdx, &newdy);
	d_line (x1, y1, x1 + newdx, y1 + newdy);
	rotate (dx, dy, -ARRANGLE, &newdx, &newdy);
	d_line (x1, y1, x1 + newdx, y1 + newdy);
    }
}

int inp_comment (DM_CELL *cellKey)
{
    struct stat statBuf;
    Comment *element;
    DM_STREAM *dmfp;
    int type;
    int x1, y1, x2, y2, x, y;

    if (dmStat (cellKey, "annotations", &statBuf) == -1) return 0;

    if (!(dmfp = dmOpenStream (cellKey, "annotations", "r"))) return -1;

    /* format record first */
    if (dmGetDesignData (dmfp, GEO_ANNOTATE) > 0) {
	ASSERT (ganno.type == GA_FORMAT);
	ASSERT (ganno.o.format.fmajor == 1);
	ASSERT (ganno.o.format.fminor == 1);
    }

    while (dmGetDesignData (dmfp, GEO_ANNOTATE) > 0) {
	MALLOC (element, Comment);
	switch (ganno.type) {
	    case GA_LINE:
		element -> type = GA_LINE;
		LINEOBJ.x1 = ganno.o.line.x1 * QUAD_LAMBDA;
		LINEOBJ.y1 = ganno.o.line.y1 * QUAD_LAMBDA;
		LINEOBJ.x2 = ganno.o.line.x2 * QUAD_LAMBDA;
		LINEOBJ.y2 = ganno.o.line.y2 * QUAD_LAMBDA;
		LINEOBJ.arrow_mode = ganno.o.line.mode;
		break;
	    case GA_TEXT:
		element -> type = GA_TEXT;
		strcpy (TEXTOBJ.text, ganno.o.text.text);
	        TEXTOBJ.x = ganno.o.text.x * QUAD_LAMBDA;
	        TEXTOBJ.y = ganno.o.text.y * QUAD_LAMBDA;
		if (ganno.o.text.ax < 0)
		    TEXTOBJ.orient = GA_LEFT;
		else if (ganno.o.text.ax > 0)
		    TEXTOBJ.orient = GA_RIGHT;
		else
		    TEXTOBJ.orient = GA_CENTER;
		break;
	    case GA_LABEL:
		element -> type = GA_LABEL;
		strcpy (LABELOBJ.name, ganno.o.Label.name);
		strcpy (LABELOBJ.class, ganno.o.Label.Class);
	        LABELOBJ.x = ganno.o.Label.x * QUAD_LAMBDA;
	        LABELOBJ.y = ganno.o.Label.y * QUAD_LAMBDA;
		if (ganno.o.Label.ax < 0)
		    LABELOBJ.orient = GA_LEFT;
		else if (ganno.o.Label.ax > 0)
		    LABELOBJ.orient = GA_RIGHT;
		else
		    LABELOBJ.orient = GA_CENTER;
		LABELOBJ.maskno = ganno.o.Label.maskno;
		break;
        }
	add_comment (element);
    }
    dmCloseStream (dmfp, COMPLETE);
    return 0;

err:
    dmCloseStream (dmfp, QUIT);
    return -1;
}

int outp_comment (DM_CELL *ckey)
{
    DM_STREAM *dmfp;
    Comment *element;

    if (!(dmfp = dmOpenStream (ckey, "annotations", "w"))) return FALSE;

    /* format record first */
    ganno.type = GA_FORMAT;
    ganno.o.format.fmajor = ganno.o.format.fminor = 1;
    dmPutDesignData (dmfp, GEO_ANNOTATE);

    for (element = firstElement; element; element = element -> next) {
	switch (element -> type) {
	    case GA_LINE:
		ganno.type = GA_LINE;
		ganno.o.line.mode = LINEOBJ.arrow_mode;
		ganno.o.line.x1 = LINEOBJ.x1 / (double)QUAD_LAMBDA;
		ganno.o.line.y1 = LINEOBJ.y1 / (double)QUAD_LAMBDA;
		ganno.o.line.x2 = LINEOBJ.x2 / (double)QUAD_LAMBDA;
		ganno.o.line.y2 = LINEOBJ.y2 / (double)QUAD_LAMBDA;
		break;
	    case GA_TEXT:
		ganno.type = GA_TEXT;
		strcpy (ganno.o.text.text, TEXTOBJ.text);
		ganno.o.text.x = TEXTOBJ.x / (double)QUAD_LAMBDA;
		ganno.o.text.y = TEXTOBJ.y / (double)QUAD_LAMBDA;
		ganno.o.text.ay = 0;
		switch (TEXTOBJ.orient) {
		    case GA_CENTER: ganno.o.text.ax =  0; break;
		    case GA_LEFT:   ganno.o.text.ax = -1; break;
		    case GA_RIGHT:  ganno.o.text.ax =  1; break;
		}
		break;
	    case GA_LABEL:
		ganno.type = GA_LABEL;
		strcpy (ganno.o.Label.name, LABELOBJ.name);
		strcpy (ganno.o.Label.Class, LABELOBJ.class);
		*ganno.o.Label.Attributes = '\0';
		ganno.o.Label.x = LABELOBJ.x / (double)QUAD_LAMBDA;
		ganno.o.Label.y = LABELOBJ.y / (double)QUAD_LAMBDA;
		ganno.o.Label.ay = 0;
		ganno.o.Label.maskno = LABELOBJ.maskno;
		switch (LABELOBJ.orient) {
		    case GA_CENTER: ganno.o.Label.ax =  0; break;
		    case GA_LEFT:   ganno.o.Label.ax = -1; break;
		    case GA_RIGHT:  ganno.o.Label.ax =  1; break;
		}
		break;
	    default:
	       fprintf (stderr, "bad annotation type %d\n", element -> type);
	       break;
        }
	dmPutDesignData (dmfp, GEO_ANNOTATE);
    }
    dmCloseStream (dmfp, COMPLETE);
    return TRUE;
}

int comment_win (Coor *ll, Coor *rr, Coor *bb, Coor *tt)
{
    Comment *element;

    if (!firstElement) return 0;

    element = firstElement;
    /* It is necessary to initialize the bounding box */
    switch (element -> type) {
    case GA_LINE:
	*ll = *rr = LINEOBJ.x1;
	*bb = *tt = LINEOBJ.y1;
	break;
    case GA_TEXT:
	*ll = *rr = TEXTOBJ.x;
	*bb = *tt = TEXTOBJ.y;
	break;
    case GA_LABEL:
	*ll = *rr = LABELOBJ.x;
	*bb = *tt = LABELOBJ.y;
	break;
    }

    for (element = firstElement; element; element = element -> next) {
	switch (element -> type) {
	    case GA_LINE:
		*ll = Min (*ll, Min (LINEOBJ.x1, LINEOBJ.x2));
		*rr = Max (*rr, Max (LINEOBJ.x1, LINEOBJ.x2));
		*bb = Min (*bb, Min (LINEOBJ.y1, LINEOBJ.y2));
		*tt = Max (*tt, Max (LINEOBJ.y1, LINEOBJ.y2));
		break;
	    case GA_TEXT:
		*ll = Min (*ll, TEXTOBJ.x - QUAD_LAMBDA);
		*rr = Max (*rr, TEXTOBJ.x + QUAD_LAMBDA);
		*bb = Min (*bb, TEXTOBJ.y - QUAD_LAMBDA);
		*tt = Max (*tt, TEXTOBJ.y + QUAD_LAMBDA);
		break;
	    case GA_LABEL:
		*ll = Min (*ll, LABELOBJ.x - QUAD_LAMBDA);
		*rr = Max (*rr, LABELOBJ.x + QUAD_LAMBDA);
		*bb = Min (*bb, LABELOBJ.y - QUAD_LAMBDA);
		*tt = Max (*tt, LABELOBJ.y + QUAD_LAMBDA);
		break;
        }
    }
    return 1;
}

int no_comments ()
{
    return (firstElement ? FALSE : TRUE);
}

void empty_comments ()
{
    while (firstElement) del_element (firstElement);
}
