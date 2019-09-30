%{
static char *rcsid = "$Id: yacc.y,v 1.1 2018/04/30 12:17:56 simon Exp $";
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
/*
 * Yacc input specification.
 * The grammer specified is used to parse an
 * SLS description of a circuit.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	FUNCTIONS:
 */
Private void resynch (int c);

/*	VARIABLES:
 */
Import	int	 yydebug, yylineno;
Import	FILE	*yyin;
Import  long  n_list_alloc;

Public	string	string_cap  = "cap";
Public	string	string_res  = "res";
Public	string	string_nenh = "nenh";
Public	string	string_penh = "penh";
Public	string	string_ndep = "ndep";
Public	string	string_inv  = "invert";
Public	string	string_nand = "nand";
Public	string	string_nor  = "nor";
Public	string	string_and  = "and";
Public	string	string_or   = "or";
Public	string	string_exor = "exor";
Public	string	string_l = "l";
Public	string	string_n = "n";
Public	string	string_p = "p";
Public	string	string_v = "v";
Public	string	string_w = "w";
Public	string	string_x = "x";
Public	string	string_y = "y";

Public	network *cur_network;
Private	object  *cur_object;
Private	object  *cur_device;
Private	int	 cur_type, conn_mode;
Private	int      ignore, attr_cnt, inst_cnt;
Private string	 attr_name[10], attr_value[10];
Private int add_instance (string iname, string cname, int cnt);

Public	hash	*nlist;
Import	hash	*def_h;
Import	stack	*net_st;
Import	string   cur_file;
Import	boolean  r_opt;
Import	boolean  c_opt;

#ifdef DEBUG
#define YYDEBUG 1
#endif

#define PRIME		1009

%}

%union	{
	int		ival;
	string		sval;
	list	       *lstp;
	pair           *prpt;
}

%token	<sval>	IDENTIFIER INTEGER FLOAT
%token		NETWORK FUNCTION PRIM TERMNL NT EXT PERM
%token 		NENH PENH NDEP RES CAP

%type   <ival>  type_id i_val
%type	<sval>	ttype node_name inst_name name attr_val ident
%type	<sval>	inst_def tor_def res_def cap_def fun_def call_def
%type   <sval>  f_float netw_name
%type   <lstp>  class_list class atom_list vnode_list
%type   <lstp>  ps_node_list net_node_list connect_list connects
%type   <lstp>  connect internal_ref node_ref
%type   <prpt>  sb_index_list index_list index
%type   <prpt>  sb_range_list range_list range

%start sls_descr

%%

sls_descr	: netw_descr
		| sls_descr netw_descr
		;
netw_descr	: netw_head decl_part netw_body
			{
				list *lp, *tmp;

				if (nlist) { rm_hash (nlist); nlist = NULL; }
				inst_cnt = 0;

				if (h_get (def_h, cur_network -> name)) {
					verb_mesg ("Network: '%s' already extracted\n", cur_network -> name);
					rm_network (cur_network);
					cur_network = NULL;
					while ((lp = (list *) pop (net_st))) {
					    do { lp = (tmp = lp) -> cdr; rm_list (tmp); } while (lp);
					}
				}
				else {
					verb_mesg ("Network: '%s' extracted\n", cur_network -> name);
					h_link (def_h, cur_network -> name, cur_network);
					while ((lp = (list *) pop (net_st))) merge (cur_network, lp);
				}
			}
		;

netw_head       : type_id NETWORK netw_name
			{
				ignore = ($1 == PRIMITIVE)? 3 : 0;
				cur_network = mk_network ($3, $1);
				if (!ignore) nlist = mk_hash (PRIME);
			}
		| type_id FUNCTION netw_name
			{
				char nme[DM_MAXNAME+2];
				sprintf (nme, "@%s", $3);
				cur_network = mk_network (nme, PRIMITIVE);
				ignore = 3;
			}
		;
type_id		: /* empty */	{ $$ = COMPOUND; }
		| PRIM		{ $$ = PRIMITIVE; }
		| EXT		{ $$ = PRIMITIVE; }
		;

