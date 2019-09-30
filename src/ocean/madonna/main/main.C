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
#include <stdio.h>
#include <iostream>
using namespace std;
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>

#include <new>		// set_new_handler()

#include "src/ocean/madonna/phil/sea.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/libseadif/sdferrors.h"

#undef int
#define  PRECISION 10
#define INITIAL_TEMPERATURE 0.50 /* initially 50 % change to move randomly */
#define INITIAL_COOLING     0.99 /* after each random move, 1 % lower temp */

extern LINKAGE_TYPE
{
   void initsignals (void);
   void sdfexit (int);
   void sdfflatcir (CIRCUIT*);
   int issubstring (STRING, STRING);
   void dumpcore (void);
}

static char *bname (char *s);

#define NOTINITIALIZED -1

int requestedGridPoints[3] = {NOTINITIALIZED, NOTINITIALIZED, NOTINITIALIZED};
int expandableDirection    = NOTINITIALIZED;

CIRCUITPTR madonna (CIRCUIT *);
void markChilds (CIRCUIT*);

static void my_new_handler (void);
static void partWriteAllCir (int, CIRCUITPTR);
static void partWriteAllCir_1 (CIRCUITPTR);
static void partWriteAllCir_2 (int, CIRCUITPTR);
static int  checkThatCircuitLooksReasonable (CIRCUITPTR);

extern char *thedate;
extern char *madonna_version;

extern int dontCheckChildPorts;

#define MAXSTR 300

int highnumpart = 16;
int stopquotient = 100;

int         processid;
int         madonnamakeminiplaza = TRUE;
int         madonnamakepartition = TRUE;
int         permutateClustersAtExit = TRUE;
int         maxNumberOfClusterPermutations = 2;
int         madonnaAllowRandomMoves = 0;
int         acceptCandidateEvenIfNegativeGain = 0;
int         set_srand = 0;
int         phil_verbose = 0;
int         rand_points = 1;
int         doCompresion = 1;
double      extraplaza = 1.0;
int         macroMinSize = 100;
int         slicingLayout = 1;
int         doTranAna = -1;
int         makeChannels = 0;

char        *circuit_name = NULL,
            *function_name = NULL,
	    *library_name = NULL,
	    *layoutname = NULL;

STRING RouteAsciiFile = NULL;

double initial_temperature = INITIAL_TEMPERATURE;
double initial_cooling = INITIAL_COOLING;

ofstream RouteAsciiStreamBuf;

static void printstatisticsinstatusfield (LAYOUT*, int, int);

