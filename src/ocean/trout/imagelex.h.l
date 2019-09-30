%{
/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
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

/*
 * tokens of parser
 */

int ydlineno;

extern int NoUniversalFeed;

int ydwrap (void)
{
    return 1;
}

%}

%p 2500

%%
"Seadif"                { return(SEADIFTOKEN); }
"Library"               { return(LIBRARYTOKEN); }
"Technology"            { return(TECHNOLOGY); }
"DesignRules"           { return(TECHDESIGNRULES); }
"NumberOfLayers"        { return(TECHNROFLAYERS); }
"WireOrient"            { return(TECHWIREORIENT); }
"WireWidth"             { return(TECHWIREWIDTH); }
"WireMaskName"          { return(TECHWIREMASKNAME); }
"ViaMaskName"           { return(TECHVIAMASKNAME); }
"ViaCellName"           { return(TECHVIACELLNAME); }

"ImageDescription"      { return(IMAGEDESCRIPTION); }
"ChipDescription"       { return(CHIPDESCRIPTION); }
"ChipImageRef"          { return(CHIPIMAGEREF); }
"ChipSize"              { return(CHIPSIZE); }
"ChipRoutingArea"       { return(CHIPROUTINGAREA); }
"CircuitImage"          { return(CIRCUITIMAGE); }
"LayoutImage"           { return(LAYOUTIMAGE); }
"LayoutModelCall"       { return(LAYOUTMODELCALL) ; }
"LayoutImageRepitition" { return(LAYOUTIMAGEREPITITION); }
"LayoutImageRepetition" { return(LAYOUTIMAGEREPITITION); }
"LayoutImagePattern"    { return(LAYOUTIMAGEPATTERN); }
"LayoutImageLayer"      { return(LAYOUTIMAGEPATTERNLAYER); }
"Rectangle"             { return(RECTANGLE); }
"GridImage"             { return(GRIDIMAGE); }
"GridSize"              { return(GRIDSIZE); }
"GridMapping"           { return(GRIDMAPPING); }
"TearLine"              { return(GRIDTEARLINE); }
"PowerLine"             { return(GRIDPOWERLINE); }
"GridConnectList"       { return(GRIDCONNECTLIST); }
"Block"                 { return(GRIDBLOCK); }
"RestrictedFeed"        { return(RESTRICTEDFEED); }
"UniversalFeed"         { if (NoUniversalFeed) return(RESTRICTEDFEED); else return(UNIVERSALFEED); }
"Feed"                  { return(FEED); }
"GridCost"              { return(GRIDCOST); }
"FeedCost"              { return(FEEDCOST); }
"GridCostList"          { return(GRIDCOSTLIST); }
"Status"                { return(STATUSTOKEN); }
"Written"               { return(WRITTEN); }
"TimeStamp"             { return(TIMESTAMP); }
"Author"                { return(AUTHOR); }
"Program"               { return(PROGRAM); }
"Comment"               { return(COMMENT); }
"("                     { return(LBR); }
")"                     { return(RBR); }
"/*"                    { /* comment skipper */
			    char c; /* for flex: must count line numbers */
loop:
			    while ((c = input ()) != '*') if (c == '\n') ydlineno++;

			    switch (input ()) {
				case '/' : break;
				case '*' : unput ('*'); goto loop;
				case '\n': ydlineno++;
				default  : goto loop;
			    }
			}
\"                      {
			    int strpt = 0;
			    char inkar;

			    while ((inkar = input ()) != '"')
			    switch (inkar) {
			    case '\n':
				fprintf (stderr, "WARNING (seadif): newline in string (line %d)\n", ydlineno);
				break;
			    case '\\':
				inkar = input (); /* read next character */
			    default:
				ydlval.str[strpt++] = inkar;
				break;
			    }
			    ydlval.str[strpt] = '\0';
			    return (STRINGTOKEN);
			}
[A-Za-z][A-Za-z0-9_]*	{ strcpy (ydlval.str, ydtext); return (STRINGTOKEN); }
"%"[0-9][A-Za-z0-9_]*	{ strcpy (ydlval.str, ydtext + 1); return (STRINGTOKEN); } /* remove precent */
"-"?[0-9]+		{ strcpy (ydlval.str, ydtext); return (NUMBER); }
"-"?[0-9]*"."[0-9]+	{ strcpy (ydlval.str, ydtext); return (NUMBER); }
[\n]			{ ++ydlineno; }
.			;
