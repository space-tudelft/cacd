/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include "util.h"
#include "cpdefs.h"
/* XXXX */
#include "fteext.h"
#include "ftedefs.h"
/* XXXX */

bool cp_noglob = true;
bool cp_noclobber = false;
bool cp_ignoreeof = false;

struct variable *variables = NULL;

wordlist * cp_varwl (struct variable *var)
{
    wordlist *wl = NULL, *w, *wx = NULL;
    char buf[BSIZE_SP], *s;
    struct variable *vt;
    int i;

    switch(var->va_type) {
        case VT_BOOL: /* Can't ever be false. */
            sprintf(buf, "%s", var->va_bool ? "true" : "false");
            break;
        case VT_NUM:
            sprintf(buf, "%d", var->va_num);
            break;
        case VT_REAL: /* This is a case where printnum isn't too good... */
            sprintf(buf, "%G", var->va_real);
            break;
        case VT_STRING:
	    s = var->va_string;
	    if (*s == '"') s++; /* cp_unquote */
	    strcpy(buf, s);
	    if ((i = strlen(buf)) && buf[i-1] == '"') buf[i-1] = '\0';
            break;
        case VT_LIST: /* The tricky case. */
            for (vt = var->va_vlist; vt; vt = vt->va_next) {
                w = cp_varwl(vt);
		if (w) {
		    if (!wl) wl = wx = w;
		    else {
			while (wx->wl_next) wx = wx->wl_next;
			wx->wl_next = w;
			w->wl_prev = wx;
			wx = w;
		    }
		}
            }
            return (wl);
        default:
            fprintf(cp_err, "cp_varwl: Internal Error: bad var type %d\n", var->va_type);
            return (wl); /* NULL */
    }
    wl = alloc(wordlist);
    wl->wl_next = wl->wl_prev = NULL;
    wl->wl_word = copy(buf);
    return (wl);
}

/* Set a variable. */

