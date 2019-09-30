/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	P.E. Menchen
 *	A.J. van Genderen
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

#include "src/csls/slserr.h"
#ifdef NOSYSINCL
#define NULL		0
#endif

#define XSTACK_SIZE	10
#define DEVTABLENGTH	10
#define STACK_OVERFLOW	0x1
#define STACK_OK	0x2

#define ALREADYDEF	1

#define QueueType	0x01
#define StackType	0x02
#define DictType	0x03
#define TermType	0x04
#define NetType		0x05
#define NetRefType	0x06
#define InstanceType	0x07
#define PmQueue		0x08
#define ImQueue		0x09

#define TransistorType	0x10
#define ResistorType	0x11
#define CapacitorType	0x12
#define NetworkType	0x13
#define FunctionType	0x14

#define DevNenh		0
#define DevPenh		1
#define DevNdep		2
#define DevRes		3
#define DevCap		4

#define XELEM		0
#define STRING		1

#define ABS(x)      ((x) < 0 ? (-(x)):(x))

