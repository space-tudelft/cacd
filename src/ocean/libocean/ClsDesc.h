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
 * ClsDesc.h - definitions of class descriptors.
 */

#ifndef __CLSDESC_H
#define __CLSDESC_H

// Here define your new class descriptor
// if You extend this library.

#define  RootClassDesc       0
#define  ReferenceClassDesc  1
#define  ObjectClassDesc     2
#define  NilClassDesc        3
#define  IteratorClassDesc   4
#define  ContainerClassDesc  5
#define  ArrayClassDesc      6
#define  CharClassDesc       7
#define  DoubleClassDesc     8
#define  IntClassDesc        9
#define  LongClassDesc       10
#define  RectangleClassDesc  11
#define  PointClassDesc      12
#define  graphElementClassDesc 13
#define  SortableClassDesc   14
#define  StringClassDesc     15
#define  __FirstUserClassDesc__  100

typedef unsigned char classType;

#endif
