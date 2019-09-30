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

#include "src/cldm/extern.h"

%}

%token MS MC ME EOL
%token BOX SBOX TERM LABEL WIRE SWIRE CONT POLY CIRCLE CPEEL
%token CX CY MX MY WIDTH R3 R6 R9 TRANS X Y SFX SFY
%token WORD WORD2 INTEGER FLOAT ILLCHAR

%%
stats		: /* E */ | stats stat
		;
stat		: ms_l    eos
		| mc_l    eos
		| me_l    eos
		| box_l   eos
		| sbox_l  eos
		| term_l  eos
		| label_l  eos
		| wire_l  eos
		| swire_l eos
		| cont_l  eos
		| poly_l  eos
		| circ_l  eos
		| cpeel_l eos
		| /* E */ eos
		| error   eos
		{ yyerrok; }
		;
ms_l		: MS name
		{
		    if (mod_key) {
			close_files ();
			dmCheckIn (mod_key, QUIT);
			mod_key = NULL;
		    }

		    err_flag = 0; /* no errors */

		    strcpy (ms_name, mc_name);
		    if (v_mode) P_E "-- ms %s\n", ms_name);

		    if (!s_mode) {
			if (check_tree (ms_name, mod_tree)) {
			    if (f_mode && !tree_ptr -> bbox) {
				pr_exit (0604, 42, ms_name);
			    }
			    else { /* already defined or used */
				tree_ptr -> errflag = 1;
				if (!f_mode)
				    pr_exit (0214, 9, ms_name);
				else
				    pr_exit (0214, 37, ms_name);
			    }
			}
			if (!err_flag) {
			    mod_key = dmCheckOut (dmproject, ms_name, WORKING, DONTCARE, LAYOUT, CREATE);
			    open_files ();
			    ini_mcbbox = ini_bbbox = 1;
			}
		    }
		}
		;
me_l   		: ME
		{
		    if (err_flag) {
			pr_exit (0204, 26, ms_name);
		    }

		    if (mod_key) {

			if (err_flag) {
			    append_tree (ms_name, &mod_tree);
			    tree_ptr -> errflag = 1;
			    close_files ();
			    dmCheckIn (mod_key, QUIT);
			}
			else {
			    write_info ();
			    close_files ();
			    dmCheckIn (mod_key, COMPLETE);
			}

			mod_key = NULL;
		        ini_mcbbox = ini_bbbox = 0;
		    }
		    rm_tree (tnam_tree);
		    rm_tree (inst_tree);
		    tnam_tree = inst_tree = NULL;
		}
		;
mc_l		: MC insname name x_scaling y_scaling mir rot tr cx cy
		{
		    if ($2 && append_tree (instance, &inst_tree)) {
		        pr_exit (034, 11, instance); /* already used */
		    }

		    if (!err_flag && !s_mode)
			proc_mc ($2, $6, $7);
		}
		;
box_l		: BOX laycode coord coord coord coord cx cy
		{
		    if (!err_flag && !s_mode)
		        proc_box ($3, $4, $5, $6);
		}
		;
sbox_l		: SBOX laycode coord coord coord coord cx cy
		{
		    if (!err_flag && !s_mode)
		        proc_sbox ($3, $4, $5, $6);
		}
		;
term_l		: TERM laycode coord coord coord coord term_name cx cy
		{
		    if (append_tree (terminal, &tnam_tree)) {
		        pr_exit (034, 12, terminal); /* already used */
		    }

		    if (!err_flag && !s_mode)
		        proc_term ($3, $4, $5, $6);
		}
		;
label_l		: LABEL laycode coord coord label_name
		{
		    if (append_tree (label, &tnam_tree)) {
		        pr_exit (0634, 12, label); /* already used */
		    }

		    if (!err_flag && !s_mode)
		        proc_label ($3, $4);
		}
		;
wire_l		: WIRE laycode dir WIDTH coord
		  half_coord comma half_coord incr cx cy
		{
		    w_dir = $3;
		    w_width = $5;
		    w_x = $6;
		    w_y = $8;
		    if (!err_flag && !s_mode)
		        proc_wire ();
		}
		;
swire_l		: SWIRE laycode WIDTH coord
		  half_coord comma half_coord incr cx cy
		{
		    w_width = $4;
		    w_x = $5;
		    w_y = $7;
		    if (!err_flag && !s_mode)
		        proc_swire ();
		}
		;
cont_l		: CONT opt_dir opt_width incr
		{
	            if (!err_flag && !s_mode)
		        proc_cont ($2, $3);
		}
		;
poly_l		: POLY laycode m_int cx cy
		{
		    if (!err_flag && !s_mode)
		        proc_poly ();
		}
		;
circ_l		: CIRCLE laycode coord coord coord opt_n cx cy
		{
		    if ($5 <= 0)
			pr_exit (0214, 48, fromitoa ($5));

		    if (!err_flag && !s_mode)
		        proc_circ ($3, $4, $5, 0, 0, 360000, $6);
		}
		;
