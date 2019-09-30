/*  rcsid = "$Id: lex.h.l,v 1.1 2018/04/30 12:17:33 simon Exp $" */
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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

%{
#ifdef DEBUG
int yydebug = 1;
#define Return(v) \
if(yydebug) user_mesg("\tLex: token %s, text '%s'\n", #v, yytext); return(v);
#else
#define Return(v) return(v)
#endif

#ifdef FLEX_SCANNER /* use GNU flex in stead of standard lex */
int yylineno;
#define YYINCR(_count) yylineno += _count /* do it yourself */
#else
#define YYINCR(_x)	/* lex already keeps the line count */
#endif

%}

D				[0-9]
E				[DEde][-+]?{D}+

%%
"/*"				{ char thechar;
				loop:
                                    while((thechar = input()) != '*')
                                    {
                                    	if (thechar == EOF || thechar == 0) return(0);
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

[\t\r ]+			; /* ignore white space */
[\n]+				YYINCR (yyleng);

"extern"			Return(EXT);
"primitive"			Return(PRIM);
"network"			Return(NETWORK);
"function"			Return(FUNCTION);
"terminal"			Return(TERMNL);
"net"				Return(NT);
"perm"				Return(PERM);

"cap"				Return(CAP);
"res"				Return(RES);
"nenh"				Return(NENH);
"penh"				Return(PENH);
"ndep"				Return(NDEP);

[a-zA-Z][a-zA-Z0-9_]*		{ yylval.sval = mk_symbol((string)yytext); Return(IDENTIFIER); }
{D}+				{ yylval.sval = mk_symbol((string)yytext); Return(INTEGER); }
{D}+("."{D}+)?({E})?[fpnumkMG]?	{ yylval.sval = mk_symbol((string)yytext); Return(FLOAT); }

.				Return(*yytext);
%%

#ifndef FLEX_SCANNER
/*
   However, when lex is used instead of flex, yyrestart() is
   not defined.
*/
void yyrestart (FILE *fp)
{
}
#endif
