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

/* This file is a copy from sls but it has been stripped and modified */

#include <stdio.h>
#include "src/libddm/dmincl.h"
#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

#define MAXHIERAR        22
#define MAXSTACKLENGTH  100

extern FILE *fp_err;

char name_space[128]; /* memory space for a name */

PATH_SPEC  pathspace[MAXHIERAR]; /* memory space for paths */
PATH_SPEC *fullpath;         /* first of the current full path specification */
PATH_SPEC *last_path = NULL; /* last  of the current full path specification */

STRING_REF *begin_str_list;
STRING_REF *end_str_list;

int sigcon;                /* boolean flag */
int no_edit;

short lastval;
simtime_t * len_sp;
simtime_t len_stack[MAXSTACKLENGTH];
SIGNALELEMENT * sgn;
SIGNALELEMENT ** sgn_sp;
SIGNALELEMENT * sgn_stack[MAXSTACKLENGTH];

struct node *Begin_node;
struct node *End_node;

int sigendless;
simtime_t simperiod;
double sigtimeunit;
int errorDetected;

extern int yylineno;

void assignExprToNodes (void);
void yyerror (char *s);

%}

%union		{
		    simtime_t lval;
		    int ival;
		    int *pival;
		    char *sval;
		    char **psval;
		    double dval;
		    double *pdval;
		    struct signalelement *signal;
		    struct path_spec *pathsp;
		}

%token 		SET TILDE FROM FILL WITH PRINT_CMD PLOT_CMD OPTION SIMPERIOD
%token 		DUMP AT DISSIPATION INITIALIZE SIGOFFSET RACES DEVICES
%token 		ONLY CHANGES SLS_PROCESS SIGUNIT OUTUNIT OUTACC MAXPAGEWIDTH
%token 		MAXNVICIN MAXTVICIN MAXLDEPTH VH VMAXL VMINH STATISTICS
%token		TDEVMIN TDEVMAX STEP DISPERIOD RANDOM FULL DEFINE_TOKEN
%token		STA_FILE DOT DOTDOT LPS RPS LSB RSB LCB RCB EQL MINUS DOLLAR
%token 		COMMA SEMICOLON COLON MULT EXCLAM NEWLINE ILLCHAR TOGGLE LEVEL
%token <ival>	LOGIC_LEVEL
%token <sval>	IDENTIFIER INT STRING
%token <dval>	POWER_TEN F_FLO

%type  <lval>	duration
%type  <ival>	integer
%type  <dval>	f_float
%type  <pdval>	f_float_option
%type  <sval>	member_name
%type  <signal>	value signal_exp value_exp
%type  <pathsp>	ref_item

%%

sim_cmd_list	: sim_cmd
		| sim_cmd_list eoc sim_cmd
		| error eoc sim_cmd
		{
		    yyerrok;
		    last_path = NULL;
		}
		;

sim_cmd		: set_cmd
		| print_cmd
		| plot_cmd
		| dump_cmd
		| dissip_cmd
		| init_cmd
		| fill_cmd
		| define_cmd
		| option_cmd
		| /* empty */
		;

set_cmd		: SET node_refs EQL signal_exp
		{
		    assignExprToNodes ();

    		    len_sp = len_stack;  /* reset stacks for signal_exp */
    		    sgn_sp = sgn_stack;
		}
		| SET node_refs COLON node_refs FROM STRING
		;

signal_exp	: value_exp
		{
		    *++len_sp = $1 -> len;
		    $$ = *sgn_sp++ = $1;
		}
		| signal_exp value_exp
		{
		    if ($2 -> len < 0)
		        *len_sp = -1;
		    else
		        *len_sp += $2 -> len;
		    $1 -> sibling = $2;
		    $$ = $2;
		}
		;

value_exp	: value
		{
		    /* default duration 1 unit */
		    $$ = $1;
		    if ($1 -> child)
		        $$ -> len = $1 -> child -> len;
		    else
		        $$ -> len = 1;
		}
		| value MULT duration
		{
		    $$ = $1;
		    if ($1 -> child)
		        $$ -> len = $1 -> child -> len * $3;
		    else
		        $$ -> len = $3;
		    if (sigcon && $3 < 0) sigendless = TRUE;
		}
		;

value		: LOGIC_LEVEL
		{
		    NEW ($$, 1, SIGNALELEMENT);
		    lastval = $$ -> val = $1;
		    sigcon = FALSE;
		}
		| LPS signal_exp RPS
		{
		    NEW ($$, 1, SIGNALELEMENT);
		    NEW ($$ -> child, 1, SIGNALELEMENT);
		    $$ -> child -> sibling = *--sgn_sp;
		    $$ -> val = $$ -> child -> val = lastval;
		    $$ -> child -> len = *len_sp--;
		    sigcon = TRUE;
		}
		;

