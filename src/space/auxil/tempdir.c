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

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/vfs.h>

#include "src/space/auxil/auxil.h"

#ifdef DRIVER
#undef  NEW
#define NEW(type,len) malloc (len * sizeof(type))
#define say Say
void Say (char *s) { printf ("%s\n", s); }
void die () { exit (1); }
#endif

static char * builtin_dirs[] = {
	"/tmp",
	"/usr/tmp",
};

static char * my_getenv (const char *s)
{
    char *p = getenv (s);
    return (p);
}

/* tempdir - Return a suitable tempdir name.
 * The user can specify a list of directories from which a temp directory
 * is chosen that has the largest free space.
 * The list of directories is specified separated by ':' characters and
 * using the environment variable SPACE_TMPDIR
 * If the environment variable SPACE_TMPDIR is not specified, the directory
 * is chosen from the list /tmp:/usr/tmp
 */
char * tempdir ()
{
    static char * spec_dirs[25];
    static char **dirs;
    static int nr_dirs;
    static int initFlag = 0;
    FILE *fp;
    char buf[128];
    struct statfs statbuf;
    struct stat sbuf;
    char *name, *p;
    double avail = 0;
    int i, imax = 0;

    if (!initFlag) {
	dirs = builtin_dirs;
	nr_dirs = (sizeof (builtin_dirs) / sizeof (*builtin_dirs));
#ifdef DRIVER
	printf ("\nThe builtin dirs are:\n");
	for (i = 0; i < nr_dirs; i++) printf ("dir%d='%s'\n", i+1, dirs[i]);
#endif
	p = my_getenv ("SPACE_TMPDIR");
	if (p) {
#ifdef DRIVER
	    printf ("\nUsing env.var. SPACE_TMPDIR\n");
#endif
	    dirs = spec_dirs;
	    nr_dirs = 0;
	    while (*p) {
		name = p;
		do {
		    if (*p == ';') break; /* separator */
		    if (*p == ':') break; /* separator */
		} while (*++p);

		if (p > name && p < name+110 && nr_dirs < 25) {
		    spec_dirs[nr_dirs++] = name;
		}
		if (!*p) break;
		*p++ = 0;
	    }
#ifdef DRIVER
	    printf ("The specified dirs are:\n");
	    for (i = 0; i < nr_dirs; i++) printf ("dir%d='%s'\n", i+1, dirs[i]);
#endif
	}
#ifdef DRIVER
	else {
	    printf ("\nIn place of the builtin dirs you can setenv SPACE_TMPDIR\n");
	}
#endif
        initFlag = 1;
    }

    for (i = 0; i < nr_dirs; i++) {
	if (stat (dirs[i], &sbuf)) continue;
        if (statfs (dirs[i], &statbuf)) continue;
	if (avail < (double)statbuf.f_bsize * statbuf.f_bavail) {
            sprintf (buf, "%s/xxx%d", dirs[i], (int)getpid ());
            if ((fp = fopen (buf, "w"))) {
		avail = (double)statbuf.f_bsize * statbuf.f_bavail;
                fclose (fp);
                unlink (buf);
	        imax = i;
            }
	}
    }
    if (avail == 0) {
	say ("tempdir: cannot use the temporary directories");
	say ("tempdir: set env.var. SPACE_TMPDIR appropriate");
	die ();
    }
    return (dirs[imax]);
}

char * tempname (const char *pfx, const char *buf, int size)
{
    char *s, *dir = tempdir ();
    int len = strlen (dir) + strlen (pfx) + 8;
    if (!(s = (char *)buf)) s = NEW (char, len);
    else if (len > size) say ("tempname: buf size too small"), die ();
    sprintf (s, "%s/%sXXXXXX", dir, pfx);
    if ((len = mkstemp (s)) == -1) say ("tempname: mkstemp error"), die ();
    close (len); /* close(fd), SdeG */
    return (s);
}

#ifdef DRIVER
int main ()
{
    char buf[80], *s;
    int i, nr_dirs = (sizeof (builtin_dirs) / sizeof (*builtin_dirs));

    s = tempdir ();
    printf ("\nTests:\n");
    printf ("tempdir1='%s'\n", s);
    printf ("tempdir2='%s'\n", tempdir ());
    printf ("tempnam1='%s'\n", tempname ("simon", buf, 80));
    if (unlink (buf)) printf ("tempnam1='%s' unlink failed!\n", buf);
    printf ("tempnam2='%s'\n", tempname ("simon", buf, 80));
    if (unlink (buf)) printf ("tempnam2='%s' unlink failed!\n", buf);
    printf ("tempnam3='%s'\n", tempname ("simon", buf, 80));
    if (unlink (buf)) printf ("tempnam3='%s' unlink failed!\n", buf);
    printf ("tempnam4='%s'\n", tempname ("simon", buf, 80));
    if (unlink (buf)) printf ("tempnam4='%s' unlink failed!\n", buf);
    printf ("\n");
    return (0);
}
#endif
