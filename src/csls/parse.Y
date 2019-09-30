%{
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

extern Netelem *notconnected;

extern Network *dev_tab[];

extern Stack 	*xs;
extern Stack 	*attr_s;

int n_net_cnt;
int n_inst_cnt = 0;
extern int n_term_cnt;

Stack   *pxs;
Link	*pq;
Queue	*pnetq;
extern Dictionary *ntw_dict;
extern Dictionary *sym_dict;
extern Dictionary *inst_dict;
extern Dictionary *dff_dict;

extern char **globNets;
extern int globNets_cnt;

#ifdef __cplusplus
  extern "C" {
#endif

void yyerror (const char *s);

#ifdef __cplusplus
  }
#endif

Netelem	*pnet;
Network *ntw;
Network *pntw;
NetReference *pnref;
NetworkInstance *inst = NULL;

char name_buf[BUFSIZ];
char ntw_name[BUFSIZ];
char attr_buf[BUFSIZ];

int fflag = 0;       /* Function flag, set if instantiation is functional    */
int bifflag = 0;     /* Built In Function flag, set for nor, or, etc.        */
int invertbifflag;   /* Inverter built in function flag, set for inv         */
int trunc_warn = 0;  /* Set if name trunc. warning given, prevents multiple  */
int externspec = 0;  /* Set if network is specified as extern                */
int mc_type;         /* Indicates what type of model call is being parsed.
                        Used for write to sim_mc stream. */
int almost_eos = 0;  /* Indicates almost end of statement */
static int TermIOType;

%}

%union {
	long   ival;
	char *sval;
	char cval;
	long *pival;
	float fval;
	double dval;
	}

%token <sval> INTEGER IDENTIFIER FLOAT

%token NETWORK EXTERNSPEC TERMINAL NET
%token NENH PENH NDEP RES CAP
%token SEMICOLON COLON COMMA PERIOD
%token LPS RPS LCB RCB LSB RSB
%token T_WIDTH T_LENGTH T_RISE T_FALL
%token ATSIGN ASSIGN DOTDOT

%type <sval> sb_index_list index
%type <sval> decl_part type_decl_list type_decl
%type <sval> type_net_list net_grp_list net_grp net_list net_ref
%type <sval> type_con_list connect_list connect internal_ref inst_net_ref
%type <sval> terminal_list terminal ttype netw_head
%type <ival> i_val type
%type <sval> inst_stmt inst_struct inst_def
%type <sval> named_instance unnamed_instance instance
%type <sval> tor_instance res_instance cap_instance ntw_instance fun_instance
%type <sval> attr_list
%type <dval> f_val attr_val real
%type <sval> attribute attr_spec
%type <sval> i_name net_name ntw_name ident keyw1 keyw2

%start sls_descr

%%

sls_descr	: netw_descr
		| sls_descr netw_descr
		;

netw_descr	: netw_head decl_part netw_body
		{
			if (end_ntw ((Network *)$1, externspec, COMPLETE))
			{
			    sls_error(yylineno, sls_errno, NULL);
			    die();
			}
		}
		| EXTERNSPEC
		{
			externspec = 1;
		}
		  netw_head decl_part
		{
			finalCheckDbTerm ();

			if (end_ntw ((Network *)$3, externspec, COMPLETE))
			{
			    sls_error(yylineno, sls_errno, NULL);
			    die();
			}

			externspec = 0;
		}
		;

netw_head	: NETWORK ntw_name
		{
			if (!externspec && forbidFirstCapital && $2
			    && $2[0] >= 'A' && $2[0] <= 'Z') {
			    sls_errno = USENONCAP;
			    sls_error (yylineno, sls_errno, $2);
			    die ();
			}

		     /* If network is not already referenced, initialze and set
		        it up. If not extern, dmCheckOut called and term, mc,
			and net streams opened. */

			if ((ntw = (Network *)init_ntw($2, externspec))) {
			    $$ = (char*) ntw;
			}
			else {
			    /* Network already referenced */
			    sls_errno = NTWAREF;
			    sls_error(yylineno, sls_errno, $2);
			    die();
			}
		}
		;

