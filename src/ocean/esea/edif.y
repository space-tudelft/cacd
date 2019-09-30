%{
/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Paul Stravers
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
 * Edif 2.0.0 parser (level 0), based on the parser by <roger@mips.com>
 */

#include "src/ocean/esea/cedif.h"
#include "src/ocean/libseadif/sea_decl.h"

/*
 * global variables that are local to the parser:
 */
static LIBRARYPTR     current_library;
static FUNCTIONPTR    current_function;
static CIRCUITPTR     current_circuit;
static STATUSPTR      current_status;
static seadifViewType current_viewtype;
static STRING         current_viewtype_string = NULL;
static INSTANCE_TPTR  current_instance;
static NETPTR         current_net;
static CIRPORTREFPTR  current_joined;
static CIRPORTREFPTR  current_cirportref;

%}

%union {
  char		    *str;
  STATUSPTR	    status;
  int		    integer;
  time_t	    time;
  LIBRARYPTR	    library;
  FUNCTIONPTR	    function;
  CIRCUITPTR	    circuit;
  seadifViewType    viewtype;
  INSTANCE_TPTR     instance;
  CIRINSTPTR        cirinst;
  NETPTR            net;
  CIRPORTREFPTR     cpref;
}

%token <integer> INT
%token <str>     STR

%token		 ANGLE
%token		 BEHAVIOR
%token		 CALCULATED
%token		 CAPACITANCE
%token		 CENTERCENTER
%token		 CENTERLEFT
%token		 CENTERRIGHT
%token		 CHARGE
%token		 CONDUCTANCE
%token		 CURRENT
%token		 DISTANCE
%token		 DOCUMENT
%token		 ENERGY
%token		 EXTEND
%token		 FLUX
%token		 FREQUENCY
%token		 GENERIC
%token		 GRAPHIC
%token		 INDUCTANCE
%token		 INOUT
%token		 INPUT
%token		 LOGICMODEL
%token		 LOWERCENTER
%token		 LOWERLEFT
%token		 LOWERRIGHT
%token		 MASKLAYOUT
%token		 MASS
%token		 MEASURED
%token		 MX
%token		 MXR90
%token		 MY
%token		 MYR90
%token		 NETLIST
%token		 OUTPUT
%token		 PCBLAYOUT
%token		 POWER
%token		 R0
%token		 R180
%token		 R270
%token		 R90
%token		 REQUIRED
%token		 RESISTANCE
%token		 RIPPER
%token		 ROUND
%token		 SCHEMATIC
%token		 STRANGER
%token		 SYMBOLIC
%token		 TEMPERATURE
%token		 TIE
%token		 TIME
%token		 TRUNCATE
%token		 UPPERCENTER
%token		 UPPERLEFT
%token		 UPPERRIGHT
%token		 VOLTAGE

%token		 ACLOAD
%token		 AFTER
%token		 ANNOTATE
%token		 APPLY
%token		 ARC
%token		 ARRAY
%token		 ARRAYMACRO
%token		 ARRAYRELATEDINFO
%token		 ARRAYSITE
%token		 ATLEAST
%token		 ATMOST
%token		 AUTHOR
%token		 BASEARRAY
%token		 BECOMES
%token		 BETWEEN
%token		 BOOLEAN
%token		 BOOLEANDISPLAY
%token		 BOOLEANMAP
%token		 BORDERPATTERN
%token		 BORDERWIDTH
%token		 BOUNDINGBOX
%token		 CELL
%token		 CELLREF
%token		 CELLTYPE
%token		 CHANGE
%token		 CIRCLE
%token		 COLOR
%token		 COMMENT
%token		 COMMENTGRAPHICS
%token		 COMPOUND
%token		 CONNECTLOCATION
%token		 CONTENTS
%token		 CORNERTYPE
%token		 CRITICALITY
%token		 CURRENTMAP
%token		 CURVE
%token		 CYCLE
%token		 DATAORIGIN
%token		 DCFANINLOAD
%token		 DCFANOUTLOAD
%token		 DCMAXFANIN
%token		 DCMAXFANOUT
%token		 DELAY
%token		 DELTA
%token		 DERIVATION
%token		 DESIGN
%token		 DESIGNATOR
%token		 DIFFERENCE
%token		 DIRECTION
%token		 DISPLAY
%token		 DOMINATES
%token		 DOT
%token		 DURATION
%token		 E_TOK
%token		 EDIF
%token		 EDIFLEVEL
%token		 EDIFVERSION
%token		 ENCLOSUREDISTANCE
%token		 ENDTYPE
%token		 ENTRY
%token		 EVENT
%token		 EXACTLY
%token		 EXTERNAL
%token		 FABRICATE
%token		 FALSE
%token		 FIGURE
%token		 FIGUREAREA
%token		 FIGUREGROUP
%token		 FIGUREGROUPOBJECT
%token		 FIGUREGROUPOVERRIDE
%token		 FIGUREGROUPREF
%token		 FIGUREPERIMETER
%token		 FIGUREWIDTH
%token		 FILLPATTERN
%token		 FOLLOW
%token		 FORBIDDENEVENT
%token		 GLOBALPORTREF
%token		 GREATERTHAN
%token		 GRIDMAP
%token		 IGNORE
%token		 INCLUDEFIGUREGROUP
%token		 INITIAL_TOK
%token		 INSTANCE
%token		 INSTANCEBACKANNOTATE
%token		 INSTANCEGROUP
%token		 INSTANCEMAP
%token		 INSTANCEREF
%token		 INTEGER
%token		 INTEGERDISPLAY
%token		 INTERFACE
%token		 INTERFIGUREGROUPSPACING
%token		 INTERSECTION
%token		 INTRAFIGUREGROUPSPACING
%token		 INVERSE
%token		 ISOLATED
%token		 JOINED
%token		 JUSTIFY
%token		 KEYWORDDISPLAY
%token		 KEYWORDLEVEL
%token		 KEYWORDMAP
%token		 LESSTHAN
%token		 LIBRARY_TOK
%token		 LIBRARYREF
%token		 LISTOFNETS
%token		 LISTOFPORTS
%token		 LOADDELAY
%token		 LOGICASSIGN
%token		 LOGICINPUT
%token		 LOGICLIST
%token		 LOGICMAPINPUT
%token		 LOGICMAPOUTPUT
%token		 LOGICONEOF
%token		 LOGICOUTPUT
%token		 LOGICPORT
%token		 LOGICREF
%token		 LOGICVALUE
%token		 LOGICWAVEFORM
%token		 MAINTAIN
%token		 MATCH
%token		 MEMBER
%token		 MINOMAX
%token		 MINOMAXDISPLAY
%token		 MNM
%token		 MULTIPLEVALUESET
%token		 MUSTJOIN
%token		 NAME
%token		 NET_TOK
%token		 NETBACKANNOTATE
%token		 NETBUNDLE
%token		 NETDELAY
%token		 NETGROUP
%token		 NETMAP
%token		 NETREF_TOK
%token		 NOCHANGE
%token		 NONPERMUTABLE
%token		 NOTALLOWED
%token		 NOTCHSPACING
%token		 NUMBER
%token		 NUMBERDEFINITION
%token		 NUMBERDISPLAY
%token		 OFFPAGECONNECTOR
%token		 OFFSETEVENT
%token		 OPENSHAPE
%token		 ORIENTATION
%token		 ORIGIN
%token		 OVERHANGDISTANCE
%token		 OVERLAPDISTANCE
%token		 OVERSIZE
%token		 OWNER
%token		 PAGE
%token		 PAGESIZE
%token		 PARAMETER
%token		 PARAMETERASSIGN
%token		 PARAMETERDISPLAY
%token		 PATH
%token		 PATHDELAY
%token		 PATHWIDTH
%token		 PERMUTABLE
%token		 PHYSICALDESIGNRULE
%token		 PLUG
%token		 POINT
%token		 POINTDISPLAY
%token		 POINTLIST
%token		 POLYGON
%token		 PORT
%token		 PORTBACKANNOTATE
%token		 PORTBUNDLE
%token		 PORTDELAY
%token		 PORTGROUP
%token		 PORTIMPLEMENTATION
%token		 PORTINSTANCE
%token		 PORTLIST
%token		 PORTLISTALIAS
%token		 PORTMAP
%token		 PORTREF
%token		 PROGRAM
%token		 PROPERTY
%token		 PROPERTYDISPLAY
%token		 PROTECTIONFRAME
%token		 PT
%token		 RANGEVECTOR
%token		 RECTANGLE
%token		 RECTANGLESIZE
%token		 RENAME
%token		 RESOLVES
%token		 SCALE
%token		 SCALEX
%token		 SCALEY
%token		 SECTION
%token		 SHAPE
%token		 SIMULATE
%token		 SIMULATIONINFO
%token		 SINGLEVALUESET
%token		 SITE
%token		 SOCKET
%token		 SOCKETSET
%token		 STATUS_TOK
%token		 STEADY
%token		 STRING_TOK
%token		 STRINGDISPLAY
%token		 STRONG
%token		 SYMBOL
%token		 SYMMETRY
%token		 TABLE
%token		 TABLEDEFAULT
%token		 TECHNOLOGY
%token		 TEXTHEIGHT
%token		 TIMEINTERVAL
%token		 TIMESTAMP
%token		 TIMING_TOK
%token		 TRANSFORM
%token		 TRANSITION
%token		 TRIGGER
%token		 TRUE_TOK
%token		 UNCONSTRAINED
%token		 UNDEFINED
%token		 UNION
%token		 UNIT
%token		 UNUSED
%token		 USERDATA
%token		 VERSION
%token		 VIEW
%token		 VIEWLIST
%token		 VIEWMAP
%token		 VIEWREF
%token		 VIEWTYPE
%token		 VISIBLE
%token		 VOLTAGEMAP
%token		 WAVEVALUE
%token		 WEAK
%token		 WEAKJOINED
%token		 WHEN
%token		 WRITTEN
%token		 LBR
%token		 RBR

