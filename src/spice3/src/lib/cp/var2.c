/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Do variable substitution.
 */

#include "spice.h"
#include "util.h"
#include "cpdefs.h"

#include <unistd.h> /* getpid */

static int vcmp();

extern struct variable *variables;

/* A variable substitution is
 * indicated by a $, and the variable name is the following string of
 * non-special characters. All variable values are inserted as a single
 * word, except for lists, which are a list of words.
 * A routine cp_usrset must be supplied by the host program to deal
 * with variables that aren't used by cshpar -- it should be
 * cp_usrset(var, isset), where var is a variable *, and isset is
 * true if the variable is being set, false if unset.
 * Also required is a routine cp_enqvar(name) which returns a struct
 * variable *, which allows the host program to provide values for
 * non-cshpar variables.
 */

wordlist * cp_variablesubst (wordlist *wlist)
{
    wordlist *wl, *nwl;
    char *s, *t, buf[BSIZE_SP];
    int i, c;

    for (wl = wlist; wl; wl = wl->wl_next) {
	t = wl->wl_word;
        while (t && (s = strchr(t, '$'))) {
	    t = s+1;
	    if (!*t || isspace(*t)) continue;
            *s++ = '\0';
	    if (*t == '{') { /* ${var} */
		s++; t++;
		while (*t && *t != '}') t++;
		if (*t) *t++ = '\0';
		else fprintf(cp_err, "Error: missing '}'.\n");
	    }
	    else {
		if (*t == '?' || *t == '#' || *t == '%') t++;
		if (isdigit(*t)) { /* $1, $2, ... */
		    while (isdigit(*t)) t++;
		}
		else {
		    if (*t == '&') { t++; /* $&var, $&@var */
			if (*t == '@') t++;
			c = '.';
		    }
		    else c = '_';
		    if (isalpha(*t) || *t == '_') { /* $var, $var[range] */
			while (isalnum(*t) || *t == '_' || *t == c) t++;
			while (*t && *t == '[') {
			    while (*t && *t != ']') t++;
			    if (*t) t++;
			    else fprintf(cp_err, "Error: missing ']'.\n");
			}
			if (c == '.')
			while (*t && *t == '(') {
			    while (*t && *t != ')') t++;
			    if (*t) t++;
			    else fprintf(cp_err, "Error: missing ')'.\n");
			}
		    }
		    if (s == t) t++;
		}
	    }
            c = *t;
            *t = '\0';
            nwl = vareval(s);
            *t = c;
	    if (!c) t = NULL;

	    s = wl->wl_word;
	    if (nwl) {
		if (*s) {
		    strcpy(buf, s);
		    strcat(buf, nwl->wl_word);
		    txfree(nwl->wl_word);
		    nwl->wl_word = copy(buf);
		}
		if (wlist == wl) wlist = nwl;
		wl->wl_word = NULL; /* SdG, don't free s, because of t */
		wl = wl_splice(wl, nwl);
	    }
	    if (t) {
		strcpy(buf, wl->wl_word); i = strlen(buf);
		strcat(buf, t);
		txfree(wl->wl_word);
		wl->wl_word = copy(buf);
		t = wl->wl_word + i;
	    }
	    if (nwl) txfree(s);
        }
    }
    return (wlist);
}

/* Evaluate a variable. */