decl_part: LPS type_decl_list RPS
	{
	    int i;

	    if ($2) {
		for (i = 0; i < globNets_cnt; i++) {

		    if (externspec && !isDefGlobalNet (globNets[i])) continue;

		    if (!(pnet = (Netelem *)
				(sym_dict -> fetch (globNets[i])))) {

			/* add net 'globNets[i]' as an extra
			   terminal to the current network */

			pnet = new Netelem (globNets[i], (Stack *)0, TermType);
			pnet -> type = T_global;
			ntw -> termq -> put ((Link *) pnet);
			pnet = new Netelem (globNets[i], (Stack *)0, TermType);
			pnet -> type = T_global;
			sym_dict -> store (pnet -> name, (char *) pnet);
			ntw -> netq -> put ((Link *) pnet);
		    }
		    else if (pnet -> xs) {
			sls_errno = ILLRANGE;
			sls_error (yylineno, sls_errno, globNets[i]);
			$2 = 0;
			break;
		    }
		}

		if (!externspec && $2) term_to_db (ntw -> termq);
	    }
	}
	| LPS RPS
	{
	    int i;

	    for (i = 0; i < globNets_cnt; i++) {

		if (externspec && !isDefGlobalNet (globNets[i])) continue;

		/* add net 'globNets[i]' as an extra
		   terminal to the current network */

		if (!ntw -> termq) ntw -> termq = new Queue (QueueType);

		pnet = new Netelem (globNets[i], (Stack *)0, TermType);
		pnet -> type = T_global;
		ntw -> termq -> put ((Link *) pnet);
		pnet = new Netelem (globNets[i], (Stack *)0, TermType);
		pnet -> type = T_global;
		sym_dict -> store (pnet -> name, (char *) pnet);
		ntw -> netq -> put ((Link *) pnet);
	    }

	    if (!externspec) term_to_db (ntw -> termq);
	}
	;

netw_body	: LCB stmt_list RCB
		;

type_decl_list	: type_decl
		{
			$$ = $1;
		}
		| type_decl_list SEMICOLON type_decl
		{
			$$ = $3 ? $1 : 0;
		}
		;

type_decl	: type terminal_list
		{
		    $$ = $2;

		    if ($1 == TermType && $2) {
			pnetq = (Queue *) $2;
			int n;

			if (TermIOType && (n = pnetq -> length ()) > 0) {
			    pq = pnetq -> first_elem ();
			    for (;;) {
				pnet = (Netelem *) pq;
				pnet -> type = TermIOType;
				if (--n == 0) break;
				pq = pnetq -> next_elem (pq);
			    }
			}

			if (ntw -> termq)
			    ntw -> termq -> append (pnetq);
			else
			    ntw -> termq = pnetq;
		    }
		}
		;

terminal_list	: terminal
		{
			if ($1) {
			    pnetq = new Queue (QueueType);
			    pnetq->put((Link *) $1);
			    $$ = (char*) pnetq;
			}
                        else
			    $$ = 0;
		}
		| terminal_list COMMA terminal
		{
			if ($1 && $3)
			    ((Queue *) $1)->put((Link *) $3);
			$$ = $1;
		}
		;

terminal	: net_name sb_index_list
		{
			if(sym_dict->fetch($1))
			{
			    /* terminal already defined */
			    sls_errno = TERMADEF;
			    sls_error(yylineno, sls_errno, $1);
			    $$ = 0;
			}
			else
			{
			    pnet = new Netelem ($1, stackcpy((Stack *) $2),
				       		TermType);
			    if (externspec && checkDbTerm (pnet) != 0)
				$$ = 0;
			    else {
				$$ = (char*) pnet;
				pnet = new Netelem ($1, stackcpy((Stack *) $2),
						    TermType);
				sym_dict->store(pnet->name, (char *) pnet);
				ntw -> netq -> put ((Link *) pnet);
			    }
			}
		}
		;

type		: TERMINAL
		{
			$$ = TermType; TermIOType = 0;
		}
		| IDENTIFIER
		{
		    char *s = $1;

		    if (strcmp (s, "input") == 0) {
			$$ = TermType; TermIOType = T_input;
		    }
		    else if (strcmp (s, "output") == 0) {
			$$ = TermType; TermIOType = T_output;
		    }
		    else if (strcmp (s, "inout") == 0) {
			$$ = TermType; TermIOType = T_inout;
		    }
		    else if (strcmp (s, "tristate") == 0) {
			$$ = TermType; TermIOType = T_tristate;
		    }
		    else {
			/* unknown type. Occurs if keyword terminal omitted. */
			sls_errno = UNKTYPE;
			sls_error(yylineno, sls_errno, $1);
			$$ = 0;
		    }
		}
		;

stmt_list	: /* empty */
		| stmt_list statement
		;

