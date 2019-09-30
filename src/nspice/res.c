/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "src/nspice/extern.h"

/* This function returns expanded names specified by path.
 * Users use traillen = 0.
 */
STRING_REF *names_from_path (int traillen, PATH_SPEC *path)
{
    char *p, *p1, *p2, *indexp;
    int currlen, i, ready, ready2;
    STRING_REF *beg_ref, *end_ref, *str_ref;
    int xvector[MAXDIM];
    char name[MAXLEN + 41]; /* 40 characters for the index */

    beg_ref = end_ref = NULL;

    while (path) {

        p = name;
        if (traillen > 0) *p++ = '.';

        strcpy (p, path -> name);
	p = p + strlen (path -> name);
	indexp = p;

        xvector[0] = path -> xarray[0][0];
        for (i = path -> xarray[0][0]; i > 0; i--) {
            xvector[i] = path -> xarray[i][0];
        }

        ready = FALSE;
        do {
	    if (xvector[0] > 0) {
		p = indexp;
	        *p++ = '[';
                for (i = 1; i <= xvector[0]; i++) {
		    if (*(p - 1) != '[') *p++ = ',';
	            sprintf (p, "%d", xvector[i]);
		    while (*++p != '\0') ;
		}
		*p++ = ']';

	    }
            *p = '\0';

            currlen = traillen + (p - name);

            if (!path -> next) {
	        NEW (str_ref, 1, STRING_REF);
		str_ref -> next = NULL;
		NEW (str_ref -> str, currlen + 1, char);
		*(str_ref -> str + currlen) = '\0';
	    }
	    else {
		str_ref = names_from_path (currlen, path -> next);
	    }

            if (!beg_ref) {
		beg_ref = end_ref = str_ref;
	    }
	    else {
		end_ref -> next = str_ref;
		end_ref = str_ref;
	    }

	    ready2 = FALSE;
	    do {
		p1 = name;
		p2 = end_ref -> str + traillen;
		while (*p1 != '\0') *p2++ = *p1++;
		if (!end_ref -> next)
		    ready2 = TRUE;
		else
		    end_ref = end_ref -> next;
	    }
	    while (!ready2);

	    /* find new indices by incrementing or decrementing
	       the current vector */

	    if (xvector[0] > 0) {
                for (i = xvector[0]; i > 0; i--) {
		    if (path -> xarray[i][0] <= path -> xarray[i][1]) {
                        xvector[i]++;
	                if (xvector[i] <= path -> xarray[i][1]) break;
	                else {
		            xvector[i] = path -> xarray[i][0];
		            if (i == 1) ready = TRUE;
	                }
		    }
		    else {
                        xvector[i]--;
	                if (xvector[i] >= path -> xarray[i][1]) break;
	                else {
		            xvector[i] = path -> xarray[i][0];
		            if (i == 1) ready = TRUE;
	                }
		    }
                }
	    }
	    else
	        ready = TRUE;
        }
        while (!ready);

        path = path -> also;
    }

    return (beg_ref);
}
