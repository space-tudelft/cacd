%{
/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
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

extern Network *dev_tab[];

extern Stack 	*attr_s;

static int m_attr = 0;
int n_net_cnt;
int n_inst_cnt = 0;

Queue	*pnetq;
extern Dictionary *ntw_dict;
extern Dictionary *sym_dict;
extern Dictionary *inst_dict;

extern char **globNets;
extern int globNets_cnt;

#ifdef __cplusplus
  extern "C" {
#endif

void yyerror (const char *s);

#ifdef DEBUG
static void pr_inst (InstanceStruct *d1, char *d3);
#endif

#ifdef __cplusplus
  }
#endif

Netelem	*pnet;
Network *ntw;
Network *pntw;
NetworkInstance *inst = NULL;

char name_buf[BUFSIZ];
char net_buf[BUFSIZ];
char ntw_name[BUFSIZ];
char topcell[BUFSIZ];
char attr_buf[BUFSIZ];
char attr_spec_buf[BUFSIZ];

char *strdummy;

int trunc_warn = 0;
int externspec = 0;
int curr_inst_type = 0;
int toppresent = 0;
int behind_header = 0;   /* used for lexical analyzer to detect
			    correct initial condition */

#ifdef ESELVS
int skipCWarning = 0;
int skipRWarning = 0;
#endif

%}

%union {
	long  ival;
	char *sval;
	char cval;
	long *pival;
	float fval;
	double dval;
	}

%token <sval> INTEGER IDENTIFIER FLOAT
%token <sval> TOR_INST JTOR_INST NETW_INST CAP_INST RES_INST
%token <sval> T_WIDTH T_LENGTH T_SCALEF

%token SUBCKT ENDS END OTHER_CARD_BEGIN VNET RNET CNET
%token PERIOD ASSIGN LPS RPS OTHER
%token NEWLINE

%type <sval> ckt_term_list
%type <sval> connect
%type <sval> terminal_list terminal
%type <sval> sub_ckt_head nom_ckt_head inst_stmt
%type <sval> x_inst_stmt x_inst_struct
%type <sval> j_inst_stmt j_inst_struct
%type <sval> t_inst_stmt t_inst_struct
%type <sval> r_inst_stmt r_inst_struct
%type <sval> c_inst_stmt c_inst_struct
%type <sval> o_inst_stmt o_inst_struct
%type <ival> area_attr i_val
%type <dval> f_val attr_val real
%type <sval> attr_spec
%type <sval> i_cell_name net_name ntw_name ident all_ident

%start spice_descr

%%

spice_descr	: file_top file_rest
		;

file_rest	: file_header sub_ckts nom_ckt
		| sub_ckts
		;

file_header	: ntw_name
		{
		    behind_header = 1;
		    toppresent = 1;

		    strcpy (topcell, $1);
#ifdef DEBUG
		    fprintf (stderr, "topcell %s\n", topcell);
#endif
		}
		  stuff eoc
		;

file_top	: /* empty */
		| file_top NEWLINE
		;

sub_ckts	: /* empty */
		| sub_ckts sub_ckt
		;

sub_ckt		: sub_ckt_head ckt_body sub_ckt_end
		{
			if (end_ntw ((Network *)$1, externspec, COMPLETE))
			{
			    sls_error (yylineno-1, sls_errno, NULL);
			    die();
			}
		}
		;

nom_ckt		: nom_ckt_head ckt_body nom_ckt_end
		{
			if (end_ntw ((Network *)$1, externspec, COMPLETE))
			{
			    sls_error (yylineno-1, sls_errno, NULL);
			    die();
			}
		}
		;

