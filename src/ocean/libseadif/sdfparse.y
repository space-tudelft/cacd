%{
/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
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
 * Parser for the ocean sea-of-gates system.
 */

#include "src/ocean/libseadif/sea_decl.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define HACK  NULL
#define FALSE 0
#define SDFDISCARDSPACES TRUE
#define MAXNAMELEN 300

/* Following are the 3 states of a state machine that checks:
 *   - whether lib.status comes before any Function;
 *   - whether fun.status comes before any Circuit;
 *   - whether cir.status, cir.cirport, cir.cirinst and
 *     cir.netlist all come before any Layout.
 * If this is the case, the info field in the index will have
 * its state(SDFFASTPARSE) bit set. Subsequent reads of the
 * lib (or fun or cir) can then quit parsing as soon as a
 * Function (or Circuit or Layout) is encountered because it
 * can be sure that nothing interesting is going to appear
 * until the end of the lib (or fun or cir). This way a
 * significant speed-up can be realized when reading a lib,
 * a fun or a cir. (No advantage for layout parsing of course.)
 */
#define SDF_SEEN_NOTHING_YET        2 /* initial state */
#define SDF_LETS_KEEP_IT_LIKE_THIS  1 /* seen consecutive Functions */
#define SDF_LOST_CAUSE              0 /* Function was followed by something else */

#define debug(string) /* fprintf (stderr, "[%d]--> %s\n", yylineno, string) */ ;
/* Alternatively, set the integer yydebug to 1. */

#define FilNam (fnprinted ? "" : printfilnam())

typedef struct
{
   short hor, ver;
}
HORVER, *HORVERPTR; /* For passing (horizontal,vertical) positions to the caller. */

typedef short MATRIX[6];	  /* Orientation matrix mtx[0..5] */

int sdfstrcasecmp (char*, char*);
void yyerror (char *s);
int yylex (void);

char *findlibname = NULL;  /* These name must be set externally.  */
char *findfunname = NULL;  /* If find...name is NULL the parser parses all ... */
char *findcirname = NULL;  /* e.g. find{lib,fun,cir}name !=NULL && findlayname==NULL */
STRING findlayname = NULL;  /* means "read all layouts in the specified circuit." */

int sdfverbose = 0;	  /* Be chatty or not... default is quiet */
NAMELISTLISTPTR libseen = NULL;
NAMELISTPTR     libparsed = NULL;
NAMELISTPTR     libunparsed = NULL;
PRIVATE char *seadifinputfilename;
PRIVATE int junklib = TRUE;	  /* False when parsing a library we want. */
PRIVATE int junkfun = TRUE;	  /* False when parsing a function we want. */
PRIVATE int junkcir = TRUE;	  /* False when parsing a circuit we want. */
PRIVATE int junkstat = TRUE;	  /* False when parsing a Status we want. */
PRIVATE int junklay = TRUE;	  /* False when parsing a layout we want. */
PRIVATE int junktm = TRUE;        /* IK, False when parsing a timing we want */
PRIVATE int fnprinted;		  /* Boolean tells whether file name has
				   * already been printed in case of error. */
/* Following are state variables when building an
 * index and they are booleans during normal parsing: */
int sdffunislastthinginlib; /* for preliminary abortion of lib parsing */
int sdfcirislastthinginfun; /* for preliminary abortion of fun parsing */
int sdflayislastthingincir; /* for preliminary abortion of cir parsing */
PRIVATE SDFINFO info;

int sdfobligetimestamp; /* for support of the sdftouch() functions */
time_t sdftimestamp;

extern  int yylineno;
extern  int sdfcopytheinput;
extern  FILEPTR sdfcopystream;

extern char sdftimecvterror[];	  /* contains error msg from sdftimecvt() */

extern  LIBTABPTR    thislibtab;  /* Current entry in the lib hash table */
extern  FUNTABPTR    thisfuntab;  /* Current entry in the fun hash table */
extern  CIRTABPTR    thiscirtab;  /* Current entry in the cir hash table */
extern  LAYTABPTR    thislaytab;  /* Current entry in the lay hash table */
extern  char sdfcurrentfileidx;	  /* Index for sdffileinfo[] */
extern SDFFILEINFO sdffileinfo[];
extern  int makeindex;
unsigned int sdfwhat;  /* specifies the parts to be read into core */
unsigned int sdfstuff; /* parts to be deleted while copying to the scratch file */
unsigned int sdfwrite; /* parts to write to the scratch file */
LIBRARYPTR  sdfwritethislib; /* library to be written onto the scratch file */
FUNCTIONPTR sdfwritethisfun; /* function to be written onto the scratch file */
CIRCUITPTR  sdfwritethiscir; /* circuit to be written onto the scratch file */
LAYOUTPTR   sdfwritethislay; /* layout to be written onto the scratch file */
PRIVATE int skipthisthingforindex = 0;
int sdfparseonelib = 0;
int sdfparseonefun = 0;
int sdfparseonecir = 0;
int sdfparseonelay = 0;
PRIVATE int sdfslicedepth = 0;	  /* counts the depth of the sliceig tree */
PRIVATE long sdfleftparenthesis;  /* File position of most recent '(' */
extern  long sdffilepos;	  /* Current position in file, see sdfinput() in lex source */
PRIVATE char sdftmpstring[MAXNAMELEN+1]; /* for copying status fields */
PRIVATE int sdfhavethisthing;
/* Some of the following (the public ones) are communicated to libio.c */
PRIVATE SEADIF       yyseadifile; /* Holds the current Seadif file. */
LIBRARYPTR   libraryptr;  /* Holds the current library (SDFLIBBODY). */
FUNCTIONPTR  functionptr; /* Holds the current Function (SDFFUNBODY). */
CIRCUITPTR   circuitptr;  /* Holds the current Circuit (SDFCIRBODY). */
LAYOUTPTR    layoutptr;	  /* Holds the current Layout (SDFLAYBODY). */
PRIVATE CIRPORTPTR   cirportlistptr; /* Holds the current CirPortList (SDFCIRPORT). */
PRIVATE CIRPORTPTR   cirportptr;  /* Holds the current CirPort. */
CIRINSTPTR   cirinstlistptr; /* Holds the current CirInstList (SDFCIRINST). */
PRIVATE CIRINSTPTR   cirinstptr;  /* Holds the current CirInstance. */
PRIVATE BUSPTR       buslistptr;  /* Holds the current bus (SDFBUS) */
PRIVATE NETREFPTR    netreflistptr; /* Hold current list of NetRefs x*/
NETPTR       netlistptr;  /* Holds the current NetList (SDFCIRNETLIST). */
PRIVATE NETPTR       netptr;	  /* Holds the current Net */
PRIVATE CIRPORTREFPTR cirportrefptr; /* Holds the current Joined list. */
PRIVATE CIRPORTREFPTR cirportreflistptr; /* Holds the current NetPortRef. */
LAYPORTPTR   layportlistptr; /* Holds the current LayPortList (SDFLAYPORT). */
PRIVATE LAYPORTPTR   layportptr;  /* Holds the current LayPort. */
LAYLABELPTR  laylabellistptr; /* Holds the current LayLabelList (SDFLAYLABEL). */
PRIVATE LAYLABELPTR  laylabelptr; /* Holds the current LayLabel. */
LAYINSTPTR   layinstptr;  /* Holds the current LayInstance (SDFLAYSLICE). */
WIREPTR      wirelistptr; /* Holds the current wire (SDFLAYWIRE). */
PRIVATE WIREPTR      wireptr;	  /* Holds the current wire. */
PRIVATE STATUSPTR    statusptr;	  /* Holds the current status description. */

				  /* IK, new structures for timing model */
PRIVATE TIMINGPTR       timingPtr;
PRIVATE TIMETERMPTR     ttermlistPtr;
PRIVATE TIMETERMPTR     timetermPtr;
PRIVATE CIRPORTREFPTR   cirportrefPtr;
PRIVATE TIMETERMREFPTR  timetermrefPtr;
PRIVATE TMMODINSTPTR    tmmodinstlistPtr;
PRIVATE TMMODINSTPTR    tmmodinstPtr;
PRIVATE NETMODPTR       netmodlistPtr;
PRIVATE NETMODPTR       netmodPtr;
PRIVATE TPATHPTR        tpathlistPtr;
PRIVATE TPATHPTR        tpathPtr;
PRIVATE TIMETERMREFPTR  starttermlistPtr;
PRIVATE TIMETERMREFPTR  endtermlistPtr;
PRIVATE TIMECOSTPTR     timecostPtr;
PRIVATE TCPOINTPTR      tcpointPtr;
PRIVATE DELASGPTR       delasgPtr;
PRIVATE DELASGINSTPTR   delasginstPtr;
PRIVATE DELASGINSTPTR   delasginstlistPtr;

PRIVATE LIBRARY_TYPE    junklibrary;
PRIVATE FUNCTION_TYPE   junkfunction;
PRIVATE CIRCUIT_TYPE    junkcircuit;
PRIVATE CIRINST_TYPE    junkcirinst;
PRIVATE CIRPORT_TYPE    junkcirport;
PRIVATE CIRPORTREF_TYPE junkcirportref;
PRIVATE NET_TYPE        junknet;
PRIVATE LAYOUT_TYPE     junklayout;
PRIVATE LAYPORT_TYPE    junklayport;
PRIVATE LAYLABEL_TYPE   junklaylabel;
PRIVATE LAYINST_TYPE    junklayinst;
PRIVATE SLICE_TYPE      junkslice;
PRIVATE WIRE_TYPE       junkwire;

PRIVATE TIMING_TYPE       junktiming;
PRIVATE TIMETERM_TYPE     junktimeterm;
PRIVATE TIMETERMREF_TYPE  junktimetermref;
PRIVATE TMMODINST_TYPE    junktmmodinst;
PRIVATE NETMOD_TYPE       junknetmod;
PRIVATE TPATH_TYPE        junktpath;
PRIVATE TIMECOST_TYPE     junktimecost;
PRIVATE TCPOINT_TYPE      junktcpoint;
PRIVATE DELASG_TYPE       junkdelasg;
PRIVATE DELASGINST_TYPE   junkdelasginst;

/* Following are prototypes for PRIVATE functions in this Yacc file: */
PRIVATE short atos (char *str);
PRIVATE char *downcase (char *str);
PRIVATE char *printfilnam (void);
PRIVATE void skipthisthing (void);
PRIVATE char *copythisthing (void);
PRIVATE void checkthatalllayhasbeenwritten (int thingsstilltobewritten);
PRIVATE void checkthatallcirhasbeenwritten (int thingsstilltobewritten);
PRIVATE void checkthatallfunhasbeenwritten (int thingsstilltobewritten);
PRIVATE void checkthatalllibhasbeenwritten (int thingsstilltobewritten);

/* THINGS returns all the things that we where requested to write MINUS
 * the things that we already wrote; in other words, it returns the things
 * that we still have to write.
 */
#define THINGS(_sdfall_, _sdfalias_) (!(sdfwrite & _sdfall_) ? \
	0 : ((sdfwrite & _sdfall_) | _sdfalias_) & ~sdfhavethisthing)

#ifndef THINGS
/* this function is equivalent to the THINGS macro */
static int THINGS (int _sdfall_, int _sdfalias_)
{
   if (!(sdfwrite & _sdfall_)) return 0;
   return ((sdfwrite & _sdfall_) | _sdfalias_) & ~sdfhavethisthing;
}
#endif

%}
%union {
        char            str[200];
	STRING          canonicstr;
	NAMELISTPTR     namelist;
	HORVER          horver;
	MATRIX          matrix;
	STATUSPTR       status;
	SEADIFPTR       seadif;
	LIBRARYPTR      library;
	FUNCTIONPTR     function;
	CIRCUITPTR      circuit;
	CIRINSTPTR      cirinst;
	CIRPORTPTR      cirport;
	CIRPORTREFPTR   cirportref;
	NETPTR          net;
        BUSPTR          bus;
        NETREFPTR       netref;
	LAYOUTPTR       layout;
	LAYPORTPTR      layport;
	LAYLABELPTR     laylabel;
	LAYINSTPTR      layinst;
	short           layer;
	SLICEPTR        slice;
	WIREPTR         wire;
	void*           nothing;
				  /* IK timing structures types */
        TIMINGPTR       timing;
        TIMETERMPTR     tterm;
	CIRPORTREFPTR   cportref;
	TIMETERMREFPTR  timetermref;
        TMMODINSTPTR    tmmodinst;
        NETMODPTR       netmod;
	BUSREFPTR       busref;
        TPATHPTR        tpath;
        TIMECOSTPTR     timecost;
        TCPOINTPTR      tcpoint;
        DELASGPTR       delasg;
        DELASGINSTPTR   delasginst;
	long            cycle;
	double          relcycletime;
}

%token <str> STRNG
%token <str> NUMBER
%token LBRTOKEN RBR
%token SEADIFTOKEN
%token LIBRARYTOKEN
%token ALIAS
%token TECHNOLOGY
%token FUNCTIONTOKEN
%token FUNCTIONTYPE
%token FUNCTIONLIBREF
%token CIRCUITTOKEN
%token ATTRIBUTE
%token CIRCUITPORTLIST
%token CIRCUITPORT
%token DIRECTION
%token CIRCUITINSTANCELIST
%token CIRCUITINSTANCE
%token CIRCUITCELLREF
%token CIRCUITFUNREF
%token CIRCUITLIBREF
%token NETLIST
%token NETTOKEN
%token BUSLISTTOKEN
%token BUSTOKEN
%token NETREFTOKEN
%token JOINED
%token NETPORTREF
%token NETINSTREF
%token LAYOUTTOKEN
%token LAYOUTPORTLIST
%token LAYOUTPORT
%token PORTPOSITION
%token PORTLAYER
%token LAYOUTLABELLIST
%token LAYOUTLABEL
%token LABELPOSITION
%token LABELLAYER
%token LAYOUTBBX
%token LAYOUTOFFSET
%token LAYOUTINSTANCELIST
%token LAYOUTINSTANCE
%token LAYOUTSLICE
%token LAYOUTCELLREF
%token LAYOUTCIRREF
%token LAYOUTFUNREF
%token LAYOUTLIBREF
%token ORIENTATION
%token WIRELIST
%token WIRETOKEN

%token STATUSTOKEN
%token WRITTEN
%token TIMESTAMP
%token AUTHOR
%token PROGRAM
%token COMMENT
				  /* IK - timing extensions: */
