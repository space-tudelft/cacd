%{
/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
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
#include "spipar.h"
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

extern int behind_header;  /* used for setting initial condition:
			      only if behind_header = 1, a name starting
			      with a 'r' should be recognized as a resistor */
extern int toppresent;

%}

%option yylineno

D	[0-9]
E	[DEde][-+]?{D}+

%START NL

%%
[*;]			{
			    char thechar;

			    while ((thechar = input()) != '\n') {
				if (thechar == EOF || thechar == 0) return(0);
			    }
			    unput ('\n');
			}
"#"[^\n]*\n             { YYINCR (1);
                          if (behind_header)
			      BEGIN NL;
			  parseLineStmt ((char *) yytext);
		        }
".subckt"		{ BEGIN 0; if (!toppresent) behind_header = 1;
			  return(SUBCKT); }
".SUBCKT"		{ BEGIN 0; if (!toppresent) behind_header = 1;
			  return(SUBCKT); }
".ends"			{ BEGIN 0; if (!toppresent) behind_header = 0;
			  return(ENDS); }
".ENDS"			{ BEGIN 0; if (!toppresent) behind_header = 0;
			  return(ENDS); }
".end"			{ BEGIN 0; return(END); }
".END"			{ BEGIN 0; return(END); }
".connect"		{ BEGIN 0; return(CNET); }
".CONNECT"		{ BEGIN 0; return(CNET); }
"."			{ BEGIN 0; return(PERIOD); }
"."[a-zA-Z0-9_]+	{ BEGIN 0; return(OTHER_CARD_BEGIN); }
"="			{ BEGIN 0; return(ASSIGN); }
"("			{ BEGIN 0; return(LPS); }
")"			{ BEGIN 0; return(RPS); }
<NL>vnet[a-zA-Z0-9_]*	{ BEGIN 0; return(VNET); }
<NL>VNET[a-zA-Z0-9_]*	{ BEGIN 0; return(VNET); }
<NL>rnet[a-zA-Z0-9_]*	{ BEGIN 0; return(RNET); }
<NL>RNET[a-zA-Z0-9_]*	{ BEGIN 0; return(RNET); }
<NL>[mM][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(TOR_INST); }
<NL>[jJqQ][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(JTOR_INST); }
<NL>[xX][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(NETW_INST); }
<NL>[rR][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(RES_INST); }
<NL>[cC][a-zA-Z0-9_]*	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(CAP_INST); }
[wW]			{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(T_WIDTH); }
[lL]			{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(T_LENGTH); }
[mM]			{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(T_SCALEF); }
[a-zA-Z$_][a-zA-Z0-9$_\077]*	{ yylval.sval = (char *) yytext;
			  if (yytext[yyleng - 1] == '?') yytext[--yyleng] = 0;
			  BEGIN 0; return(IDENTIFIER); }
"-"?{D}+		{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(INTEGER); }
"-"?{D}+("."{D}+)?({E})?([a-zA-Z_]*)?	{ yylval.sval = (char *) yytext;
			  BEGIN 0; return(FLOAT); }
"\n+"                   { YYINCR (1);
			  /* this is a line (card) continuation. */
			}
[\r\t ]+		{ BEGIN 0; }
\n			{ if (behind_header) BEGIN NL;
			  YYINCR (1);
			  return (NEWLINE); }
[^*^=^.^(^)^ ^\n^\t^+^-]+	{ BEGIN 0; return(OTHER); }

%%

int Input ()
{
    return (input ());
}

int resynch (int c)
{
    int c1,
        newline = 0;

    fprintf(stderr," \"");
    if((char *)yytext)
        fprintf(stderr,"%s",yytext);
    while((c1 = Input()) != c && c1 != 0 && c1 != EOF)
    {
        if(!(newline = (newline || ((c1=='\n')?1:0))))
        fprintf(stderr,"%c",c1);
    }

    fprintf(stderr,"\"\n");

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