%type <str>	  NameDef LibNameDef Ident Name _Name Rename __Rename
		  EdifFileName CellNameDef CellNameRef FigGrpNameDef
		  InstNameDef KeywordName LayerNameDef LogicNameDef NameRef
		  NetNameDef PortNameDef PropNameDef RuleNameDef ValueNameDef
		  ViewNameDef Str Author Program _PortRef InstanceRef
                  InstNameRef LibNameRef PortNameRef
%type <status>	  Status
%type <integer>	  Int
%type <time>	  TimeStamp
%type <library>	  Library External
%type <function>  Cell
%type <viewtype>  ViewType _ViewType View
%type <instance>  Instance
%type <cpref>     Joined
%type <net>       Net

%start  Edif

%%

Edif :          LBR EDIF EdifFileName EdifVersion EdifLevel KeywordMap
                {
		edif_source.filename = $3;
		edif_source.library = NULL;
		edif_source.status = NULL;
		}
		_Edif RBR
     ;

_Edif :
      |         _Edif Status   { edif_source.status = $2; }
      |         _Edif External
                {
		if (externalBehavesLikeLibrary)
		   {
		   LIBRARYPTR last_library = edif_source.library;
		   while (last_library && last_library->next)
		      last_library = last_library->next;
		   if (last_library)
		      last_library->next = $2;
		   else
		      edif_source.library = $2;
		   }
                }
      |         _Edif Library
                {
		LIBRARYPTR last_library = edif_source.library;
		while (last_library && last_library->next)
		   last_library = last_library->next;
		if (last_library)
		   last_library->next = $2;
		else
		   edif_source.library = $2;
                }
      |         _Edif Design
      |         _Edif Comment
      |         _Edif UserData
      ;

EdifFileName :  NameDef { $<str>$ = $1; }
             ;

EdifLevel :     LBR EDIFLEVEL Int RBR
          ;

EdifVersion :   LBR EDIFVERSION Int Int Int RBR
            ;

AcLoad :        LBR ACLOAD _AcLoad RBR
       ;

_AcLoad :       MiNoMaValue
        |       MiNoMaDisp
        ;

After :         LBR AFTER _After RBR
      ;

_After :        MiNoMaValue
       |        _After Follow
       |        _After Maintain
       |        _After LogicAssn
       |        _After Comment
       |        _After UserData
       ;

Annotate :      LBR ANNOTATE _Annotate RBR
         ;

_Annotate :     Str        {;}
          |     StrDisplay {;}
          ;

Apply :         LBR APPLY _Apply RBR
      ;

_Apply :        Cycle
       |        _Apply LogicIn
       |        _Apply LogicOut
       |        _Apply Comment
       |        _Apply UserData
       ;

Arc :           LBR ARC PointValue PointValue PointValue RBR
    ;

Array :         LBR ARRAY NameDef Int _Array RBR
      ;

_Array :
       |        Int {;}
       ;

ArrayMacro :    LBR ARRAYMACRO Plug RBR
           ;

ArrayRelInfo :  LBR ARRAYRELATEDINFO _ArrayRelInfo RBR
             ;

_ArrayRelInfo : BaseArray
              | ArraySite
              | ArrayMacro
              | _ArrayRelInfo Comment
              | _ArrayRelInfo UserData
              ;

ArraySite :     LBR ARRAYSITE Socket RBR
          ;

AtLeast :       LBR ATLEAST ScaledInt RBR
        ;

AtMost :        LBR ATMOST ScaledInt RBR
       ;

Author :        LBR AUTHOR Str RBR { $<str>$ = $3; }
       ;

BaseArray :     LBR BASEARRAY RBR
          ;

Becomes :       LBR BECOMES _Becomes RBR
        ;

_Becomes :      LogicNameRef
         |      LogicList
         |      LogicOneOf
         ;

Between :       LBR BETWEEN __Between _Between RBR
        ;

__Between :     AtLeast
          |     GreaterThan
          ;

_Between :      AtMost
         |      LessThan
         ;

Boolean :       LBR BOOLEAN _Boolean RBR
        ;

_Boolean :
         |      _Boolean BooleanValue
         |      _Boolean BooleanDisp
         |      _Boolean Boolean
         ;

BooleanDisp :   LBR BOOLEANDISPLAY _BooleanDisp RBR
            ;

_BooleanDisp :  BooleanValue
             |  _BooleanDisp Display
             ;

BooleanMap :    LBR BOOLEANMAP BooleanValue RBR
           ;

BooleanValue :  True
             |  False
             ;

BorderPat :     LBR BORDERPATTERN Int Int Boolean RBR
          ;

BorderWidth :   LBR BORDERWIDTH Int RBR
            ;

BoundBox :      LBR BOUNDINGBOX Rectangle RBR
         ;

Cell :          LBR CELL CellNameDef
                {
		NewFunction(current_function);
		current_function->name = $3;
		}
		_Cell RBR
                {
                $<function>$ = current_function;
                }
     ;

_Cell :         CellType
      |         _Cell Status   { current_function->status = $2; }
      |         _Cell ViewMap
      |         _Cell View
                {
		switch($2)
		   {
		case SeadifCircuitView:
		   current_circuit->name = cs(current_function->name);
		   current_circuit->function = current_function;
		   current_function->circuit = current_circuit;
		   break;
		default:
		   break;
		   }
		}
      |         _Cell Comment
      |         _Cell UserData
      |         _Cell Property
      ;

CellNameDef :   NameDef { $<str>$ = $1; }
            ;

CellRef :       LBR CELLREF CellNameRef
                {
		if (current_instance)
		   current_instance->cell_ref = $3;
		}
		_CellRef RBR
        ;

_CellRef :
         /* empty, that is: no library specified */
                {
		if (current_instance)
		   current_instance->library_ref = cs(current_library->name);
		}
         |      LibraryRef
         ;

CellNameRef :   NameRef { $<str>$ = $1; }
            ;

CellType :      LBR CELLTYPE _CellType RBR
         ;

_CellType :     TIE
          |     RIPPER
          |     GENERIC
          ;

Change :        LBR CHANGE __Change _Change RBR
       ;

__Change :      PortNameRef {;}
         |      PortRef     {;}
         |      PortList
         ;

_Change :
        |       Becomes
        |       Transition
        ;

Circle :        LBR CIRCLE PointValue PointValue _Circle RBR
       ;

_Circle :
        |       _Circle Property
        ;

Color :         LBR COLOR ScaledInt ScaledInt ScaledInt RBR
      ;

Comment :       LBR COMMENT _Comment RBR
        ;

_Comment :
         |      _Comment Str
         ;

CommGraph :     LBR COMMENTGRAPHICS _CommGraph RBR
          ;