statement	: net_stmt SEMICOLON
		{
			almost_eos = 0;
		}
		| inst_stmt SEMICOLON
		{
			if(inst)
			{
			    if(ntw->mcq == NULL)
				ntw -> mcq = new Queue (QueueType);

			    ntw -> mcq -> put ((Link *) inst);
			    inst_to_db((NetworkInstance *) inst);
			}

                        inst = NULL;
			almost_eos = 0;
		}
		| SEMICOLON
		| error
		{
		    	if(resynch(';'))
			{
		    	    yyerrok;
		    	    yyclearin;
			}
			else
			    die();
		}
		;

inst_stmt	: inst_def type_con_list
		{
		    int net_cnt;
		    int inst_cnt;
		    Stack * inst_xs;
		    inst = (NetworkInstance *) $1;

		    if ($1 && $2) {
                        if (((Link *) $1) -> type == FunctionType) {
				mc_type = 'f';   /* model call is functional */
			        attr_s -> push (strsav ((char*)"f"));
		        }

		        if (bifflag)
			{
				Xelem *pxelem;

				mc_type = 'b'; /* model call - built in func */
			        inst_xs = inst -> inst_struct -> inst_construct;
			        net_cnt = getnetcnt((Queue *) $2);
			        if(inst_xs)
			        {
				    inst_cnt = getxslength(inst_xs);
				    net_cnt = net_cnt / inst_cnt;
			        }

				pnet =
			       (Netelem *) inst -> ntw -> termq -> first_elem();

				pxs = pnet -> xs;
				pxelem = (Xelem *) *pxs -> base();
				pxelem -> right_bound = net_cnt - 2;

			        sprintf(attr_buf, "n=%d", net_cnt - 1);
			        attr_s -> push (strsav(attr_buf));
				bifflag = 0;

				if (net_cnt < 2)
				    sls_error (yylineno, ATERM_LT_FTERM, 0);
				else if (net_cnt > 2 && invertbifflag)
				    sls_error (yylineno, ATERM_GT_FTERM, 0);
			}

                        /* set n_*_cnt for possibly adding extra global nets
			   in match_net() */

                        if (((Link *) $1) -> type == NetworkType) {
			    inst_xs = inst -> inst_struct -> inst_construct;
			    if(inst_xs)
				n_inst_cnt = getxslength (inst_xs);
			    else
				n_inst_cnt = 1;
			    n_net_cnt = getnetcnt((Queue *) $2);
			    n_term_cnt = gettermcnt (inst -> ntw);
			    if (n_net_cnt % n_inst_cnt != 0) n_inst_cnt = 0;
		        }
			else
			    n_inst_cnt = 0;

			inst -> ntw_attr = stackcpy(attr_s);
			if(inst_net_eqv(inst, (Queue *) $2))
			{
			    sls_error(yylineno, sls_errno, NULL);
			}
			n_inst_cnt = 0;
		    }

		    almost_eos = 1;
		}
		;

inst_def	: named_instance
		{
			$$ = $1;
		}
		| unnamed_instance
		{
			$$ = $1;
		}
		;

named_instance	: LCB inst_struct RCB instance
		{
			if ($2 && $4) {
			    inst = (NetworkInstance *) $4;

			    inst->inst_struct = (InstanceStruct *) $2;
			    inst_dict->store(inst->inst_struct->inst_name,
					     (char *) inst);
			    $$ = $4;
			}
			else
			    $$ = 0;
		}
		;

unnamed_instance: instance
		{
			inst = (NetworkInstance *) $1;

			inst->inst_struct = new InstanceStruct
				((char *) ntw->genname(InstanceType), NULL);
			inst_dict->store(inst->inst_struct->inst_name,
					 (char *) inst);
			$$ = $1;
		}
		;

instance	: tor_instance
		{
			((Link *) $1) -> type = TransistorType;
			$$ = $1;
		}
		| res_instance
		{
			((Link *) $1) -> type = ResistorType;
			$$ = $1;
		}
		| cap_instance
		{
			((Link *) $1) -> type = CapacitorType;
			$$ = $1;
		}
		| ntw_instance
		{
		        mc_type = 'c';            /* model call is a circuit */
			((Link *) $1) -> type = NetworkType;
			$$ = $1;
		}
		| fun_instance
		{
			((Link *) $1) -> type = FunctionType;
			$$ = $1;
		}
		;

tor_instance	: ttype attr_list
		{
			$$ = (char*) new NetworkInstance((Network *)$1, NULL);
		}
		;

