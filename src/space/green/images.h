/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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
 * Changed by Theo Smedes for first CYLBEM implementation.
 * see also mkgreen.c, init.c, colloc.c
 * started: 20-06-1995
 * last modified: 26-06-1995
 */

/*
 * Generated automatically by a Mathematica program.
 * T.S.:
 * Originally generated automatically by a Mathematica program for Dirichlet
 * Green's function.
 * However heavily edited for Neuman Green's function.
 * This file is included in makeGreen.c
 */

/* T.S.:
 * added first argument '0' for greenType: capacitance Green's function
 */

if (greenType > 1) goto substrate_greens;

greenSeries (0, 2, 1, 1) {
	n = 1 + p(0);
	S = k(1,n)/e(1);
	image(-S,-2*t(1)*(n),-1);
	image(-S, 2*t(1)*(n),-1);
	image( S,-2*t(1)*(n), 1);
	image( S, 2*t(1)*(n), 1);
	goto ret;
}

greenSeries (0, 2, 2, 1) {
    if (useOldImages) {
	n = 1 + p(0);
	S = k(1,n)/e(1);
	image(-S, 2*t(1)*(n-1), 1);
	image(-S, 2*t(1)*(n  ),-1);
	image( S, 2*t(1)*(n-1),-1);
	image( S, 2*t(1)*(n  ), 1);
    }
    else {
	n = 1 + p(0);
	S = 2*k(1,n)/(e(1)+e(2));
	image(-S, 2*t(1)*(n),-1);
	image( S, 2*t(1)*(n), 1);
    }
	goto ret;
}

greenSeries (0, 2, 1, 2) {
    if (useOldImages) {
	n = 1 + p(0);
	S = k(1,n)/e(2);
	image(-S,-2*t(1)*(n-1),-1);
	image(-S,-2*t(1)*(n  ),-1);
	image( S, 2*t(1)*(n-1), 1);
	image( S, 2*t(1)*(n  ), 1);
    }
    else {
	n = 1 + p(0);
	S = 2*k(1,n)/(e(1)+e(2));
	image(-S,-2*t(1)*(n),-1);
	image( S, 2*t(1)*(n), 1);
    }
	goto ret;
}

greenSeries (0, 2, 2, 2) {
    if (useOldImages) {
	n = 1 + p(0);
	S = k(1,n)/e(2);
	image(-S, 2*t(1)*(n-2), 1);
	image( S, 2*t(1)*(n  ), 1);
    }
    else {
	n = 1 + p(0);
	S = 2*k(1,n)/(e(1)+e(2));
	image(-S, 2*t(1)*(n-1), 1);
	image( S, 2*t(1)*(n  ), 1);
    }
	goto ret;
}

greenSeries (0, 3, 1, 1) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(1);
	image(-S,-2*t(1)*(n+1) - 2*t(2)*(m),-1);
	image(-S, 2*t(1)*(n+1) + 2*t(2)*(m),-1);
	image( S,-2*t(1)*(n+1) - 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(1);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 2, 1) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(1);
	image(-S, 2*t(1)*(n  ) + 2*t(2)*(m), 1);
	image(-S, 2*t(1)*(n+1) + 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(1);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(1);
	image(-S,-2*t(1)*(n  ) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n  ) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 3, 1) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(1);
	image(-S, 2*t(1)*(n  ) + 2*t(2)*(m), 1);
	image(-S, 2*t(1)*(n+1) + 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m  ), 1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(1);
	image(-S, 2*t(1)*(n  ) + 2*t(2)*(  m), 1);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(  m),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 1, 2) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(2);
	image(-S,-2*t(1)*(n+1) - 2*t(2)*(m),-1);
	image(-S,-2*t(1)*(n  ) - 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(2);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(2);
	image(-S,-2*t(1)*(n  ) - 2*t(2)*(m+1), 1);
	image(-S,-2*t(1)*(n-1) - 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(m+1),-1);
	goto ret;
}