%token TIMINGTOKEN
%token TIMETERMLISTTOKEN
%token TIMETERMTOKEN
%token CIRPORTREFTOKEN
%token TIMETERMREFTOKEN
%token TMMODINSTREFTOKEN
%token INPUTLOADTOKEN
%token INPUTDRIVETOKEN
%token REQINPUTTIMETOKEN
%token OUTPUTTIMETOKEN
%token TMMODINSTLISTTOKEN
%token TMMODINSTTOKEN
%token CINSTREFTOKEN
%token TIMINGREFTOKEN
%token NETMODLISTTOKEN
%token NETMODTOKEN
%token BUSREFTOKEN
%token TPATHLISTTOKEN
%token TPATHTOKEN
%token STARTTERMLISTTOKEN
%token ENDTERMLISTTOKEN
%token TIMECOSTTOKEN
%token TCPOINTTOKEN
%token DELASGTOKEN
%token CLOCKCYCLETOKEN
%token DELASGINSTLISTTOKEN
%token DELASGINSTTOKEN
%token TPATHREFTOKEN
%token TCPOINTREFTOKEN

%type <canonicstr> SeadifFileName Technology
%type <canonicstr> NetInstRef _NetInstRef Alias
%type <canonicstr> FunctionType OptionalString Attribute
%type <namelist> CirFunRef CirLibRef LayCirRef LayFunRef LayLibRef
%type <horver> LayBoundingBox LayOffset PortPos LabelPos
%type <matrix> Orientation
%type <status> SeaStatus LibStatus CirStatus FunStatus LayStatus
%type <seadif> Seadif
%type <library> Library _Library
%type <function> Function _Function
%type <circuit> CircuitImpl _CircuitImpl
%type <cirinst> CirInstance CirInstList _CirInstList _CirInstRef CirCellRef
%type <cirport> CirPortList _CirPortList CirPort
%type <cirportref> NetPortRef _NetPortRef NetPortRefInBus
%type <net> NetList _NetList Net
%type <bus> Bus _BusList BusList
%type <netref> NetRef NetRefList
%type <layout> LayoutImpl _LayoutImpl
%type <layport> LayPortList _LayPortList LayPort _LayPort
%type <laylabel> LayLabelList _LayLabelList LayLabel _LayLabel
%type <layer> PortLayer
%type <layer> LabelLayer
%type <layinst> LayInstance
%type <slice> LayInstList _LayInstList LaySlice _LaySlice LaySliceRef
%type <wire> WireList _WireList Wire
%type <nothing> Seadifentry
				  /* IK, ... and timing extensions : */
%type <timing> Timing _Timing
%type <tterm> TimeTerm _TimeTerm TimeTermList _TimeTermList
%type <cirportref> CirPortRef
%type <timetermref> TimeTermRef _TimeTermRef  TmModInstRef
%type <relcycletime> ReqInputTime OutputTime InputLoad InputDrive
%type <tmmodinst> TmModInst TmModInstList _TmModInstList
%type <canonicstr> TimingRef CInstRef
%type <netmod> NetMod _NetMod NetModList _NetModList
%type <busref> BusRef
%type <tpath> TPath  TPathList _TPathList TPathRef
%type <timetermref> StartTermList StartTermList_ EndTermList EndTermList_
%type <timecost> TimeCost _TimeCost
%type <tcpoint> TcPoint TcPointRef
%type <delasg> DelAsg _DelAsg
%type <cycle> ClockCycle
%type <delasginst> DelAsgInst  DelAsgInstList _DelAsgInstList
%type <status>  TmStatus
%start Seadifentry

%%

Seadifentry    : Seadif       { $<nothing>$ = (void *)0; }
               | Library      { $<nothing>$ = (void *)0; }
               | Function     { $<nothing>$ = (void *)0; }
               | CircuitImpl  { $<nothing>$ = (void *)0; }
               | LayoutImpl   { $<nothing>$ = (void *)0; }
               ;

Seadif         : LBR SEADIFTOKEN SeadifFileName
{
    debug ("LBR SEADIFTOKEN SeadifFileName")
    fnprinted = FALSE;
    yyseadifile.filename = $3;
    yyseadifile.library = NULL;
    yyseadifile.status = NULL;
}
                 _Seadif RBR	  /* Skip EdifVersion, EdifLevel, KeywordMap */
{
    debug ("_Seadif RBR")
    $$ = (&yyseadifile);
}
                ;

SeadifFileName : STRNG
{
    debug ("STRNG")
    $$ = canonicstring ($1);
}
	       ;

_Seadif        : /* nothing */
{
    debug ("empty _Seadif");
}
               | _Seadif Library
{
    debug ("_Seadif Library");
}
               | _Seadif SeaStatus
{
    debug ("_Seadif Status");
}
               | _Seadif Comment
{
    debug ("_Seadif Comment");
}
               | _Seadif UserData
{
    debug ("_Seadif UserData");
}
               ;

/*
 * Library module
 */
Library        : LBR LIBRARYTOKEN STRNG
{
    debug ("LBR LIBRARYTOKEN STRNG");
    sdfhavethisthing &= ~(SDFLIBALL | SDF_X_LIBALIAS);
    sdfhavethisthing |= SDFLIBBODY;
    if (makeindex && !skipthisthingforindex)
    {
	junklibrary.name = canonicstring ($3);
	if (existslib (junklibrary.name))
	{
	    sdfreport (Error, "Makeindex: multiple libraries \"%s\"\n"
		"           file \"%s\", char pos %d\n"
		"           file \"%s\", char pos %d",
		$3, sdffileinfo[(int)sdfcurrentfileidx].name, sdfleftparenthesis,
		sdffileinfo[(int)thislibtab->info.file].name, thislibtab->info.fpos);
	    skipthisthingforindex = 1; /* 1 means: skip this lib for index */
	    return (1);
	}
	else
	{
	    info.what = 0;
	    info.state = 0;
	    info.file = sdfcurrentfileidx;
	    info.fpos = sdfleftparenthesis; /* position of most recent '(' */
	    addlibtohashtable (&junklibrary, &info);
	    sdffunislastthinginlib = SDF_SEEN_NOTHING_YET;
	}
	forgetstring (junklibrary.name);
    }

    if (sdfstuff & SDFLIBBODY) sdfabortcopy (SDFDISCARDSPACES);

    /* Don't check the name of the library */
    if (sdfwhat & SDFLIBBODY)
    {
	junklib = FALSE;		  /* Encountered a library that must be parsed. */
	NewLibrary (libraryptr);
	libraryptr->name = canonicstring ($3);
	if (sdfverbose) fprintf (stderr, "...(Library \"%s\")\n", libraryptr->name);
    }
    else
    {
	junklib = TRUE;		  /* We do not want this library: parse but do not */
	libraryptr = &junklibrary;	  /*    actually reserve storage for its contents. */
	libraryptr->name = NULL;
    }
    libraryptr->next = yyseadifile.library;
    if (!junklib)
	yyseadifile.library = libraryptr; /* Link in front of library list. */
}
                 _Library RBR
{
    int thingsstilltobewritten;

    debug ("_Library RBR")
    if ((thingsstilltobewritten = THINGS (SDFLIBALL, SDF_X_LIBALIAS)))
    {
	if (sdfcopytheinput)
	{ /* First, flush whatever is in the copy buffer up till RBR */
	    sdfuncopysincelastchar (')'); /* undo the RBR */
	    sdfdodelayedcopy (0);	  /* flush */
	}
	checkthatalllibhasbeenwritten (thingsstilltobewritten);
	if (sdfcopytheinput) sdfpushcharoncopystream (')'); /* redo the RBR */
    }

    if (sdfstuff & SDFLIBBODY) sdfcopytheinput = TRUE; /* resume copying */
    if (skipthisthingforindex == 1) skipthisthingforindex = 0;
    if (makeindex)
    {
	if (sdffunislastthinginlib == SDF_SEEN_NOTHING_YET ||
	    sdffunislastthinginlib == SDF_LETS_KEEP_IT_LIKE_THIS)
	    /* The good news: we can stop parsing this lib as soon as we encounter a Function */
	    thislibtab->info.state |= SDFFASTPARSE;
    }

    if (!makeindex && sdfparseonelib) /* Parse only one lib */
			    YYACCEPT; /* Quit the parser */
    $$ = libraryptr;
}
               ;

_Library       :
{
    debug ("empty _Library")
    $$ = libraryptr;
}
               | _Library Technology
{
    debug ("_Library Technology")
    if (!junklib && libraryptr->technology)
	sdfreport (Warning, "%s\nLibrary '%s' has more than one Technology section,\n"
	    "all but first ignored. (Line %d)", FilNam, libraryptr->name, yylineno);
    else
	libraryptr->technology = $2;
    $$ = libraryptr;
}
	       | _Library Alias
{
    if (makeindex && $2) sdfmakelibalias ($2, thislibtab->name);
    $$ = libraryptr;
}
               | _Library LibStatus
{
    debug ("_Library Status")
    if (!junklib && libraryptr->status)
	sdfreport (Warning, "%s\nLibrary '%s' has more than one Status section,\n"
	    "all but first ignored. (Line %d)", FilNam, libraryptr->name, yylineno);
    else
	libraryptr->status = $2;
    $$ = libraryptr;
}

               | _Library Function
{
    debug ("_Library Function")
    if ($2) { /* if not junkfun then link in funlist */
	($2)->next = libraryptr->function;
	libraryptr->function = $2;
    }
    $$ = libraryptr;
}
               | _Library Comment
{
    debug ("_Library Comment")
    $$ = libraryptr;
}
               | _Library UserData
{
    debug ("_Library UserData")
    $$ = libraryptr;
}
               ;


Technology     : LBR TECHNOLOGY
{
    if (sdfstuff & SDFLIBBODY) {
	sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
	if (sdfwrite & SDFLIBBODY) {
	    sdfdodelayedcopy (0); /* flush */
	    fprintf (sdfcopystream, "\n(Technology \"%s\")\n", sdfwritethislib->technology);
	}
    }
}
                 STRNG _Stuff RBR
{
    if (sdfstuff & SDFLIBBODY) sdfcopytheinput = TRUE; /* resume copying */
    $$ = junklib ? NULL : canonicstring ($4);
}
;

/*
 * library/function
 */
Function       : LBR FUNCTIONTOKEN STRNG
{
    int wearecopyingtheinput = sdfcopytheinput;
    int thingsstilltobewritten;

    debug ("LBR FUNCTIONTOKEN STRNG");
    sdfhavethisthing &= ~(SDFFUNALL | SDF_X_FUNALIAS);
    sdfhavethisthing |= SDFFUNBODY;

    if (!makeindex && sdffunislastthinginlib)
    { /* we're gonna abort this parse thing, seems like we've seen enough... */
	if (sdfstuff & SDFFUNBODY)
	sdfabortcopy (SDFDISCARDSPACES);
	if ((thingsstilltobewritten = THINGS (SDFLIBALL, SDF_X_LIBALIAS)))
	{
	    if (wearecopyingtheinput) sdfdodelayedcopy (0); /* flush */
	    checkthatalllibhasbeenwritten (thingsstilltobewritten);
	}
	if (wearecopyingtheinput)
	{
	    /* unfortunately we're also gonna miss the final ')', better simulate one... */
	    sdfcopytheinput = TRUE;
	    sdfpushcharoncopystream (')');
	}
	YYACCEPT;
    }

    if (makeindex && !skipthisthingforindex)
    {
	junkfunction.name = canonicstring ($3);
	if (existsfun (junkfunction.name, thislibtab->name))
	{
	    sdfreport (Error, "Makeindex: multiple functions (%s(%s))\n"
		"           file \"%s\"\n"
		"           file \"%s\"",
		$3, thislibtab->name,
		sdffileinfo[(int)sdfcurrentfileidx].name,
		sdffileinfo[(int)thisfuntab->info.file].name);
	    skipthisthingforindex = 2; /* 2 means: skip this fun for index */
	    return (1);
	}
	else
	{
	    info.what = 0;
	    info.state = 0;
	    info.file = sdfcurrentfileidx;
	    info.fpos = sdfleftparenthesis; /* position of most recent '(' */
	    addfuntohashtable (&junkfunction, thislibtab, &info);
	}
	forgetstring (junkfunction.name);
	sdfcirislastthinginfun = SDF_SEEN_NOTHING_YET; /* initialize */

	if (sdffunislastthinginlib == SDF_SEEN_NOTHING_YET ||
	    sdffunislastthinginlib == SDF_LETS_KEEP_IT_LIKE_THIS)
	    sdffunislastthinginlib = SDF_LETS_KEEP_IT_LIKE_THIS;
    }

    if (sdfstuff & SDFFUNBODY) sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
    if (sdfwhat & SDFFUNBODY) {
	junkfun = FALSE;
	NewFunction (functionptr);
	functionptr->name = canonicstring ($3);
	if (sdfverbose) fprintf (stderr, "......(Function \"%s\")\n", functionptr->name);
    }
    else {
	junkfun = TRUE;
	functionptr = &junkfunction;
	functionptr->name = NULL;
    }
    functionptr->library = libraryptr;
}
                 _Function RBR
{
    int thingsstilltobewritten;

    if ((thingsstilltobewritten = THINGS (SDFFUNALL, SDF_X_FUNALIAS)))
    {
	if (sdfcopytheinput)
	{ /* First, flush whatever is in the copy buffer up till RBR */
	    sdfuncopysincelastchar (')'); /* undo the RBR */
	    sdfdodelayedcopy (0);	  /* flush */
	}
	checkthatallfunhasbeenwritten (thingsstilltobewritten);
	if (sdfcopytheinput) sdfpushcharoncopystream (')'); /* redo the RBR */
    }

    if (makeindex)
    {
	if (sdfcirislastthinginfun == SDF_SEEN_NOTHING_YET ||
	    sdfcirislastthinginfun == SDF_LETS_KEEP_IT_LIKE_THIS)
	    /* The good news: we can stop parsing this fun as soon as we encounter a Circuit */
	    thisfuntab->info.state |= SDFFASTPARSE;
    }

    if (skipthisthingforindex == 2) skipthisthingforindex = 0;

    if (sdfstuff & SDFFUNBODY) sdfcopytheinput = TRUE; /* resume copying */

    if (junkfun) $$ = NULL;
    else { junkfun = TRUE; $$ = functionptr; }

    if (!makeindex && sdfparseonefun) YYACCEPT; /* quit the parser */
}
               ;

_Function      :
{
    debug ("empty _Function")
    $$ = functionptr;
}
	       | _Function Alias
{
    if (makeindex && $2) sdfmakefunalias ($2, thisfuntab->name, thisfuntab->library->name);
}
               | _Function FunctionType
{
    debug ("_Function FunctionType")
    if (!junkfun && functionptr->type)
	sdfreport (Warning, "%s\nFunction '%s' has more than one FunctionType section,\n"
	    "all but first ignored. (Line %d)", FilNam, functionptr->name, yylineno);
    else
	functionptr->type = $2;
    $$ = functionptr;
}
               | _Function CircuitImpl
{
    debug ("_Function CircuitImpl");
    if ($2) { /* link circuit in front of the function's circuit list */
	($2)->next = functionptr->circuit;
	functionptr->circuit = $2;
    }
    $$ = functionptr;
}
               | _Function FunStatus
{
    debug ("_Function Status")
    if (!junkfun && functionptr->status)
	sdfreport (Warning, "%s\nFunction '%s' has more than one Status section,\n"
	    "all but first ignored. (Line %d)", FilNam, functionptr->name, yylineno);
    else
	functionptr->status = $2;
    $$ = functionptr;
}
               | _Function Comment
{
    $$ = functionptr;
}
               | _Function UserData
{
    $$ = functionptr;
}
               ;

