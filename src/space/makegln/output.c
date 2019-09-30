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

#include "src/space/makegln/config.h"
#include <stdio.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "src/libddm/dmincl.h"
#include "src/space/auxil/auxil.h"
#include "src/space/makegln/makegln.h"
#include "src/space/makegln/proto.h"

static ecnt_t infoTotalInQueue = 0;
static ecnt_t infoMaxInQueue = 0;
static ecnt_t infoNowInTemp = 0;
static ecnt_t infoMaxInTemp = 0;
static ecnt_t infoTotalOut = 0;
static long infoMaxTempSize = 0;

#ifdef DEBUG_OUTPUT
#define MAX_IN_QUEUE	11
#else	/* not DEBUG_OUTPUT */
#define MAX_IN_QUEUE	20000
#endif	/* not DEBUG_OUTPUT */

static edge_t * head = NULL, *tail = NULL;
static DM_STREAM * outputStream = NULL;
static ecnt_t  ready_in_queue = 0;
static edge_t * middle = NULL;
static FILE * tmp_fp = NULL;
static char *tmp_name;

static coor_t cursor_x = 0;
static coor_t cursor_y = 0;

static char * compressCmd = "compress";
static char * unCompressCmd = "uncompress";

/* local operations */
Private void queueWriteEdge (FILE *fp, edge_t *e);
Private edge_t *queueReadEdge (FILE *fp);
Private void initQueue (void);
Private void flushQueue (void);
Private long fileSize (char *name);
Private int found_cmds (char *cmd1, char *cmd2);

void openOutput (DM_CELL *cellKey, char *mask)
{
    outputStream = dmOpenStream (cellKey, mprintf ("%s_gln", mask), "w");
    initQueue ();
}

void closeOutput ()
{
    flushQueue ();
    dmCloseStream (outputStream, COMPLETE), outputStream = NULL;
}

void selectForOutput (edge_t *edge)
{
    Debug (printEdge ("enque", edge));
    /*
     * Can combine ready and remain in one status field.
     * This can save memory.
     */
    edge -> ready = FALSE;
    edge -> remain = FALSE;
    if (tail) {
	tail -> next = edge;
	tail = edge;
    }
    else  {
	head = tail = edge;
    }
    infoTotalInQueue++;
    infoMaxInQueue = Max (infoMaxInQueue, infoTotalInQueue);
}

void readyForOutput (edge_t *edge)
{
    Debug (printEdge ("ready", edge));
    edge -> ready = TRUE;
    if (edge -> remain == FALSE) ready_in_queue++;
}

