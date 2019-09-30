/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
 *	Ireneusz Karkowski
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
/*
 * blif2sls - converter from blif to sls
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;
#include <time.h>
#include "src/ocean/blif2sls/Network.h"

const char *blif2sls_version = "1.17";

Array prototypes(10,10);

ofstream os;

extern int  blif2slsparse();
extern int  blif2slsdebug;
extern int  blif2slslineno;

int  doWriteSta = 0;	// If this flas is 1 sta file for simeye will be generated.
int  latchOutExt = 0;	// This flag controls if the outputs of all
			// latches should be connected to the external
			// terminals (can be used in debug mode
			// together with simeye state displaying facility)

int main(int argc,char *argv[])
{
  extern int  optind;
  extern char *optarg;

  int i;

  char *blif_file = NULL;
  char *sls_file = NULL;
  char *special_file = NULL;
  char *inc_file = (char*)"oplib.ext";
  char  buf1[200], buf2[200];

  blif2slsdebug = 0;

  cerr << "blif2sls version " << blif2sls_version << ".\n" << endl;

    // #################### Parse options #######################

  while ((i = getopt (argc,argv,"haxvb:s:i:c:")) != EOF)
  {
    switch (i)
    {
    case 'h':   /* print help */
      cout << "This program converts network description in blif\n"
	   << "(Berkeley Logic Interchange Format) to sls\n"
	   << "(Switch Level Simulator) format.\n\n";

      cout << "usage: " << argv[0] <<
	      " [..options..] [network name]\n\n";

      cout << "options:\n"
           << "-b <filename> - take input from file\n"
           << "-s <filename> - send output to file\n"
           << "-i <filename> - include this file in the beginning of sls file\n"
	   << "                (default: oplib.ext)\n"
	   << "-h this info\n"
	   << "-v verbose parser mode\n"
	   << "-a generate sta file for simeye\n"
	   << "-c <filename> read the names of special terminals from file\n"
	   << "-x generate external terminals connected to all latches'\n"
	   << "   outputs\n"
	   << endl;
      exit(0);
      break;
    case 'b':   /* blif name */
      blif_file = optarg;
      break;
    case 's':   /* sls name */
      sls_file = optarg;
      break;
    case 'i':   /* include file name */
      inc_file = optarg;
      break;
    case 'c':   /* specials' file name */
      special_file = optarg;
      break;
    case 'v':   /* verbose */
      blif2slsdebug = 1;
      break;
    case 'x':   /* latchOutExt */
      latchOutExt = 1;
      break;
    case 'a':   /* doWriteSta */
      doWriteSta = 1;
      break;
    case '?':
      fprintf(stderr,"\nIllegal argument.\n\n");
      exit(1);
      break;
    default:
      break;
    }
  }

  if (!blif_file)
    if (optind == argc - 1)
    {
      strcpy(buf1, argv[optind]);
      strcpy(buf2, buf1);
      blif_file = strcat(buf1, ".blif");
      sls_file  = strcat(buf2, ".sls");
    }

                                // This is only a dummy which we use to setup
				// the specials list....

  Network dummy(String("dummy"));
  dummy.readSpecials(special_file);

				// because yacc parser uses stdio anyway
				// so let\'s better use old stuff
  if (blif_file)
    if (!freopen(blif_file, "r", stdin))
    {
      cerr << "error opening input file \"" << blif_file << "\"." << endl;
      exit(1);
    }

				// but here let\'s use the streams
  if (!sls_file)
  {
    std::cerr << "Please specify a filename. Output to stdout currently not supported." << std::endl;
  }
  else
  {
    os.open(sls_file);
    if (!os)
    {
      cerr << "error opening output file \"" << sls_file << "\"." << endl;
      exit(1);
    }
  }
  time_t currentTime;
  time(&currentTime);
  os << "/*  This file was automatically created by blif2sls.\n\n    " << ctime(&currentTime) << "*/\n\n";

				// now find out if there\'s and include file
  ifstream incS(inc_file);

  if(!incS)
  {
    cerr << "Error: No file with networks' prototypes " << inc_file << " found ..." << endl;
    exit(1);
  }
  else
  {
				// first let\'s copy all the junk to
                                // the output file

     os << "#include<" << inc_file << ">\n";
				// function for simple rewinding of the file
    while(incS)
    {
      String ntg("");
      Network *n = new Network(ntg);

      if(n->scanPrototype(incS))
      {
	cerr << "Error reading networks' prototypes - quitting..." << endl;
	exit(1);
      }
      prototypes.add(*n);
    }
  }

  blif2slslineno = 1;
  if(blif2slsparse())
    cerr << "error during conversion ... Bye. " << endl;

  return 0;
}