/*
 * library/function/type
 * Should specify the class of function: elementary boolean, complex, etc.
 */
FunctionType   : LBR FUNCTIONTYPE
{
    if (sdfstuff & SDFFUNTYPE) {
	sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
	if (sdfwrite & SDFFUNTYPE) {
	    sdfdodelayedcopy (0); /* flush */
	    dump_funtype (sdfcopystream, sdfwritethisfun->type);
	}
    }
}
                 STRNG RBR
{
    sdfhavethisthing |= SDFFUNTYPE;
    if (sdfstuff & SDFFUNTYPE) sdfcopytheinput = TRUE; /* resume copying */
    $$ = junkfun ? NULL : canonicstring ($4);
}
               ;

/*
 * Library/Function/CircuitImpl
 * Contains the Logical (circuit) Implementation (= netlist) of a
 * boolean function
 */
CircuitImpl    : LBR CIRCUITTOKEN STRNG
{
    int wearecopyingtheinput = sdfcopytheinput;
    int thingsstilltobewritten;

    debug ("LBR CIRCUITTOKEN STRNG");
    sdfhavethisthing &= ~(SDFCIRALL | SDF_X_CIRALIAS);
    sdfhavethisthing |= SDFCIRBODY;

    if (!makeindex && sdfcirislastthinginfun)
    { /* we're gonna abort this parse thing, seems like we've seen enough... */
	if (sdfstuff & SDFCIRBODY) sdfabortcopy (SDFDISCARDSPACES);
	if ((thingsstilltobewritten = THINGS (SDFFUNALL, SDF_X_FUNALIAS)))
	{
	    if (wearecopyingtheinput) sdfdodelayedcopy (0); /* flush */
	    checkthatallfunhasbeenwritten (thingsstilltobewritten);
	}
	if (wearecopyingtheinput)
	{
	    /* unfortunately we're also gonna miss the final ')', better simulate one... */
	    sdfcopytheinput = TRUE;
	    sdfpushcharoncopystream (')');
	}
	YYACCEPT;
    }

    if (makeindex && !skipthisthingforindex)
    {
	junkcircuit.name = canonicstring ($3);
	if (existscir (junkcircuit.name, thisfuntab->name, thislibtab->name))
	{
	    sdfreport (Error, "Makeindex: multiple circuits (%s(%s(%s)))\n"
		"           file \"%s\"\n"
		"           file \"%s\"",
		$3, thisfuntab->name, thislibtab->name,
		sdffileinfo[(int)sdfcurrentfileidx].name,
		sdffileinfo[(int)thiscirtab->info.file].name);
	    skipthisthingforindex = 3; /* 3 means: skip this cir for index */
	    return (1);
	}
	else
	{
	    info.what = 0;
	    info.state = 0;
	    info.file = sdfcurrentfileidx;
	    info.fpos = sdfleftparenthesis; /* position of most recent '(' */
	    addcirtohashtable (&junkcircuit, thisfuntab, &info);
	}
	forgetstring (junkcircuit.name);
	sdflayislastthingincir = SDF_SEEN_NOTHING_YET; /* initialize */

	if (sdfcirislastthinginfun == SDF_SEEN_NOTHING_YET ||
	    sdfcirislastthinginfun == SDF_LETS_KEEP_IT_LIKE_THIS)
	    sdfcirislastthinginfun = SDF_LETS_KEEP_IT_LIKE_THIS;
    }

    if (sdfstuff & SDFCIRBODY) sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
    if (sdfwhat & SDFCIRBODY)
    {
	junkcir = FALSE;
	NewCircuit (circuitptr);
	circuitptr->name = canonicstring ($3);
	if (sdfverbose) fprintf (stderr, ".........(Circuit \"%s\")\n", circuitptr->name);
    }
    else
    {
	junkcir = TRUE;
	circuitptr = &junkcircuit;
	circuitptr->name = NULL;
    }
    circuitptr->function = functionptr;
}
                 _CircuitImpl RBR
{
    int thingsstilltobewritten;

    if ((thingsstilltobewritten = THINGS (SDFCIRALL, SDF_X_CIRALIAS)))
    {
	if (sdfcopytheinput)
	{ /* First, flush whatever is in the copy buffer up till RBR */
	sdfuncopysincelastchar (')'); /* undo the RBR */
	sdfdodelayedcopy (0);	  /* flush */
	}
	checkthatallcirhasbeenwritten (thingsstilltobewritten);
	if (sdfcopytheinput)
	sdfpushcharoncopystream (')'); /* redo the RBR */
    }

    if (makeindex)
    {
	if (sdflayislastthingincir == SDF_SEEN_NOTHING_YET ||
	    sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
	    /* The good news: we can stop parsing this cir as soon as we encounter a Layout */
	    thiscirtab->info.state |= SDFFASTPARSE;
    }

    if (skipthisthingforindex == 3) skipthisthingforindex = 0;
    if (sdfstuff & SDFCIRBODY) sdfcopytheinput = TRUE; /* resume copying */

    if (junkcir) $$ = NULL;
    else { junkcir = TRUE; $$ = circuitptr; }

    if (!makeindex && sdfparseonecir) YYACCEPT; /* quit the parser */
}
               ;

_CircuitImpl   :
{
    debug ("empty _CircuitImpl")
    $$ = circuitptr;
}
	       | _CircuitImpl Alias
{
    if (makeindex && $2) sdfmakeciralias ($2, thiscirtab->name,
					thiscirtab->function->name,
					thiscirtab->function->library->name);
}
               | _CircuitImpl CirPortList
{
    debug ("_CircuitImpl CirPortList")
    if (!junkcir && circuitptr->cirport)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one CirPortList section,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->cirport = $2;
    $$ = circuitptr;
}
/*             | _CircuitImpl Permutable     not implemented, but could be useful */
               | _CircuitImpl Attribute
{
    debug ("_CircuitImpl Attribute")
    if (!junkcir && circuitptr->attribute)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one Attribute section,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->attribute = $2;
    $$ = circuitptr;
}
               | _CircuitImpl CirInstList
{
    debug ("_CircuitImpl CirInstList")
    if (!junkcir && circuitptr->cirinst)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one CirInstList section,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->cirinst = $2;
    $$ = circuitptr;
}
               | _CircuitImpl NetList
{
    debug ("_CircuitImpl NetList")
    if (!junkcir && circuitptr->netlist)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one NetList,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->netlist = $2;
    $$ = circuitptr;
}
               | _CircuitImpl BusList
{
    debug ("_CircuitImpl BusList")
    if (!junkcir && circuitptr->buslist)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one BusList,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->buslist = $2;
    $$ = circuitptr;
}
               | _CircuitImpl LayoutImpl
{
    debug ("_CircuitImpl LayoutImpl")
    if ($2) { /* if not junklay... */
	($2)->next = circuitptr->layout;
	circuitptr->layout = $2;
    }
    $$ = circuitptr;
}
				  /* IK, .. and our new timing model */
               | _CircuitImpl Timing
{
    debug ("_CircuitImpl Timing");
    if ($2) { /* if not junktm... */
	($2)->next = circuitptr->timing;
	circuitptr->timing = $2;
    }
    $$ = circuitptr;
}
               | _CircuitImpl CirStatus
{
    debug ("_CircuitImpl Status")
    if (!junkcir && circuitptr->status)
	sdfreport (Warning, "%s\nCircuit '%s' has more than one Status section,\n"
	    "all but first ignored. (Line %d)", FilNam, circuitptr->name, yylineno);
    else
	circuitptr->status = $2;
    $$ = circuitptr;
}
               | _CircuitImpl Comment
{
    $$ = circuitptr;
}
               | _CircuitImpl UserData
{
    $$ = circuitptr;
}
               ;

/*
 * Library/Function/LogicImpl/CirInterface/CirPort
 * Logical terminal description
 */
CirPortList    : LBR CIRCUITPORTLIST
{
    debug ("LBR CIRCUITPORTLIST");
    sdfhavethisthing |= SDFCIRPORT;

    if (makeindex) {
	if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
	    /* Will not be able to efficiently parse this circuit */
	    sdflayislastthingincir = SDF_LOST_CAUSE;
    }

    if (sdfstuff & SDFCIRPORT) {
	sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
	if (sdfwrite & SDFCIRPORT && sdfwritethiscir->cirport) {
	    sdfdodelayedcopy (0); /* flush */
	    dump_cirportlist (sdfcopystream, sdfwritethiscir->cirport);
	}
    }
    cirportlistptr = NULL;
}
                _CirPortList RBR
{
    debug ("_CirPortList RBR")
    if (sdfstuff & SDFCIRPORT) sdfcopytheinput = TRUE; /* resume copying */
    $$ = (sdfwhat & SDFCIRPORT) ? cirportlistptr : NULL;
}
               ;

_CirPortList   :
{
    debug ("empty _CirPortList")
    $$ = cirportlistptr;
}
               | _CirPortList CirPort
{
    debug ("_CirPortList CirPort")
    ($2)->next = cirportlistptr;
    $$ = cirportlistptr = ($2); /* link CirPort in front of CirPortList and return */
}
               ;

CirPort        : LBR CIRCUITPORT STRNG
{
    debug ("LBR CIRCUITPORT STRNG")
    if (sdfwhat & SDFCIRPORT) {
	NewCirport (cirportptr);
	cirportptr->name = canonicstring ($3);
	cirportptr->net = NULL;
#ifdef SDF_PORT_DIRECTIONS
	cirportptr->direction = SDF_PORT_UNKNOWN;
#endif
    }
    else
	cirportptr = &junkcirport;
}
                 _CirPort RBR
{
    debug ("_CirPort RBR")
    $$ = cirportptr;
}
               ;

_CirPort       :
{
    debug ("empty _CirPort");
}
               | _CirPort Comment
{
    debug ("_CirPort Comment");
}
               | _CirPort UserData
{
    debug ("_CirPort UserData");
}
               | _CirPort CirportDirection
               ;

CirportDirection : LBR DIRECTION STRNG RBR
{
#ifdef SDF_PORT_DIRECTIONS
    if (sdfstrcasecmp ($3, "in") == 0)
	cirportptr->direction = SDF_PORT_IN;
    else if (sdfstrcasecmp ($3, "out") == 0)
	cirportptr->direction = SDF_PORT_OUT;
    else if (sdfstrcasecmp ($3, "inout") == 0)
	cirportptr->direction = SDF_PORT_INOUT;
    else {
	sdfreport (Error, "Illegal Direction \"%s\"", $3);
	cirportptr->direction = SDF_PORT_UNKNOWN;
    }
#endif
}
;

BusList        : LBR BUSLISTTOKEN
{
    debug ("LBR BUSLISTTOKEN")
    sdfhavethisthing |= SDFCIRBUS;

    if (makeindex) {
	if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
	    /* Will not be able to efficiently parse this circuit */
	    sdflayislastthingincir = SDF_LOST_CAUSE;
    }
    if (sdfstuff & SDFCIRBUS) {
	sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
	if (sdfwrite & SDFCIRBUS && sdfwritethiscir->buslist) {
	    sdfdodelayedcopy (0);	  /* flush */
	    dump_buslist (sdfcopystream, sdfwritethiscir->buslist);
	}
    }
    buslistptr = NULL;
}
                 _BusList RBR
{
    debug ("_BusList RBR")
    if (sdfstuff & SDFCIRBUS) sdfcopytheinput = TRUE; /* resume copying */
    $$ = $4; /* $4 is return value of _BusList */
}
               ;


_BusList       :
{
    debug ("empty _BusList")
    $$ = buslistptr;
}
               | _BusList Bus
{
    debug ("_BusList Bus")
    ($2)->next = buslistptr;
    $$ = buslistptr = $2;
}
               ;


Bus            : LBR BUSTOKEN STRNG
{
    BUSPTR busptr;
    debug ("LBR BUSTOKEN STRNG")
    netreflistptr = NULL;
    NewBus (busptr);
    busptr->name = canonicstring ($3);
    $<bus>$ = busptr; /* <bus> is the type of the return value */
}
                 NetRefList RBR
{
    debug ("NetRefList RBR")
    ($<bus>4)->netref = netreflistptr;  /* horrible syntax, isn't it? */
    $$ = $<bus>4;
}
               ;

NetRefList     :
{
    debug ("empty NetRefList")
    $$ = netreflistptr;
}
               | NetRefList NetRef
{
    debug ("NetRefList NetRef")
    ($2)->next = netreflistptr;
    $$ = netreflistptr = $2; /* link NetRef in front of NetRefList and return */
}
               ;

NetRef         : LBR NETREFTOKEN STRNG
{
    NETREFPTR netref;
    debug ("LBR NETREFTOKEN STRNG NetPortRefInBus RBR")
    NewNetRef (netref);
    /* NASTY HACK, we solve this reference later: */
    netref->net = (NETPTR)canonicstring ($3);
    $<netref>$ = netref;
}
                 NetPortRefInBus RBR
{
    ($<netref>4)->cirport = NULL;
    $$ = $<netref>4;
}
;

NetPortRefInBus : /* not supported stuff */
{
    $$ = NULL;
}
;

/*
 * Library/Function/LogicImpl/CirContents/CirInstance
 * Model-call construct
 * e.g.  (cirinstance port_123 (circellref nand2 (cirlibref bieb_2)))
 * Maybe the references should be ID numbers instead of strings
 */
CirInstList    : LBR CIRCUITINSTANCELIST
{
debug ("LBR CIRCUITINSTANCELIST");
sdfhavethisthing |= SDFCIRINST;
if (sdfstuff & SDFCIRINST)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFCIRINST && sdfwritethiscir->cirinst)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_cirinstlist (sdfcopystream, sdfwritethiscir);
      }
   }
cirinstlistptr = NULL;
}
                 _CirInstList RBR
{
debug ("_CirInstList RBR")
if (sdfstuff & SDFCIRINST) sdfcopytheinput = TRUE; /* resume copying */
$$ = (sdfwhat & SDFCIRINST) ? cirinstlistptr : NULL;
}
               ;

_CirInstList   :
{
debug ("empty _CirInstList")
$$ = cirinstlistptr;
}
               | _CirInstList CirInstance
{
debug ("_CirInstList CirInstance")
($2)->next = cirinstlistptr;
cirinstlistptr = $2; /* link CirInstance in front of the CirInstList */
$$ = cirinstlistptr;
}
               ;