void scanAdvance ()
{
    edge_t *e, *hlp, *hlp_prev;

    Debug (fprintf (stderr, "ScanAdvance start\n"));

    while (head && (head -> remain == FALSE) && (head -> ready == TRUE)) {
	e = head, head = head -> next;
	Debug (printEdge ("out", e));
	ggln.xl = e -> xl;
	ggln.yl = e -> yl;
	ggln.xr = e -> xr;
	ggln.yr = e -> yr;

#if 0
#ifndef MAKEMESH
        /* Note: with makemesh gglns extending the bounding box may
	   be input because of an extension of the substrate. */
	if (ggln.xl < bxl || ggln.xr > bxr
	||  ggln.yl < byb || ggln.yl > byt
	||  ggln.yr < byb || ggln.yr > byt) {
#ifdef MAKESIZE
	    if (ggln.yl != byb && ggln.yl != byt)
#endif
            say ("bounding box inconsistency (possibly error in db)");
	}
#endif
#endif

#ifdef MAKESIZE
	/* If edge is not an edge used for shrinking the layout */
	if (ggln.yl != byb && ggln.yl != byt)
#endif /* MAKESIZE */
	{
#ifndef MAKEMESH
	    if (ggln.xl < bxl) say ("bounding box inconsistency (ggln.xl < bxl)");
	    if (ggln.xr > bxr) say ("bounding box inconsistency (ggln.xr > bxr)");
	    if (ggln.yl < byb) say ("bounding box inconsistency (ggln.yl < byb)");
	    if (ggln.yl > byt) say ("bounding box inconsistency (ggln.yl > byt)");
	    if (ggln.yr < byb) say ("bounding box inconsistency (ggln.yr < byb)");
	    if (ggln.yr > byt) say ("bounding box inconsistency (ggln.yr > byt)");
#endif /* MAKEMESH */
	    if (!optNoOutput) dmPutDesignData (outputStream, GEO_GLN);
	    infoTotalOut++;
	}
	ready_in_queue--;

	disposeEdge (e);
	infoTotalInQueue--;
	middle = head;
    }

    if (middle == NULL) middle = head;
    hlp_prev = hlp = middle;

    while (ready_in_queue > MAX_IN_QUEUE) {
	while (hlp && (hlp -> ready == FALSE || hlp -> remain == TRUE)) {
	    hlp -> remain = TRUE;
	    hlp_prev = hlp;
	    hlp = hlp -> next;
	}

	while (hlp && hlp -> ready == TRUE) {
	    if (hlp -> remain == FALSE) {
		queueWriteEdge (tmp_fp, hlp);
		hlp_prev -> next = hlp -> next;
		disposeEdge (hlp);
		infoTotalInQueue--;
		hlp = hlp_prev -> next;

		ready_in_queue--;
	    }
	    else {
		ASSERT (0);
		hlp_prev = hlp;
		hlp = hlp -> next;
	    }
	}
    }

    if (hlp == NULL) tail = hlp_prev;
    middle = hlp_prev;

    if (head == NULL) tail = NULL;

    Debug (fprintf (stderr, "ScanAdvance end\n"));
}

/* queueWriteEdge - put an edge in tmp file
 *
 * Write differences instead of absolute coordinates, to save space.
 * Differences are taken accros record boundaries,
 * this works very well because the edges are in scanline order.
 */
Private void queueWriteEdge (FILE *fp, edge_t *e)
{
    long xl, xr, yl, yr;

    e -> xr -= e -> xl;
    e -> yr -= e -> yl;

    e -> xl -= cursor_x;
    cursor_x += e -> xl;

    if (e -> xl != 0) cursor_y = 0; /* to prevent range overflow */
    e -> yl -= cursor_y;
    cursor_y += e -> yl;

    xl = e -> xl;
    xr = e -> xr;
    yl = e -> yl;
    yr = e -> yr;
    if (pack4D (fp, yl, xr, yr, xl) != 0) {
	say ("tmp file write error, size %ld bytes",
	     (long) (optCompress ? fileSize (tmp_name) : ftell (fp)));
	perror (tmp_name);
	die ();
    }

    infoNowInTemp++;
    infoMaxInTemp = Max (infoMaxInTemp, infoNowInTemp);
}

/* queueReadEdge - get an edge from tmp file.
 *
 * See the comments under queueWriteEdge for the format.
 * This routine performs an 'inverse' transformation
 * to the coordinates.
 */
Private edge_t * queueReadEdge (FILE *fp)
{
    static edge_t e;
    long xl, xr, yl, yr;

    if (unpack4D (fp, &yl, &xr, &yr, &xl) == EOF) {
	return (NULL);
    }
    else {
	infoNowInTemp--;

	e.xl = xl;
	e.xr = xr;
	e.yl = yl;
	e.yr = yr;

	if (e.xl != 0) cursor_y = 0;
	e.xl += cursor_x;
	e.yl += cursor_y;
	cursor_x = e.xl;
	cursor_y = e.yl;

	e.xr += e.xl;
	e.yr += e.yl;

	if (e.yr == e.yl) e.slope = 0;
	else if (e.yr - e.yl == e.xr - e.xl) e.slope = 1;
	else if (e.yr - e.yl == e.xl - e.xr) e.slope = -1;
#ifdef MAKEMESH
        else if (e.xl == e.xr) {
	    if (e.yr > e.yl)
	        e.slope = 2;   /* use 2 for 90 degrees */
	    else
		e.slope = -2;   /* use -2 for -90 degrees */
	}
#endif
	else ASSERT (0);

	return (&e);
    }
}

