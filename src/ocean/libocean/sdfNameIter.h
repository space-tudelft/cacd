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

#ifndef __SDFNAMEITER_H
#define __SDFNAMEITER_H

// This defines an iterator that must be initialized with extended regular
// expressions (refer to the manual regexp(5)) as the names of the library, the
// function, the circuit and the layout. Following this initialization the
// operator () of the iterator returns the names of the seadif objects that
// match the regular expressions.
//
// Programming example #1:
// Print the names of all functions in the library "primitives".
// ----------------------------------------------------------------------
// sdfNameIterator seadifName("primitives", ".*");
// while (seadifName())
// {
//    cout << seadifName.sdfName() << "\n";
// }
// ----------------------------------------------------------------------
//
// Programming example #2:
// Remove all layouts whos names start with "Tmp".
// ----------------------------------------------------------------------
// sdfNameIterator theLayout(".*", ".*", ".*", "Tmp.*");
// if (!theLayout.more())
//    cerr << "No " << theLayout.nameTypeString()
//         << " with that name exists ...\n" << flush;
// while (theLayout())
// {
//     cout << "removing " << theLayout.sdfName() << " ... " << flush;
//     int result = theLayout.sdfremove();
//     if (result)
//        cout << "done\n";
//     else
//        cout << "FAILED\n";
// }
// ----------------------------------------------------------------------

#include "src/ocean/libseadif/sealib.h"
#include <sys/types.h>
#include <iostream>
using namespace std;
#include "src/ocean/libocean/regex.h"

enum sdfNameType
{
   SeadifNoName = 0,
   SeadifLayoutName,
   SeadifCircuitName,
   SeadifFunctionName,
   SeadifLibraryName
};

class sdfNameIterator
{
private:
   regex_t rc_lname, rc_cname, rc_fname, rc_bname;
   STRING  ere_lname, ere_cname, ere_fname, ere_bname;
   STRING  _lname, _cname, _fname, _bname;
   sdfNameType theNameType;
   int         doNotIterateAnymore;
   int         alreadyCopiedNames;
   sdfNameType foundMatchWhenLookedAhead; // support for more() member
   int         alreadyLookedAhead;        // support for more() member
   LIBTABPTR bptr; FUNTABPTR fptr; CIRTABPTR cptr; LAYTABPTR lptr;
   int lastActionWasRead;
   LIBRARYPTR  _thislib;
   FUNCTIONPTR _thisfun;
   CIRCUITPTR  _thiscir;
   LAYOUTPTR   _thislay;
   int libRemoved() {return (bptr->info.state & SDFREMOVED) == SDFREMOVED;}
   int funRemoved() {return (fptr->info.state & SDFREMOVED) == SDFREMOVED;}
   int cirRemoved() {return (cptr->info.state & SDFREMOVED) == SDFREMOVED;}
   int layRemoved() {return (lptr->info.state & SDFREMOVED) == SDFREMOVED;}
   int findNextLibrary();
   int findNextFunction();
   int findNextCircuit();
   int findNextLayout();
   void copyNames();
   char *insertMetas(const char *);
   void error(char *);
public:
   sdfNameIterator(const char *ere_bname = NULL, const char *ere_fname = NULL,
		   const char *ere_cname = NULL, const char *ere_lname = NULL,
		   const int wholeWords = TRUE);
   ~sdfNameIterator();
   void initialize(const char *ere_bname = NULL, const char *ere_fname = NULL,
		   const char *ere_cname = NULL, const char *ere_lname = NULL,
		   const int wholeWords = TRUE);
   sdfNameType operator()();	// iterate next seadif name
   int more();		// is there any more to come?
   STRING lname() {return _lname;}
   STRING cname() {return _cname;}
   STRING fname() {return _fname;}
   STRING bname() {return _bname;}
   sdfNameType nameType() {return theNameType;}
   int sdfread(int what, int exitOnError = -1); // read current seadif object
   int sdfreadall(int what, int exitOnError = -1); // read recursively
   int sdfwrite(int what, int exitOnError = -1); // write current seadif object
   int sdfremove(int exitOnError = -1); // remove the current object
   char *nameTypeString(int plural = 0); // e.g. "layout" or "layouts"
   char *sdfName(int typeString = 0); // full name of current seadif object
   // these four are valid after sdfread() and MUST be valid before sdfwrite():
   LIBRARYPTR  thislib() {return _thislib;} // current seadif library
   FUNCTIONPTR thisfun() {return _thisfun;} // current seadif function
   CIRCUITPTR  thiscir() {return _thiscir;} // current seadif circuit
   LAYOUTPTR   thislay() {return _thislay;} // current seadif layout
   void sdfErrMsg(ostream& strm = cerr, int exitValue = -1); // print msg if fail
};

// just a function to convert from sdfNameType to a string:
char *sdfNameTypeString(sdfNameType theNameType, int plural = 0);

#endif