_CommGraph :
           |    _CommGraph Annotate
           |    _CommGraph Figure
           |    _CommGraph Instance
           |    _CommGraph BoundBox
           |    _CommGraph Property
           |    _CommGraph Comment
           |    _CommGraph UserData
           ;

Compound :      LBR COMPOUND LogicNameRef RBR
         ;

Contents :      LBR CONTENTS _Contents RBR
         ;

_Contents :
          |     _Contents Instance
                {
		CIRINSTPTR cinst;
		switch (current_viewtype)
		   {
		case SeadifCircuitView:
		   NewCirinst(cinst);
		   cinst->name = ($2)->instance_name;
		   ($2)->instance_name = NULL;
		   cinst->circuit = (CIRCUITPTR) $2; /* __HACK__ */
		   cinst->curcirc = current_circuit;
		   cinst->next = current_circuit->cirinst;
		   current_circuit->cirinst = cinst;
		   break;
		default:
		   break;
		   }
		}
          |     _Contents OffPageConn
          |     _Contents Figure
          |     _Contents Section
          |     _Contents Net
                {
		switch (current_viewtype)
		   {
		case SeadifCircuitView:
		   ($2)->next = current_circuit->netlist;
		   current_circuit->netlist = $2;
		   break;
		default:
		   break;
		   }
		}
          |     _Contents NetBundle
          |     _Contents Page
          |     _Contents CommGraph
          |     _Contents PortImpl
          |     _Contents Timing
          |     _Contents Simulate
          |     _Contents When
          |     _Contents Follow
          |     _Contents LogicPort
          |     _Contents BoundBox
          |     _Contents Comment
          |     _Contents UserData
          ;

ConnectLoc :    LBR CONNECTLOCATION _ConnectLoc RBR
           ;

_ConnectLoc :
            |   Figure
            ;

CornerType :    LBR CORNERTYPE _CornerType RBR
           ;

_CornerType :   EXTEND
            |   ROUND
            |   TRUNCATE
            ;

Criticality :   LBR CRITICALITY _Criticality RBR
            ;

_Criticality :  Int {;}
             |  IntDisplay {;}
             ;

CurrentMap :    LBR CURRENTMAP MiNoMaValue RBR
           ;

Curve :         LBR CURVE _Curve RBR
      ;

_Curve :
       |        _Curve Arc
       |        _Curve PointValue
       ;

Cycle :         LBR CYCLE Int _Cycle RBR
      ;

_Cycle :
       |        Duration
       ;

DataOrigin :    LBR DATAORIGIN Str _DataOrigin RBR
           ;

_DataOrigin :
            |   Version
            ;

DcFanInLoad :   LBR DCFANINLOAD _DcFanInLoad RBR
            ;

_DcFanInLoad :  ScaledInt
             |  NumbDisplay
             ;

DcFanOutLoad :  LBR DCFANOUTLOAD _DcFanOutLoad RBR
             ;

_DcFanOutLoad : ScaledInt
              | NumbDisplay
              ;

DcMaxFanIn :    LBR DCMAXFANIN _DcMaxFanIn RBR
           ;

_DcMaxFanIn :   ScaledInt
            |   NumbDisplay
            ;

DcMaxFanOut :   LBR DCMAXFANOUT _DcMaxFanOut RBR
            ;

_DcMaxFanOut :  ScaledInt
             |  NumbDisplay
             ;

Delay :         LBR DELAY _Delay RBR
      ;

_Delay :        MiNoMaValue
       |        MiNoMaDisp
       ;

Delta :         LBR DELTA _Delta RBR
      ;

_Delta :
       |        _Delta PointValue
       ;

Derivation :    LBR DERIVATION _Derivation RBR
           ;

_Derivation :   CALCULATED
            |   MEASURED
            |   REQUIRED
            ;

Design :        LBR DESIGN DesignNameDef
                {
		current_instance = NULL; /* disable assign to current_instance*/
		}
                _Design RBR
       ;

_Design :       CellRef
        |       _Design Status
        |       _Design Comment
        |       _Design Property
        |       _Design UserData
        ;

Designator :    LBR DESIGNATOR _Designator RBR
           ;

_Designator :   Str        {;}
            |   StrDisplay {;}
            ;

DesignNameDef : NameDef { $<str>$ = $1; }
              ;

DesignRule :    LBR PHYSICALDESIGNRULE _DesignRule RBR
           ;

_DesignRule :
            |   _DesignRule FigureWidth
            |   _DesignRule FigureArea
            |   _DesignRule RectSize
            |   _DesignRule FigurePerim
            |   _DesignRule OverlapDist
            |   _DesignRule OverhngDist
            |   _DesignRule EncloseDist
            |   _DesignRule InterFigGrp
            |   _DesignRule IntraFigGrp
            |   _DesignRule NotchSpace
            |   _DesignRule NotAllowed
            |   _DesignRule FigGrp
            |   _DesignRule Comment
            |   _DesignRule UserData
            ;

Difference :    LBR DIFFERENCE _Difference RBR
           ;

_Difference :   FigGrpRef
            |   FigureOp
            |   _Difference FigGrpRef
            |   _Difference FigureOp
            ;

Direction :     LBR DIRECTION _Direction RBR
          ;

_Direction :    INOUT
                {
#ifdef SDF_PORT_DIRECTIONS
                   current_circuit->cirport->direction = SDF_PORT_INOUT;
#endif
                }
           |    INPUT
                {
#ifdef SDF_PORT_DIRECTIONS
                   current_circuit->cirport->direction = SDF_PORT_IN;
#endif
                }
           |    OUTPUT
                {
#ifdef SDF_PORT_DIRECTIONS
                   current_circuit->cirport->direction = SDF_PORT_OUT;
#endif
                }
           ;

Display :       LBR DISPLAY _Display _DisplayJust _DisplayOrien _DisplayOrg RBR
        ;

_Display :      FigGrpNameRef
         |      FigGrpOver
         ;

_DisplayJust :
             |  Justify
             ;

_DisplayOrien :
              | Orientation
              ;

_DisplayOrg :
            |   Origin
            ;

Dominates :     LBR DOMINATES _Dominates RBR
          ;

_Dominates :
           |    _Dominates LogicNameRef
           ;

Dot :           LBR DOT _Dot RBR
    ;

_Dot :          PointValue
     |          _Dot Property
     ;

Duration :      LBR DURATION ScaledInt RBR
         ;

EncloseDist :   LBR ENCLOSUREDISTANCE RuleNameDef FigGrpObj FigGrpObj _EncloseDist RBR
            ;

_EncloseDist :  Range
             |  SingleValSet
             |  _EncloseDist Comment
             |  _EncloseDist UserData
             ;

EndType :       LBR ENDTYPE _EndType RBR
        ;

_EndType :      EXTEND
         |      ROUND
         |      TRUNCATE
         ;

Entry :         LBR ENTRY ___Entry __Entry _Entry RBR
      ;

___Entry :      Match
         |      Change
         |      Steady
         ;

__Entry :       LogicRef
        |       PortRef
        |       NoChange
        |       Table
        ;

_Entry :
       |        Delay
       |        LoadDelay
       ;

Event :         LBR EVENT _Event RBR
      ;

_Event :        PortRef
       |        PortList
       |        PortGroup
       |        NetRef
       |        NetGroup
       |        _Event Transition
       |        _Event Becomes
       ;

Exactly :       LBR EXACTLY ScaledInt RBR
        ;

External :      LBR EXTERNAL LibNameDef EdifLevel
                {
                NewLibrary(current_library);
		current_library->name = $3; /* $3 already is canonic */
		/* current library goes at end of list: */
                }
                _External RBR
                {
		$<library>$ = current_library;
                }
         ;

_External :     Technology
          |     _External Status  { current_library->status = $2; }
          |     _External Cell
                {
		FUNCTIONPTR last_function = current_library->function;
		while (last_function && last_function->next)
		   last_function = last_function->next;
		if (last_function)
		   last_function->next = $2;
		else
		   current_library->function = $2;
		($2)->library = current_library;
		}
          |     _External Comment
          |     _External UserData
          ;

Fabricate :     LBR FABRICATE LayerNameDef FigGrpNameRef RBR
          ;

False :         LBR FALSE RBR
      ;

FigGrp :        LBR FIGUREGROUP _FigGrp RBR
       ;

