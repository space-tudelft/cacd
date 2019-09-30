%{

/*
 * ISC License
 *
 * Copyright (C) 1994-2015 by
 *	Frederik Beeftink
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/xspice/incl.h"
#include "parse.h"

#ifdef FLEX_SCANNER /* if this is GNU flex in stead of regular lex */
int yylineno;
#define YYINCR(_count) yylineno += _count /* flex does not do this for us */
#else
#define YYINCR(_x)
#endif

%}

%%
"#"			{ while (input () != '\n'); YYINCR(1); }
"-"			{ return (MINUS); }
"="			{ return (EQUAL); }
"*"			{ return (MULT); }
"/"			{ return (DIV); }
"^"                     { return (POW); }
"("			{ return (LPS); }
")"			{ return (RPS); }
":"			{ return (COLON); }
"+"			{ return (PLUS); }
"$"                     { return (DOLLAR); }
","			{ return (COMMA); }
";"			{ return (SEMICOLON); }
"unity"			{ return (UNITY); }
"model"			{ return (MODEL); }
[a-zA-Z_][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  return (IDENTIFIER);
			}
("-")?[0-9]+("."[0-9]+)? {
			  yylval.sval = (char *) yytext;
			  return (DOUBLE);
			}
("-")?[0-9]+("."[0-9]+)?[deDE][-+]?[0-9]+ {
			  yylval.sval = (char *) yytext;
			  return (DOUBLE);
			}
("-")?[0-9]+("."[0-9]+)?[afpnumkMGTP] {
			  yylval.sval = (char *) yytext;
			  return (DOUBLE);
			}
("-")?[0-9]+("."[0-9]+)?[deDE][-+]?[0-9]+[a-zA-Z_]+ {
			  yylval.sval = (char *) yytext;
			  return (DOUBLE2);
			}
("-")?[0-9]+("."[0-9]+)?[a-zA-Z_]+ {
			  yylval.sval = (char *) yytext;
			  return (DOUBLE2);
			}
[\r\t ]+	        ;
[\n]+                   YYINCR(yyleng);
.			{ return (ILLCHAR); }

%%

void scanrestart (FILE *input_file, int nr)
{
    yyin = input_file;
#ifdef FLEX_SCANNER
    if (nr > 0) yyrestart (yyin);
#endif
    yylineno = 1;
    yyparse ();
    fclose (yyin);
}
