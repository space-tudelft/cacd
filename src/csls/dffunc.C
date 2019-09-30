/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	P.E. Menchen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

extern Dictionary *dff_dict;
extern DM_PROJECT *dmproject;
extern char f_view[BUFSIZ];     /* Viewtype to look for functional cells */

Stack * xs_rf = NULL;

Network *read_dff (char *name)
{
    long lower[10], upper[10];
    char attribute_string[256];
    DM_CELL * f_key;
    DM_STREAM * f_stream;
    Network * ntw;
    Xelem * stkelem;
    struct stat buf;
    char * new_name;
    DM_PROJECT *projkey;
    int imported;
    int k;
    int exist;

    if ((exist = existCell (name, f_view)) == 0) return (NULL);

    imported = ((exist == 1)?(LOCAL):(IMPORTED));

    projkey = dmFindProjKey (imported, name, dmproject, &new_name, f_view);

    f_key = dmCheckOut (projkey, new_name, ACTUAL, DONTCARE, f_view, READONLY);

    if (dmStat (f_key, "fterm", &buf) == 0) {
	f_stream = dmOpenStream (f_key, "fterm", "r");
    }
    else {
	if (imported == IMPORTED) dmCloseProject (projkey, COMPLETE);

        return (NULL);
        /* apparently, the circuit view only contains a real circuit */
    }
    dm_get_do_not_alloc = 1;
    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    ntw = new Network (name);
    dff_dict -> store (ntw->ntw_name, (char *) ntw);
    if (imported == IMPORTED)
	ntw -> local = 0;
    else
	ntw -> local = 1;

    if (xs_rf == NULL) xs_rf = new Stack (XSTACK_SIZE);

    ntw -> termq = new Queue (QueueType);

    while (dmGetDesignData (f_stream, CIR_TERM) > 0 ) {
        xs_rf -> reset ();
        for (k = 0; k < cterm.term_dim; k++) {
            stkelem = new Xelem ((short)cterm.term_lower[k], (short)cterm.term_upper[k]);
            if (xs_rf -> push ((char *)stkelem) == STACK_OVERFLOW)
                fprintf (stderr, "xstack overflow\n");
        }
        ntw -> termq -> put ((Link *) new Netelem (cterm.term_name, stackcpy (xs_rf), TermType));
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (f_stream, COMPLETE);
    dmCheckIn (f_key, COMPLETE);

    if (imported == IMPORTED) dmCloseProject (projkey, COMPLETE);

    return (ntw);
}
