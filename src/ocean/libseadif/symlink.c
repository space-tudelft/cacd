/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
#include <unistd.h>

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/libseadif/sea_decl.h"

#ifdef BSD
#include <sys/param.h>
#define MAXPATH MAXPATHLEN
#else
#define MAXPATH 1024 /* maximum length of canonic path */
#endif
#define MAXSYMPATH MAXPATH+100

/* Transform the path to its absolute canonical equivalent, that is,
 * without symbolic links and starting with a '/'.
 * And not containing ".." and "." components in the path.
 * And not containing multiple slashes "//" in the path.
 * And not ending the path with a "/", except when the path is "/".
 * The old canonicpath and canonicpath2 functions are replaced (SdeG).
 *
 * Every time readlink() finds a path for a symbolic link, the symbolic
 * link is replaced by the found path and we start again to replace
 * possible new symbolic links. Note that the found path can contain
 * dots and unwanted slashes which must be removed.
 *
 * For instance we start with the path "/home/stravers/foobar", which
 * contains the following symbolic links:
 *	/home/stravers -> /export/home1/stravers/.
 *	/export/home1  -> /files1//
 *
 * "/home/stravers" is replaced by "/export/home1/stravers"
 * and "/export/home1" is replaced by "/files1".
 * Thus the resulting path is "/files1/stravers/foobar".
 */
char *abscanonicpath (char *path)
{
    char sympath[MAXSYMPATH+1];
    char cpath[MAXSYMPATH+1];
    char buf[MAXPATH+1];
    int  c, ci, si, len, l, start_i, len_si;
    int  nr_times = 0;
    int  remove_dots = 0;

    /* check input for dots and slashes and copy into sympath */
    ci = si = 0;
    if (*path == '/') { sympath[si++] = '/'; ci++; }
    for (;;) {
	start_i = si;
	while ((c = path[ci++]) && c != '/') {
	    if (si > MAXSYMPATH) sdfreport (Fatal, "too long sympath");
	    sympath[si++] = c;
	}
	if (sympath[start_i] == '.' && si == start_i+1) --si; /* skip "." */
	if (!c) break;
	if (si > start_i) sympath[si++] = '/';
    }
    if (si > 1 && sympath[si-1] == '/') --si;
    sympath[si] = '\0';
    len_si = si;

start:
#ifdef DEBUG
    printf ("\nsympath='%s'\n", sympath);
#endif

    ci = si = 0;

    if (*sympath == '/') { cpath[ci++] = '/'; si++; }

    for (;;) {
	start_i = ci;
	while ((c = sympath[si++]) && c != '/') cpath[ci++] = c;

	if (cpath[start_i] == '.' && ci == start_i+2
	 && cpath[start_i+1] == '.') { /* skip ".." */
	    len = 0; remove_dots = 1;
	}
	else {
	    cpath[ci] = '\0';
#ifdef DEBUG
	    printf ("rd_link='%s'  ", cpath);
#endif
	    len = readlink (cpath, buf, MAXPATH);
#ifdef DEBUG
	    if (len > 0) buf[len] = 0; else *buf = 0;
	    printf ("len=%2d  buf='%s'\n", len, buf);
#endif
	}
	if (len > 0) { /* cpath is symbolic link to buf */
	    l = 0; buf[len] = '\0';
	    if (buf[0] == '/') { /* symbolic link to absolute path */
		 ci = 0; cpath[ci++] = '/'; ++l; }
	    else ci = start_i; /* symbolic link to relative path */
	    for (;;) {
		start_i = ci;
		while ((c = buf[l++]) && c != '/') cpath[ci++] = c;
		if (cpath[start_i] == '.' && ci == start_i+1) --ci; /* skip "." */
		if (!c) break;
		if (ci > start_i) cpath[ci++] = '/';
	    }
	    len_si -= si;
	    if (len_si > 0) { /* add rest of sympath */
		if (ci && cpath[ci-1] != '/') cpath[ci++] = '/';
		if (ci + len_si > MAXSYMPATH) sdfreport (Fatal, "too long sympath");
		while ((c = sympath[si++])) cpath[ci++] = c;
	    }
	    else {
		if (ci > 1 && cpath[ci-1] == '/') --ci;
		if (ci > MAXSYMPATH) sdfreport (Fatal, "too long sympath");
	    }
	    cpath[ci] = '\0';
	    if (++nr_times < 20) {
		len_si = ci;
		strcpy (sympath, cpath);
		goto start;
	    }
	    remove_dots = 1;
	    break;
	}
	if (!c) break; /* end of sympath */
	cpath[ci++] = '/';
    }

    while (ci > 1 && cpath[ci-1] == '/') --ci;
    cpath[ci] = '\0';

#ifdef DEBUG
    printf ("\nsympath='%s'\n", cpath);
#endif
    path = cpath;

    if (*cpath != '/') { /* make relative path absolute */
	char *cwd = sdfgetcwd ();
	/* the cwd path does not contain symbolic links */

	if (!*cpath) { path = cwd; goto ret; }

	ci += si = strlen (cwd);
	if (si > 1) ++ci; /* add slash */
	if (ci > MAXSYMPATH) sdfreport (Fatal, "too long sympath");

	strcpy (sympath, cwd);
	if (si > 1) sympath[si++] = '/'; /* not root */
	strcpy (sympath + si, cpath);
	if (!remove_dots) { path = sympath; goto ret; }
    }
    else {
	if (!remove_dots) goto ret;
	strcpy (sympath, cpath);
    }

    /* remove ".." components from abs. sympath */
    cpath[0] = '/';
    si = ci = 1;
    do {
	start_i = ci;
	while ((c = sympath[si++]) && c != '/') cpath[ci++] = c;

	if (cpath[start_i] == '.' && ci == start_i+2
	 && cpath[start_i+1] == '.') { /* ".." component */
	    ci = start_i; /* go one directory up */
	    if (--ci > 0) while (--ci > 0 && cpath[ci] != '/');
	    ++ci; /* ci >= 1 */
	}
	else if (ci > start_i) cpath[ci++] = '/';
    } while (c);

    if (ci > 1) --ci;
    cpath[ci] = '\0';
ret:
    if (nr_times == 20) sdfreport (Warning, "too many links: %s", path);
    return (cs (path));
}

/* This public function was added for convenience and efficiency */
char *sdfgetcwd (void)
{
    static char *thecwd = NULL;
    char cwd[MAXPATH+1];

    if (!thecwd) {
	thecwd = getcwd (cwd, MAXPATH+1);
	if (thecwd && *thecwd) thecwd = cs (thecwd);
	if (!thecwd) sdfreport (Fatal, "cannot get cwd");
	if (*thecwd != '/') sdfreport (Fatal, "cwd no abs. path");
    }
    return thecwd;
}