sub_ckt_head	: SUBCKT ntw_name
		{
#ifdef DEBUG
			fprintf (stderr, "subckt %s\n", $2);
#endif
			if ((ntw = (Network *)init_ntw($2, externspec))) {
			    $<sval>$ = (char*) ntw;
			}
			else {
			    sls_errno = NTWAREF;
			    sls_error(yylineno, sls_errno, $2);
			    die();
			}
		}
		  ckt_term_list eoc
		{
			int i;

			$$ = (char*)ntw;

			if ($4 && globNets_cnt > 0) {
			    pnetq = NULL;
			    for (i = 0; i < globNets_cnt; i++) {

				if (!(pnet =
				  (Netelem *)(sym_dict->fetch(globNets[i])))) {

                                    /* add net 'globNets[i]' as an extra
				       terminal to the current network */

                                    if (!pnetq) {
					pnetq = new Queue (QueueType);
				    }

				    pnet = new Netelem (globNets[i], (Stack *) 0, TermType);
				    pnet -> type = pnet -> type | N_GLOBADDED;
				    pnetq -> put ((Link *) pnet);
				    pnet = new Netelem (globNets[i], (Stack *) 0, TermType);
				    pnet -> type = pnet -> type | N_GLOBADDED;
				    sym_dict->store (pnet->name, (char *) pnet);
				    ntw -> netq -> put ((Link *) pnet);
				}
				else if (pnet -> xs) {
				    sls_errno = ILLRANGE;
				    sls_error (yylineno-1, sls_errno, globNets[i]);
				    $$ = 0;
				    break;
				}
			    }

			    if (pnetq) {
				if(ntw->termq)
				    ntw->termq->append((Queue *) pnetq);
				else
				    ntw -> termq = (Queue *) pnetq;
			    }
			}

			if ($$) {
			    if ($4)
				term_to_db((Queue *) $4);
			}
		}
		;

nom_ckt_head	: /* empty */
		{
#ifdef DEBUG
			fprintf (stderr, "ckt %s\n", topcell);
#endif
			if ((ntw = (Network *)init_ntw(topcell, externspec))) {
			    $$ = (char*) ntw;
			}
			else {
			    sls_errno = NTWAREF;
			    sls_error(yylineno, sls_errno, topcell);
			    die();
			}
		}
		;

ckt_body	: stmt_list
		;

sub_ckt_end	: ENDS stuff eoc
		;

nom_ckt_end	: END stuff eoc
		{
		    toppresent = 0;
		    behind_header = 0;
		}
		;

