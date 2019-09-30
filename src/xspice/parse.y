%{

/*
 * ISC License
 *
 * Copyright (C) 1994-2013 by
 *	Frederik Beeftink
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/xspice/incl.h"

extern int yyleng;

#ifdef __cplusplus
  extern "C" {
#endif
static void yyerror (char *s);
static char *streqn (char *s);
static char *slstos (char *s);
static void setUnity (char *name, char *val);
static struct lib_model *getModel (char *name);
#ifdef __cplusplus
  }
#endif

#define MAXUNITY  50
#define MAXMODEL 100
#define MAXEQN  1024

extern char *argv0;
extern char *parFileName;
extern int yylineno;

extern struct lib_model *lm_head;
static struct lib_model *model = NULL;
static struct model_par *last_par_list;

char *modelNameTab[MAXMODEL];
static int model_cnt = 0;

char *unityNameTab[MAXUNITY];
char *unityValueTab[MAXUNITY];
static int unity_cnt = 0;

static int eqnpos = 0;
static int varval = 0;

char eqnbuf[MAXEQN];
char modbuf[DM_MAXNAME+1];
char modbf2[DM_MAXNAME+1];
char nambuf[DM_MAXNAME+1];
char varbuf[DM_MAXNAME+2];

static char *tlnam = "too long name";
static char *tleqn = "too large equation";

%}

%union		{
		 // double dval;
                    char * sval;
  		    char   cval;
                    struct model_par *mpar;
		}

%token 		PLUS MINUS EQUAL MULT DIV COMMA SEMICOLON COLON BAR
                POWER NOT LPS RPS DOLLAR UNITY MODEL ILLCHAR
%token <sval>	DOUBLE DOUBLE2 IDENTIFIER

%type  <sval>	name mod mod2 variable expr
%type  <mpar>   par_terms par_list parameter
%type  <cval>	par_sep

%left MULT DIV
%left PLUS MINUS
%left POW
%left UMINUS

%%

file_descr	: /* empty */
		| file_descr unity
		| file_descr model
		;

unity		: UNITY name DOUBLE
		{
			setUnity ($2, $3);
		}
		;

model		: MODEL name
		{
			model = getModel ($2);
		}
		  name LPS par_terms RPS
		{
		    if (model) {
			struct lib_model *lm;
			char *s, *t;
			t = s = $4;
			while (*t) { *t = tolower (*t); t++; }
			t = model -> type_name;
			if (strcmp (s, t) != 0) {
#ifdef XSPICE
			    P_E "%s: model '%s' library type different from control file type (`%s' versus `%s').\n",
				argv0, model -> name, s, t);
#endif
			    t = strsave (s);
			}
			model -> specified = t;
			model -> par_list = $6;
			/* are there more models with the same name? */
			for (lm = model -> next; lm; lm = lm -> next) {
			    if (strcmp (lm -> name, model -> name) == 0) {
				lm -> specified = model -> specified;
				lm -> par_list  = model -> par_list;
				lm -> var_in_par = model -> var_in_par;
			    }
			}
			model = NULL;
		    }
		}
                ;

par_terms       : /* empty */	{ $$ = NULL; }
                | par_list	{ $$ = $1; }
                ;

par_list        : parameter
                {
		    $$ = last_par_list = $1;
		}
                | par_list parameter
                {
		    if (model) {
			last_par_list -> next = $2;
			last_par_list = last_par_list -> next;
			$$ = $1;
		    }
		}
                ;

parameter       : mod EQUAL expr par_sep
                {
		    if (model) {
			char *m = $1;
			PALLOC ($$, 1, struct model_par);
			if (varval) {
			    $$ -> var_in_val = model -> var_in_par = 1;
			    $$ -> value = strsave ($3);
			}
			else if (strcmp (m, "level") == 0 || strcmp (m, "version") == 0) {
			    sprintf (varbuf, "%s=%s", m, $3); m = varbuf;
			}
			else {
			    double geom[1];
			    $$ -> value = strsave (fvalEqn ($3, geom));
			}
			$$ -> name = strsave (m);
			$$ -> separator = $4;
		    }
		    eqnpos = 0;
		    varval = 0;
		}
		| mod EQUAL mod2 par_sep
		{
		    if (model) {
			sprintf (eqnbuf, "%s=%s", $1, $3);
			PALLOC ($$, 1, struct model_par);
			$$ -> name = strsave (eqnbuf);
			$$ -> separator = $4;
		    }
		}
		| mod par_sep
		{
		    if (model) {
			PALLOC ($$, 1, struct model_par);
			$$ -> name = strsave ($1);
			$$ -> separator = $2;
		    }
		}
                ;

