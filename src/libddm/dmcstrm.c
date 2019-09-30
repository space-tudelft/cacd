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

static  DM_STREAM * stream_admin[DM_MAXSTREAMS];
static  int streamno = -1;

int dmCloseStream (DM_STREAM *dmfile, int mode)
{
    if (!dmfile) {
	dmerrno = DME_BADKEY;
	dmError ("dmCloseStream");
	return (-1);
    }
    if (!(mode == COMPLETE || mode == QUIT || mode == CONTINUE)) {
	dmerrno = DME_BADARG;
	dmError ("dmCloseStream");
	return (-1);
    }

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "keyno: %d, streamno: %d, fildes: %d\n",
	dmfile -> dmkey -> keyno, dmfile -> streamno, fileno (dmfile -> dmfp));
#endif

    if (dmfile -> dmfp && fclose (dmfile -> dmfp) == EOF) {
	dmerrno = DME_SYS;
	dmError ("dmCloseStream");
	return (-1);
    }

    if (mode == QUIT &&
	(dmfile -> mode[0] == 'w' || dmfile -> mode[0] == 'a'
	 || dmfile -> mode[1] == '+')) {
	/* If mode == QUIT and stream was open for writing, empty it ! */
	_dmCreateEmptyStream (dmfile -> dmkey, dmfile -> stream);
    }

    _dmRm_streamkey (dmfile);

    return (0);
}

/*
** This function allocates a structure describing an
** stream which is passed to the user and initializes it.
** Information must still be written to this structure.
*/
DM_STREAM * _dmMk_streamkey ()
{
    DM_STREAM * key;

    if (++streamno == DM_MAXSTREAMS) {
	_dmFatal ("_dmMk_streamkey: too many open streams", "", "");
    }

    if ((key = (DM_STREAM *) malloc (sizeof (DM_STREAM))) == NULL) {
	_dmFatal ("_dmMk_streamkey: cannot alloc key", "", "");
    }

    stream_admin[streamno] = key;
    key -> streamno = streamno;
    return (key);
}

/*
** This function removes the structure which is passed to
** the user at dmOpenStream time and which identificates the
** stream opened.
*/
void _dmRm_streamkey (DM_STREAM *key)
{
    if (key -> streamno != streamno) {
	stream_admin[key -> streamno] = stream_admin[streamno];
	stream_admin[streamno] -> streamno = key -> streamno;
    }
    --streamno;
    _dmStrFree (key -> stream);
    _dmStrFree (key -> mode);
    free (key);
}

void dmCloseCellStreams (DM_CELL *cellkey, int mode)
{
    register int i;
    for (i = streamno; i >= 0; --i)
	if (stream_admin[i] -> dmkey == cellkey)
	    dmCloseStream (stream_admin[i], mode);
}