CirInstance    : LBR CIRCUITINSTANCE STRNG
{
debug ("LBR CIRCUITINSTANCE STRNG")
if (makeindex)
   {
   if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* Will not be able to efficiently parse this circuit */
      sdflayislastthingincir = SDF_LOST_CAUSE;
   }
if (sdfwhat & SDFCIRINST)
   {
   NewCirinst (cirinstptr);
   cirinstptr->name = canonicstring ($3);
   cirinstptr->circuit = HACK; /* Dit worden twee hele NARE HACKS... (see below) */
   cirinstptr->curcirc = HACK;
   }
else
   cirinstptr = &junkcirinst;
}
                 _CirInstRef OptionalString RBR
{
debug ("_CirInstRef RBR")
cirinstptr->attribute = $6;	  /* OptionalString is the attribute string */
$$ = cirinstptr;
}
;

OptionalString : /* empty */
{
$$ = (STRING)NULL;
}
               | STRNG
{
$$ = canonicstring ($1);
}
;

_CirInstRef    : CirCellRef
               | _CirInstRef Comment
               | _CirInstRef UserData
               ;

CirCellRef     : LBR CIRCUITCELLREF STRNG _Stuff CirFunRef RBR
{
/* This is where we face a problem: we cannot solve the reference
 * because it may be that we have not processed the referenced cell.
 * We cannot even be sure that the referenced cell is in the current
 * library. The solution adapted here is to temporarly store the name
 * of the referenced cell in cirinstptr->circuit (with a nasty type
 * cast) and the name of the referenced library in cirinstptr->curcirc
 * (also requires a type cast). After the parser has finished with
 * this Seadif file, another function solves all the references. This
 * function may need to call the parser again in order to solve
 * references to libraries not present in this Seadif file, etc, etc.
 */
debug ("LBR CIRCUITCELLREF STRNG RBR")
if (sdfwhat & SDFCIRINST)
   {
   cirinstptr->circuit = (CIRCUITPTR)canonicstring ($3);
   cirinstptr->curcirc = (CIRCUITPTR)$5; /* namelist: FunRef,LibRef */
   }
$$ = cirinstptr;
}
               ;

CirFunRef      : LBR CIRCUITFUNREF STRNG _Stuff CirLibRef RBR /* Lib implicit */
{
if (sdfwhat & SDFCIRINST)
   {
   NAMELISTPTR p;
   NewNamelist (p);
   p->name = canonicstring ($3);
   p->next = $5;
   $$ = p;
   }
else
   $$ = (NAMELISTPTR)NULL;
}
               |		  /* empty */
{
$$ = (NAMELISTPTR)NULL;
}
               ;


CirLibRef      : LBR CIRCUITLIBREF STRNG _Stuff RBR
{
if (sdfwhat & SDFCIRINST)
   {
   NAMELISTPTR p;
   NewNamelist (p);
   p->name = canonicstring ($3);
   p->next = NULL;
   $$ = p;
   }
else
   $$ = (NAMELISTPTR)NULL;
}
               |		  /* empty */
{
$$ = (NAMELISTPTR)NULL;
}
               ;
/*
 * Library/Function/LogicImpl/CirContents/CirNet
 * Net construct
 * e.g. (cirnet phi1 (joined (cirportref clock_in (cirinstref port_123))
 *                           (cirportref phi_in)                         nothing specified implies father cell
 *                           (cirportref ck_in (cirinstref port_233))))
 */
NetList        : LBR NETLIST
{
debug ("LBR NETLIST");
sdfhavethisthing |= SDFCIRNETLIST;
if (makeindex)
   {
   if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* Will not be able to efficiently parse this circuit */
      sdflayislastthingincir = SDF_LOST_CAUSE;
   }
if (sdfstuff & SDFCIRNETLIST)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFCIRNETLIST && sdfwritethiscir->netlist)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_netlist (sdfcopystream, sdfwritethiscir->netlist);
      }
   }
netlistptr = NULL;
}
                 _NetList RBR
{
debug ("_NetList RBR")
if (sdfstuff & SDFCIRNETLIST) sdfcopytheinput = TRUE; /* resume copying */
$$ = (sdfwhat & SDFCIRNETLIST) ? netlistptr : NULL;
}
               ;

_NetList       :
{
debug ("empty _NetList")
$$ = netlistptr;
}
               | _NetList Net
{
debug ("_NetList Net")
($2)->next = netlistptr;
netlistptr = $2; /* link Net in front of the NetList */
$$ = netlistptr;
}
               ;

Net            : LBR NETTOKEN STRNG
{
debug ("LBR NETTOKEN STRNG")
if (sdfwhat & SDFCIRNETLIST)
   {
   NewNet (netptr);
   netptr->name = canonicstring ($3);
   netptr->circuit = circuitptr;
   cirportreflistptr = NULL;
   }
else
   netptr = &junknet;
}
                 _Net RBR
{
debug ("_Net RBR")
netptr->terminals = cirportreflistptr;
$$ = netptr;
}
               ;

_Net           : Joined
{debug ("Joined");}
               | _Net Comment
               | _Net UserData
               ;

Joined         : LBR JOINED _Joined RBR
{
debug ("LBR JOINED _Joined RBR");
}
               ;

_Joined        : /* empty */
{
debug ("empty _Joined");
}
               | _Joined NetPortRef
{
debug ("_Joined NetPortRef")
($2)->next = cirportreflistptr;
cirportreflistptr = $2;
}
               | _Joined UserData
{
debug ("_Joined UserData");
}
               ;

NetPortRef     : LBR NETPORTREF
{
debug ("LBR NETPORTREF")
if (sdfwhat & SDFCIRNETLIST)
   {
   NewCirportref (cirportrefptr);
   cirportrefptr->cirport = HACK; /* weer twee nare hacks! */
   cirportrefptr->cirinst = HACK;
   cirportrefptr->net = netptr;
   }
else
   cirportrefptr = &junkcirportref;
}
                _NetPortRef RBR
{
debug ("_NetPortRef RBR")
$$ = cirportrefptr;
}
               ;

_NetPortRef    : STRNG
{
debug ("STRNG");
if (sdfwhat & SDFCIRNETLIST)
   {
   /* Nasty type cast, see also CirCellRef (above). */
   cirportrefptr->cirport = (CIRPORTPTR)canonicstring ($1);
   cirportrefptr->cirinst = NULL; /* No NetInstRef: must refer to circuit's own terminals. */
   }
$$ = cirportrefptr;
}
               | STRNG NetInstRef
{
debug ("STRNG NetInstRef")
if (sdfwhat & SDFCIRNETLIST)
   {
   /* Nasty type cast, see also CirCellRef (above). */
   cirportrefptr->cirport = (CIRPORTPTR)canonicstring ($1);
   /* NetInstRef returns the name of the circuit instance to which this cirport belongs. */
   cirportrefptr->cirinst = (CIRINSTPTR)$2;
   }
$$ = cirportrefptr;
}
               ;

NetInstRef     : _NetInstRef
{
debug ("_NetInstRef")
$$ = $1;
}
               | NetInstRef Comment
{
debug ("NetInstRef Comment")
$$ = $1;
}
               | NetInstRef UserData
{
debug ("NetInstRef UserData")
$$ = $1;
}
               ;

_NetInstRef    : LBR NETINSTREF STRNG RBR
{
debug ("LBR NETINSTREF STRNG RBR")
$$ = (sdfwhat & SDFCIRNETLIST) ? canonicstring ($3) : NULL;
}
               ;

LayoutImpl     : LBR LAYOUTTOKEN STRNG
{
int wearecopyingtheinput = sdfcopytheinput;
int thingsstilltobewritten;
debug ("LBR LAYOUTTOKEN LayoutName");
sdfhavethisthing &= ~(SDFLAYALL | SDF_X_LAYALIAS);
sdfhavethisthing |= SDFLAYBODY;
if (!makeindex && sdflayislastthingincir)
   { /* we're gonna abort this parse thing, seems like we've seen enough... */
   if (sdfstuff & SDFLAYBODY)
      sdfabortcopy (SDFDISCARDSPACES);
   if ((thingsstilltobewritten = THINGS (SDFCIRALL, SDF_X_CIRALIAS)))
      {
      if (wearecopyingtheinput)
	 sdfdodelayedcopy (0);	  /* flush */
      checkthatallcirhasbeenwritten (thingsstilltobewritten);
      }
   if (wearecopyingtheinput)
      {
      /* unfortunately we're also gonna miss the final ')', better simulate one... */
      sdfcopytheinput = TRUE;
      sdfpushcharoncopystream (')');
      }
   YYACCEPT;
   }
if (makeindex && !skipthisthingforindex)
   {
   junklayout.name = canonicstring ($3);
   if (existslay ($3, thiscirtab->name, thisfuntab->name, thislibtab->name))
      {
      sdfreport (Error, "Makeindex: multiple layouts (%s(%s(%s(%s))))\n"
		"           file \"%s\"\n"
		"           file \"%s\"",
		$3, thiscirtab->name, thisfuntab->name, thislibtab->name,
		sdffileinfo[(int)sdfcurrentfileidx].name,
		sdffileinfo[(int)thislaytab->info.file].name);
      skipthisthingforindex = 4; /* 4 means: skip this lay for index */
      return (1);
      }
   else
      {
      info.what = 0;
      info.state = 0;
      info.file = sdfcurrentfileidx;
      info.fpos = sdfleftparenthesis; /* position of most recent '(' */
      addlaytohashtable (&junklayout, thiscirtab, &info);
      }
   forgetstring (junklayout.name);
   }
if (sdfstuff & SDFLAYBODY)
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
if (sdfwhat & SDFLAYBODY)
   {
   junklay = FALSE;
   NewLayout (layoutptr);
   layoutptr->name = canonicstring ($3);
   if (sdfverbose) fprintf (stderr, "............(Layout \"%s\")\n", layoutptr->name);
   }
else
   {
   junklay = TRUE;
   layoutptr = &junklayout;
   layoutptr->name = NULL;
   }
layoutptr->circuit = circuitptr;
}
                 _LayoutImpl RBR
{
int thingsstilltobewritten;
if ((thingsstilltobewritten = THINGS (SDFLAYALL, SDF_X_LAYALIAS)))
   {
   /* Actually, this piece of code would be shorter if we could
    * execute it after the rule _LayoutImpl and before RBR. However
    * yacc only decides that _LayoutImpl has finished when it encounters
    * RBR and consequently the ideal moment for this code to be executed
    * can only be determined when it is already too late...
    */
   if (sdfcopytheinput)
      { /* First, flush whatever is in the copy buffer up till RBR */
      sdfuncopysincelastchar (')'); /* undo the RBR */
      sdfdodelayedcopy (0);	  /* flush */
      }
   checkthatalllayhasbeenwritten (thingsstilltobewritten);
   if (sdfcopytheinput)
      sdfpushcharoncopystream (')'); /* redo the RBR */
   }
if (skipthisthingforindex == 4)
   skipthisthingforindex = 0;
if (sdfstuff & SDFLAYBODY)
   sdfcopytheinput = TRUE;	  /* resume copying */
if (junklay) $$ = NULL;
else
   {
   junklay = TRUE;
   $$ = layoutptr;
   }
if (!makeindex && sdfparseonelay)
   YYACCEPT;			  /* quit the parser */
}
               ;

