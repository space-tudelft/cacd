/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
 *	Paul Stravers
 *	Patrick Groeneveld
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

#ifndef __SEADIFGRAPH_H
#define __SEADIFGRAPH_H

#include "src/ocean/libocean/sdfset.h"
#include "src/ocean/libocean/sdfGraph.h"

//////////////////////////////////////////////////////////////////////////////
//				     grNet                                  //
//////////////////////////////////////////////////////////////////////////////
// We must be able to build a set of grNet objects, that's why we derive from
// sdfsetElmt. A set of grNets is a bus (actually: anotherBus).
//
class grNet: public gNet, public sdfsetElmt
{
public:
   grNet(NETPTR n);
   virtual void print() {gNet::print(); sdfsetElmt::print();}
};

gNet *new_grNet(NETPTR sdfnet);	// used by buildCircuitGraph() in makebus.C;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//				  anotherBus                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// This class holds a set of grNets that form one bus. We also derive from
// sdfsetElmt so that we can have a set of buses.
class anotherBus: public sdfset, public sdfsetElmt
{
private:
   char *_busName;
public:
   anotherBus(char *busName);
   ~anotherBus();
   STRING busName() {return _busName;}
   virtual void print() {sdfset::print(); sdfsetElmt::print();}
};

#endif // __SEADIFGRAPH_H