#define EXEC (S_IXUSR|S_IXGRP|S_IXOTH)
#define DIR_ENTRY  S_ISDIR(sbuf.st_mode)
#define FILE_ENTRY S_ISREG(sbuf.st_mode)

Private int found_cmds (char *cmd1, char *cmd2)
{
    char buf[1024];
    long dev[128];
    long ino[128];
    char *pp[128];
    char  tc[150];
    char *te[150];
    struct stat sbuf;
    struct dirent *dp;
    DIR *dirp;
    char *cmd, *s, *t;
    long d, i;
    int ppnr, tenr, n, rv = 0;

    if (!cmd1) return (rv);

    ppnr = tenr = 0;

    if (!(t = getenv ("PATH"))) t = "";
    while ((s = t)) {
	while (*t && *t != ';') {
	    if (*t == ':') break;
	    ++t;
	}
	if (*t) {
	    if (tenr == 150) {
		fprintf (stderr, "too many paths!\n");
		break;
	    }
	    tc[tenr] = *t;
	    te[tenr++] = t;
	    *t++ = 0;
	}
	else t = 0;
	if (!*s) s = ".";
	if (stat (s, &sbuf) == 0 && DIR_ENTRY) {
	    d = sbuf.st_dev; i = sbuf.st_ino;
	    for (n = 0; n < ppnr; ++n)
		if (d == dev[n] && i == ino[n]) {
		    if (*pp[n] != '/') pp[n] = s;
		    break;
		}
	    if (n == ppnr) {
		if (n == 128) {
		    fprintf (stderr, "too many paths!\n");
		    break;
		}
		dev[n] = d;
		ino[n] = i;
		pp[n]  = s;
		++ppnr;
	    }
	}
    }

    for (n = 0; n < ppnr; ++n) {

	s = pp[n];
	if (!(dirp = opendir (s))) continue;

	t = buf;
	while ((*t++ = *s++));
	--t;
	*t++ = '/';

	while ((dp = readdir (dirp))) {
	    s = dp -> d_name;
	    if (s[0] == '.' && (!s[1] || (s[1] == '.' && !s[2]))) continue;

	    cmd = cmd1 ? cmd1 : cmd2;
again:
	    if (strcmp (s, cmd) == 0) {
		(void) strcpy (t, s);
		if (stat (buf, &sbuf) == 0 && FILE_ENTRY) {
		    if (sbuf.st_mode & EXEC) {
			if (cmd == cmd1) cmd1 = NULL;
			else             cmd2 = NULL;
			if (!cmd1 && !cmd2) {
			    rv = 1;
			    goto ready;
			}
		    }
		}
	    }
	    else if (cmd != cmd2 && cmd2) {
		cmd = cmd2;
		goto again;
	    }
	}

	(void) closedir (dirp);
    }
ready:
    /* restore environ path */
    for (n = 0; n < tenr; ++n) *te[n] = tc[n];

    return (rv);
}

Private void initQueue ()
{
    static char buf[256];
    static int cmds = 0;
    Debug (fprintf (stderr, "InitQueue start\n"));

    ready_in_queue = 0;
    middle = NULL;
    tmp_name = tempname ("mkgln", buf, 256);

    cursor_x = 0;
    cursor_y = 0;

    if (optCompress) {
	if (cmds || (cmds = found_cmds (compressCmd, unCompressCmd)))
	    tmp_fp = popen (mprintf ("%s > %s", compressCmd, tmp_name), "w");
	else
	    tmp_fp = NULL;
	if (!tmp_fp) {
	    say ("Temp file compression turned off");
	    optCompress = FALSE;
	}
    }

    if (!optCompress) {
	tmp_fp = cfopen (tmp_name, "wb+");
	unlink (tmp_name);
    }

    Debug (fprintf (stderr, "InitQueue end\n"));
}

