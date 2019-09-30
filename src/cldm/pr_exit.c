/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/extern.h"

char *err_list[] = {
/* 0 */  "%s",
/* 1 */  "model end expected",
/* 2 */  "model start expected",
/* 3 */  "no wire statement specified",
/* 4 */  "error: model '%s' already found in (imp)celllist",
/* 5 */  "more than 25 syntax errors",
/* 6 */  "interrupted due to signal: %s",
/* 7 */  "-- program finished --",
/* 8 */  "fatal co-ordinate buffer overflow",
/* 9 */  "model '%s' already exists, not overwritten",
/* 10 */ "model '%s' does not exist",
/* 11 */ "already used instance name: %s",
/* 12 */ "already defined terminal/label name: %s",
/* 13 */ "invalid repetition parameter: %s",
/* 14 */ "warning: parameter %s not on 0.5 lambda grid",
/* 15 */ "warning: too long layercode: %s",
/* 16 */ "warning: too long model name: %s",
/* 17 */ "warning: too long instance name: %s",
/* 18 */ "warning: too long terminal/label name: %s",
/* 19 */ "-- truncated to %s characters",
/* 20 */ "unrecognized layercode: %s",
/* 21 */ "too many arguments specified",
/* 22 */ "cannot access file: %s",
/* 23 */ "no regular file: %s",
/* 24 */ "cannot open file: %s",
/* 25 */ "warning: repetition parameter %s = 0",
/* 26 */ "warning: model '%s' not placed!",
/* 27 */ "cannot allocate enough core",
/* 28 */ "%s: cannot read model info",
/* 29 */ "illegal terminal/label layercode: %s",
/* 30 */ "%s: illegal co-ordinates",
/* 31 */ "%s: illegal increment (0 value(s))",
/* 32 */ "%s: width must be greater than 0",
/* 33 */ "poly: the line parts (partly) overlap",
/* 34 */ "%s: too less co-ordinates",
/* 35 */ "%s: odd number of co-ordinates",
/* 36 */ "poly: last co-ordinates not equal to first pair",
/* 37 */ "model '%s' already defined|used, not overwritten",
/* 38 */ "poly: illegal direction change in co-ordinates",
/* 39 */ "poly: intersecting lines not allowed",
/* 40 */ "parameter not integer: %s",
/* 41 */ "illegal angle: %s",
/* 42 */ "warning: model '%s' already exists, overwritten",
/* 43 */ "warning: value rounded to: %s",
/* 44 */ "warning: angle %s not of 0.001 degree resolution",
/* 45 */ "warning: wire: co-ordinate not on lambda grid",
/* 46 */ "mc: illegal scaling factor: %s",
/* 47 */ "warning: circle/cpeel: n set to %s",
/* 48 */ "circle/cpeel: illegal radius %s",
/* 49 */ "cpeel: %s may not be equal",
/* 50 */ "swire: too big direction change",
/* 51 */ "warning: parameter %s not on 0.25 lambda grid",
/* 52 */ "model definition of model '%s' had error",
/* 53 */ "warning: %s: not 45 degree feature(s)",
/* 54 */ "error: but cannot find error message"
};

void pr_exit (int mode, int nr, char *cs)
{
    int i;
    i = (nr < 0) ? -nr : nr;
    if (i > 54) i = 54;

    if (mode & 004) P_E "%s: ", argv0);
    if (mode & 010) P_E "%d: ", yylineno);
    P_E err_list[i], cs);
    if (mode & 040) P_E ", item: %s", textval());
    if (mode & 020) P_E ", in model: %s", ms_name);
    P_E "\n");

    if (mode & 01) {
	dmQuit ();
	if (b_mode) P_E "\7");
	if (mode & 0100) P_E "%s: -- program aborted --\n", argv0);
	if (mode & 02) exit (1);
	exit (0);
    }

    if (!(mode & 0200)) if (++err_cnt > 25) pr_exit (0107, 5, 0);
    if (!(mode & 0400)) ++err_flag;
}