_FigGrp :       FigGrpNameDef         {;}
        |       _FigGrp CornerType
        |       _FigGrp EndType
        |       _FigGrp PathWidth
        |       _FigGrp BorderWidth
        |       _FigGrp Color
        |       _FigGrp FillPattern
        |       _FigGrp BorderPat
        |       _FigGrp TextHeight
        |       _FigGrp Visible
        |       _FigGrp Comment
        |       _FigGrp Property
        |       _FigGrp UserData
        |       _FigGrp IncFigGrp
        ;

FigGrpNameDef : NameDef { $<str>$ = $1; }
              ;

FigGrpNameRef : NameRef { $<str>$ = $1; }
              ;

FigGrpObj :     LBR FIGUREGROUPOBJECT _FigGrpObj RBR
          ;

_FigGrpObj :    FigGrpNameRef
           |    FigGrpRef
           |    FigureOp
           ;

FigGrpOver :    LBR FIGUREGROUPOVERRIDE _FigGrpOver RBR
           ;

_FigGrpOver :   FigGrpNameRef
            |   _FigGrpOver CornerType
            |   _FigGrpOver EndType
            |   _FigGrpOver PathWidth
            |   _FigGrpOver BorderWidth
            |   _FigGrpOver Color
            |   _FigGrpOver FillPattern
            |   _FigGrpOver BorderPat
            |   _FigGrpOver TextHeight
            |   _FigGrpOver Visible
            |   _FigGrpOver Comment
            |   _FigGrpOver Property
            |   _FigGrpOver UserData
            ;

FigGrpRef :     LBR FIGUREGROUPREF FigGrpNameRef _FigGrpRef RBR
          ;

_FigGrpRef :
           |    LibraryRef
           ;

Figure :        LBR FIGURE _Figure RBR
       ;

_Figure :       FigGrpNameDef    {;}
        |       FigGrpOver       {;}
        |       _Figure Circle
        |       _Figure Dot
        |       _Figure OpenShape
        |       _Figure Path
        |       _Figure Polygon
        |       _Figure Rectangle
        |       _Figure Shape
        |       _Figure Comment
        |       _Figure UserData
        ;

FigureArea :    LBR FIGUREAREA RuleNameDef FigGrpObj _FigureArea RBR
           ;

_FigureArea :   Range
            |   SingleValSet
            |   _FigureArea Comment
            |   _FigureArea UserData
            ;

FigureOp :      Intersection
         |      Union
         |      Difference
         |      Inverse
         |      Oversize
         ;

FigurePerim :   LBR FIGUREPERIMETER RuleNameDef FigGrpObj _FigurePerim RBR
            ;

_FigurePerim :  Range
             |  SingleValSet
             |  _FigurePerim Comment
             |  _FigurePerim UserData
             ;

FigureWidth :   LBR FIGUREWIDTH RuleNameDef FigGrpObj _FigureWidth RBR
            ;

_FigureWidth :  Range
             |  SingleValSet
             |  _FigureWidth Comment
             |  _FigureWidth UserData
             ;

FillPattern :   LBR FILLPATTERN Int Int Boolean RBR
            ;

Follow :        LBR FOLLOW __Follow _Follow RBR
       ;

__Follow :      PortNameRef {;}
         |      PortRef     {;}
         ;

_Follow :       PortRef
        |       Table
        |       _Follow Delay
        |       _Follow LoadDelay
        ;

Forbidden :     LBR FORBIDDENEVENT _Forbidden RBR
          ;

_Forbidden :    TimeIntval
           |    _Forbidden Event
           ;

GlobPortRef :   LBR GLOBALPORTREF PortNameRef RBR
            ;

GreaterThan :   LBR GREATERTHAN ScaledInt RBR
            ;

GridMap :       LBR GRIDMAP ScaledInt ScaledInt RBR
        ;

Ignore :        LBR IGNORE RBR
       ;

IncFigGrp :     LBR INCLUDEFIGUREGROUP _IncFigGrp RBR
          ;

_IncFigGrp :    FigGrpRef
           |    FigureOp
           ;

Initial :       LBR INITIAL_TOK RBR
        ;

Instance :      LBR INSTANCE InstNameDef
                {
		NewInstance_t(current_instance);
		current_instance->instance_name = $3;
		break;
		}
                _Instance RBR
                {
		$<instance>$ = current_instance;
		}
         ;

_Instance :     ViewRef
          |     ViewList
          |     _Instance Transform
          |     _Instance ParamAssign
          |     _Instance PortInst
          |     _Instance Timing
          |     _Instance Designator
          |     _Instance Property
          |     _Instance Comment
          |     _Instance UserData
          ;

InstanceRef :   LBR INSTANCEREF InstNameRef _InstanceRef RBR
                {
		$<str>$ = $3;
		}
            ;

_InstanceRef :
             |  InstanceRef {;}
             |  ViewRef
             ;

InstBackAn :    LBR INSTANCEBACKANNOTATE _InstBackAn RBR
           ;

_InstBackAn :   InstanceRef {;}
            |   _InstBackAn Designator
            |   _InstBackAn Timing
            |   _InstBackAn Property
            |   _InstBackAn Comment
            ;

InstGroup :     LBR INSTANCEGROUP _InstGroup RBR
          ;

_InstGroup :
           |    _InstGroup InstanceRef {;}
           ;

InstMap :       LBR INSTANCEMAP _InstMap RBR
        ;

_InstMap :
         |      _InstMap InstanceRef {;}
         |      _InstMap InstGroup
         |      _InstMap Comment
         |      _InstMap UserData
         ;

InstNameDef :   NameDef { $<str>$ = $1; }
            |   Array   { $<str>$ = NULL; }
            ;

InstNameRef :   NameRef { $<str>$ = $1; }
            |   Member  { $<str>$ = NULL; }
            ;

IntDisplay :    LBR INTEGERDISPLAY _IntDisplay RBR
           ;

_IntDisplay :   Int {;}
            |   _IntDisplay Display
            ;

Integer :       LBR INTEGER _Integer RBR
        ;

_Integer :
         |      _Integer Int
         |      _Integer IntDisplay
         |      _Integer Integer
         ;

Interface :     LBR INTERFACE _Interface RBR
          ;

_Interface :
           |    _Interface Port
           |    _Interface PortBundle
           |    _Interface Symbol
           |    _Interface ProtectFrame
           |    _Interface ArrayRelInfo
           |    _Interface Parameter
           |    _Interface Joined
           |    _Interface MustJoin
           |    _Interface WeakJoined
           |    _Interface Permutable
           |    _Interface Timing
           |    _Interface Simulate
           |    _Interface Designator
           |    _Interface Property
           |    _Interface Comment
           |    _Interface UserData
           ;

InterFigGrp :   LBR INTERFIGUREGROUPSPACING RuleNameDef FigGrpObj FigGrpObj _InterFigGrp RBR
            ;

_InterFigGrp :  Range
             |  SingleValSet
             |  _InterFigGrp Comment
             |  _InterFigGrp UserData
             ;

Intersection :  LBR INTERSECTION _Intersection RBR
             ;

_Intersection : FigGrpRef
              | FigureOp
              | _Intersection FigGrpRef
              | _Intersection FigureOp
              ;

IntraFigGrp :   LBR INTRAFIGUREGROUPSPACING RuleNameDef FigGrpObj _IntraFigGrp RBR
            ;

_IntraFigGrp :  Range
             |  SingleValSet
             |  _IntraFigGrp Comment
             |  _IntraFigGrp UserData
             ;

Inverse :       LBR INVERSE _Inverse RBR
        ;

_Inverse :      FigGrpRef
         |      FigureOp
         ;

Isolated :      LBR ISOLATED RBR
         ;

Joined :        LBR JOINED
                {
		current_joined = NULL;
		}
                _Joined RBR
                {
		$<cpref>$ = current_joined;
		}
       ;

_Joined :
        |       _Joined PortRef
                {
		current_cirportref->next = current_joined;
		current_joined = current_cirportref;
		}
        |       _Joined PortList
        |       _Joined GlobPortRef
        ;

Justify :       LBR JUSTIFY _Justify RBR
        ;

_Justify :      CENTERCENTER
         |      CENTERLEFT
         |      CENTERRIGHT
         |      LOWERCENTER
         |      LOWERLEFT
         |      LOWERRIGHT
         |      UPPERCENTER
         |      UPPERLEFT
         |      UPPERRIGHT
         ;

