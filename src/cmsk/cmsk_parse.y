%{
/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
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

#include "src/cmsk/extern.h"

#define P_E fprintf(stderr,
#define ROUND(x) (x < 0)?(x - 0.5001):(x + 0.5001)

struct list *newlistptr;
char  *cp;
int    pfactor;
int    t_nr;
int    mlistflag;
int    mlistdef;
extern int yylineno;
int    p_nr, p_mode;
double p_arr[5];
%}

%token MASKS MASKLIST MASKEY
%token PFACTOR PATTERN END
%token RA RR TOK_RS LA POLY TEXT
%token POLYGON PATH CIRCLE CPEEL
%token T1 T2 T3 X Y CX CY MX MY
%token ROTATE ROT0 ROT90 ROT180 ROT270
%token EXTRACT REMOVE TRANSFER
%token FORMMASK GROW AND OR NOT
%token ABSOLUTE RELATIVE SLANTING TEST
%token PATNAME MASKNAME STRING
%token INTEGER FLOAT
%token EOL GARBAGE

%start defs
%%
defs		: /* empty file */
		| defs definition
		;
definition	: pat_def
		| masksdef eol
		| mlistsdef eol
		| PFACTOR integer { pfactor = $2; } eol
		| commands eol
		| eol
		| error
		{
		    pr_err1 ("warning: unidentified statement skipped");
		}
		  eol
		;

pat_def		:
		{
		    cl_boxes_etc ();
		}
		  beg_pattern p_stats end_pattern
		{
		    if (pat_key) write_info ();
		    *pat_name = '\0';
		}
		;
beg_pattern	: PATTERN patname
		{
		    strcpy (pat_name, words[$2]);
		    if (v_mode) pr_err2 ("pattern", pat_name);

		    if (check_tree (pat_name)) {
			/* pattern name found */
			pr_err ("error: pattern", mpn (pat_name),
				    "already defined");
			killpattern ();
		    }

		    if (makepattern)
			mk_patdir ();
		}
		  eol
		| PATTERN error
		{
		    pr_err2 ("error: illegal pattern begin", mpn (pat_name));
		    killpattern ();
		}
		  eol
		{
		    yyerrok;
		}
		;
end_pattern	: END patname
		{
		    if (strcmp (pat_name, words[$2])) {
			pr_err2 ("error: illegal pattern end, expected end",
			    mpn (pat_name));
			killpattern ();
		    }
		}
		  eol
		| END error
		{
		    pr_err2 ("error: illegal pattern end", mpn (pat_name));
		    killpattern ();
		}
		  eol
		;

p_stats		: /* empty */
		| p_stats pat_stat
		;
pat_stat	: simple_pattern eol
		| complex_pattern eol
		| slanting_pattern eol
		| pat_call eol
		| functioncall eol
		| masksdef eol
		| mlistsdef eol
		| eol
		| error
		{
		    pr_err2 ("error: unidentified statement part:", textval());
		    killpattern ();
		}
		  eol
		;

simple_pattern	: RA '(' masklist ',' coord ',' coord ',' coord ',' coord ')' placement
		{
		    if (pat_key) {
			p_x[0] = $5;
			p_x[1] = $7;
			p_y[0] = $9;
			p_y[1] = $11;
			printonebox ();
		    }
		}
		| RR '(' masklist ',' coord ',' coord ')' placement
		{
		    if (pat_key) {
			p_x[0] = 0;
			p_x[1] = $5;
			p_y[0] = 0;
			p_y[1] = $7;
			printonebox ();
		    }
		}
		;

complex_pattern : la_pattern placement
		{
		    if (pat_key)
			pr_la_pattern ();
		}
		| poly_pattern placement
		{
		    if (pat_key)
			pr_boxes_from_edges ();
		}
		| text_pattern placement
		{
		    if (pat_key)
			pr_boxes_from_edges ();
		}
		;
la_pattern	: LA '(' masklist ',' coord
		{
		    line_width = $5;
		    lw_tan225 = line_width * TAN225;
		    lw_hsqrt2 = line_width * HSQRT2;
		}
		    line_start lin_segments ')'
		;
line_start	: ',' coord ',' direction ',' coord ',' coord
		{
		if (pat_key) {
		    register struct la_elmt *t1, *t2;
		    for (t1 = first_la_elmt; t1; t1 = t2) {
			t2 = t1 -> next;
			FREE (t1);
		    }
		    ALLOC_STRUCT (first_la_elmt, la_elmt);
		    last_la_elmt = first_la_elmt;
		    last_la_elmt -> gtype = 0;
		    if ($4 == 0) { /* X-direction */
			x1 = $2; g_y1 = $6; x2 = $8;
			if (x2 < x1) {
			    last_la_elmt -> xl = x2;
			    last_la_elmt -> xr = x1;
			    last_la_elmt -> yb = g_y1 - line_width;
			    last_la_elmt -> yt = g_y1;
			}
			else {
			    last_la_elmt -> xl = x1;
			    last_la_elmt -> xr = x2;
			    last_la_elmt -> yb = g_y1;
			    last_la_elmt -> yt = g_y1 + line_width;
			}
			prev_grow = x2 - x1;
			x1 = x2; dir_flag = 1;
		    }
		    else {		/* Y-direction */
			g_y1 = $2; x1 = $6; y2 = $8;
			if (y2 < g_y1) {
			    last_la_elmt -> xl = x1;
			    last_la_elmt -> xr = x1 + line_width;
			    last_la_elmt -> yb = y2;
			    last_la_elmt -> yt = g_y1;
			}
			else {
			    last_la_elmt -> xl = x1 - line_width;
			    last_la_elmt -> xr = x1;
			    last_la_elmt -> yb = g_y1;
			    last_la_elmt -> yt = y2;
			}
			prev_grow = y2 - g_y1;
			g_y1 = y2; dir_flag = 0;
		    }
		}
		}
		;