decl_part	: '(' type_decl_list ')'
		| error {
		    		resynch (')');
		    		yyerrok;
		    		yyclearin;
			}
		;

netw_body	: '{' stmt_list '}'
		| '{' PERM p_expr ';' '}'
		| /* empty */
		;

p_expr		: '{' class_list '}'
			{
				list *class, *lp, *next;

if (!ignore) user_mesg ("In sls file '%s' line %d: warning: network '%s' not primitive\n", cur_file, yylineno, cur_network->name);
				set_termcol (cur_network, $2);
				for (class = $2; class; class = next) {
				    for (lp = class -> car; lp; lp = next) {
					next = lp -> cdr;
					rm_list (lp);
				    }
				    next = class -> cdr;
				    rm_list (class);
				}
			}
		;
class_list	: class {
				$$ = add_list (NULL, $1);
			}
		| class_list class
			{
				$$ = add_list ( $1 , $2);
			}
		;
class		: '(' atom_list ')'
			{
				$$ = $2;
			}
		;
atom_list	: ident {
				object *term = is_term (cur_network, $1);
				if (!term) err_mesg ("Network '%s', perm: terminal '%s' undefined\n", cur_network->name, $1);
				rm_symbol ($1);
				$$ = add_list (NULL, term);
			}
		| atom_list ',' ident
			{
				object *term = is_term (cur_network, $3);
				if (!term) err_mesg ("Network '%s', perm: terminal '%s' undefined\n", cur_network->name, $3);
				rm_symbol ($3);
				$$ = add_list ( $1 , term);
			}
		;
ident		: IDENTIFIER { $$ = $1; }
		| IDENTIFIER '[' i_val ']'
			{
				char buf[100];
				sprintf (buf, "%s[%d]", $1, $3);
				rm_symbol ($1);
				$$ = mk_symbol (buf);
			}
		;

type_decl_list	: type_decl
		| type_decl_list ';' type_decl
		;
type_decl	: TERMNL term_decl_list
		;
term_decl_list	: term_decl
		| term_decl_list ',' term_decl
		;
term_decl	: name	{
				cur_object = mk_object ($1, TERMINAL);
				rm_symbol ($1);
			}
		  sb_range_list
			{
				pair *p, *next;

				for (p = $3; p; p = next) {
					next = p -> next;
					array (cur_object, p->first, p->last);
					rm_pair (p);
				}
				add_object (cur_network, cur_object);
			}
		;

stmt_list	: /* empty */
		| stmt_list statement
		;
statement	:  net_stmt ';'
		| inst_stmt ';'
		| /* nul */ ';'
		| error {
		    		resynch (';');
		    		yyerrok;
		    		yyclearin;
			}
		;

net_stmt	: NT '{' net_spec '}'
		;
net_spec	: vnode_list
			{
			    if ($1) {
				list *ThiS, *next, *prev, *head;

				if ($1 -> cdr) { /* merge element wise */
				    for (next = $1; next; next = next -> cdr) {
					while (next -> car) {
					    head = NULL;
					    for (ThiS = next; ThiS; ThiS = ThiS -> cdr) {
						if ((prev = ThiS -> car)) {
						    ThiS -> car = prev -> cdr;
						    prev -> cdr = head;
						    head = prev;
						}
					    }
					    push (net_st, head);
					}
				    }
				}
				else { /* only define */
				    for (ThiS = $1 -> car; ThiS; ThiS = next) {
					next = ThiS -> cdr; rm_list (ThiS);
				    }
				}
				for (ThiS = $1; ThiS; ThiS = next) {
				    next = ThiS -> cdr; rm_list (ThiS);
				}
			    }
			}
		| net_node_list			/* merge nets to one */
			{
				if ($1) push (net_st, invert_list ($1));
			}
		;
vnode_list	: ps_node_list { $$ = $1; }	/* define only */
		| vnode_list ',' ps_node_list	/* cluster operation */
			{
				if ($3) $3 -> cdr = $1;
				$$ = $3;
			}
		;
