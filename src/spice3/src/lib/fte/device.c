/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to query and alter devices.
 */

#include "spice.h"
#include "gendefs.h"
#include "cktdefs.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "dgen.h"

/*
 *	show: list device operating point info
 *		show
 *		show devs : params
 *		show devs : params ; devs : params
 *		show dev dev dev : param param param , dev dev : param param
 *		show t : param param param , t : param param
 *
 */

static int printstr();
static int printvals();
static int bogus1();
static int bogus2();
static void old_show();
static void all_show();
static void com_alter_common();
static void listparam();
static void param_forall();
static wordlist *devexpand();

static int count;

void com_showmod(wordlist *wl)
{
    all_show(wl, 1);
}

void com_show(wordlist *wl)
{
    all_show(wl, 0);
}

static void all_show(wordlist *wl, int mode)
{
    wordlist	*params, *thisgroup, *prev, *next, *w;
    wordlist	*fl = NULL, *p1, *w1 = NULL;
    int		screen_width;
    dgen	*dg, *listdg;
    int		instances;
    int		i, j, n;
    int		param_flag, dev_flag;

    if (!ft_curckt) {
        fprintf(cp_err, "Error: no circuit loaded\n");
        return;
    }

    if (wl && eq(wl->wl_word, "-v")) { old_show(wl->wl_next); return; }

    if (!cp_getvar("width", VT_NUM, (char *) &screen_width))
	    screen_width = DEF_WIDTH;
    count = screen_width / 11 - 1;

    while (wl && (eq(wl->wl_word, ";") || eq(wl->wl_word, ","))) wl = wl->wl_next;
    thisgroup = wl;

    n = 0;
    do {
	prev = params = next = p1 = NULL;
	dev_flag = param_flag = 0;

	/* find the parameter list and the next group */
	for (w = thisgroup; w; w = next) {
	    next = w->wl_next;
	    if (eq(w->wl_word, "++") || eq(w->wl_word, "all")) {
		if (params) {
		    param_flag = DGEN_ALLPARAMS;
		    if (prev) { prev->wl_next = next; w->wl_next = fl; fl = w; } else params = next;
		} else {
		    dev_flag = DGEN_ALLDEVS;
		    if (prev) { prev->wl_next = next; w->wl_next = fl; fl = w; } else thisgroup = next;
		}
	    } else if (eq(w->wl_word, "+")) {
		if (params) {
		    param_flag = DGEN_DEFPARAMS;
		    if (prev) { prev->wl_next = next; w->wl_next = fl; fl = w; } else params = next;
		} else {
		    dev_flag = DGEN_DEFDEVS;
		    if (prev) { prev->wl_next = next; w->wl_next = fl; fl = w; } else thisgroup = next;
		}
	    } else if (eq(w->wl_word, ":")) {
		if (params) {
		    if (prev) { prev->wl_next = next; w->wl_next = fl; fl = w; } else params = next;
		} else {
		    params = next;
		    if (prev) { prev->wl_next = NULL; p1 = prev; w1 = w; prev = NULL; }
		    else thisgroup = NULL;
		}
	    } else if (eq(w->wl_word, ";") || eq(w->wl_word, ",")) {
		if (prev) prev->wl_next = NULL;
		break;
	    } else prev = w;
	}

	instances = 0;
	for (dg = dgen_init(ft_curckt->ci_ckt, thisgroup, 1, dev_flag, mode);
		dg; dgen_nth_next(&dg, count))
	{
	    instances = 1;
	    if (dg->flags & DGEN_INSTANCE) {
		instances = 2;
		printf(" %s: %s\n",
			ft_sim->devices[dg->dev_type_no]->name,
			ft_sim->devices[dg->dev_type_no]->description);
		n += 1;

		i = 0;
		do {
			printf(" device   ");
			j = dgen_for_n(dg, count, printstr, "n", i);
			i += 1;
			printf("\n");
		} while (j);

		if (ft_sim->devices[dg->dev_type_no]->numModelParms) {
			i = 0;
			do {
				printf(" model    ");
				j = dgen_for_n(dg, count, printstr, "m", i);
				i += 1;
				printf("\n");
			} while (j);
		}
		listdg = dg;

		if (param_flag)
		    param_forall(dg, param_flag);
		else if (!params)
		    param_forall(dg, DGEN_DEFPARAMS);
		if (params) wl_forall(params, listparam, dg);
		printf("\n");

	    } else if (ft_sim->devices[dg->dev_type_no]->numModelParms) {
		printf(" %s models (%s)\n",
			ft_sim->devices[dg->dev_type_no]->name,
			ft_sim->devices[dg->dev_type_no]->description);
		n += 1;
		i = 0;
		do {
			printf(" model    ");
			j = dgen_for_n(dg, count, printstr, "m", i);
			i += 1;
			printf("\n");
		} while (j);
		printf("\n");

		if (param_flag)
		    param_forall(dg, param_flag);
		else if (!params)
		    param_forall(dg, DGEN_DEFPARAMS);
		if (params) wl_forall(params, listparam, dg);
		printf("\n");
	    }
	}

	if (p1) p1->wl_next = w1;
	if (prev) prev->wl_next = w;

    } while ((thisgroup = next));

    if (fl) wl_free(fl);

    if (!n) {
	    if (instances == 0)
		printf("No matching instances or models\n");
	    else if (instances == 1)
		printf("No matching models\n");
	    else
		printf("No matching elements\n");
    }
}

