/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sea_decl.h"
#include <time.h>
#include <stdlib.h>
#include "src/ocean/libseadif/sysdep.h"

char sdftimecvterror[200] = "";

/* Convert yy/mo/dd/hh/mi/ss format to time_t (seconds since Jan. 1, 1970).
 * On success, sdftimecvt returns TRUE and leaves the result in thetime.
 * On failure the function returns 0 and leaves an error message in the global
 * string sdftimecvterror.
 */
int sdftimecvt (time_t *thetime, int yy, int mo, int dd, int hh, int mi, int ss)
{
    time_t tm;
    char  *tz;
    int days, ddFeb, tzone;

    if (!(tz = getenv ("TZ"))) tzone = -1; /* default MET */
    else {
	while (*tz >= 'A') ++tz;
	tzone = atoi (tz);
    }

    ddFeb = 28;
    if ((yy % 4) == 0) ++ddFeb;	/* leap year */

    if (yy >= 1970) yy -= 1900;
    else if (yy < 70) yy += 100; /* years after 2000 */
    days = (yy - 70) * 365;
    days += ((yy - 69) / 4); /* previous years leap days */

    switch (mo) {
	case 12: days += 30;	/* Nov */
	case 11: days += 31;	/* Oct */
	case 10: days += 30;	/* Sep */
	case  9: days += 31;	/* Aug */
	case  8: days += 31;	/* Jul */
	case  7: days += 30;	/* Jun */
	case  6: days += 31;	/* May */
	case  5: days += 30;	/* Apr */
	case  4: days += 31;	/* Mar */
	case  3: days += ddFeb;	/* Feb */
	case  2: days += 31;	/* Jan */
	case  1: break;
	default:
	    sprintf (sdftimecvterror, "invalid month specified: %d", mo);
	    return 0;
    }

    if (dd < 1 || dd > 31
    || (dd > 30 && (mo == 4 || mo == 6 || mo == 9 || mo == 11))
    || (dd > ddFeb && mo == 2)) {
	sprintf (sdftimecvterror, "invalid day specified: %d", dd);
	return 0;
    }
    tm = (days + dd - 1) * 24 * 60 * 60;

    if (hh < 0 || hh > 23) {
	sprintf (sdftimecvterror, "invalid hour specified: %d", hh);
	return 0;
    }
    hh += tzone; /* correct for time zone */
    tm += hh * 60 * 60;

    if (mi < 0 || mi > 59) {
	sprintf (sdftimecvterror, "invalid minute specified: %d", mi);
	return 0;
    }
    tm += mi * 60;

    if (ss < 0 || ss > 59) {
	sprintf (sdftimecvterror, "invalid second specified: %d", ss);
	return 0;
    }
    tm += ss;

    /* Now we need to know if currently daylight saving time is in effect.
     * We do this by calling localtime() and check the tm_isdst flag.
     * This is not completely accurate around 2am on the switch-over days.
     */
    if (localtime (&tm) -> tm_isdst) tm -= 60 * 60; /* adjust for DST */

    if (tm < 0) {
	sprintf (sdftimecvterror, "negative time: time out of date");
	return 0;
    }

    *thetime = tm;
    return TRUE;
}
