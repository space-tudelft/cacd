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
 * Various user defined functions.
 */

#include <iostream>
using namespace std;
#include "src/ocean/libseadif/sea_decl.h"

#ifndef __STDLIB_H
#include <stdlib.h>
#endif

#ifndef __USRLIB_H
#include "src/ocean/madonna/phil/usrlib.h"
#endif

#include <malloc.h>

const char *usrErrCodes[] = {
/*  NO_ERROR    0   */    "",
/*  EDOS        1   */    "Operating System Dummy",
/*  ESYS        2   */    "C++ System Dummy",
/*  ENOTMEM     3   */    "Not enough memory",
/*  EWRCFG      4   */    "Wrong parameter",
/*  EINPDAT     5   */    "Input data error",
/*  EFNFND      6   */    "File not found",
/*  EUNKNOW     7   */    "Unknown error",
/*  EWRITE      8   */    "Can not write data",
/*  ETASKIMP    9   */    "Can not do task",
/*  EINDEX     10   */    "Index out of range"
};

// display error message to stderr and exit the program
void usrErr (char *place, int nr)
{
    warning (place, nr);
    sdfexit (1);
}

// display warning message to stderr
void warning (char *place, int nr)
{
    cerr << "\n" << place << " : " << usrErrCodes[nr] << " !\n";
}

// allocate 2-dimensional array of data objects of requested size
void** allocArray2 (int xsize, int ysize, int itemSize)
{
    void** arr = (void**) calloc (xsize, sizeof(void*));

    if (!arr) usrErr ((char*)"allocArray2", ENOTMEM);

    for (int i = 0; i < xsize; i++) {
	if (!(arr[i] = calloc (ysize, itemSize)))
	      usrErr ((char*)"allocArray2", ENOTMEM);
    }
    return arr;
}

// free 2-dimensional array
void freeArray2 (int xsize, void** arr)
{
    for (int i = 0; i < xsize; i++) free (arr[i]);
    free (arr);
}
