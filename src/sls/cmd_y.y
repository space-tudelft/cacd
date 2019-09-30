%{
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#include "src/sls/extern.h"

extern void dis_addname (PATH_SPEC *path);
extern int findnodes (PATH_SPEC *path, MODELCALLTABLE *mcall, int hashed, NODE_REF_LIST **return_list, int permanent);
extern int *nums_of_signals (PATH_SPEC *path, RES_FILE *rf);
extern void plot_addname (PATH_SPEC *path);
extern RES_FILE *read_paths (FILE *fp, char *fn);
extern void yy0error (char *s);
extern int  yy0lex   (void);
extern int  yy0parse (void);

static void cslserror (int errtype, char *s1, char *s2);
static SIGNALELEMENT *copysgn (SIGNALELEMENT *sgn);
static SIGNALELEMENT *read_signal (FILE *fp, char *fn, long offset, int sig_cnt, int pos);

#undef MAXINT
#undef MAXLONG

#define  MAXSTACKLENGTH  100

char name_space[128];   /* memory space for a name */
int vardummy;

PATH_SPEC  pathspace[MAXHIERAR];  /* memory space for paths */
PATH_SPEC * fullpath;         /* first of the current full path specification */
PATH_SPEC * last_path = NULL;  /* last of the current full path specification */

NODE_REF_LIST * nrl_end;        /* end of the current node ref list */
int nodes_in_res_path = FALSE;  /* when this variable is TRUE it indicates    */
			        /* that nodes are being parsed which will be  */
				/* printed and which therefore will be in the */
				/* res path */
int nodes_in_plot_path = FALSE; /* when this variable is TRUE it indicates    */
			        /* that nodes are being parsed which will be  */
				/* plotted                                    */
int nodes_in_dis_path = FALSE;  /* when this variable is TRUE it indicates    */
			        /* that nodes are being parsed whose          */
				/* dissipation will be plotted                */

PATH_SPEC * readp_begin = NULL; /* begin of path specs of list of nodes which */
                                /* has to be read from res file               */
PATH_SPEC * readp_end = NULL;
int read_res_refs = FALSE;   /* this variable is true when nodes are being    */
                             /* parsed which has to be read from a res file */
int sigcon;                /* boolean flag */
int exclam_flag = FALSE;  /* an exclamation sign has been read   */
int filltype;
NODE_REF_LIST * freadnref;

ABSTRACT_VALUE *curr_av_el;
int av_cnt;

int try_sta_file = 0;   /* If set, try to read .sta file for extra commands */

SIGNALELEMENT * sgn;
short lastval;
simtime_t * len_sp;
simtime_t len_stack[MAXSTACKLENGTH];
SIGNALELEMENT ** sgn_sp;
SIGNALELEMENT * sgn_stack[MAXSTACKLENGTH];

#ifdef YYBISON
extern char *yy0text;  /* exported from LEX output */
#endif

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
		    struct node_ref_list *nrl;
		}

%token 		SET TILDE FROM FILL WITH PRINT PLOT_TOKEN OPTION SIMPERIOD DUMP
%token 		AT DISSIPATION INITIALIZE SIGOFFSET RACES DEVICES STATISTICS
%token 		ONLY CHANGES SLS_PROCESS SIGUNIT OUTUNIT OUTACC MAXPAGEWIDTH
%token 		MAXNVICIN MAXTVICIN MAXLDEPTH VH VMAXL VMINH
%token		TDEVMIN TDEVMAX STEP DISPERIOD RANDOM FULL DEFINE_TOKEN
%token		STA_FILE TST_FILE DOT DOTDOT LPS RPS LSB RSB LCB RCB EQL MINUS
%token 		DOLLAR COMMA SEMICOLON COLON MULT EXCLAM NEWLINE ILLCHAR
%token <ival>	LEVEL LOGIC_LEVEL TOGGLE
%token <sval>	IDENTIFIER INT STRING
%token <dval>	POWER_TEN F_FLO

