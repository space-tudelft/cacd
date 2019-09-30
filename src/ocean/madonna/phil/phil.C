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
 * Detailed placement in Sea Of Gates
 */

#include <iostream>
using namespace std;
#include <stdlib.h>
#include <string.h>
#include "src/ocean/madonna/phil/plcm.h"
#include "src/ocean/madonna/phil/parserIf.h"

Plcm* plcm;
ParserInterface parInt;	        // This object serves as an interface
				// to "image.seadif" file parser.

extern int phil_verbose;
extern int rand_points;
extern int doCompresion;
extern int set_srand;
extern int macroMinSize;

extern int slicingLayout;
extern int doTranAna;
extern int makeChannels;

extern int requestedGridPoints[]; // Here we keep initial size of placement plane.
extern int expandableDirection;	  // How to increase its size.

#define  INC_FACT     1.3

//-------------------------------------------------------------
// Main routine of detailed placement.
//
// Assumes that :
//	1. Seadif database is already opened
//	2. Partitioned circuit is left in the memory structure
//		pointed by fromPartiotioner pointer.
//
// After calling it external call closing database is required.
//-------------------------------------------------------------

int phil (char *cName, char *fName, char *lName, char *oName, double /* magn */,
		CIRCUIT* fromPartitioner,
		CIRCUIT* realCircuit,
		GLOBAL_ROUTING* glob_rout)
{
    cout << "\n------ Detailed placement of (" << oName << "(" << cName
         << "(" << fName << "(" << lName << "))))...\n";

    if (doTranAna < 0) { doTranAna = 0; }
    if (doTranAna > 0) { doTranAna = 1; slicingLayout = 1; rand_points = 1; }

    plcm = new Plcm (cName, fName, lName, parInt.getImageDesc(), parInt.getImageMap(),
		     oName,
		     fromPartitioner,
		     (makeChannels == 1 ? glob_rout : NULL),
		     Boolean(slicingLayout),
		     Boolean(doTranAna),
                     Boolean(phil_verbose),
                     Boolean(rand_points),
		     set_srand, macroMinSize);

    cout << "------ Reading input data" << endl;

    plcm -> read ();

    cout << "------ Creating temporary data structures" << endl;

    plcm -> prepare ();

    int done;

    do
    {
	cout << "------ Placement plane size : ["
		<< requestedGridPoints[HOR] << ":"
		<< requestedGridPoints[VER] << "]" << endl;

	plcm -> createPlane (requestedGridPoints[HOR], requestedGridPoints[VER]);

	cout << "------ Doing placement " << endl;

	done = !plcm -> placement ();

      if (!done) // will have to increase the size of the placement plane
      {
	if (expandableDirection == HOR)
	    requestedGridPoints[HOR] = int(INC_FACT * requestedGridPoints[HOR]);
	else
	    requestedGridPoints[VER] = int(INC_FACT * requestedGridPoints[VER]);
	plcm -> recover ();
      }
    } while (!done);

    // one small detail:
    // let\'s properly set the bounding box
    // (it may be smaller than placement plane size)

    plcm -> setBbx ();

    if (doCompresion > 0)
    {
	cout << "------ Compacting  " << endl;
	plcm -> compaction ();
    }

    if (doTranAna)
    {
	cout << "------ Transparencies Analysis. " << endl;
	plcm -> doTranspAnalysis ();
    }

//  if (!doTranAna) // it\'s be passed to the global router, so don\'t write it yet.
    if (1)
    {
	cout << "------ Writing created layout to database" << endl;
	plcm -> write (realCircuit);
    }
    else
	plcm -> write (realCircuit, false); // false == don\'t write

    delete plcm;

    return 0;
}

void readImageFile ()
{
    parInt.read ();
}

IMAGEDESC* getImageDesc ()
{
    return parInt.getImageDesc ();
}