greenSeries (0, 3, 2, 2) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(2);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(2);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(2);
	image(-S,-2*t(1)*(n-1) - 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n+1) - 2*t(2)*(m+1), 1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 3, 2) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(2);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(2);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m  ), 1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(2);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m  ), 1);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 1, 3) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(3);
	image(-S,-2*t(1)*(n+1) - 2*t(2)*(m),-1);
	image(-S,-2*t(1)*(n  ) - 2*t(2)*(m),-1);
	image( S, 2*t(1)*(n  ) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(3);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m  ),-1);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m  ), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(3);
	image(-S, 2*t(1)*(n  ) + 2*t(2)*(m  ), 1);
	image(-S,-2*t(1)*(n-1) - 2*t(2)*(m+1),-1);
	image( S,-2*t(1)*(n  ) - 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 2, 3) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(3);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(3);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m  ),-1);
	image(-S,-2*t(1)*(n) - 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m  ), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(3);
	image(-S,-2*t(1)*(n+1) - 2*t(2)*(m  ),-1);
	image(-S,-2*t(1)*(n-1) - 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m  ), 1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
	goto ret;
}

greenSeries (0, 3, 3, 3) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(3);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(3);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m-1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(3);
	image(-S, 2*t(1)*(n+1) + 2*t(2)*(m-1), 1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
}
goto ret;

substrate_greens:
if (greenType > 2) goto substrate_type3;

/* T.S.:
 * value 2 for first argument denotes substrate resistance Green's function.
 */

greenSeries (2, 2, 1, 1) {
/* T.S.:
 * this only implements a limited green's function (z0==0, b==0)
 */
	n = p(0);
	S = 2*k(1,n)/s(1);
	D = 2*b(1)*(n);
    if (useOldImages) {
	image(S,-D, 1);
	image(S, D, 1);
    }
    else {
	image(2*S,-D, 1);
    }
	goto ret;
}

substrate_type3:
/* SdeG:
 * value 3 for first argument denotes grounded substrate resistance Green's function.
 */

greenSeries (3, 2, 1, 1) {
	n = 1 + p(0);
	S = k(1,n)/e(1);
	D = 2*t(1)*(n);
	image(-S,-D,-1);
	image(-S, D,-1);
	image( S,-D, 1);
	image( S, D, 1);
	goto ret;
}

greenSeries (3, 2, 2, 2) {
	n = 1 + p(0);
    if (useOldImages) {
	S = k(1,n)/e(2);
	image(-S, 2*t(1)*(n-2), 1);
	image( S, 2*t(1)*(n  ), 1);
    }
    else {
	S = 2*k(1,n)/(e(1)+e(2));
	image(-S, 2*t(1)*(n-1), 1);
	image( S, 2*t(1)*(n  ), 1);
    }
	goto ret;
}

greenSeries (3, 3, 2, 2) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
    if (useOldImages) {
	S = k(1,u+1)*k(2,m)/e(2);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m+1),-1);
	S = k(1,u)*k(2,m+1)/e(2);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+2),-1);
	S = k(1,u+1)*k(2,m+1)/e(2);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m  ),-1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+2),-1);
    }
    else {
	S = k(1,u+1)*(k(2,m) + 2*k(2,m+1))/e(2);
	image(-S , 2*t(1)*(n-1) + 2*t(2)*(m+1),-1);
	S = k(1,u+1)*k(2,m)/e(2);
	image( S , 2*t(1)*(n+1) + 2*t(2)*(m+1),-1);
	S = k(1,u)*k(2,m+1)/e(2);
	image( S  , 2*t(1)*(n ) + 2*t(2)*(m  ),-1);
	image(-S*2, 2*t(1)*(n ) + 2*t(2)*(m+1),-1);
	image( S  , 2*t(1)*(n ) + 2*t(2)*(m+2),-1);
	S = k(1,u+1)*k(2,m+1)/e(2);
	image( S , 2*t(1)*(n+1) + 2*t(2)*(m  ),-1);
	image( S , 2*t(1)*(n-1) + 2*t(2)*(m+2),-1);
    }
	goto ret;
}

greenSeries (3, 3, 3, 3) {
	n = p(0) - p(2);
	u = p(0) + p(2);
	m = p(1) + p(2);
	S = k(1,u+1)*k(2,m)/e(3);
	image(-S, 2*t(1)*(n-1) + 2*t(2)*(m), 1);
	image( S, 2*t(1)*(n+1) + 2*t(2)*(m), 1);
	S = k(1,u)*k(2,m+1)/e(3);
	image(-S, 2*t(1)*(n) + 2*t(2)*(m-1), 1);
	image( S, 2*t(1)*(n) + 2*t(2)*(m+1), 1);
	S = k(1,u+1)*k(2,m+1)/e(3);
	image(-S, 2*t(1)*(n+1) + 2*t(2)*(m-1), 1);
	image( S, 2*t(1)*(n-1) + 2*t(2)*(m+1), 1);
}
