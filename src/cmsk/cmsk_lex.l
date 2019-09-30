%{
/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
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

#include "src/cmsk/extern.h"
#define SCANNER
#include "cmsk_parse.h"
/*
** Source program for lex (unix utility) on behalf of
** a translator from the language CMSK (CircuitMaSK)
** to the database of the IC-layout system of Delft.
*/

extern int pfactor;
extern double d_f;

#ifdef YYSTYPE
extern YYSTYPE yylval;
#else
extern int yylval;
#endif

#ifdef FLEX_SCANNER /* use GNU flex in stead of regular lex */
int yylineno;
#else

/*
**  Macro that redefines input, so that lowercase
**  conversion takes place.
**  Note: No line counter is include here !
*/
#undef  input()
#define input() (\
((yytchar=(yysptr>yysbuf)?(*--yysptr):getc(yyin)) >= 'A')?\
((yytchar<= 'Z')?(yytchar+32):(yytchar)):\
((yytchar== EOF)?(0):(yytchar)))

#undef  unput()
#define unput(c) { *yysptr++ = (c); }

#define GETC() (yysptr>yysbuf)?(*--yysptr):getc(yyin)

#endif  /* not FLEX_SCANNER */

int line_no;
%}

IDENT       [a-z][a-z0-9\-]*
STR         [a-z0-9+\-][a-z0-9+\- ]*
DIGITS      [0-9]+
ODIGITS     [0-9]*

%start parameters placement msk_man pat_man
%start tra_man txt_man txt_hhh txt_str

%a 20000
%p 20000
%o 20000
%n 10000

%%
<msk_man>and		{ return (AND); }
<msk_man>or		{ return (OR);  }
<msk_man>not		{ return (NOT); }
<msk_man>")"		{ return (')'); }
<msk_man>{IDENT}	{ yylval = savename (yytext);
			  return (MASKNAME); }
<tra_man>{IDENT}	{ yylval = savename (yytext);
			  BEGIN pat_man; return (MASKNAME); }
<pat_man>")"		{ return (')'); }
<pat_man>{IDENT}	{ yylval = savename (yytext);
			  return (PATNAME); }
<parameters>a		{ return (ABSOLUTE); }
<parameters>r		{ return (RELATIVE); }
<parameters>s		{ return (SLANTING); }
<parameters>t		{ return (TEST); }
<parameters>x		{ return (X); }
<parameters>y		{ return (Y); }
<parameters>{IDENT}	{ yylval = savename (yytext);
			  return (MASKNAME); }
<placement>t1		{ return (T1); }
<placement>t2		{ return (T2); }
<placement>t3		{ return (T3); }
<placement>r		{ return (ROTATE); }
<placement>r0		{ return (ROT0); }
<placement>r90		{ return (ROT90); }
<placement>r180		{ return (ROT180); }
<placement>r270		{ return (ROT270); }
<placement>mx		{ return (MX); }
<placement>my		{ return (MY); }
<placement>copyx	{ return (CX); }
<placement>copyy	{ return (CY); }
<placement>cx		{ return (CX); }
<placement>cy		{ return (CY); }
<placement>{IDENT}	{ yylval = savename (yytext);
			  return (MASKNAME);
			}
<txt_man>{IDENT}	{ yylval = savename (yytext);
			  BEGIN txt_hhh;
			  return (MASKNAME);
			}
<txt_hhh>[-+]?{DIGITS}	{
		  yylval = atoi (yytext);
		  BEGIN txt_str;
		  return (INTEGER);
		}
<txt_hhh>[-+]?{DIGITS}p	{
		  if (!pfactor) {
		      if (line_no != yylineno) {
			  line_no = yylineno;
			  pr_err1 ("error: pfactor not set, p not allowed");
		      }
		      killpattern ();
		  }
		  yylval = pfactor * atoi (yytext);
		  BEGIN txt_str;
		  return (INTEGER);
		}
<txt_hhh>[-+]?{ODIGITS}"."{DIGITS} {
		  d_f = atof (yytext);
		  BEGIN txt_str;
		  return (FLOAT);
		}
<txt_hhh>[-+]?{ODIGITS}"."{DIGITS}p {
		  if (!pfactor) {
		      if (line_no != yylineno) {
			  line_no = yylineno;
			  pr_err1 ("error: pfactor not set, p not allowed");
		      }
		      killpattern ();
		  }
		  d_f = pfactor * atof (yytext);
		  BEGIN txt_str;
		  return (FLOAT);
		}