void cp_vset (char *varname, int type, char *value)
{
    struct variable *v, *u, *w;
    char buf[BSIZE_SP];
    int i;
    bool alreadythere = false;

    /* cp_unquote(varname) */
    if (*varname == '"') varname++;
    varname = strcpy(buf, varname);
    if ((i = strlen(buf)) && buf[i-1] == '"') buf[i-1] = '\0';

    w = NULL;
    for (v = variables; v; v = v->va_next) {
        if (eq(varname, v->va_name)) { alreadythere = true; break; }
	w = v;
    }
    if (!v) {
        v = alloc(struct variable);
        v->va_name = copy(varname);
        v->va_next = NULL;
    }
    switch (type) {
    case VT_BOOL:
        if (*((bool *) value) == false) { cp_remvar(varname); return; }
	v->va_bool = true;
        break;
    case VT_NUM:
        v->va_num = *(int *) value;
	if (eq(varname, "history")) { cp_maxhistlength = v->va_num;
	    if (cp_maxhistlength < 4) cp_maxhistlength = 4;
	    if (cp_maxhistlength > 1000) cp_maxhistlength = 1000;
	}
        break;
    case VT_REAL:
        v->va_real = *(double *) value;
	if (eq(varname, "history")) { cp_maxhistlength = v->va_real;
	    if (cp_maxhistlength < 4) cp_maxhistlength = 4;
	    if (cp_maxhistlength > 1000) cp_maxhistlength = 1000;
	    v->va_real = cp_maxhistlength;
	}
        break;
    case VT_STRING:
        v->va_string = copy(value);
	if (eq(varname, "prompt")) cp_promptstring = v->va_string;
        break;
    case VT_LIST:
        v->va_vlist = (struct variable *) value;
        break;
    default:
        fprintf(cp_err, "cp_vset: Internal Error: bad var type %d\n", type);
        return;
    }
    v->va_type = type;

    /* Now, see if there is anything interesting going on. We recognise
     * these special variables: noglob, nonomatch, history, echo,
     * noclobber, prompt, and verbose. cp_remvar looks for these variables
     * too. The host program will get any others.
     */
    if (eq(varname, "noglob"))
        cp_noglob = true;
    else if (eq(varname, "noclobber"))
        cp_noclobber = true;
    else if (eq(varname, "ignoreeof"))
        cp_ignoreeof = true;
    else if (eq(varname, "cpdebug")) {
        cp_debug = true;
#ifndef CPDEBUG
        fprintf(cp_err, "Warning: program not compiled with cshpar debug messages\n");
#endif
    }

    switch (i = cp_usrset(v, true)) {
    case US_OK: /* Normal case. */
        if (!alreadythere) {
            v->va_next = variables;
            variables = v;
        }
        break;
    case US_DONTRECORD: /* Do nothing... */
        if (alreadythere)
            fprintf(cp_err, "cp_vset: Internal Error: %s already there, but 'dont record'\n", v->va_name);
        break;
    case US_READONLY:
        fprintf(cp_err, "Error: %s is a read-only variable.\n", v->va_name);
        if (alreadythere)
            fprintf(cp_err, "cp_vset: Internal Error: it was already there too!\n");
        break;
    case US_SIMVAR:
	if (alreadythere) {
	    /* somehow it got into the front-end list of variables */
	    if (w) w->va_next = v->va_next;
	    else variables = v->va_next;
	}
	alreadythere = false;
	if (ft_curckt) {
	    for (u = ft_curckt->ci_vars; u; u = u->va_next)
		if (eq(varname, u->va_name)) {
		    alreadythere = true;
		    break;
		}
	    if (!alreadythere) {
		v->va_next = ft_curckt->ci_vars;
		ft_curckt->ci_vars = v;
	    } else {
		w = u->va_next;
		bcopy(v, u, sizeof(*u));
		u->va_next = w;
	    }
	}
	break;
    case US_NOSIMVAR: /* What do you do? */
	txfree(v);
	break;
    default:
	fprintf(cp_err, "cp_vset: Internal Error: bad US value %d\n", i);
    }
}

struct variable * cp_setparse (wordlist *wl)
{
    char *name, *val, *s;
    double *td;
    struct variable *listv = NULL, *vv, *lv = NULL;
    struct variable *vars = NULL;
    char buf[BSIZE_SP];
    int balance, i;

