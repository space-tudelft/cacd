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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "src/nspice/extern.h"

#define  MAXHIERAR        22
#define  MAXSTACKLENGTH  100

char name_space[128]; /* memory space for a name */

PATH_SPEC  pathspace[MAXHIERAR]; /* memory space for paths */
PATH_SPEC *fullpath;         /* first of the current full path specification */
PATH_SPEC *last_path = NULL; /* last of the current full path specification */

STRING_REF *begin_str_list, *end_str_list;
STRING_REF *names_from_path (int traillen, PATH_SPEC *path); // res.c

int sigcon; /* boolean flag */

short lastval;
int *len_sp;
int len_stack[MAXSTACKLENGTH];
SIGNALELEMENT **sgn_sp;
SIGNALELEMENT *sgn_stack[MAXSTACKLENGTH];

struct node *Begin_node, *End_node;
struct node_ref *Begin_print, *End_print;

char *fn_cmd;

int sigendless;
int simperiod;
int simlevel;
int idummy;
char *cdummy;
double ddummy;
double sigunit;
double vhigh_vh;
int vhigh_vh_spec = 0;

extern int yylineno;

char *textval (void); // cmd_l.l
void assignExprToNodes (void);
void yyerror (char *s);

%}

%union		{
		    int ival;
		    int *pival;
		    char *sval;
		    char **psval;
		    double dval;
		    double *pdval;
		    SIGNALELEMENT *signal;
		    PATH_SPEC *pathsp;
		}

%token 		SET TILDE FROM FILL WITH PRINT PLOT OPTION SIMPERIOD DUMP AT
%token 		DISSIPATION INITIALIZE SIGOFFSET RACES DEVICES STATISTICS
%token 		ONLY CHANGES SLS_PROCESS SIGUNIT OUTUNIT OUTACC MAXPAGEWIDTH
%token 		MAXNVICIN MAXTVICIN MAXLDEPTH VH VMAXL VMINH
%token		TDEVMIN TDEVMAX STEP DISPERIOD RANDOM FULL DEFINE_TOKEN
%token		STA_FILE DOT DOTDOT LPS RPS LSB RSB LCB RCB EQL MINUS DOLLAR
%token 		COMMA SEMICOLON COLON MULT EXCLAM NEWLINE
%token <ival>	LEVEL LOGIC_LEVEL TOGGLE
%token <sval>	IDENTIFIER INT STRING
%token <dval>	POWER_TEN F_FLO

%type  <ival>	duration index integer
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

    		    len_sp = len_stack; /* reset stacks for signal_exp */
    		    sgn_sp = sgn_stack;
		}
		| SET node_refs COLON node_refs FROM STRING
		{
		    yyerror ("warning: unsupported set command");
		}
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

duration	: integer
		{
		    $$ = $1;
		}
		| TILDE
		{
		    $$ = -1;
		}
		;

print_cmd	: PRINT node_refs
		;

plot_cmd	: PLOT node_refs
		{
		    STRING_REF *sref;

		    for (sref = begin_str_list; sref; sref = sref -> next) {
			if (!Begin_print) {
			    NEW (Begin_print, 1, struct node_ref);
			    End_print = Begin_print;
			}
			else {
			    NEW (End_print -> next, 1, struct node_ref);
			    End_print = End_print -> next;
			}
			End_print -> name = sref -> str;
			End_print -> next = NULL;
		    }
		}
		;

dissip_cmd	: DISSIPATION dis_node_refs
		;

dis_node_refs	: node_refs
		| /* empty */
		;

