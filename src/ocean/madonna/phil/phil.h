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

//#include "src/ocean/madonna/lia/basedefs.h"
#include "src/ocean/madonna/phil/usrlib.h"
#include "src/ocean/madonna/phil/sea.h"

#ifndef __PHIL_H
#define __PHIL_H

// Some global definitions :

#define PointClass      __firstUserClass
#define DivTabClass       (PointClass    + 1)
#define ClusterClass      (DivTabClass   + 1)
#define PatternClass      (ClusterClass  + 1)
#define WindowClass       (PatternClass  + 1)
#define PlaneClass        (WindowClass   + 1)
#define PlcmClass         (PlaneClass    + 1)
#define ClstClass         (PlcmClass     + 1)
#define CriPointClass     (ClstClass     + 1)
#define ImageMapClass     (CriPointClass + 1)
#define LayMapClass       (ImageMapClass + 1)
#define CriNetClass       (LayMapClass   + 1)
#define PlaneCellClass    (CriNetClass   + 1)
#define CriCandidateClass (PlaneCellClass + 1)
#define ProtAreaClass     (CriCandidateClass + 1)

// Layer identifiers :

typedef enum
{
  NoneLayer = 0,
  DiffusionLayer = 0x01, // or restricted feed
  PolyLayer      = 0x02, // or universal feed
  Metal1Layer    = 0x04,
  Metal2Layer    = 0x08,
  MetalsViaLayer = 0x10, // via from metal 1 to metal 2
  ViaLayer       = 0x20, // via from metal

} layerType;

typedef enum // number of bit inside layoutMapType
{
  CriticalAreaFlag = 0x40,
  TerminalFlag     = 0x80

} flagType;

typedef struct
{
  unsigned  char  layers;  // bits in layers and visited have
  unsigned  char  visited; // meaning defined in layerType
  unsigned  char  uniNo;
  unsigned  int   termNo;

  unsigned  char  upWay;   // information about layers around connected
  unsigned  char  downWay; // with this of our actual grid point
  unsigned  char  leftWay;
  unsigned  char  rightWay;

} layoutMapType; // one grid point

// Some global functions :

layerType recognizeLayer (short layer);

#include "phil_glob.h"

#endif
