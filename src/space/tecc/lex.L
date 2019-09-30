%{
/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include "src/space/libmin/minrobdd.h"
#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"
#include "parse.h"

#ifdef FLEX_SCANNER /* if this is GNU flex in stead of regular lex */
#define YYINCR(_count) yylineno += _count /* flex does not do this for us */
#else
#define YYINCR(_x)
#endif
%}

%option yylineno

E		[DEde][-+]?[0-9]+

%%
"#"			{
#if defined(FLEX_SCANNER) && defined(__cplusplus)
                          while (yyinput () != '\n');
#else
                          while (input () != '\n');
#endif
                          YYINCR(1);
			}
"-"			{ return (MINUS); }
"="			{ return (EQUAL); }
"!"			{ return (NOT); }
"("			{ return (LPS); }
")"			{ return (RPS); }
":"			{ return (COLON); }
"|"			{ return (OR); }
"%"			{ return (PERC); }
"/"			{ return (SLASH); }
"@"[0-9a-fA-F]+		{ yylval.sval = yytext+1; return (HEX_NAME); }
"@sub"                  { return (SUB); }
"@gnd"                  { return (GND); }
"keys"			{ return (KEYS); }
"maxkeys"		{ return (MAXKEYS); }
"bem_depth"		{ return (BEM_DEPTH); }
"conductors"		{ return (CONDUCTORS); }
"transistors"		{ return (TRANSISTORS); }
"contacts"		{ return (CONTACTS); }
"capacitances"		{ return (CAPACITANCES); }
"vdimensions"		{ return (VDIMENSIONS); }
"restype"		{ return (RESTYPE); }
"subconn"		{ return (SUBCONN); }
"viamask"		{ return (VIAMASK); }
"wafer"			{ return (WAFER); }
"eshapes"		{ return (ESHAPES); }
"cshapes"		{ return (CSHAPES); }
"unit"			{ return (UNIT); }
"resistance"		{ return (RESISTANCE); }
"c_resistance"		{ return (C_RESISTANCE); }
"s_resistance"		{ return (S_RESISTANCE); }
"capacitance"		{ return (CAPACITANCE); }
"a_capacitance"		{ return (A_CAPACITANCE); }
"e_capacitance"		{ return (E_CAPACITANCE); }
"distance"		{ return (DISTANCE); }
"vdimension"		{ return (VDIMENSION); }
"omit_cap3d"		{ return (OMIT_CAP3D); }
"keep_cap2d"		{ return (KEEP_CAP2D); }
"shape"			{ return (SHAPE); }
"bjtors"		{ return (BIPOTORS); }
"junction"		{ return (JUNCTION); }
"junctions"		{ return (JUNCTIONS); }
"connects"		{ return (CONNECTS); }
"layer_depth"		{ return (LAYERDEPTH); }
"new"			{ return (NEWMASK); }
"resize"		{ return (RESIZEMASK); }
"fets"			{ return (FETS); }
"bjts"			{ return (BJTS); }
"sublayers"             { return (SUBLAYERS); }
"subcaplayers"          { return (SUBCAPLAYERS); }
"dielectrics"           { return (DIELECTRICS); }
"zq_values"             { return (ZQ_VALUES); }
"zp_values"             { return (ZP_VALUES); }
"grid_count"            { return (GRID_COUNT); }
"neumann_simulation_ratio" { return (NEUMANN_SIMULATION_RATIO); }
"num_annealing_iterations" { return (NUM_ANNEALING_ITERATIONS); }
"max_determinant_binning_error" { return (MAX_DETERMINANT_BINNING_ERROR); }
"max_adjoint_binning_error"     { return (MAX_ADJOINT_BINNING_ERROR); }
"max_annealed_inverse_matrix_binning_error" { return (MAX_ANNEALED_INVERSE_MATRIX_BINNING_ERROR); }
"max_preprocessed_annealing_matrices_binning_error" { return (MAX_PREPROCESSED_ANNEALING_MATRICES_BINNING_ERROR); }
"max_reduce_error" { return (MAX_REDUCE_ERROR); }
"max_annealing_error" { return (MAX_ANNEALING_ERROR); }
"r_switch"              { return (R_SWITCH); }
"r_values"              { return (R_VALUES); }
"selfsubres"            { return (SELFSUBRES); }
"set"			{ return (SET); }
"specific_resistance"	{ return (SPECIFIC_RES); }
"coupsubres"            { return (COUPSUBRES); }
"colors"                { return (COLORS); }
[a-zA-Z_][a-zA-Z0-9_]*	{ yylval.sval = yytext; return (IDENTIFIER); }
("-")?[0-9]+		{ yylval.ival = atoi (yytext); return (INTEGER); }
("-")?[0-9]+("."[0-9]+)?({E})?[fpnumkMG]?	{
			char c = yytext[ yyleng - 1 ];
			if (c >= '0' && c <= '9')
			    yylval.dval = atof (yytext);
			else {
			    yytext[ yyleng - 1 ] = '\0';
			    yylval.dval = atof (yytext);
			    switch (c) {
			 	case 'G': yylval.dval *= 1e9; break;
			 	case 'M': yylval.dval *= 1e6; break;
			 	case 'k': yylval.dval *= 1e3; break;
			 	case 'm': yylval.dval *= 1e-3; break;
			 	case 'u': yylval.dval *= 1e-6; break;
			 	case 'n': yylval.dval *= 1e-9; break;
			 	case 'p': yylval.dval *= 1e-12; break;
			 	case 'f': yylval.dval *= 1e-15; break;
			    }
			}
			  return (DOUBLE);
			}
[\t \r]+		;
[\n]+			{ YYINCR (yyleng); }
.			{ return (ILLEGAL); }

%%

int yywrap ()
{
    return (1);
}

void yyerror (const char *s)
{
    char mesbuf[32];
    sprintf (mesbuf, "at line %d\n", yylineno);
    fatalErr ((char *)s, mesbuf);
}
