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

#include "src/nspice/define.h"
#include "src/nspice/type.h"
#include "src/nspice/extern.h"

extern char *fn_cmd;
extern FILE *yyin;
extern double sigunit;
extern int simperiod;
extern int yylineno;

long Endtime;
double Timescaling;

static unsigned long curr_time;
static int Steps;
static struct sig_value *end_value;

#define MAXTIME 2000000000

int addSgnPart (SIGNALELEMENT *expr, int nr);
void cmdinits (void);
int yyparse (void);

void readCom (char *filename)
{
    struct node *sig;

    if (!(yyin = fopen (filename, "r"))) {
	message ("Cannot open %s", filename, 0);
	exit (1);
    }

    fn_cmd = filename;
    cmdinits ();
    yylineno = 1;
    yyparse ();

    if ((Timescaling = sigunit) <= 0) {
	message ("missing/incorrect '%s' in %s", "option sigunit", filename);
	Timescaling = 1;
	message ("warning: using default sigunit of 1 second", 0, 0);
    }

    Endtime = simperiod > 0 ? simperiod : 0;
    if (Endtime > MAXTIME) {
	char buf[20];
	sprintf (buf, "%ld", (Endtime = MAXTIME));
	message ("warning: very long simperiod (max = %s)", buf, 0);
    }

    for (sig = Begin_signal = Begin_node; sig; sig = sig -> next) {
	NEW (end_value, 1, struct sig_value);
	end_value -> time  = 0;
	end_value -> value = 1;
	sig -> begin_value = end_value;

        curr_time = 0;
	Steps = 0;
        addSgnPart (sig -> expr, 1);
	end_value -> next = NULL;
    }
}

int addSgnPart (SIGNALELEMENT *expr, int nr)
{
    struct sig_value *sval;
    SIGNALELEMENT *sibling_expr;

    while (nr > 0 || nr == -1) {

	for (sibling_expr = expr -> sibling; sibling_expr; sibling_expr = sibling_expr -> sibling) {

	    if (sibling_expr -> child) {
		int newnr = -1;
		if (sibling_expr -> len > 0 && sibling_expr -> child -> len > 0)
		    newnr = sibling_expr -> len / sibling_expr -> child -> len;

		if (addSgnPart (sibling_expr -> child, newnr)) return (1);
	    }
	    else {
		int value;
		     if (sibling_expr -> val == H_state) value = 2;
		else if (sibling_expr -> val == L_state) value = 0;
		else if (sibling_expr -> val == X_state) value = 1;
		else if (sibling_expr -> val == Free_state) value = -1;
		else { fprintf (stderr, "He ?\n"); value = 1; }

		if (curr_time == 0) end_value -> value = value;
		else if (end_value -> value != value) {
		    if (++Steps > 1000000) {
			message ("Sorry, more than 1000000 time steps!", 0, 0);
			message ("Please, choice a (shorter) simperiod!", 0, 0);
			exit (1);
		    }
		    NEW (sval, 1, struct sig_value);
		    sval -> time = curr_time;
		    sval -> value = value;
		    end_value -> next = sval;
		    end_value = sval;
		}

                if (sibling_expr -> len > 0) {
		    curr_time += sibling_expr -> len;
		}
		else return (1); /* infinity, stop */

		if (simperiod < 0 && curr_time > Endtime) {
		    if (curr_time > MAXTIME) {
			if (Endtime < MAXTIME) {
			    char buf[20];
			    sprintf (buf, "%ld", (Endtime = MAXTIME));
			    message ("warning: very long simperiod (max = %s)", buf, 0);
			}
			return (1);
		    }
		    Endtime = curr_time;
		}
		if (simperiod > 0 && curr_time > Endtime) return (1); /* end reached */
	    }
	}

	if (nr != -1) nr--;
    }

    return (0);
}