ps_node_list	: '(' net_node_list ')'
			{
				$$ = $2 ? add_list (NULL, invert_list ($2)) : NULL;
			}
		;
net_node_list	: node_ref { $$ = $1; }
		| net_node_list ',' node_ref
			{
				if ($3) {
				    list *l = $3;
				    while (l -> cdr) l = l -> cdr;
				    l -> cdr = $1;
				}
				$$ = $3;
			}
		;

inst_stmt	: '{' inst_name sb_range_list '}' inst_def connect_list
			{
				pair *next, *p;
				list *l, *d, *n, *m;
				int n_devs, cnt, nt;

				++inst_cnt;

				/* count number of devices */
				n_devs = 1;
				for (p = $3; p; p = p -> next) {
				    if (p->last > p->first) n_devs *= (p->last - p->first) + 1;
				    if (p->last < p->first) n_devs *= (p->first - p->last) + 1;
				}

				if (!ignore) {
				    /* count number of nets */
				    cnt = 0; for (l = $6; l; l = l -> cdr) ++cnt;
				    nt = add_instance ($2, $5, cnt / n_devs);
if (cnt != n_devs * nt) err_mesg ("In sls file '%s' line %d: incorrect number of nets\n", cur_file, yylineno);
				    for (p = $3; p; p = next) {
					next = p -> next;
					array (cur_device, p->first, p->last);
					rm_pair (p);
				    }
				    netw_connect (cur_device, $6, cnt / n_devs, conn_mode);
				    add_object (cur_network, cur_device);
				}
				else if (ignore == 1) { /* RES */
				    if (conn_mode == INST_MAJOR) {
					cnt = 0;
					for (l = $6; l && l->cdr; l = n) {
					    d = l->cdr; n = d->cdr;
					    l->cdr = NULL; d->cdr = l; /* invert_list */
					    push (net_st, d);
					    ++cnt;
					}
if (l || cnt != n_devs) err_mesg ("In sls file '%s' line %d: incorrect number of nets\n", cur_file, yylineno);
				    }
				    else {
					d = l = $6;
					for (cnt = 0; cnt < n_devs && d; ++cnt) d = d -> cdr;
					for (cnt = 0; cnt < n_devs && d; ++cnt) {
					    n = l->cdr; m = d->cdr;
					    l->cdr = NULL; d->cdr = l; /* invert_list */
					    push (net_st, d);
					    l = n; d = m;
					}
if (d || cnt != n_devs) err_mesg ("In sls file '%s' line %d: incorrect number of nets\n", cur_file, yylineno);
				    }
				    ignore = 0;
				}
				else if (ignore == 2) { /* CAP */
				    ignore = 0;
				}
				attr_cnt = 0;
			}
		| inst_def connect_list
			{
				char iname[20];
				int cnt, nt;
				list *l, *d;

				++inst_cnt;

				if (!ignore) {
				    /* count number of nets (device pins) */
				    cnt = 0; for (l = $2; l; l = l -> cdr) ++cnt;
				    sprintf (iname, "_%u", inst_cnt);
				    nt = add_instance (iname, $1, cnt);
if (cnt != nt) err_mesg ("In sls file '%s' line %d: incorrect number of nets\n", cur_file, yylineno);
				    netw_connect (cur_device, $2, cnt, INST_MAJOR);
				    add_object (cur_network, cur_device);
				}
				else if (ignore == 1) { /* RES */
				    l = $2; d = l->cdr;
if (!d || d->cdr) err_mesg ("In sls file '%s' line %d: incorrect number of nets\n", cur_file, yylineno);
				    l->cdr = NULL; d->cdr = l; /* invert_list */
				    push (net_st, d);
				    ignore = 0;
				}
				else if (ignore == 2) { /* CAP */
				    ignore = 0;
				}
				attr_cnt = 0;
			}
		;

inst_def	: tor_def | res_def | cap_def | fun_def | call_def
		;
tor_def		: ttype    attr_list { cur_type = 'd'; $$ = $1; }
		;
fun_def		: '@' name attr_list { cur_type = 'f'; $$ = $2; }
		;