_LayoutImpl    :
{
debug ("empty _LayoutImpl")
$$ = layoutptr;
}
	       | _LayoutImpl Alias
{
if (makeindex && $2) sdfmakelayalias ($2, thislaytab->name,
					   thislaytab->circuit->name,
					   thislaytab->circuit->function->name,
					   thislaytab->circuit->function->library->name);
}
               | _LayoutImpl Attribute /* this currently does nothing....!!! */
{
debug ("_LayoutImpl Attribute")
if (!junklay && layoutptr->layport)
   sdfreport (Warning, "%s\nLayout '%s' has more than one Attribute section,\n"
	     "all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
/* NOT SUPPORTED FOR LAYOUT: layoutptr->attribute = canonicstring ($2); */
$$ = layoutptr;
}
               | _LayoutImpl LayPortList
{
debug ("_LayoutImpl LayPortList")
if (!junklay && layoutptr->layport)
   sdfreport (Warning, "%s\nLayout '%s' has more than one LayoutPortList section,\n"
	     "all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   layoutptr->layport = sdfwhat&SDFLAYPORT ? $2 : NULL;
$$ = layoutptr;
}
               | _LayoutImpl LayLabelList
{
debug ("_LayoutImpl LayLabelList")
if (!junklay && layoutptr->laylabel)
   sdfreport (Warning, "%s\nLayout '%s' has more than one LayoutLabelList section,\n"
	     "all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   layoutptr->laylabel = sdfwhat&SDFLAYLABEL ? $2 : NULL;
$$ = layoutptr;
}
               | _LayoutImpl LayBoundingBox
{
debug ("_LayoutImpl LayBoundingBox")
if (!junklay && (layoutptr->bbx[HOR] != 0 || layoutptr->bbx[VER] != 0))
   sdfreport (Warning, "%s\nLayout '%s' has more than one Layoutbbx section,\n"
	     "all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   {
   layoutptr->bbx[HOR] = ($2).hor;
   layoutptr->bbx[VER] = ($2).ver;
   }
$$ = layoutptr;
}
               | _LayoutImpl LayOffset
{
debug ("_LayoutImpl LayOffset")
if (!junklay && (layoutptr->off[HOR] != 0 || layoutptr->off[VER] != 0))
   sdfreport (Warning, "%s\nLayout '%s' has more than one LayoutOffset section,\n"
	     "all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   {
   layoutptr->off[HOR] = ($2).hor;
   layoutptr->off[VER] = ($2).ver;
   }
$$ = layoutptr;
}
               | _LayoutImpl LayInstList
{
debug ("_LayoutImpl LayInstList")
if (!junklay && layoutptr->slice)
   {
   sdfreport (Warning, "%s\nLayout '%s' has more than one LayoutInstanceList"
	" or LayoutSlice section,\n"
	"all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
   }
else if (sdfwhat & SDFLAYSLICE)
   {
   layoutptr->slice = $2;
   /* This one calls mfree which is a **DISASTER** if we did not mnew() anything... */
   slicecleanup (&layoutptr->slice);
   }
$$ = layoutptr;
}
               | _LayoutImpl LaySlice
{
debug ("_LayoutImpl LaySlice")
if (!junklay && layoutptr->slice)
   {
   sdfreport (Warning, "%s\nLayout '%s' has more than one LayoutInstanceList"
	" or LayoutSlice section,\n"
	"all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
   }
else if (sdfwhat & SDFLAYSLICE)
   {
   layoutptr->slice = $2;
   /* This one calls mfree which is a **DISASTER** if we did not mnew() anything... */
   slicecleanup (&layoutptr->slice);
   }
$$ = layoutptr;
}
               | _LayoutImpl WireList
{
debug ("_LayoutImpl WireList")
if (!junklay && layoutptr->wire)
   sdfreport (Warning, "%s\nLayout '%s' has more than one WireList section,\n"
	"all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   layoutptr->wire = sdfwhat&SDFLAYWIRE ? $2 : NULL;
$$ = layoutptr;
}
               | _LayoutImpl LayStatus
{
debug ("_LayoutImpl Status")
if (!junklay && layoutptr->status)
   sdfreport (Warning, "%s\nLayout '%s' has more than one Status section,\n"
	"all but first ignored. (Line %d)", FilNam, layoutptr->name, yylineno);
else
   layoutptr->status = $2;
$$ = layoutptr;
}
               | _LayoutImpl Comment
{
debug ("_LayoutImpl Comment")
$$ = $1;
}
               | _LayoutImpl UserData
{
debug ("_LayoutImpl UserData")
$$ = $1;
}
               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/LayInterface/LayPortList
 * Port/terminals on layout level
 */
LayPortList    : LBR LAYOUTPORTLIST
{
debug ("LBR LAYOUTPORTLIST");
sdfhavethisthing |= SDFLAYPORT;
if (sdfstuff & SDFLAYPORT)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYPORT && sdfwritethislay->layport)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_layportlist (sdfcopystream, sdfwritethislay->layport);
      }
   }
layportlistptr = NULL;
}
                 _LayPortList RBR
{
debug ("_LayPortList RBR")
if (sdfstuff & SDFLAYPORT) sdfcopytheinput = TRUE; /* resume copying */
$$ = (sdfwhat & SDFLAYPORT) ? layportlistptr : NULL;
}
               ;

_LayPortList   :
{
debug ("empty _LayPortList")
$$ = layportlistptr;
}
               | _LayPortList LayPort
{
debug ("_LayPortList LayPort")
($2)->next = layportlistptr;
layportlistptr = $2;
$$ = layportlistptr;
}
               ;

LayPort        : LBR LAYOUTPORT STRNG
{
debug ("LBR LAYOUTPORT STRNG");
if (sdfwhat & SDFLAYPORT)
   {
   NewLayport (layportptr);
   layportptr->cirport = HACK; /* Just to make clear that this one is going to be misused... */
   /* Nasty HACK, as usual. We solve this symbolic reference later. */
   layportptr->cirport = (CIRPORTPTR)canonicstring ($3);
   }
else
   {
   layportptr = &junklayport;
   /* avoid error message 'more than one..' */
   layportptr->pos[HOR] = layportptr->pos[VER] = layportptr->layer = 0;
   }
}
                 _LayPort RBR
{
debug ("_LayPort RBR")
$$ = layportptr;
}
               ;

_LayPort       :
{
debug ("empty _LayPort")
$$ = layportptr;
}
               | _LayPort PortPos
{
debug ("_LayPort PortPos")
if (layportptr->pos[HOR] != 0 || layportptr->pos[VER] != 0)
   sdfreport (Warning, "%s\nLayPort '%s' has more than one PortPos section,\n"
	"all but first ignored. (Line %d)", FilNam, layportptr->cirport/*BEWARE: HACK*/, yylineno);
else
   {
   layportptr->pos[HOR] = ($2).hor;
   layportptr->pos[VER] = ($2).ver;
   }
$$ = layportptr;
}
               | _LayPort PortLayer
{
debug ("_LayPort PortLayer")
if (layportptr->layer)
   sdfreport (Warning, "%s\nLayPort '%s' has more than one Layer section,\n"
	"all but first ignored. (Line %d)", FilNam, layportptr->cirport/*BEWARE: HACK*/, yylineno);
else
   layportptr->layer = $2;
$$ = layportptr;
}

               | _LayPort Comment
{
debug ("_LayPort Comment");
}

               | _LayPort UserData
{
debug ("_LayPort UserData");
}
               ;

PortPos        : LBR PORTPOSITION NUMBER NUMBER RBR
{
HORVER hv;
debug ("LBR PORTPOSITION NUMBER NUMBER RBR");
if (sdfwhat & SDFLAYPORT)
   {
   hv.hor = atos ($3);
   hv.ver = atos ($4);
   }
$$ = hv;
}
               ;

PortLayer      : LBR PORTLAYER NUMBER RBR
{
debug ("LBR PORTLAYER STRNG RBR")
$$ = (sdfwhat & SDFLAYPORT) ? atos ($3) : 0;
}
               ;


/*
 * Library/Function/LogicImpl/LayoutImpl/LayInterface/LayLabelList
 * Labels on layout level
 */
LayLabelList    : LBR LAYOUTLABELLIST
{
debug ("LBR LAYOUTLABELLIST");
sdfhavethisthing |= SDFLAYLABEL;
if (sdfstuff & SDFLAYLABEL)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYLABEL && sdfwritethislay->laylabel)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_laylabellist (sdfcopystream, sdfwritethislay->laylabel);
      }
   }
laylabellistptr = NULL;
}
                 _LayLabelList RBR
{
debug ("_LayLabelList RBR")
if (sdfstuff & SDFLAYLABEL) sdfcopytheinput = TRUE; /* resume copying */
$$ = (sdfwhat & SDFLAYLABEL) ? laylabellistptr : NULL;
}
               ;

_LayLabelList   :
{
debug ("empty _LayLabelList")
$$ = laylabellistptr;
}
               | _LayLabelList LayLabel
{
debug ("_LayLabelList LayLabel")
($2)->next = laylabellistptr;
laylabellistptr = $2;
$$ = laylabellistptr;
}
               ;

LayLabel        : LBR LAYOUTLABEL STRNG
{
debug ("LBR LAYOUTLABEL STRNG");
if (sdfwhat & SDFLAYLABEL)
   {
   NewLaylabel (laylabelptr);
   laylabelptr->name = canonicstring ($3);
   }
else
   {
   laylabelptr = &junklaylabel;
   /* avoid error message 'more than one..' */
   laylabelptr->pos[HOR] = laylabelptr->pos[VER] = laylabelptr->layer = 0;
   }
}
                 _LayLabel RBR
{
debug ("_LayLabel RBR")
$$ = laylabelptr;
}
               ;

_LayLabel       :
{
debug ("empty _LayLabel")
$$ = laylabelptr;
}
               | _LayLabel LabelPos
{
debug ("_LayLabel LabelPos")
if (laylabelptr->pos[HOR] != 0 || laylabelptr->pos[VER] != 0)
   sdfreport (Warning, "%s\nLayLabel '%s' has more than one LabelPos section,\n"
	"all but first ignored. (Line %d)", FilNam, laylabelptr->name, yylineno);
else
   {
   laylabelptr->pos[HOR] = ($2).hor;
   laylabelptr->pos[VER] = ($2).ver;
   }
$$ = laylabelptr;
}
               | _LayLabel LabelLayer
{
debug ("_LayLabel LabelLayer")
if (laylabelptr->layer)
   sdfreport (Warning, "%s\nLayLabel '%s' has more than one Layer section,\n"
	"all but first ignored. (Line %d)", FilNam, laylabelptr->name, yylineno);
else
   laylabelptr->layer = $2;
$$ = laylabelptr;
}

               | _LayLabel Comment
{
debug ("_LayLabel Comment");
}

               | _LayLabel UserData
{
debug ("_LayLabel UserData");
}
               ;

LabelPos        : LBR LABELPOSITION NUMBER NUMBER RBR
{
HORVER hv;
debug ("LBR LABELPOSITION NUMBER NUMBER RBR");
if (sdfwhat & SDFLAYLABEL)
   {
   hv.hor = atos ($3);
   hv.ver = atos ($4);
   }
$$ = hv;
}
               ;

LabelLayer      : LBR LABELLAYER NUMBER RBR
{
debug ("LBR LABELLAYER STRNG RBR")
$$ = (sdfwhat & SDFLAYLABEL) ? atos ($3) : 0;
}
               ;


/*
 * Library/Function/LogicImpl/LayoutImpl/LayInterface/LayBoundingBox
 * bounding box of the circuit
 */
LayBoundingBox : LBR LAYOUTBBX
{
debug ("LBR LAYOUTBBX NUMBER NUMBER RBR");
if (sdfstuff & SDFLAYBBX)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYBBX)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_bbx (sdfcopystream, sdfwritethislay->bbx);
      }
   }
}
                NUMBER NUMBER RBR
{
HORVER hv;
sdfhavethisthing |= SDFLAYBBX;
if (sdfstuff & SDFLAYBBX)
   sdfcopytheinput = TRUE;	  /* resume copying */
if (sdfwhat & SDFLAYBBX)
   {
   hv.hor = atos ($4);
   hv.ver = atos ($5);
   }
$$ = hv;
}
               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/LayInterface/LayBoundingBox
 * offset relative to basic image element
 */
LayOffset      : LBR LAYOUTOFFSET
{
debug ("LBR LAYOUTOFFSET NUMBER NUMBER RBR")
if (sdfstuff & SDFLAYOFF)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYOFF)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_off (sdfcopystream, sdfwritethislay->off);
      }
   }
}
                 NUMBER NUMBER RBR
{
HORVER hv;
sdfhavethisthing |= SDFLAYOFF;
if (sdfstuff & SDFLAYOFF)
   sdfcopytheinput = TRUE;	  /* resume copying */
if (sdfwhat & SDFLAYOFF)
   {
   hv.hor = atos ($4);
   hv.ver = atos ($5);
   }
$$ = hv;
}

               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/LayInstance
 * model-call of physical cell
 */
LayInstList    : LBR LAYOUTINSTANCELIST
{
debug ("LBR LAYOUTINSTANCELIST");
sdfhavethisthing |= SDFLAYSLICE;
if (++sdfslicedepth == 1 && sdfstuff & SDFLAYSLICE)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYSLICE && sdfwritethislay->slice)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_slice (sdfcopystream, sdfwritethislay->slice);
      }
   }
}
                 _LayInstList RBR
{
SLICEPTR   sliptr;

if (--sdfslicedepth == 0 && sdfstuff & SDFLAYSLICE)
   sdfcopytheinput = TRUE;	  /* resume copying */
debug ("_LayInstList RBR");
if (sdfwhat & SDFLAYSLICE)
   {
   NewSlice (sliptr);
   sliptr->chld_type = SLICE_CHLD;
   sliptr->chld.slice = $4;	  /* $4 is _LayInstList */
   /* LayoutInstanceList is like a LayoutSlice without ordination. */
   sliptr->ordination = CHAOS;
   }
else
   sliptr = &junkslice;
$$ = sliptr;
}
               ;

_LayInstList   :
{
debug ("empty _LayInstList")
$$ = (SLICEPTR)NULL;		  /* NO list is NULL list, or what? */
}
               | _LayInstList LayInstance
{
SLICEPTR sliptr;

debug ("_LayInstList LayInstance");
if (sdfwhat & SDFLAYSLICE)
   {
   /* Unfortunately we cannot link a LAYINST into the SLICE list. Therefore, we create this */
   /* dummy SLICE which has only one child. The parent rule _LayoutImpl cleans up the mess. */
   NewSlice (sliptr);
   sliptr->ordination = CHAOS;
   sliptr->chld_type = LAYINST_CHLD;
   sliptr->chld.layinst = $2;	  /* Reference to the actual LayInstance. */
   sliptr->next = $1;		  /* Link this dummy in front of the _LayInstList... */
   }
else
   sliptr = &junkslice;
$$ = sliptr;			  /* ...and return this new list. */
}
               | _LayInstList LaySlice
{
debug ("_LayInstList LaySlice")
($2)->next = $1;
$$ = $2;
}
               | _LayInstList LayInstList
{
debug ("_LayInstList LayInstList")
($2)->next = $1;
$$ = $2;
}
               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/LaySlice
 * a slice
 */
LaySlice       : LBR LAYOUTSLICE STRNG
{
debug ("LBR LAYOUTSLICE STRNG");
sdfhavethisthing |= SDFLAYSLICE;
if (++sdfslicedepth == 1 && sdfstuff & SDFLAYSLICE)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYSLICE && sdfwritethislay->slice)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_slice (sdfcopystream, sdfwritethislay->slice);
      }
   }
}
                _LaySlice RBR
{
SLICEPTR   sliptr;
char       *ordstring;
short      ordination;

debug ("_LaySlice RBR");
if (sdfwhat & SDFLAYSLICE)
   {
   if (strcmp ("horizontal", ordstring = downcase ($3)) == 0)
      ordination = HORIZONTAL;
   else if (strcmp ("vertical", ordstring) == 0)
      ordination = VERTICAL;
   else if (strcmp ("chaos", ordstring) == 0)
      ordination = CHAOS;
   else
      {
      sdfreport (Warning, "%s\nOrdination '%s' not recognized for LayoutSlice statement"
		" in layout (%s(%s(%s(%s))))\n",
		FilNam, $3, layoutptr->name, layoutptr->circuit->name,
		layoutptr->circuit->function->name,
		layoutptr->circuit->function->library->name);
      sdfreport (Warning, "I shall assume CHAOS if you don't mind. (Line %d)", yylineno);
      ordination = CHAOS;
      }
   NewSlice (sliptr);
   sliptr->ordination = ordination;
   sliptr->chld_type = SLICE_CHLD;
   sliptr->chld.slice = $5;
   }
else
   sliptr = &junkslice;
if (--sdfslicedepth == 0 && sdfstuff & SDFLAYSLICE)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = sliptr;
}
               ;

_LaySlice      :
{
debug ("empty _LaySlice")
$$ = NULL;			  /* We need the NULL pointer. */
}
               | _LaySlice LaySliceRef
{
debug ("_LaySlice LaySliceRef")
($2)->next = $1;
$$ = $2;
}
               ;

LaySliceRef    : LayInstList
{
debug ("LayInstList")
$$ = $1;
}
               | LayInstance
{
SLICEPTR sliptr;

debug ("LayInstance");
if (sdfwhat & SDFLAYSLICE)
   {
   /* Unfortunately we cannot link a LAYINST into the SLICE list. Therefore, we create this */
   /* dummy SLICE which has only one child. The parent rule _LayoutImpl cleans up the mess. */
   NewSlice (sliptr);
   sliptr->ordination = CHAOS;
   sliptr->chld_type = LAYINST_CHLD;
   sliptr->chld.layinst = $1;	  /* Reference to the actual LayInstance. */
   }
else
   sliptr = &junkslice;
$$ = sliptr;
}
               | LaySlice
{
debug ("LaySlice")
$$ = $1;
}
               ;

LayInstance    : LBR LAYOUTINSTANCE STRNG
{
debug ("LBR LAYOUTINSTANCE STRNG");
if (sdfwhat & SDFLAYSLICE)
   {
   NewLayinst (layinstptr);
   layinstptr->name = canonicstring ($3);
   layinstptr->layout = HACK;	  /* We're going to be dirty again... */
   }
else
   layinstptr = &junklayinst;
}
                 _LayInstRef RBR
{
debug ("_LayInstRef RBR")
$$ = layinstptr;
}
               ;

_LayInstRef    : LayCellRef
{
debug ("LayCellRef");
}
               | _LayInstRef Orientation
{
debug ("_LayInstRef Orientation");
}
               | _LayInstRef Comment
{
debug ("_LayInstRef Comment");
}
               | _LayInstRef UserData
{
debug ("_LayInstRef UserData");
}
               ;

