/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

/*
 * Binary search algorithm, generalized from Knuth (6.2.1) Algorithm B.
 */

char *Bsearch (char *key, char *base, unsigned nel, unsigned width, int (*compar)())
/* key    = Key to be located */
/* base   = Beginning of table */
/* nel    = Number of elements in the table */
/* width  = Width of an element (bytes) */
/* compar = Comparison function */
{
	int two_width = width + width;
	char * last = base + width * (nel - 1); /* Last element in table */

	while (last >= base) {

		register char * p = base + width * ((last - base)/two_width);
		register int res = (*compar)(key, p);

		if (res == 0)
			return (p);	/* Key found */
		if (res < 0)
			last = p - (int) width;
			/* use type conversion from unsigned to int to prevent
			   mysterious error on hp735 (AvG) */
		else
			base = p + width;
	}
	return ((char *) 0);		/* Key not found */
}