lin_segments	: /* empty */
		| lin_segments line_segment
		;
line_segment	: ',' coord
		{
		if (pat_key) {
		    register struct la_elmt *new1;
		    ALLOC_STRUCT (new1, la_elmt);
		    new1 -> gtype = 0;
		    if (dir_flag == 0) { /* X-direction */
			x2 = $2;
			if (x2 < x1) {
			    if (prev_grow <= 0) {
				new1 -> xr = x1 + line_width;
				last_la_elmt -> yb -= line_width;
			    }
			    else {
				new1 -> xr = x1;
			    }
			    new1 -> xl = x2;
			    new1 -> yb = g_y1 - line_width;
			    new1 -> yt = g_y1;
			}
			else {
			    if (prev_grow >= 0) {
				new1 -> xl = x1 - line_width;
				last_la_elmt -> yt += line_width;
			    }
			    else {
				new1 -> xl = x1;
			    }
			    new1 -> xr = x2;
			    new1 -> yb = g_y1;
			    new1 -> yt = g_y1 + line_width;
			}
			prev_grow = x2 - x1;
			x1 = x2; dir_flag = 1;
		    }
		    else {		/* Y-direction */
			y2 = $2;
			if (y2 < g_y1) {
			    if (prev_grow >= 0) {
				new1 -> yt = g_y1 + line_width;
				last_la_elmt -> xr += line_width;
			    }
			    else {
				new1 -> yt = g_y1;
			    }
			    new1 -> xl = x1;
			    new1 -> xr = x1 + line_width;
			    new1 -> yb = y2;
			}
			else {
			    if (prev_grow <= 0) {
				new1 -> yb = g_y1 - line_width;
				last_la_elmt -> xl -= line_width;
			    }
			    else {
				new1 -> yb = g_y1;
			    }
			    new1 -> xl = x1 - line_width;
			    new1 -> xr = x1;
			    new1 -> yt = y2;
			}
			prev_grow = y2 - g_y1;
			g_y1 = y2; dir_flag = 0;
		    }
		    last_la_elmt -> next = new1;
		    new1 -> next = NULL;
		    last_la_elmt = new1;
		}
		}
		| ',' SLANTING ',' direction ',' coord ',' coord
		{
		if (pat_key) {
		    register struct la_elmt *new1;
		    register struct la_elmt *new2;
		    ALLOC_STRUCT (new1, la_elmt);
		    ALLOC_STRUCT (new2, la_elmt);
		    if (dir_flag == 0) { /* X-direction */
			if ($4 == 0) { /* SX */
			    x2 = $8; y2 = $6;
			    if (x2 < x1) {
				new2 -> gtype = LA_G_XR;
				new2 -> xl = x2;
				new2 -> yb = y2 - line_width;
				new2 -> yt = y2;
				if (prev_grow <= 0) {
				    new1 -> gtype = LA_SB_N;
				    last_la_elmt -> gtype |= LA_G_YB;
				    last_la_elmt -> yb -= lw_tan225;
				    new2 -> xr = x1 - (g_y1 - y2) + lw_tan225;
				    new1 -> xl = new2 -> xr;
				    new1 -> yb = new2 -> yb;
				    new1 -> xr = last_la_elmt -> xr - lw_hsqrt2;
				    new1 -> yt = last_la_elmt -> yb + lw_hsqrt2;
				}
				else {
				    new1 -> gtype = LA_SB_P;
				    last_la_elmt -> gtype |= LA_G_YT;
				    new2 -> xr = x1 - (y2 - g_y1);
				    new1 -> xl = x1 - lw_hsqrt2;
				    new1 -> yb = g_y1 - lw_hsqrt2;
				    new1 -> xr = new2 -> xr;
				    new1 -> yt = y2;
				}
			    }
			    else { /* x2 >= x1 */
				new2 -> gtype = LA_G_XL;
				new2 -> xr = x2;
				new2 -> yb = y2;
				new2 -> yt = y2 + line_width;
				if (prev_grow >= 0) {
				    new1 -> gtype = LA_SB_N;
				    last_la_elmt -> gtype |= LA_G_YT;
				    last_la_elmt -> yt += lw_tan225;
				    new2 -> xl = x1 + (y2 - g_y1) - lw_tan225;
				    new1 -> xl = last_la_elmt -> xl + lw_hsqrt2;
				    new1 -> yb = last_la_elmt -> yt - lw_hsqrt2;
				    new1 -> xr = new2 -> xl;
				    new1 -> yt = new2 -> yt;
				}
				else {
				    new1 -> gtype = LA_SB_P;
				    last_la_elmt -> gtype |= LA_G_YB;
				    new2 -> xl = x1 + (g_y1 - y2);
				    new1 -> xl = new2 -> xl;
				    new1 -> yb = y2;
				    new1 -> xr = x1 + lw_hsqrt2;
				    new1 -> yt = g_y1 + lw_hsqrt2;
				}
			    }
			    prev_grow = x2 - x1;
			    dir_flag = 1;
			}
			else { /* SY */
			    x2 = $6; y2 = $8;
			    if (x2 < x1) {
				if (prev_grow <= 0) {
				    last_la_elmt -> gtype |= LA_G_YB;
				    last_la_elmt -> yb -= lw_tan225;
				    new1 -> gtype = LA_SB_N;
				    new2 -> gtype = LA_G_YT;
				    new2 -> xl = x2;
				    new2 -> xr = x2 + line_width;
				    new2 -> yb = y2;
				    new2 -> yt = g_y1 - (x1 - x2);
				    new1 -> xl = x2 + lw_hsqrt2;
				    new1 -> yb = new2 -> yt - lw_hsqrt2;
				    new1 -> xr = last_la_elmt -> xr - lw_hsqrt2;
				    new1 -> yt = last_la_elmt -> yb + lw_hsqrt2;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_YT;
				    new1 -> gtype = LA_SB_P;
				    new2 -> gtype = LA_G_YB;
				    new2 -> xl = x2 - line_width;
				    new2 -> xr = x2;
				    new2 -> yb = g_y1 + (x1 - x2) - lw_tan225;
				    new2 -> yt = y2;
				    new1 -> xl = x1 - lw_hsqrt2;
				    new1 -> yb = g_y1 - lw_hsqrt2;
				    new1 -> xr = new2 -> xl + lw_hsqrt2;
				    new1 -> yt = new2 -> yb + lw_hsqrt2;
				}
			    }
			    else { /* x2 >= x1 */
				if (prev_grow >= 0) {
				    last_la_elmt -> gtype |= LA_G_YT;
				    last_la_elmt -> yt += lw_tan225;
				    new1 -> gtype = LA_SB_N;
				    new2 -> gtype = LA_G_YB;
				    new2 -> xl = x2 - line_width;
				    new2 -> xr = x2;
				    new2 -> yb = g_y1 + (x2 - x1);
				    new2 -> yt = y2;
				    new1 -> xl = last_la_elmt -> xl + lw_hsqrt2;
				    new1 -> yb = last_la_elmt -> yt - lw_hsqrt2;
				    new1 -> xr = x2 - lw_hsqrt2;
				    new1 -> yt = new2 -> yb + lw_hsqrt2;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_YB;
				    new1 -> gtype = LA_SB_P;
				    new2 -> gtype = LA_G_YT;
				    new2 -> xl = x2;
				    new2 -> xr = x2 + line_width;
				    new2 -> yb = y2;
				    new2 -> yt = g_y1 - (x2 - x1) + lw_tan225;
				    new1 -> xl = new2 -> xr - lw_hsqrt2;
				    new1 -> yb = new2 -> yt - lw_hsqrt2;
				    new1 -> xr = x1 + lw_hsqrt2;
				    new1 -> yt = g_y1 + lw_hsqrt2;
				}
			    }
			    prev_grow = y2 - g_y1;
			    dir_flag = 0;
			}
		    }
		    else {		/* Y-direction */
			if ($4 == 0) { /* SX */
			    x2 = $8; y2 = $6;
			    if (y2 < g_y1) {
				if (prev_grow <= 0) {
				    last_la_elmt -> gtype |= LA_G_XL;
				    new1 -> gtype = LA_SB_N;
				    new2 -> gtype = LA_G_XR;
				    new2 -> xl = x2;
				    new2 -> xr = x1 - (g_y1 - y2) + lw_tan225;
				    new2 -> yb = y2 - line_width;
				    new2 -> yt = y2;
				    new1 -> xl = new2 -> xr;
				    new1 -> yb = new2 -> yb;
				    new1 -> xr = x1;
				    new1 -> yt = g_y1;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_XR;
				    last_la_elmt -> xr += lw_tan225;
				    new1 -> gtype = LA_SB_P;
				    new2 -> gtype = LA_G_XL;
				    new2 -> xl = x1 + (g_y1 - y2);
				    new2 -> xr = x2;
				    new2 -> yb = y2;
				    new2 -> yt = y2 + line_width;
				    new1 -> xl = new2 -> xl;
				    new1 -> yb = y2;
				    new1 -> xr = last_la_elmt -> xr;
				    new1 -> yt = last_la_elmt -> yt;
				}
			    }
			    else { /* y2 >= g_y1 */
				if (prev_grow >= 0) {
				    last_la_elmt -> gtype |= LA_G_XR;
				    new1 -> gtype = LA_SB_N;
				    new2 -> gtype = LA_G_XL;
				    new2 -> xl = x1 + (y2 - g_y1) - lw_tan225;
				    new2 -> xr = x2;
				    new2 -> yb = y2;
				    new2 -> yt = y2 + line_width;
				    new1 -> xl = x1;
				    new1 -> yb = g_y1;
				    new1 -> xr = new2 -> xl;
				    new1 -> yt = new2 -> yt;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_XL;
				    last_la_elmt -> xl -= lw_tan225;
				    new1 -> gtype = LA_SB_P;
				    new2 -> gtype = LA_G_XR;
				    new2 -> xl = x2;
				    new2 -> xr = x1 - (y2 - g_y1);
				    new2 -> yb = y2 - line_width;
				    new2 -> yt = y2;
				    new1 -> xl = last_la_elmt -> xl;
				    new1 -> yb = last_la_elmt -> yb;
				    new1 -> xr = new2 -> xr;
				    new1 -> yt = y2;
				}
			    }
			    prev_grow = x2 - x1;
			    dir_flag = 1;
			}
			else { /* SY */
			    x2 = $6; y2 = $8;
			    if (y2 < g_y1) {
				new2 -> gtype = LA_G_YT;
				new2 -> xl = x2;
				new2 -> xr = x2 + line_width;
				new2 -> yb = y2;
				if (prev_grow <= 0) {
				    last_la_elmt -> gtype |= LA_G_XL;
				    new1 -> gtype = LA_SB_N;
				    new2 -> yt = g_y1 - (x1 - x2);
				    new1 -> xl = x2 + lw_hsqrt2;
				    new1 -> yb = new2 -> yt - lw_hsqrt2;
				    new1 -> xr = x1;
				    new1 -> yt = g_y1;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_XR;
				    last_la_elmt -> xr += lw_tan225;
				    new1 -> gtype = LA_SB_P;
				    new2 -> yt = g_y1 - (x2 - x1) + lw_tan225;
				    new1 -> xl = new2 -> xr - lw_hsqrt2;
				    new1 -> yb = new2 -> yt - lw_hsqrt2;
				    new1 -> xr = last_la_elmt -> xr;
				    new1 -> yt = last_la_elmt -> yt;
				}
			    }
			    else { /* y2 >= g_y1 */
				new2 -> gtype = LA_G_YB;
				new2 -> xl = x2 - line_width;
				new2 -> xr = x2;
				new2 -> yt = y2;
				if (prev_grow >= 0) {
				    last_la_elmt -> gtype |= LA_G_XR;
				    new1 -> gtype = LA_SB_N;
				    new2 -> yb = g_y1 + (x2 - x1);
				    new1 -> xl = x1;
				    new1 -> yb = g_y1;
				    new1 -> xr = x2 - lw_hsqrt2;
				    new1 -> yt = new2 -> yb + lw_hsqrt2;
				}
				else {
				    last_la_elmt -> gtype |= LA_G_XL;
				    last_la_elmt -> xl -= lw_tan225;
				    new1 -> gtype = LA_SB_P;
				    new2 -> yb = g_y1 + (x1 - x2) - lw_tan225;
				    new1 -> xl = last_la_elmt -> xl;
				    new1 -> yb = last_la_elmt -> yb;
				    new1 -> xr = new2 -> xl + lw_hsqrt2;
				    new1 -> yt = new2 -> yb + lw_hsqrt2;
				}
			    }
			    prev_grow = y2 - g_y1;
			    dir_flag = 0;
			}
		    }
		    last_la_elmt -> next = new1;
		    new1 -> next = new2;
		    new2 -> next = NULL;
		    last_la_elmt = new2;
		    x1 = x2; g_y1 = y2;
		}
		}
		;