KeywordDisp :   LBR KEYWORDDISPLAY _KeywordDisp RBR
            ;

_KeywordDisp :  KeywordName  {;}
             |  _KeywordDisp Display
             ;

KeywordLevel :  LBR KEYWORDLEVEL Int RBR
             ;

KeywordMap :    LBR KEYWORDMAP _KeywordMap RBR
           ;

_KeywordMap :   KeywordLevel
            |   _KeywordMap Comment
            ;

KeywordName :   Ident { $<str>$ = $1; }
            ;

LayerNameDef :  NameDef { $<str>$ = $1; }
             ;

LessThan :      LBR LESSTHAN ScaledInt RBR
         ;

LibNameDef :    NameDef { $<str>$ = $1; }
           ;

LibNameRef :    NameRef { $<str>$ = $1; }
           ;

Library :       LBR LIBRARY_TOK LibNameDef EdifLevel
                {
                NewLibrary(current_library);
		current_library->name = $3; /* $3 already is canonic */
		/* current library goes at end of list: */
                }
                _Library RBR
                {
		$<library>$ = current_library;
                }
        ;

_Library :      Technology
         |      _Library Status  { current_library->status = $2; }
         |      _Library Cell
                {
		FUNCTIONPTR last_function = current_library->function;
		while (last_function && last_function->next)
		   last_function = last_function->next;
		if (last_function)
		   last_function->next = $2;
		else
		   current_library->function = $2;
		($2)->library = current_library;
		}
         |      _Library Comment
         |      _Library UserData
         ;

LibraryRef :    LBR LIBRARYREF LibNameRef RBR
                {
		if (current_instance)
		   current_instance->library_ref = $3;
		}
           ;

ListOfNets :    LBR LISTOFNETS _ListOfNets RBR
           ;

_ListOfNets :
            |   _ListOfNets Net
            ;

ListOfPorts :   LBR LISTOFPORTS _ListOfPorts RBR
            ;

_ListOfPorts :
             |  _ListOfPorts Port
             |  _ListOfPorts PortBundle
             ;

LoadDelay :     LBR LOADDELAY _LoadDelay _LoadDelay RBR
          ;

_LoadDelay :    MiNoMaValue
           |    MiNoMaDisp
           ;

LogicAssn :     LBR LOGICASSIGN ___LogicAssn __LogicAssn _LogicAssn RBR
          ;

___LogicAssn :  PortNameRef {;}
             |  PortRef     {;}
             ;

__LogicAssn :   PortRef
            |   LogicRef
            |   Table
            ;

_LogicAssn :
           |    Delay
           |    LoadDelay
           ;

LogicIn :       LBR LOGICINPUT _LogicIn RBR
        ;

_LogicIn :      PortList
         |      PortRef             {;}
         |      PortNameRef         {;}
         |      _LogicIn LogicWave
         ;

LogicList :     LBR LOGICLIST _LogicList RBR
          ;

_LogicList :
           |    _LogicList LogicNameRef
           |    _LogicList LogicOneOf
           |    _LogicList Ignore
           ;

LogicMapIn :    LBR LOGICMAPINPUT _LogicMapIn RBR
           ;

_LogicMapIn :
            |   _LogicMapIn LogicNameRef
            ;

LogicMapOut :   LBR LOGICMAPOUTPUT _LogicMapOut RBR
            ;

_LogicMapOut :
             |  _LogicMapOut LogicNameRef
             ;

LogicNameDef :  NameDef { $<str>$ = $1; }
             ;

LogicNameRef :  NameRef { $<str>$ = $1; }
             ;

LogicOneOf :    LBR LOGICONEOF _LogicOneOf RBR
           ;

_LogicOneOf :
            |   _LogicOneOf LogicNameRef
            |   _LogicOneOf LogicList
            ;

LogicOut :      LBR LOGICOUTPUT _LogicOut RBR
         ;

_LogicOut :     PortList
          |     PortRef     {;}
          |     PortNameRef {;}
          |     _LogicOut LogicWave
          ;

LogicPort :     LBR LOGICPORT _LogicPort RBR
          ;

_LogicPort :    PortNameDef {;}
           |    _LogicPort Property
           |    _LogicPort Comment
           |    _LogicPort UserData
           ;

LogicRef :      LBR LOGICREF LogicNameRef _LogicRef RBR
         ;

_LogicRef :
          |     LibraryRef
          ;

LogicValue :    LBR LOGICVALUE _LogicValue RBR
           ;

_LogicValue :   LogicNameDef {;}
            |   _LogicValue VoltageMap
            |   _LogicValue CurrentMap
            |   _LogicValue BooleanMap
            |   _LogicValue Compound
            |   _LogicValue Weak
            |   _LogicValue Strong
            |   _LogicValue Dominates
            |   _LogicValue LogicMapOut
            |   _LogicValue LogicMapIn
            |   _LogicValue Isolated
            |   _LogicValue Resolves
            |   _LogicValue Property
            |   _LogicValue Comment
            |   _LogicValue UserData
            ;

LogicWave :     LBR LOGICWAVEFORM _LogicWave RBR
          ;

_LogicWave :
           |    _LogicWave LogicNameRef
           |    _LogicWave LogicList
           |    _LogicWave LogicOneOf
           |    _LogicWave Ignore
           ;

Maintain :      LBR MAINTAIN __Maintain _Maintain RBR
         ;

__Maintain :    PortNameRef {;}
           |    PortRef     {;}
           ;

_Maintain :
          |     Delay
          |     LoadDelay
          ;

Match :         LBR MATCH __Match _Match RBR
      ;

__Match :       PortNameRef {;}
        |       PortRef     {;}
        |       PortList
        ;

_Match :        LogicNameRef
       |        LogicList
       |        LogicOneOf
       ;

Member :        LBR MEMBER NameRef _Member RBR
       ;

_Member :       Int {;}
        |       _Member Int
        ;

MiNoMa :        LBR MINOMAX _MiNoMa RBR
       ;

_MiNoMa :
        |       _MiNoMa MiNoMaValue
        |       _MiNoMa MiNoMaDisp
        |       _MiNoMa MiNoMa
        ;

MiNoMaDisp :    LBR MINOMAXDISPLAY _MiNoMaDisp RBR
           ;

_MiNoMaDisp :   MiNoMaValue
            |   _MiNoMaDisp Display
            ;

MiNoMaValue :   Mnm
            |   ScaledInt
            ;

Mnm :           LBR MNM _Mnm _Mnm _Mnm RBR
    ;

_Mnm :          ScaledInt
     |          Undefined
     |          Unconstrained
     ;

MultValSet :    LBR MULTIPLEVALUESET _MultValSet RBR
           ;

_MultValSet :
            |   _MultValSet RangeVector
            ;

MustJoin :      LBR MUSTJOIN _MustJoin RBR
         ;

_MustJoin :
          |     _MustJoin PortRef
          |     _MustJoin PortList
          |     _MustJoin WeakJoined
          |     _MustJoin Joined
          ;

Name :          LBR NAME _Name RBR { $<str>$ = $3; }
     ;

_Name :         Ident           { $<str>$ = $1; }
      |         _Name Display   { $<str>$ = $1; }
      ;

NameDef :       Ident   { $<str>$ = $1; }
        |       Name    { $<str>$ = $1; }
        |       Rename  { $<str>$ = $1; }
        ;

NameRef :       Ident { $<str>$ = $1; }
        |       Name  { $<str>$ = $1; }
        ;

Net :           LBR NET_TOK NetNameDef
                {
		NewNet(current_net);
		current_net->name = $3;
		current_net->circuit = current_circuit;
		}
                _Net RBR
                {
		int num_term = 0;
		CIRPORTREFPTR cpr = current_net->terminals;
		for (; cpr; cpr = cpr->next) ++num_term;
		current_net->num_term = num_term;
		$<net>$ = current_net;
		}
    ;

_Net :          Joined { current_net->terminals = $1; }
     |          _Net Criticality
     |          _Net NetDelay
     |          _Net Figure
     |          _Net Net
     |          _Net Instance
     |          _Net CommGraph
     |          _Net Property
     |          _Net Comment
     |          _Net UserData
     ;

NetBackAn :     LBR NETBACKANNOTATE _NetBackAn RBR
          ;

