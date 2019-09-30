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

#include "src/cfun/func_parsedefs.h"

FTERM		*ftrm_list = NULL;
FTERM		*trm_buf = NULL;

char		Func_name[DM_MAXNAME+1];
char		name_buf[BUFSIZ];
char 		ind0tmp[BUFSIZ];
char 		ind1tmp[BUFSIZ];

int 		indvar = 0;
int 		indvar0 = 0;
int 		indvar1 = 0;
int 		trunc_warn = 0;
int 		adm_bsalloc_flag = 0;
%}

%union {
	int   ival;
	char cval;
	char *sval;
	}

%token <sval> IDENTIFIER INTEGER
%token OUTPUT INOUT INPUT INREAD STATE
%token DELAY MODE CAPADD STATCAP DYNCAP CAPVAL
%token FUNCTION SEMICOLON COMMA
%token LOADL INITL BEHAVIOR END
%token LSB RSB LCB RCB LPS RPS
%token CHAR INT FLOAT DOUBLE
%token NODE VICIN MIN MAX

%type <ival> trm_type type_decl_list type_decl
%type <ival> state_type state_decl_list state_decl_part
%type <sval> index trm_index
%type <sval> name func_name terminal terminal_list
%type <sval> state_decl  decl_part
%type <ival> vicin  minmax
%start total_descr

%%
total_descr	: parts_C sls_descr parts_C
		;

parts_C		: /* empty */
		;

sls_descr	: func_descr
		;

func_descr	: func_head func_body
		;

func_head	: func_call decl_part state_decl_part
		{
			print_func_head('L');
		}
		;

func_call	: FUNCTION func_name
		{
			strcpy(Func_name,$2);
		}
		;

func_name	: name
		;

decl_part	: LPS type_decl_list RPS
		{
		}
		;

type_decl_list	: type_decl
		{
			$$ = $1;
		}
		| type_decl_list SEMICOLON type_decl
		{
			$$ = $1;
		}
		;

type_decl	: trm_type terminal_list
		{
			ftrm_list=add_list(ftrm_list,trm_buf,$1);
					/* trm_buf is added to the        */
					/* final list ftrm_list           */
			trm_buf=NULL;
		}
		;

state_decl_part	: /* empty */
		{
		}
		| STATE LCB state_decl_list RCB
		{
		}
		;

state_decl_list	: state_type state_decl SEMICOLON
		{
			ftrm_list=add_list(ftrm_list,trm_buf,$1);
					/* trm_buf is added to the        */
					/* final list ftrm_list           */
			trm_buf=NULL;
		}
		| state_decl_list state_type state_decl SEMICOLON
		{
			ftrm_list=add_list(ftrm_list,trm_buf,$2);
					/* trm_buf is added to the        */
					/* final list ftrm_list           */
			trm_buf=NULL;
		}
		;

state_decl	: terminal_list
		;

terminal_list	: terminal
		{
			check_term(trm_buf,$1,indvar0,indvar1);
		        trm_buf=fill_list(trm_buf,$1,
					  atoi(ind0tmp),atoi(ind1tmp),
					  NoPlace,NoType);
					/* when a terminal is alright, it */
					/* is placed in the list trm_buf  */
			$$ = $1;
		}
		| terminal_list COMMA terminal
		{
			check_term(ftrm_list,$3,indvar0,indvar1);
		        trm_buf=fill_list(trm_buf,$3,
					  atoi(ind0tmp),atoi(ind1tmp),
					  NoPlace,NoType);
					/* when a terminal is alright, it */
					/* is placed in the list trm_buf  */
			$$ = $3;
		}
		;

terminal	: name trm_index
		{
			$$ = $1;
		}
		;

trm_type	: OUTPUT
		{
			$$ = OutpTerm;
		}
		| INOUT
		{
			$$ = InoTerm;
		}
		| INPUT
		{
			$$ = InpTerm;
		}
		| INREAD
		{
			$$ = InrTerm;
		}
		;

state_type	: CHAR
		{
			$$ = StateChar;
		}
		| INT
		{
			$$ = StateInt;
		}
		| FLOAT
		{
			$$ = StateFloat;
		}
		| DOUBLE
		{
			$$ = StateDouble;
		}
		;

func_body	: load_body init_body behav_body
		;

load_body	: /* empty */
		{
			print_func_foot();
			fprintf(yyout,"\n}\n");
			lineno (yylineno);
			print_func_head('I');
		}
		| LOADL init_code
		{
			print_func_head('I');
		}
		;

init_body	: /* empty */
		{
			print_func_foot();
			fprintf(yyout,"\n}\n");
			lineno (yylineno);
			print_func_head('E');
		}
		| INITL init_code
		{
			print_func_head('E');
		}
		;

behav_body	: /* empty */
		{
			print_func_foot();
			fprintf(yyout,"\n}\n");
			lineno (yylineno);
		}
		| BEHAVIOR behav_code
		;

init_code	: /* empty */
		| routine_call
		| init_code routine_call
		;

behav_code	: /* empty */
		| routine_call
		| behav_code routine_call
		;

routine_call	: DELAY MODE expr COMMA terminal RPS
		{
		     delay_eval($5,ind0tmp,ind1tmp,indvar0,indvar1);
		}
		| CAPADD expr COMMA terminal RPS
		{
		     capadd_eval($4,ind0tmp,ind1tmp,indvar0,indvar1);
		}
		| statcap_call
		| dyncap_call
		| cap_call
		;

statcap_call	: STATCAP LPS vicin COMMA minmax COMMA terminal RPS
		{
		     getcap_eval("statcap", $3, $5, $7, ind0tmp, ind1tmp, indvar0, indvar1);
		}
		;

dyncap_call	: DYNCAP LPS vicin COMMA minmax COMMA terminal RPS
		{
		     getcap_eval("dyncap", $3, $5, $7, ind0tmp, ind1tmp, indvar0, indvar1);
		}
		;

cap_call	: CAPVAL LPS vicin COMMA minmax COMMA terminal RPS
		{
		     getcap_eval("cap", $3, $5, $7, ind0tmp, ind1tmp, indvar0, indvar1);
		}
		;

vicin		: NODE
		{
		    $$ = 0;
		}
		| VICIN
		{
		    $$ = 1;
		}
		;

minmax		: MIN
		{
		    $$ = MIN_CAP;
		}
		| MAX
		{
		    $$ = MAX_CAP;
		}
		;

expr		: /* empty */
		| expr statcap_call
		| expr dyncap_call
		| expr cap_call
		;

trm_index	: /* empty */
		{
			strcpy (ind0tmp, "000");
			strcpy (ind1tmp, "000");
			indvar0 = FUNNOIND;
			indvar1 = FUNNOIND;
		}
		| LSB index RSB
		{
			strcpy (ind0tmp, $2);
			strcpy (ind1tmp, "000");
			indvar0 = indvar;
			indvar1 = FUNNOIND;
		}
		| trm_index LSB index RSB
		{
			strcpy (ind1tmp, $3);
			indvar1 = indvar;
		}
		;

index		: INTEGER
		{
			$$ = $1;
			indvar = FUNINT;
		}
		| IDENTIFIER
		{
			$$ = $1;
			indvar = FUNVAR;
		}
		;

name		: IDENTIFIER
		{
			strcpy (name_buf, $1);

			if (dmTestname (name_buf) == 1) {
			    if (!trunc_warn) {
				fprintf (stderr, "warning: name(s) truncated to %d characters\n", (int)strlen (name_buf));
				trunc_warn = 1;
			    }
			}
			$$ = name_buf;
		}
		;
%%

int yywrap ()
{
    return (1);
}
