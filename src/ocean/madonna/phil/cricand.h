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

#ifndef __CRICAND_H
#define __CRICAND_H

#include "src/ocean/madonna/phil/cri.h"
#include "src/ocean/madonna/phil/cluster.h"

// This class used in the list of potential critical points in basic image.
// It contains additional information about neighbor sectors of critical point
// in case of restricted feed and sectors in which there's a critical universal feed.
// This information is stored in field: neighbors - appropriate bits are set.

class CriCandidate : public CriPoint
{
public :
  CriCandidate(int x, int y, layerType lay, unsigned int k, clusterMapType nb):
					CriPoint(x, y, lay, k), neighbors(nb) {};
 ~CriCandidate() {};

  virtual classType myNo() const { return CriCandidateClass; }
  virtual char*   myName() const { return (char*)"CriCandidate"; }

  clusterMapType neighbors;
};

#endif