direction	: X { $$ = 0; }
		| Y { $$ = 1; }
		;

poly_pattern	: POLY '(' masklist ',' start_pair coordi_pairs ')'
		{
		    if (pat_key)
			sort_ins_edge (start_x, y_previous, start_y, RIGHT);
		}
		;
start_pair	: coord ',' coord
		{
		    start_x = $1; start_y = y_previous = $3;
		}
		;
coordi_pairs	: /* empty */
		| coordi_pairs ',' coord_pair
		;
coord_pair	: coord ',' coord
		{
		    if (pat_key)
			sort_ins_edge ($1, y_previous, $3, RIGHT);

		    y_previous = $3;
		}
		;

text_pattern	: TEXT '(' MASKNAME ',' coord ',' STRING ')'
		{
		    masknr = getmaskname (words[$3]);
		    if (masknr < 0 || !layset[masknr]) {
			pr_err2 ("error: unknown mask:", words[$3]);
			killpattern ();
		    }

		    if (pat_key) {
			nroflistmasks = 1;
			mlist[0][0] = masknr;
			mlist[0][1] = 0;
			proc_text ($5, words[$7]);
		    }
		}
		;

slanting_pattern: rs_pattern placement
		{
		    if (pat_key)
			pr_rs_pattern ();
		}
		| polygon_pattern placement
		{
		    if (pat_key)
			pr_po_pattern ();
		}
		| path_pattern placement
		{
		    if (pat_key)
			pr_pa_pattern ();
		}
		| circle_pattern placement
		{
		    if (pat_key)
			pr_ci_pattern ();
		}
		| cpeel_pattern placement
		{
		    if (pat_key)
			pr_cp_pattern ();
		}
		;

