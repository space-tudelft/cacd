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

#ifdef FLEX_SCANNER
int yy1lineno;
#define YYINCR(_count) yy1lineno += _count /* have to do this ourselves ... */
#else
#define YYINCR(_x)
#endif

%}

%%
"/*"			{
			    int c = input ();

			    do while (c != '*') {
				if (c == 0 || c == EOF) return (0);
				if (c == '\n') YYINCR (1);
				c = input (); }
			    while ((c = input ()) != '/');
			}
"#"                     { int thechar;
                            while ((thechar = input())) {
                                if (thechar == '\n') {
                                  YYINCR (1);
                                  break;
                                }
                                else if (thechar ==  EOF  || thechar == 0) {
                                  return(0);
                                }
                            }
                        }
vh			{ return(VH); }
vswitch			{ return(VSWITCH); }
vminh			{ return(VMINH); }
vmaxl			{ return(VMAXL); }
krise			{ return(KRISE); }
kfall			{ return(KFALL); }
nenh			{ return(NENH); }
penh			{ return(PENH); }
ndep			{ return(NDEP); }
end			{ return(END); }
pullup			{ return(PULLUP); }
pulldown		{ return(PULLDOWN); }
passup			{ return(PASSUP); }
passdown		{ return(PASSDOWN); }
load			{ return(LOAD); }
superload		{ return(SUPERLOAD); }
rstat			{ return(RSTAT); }
rsatu			{ return(RSATU); }
cgstat			{ return(CGSTAT); }
cgrise			{ return(CGRISE); }
cgfall			{ return(CGFALL); }
cestat			{ return(CESTAT); }
cerise			{ return(CERISE); }
cefall			{ return(CEFALL); }
rdyn			{ return(RDYN); }
cch			{ return(CCH); }
loffset			{ return(LOFFSET); }
woffset			{ return(WOFFSET); }
l			{ return(L); }
w			{ return(W); }
=			{ return(EQUAL); }
:			{ return(COLON); }
-?[0-9]+		{ yylval.sval = (char *)yytext; return(INT); }
-?[0-9]+("."[0-9]+)?k   { yylval.fval = 1e3 * atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?    { yylval.fval = atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?m   { yylval.fval = 1e-3  * atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?u   { yylval.fval = 1e-6  * atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?n   { yylval.fval = 1e-9  * atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?p   { yylval.fval = 1e-12 * atof(yytext); return(FLO); }
-?[0-9]+("."[0-9]+)?f   { yylval.fval = 1e-15 * atof(yytext); return(FLO); }
[\t ]+			;
[\n]+                   { YYINCR (yyleng); }
%%
