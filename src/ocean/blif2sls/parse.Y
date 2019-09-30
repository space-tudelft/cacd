%{
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

#define  YYDEBUG 9

#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
using namespace std;
#include <ctype.h>
#include "src/ocean/blif2sls/Network.h"

extern Array prototypes;

extern ofstream os;
extern int latchOutExt;
extern int doWriteSta;

ofstream cmdS;

Network *theNetwork;
Network *aCall;
Network *prototype;
Array   *missing;
static Boolean LatchOrderFlag = true;
static Boolean in_exdc = false;	/* this to avoid taking .names statements
				   from external dc section */
static int numberOfLatchOuts = 0; /* used when latchOutExt == 1 */
extern int blif2slslineno;
extern int blif2slsdebug;

void blif2slserror (char*);
int  blif2slslex (void);
				  /* local functions */
void ts (void);
void ni (const char*);
void wf (const char*);
String& cn (String&);
void findMissing (Network& call, Network& pro, Array& mis);

%}

%union {
	int  itg;
	char str[200];
}

%token <str> STRING_TOKEN
%token <str> NUMBER
%token EOL_TOKEN
%token ASSIGN_TOKEN

%token MODEL_TOKEN
%token INPUTS_TOKEN
%token OUTPUTS_TOKEN
%token AREA_TOKEN
%token DELAY_TOKEN
%token WIRE_LOAD_SLOPE_TOKEN
%token WIRE_TOKEN
%token INPUT_ARRIVAL_TOKEN
%token DEFAULT_INPUT_ARRIVAL_TOKEN
%token OUTPUT_REQUIRED_TOKEN
%token DEFAULT_OUTPUT_REQUIRED_TOKEN
%token DEFAULT_MAX_INPUT_LOAD_TOKEN
%token INPUT_DRIVE_TOKEN
%token DEFAULT_INPUT_DRIVE_TOKEN
%token OUTPUT_LOAD_TOKEN
%token DEFAULT_OUTPUT_LOAD_TOKEN
%token GATE_TOKEN
%token MLATCH_TOKEN
%token CLOCK_TOKEN
%token NAMES_TOKEN
%token EXDC_TOKEN
%token SUBCKT_TOKEN
%token LATCH_TOKEN
%token LATCH_ORDER_TOKEN
%token CODE_TOKEN
%token CYCLE_TOKEN
%token CLOCK_EVENT_TOKEN
%token END_KISS_TOKEN
%token END_TOKEN

%start Model

%%



Model          : MODEL_TOKEN  BlifModelName InputsList OutputsList _Model END_TOKEN EOL_TOKEN
{
				  /* first we have to add some missing
				     terminals (see findMissing())   */
  Iterator iter(*missing);

  while(int(iter))
  {
    String& ss=(String&)(Object&)iter;
    ++iter;
    theNetwork->addTerm(ss);
  }

  if(latchOutExt)		  /* external connections to latch outputs */
  {
      ostringstream latchOutStr;
      latchOutStr << "LatchOut[1.." << numberOfLatchOuts << "]" << ends;

      theNetwork->addTerm(* new String((char*) latchOutStr.str().c_str()));
  }



  os << "\n" << *theNetwork << endl;	  /* dumping the network */
  if(doWriteSta)
  {
    cmdS << "\nprint State\n";		  /* finishing touch for states file */
    cmdS.close();
  }

				  /* bacause there can be some more models */
				  /* we have to erase that one..           */
  delete theNetwork;
  delete missing;
}
               ;

BlifModelName  : STRING_TOKEN  EOL_TOKEN
{
				  /* let's allocate a new network ... */

				  /* a small problem - quite often we
                                     get here name.kiss2 - le's better
				     get rid of this kiss2*/
  String name($1);

  char buf[200],
       *d=&buf[0];
  const char  *s=(const char*)name,
              *kiss_start=strstr(s,".kiss2");

  while(*s && s!=kiss_start) *d++=*s++;
  *d=0;

  String cnName(&buf[0]);
  cn(cnName);
  theNetwork= new Network (cnName );

  missing   = new Array (10,10);
  numberOfLatchOuts=0;

  char bufer[200],*cmdName;

  strcpy(bufer,cnName);
  cmdName=strcat(bufer,".sta");

  if(doWriteSta)
  {
    cmdS.open(cmdName);
    if(!cmdS)
    {
      cerr << "error opening output file \"" << cmdName << "\"." << endl;
      exit(1);
    }
  }
}
               ;