static int printstr(dgen *dg, char *name)
{
    if (*name == 'n') {
	if (dg->instance)
	    printf(" %9.9s", (char*)dg->instance->GENname);
	else
	    printf(" <%s>", "???????");
    } else if (*name == 'm') {
	if (dg->model)
	    printf(" %9.9s", (char*)dg->model->GENmodName);
	else
	    printf(" <%s>", "???????");
    } else
	printf("  <error> ");

    return 0;
}

static void param_forall(dgen *dg, int flags)
{
    int	i, j, k, found;
    int xcount;
    IFparm *plist;

    found = 0;

    if (dg->flags & DGEN_INSTANCE) {
	xcount = *ft_sim->devices[dg->dev_type_no]->numInstanceParms;
	plist = ft_sim->devices[dg->dev_type_no]->instanceParms;
    } else {
	xcount = *ft_sim->devices[dg->dev_type_no]->numModelParms;
	plist = ft_sim->devices[dg->dev_type_no]->modelParms;
    }

    for (i = 0; i < xcount; i++) {
	if (plist[i].dataType & IF_ASK) {
	    if ((((CKTcircuit *) (dg->ckt))->CKTrhsOld
		|| (plist[i].dataType & IF_SET))
		&& (!(plist[i].dataType & (IF_REDUNDANT | IF_UNINTERESTING))
		|| (flags == DGEN_ALLPARAMS
		&& !(plist[i].dataType & IF_REDUNDANT))))
	    {
		j = 0;
		do {
			if (!j)
			    printf("%10.10s", plist[i].keyword);
			else
			    printf("          ");
			k = dgen_for_n(dg, count, printvals, (char *) (plist + i), j);
			printf("\n");
			j += 1;
		} while (k);
	    }
	}
    }
}

