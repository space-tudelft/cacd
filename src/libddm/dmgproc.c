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

#define ALLOC(ptr,el,name) \
if(!(ptr=(name *)malloc((unsigned)((el)*sizeof(name)))))goto e3

/*
** _dmGetProcess is used to read the process data in core,
** after which it is accessible by the calling program.
*/
DM_PROCDATA *_dmGetProcess (DM_PROJECT *dmproject)
{
    if (!dmproject -> maskdata)
    if (dmproject -> procid < 0
        || (dmproject -> maskdata = _dmSearchProcInProjKeys (dmproject -> procid)) == NULL) {
	/* specified as dmproject -> procpath or not yet present,
           do the actual getting of the maskdata */
	dmproject -> maskdata = _dmDoGetProcess (dmproject -> procid, dmproject);
    }
    return (dmproject -> maskdata);
}

/* Actual getting of process information is localized in a
** separate routine with the process_id as its argument, to
** allow dedicated programs to use it without opening a project,
** and dmproject as an optional non-zero argument in case
** process_id < 0 and the process path is in dmproject -> procpath.
** Tecc needs a filename.
*/
DM_PROCDATA *_dmDoGetProcess (int process_id, DM_PROJECT *dmproject)
{
    char   *path;
    struct stat sbuf;
#ifdef OLDMETHODMASKDATA
    DM_PROCDATA * process;
    char proc_name[256];
    char *p;
#endif

    if ((path = _dmDoGetProcPath (process_id, "maskdata", dmproject)) == NULL)
	return (NULL);
    else {
        if (stat (path, &sbuf) != 0) {
            /* No maskdata file present for this process. */
#ifdef OLDMETHODMASKDATA
            /* Disabled the following Oct 6th, 2000 because otherwise it is
               so hard to detect when no valid process info is present (AvG). */

            /* Make a DM_PROCDATA structure with a minimum amount of
               information.
            */
	    ALLOC (process, 1, DM_PROCDATA);
	    process -> nomasks = 0;
            for (p = path + strlen (path) - strlen ("maskdata") - 2;
                 *p != '/' && *p != '\\' && p > path; p--);
            strcpy (proc_name, p + 1);
            proc_name[strlen (proc_name) - strlen ("maskdata") - 1] = '\0';
	    ALLOC (process -> pr_name, strlen (proc_name) + 1, char);
	    _dmSprintf (process -> pr_name, "%s", proc_name);
            ALLOC (process -> pr_type, 1, char);
	    _dmSprintf (process -> pr_type, "");
            ALLOC (process -> pr_info, 1, char);
	    _dmSprintf (process -> pr_info, "");
            process -> mask_name = NULL;
            process -> mask_info = NULL;
            process -> mask_no = NULL;
            process -> pgt_no = NULL;
            process -> pgt_type = NULL;
            process -> CM = NULL;
            process -> RT = NULL;
            process -> PLOT = NULL;
	    return (process);
#else
            dmerrno = DME_PRDATA;
            dmError (path);
            return (NULL);
#endif
        }
        else
	    return (_dmDoGetProcessFile (path));
    }
#ifdef OLDMETHODMASKDATA
e3:
#endif
    dmerrno = DME_CORE;
    return (NULL);
}