LayCellRef     : LBR LAYOUTCELLREF STRNG _Stuff LayCirRef OptionalString RBR
{
/* Solve this reference later. Until then, we apologize for the inconvenience: */
if (sdfwhat & SDFLAYSLICE)
   {
   layinstptr->layout = (LAYOUTPTR)canonicstring ($3);
   layinstptr->flag.p = (void *)$5;	  /* namelist: Cir,Fun,Lib */
   /* We just ignore the OptionalString: we don't support attributes for layout
    *
    * layinstptr->attribute = canonicstring ($6);
    */
   }
}
               ;

LayCirRef      : LBR LAYOUTCIRREF STRNG _Stuff LayFunRef RBR
{
if (sdfwhat & SDFLAYSLICE)
   {
   NAMELISTPTR p;
   NewNamelist (p);
   p->name = canonicstring ($3);
   p->next = ($5);
   $$ = p;
   }
else
   $$ = (NAMELISTPTR)NULL;
}
               |		  /* empty */
{
$$ = (NAMELISTPTR)NULL;
}
               ;

LayFunRef      : LBR LAYOUTFUNREF STRNG _Stuff LayLibRef RBR
{
if (sdfwhat & SDFLAYSLICE)
   {
   NAMELISTPTR p;
   NewNamelist (p);
   p->name = canonicstring ($3);
   p->next = ($5);
   $$ = p;
   }
else
   $$ = (NAMELISTPTR)NULL;
}
               |		  /* empty */
{
$$ = (NAMELISTPTR)NULL;
}
               ;


LayLibRef      : LBR LAYOUTLIBREF STRNG _Stuff RBR
{
if (sdfwhat & SDFLAYSLICE)
   {
   NAMELISTPTR p;
   NewNamelist (p);
   p->name = canonicstring ($3);
   p->next = NULL;
   $$ = p;
   }
else
   $$ = (NAMELISTPTR)NULL;
}
               |		  /* empty */
{
$$ = (NAMELISTPTR)NULL;
}
               ;

_Stuff     :
{
debug ("empty _Stuff");
}
               | _Stuff Comment
{
debug ("_Stuff Comment");
}
               | _Stuff UserData
{
debug ("_Stuff UserData");
}
               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/LayContents/LayInstance/Orientation
 * orientation info of model call of physical cell
 * Contains a standard orientaion matrix
 */
Orientation    : LBR ORIENTATION NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
{
debug ("LBR ORIENTATION NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR");
if (sdfwhat & SDFLAYSLICE)
   {
   layinstptr->mtx[0] = atos ($3);
   layinstptr->mtx[1] = atos ($4);
   layinstptr->mtx[2] = atos ($5);
   layinstptr->mtx[3] = atos ($6);
   layinstptr->mtx[4] = atos ($7);
   layinstptr->mtx[5] = atos ($8);
   }
}
               ;

/*
 * Library/Function/LogicImpl/LayoutImpl/WireList
 * The grid-matrix wire map containing (part of) the actual layout of the wires in a layer.
 */
WireList    : LBR WIRELIST
{
debug ("LBR WIRELIST");
sdfhavethisthing |= SDFLAYWIRE;
if (sdfstuff & SDFLAYWIRE)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYWIRE && sdfwritethislay->wire)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_wirelist (sdfcopystream, sdfwritethislay->wire);
      }
   }
}
               _WireList RBR
{
debug ("_WireList RBR");
if (sdfstuff & SDFLAYWIRE)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = wirelistptr = (sdfwhat&SDFLAYWIRE ? $4 : NULL);
}

               ;

_WireList   :
{
debug ("empty _WireList")
$$ = NULL;
}
               | _WireList Wire
{
debug ("_WireList Wire")
($2)->next = $1;
$$ = $2;
}
               ;

Wire           : LBR WIRETOKEN NUMBER NUMBER NUMBER NUMBER NUMBER RBR
{
debug ("LBR WIRETOKEN WireLayer");
if (sdfwhat & SDFLAYWIRE)
   {
   NewWire (wireptr);
   wireptr->layer = atos ($3);
   wireptr->crd[XL] = atos ($4);
   wireptr->crd[XR] = atos ($5);
   wireptr->crd[YB] = atos ($6);
   wireptr->crd[YT] = atos ($7);
   wireptr->next = NULL;
   }
else
   wireptr = &junkwire;
$$ = wireptr;
}
;


LibStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN");
sdfhavethisthing |= SDFLIBSTAT;
if (makeindex)
   {
   if (sdffunislastthinginlib == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* will not be able to efficiently parse this lib */
      sdffunislastthinginlib = SDF_LOST_CAUSE;
   }
if (sdfstuff & SDFLIBSTAT)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLIBSTAT && sdfwritethislib->status)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_status (sdfcopystream, sdfwritethislib->status);
      }
   }
if (sdfwhat & SDFLIBSTAT)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
if (sdfstuff & SDFLIBSTAT)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = statusptr;
}
               ;


SeaStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN")
if (!makeindex)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
$$ = statusptr;
}
               ;

CirStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN");
sdfhavethisthing |= SDFCIRSTAT;
if (makeindex)
   {
   if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* will not be able to efficiently parse this circuit */
      sdflayislastthingincir = SDF_LOST_CAUSE;
   }
if (sdfstuff & SDFCIRSTAT)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFCIRSTAT && sdfwritethiscir->status)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_status (sdfcopystream, sdfwritethiscir->status);
      }
   }
if (sdfwhat & SDFCIRSTAT)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
if (sdfstuff & SDFCIRSTAT)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = statusptr;
}
               ;


LayStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN");
sdfhavethisthing |= SDFLAYSTAT;
if (sdfstuff & SDFLAYSTAT)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFLAYSTAT && sdfwritethislay->status)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_status (sdfcopystream, sdfwritethislay->status);
      }
   }
if (sdfwhat & SDFLAYSTAT)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
if (sdfstuff & SDFLAYSTAT)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = statusptr;
}
               ;


FunStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN");
sdfhavethisthing |= SDFFUNSTAT;
if (makeindex)
   {
   if (sdfcirislastthinginfun == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* will not be able to efficiently parse this function */
      sdfcirislastthinginfun = SDF_LOST_CAUSE;
   }
if (sdfstuff & SDFFUNSTAT)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFFUNSTAT && sdfwritethisfun->status)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_status (sdfcopystream, sdfwritethisfun->status);
      }
   }
if (sdfwhat & SDFFUNSTAT)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
if (sdfstuff & SDFFUNSTAT)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = statusptr;
}
               ;


/*
 * BASIC functions
 */


_Status        :
{
debug ("empty _Status");
}
               | _Status Written
{
debug ("_Status Written");
}
/* This violates Edif, but what the hack: */
               | _Status _Written
{
debug ("_Status _Written");
}
               | _Status Comment
{
debug ("_Status Comment");
}
               | _Status UserData
{
debug ("_Status UserData");
}
               ;

Written        : LBR WRITTEN _Written RBR
{
debug ("LBR WRITTEN _Written RBR");
}
               ;

_Written       : /* empty */
{
debug ("_Written empty");
}
               | _Written TimeStamp
{
debug ("TimeStamp");
}
               | _Written Author
{
debug ("_Written Author");
}
               | _Written Program
{
debug ("_Written Program");
}
/*             | _Written DataOrigin     not implemented */
/*             | _Written Property       not implemented */
               | _Written Comment
{
debug ("_Written Comment");
}
               | _Written UserData
{
debug ("_Written UserData");
}
               ;

                     /* year month day hour minute second */
TimeStamp      : LBR TIMESTAMP NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
{
    debug ("LBR TIMESTAMP NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR")

    if (!junkstat)
    {
	time_t thetime = 0;
	int yy = atos ($3);
	int mo = atos ($4);
	int dd = atos ($5);
	int hh = atos ($6);
	int mi = atos ($7);
	int ss = atos ($8);

	if (!yy && !mo && !dd && !hh && !mi && !ss) ; /* special case */
	else if (!sdftimecvt (&thetime, yy, mo, dd, hh, mi, ss)) {
	    sdfreport (Error, "%s\nI found an error in the TimeStamp on line %d:\n"
		"%s. I will assume (TimeStamp 0 0 0 0 0 0)\n", FilNam, yylineno, sdftimecvterror);
	}
	statusptr->timestamp = thetime;
    }
}
;

Author         : LBR AUTHOR /* followed by ANYTHING [...] RBR */
{
debug ("LBR AUTHOR /* followed by ANYTHING [...] RBR */")
if (!junkstat)
   statusptr->author = canonicstring (copythisthing ());
else
   skipthisthing ();
}
               ;

Program        : LBR PROGRAM /* followed by ANYTHING [...] RBR */
{
debug ("LBR PROGRAM /* followed by ANYTHING [...] RBR */")
if (!junkstat)
   statusptr->program = canonicstring (copythisthing ());
else
   skipthisthing ();
}
               ;

UserData       : LBR STRNG /* followed by ANYTHING [...] RBR */
{
debug ("LBR STRNG /* followed by ANYTHING [...] RBR */")
sdfreport (Warning, "%s\nI did not recognize your statement '%s' at line %d ...,\n", FilNam, $2, yylineno);
skipthisthing ();
sdfreport (Warning, "... so it seemed better to skip everything up till line %d.\n", yylineno);
}
               ;

Comment        : LBR COMMENT /* followed by ANYTHING [...] RBR */
{
debug ("LBR COMMENT /* followed by ANYTHING [...] RBR */")
skipthisthing ();
}
;

LBR            : LBRTOKEN
{
sdfleftparenthesis = sdffilepos; /* record the file position of the most recent '(' */
}
;

Attribute      : LBR ATTRIBUTE OptionalString RBR
{
$$ = $3;
}
;
				  /* IK, ... here parsing timing structures */

Timing         : LBR TIMINGTOKEN STRNG
{
debug ("LBR TIMINGTOKEN STRNG");
sdfhavethisthing |= SDFCIRTM;

if (makeindex)
   {
   if (sdflayislastthingincir == SDF_LETS_KEEP_IT_LIKE_THIS)
      /* Will not be able to efficiently parse this circuit */
      sdflayislastthingincir = SDF_LOST_CAUSE;
   }

if (sdfstuff & SDFCIRTM)
   {
   sdfabortcopy (SDFDISCARDSPACES); /* don't copy this statement */
   if (sdfwrite & SDFCIRTM && sdfwritethiscir->timing)
      {
      sdfdodelayedcopy (0);	  /* flush */
      dump_timing (sdfcopystream, sdfwritethiscir->timing);
      }
   }
if (sdfwhat & SDFCIRTM)
{
  NewTiming (timingPtr);
  timingPtr->name = canonicstring ($3);
  junktm = FALSE;
}
else
{
  timingPtr = &junktiming;
  timingPtr->name = NULL;
  junktm = TRUE;
}

timingPtr->circuit = circuitptr;

if (sdfverbose && sdfwhat == SDFCIRTM)
  fprintf (stderr, "............(Timing \"%s\")\n", timingPtr->name);
}
                 _Timing RBR
{
debug ("_Timing RBR")
if (sdfstuff & SDFCIRTM)
   sdfcopytheinput = TRUE;	  /* resume copying */
$$ = (sdfwhat & SDFCIRTM) ? timingPtr : NULL;
}
               ;

_Timing        :
{
   debug ("empty _Timing")
   $$ = timingPtr;
}
               | _Timing  DelAsg
{
   debug ("_Timing  DelAsg")
   ($2)->next = timingPtr->delays;
   timingPtr->delays = $2;
   $$ = timingPtr;
}
               | _Timing  TimeCost
{
   debug ("_Timing  TimeCost")
   timingPtr->timeCost = $2;
   $$ = timingPtr;
}
               | _Timing  TPathList
{
   debug ("_Timing  TPathList")
   timingPtr->tPaths = $2;
   $$ = timingPtr;
}
               | _Timing  NetModList
{
   debug ("_Timing  NetModList")
   timingPtr->netmods = $2;
   $$ = timingPtr;
}
               | _Timing  TimeTermList
{
   debug ("_Timing  TimeTermList")
   timingPtr->t_terms = $2;
   $$ = timingPtr;
}
               | _Timing  TmModInstList
{
   debug ("_Timing  TmModInstList")
   timingPtr->tminstlist = $2;
   $$ = timingPtr;
}

               | _Timing  TmStatus
{
   debug ("_Timing Status")
   timingPtr->status = $2;
   $$ = timingPtr;
}
               | _Timing  Comment
{
   $$ = timingPtr;
}
               | _Timing  UserData
{
   $$ = timingPtr;
}
               ;

TimeTermList   : LBR TIMETERMLISTTOKEN
{
   debug ("LBR TIMETERMLIST")
   ttermlistPtr = NULL;
}
                 _TimeTermList RBR
{
   debug ("_TimeTermList RBR")
   $$ = ttermlistPtr;
}
               ;



TmStatus      : LBR STATUSTOKEN
{
debug ("LBR STATUSTOKEN");
if (sdfwhat & SDFCIRTM)
   {
   NewStatus (statusptr);
   junkstat = FALSE;
   }
else
   {
   statusptr = NULL;
   junkstat = TRUE;
   }
}
                 _Status RBR
{
debug ("_Status RBR")
$$ = statusptr;
}
               ;


_TimeTermList  :
{
   debug ("empty _TimeTermList")
   $$ = ttermlistPtr;
}
               | _TimeTermList TimeTerm
{
   debug ("_TimeTermList TimeTerm")
   ($2)->next = ttermlistPtr;
   ttermlistPtr = $2;
   $$ = ttermlistPtr;
}
               ;

TimeTerm       : LBR TIMETERMTOKEN STRNG NUMBER
{
   debug ("LBR TIMETERMTOKEN STRNG NUMBER")

   if (sdfwhat & SDFCIRTM)
   {
     NewTimeTerm (timetermPtr);

     timetermPtr->termreflist = NULL; /* some initialization */
     timetermPtr->cirportlist = NULL;
     timetermPtr->next = NULL;
     timetermPtr->outputTime = -1.0;
     timetermPtr->reqInputTime = -1.0;
     timetermPtr->load = -1.0;
     timetermPtr->drive = -1.0;

     timetermPtr->name = canonicstring ($3);
     timetermPtr->type = (tTermType)atos ($4);
     if (timetermPtr->type != InputTTerm &&
	timetermPtr->type != OutputTTerm &&
	timetermPtr->type != InternalRegTTerm &&
	timetermPtr->type != InternalClkTTerm  &&
	timetermPtr->type != BiDirPortTTerm
	)
     {
       sdfreport (Warning, "%s\nWrong time terminal type - default"
		 " InputTTerm assumed - on line %d:\n", FilNam, yylineno);
       timetermPtr->type = InputTTerm;
     }
   }
   else
   {
     timetermPtr = &junktimeterm;
   }
   timetermPtr->timing = timingPtr;
}
                 _TimeTerm RBR
{
   if (!timetermPtr->termreflist && !timetermPtr->cirportlist)
      sdfreport (Warning, "%s\nNo time terminal or cirport references"
		" specified - on line %d:\n", FilNam, yylineno);

   debug ("_TimeTerm RBR")
   $$ = timetermPtr;
}
                ;