int main (int argc, char *argv[])
{
   extern int optind, opterr, optopt; /* used with getopt() */
   extern char *optarg;
   int i, readonlythisthing = 0, doflattenfirst = 0,
   doflattenonly = 0, checkNetlistFirst = TRUE;

   CIRCUITPTR  madonnapartitionedcircuit;
   CIRCUITPTR  thecircuit;
   char *argv0 = bname (argv[0]);
   opterr = 0; /* prevent getopt error message */

   setvbuf (stdout, NULL, _IOLBF, NULL);
   setvbuf (stderr, NULL, _IOLBF, NULL);

   printf ("\nMadonna version %s (compiled %s)\n\n", madonna_version, thedate);

   set_new_handler (&my_new_handler); // type message if operator new() fails

   /* Parse options */
   while ((i = getopt (argc, argv, "l:f:c:S:haAbrtT:viuj:s:d:q:o:mMpP:kLe:x:y:gwW:n:R:C")) >= 0)
   {
      switch (i)
      {
      case 'h':   /* print help */
	printf ("Usage: %s [..options..] [circuit_name]\n\n", argv0);
	printf ("options:\n"
		"-x <size>    Preferred width of the placement in gridpoints\n"
		"-y <size>    Preferred heigth of the placement in gridpoints\n"
		"-e <dir>     Where <dir> is either 'x' or 'y': direction to grow\n"
		"-c <name>    The circuit name\n"
		"-f <name>    The function name\n"
		"-l <name>    The library name\n"
		"-o <lname>   Layout name of output (default Madonna/Prince)\n"
		"-p           Do not exhaustively permutate the partitions at exit\n"
		"-P <num>     Permutate at most <num> times the partitions\n"
		"-S <size>    Set minimal \"huge\" cell size (default 100 b.c.)\n"
		"-M           Never treat cells as \"huge\"\n"
		"-q           Set stop quotient (default 100)\n"
		"-v           Debugging mode for detailed placement\n"
		"-a           Not random point selection in detailed placement\n"
		"-b           Do not do compaction after detailed placement\n"
		"-j <number>  Positive value to initialize random generator\n"
		"-g           Do not check that at most 50%% of instances are transistors\n"
		"-R <fname>   Set file name for ascii dump of global routes\n"
		"-L           Create flat layout instead of slicing structure\n"
		"-T <num>     If num=0 do not do transparency analysis, num=1 do it\n"
		"-C           Allocate place for channels before placement (default off)\n"
		"-w           Allow random moves\n"
		"-W <temp>    Start with <temp> percent random moves (def=%1.2f)\n"
		"-d <numparts>  Do not partition beyond numparts\n"
		"-r           Only read circuit, then exit\n"
		"-m           Do not make miniplaza\n"
		"-n <cool>    Multiply <temp> by <cool> after each random move (def=%1.2f)\n"
		"-s <size>    Set initial plaza magnification (default is 2.0)\n"
		"-t           Flatten before partitioning\n"
		"-u           Only flatten circuit, no partitioning\n"
		"-k           Do not lock the Seadif database while running madonna\n\n",
		(float)initial_temperature, (float)initial_cooling);
	 exit (0);
	 break;
      case 'c':   /* circuit name */
	 circuit_name = cs (optarg);
	 break;
      case 'f':   /* function name */
	 function_name = cs (optarg);
	 break;
      case 'j':   /* random gen seed*/
	 set_srand = atoi (optarg);
	 break;
      case 'a':   /* detailed placement random or sequential */
	 rand_points = 0;
	 break;
      case 'b':   /* do not do compaction after placement */
	 doCompresion = 0;
	 break;
      case 'l':   /* library name */
	 library_name = cs (optarg);
	 break;
      case 'k':
         {extern int sdfmakelockfiles; sdfmakelockfiles = 0;}
	 break;
      case 'r':
	 readonlythisthing = TRUE;
	 break;
      case 'R': // set file name for ascii dump of global routes
	 RouteAsciiFile = cs (optarg);
	 break;
      case 'm':
	 madonnamakeminiplaza = 0;
	 break;
      case 'p':
	 permutateClustersAtExit = 0;
	 break;
      case 'P':
	 maxNumberOfClusterPermutations = atoi (optarg);
	 break;
      case 'L':
	 slicingLayout = 0;
	 break;
      case 'T':
	 doTranAna = atoi (optarg);
	 break;
      case 'C':
         makeChannels = 1;
	 break;
      case 't':
	 doflattenfirst = TRUE;
	 break;
      case 'v':
	 phil_verbose = TRUE;
	 break;
      case 'u':
	 doflattenonly = TRUE;
	 break;
      case 's':
	 extraplaza = atof (optarg);
	 break;
      case 'S':
	 macroMinSize = atoi (optarg);
	 break;
      case 'M':
	 macroMinSize = MAXINT;
	 break;
      case 'q':
	 stopquotient = atoi (optarg);
	 break;
      case 'd':
	 highnumpart = atoi (optarg);
	 break;
      case 'o':
	 layoutname = cs (optarg);
	 break;
      case 'g':
	 checkNetlistFirst = 0;
	 break;
      case 'x':   /* requestedGridPoints[HOR]; */
         if ((requestedGridPoints[HOR] = atoi (optarg)) <= 0)
	 {
	    printf ("option '-x' requires a positive integer\n");
	    exit (1);
	 }
         break;
      case 'y':   /* requestedGridPoints[VER]; */
         if ((requestedGridPoints[VER] = atoi (optarg)) <= 0)
	 {
	    printf ("option '-y' requires a positive integer\n");
	    exit (1);
	 }
         break;
      case 'e':   /* expandable direction */
	 switch (optarg[0])
	 {
	 case 'x':
	 case 'X':
	    expandableDirection = HOR;
	    break;
	 case 'y':
	 case 'Y':
	    expandableDirection = VER;
	    break;
	 default:
	    cout << "illegal argument to -e option, must be 'x' or 'y'...\n";
	    exit (1);
	    break;
	 }
	 break;
      case 'w': /* allow random moves */
	 madonnaAllowRandomMoves = TRUE;
	 break;
      case 'W':	/* starting temperature, must be in range [0..1] */
	 madonnaAllowRandomMoves = TRUE;
	 initial_temperature = atof (optarg);
	 if (initial_temperature < 0.0 || initial_temperature > 1.0)
	 {
	    fprintf (stderr, "initial temperature must be in range [0..1]\n");
	    exit (1);
	 }
	 break;
      case 'n': /* cooling speed, must be in range [0..1] */
	 madonnaAllowRandomMoves = TRUE;
	 initial_cooling = atof (optarg);
	 if (initial_cooling < 0.0 || initial_cooling > 1.0)
	 {
	    fprintf (stderr, "initial cooling must be in range [0..1]\n");
	    exit (1);
	 }
	 break;
      case 'A':
	 acceptCandidateEvenIfNegativeGain = TRUE;
	 break;
      case '?':
	 fprintf (stderr, "%s: invalid option -- '%c'\n", argv0, optopt > 32 ? optopt : i);
      default:
	 fprintf (stderr, "%s: illegal argument, use -h to print help\n\n", argv0);
	 exit (1);
      }
   }

   initsignals ();

   if (expandableDirection == NOTINITIALIZED)
   {
      // grow vertically by default...:
      expandableDirection = VER;
   }

#if 0
   // only compact the layout if did not ask for a box of a certain size:
   if (requestedGridPoints[HOR] != NOTINITIALIZED &&
       requestedGridPoints[VER] != NOTINITIALIZED)
      doCompresion = 0;
#endif

   if (doTranAna == 1)
   {
     slicingLayout = 1;
     rand_points = 0;
     makeChannels = 0;
   }

   if (makeChannels)
     doTranAna = 0;

   if (!circuit_name) {
      if (optind == argc - 1)
	 circuit_name = cs (argv[optind]);
      else
      {
	 cout << "please specify a circuit name ...\n";
	 exit (1);
      }
   }

   dontCheckChildPorts = 1;

   if (!function_name) function_name = cs (circuit_name);

   if (!library_name)
   {
      if (!(library_name = sdfgetcwd ())) {
	 cout << "Cannot get the current working directory...\n";
	 exit (1);
      }
      library_name = bname (library_name);
      if (!*library_name) {
	 cout << "No name, root cannot be the project directory...\n";
	 exit (1);
      }
      library_name = cs (library_name);
   }

   if ((i = sdfopen ()) != SDF_NOERROR)
   {
      if (i == SDFERROR_FILELOCK)
      {
	 cerr <<
	    "ERROR: The seadif database is locked by another program.\n"
	    "       Try again later, because only one program at the time\n"
	    "       can access it. If you are sure that nobody else is\n"
	    "       working on the database, you can remove the lockfiles.\n";
      }
      else
	 cerr << "ERROR: cannot open seadif database.\n";
      sdfexit (i);
   }

   if (RouteAsciiFile)
      RouteAsciiStreamBuf.open (RouteAsciiFile, ios::out);
      if (! RouteAsciiStreamBuf)
      {
	 cerr << "WARNING: cannot open file \"" << RouteAsciiFile
	      << "\" to dump global routes on. (ignored)\n\n" << flush;
	 RouteAsciiFile = NULL;
      }

   fprintf (stdout, "reading circuit \"%s(%s(%s))\" ...",
	   circuit_name, function_name, library_name);
   fflush (stdout);

   if (!sdfreadallcir (SDFCIRNETLIST+SDFCIRSTAT, circuit_name, function_name, library_name))
   {
      fprintf (stdout, "\nERROR: apparently your cell \"%s(%s(%s))\" is out to lunch...\n",
	      circuit_name, function_name, library_name);
      sdfexit (1);
   }
   fprintf (stdout, "done\n");

   if (!thiscir->cirinst) {
      fprintf (stdout, "\nERROR: no instances found...\n");
      sdfexit (1);
   }

   if (readonlythisthing) sdfexit (0);

   thecircuit = thiscir;

   if (madonnamakepartition) markChilds (thiscir);

   if (doflattenfirst || doflattenonly) sdfflatcir (thecircuit);

   if (doflattenonly)
   {
      /* append "_f" to circuit name */
      char tmpstr[MAXSTR+1];
      strncpy (tmpstr, thecircuit->name, MAXSTR); strncat (tmpstr, "_f", MAXSTR);
      fs (thecircuit->name); thecircuit->name = cs (tmpstr);
      sdfwritecir (SDFCIRALL, thecircuit);
      sdfclose ();
      sdfexit (0);
   }

   if (checkNetlistFirst)
      if (!checkThatCircuitLooksReasonable (thecircuit))
      {
	 fprintf (stdout,
		 "ERROR: your netlist contains too much transistors and/or\n"
		 "       capacitors to my taste (> 50%%). I assume it is not\n"
		 "       your intention to place this circuit. Use -g option\n"
		 "       to enforce placement of this strange circuit...\n");
	 sdfexit (SDFERROR_MADONNA_FAILED);
      }

   time_t     totaltime1, totaltime2;
   int        seconds;
   long       clock_ticks_per_second;
   struct tms tmsbuf;

   times (&tmsbuf);
   totaltime1 = tmsbuf.tms_utime+tmsbuf.tms_stime+tmsbuf.tms_cutime+tmsbuf.tms_cstime;

   fprintf (stdout, "\nKissing madonna hello...\n");
   madonnapartitionedcircuit = madonna (thecircuit);
   fprintf (stdout, "Kissing madonna good bye...\n");

   times (&tmsbuf);
   totaltime2 = tmsbuf.tms_utime + tmsbuf.tms_stime + tmsbuf.tms_cutime + tmsbuf.tms_cstime;

#ifdef CLK_TCK
   clock_ticks_per_second = CLK_TCK;
#else
   clock_ticks_per_second = sysconf (_SC_CLK_TCK);
#endif
   seconds = int ((PRECISION * (totaltime2 - totaltime1)) / clock_ticks_per_second);
   fprintf (stdout, "\nMadonna took %d.%d seconds of your cpu\n",
	   seconds / PRECISION, seconds % PRECISION);
   if (madonnamakeminiplaza)
      printstatisticsinstatusfield (thecircuit->layout, seconds / PRECISION, highnumpart);

   if (madonnamakepartition && !madonnamakeminiplaza && madonnapartitionedcircuit)
   {
      fprintf (stdout, "writing circuit \"%s(%s(%s))\"\n",
	      madonnapartitionedcircuit->name, madonnapartitionedcircuit->function->name,
	      madonnapartitionedcircuit->function->library->name);
      partWriteAllCir (SDFCIRALL, madonnapartitionedcircuit);
   }

   /*
      if (thecircuit->layout)
      {
	 if (layoutname)
	 {
	    fs (thecircuit->layout->name);
	    thecircuit->layout->name = cs (layoutname);
	 }
	 fprintf (stdout, "writing layout \"%s(%s(%s(%s)))\"\n",
		 thecircuit->layout->name, thecircuit->name, thecircuit->function->name,
		 thecircuit->function->library->name);
	 sdfwritealllay (SDFLAYALL, thecircuit->layout);
      }
   */

   sdfclose ();

   if (RouteAsciiFile) RouteAsciiStreamBuf.close ();

   exit (SDF_NOERROR);
   return (SDF_NOERROR);
}