rs_pattern	: TOK_RS '(' masklist
		    ',' coord ',' coord ',' coord ',' coord ')'
		{
		    p_x[0] = $5; p_y[0] = $7;
		    p_x[1] = $9; p_y[1] = $11;
		}
		;

polygon_pattern	: POLYGON '(' masklist startpoint pointlist ')'
		{
		    if (p_incr > 50) {
			pr_err1 ("error: polygon: more than 50 increments");
			killpattern ();
		    }
		    else {
			if (p_x[p_incr] != p_x[0] || p_y[p_incr] != p_y[0]) {
			    pr_err1 ("error: polygon: endpoint not equal to startpoint");
			    P_E "%s: polygon: endpoint is (%s",
				argv0, d2a (p_x[p_incr]));
			    P_E ",%s)\n", d2a (p_y[p_incr]));
			    killpattern ();
			}
		    }
		}
		;
startpoint	: ',' coord ',' coord
		{
		    p_nr = p_incr = 0;
		    p_x[p_incr] = $2;
		    p_y[p_incr] = $4;
		}
		;
pointlist	: edgepoint
		| pointlist edgepoint
		;
edgepoint	: ',' ABSOLUTE ',' coord ',' coord
		{
		    if (++p_incr <= 50) {
			p_x[p_incr] = $4;
			p_y[p_incr] = $6;
		    }
		}
		| ',' RELATIVE ',' coord ',' coord
		{
		    if (++p_incr <= 50) {
			p_x[p_incr] = p_x[p_incr-1] + $4;
			p_y[p_incr] = p_y[p_incr-1] + $6;
		    }
		}
		;

path_pattern	: PATH '(' masklist startpoint pathptlist ')'
		{
		    if (p_mode) {
			upd_path_coords (0);
		    }
		}
		;
pathptlist	: pathpoint
		{
		    p_mode = $1;
		    if (p_mode == 0 || p_mode == 3) {
			pr_err1 ("syntax error: path: missing a or r");
			killpattern ();
			p_mode = 0;
		    }
		}
		| pathptlist pathpoint
		{
		    if (p_mode) {
			if ($2 == 0) {
			    if (p_nr > 4 || (p_nr > 2 && p_mode == 3)) {
				pr_err1 ("syntax error: path: missing a or r");
				killpattern ();
				p_mode = 0;
			    }
			}
			else {
			    upd_path_coords ($2);
			}
		    }
		}
		;
pathpoint	: ',' coord    { $$ = 0; if (++p_nr <= 4) p_arr[p_nr] = $2; }
		| ',' ABSOLUTE { $$ = 1; }
		| ',' RELATIVE { $$ = 2; }
		| ',' TEST     { $$ = 3; }
		;