%type  <lval>	duration
%type  <ival>	integer def_sig_val escape_char
%type  <pival>	int_option toggle_option
%type  <dval>	pow_ten f_float
%type  <pdval>	pow_ten_option f_float_option
%type  <sval>	member_name
%type  <signal>	value signal_exp value_exp
%type  <nrl>	node_refs ref_item

%%

sim_cmd_list	: sim_cmd
		| sim_cmd_list eoc sim_cmd
		| error eoc sim_cmd
		{
		    yyerrok;
		    last_path = NULL;
		    nodes_in_res_path = FALSE;
		    readp_begin = NULL;
		    readp_end = NULL;
		    read_res_refs = FALSE;
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
		    NODE_REF_LIST *p;
		    FORCEDSIGNAL *fsgn;
		    int first;

		    PALLOC (sgn,1,SIGNALELEMENT);
		    sgn -> sibling = *--sgn_sp;
		    sgn -> val = lastval;
		    sgn -> len = *len_sp;
		    first = TRUE;
		    p = $2;
		    while (p) {
			if (p -> nx >= 0) {

			    if (first)
				first = FALSE;
			    else
				sgn = copysgn (sgn);

			    if (N[p -> nx].forcedinfo) {
				cslserror (ERROR2, "already signal attached to node", hiername (p -> nx));
			    }
			    else {
		                PALLOC (fsgn,1,FORCEDSIGNAL);
		                fsgn -> insignal = sgn;
				fsgn -> sigmult = -1;
				fsgn -> fox = -1;
		                N[p -> nx].forcedinfo = fsgn;
		                N[p -> nx].inp = TRUE;
			    }
			}
		        p = p -> next;
		    }

    		    len_sp = len_stack;  /* reset stacks for signal_exp */
    		    sgn_sp = sgn_stack;
		}
		| SET node_refs COLON
		{
		    read_res_refs = TRUE;
		    readp_begin = NULL;
		}
		  node_refs FROM STRING
		{
		    NODE_REF_LIST *p;
		    int n, pos;
		    int * nums;
		    int sig_cnt;
		    long offset;
		    char fn[256];
		    FILE * fp;
		    SIGNALELEMENT * sgnel;
		    FORCEDSIGNAL * fsgn;
		    PATH_SPEC * readp;
		    RES_FILE * rf;

		    sprintf (fn, "%s.res", $7);
		    OPENR (fp, fn);
		    rf = read_paths (fp, fn);
		    sig_cnt = rf -> sig_cnt;
		    offset  = rf -> offset;

		    p = $2;
		    readp = readp_begin;
		    while (readp != NULL) {

		        nums = nums_of_signals (readp, rf);

			for (n = 1; n <= *nums; n++, p = p -> next) {

		            if (p == NULL) {
		                cslserror (ERROR2, "number of left nodes smaller than", "number of right nodes");
                                readp = NULL;
                                break;
                            }

			    if (p -> nx < 0) continue;

			    if ((pos = nums[n]) == 0) {
				cslserror (ERROR2, "cannot find node in res file", NULL);
                                continue;
                            }
			    else if (pos < 0) {
				cslserror (ERROR2, "wrong indices for node in res file", NULL);
                                continue;
                            }

		            sgnel = read_signal (fp, fn, offset, sig_cnt, pos);

			    if (N[p -> nx].funcoutp) {
			        cslserror (ERROR2, "signal is set to function output", hiername (p -> nx));
                                continue;
			    }

			    if (N[p -> nx].forcedinfo) {
			        cslserror (ERROR2, "already signal attached to node", hiername (p -> nx));
                                continue;
			    }

		            PALLOC (fsgn,1,FORCEDSIGNAL);
		            fsgn -> insignal = sgnel;
			    fsgn -> fox = -1;
			    rewind (fp);
			    fscanf (fp, "%e", &(fsgn -> sigmult));
		            N[p -> nx].forcedinfo = fsgn;
		            N[p -> nx].inp = TRUE;
			}
			if (readp == NULL) break;

			readp = readp -> also;
		    }

		    if (p != NULL) {
			cslserror (ERROR2, "number of left nodes is larger than", "number of right nodes");
		    }

                    CLOSE (fp);
		    read_res_refs = FALSE;
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
		    if (sigcon && $3 < 0)
			sigendless = TRUE;
		}
		;