ckt_term_list	: terminal_list
		{
			if($1)
			{
			    if(ntw->termq)
				ntw->termq->append((Queue *) $1);
			    else
			    	ntw -> termq = (Queue *) $1;
			    $$ = (char*) $1;
			}
			else {
			    $$ = 0;
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
		| terminal_list terminal
		{
			if ($1 && $2)
			    ((Queue *) $1)->put((Link *) $2);
			$$ = $1;
		}
		;

terminal	: net_name
		{
			if(sym_dict->fetch($1))
			{
			    sls_errno = TERMADEF;
			    sls_error(yylineno, sls_errno, $1);
			    $$ = 0;
			}
			else
			{
			    pnet = new Netelem ($1, (Stack *) 0, TermType);
			    $$ = (char*) pnet;
			    pnet = new Netelem ($1, (Stack *) 0, TermType);
			    sym_dict->store(pnet->name, (char *) pnet);
			    ntw -> netq -> put ((Link *) pnet);
			}
		}
		;

stmt_list	: /* empty */
		| stmt_list statement
		;

statement	: inst_stmt eoc
		{
			if(inst && curr_inst_type) {
			    if(ntw->mcq == NULL)
				ntw -> mcq = new Queue (QueueType);

			    ntw -> mcq -> put ((Link *) inst);
			    inst_to_db((NetworkInstance *) inst);
			}
			inst = NULL;
		}
		| net_stmt   eoc
		| other_card eoc
		| error
		{
		    	if (resynch ('\n')) {
		    	    yyerrok;
		    	    yyclearin;
			}
			else
			    die();
		}
		;

net_stmt	: VNET two_con_list real
		{
		    if ($3 < 1.0e-9 && $3 > -1.0e-9) {
			if (neteqv (pnetq))
			    sls_error(yylineno, sls_errno, NULL);
		    }
		}
		| RNET two_con_list real
		{
		    /* do nothing (it may be a grounded rnet !) */
		}
		| CNET two_con_list
		{
		    if (neteqv (pnetq))
			sls_error(yylineno, sls_errno, NULL);
		}
		;

inst_stmt	: x_inst_stmt
		| j_inst_stmt
		| t_inst_stmt
		| r_inst_stmt
		| c_inst_stmt
		| o_inst_stmt
		;

other_card	: OTHER_CARD_BEGIN stuff
		;

x_inst_stmt	: x_inst_struct connect_list ident
		{
#ifdef DEBUG
		    pr_inst ((InstanceStruct *) $1, $3);
#endif
		    if ($1) {
			if ($3 == NULL) yyerror (NULL);
			if ((pntw = (Network *) ntw_dict -> fetch($3)) == NULL)
			{
			    if ((pntw = read_ntw ($3)) == NULL)
			    {
				sls_errno = UNKNETWORK;
				sls_error(yylineno-1, sls_errno, $3);
				die();
			    }
			}

			$$ = (char*) new NetworkInstance(pntw, NULL);

			((Link *)$$) -> type = curr_inst_type;

			inst = (NetworkInstance *) $$;

			inst->inst_struct = (InstanceStruct *) $1;
			inst_dict->store(inst->inst_struct->inst_name, (char*) inst);

                        /* set n_*_cnt for possibly adding extra global nets
			   in match_net() */

			n_inst_cnt = 1;
			n_net_cnt = getnetcnt (pnetq);

			inst -> ntw_attr = NULL;
			if (inst_net_eqv (inst, pnetq))
			    sls_error(yylineno-1, sls_errno, NULL);
			n_inst_cnt = 0;
		    }
		}
		;

j_inst_stmt	: j_inst_struct three_con_list ident area_attr
		{
#ifdef DEBUG
		    pr_inst ((InstanceStruct *) $1, $3);
#endif
		    if ($1) {
			if ($3 == NULL) yyerror (NULL);
			if ((pntw = (Network *) ntw_dict -> fetch($3)) == NULL)
			{
			    if ((pntw = read_ntw ($3)) == NULL)
			    {
				sls_errno = UNKNETWORK;
				sls_error(yylineno-1, sls_errno, $3);
				die();
			    }
			}

			$$ = (char*) new NetworkInstance(pntw, NULL);

			((Link *)$$) -> type = curr_inst_type;

			inst = (NetworkInstance *) $$;

			inst->inst_struct = (InstanceStruct *) $1;
			inst_dict->store(inst->inst_struct->inst_name, (char*) inst);

			if ($4)
			    inst -> ntw_attr = stackcpy (attr_s);
			else
			    inst -> ntw_attr = NULL;
			if (inst_net_eqv (inst, pnetq))
			    sls_error(yylineno-1, sls_errno, NULL);
		    }
		}
		;

t_inst_stmt	: t_inst_struct four_con_list ident attr_list
		{
		    int dev;
#ifdef DEBUG
		    pr_inst ((InstanceStruct *) $1, $3);
#endif
		    if ($1) {
			if ($3 == NULL) yyerror (NULL);
			if ((pntw = (Network *) ntw_dict -> fetch($3)) == NULL) {
			    if ((pntw = read_ntw ($3)) == NULL) {

				/* If the transistor is not defined in the
				   database, it must be a predefined transistor.
				*/
				if (strcmp ($3, "nenh") == 0)
				    dev = DevNenh;
				else if (strcmp ($3, "penh") == 0)
				    dev = DevPenh;
				else if (strcmp ($3, "ndep") == 0)
				    dev = DevNdep;
				else {
				    dev = DevNenh;
				    sls_errno = UNKDEVICE;
				    sls_error(yylineno-1, sls_errno, $3);
				    die();
				}
				pntw = dev_tab[dev];
			    }
			}
			$$ = (char*) new NetworkInstance(pntw, NULL);

			((Link *)$$) -> type = curr_inst_type;

			inst = (NetworkInstance *) $$;

			inst->inst_struct = (InstanceStruct *) $1;
			inst_dict->store(inst->inst_struct->inst_name, (char *)inst);

			inst -> ntw_attr = stackcpy(attr_s);
			if (inst_net_eqv (inst, pnetq))
			    sls_error(yylineno-1, sls_errno, NULL);
		    }
		}
		;

r_inst_stmt	: r_inst_struct two_con_list i_cell_name single_attr
		{
#ifdef DEBUG
		    pr_inst ((InstanceStruct *) $1, $3);
#endif
		    if ($1) {
			$$ = (char*) new NetworkInstance(dev_tab[DevRes], NULL);

			((Link *)$$) -> type = curr_inst_type;

			inst = (NetworkInstance *) $$;

			inst->inst_struct = (InstanceStruct *) $1;
			inst_dict->store(inst->inst_struct->inst_name, (char*) inst);

			inst -> ntw_attr = stackcpy(attr_s);
			if (inst_net_eqv (inst, pnetq))
			    sls_error(yylineno-1, sls_errno, NULL);
		    }
		}
		;

c_inst_stmt	: c_inst_struct two_con_list i_cell_name single_attr
		{
#ifdef DEBUG
		    pr_inst ((InstanceStruct *) $1, $3);
#endif
		    if ($1) {
			$$ = (char*) new NetworkInstance(dev_tab[DevCap], NULL);

			((Link *)$$) -> type = curr_inst_type;

			inst = (NetworkInstance *) $$;

			inst->inst_struct = (InstanceStruct *) $1;
			inst_dict->store(inst->inst_struct->inst_name, (char*) inst);

			inst -> ntw_attr = stackcpy(attr_s);
			if (inst_net_eqv (inst, pnetq))
			    sls_error(yylineno-1, sls_errno, NULL);
		    }
		}
		;

o_inst_stmt	: o_inst_struct stuff
		;

x_inst_struct	: NETW_INST
                {
			curr_inst_type = NetworkType;

                        if (sym_dict -> fetch ($1)) {
                            sls_errno = INSTADEFASNODE;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else if (inst_dict -> fetch ($1)) {
                            sls_errno = INSTADEF;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else {
                            $$ = (char*) new InstanceStruct (strsav ($1), (Stack *) 0);
                        }
                }
                ;

j_inst_struct	: JTOR_INST
                {
			curr_inst_type = NetworkType;

                        if (sym_dict -> fetch ($1)) {
                            sls_errno = INSTADEFASNODE;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else if (inst_dict -> fetch ($1)) {
                            sls_errno = INSTADEF;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else {
                            $$ = (char*) new InstanceStruct (strsav ($1), (Stack *) 0);
                        }
                }
                ;

t_inst_struct	: TOR_INST
                {
			curr_inst_type = TransistorType;

                        if (sym_dict -> fetch ($1)) {
                            sls_errno = INSTADEFASNODE;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else if (inst_dict -> fetch ($1)) {
                            sls_errno = INSTADEF;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else {
                            $$ = (char*) new InstanceStruct (strsav ($1), (Stack *) 0);
                        }
                }
                ;

r_inst_struct	: RES_INST
                {
#ifndef ESELVS
			curr_inst_type = ResistorType;

                        if (sym_dict -> fetch ($1)) {
                            sls_errno = INSTADEFASNODE;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else if (inst_dict -> fetch ($1)) {
                            sls_errno = INSTADEF;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else {
                            $$ = (char*) new InstanceStruct (strsav ($1), (Stack *) 0);
                        }
#else
                        if (!skipRWarning) {
			    fprintf (stderr,
			    "Warning: skipped resistance(s) in input file\n");
			    skipRWarning = 1;
			}
			$$ = 0;
#endif
                }
                ;

c_inst_struct	: CAP_INST
                {
#ifndef ESELVS
			curr_inst_type = CapacitorType;

                        if (sym_dict -> fetch ($1)) {
                            sls_errno = INSTADEFASNODE;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else if (inst_dict -> fetch ($1)) {
                            sls_errno = INSTADEF;
                            sls_error (yylineno, sls_errno, $1);
                            $$ = 0;
                        }
                        else {
                            $$ = (char*) new InstanceStruct (strsav ($1), (Stack *) 0);
                        }
#else
                        if (!skipCWarning) {
			    fprintf (stderr,
			    "Warning: skipped capacitance(s) in input file\n");
			    skipCWarning = 1;
			}
			$$ = 0;
#endif
                }
                ;

o_inst_struct	: ident
		{
			curr_inst_type = 0;
			$$ = 0;
		}
		;

attr_list	: /* empty */
		{
			attr_s -> reset ();
		}
		| attr_list i_attribute
		;

area_attr	: /* empty */
		{
			$$ = 0;
		}
		| real
		{
			attr_s -> reset ();
		    	sprintf (attr_buf, "scalef=%g", $1);
			attr_s -> push (strsav (attr_buf));
			$$ = 1;
		}
		| T_SCALEF ASSIGN real
		{
			attr_s -> reset ();
		    	sprintf (attr_buf, "scalef=%g", $3);
			attr_s -> push (strsav (attr_buf));
			$$ = 1;
		}
		;

single_attr	: real
		{
			attr_s -> reset ();
		    	sprintf (attr_buf, "v=%g", $1);
			attr_s -> push (strsav (attr_buf));
		}
		;

i_attribute	: attr_spec ASSIGN attr_val
		{
		    double d = $3;
		    int skip = 0;

		    if (m_attr) {
			int i = (int) d;
			if (d < 1 || d != (double) i) {
			  skip = 1;
			  fprintf (stderr,
			    "line %d: Warning: m=%g skipped in input file!\n",
			    yylineno, d);
			}
			m_attr = 0;
		    }
		    if (!skip) {
		    	sprintf (attr_buf, "%s=%g", $1, d);
			attr_s -> push (strsav (attr_buf));
		    }
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
		| T_SCALEF
		{
			m_attr = 1;
			$$ = (char*) "scalef";
		}
		| IDENTIFIER
		{
			strcpy (attr_spec_buf, $1);
			$$ = attr_spec_buf;
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
			$$ = cvt_atof ($1);
		}
		;

two_con_list	: connect connect
		{
		    pnetq = new Queue (QueueType);
		    pnetq -> type = PmQueue;
		    if ($1) pnetq -> put ((Link *) $1);
		    if ($2) pnetq -> put ((Link *) $2);
		}
		;

three_con_list	: connect connect connect
		{
		    pnetq = new Queue (QueueType);
		    pnetq -> type = PmQueue;
		    if ($1) pnetq -> put ((Link *) $1);
		    if ($2) pnetq -> put ((Link *) $2);
		    if ($3) pnetq -> put ((Link *) $3);
		}
		;

four_con_list	: connect connect connect connect
		{
		    pnetq = new Queue (QueueType);
		    pnetq -> type = PmQueue;
		    if ($1) pnetq -> put ((Link *) $1);
		    if ($2) pnetq -> put ((Link *) $2);
		    if ($3) pnetq -> put ((Link *) $3);
		    if ($4) pnetq -> put ((Link *) $4);
		}
		;

connect_list	: connect
		{
		    pnetq = new Queue (QueueType);
		    pnetq -> type = PmQueue;
		    if ($1) pnetq -> put ((Link *) $1);
		}
		| connect_list connect
		{
		    if ($2) pnetq -> put ((Link *) $2);
		}
		;

connect		: net_name
		{
			/* If net yet not defined, a new net will be defined
			   and stored in the symbol table and put in the
			   network net queue. Additionally a net reference
			   is defined and returned by this action	    */
#ifdef DEBUG
			fprintf (stderr, "net_ref: %s\n", $1);
#endif
                        if(inst_dict->fetch($1)) {
                            sls_errno = NODEADEFASINST;
                            sls_error(yylineno, sls_errno, $1);
                            $$ = 0;
                        }
			else {
			    if (!(pnet = (Netelem *)(sym_dict->fetch($1)))) {
				pnet = new Netelem ($1, NULL, NetType);
				sym_dict->store(pnet->name, (char *) pnet);
				ntw -> netq -> put((Link*) pnet);
			    }
			    $$ = (char*) new NetReference (pnet, NULL);
			}
		}
		;

i_cell_name	: /* empty */
		{
		    $$ = 0;
		}
		| ident
		{
		    $$ = $1;
		}
		;

ntw_name	: ident
		{
			strcpy (ntw_name, $1);
			$$ = ntw_name;
		}
		;

net_name	: i_val
		{
		    	sprintf (net_buf, "%ld", $1);
			$$ = net_buf;
		}
		| ident
		{
			strcpy (net_buf, $1);
			$$ = net_buf;
		}
		;

stuff		: /* empty */
		| stuff stuff_item
		;

stuff_item	: INTEGER    { strdummy = $1; }
		| IDENTIFIER { strdummy = $1; }
		| FLOAT      { strdummy = $1; }
		| SUBCKT
		| ENDS
		| END
		| OTHER_CARD_BEGIN
		| PERIOD
		| ASSIGN
		| LPS
		| RPS
		| T_WIDTH  { strdummy = $1; }
		| T_LENGTH { strdummy = $1; }
		| T_SCALEF { strdummy = $1; }
		| OTHER
		;

ident		: all_ident
		{
			strcpy (name_buf, $1);

			if (dmTestname (name_buf) == 1) {
			    if (!trunc_warn && !noWarnings) {
			        fprintf (stderr, "Warning: name(s) truncated to %d characters\n",
				    (int)strlen (name_buf));
			        trunc_warn = 1;
			    }
			}
			$$ = name_buf;
		}
		;

all_ident	: IDENTIFIER { $$ = $1; }
		| T_WIDTH  { $$ = $1; }
		| T_LENGTH { $$ = $1; }
		| T_SCALEF { $$ = $1; }
                ;

i_val		: INTEGER { $$ = atoi($1); }
		;

eoc		: NEWLINE
		| eoc NEWLINE
		;

%%

void yyerror (const char *s)
{
    sls_error (yylineno, SYNTAX, NULL);
}

#ifdef DEBUG
static void pr_inst (InstanceStruct *d1, char *d3)
{
    fprintf (stderr, "instance %s %s\n",
	d1 ? d1 -> inst_name : "<unknown>", d3 ? d3 : "<unknown>");
}
#endif
