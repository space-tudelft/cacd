/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
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
 * SIMPLE CLASS LIBRARY
 *
 * This file contains definition of classes: Root
 */

#ifndef __ROOT_H
#define __ROOT_H

#ifdef NULL
#undef NULL
#define NULL 0
#endif

class Object;

// This is the lowest level of hierarchy.
// This class contains static members used everywhere.
//
class Root
{
public:
    Root() {};
    virtual ~Root() {};

    virtual const char* className() const = 0;

    static void  criticalError (int err, Object& o);
    static void  criticalError (char* msg, Object& o);
    static void  error (int err, Object& o);
    static void  error (char* msg, Object& o);
    static void  warning (int err, Object& o);
    static void  warning (char* msg, Object& o);

    static const char  *errMsgs[];
    static int   errMsgNum;

    static void  **allocArray2 (int xsize, int ysize, int itemSize);
    static void  freeArray2 (int xsize, void **toFree);
};

// Common error numbers definitions:

#define  NO_ERROR    0
#define  EDOS        1
#define  ESYS        2
#define  ENOTMEM     3
#define  EWRCFG      4
#define  EINPDAT     5
#define  EFNFND      6
#define  EUNKNOW     7
#define  EWRITE      8
#define  ETASKIMP    9
#define  EINDEX     10
#define  ENOTIMP    11
#define  ESHNIMP    12

#endif