value		: LOGIC_LEVEL
		{
		    PALLOC ($$, 1, SIGNALELEMENT);
		    lastval = $$ -> val = $1;
		    sigcon = FALSE;
		}
		| LPS signal_exp RPS
		{
		    PALLOC ($$, 1, SIGNALELEMENT);
		    PALLOC ($$ -> child, 1, SIGNALELEMENT);
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

print_cmd	: PRINT
		{
		    nodes_in_res_path = TRUE;
		}
		  node_refs
		{
		    int i;
		    ABSTRACT_OUTPUT *ab_el;
		    NODE_REF_LIST * ref_el;

		    if (pl_begin == NULL)
			pl_end = pl_begin = $3;
		    else
			pl_end = pl_end -> next = $3;

		    while (pl_end) {
			if (pl_end -> nx >= 0) {
			    pnodes_cnt++;
			    if (pl_end -> nx < N_cnt) {
				N[ pl_end -> nx ].outp = TRUE;
			    }
			    else {
				ab_el = abstractl_begin;
				i = pl_end -> nx - N_cnt;
				while (i > 0) {
				    ab_el = ab_el -> next;
				    i--;
				}
				ref_el = ab_el -> inputs;
				while (ref_el) {
				    N[ ref_el -> nx ].outp = TRUE;
				    ref_el = ref_el -> next;
				}
			    }
			}
			/* else it was a comma */
			if (pl_end -> next == NULL) break;
			pl_end = pl_end -> next;
		    }

		    nodes_in_res_path = FALSE;
		}
		;

plot_cmd	: PLOT_TOKEN
		{
		    nodes_in_plot_path = TRUE;
		}
		  node_refs
		{
		    if (plotl_begin == NULL)
			plotl_end = plotl_begin = $3;
		    else
			plotl_end -> next = $3;

		    if (plotl_end)
			while (plotl_end -> next)
			    plotl_end = plotl_end -> next;

		    nodes_in_plot_path = FALSE;
		}
		;

dissip_cmd	: DISSIPATION
		{
                    dissip = TRUE;
		    nodes_in_dis_path = TRUE;
		}
		  dis_node_refs
		{
		    nodes_in_dis_path = FALSE;
		}
		;

dis_node_refs	: node_refs
		{
		    if (disl_begin == NULL) {
			disl_end = disl_begin = $1;
			if (disl_end && disl_end -> nx >= 0)
				N[disl_end -> nx].dissip = TRUE;
		    }
		    else
			disl_end -> next = $1;

		    if (disl_end)
			while (disl_end -> next) {
			    disl_end = disl_end -> next;
			    if (disl_end -> nx >= 0)
				N[disl_end -> nx].dissip = TRUE;
			}
		}
		| /* empty */
		;

node_refs	: ref_item
		{
		    if ( ! read_res_refs ) {
			if ((nrl_end = $1))
			    while (nrl_end -> next)
				nrl_end = nrl_end -> next;
			$$ = $1;
		    }
		}
		| node_refs ref_item
		{
		    if ( ! read_res_refs ) {
			if (nrl_end)
			    nrl_end -> next = $2;
			else
			    nrl_end = $2;
			if (nrl_end)
			    while (nrl_end -> next)
				nrl_end = nrl_end -> next;
			$$ = $1 ? $1 : $2;
		    }
		}
		;

ref_item	: full_node_ref
		{
		    RES_PATH * rp;
		    int num;
		    PATH_SPEC * path;
		    PATH_SPEC * nodepath;
		    NODE_REF_LIST * ref_el;
		    ABSTRACT_OUTPUT * ab_el;
		    int n;
		    int cnt;
                    char nodename[DM_MAXNAME + 5];
		    char namebuf[DM_MAXNAME + 1];
		    int i;
		    int k;
		    int ok;

                    if ( ! read_res_refs ) {
                        nodepath = fullpath;
		        while (nodepath -> next != NULL)
		            nodepath = nodepath -> next;
			    /* to find the node name */
                        if (nodepath == fullpath)
                            sprintf (nodename, "%s", nodepath -> name);
                        else
                            sprintf (nodename, "-.%s", nodepath -> name);

			ab_el = NULL;
			ok = FALSE;

			if (nodepath == fullpath) {
			    /* Look if the name is a
			       defined abstract output */
			    ab_el = abstractl_begin;
			    cnt = 0;
			    while (ab_el) {
				if (strcmp (ab_el -> name, nodename) == 0)
				    break;
                                cnt++;
				ab_el = ab_el -> next;
			    }
			    if (ab_el) {
				/* It is an abtract output */
				if (nodes_in_res_path) {
				    PALLOC ($$, 1, NODE_REF_LIST);
				    $$ -> nx = N_cnt + cnt;
				    $$ -> xptr = NULL;
				    $$ -> next = NULL;
				    ok = TRUE;
				}
				else {
				    cslserror (ERROR2, "inappropriate reference to variable", nodename);
				    $$ = NULL;
				}
			    }
			}

                        if (ab_el == NULL) {
			    switch (findnodes (fullpath, NULL, FALSE, &$$, TRUE)) {
			    case NAMENEG :
	                        cslserror (ERROR2, "undefined node", nodename);
			        break;
			    case NOPATH :
	                        cslserror (ERROR2, "internal error", NULL);
			        break;
			    case NONODE :
	                        cslserror (ERROR2, "not a node is name", nodename);
			        break;
			    case REFIERR :
	                        cslserror (ERROR2, "incorrect indices for node", nodename);
			        break;
			    case REFINEG :
	                        cslserror (ERROR2, "not defined as array is node", nodename);
			        break;
		            case REFIMIS :
	                        cslserror (ERROR2, "indices missing for node", nodename);
			        break;
			    case NODETYPE :
		                ok = TRUE;
			        break;  /* the node specification was correct */
		            default :
	                        cslserror (ERROR2, nodename, "is no node");
		                break;
			    }
		        }

		        if (nodes_in_res_path && ok) {

		            if (exclam_flag) {
		                for (k = 0;
		                     (namebuf[k+1] = fullpath ->name[k]) != '\0'
		                     && k < DM_MAXNAME;
		                     k++) {
		                }
		                namebuf[0] = '!';
		                namebuf[k+1] = '\0';
		                strcpy (fullpath -> name, namebuf);
		                ref_el = $$;
		                while (ref_el != NULL) {
		                    n = arr_new ((char **)(void *)&prinvert);
		                    prinvert[ n ] = ref_el;
		                    ref_el = ref_el -> next;
		                }
		            }

			    PALLOC (rp, 1, RES_PATH);
			    if (rp_begin == NULL) {
			        rp_begin = rp_end = rp;
			    }
			    else {
			        rp_end -> next = rp;
			        rp_end = rp_end -> next;
			    }
			    rp_end -> path = fullpath;
			    rp_end -> next = NULL;
			    num = 1;
			    for (path = fullpath; path != NULL;
			         path = path -> next) {
			        for (i = 1; i < path -> xarray[0][0]; i++) {
				    num = num * (path -> xarray[i][1]
					         - path -> xarray[i][0]);
			        }
			    }
			    rp_end -> totnum = num;
		        }

		        if (nodes_in_plot_path && ok) {
                            for (ref_el = $$;
		                 ref_el != NULL;
                                 ref_el = ref_el ->next) {
			        if (ref_el -> nx >= 0) {
		                    if (plotnodes_cnt < MAXPLOT) {
					N[ ref_el -> nx ].plot++;
					++plotnodes_cnt;
		                    }
		                    else {
	                                cslserror (ERROR2, "too many nodes to be plotted", NULL);
		                        ok = FALSE;
		                    }
			        }
			        /* else it was a comma */
                            }
		            if (ok)
		                plot_addname (fullpath);
		        }

		        if (nodes_in_dis_path && ok) {
			    dis_addname (fullpath);
			}

			if ( ! ok ) {
		            PALLOC ($$, 1, NODE_REF_LIST);
		            $$ -> nx = -1;
		        }
		    }
		    else {
			if (readp_begin == NULL)
			    readp_begin = fullpath;
			else
			    readp_end -> also = fullpath;
			readp_end = fullpath;
		    }

		    last_path = NULL;  /* for the next call */
		    exclam_flag = FALSE; /* in case it has become TRUE */
		}
		| COMMA
		{
		    if (! read_res_refs) {
			if (nodes_in_res_path) {
			    PALLOC ($$, 1, NODE_REF_LIST);
			    $$ -> nx = -1;
			}
			else
			    $$ = NULL;
		    }
		}
		;

full_node_ref 	: member_ref
		| EXCLAM
		{
		     exclam_flag = TRUE;
		}
		  member_ref
		| full_node_ref DOT member_ref
		;

member_ref	: member_name
		{
		    if (last_path == NULL) {
			/* this is the leftmost (the first) member of a path */
			if (nodes_in_res_path || read_res_refs) {
			    PALLOC (last_path, 1, PATH_SPEC);
			}
			else {
			    last_path = pathspace;
			}
			fullpath = last_path;
		    }
		    else {
			if (nodes_in_res_path || read_res_refs) {
			    PALLOC (last_path -> next, 1, PATH_SPEC);
			}
			else {
		            last_path -> next = last_path + 1;
			}
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
		    strcpy (name_space, yy0text);
		    $$ = name_space;
		}
		;

keyword		: SET
		| LEVEL		{ vardummy = $1; }
		| LOGIC_LEVEL	{ vardummy = $1; }
		| FROM
		| FILL
		| WITH
		| PRINT
		| PLOT_TOKEN
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
		| TOGGLE	{ vardummy = $1; }
		| STEP
		| RANDOM
		| FULL
		| DEFINE_TOKEN
		| STA_FILE
		| TST_FILE
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
		{
		    *$1 = $3;
		}
		| int_option EQL integer
		{
		    if ( $1 != NULL )
		        *$1 = $3;
		    else {
		        switch ( $3 ) {
			    case 1 :
				proclogic = FALSE;
				delaysim = FALSE;
				break;
			    case 2 :
				proclogic = TRUE;
				delaysim = FALSE;
			        break;
			    case 3 :
				proclogic = TRUE;
				delaysim  = TRUE;
				break;
			    default:
				cslserror (ERROR2, "illegal simulation level", NULL);
				break;
		        }
		    }
		}
		| SIMPERIOD EQL INT
		{
		    simperiod = atoll ($3);
		}
		| DISPERIOD EQL INT
		{
		    disperiod = atoll ($3);
		}
		| SIGOFFSET EQL INT
		{
		    sig_toffset = atoll ($3);
		}
		| pow_ten_option EQL pow_ten
		{
		    *$1 = $3;
		}
		| f_float_option EQL f_float
		{
		    *$1 = $3;
		}
		| SLS_PROCESS EQL STRING
		{
		    PALLOC (fn_proc, strlen ($3) + 1, char);
		    strcpy (fn_proc, $3);
		}
		;

toggle_option	: STEP
		{
		    $$ = &outonchange;
		}
		| PRINT RACES
		{
		    $$ = &printraces;
		}
		| ONLY CHANGES
		{
		    $$ = &printremain;
		}
		| PRINT DEVICES
		{
		    $$ = &printdevices;
		}
		| PRINT STATISTICS
		{
		    $$ = &printstatis;
		}
		| INITIALIZE RANDOM
		{
		    $$ = &random_initialization;
		}
		| INITIALIZE FULL RANDOM
		{
		    $$ = &random_td_initialization;
		}
		| STA_FILE
		{
		    $$ = &try_sta_file;
		}
		| TST_FILE
		{
		    $$ = &tester_output;
		}
		;

int_option	: MAXNVICIN
		{
		    cslserror (WARNING, "maxnvicin specification ignored", NULL);
		    $$ = &vardummy;
		}
		| MAXTVICIN
		{
		    cslserror (WARNING, "maxtvicin specification ignored", NULL);
		    $$ = &vardummy;
		}
		| MAXLDEPTH
		{
		    $$ = &logic_depth;
		}
		| MAXPAGEWIDTH
		{
		    $$ = &maxpagewidth;
		}
		| LEVEL
		{
		    $$ = NULL;
		}
		;

pow_ten_option	: OUTUNIT
		{
		    $$ = &outtimeunit;
		}
		| OUTACC
		{
		    $$ = &outtimeaccur;
		}
		;

pow_ten		: INT
		{
		    $$ = atof ($1);
		}
		| POWER_TEN
		{
		    $$ = $1;
		}
		;

f_float_option	: SIGUNIT
		{
		    $$ = &sigtimeunit;
		}
		| VH
		{
		    $$ = &vHtmp;
		}
		| VMAXL
		{
		    $$ = &vmaxL;
		}
		| VMINH
		{
		    $$ = &vminH;
		}
		| TDEVMIN
		{
		    $$ = &mindevtime;
		}
		| TDEVMAX
		{
		    $$ = &maxdevtime;
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

dump_cmd	: DUMP AT INT
		{
		    int i = arr_new ((char **)(void *)&tdumps);
		    tdumps[i] = atoll ($3);
		}
		;

init_cmd	: INITIALIZE FROM STRING
		{
		    PALLOC (fn_init, strlen ($3) + 1, char);
		    strcpy (fn_init, $3);
		}
		;

fill_cmd	: FILL full_node_ref WITH
                {
		    char varname[DM_MAXNAME + 5];
		    PATH_SPEC * varpath;

                    varpath = fullpath;
		    while (varpath -> next != NULL)
		        varpath = varpath -> next;
		        /* to find the var name */
                    if (varpath == fullpath)
                        sprintf (varname, "%s", varpath -> name);
                    else
                        sprintf (varname, "-.%s", varpath -> name);

		    switch ((filltype = findnodes (fullpath, NULL, FALSE, &freadnref, TRUE))) {
			case NAMENEG :
	                    cslserror (ERROR2, "undefined variable", varname);
			    break;
			case NOPATH :
			case EMPTYTYPE :
	                    cslserror (ERROR2, "internal error", NULL);
			    break;
			case NONODE :
	                    cslserror (ERROR2, "not a variable is name", varname);
			    break;
			case REFIERR :
	                    cslserror (ERROR2, "incorrect indices for variable", varname);
			    break;
			case REFINEG :
	                    cslserror (ERROR2, "not defined as array is variable", varname);
			    break;
		        case REFIMIS :
	                    cslserror (ERROR2, "indices missing for variable", varname);
			    break;
		        default :
		            break;
		    }
		    last_path = NULL;  /* for the next call */
		    exclam_flag = FALSE; /* in case it has become TRUE */
		}
		  fillvals
		{
		    if (freadnref != NULL)
	                cslserror (ERROR2, "number of values less than number of variables", NULL);
		}
		;

fillvals	: fillchars
		| fillvals fillchars
		| fillint
		| fillvals fillint
		| fillfloat
		| fillvals fillfloat
		;

fillchars	: STRING
		{
		    char * p;

		    if (filltype != CHARTYPE)
	                cslserror (ERROR2, "variable is not of type char", NULL);
                    else {
		        for (p = $1; *p != '\0'; p++) {
		            if (freadnref != NULL) {
		                *(char *)(freadnref -> xptr) = *p;
		                freadnref = freadnref -> next;
		            }
		            else {
	                        cslserror (ERROR2, "number of values larger than number of variables", NULL);
		                break;
		            }
		        }
		    }
		}
		;

fillint		: integer
		{
		    if (filltype != INTEGERTYPE && filltype != FLOATTYPE && filltype != DOUBLETYPE)
	                cslserror (ERROR2, "variable is not of type float, double or integer", NULL);
                    else {
		        if (freadnref != NULL) {
                            if (filltype == INTEGERTYPE)
		                *(int *)(freadnref -> xptr) = $1;
		            else if (filltype == FLOATTYPE)
		                *(float *)(freadnref -> xptr) = $1;
			    else
		                *(double *)(freadnref -> xptr) = $1;
		            freadnref = freadnref -> next;
		        }
		        else {
	                    cslserror (ERROR2, "number of values larger than number of variables", NULL);
		            break;
		        }
		    }
		}
		;

fillfloat	: F_FLO
		{
		    if (filltype != FLOATTYPE && filltype != DOUBLETYPE)
	                cslserror (ERROR2, "variable is not of type float or double", NULL);
                    else {
		        if (freadnref != NULL) {
		            if (filltype == FLOATTYPE)
		                *(float *)(freadnref -> xptr) = $1;
			    else
		                *(double *)(freadnref -> xptr) = $1;
		            freadnref = freadnref -> next;
		        }
		        else {
	                    cslserror (ERROR2, "number of values larger than number of variables", NULL);
		            break;
		        }
		    }
		}
		;

define_cmd	: DEFINE_TOKEN node_refs COLON member_name
                {
		    ABSTRACT_OUTPUT *new_ao_el;
		    ABSTRACT_OUTPUT *ab_el;
		    NODE_REF_LIST *nrl;
		    int err = 0;

		    ab_el = abstractl_begin;
		    while (ab_el) {
			if (strcmp (ab_el -> name, $4) == 0) {
			    cslserror (ERROR2, "re-definition of name", $4);
			    err = 1;
			    break;
			}
			ab_el = ab_el -> next;
		    }

		    if (!err) {
			strcpy (pathspace -> name, $4);
			pathspace -> next = NULL;
			pathspace -> also = NULL;
			pathspace -> xarray[0][0] = 0;
			if (findnodes (pathspace, NULL, FALSE, &nrl, FALSE) != NAMENEG) {
			    cslserror (ERROR2, "defined name already in use:", $4);
			}
		    }

		    if (!err) {
			PALLOC (new_ao_el, 1, struct abstract_output);
			PALLOC (new_ao_el -> name, strlen ($4) + 1, char);
			strcpy (new_ao_el -> name, $4);
			new_ao_el -> inputs = $2;
			new_ao_el -> nr_inputs = 0;
			nrl = new_ao_el -> inputs;
			while (nrl) {
			    if (nrl -> nx >= 0)
				(new_ao_el -> nr_inputs)++;
			    nrl = nrl -> next;
			}
			new_ao_el -> vals = NULL;
			new_ao_el -> next = NULL;

			if (abstractl_begin == NULL)
			    abstractl_begin = abstractl_end = new_ao_el;
			else
			    abstractl_end -> next = new_ao_el;
			abstractl_end = new_ao_el;
		    }
		}
		  define_entries
		;

define_entries	: define_entry
		| define_entries define_entry
		;

define_entry	: def_sig_vals COLON escape_char member_name
		{
		    if ($3) {
			PALLOC (curr_av_el -> out_value, strlen ($4) + 2, char);
			sprintf (curr_av_el -> out_value, "$%s", $4);
		    }
		    else {
			PALLOC (curr_av_el -> out_value, strlen ($4) + 1, char);
			strcpy (curr_av_el -> out_value, $4);
		    }

		    if (av_cnt != abstractl_end -> nr_inputs) {
			cslserror (ERROR2, "incorrect number of input values for output value", curr_av_el -> out_value);
		    }
		}
		;

escape_char	: DOLLAR
		{
		    $$ = 1;
		}
		| /* empty */
		{
		    $$ = 0;
		}
		;

def_sig_vals	: def_sig_val
                {
		    ABSTRACT_VALUE *new_av_el;

		    PALLOC (new_av_el, 1, struct abstract_value);
		    PALLOC (new_av_el -> in_values, abstractl_end -> nr_inputs, int);
		    new_av_el -> next = NULL;

                    if (abstractl_end -> vals == NULL)
			abstractl_end -> vals = new_av_el;
		    else
			curr_av_el -> next = new_av_el;
		    curr_av_el = new_av_el;

		    av_cnt = 0;

		    if (av_cnt < abstractl_end -> nr_inputs)
			curr_av_el -> in_values[av_cnt] = $1;

		    av_cnt++;
		}
		| def_sig_vals def_sig_val
		{
		    if (av_cnt < abstractl_end -> nr_inputs)
			curr_av_el -> in_values[av_cnt] = $2;

		    av_cnt++;
		}
		;

def_sig_val	: LOGIC_LEVEL
                {
		    $$ = $1;
		}
		| MINUS
		{
		    $$ = Dontcare;
		}
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

void cmdinits ()
{
    len_sp = len_stack;
    sgn_sp = sgn_stack;

    arr_init ((char **)(void *)&tdumps, sizeof (simtime_t), 6, 2.0);
    arr_init ((char **)(void *)&prinvert, sizeof (NODE_REF_LIST *), 10, 2.0);
}

void yy0error (char *s)
{
    cslserror (ERROR2, s, NULL);
}

static void cslserror (int errtype, char *s1, char *s2)
{
    int lineno = yy0lineno;

    if (yychar == NEWLINE) lineno--;

    slserror (fn_cmd, lineno, errtype, s1, s2);
}

static SIGNALELEMENT *read_signal (FILE *fp, char *fn, long offset, int sig_cnt, int pos)
/* reads the signal description from file 'fp' and returns it */
/* as a list of signalelements */
{
    SIGNALELEMENT *father, *sgnel;
    char val, valprev;
    simtime_t t, tprev;

    PALLOC (father, 1, SIGNALELEMENT);

    fseek (fp, offset, 0);

    valprev = '#';
    val = '#';
    tprev = -1;
    t = -1;
    sgnel = 0;
    while (fscanf (fp, "%lld", &t) > 0) {

	if (t == tprev) {
	    fseek (fp, (long)(sig_cnt * sizeof (char)), 1);
	    continue;
	}

        fseek (fp, (long)((pos - 1) * sizeof (char)), 1);
	val = fgetc (fp);
	if (val == valprev || val == '.') {
            fseek (fp, (long)((sig_cnt - pos) * sizeof (char)), 1);
	    continue;
	}

        if (father -> sibling == NULL) {
            PALLOC (father -> sibling, 1, SIGNALELEMENT);
	    sgnel = father -> sibling;
	}
	else {
	    sgnel -> len = t - tprev;
            PALLOC (sgnel -> sibling, 1, SIGNALELEMENT);
	    sgnel = sgnel -> sibling;
	}
	switch (val) {
	    case 'h' :
	        sgnel -> val = H_state;
		break;
	    case 'l' :
	        sgnel -> val = L_state;
		break;
	    case 'x' :
	        sgnel -> val = X_state;
		break;
	    case 'f' :
	        sgnel -> val = F_state;
		break;
	    default :
		slserror (NULL, 0, ERROR1, "illegal character in", fn);
		break;
	}

        fseek (fp, (long)((sig_cnt - pos) * sizeof (char)), 1);
	tprev = t;
	valprev = val;
    }

    if (sgnel) {
	sgnel -> len = -1;
        father -> val = sgnel -> val;
        father -> len = -1;
    }
    else {
        father -> val = '?';
        father -> len = 0;
    }

    return (father);
}

static SIGNALELEMENT *copysgn (SIGNALELEMENT *sgn)
{
    SIGNALELEMENT *newsgn;

    PALLOC (newsgn, 1, SIGNALELEMENT);
    newsgn -> val = sgn -> val;
    newsgn -> len = sgn -> len;

    if (sgn -> sibling)
	newsgn -> sibling = copysgn (sgn -> sibling);
    if (sgn -> child)
	newsgn -> child = copysgn (sgn -> child);

    return (newsgn);
}

#ifndef YY_CURRENT_BUFFER
#define YY_CURRENT_BUFFER yy_current_buffer
#endif

/* next function resets the state of yylex() */
void restart_scanner (FILE *inputfile)
{
#ifdef FLEX_SCANNER
    if (YY_CURRENT_BUFFER) yyrestart (inputfile);
#else
    /* some versions of lex may require "yysptr = yysbuf" */
#endif
}

int yy0wrap ()
{
    return (1);
}
