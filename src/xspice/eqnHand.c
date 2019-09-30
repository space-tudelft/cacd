
/*
 * ISC License
 *
 * Copyright (C) 1994-2013 by
 *	Frederik Beeftink
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

#include <math.h>
#include "src/xspice/incl.h"

extern char *errorNameTab[];
extern int errorName_cnt;
extern struct lib_model *lm_curr;

static double fvalExpr (double opA, double opB, int action)
{
    switch (action) {
	case '*': return (opB * opA);
	case '/': return (opB / opA);
	case '+': return (opB + opA);
	case '-': return (opB - opA);
	case '^': return (pow (opB, opA));
    }

    return (0.0);  /* for compiler */
}

static double convertOperand (char *operand, double geom[])
{
    char * s = operand;
    while (*++s) *s = toupper (*s);
    ++operand;
	 if (strcmp (operand, "AE") == 0) return (geom[AE]);
    else if (strcmp (operand, "PE") == 0) return (geom[PE]);
    else if (strcmp (operand, "WB") == 0) return (geom[WB]);
    else if (strcmp (operand, "W" ) == 0) return (geom[WI]);
    else if (strcmp (operand, "L" ) == 0) return (geom[LE]);
    else if (strcmp (operand, "V" ) == 0) return (geom[VL]);
    else {
	char bf[512];
	int i;

	--operand;
	sprintf (bf, "%s:%s", operand, lm_curr -> name);
	for (i = 0; i < errorName_cnt; ++i)
	    if (strcmp (bf, errorNameTab[i]) == 0) break;
	if (i == errorName_cnt && i < MAX_IN_TAB) {
	    errorNameTab[ errorName_cnt++ ] = strsave (bf);
	    P_E "Warning: %s: unknown parameter in model: %s\n", operand, lm_curr -> name);
	}
    }
    return (0.0);
}

char *fvalEqn (char *eqn, double geom[])
{
    int i, len;
    int j = -1;
    int oper;
    char operand[32];
    double opA, opB;
    static char val_eqn[32];
    char st_op[32];
    double st_val[32];
    int sp_op = -1;
    int sp_val = -1;

    if (!eqn || !*eqn) {
	strcpy (val_eqn, "0");
	return (val_eqn);
    }

    len = strlen (eqn);
    for (i = 0; i < len; i++) {
	if (eqn[i] != ' ' && eqn[i] != '\t') {
	    switch (eqn[i]) {

	    case '(':
		st_op[++sp_op] = eqn[i];
		break;

	    case '^':
	    case '*':
	    case '/':
		if (j >= 0) {
		    operand[++j] = '\0';
		    if (operand[0] == '$')
			st_val[++sp_val] = convertOperand (operand, geom);
		    else
			st_val[++sp_val] = atof (operand);
		    j = -1;

		    if (sp_op >= 0 && sp_val > 0
			&& (st_op[sp_op] == '*' || st_op[sp_op] == '/')) {
			opA = st_val[sp_val--];
			opB = st_val[sp_val--];
			oper = st_op[sp_op--];
			st_val[++sp_val] = fvalExpr (opA, opB, oper);
		    }
		}
		st_op[++sp_op] = eqn[i];
		break;

	    case '+':
	    case '-':
		if (((operand[j] == 'e' || operand[j] == 'E')
		     && operand[j - 1] >= '0'
		     && operand[j - 1] <= '9') || j < 0)
		    operand[++j] = eqn[i];
		else {
		    operand[++j] = '\0';
		    if (operand[0] == '$')
			st_val[++sp_val] = convertOperand (operand, geom);
		    else
			st_val[++sp_val] = atof (operand);
		    j = -1;

		    if (sp_op >= 0 && sp_val > 0
			&& (st_op[sp_op] == '*' || st_op[sp_op] == '/')) {
			opA = st_val[sp_val--];
			opB = st_val[sp_val--];
			oper = st_op[sp_op--];
			st_val[++sp_val] = fvalExpr (opA, opB, oper);
		    }
		    st_op[++sp_op] = eqn[i];
		}
		break;

	    case ')':
		if (j >= 0) {
		    operand[++j] = '\0';
		    if (operand[0] == '$')
			st_val[++sp_val] = convertOperand (operand, geom);
		    else
			st_val[++sp_val] = atof (operand);
		    j = -1;
		}
		while ((oper = st_op[sp_op--]) != '(') {
		    opA = st_val[sp_val--];
		    opB = st_val[sp_val--];
		    st_val[++sp_val] = fvalExpr (opA, opB, oper);
		}
		break;

	    default:
		operand[++j] = eqn[i];
	    }
	}
    }
    if (j >= 0) {
	operand[++j] = '\0';
	if (operand[0] == '$')
	    st_val[++sp_val] = convertOperand (operand, geom);
	else
	    st_val[++sp_val] = atof (operand);
	j = -1;
    }
    while (sp_op >= 0) {
	oper = st_op[sp_op--];
	opA = st_val[sp_val--];
	opB = st_val[sp_val--];
	st_val[++sp_val] = fvalExpr (opA, opB, oper);
    }
    sprintf (val_eqn, "%.6e", st_val[sp_val]);
    return (val_eqn);
}