cpeel_l		: CPEEL laycode coord coord coord coord angle angle opt_n cx cy
		{
		    int r1, r2;

		    if ($5 > $6) { r1 = $5; r2 = $6; }
		    else         { r1 = $6; r2 = $5; }

		    if (r1 <= 0)  pr_exit (0214, 48, fromitoa (r1));
		    if (r2 <  0)  pr_exit (0214, 48, fromitoa (r2));
		    if (r1 == r2) pr_exit (0214, 49, "radia");
		    if ($7 == $8) pr_exit (0214, 49, "angles");

		    if (!err_flag && !s_mode)
		        proc_circ ($3, $4, r1, r2, $7, $8, $9);
		}
		;
opt_n		: /* E */ { $$ = 32; }
		| integer
		{
		    tmp_i = $1 / 8;
		    tmp_i *= 8;
		    if (tmp_i > 32000) tmp_i = 32000;
		    else if (tmp_i <  8) tmp_i = 8;
		    if (tmp_i != $1)
			pr_exit (0614, 47, fromitoa (tmp_i));
		    $$ = tmp_i;
		}
		;
x_scaling	: /* E */ { sfx = 1; }
		| SFX integer
		{
		    if ((sfx = $2) < 1)
			pr_exit (0214, 46, fromitoa (sfx));
		}
		;
y_scaling	: /* E */ { sfy = 1; }
		| SFY integer
		{
		    if ((sfy = $2) < 1)
			pr_exit (0214, 46, fromitoa (sfy));
		}
		;
mir		: /* E */ { $$ = 0; }
		| MX      { $$ = 1; }
		| MY      { $$ = 2; }
		;
rot		: /* E */ { $$ = 0; }
		| R3      { $$ = 90; }
		| R6      { $$ = 180; }
		| R9      { $$ = 270; }
		;
tr        	: /* E */ 	      { tx = 0; ty = 0; }
		| trans coord comma coord { tx = $2; ty = $4; }
		;
trans		: /* E */ | TRANS ;
cx		: /* E */ { dx = 0; nx = 0; }
		| CX coord comma integer
		{
		   dx = $2;
		   nx = $4;
		   if (nx < 0)
		      pr_exit (034, 13, "nx"); /* invalid rep. parameter */
		   else if (nx == 0)
		      pr_exit (0634, 25, "nx");
		}
		;
cy		: /* E */ { dy = 0; ny = 0; }
		| CY coord comma integer
         	{
		   dy = $2;
		   ny = $4;
		   if (ny < 0)
		      pr_exit (034, 13, "ny"); /* invalid rep. parameter */
		   else if (ny == 0)
		      pr_exit (0634, 25, "ny");
		}
		;
comma		: /* E */ | ',' ;
insname		: /* E */           { $$ = 0; }
		| '<' inst_name '>' { $$ = 1; }
		;
name		: WORD
		{
		   n_tok = 0;
		   strncpy (mc_name, textval(), DM_MAXNAME);
		   if (yyleng > DM_MAXNAME) {
		       pr_exit (0634, 16, textval());
		       sprintf (name_len, "%d", DM_MAXNAME);
		       pr_exit (0600, 19, name_len);
		       mc_name[DM_MAXNAME] = '\0';
		   }
		}
		;
inst_name	: WORD
		{
		   strncpy (instance, textval(), DM_MAXNAME);
		   if (yyleng > DM_MAXNAME) {
		       pr_exit (0634, 17, textval());
		       sprintf (name_len, "%d", DM_MAXNAME);
		       pr_exit (0600, 19, name_len);
		       instance[DM_MAXNAME] = '\0';
		   }
		}
		| WORD2
		{
		   strncpy (instance, textval(), DM_MAXNAME);
		   if (yyleng > DM_MAXNAME) {
		       pr_exit (0634, 17, textval());
		       sprintf (name_len, "%d", DM_MAXNAME);
		       pr_exit (0600, 19, name_len);
		       instance[DM_MAXNAME] = '\0';
		   }
		}
		;
term_name	: name_str
		{
		   n_tok = t_tok = 0;
		   strncpy (terminal, textval(), DM_MAXNAME);
		   if (yyleng > DM_MAXNAME) {
		       pr_exit (0634, 18, textval());
		       sprintf (name_len, "%d", DM_MAXNAME);
		       pr_exit (0600, 19, name_len);
		       terminal[DM_MAXNAME] = '\0';
		   }
		}
		;
label_name	: name_str
		{
		   n_tok = t_tok = 0;
		   strncpy (label, textval(), DM_MAXNAME);
		   if (yyleng > DM_MAXNAME) {
		       pr_exit (0634, 18, textval());
		       sprintf (name_len, "%d", DM_MAXNAME);
		       pr_exit (0600, 19, name_len);
		       label[DM_MAXNAME] = '\0';
		   }
		}
		;