<txt_str>{STR}	{ yylval = savename (yytext); return (STRING); }

masks		{ BEGIN msk_man; return (MASKS); }
masklist	{ BEGIN msk_man; return (MASKLIST); }
pfactor		{ return (PFACTOR); }
pattern		{ return (PATTERN); }
end		{ return (END); }
maskey		{ return (MASKEY); }

ra		{ BEGIN parameters; return (RA); }
rr		{ BEGIN parameters; return (RR); }
rs		{ BEGIN parameters; return (TOK_RS); }
la		{ BEGIN parameters; return (LA); }
poly		{ BEGIN parameters; return (POLY); }
text		{ BEGIN txt_man;    return (TEXT); }
circle		{ BEGIN parameters; return (CIRCLE); }
cpeel		{ BEGIN parameters; return (CPEEL); }
polygon		{ BEGIN parameters; return (POLYGON); }
path		{ BEGIN parameters; return (PATH); }

remove		{ BEGIN pat_man; return (REMOVE); }
transfer	{ BEGIN tra_man; return (TRANSFER); }
extract		{ BEGIN pat_man; return (EXTRACT); }
formmask	{ BEGIN msk_man; return (FORMMASK); }
grow		{ BEGIN msk_man; return (GROW); }

plottext	{ skip_statement (yytext); }
patlim		{ skip_statement (yytext); }
scriblin	{ skip_statement (yytext); }

usefile		{ skip_statement (yytext); }
list		{ skip_statement (yytext); }
lblist		{ skip_statement (yytext); }
restrict	{ skip_statement (yytext); }
resolutn	{ skip_statement (yytext);
		  pr_err2 ("warning: resolution is", d2a (1.0));
		}
readfrom	{ skip_statement (yytext); }
clear		{ skip_statement (yytext); }
clist		{ skip_statement (yytext); }
plist		{ skip_statement (yytext); }
listfile	{ skip_statement (yytext); }
resave		{ skip_statement (yytext); }
save		{ skip_statement (yytext); }
delete		{ skip_statement (yytext); }
patkey		{ skip_statement (yytext); }
patmap		{ skip_statement (yytext); }
check		{ skip_statement (yytext); }
output		{ skip_statement (yytext); }
display		{ skip_statement (yytext); }
menu		{ skip_statement (yytext); }
finish		{ skip_statement (yytext); }
newjob		{ skip_statement (yytext); }

{IDENT}		{ yylval = savename (yytext);
		  BEGIN placement; return (PATNAME);
		}
[-+]?{DIGITS}	{
		  yylval = atoi (yytext);
		  return (INTEGER);
		}
[-+]?{DIGITS}p	{
		  if (!pfactor) {
		      if (line_no != yylineno) {
			  line_no = yylineno;
			  pr_err1 ("error: pfactor not set, p not allowed");
		      }
		      killpattern ();
		  }
		  yylval = pfactor * atoi (yytext);
		  return (INTEGER);
		}
[-+]?{ODIGITS}"."{DIGITS} {
		  d_f = atof (yytext);
		  return (FLOAT);
		}
[-+]?{ODIGITS}"."{DIGITS}p {
		  if (!pfactor) {
		      if (line_no != yylineno) {
			  line_no = yylineno;
			  pr_err1 ("error: pfactor not set, p not allowed");
		      }
		      killpattern ();
		  }
		  d_f = pfactor * atof (yytext);
		  return (FLOAT);
		}
","		{ return (','); }
"("		{ return ('('); }
")"		{ BEGIN placement; return (')'); }
[ \t\r\f]	{ ; /* do nothing */ }
"*"[ \t\r]*\n	{ ++yylineno; /* ignore newline */ }
":".*		{ ; /* do nothing */ }
\n		{ BEGIN 0; return (EOL); }
.		{ return (GARBAGE); }
%%

void skip_statement (char *s)
{
    int c;

    pr_err ("warning:", s, "statement skipped");

    while ((c = input ()) != '\n' && c != EOF) {
        if (c == '*') {
            while ((c = input ()) == ' ' || c == '\t') ;
            if (c == EOF) break;
            if (c == '\n') ++yylineno; /* ignore newline */
        }
    }
    unput ('\n');
}

/* This one can be used in other files instead of yytext
   to overcome problems with the type definition for yytext
   (it maybe char *yytext or char yytext[]. */
char *textval ()
{
    return ((char *)yytext);
}
