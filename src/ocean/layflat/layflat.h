/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Simon de Graaf
 *	Kees-Jan van der Kolk
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

#ifndef _LAYFLAT_H
#define _LAYFLAT_H

#define XL 0
#define XR 1
#define YB 2
#define YT 3

/* values for the mtx[] index */
#define A11 0
#define A12 1
#define B1  2
#define A21 3
#define A22 4
#define B2  5

#define HOR 0
#define VER 1

#define TOPLEVEL 0

#define MAXINT 0x7fffffff
#define MININT 0x80000000

/* type of a matrix element (in Seadif it's short, in Nelsis it's long) */
typedef long MTXELM;

typedef struct _DIFF_TERM
{
    char name[DM_MAXNAME+1];
    struct _EQUIV_TERM *equiv;
    struct _DIFF_TERM  *next;
} DIFF_TERM;

typedef struct _EQUIV_TERM
{
    struct geo_term    term;
    struct _EQUIV_TERM *next;
} EQUIV_TERM;

#endif