circle_pattern	: CIRCLE '(' masklist ',' coord ',' coord ',' coord opt_n ')'
		{
		    p_x[0] = $5;
		    p_y[0] = $7;
		    p_x[1] = $9;
		    if (p_x[1] < 0) {
			pr_err2 ("error: circle: illegal radius:", d2a (p_x[1]));
			killpattern ();
		    }
		}
		;
cpeel_pattern	: CPEEL '(' masklist ',' coord ',' coord
		    ',' coord ',' coord ',' angle ',' angle opt_n ')'
		{
		    p_x[0] = $5;
		    p_y[0] = $7;
		    if ($11 < $9) {
			p_x[1] = $9; p_y[1] = $11;
		    }
		    else {
			p_x[1] = $11; p_y[1] = $9;
		    }
		    if (p_y[1] < 0) {
			pr_err1 ("error: cpeel: radius less than zero");
			killpattern ();
		    }
		    p_x[2] = $13;
		    p_y[2] = $15;
		    if (p_y[2] == p_x[2]) {
			pr_err1 ("error: cpeel: angles are equal");
			killpattern ();
		    }
		}
		;
opt_n		: /* empty */ { n_edges = 32; }
		| ',' integer
		{
		    n_edges = $2;
		    if (n_edges < 0 || n_edges > 32000) {
			pr_err1i ("error: circle/cpeel: illegal n:", n_edges);
			killpattern ();
		    }
		    else {
			n_edges /= 8;
			n_edges *= 8;
			if (n_edges > 96) n_edges = 96;
			else if (n_edges <  8) n_edges = 8;
			if (n_edges != $2)
			    pr_err1i ("warning: circle/cpeel: n set to:",
					n_edges);
		    }
		}
		;

pat_call	: patname tplacement
		{
		  if (t_nr)
		    pr_err1 ("warning: T[123] special processing not implemented");

		  if (pat_key) {
		    if (!check_tree (words[$1])) {
			/* pattern name missing */
			pr_err ("error: pattern", mpn (words[$1]),
					"is not defined");
			killpattern ();
		    }
		    else {
			if (!tree_ptr -> correct) {
			    pr_err ("error: pattern", mpn (words[$1]),
					"defined but not placed");
			    killpattern ();
			}
			if (pat_key) pr_pat_call (words[$1]);
		    }
		  }
		}
		;