static void listparam (wordlist *p, dgen *dg)
{
    int	i, j, k, found;
    int	xcount;
    IFparm *plist;

    found = 0;

    if (dg->flags & DGEN_INSTANCE) {
	xcount = *ft_sim->devices[dg->dev_type_no]->numInstanceParms;
	plist = ft_sim->devices[dg->dev_type_no]->instanceParms;
    } else {
	xcount = *ft_sim->devices[dg->dev_type_no]->numModelParms;
	plist = ft_sim->devices[dg->dev_type_no]->modelParms;
    }

    for (i = 0; i < xcount; i++) {
	if (cieq(p->wl_word, plist[i].keyword) && (plist[i].dataType & IF_ASK)) {
	    found = 1;
	    break;
	}
    }

    if (found) {
	if ((((CKTcircuit *) (dg->ckt))->CKTrhsOld
	    || (plist[i].dataType & IF_SET)))
	{
	    j = 0;
	    do {
		if (!j)
		    printf("%10.10s", p->wl_word);
		else
		    printf("          ");
		k = dgen_for_n(dg, count, printvals, plist + i, j);
		printf("\n");
		j += 1;
	    } while (k > 0);
	} else {
	    j = 0;
	    do {
		if (!j)
		    printf("%10.10s", p->wl_word);
		else
		    printf("          ");
		k = dgen_for_n(dg, count, bogus1, 0, j);
		printf("\n");
		j += 1;
	    } while (k > 0);
	}
    } else {
	j = 0;
	do {
	    if (!j)
		printf("%10.10s", p->wl_word);
	    else
		printf("          ");
	    k = dgen_for_n(dg, count, bogus2, 0, j);
	    printf("\n");
	    j += 1;
	} while (k > 0);
    }
}

static int bogus1(dgen *dg)
{
    printf(" ---------");
    return 0;
}

static int bogus2(dgen *dg)
{
    printf(" ?????????");
    return 0;
}

static int printvals(dgen *dg, IFparm *p, int i)
{
    IFvalue	val;
    int		n;

    if (dg->flags & DGEN_INSTANCE)
	(*ft_sim->askInstanceQuest)(ft_curckt->ci_ckt, dg->instance, p->id, &val, &val);
    else
	(*ft_sim->askModelQuest)(ft_curckt->ci_ckt, dg->model, p->id, &val, &val);

    if (p->dataType & IF_VECTOR)
	n = val.v.numValue;
    else
	n = 1;

    if (((p->dataType & IF_VARTYPES) & ~IF_VECTOR) == IF_COMPLEX)
	n *= 2;

    if (i >= n) {
	if (i == 0)
	    printf("         -");
	else
	    printf("          ");
	return 0;
    }

    if (p->dataType & IF_VECTOR) {
	switch ((p->dataType & IF_VARTYPES) & ~IF_VECTOR) {
	    case IF_FLAG:
		    printf(" %9d", val.v.vec.iVec[i]);
		    break;
	    case IF_INTEGER:
		    printf(" %9d", val.v.vec.iVec[i]);
		    break;
	    case IF_REAL:
		    printf(" %9.3g", val.v.vec.rVec[i]);
		    break;
	    case IF_COMPLEX:
		    if (!(i % 2))
			    printf(" %9.3g", val.v.vec.cVec[i / 2].real);
		    else
			    printf(" %9.3g", val.v.vec.cVec[i / 2].imag);
		    break;
	    case IF_STRING:
		    printf(" %9.9s", val.v.vec.sVec[i]);
		    break;
	    case IF_INSTANCE:
		    printf(" %9.9s", (char*)val.v.vec.uVec[i]);
		    break;
	    default:
		    printf(" ******** ");
	}
    } else {
	switch ((p->dataType & IF_VARTYPES) & ~IF_VECTOR) {
	    case IF_FLAG:
		    printf(" %9d", val.iValue);
		    break;
	    case IF_INTEGER:
		    printf(" %9d", val.iValue);
		    break;
	    case IF_REAL:
		    printf(" %9.3g", val.rValue);
		    break;
	    case IF_COMPLEX:
		    if (i % 2)
			    printf(" %9.3g", val.cValue.real);
		    else
			    printf(" %9.3g", val.cValue.imag);
		    break;
	    case IF_STRING:
		    printf(" %9.9s", val.sValue);
		    break;
	    case IF_INSTANCE:
		    printf(" %9.9s ", (char*)val.uValue);
		    break;
	    default:
		    printf(" ******** ");
	}
    }

    return n - 1;
}

