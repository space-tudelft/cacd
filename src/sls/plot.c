/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#include "src/sls/extern.h"

FILE * fp_plt = NULL;
simtime_t lastplottime = -1;

static int *PX;
int PX_cnt;

void plot_addname (PATH_SPEC *path)
{
    STRING_REF * str_ref;

    if (make_output) return;

    str_ref = names_from_path (0, path);

    if (!fp_plt) {
	OPENW (fp_plt, fn_plt);
	fprintf (fp_plt, "%12d \n", 0);  /* space for number of signals */
    }

    while (str_ref) {
	fprintf (fp_plt, "%s\n", str_ref -> str);
	str_ref = str_ref -> next;
    }
}

void plot_scale ()
{
    if (fp_plt) {
	fprintf (fp_plt, "%e\n", outtimeaccur);
	fprintf (fp_plt, "%e\n", vHtmp / vH);
	fprintf (fp_plt, "0 %d\n", vH);
    }
}

static void plot_index (int x, char bound, int v)
{
    if (lastplottime != tcurr) {
	fflush (fp_plt);
        if (lastplottime >= 0)
            fprintf (fp_plt, "\n%lld", tcurr);
        else
            fprintf (fp_plt, "%lld", tcurr);
        lastplottime = tcurr;
    }

    fprintf (fp_plt, " %d%c %d", x, bound, v);
}

void plot_node (NODE *n, char bound, int v)
{
    int cnt, x;

    x = n -> plot;
    for (cnt = PX[x]; cnt > 0; cnt--) {
	x++;
	plot_index (PX[x], bound, v);
    }
}

void plot_begin ()
{
    NODE_REF_LIST * plt_ref;
    UPAIR * up;
    int i;
    int newPX_cnt;
    int p_cnt;
    int x;

    if (fp_plt) {

	if (plotl_begin) {

	    /* create PX[] */

	    PALLOC (PX, 1 + plotnodes_cnt * 2, int);

	    /* make N[].plot point to an index of PX[],
	       and fill PX[] (partly) with number of through-references */

	    PX_cnt = 1;  /* !!! */
	    for (i = 0; i < N_cnt; i++) {
		if (N[i].plot > 0) {
		    newPX_cnt = PX_cnt + 1 + N[i].plot;
		    PX[PX_cnt] = N[i].plot;
		    N[i].plot = PX_cnt;
		    PX_cnt++;
		    while (PX_cnt < newPX_cnt) {
			PX[PX_cnt] = -1;  /* indicating empty position */
			PX_cnt++;
		    }
		}
	    }

            /* create references from PX[] to indices in plotfile */

	    plt_ref = plotl_begin;
	    p_cnt = 1;
	    while (plt_ref) {
		if (plt_ref -> nx >= 0) {
		    i = N[plt_ref -> nx].plot;
		    while (PX[i] >= 0) i++;
		    PX[i] = p_cnt++;
		}
		plt_ref = plt_ref -> next;
	    }
	}

        plt_ref = plotl_begin;
	x = 1;
        while (plt_ref) {
            if (plt_ref -> nx >= 0) {
                up = uminmax (&N[plt_ref -> nx]);
                plot_index (x, 'l', (int)(up -> umin));
                plot_index (x, 'u', (int)(up -> umax));
		x++;
            }
            plt_ref = plt_ref -> next;
        }

	fflush (fp_plt);
    }
}

void plot_end ()
{
    NODE_REF_LIST * plt_ref;
    UPAIR * up;
    int x;

    if (fp_plt) {
        plt_ref = plotl_begin;
	x = 1;
        while (plt_ref) {
            if (plt_ref -> nx >= 0) {
                up = uminmax (&N[plt_ref -> nx]);
                plot_index (x, 'l', (int)(up -> umin));
                plot_index (x, 'u', (int)(up -> umax));
		x++;
            }
            plt_ref = plt_ref -> next;
        }
        fprintf (fp_plt, "\n");
	rewind (fp_plt);
	fprintf (fp_plt, "%12d", plotnodes_cnt);
	fclose (fp_plt);
    }
}