res_instance	: RES real
		{
			attr_s -> reset();
			sprintf(attr_buf,"v=%g", $2);
 			attr_s -> push(strsav(attr_buf));

			$$ = (char*) new NetworkInstance(dev_tab[DevRes], NULL);
		}
		;

cap_instance    : CAP real
		{
			attr_s -> reset();
			sprintf(attr_buf,"v=%g", $2);
 			attr_s -> push(strsav(attr_buf));

			$$ = (char*) new NetworkInstance(dev_tab[DevCap], NULL);
		}
		;

ntw_instance    : ntw_name attr_list
		{
		    if(fflag)
		    {
			if(!(pntw = is_bifunc($1)))
			{
		            if ((pntw = (Network *) dff_dict -> fetch($1))
			         == NULL)
			    {
		                if ((pntw = read_dff ($1)) == NULL)
			        {
			            sls_errno = UNKFUNC;
			            sls_error(yylineno, sls_errno, $1);
			            die();
			        }
		            }
			}
			else {
		            bifflag = 1;

			    if ($1[0] == 'i'
				&& strcmp ($1, "invert") == 0)
				invertbifflag = 1;
			    else
				invertbifflag = 0;
			}
		    }
		    else
		    {
		        if ((pntw = (Network *) ntw_dict -> fetch($1)) == NULL)
			{
			    if (externRequired) {
			        sls_errno = NOEXTERNDEF;
			        sls_error(yylineno, sls_errno, $1);
			        die();
			    }

		            if ((pntw = read_ntw ($1)) == NULL)
			    {
			        sls_errno = UNKNETWORK;
			        sls_error(yylineno, sls_errno, $1);
			        die();
			    }
			}
		    }

		    $$ = (char*) new NetworkInstance(pntw, NULL);
		}
		;

inst_struct	: i_name sb_index_list
		{
			if(sym_dict->fetch($1))
			{
			    sls_errno = INSTADEFASNODE;
			    sls_error(yylineno, sls_errno, $1);
			    $$ = 0;
			}
			else if(inst_dict->fetch($1))
			{
			    sls_errno = INSTADEF;
			    sls_error(yylineno, sls_errno, $1);
			    $$ = 0;
			}
			else
			{
			    $$ = (char*) new InstanceStruct (strsav($1),
				           stackcpy((Stack *) $2));
			}
		}
		;

fun_instance	: ATSIGN
		{
			fflag++;
		}
		  ntw_instance
		{
			fflag = 0;
			$$ = $3;
		}
		;

ttype		: NENH
		{
			$$ = (char*) dev_tab[DevNenh];
		}
		| PENH
		{
			$$ = (char*) dev_tab[DevPenh];
		}
		| NDEP
		{
			$$ = (char*) dev_tab[DevNdep];
		}
		;

attr_list	: /* empty */
		{
			attr_s->reset();
			$$ = (char*) attr_s;
		}
		| attr_list attribute
		{
			((Stack *) $1) -> push (strsav ($2));
			$$ = $1;
		}
		;

attribute	: attr_spec ASSIGN attr_val
		{
		    	sprintf(attr_buf, "%s=%g", $1, $3);
			$$ = attr_buf;
		}
		;

attr_spec	: T_WIDTH
		{
			$$ = (char*) "w";
		}
		| T_LENGTH
		{
			$$ = (char*) "l";
		}
		| T_RISE
		{
			$$ = (char*) "tr";
		}
		| T_FALL
		{
			$$ = (char*) "tf";
		}
		;

attr_val	: real
		{
			$$ = $1;
		}
		;

real		: i_val
		{
			$$ = (double) $1;
		}
		| f_val
		{
			$$ = $1;
		}
		;

f_val		: FLOAT
		{
			$$ = cvt_atof($1);
		}
		;

type_con_list	: LCB connect_list RCB /* argument major connect list */
		{
			((Queue *) $2)->type = PmQueue;
			$$ = $2;
		}
		| LPS connect_list RPS /* instance major connect list */
		{
			((Queue *) $2)->type = ImQueue;
			$$ = $2;
		}
		;

connect_list	: connect
		{
		    pnetq = new Queue (QueueType);

		    if($1)
		    {
			pnetq->put((Link *) $1);
		    }

		    $$ = (char*) pnetq;
		}
		| connect_list COMMA connect
		{
		    if($3)
		    {
			((Queue *) $1)->put((Link *) $3);
		    }

		    $$ = $1;
		}
		;

