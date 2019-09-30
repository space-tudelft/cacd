%{
/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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
 * Tokens of parser.
 */

#ifdef FLEX_SCANNER
int ydlineno; /* not defined in Flex, but already defined in Lex */
#endif

#if defined(__cplusplus) && defined(FLEX_SCANNER)
#define INPUT_CHAR() ydinput()
#else
#define INPUT_CHAR() input()
#endif

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
"DummyMaskName"         { return(TECHDUMMYMASKNAME); }
"ViaMaskName"           { return(TECHVIAMASKNAME); }
"ViaCellName"           { return(TECHVIACELLNAME); }

"ImageDescription"      { return(IMAGEDESCRIPTION); }
"CircuitImage"          { return(CIRCUITIMAGE); }
"LayoutImage"           { return(LAYOUTIMAGE); }
"ChipDescription"       { return(CHIPDESCRIPTION); }
"ChipImageRef"          { return(CHIPIMAGEREF); }
"ChipSize"              { return(CHIPSIZE); }
"LayoutModelCall"       { return(LAYOUTMODELCALL) ; }
"LayoutImageRepitition" { /* spelling error for backward compatibility ...: */
                          return(LAYOUTIMAGEREPETITION); }
"LayoutImageRepetition" { return(LAYOUTIMAGEREPETITION); }
"LayoutImagePattern"    { return(LAYOUTIMAGEPATTERN); }
"LayoutImageLayer"      { return(LAYOUTIMAGEPATTERNLAYER); }
"Rectangle"             { return(RECTANGLE); }
"GridImage"             { return(GRIDIMAGE); }
"GridSize"              { return(GRIDSIZE); }
"GridMapping"           { return(GRIDMAPPING); }
"OverlayMapping"        { return(OVERLAYMAPPING); }
"GridConnectList"       { return(GRIDCONNECTLIST); }
"Block"                 { return(GRIDBLOCK); }
"RestrictedFeed"        { return(RESTRICTEDFEED); }
"UniversalFeed"         { return(UNIVERSALFEED); }
"Feed"                  { return(FEED); }
"GridCost"              { return(GRIDCOST); }
"GridCostList"          { return(GRIDCOSTLIST); }
"Status"                { return(STATUSTOKEN); }
"Written"               { return(WRITTEN); }
"TimeStamp"             { return(TIMESTAMP); }
"Author"                { return(AUTHOR); }
"Program"               { return(PROGRAM); }
"Comment"               { return(COMMENT); }
"SpiceParameters"       { return(SPICEPARAMETERS); }
"Parameter"             { return(PARAMETER); }
"Model"                 { return(MODEL); }
"("                     { return(LBR); }
")"                     { return(RBR); }
"/*"                    { /* comment skipper */
			    int c; /* for flex: must count line numbers */
loop:
                            while ((c = INPUT_CHAR()) != '*') if (c == '\n') ydlineno++;

			    switch (INPUT_CHAR()) {
				case '/' : break;
				case '*' : unput ('*'); goto loop;
				case '\n': ydlineno++;
				default  : goto loop;
                            }
			}
\"                      {
			    int strpt = 0;
			    int inkar;

			    while ((inkar = INPUT_CHAR()) != '"')
			    switch (inkar) {
			    case '\n':
				fprintf (stderr, "WARNING (seadif): newline in string (line %d)\n", ydlineno);
				break;
			    case '\\':
				inkar = INPUT_CHAR(); /* read next character */
			    default:
				ydlval.str[strpt++] = inkar;
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
