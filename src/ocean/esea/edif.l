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
 * This scanner parses EDIF 2.0.0 tokens.
 */

#include "src/ocean/esea/cedif.h"
#include "edif.h"
#include <ctype.h>		  /* for declaration of isprint() */

#ifdef FLEX_SCANNER
int ediflineno;
#define YYINCR(n) ediflineno += n
#else
#define YYINCR(n)
#endif

int edifwrap (void)
{
    return 1;
}

%}

%e 5000
%p 12000
%n 2500
%a 4000

%option caseless

%%
"ANGLE"                       { return ANGLE; }
"BEHAVIOR"                    { return BEHAVIOR; }
"CALCULATED"                  { return CALCULATED; }
"CAPACITANCE"                 { return CAPACITANCE; }
"CENTERCENTER"                { return CENTERCENTER; }
"CENTERLEFT"                  { return CENTERLEFT; }
"CENTERRIGHT"                 { return CENTERRIGHT; }
"CHARGE"                      { return CHARGE; }
"CONDUCTANCE"                 { return CONDUCTANCE; }
"CURRENT"                     { return CURRENT; }
"DISTANCE"                    { return DISTANCE; }
"DOCUMENT"                    { return DOCUMENT; }
"ENERGY"                      { return ENERGY; }
"EXTEND"                      { return EXTEND; }
"FLUX"                        { return FLUX; }
"FREQUENCY"                   { return FREQUENCY; }
"GENERIC"                     { return GENERIC; }
"GRAPHIC"                     { return GRAPHIC; }
"INDUCTANCE"                  { return INDUCTANCE; }
"INOUT"                       { return INOUT; }
"INPUT"                       { return INPUT; }
"LOGICMODEL"                  { return LOGICMODEL; }
"LOWERCENTER"                 { return LOWERCENTER; }
"LOWERLEFT"                   { return LOWERLEFT; }
"LOWERRIGHT"                  { return LOWERRIGHT; }
"MASKLAYOUT"                  { return MASKLAYOUT; }
"MASS"                        { return MASS; }
"MEASURED"                    { return MEASURED; }
"MX"                          { return MX; }
"MXR90"                       { return MXR90; }
"MY"                          { return MY; }
"MYR90"                       { return MYR90; }
"NETLIST"                     { return NETLIST; }
"OUTPUT"                      { return OUTPUT; }
"PCBLAYOUT"                   { return PCBLAYOUT; }
"POWER"                       { return POWER; }
"R0"                          { return R0; }
"R180"                        { return R180; }
"R270"                        { return R270; }
"R90"                         { return R90; }
"REQUIRED"                    { return REQUIRED; }
"RESISTANCE"                  { return RESISTANCE; }
"RIPPER"                      { return RIPPER; }
"ROUND"                       { return ROUND; }
"SCHEMATIC"                   { return SCHEMATIC; }
"STRANGER"                    { return STRANGER; }
"SYMBOLIC"                    { return SYMBOLIC; }
"TEMPERATURE"                 { return TEMPERATURE; }
"TIE"                         { return TIE; }
"TIME"                        { return TIME; }
"TRUNCATE"                    { return TRUNCATE; }
"UPPERCENTER"                 { return UPPERCENTER; }
"UPPERLEFT"                   { return UPPERLEFT; }
"UPPERRIGHT"                  { return UPPERRIGHT; }
"VOLTAGE"                     { return VOLTAGE; }
"acLoad"                      { return ACLOAD; }
"after"                       { return AFTER; }
"annotate"                    { return ANNOTATE; }
"apply"                       { return APPLY; }
"arc"                         { return ARC; }
"array"                       { return ARRAY; }
"arrayMacro"                  { return ARRAYMACRO; }
"arrayRelatedInfo"            { return ARRAYRELATEDINFO; }
"arraySite"                   { return ARRAYSITE; }
"atLeast"                     { return ATLEAST; }
"atMost"                      { return ATMOST; }
"author"                      { return AUTHOR; }
"baseArray"                   { return BASEARRAY; }
"becomes"                     { return BECOMES; }
"between"                     { return BETWEEN; }
"boolean"                     { return BOOLEAN; }
"booleanDisplay"              { return BOOLEANDISPLAY; }
"booleanMap"                  { return BOOLEANMAP; }
"borderPattern"               { return BORDERPATTERN; }
"borderWidth"                 { return BORDERWIDTH; }
"boundingBox"                 { return BOUNDINGBOX; }
"cell"                        { return CELL; }
"cellRef"                     { return CELLREF; }
"cellType"                    { return CELLTYPE; }
"change"                      { return CHANGE; }
"circle"                      { return CIRCLE; }
"color"                       { return COLOR; }
"comment"                     { return COMMENT; }
"commentGraphics"             { return COMMENTGRAPHICS; }
"compound"                    { return COMPOUND; }
"connectLocation"             { return CONNECTLOCATION; }
"contents"                    { return CONTENTS; }
"cornerType"                  { return CORNERTYPE; }
"criticality"                 { return CRITICALITY; }
"currentMap"                  { return CURRENTMAP; }
"curve"                       { return CURVE; }
"cycle"                       { return CYCLE; }
"dataOrigin"                  { return DATAORIGIN; }
"dcFaninLoad"                 { return DCFANINLOAD; }
"dcFanoutLoad"                { return DCFANOUTLOAD; }
"dcMaxFanin"                  { return DCMAXFANIN; }
"dcMaxFanout"                 { return DCMAXFANOUT; }
"delay"                       { return DELAY; }
"delta"                       { return DELTA; }
"derivation"                  { return DERIVATION; }
"design"                      { return DESIGN; }
"designator"                  { return DESIGNATOR; }
"difference"                  { return DIFFERENCE; }
"direction"                   { return DIRECTION; }
"display"                     { return DISPLAY; }
"dominates"                   { return DOMINATES; }
"dot"                         { return DOT; }
"duration"                    { return DURATION; }
"e"                           { return E_TOK; }
"edif"                        { return EDIF; }
"edifLevel"                   { return EDIFLEVEL; }
"edifVersion"                 { return EDIFVERSION; }
"enclosureDistance"           { return ENCLOSUREDISTANCE; }
"endType"                     { return ENDTYPE; }
"entry"                       { return ENTRY; }
"event"                       { return EVENT; }
"exactly"                     { return EXACTLY; }
"external"                    { return EXTERNAL; }
"fabricate"                   { return FABRICATE; }
"false"                       { return FALSE; }
"figure"                      { return FIGURE; }
"figureArea"                  { return FIGUREAREA; }
"figureGroup"                 { return FIGUREGROUP; }
"figureGroupObject"           { return FIGUREGROUPOBJECT; }
"figureGroupOverride"         { return FIGUREGROUPOVERRIDE; }
"figureGroupRef"              { return FIGUREGROUPREF; }
"figurePerimeter"             { return FIGUREPERIMETER; }
"figureWidth"                 { return FIGUREWIDTH; }
"fillPattern"                 { return FILLPATTERN; }
"follow"                      { return FOLLOW; }
"forbiddenEvent"              { return FORBIDDENEVENT; }
"globalPortref"               { return GLOBALPORTREF; }
"greaterThan"                 { return GREATERTHAN; }
"gridMap"                     { return GRIDMAP; }
"ignore"                      { return IGNORE; }
"includeFigureGroup"          { return INCLUDEFIGUREGROUP; }
"initial"                     { return INITIAL_TOK; }
"instance"                    { return INSTANCE; }
"instanceBackAnnotate"        { return INSTANCEBACKANNOTATE; }
"instanceGroup"               { return INSTANCEGROUP; }
"instanceMap"                 { return INSTANCEMAP; }
"instanceRef"                 { return INSTANCEREF; }
"integer"                     { return INTEGER; }
"integerDisplay"              { return INTEGERDISPLAY; }
"interface"                   { return INTERFACE; }
"interFigureGroupSpacing"     { return INTERFIGUREGROUPSPACING; }
"intersection"                { return INTERSECTION; }
"intraFigureGroupSpacing"     { return INTRAFIGUREGROUPSPACING; }
"inverse"                     { return INVERSE; }
"isolated"                    { return ISOLATED; }
"joined"                      { return JOINED; }
"justify"                     { return JUSTIFY; }
"keywordDisplay"              { return KEYWORDDISPLAY; }
"keywordLevel"                { return KEYWORDLEVEL; }
"keywordMap"                  { return KEYWORDMAP; }
"lessThan"                    { return LESSTHAN; }
"library"                     { return LIBRARY_TOK; }
"libraryRef"                  { return LIBRARYREF; }
"listofNets"                  { return LISTOFNETS; }
"listOfPorts"                 { return LISTOFPORTS; }
"loadDelay"                   { return LOADDELAY; }
"logicAssign"                 { return LOGICASSIGN; }
"logicInput"                  { return LOGICINPUT; }
"logicList"                   { return LOGICLIST; }
"logicMapinput"               { return LOGICMAPINPUT; }
"logicMapOutput"              { return LOGICMAPOUTPUT; }
"logicOneOf"                  { return LOGICONEOF; }
"logicOutput"                 { return LOGICOUTPUT; }
"logicPort"                   { return LOGICPORT; }
"logicRef"                    { return LOGICREF; }
"logicValue"                  { return LOGICVALUE; }
"logicWaveform"               { return LOGICWAVEFORM; }
"maintain"                    { return MAINTAIN; }
"match"                       { return MATCH; }
"member"                      { return MEMBER; }
"miNoMax"                     { return MINOMAX; }
"miNoMaxDisplay"              { return MINOMAXDISPLAY; }
"mnm"                         { return MNM; }
"multipleValueSet"            { return MULTIPLEVALUESET; }
"mustJoin"                    { return MUSTJOIN; }
"name"                        { return NAME; }
"net"                         { return NET_TOK; }
"netBackAnnotate"             { return NETBACKANNOTATE; }
"netBundle"                   { return NETBUNDLE; }
"netDelay"                    { return NETDELAY; }
"netGroup"                    { return NETGROUP; }
"netMap"                      { return NETMAP; }
"netRef"                      { return NETREF_TOK; }
"noChange"                    { return NOCHANGE; }
"nonPermutable"               { return NONPERMUTABLE; }
"notAllowed"                  { return NOTALLOWED; }
"notchSpacing"                { return NOTCHSPACING; }
"number"                      { return NUMBER; }
"numberDefinition"            { return NUMBERDEFINITION; }
"numberDisplay"               { return NUMBERDISPLAY; }
"offPageConnector"            { return OFFPAGECONNECTOR; }
"offsetEvent"                 { return OFFSETEVENT; }
"openShape"                   { return OPENSHAPE; }
"orientation"                 { return ORIENTATION; }
"origin"                      { return ORIGIN; }
"overhangDistance"            { return OVERHANGDISTANCE; }
"overlapDistance"             { return OVERLAPDISTANCE; }
"oversize"                    { return OVERSIZE; }
"owner"                       { return OWNER; }
"page"                        { return PAGE; }
"pageSize"                    { return PAGESIZE; }
"parameter"                   { return PARAMETER; }
"parameterAssign"             { return PARAMETERASSIGN; }
"parameterDisplay"            { return PARAMETERDISPLAY; }
"path"                        { return PATH; }
"pathDelay"                   { return PATHDELAY; }
"pathWidth"                   { return PATHWIDTH; }
"permutable"                  { return PERMUTABLE; }
"physicalDesignRule"          { return PHYSICALDESIGNRULE; }
"plug"                        { return PLUG; }
"point"                       { return POINT; }
"pointDisplay"                { return POINTDISPLAY; }
"pointList"                   { return POINTLIST; }
"polygon"                     { return POLYGON; }
"port"                        { return PORT; }
"portBackAnnotate"            { return PORTBACKANNOTATE; }
"portBundle"                  { return PORTBUNDLE; }
"portDelay"                   { return PORTDELAY; }
"portGroup"                   { return PORTGROUP; }
"portImplementation"          { return PORTIMPLEMENTATION; }
"portInstance"                { return PORTINSTANCE; }
"portList"                    { return PORTLIST; }
"portListAlias"               { return PORTLISTALIAS; }
"portMap"                     { return PORTMAP; }
"portRef"                     { return PORTREF; }
"program"                     { return PROGRAM; }
"property"                    { return PROPERTY; }
"propertyDisplay"             { return PROPERTYDISPLAY; }
"protectionFrame"             { return PROTECTIONFRAME; }
"pt"                          { return PT; }
"rangeVector"                 { return RANGEVECTOR; }
"rectangle"                   { return RECTANGLE; }
"rectangleSize"               { return RECTANGLESIZE; }
"rename"                      { return RENAME; }
"resolves"                    { return RESOLVES; }
"scale"                       { return SCALE; }
"scalex"                      { return SCALEX; }
"scaley"                      { return SCALEY; }
"section"                     { return SECTION; }
"shape"                       { return SHAPE; }
"simulate"                    { return SIMULATE; }
"simulationInfo"              { return SIMULATIONINFO; }
"singleValueSet"              { return SINGLEVALUESET; }
"site"                        { return SITE; }
"socket"                      { return SOCKET; }
"socketSet"                   { return SOCKETSET; }
"status"                      { return STATUS_TOK; }
"steady"                      { return STEADY; }
"string"                      { return STRING_TOK; }
"stringDisplay"               { return STRINGDISPLAY; }
"strong"                      { return STRONG; }
"symbol"                      { return SYMBOL; }
"symmetry"                    { return SYMMETRY; }
"table"                       { return TABLE; }
"tableDefault"                { return TABLEDEFAULT; }
"technology"                  { return TECHNOLOGY; }
"textheight"                  { return TEXTHEIGHT; }
"timeinterval"                { return TIMEINTERVAL; }
"timeStamp"                   { return TIMESTAMP; }
"timing"                      { return TIMING_TOK; }
"transform"                   { return TRANSFORM; }
"transition"                  { return TRANSITION; }
"trigger"                     { return TRIGGER; }
"true"                        { return TRUE_TOK; }
"unconstrained"               { return UNCONSTRAINED; }
"undefined"                   { return UNDEFINED; }
"union"                       { return UNION; }
"unit"                        { return UNIT; }
"unused"                      { return UNUSED; }
"userData"                    { return USERDATA; }
"version"                     { return VERSION; }
"view"                        { return VIEW; }
"viewList"                    { return VIEWLIST; }
"viewMap"                     { return VIEWMAP; }
"viewRef"                     { return VIEWREF; }
"viewType"                    { return VIEWTYPE; }
"visible"                     { return VISIBLE; }
"voltageMap"                  { return VOLTAGEMAP; }
"waveValue"                   { return WAVEVALUE; }
"weak"                        { return WEAK; }
"weakJoined"                  { return WEAKJOINED; }
"when"                        { return WHEN; }
"written"                     { return WRITTEN; }
"("                           { return LBR; }
")"                           { return RBR; }
[\n]                          { YYINCR(1); }
[A-Za-z][A-Za-z0-9_]*	      {
			      ediflval.str = cs(ediftext);
			      return STR;
			      }
"%"[0-9][A-Za-z0-9_]*	      {  /* escaped string which starts with percent */
			      ediflval.str = cs(ediftext+1);
			      return STR;
			      }
&[A-Za-z0-9_]+                {
			      ediflval.str = cs(ediftext+1);
			      return STR;
			      }
\"[^\"\n]*\"                  {
                              ediftext[edifleng - 1] = '\0'; /* strip last quote */
                              ediflval.str = cs(ediftext+1);
			      return STR;
                              }
"-"?[0-9]+		      {
			      ediflval.integer = atoi(ediftext);
			      return INT;
			      }
[ \t]+                        ;
.                             {
			      int thechar = *ediftext;
                              if (isprint(thechar)) /* is it printable ? */
				 report (eWarning, "line %d: funny character '%c' skipped", ediflineno, thechar);
			      else
				 report (eWarning, "line %d: funny character 0x%x skipped", ediflineno, thechar);
			      }