InputsList     :
               | InputsList Inputs
               ;

OutputsList    :
               | OutputsList Outputs
               ;

Inputs         : INPUTS_TOKEN InputTermList EOL_TOKEN
               ;

InputTermList  :
               | InputTermList InputTerm
               ;

InputTerm      : STRING_TOKEN
{
  theNetwork->addTerm(cn(* new String($1)));
	/* a list of external terminals of our network */
}
              ;

Outputs        : OUTPUTS_TOKEN OutputTermList EOL_TOKEN
               ;

OutputTermList :
               | OutputTermList OutputTerm
               ;

OutputTerm     : STRING_TOKEN
{
  theNetwork->addTerm(cn(* new String($1)));
	/* a list of external terminals of our network */
}
               ;

_Model         :
	       | _Model  Area
	       | _Model  Delay
	       | _Model  Wire_Load_Slope
	       | _Model  Wire
	       | _Model  Input_Arrival
	       | _Model  Default_Input_Arrival
	       | _Model  Output_Required
	       | _Model  Default_Output_Required
	       | _Model  Input_Drive
	       | _Model  Default_Input_Drive
	       | _Model  Default_Max_Input_Load
	       | _Model  Output_Load
	       | _Model  Default_Output_Load
	       | _Model  Gate
	       | _Model  Mlatch
	       | _Model  Clock
	       | _Model  Names
               | _Model  Exdc
               | _Model  Subckt
               | _Model  Latch
               | _Model  Code
               | _Model  Latch_order
               | _Model  Cycle
               | _Model  Clock_Event
               | _Model  End_Kiss
               | _Model  OtherGarbage
               ;

OtherGarbage   : List EOL_TOKEN
               ;

Area           : AREA_TOKEN NUMBER EOL_TOKEN
{
  ts();
  ni(".area");
}
               ;

Delay          : DELAY_TOKEN STRING_TOKEN STRING_TOKEN NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER EOL_TOKEN
{
  ni(".delay");
}
               ;

Wire_Load_Slope : WIRE_LOAD_SLOPE_TOKEN NUMBER EOL_TOKEN
{
  ni(".wire_load_slope");
}
               ;

Wire           : WIRE_TOKEN List EOL_TOKEN
{
  ni(".wire");
}
               ;

Input_Arrival  : INPUT_ARRIVAL_TOKEN STRING_TOKEN NUMBER NUMBER EOL_TOKEN
               | INPUT_ARRIVAL_TOKEN STRING_TOKEN NUMBER NUMBER NUMBER NUMBER EOL_TOKEN
{
  ni(".input_arrival");
}
               ;


Default_Input_Arrival     : DEFAULT_INPUT_ARRIVAL_TOKEN NUMBER NUMBER EOL_TOKEN
{
  ni(".default_input_arrival");
}
               ;

Output_Required           : OUTPUT_REQUIRED_TOKEN STRING_TOKEN NUMBER NUMBER EOL_TOKEN
                          | OUTPUT_REQUIRED_TOKEN STRING_TOKEN NUMBER NUMBER NUMBER NUMBER   EOL_TOKEN
{
  ni(".output_required");
}
               ;



Default_Output_Required   : DEFAULT_OUTPUT_REQUIRED_TOKEN NUMBER NUMBER EOL_TOKEN
{
  ni(".default_output_required");
}
               ;


Input_Drive               : INPUT_DRIVE_TOKEN STRING_TOKEN NUMBER NUMBER EOL_TOKEN
{
  ni(".input_drive");
}
               ;

Default_Max_Input_Load    : DEFAULT_MAX_INPUT_LOAD_TOKEN NUMBER EOL_TOKEN
{
  ni(".default_max_input_load");
}
               ;


Default_Input_Drive       : DEFAULT_INPUT_DRIVE_TOKEN NUMBER NUMBER EOL_TOKEN
{
  ni(".default_input_drive");
}
               ;

Output_Load               : OUTPUT_LOAD_TOKEN STRING_TOKEN NUMBER EOL_TOKEN
{
  ni(".output_load");
}
               ;

Default_Output_Load       : DEFAULT_OUTPUT_LOAD_TOKEN NUMBER EOL_TOKEN
{
  ni(".default_output_load");
}
               ;

Clock          : CLOCK_TOKEN List EOL_TOKEN
{
  ni(".clock");
}
               ;
