%{
/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	T.G.R. van Leuken
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
#include "cif_parse.h"
#include "src/cldm/extern.h"
#ifdef YYSTYPE
extern YYSTYPE yylval;
#else
extern int yylval;
#endif

int rotate;
char userChar[BUFSIZ];
int lineno=1;

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

#endif /* not FLEX_SCANNER */

%}

%start 	layer_code other user

%%

<other>[-]?[0-9]+	{BEGIN other; yylval = atoi (yytext); return(INTEGER);}
"("			{ read_to_nl (); }
;			{BEGIN user; rotate = 0; return(SEMI);}
P			{BEGIN other; return(POLYGON);}
B			{BEGIN other; return(BOX);}
R			{BEGIN other;
				if (rotate == 0) return(ROUNDFLASH);
				else return(ROTATE);}
W			{BEGIN other; return(WIRE);}
L 			{BEGIN layer_code; return(LAYER);}
DS			{BEGIN other; return(START);}
DF			{BEGIN other; return(FINISH);}
DD			{BEGIN other; return(DELETE);}
C			{rotate = 1; BEGIN other; return(CALL);}
E			{BEGIN other; return(END);}
T			{BEGIN other; return(TRANS);}
TR			{BEGIN other; return(TRANS);}
MX			{BEGIN other; return(MX);}
MY			{BEGIN other; return(MY);}
<layer_code>[0-9A-Z]+	{BEGIN other; return(SHORTNAME);}
<user>[0-9]		{int i;
                        BEGIN other; yylval = atoi (yytext);
			i = 0;
			while ((userChar[i] = input()) != ';') {
				if (userChar[i] == YYEOF) yyerror ("unexpected end of file");
				if (userChar[i] == '\n') yylineno++;
				if (++i == BUFSIZ) yyerror ("missing ';', buffer overflow");
			}
			unput(userChar[i]);
			userChar[i] = '\0';
			return(USER);}
.			;
"\n"			yylineno++;
%%

int try_input ()
{
    char c;
    int i = 0;

    c = input();

    if (c == '(') {
	while ((c = input()) != ')') {
	    if (c == YYEOF) yyerror ("unexpected end of file");
	    if (c == '\n') yylineno++;
	    userChar[i] = c;
	    if (++i == BUFSIZ) yyerror ("missing ')', buffer overflow");
	}
	userChar[i] = '\0';
    }
    else
	unput (c);
    return (i);
}

void read_to_nl ()
{
    char buf[20], c;
    int i = 0;

    c = input();

    if (h_x_mode == -1 && (c == 'o' || c == 'n' || c == 'u')) {
	int n = 0;
	do {
	    if (n == 20) goto s2;
	    buf[n++] = c;
	    c = input();
	    if (c == '(' || c == '\n' || c == YYEOF) goto s2;
	} while (c != ')') ;

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
	return;
    }
s2:
    while (c != YYEOF) {
	if (c == ')') if (--i < 0) break;
	if (c == '(') i++;
	if (c == '\n') yylineno++;
	c = input();
    }
    if (c == YYEOF) yyerror ("unexpected end of file");
}

#ifdef yywrap
#undef yywrap
#endif

int yywrap ()
{
    if (feof (yyin)) {
	if (!done_user_term && done_user_94) {
	    P_E "%s: WARNING: No terminals are written!\n", argv0);
	    P_E "%s: To use the old user 94 mode, give option -z!\n", argv0);
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
