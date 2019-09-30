%{
/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
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

#include "func_parse.h"  // <- This is the output of bison.
#include "src/cfun/func_parsedefs.h"

#ifdef FLEX_SCANNER
int yylineno;
int getNLcnt (char *s);
#define YYINCR(_count) yylineno += _count /* have to do this ourselves ... */
#define GETNLCNT(_s)    getNLcnt (_s)
#else
#define YYINCR(_x)
#define GETNLCNT(_s)
#endif

#define LEOF 0		/* returned by input() on EOF */

%}

D	[0-9]
E	[Ee][-+]?{D}+

%S  FUNCT STAT RTNCLL RREXPR BLOCK BTFUN BWFUN
%p  4000

%%
	int c;
	char inyy[BUFSIZ];
	static int blflag = 0;	/* is it in a block ?                     */
	static int brprs = 0;	/* bracket pairs used in a block          */
	static int rrexpr = 0;	/* number of routines in an expression    */
	static int RTNpr = 0;	/* number of opened routines              */
	static int BTprprs = 0;	/* number of bracket pairs in BT function */
	static int BWprprs = 0;	/* number of bracket pairs in BW function */
	static int BTcalls = 0;	/* number of calls of BT function         */
	static int BWcalls = 0;	/* number of calls of BW function         */

"function"			{ BEGIN FUNCT;
				  return (FUNCTION); }
"state"				{ BEGIN STAT;
				  return (STATE); }
"load"[ \t\n]*"{"		{ BEGIN BLOCK;
				  blflag = 1;
				  adm_bsalloc_flag = 0;
				  brprs++;
				  YYINCR (GETNLCNT (yytext));
				  return (LOADL); }
"initial"[ \t\n]*"{"		{ BEGIN BLOCK;
				  blflag = 1;
				  adm_bsalloc_flag = 0;
				  brprs++;
				  YYINCR (GETNLCNT (yytext));
				  return (INITL); }
"behavior"[ \t\n]*"{"		{ BEGIN BLOCK;
				  blflag = 1;
				  adm_bsalloc_flag = 0;
				  brprs++;
				  YYINCR (GETNLCNT (yytext));
				  return (BEHAVIOR); }
<FUNCT>"output"			{ return (OUTPUT); }
<FUNCT>"inout"			{ return (INOUT); }
<FUNCT>"input"			{ return (INPUT); }
<FUNCT>"inread"			{ return (INREAD); }
<FUNCT>"("			{ return (LPS); }
<FUNCT>;?[ \t\n]*")"		{ BEGIN 0;
				  YYINCR (GETNLCNT (yytext));
				  return (RPS); }
<STAT>"char"			{ return (CHAR); }
<STAT>"float"			{ return (FLOAT); }
<STAT>"double"			{ return (DOUBLE); }
<STAT>"int"			{ return (INT); }
<STAT>"{"			{ return (LCB); }
<STAT>"}"			{ BEGIN 0;
				  return (RCB); }
<STAT,FUNCT>";"			{ return (SEMICOLON); }
<STAT,FUNCT>","			{ return (COMMA); }

<RTNCLL>"NODE"			{ return (NODE); }
<RTNCLL>"VICIN"			{ return (VICIN); }
<RTNCLL>"MIN"			{ return (MIN); }
<RTNCLL>"MAX"			{ return (MAX); }

<RTNCLL,STAT,FUNCT>"["		{ return (LSB); }
<RTNCLL,STAT,FUNCT>"]"		{ return (RSB); }
<RTNCLL,STAT,FUNCT>[\t\n ]+	{
				  YYINCR (GETNLCNT (yytext));
				}
<RTNCLL,STAT,FUNCT>{D}+			{ yylval.sval = strsav (yytext);
				  	  return (INTEGER); }
<RTNCLL,STAT,FUNCT>[a-zA-Z][a-zA-Z0-9_]*	{ yylval.sval = strsav (yytext);
		  			  return (IDENTIFIER); }

<BLOCK,RREXPR>"statcap_val"	{ BEGIN RTNCLL;
				  return (STATCAP); }
<BLOCK,RREXPR>"dyncap_val"	{ BEGIN RTNCLL;
				  return (DYNCAP); }
<BLOCK,RREXPR>"cap_val"		{ BEGIN RTNCLL;
				  return (CAPVAL); }
<BLOCK>"delay"[ \t\n]*"("	{ BEGIN RTNCLL;
				  YYINCR (GETNLCNT (yytext));
				  ECHO;
				  return (DELAY); }
<BLOCK>"cap_add"[ \t\n]*"("	{ BEGIN RREXPR;
				  rrexpr++;
				  YYINCR (GETNLCNT (yytext));
				  ECHO;
				  return (CAPADD); }
<RTNCLL>("'r'"|"'f'"|"'b'")[ \t\n]*","	{ BEGIN RREXPR;
				  	  rrexpr++;
					  YYINCR (GETNLCNT (yytext));
			  		  ECHO;
					  fprintf (yyout, "(float)");
					    /* because a 0 as 2nd arg
					    arg might give troubles */
					  return (MODE); }
<RREXPR>"("			{ RTNpr++;
				  fprintf (yyout, "("); }
<RREXPR>")"			{ RTNpr--;
				  fprintf (yyout, ")"); }
<RREXPR>","			{ if (RTNpr == 0) {
					BEGIN RTNCLL;
					rrexpr--;
					return (COMMA);
				  }
				  else
				  	fprintf (yyout, ",");
				}
<RTNCLL>"("			{ return (LPS); }
<RTNCLL>","			{ return (COMMA); }
<RTNCLL>")"			{ if (rrexpr == 0)
					BEGIN BLOCK;
				  else
					BEGIN RREXPR;
				  return (RPS); }
