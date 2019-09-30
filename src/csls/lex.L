%{
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"
#include "parse.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

void scanrestart (FILE *input_file);

#ifdef FLEX_SCANNER
#define input yyinput
#define YYINCR(_count) yylineno += _count /* have to do this ourselves ... */
#else
#ifndef input
#define input lex_input
#endif
#define YYINCR(_x)
#endif

%}

%option yylineno

D	[0-9]
E	[DEde][-+]?{D}+

%%
"/*"			{ char thechar;
			  loop:
				while((thechar = input()) != '*')
				{
				    if (thechar == EOF || thechar == 0)
					return(0);
				    if (thechar == '\n') YYINCR (1);
				}
				switch(input())
				{
				    case EOF:
				    case 0: return(0);
				    case '/': break;
				    case '*': unput('*'); goto loop;
				    case '\n': YYINCR (1);
				    default : goto loop;
				}
			}
"#"[^\n]*\n 		{ YYINCR (1);
			  parseLineStmt ((char *) yytext);
			}
"network"		{ return(NETWORK); }
"extern"		{ return(EXTERNSPEC); }
"terminal"		{ return(TERMINAL); }
"net"			{ return(NET)     ; }
"nenh"			{ return(NENH); }
"penh"			{ return(PENH); }
"ndep"			{ return(NDEP); }
"res"			{ return(RES)     ; }
"cap"			{ return(CAP)     ; }
"w"			{ return(T_WIDTH) ; }
"l"			{ return(T_LENGTH); }
"tr"			{ return(T_RISE); }
"tf"			{ return(T_FALL); }
".."			{ return(DOTDOT)  ; }
"="			{ return(ASSIGN)  ; }
"("			{ return(LPS)     ; }
")"			{ return(RPS)     ; }
"["			{ return(LSB)     ; }
"]"			{ return(RSB)     ; }
"{"			{ return(LCB)     ; }
"}"			{ return(RCB)     ; }
","			{ return(COMMA)   ; }
";"			{ return(SEMICOLON); }
"."			{ return(PERIOD); }
"@"			{ return(ATSIGN); }
[a-zA-Z][a-zA-Z0-9_]*		{ yylval.sval = (char *) yytext;
			  return(IDENTIFIER); }
{D}+			{ yylval.sval = (char *) yytext;
			  return(INTEGER); }
{D}+("."{D}+)?({E})?[afpnumkMG]? { yylval.sval = (char *) yytext;
			  return(FLOAT); }
[\n]+			{ YYINCR(yyleng);}
[\r\t ]+		;

%%

int Input ()
{
    return (input ());
}

extern int almost_eos;

int resynch (int c)
{
    int c1,
        newline = 0;

    if (almost_eos) {
        fprintf (stderr, "   \";\" expected");
        if ((char *)yytext) fprintf (stderr, ", found \"%s\"\n", yytext);
        else fprintf (stderr, "\n");
        while((c1 = Input()) != c && c1 != 0 && c1 != EOF);
    }
    else {
        fprintf(stderr," \"");
        if((char *)yytext)
            fprintf(stderr,"%s",yytext);
        while((c1 = Input()) != c && c1 != 0 && c1 != EOF)
        {
            if(!(newline = (newline || ((c1=='\n')?1:0))))
            fprintf(stderr,"%c",c1);
        }

        fprintf(stderr,"\"\n");
    }

    return(c1 != 0 && c1 != EOF);
}

int yywrap ()
{
    return (1);
}

void scanrestart (FILE *input_file)
{
     yylineno = 1;
#ifdef FLEX_SCANNER
     /* We need to call yyrestart() on DEC Alpha to make it work. */
     yyrestart (input_file);
#endif
}

/* This one can be used in other files instead of yytext
   to overcome problems with the type definition for yytext
   (it maybe char *yytext or char yytext[]. */
char * textval ()
{
    return ((char *)yytext);
}