//  Fills out a status field of all circuit instances with key word "mad_prim".
//
void markChilds (CIRCUIT *cPtr)
{
  char *keyName = (char*)",mad_prim";
  STATUS *stat;

  for (CIRINST *ciPtr = cPtr->cirinst; ciPtr; ciPtr = ciPtr->next)
  {
    CIRCUIT *cur = ciPtr->circuit;

    if (!cur) {
      fprintf (stderr, "\n ill formed circuit instance %s\n, quitting ...\n\n", ciPtr->name);
      sdfexit (1);
    }
    if (!cur->status)
    {
      NewStatus (stat);
      cur->status = stat;
      stat->timestamp = 0;
      stat->author  = cs ((char*)"Madonna");
      stat->program = cs ((char*)"");
    }
    if (!strstr (cur->status->program, keyName))
      // we have to append it
    {
      char buf[200];

      buf[0] = '\0';
      strncat (buf, cur->status->program, 200);
      strncat (buf, keyName, 200);
      fs (cur->status->program);
      cur->status->program = cs (buf);
    }
  }
}

// We need this set of 3 functions because we don't want writing circuits
// with "mad_prim" key in status field back to the database.
//

#define SDFWRITEALLMASK 0x8000    /* use bit 15 to indicate whether written or not */

static void partWriteAllCir (int what, CIRCUITPTR cir)
{
    partWriteAllCir_1 (cir);    /* initialize flag bits */
    partWriteAllCir_2 (what, cir); /* perform the write */
}