wordlist * vareval (char *string)
{
    struct variable *v;
    wordlist *wl;
    char buf[BSIZE_SP], *s, *range;
    int i, up, low;

    cp_wstrip(string);
    if ((range = strchr(string, '['))) *range++ = '\0';

    switch (*string) {
    case '$':
        wl = alloc(wordlist);
        wl->wl_next = wl->wl_prev = NULL;
        sprintf(buf, "%d", getpid());
        wl->wl_word = copy(buf);
        return (wl);

    case '<':
        fflush(cp_out);
        if (!fgets(buf, BSIZE_SP, cp_in)) {
            clearerr(cp_in);
            strcpy(buf, "EOF");
        }
        for (s = buf; *s && *s != '\n'; s++) ;
        *s = '\0';
        wl = cp_lexer(buf);
        /* This is a hack. */
        if (wl && !wl->wl_word) wl->wl_word = copy("");
        return (wl);

    case '?':
    case '#':
    case '%':
        string++;
        for (v = variables; v; v = v->va_next)
		if (eq(v->va_name, string)) break;
        if (!v) v = cp_enqvar(string);
	string--;
	i = 0;
        if (!v) {
	    if (*string != '?') {
		fprintf(cp_err, "Error: %s: no such variable.\n", string+1);
		return (NULL);
	    }
        } else if (*string == '#') {
	    if (v->va_type == VT_LIST)
		for (v = v->va_vlist; v; v = v->va_next) ++i;
	    else
		i = 1;
        } else if (*string == '%') {
	    i = strlen(string+1);
	} else i = 1;
        sprintf(buf, "%d", i);
        wl = alloc(wordlist);
        wl->wl_next = wl->wl_prev = NULL;
        wl->wl_word = copy(buf);
        return (wl);

    case '\0':
        wl = alloc(wordlist);
        wl->wl_next = wl->wl_prev = NULL;
        wl->wl_word = copy("$");
        return (wl);
    }

    /* The notation var[stuff] has two meanings...  If this is a real
     * variable, then the [] denotes range, but if this is a strange
     * (e.g, device parameter) variable, it could be anything...
     */
    for (v = variables; v; v = v->va_next) if (eq(v->va_name, string)) break;
    if (!v && isdigit(*string)) {
        for (v = variables; v; v = v->va_next) if (eq(v->va_name, "argv")) break;
	if (v) range = string;
    }
    if (!v) {
	if (range) { --range; *range = '['; range = NULL; }
        v = cp_enqvar(string);
    }
    if (!v && (s = getenv(string))) {
        wl = alloc(wordlist);
        wl->wl_next = wl->wl_prev = NULL;
        wl->wl_word = copy(s);
        return (wl);
    }
    if (!v) {
        fprintf(cp_err, "Error: %s: no such variable.\n", string);
        return (NULL);
    }
    wl = cp_varwl(v);

    /* Now parse and deal with 'range' ... */
    if (range && wl) {
        for (low = 0; isdigit(*range); range++)
            low = low * 10 + *range - '0';
        if ((*range == '-') && isdigit(range[1]))
            for (up = 0, range++; isdigit(*range); range++)
                up = up * 10 + *range - '0';
        else if (*range == '-')
            up = wl_length(wl);
        else
            up = low;
        up--, low--;
        wl = wl_range(wl, low, up);
    }
    return (wl);
}

/* Print the values of currently defined variables. */

struct xxx {
    struct variable *x_v;
    char x_char;
} ;

void cp_vprint()
{
    struct variable *v, *uv1, *uv2;
    wordlist *wl;
    int i, j;
    char *s;
    struct xxx *vars;

    cp_usrvars(&uv1, &uv2);

    i = 0;
    for (v = uv1; v; v = v->va_next) i++;
    for (v = uv2; v; v = v->va_next) i++;
    for (v = variables; v; v = v->va_next) i++;

    vars = allocn(struct xxx, i);

    i = 0;
    for (v = variables; v; v = v->va_next, i++) {
        vars[i].x_v = v;
        vars[i].x_char = ' ';
    }
    for (v = uv1; v; v = v->va_next, i++) {
        vars[i].x_v = v;
        vars[i].x_char = '*';
    }
    for (v = uv2; v; v = v->va_next, i++) {
        vars[i].x_v = v;
        vars[i].x_char = '+';
    }

    qsort(vars, i, sizeof (struct xxx), vcmp);

    out_init();

    for (j = 0; j < i; j++) {
        if (j && eq(vars[j].x_v->va_name, vars[j-1].x_v->va_name)) continue;
        v = vars[j].x_v;
        if (v->va_type == VT_BOOL) {
	    sprintf(out_pbuf, "%c %s\n", vars[j].x_char, v->va_name);
	    out_send(out_pbuf);
        } else {
	    sprintf(out_pbuf, "%c %-6s\t", vars[j].x_char, v->va_name);
	    out_send(out_pbuf);
            wl = vareval(v->va_name);
            s = wl_flatten(wl);
            wl_free(wl);
            if (v->va_type == VT_LIST) {
                out_send("( "); out_send(s); out_send(" )\n");
	    }
            else { out_send(s); out_send("\n"); }
            txfree(s);
        }
    }

    txfree(vars);
}