_TimeTerm       :
{
   debug ("empty _TimeTerm")
   $$ = timetermPtr;
}
               | _TimeTerm  TimeTermRef
{
   debug ("_TimeTerm  TimeTermRef")
   ($2)->next = timetermPtr->termreflist;
   timetermPtr->termreflist = $2;
   if (!($2)->inst)
      sdfreport (Warning, "%s\nTiming model instance for this terminal"
		" undefined - on line %d:\n", FilNam, yylineno);

   $$ = timetermPtr;
}
               | _TimeTerm  CirPortRef
{
   debug ("_TimeTerm  CirPortRef")
   ($2)->next = timetermPtr->cirportlist;
   timetermPtr->cirportlist = $2;
   $$ = timetermPtr;
}
               | _TimeTerm  TimeCost
{
   debug ("_TimeTerm  TimeCost")
   timetermPtr->timecost = $2;
   $$ = timetermPtr;
}

              | _TimeTerm   InputLoad
{
   debug ("_TimeTerm  InputLoad")
   timetermPtr->load = $2;
   $$ = timetermPtr;
}
              | _TimeTerm   InputDrive
{
   debug ("_TimeTerm  InputDrive")
   timetermPtr->drive = $2;
   $$ = timetermPtr;
}

              | _TimeTerm   ReqInputTime
{
   debug ("_TimeTerm  ReqInputTime")
   timetermPtr->reqInputTime = $2;
   $$ = timetermPtr;
}
              | _TimeTerm   OutputTime
{
   debug ("_TimeTerm  OutputTime")
   timetermPtr->outputTime = $2;
   $$ = timetermPtr;
}
              ;


CirPortRef    : LBR CIRPORTREFTOKEN STRNG RBR
{
   debug ("LBR CIRPORTREFTOKEN STRNG RBR")
   if (sdfwhat & SDFCIRTM)
   {
     NewCirportref (cirportrefPtr);
     cirportrefPtr->cirport = (CIRPORTPTR)canonicstring ($3);
   }
   else
   {
     cirportrefPtr = &junkcirportref;
   }
   $$ = cirportrefPtr;
}
           ;

TimeTermRef    : LBR TIMETERMREFTOKEN
{
   debug ("LBR TIMETERMREFTOKEN")

   if (sdfwhat & SDFCIRTM)
   {
     NewTimeTermRef (timetermrefPtr);
     timetermrefPtr->inst = NULL;
   }
   else
   {
     timetermrefPtr = &junktimetermref;
   }

}
                 _TimeTermRef RBR
{
   debug ("_TimeTermRef RBR")
   $$ = timetermrefPtr;
}
              ;

/* This construct is because a terminal can be referenced in two ways
   1. First as a current level timing terminal
   2. As a timing terminal of timing model instance - then it should also
      contain information about timing model instance that it belongs to
*/

_TimeTermRef : STRNG
{
   debug ("STRNG")
   timetermrefPtr->term = (TIMETERMPTR)canonicstring ($1);
   timetermrefPtr->inst = NULL;
   $$ = timetermrefPtr;
}
              | STRNG TmModInstRef
{
   debug ("STRNG TmModInstRef")
   timetermrefPtr->term = (TIMETERMPTR)canonicstring ($1);
   $$ = timetermrefPtr;
}
              ;


TmModInstRef  : LBR TMMODINSTREFTOKEN STRNG RBR
{
   timetermrefPtr->inst = (TMMODINSTPTR)canonicstring ($3);
   $$ = timetermrefPtr;
}
             ;



ReqInputTime : LBR REQINPUTTIMETOKEN STRNG RBR
{
   debug (" LBR REQINPUTTIMETOKEN STRNG RBR")
   $$ = atof ($3);
}
             ;

OutputTime   : LBR OUTPUTTIMETOKEN STRNG RBR
{
   debug (" LBR OUTPUTTIMETOKEN STRNG RBR")
   $$ = atof ($3);
}
             ;

InputLoad   : LBR INPUTLOADTOKEN STRNG RBR
{
   debug (" LBR INPUTLOADTOKEN STRNG RBR")
   if (timetermPtr->type != InputTTerm &&
       timetermPtr->type != OutputTTerm &&
       timetermPtr->type != BiDirPortTTerm )
      sdfreport (Warning, "%s\nThis is neither an input "
	    "nor output time terminal - on line %d:\n", FilNam, yylineno);
   $$ = atof ($3);
}
             ;
InputDrive   : LBR INPUTDRIVETOKEN STRNG RBR
{
   debug (" LBR INPUTDRIVETOKEN STRNG RBR")
   if (timetermPtr->type != InputTTerm &&
       timetermPtr->type != BiDirPortTTerm
       )
      sdfreport (Warning, "%s\nThis is not an input "
	    "time terminal - on line %d:\n", FilNam, yylineno);
   $$ = atof ($3);
}
             ;




TmModInstList : LBR  TMMODINSTLISTTOKEN
{
   debug ("LBR  TMMODINSTLISTTOKEN")
   tmmodinstlistPtr = NULL;
}
                _TmModInstList RBR
{
   debug ("_TmModInstList RBR")
   $$ = tmmodinstlistPtr;
}
              ;


_TmModInstList :
{
   debug ("_TmModInstList - empty ")
   $$ = tmmodinstlistPtr;
}
               | _TmModInstList TmModInst
{
   debug ("_TmModInstList TmModInst")
   ($2)->next = tmmodinstlistPtr;
   tmmodinstlistPtr = $2;
   $$ = tmmodinstlistPtr;
}
               ;

TmModInst      : LBR TMMODINSTTOKEN STRNG CInstRef TimingRef RBR
{
   debug ("LBR TMMODINSTTOKEN STRNG CInstRef TimingRef RBR")
   if (sdfwhat & SDFCIRTM)
   {
     NewTmModInst (tmmodinstPtr);
     tmmodinstPtr->name = canonicstring ($3);
     tmmodinstPtr->cirinst = (CIRINSTPTR)$4;
     tmmodinstPtr->timing = (TIMINGPTR)$5;            /* returned type will be string */
   }
   else
     tmmodinstPtr = &junktmmodinst;

   tmmodinstPtr->parent = timingPtr;
   $$ = tmmodinstPtr;
}
               ;


TimingRef       : LBR TIMINGREFTOKEN STRNG RBR
{
   debug ("LBR TIMINGREFTOKEN STRNG RBR")
   $$ = canonicstring ($3);
}
              ;
CInstRef       : LBR CINSTREFTOKEN STRNG RBR
{
   debug ("LBR TIMINGREFTOKEN STRNG RBR")
   $$ = canonicstring ($3);
}
              ;


NetModList   : LBR NETMODLISTTOKEN
{
   debug ("LBR NETMODLISTTOKEN")
   netmodlistPtr = NULL;
}
                 _NetModList RBR
{
   debug ("_NetModList RBR")
   $$ = netmodlistPtr;
}
               ;

_NetModList  :
{
   debug ("empty _NetModList")
   $$ = netmodlistPtr;
}
               | _NetModList NetMod
{
   debug ("_NetModList NetMod")
   ($2)->next = netmodlistPtr;
   netmodlistPtr = $2;
   $$ = netmodlistPtr;
}
               ;

NetMod         : LBR NETMODTOKEN STRNG
{
   debug ("LBR NETMODTOKEN")

   if (sdfwhat & SDFCIRTM)
   {
     NewNetMod (netmodPtr);
     netmodPtr->name = canonicstring ($3);
     netmodPtr->netlist = NULL;
     netmodPtr->buslist = NULL;
   }
   else
     netmodPtr = &junknetmod;
}
                 _NetMod  TimeCost RBR
{
   netmodPtr->cost = $6;
   if (!netmodPtr->netlist && !netmodPtr->buslist)
      sdfreport (Warning, "%s\nNo references to nets or buses for net "
	    "model defined - on line %d:\n", FilNam, yylineno);

   $$ = netmodPtr;
}
               ;

_NetMod        :
{
   debug ("_NetMod empty")
   $$ = netmodPtr;
}
               | _NetMod  NetRef
{
   debug ("_NetMod  NetRef")
   ($2)->next = netmodPtr->netlist;
   netmodPtr->netlist = $2;
   $$ = netmodPtr;
}
               | _NetMod  BusRef
{
   debug ("_NetMod  BusRef")
   ($2)->next = netmodPtr->buslist;
   netmodPtr->buslist = $2;
   $$ = netmodPtr;
}
               ;


BusRef         : LBR BUSREFTOKEN STRNG RBR
{
BUSREFPTR busref = NULL;
debug ("LBR BUSREFTOKENTOKEN STRNG RBR")
if (sdfwhat & SDFCIRTM)
{
  NewBusRef (busref);
}

/* NASTY HACK, we solve this reference later: */
busref->bus = (BUSPTR)canonicstring ($3);
$$ = busref;
}
;


TPathList     : LBR TPATHLISTTOKEN
{
   debug ("LBR TPATHLIST")
   tpathlistPtr = NULL;
   $<tpath>$ = tpathlistPtr;
}
                _TPathList    RBR
{
   $$ = tpathlistPtr;
}
             ;

_TPathList   :
{
   debug ("empty _TPathList ")
   $$ = tpathlistPtr;
}
             | _TPathList  TPath
{
   debug ("_TPathList  TPath")
   ($2)->next = tpathlistPtr;
   tpathlistPtr = $2;
   $$ = tpathlistPtr;
}
             ;

TPath        :  LBR TPATHTOKEN  STRNG
{
   debug ("LBR TPATHTOKEN  STRNG")
   if (sdfwhat & SDFCIRTM)
   {
     NewTPath (tpathPtr);
     tpathPtr->name = canonicstring ($3);
   }
   else
   {
     tpathPtr = &junktpath;
   }
   tpathPtr->parent = timingPtr;
   $<tpath>$ = tpathPtr;
}
                StartTermList EndTermList TimeCost RBR
{
   debug ("StartTermList EndTermList TimeCost RBR")
   tpathPtr->startTermList = $5;
   tpathPtr->endTermList = $6;
   if (!tpathPtr->startTermList || !tpathPtr->endTermList)
      sdfreport (Warning, "%s\nMust define both start and end terminals "
	    "for TPath - line %d:\n", FilNam, yylineno);


   tpathPtr->timeCost = $7;
   $$ = tpathPtr;
}
            ;

StartTermList : LBR STARTTERMLISTTOKEN
{
   debug ("LBR STARTTERMLISTTOKEN")
   starttermlistPtr = NULL;
   $<timetermref>$ = starttermlistPtr;
}
                StartTermList_ RBR
{
   debug ("StartTermList_ RBR")
   $$ = starttermlistPtr;
}
                ;

StartTermList_  :
{
   debug ("empty StartTermList_")
   $$ = starttermlistPtr;
}
                 | StartTermList_ TimeTermRef
{
   debug ("StartTermList_ TimeTermRef")
   if (($2)->inst)
      sdfreport (Warning, "%s\nOnly current level timing terminals references "
		"allowed - on line %d:\n", FilNam, yylineno);
   ($2)->next = starttermlistPtr;
   starttermlistPtr = $2;
   $$ = starttermlistPtr;
}
                ;

EndTermList : LBR ENDTERMLISTTOKEN
{
   debug ("LBR STARTTERMLISTTOKEN")
   endtermlistPtr = NULL;
   $<timetermref>$ = endtermlistPtr;
}
                EndTermList_ RBR
{
   debug ("EndTermList_ RBR")
   $$ = endtermlistPtr;
}
                ;

EndTermList_  :
{
   debug ("empty EndTermList_")
   $$ = endtermlistPtr;
}
                | EndTermList_ TimeTermRef
{
   debug ("EndTermList_ TimeTermRef")
   if (($2)->inst)
      sdfreport (Warning, "%s\nOnly current level timing terminals references "
		"allowed - on line %d:\n", FilNam, yylineno);
   ($2)->next = endtermlistPtr;
   endtermlistPtr = $2;
   $$ = endtermlistPtr;
}
                ;

TimeCost       : LBR  TIMECOSTTOKEN
{
   debug ("LBR TIMECOSTTOKEN")
   if (sdfwhat & SDFCIRTM)
   {
     NewTimeCost (timecostPtr);
     timecostPtr->p_num = 0;
   }
   else
     timecostPtr = &junktimecost;

   $<timecost>$ = timecostPtr;
}
                 _TimeCost RBR
{
   debug ("_TimeCost RBR")
/* if (timecostPtr->p_num == 0)
      sdfreport (Warning, "%s\nMust define at least one point - on line %d:\n", FilNam, yylineno);
*/
   $$ = timecostPtr;
}
                 ;

_TimeCost        :
{
   debug ("empty _TimeCost")
   $$ = timecostPtr;
}
                | _TimeCost TcPoint
{
   debug ("_TimeCost TcPoint");
   timecostPtr->p_num++;
   ($2)->next = timecostPtr->points;
   timecostPtr->points = $2;
   $$ = timecostPtr;
}
                ;

TcPoint         : LBR TCPOINTTOKEN STRNG NUMBER NUMBER STRNG RBR
{
   debug ("LBR TCPOINTTOKEN STRNG NUMBER NUMBER STRNG RBR")
   if (sdfwhat & SDFCIRTM)
   {
     NewTcPoint (tcpointPtr);
     tcpointPtr->name = canonicstring ($3);
     tcpointPtr->delay = atos ($4);
     tcpointPtr->cost = atos ($5);
     tcpointPtr->wayOfImplementing = canonicstring ($6);
   }
   else
     tcpointPtr = &junktcpoint;

   $$ = tcpointPtr;
}
               ;


DelAsg         : LBR DELASGTOKEN STRNG
{
   debug ("LBR DELASGTOKEN STRNG")
   if (sdfwhat & SDFCIRTM)
   {
     NewDelAsg (delasgPtr);
     delasgPtr->name = canonicstring ($3);
     delasgPtr->timing = timingPtr;
     delasgPtr->clockCycle = -1;

   }
   else
     delasgPtr = &junkdelasg;
   $<delasg>$ = delasgPtr;
}
               _DelAsg RBR
{
   debug ("_DelAsg RBR")
   $$ = delasgPtr;
}
              ;

_DelAsg        :
{
   debug ("empty _DelAsg")
   $$ = delasgPtr;
}
               | _DelAsg  TmStatus
{
   debug ("_DelAsg  Status")
   delasgPtr->status = $2;
   $$ = delasgPtr;
}
               | _DelAsg  ClockCycle
{
   debug ("_DelAsg  ClockCycle")
   delasgPtr->clockCycle = $2;
   $$ = delasgPtr;
}
               | _DelAsg  DelAsgInstList
{
   debug ("_DelAsg  DelAsgInstList")
   delasgPtr->pathDelays = $2;
   $$ = delasgPtr;
}
               | _DelAsg  Comment
{
   $$ = delasgPtr;
}
               | _DelAsg  UserData
{
   $$ = delasgPtr;
}
;

