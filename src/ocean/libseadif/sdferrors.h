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
/*
 * This is the list of return statussus for the seadif tools.
 */

#define SDF_NOERROR               0 /* no error */

/* FATAL errors */
#define SDFERROR_NOT_FOUND        9 /* cell was not found */
#define SDFERROR_CALL            10 /* called with wrong parameters */
#define SDFERROR_SEADIF          11 /* problems opening/working with seadif */
#define SDFERROR_FILELOCK        18 /* file in the database is locked, cannot open */
#define SDFERROR_INCOMPLETE_DATA 12 /* (sea) task aborted because of incomplete data */
#define SDFERROR_NELSEA_FAILED   13 /* nelsea failed for some reason */
#define SDFERROR_MADONNA_FAILED  14 /* madonna failed for some reason */
#define SDFERROR_ROUTER_FAILED   15 /* router failed completely for some reason */
#define SDFERROR_RUNPROG         16 /* call of sub-program failed */

#define SDFERROR_NELSIS_OPENING  20 /* NELSIS cannot be opened */
#define SDFERROR_NELSIS_CELL     21 /* NELSIS cannot find some file */
#define SDFERROR_NELSIS_INTERNAL 22 /* any other nelsis error */

/*
 * non-fatal errors
 */
#define SDFERROR_WARNINGS        50 /* some non-fatal warnings were given */
#define SDFERROR_INCOMPLETE      51 /* routing was incomplete */

