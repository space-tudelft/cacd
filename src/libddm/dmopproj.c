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

/* The file .dmrc contains, among other things, a software release number.
** This number is set by mkpr. dmOpenProject now has to verify whether
** the tool operates on a valid (i.e. compatible) database.
**
** If dm_extended_format == 1
**    doubles ('F') are written as two integers
** else
**    doubles ('F') are written as one integer
** Also, the maximum length of strings is determined by dm_maxname,
** which is dependent on dmproject -> release.
*/
int dm_extended_format = 0;
int dm_maxname = DM_MAXNAME;

extern int  dmInit_flag;

static int readline (FILE *fp, char *linebuf)
{
    int c, i = 0;
    while ((c = fgetc (fp)) != '\n') {
	if (c < ' ' || c >= 127) {
	    if (c == '\r' && fgetc (fp) == '\n') break;
	    return -1;
	}
	if (i > DM_MAXPATHLEN-24) return -1;
	if (c == '\\') c = '/';
	linebuf[i++] = c;
    }
    if (i == 0 || linebuf[0] == ' ' || linebuf[i-1] == ' ') return -1;
    linebuf[i] = 0;
    return i;
}

DM_PROJECT * dmOpenProject (char *projectname, int mode)
{
    DM_PROJECT * dmproject;
    char   *dmfile;
    FILE   *fp;
    char    path[DM_MAXPATHLEN+8];
    char    linebuf[DM_MAXPATHLEN];
    int     view_entry;
    register char *cp;
    int     c, plen;

    if (dmInit_flag == 0) {
	dmerrno = DME_NOINIT;
	dmError ("dmOpenProject");
	return (NULL);
    }

    *path = 0;
    if (!projectname || !*projectname) goto fail_badpr;

    if ((plen = strlen (projectname)) > DM_MAXPATHLEN) goto fail_sys;
    c = 0;

    if (*projectname != '/') { /* relative path */
	if (!(cp = getenv ("CWD")))
	    if (!(cp = getcwd (path, DM_MAXPATHLEN))) goto fail_sys;
	c = strlen (cp);
	if ((plen += c + 1) > DM_MAXPATHLEN) goto fail_sys;
	if (cp != path) strcpy (path, cp);
	path[c++] = '/';
    }

    strcpy (path+c, projectname);

    /* get cwd path */
    if (!getcwd (linebuf, DM_MAXPATHLEN)) goto fail_sys;

    /* chdir to project and get absolute path */
    if (chdir (path)) goto fail_sys;
    if (!(cp = getcwd (path, DM_MAXPATHLEN))) goto fail_sys;

    /* go back to cwd */
    if (chdir (linebuf)) goto fail_sys;

    if ((dmproject = _dmCh_proj (path)) != NULL) {
	dmerrno = DME_PRLOCK;
	return (dmproject);
    }
    dmerrno = 0;

    if (!(dmproject = _dmMk_projectkey ())) return (NULL);

    dmproject -> dmpath = cp = _dmStrSave (path);
    if ((cp = strrchr (cp, '/'))) ++cp;
    if (!cp || !*cp) goto fail_badpr;
    dmproject -> projectid = cp;
    dmproject -> mode = mode;

    dmfile = path;
    strcpy (dmfile + strlen (dmfile), "/.dmrc");

    if (!(fp = fopen (dmfile, (mode == PROJ_WRITE)? "r+" : "r"))) {
	dmerrno = DME_NODMRC; /* cannot open file .dmrc */
	dmError (dmproject -> dmpath);
	goto fail2;
    }

    for (view_entry = 0; view_entry < DM_NOVIEWS; view_entry++) {
	dmproject -> celllist[view_entry] = NULL;
	dmproject -> impcelllist[view_entry] = NULL;
    }
    dmproject -> maskdata = NULL;

#ifndef NOPACK
    if ((c = readline (fp, linebuf)) < 1 || c > 3) {
	dmerrno = DME_BADREL; goto fail;
    }
#else
    if ((c = readline (fp, linebuf)) < 2 || c > 4 || linebuf[c-1] != 'U') {
	dmerrno = DME_BADREL; goto fail;
    }
#endif
    dmproject -> release = atoi (linebuf);
    if (dmproject -> release < 302 || dmproject -> release > 302) {
	if (dmproject -> release > 302 || dm_extended_format >= 0) {
	    if (dmproject -> release < 302) fprintf (stderr, "libddm: old project, use dbclean -u\n");
	    dmerrno = DME_BADREL; goto fail;
	}
    }

    cp = dmproject -> procpath;
    if ((c = readline (fp, cp)) < 1) goto fail3;
    if (isdigit (*cp)) { /* process id specified */
	if (c > 5) goto fail3;
	dmproject -> procid = atoi (cp);
	*cp = 0;
    }
    else { /* process directory path specified */
	dmproject -> procid = -9;
    }

    if ((c = readline (fp, linebuf)) < 1 || c > 20) goto fail3;
    dmproject -> lambda = atof (linebuf);

    if ((c = readline (fp, linebuf)) < 1 || c > 5) goto fail3;
    dmproject -> n_samples = atoi (linebuf);

    fclose (fp);

    _dmSetReleaseProperties (dmproject);

#ifdef PRLOCK
 /* directory is a valid project; if mode is write lock it */
    if (mode == PROJ_WRITE && _dmLockProject (dmproject -> dmpath)) {
	goto fail2;
    }
#endif

    return (dmproject);

fail3:
    dmerrno = DME_FMT;
fail:
    fclose (fp);
    dmError (dmfile);
fail2:
    _dmRm_projectkey (dmproject);
    return (NULL);
fail_sys:
    dmerrno = DME_SYS;
    dmError ("dmOpenProject");
    return (NULL);
fail_badpr:
    dmerrno = DME_BADPR;
    dmError (path);
    return (NULL);
}

void _dmSetReleaseProperties (DM_PROJECT *dmproject)
{
    if (dmproject -> release > 100) {
        dm_extended_format = 1; dm_maxname = (dmproject -> release == 301)? DM_MAXNAME_OLD1 : DM_MAXNAME;
    }
    else {
	dm_extended_format = 0; dm_maxname = DM_MAXNAME_OLD;
    }
}