    while (wl) {
	s = wl->wl_word;
	if (*s == '"') s++; /* cp_unquote */
	name = strcpy(buf, s);
	if ((s = strchr(name, '='))) *s = '\0';
	if ((i = strlen(name)) && name[i-1] == '"') name[i-1] = '\0'; /* unquote */
	if (!*name) goto err;

        wl = wl->wl_next;
        if (!s && (!wl || *wl->wl_word != '=')) {
            vv = alloc(struct variable);
            vv->va_name = copy(name);
            vv->va_type = VT_BOOL;
            vv->va_bool = true;
            vv->va_next = vars;
            vars = vv;
            continue;
        }

	if (s) {
            val = s+1;
	    if (*val == '"') val++; /* cp_unquote */
            if (!*val) {
                if (!wl) goto err;
		val = wl->wl_word;
		if (*val == '"') val++; /* unquote */
		wl = wl->wl_next;
		val = strcpy(s+1, val);
            }
        } else if (wl && *wl->wl_word == '=') {
	    s = name + i;
            val = wl->wl_word + 1;
	    if (*val == '"') val++; /* cp_unquote */
            wl = wl->wl_next;
            if (!*val) {
                if (!wl) goto err;
		val = wl->wl_word;
		if (*val == '"') val++; /* unquote */
		wl = wl->wl_next;
	    }
	    val = strcpy(s+1, val);
        } else goto err;

	if ((i = strlen(val)) && val[i-1] == '"') val[i-1] = '\0'; /* unquote */

        if (eq(val, "(")) {
            /* The beginning of a list... We have to walk down
             * the list until we find a close paren... If there
             * are nested ()'s, treat them as tokens...
             */
            balance = 1;
            while (wl && wl->wl_word) {
                if (eq(wl->wl_word, "(")) { /* ) ( */
                    balance++;
                } else if (eq(wl->wl_word, ")")) {
                    if (!--balance) break;
                }
                vv = alloc(struct variable);
		vv->va_next = NULL;
		s = wl->wl_word;
		if (*s == '"') s++; /* cp_unquote */
		s = strcpy(val, s);
		if ((i = strlen(val)) && val[i-1] == '"') val[i-1] = '\0';
                td = ft_numparse(&s, false);
                if (td) {
                    vv->va_type = VT_REAL;
                    vv->va_real = *td;
                } else {
                    vv->va_type = VT_STRING;
                    vv->va_string = copy(val);
                }
                if (listv) {
                    lv->va_next = vv;
                    lv = vv;
                } else
                    listv = lv = vv;
                wl = wl->wl_next;
            }
            if (balance && !wl) goto err;
            vv = alloc(struct variable);
            vv->va_name = copy(name);
            vv->va_type = VT_LIST;
            vv->va_vlist = listv;
            vv->va_next = vars;
            vars = vv;

            wl = wl->wl_next;
            continue;
        }

        td = ft_numparse(&val, false);

        vv = alloc(struct variable);
        vv->va_name = copy(name);
        vv->va_next = vars;
        vars = vv;
        if (td) {
            /* We should try to get VT_NUM ! */
            vv->va_type = VT_REAL;
            vv->va_real = *td;
        } else {
            vv->va_type = VT_STRING;
            vv->va_string = copy(val);
        }
    }
    return (vars);
err:
    fprintf(cp_err, "Error: bad set form.\n");
    return (NULL);
}

void cp_remvar (char *varname)
{
    struct variable *v, *u, *lv = NULL;
    bool found = true;
    int i;

    for (v = variables; v; v = v->va_next) {
        if (eq(v->va_name, varname)) break;
        lv = v;
    }
    if (!v) {
        /* Gotta make up a var struct for cp_usrset()... */
        v = alloc(struct variable);
	ZERO(v, struct variable);
        v->va_name = varname;
        v->va_type = VT_NUM;
        v->va_bool = 0;
        found = false;
    }

    /* Note that 'unset history' doesn't do anything here. Causes trouble...
     */
    if (eq(varname, "noglob"))
        cp_noglob = false;
    else if (eq(varname, "noclobber"))
        cp_noclobber = false;
    else if (eq(varname, "prompt"))
        cp_promptstring = NULL;
    else if (eq(varname, "cpdebug"))
        cp_debug = false;
    else if (eq(varname, "ignoreeof"))
        cp_ignoreeof = false;

    switch (i = cp_usrset(v, false)) {
    case US_OK: /* Normal case. */
        if (found) {
            if (lv) lv->va_next = v->va_next;
            else variables = v->va_next;
        }
        break;
    case US_DONTRECORD: /* Do nothing... */
        if (found) fprintf(cp_err, "cp_remvar: Internal Error: var %s\n", varname);
        break;
    case US_READONLY: /* Badness... */
        fprintf(cp_err, "Error: %s is read-only.\n", v->va_name);
        if (found) fprintf(cp_err, "cp_remvar: Internal Error: var %s\n", varname);
        break;
    case US_SIMVAR:
	lv = NULL;
	if (ft_curckt) {
	    for (u = ft_curckt->ci_vars; u; u = u->va_next) {
		if (eq(varname, u->va_name)) break;
		lv = u;
	    }
	    if (u) {
		if (lv) lv->va_next = u->va_next;
		else ft_curckt->ci_vars = u->va_next;
		txfree(u);
	    }
	}
	break;
    default:
	fprintf(cp_err, "cp_remvar: Internal Error: US value %d\n", i);
    }

    txfree(v);
}