_NetBackAn :    NetRef
           |    _NetBackAn NetDelay
           |    _NetBackAn Criticality
           |    _NetBackAn Property
           |    _NetBackAn Comment
           ;

NetBundle :     LBR NETBUNDLE NetNameDef _NetBundle RBR
          ;

_NetBundle :    ListOfNets
           |    _NetBundle Figure
           |    _NetBundle CommGraph
           |    _NetBundle Property
           |    _NetBundle Comment
           |    _NetBundle UserData
           ;

NetDelay :      LBR NETDELAY Derivation _NetDelay RBR
         ;

_NetDelay :     Delay
          |     _NetDelay Transition
          |     _NetDelay Becomes
          ;

NetGroup :      LBR NETGROUP _NetGroup RBR
         ;

_NetGroup :
          |     _NetGroup NetNameRef
          |     _NetGroup NetRef
          ;

NetMap :        LBR NETMAP _NetMap RBR
       ;

_NetMap :
        |       _NetMap NetRef
        |       _NetMap NetGroup
        |       _NetMap Comment
        |       _NetMap UserData
        ;

NetNameDef :    NameDef { $<str>$ = $1; }
           |    Array   { $<str>$ = NULL; }
           ;

NetNameRef :    NameRef { $<str>$ = $1; }
           |    Member  { $<str>$ = NULL; }
           ;

NetRef :        LBR NETREF_TOK NetNameRef _NetRef RBR
       ;

_NetRef :
        |       NetRef
        |       InstanceRef {;}
        |       ViewRef
        ;

NoChange :      LBR NOCHANGE RBR
         ;

NonPermut :     LBR NONPERMUTABLE _NonPermut RBR
          ;

_NonPermut :
           |    _NonPermut PortRef
           |    _NonPermut Permutable
           ;

NotAllowed :    LBR NOTALLOWED RuleNameDef _NotAllowed RBR
           ;

_NotAllowed :   FigGrpObj
            |   _NotAllowed Comment
            |   _NotAllowed UserData
            ;

NotchSpace :    LBR NOTCHSPACING RuleNameDef FigGrpObj _NotchSpace RBR
           ;

_NotchSpace :   Range
            |   SingleValSet
            |   _NotchSpace Comment
            |   _NotchSpace UserData
            ;

Number :        LBR NUMBER _Number RBR
       ;

_Number :
        |       _Number ScaledInt
        |       _Number NumbDisplay
        |       _Number Number
        ;

NumbDisplay :   LBR NUMBERDISPLAY _NumbDisplay RBR
            ;

_NumbDisplay :  ScaledInt
             |  _NumbDisplay Display
             ;

NumberDefn :    LBR NUMBERDEFINITION _NumberDefn RBR
           ;

_NumberDefn :
            |   _NumberDefn Scale
            |   _NumberDefn GridMap
            |   _NumberDefn Comment
            ;

OffPageConn :   LBR OFFPAGECONNECTOR _OffPageConn RBR
            ;

_OffPageConn :  PortNameDef {;}
             |  _OffPageConn Unused
             |  _OffPageConn Property
             |  _OffPageConn Comment
             |  _OffPageConn UserData
             ;

OffsetEvent :   LBR OFFSETEVENT Event ScaledInt RBR
            ;

OpenShape :     LBR OPENSHAPE _OpenShape RBR
          ;

_OpenShape :    Curve
           |    _OpenShape Property
           ;

Orientation :   LBR ORIENTATION _Orientation RBR
            ;

_Orientation :  R0
             |  R90
             |  R180
             |  R270
             |  MX
             |  MY
             |  MYR90
             |  MXR90
             ;

Origin :        LBR ORIGIN PointValue RBR
       ;

OverhngDist :   LBR OVERHANGDISTANCE RuleNameDef FigGrpObj FigGrpObj _OverhngDist RBR
            ;

_OverhngDist :  Range
             |  SingleValSet
             |  _OverhngDist Comment
             |  _OverhngDist UserData
             ;

OverlapDist :   LBR OVERLAPDISTANCE RuleNameDef FigGrpObj FigGrpObj _OverlapDist RBR
            ;

_OverlapDist :  Range
             |  SingleValSet
             |  _OverlapDist Comment
             |  _OverlapDist UserData
             ;

Oversize :      LBR OVERSIZE Int _Oversize CornerType RBR
         ;

_Oversize :     FigGrpRef
          |     FigureOp
          ;

Owner :         LBR OWNER Str RBR
      ;

Page :          LBR PAGE _Page RBR
     ;

_Page :         InstNameDef {;}
      |         _Page Instance
      |         _Page Net
      |         _Page NetBundle
      |         _Page CommGraph
      |         _Page PortImpl
      |         _Page PageSize
      |         _Page BoundBox
      |         _Page Comment
      |         _Page UserData
      ;

PageSize :      LBR PAGESIZE Rectangle RBR
         ;

ParamDisp :     LBR PARAMETERDISPLAY _ParamDisp RBR
          ;

_ParamDisp :    ValueNameRef
           |    _ParamDisp Display
           ;

Parameter :     LBR PARAMETER ValueNameDef TypedValue _Parameter RBR
          ;

_Parameter :
           |    Unit
           ;

ParamAssign :   LBR PARAMETERASSIGN ValueNameRef TypedValue RBR
            ;

Path :          LBR PATH _Path RBR
     ;

_Path :         PointList
      |         _Path Property
      ;

PathDelay :     LBR PATHDELAY _PathDelay RBR
          ;

_PathDelay :    Delay
           |    _PathDelay Event
           ;

PathWidth :     LBR PATHWIDTH Int RBR
          ;

Permutable :    LBR PERMUTABLE _Permutable RBR
           ;

_Permutable :
            |   _Permutable PortRef
            |   _Permutable Permutable
            |   _Permutable NonPermut
            ;

Plug :          LBR PLUG _Plug RBR
     ;

_Plug :
      |         _Plug SocketSet
      ;

Point :         LBR POINT _Point RBR
      ;

_Point :
       |        _Point PointValue
       |        _Point PointDisp
       |        _Point Point
       ;

PointDisp :     LBR POINTDISPLAY _PointDisp RBR
          ;

_PointDisp :    PointValue
           |    _PointDisp Display
           ;

PointList :     LBR POINTLIST _PointList RBR
          ;

_PointList :
           |    _PointList PointValue
           ;

PointValue :    LBR PT Int Int RBR
           ;

Polygon :       LBR POLYGON _Polygon RBR
        ;

_Polygon :      PointList
         |      _Polygon Property
         ;

Port :          LBR PORT
                {
		CIRPORTPTR cp;
		switch (current_viewtype)
		   {
		case SeadifCircuitView: /* create CirPort and link in list */
		   NewCirport(cp); cp->name = NULL;
		   cp->next = current_circuit->cirport;
#ifdef SDF_PORT_DIRECTIONS
                   cp->direction = SDF_PORT_UNKNOWN;
#endif
		   current_circuit->cirport = cp;
		   break;
		default:
		   report(eFatal,"line %d: this port is not supported\n",ediflineno);
		   break;
		   }
                }
                _Port RBR
     ;

_Port :         PortNameDef
                {current_circuit->cirport->name = cs($1);}
      |         _Port Direction
      |         _Port Unused
      |         _Port PortDelay
      |         _Port Designator
      |         _Port DcFanInLoad
      |         _Port DcFanOutLoad
      |         _Port DcMaxFanIn
      |         _Port DcMaxFanOut
      |         _Port AcLoad
      |         _Port Property
      |         _Port Comment
      |         _Port UserData
      ;

PortBackAn :    LBR PORTBACKANNOTATE _PortBackAn RBR
           ;

_PortBackAn :   PortRef
            |   _PortBackAn Designator
            |   _PortBackAn PortDelay
            |   _PortBackAn DcFanInLoad
            |   _PortBackAn DcFanOutLoad
            |   _PortBackAn DcMaxFanIn
            |   _PortBackAn DcMaxFanOut
            |   _PortBackAn AcLoad
            |   _PortBackAn Property
            |   _PortBackAn Comment
            ;

PortBundle :    LBR PORTBUNDLE PortNameDef _PortBundle RBR
           ;

_PortBundle :   ListOfPorts
            |   _PortBundle Property
            |   _PortBundle Comment
            |   _PortBundle UserData
            ;

PortDelay :     LBR PORTDELAY Derivation _PortDelay RBR
          ;