<BLOCK>"{"			{
				  brprs++;
				  fprintf (yyout, "{");
			 	}

<BLOCK>"}"			{
				  brprs--;
				  if (brprs == 0) {
			    	      print_func_foot ();
				      BEGIN 0;
				      blflag = 0;
				  }
				  fprintf (yyout, "}");
				  if (brprs == 0) {
				      fprintf (yyout, "\n");
				      lineno (yylineno);
				  }
				}


\"				{  putc ('"', yyout);
				   for (c = input (); c != '"'; c = input ()) {
					if (c == '\\') {
					    putc (c, yyout);
					    c = input ();
					    if (c == '\n') YYINCR (1);
					}
					else if (c == '\n') {
					    YYINCR (1);
					    yylineno --;
					    yyerror ("newline in string");
					    yylineno ++;
					    break;
					}
					if (c == LEOF) {
					    yyerror ("unterminated string");
					    die (5, "", "");
					}
					putc (c, yyout);
				    }
				    putc (c, yyout);
				}

"/*"				{  ECHO;
				   for (;;) {
					if ((c = input ()) == '*') {
					    if (c == '\n') YYINCR (1);
					    putc (c, yyout);
					    if ((c = input ()) == '*') {
					    	unput (c);
					    }
					    else if (c != LEOF) {
					    	putc (c, yyout);
						if (c == '\n') YYINCR (1);
						if (c == '/')
					    	    break;
					    }
					}
					else if (c != LEOF) {
					    putc (c, yyout);
					}

					if (c == LEOF) {
					    yyerror ("unterminated comment");
					    die (5, "", "");
					    break;
					}
				    }
				}
^#include.*\n			{  ECHO;
				   YYINCR (1);
				   lineno (yylineno);
				}
"BTEXOR"			{  BEGIN BTFUN;
				   fprintf (yyout, "BTEXOR");
				   BTcalls++; }
"BTEXNOR"			{  BEGIN BTFUN;
				   fprintf (yyout, "BTEXNOR");
				   BTcalls++; }
"BTAND"				{  BEGIN BTFUN;
				   fprintf (yyout, "BTAND");
				   BTcalls++; }
"BTNAND"			{  BEGIN BTFUN;
				   fprintf (yyout, "BTNAND");
				   BTcalls++; }
"BTOR"				{  BEGIN BTFUN;
				   fprintf (yyout, "BTOR");
				   BTcalls++; }
"BTNOR"				{  BEGIN BTFUN;
				   fprintf (yyout, "BTNOR");
				   BTcalls++; }
"BWEXOR"			{  BEGIN BWFUN;
				   fprintf (yyout, "BWEXOR");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWEXNOR"			{  BEGIN BWFUN;
				   fprintf (yyout, "BWEXNOR");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWAND"				{  BEGIN BWFUN;
				   fprintf (yyout, "BWAND");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWNAND"			{  BEGIN BWFUN;
				   fprintf (yyout, "BWNAND");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWOR"				{  BEGIN BWFUN;
				   fprintf (yyout, "BWOR");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWNOR"				{  BEGIN BWFUN;
				   fprintf (yyout, "BWNOR");
				   adm_bsalloc_flag = 1;
				   BWcalls++; }
"BWINVERT"			{  adm_bsalloc_flag = 1;
				   fprintf (yyout, "BWINVERT"); }
"BSROTATE"			{  adm_bsalloc_flag = 1;
				   fprintf (yyout, "BSROTATE"); }
"ITOBS"				{  adm_bsalloc_flag = 1;
				   fprintf (yyout, "ITOBS"); }
"ITOTC"				{  adm_bsalloc_flag = 1;
				   fprintf (yyout, "ITOTC"); }
<BTFUN>"("			{
				  BTprprs++;
				  fprintf (yyout, "(");
			 	}

<BTFUN>")"			{
				  if ((BTprprs-BTcalls) == 0) {
				      BTcalls--;
				      if (BTcalls != 0)
					  BEGIN BTFUN;
				      else if (blflag == 1)
					  BEGIN BLOCK;
				      else
				      	  BEGIN 0;
				      fprintf (yyout, ", '@' )");
				  }
				  else
				      fprintf (yyout, ")");
				  BTprprs--;
				}

<BWFUN>"("			{
				  BWprprs++;
				  fprintf (yyout, "(");
			 	}

<BWFUN>")"			{
				  if ((BWprprs-BWcalls) == 0) {
				      BWcalls--;
				      if (BWcalls != 0)
					  BEGIN BWFUN;
				      else if (blflag == 1)
					  BEGIN BLOCK;
				      else
				      	  BEGIN 0;
				      fprintf (yyout, ", \"ENDVAR\" )");
				  }
				  else
				      fprintf (yyout, ")");
				  BWprprs--;
				}
^#[ \t]*{D}+			{ int i;
				  for (i = 1; yytext[i] == ' ' &&
					      yytext[i] == '\t'; i++);
				  yylineno = atoi (yytext + i) - 1;
				  for (i = 0;
				      (inyy[i] = input ()) != '\n'
						      && inyy[i] != EOF; i++);
				  if (inyy[i] == '\n') YYINCR (1);
				  inyy[i] = '\0';
				  fprintf (yyout, "#line %d%s\n",
				      yylineno + 1, inyy);

				}
[a-zA-Z][a-zA-Z0-9_]*		{ ECHO; }

[\n]+				{ ECHO;
				  YYINCR(yyleng);
				}
%%

#ifdef FLEX_SCANNER
int getNLcnt (char *s)
{
    char *p = s;
    int i = 0;

    while (*p)
	if (*p++ == '\n') i++;

    return (i);
}
#endif
