%{
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
/*
 * tokens of parser
 */

//int pplineno;

#ifdef __cplusplus
#define input() ppinput()
#endif

#undef  YY_FATAL_ERROR
#define YY_FATAL_ERROR(msg) \
do { \
    cerr << msg << "\n" << flush; \
    exit(1); \
} while (0)

#ifdef __linux /* something strange going on in stdio.h ... */
#undef stdin
#undef stdout
#endif

%}

%p 2500

%%
"Seadif"                { return SEADIFTOKEN; }
"ImageDescription"      { return IMAGEDESCRIPTION; }
"GridImage"             { return GRIDIMAGE;}
"GridConnectList"       { return GRIDCONNECTLIST; }
"RestrictedFeed"        { return RESTRICTEDFEED; }
"UniversalFeed"         { return UNIVERSALFEED; }
"ExternalFeed"          { return EXTERNALFEED; }
"BusFeed"               { return BUSFEED; }
"Feed"                  { return FEED; }
"Axis"                  { return AXIS; }
"MirrorAxis"            { return MIRRORAXIS; }
"GridSize"              { return GRIDSIZE;}
"ImageOverlap"          { return IMAGEOVERLAP; }
"Technology"            { return TECHNOLOGY; }
"DesignRules"           { return TECHDESIGNRULES; }
"NumberOfLayers"        { return TECHNROFLAYERS; }
"WireOrient"            { return TECHWIREORIENT; }
"EstimatedTransparency" { return ESTIMATEDTRANSPARENCY; }
"PowerLine"             { return POWERLINE_TOK; }

"Comment"               { return COMMENT; }
"("                     { return LBR; }
")"                     { return RBR; }
"/*"                    { /* comment skipper */
			    char c; /* for flex: must count line numbers */
loop:
			    while ((c = input ()) != '*') if (c == '\n') pplineno++;

			    switch (input ()) {
				case '/' : break;
				case '*' : unput ('*'); goto loop;
				case '\n': pplineno++;
				default  : goto loop;
			    }
			}
\"                      {
			    int strpt = 0;
			    char inkar;

			    while ((inkar = input ()) != '"')
			    switch (inkar) {
			    case '\n':
				cerr << "WARNING (seadif): newline in string (line " << pplineno << ")\n" << flush;
				break;
			    case '\\':
				inkar = input (); /* read next character */
			    default:
				pplval.str[strpt++] = inkar;
				break;
			    }
			    pplval.str[strpt] = '\0';
			    return (STRINGTOKEN);
			}
[A-Za-z][A-Za-z0-9_]*	{ strcpy (pplval.str, (char*)pptext); return (STRINGTOKEN); }
"%"[0-9][A-Za-z0-9_]*	{ strcpy (pplval.str, (char*)pptext + 1); return (STRINGTOKEN); } /* remove precent */
"-"?[0-9]+		{ strcpy (pplval.str, (char*)pptext); return (NUMBER); }
"-"?[0-9]*"."[0-9]+	{ strcpy (pplval.str, (char*)pptext); return (NUMBER); }
[\n]			{ ++pplineno; }
.			;