ClockCycle   : LBR CLOCKCYCLETOKEN NUMBER RBR
{
   debug ("LBR CLOCKCYCLETOKEN NUMBER RBR")
   $$ = atol ($3);
}
;

DelAsgInstList  : LBR DELASGINSTLISTTOKEN
{
   debug ("LBR DELASGINSTLISTTOKEN")
   delasginstlistPtr = NULL;
   $<delasginst>$ = delasginstlistPtr;
}
                  _DelAsgInstList RBR
{
   debug ("_DelAsgInstList RBR")
   $$ = delasginstlistPtr;
}
;

_DelAsgInstList  :
{
   debug ("empty _DelAsgInstList")
   $$ = delasginstlistPtr;
}
                 | _DelAsgInstList  DelAsgInst
{
   debug ("_DelAsgInstList  DelAsgInst")
   ($2)->next = delasginstlistPtr;
   delasginstlistPtr = $2;
   $$ = delasginstlistPtr;
}
;

DelAsgInst       : LBR  DELASGINSTTOKEN  STRNG  TPathRef  TcPointRef  RBR
{
debug ("LBR  DELASGINSTTOKEN  STRNG  TPathRef  TcPointRef RBR")
  if (sdfwhat & SDFCIRTM)
  {
    NewDelAsgInst (delasginstPtr);
    delasginstPtr->name = canonicstring ($3);
    delasginstPtr->tPath = $4;	  /* here we will get string pointers */
    delasginstPtr->selected = $5; /* because these references cannot be now solved */
  }
  else
    delasginstPtr = &junkdelasginst;

   $$ = delasginstPtr;
}
;

TPathRef         : LBR TPATHREFTOKEN   STRNG RBR
{
   debug ("LBR TPATHREFTOKEN   STRNG RBR")
   $$ = (TPATHPTR)canonicstring ($3);  /* this reference will be solved later */
}
;

TcPointRef         : LBR TCPOINTREFTOKEN   STRNG RBR
{
   debug ("LBR TCPOINTREFTOKEN   STRNG RBR")
   $$ = (TCPOINTPTR)canonicstring ($3);  /* this reference will be solved later */
}
				  /* ##########  IK, end of timing extensions  */
;

Alias       : LBR ALIAS STRNG RBR
{
    if (sdfwrite)
    {
	STRING thealias = NULL;

	if (sdfparseonelib)
	{
	    thealias = sdflibalias (sdfwritethislib->name);
	    sdfhavethisthing |= SDF_X_LIBALIAS;
	}
	else if (sdfparseonefun)
	{
	    thealias = sdffunalias (sdfwritethisfun->name, sdfwritethisfun->library->name);
	    sdfhavethisthing |= SDF_X_FUNALIAS;
	}
	else if (sdfparseonecir)
	{
	    thealias = sdfciralias (sdfwritethiscir->name, sdfwritethiscir->function->name,
	    sdfwritethiscir->function->library->name);
	    sdfhavethisthing |= SDF_X_CIRALIAS;
	}
	else if (sdfparseonelay)
	{
	    thealias = sdflayalias (sdfwritethislay->name, sdfwritethislay->circuit->name,
	    sdfwritethislay->circuit->function->name,
	    sdfwritethislay->circuit->function->library->name);
	    sdfhavethisthing |= SDF_X_LAYALIAS;
	}
	sdfabortcopy (SDFDISCARDSPACES);
	sdfdodelayedcopy (0);	  /* flush */
	dump_alias (sdfcopystream, thealias);
	sdfcopytheinput = TRUE;
    }
    $$ = $3;
}
;
%%

#include "sdflex.h"

#ifndef YY_CURRENT_BUFFER
#define YY_CURRENT_BUFFER yy_current_buffer
#endif

void yyerror (char *s)
{
    sdfreport (Error, "%s (Seadif parser): %s\nTry line %d.", FilNam, s, yylineno);
}

/* Convert ascii string to a short integer. Report on overflow. */
PRIVATE short atos (char *str)
{
    long  base, sign, value, digit;
    short result;
    char *orgstr = str, c;

    sign  = 1; /* default is positive */
    base = 10; /* default is decimal */
    value = 0;

    if (!(c = *str++)) return ((short)value);
    else if (c == '-') { c = *str++; sign = -1; } /* negative number */
    else if (c == '+') c = *str++; /* skip positive sign */

    if (!c) return ((short)value);
    else if (c == '0') /* number starts with a zero digit, must be octal or hex */
    {
	c = *str++;
	if (!c) return ((short)value);
	else if (c == 'x' || c == 'X') { /* hex */
	    base = 16;
	    c = *str++;
	}
	else base = 8; /* octal */
    }

    do {
	digit = c - '0'; /* compute digit's value */
	if (digit >= base || digit < 0)
	{
	    if (base == 8) str = "an octal";
	    else if (base == 16) str = "a hexadecimal";
	    else str = "a decimal";

	    sdfreport (Error, "%s\nDigit '%c' not allowed in %s number.\n"
		"Assume zero value and hope for the best (line %d).", FilNam, c, str, yylineno);
	    return ((short)0);
	}
	value = value * base + digit; /* compute number's value */
    }
    while ((c = *str++) != '\0');

    result = (short)(value *= sign); /* convert long integer to short integer */

    if ((long)result != value) {
	sdfreport (Warning, "%s\nI cannot store your number '%s' in a short integer.\n"
		"Assume zero value and hope for the best (line %d).\n", FilNam, orgstr, yylineno);
	return ((short)0);
    }
    return (result);
}

PRIVATE char *downcase (char *str)
{
    char *s;
    int c, captolower = 'A' - 'a';

    for (s = str; (c = *s); ++s)
	if (c >= 'A' && c <= 'Z') *s = c - captolower;
    return (str);
}

PRIVATE char *printfilnam ()
{
    char *s;

    fnprinted = TRUE;

    s = (char*)malloc (strlen (seadifinputfilename) + 32);
    if (!s) sdfreport (Fatal, "(Seadif parser): cannot malloc");
    sprintf (s, "\n*** TROUBLE in file ``%s'' ***\n", seadifinputfilename);
    return (s);
}

int sdfparse (int idx)
{
    if (idx >= 0)
    {
	yyin = sdffileinfo[idx].fdes;
	seadifinputfilename = sdffileinfo[idx].name;
    }
    libraryptr  = &junklibrary;  libraryptr  -> name = NULL;
    functionptr = &junkfunction; functionptr -> name = NULL;
    circuitptr  = &junkcircuit;  circuitptr  -> name = NULL;
    layoutptr   = &junklayout;   layoutptr   -> name = NULL;

    if (YY_CURRENT_BUFFER) yyrestart (yyin); /* re-init the parser */

    sdfreadidx = sdfdocopy = 0;
    yylineno = 1;	/* reset the line number for yyerror() */
    sdffilepos = -1;	/* reset file position (for subsequent use with fseek) */

    if (yyparse () == 0) {
	if (sdfcopytheinput) {
	    /* I already forgot where this is for... better not remove it ! */
	    sdfdodelayedcopy (0);	  /* flush */
	    putc ('\n', sdfcopystream);
	}
	return (0); /* 0 means OK */
    }
    return (1);
}

/* This one assumes that you've already seen a '(' and
 * now want to skip the remaining part of the expression.
 */
PRIVATE void skipthisthing ()
{
    int bracecount = 1, sometoken;

    while (bracecount > 0)
	if ((sometoken = yylex ()) == LBRTOKEN)
	    ++bracecount;
	else if (sometoken == RBR)
	    --bracecount;
}

/* This one attempts to copy the S-expression in the input stream
 * and return this copy. Doesn't do a very good job -- might be
 * fixed in a future release.
 */
PRIVATE char *copythisthing ()
{
    char *s;
    int bracecount = 1, sometoken, len, j = 0;

    while (bracecount > 0)
    {
	if ((sometoken = yylex ()) == LBRTOKEN) {
	    if (j >= MAXNAMELEN) goto err;
	    sdftmpstring[j++] = '(';
	    ++bracecount;
	}
	else if (sometoken == RBR) {
	    if (--bracecount > 0) { /* don't copy the last ')' */
		if (j >= MAXNAMELEN) goto err;
		sdftmpstring[j++] = ')';
	    }
	}
	else {
	    s = sometoken == STRNG ? yylval.str : "#keyword#";
	    len = strlen (s);
	    if (j + len > MAXNAMELEN) goto err;
	    strcpy (sdftmpstring + j, s); j += len;
	}
	if (bracecount) {
	    if (j >= MAXNAMELEN) goto err;
	    sdftmpstring[j++] = ' ';
	}
    }
    /* get rid of trailing space */
    while (--j > 0 && sdftmpstring[j] == ' ') ;
    sdftmpstring[j+1] = '\0';
    return (sdftmpstring);
err:
    sdfreport (Fatal, "%s (Seadif parser): buf overflow\nTry line %d.", FilNam, yylineno);
    return (sdftmpstring);
}

PRIVATE void checkthatalllayhasbeenwritten (int thingsstilltobewritten)
{
    int oldspacing;
    fprintf (stderr, "checkthatalllayhasbeenwritten %x %x %x\n",
	thingsstilltobewritten, SDFLAYPORT, SDFLAYLABEL);
    /* need to write things not present in the current layout */
    oldspacing = setdumpspacing (0); /* save spaces in the scratch file */
    if (thingsstilltobewritten & SDF_X_LAYALIAS)
	dump_alias (sdfcopystream, sdflayalias (
		sdfwritethislay->name,
		sdfwritethislay->circuit->name,
		sdfwritethislay->circuit->function->name,
		sdfwritethislay->circuit->function->library->name));
    if (thingsstilltobewritten & SDFLAYSTAT && sdfwritethislay->status)
	dump_status (sdfcopystream, sdfwritethislay->status);
    if (thingsstilltobewritten & SDFLAYOFF && sdfwritethislay)
	dump_off (sdfcopystream, sdfwritethislay->off);
    if (thingsstilltobewritten & SDFLAYBBX && sdfwritethislay)
	dump_bbx (sdfcopystream, sdfwritethislay->bbx);
    if (thingsstilltobewritten & SDFLAYPORT && sdfwritethislay)
	dump_layportlist (sdfcopystream, sdfwritethislay->layport);
    if (thingsstilltobewritten & SDFLAYLABEL && sdfwritethislay)
	dump_laylabellist (sdfcopystream, sdfwritethislay->laylabel);
    if (thingsstilltobewritten & SDFLAYSLICE && sdfwritethislay->slice)
	dump_slice (sdfcopystream, sdfwritethislay->slice);
    if (thingsstilltobewritten & SDFLAYWIRE && sdfwritethislay->wire)
	dump_wirelist (sdfcopystream, sdfwritethislay->wire);
    setdumpspacing (oldspacing);
}

PRIVATE void checkthatallcirhasbeenwritten (int thingsstilltobewritten)
{
    /* need to write things not present in the current circuit */
    int oldspacing = setdumpspacing (0); /* save spaces in the scratch file */
    if (thingsstilltobewritten & SDF_X_CIRALIAS)
	dump_alias (sdfcopystream, sdfciralias (
		sdfwritethiscir->name,
		sdfwritethiscir->function->name,
		sdfwritethiscir->function->library->name));
    if (thingsstilltobewritten & SDFCIRSTAT && sdfwritethiscir->status)
	dump_status (sdfcopystream, sdfwritethiscir->status);
    if (thingsstilltobewritten & SDFCIRPORT && sdfwritethiscir)
	dump_cirportlist (sdfcopystream, sdfwritethiscir->cirport);
    if (thingsstilltobewritten & SDFCIRINST && sdfwritethiscir->cirinst)
	dump_cirinst (sdfcopystream, sdfwritethiscir->cirinst);
    if (thingsstilltobewritten & SDFCIRNETLIST && sdfwritethiscir->netlist)
	dump_netlist (sdfcopystream, sdfwritethiscir->netlist);
    if (thingsstilltobewritten & SDFCIRBUS && sdfwritethiscir->buslist)
	dump_buslist (sdfcopystream, sdfwritethiscir->buslist);
    if (thingsstilltobewritten & SDFCIRTM && sdfwritethiscir->timing)
	dump_timing (sdfcopystream, sdfwritethiscir->timing);
    setdumpspacing (oldspacing);
}

PRIVATE void checkthatallfunhasbeenwritten (int thingsstilltobewritten)
{
    /* need to write things not present in the current function */
    int oldspacing = setdumpspacing (0); /* save spaces in the scratch file */
    if (thingsstilltobewritten & SDF_X_FUNALIAS)
	dump_alias (sdfcopystream, sdffunalias (
		sdfwritethisfun->name,
		sdfwritethisfun->library->name));
    if (thingsstilltobewritten & SDFFUNSTAT && sdfwritethisfun->status)
	dump_status (sdfcopystream, sdfwritethisfun->status);
    if (thingsstilltobewritten & SDFFUNTYPE && sdfwritethisfun->type)
	dump_funtype (sdfcopystream, sdfwritethisfun->type);
    setdumpspacing (oldspacing);
}

PRIVATE void checkthatalllibhasbeenwritten (int thingsstilltobewritten)
{
    /* need to write things not present in the current function */
    int oldspacing = setdumpspacing (0); /* save spaces in the scratch file */
    if (thingsstilltobewritten & SDF_X_LIBALIAS)
	dump_alias (sdfcopystream, sdflibalias (sdfwritethislib->name));
    if (thingsstilltobewritten & SDFLIBSTAT && sdfwritethislib->status)
	dump_status (sdfcopystream, sdfwritethislib->status);
    setdumpspacing (oldspacing);
}

int nextchar (FILEPTR stream)
{
    int c = getc (stream);
    ungetc (c, stream);
    return (c);
}

#ifdef __cplusplus_xxx

/* We defined the folowing 2 macros in seadif.l, which is included above as
 * "flex.seadif.c". This time make sure we get the REAL free() and malloc()
 * from the standard C library, and NOT recursive calls to cplusplusfree()
 * and to cplusplusmalloc()...
 */
#undef malloc	/* #define'd as cplusplusmalloc(x) */
#undef free	/* #define'd as cplusplusfree(x) */

void cplusplusfree (char *p)
{
    free (p);
}

char *cplusplusmalloc (unsigned n)
{
    return (char *)malloc (n);
}

#endif /* __cplusplus */

int yywrap (void)
{
    return 1;
}

int sdfstrcasecmp (char *s1, char *s2)
{
   while (*s1 && *s2) {
	if (tolower (*s1) != tolower (*s2)) break;
	++s1;
	++s2;
   }
   return (int)(*s1 - *s2);
}