res_def		: RES f_float { cur_type = 'r'; $$ = $2; if (r_opt && !ignore) ignore = 1; }
		;
cap_def	        : CAP f_float { cur_type = 'c'; $$ = $2; if (c_opt && !ignore) ignore = 2; }
		;
call_def        : name	{ cur_type = 'n'; $$ = $1; }
		;

netw_name	: ttype	{ $$ = $1; }
		| RES	{ $$ = string_res; }
		| CAP	{ $$ = string_cap; }
		| name	{ $$ = $1; }
		;
ttype		: NENH	{ $$ = string_nenh; }
		| PENH	{ $$ = string_penh; }
		| NDEP	{ $$ = string_ndep; }
		;

attr_list	: /* empty */
		| attr_list attribute
		;
attribute	: name '=' attr_val
		{
			attr_name [attr_cnt] = $1;
			attr_value[attr_cnt] = $3;
			attr_cnt++;
		}
		;
attr_val	: INTEGER    { $$ = $1; }
		| IDENTIFIER { $$ = $1; }
		| FLOAT      { $$ = $1; }
		;

f_float		: INTEGER { $$ = $1; }
		| FLOAT   { $$ = $1; }
		;

connect_list	: '(' connects ')' { $$ = invert_list ($2); conn_mode = INST_MAJOR; }
		| '{' connects '}' { $$ = invert_list ($2); conn_mode = PARM_MAJOR; }
		;
connects	: connect { $$ = $1; }
		| connects ',' connect
			{
			    if ($3) {
				list *ThiS = $3;

				/* determine tail of list */
				while (ThiS -> cdr) ThiS = ThiS -> cdr;
				ThiS -> cdr = $1; /* merge lists */
			    }
			    $$ = $3;
			}
		;
connect		: internal_ref	{ $$ = $1; }
		| node_ref	{ $$ = $1; }
		| /* empty */	{ $$ = (ignore < 2)? add_list (NULL, NULL) : NULL; }
		;
internal_ref	: sb_index_list '.' node_ref
				{ $$ = $3; } /* Not Yet Implemented */
		;

node_ref	: node_name sb_index_list
			{
			    list	*head = NULL;
			    char	nme[DM_MAXNAME+20];
			    object	*tmp;
			    pair *cur_pair, *sub_pairs;

			    if (ignore < 2) {
				if ((sub_pairs = $2)) {
				    do {
					make_index ($1, sub_pairs, nme);
					if (!(tmp = is_net (cur_network, nme))) {
					    tmp = mk_object (nme, NET); add_object (cur_network, tmp);
					}
					head = add_list (head, tmp);
				    } while (incr_index (sub_pairs));
				    /* clean up */
				    do {
					cur_pair = sub_pairs;
					sub_pairs = sub_pairs -> next;
					rm_pair (cur_pair);
				    } while (sub_pairs);
				}
				else {
				    if (!(tmp = is_net (cur_network, $1))) {
					tmp = mk_object ($1, NET); add_object (cur_network, tmp);
				    }
				    head = add_list (head, tmp);
				}
			    }
			    $$ = head;
			}
		;

node_name	: INTEGER { $$ = $1; }
		| name    { $$ = $1; }
		;

inst_name	: name { $$ = $1; }
		| '.'  { $$ = mk_symbol ("."); }
		;

name		: IDENTIFIER { $$ = $1; }
		;

sb_index_list	: /* empty */ { $$ = NULL; }
		| '[' index_list ']' { $$ = $2; }
		;
index_list	: index { $$ = $1; }
		| index_list ',' index
			{
			    if ($3) $3 -> next = $1;
			    $$ = $3;
			}
		;
index		: i_val {
			    if (ignore < 2) {
				$$ = mk_pair();
				$$ -> first = $1;
				$$ -> last  = $1;
				$$ -> index = $1;
			    }
			    else
			        $$ = NULL;
			}
		| range { $$ = $1; }
		;

sb_range_list	: /* empty */ { $$ = NULL; }
		| '[' range_list ']' { $$ = $2; }
		;