static void partWriteAllCir_1 (CIRCUITPTR cir)
{
    CIRINSTPTR ci;

    if (cir->status && issubstring (cir->status->program, (char*)"mad_prim"))
	cir->flag.l |= SDFWRITEALLMASK; /* do not write it to database */
    else {
	cir->flag.l &= ~SDFWRITEALLMASK; /* clear bit 'written' */
	for (ci = cir->cirinst; ci; ci = ci->next)
	    partWriteAllCir_1 (ci->circuit);
    }
}

static void partWriteAllCir_2 (int what, CIRCUITPTR cir)
{
    CIRINSTPTR ci;

    if (cir->flag.l & SDFWRITEALLMASK) return; /* mad_prim or already wrote this one */

    what &= SDFCIRALL;
    if (!sdfwritecir (what, cir)) err (7, (char*)"sdfwriteallcir_2: cannot write circuit");
    cir->flag.l |= SDFWRITEALLMASK; /* mark as 'written' */
    for (ci = cir->cirinst; ci; ci = ci->next)
	partWriteAllCir_2 (what, ci->circuit);
}

#define MAXSTR 300

/* some hardware doesn't like big arrays on the stack: */
static char strng[MAXSTR+1];

static void printstatisticsinstatusfield (LAYOUT *layout, int cputime, int numparts)
{
   STATUSPTR status;

   if (!layout) return;

   status = layout->status;
   if (!status) {
      NewStatus (status);
      layout->status = status;
   }

   sprintf (strng, "cputime=%ds,numparts=%d,pid=%d", cputime, numparts, processid);

   if (status->program) {
      /* already some status information, don't destroy */
      strncat (strng, "; ", MAXSTR);
      strncat (strng, status->program, MAXSTR);
      fs (status->program);
   }
   status->program = cs (strng);
}