connect		: internal_ref
		{
			$$ = $1;
		}
		| net_ref
		{
			$$ = $1;
		}
		| /* empty */
		{
			$$ = (char*) new NetReference (notconnected, NULL);
		}
		;

internal_ref	: sb_index_list
		{
		    pxs = stackcpy((Stack *) $1);
		}
		  PERIOD inst_net_ref
		{
		    int inst_xs_len = 0;
		    int net_xs_len = 0;
		    int net_len = 0;

		    pnref = (NetReference *) $4;

		    if(pnref)
		    {
			pnref -> inst = inst;
			if(chk_bounds(pnref->inst->inst_struct->inst_construct, pxs))
			{
			    sls_errno = ILLRANGE;
			    sls_error(yylineno, sls_errno, (char*)
			    	      "range not within declared bounds");
			    $$ = 0;
			}
			else
			{
			    pnref -> inst_xs = pxs;

		            inst_xs_len = getxslength(pxs);
		            net_xs_len = getxslength(pnref->net_xs);
			    net_len = (inst_xs_len ? inst_xs_len : 1) *
			    	      (net_xs_len ? net_xs_len : 1);
			    if (net_len) {
				Xelem *pxelem = new Xelem (0, net_len - 1);
				xs -> reset ();
			        xs -> push ((char *) pxelem);
				pxs = stackcpy (xs);
				pnref -> ref_xs = stackcpy (xs);
			    }
			    else
				pnref -> ref_xs = pxs = 0;

			    pnet = new Netelem ((char *) ntw->genname(NetType),
					        pxs, NetType);

			    sym_dict -> store(pnet -> name, (char *) pnet);
			    ntw -> netq -> put ((Link *) pnet);

			    $$ = (char*) new NetReference (pnet,
							 stackcpy(pnet->xs));
                            if (doSimpleNet) {
				joinSimple (pnet, pnref -> ref_xs,
					    pnref -> inst, pnref -> inst_xs,
					    pnref -> net, pnref -> net_xs);
				delete pnref;
			    }
			    else {
				if(!pnet->eqv)
				    pnet->eqv = new Queue (QueueType);

				pnet -> eqv -> put ((Link *) pnref);
			    }
			}
		    }
		    else
			$$ = 0;
		}
		;

inst_net_ref	: net_name sb_index_list
		{
		    Netelem * pterm;
		    Stack *pxs1;

		    pxs1 = stackcpy((Stack *) $2);

		    if ((pterm = findterm(inst, $1))) {
			if (chk_bounds(pterm->xs, pxs1)) {
			    sls_errno = ILLRANGE;
			    sls_error (yylineno, sls_errno, (char*)"range not within declared bounds");
			    $$ = 0;
			}
			else
			    $$ = (char*) new NetReference (pterm, pxs1);
		    }
		    else {
			sls_errno = UNKTERM;
			sls_error (yylineno, sls_errno, $1);
			$$ = 0;
		    }
		}
		;

net_stmt	: NET LCB type_net_list RCB
		{
		    if($3)
		    {
			if(neteqv((Queue *) $3))
			{
			    sls_error(yylineno, sls_errno, NULL);
			}
	 	    }

		    almost_eos = 1;
		}
		;

type_net_list	: net_grp_list
		{
		    $$ = $1;
		}
		| net_list
		{
		    $$ = $1;
		}
		;

net_grp_list	: net_grp
		{
		    if($1)
		    {
			pnetq = new Queue (QueueType);
			pnetq->put((Link *) $1);
			$$ = (char*) pnetq;
		    }
		    else
			$$ = $1;
		}
		| net_grp_list COMMA net_grp
		{
		    if($1 && $3)
		    {
			((Queue *) $1)->put((Link *) $3);
			$$ = $1;
		    }
		    else
			$$ = 0;
		}
		;

net_grp		: LPS net_list RPS
		{
			$$ = $2;
		}
		;

net_list	: net_ref
		{
		    if($1)
		    {
			pnetq = new Queue (QueueType);
			pnetq->put((Link *) $1);
			$$ = (char*) pnetq;
		    }
		    else
			$$ = $1;
		}
		| net_list COMMA net_ref
		{
		    if($1 && $3)
		    {
			    ((Queue *) $1)->put((Link *) $3);
			    $$ = $1;
		    }
		    else
			$$ = 0;
		}
		;

