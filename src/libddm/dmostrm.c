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

/*
** This function opens the file and returns a file pointer
** to the file, as indicated in the key argument.
** NULL is returned when the request can not be granted.
*/
DM_STREAM * dmOpenStream (DM_CELL *key, const char *stream, const char *mode)
{
    char    path[MAXLINE];
    char   *view;
    int     is_info2;
    int     is_info3;
    char   *file;
    DM_STREAM * dmfile;
    char    bmode[8];
    int l;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "keyno: %d, stream: %s, mode: %s\n", key -> keyno, stream, mode);
#endif

    if (_dmCh_key (key) != 0) {
	dmError2 ("dmOpenStream: stream", (char*)stream);
	return (NULL);
    }

    file = (char*)stream; /* direct mapping of stream to file? */
    is_info2 = 0;
    is_info3 = 0;
    if (strcmp (file, "info2") == 0) {
	++is_info2;
	file = "info";
    }
    else if (strcmp (file, "info3") == 0) {
	++is_info3;
	file = "info";
    }

    view = key -> view;

    if (mode[0] == 'w' || mode[0] == 'a' || mode[1] == '+') {
	if (is_info2 || is_info3) {
	    mode =  "r+";
	}
	if (key -> mode == READONLY) {
	    dmerrno = DME_MODE;		/* invalid mode for key */
	    _dmSprintf (path, "%s: %s", stream, mode);
	    dmError2 ("dmOpenStream", path);
	    return (NULL);
	}
    }

    _dmSprintf (path, "%s/%s/%s/%s", key -> dmproject -> dmpath, view, key -> cell, file);

    strcpy (bmode, mode);
#ifndef NOPACK
    /* use binary mode */
    l = strlen(bmode);
    if (bmode[l - 1] == '+') {
        bmode[l - 1] = 'b';
        bmode[l] = '+';
        bmode[l + 1] = '\0';
    }
    else {
        bmode[l] = 'b';
        bmode[l + 1] = '\0';
    }
#endif

    dmfile = _dmMk_streamkey ();
    dmfile -> dmfp = fopen (path, bmode);
    dmfile -> dmkey = key;
    dmfile -> stream = _dmStrSave ((char*)stream);
    dmfile -> mode = _dmStrSave (bmode);

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "path: %s, fildes: %d\n",
	    path, (dmfile -> dmfp ? fileno (dmfile -> dmfp) : -1));
#endif

    if (dmfile -> dmfp == NULL) {
	dmerrno = DME_FOPEN;
	dmError2 ("dmOpenStream", (char*)stream);
	goto fail;
    }
    else if (is_info3) {
	fseek (dmfile -> dmfp, 195L, 0);/* 3 x (4 x 16 + 1) */
    }
    else if (is_info2) {
	fseek (dmfile -> dmfp, 276L, 0);
	/* 3 x (4 x 16 + 1) + 5 x 16 + 1 */
    }

    return (dmfile);
fail:
    _dmRm_streamkey (dmfile);
    return (NULL);
}

void _dmCreateEmptyStream (DM_CELL *key, char *streamname)
{
    FILE *fp;
    char path[MAXLINE];

    _dmSprintf (path, "%s/%s/%s/%s", key -> dmproject -> dmpath, key -> view, key -> cell, streamname);
    if ((fp = fopen (path, "w"))) fclose (fp);
}