_PortDelay :    Delay
           |    LoadDelay
           |    _PortDelay Transition
           |    _PortDelay Becomes
           ;

PortGroup :     LBR PORTGROUP _PortGroup RBR
          ;

_PortGroup :
           |    _PortGroup PortNameRef
           |    _PortGroup PortRef
           ;

PortImpl :      LBR PORTIMPLEMENTATION _PortImpl RBR
         ;

_PortImpl :     PortRef     {;}
          |     PortNameRef {;}
          |     _PortImpl ConnectLoc
          |     _PortImpl Figure
          |     _PortImpl Instance
          |     _PortImpl CommGraph
          |     _PortImpl PropDisplay
          |     _PortImpl KeywordDisp
          |     _PortImpl Property
          |     _PortImpl UserData
          |     _PortImpl Comment
          ;

PortInst :      LBR PORTINSTANCE _PortInst RBR
         ;

_PortInst :     PortRef            {;}
          |     PortNameRef        {;}
          |     _PortInst Unused
          |     _PortInst PortDelay
          |     _PortInst Designator
          |     _PortInst DcFanInLoad
          |     _PortInst DcFanOutLoad
          |     _PortInst DcMaxFanIn
          |     _PortInst DcMaxFanOut
          |     _PortInst AcLoad
          |     _PortInst Property
          |     _PortInst Comment
          |     _PortInst UserData
          ;

PortList :      LBR PORTLIST _PortList RBR
         ;

_PortList :
          |     _PortList PortRef
          |     _PortList PortNameRef
          ;

PortListAls :   LBR PORTLISTALIAS PortNameDef PortList RBR
            ;

PortMap :       LBR PORTMAP _PortMap RBR
        ;

_PortMap :
         |      _PortMap PortRef
         |      _PortMap PortGroup
         |      _PortMap Comment
         |      _PortMap UserData
         ;

PortNameDef :   NameDef { $<str>$ = $1; }
            |   Array   { $<str>$ = NULL; }
            ;

PortNameRef :   NameRef { $<str>$ = $1; }
            |   Member  { $<str>$ = NULL; }
            ;

PortRef :       LBR PORTREF PortNameRef _PortRef RBR
                {
		switch (current_viewtype)
		   {
		case SeadifCircuitView:
		   NewCirportref(current_cirportref);
		   current_cirportref->cirport = (CIRPORTPTR) $3; /* __HACK__ */
		   current_cirportref->cirinst = (CIRINSTPTR) $4; /* __HACK__ */
		   current_cirportref->net = current_net;
		   break;
		default:
		   break;
		   }
		}
        ;

_PortRef :      { $<str>$ = NULL; /* empty */ }
         |      PortRef          { $<str>$ = NULL; }
         |      InstanceRef      { $<str>$ = $1; }
         |      ViewRef          { $<str>$ = NULL; }
         ;

Program :       LBR PROGRAM Str _Program RBR { $<str>$ = $3; }
        ;

_Program :
         |      Version
         ;

PropDisplay :   LBR PROPERTYDISPLAY _PropDisplay RBR
            ;

_PropDisplay :  PropNameRef
             |  _PropDisplay Display
             ;

Property :      LBR PROPERTY PropNameDef _Property RBR
         ;

_Property :     TypedValue
          |     _Property Owner
          |     _Property Unit
          |     _Property Property
          |     _Property Comment
          ;

PropNameDef :   NameDef { $<str>$ = $1; }
            ;

PropNameRef :   NameRef { $<str>$ = $1; }
            ;

ProtectFrame :  LBR PROTECTIONFRAME _ProtectFrame RBR
             ;

_ProtectFrame :
              | _ProtectFrame PortImpl
              | _ProtectFrame Figure
              | _ProtectFrame Instance
              | _ProtectFrame CommGraph
              | _ProtectFrame BoundBox
              | _ProtectFrame PropDisplay
              | _ProtectFrame KeywordDisp
              | _ProtectFrame ParamDisp
              | _ProtectFrame Property
              | _ProtectFrame Comment
              | _ProtectFrame UserData
              ;

Range :         LessThan
      |         GreaterThan
      |         AtMost
      |         AtLeast
      |         Exactly
      |         Between
      ;

RangeVector :   LBR RANGEVECTOR _RangeVector RBR
            ;

_RangeVector :
             |  _RangeVector Range
             |  _RangeVector SingleValSet
             ;

Rectangle :     LBR RECTANGLE PointValue _Rectangle RBR
          ;

_Rectangle :    PointValue
           |    _Rectangle Property
           ;

RectSize :      LBR RECTANGLESIZE RuleNameDef FigGrpObj _RectSize RBR
         ;

_RectSize :     RangeVector
          |     MultValSet
          |     _RectSize Comment
          |     _RectSize UserData
          ;

Rename :        LBR RENAME __Rename _Rename RBR
                {
		/* we ignore the rename facility, return the EDIF name: */
		$<str>$ = $3;
                }
       ;

__Rename :      Ident { $<str>$ = $1; }
         |      Name  { $<str>$ = $1; }
         ;

_Rename :       Str        { $<str>$ = $1; }
        |       StrDisplay { $<str>$ = NULL; }
        ;

Resolves :      LBR RESOLVES _Resolves RBR
         ;

_Resolves :
          |     _Resolves LogicNameRef
          ;

RuleNameDef :   NameDef { $<str>$ = $1; }
            ;

Scale :         LBR SCALE ScaledInt ScaledInt Unit RBR
      ;

ScaledInt :     Int {;}
          |     LBR E_TOK Int Int RBR
          ;

ScaleX :        LBR SCALEX Int Int RBR
       ;

ScaleY :        LBR SCALEY Int Int RBR
       ;

Section :       LBR SECTION _Section RBR
        ;

_Section :      Str {;}
         |      _Section Section
         |      _Section Str
         |      _Section Instance
         ;

Shape :         LBR SHAPE _Shape RBR
      ;

_Shape :        Curve
       |        _Shape Property
       ;

SimNameDef :    NameDef { $<str>$ = $1; }
           ;

Simulate :      LBR SIMULATE _Simulate RBR
         ;

_Simulate :     SimNameDef
          |     _Simulate PortListAls
          |     _Simulate WaveValue
          |     _Simulate Apply
          |     _Simulate Comment
          |     _Simulate UserData
          ;

SimulInfo :     LBR SIMULATIONINFO _SimulInfo RBR
          ;

_SimulInfo :
           |    _SimulInfo LogicValue
           |    _SimulInfo Comment
           |    _SimulInfo UserData
           ;

SingleValSet :  LBR SINGLEVALUESET _SingleValSet RBR
             ;

_SingleValSet :
              | Range
              ;

Site :          LBR SITE ViewRef _Site RBR
     ;

_Site :
      |         Transform
      ;

Socket :        LBR SOCKET _Socket RBR
       ;

_Socket :
        |       Symmetry
        ;

SocketSet :     LBR SOCKETSET _SocketSet RBR
          ;

_SocketSet :    Symmetry
           |    _SocketSet Site
           ;

Status :        LBR STATUS_TOK
                {
		NewStatus(current_status);
                }
                _Status RBR
                {
		$<status>$ = current_status;
		}
       ;

_Status :
        |       _Status Written
        |       _Status Comment
        |       _Status UserData
        ;

Steady :        LBR STEADY __Steady _Steady RBR
       ;

__Steady :      PortNameRef {;}
         |      PortRef     {;}
         |      PortList
         ;

_Steady :       Duration
        |       _Steady Transition
        |       _Steady Becomes
        ;

StrDisplay :    LBR STRINGDISPLAY _StrDisplay RBR
           ;

String :        LBR STRING_TOK _String RBR
       ;

_String :
        |       _String Str
        |       _String StrDisplay
        |       _String String
        ;

_StrDisplay :   Str {;}
            |   _StrDisplay Display
            ;

Strong :        LBR STRONG LogicNameRef RBR
       ;

Symbol :        LBR SYMBOL _Symbol RBR
       ;

_Symbol :
        |       _Symbol PortImpl
        |       _Symbol Figure
        |       _Symbol Instance
        |       _Symbol CommGraph
        |       _Symbol Annotate
        |       _Symbol PageSize
        |       _Symbol BoundBox
        |       _Symbol PropDisplay
        |       _Symbol KeywordDisp
        |       _Symbol ParamDisp
        |       _Symbol Property
        |       _Symbol Comment
        |       _Symbol UserData
        ;

