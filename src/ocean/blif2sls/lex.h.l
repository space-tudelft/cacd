%{
/*
 * ISC License
 *
 * Copyright (C) 1992-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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

extern int input(void);
//int blif2slslineno;

#ifdef __cplusplus
#define input() blif2slsinput()
#endif

%}
%option yylineno
%p 2500
%%
".model"                  { return(MODEL_TOKEN); }
".inputs"                 { return(INPUTS_TOKEN); }
".outputs"                { return(OUTPUTS_TOKEN);}
".area"                   { return(AREA_TOKEN);}
".delay"                  { return(DELAY_TOKEN);}
".wire_load_slope"        { return(WIRE_LOAD_SLOPE_TOKEN); }
".wire"                   { return(WIRE_TOKEN); }
".input_arrival"          { return(INPUT_ARRIVAL_TOKEN); }
".default_input_arrival"  { return(DEFAULT_INPUT_ARRIVAL_TOKEN); }
".output_required"        { return(OUTPUT_REQUIRED_TOKEN); }
".default_output_required" { return(DEFAULT_OUTPUT_REQUIRED_TOKEN); }
".input_drive"            { return(INPUT_DRIVE_TOKEN); }
".default_input_drive"    { return(DEFAULT_INPUT_DRIVE_TOKEN); }
".output_load"            { return(OUTPUT_LOAD_TOKEN);}
".default_output_load"    { return(DEFAULT_OUTPUT_LOAD_TOKEN);}
".default_max_input_load" { return(DEFAULT_MAX_INPUT_LOAD_TOKEN) ;}
".gate"                   { return(GATE_TOKEN);}
".mlatch"                 { return(MLATCH_TOKEN);}
".clock"                  { return(CLOCK_TOKEN);}
".names"                  { return(NAMES_TOKEN);}
".exdc"                   { return(EXDC_TOKEN);}
".subckt"                 { return(SUBCKT_TOKEN);}
".latch"                  { return(LATCH_TOKEN);}
".cycle"                  { return(CYCLE_TOKEN);}
".code"                   { return(CODE_TOKEN);}
".latch_order"            { return(LATCH_ORDER_TOKEN);}
".clock_event"            { return(CLOCK_EVENT_TOKEN);}
".end_kiss"               { return(END_KISS_TOKEN);}
".end"                    { return(END_TOKEN);}
"#"			{ /* comment skipper */
			int c;
			while ((c = input()) != '\n');
			blif2slslineno++; /* for flex: must count line numbers */
			return (EOL_TOKEN);
			}
"\\"			{ /* new line skipper */
			int c;
			while ((c = input()) == ' '); /* skip all */
			if (c != '\n') cerr << "WARNING (blif): syntax error (line "
					<< blif2slslineno << " )\n" << endl;
			blif2slslineno++; /* for flex: must count line numbers */
			}
\"			{
			int c, i = 0;
			while ((c = input()) != '"') {
			    if (c == '\n') {
				cerr << "WARNING (blif): newline in string (line "
					<< blif2slslineno << " )\n" << endl;
			    } else {
				if (c == '\\') c = input(); /* read next character */
				blif2slslval.str[i++] = c;
			    }
			}
			blif2slslval.str[i] = '\0';
			return(STRING_TOKEN);
			}
[A-Za-z][A-Za-z0-9_/.]*	{
			strcpy(blif2slslval.str,(char*)blif2slstext);
			return(STRING_TOKEN);
			}
"["[A-Za-z0-9_/.]*"]"	{
			strcpy(blif2slslval.str,(char*)blif2slstext);
			return(STRING_TOKEN);
			}
			/* sometimes you might get name[0:2] */
[A-Za-z][A-Za-z0-9_/.]*"["[A-Za-z0-9_/.:]*"]"	{
			strcpy(blif2slslval.str,(char*)blif2slstext);
			return(STRING_TOKEN);
			}
"%"[0-9][A-Za-z0-9_/.]*	{ /* escaped string which starts with percent */
			strcpy(blif2slslval.str,(char*)blif2slstext + 1);
			return(STRING_TOKEN);
			}
"-"?[0-9]+		{
			strcpy(blif2slslval.str,(char*)blif2slstext);
			return(STRING_TOKEN);
			}
"-"?[0-9]*"."[0-9]+	{
			strcpy(blif2slslval.str,(char*)blif2slstext);
			return(NUMBER);
			}
[\t]			;
"="			return(ASSIGN_TOKEN);
[\n]			++blif2slslineno; return(EOL_TOKEN);
.			;

