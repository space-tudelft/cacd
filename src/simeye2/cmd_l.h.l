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

#ifdef FLEX_SCANNER /* use GNU flex in stead of standard lex */
int yylineno;
#define YYINCR(_count) yylineno += _count /* do it yourself */
#else
#define YYINCR(_x) /* lex already keeps the line count */
#endif

extern int no_edit;

extern int read_to_ec (void);
extern int read_to_nl (void);

%}
E		[DEde][-+]?[0-9]+
%%
"/*"			{ if (!read_to_ec ()) return (0); }
"//"			{ if (!read_to_nl ()) return (0); }
"#"			{ if (!read_to_nl ()) return (0); }
"set"			{ no_edit = 0; return (SET); }
"l"			{ yylval.ival = L_state; return (LOGIC_LEVEL); }
"h"			{ yylval.ival = H_state; return (LOGIC_LEVEL); }
"x"			{ yylval.ival = X_state; return (LOGIC_LEVEL); }
"f"			{ yylval.ival = F_state; return (LOGIC_LEVEL); }
"~"			{ return (TILDE); }
"from"			{ return (FROM); }
"fill"			{ return (FILL); }
"with"			{ return (WITH); }
"print"			{ return (PRINT_CMD); }
"plot"			{ return (PLOT_CMD); }
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
"random"		{ return (RANDOM); }
"full"			{ return (FULL); }
"define"                { return (DEFINE_TOKEN); }
"sta_file"		{ return (STA_FILE); }
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
			  yytext[yyleng-1] = '\0';
			  yylval.sval = (char *)yytext + 1;
			  return (STRING);
			}
[a-zA-Z_][a-zA-Z0-9_]*	{ yylval.sval = (char *)yytext; return (IDENTIFIER); }
[0-9]+			{ yylval.sval = (char *)yytext; return (INT); }
[1][0]*[fpnumkMG]?   	{
			  yylval.dval = slstof (yytext);
			  return (POWER_TEN);
			}
[0-9]+("."[0-9]+)?({E})?[fpnumkMG]?	{
			  yylval.dval = slstof (yytext);
			  return (F_FLO);
			}
[\t ]+			;
"\\"[\n]		{ YYINCR(1); }
[\n]+			{ YYINCR(yyleng); return (NEWLINE); }
[a-zA-Z0-9_]+		{ return (ILLCHAR); }
.			{ return (ILLCHAR); }
%%

/* This one can be used in other files instead of yytext
   to overcome problems with the type definition for yytext
   (it maybe char *yytext or char yytext[]. */
char * textval ()
{
    return ((char *)yytext);
}

int read_to_ec (void)
{
    int c = input ();

    while (c == ' ' || c == '\t') c = input ();
    if (c == 'n'
	&& (c = input ()) == 'o'
	&& (c = input ()) == '_'
	&& (c = input ()) == 'e'
	&& (c = input ()) == 'd'
	&& (c = input ()) == 'i'
	&& (c = input ()) == 't') { no_edit = 1;
	c = input (); }

    do while (c != '*') {
	if (c == 0 || c == EOF) return (0);
	if (c == '\n') YYINCR (1);
	c = input (); }
    while ((c = input ()) != '/');
    return (1);
}

int read_to_nl (void)
{
    int thechar;
    while ((thechar = input())) {
	if (thechar == '\n') {
	  YYINCR (1);
	  break;
	}
	else if (thechar ==  EOF  || thechar == 0) return (0);
    }
    return (1);
}