expr            : variable
                | LPS expr RPS
		{
		    char *b, *s, *t;

		    if ((eqnpos += 2) > MAXEQN) yyerror (tleqn);
		    b = $2;
		    t = eqnbuf + eqnpos;
		    *--t = 0;
		    *--t = ')';
		    s = --t;
		    while (s > b) *t-- = *--s;
		    *b = '(';
		    $$ = b;
		}
                | expr POW   expr { $$ = $1; *($3 - 1) = '^'; }
                | expr PLUS  expr { $$ = $1; *($3 - 1) = '+'; }
                | expr MINUS expr { $$ = $1; *($3 - 1) = '-'; }
                | expr MULT  expr { $$ = $1; *($3 - 1) = '*'; }
                | expr DIV   expr { $$ = $1; *($3 - 1) = '/'; }
                ;

variable        : DOUBLE
                {
		    char *s, *val = $1;
		    s = val + yyleng - 1;
		    val = streqn (val);
		    if (!isdigit (*s)) {
			eqnpos -= 2;
			streqn (slstos (s));
		    }
		    $$ = val;
		}
                | DOLLAR IDENTIFIER
                {
		    char *t;
		    int i;

		    if (yyleng > DM_MAXNAME) yyerror (tlnam);

		    t = $2;
		    for (i = 0; i < unity_cnt; i++) {
			if (strcmp (unityNameTab[i], t) == 0) break;
		    }
		    if (i < unity_cnt) t = streqn (unityValueTab[i]);
		    else {
			varval = 1;
			eqnbuf[eqnpos++] = '$';
			t = streqn (t); --t;
		    }
		    $$ = t;
		}
                ;

name		: IDENTIFIER
		{
		    if (yyleng > DM_MAXNAME) yyerror (tlnam);
		    $$ = strcpy (nambuf, $1);
		}
		;

mod		: IDENTIFIER
		{
		    if (yyleng > DM_MAXNAME) yyerror (tlnam);
		    $$ = strcpy (modbuf, $1);
		}
		;

mod2		: IDENTIFIER
		{
		    if (yyleng > DM_MAXNAME) yyerror (tlnam);
		    $$ = strcpy (modbf2, $1);
		}
		;

par_sep		: COMMA 	{ $$ = ','; }
		| SEMICOLON	{ $$ = ';'; }
		| /* empty */	{ $$ = 0; }
		;

%%

#ifdef yywrap
#undef yywrap   /* flex defines yywrap 1 */
#endif

static void yyerror (char *s)
{
    sprintf (eqnbuf, "file '%s',\n   %s at line %d\n", parFileName, s, yylineno);
    fatalErr (eqnbuf, NULL);
}

static char *streqn (char *s)
{
    char *t, *b;

    t = b = eqnbuf + eqnpos;
    while (*s) *t++ = *s++;
    *t++ = 0;
    eqnpos += t - b;
    if (eqnpos > MAXEQN) yyerror (tleqn);
    return (b);
}

static char *slstos (char *s)
{
    switch (*s) {
    case 'P': return ("e15");
    case 'T': return ("e12");
    case 'G': return ("e9");
    case 'M': return ("e6");
    case 'k': return ("e3");
    case 'm': return ("e-3");
    case 'u': return ("e-6");
    case 'n': return ("e-9");
    case 'p': return ("e-12");
    case 'f': return ("e-15");
    case 'a': return ("e-18");
    }
    return ("");
}

static void setUnity (char *name, char *val)
{
    char *s, *t;
    int i;
    for (i = 0; i < unity_cnt; i++)
	if (strcmp (name, unityNameTab[i]) == 0) goto value;
    if (unity_cnt >= MAXUNITY) goto err2;
    unityNameTab[unity_cnt++] = strsave (name);
value:
    s = val + yyleng - 1;
    if (!isdigit (*s)) {
	t = slstos (s); *s = 0;
	sprintf (varbuf, "%s%s", val, t);
	val = varbuf;
    }
    unityValueTab[i] = strsave (val);
    return;
err2:
    sprintf (eqnbuf, "file '%s', line %d\n   Too many unity names!", parFileName, yylineno);
    fatalErr (eqnbuf, NULL);
}

static struct lib_model *getModel (char *name)
{
    struct lib_model *lm;
    int i;

    for (lm = lm_head; lm; lm = lm -> next)
	if (strcmp (name, lm -> name) == 0) break;
    if (!lm) {
#if 0
	P_E "Warning: range not specified for model: %s\n", name);
#endif
	return (lm);
    }

    for (i = 0; i < model_cnt; i++)
	if (lm -> name == modelNameTab[i]) goto err1;
    if (model_cnt >= MAXMODEL) goto err2;
    modelNameTab[model_cnt++] = lm -> name;
    return (lm);
err1:
    sprintf (eqnbuf, "file '%s', line %d\n   Multiple use of model name:", parFileName, yylineno);
    fatalErr (eqnbuf, name);
err2:
    sprintf (eqnbuf, "file '%s', line %d\n   Too many model names!", parFileName, yylineno);
    fatalErr (eqnbuf, NULL);
    return (lm);
}

int yywrap () /* end-of-input */
{
    while (unity_cnt > 0) free (unityNameTab[--unity_cnt]);
    return (1);
}