Code           : CODE_TOKEN STRING_TOKEN STRING_TOKEN EOL_TOKEN
{
  os << "/*     Code Assignment: " << $2 << "  " << $3 << "            */\n";
  if(doWriteSta)
  {
    cmdS << " \\\n           ";

    for (char* ptr = $3; *ptr; ptr++)
      if( *ptr == '1')
	cmdS << " h";
      else
	cmdS << " l";

    cmdS << " : " << $2 ;
  }
}
               ;

Latch_order    : LATCH_ORDER_TOKEN LatchNames EOL_TOKEN
{
  os << "\n";
  if(doWriteSta)
  {
    if(latchOutExt)
      cmdS << " LatchOut[1.." << numberOfLatchOuts << "] ";
    cmdS << " : State ";
  }
}
               ;


LatchNames      :
                | LatchNames STRING_TOKEN
{
  if (LatchOrderFlag)
  {
    os << "\n/*     Latch order:                         "
      "                 */\n";
    if(doWriteSta)
      cmdS << "define ";
  }
  LatchOrderFlag=false;
  os << "/*                      ";
  os.width(15);
  os << $2 << " ,                    */\n";
  if(doWriteSta)
  {
    if (!latchOutExt) cmdS << " " << $2 ;
    else
    {
      numberOfLatchOuts++;	/* we only count them... */
				/* and add a net between them and external terminal */
      ostringstream los;
      los << "LatchOut["<< numberOfLatchOuts << "]" << ends;
      String s1($2),
      s2((char*) los.str().c_str());
      Equivalence& eq = * new Equivalence(cn(s1),s2);

      theNetwork->addEquiv(eq);
    }
  }
}
                ;

Exdc           : EXDC_TOKEN EOL_TOKEN Exdc_InputsList Exdc_OutputsList
{
  wf(".exdc");
}
               ;

Exdc_InputsList     :
		| Exdc_InputsList Exdc_Inputs
		{
		    in_exdc=true;
		}
		;

Exdc_OutputsList    :
               | Exdc_OutputsList Exdc_Outputs
               ;


Exdc_Inputs         : INPUTS_TOKEN Exdc_InputTermList EOL_TOKEN
               ;

Exdc_InputTermList  :
               | Exdc_InputTermList Exdc_InputTerm
               ;

Exdc_InputTerm      : STRING_TOKEN
{
  ;
}
              ;

Exdc_Outputs        : OUTPUTS_TOKEN Exdc_OutputTermList EOL_TOKEN
               ;


Exdc_OutputTermList :
               | Exdc_OutputTermList Exdc_OutputTerm
               ;


Exdc_OutputTerm     : STRING_TOKEN
{
;
}
               ;



Names          : NAMES_TOKEN _Names EOL_TOKEN
               ;


_Names         :
               | STRING_TOKEN
{
 ;
}
               | STRING_TOKEN STRING_TOKEN
{
  if(!in_exdc)
  {
    String s1($1),
         s2($2);
    Equivalence& eq= * new Equivalence(cn(s1),cn(s2));

    theNetwork->addEquiv(eq);
  }
}
               | STRING_TOKEN STRING_TOKEN List
{
  ;
}
               ;

Latch          : LATCH_TOKEN List EOL_TOKEN
{
  wf(".latch");
}
               ;



Cycle          : CYCLE_TOKEN NUMBER EOL_TOKEN
{
  ni(".cycle");
}
               ;

Clock_Event    : CLOCK_EVENT_TOKEN List EOL_TOKEN
{
  ni(".clock_event");
}
               ;

End_Kiss       : END_KISS_TOKEN EOL_TOKEN
{
  ni(".end_kiss");
}
               ;


Subckt         : SUBCKT_TOKEN GateName FormalActualList EOL_TOKEN
{
  findMissing(*aCall,*prototype,*missing);
  theNetwork->add(*aCall);
}
               ;

Gate           : GATE_TOKEN GateName FormalActualList EOL_TOKEN
{
  findMissing(*aCall,*prototype,*missing);
  theNetwork->add(*aCall);
}
               ;

Mlatch         : MLATCH_TOKEN GateName FormalActualList STRING_TOKEN STRING_TOKEN EOL_TOKEN
{
  findMissing(*aCall,*prototype,*missing);
  theNetwork->add(*aCall);
}
               ;