static int vcmp (struct xxx *v1, struct xxx *v2)
{
    int i;

    if ((i = strcmp(v1->x_v->va_name, v2->x_v->va_name)))
        return (i);
    else
        return (v1->x_char - v2->x_char);
}

/* The set command. Syntax is
 * set [opt ...] [opt = val ...]. Val may be a string, an int, a float,
 * or a list of the form (elt1 elt2 ...).
 */

void com_set (wordlist *wl)
{
    struct variable *vars;
    char *s;

    if (!wl) { cp_vprint(); return; }

    vars = cp_setparse(wl);

    /* This is sort of a hassle... */
    while (vars) {
        switch (vars->va_type) {
            case VT_BOOL:
		s = (char *) &vars->va_bool;
		break;
            case VT_NUM:
		s = (char *) &vars->va_num;
		break;
            case VT_REAL:
		s = (char *) &vars->va_real;
		break;
            case VT_STRING:
		s = vars->va_string;
		break;
            case VT_LIST:
		s = (char *) vars->va_vlist;
		break;
            default:
		s = NULL; /* may not happen */
        }
        cp_vset(vars->va_name, vars->va_type, s);
        vars = vars->va_next;
    }
}

void com_unset (wordlist *wl)
{
    struct variable *var, *nv;

    if (!wl) return;

    if (eq(wl->wl_word, "*")) {
        for (var = variables; var; var = nv) {
            nv = var->va_next;
            cp_remvar(var->va_name);
        }
        wl = wl->wl_next;
    }
    while (wl) {
        cp_remvar(wl->wl_word);
        wl = wl->wl_next;
    }
}

/* Shift a list variable, by default argv, one to the left (or more if a
 * second argument is given.
 */

void com_shift (wordlist *wl)
{
    struct variable *v, *vv;
    char *n = "argv";
    int num = 1;

    if (wl) {
        n = wl->wl_word;
        wl = wl->wl_next;
    }
    if (wl) num = scannum(wl->wl_word);

    for (v = variables; v; v = v->va_next)
        if (eq(v->va_name, n)) break;
    if (!v) {
        fprintf(cp_err, "Error: %s: no such variable\n", n);
        return;
    }
    if (v->va_type != VT_LIST) {
        fprintf(cp_err, "Error: %s not of type list\n", n);
        return;
    }
    for (vv = v->va_vlist; vv && (num > 0); num--)
        vv = vv->va_next;
    if (num) {
        fprintf(cp_err, "Error: variable %s not long enough\n", n);
        return;
    }

    v->va_vlist = vv;
}

/* Determine the value of a variable.  Fail if the variable is unset,
 * and if the type doesn't match, try and make it work...
 */
bool cp_getvar (char *name, int type, char *retval)
{
    struct variable *v;

    for (v = variables; v; v = v->va_next)
        if (eq(name, v->va_name)) break;
    if (!v) {
        if (type == VT_BOOL) *(bool *) retval = false;
        return (false);
    }
    if (v->va_type == type) {
        switch (type) {
            case VT_BOOL:
                *(bool *) retval = true;
                break;
            case VT_NUM: {
                int *i;
                i = (int *) retval;
                *i = v->va_num;
                break;
            }
            case VT_REAL: {
                double *d;
                d = (double *) retval;
                *d = v->va_real;
                break;
            }
            case VT_STRING: {
		/* Gotta be careful to have room. */
		strcpy(retval, v->va_string);
		break;
            }
            case VT_LIST: { /* Funny case... */
                struct variable **tv;
                tv = (struct variable **) retval;
                *tv = v->va_vlist;
                break;
            }
            default:
                fprintf(cp_err, "cp_getvar: Internal Error: bad var type %d\n", type);
                break;
        }
        return (true);
    } else {
        /* Try to coerce it.. */
        if (type == VT_NUM && v->va_type == VT_REAL) {
            int *i;
            i = (int *) retval;
            *i = (int) v->va_real;
            return (true);
        } else if (type == VT_REAL && v->va_type == VT_NUM) {
            double *d;
            d = (double *) retval;
            *d = (double) v->va_num;
            return (true);
        } else if (type == VT_STRING && v->va_type == VT_NUM) {
            sprintf(retval, "%d", v->va_num);
            return (true);
        } else if (type == VT_STRING && v->va_type == VT_REAL) {
            sprintf(retval, "%f", v->va_real);
            return (true);
        }
        return (false);
    }
}