node_refs	: ref_item
		{
		    begin_str_list = NULL;

		    if ($1) {
			if (begin_str_list) {
			    end_str_list -> next = names_from_path (0, fullpath);
			    while (end_str_list -> next)
				end_str_list = end_str_list -> next;
			}
			else {
			    begin_str_list = end_str_list = names_from_path (0, fullpath);
			    while (end_str_list -> next)
				end_str_list = end_str_list -> next;
			}
		    }
		}
		| node_refs ref_item
		{
		    if ($2) {
			if (begin_str_list) {
			    end_str_list -> next = names_from_path (0, fullpath);
			    while (end_str_list -> next)
				end_str_list = end_str_list -> next;
			}
			else {
			    begin_str_list = end_str_list = names_from_path (0, fullpath);
			    while (end_str_list -> next)
				end_str_list = end_str_list -> next;
			}
		    }
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
		| LEVEL { idummy = $1; }
		| LOGIC_LEVEL { idummy = $1; }
		| FROM
		| FILL
		| WITH
		| PRINT
		| PLOT
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
		| TOGGLE { idummy = $1; }
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

index_list	: index { idummy = $1; }
		| index_list COMMA index { idummy = $3; }
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
		| SIMPERIOD EQL INT
		{
		    simperiod = atoi ($3);
		}
		| LEVEL EQL INT
		{
		    simlevel = atoi ($3);
		}
		| int_option EQL INT
		| pow_ten_option EQL pow_ten
		| f_float_option EQL f_float
		{
		    if ($1) *$1 = $3;
		}
		| string_option EQL STRING
		;

toggle_option	: STEP
		| PRINT RACES
		| ONLY CHANGES
		| PRINT DEVICES
		| PRINT STATISTICS
		| INITIALIZE RANDOM
		| INITIALIZE FULL RANDOM
		| STA_FILE
		;

int_option	: DISPERIOD
		| MAXNVICIN
		| MAXTVICIN
		| MAXLDEPTH
		| MAXPAGEWIDTH
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
		    $$ = &sigunit;
		}
		| VH
		{
		    $$ = &vhigh_vh;
		    vhigh_vh_spec = 1;
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

fill_cmd	: FILL full_node_ref WITH
		  fillvals
		;

fillvals	: fillchars
		| fillvals fillchars
		| fillint
		| fillvals fillint
		| fillfloat
		| fillvals fillfloat
		;

fillchars	: STRING { cdummy = $1; }
		;

fillint		: integer { idummy = $1; }
		;

fillfloat	: F_FLO { ddummy = $1; }
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

def_sig_val     : LOGIC_LEVEL { idummy = $1; }
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

void cmdinits ()
{
    len_sp = len_stack;
    sgn_sp = sgn_stack;

    sigendless = FALSE;
    simperiod = -1;
    sigunit = -1;

    Begin_node  = End_node  = NULL;
    Begin_print = End_print = NULL;
}

void cslserror (char *s1, char *s2)
{
    char buf[264];
    int lineno = yylineno;

    if (yychar == NEWLINE) lineno--;

    sprintf (buf, "%s, line %d: %s %s", fn_cmd, lineno, s1, s2);
    message (buf, 0, 0);
}

void yyerror (char *s)
{
    cslserror (s, "");
}

void assignExprToNodes ()
{
    STRING_REF *str_ref;
    struct node *n;
    SIGNALELEMENT *sgn;

    NEW (sgn, 1, SIGNALELEMENT);
    sgn -> sibling = *--sgn_sp;
    sgn -> val = lastval;
    sgn -> len = *len_sp;

    for (str_ref = begin_str_list; str_ref; str_ref = str_ref -> next) {

	/* perform linear search to find node */

	n = Begin_node;
	while (n && !strsame (str_ref -> str, n -> name)) n = n -> next;

        /* add expression to current expression of node or create new node */

	if (n) {
	    cslserror ("signal already attached to node", n -> name);
	}
	else {
	    NEW (n, 1, struct node);
	    n -> name = str_ref -> str;
	    n -> expr = sgn;
	    n -> next = NULL;
	    if (End_node) {
		End_node -> next = n;
		End_node = n;
	    }
	    else {
		Begin_node = End_node = n;
	    }
	}
    }
}

int yywrap ()
{
    return (1);
}