GateName       : STRING_TOKEN
{

  String  cName($1);
				  /* bacause to determine the positions */
				  /* of terminals we are going to use   */
				  /* a prototypes, let's better find    */
				  /* the right one...                   */
  Iterator iter(prototypes);
  Boolean found = false;
  while(int(iter))
  {
    prototype = &(Network&)(Object&)iter;
    ++iter;

    if(prototype->getName() == cName)
    {
				  /* that's the right one */
      found=true;
      break;
    }
  }
  if(!found)
  {
    cerr << "Could not find a prototype of the network \"" <<
	    cName << "\" , quiting ..." << endl;

    if (strcmp(cName,"zero")==0 || strcmp(cName,"one")==0 )
      cerr << "\nTip (when used in kissis): check if maybe in the\n"
	   << "input file you specified one of the outputs to be \n"
	   << "always 0 or 1. If this is the case correct it by\n"
	   << "removing this output or by changing one of the\n"
	   << "transitions."
	   << endl;

    exit(1);
  }

				  /* new call to a network */
  aCall = new Network(cName,prototype);
}
               ;

FormalActualList :
                 | FormalActualList Pair
                 ;

Pair           : STRING_TOKEN ASSIGN_TOKEN STRING_TOKEN
{
				  /* first we have to find out which */
				  /* of the terminals it is          */

  int num=prototype->getTermNum(String($1));

  if(num < 0)			  /* error */
  {
    cerr << "Could not find a formal terminal \""<< cn(*new String($1)) <<
            "\" in the prototype of the network \"" <<
	    prototype->getName() << "\" , quiting ..." << endl;

    exit(1);
  }
  aCall->addTerm(cn((*new String($3))),num);
}
               ;

List            :		  /* unused arguments skipper */
                | List STRING_TOKEN
                ;

%%

    // Note: flex forgets to rename yyinput; therefore, do it ourselves.
#define blif2slsinput yyinput

#include "lex.h"
				  /* WARNING ! this may be not portable -   */
				  /* some systems may require const char*   */
void blif2slserror (char *s)
{
  cout << flush;
  cerr << "ERROR (Blif format error): " << s << "  Try line " << blif2slslineno << endl;
}

#ifndef blif2slswrap
int blif2slswrap() {return 1;}
#endif

void ts() // tests output stream for errors...
{
  if (!os) {
    cerr << "Error writing output file ! " << endl;
    exit(1);
  }
}

void ni (const char* tk)
// write the message that this feature is not implemented in sls
{
  if (blif2slsdebug)
    cerr << "Warning: \"" << tk <<
          "\" doesn't make any sense in sls - skipping"  << endl;
}

void wf (const char* tk)
// This one is called for blif rule which are illegal in this context.
{
  if (blif2slsdebug)
    cerr << "Warning: rule \"" << tk << "\" cannot be converted into sls.\n "
       << "Probably you've got the wrong file. (see man write_blif)." << endl;
}

String& cn (String& nm)
// Corrects the name in such a way that it doesn't contain any illegal in sls symbols.
{
  const char *c, *end;
  char *buf, *p;
  int len = nm.len();

  c = nm;
  end = c + len;
  p = buf = new char[len+1];

  if (!isalpha (*c)) *p++ = 'n';

  while (*c && c < end)
  {
    if (*c == '[' || *c == ']' || *c == '/' || *c == '.')
      *p++ = '_';
    else
      *p++ = *c;
    c++;
  }
  *p = '\0';

  /* now in buf we have our corrected string starting from 'n' letter */
  nm = String(buf);
  delete buf;
  return nm;
}

void findMissing (Network& call, Network& pro, Array& mis)
//
// Some of the terminals of the network may be missing. Reasons
// for this can be:
// * vdd,vss oraz clocks are not called in blif (blif doesn't care about
//   them - they have to be however connected with the outer world in sls.
//   Therefore we collect them in the Array missing, and add them at the
//   end of the definition of the network's terminals.
// * other ie. Q inverted in D latch (we really don't use - it's
//   unconnected) such terminals we simply skip. In sls version you'll
//   find an ", ," instead of terminal name
{
  for(int i=0;i<pro.numberOfArg();i++)
  {
    String& realTerm=call.getArg(i);
    if(realTerm == NOTHING)
    {
      String& formalTerm=pro.getArg(i);

      if (call.isSpecial(formalTerm))
	if(!mis.includes(formalTerm))
	{
	  mis.add(formalTerm);
	}
    }
  }
}
