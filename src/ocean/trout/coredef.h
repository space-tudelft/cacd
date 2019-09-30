/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
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

/*
 * coredef.h
 *
 *    CORE-IMAGE DEPENDENT DEFINES
 *********************************************************/

#define  COREUNIT        char     /* unit in which the image is stored */
#define  GRIDADRESSUNIT  long     /* unit of index */

/*
 * Meaning of the individual bits in the grid.
 * The first 6 bits code the connection pattern of the wire-element.
 * The relative ordering of these bits should not be changed.
 * The next two bits code the state:
 * Bit 8 (FRONT2) Bit 7 (FRONT1)
 *     0          0        grid point is free (unoccupied)
 *     0          1        grid point belongs to first expansion front (source)
 *     1          0        grid point belongs to second expandsion front (destination)
 *     1          1        grid point is occupied
 */
#define  L        0     /* connects to left */
#define  R        1     /* connects to right */
#define  B        2     /* connects to bottom */
#define  T        3     /* connects to top */
#define  D        4     /* connects downward (a via) */
#define  U        5     /* connects upward (a via) */

#define  HERE     U+1

#define  PATTERNMASK  0x3F  /* 00111111  masks of the pattern */
#define  STATEMASK    0xC0  /* 11000000  masks of the state */
#define  FRONT1       0x40  /* 01000000  mask of front1 */
#define  FRONT2       0x80  /* 10000000  mask of front2 */

#define  HORIZONTAL   0     /* layer orientation */
#define  VERTICAL     1

/* step though each of the 6 possible offsets */
#define for_all_offsets(offset) for (offset = L; offset < HERE; ++offset)