range_list	: range { $$ = $1; }
		| range_list ',' range
			{
			    if ($3) $3 -> next = $1;
			    $$ = $3;
			}
		;
range		: i_val '.' '.' i_val
			{
			    if (ignore < 2) {
				$$ = mk_pair();
				$$ -> first = $1;
				$$ -> last  = $4;
				$$ -> index = $1;
			    }
			    else
				$$ = NULL;
			}
		;

i_val		: INTEGER { $$ = atoi ($1); rm_symbol ($1); }
		;
%%

#include "lex.h"

void yyerror (char *s)
{
    fprintf (stderr, "%s in line %d of sls file '%s'\n", s, yylineno, cur_file);
    my_exit (1);
}

/*
 * Creates a pair structure.
 */
Public pair *mk_pair()
{
	pair *p;
	Malloc (p, 1, pair);
	p -> next  = NULL;
	p -> first = 0;
	p -> last  = 0;
	p -> index = 0;
	return (p);
}

/*
 * Removes a structure of type pair.
 */
Public void rm_pair (pair *p)
{
	Free (p);
}

/*
 * Creates a new list structure on top of 'prev' list
 * Returns the new top list pointer.
 */
Public list *add_list (list *prev, void *item)
{
	list *ThiS = mk_list();
	ThiS -> car = item;
	ThiS -> cdr = prev;
	return ThiS;
}

/*
 * Creates a structure of type list
 * and returns a pointer to it.
 */
Private list * empty_lists = NULL;

Public list *mk_list()
{
	list *ThiS;

	if (empty_lists) {
	    ThiS = empty_lists;
	    empty_lists = ThiS -> cdr;
	}
	else {
	    Malloc (ThiS, 1, list);
	}

	n_list_alloc++;
	return(ThiS);
}

/*
 * Deletes a structure of type list.
 */
Public void rm_list (list *ThiS)
{
    ThiS -> cdr = empty_lists;
    empty_lists = ThiS;
    n_list_alloc--;
}

Private void resynch (int c)
{
    int c1, newline = 0;

    fprintf (stderr, " \"");
    if (yytext) fprintf (stderr, "%s", yytext);
    while ((c1 = input()) != c) {
	if (!(newline = (newline || ((c1=='\n')?1:0))))
        fprintf (stderr, "%c", c1);
    }
    fprintf (stderr, "\"\n");
}

/*
 * Inverts the order of the specified single linked
 * list and returns a pointer to the new list head.
 */
Public list *invert_list (list *next)
{
    list *head, *prev;

    head = NULL;

    while (next)
    {
	prev = head; 	/* prev trails head */
	head = next;	/* head trails next */
	next = head -> cdr;
	head -> cdr = prev; /* link head to prev node */
    }

    return (head);
}

/*
 * Returns the name extended with the proper
 * indices indicated by "list".
 */
Public void make_index (string name, pair *a_list, string work)
{
	long	stck[20];
	pair	*p;
	int i, sp = 0;

	for (p = a_list; p; p = p -> next) stck[sp++] = p->index;
	if (sp) {
		sprintf (work, "%s[", name);
		i = strlen (work);
		while (--sp) {
			sprintf (work+i, "%ld,", stck[sp]);
			i += strlen (work+i);
		}
		sprintf (work+i, "%ld]", stck[sp]);
	}
	else
		strcpy (work, name);
}

/*
 * Increments the set of indices of 'a_list' by one.
 * When no increment is possible (all indices have
 * reached their upper bound) False is returned.
 */
Public boolean incr_index (pair *a_list)
{
	pair *p;

	for (p = a_list; p; p = p -> next) {
	    if (p -> last > p -> first) {
		if (++p -> index <= p -> last) return (True);
	    } else {
		if (--p -> index >= p -> last) return (True);
	    }
	    p -> index = p -> first;
	}
	return (False);
}