functioncall	: REMOVE '(' patname ')'
		{
		    if (!check_tree (words[$3])) {
			/* pattern name missing */
			pr_err2 ("error: unknown pattern:", words[$3]);
			killpattern ();
		    }
		    else {
			if (!tree_ptr -> correct) {
			    pr_err ("error: pattern", mpn (words[$3]),
					"defined but not placed");
			    killpattern ();
			}
		    }

		    if (pat_key) {
			fflush (fpmc -> dmfp); fflush (fpbox -> dmfp);
			get_pattern (words[$3], edges2, tree_ptr -> impcell);
			get_pattern (pat_name,  edges1, (IMPCELL *)0);

			dmCloseStream (fpbox, COMPLETE);
			dmCloseStream (fpmc, COMPLETE);
			fpbox = dmOpenStream (pat_key, "box", "w");
			fpmc = dmOpenStream (pat_key, "mc", "w");

			clean_bb ();
			invert_edges (edges2);
			mix_and_scan_and_grow (1, 0);
			mk_all_boxes_of_edges ();
			rm_array (edges1);
			edgeptr = mostleft = mostright = NULL;
		    }
		}
		| TRANSFER '(' MASKNAME ',' patname opt_grow ')'
		{
		    masknr = getmaskname (words[$3]);
		    if (masknr < 0 || !layset[masknr]) {
			pr_err2 ("error: unknown mask:", words[$3]);
			killpattern ();
		    }

		    if (!check_tree (words[$5])) {
			/* pattern name missing */
			pr_err2 ("error: unknown pattern:", words[$5]);
			killpattern ();
		    }
		    else {
			if (!tree_ptr -> correct) {
			    pr_err ("error: pattern", mpn (words[$5]),
					"defined but not placed");
			    killpattern ();
			}
		    }

		    if (pat_key) {
			fflush (fpbox -> dmfp); fflush (fpmc -> dmfp);
			get_pattern (words[$5], edges1, tree_ptr -> impcell);
			get_pattern (pat_name,  edges2, (IMPCELL *)0);
			mix_and_scan_and_grow (2, $6);
			mk_mask_boxes_of_edges (masknr);
			rm_array (edges1);
			edgeptr = mostleft = mostright = NULL;
		    }
		}
		| EXTRACT '(' patname ',' patname ')'
		{
		    struct ptree *t;

		    if (!check_tree (words[$3])) {
			/* pattern name missing */
			pr_err2 ("error: unknown pattern:", words[$3]);
			killpattern ();
		    }
		    else {
			if (!tree_ptr -> correct) {
			    pr_err ("error: pattern", mpn (words[$3]),
					"defined but not placed");
			    killpattern ();
			}
		    }
		    t = tree_ptr;

		    if (!check_tree (words[$5])) {
			/* pattern name missing */
			pr_err2 ("error: unknown pattern:", words[$5]);
			killpattern ();
		    }
		    else {
			if (!tree_ptr -> correct) {
			    pr_err ("error: pattern", mpn (words[$5]),
					"defined but not placed");
			    killpattern ();
			}
		    }

		    if (pat_key) {
			get_pattern (words[$3], edges1, t -> impcell);
			get_pattern (words[$5], edges2, tree_ptr -> impcell);
			mix_and_scan_and_grow (2, 0);
			mk_all_boxes_of_edges ();
			rm_array (edges1);
			edgeptr = mostleft = mostright = NULL;
		    }
		}
		| FORMMASK MASKNAME
		    '(' MASKNAME opt_not operator MASKNAME opt_not ')'
		{
		    dest_mask = getmaskname (words[$2]);
		    if (dest_mask < 0 || !layset[dest_mask]) {
			pr_err2 ("error: unknown mask:", words[$2]);
			killpattern ();
		    }
		    src1_mask = getmaskname (words[$4]);
		    if (src1_mask < 0 || !layset[src1_mask]) {
			pr_err2 ("error: unknown mask:", words[$4]);
			killpattern ();
		    }
		    src2_mask = getmaskname (words[$7]);
		    if (src2_mask < 0 || !layset[src2_mask]) {
			pr_err2 ("error: unknown mask:", words[$7]);
			killpattern ();
		    }

		    if (pat_key) {
			fflush (fpmc -> dmfp); fflush (fpbox -> dmfp);
			get_pattern (pat_name, edges1, (IMPCELL *)0);
			edgeptr = mostleft = mostright = NULL;
			if ($5) {
			    for (fmptr = edges1[1][src1_mask]; fmptr; fmptr = fmptr->rnext)
				sort_ins_edge (fmptr->x, fmptr->ybottom,
				    fmptr->ytop, -fmptr->bodybit);
			}
			else {
			    for (fmptr = edges1[1][src1_mask]; fmptr; fmptr = fmptr->rnext)
				sort_ins_edge (fmptr->x, fmptr->ybottom,
				    fmptr->ytop, fmptr->bodybit);
			}
			if ($8) {
			    for (fmptr = edges1[1][src2_mask]; fmptr; fmptr = fmptr->rnext)
				sort_ins_edge (fmptr->x, fmptr->ybottom,
				    fmptr->ytop, -fmptr->bodybit);
			}
			else {
			    for (fmptr = edges1[1][src2_mask]; fmptr; fmptr = fmptr->rnext)
				sort_ins_edge (fmptr->x, fmptr->ybottom,
				    fmptr->ytop, fmptr->bodybit);
			}

			if ($5 || $8 || $6 == 1)
			    scan_edges (1);
			else
			    scan_edges (2);

			if (src1_mask == dest_mask && $6 == 2) {

			    dmCloseStream (fpbox, COMPLETE);
			    dmCloseStream (fpmc, COMPLETE);
			    fpbox = dmOpenStream (pat_key, "box", "w");
			    fpmc = dmOpenStream (pat_key, "mc", "w");

			    clean_bb ();
			    for (fmptr = edges1[1][dest_mask]; fmptr; fmptr = fmptr->rnext)
			    {
				fmptr -> list = fr_edge_structs;
				fr_edge_structs = fmptr;
			    }
			    setarraytoptr (edges1, dest_mask);
			    mk_all_boxes_of_edges ();
			    edgeptr = mostleft = mostright = NULL;
			}
			else {
			    masknr = dest_mask;
			    mk_boxes_of_edges ();
			    for (edgeptr = mostleft; edgeptr; edgeptr = edgeptr->rnext)
				free_edge (edgeptr);
			}
			rm_array (edges1);
		    }
		}
		| GROW '(' MASKNAME ',' coord ')'
		{
		    masknr = getmaskname (words[$3]);
		    if (masknr < 0 || !layset[masknr]) {
			pr_err2 ("error: unknown mask:", words[$3]);
			killpattern ();
		    }

		    if (pat_key) {
			fflush (fpbox -> dmfp); fflush (fpmc -> dmfp);
			get_pattern (pat_name, edges1, (IMPCELL *)0);
			setedgeptrto (edges1, masknr);
			grow ($5);
			setarraytoptr (edges1, masknr);

			if ($5 < 0) {
			    dmCloseStream (fpbox, COMPLETE);
			    dmCloseStream (fpmc, COMPLETE);
			    fpbox = dmOpenStream (pat_key, "box", "w");
			    fpmc = dmOpenStream (pat_key, "mc", "w");

			    clean_bb ();
			    mk_all_boxes_of_edges ();
			}
			else
			    mk_boxes_of_edges ();

			rm_array (edges1);
			edgeptr = mostleft = mostright = NULL;
		    }
		}
		;

opt_not		: /* empty */ { $$ = 0; }
		| '(' NOT ')' { $$ = 1; }
		;
operator	: AND { $$ = 2; }
		| OR  { $$ = 1; }
		;
opt_grow	: /* empty */	{ $$ =  0; }
		| ',' coord	{ $$ = $2; }
		;

masksdef	: MASKS
		{
		    for (masknr = 0; masknr < nrofmasks; ++masknr) {
			layset[masknr] = 0; /* off */
		    }
		    for (listptr = mlistnames; listptr; ) {
			newlistptr = listptr -> next;
			FREE (listptr);
			listptr = newlistptr;
		    }
		    mlistnames = NULL;
		}
		  mask_names
		;
mask_names	: maskname
		| mask_names ',' maskname
		;
maskname	: MASKNAME
		{
		    masknr = getmaskname (words[$1]);
		    if (masknr < 0) {
			pr_err2 ("error: unknown mask:", words[$1]);
		    }
		    else {
			layset[masknr] = 1; /* on */
		    }
		}
		| error
		{
		    pr_err2 ("error: illegal mask name:", textval());
		}
		;

mlistsdef	: MASKLIST ml_defs
		;
ml_defs		: ml_def
		| ml_defs ',' ml_def
		;
ml_def		: MASKNAME
		{
		    mlistdef = 1;
		    newlistptr = NULL;
		    masknr = getmaskname (words[$1]);
		    if (masknr >= 0 && layset[masknr]) {
			pr_err ("error:", words[$1],
			    "already used as mask name");
		    }
		    else if (getlistname (words[$1])) {
			pr_err ("error:", words[$1],
			    "already used as masklist name");
		    }
		    else {
			ALLOC_STRUCT (newlistptr, list);
			strcpy (newlistptr->name, words[$1]);
			newlistptr->next = NULL;
			if (!mlistnames) {
			    mlistnames = newlistptr;
			}
			else {
			    endlistptr->next = newlistptr;
			}
			endlistptr = newlistptr;
		    }
		}
		  '(' masklist ')'
		{
		    if (newlistptr) {
			for (masknr = 0; masknr < nroflistmasks; ++masknr) {
			    newlistptr->element[masknr][0] = mlist[masknr][0];
			    newlistptr->element[masknr][1] = mlist[masknr][1];
			}
			newlistptr->nrofelements = nroflistmasks;
		    }
		    mlistdef = 0;
		}
		| error
		{
		    mlistdef = 0;
		}
		;

