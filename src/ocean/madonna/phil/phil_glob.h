/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
 * Phil_glob - set of global phil functions
 */

#include "src/ocean/madonna/phil/image.h"
#include "src/ocean/madonna/partitioner/routing.h"

// This structure is used to pass
// the transparency analysis results after
// detailed placement to the global router.

typedef struct _SLICE_INFO
{
  int cX;	// bottom left corner of the slice
  int cY;	// (in basic cell coordinates system)

  int width;	// sizes of the slice (also in basic
  int height;	// cell coordinates system)

  int* layerTrans; // Each position says how many wires can
		// still go through this slice for given layer number.
		// Size of the array == number of layers.

} SLICE_INFO, SLICE_INFO_TYPE, *SLICE_INFOPTR;

int phil (char *cName, char *fName, char *lName, char *oName, double magn,
		CIRCUIT* fromPartitioner,
		CIRCUIT* realCircuit   = NULL,
		GLOBAL_ROUTING* g_rout = NULL);

void       readImageFile();
IMAGEDESC* getImageDesc();