duration	: INT
		{
		    $$ = atoll ($1);
		}
		| TILDE
		{
		    $$ = -1;
		}
		;

print_cmd	: PRINT_CMD node_refs
		;

plot_cmd	: PLOT_CMD node_refs
		;

dissip_cmd	: DISSIPATION dis_node_refs
		;

dis_node_refs	: node_refs
		| /* empty */
		;

node_refs	: ref_item
		{
		    begin_str_list = end_str_list = names_from_path (0, fullpath);
		    while (end_str_list -> next) end_str_list = end_str_list -> next;
		}
		| node_refs ref_item
		{
		    end_str_list -> next = names_from_path (0, fullpath);
		    while (end_str_list -> next) end_str_list = end_str_list -> next;
		}
		;

ref_item	: full_node_ref
		{
		    $$ = fullpath;

		    last_path = NULL;  /* for the next call */
		}
		| COMMA
		{
		    $$ = NULL;
		}
		;

full_node_ref 	: member_ref
		| EXCLAM member_ref
		| full_node_ref DOT member_ref
		;

member_ref	: member_name
		{
		    if (last_path == NULL) {
			/* this is the leftmost (the first) member of a path */
			last_path = pathspace;
			fullpath = last_path;
		    }
		    else {
			last_path -> next = last_path + 1;
		        last_path = last_path -> next;
		    }
		    last_path -> next = NULL;
		    last_path -> also = NULL;
		    strcpy (last_path -> name, $1);
		}
		  ref_indices
		;

member_name	: INT
		{
		    strcpy (name_space, $1);   /* copying is necessary */
		    $$ = name_space;
		}
		| IDENTIFIER
		{
		    strcpy (name_space, $1);
		    $$ = name_space;
		}
		| keyword
		{
		    strcpy (name_space, textval ());
		    $$ = name_space;
		}
		;

keyword		: SET
		| LEVEL
		| LOGIC_LEVEL
		| FROM
		| FILL
		| WITH
		| PRINT_CMD
		| PLOT_CMD
		| OPTION
		| SIMPERIOD
		| DISPERIOD
		| DISSIPATION
		| DUMP
		| AT
		| INITIALIZE
		| SIGOFFSET
		| RACES
		| DEVICES
		| STATISTICS
		| ONLY
		| CHANGES
		| SLS_PROCESS
		| SIGUNIT
		| OUTUNIT
		| OUTACC
		| MAXPAGEWIDTH
		| MAXNVICIN
		| MAXTVICIN
		| MAXLDEPTH
		| VH
		| VMAXL
		| VMINH
		| TDEVMIN
		| TDEVMAX
		| TOGGLE
		| STEP
		| RANDOM
		| FULL
		| DEFINE_TOKEN
		| STA_FILE
		;

ref_indices	: /* empty */
		{
		    last_path -> xarray[0][0] = 0;
		}
		| LSB
		{
		    last_path -> xarray[0][0] = 0;
		}
		  index_list RSB
		;

index_list	: index
		| index_list COMMA index
		;

index		: integer
		{
		    last_path -> xarray[0][0]++;
		    last_path -> xarray[last_path -> xarray[0][0]][0] = $1;
		    last_path -> xarray[last_path -> xarray[0][0]][1] = $1;
		}
		| integer DOTDOT integer
		{
		    last_path -> xarray[0][0]++;
		    last_path -> xarray[last_path -> xarray[0][0]][0] = $1;
		    last_path -> xarray[last_path -> xarray[0][0]][1] = $3;
		}
		;

option_cmd	: OPTION option
		| option_cmd option
		;

option		: toggle_option EQL TOGGLE
		| int_option EQL INT
		| SIMPERIOD EQL INT
		{
		    simperiod = atoll ($3);
		}
		| pow_ten_option EQL pow_ten
		| f_float_option EQL f_float
		{
		    if ($1) *$1 = $3;
		}
		| string_option EQL STRING
		;

toggle_option	: STEP
		| PRINT_CMD RACES
		| ONLY CHANGES
		| PRINT_CMD DEVICES
		| PRINT_CMD STATISTICS
		| INITIALIZE RANDOM
		| INITIALIZE FULL RANDOM
		| STA_FILE
		;

int_option	: DISPERIOD
		| MAXNVICIN
		| MAXTVICIN
		| MAXLDEPTH
		| MAXPAGEWIDTH
		| LEVEL
		| SIGOFFSET
		;