Symmetry :      LBR SYMMETRY _Symmetry RBR
         ;

_Symmetry :
          |     _Symmetry Transform
          ;

Table :         LBR TABLE _Table RBR
      ;

_Table :
       |        _Table Entry
       |        _Table TableDeflt
       ;

TableDeflt :    LBR TABLEDEFAULT __TableDeflt _TableDeflt RBR
           ;

__TableDeflt :  LogicRef
             |  PortRef
             |  NoChange
             |  Table
             ;

_TableDeflt :
            |   Delay
            |   LoadDelay
            ;

Technology :    LBR TECHNOLOGY _Technology RBR
           ;

_Technology :   NumberDefn
            |   _Technology FigGrp
            |   _Technology Fabricate
            |   _Technology SimulInfo
            |   _Technology DesignRule
            |   _Technology Comment
            |   _Technology UserData
            ;

TextHeight :    LBR TEXTHEIGHT Int RBR
           ;

TimeIntval :    LBR TIMEINTERVAL __TimeIntval _TimeIntval RBR
           ;

__TimeIntval :  Event
             |  OffsetEvent
             ;

_TimeIntval :   Event
            |   OffsetEvent
            |   Duration
            ;

TimeStamp :     LBR TIMESTAMP Int Int Int Int Int Int RBR
	{
	    time_t thetime = 0;
	    sdftimecvt (&thetime, $3, $4, $5, $6, $7, $8);
	    $<time>$ = thetime;
	}
	;

Timing :        LBR TIMING_TOK _Timing RBR
       ;

_Timing :       Derivation
        |       _Timing PathDelay
        |       _Timing Forbidden
        |       _Timing Comment
        |       _Timing UserData
        ;

Transform :     LBR TRANSFORM _TransX _TransY _TransDelta _TransOrien _TransOrg RBR
          ;

_TransX :
        |       ScaleX
        ;

_TransY :
        |       ScaleY
        ;

_TransDelta :
            |   Delta
            ;

_TransOrien :
            |   Orientation
            ;

_TransOrg :
          |     Origin
          ;

Transition :    LBR TRANSITION _Transition _Transition RBR
           ;

_Transition :   LogicNameRef
            |   LogicList
            |   LogicOneOf
            ;

Trigger :       LBR TRIGGER _Trigger RBR
        ;

_Trigger :
         |      _Trigger Change
         |      _Trigger Steady
         |      _Trigger Initial
         ;

True :          LBR TRUE_TOK RBR
     ;

TypedValue :    Boolean
           |    Integer
           |    MiNoMa
           |    Number
           |    Point
           |    String
           ;

Unconstrained : LBR UNCONSTRAINED RBR
              ;

Undefined :     LBR UNDEFINED RBR
          ;

Union :         LBR UNION _Union RBR
      ;

_Union :        FigGrpRef
       |        FigureOp
       |        _Union FigGrpRef
       |        _Union FigureOp
       ;

Unit :          LBR UNIT _Unit RBR
     ;

_Unit :         DISTANCE
      |         CAPACITANCE
      |         CURRENT
      |         RESISTANCE
      |         TEMPERATURE
      |         TIME
      |         VOLTAGE
      |         MASS
      |         FREQUENCY
      |         INDUCTANCE
      |         ENERGY
      |         POWER
      |         CHARGE
      |         CONDUCTANCE
      |         FLUX
      |         ANGLE
      ;

Unused :        LBR UNUSED RBR
       ;

UserData :      LBR USERDATA _UserData RBR
         ;

_UserData :     /* empty */
          |     _UserData Int
          |     _UserData Str
          |     _UserData LBR _UserData RBR
          ;

ValueNameDef :  NameDef { $<str>$ = $1; }
             |  Array   { $<str>$ = NULL; }
             ;

ValueNameRef :  NameRef { $<str>$ = $1; }
             |  Member  { $<str>$ = NULL; }
             ;

Version :       LBR VERSION Str RBR
        ;

View	:	LBR VIEW ViewNameDef ViewType
	{
	    current_viewtype = $4;

	    if (current_viewtype == SeadifCircuitView)
		NewCircuit (current_circuit);
	    else
		report (eFatal, "line %d: cannot handle viewType \"%s\"",
		    ediflineno, current_viewtype_string);
	}
		View_ RBR
	{
	    $<viewtype>$ = current_viewtype;
	}
	;
View_	:	Interface
	|	View_ Status
	|	View_ Contents
	|	View_ Comment
	|	View_ Property
	|	View_ UserData
	;

ViewList :	LBR VIEWLIST ViewList_ RBR
	;
ViewList_ :
	|	ViewList_ ViewRef
	|	ViewList_ ViewList
	;

ViewMap	:	LBR VIEWMAP ViewMap_ RBR
	;
ViewMap_ :
	|	ViewMap_ PortMap
	|	ViewMap_ PortBackAn
	|	ViewMap_ InstMap
	|	ViewMap_ InstBackAn
	|	ViewMap_ NetMap
	|	ViewMap_ NetBackAn
	|	ViewMap_ Comment
	|	ViewMap_ UserData
	;

ViewNameDef :   NameDef { $<str>$ = $1; }
            ;

ViewRef :       LBR VIEWREF
                {
		/* the name of the VIEWREF can be any string. In particular, it
		 * can be an edif keyword. This sort of thing means that the
		 * edif syntax (at least as generated by Cadence "edifout") is
		 * not context-free. Oh well...
		 */
		ediflex();	  /* we call ediflex() ourselves... */
		/* the VIEWREF identifier is now in (char *)ediftext */
		if (current_instance)
		   current_instance->view_name_ref = cs(ediftext);
                }
                _ViewRef RBR
        ;

_ViewRef :
         |      CellRef
         ;

ViewType :      LBR VIEWTYPE _ViewType
                {
		/* save the name of the current view type
		   (e.g. for error messages) */
		if (current_viewtype_string)
		   fs(current_viewtype_string);
		current_viewtype_string = cs(ediftext);
		}
                RBR
                {
                $<viewtype>$ = $3;
                }
         ;

_ViewType :     MASKLAYOUT { $<viewtype>$ = SeadifLayoutView; }
          |     PCBLAYOUT  { $<viewtype>$ = SeadifNoView; }
          |     NETLIST    { $<viewtype>$ = SeadifCircuitView; }
          |     SCHEMATIC  { $<viewtype>$ = SeadifNoView; }
          |     SYMBOLIC   { $<viewtype>$ = SeadifNoView; }
          |     BEHAVIOR   { $<viewtype>$ = SeadifFunctionView; }
          |     LOGICMODEL { $<viewtype>$ = SeadifNoView; }
          |     DOCUMENT   { $<viewtype>$ = SeadifNoView; }
          |     GRAPHIC    { $<viewtype>$ = SeadifNoView; }
          |     STRANGER   { $<viewtype>$ = SeadifNoView; }
          ;

Visible :       LBR VISIBLE BooleanValue RBR
        ;

VoltageMap :    LBR VOLTAGEMAP MiNoMaValue RBR
           ;

WaveValue :     LBR WAVEVALUE LogicNameDef ScaledInt LogicWave RBR
          ;

Weak :          LBR WEAK LogicNameRef RBR
     ;

WeakJoined :    LBR WEAKJOINED _WeakJoined RBR
           ;

_WeakJoined :
            |   _WeakJoined PortRef
            |   _WeakJoined PortList
            |   _WeakJoined Joined
            ;

When :          LBR WHEN _When RBR
     ;

_When :         Trigger
      |         _When After
      |         _When Follow
      |         _When Maintain
      |         _When LogicAssn
      |         _When Comment
      |         _When UserData
      ;

Written :       LBR WRITTEN _Written RBR
        ;

_Written :      TimeStamp          { current_status->timestamp = $1; }
         |      _Written Author    { current_status->author = $2; }
         |      _Written Program   { current_status->program = $2; }
         |      _Written DataOrigin
         |      _Written Property
         |      _Written Comment
         |      _Written UserData
         ;

Str :           STR
    ;

Ident :         STR
      ;

Int :           INT
    ;

%%

int ediferror (char *mesg)
{
   report (eFatal, "Edif parser, line %d: %s", ediflineno, mesg);
   return 0;
}
