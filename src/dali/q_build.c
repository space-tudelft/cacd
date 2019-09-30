/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
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

#include "src/dali/header.h"

#define	INIT_WINDOW 2048

/*
** Build an empty quadtree: qroot.
** OUTPUT: pointer to root of empty quad tree.
*/
qtree_t * qtree_build ()
{
    qtree_t *qroot;		/* qroot is root of quadtree */
    MALLOC (qroot, qtree_t);	/* allocate memory for root */
    qroot -> object = NULL;	/* linked list of nodes : empty */
    qroot -> reference = NULL;
    qroot -> Ncount = 0;	/* number of objects	: 0 */
    qroot -> Uleft  = NULL;
    qroot -> Uright = NULL;
    qroot -> Lleft  = NULL;
    qroot -> Lright = NULL;
    qroot -> quadrant[0] = 0;	/* predefined region */
    qroot -> quadrant[1] = 0;
    qroot -> quadrant[2] = INIT_WINDOW;
    qroot -> quadrant[3] = INIT_WINDOW;
    return (qroot);
}
