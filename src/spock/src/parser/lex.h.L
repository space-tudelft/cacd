%{
/*
 * ISC License
 *
 * Copyright (C) 2000-2018 by
 *	Xander Burgerhout
 *	Simon de Graaf
 *	N.P. van der Meijs
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
/******************************************************************************
 *  Name        : lex.l
 *
 *  Purpose     : lexical analyser
 *
 *  Author      : Xander Burgerhout
 *
 *  Date        : Jul 05, 2000
 ******************************************************************************/

#define YYINCR(_count) yylineno += _count
#define input yyinput

#include <string>

string string_parse_buffer;

//int yylineno = 1;
int yyplines = 0;

%}

%option yylineno
%option noyywrap
%x STRINGST

D       [0-9]

%%
"/*"        { int c;
              loop:
                   while ((c = input()) != '*') {
                        if (c == EOF || c == 0) return(0);
                        if (c == '\n') YYINCR (1);
                   }
                   switch (input()) {
                        case EOF:
                        case 0: return(0);
                        case '/': break;
                        case '*': unput('*'); goto loop;
                        case '\n': YYINCR (1);
                        default : goto loop;
                   }
            }

"VERSION"	{ return VERSION; }
"ICONPATH"	{ return ICONPATH; }

"define"	{ return DEFINE; }

"real"		|
"integer"	|
"identifier"	|
"conditionlist"	|
"string"	|
"color"		|
"dalistyle"	|
"item"		|
"dropdown"	|
"combobox"	|
"spreadsheet"	|
"section"	|
"scrollframe"	|
"tabpage"	|
"paramlist"             {
                            yylval.strval = new string(yytext);
                            return COMPONENT_TYPE;
                        }

"title"		|
"hint"		|
"default"	|
"align"		|
"filename"	|
"adddatafrom"	|
"unit"		|
"pixmap"	|
"option"	|
"validator"             {
                            yylval.strval = new string(yytext);
                            return PROPERTY;
                        }

"generator"             {
			    if (!skip_generators) return GENERATOR;
			    fprintf(stderr, "skipping_generators!");
			    return 0; // EOF
			}
"generate"              { return GENERATE; }
"map"                   { return MAP; }
"foreach"               { return FOREACH; }
"-inf"			{ return M_INFINITY; }
"+inf"			{ return P_INFINITY; }
"inf"			{ return P_INFINITY; }
"application"           { return APPLICATION; }
"if"                    { return IF; }
"else"                  { return ELSE; }

[a-zA-Z_]+[a-zA-Z0-9_]* {
                            yylval.strval = new string(yytext);
                            return (IDENTIFIER);
                        }
"$"                     { return DOLLAR; }
"."                     { return MEMBER; }
":"                     { return COLON; }
";"                     { return SCOLON; }
","                     { return COMMA; }
"//"                    { while (input () != '\n'); YYINCR(1); }
"{"                     { return LCB; }
"}"                     { return RCB; }
"["                     { return LSB; }
"]"                     { return RSB; }
"("                     { return LPS; }
")"                     { return RPS; }
"<"                     { return LT; }
">"                     { return GT; }
"&&"                    { return AND; }
"||"                    { return OR; }
"="                     { return EQ; }
"=="                    { return EQEQ; }
"!"                     { return NOT; }
"!="                    { return NOTEQ; }
"+"                     { return PLUS; }
"-"                     { return MINUS; }

\"                      {
                            BEGIN STRINGST;
                            string_parse_buffer = "";
                        }
<STRINGST>\\n           { string_parse_buffer += "\n";}
<STRINGST>\\t           { string_parse_buffer += "\t";}
<STRINGST>\\\"          { string_parse_buffer += "\"";}
<STRINGST>\"            {
                            yylval.strval = new string(string_parse_buffer);
                            BEGIN 0;
                            return STRING;
                        }
<STRINGST>\n            { fprintf(stderr, "WARNING: string spanning multiple lines!\n"); }
<STRINGST>.             { string_parse_buffer += *yytext; }

[\n]			{ YYINCR(yyleng);
			  if (!(++yyplines % 10)) fprintf(stderr, ".");
                        }
[\t ]+                  ;

"-"?{D}+                {
                            yylval.strval = new string(yytext);
                            return (INTEGER);
                        }
"-"?{D}+"."?{D}*([Ee]"-"?{D}+)? |
"-"?{D}*"."?{D}+([Ee]"-"?{D}+)? {
                            yylval.strval = new string(yytext);
                            return (DOUBLE);
                        }
%%