pow_ten_option	: OUTUNIT
		| OUTACC
		;

pow_ten		: INT
		| POWER_TEN
		;

f_float_option	: SIGUNIT
		{
		    $$ = &sigtimeunit;
		}
		| VH
		{
		    $$ = NULL;
		}
		| VMAXL
		{
		    $$ = NULL;
		}
		| VMINH
		{
		    $$ = NULL;
		}
		| TDEVMIN
		{
		    $$ = NULL;
		}
		| TDEVMAX
		{
		    $$ = NULL;
		}
		;

f_float		: INT
		{
		    $$ = atof ($1);
		}
		| POWER_TEN
		{
		    $$ = $1;
		}
		| F_FLO
		{
		    $$ = $1;
		}
		;

string_option	: SLS_PROCESS
		;

dump_cmd	: DUMP AT integer
		;

init_cmd	: INITIALIZE FROM STRING
		;

fill_cmd	: FILL full_node_ref WITH fillvals
		;

fillvals	: fillchars
		| fillvals fillchars
		| fillint
		| fillvals fillint
		| fillfloat
		| fillvals fillfloat
		;

fillchars	: STRING
		;

fillint		: INT
		;

fillfloat	: F_FLO
		;

define_cmd      : DEFINE_TOKEN node_refs COLON member_name define_entries
		;

define_entries  : define_entry
		| define_entries define_entry
		;

define_entry    : def_sig_vals COLON escape_char member_name
		;

escape_char     : DOLLAR
		| /* empty */
		;

def_sig_vals    : def_sig_val
		| def_sig_vals def_sig_val
		;

def_sig_val     : LOGIC_LEVEL
		| MINUS
		;

integer		: INT
		{
		    $$ = atoi ($1);
		}
		;

eoc		: SEMICOLON
		| NEWLINE
		;

%%
#include "cmd_l.h"

#ifndef YY_CURRENT_BUFFER
#define YY_CURRENT_BUFFER yy_current_buffer
#endif

void cmdinits ()
{
    /* re-initialize (reset the state of) the lex scanner (yylex) */
#ifdef FLEX_SCANNER
    if (YY_CURRENT_BUFFER) yyrestart (yyin);
#else
    ; /* some versions of lex may require "yysptr = yysbuf" */
#endif
    yylineno = 1;

    len_sp = len_stack;
    sgn_sp = sgn_stack;

    sigendless = FALSE;
    simperiod = -1;
    sigtimeunit = 1;

    Begin_node = NULL;
    End_node = NULL;

    errorDetected = 0;
}

void cslserror (char *s)
{
    int lineno = yychar == NEWLINE ? yylineno-1 : yylineno;

    if (fp_err) {
	if (!errorDetected)
	    fprintf (fp_err, "File \"%s\" parse errors:\n", Commandfile);
	fprintf (fp_err, "line %d: %s\n", lineno, s);
    }
    else if (!errorDetected) {
	char buf[264];
	sprintf (buf, "%s, line %d: %s", Commandfile, lineno, s);
	windowMessage (buf, -1);
    }
    errorDetected = 1;
}

void yyerror (char *s)
{
    cslserror (s);
}

SIGNALELEMENT *copysgn (SIGNALELEMENT *sgn)
{
    SIGNALELEMENT *newsgn;

    NEW (newsgn, 1, SIGNALELEMENT);
    newsgn -> val = sgn -> val;
    newsgn -> len = sgn -> len;

    if (sgn -> sibling)
	newsgn -> sibling = copysgn (sgn -> sibling);
    if (sgn -> child)
	newsgn -> child = copysgn (sgn -> child);

    return (newsgn);
}

void assignExprToNodes ()
{
    STRING_REF *str_ref;
    struct node *n;
    SIGNALELEMENT *sgn;
    int first = 1;

    NEW (sgn, 1, SIGNALELEMENT);
    sgn -> sibling = *--sgn_sp;
    sgn -> val = lastval;
    sgn -> len = *len_sp;

    for (str_ref = begin_str_list; str_ref; str_ref = str_ref -> next) {

	n = Begin_node;
	while (n && !strsame (str_ref -> str, n -> name)) n = n -> next;
	if (n) {
	    cslserror ("signal multipally connected to same node");
	    continue;
	}

	NEW (n, 1, struct node);
	n -> name = str_ref -> str;
	n -> next = NULL;
	n -> no_edit = no_edit;
	if (first) {
	    first = 0;
	    n -> expr = sgn;
	}
	else
	    n -> expr = copysgn (sgn);

	if (End_node)
	    End_node -> next = n;
	else
	    Begin_node = n;
	End_node = n;
    }
}