Private void flushQueue ()
{
    edge_t *tmp_d, *tmp, *remove;

    Debug (fprintf (stderr, "FlushQueue start\n"));

    tick ("flush queue");

    cursor_x = 0;
    cursor_y = 0;

    if (optCompress) {
	pclose (tmp_fp);
	tmp_fp = NULL;
	infoMaxTempSize = Max (infoMaxTempSize, fileSize (tmp_name));
	tmp_fp = popen (mprintf ("%s < %s", unCompressCmd, tmp_name), "r");
	if (tmp_fp == NULL) {
	    say ("cannot uncompress");
	    perror (mprintf ("%s < %s", unCompressCmd, tmp_name));
	    die ();
	}
    }
    else {
	fflush (tmp_fp);
	infoMaxTempSize = Max (infoMaxTempSize, ftell (tmp_fp));
	fseek (tmp_fp, 0, 0);
    }

    tmp = head;
    tmp_d = queueReadEdge (tmp_fp);
    while (tmp || tmp_d) {
	while (tmp && (tmp_d == NULL || smaller (tmp, tmp_d))) {

	    Debug (printEdge ("rest", tmp));
	    ASSERT (tmp -> ready == TRUE);
	    if (tmp -> remain == FALSE) ready_in_queue--;
	    ggln.xl = tmp -> xl;
	    ggln.yl = tmp -> yl;
	    ggln.xr = tmp -> xr;
	    ggln.yr = tmp -> yr;
#ifdef MAKESIZE
	/* If edge is not an edge used for shrinking the layout */
	if (ggln.yl != byb && ggln.yl != byt)
#endif /* MAKESIZE */
	{
	    if (!optNoOutput) dmPutDesignData (outputStream, GEO_GLN);
	    infoTotalOut++;
	}
	    remove = tmp;
	    tmp = tmp -> next;
	    disposeEdge (remove);
	    infoTotalInQueue--;
	}

	while (tmp_d && (tmp == NULL || smaller (tmp_d, tmp))) {
	    Debug (printEdge ("from file", tmp_d));
	    ggln.xl = tmp_d -> xl;
	    ggln.yl = tmp_d -> yl;
	    ggln.xr = tmp_d -> xr;
	    ggln.yr = tmp_d -> yr;
	    if (!optNoOutput) dmPutDesignData (outputStream, GEO_GLN);
	    infoTotalOut++;

	    tmp_d = queueReadEdge (tmp_fp);
	}
    }

    head = tail = NULL;

    if (optCompress) {
	pclose (tmp_fp);
    }
    else {
	fclose (tmp_fp);
    }

    tmp_fp = NULL;

    unlink (tmp_name);

    ASSERT (ready_in_queue == 0);
    ASSERT (infoNowInTemp == 0);

    tock ("flush queue");

    Debug (fprintf (stderr, "FlushQueue end\n"));
}

Private long fileSize (char *name)
{
    struct stat statbuf;
    stat (name, &statbuf);
    return ((long) statbuf.st_size);
}

void outputEdge (edge_t *e)
{
    ggln.xl = e -> xl;
    ggln.yl = e -> yl;
    ggln.xr = e -> xr;
    ggln.yr = e -> yr;
    if (!optNoOutput) dmPutDesignData (outputStream, GEO_GLN);
    infoTotalOut++;
}

/* Emergency exit for signal handling
 */
void outputCleanUp ()
{
    if (outputStream) {
	dmCloseStream (outputStream, QUIT);
	outputStream = NULL;
    }
    if (optCompress) {
	if (tmp_fp) pclose (tmp_fp);
	tmp_fp = NULL;
	unlink (tmp_name);
    }
}

void outputPrintInfo (FILE *fp)
{
    fprintf (fp, "\n");
    fprintf (fp, "\ttotal # output edges        : %ld\n",(long)infoTotalOut);
    fprintf (fp, "\tmax output edges in core    : %ld\n",(long)infoMaxInQueue);
    fprintf (fp, "\tmax # of edges in temp file : %ld\n",(long)infoMaxInTemp);
    fprintf (fp, "\tsize of tempfile for output : %ld\n",(long)infoMaxTempSize);
}