masklist	: MASKNAME
		{
		    mlistflag = 0;
		    mlist[0][0] = -1;
		    mlist[0][1] = 0;
		    nroflistmasks = 1;

		    masknr = getmaskname (words[$1]);
		    if (masknr >= 0 && layset[masknr]) {
			mlist[0][0] = masknr;
		    }
		    else {
			if (!mlistdef && (listptr = getlistname (words[$1]))) {
			    ++mlistflag;
			    nroflistmasks = listptr->nrofelements;
			    for (masknr = 0; masknr < nroflistmasks; ++masknr) {
				mlist[masknr][0] = listptr->element[masknr][0];
				mlist[masknr][1] = listptr->element[masknr][1];
				if (mlist[masknr][0] < 0) {
				    pr_err2 ("error: unknown mask in masklist:", words[$1]);
				    killpattern ();
				    break;
				}
			    }
			}
			else {
			    pr_err2 ("error: unknown mask:", words[$1]);
			    if (!mlistdef) killpattern ();
			}
		    }
		}
		| masklist ',' MASKNAME ',' coord
		{
		    if (mlistflag) {
			pr_err2 ("error: masklist should only contain one",
			    "masklist name or one or more mask names");
			killpattern ();
		    }
		    else {
			mlist[nroflistmasks][0] = -1;
			mlist[nroflistmasks][1] = $5;
			masknr = getmaskname (words[$3]);
			if (masknr >= 0 && layset[masknr]) {
			    mlist[nroflistmasks][0] = masknr;
			}
			else {
			    pr_err2 ("error: unknown mask:", words[$3]);
			    if (!mlistdef) killpattern ();
			}
			++nroflistmasks;
		    }
		}
		;

commands	: MASKEY
		{
		    P_E "== basic mask list ==\n nr: ");
		    for (masknr = -1; ++masknr < nrofmasks;)
			P_E "%-4d", masknr);
		    P_E "\ncmk: ");
		    for (masknr = -1; ++masknr < nrofmasks;)
			P_E "%-4s", cmklay[masknr]);
		    P_E "\nldm: ");
		    for (masknr = -1; ++masknr < nrofmasks;)
			P_E "%-4s", ldmlay[masknr]);
		    P_E "\nset: ");
		    for (masknr = -1; ++masknr < nrofmasks;)
			P_E "%-4d", layset[masknr]);
		    P_E "\n\n");

		    for (listptr = mlistnames; listptr;) {
			P_E "== masklist: %s ==\nmsknr:", listptr->name);
			for (masknr = -1; ++masknr < listptr->nrofelements;)
			    P_E "%4d", listptr->element[masknr][0]);
			P_E "\ndelta:");
			for (masknr = -1; ++masknr < listptr->nrofelements;)
			    P_E "%4d", listptr->element[masknr][1]);
			P_E "\n\n");
			listptr = listptr -> next;
		    }
		}
		;

tplacement	:
		{
		    t_nr = 0;
		    mirrorflag = rotateflag = 0;
		    x_origin = y_origin = 0;
		    cxnumber = cynumber = 0;
		}
		  tplac_ment
		;
placement	:
		{
		    mirrorflag = rotateflag = 0;
		    x_origin = y_origin = 0;
		    cxnumber = cynumber = 0;
		}
		  plac_ment
		;

tplac_ment	: t_item
		| t_item ',' p_rest1
		| p_rest1
		| /* empty */
		| error
		{
		    pr_err1 ("error: illegal placement statement");
		    killpattern ();
		}
		;
plac_ment	: mirror
		| mirror ',' p_rest2
		| p_rest2
		| /* empty */
		| error
		{
		    pr_err1 ("error: illegal placement statement");
		    killpattern ();
		}
		;
p_rest1		: mirror
		| mirror ',' p_rest2
		| p_rest2
		;
p_rest2		: rotate
		| rotate ',' p_rest3
		| p_rest3
		;
p_rest3		: origin
		| origin ',' p_rest4
		| p_rest4
		;
p_rest4		: copyx
		| copyx ',' copyy
		| copyy
		;

t_item		: T1 { t_nr = 1; }
		| T2 { t_nr = 2; }
		| T3 { t_nr = 3; }
		;
mirror		: MX { mirrorflag = 1; }
		| MY { mirrorflag = -1; }
		;
rotate		: ROTATE integer
		{
		    rotateflag = $2;

		    switch (rotateflag) {
		    case   0: break;
		    case  90: break;
		    case 180: break;
		    case 270: break;
		    default:
			pr_err1i ("error: illegal rotation angle:", rotateflag);
			killpattern ();
		    }
		}
		| ROT0    { rotateflag =   0; }
		| ROT90   { rotateflag =  90; }
		| ROT180  { rotateflag = 180; }
		| ROT270  { rotateflag = 270; }
		;
origin		: coord ',' coord
		{
		    x_origin = $1; y_origin = $3;
		}
		;
copyx		: CX coord ',' integer
		{
		    cxdistance = $2; cxnumber = $4;
		    if (cxnumber <= 0) {
			if (cxnumber < 0) {
			    pr_err1 ("error: illegal number of x-copies");
			    killpattern ();
			}
			else
			    pr_err1 ("warning: number of x-copies is 0");
		    }
		}
		;
copyy		: CY coord ',' integer
		{
		    cydistance = $2; cynumber = $4;
		    if (cynumber <= 0) {
			if (cynumber < 0) {
			    pr_err1 ("error: illegal number of y-copies");
			    killpattern ();
			}
			else
			    pr_err1 ("warning: number of y-copies is 0");
		    }
		}
		;

