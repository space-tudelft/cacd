%{
/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#define SCANNER
#include "ldm_parse.h"
#include "src/cldm/extern.h"
#ifdef YYSTYPE
extern YYSTYPE yylval;
#else
extern int yylval;
#endif

#ifdef FLEX_SCANNER /* use GNU flex in stead of regular lex */
#define YYEOF EOF
int yylineno;
#else
#define YYEOF 0
/*
**  Redefines of standard lex input/unput macro's
**  Note: No line counter is include here !
*/
#undef  input
#define input() (\
((yytchar=(yysptr>yysbuf)?(*--yysptr):getc(yyin)) == EOF)?\
(YYEOF):(yytchar))

#undef  unput
#define unput(c) { *yysptr++ = (c); }

#define GETC() (yysptr>yysbuf)?(*--yysptr):getc(yyin)
#endif
%}

%%
ms		{
		    if (n_tok) return (WORD);
		    else {
			if (ms_tok) {
			    pr_exit (014, 1, 0);
			    pr_exit (04, 26, ms_name);
			}
			w_tok = 0;
			n_tok = ms_tok = 1;
			return (MS);
		    }
		}
me		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			}
			w_tok = ms_tok = 0;
			return (ME);
		    }
		}
mc		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (MC);
		    }
		}
box		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (BOX);
		    }
		}
sbox		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (SBOX);
		    }
		}
term		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = t_tok = 1;
			return (TERM);
		    }
		}
label		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = t_tok = 1;
			return (LABEL);
		    }
		}
poly		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (POLY);
		   }
		}
wire		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			n_tok = w_tok = 1;
			return (WIRE);
		    }
		}
swire		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (SWIRE);
		    }
		}
cont		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			if (!w_tok) {
			    pr_exit (014, 3, 0);
			    w_tok = 1;
			}
			return (CONT);
		    }
		}
circle		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (CIRCLE);
		    }
		}
cpeel		{
		    if (n_tok) return (WORD);
		    else {
			if (!ms_tok) {
			    pr_exit (014, 2, 0);
			    ms_tok = 1;
			}
			w_tok = 0;
			n_tok = 1;
			return (CPEEL);
		    }
		}
cx		{ if (n_tok) return (WORD); else return (CX); }
cy		{ if (n_tok) return (WORD); else return (CY); }
mx		{ if (n_tok) return (WORD); else return (MX); }
my		{ if (n_tok) return (WORD); else return (MY); }
t		{ if (n_tok) return (WORD); else return (TRANS); }
w		{ if (n_tok) return (WORD); else return (WIDTH); }
x		{ if (n_tok) return (WORD); else return (X); }
y		{ if (n_tok) return (WORD); else return (Y); }
r3		{ if (n_tok) return (WORD); else return (R3); }
r6		{ if (n_tok) return (WORD); else return (R6); }
r9		{ if (n_tok) return (WORD); else return (R9); }
sfx		{ if (n_tok) return (WORD); else return (SFX); }
sfy		{ if (n_tok) return (WORD); else return (SFY); }
","		{ return (','); }
"<"		{ return ('<'); }
">"		{ return ('>'); }
\n		{ return (EOL); }
":"		{ read_to_nl (); return (':'); }
[A-Za-z][A-Za-z0-9_]*   { return (WORD); }
[-+]?[0-9]+	|
[-+]?[0-9]+"."	{
		  yylval = atoi (yytext); return (INTEGER);
		}
[-+]?[0-9]*"."[0-9]+ {
		  d_f = atof (yytext); return (FLOAT);
		}
[A-Za-z0-9_]+	{ return (WORD2); }
[ \t\r]		{ ; }
"\\".*\n	{ ++yylineno; /* ignore newline */ }
.		{ read_to_nl (); return (ILLCHAR); }
%%

void read_to_nl ()
{
    char buf[20], c;

    c = input ();

    if (*yytext == ':' && c == ':' && h_x_mode == -1) {
	int n = 0;
	while ((c = input ()) != '\n' && c != YYEOF) {
	    if (!n) {
		if (c == ' ') continue;
		if (c != 'o' && c != 'n' && c != 'u') goto s2;
	    }
	    if (n == 20) goto s2;
	    buf[n++] = c;
	}
	if (c == '\n') {
	    buf[n] = 0;
	    if (*buf == 'u') {
		char *s = buf + 4;
		*s = 0;
		if (strcmp (buf, "use ") == 0) {
		    while (*++s && *s != ' ') ;
		}
		if (*s++ == ' ') {
		    if (strcmp (s, "with -o") == 0) h_x_mode = 0;
		    else
		    if (strcmp (s, "without -o") == 0) h_x_mode = 1;
		}
	    }
	    else if (*buf == 'n') {
		buf[14] = 0;
		if (strcmp (buf, "no-origin mode") == 0) h_x_mode = 0;
	    }
	    else {
		buf[11] = 0;
		if (strcmp (buf, "origin mode") == 0) h_x_mode = 1;
	    }
	}
    }
    else if (c != '\n' && c != YYEOF) {
s2:
	while ((c = input ()) != '\n' && c != YYEOF) ;
    }
    unput ('\n');
}

#ifdef yywrap
#undef yywrap
#endif

int yywrap ()
{
    if (feof (yyin)) {
	if (ms_tok) {
	    pr_exit (0114, 1, 0);
	    pr_exit (04, 26, ms_name);
	    pr_exit (07, 7, 0);
        }
	pr_exit (05, 7, 0);
    }
    return (0);
}

/* This one can be used in other files instead of yytext
   to overcome problems with the type definition for yytext
   (it maybe char *yytext or char yytext[]. */
char * textval ()
{
    return ((char *)yytext);
}
