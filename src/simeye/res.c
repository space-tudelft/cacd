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

#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

RES_FILE *RF = NULL; /* begin of res file list */
RES_FILE *end_RF;    /* end of res file list */

int ret_arr_size = 0;
int *ret_arr = NULL;

/* Returns expanded names specified by path.
   Users use traillen = 0.
*/
STRING_REF * names_from_path (int traillen, PATH_SPEC *path)
{
    char *p, *p1, *p2, *indexp;
    int currlen, i, ready, ready2;
    STRING_REF *beg_ref, *end_ref, *str_ref;
    int xvector[MAXDIM];
    char name[DM_MAXNAME + 41]; /* 40 characters for the index */

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
		    if (*(p - 1) != '[' ) *p++ = ',';
	            sprintf (p, "%d", xvector[i]);
		    while (*++p != '\0') ;
		}
		*p++ = ']';
	    }
            *p = '\0';

            currlen = traillen + (p - name);

            if (path -> next == NULL) {
	        NEW (str_ref, 1, STRING_REF);
		str_ref -> next = NULL;
		NEW (str_ref -> str, currlen + 1, char);
		*(str_ref -> str + currlen) = '\0';
	    }
	    else {
		str_ref = names_from_path (currlen, path -> next);
	    }

            if (beg_ref == NULL) {
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
		while (*p1 != '\0')
		    *p2++ = *p1++;
		if (end_ref -> next == NULL)
		    ready2 = TRUE;
		else
		    end_ref = end_ref -> next;
	    }
	    while (! ready2);

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
        while (! ready);

        path = path -> also;
    }

    return (beg_ref);
}

/* Writes path into file fp according the network format.
   This function doesn't bother with the 'also' pointer.
*/
void wr_path (FILE *fp, PATH_SPEC *path)
{
    int i;

    fprintf (fp, " (");
    while (path) {
	if (path -> xarray[0][0] > 0) {
	    fprintf (fp, " (");
	    fprintf (fp, "%s", path -> name);
	    for (i = 1; i <= path -> xarray[0][0]; i++) {
		if (path -> xarray[i][0] == path -> xarray[i][1]) {
		    fprintf (fp, " %d", path -> xarray[i][0]);
		}
		else {
		    fprintf (fp, " (%d %d)", path -> xarray[i][0], path -> xarray[i][1]);
		}
	    }
	    fprintf (fp, ")");
	}
	else {
	    fprintf (fp, " %s", path -> name);
	}
	path = path -> next;
    }
    fprintf (fp, " )");
}

/* Writes full paths into file fp according the network format.
   This function doesn't bother with the 'also' pointer.
*/
void write_paths (FILE *fp, RES_PATH *paths)
{
    while (paths) {
	wr_path (fp, paths -> path);
	paths = paths -> next;
    }
}

/* Skips all spaces and tabs until next non-space
   or newline character (which is returned).
*/
int skipspace (FILE *fp)
{
    int c;
    while ((c = getc (fp)) == ' ' || c == '\t' ) ;
    ungetc (c, fp);
    return (c);
}

/* Reads the signal paths for file pointed to by fp and with name fn.
   Returns the pointer to resfile information.
*/
RES_FILE *read_paths (FILE *fp, char *fn)
{
    RES_PATH *end_r_path, *r_path;
    PATH_SPEC *end_p, *p;
    int c, i, up, low, help;

    if (!RF) {
	NEW (RF, 1, RES_FILE);
	end_RF = RF;
    }
    else {
	NEW (end_RF -> next, 1, RES_FILE);
	end_RF = end_RF -> next;
    }
    end_RF -> next = NULL;
    end_RF -> signals = NULL;
    end_RF -> sig_cnt = 0;
    end_RF -> fp = fp;
    strcpy (end_RF -> name, fn);

    end_r_path = NULL; end_p = NULL; /* suppress compiler warning */
    rewind (fp);
    if (fscanf (fp, "%le", &(end_RF -> scaling)) <= 0) return (NULL);
    while (skipspace (fp) == '(') {
	getc (fp);  /* '(' */

        NEW (r_path, 1, RES_PATH);
	if (end_RF -> signals == NULL) {
	    end_RF -> signals = r_path;
	    end_r_path = r_path;
	}
	else {
	    end_r_path -> next = r_path;
	    end_r_path = r_path;
	}
	end_r_path -> next = NULL;
	end_r_path -> totnum = 1;

	while (skipspace (fp) != ')') {

	    NEW (p, 1, PATH_SPEC);
	    if (end_r_path -> path == NULL) {
	        end_r_path -> path = p;
		end_p = p;
	    }
	    else {
		end_p -> next = p;
		end_p = p;
	    }
            end_p -> next = NULL;
	    end_p -> xarray[0][0] = 0;

            if (skipspace (fp) == '(') {
	        getc (fp);  /* '(' */
		fscanf (fp, "%s", end_p -> name);

	        i = 1;
	        while (skipspace (fp) != ')') {
	            if (skipspace (fp) == '(') {
                        getc (fp);  /* '(' */
	                fscanf (fp, "%d", &low);
	                fscanf (fp, "%d", &up);
		        skipspace (fp);
                        getc (fp);  /* ')' */
	            }
	            else {
	                fscanf (fp, "%d", &up);
			low = up;
                    }
		    end_p -> xarray[i][0] = low;
		    end_p -> xarray[i][1] = up;
                    if (low > up) {
                        help = up;
                        up = low;
                        low = help;
                    }
	            (end_p -> xarray[0][0])++;
		    end_r_path -> totnum = end_r_path -> totnum * (up - low + 1);
	            i++;
	        }
                getc (fp);  /* ')' */
	    }
	    else {
		fscanf (fp, "%s", end_p -> name);
	    }

	}

	end_RF -> sig_cnt += end_r_path -> totnum;
	skipspace (fp);
	getc (fp);  /* ')' */
    }

    while ((c = getc (fp)) != '\n' && c != EOF) ;

    if (c == EOF) return (NULL);

    end_RF -> offset = ftell (fp);

    return (end_RF);
}