angle		: INTEGER
		{
		    if (yylval < 0 || yylval > 360) {
			pr_err1i ("error: cpeel: illegal angle:", yylval);
			killpattern ();
		    }
		    $$ = 10 * yylval;
		}
		| FLOAT
		{
		    if (d_f < 0 || d_f > 360) {
			pr_err1f ("error: cpeel: illegal angle:", d_f);
			killpattern ();
		    }
		    d_f = 10 * d_f;
		    $$ = ROUND (d_f);
		    d_i = $$;
		    d_i = d_i - d_f;
		    if (d_i > 0.0001 || d_i < -0.0001) {
			pr_err ("warning: cpeel: angle", d2a (d_f),
				    "not of correct resolution");
			d_i = $$;
			pr_err2 ("warning: value rounded to:", d2a (d_i));
		    }
		}
		;
coord		: INTEGER
		{
		    if (int_res) {
			$$ = int_res * yylval;
		    }
		    else {
			d_f = yylval / resol;
			$$ = ROUND (d_f);
			d_i = $$;
			d_i = d_i - d_f;
			if (d_i > 0.0001 || d_i < -0.0001) {
			    pr_err ("warning: parameter", d2a (d_f),
				    "not of correct resolution");
			    d_i = $$;
			    pr_err2 ("warning: value rounded to:", d2a (d_i));
			}
		    }
		}
		| FLOAT
		{
		    d_f = d_f / resol;
		    $$ = ROUND (d_f);
		    d_i = $$;
		    d_i = d_i - d_f;
		    if (d_i > 0.0001 || d_i < -0.0001) {
			pr_err ("warning: parameter", d2a (d_f),
				    "not of correct resolution");
			d_i = $$;
			pr_err2 ("warning: value rounded to:", d2a (d_i));
		    }
		}
		;
integer		: INTEGER
		{
		    $$ = yylval;
		}
		| FLOAT
		{
		    pr_err1f ("error: parameter not integer:", d_f);
		    killpattern ();
		}
		;

patname		: PATNAME
		{
		    $$ = $1;
		    for (cp = words[$1]; *cp != '\0'; ++cp) {
			if (*cp == '-') *cp = '_';
		    }

		    if (cp - words[$1] > DM_MAXNAME) {
			words[$1][DM_MAXNAME] = '\0';
			pr_err2 ("warning: pattern name truncated to",
			    mpn (words[$1]));
		    }
		}
		;
eol		: EOL
		{
		  ++yylineno;
		  wordsindex = 0;
		}
		;
%%

void upd_path_coords (int new_mode)
{
    if (p_nr < 2) {
	pr_err1 ("syntax error: path: missing coordinates");
	killpattern ();
	p_mode = 0;
	return;
    }

    if (p_mode == 3) { /* test only */
	if (p_x[p_incr] != p_arr[1] || p_y[p_incr] != p_arr[2]) {
	    pr_err1 ("error: path: lastpoint not equal to testpoint");
	    P_E "%s: path: lastpoint is (%s",
		    argv0, d2a (p_x[p_incr]));
	    P_E ",%s) ", d2a (p_y[p_incr]));
	    P_E "testpoint is (%s", d2a (p_arr[1]));
	    P_E ",%s)\n", d2a (p_arr[2]));
	    killpattern ();
	    p_mode = 0;
	    return;
	}
    }
    else {
	if (++p_incr > 50) {
	    pr_err1 ("error: path: more than 50 edge points");
	    killpattern ();
	    p_mode = 0;
	    return;
	}
	if (p_incr == 1) {
	    if (p_nr < 3) {
		pr_err1 ("syntax error: path: missing width");
		killpattern ();
		p_mode = 0;
		return;
	    }
	    p_width = p_arr[3];
	}

	switch (p_nr) {
	case 4:
	    if (p_arr[4] != p_width) {
		pr_err1 ("path: width change not implemented");
		killpattern ();
		p_mode = 0;
		return;
	    }
	case 3:
	    if (p_arr[3] != p_width) {
		pr_err1 ("path: width change not implemented");
		killpattern ();
		p_mode = 0;
		return;
	    }
	case 2:
	    if (p_mode == 1) { /* abs */
		p_x[p_incr] = p_arr[1];
		p_y[p_incr] = p_arr[2];
	    }
	    else { /* rel */
		p_x[p_incr] = p_x[p_incr-1] + p_arr[1];
		p_y[p_incr] = p_y[p_incr-1] + p_arr[2];
	    }
	}
    }
    p_nr = 0;
    p_mode = new_mode;
}

int savename (char *s)
{
    char *w;
    register int i;

    if (wordsindex >= MAXWORDS) {
	pr_err1 ("fatal error: word buffer overflow");
	die ();
    }
    w = words[wordsindex];
    i = 0;
    while (*s && i < DM_MAXNAME) w[i++] = *s++;
    w[i] = 0;
    return (wordsindex++);
}

void yyerror (char *s)
{
    P_E "%s: line %d: %s\n", argv0, yylineno, s);
}

void pr_err1 (char *s1)
{
    P_E "%s: line %d: %s\n", argv0, yylineno, s1);
}

void pr_err1i (char *s1, int i)
{
    P_E "%s: line %d: %s %d\n", argv0, yylineno, s1, i);
}

void pr_err1f (char *s1, double d)
{
    P_E "%s: line %d: %s %g\n", argv0, yylineno, s1, d);
}

void pr_err2 (char *s1, char *s2)
{
    P_E "%s: line %d: %s %s\n", argv0, yylineno, s1, s2);
}

void pr_err (char *s1, char *s2, char *s3)
{
    P_E "%s: line %d: %s %s %s\n", argv0, yylineno, s1, s2, s3);
}

int yywrap ()
{
    return (1);
}
