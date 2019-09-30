%{
/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "cmd_y.h"

double slstof (void);
void yyerror (char *s);

#ifdef FLEX_SCANNER /* use GNU flex in stead of regular lex */
int yylineno;
#define YYINCR(_count) yylineno += _count /* have to do this ourselves ... */
#else
#define YYINCR(_x)
#endif

%}
E		[DEde][-+]?[0-9]+
%%
"/*"			{
			int thechar;
			loop:
			    while ((thechar = input ()) != '*') {
				if (thechar == EOF || thechar == 0) return (0);
				if (thechar == '\n') YYINCR (1);
			    }
			    switch (input ()) {
				case EOF :
				case 0 : return (0);
				case '/' : break;
				case '*' : unput ('*'); goto loop;
				case '\n': YYINCR (1);
				default : goto loop;
			    }
			}
"*%"			{
			int cntr = 0;
			int thechar;
			loop2:
			    while ((thechar = input ()) != '*') {
				if (thechar == EOF) return (0);
				if (thechar == '\n') YYINCR (1);
			    }
			    switch (input ()) {
				case 0 : return (0);
				case '%' : if (cntr == 1) break;
			                   else {
			                       cntr++;
			                       goto loop2;
                                           }
				case '*' : unput ('*'); goto loop2;
				case '\n': YYINCR (1);
				default : goto loop2;
			    }
			}
"set"			{ return (SET); }
"l"			{ yylval.ival = L_state; return (LOGIC_LEVEL); }
"h"			{ yylval.ival = H_state; return (LOGIC_LEVEL); }
"x"			{ yylval.ival = X_state; return (LOGIC_LEVEL); }
"f"			{ yylval.ival = Free_state; return (LOGIC_LEVEL); }
"~"			{ return (TILDE); }
"from"			{ return (FROM); }
"fill"			{ return (FILL); }
"with"			{ return (WITH); }
"print"			{ return (PRINT); }
"plot"			{ return (PLOT); }
"option"		{ return (OPTION); }
"simperiod"		{ return (SIMPERIOD); }
"disperiod"		{ return (DISPERIOD); }
"dissipation"		{ return (DISSIPATION); }
"level"			{ return (LEVEL); }
"dump"			{ return (DUMP); }
"at"			{ return (AT); }
"initialize"		{ return (INITIALIZE); }
"sigoffset"		{ return (SIGOFFSET); }
"races"			{ return (RACES); }
"devices"		{ return (DEVICES); }
"statistics"		{ return (STATISTICS); }
"only"			{ return (ONLY); }
"changes"		{ return (CHANGES); }
"process"		{ return (SLS_PROCESS); }
"sigunit"		{ return (SIGUNIT); }
"outunit"		{ return (OUTUNIT); }
"outacc"		{ return (OUTACC); }
"maxpagewidth" 		{ return (MAXPAGEWIDTH); }
"maxnvicin"		{ return (MAXNVICIN); }
"maxtvicin"		{ return (MAXTVICIN); }
"maxldepth"		{ return (MAXLDEPTH); }
"vh"			{ return (VH); }
"vmaxl"			{ return (VMAXL); }
"vminh"			{ return (VMINH); }
"tdevmin"		{ return (TDEVMIN); }
"tdevmax"		{ return (TDEVMAX); }
"on"			{ yylval.ival = TRUE; return (TOGGLE); }
"off"			{ yylval.ival = FALSE; return (TOGGLE); }
"step"			{ return (STEP); }
"random"                { return (RANDOM); }
"full"                  { return (FULL); }
"define"                { return (DEFINE_TOKEN); }
"sta_file"              { return (STA_FILE); }
"."			{ return (DOT); }
".."			{ return (DOTDOT); }
"("			{ return (LPS); }
")"			{ return (RPS); }
"["			{ return (LSB); }
"]"			{ return (RSB); }
"{"			{ return (LCB); }
"}"			{ return (RCB); }
"="			{ return (EQL); }
","			{ return (COMMA); }
";"			{ return (SEMICOLON); }
":"			{ return (COLON); }
"*"			{ return (MULT); }
"!"			{ return (EXCLAM); }
"-"                     { return (MINUS); }
"$"                     { return (DOLLAR); }
\"[^"]*\"		{
			    yytext[yyleng - 1] = '\0';
			    yylval.sval = yytext + 1;
			    return (STRING);
			}
[a-zA-Z_][a-zA-Z0-9_]*	{ yylval.sval = yytext; return (IDENTIFIER); }
[0-9]+			{ yylval.sval = yytext; return (INT); }
[1][0]*[afpnumkMG]?   	{
			    yylval.dval = slstof ();
			    return (POWER_TEN);
			}
[0-9]+("."[0-9]+)?({E})?[afpnumkMG]?	{
			    yylval.dval = slstof ();
			    return (F_FLO);
			}
[\t ]+			;
"\\"[\n]		{ YYINCR (1); }
[\n]+			{ YYINCR (yyleng); return (NEWLINE); }

%%

double slstof ()
{
    double val = -1;
    int c = yytext[yyleng - 1];

    if (isdigit (c)) val = atof (yytext);
    else {
	yytext[yyleng - 1] = '\0';
	switch (c) {
	case 'G': val = 1e9   * atof (yytext); break;
	case 'M': val = 1e6   * atof (yytext); break;
	case 'k': val = 1e3   * atof (yytext); break;
	case 'm': val = 1e-3  * atof (yytext); break;
	case 'u': val = 1e-6  * atof (yytext); break;
	case 'n': val = 1e-9  * atof (yytext); break;
	case 'p': val = 1e-12 * atof (yytext); break;
	case 'f': val = 1e-15 * atof (yytext); break;
	case 'a': val = 1e-18 * atof (yytext); break;
	}
    }
    return (val);
}

/* This one can be used in other files instead of yytext
 * to overcome problems with the type definition for yytext
 * (it maybe char *yytext or char yytext[].
 */
char *textval ()
{
    return ((char *)yytext);
}
