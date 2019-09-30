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

static void getsigev  (SIGNALELEMENT *head, simtime_t t, simtime_t *tevent);
static int  getsigval (SIGNALELEMENT *head, simtime_t t);

void getsignal (NODE *n, simtime_t t, SIGNALEVENT *sigev)
{
    static int warningdone = FALSE;
    simtime_t t_s;
    simtime_t tevent_s;
    simtime_t tnextevent_s;
    simtime_t tev;
    simtime_t tnev;
    double mult;
    int first;

    first = TRUE;
    if (t < 0) {
        sigev -> time = 0;    /* the first event has to be get */
        sigev -> val = getsigval (n -> forcedinfo -> insignal, 0);
        return;
    }

    if (n -> forcedinfo -> sigmult != 1)
	mult = doub_conv (n -> forcedinfo -> sigmult * sigtoint);
    else
	mult = sigtoint;

    t_s = D_ROUND (t / mult);

    getsigev (n -> forcedinfo -> insignal, t_s, &tnextevent_s);
    do {
	if (!first && !warningdone) {
	    slserror (NULL, 0, WARNING, "multiple signal change within simulation time accurracy", NULL);
	    warningdone = TRUE;
	}
	first = FALSE;
	tevent_s = tnextevent_s;
	if (tevent_s >= 0)
            getsigev (n -> forcedinfo -> insignal, tevent_s, &tnextevent_s);
        tev  = D_ROUND (tevent_s     * mult);
        tnev = D_ROUND (tnextevent_s * mult);
    }
    while ((tev == tnev || tev <= t) && tevent_s >= 0 && tnextevent_s >= 0);

    if (tevent_s < 0 || tev == t) {
	sigev -> time = -1;
    }
    else {
        sigev -> time = tev;
        sigev -> val = getsigval (n -> forcedinfo -> insignal, tevent_s);
    }
}

static void getsigev1 (SIGNALELEMENT *sgn_ptr, simtime_t t, simtime_t *tevent)
{
    if (t < sgn_ptr -> len || sgn_ptr -> len < 0) {
	if (sgn_ptr -> child) {
	    getsigev1 (sgn_ptr -> child -> sibling, t % sgn_ptr -> child -> len, tevent);
	}
	else {
	    *tevent += sgn_ptr -> len - t;
	}
    }
    else
	if (t >= sgn_ptr -> len) {
	    getsigev1 (sgn_ptr -> sibling, t - sgn_ptr -> len, tevent);
	}
}

static void getsigev (SIGNALELEMENT *sgn_ptr, simtime_t t, simtime_t *tevent)
{
    t += sig_toffset;

    *tevent = 0;

    if (t >= 0) {
	if (t >= sgn_ptr -> len && sgn_ptr -> len > 0) {
	    *tevent = -1;
	}
	else {
	    getsigev1 (sgn_ptr -> sibling, t, tevent);
	    if (*tevent >= 0)
		*tevent += t;
	}
    }

    *tevent -= sig_toffset;
}

static int getsigval1 (SIGNALELEMENT *sgn_ptr, simtime_t t)
{
    if (t < sgn_ptr -> len || sgn_ptr -> len < 0) {
	if (sgn_ptr -> child)
	    return (getsigval1 (sgn_ptr -> child -> sibling, t % sgn_ptr -> child -> len));
	else
	    return (sgn_ptr -> val);
    }

    return (getsigval1 (sgn_ptr -> sibling, t - sgn_ptr -> len));
}

static int getsigval (SIGNALELEMENT *sgn_ptr, simtime_t t)
{
    t += sig_toffset;

    if (t < 0)
	return (sgn_ptr -> sibling -> val);
    else {
	if (t >= sgn_ptr -> len && sgn_ptr -> len > 0)
	    return (sgn_ptr -> val);
	else
	    return (getsigval1 (sgn_ptr -> sibling, t));
    }
}

double doub_conv (double d)
/* converts the first 6 decimals of double */
/* to a new double and fill rest with 0's */
{
    int i;
    double e;

    if (d < 0.5) {
	if (d <= 0) slserror (NULL, 0, ERROR1, "doub_conv: d <= 0", NULL);
	e = 1;
	while (d < 0.5) { e *= 10; d *= 10; }
	i = D_ROUND (d * 100000);
	d = (double)i / e / 100000;
    }
    else {
	e = 1;
	while (d > 5.0) { e *= 10; d /= 10; }
	i = D_ROUND (d * 100000);
	d = (double)i * e / 100000;
    }

    return (d);
}