/* (old "show" command)
 * Display various device parameters.  The syntax of this command is
 *   show devicelist : parmlist
 * where devicelist can be "all", the name of a device, a string like r*,
 * which means all devices with names that begin with 'r', repeated one
 * or more times.   The parms are names of parameters that are (hopefully)
 * valid for all the named devices, or "all".
 */
static void old_show(wordlist *wl)
{
    wordlist *devs, *dev, *parms, *tw, *ww, *w;
    struct variable *v;
    char *nn;

    devs = wl;
    while (wl && !eq(wl->wl_word, ":")) wl = wl->wl_next;

    if (!wl) parms = NULL;
    else {
        parms = wl->wl_next;
	for (tw = parms; tw; tw = tw->wl_next)
	    if (eq(tw->wl_word, "all")) { parms = NULL; break; }
    }

    /* Now expand the devicelist... */
    for (tw = NULL; devs != wl; devs = devs->wl_next) {
        inp_casefix(devs->wl_word);
        tw = wl_append(tw, devexpand(devs->wl_word));
    }
    devs = tw;
    if (!devs) devs = cp_cctowl(ft_curckt->ci_devices);

    out_init();

    for (dev = devs; dev; dev = dev->wl_next) {
        out_printf("%s:\n", dev->wl_word);
        if (parms) {
            for (tw = parms; tw; tw = tw->wl_next) {
                nn = copy(dev->wl_word);
                v = (*if_getparam)(ft_curckt->ci_ckt, &nn, tw->wl_word, 0, 0);
		if (!v)
		    v = (*if_getparam)(ft_curckt->ci_ckt, &nn, tw->wl_word, 0, 1);
                if (v) {
                    out_printf("\t%s =", tw->wl_word);
                    for (w = ww = cp_varwl(v); ww; ww = ww->wl_next)
                        out_printf(" %s", ww->wl_word);
		    wl_free (w);
                    out_send("\n");
                }
            }
        } else {
            nn = copy(dev->wl_word);
            v = (*if_getparam)(ft_curckt->ci_ckt, &nn, "all", 0, 0);
	    if (!v)
		v = (*if_getparam)(ft_curckt->ci_ckt, &nn, "all", 0, 1);
            while (v) {
                out_printf("\t%s =", v->va_name);
                for (w = ww = cp_varwl(v); ww; ww = ww->wl_next)
                    out_printf(" %s", ww->wl_word);
		wl_free (w);
                out_send("\n");
                v = v->va_next;
            }
        }
    }
    wl_free (devs);
}

/* Alter a device parameter.  The new syntax here is
 *	alter @device[parameter] = expr
 *	alter device = expr
 *	alter device parameter = expr
 * expr must be real (complex isn't handled right now, integer is fine though,
 * but no strings ... for booleans, use 0/1).
 */
void com_alter(wordlist *wl)
{
    if (!wl) {
	fprintf(cp_err, "usage: alter dev param = expression\n");
	fprintf(cp_err, "  or   alter @dev[param] = expression\n");
	fprintf(cp_err, "  or   alter dev = expression\n");
	return;
    }
    com_alter_common(wl, 0);
}

void com_altermod(wordlist *wl)
{
    com_alter_common(wl, 1);
}

