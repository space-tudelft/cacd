/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Viorica Simion
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

#ifndef __COLAPSHEAD_H
#define __COLAPSHEAD_H

#include <time.h>
#include <iostream>
using namespace std;
#include <string.h>
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sdferrors.h"
#include "src/ocean/libocean/sdfNameIter.h"
#include "src/ocean/libseadif/libstruct.h"
#include <ctype.h>
#include <unistd.h>		// prototypes getopt()
#include <stdlib.h>		// prototypes exit()
#include "src/ocean/colaps/thedate.h"

#define ITEXISTS 0  // check if a net or inst name already exist
#define MAKEIT   1  // if not, use the new name
#define NOCOPY   1  // net with more then 2 self terminals
#define NETEXIST 1  // check if net still exist
#define NONET    0  // or was it collapsed

// define an instance path structure
typedef struct _InstPath
{
   STRING circuitName;
   struct _InstPath *next;
}
InstPath, InstPathType, *InstPathPtr;

//define a list of instances for cirport flag
typedef struct _CirportFlag
{
   CIRINSTPTR inst;
   struct _CirportFlag *next;
}
CirportFlag, CirportFlagType, *CirportFlagPtr;

// define a list for instances that are not flated
typedef struct _NonFlat
{
   STRING inst;      // do not flat this instance
   STRING circuit;   // do not flat this circuit
   struct _NonFlat *next;
}
NonFlat, NonFlatType, *NonFlatPtr;

// define a list for nets
typedef struct _ShortFlag
{
   NETPTR net;   // net to be collapsed
   STRING name;  // problematic net in child circuit
   struct _ShortFlag *next;  // next short net
}
ShortFlag, ShortFlagType, *ShortFlagPtr;

// define the work structure for flattening the instances
typedef struct _WorkFlag
{
   CIRINSTPTR next;   // next instance flated
   ShortFlagPtr shortFlag;        // net doubled
   CIRINSTPTR refInst; // refernce instance pointer
   CIRCUITPTR circuit; // flat circuit pointer
}
WorkFlag, WorkFlagType, *WorkFlagPtr;

// define my own flag structure
typedef struct _MyFlag
{
   WorkFlagPtr workFlag;
   InstPathPtr instPath; // keep track of the circuit path
}
MyFlag, MyFlagType, *MyFlagPtr;

#define NewMyFlag(p)      ((p) = (MyFlagPtr)mnew(sizeof(MyFlagType)))
#define NewShortFlag(p)   ((p) = (ShortFlagPtr)mnew(sizeof(ShortFlagType)))
#define NewNonFlat(p)     ((p) = (NonFlatPtr)mnew(sizeof(NonFlatType)))
#define NewCirportFlag(p) ((p) = (CirportFlagPtr)mnew(sizeof(CirportFlagType)))
#define NewInstPath(p)    ((p) = (InstPathPtr)mnew(sizeof(InstPathType)))
#define NewWorkFlag(p)    ((p) = (WorkFlagPtr)mnew(sizeof(WorkFlagType)))

// public functions: in liftAll.C
void colaps(CIRCUITPTR sdfCircuit, NonFlatPtr firstNon, int toCellLib, char *outfile, char *outcircuit);
CIRINSTPTR liftAll (CIRINSTPTR inst);
CIRINSTPTR liftAll (CIRINSTPTR inst, NonFlatPtr firstNon);
CIRINSTPTR liftAllToCell (CIRINSTPTR inst);
CIRINSTPTR liftAllToCell (CIRINSTPTR inst, NonFlatPtr firstNon);
int checkInst (CIRINSTPTR inst, NonFlatPtr firstNon);
int checkForLibCell (CIRINSTPTR inst);
CIRINSTPTR findNextInst(CIRINSTPTR inst);
void colapsError (const char *s);

// in colapsInst.C
CIRINSTPTR flatCirinst (_CIRINST *instToBeFlat);
InstPathPtr addCircuitPath (InstPathPtr firstPath, InstPathPtr oldPath, CIRCUITPTR circuit);
InstPathPtr addCircuitPath (InstPathPtr firstPath, InstPathPtr newPath);

// in colapsNet.C
void liftNets (CIRCUITPTR circuit);
void findCirport(NETPTR net);
NETPTR flatNetlist(NETPTR netToBeFlat);
int checkInstName (CIRINSTPTR inst, NonFlatPtr firstNon);
CIRINSTPTR flatCirinst (_CIRINST *instToBeFlat);
CIRPORTREFPTR flatTerminal(CIRPORTREFPTR termToBeFlat);
CIRPORTREFPTR deleteTerm(CIRPORTREFPTR shortTerm);
NETPTR deleteNet(NETPTR net);
void cleanUp(CIRINSTPTR firstInst);
CIRPORTREFPTR colapsNets(CIRPORTREFPTR shortTerm, ShortFlagPtr bufShort);
CIRINSTPTR findInst (CIRINSTPTR prevInst, CIRINSTPTR refInst);
NETPTR addNewNets (NETPTR lastCurNet);
int checkNet(NETPTR onlineNet,CIRPORTPTR onlineCirport,CIRINSTPTR onlineINST);
NETPTR addNewTerm(NETPTR newNet,NETPTR onlineNet,CIRINSTPTR onlineInst);
CIRPORTPTR addInstForCirport (CIRINSTPTR inst, CIRPORTPTR cirport);

// in makeName.C
int checkName(STRING newName, NETPTR netOrInst, NETPTR lastCurNet);
STRING makeName (NETPTR net, NETPTR lastCurNe);
char *concat(char* fatherName, char* childName);
char *concatName(char* fatherName, char* childName);
char *long2ascii(long n);

//in localCopy.C
CIRCUITPTR makeLocalCopy (CIRCUITPTR remote, char *outcircuit);
LIBRARYPTR findLocalLib ();
char *onlyLibName (char *s);
CIRCUITPTR setCurcirc (CIRCUITPTR cirlocal);

// in readNonFlat.C
NonFlatPtr readNonFlat (STRING infile);

// in cleanUp.C
void cleanUp(CIRCUITPTR sdfcircuit, NonFlatPtr firstNonFlat);
int checkInstName (STRING name, NonFlatPtr firstNon);
int checkNameSf(STRING newName, NETPTR lastNet);
char *concatEnd(char* fatherName, char* childName);

// in printPath.C
void printPath (CIRCUITPTR sdfcircuit, char* outfile, char *outcircuit);

#endif
