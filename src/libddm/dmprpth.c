/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/libddm/dmstd.h"
#include <dirent.h>
#include <strings.h>

/*
** _dmGetProcPath will build a path to the file 'file_name'
** for the process identified by the proc-id that is stored
** in the dmproject key.
*/
char * _dmGetProcPath (DM_PROJECT *dmproject, char *file_name)
{
    return (_dmDoGetProcPath (dmproject -> procid, file_name, dmproject));
}

/* actual building of path to process file with the name 'file_name'.
** separate routine with the process_id as its argument, to
** allow dedicated programs to use it without opening a project.
*/
char * _dmDoGetProcPath (int process_id, char *file_name, DM_PROJECT *dmproject)
{
    static char path_name[MAXLINE];
    static char buf[MAXLINE];	/* return buffer */
    FILE * fp;
    char    idstr[6];
    char    proc_id[40];
    static int old_process_id = -1;
    static char proc_name[40];
    char   *cp;
    int     i;
    DIR    *dirp;

    if (dmproject && dmproject -> procpath[0] != '\0') {
        strcpy (path_name, dmproject -> procpath);
	old_process_id = -1;
        goto path_ready;
    }

    if (process_id == old_process_id)
	goto path_ready;

    old_process_id = -1;

    _dmSprintf (idstr, "%d", process_id);
    _dmSprintf (buf, "%s/share/lib/process/processlist", icdpath);
    fp = fopen (buf, "r");

#ifdef DM_DEBUG
    IFDEBUG
	fprintf (stderr, "fildes: %d, path: %s\n",
	    (fp ? fileno (fp) : -1), buf);
#endif /* DM_DEBUG */

    if (fp == NULL)
	goto e1;    /* cannot read processlist */

    i = 0;
    while (fgets (buf, MAXLINE, fp) != NULL) {
	if (sscanf (buf, "%s%s", proc_id, proc_name) != 2)
	    continue;
	if (*proc_id == '#')
	    continue;
	if (strcmp (proc_id, idstr) == 0) {
	    ++i;		/* found */
	    break;
	}
    }
    fclose (fp);

    if (!i)
	goto e2;   /* cannot find process id */

    if ((cp = strchr (proc_name, '#')))
	*cp = '\0';

    old_process_id = process_id;
    _dmSprintf (path_name, "%s/share/lib/process/%s", icdpath, proc_name);

path_ready:
    if ((dirp = opendir (path_name)) == NULL) {
        goto e3;      /* cannot read process path */
    }
    closedir (dirp);
    _dmSprintf (buf, "%s/%s", path_name, file_name);

    return (buf);

e1:
    dmerrno = DME_PRDATA;
    dmError (buf);
    goto ret;
e2:
    dmerrno = DME_PRID;
    dmError ("_dmGetProcPath");
    goto ret;
e3:
    dmerrno = DME_PRDATA;
    dmError (path_name);
    goto ret;
ret:
    return (NULL);
}
