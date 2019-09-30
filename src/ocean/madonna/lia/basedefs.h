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
#ifndef __BASEDEFS_H
#define __BASEDEFS_H

// This file contains some basic declarations used everywhere

typedef unsigned int  classType;
typedef unsigned int  sizeType;
typedef int           countType;

#define itemClass        0
#define dummyClass       (itemClass+1)
#define listElementClass (dummyClass+1)
#define boxClass         (listElementClass+1)
#define packageClass     (boxClass+1)
#define arrayClass       (packageClass+1)
#define listClass        (arrayClass+1)

#define __endOfLibClass  255
#define __firstUserClass __endOfLibClass+1

#endif