int costmincut (int netdistr[], int numparts)
{
   int cost = 0;

   if (numparts != 2) {
      fprintf (stderr, "costmincut: cannot compute cost for %d partitions!\n", numparts);
      dumpcore ();
   }
   if (netdistr[0] > 0 && netdistr[1] > 0) cost = 1;
   return (cost);
}

void printpartstat (TOTALPPTR total)
{
   fprintf (stdout, "\n(genpart (%s(%s(%s)))", total->topcell->name,
      total->topcell->function->name, total->topcell->function->library->name);
   fprintf (stdout, "\n    (numparts %d) (strtnetcost %d) (bestnetcost %d)",
      total->numparts, total->strtnetcost, total->bestnetcost);
   fprintf (stdout, "\n    (nmoves %d) (area %d)", total->nmoves, total->area);
   fprintf (stdout, ")\n\n");
}

static char *bname (char *s)
{
   char *p = strrchr (s, '/');
   if (p) return p+1;
   return s;
}

// Currently, this just checks that less than 50 % of the circuit instances are
// transistors. This turned out to be necessary because a lot of people
// unintendedly started placing extracted netlists...
static int checkThatCircuitLooksReasonable (CIRCUITPTR thecircuit)
{
   int numInst = 0;
   int numTrans = 0;
   STRING nenhStr = cs ((char*)"nenh");
   STRING penhStr = cs ((char*)"penh");
   STRING capStr  = cs ((char*)"cap");
   STRING resStr  = cs ((char*)"res");

   for (CIRINSTPTR cinst = thecircuit->cirinst; cinst; cinst = cinst->next)
   {
      if (cinst->circuit->name == nenhStr || cinst->circuit->name == penhStr ||
	  cinst->circuit->name == capStr || cinst->circuit->name == resStr)
	 numTrans += 1;
      numInst += 1;
   }
   fs (nenhStr);
   fs (penhStr);
   fs (capStr);
   fs (resStr);
   if (double (numTrans)/numInst > 0.5) return 0; // too much transistors...
   return TRUE;
}

// This one is called if ::operator new() fails to allocate enough memory:
static void my_new_handler (void)
{
   cerr << "\n"
	<< "FATAL: I cannot allocate enough memory." << endl
	<< "       Ask your sysop to configure more swap space ..." << endl;
   sdfexit (1);
}
