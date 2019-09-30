%{
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#ifdef FLEX_SCANNER /* use GNU flex in stead of regular lex */
int yy0lineno;
#define YYINCR(_count) yy0lineno += _count /* have to do this ourselves ... */
#else
#define YYINCR(_x)
#endif

extern void do_atof (void);
extern int read_to_ec (void);
extern int read_to_nl (void);

%}
E		[DEde][-+]?[0-9]+
%%
"/*"			{ if (!read_to_ec ()) return (0); }
"//"			{ if (!read_to_nl ()) return (0); }
"#"			{ if (!read_to_nl ()) return (0); }
"set"			{ return (SET); }
"l"			{ yylval.ival = L_state; return (LOGIC_LEVEL); }
"h"			{ yylval.ival = H_state; return (LOGIC_LEVEL); }
"x"			{ yylval.ival = X_state; return (LOGIC_LEVEL); }
"f"			{ yylval.ival = F_state; return (LOGIC_LEVEL); }
"~"			{ return (TILDE); }
"from"			{ return (FROM); }
"fill"			{ return (FILL); }
"with"			{ return (WITH); }
"print"			{ return (PRINT); }
"plot"			{ return (PLOT_TOKEN);
/* "PLOT" conflicts with dmincl.h therefore have to use "PLOT_TOKEN" ...: */
                        }
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
"define"		{ return (DEFINE_TOKEN); }
"sta_file"		{ return (STA_FILE); }
"tst_file"		{ return (TST_FILE); }
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
"-"			{ return (MINUS); }
"$"			{ return (DOLLAR); }
\"[^"]*\"		{ yytext[yyleng-1] = '\0';
			  yylval.sval = (char*)yytext + 1; return (STRING); }
[a-zA-Z_][a-zA-Z0-9_]*	{ yylval.sval = (char*)yytext; return (IDENTIFIER); }
[0-9]+			{ yylval.sval = (char*)yytext; return (INT); }
[1][0]*[fpnumkMG]?	{ do_atof (); return (POWER_TEN); }
[0-9]+("."[0-9]+)?({E})?[fpnumkMG]? { do_atof (); return (F_FLO); }
[\t ]+			;
"\\"[\n]		{ YYINCR(1); }
[\n]+			{ YYINCR(yyleng); return (NEWLINE); }
.			return (ILLCHAR);
%%

void do_atof (void)
{
    int c = yytext[yyleng-1];
    if (isdigit(c))
	yylval.dval = atof (yytext);
    else {
	yytext[yyleng-1] = '\0';
	switch (c) {
	    case 'G': yylval.dval = 1e9 * atof (yytext); break;
	    case 'M': yylval.dval = 1e6 * atof (yytext); break;
	    case 'k': yylval.dval = 1e3 * atof (yytext); break;
	    case 'm': yylval.dval = 1e-3 * atof (yytext); break;
	    case 'u': yylval.dval = 1e-6 * atof (yytext); break;
	    case 'n': yylval.dval = 1e-9 * atof (yytext); break;
	    case 'p': yylval.dval = 1e-12 * atof (yytext); break;
	    case 'f': yylval.dval = 1e-15 * atof (yytext); break;
	}
    }
}

int read_to_ec (void)
{
    int thechar;
loop:
    while ((thechar = input()) != '*') {
	if (thechar == EOF || thechar == 0) return (0);
	if (thechar == '\n') YYINCR (1);
    }
    switch (input()) {
	case EOF :
	case 0   : return (0);
	case '/' : break;
	case '*' : unput ('*'); goto loop;
	case '\n': YYINCR (1);
	default: goto loop;
    }
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
