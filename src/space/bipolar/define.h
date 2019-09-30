/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	Frederik Beeftink
 *	Simon de Graaf
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

/* Anode and cathode terminals
 * of the junctions.
 */
#define AN	0
#define CA	1

/* Collector, emitter and base
 * terminals of the bipolar transistors.
 */
#define CO	0
#define EM	1
#define BA	2
#define SU	3

/* Get junction terminal of
 * same and opposite type as polnode
 */
#define SPN(pn)  (pn -> type == 'n' ? CA : AN)
#define OPN(pn)  (pn -> type == 'n' ? AN : CA)

