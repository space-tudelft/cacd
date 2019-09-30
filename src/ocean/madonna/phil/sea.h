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
 * Sealib functions declarations
 */

#ifndef __SEA_H
#define __SEA_H

// Some common definitions:
//
#include "src/ocean/madonna/phil/image.h"

#undef  NULL
#define NULL 0

#if defined(__GNUC__) && ((__GNUC__ >= 2 && __GNUC_MINOR__ >= 6) || __GNUC__ >= 3)
typedef bool Boolean;
#else
typedef enum { false = int(0), true = int(1) } Boolean;
#endif

#include "src/ocean/libseadif/sea_func.h"
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/phil/im.h"

#endif