DM_PROCDATA *_dmDoGetProcessFile (char *path)
{
    FILE * fp;
    char    buf[MAXLINE];
    char    help[MAXLINE];
    char   *proc_name;
    char   *cp;
    char   *end_p;
    int     nomasks;
    int     mask_index;
    int     i;
    int     j;
    DM_PROCDATA * process;
    int lineno = 0;

    fp = fopen (path, "r");

#ifdef DM_DEBUG
    IFDEBUG
	fprintf (stderr, "fildes: %d, path: %s\n",
	    (fp ? fileno (fp) : -1), path);
#endif /* DM_DEBUG */

    if (fp == NULL) goto e2;

    ALLOC (process, 1, DM_PROCDATA);

    do {
        lineno++;
	if (!fgets (buf, MAXLINE, fp)) goto e4;
	if (sscanf (buf, "%s", help) <= 0)
	    strcpy (buf, "#");  /* empty line */
    } while (*buf == '#');

/* decode process-type from buf */
    if (!(cp = strchr (buf, '"'))) goto e4; /* pr_type should be present */
    cp++;
    if (!(end_p = strchr (cp, '"'))) goto e4;
    *end_p = '\0';

    ALLOC (process -> pr_type, end_p - cp + 1, char);
    _dmSprintf (process -> pr_type, "%s", cp);

/* decode optional process_info from remaining part of buf */
    if (!(cp = strchr (end_p + 1, '"'))) {
	cp = "";	/* no pr_info present */
	end_p = cp;
    }
    else {
	cp++;
	if (!(end_p = strchr (cp, '"'))) goto e4;
	*end_p = '\0';
    }

    ALLOC (process -> pr_info, end_p - cp + 1, char);
    _dmSprintf (process -> pr_info, "%s", cp);

/* pr_name must be decoded from path */
    if ((cp = strrchr (path, '/'))) {
	*cp = '\0';
	if ((proc_name = strrchr (path, '/'))) ++proc_name;
	else proc_name = path;
    }
    else proc_name = "";

/* allocate and fill pr_name */
    ALLOC (process -> pr_name, strlen (proc_name) + 1, char);
    _dmSprintf (process -> pr_name, "%s", proc_name);
    if (cp) *cp = '/';

/* count the number of masks */

    nomasks = 0;
    i = lineno++;
    while (fgets (buf, MAXLINE, fp)) {
	if (sscanf (buf, "%s", help) == 1 && help[0] != '#') nomasks++;
	lineno++;
    }
    if (nomasks == 0 || nomasks > DM_MAXNOMASKS) {
	fprintf (stderr, "illegal number of masks, nomasks = %d\n", nomasks);
	goto e4;
    }

    rewind (fp);
    for (lineno = 0; lineno < i;) {
	++lineno;
	if (!fgets (buf, MAXLINE, fp)) goto e4;
    }

/* allocate mask_name, mask_no and mask_type pointers	*/

    ALLOC (process -> mask_name, nomasks, char *);
    ALLOC (process -> mask_info, nomasks, char *);
    ALLOC (process -> mask_no, nomasks, int);
    ALLOC (process -> mask_type, nomasks, int);

/* allocate pgt_ pointers */

    ALLOC (process -> pgt_no, nomasks, int);
    ALLOC (process -> pgt_type, nomasks, int);

/* allocate pointers to dev structures of:
** ColorMask, RamTek and PLOTter
*/
    ALLOC (process -> CM, nomasks, struct device   *);
    ALLOC (process -> RT, nomasks, struct device   *);
    ALLOC (process -> PLOT, nomasks, struct device *);

    process -> nomasks = nomasks;

    for (i = 0; i < nomasks; i++) {
	ALLOC (process -> mask_name[i], DM_MAXLAY + 1, char);
	ALLOC (process -> CM[i], 1, struct device);
	ALLOC (process -> RT[i], 1, struct device);
	ALLOC (process -> PLOT[i], 1, struct device);
	process -> mask_no [i] = -1;
    }

/* store mask information into process structure */

    for (i = 0; i < nomasks; i++) {
	do {
            lineno++;
	    if (!fgets (buf, MAXLINE, fp)) goto e4;
	    if (sscanf (buf, "%s", help) <= 0)
		strcpy (buf, "#");  /* empty line */
	} while (*buf == '#');

	if (sscanf (buf, "%d", &mask_index) == 1) {
            /* index is specified */

	    /* test mask_index: in valid range and entry should be free */
	    if (mask_index < 0 || mask_index >= nomasks
			        || process -> mask_no[mask_index] != -1) {
	        goto e4;
	    }

            /* remove index from buf string */
            strcpy (help, buf);
            for (j = 0; isspace ((int)help[j]) || isdigit ((int)help[j]); j++);
            strcpy (buf, help + j);
        }
        else mask_index = i;

        process -> mask_no[mask_index] = mask_index;
	if (sscanf (buf, "%s%d%d%d%d%d%d%d%d%d",
		    process -> mask_name[mask_index],
		    &process -> mask_type[mask_index],
		    &process -> pgt_no[mask_index],
		    &process -> pgt_type[mask_index],
		    &process -> CM[mask_index] -> code,
		    &process -> CM[mask_index] -> fill,
		    &process -> RT[mask_index] -> code,
		    &process -> RT[mask_index] -> fill,
		    &process -> PLOT[mask_index] -> code,
		    &process -> PLOT[mask_index] -> fill) != 10)
	    goto e4;

	/* decode optional mask_info from buf */
	if (!(cp = strchr (buf, '"'))) {
	    cp = "";	/* no mask_info present */
	    end_p = cp;
	}
	else {
	    cp++;
	    if (!(end_p = strchr (cp, '"'))) goto e4;
	    *end_p = '\0';
	}

	ALLOC (process -> mask_info[mask_index], end_p - cp + 1, char);
	_dmSprintf (process -> mask_info[mask_index], "%s", cp);
    }

    if (i != nomasks) goto e4;

    fclose (fp);
    return (process);
e2:
    dmerrno = DME_PRDATA;
    goto ret;
e3:
    dmerrno = DME_CORE;
    goto ret;
e4:
    fprintf (stderr, "error in file '%s' at line %d\n", path, lineno);
    dmerrno = DME_PRREAD;
ret:
    dmError ("_dmGetProcess");
    return (NULL);
}