net_ref		: net_name sb_index_list
		{
		    /* If net not defined: Define the net */
			pxs = stackcpy((Stack *) $2);

			if(inst_dict->fetch($1))
			{
			    sls_errno = NODEADEFASINST;
			    sls_error(yylineno, sls_errno, $1);
			    $$ = 0;
			}
			else if ((pnet = (Netelem *)(sym_dict->fetch($1)))
								     == NULL)
			{
			        pnet = new Netelem ($1, pxs, NetType);
			        sym_dict->store(pnet->name, (char *) pnet);
				ntw -> netq -> put ((Link *) pnet);
				$$ = (char*) new NetReference (pnet, pxs);
			}
			else
			{
			    if(chk_bounds(pnet->xs, pxs))
			    {
			        sls_errno = ILLRANGE;
			        sls_error(yylineno, sls_errno, (char*)
				          "range not within declared bounds");
			        $$ = 0;
			    }
			    else
				$$ = (char*) new NetReference (pnet, pxs);
			}
		}
		| INTEGER
		{
			/* If net yet not defined, a new net will be defined
			   and stored in the symbol table and put in the
			   network net queue. Additionally a net reference
			   is defined and returned by this action	    */

			if((pnet = (Netelem *)(sym_dict->fetch($1))) == NULL)
			{
			    pnet = new Netelem ($1, NULL, NetType);
			    sym_dict->store(pnet->name, (char *) pnet);
			    ntw -> netq -> put ((Link *) pnet);
			}
			$$ = (char*) new NetReference (pnet,NULL);
		}
		;

sb_index_list	: /* empty */
		{
			$$ = 0;
		}
		| LSB index_list RSB
		{
			$$ = (char*) xs;
		}
		;

index_list	: index
		{
			xs->reset();
			if(xs->push((char *)$1) == STACK_OVERFLOW) {
			    fprintf(stderr,"xstack overflow\n");
		            die ();
		        }

		}
		| index_list COMMA index
		{
			if(xs->push((char *)$3) == STACK_OVERFLOW) {
			    fprintf(stderr,"xstack overflow\n");
		            die ();
		        }
		}
		;

index		: i_val
		{
			$$ = (char*) new Xelem ($1, $1);
		}
		| i_val DOTDOT i_val
		{
			$$ = (char*) new Xelem ($1, $3);
		}
		;

i_val		: INTEGER
		{
			$$ = atoi($1);
		}
		;

ntw_name	: ident
		{
			strcpy(ntw_name, $1);
			$$ = ntw_name;
		}
		| keyw1
		{
			strcpy(ntw_name, $1);
			$$ = ntw_name;
		}
		;

i_name		: ident
		| keyw1
		| keyw2
		| PERIOD
		{
			$$ = ntw->genname(InstanceType);
		}
		;

net_name	: ident
		{
			strcpy(ntw_name, $1);
			$$ = ntw_name;
		}
		| keyw1
		{
			strcpy(ntw_name, $1);
			$$ = ntw_name;
		}
		| keyw2
		{
			strcpy(ntw_name, $1);
			$$ = ntw_name;
		}
		;

ident		: IDENTIFIER
		{
			strcpy (name_buf, $1);

			if (dmTestname (name_buf) == 1) {
			    if (!trunc_warn && !noWarnings) {
			        fprintf (stderr,
			        "Warning: name(s) truncated to %d characters\n",
			        (int)strlen (name_buf));
			        trunc_warn = 1;
			    }
			}
			$$ = name_buf;
		}
		;

keyw1		: keyword1
		{
			strcpy (name_buf, textval());

			if (dmTestname (name_buf) == 1) {
			    if (!trunc_warn && !noWarnings) {
			        fprintf (stderr,
			        "Warning: name(s) truncated to %d characters\n",
			        (int)strlen (name_buf));
			        trunc_warn = 1;
			    }
			}
			$$ = name_buf;
		}
		;

keyw2		: keyword2
		{
			strcpy (name_buf, textval());

			if (dmTestname (name_buf) == 1) {
			    if (!trunc_warn && !noWarnings) {
			        fprintf (stderr,
			        "Warning: name(s) truncated to %d characters\n",
			        (int)strlen (name_buf));
			        trunc_warn = 1;
			    }
			}
			$$ = name_buf;
		}
		;

keyword1	: NETWORK
		| EXTERNSPEC
		| TERMINAL
		| RES
		| CAP
		| T_WIDTH
		| T_LENGTH
		| T_RISE
		| T_FALL
		;

keyword2	: NENH
		| PENH
		| NDEP
		| NET
		;
%%

void yyerror (const char *s)
{
    sls_error (yylineno, SYNTAX, NULL);
}