Private network *check_elm (string elm)
{
    network *n_network;
    object  *term;

    if (!(n_network = h_get (def_h, elm))) { /* cap and res */
	n_network = mk_network (elm, PRIMITIVE);
	term = mk_object (string_p, TERMINAL); add_object (n_network, term); term -> color = 2;
	term = mk_object (string_n, TERMINAL); add_object (n_network, term); term -> color = 2;
	h_link (def_h, n_network -> name, n_network);
    }
    return n_network;
}

Private network *check_tor (string nme, int cnt)
{
    network *n_network;
    object  *term;

    if (!(n_network = h_get (def_h, nme))) { /* nenh, penh, ndep */
	long prime;
	n_network = mk_network (nme, PRIMITIVE);
	rewind_gen ();
	prime = gen_prime ();
	term = mk_object ("g", TERMINAL); add_object (n_network, term); term -> color = prime;
	prime = gen_prime ();
	term = mk_object ("d", TERMINAL); add_object (n_network, term); term -> color = prime;
	term = mk_object ("s", TERMINAL); add_object (n_network, term); term -> color = prime;
	if (cnt == 4) {
	    prime = gen_prime ();
	    term = mk_object ("b", TERMINAL); add_object (n_network, term); term -> color = prime;
	}
	else Assert (cnt == 3);
	h_link (def_h, n_network -> name, n_network);
    }
    return n_network;
}

Private network *check_fun (string name, int ninputs)
{
    char nme[DM_MAXNAME+3];
    network *n_network;
    object  *term;

    if (strcmp (name, string_inv) == 0
     || strcmp (name, string_nand) == 0
     || strcmp (name, string_nor) == 0
     || strcmp (name, string_and) == 0
     || strcmp (name, string_or) == 0
     || strcmp (name, string_exor) == 0) {

	Assert (ninputs > 0 && ninputs < 10);
	if (ninputs > 1) {
	    sprintf (nme, "@%s%d", name, ninputs);
	} else {
	    sprintf (nme, "@%s", name);
	}
	if (!(n_network = h_get (def_h, nme))) {
	    long prime;
	    int i;
	    n_network = mk_network (nme, PRIMITIVE);
	    rewind_gen ();
	    prime = gen_prime ();
	    for (i = 0; i < ninputs; ++i) {
		sprintf (nme, "i[%d]", i);
		term = mk_object (nme, TERMINAL); add_object (n_network, term); term -> color = prime;
	    }
	    term = mk_object ("o", TERMINAL); add_object (n_network, term); term -> color = gen_prime ();
	    h_link (def_h, n_network -> name, n_network);
	}
    }
    else {
	sprintf (nme, "@%s", name);
	if (!(n_network = h_get (def_h, nme)))
	    err_mesg ("In file '%s' line %d: function '%s' undefined\n", cur_file, yylineno, name);
    }
    return n_network;
}

Private int add_instance (string iname, string cname, int cnt)
{
    network *n_network;
    int nt = 2;

    switch (cur_type) {
    case 'c':
	if (c_opt) break;
	n_network = check_elm (string_cap);
	cur_device = mk_device (iname, n_network);
	set_par (cur_device, string_v, cname);
	break;
    case 'r':
	if (r_opt) break;
	n_network = check_elm (string_res);
	cur_device = mk_device (iname, n_network);
	set_par (cur_device, string_v, cname);
	break;
    case 'd':
	n_network = check_tor (cname, cnt);
	cur_device = mk_device (iname, n_network);
	while (--attr_cnt >= 0) set_par (cur_device, attr_name[attr_cnt], attr_value[attr_cnt]);
	nt = n_network -> n_terms;
	break;
    case 'f':
	n_network = check_fun (cname, cnt - 1);
	cur_device = mk_device (iname, n_network);
	while (--attr_cnt >= 0) set_par (cur_device, attr_name[attr_cnt], attr_value[attr_cnt]);
	nt = n_network -> n_terms;
	break;
    default:
	Assert (cur_type == 'n');
	if (!(n_network = h_get (def_h, cname)))
	    err_mesg ("In file '%s' line %d: network '%s' undefined\n", cur_file, yylineno, cname);
	cur_device = mk_device (iname, n_network);
	nt = n_network -> n_terms;
    }
    return nt;
}