name_str	: WORD
		| WORD2
		| INTEGER
		;
laycode		: WORD
		{
		   if (!t_tok) n_tok = 0;
	           strncpy (layer, textval(), DM_MAXLAY);
		   if (yyleng > DM_MAXLAY) {
		       pr_exit (0634, 15, textval());
		       sprintf (name_len, "%d", DM_MAXLAY);
		       pr_exit (0600, 19, name_len);
		       layer[DM_MAXLAY] = '\0';
		   }

		   if (!s_mode) {
		       for (lay_code = 0;
			       lay_code < process->nomasks; ++lay_code)
			   if (!strcmp (layer, process->mask_name[lay_code]))
			       goto label1;

		       pr_exit (034, 20, layer); /* unrecogn. laycode */
		   }
		   label1: ;
		}
		;
m_int		: kwart_coord
                {
                   int_val[0] = $1; int_ind = 1;
                }
		| m_int kwart_coord
		{
		    if (int_ind >= NOINTS)
			pr_exit (0137, 8, 0);

		    int_val[int_ind++] = $2;
		}
		;
incr		: half_coord
                {
                   int_val[0] = $1; int_ind = 1;
                }
		| incr half_coord
                {
		   if (int_ind >= NOINTS)
		       pr_exit (0137, 8, 0);

		   int_val[int_ind++] = $2;
                }
		;
opt_dir		: /* E */ { $$ = 0; }
		| dir
		;
dir		: X       { $$ = -1; }
		| Y       { $$ =  1; }
		;
opt_width	: /* E */     { $$ = w_width; }
		| WIDTH coord { $$ = $2; }
		;
angle		: INTEGER
		{
		    if (yylval < 0 || yylval > 36000) {
			pr_exit (0214, 41, fromitoa (yylval));
		    }
		    $$ = 1000 * yylval;
		}
		| FLOAT
		{
		    if (d_f < 0 || d_f > 360) {
			pr_exit (0214, 41, textval());
		    }
		    d_f = 1000 * d_f;
		    tmp_d = ROUND (d_f);
		    tmp_d = tmp_i = (int)tmp_d;
		    if (!r_mode) {
		        tmp_d = tmp_d - d_f;
		        if (tmp_d > 0.0001 || tmp_d < -0.0001) {
			    pr_exit (0614, 44, textval());
			    pr_exit (0614, 43, fromftoa ((double)tmp_i / 1000));
		        }
		    }
		    $$ = tmp_i;
		}
		;
kwart_coord	: INTEGER
		{
		    $$ = 4 * yylval;
		}
		| FLOAT
		{
		    d_f = 4 * d_f;
		    tmp_d = ROUND (d_f);
		    tmp_d = tmp_i = (int)tmp_d;
		    if (!r_mode) {
		        tmp_d = tmp_d - d_f;
		        if (tmp_d > 0.0001 || tmp_d < -0.0001) {
			    pr_exit (0614, 51, textval());
			    pr_exit (0614, 43, fromftoa ((double)tmp_i / 4));
		        }
		    }
		    $$ = tmp_i;
		}
		;
half_coord	: INTEGER
		{
		    $$ = 2 * yylval;
		}
		| FLOAT
		{
		    d_f = 2 * d_f;
		    tmp_d = ROUND (d_f);
		    tmp_d = tmp_i = (int)tmp_d;
		    if (!r_mode) {
		        tmp_d = tmp_d - d_f;
		        if (tmp_d > 0.0001 || tmp_d < -0.0001) {
			    pr_exit (0614, 14, textval());
			    pr_exit (0614, 43, fromftoa ((double)tmp_i / 2));
		        }
		    }
		    $$ = tmp_i;
		}
		;
coord		: INTEGER
		{
		    $$ = yylval;
		}
		| FLOAT
		{
		    pr_exit (0214, 40, textval());
		    if (yylval <= 0) $$ = 8;
		    else $$ = yylval + 8; /* disable other messages */
		}
		;
integer		: INTEGER
		{
		    $$ = yylval;
		}
		| FLOAT
		{
		    pr_exit (0214, 40, textval());
		    $$ = 8; /* disable other messages */
		}
		;
eos		: comment EOL { yylineno++; } ;
comment		: /* E */ | ':' ;
%%

void yyerror (char *cs)
{
    char *s = textval();
    int c = s[0];
    s[16] = '\0';
    if ((c >= '\0' && c <= ' ') || c > '\176') {
	switch (c) {
	    case '\n': sprintf (s, "eol"); break;
	    case '\0': sprintf (s, "eof"); break;
	    default:   sprintf (s, "\\%03o", c);
	}
    }
    pr_exit (074, 0, cs);
    t_tok = n_tok = 0;
}