static void com_alter_common(wordlist *wl, int do_model)
{
#ifdef notdef
    struct variable var, *nv, *prev;
    double *dd;
#endif
    wordlist *eqword, *words;
    char *dev, *p;
    char *param;
    struct dvec *dv;
    struct pnode *names;

    if (!ft_curckt) {
        fprintf(cp_err, "Error: no circuit loaded\n");
        return;
    }

    words = wl;
    while (words) {
	p = words->wl_word;
	eqword = words;
	words = words->wl_next;
	if (eq(p, "=")) {
	    break;
	}
    }
    if (!words) {
	fprintf(cp_err, "Error: no assignment found.\n");
	return;
    }

    /* device parameter = expr
       device = expr
       @dev[param] = expr
     */

    dev = NULL;
    param = NULL;
    words = wl;
    while (words != eqword) {
	p = words->wl_word;
	if (param) {
	    fprintf(cp_err, "Error: excess parameter name \"%s\" ignored.\n", p);
	} else if (dev) {
	    param = words->wl_word;
	} else if (*p == '@' || *p == '#') {
	    dev = p + 1;
	    p = strchr(p, '[');
	    if (p) {
		*p++ = 0;
		param = p;
		p = strchr(p, ']');
		if (p) *p = 0;
	    }
	} else {
	    dev = p;
	}
	words = words->wl_next;
    }
    if (!dev) {
	fprintf(cp_err, "Error: no model or device name provided.\n");
	return;
    }

    words = eqword->wl_next;
    names = ft_getpnames(words, false);
    if (!names) {
	fprintf(cp_err, "Error: cannot parse new parameter value.\n");
	return;
    }
    dv = ft_evaluate(names);
    free_pnode(names);
    if (!dv)
	return;
    if (dv->v_length < 1) {
	fprintf(cp_err, "Error: cannot evaluate new parameter value.\n");
	return;
    }

    if_setparam(ft_curckt->ci_ckt, &dev, param, dv, do_model);

    /* Vector data (dv) should get garbage-collected. */

    return;

#ifdef notdef
    while (wl) {
	param = wl->wl_word;
	wl = wl->wl_next;

	if (!wl) {
	    val = param;
	    param = NULL;
	} else {
	    val = wl->wl_word;
	    wl = wl->wl_next;
	}

	/* Now figure out what the value should be... */
	if (eq(val, "true")) {
	    var.va_type = VT_BOOL;
	    var.va_bool = true;
	} else if (eq(val, "false")) {
	    var.va_type = VT_BOOL;
	    var.va_bool = false;
	} else if (eq(val, "[")) {
	    var.va_type = VT_LIST;
	    prev = NULL;
	    while (wl && !eq(wl->wl_word, "]")) {
		val = wl->wl_word;
		nv = alloc(struct variable);
		if (dd = ft_numparse(&val, false)) {
			nv->va_type = VT_REAL;
			nv->va_real = *dd;
		} else {
			fprintf(cp_err, "Error: \"%s\" is not a number\n", val);
			break;
		}
		if (!prev)
		    var.va_vlist = nv;
		else
		    prev->va_next = nv;
		nv->va_next = NULL;
		wl = wl->wl_next;
		prev = nv;
	    }
	    if (wl && eq(wl->wl_word, "]")) {
		wl = wl->wl_next;
	    } else {
		while (nv) {
			prev = nv->va_next;
			tfree(nv);
			nv = prev;
		}
		return;
	    }
	} else if (dd = ft_numparse(&val, false)) {
	    var.va_type = VT_REAL;
	    var.va_real = *dd;
	} else {
	    var.va_type = VT_STRING;
	    var.va_string = val;
	}

        if_setparam(ft_curckt->ci_ckt, &dev, param, &var, do_model);

	if (var.va_type == VT_LIST) {
	    for (nv = var.va_vlist; nv; nv = prev) {
		prev = nv->va_next;
		tfree(nv);
	    }
	}
    }
#endif
}

/* Given a device name, possibly with wildcards, return the matches. */

static wordlist * devexpand(char *name)
{
    wordlist *wl, *devices, *tw;

    if (strchr(name, '*') || strchr(name, '[') || strchr(name, '?')) {
        devices = cp_cctowl(ft_curckt->ci_devices);
        for (wl = NULL; devices; devices = devices->wl_next)
            if (cp_globmatch(name, devices->wl_word)) {
                tw = alloc(wordlist);
                if (wl) {
                    wl->wl_prev = tw;
                    tw->wl_next = wl;
                    wl = tw;
                } else
                    wl = tw;
                wl->wl_word = devices->wl_word;
            }
    } else if (eq(name, "all")) {
        wl = cp_cctowl(ft_curckt->ci_devices);
    } else {
        wl = alloc(wordlist);
        wl->wl_word = name;
    }
    wl_sort(wl);
    return (wl);
}
