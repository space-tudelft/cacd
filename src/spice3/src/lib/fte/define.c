/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * User-defined functions. The user defines the function with
 *  define func(arg1, arg2, arg3) <expression involving args...>
 * Then when he types "func(1, 2, 3)", the commas are interpreted as
 * binary operations of the lowest priority by the parser, and ft_substdef()
 * below is given a chance to fill things in and return what the parse tree
 * would have been had the entire thing been typed.
 * Note that we have to take some care to distinguish between functions
 * with the same name and different arities.
 */

#include "spice.h"
#include "util.h"
#include "misc.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "fteparse.h"

static struct pnode *ntharg();
static struct pnode *trcopy();
static void prdefs();
static void prtree();
static void prtree1();
static void savetree();

static struct udfunc *udfuncs = NULL;

/* Set up a function definition. */

void com_define(wordlist *wlist)
{
    int arity = 0, i;
    char buf[BSIZE_SP], *s, *t;
    wordlist *wl;
    struct pnode *pn;
    struct udfunc *udf;

    /* If there's nothing then print all the definitions. */
    buf[0] = '\0';
    if (!wlist) { prdefs(buf); return; }

    /* Accumulate the function head in the buffer, w/out spaces. A
     * useful thing here would be to check to make sure that there
     * are no formal parameters here called "list". But you have
     * to try really hard to break this here.
     */
    t = NULL;
    for (wl = wlist; wl; wl = wl->wl_next) {
	if (!t) {
	    t = strchr(wl->wl_word, '(');
	    if (wl != wlist && (!t || t > wl->wl_word)) strcat(buf, " ");
	}
        strcat(buf, wl->wl_word);
	if ((s = strchr(wl->wl_word, ')'))) break;
    }
    t = NULL;
    if (wl) {
        if (*++s) { /* Just in case */
	    t = wl->wl_word;
            wl->wl_word = s;
	    if ((s = strchr(buf, ')'))) *++s = '\0';
	}
	else wl = wl->wl_next;
    }

    /* If that's all, then print the definition. */
    if (!wl) { prdefs(buf); return; }

    /* Now check to see if this is a valid name for a function (i.e,
     * there isn't a predefined function of the same name).
     */
    i = 0;
    for (s = buf; *s && *s != '('; s++) {
        if (isspace(*s)) ++i;
    }
    if (!*s || i) { *s = '\0';
	fprintf(cp_err, "Error: '%s' is not valid function name.\n", buf);
	if (t) wl->wl_word = t;
	return;
    }
    *s = '\0';
    for (i = 0; ft_funcs[i].fu_name; i++)
        if (eq(ft_funcs[i].fu_name, buf)) {
            fprintf(cp_err, "Error: '%s' is a predefined function.\n", buf);
	    if (t) wl->wl_word = t;
            return;
        }
    *s = '(';

    /* Parse the rest of it. We can't know if there are the right
     * number of undefined variables in the expression.
     */
    pn = ft_getpnames(wl, false);
    if (t) wl->wl_word = t;
    if (!pn) return;

    /* This is a pain -- when things are garbage-collected, any
     * vectors that may have been mentioned here will be thrown
     * away. So go down the tree and save any vectors that aren't
     * formal parameters.
     */
    savetree(pn);

    /* Format the name properly and add to the list. */
    t = copy(buf);
    for (s = t; *s; s++) {
        if (*s == '(') {
            *s = '\0';
            if (s[1] != ')') arity++;
        } else if (*s == ')') {
            *s = '\0';
        } else if (*s == ',') {
            *s = '\0';
            arity++;
        }
    }

    for (udf = udfuncs; udf; udf = udf->ud_next)
        if (prefix(t, udf->ud_name) && (arity == udf->ud_arity))
            break;
    if (!udf) {
        udf = alloc(struct udfunc);
        if (udfuncs == NULL) {
            udfuncs = udf;
            udf->ud_next = NULL;
	} else {
            udf->ud_next = udfuncs;
            udfuncs = udf;
        }
    }
    udf->ud_text = pn;
    udf->ud_name = t;
    udf->ud_arity = arity;
    cp_addkword(CT_UDFUNCS, t);
}

/* Kludge. */

static void savetree(struct pnode *pn)
{
    struct dvec *d;

    if (pn->pn_value) {
        /* We specifically don't add this to the plot list
         * so it won't get gc'ed.
         */
        d = pn->pn_value;
        if ((d->v_length != 0) || eq(d->v_name, "list")) {
            pn->pn_value = alloc(struct dvec);
	    ZERO(pn->pn_value, struct dvec);
            pn->pn_value->v_name = copy(d->v_name);
            pn->pn_value->v_length = d->v_length;
            pn->pn_value->v_type = d->v_type;
            pn->pn_value->v_flags = d->v_flags;
            pn->pn_value->v_plot = d->v_plot;
            if (isreal(d)) {
                pn->pn_value->v_realdata = allocn(double, d->v_length);
                bcopy((char *) d->v_realdata,
                    (char *) pn->pn_value->v_realdata, sizeof(double) * d->v_length);
            } else {
                pn->pn_value->v_compdata = allocn(complex, d->v_length);
                bcopy((char *) d->v_compdata,
                    (char *) pn->pn_value->v_compdata, sizeof(complex) * d->v_length);
            }
        }
    } else if (pn->pn_op) {
        savetree(pn->pn_left);
        if (pn->pn_op->op_arity == 2) savetree(pn->pn_right);
    } else if (pn->pn_func) {
        savetree(pn->pn_left);
    }
}

/* A bunch of junk to print out nodes. */

static void prdefs(char *name)
{
    struct udfunc *udf;
    char *s;

    if ((s = strchr(name, '('))) *s = '\0';

    for (udf = udfuncs; udf; udf = udf->ud_next)
	if (!*name || eq(name, udf->ud_name)) prtree(udf);
}

/* Print out one definition. */

static void prtree(struct udfunc *ud)
{
    char *s, buf[BSIZE_SP];

    /* Print the head. */
    buf[0] = '\0';
    strcat(buf, ud->ud_name);
    for (s = ud->ud_name; *s; s++);
    strcat(buf, " (");
    s++;
    while (*s) {
        strcat(buf, s);
        while (*s) s++;
        if (s[1]) strcat(buf, ", ");
        s++;
    }
    strcat(buf, ") = ");
    fputs(buf, cp_out);
    prtree1(ud->ud_text, cp_out);
    putc('\n', cp_out);
}

static void prtree1(struct pnode *pn, FILE *fp)
{
    if (pn->pn_value) {
        fputs(pn->pn_value->v_name, fp);
    } else if (pn->pn_func) {
        fprintf(fp, "%s (", pn->pn_func->fu_name);
        prtree1(pn->pn_left, fp);
        fputs(")", fp);
    } else if (pn->pn_op && (pn->pn_op->op_arity == 2)) {
        fputs("(", fp);
        prtree1(pn->pn_left, fp);
        fprintf(fp, ")%s(", pn->pn_op->op_name);
        prtree1(pn->pn_right, fp);
        fputs(")", fp);
    } else if (pn->pn_op && (pn->pn_op->op_arity == 1)) {
        fprintf(fp, "%s(", pn->pn_op->op_name);
        prtree1(pn->pn_left, fp);
        fputs(")", fp);
    } else
        fputs("<something strange>", fp);
}

struct pnode * ft_substdef(char *name, struct pnode *args)
{
    struct udfunc *udf;
    struct pnode *tp;
    char *s;
    struct udfunc *udf_found = NULL;
    int arity = 0;

    if (args)
        arity = 1;
    for (tp = args; tp && tp->pn_op && (tp->pn_op->op_num == COMMA); tp =
            tp->pn_right)
        arity++;
    for (udf = udfuncs; udf; udf = udf->ud_next)
        if (eq(name, udf->ud_name)) {
            if (arity == udf->ud_arity) break;
	    udf_found = udf;
        }
    if (udf == NULL) {
        if (udf_found)
            fprintf(cp_err,
        "Warning: the user-defined function %s has %d args\n",
		name, udf_found->ud_arity);
        return (NULL);
    }
    for (s = udf->ud_name; *s; s++)
        ;
    s++;

    /* Now we have to traverse the tree and copy it over,
     * substituting args.
     */
    return (trcopy(udf->ud_text, s, args));
}

/* Copy the tree and replace formal args with the right stuff. The way
 * we know that something might be a formal arg is when it is a dvec
 * with length 0 and a name that isn't "list". I hope nobody calls their
 * formal parameters "list".
 */
static struct pnode * trcopy(tree, args, nn)
    struct pnode *tree;
    char *args;
    struct pnode *nn;
{
    struct pnode *pn;
    struct dvec *d;
    char *s;
    int i;

    if (tree->pn_value) {
        d = tree->pn_value;
        if ((d->v_length == 0) && !eq(d->v_name, "list")) {
            /* Yep, it's a formal parameter. Substitute for it.
             * IMPORTANT: we never free parse trees, so we
             * needn't worry that they aren't trees here.
             */
            s = args;
            i = 1;
            while (*s) {
                if (eq(s, d->v_name))
                    break;
                else
                    i++;
                while (*s++);   /* Get past the last '\0'. */
            }
            if (*s)
                return (ntharg(i, nn));
            else
                return (tree);
        } else
            return (tree);
    } else if (tree->pn_func) {
        struct func *func;

        func = alloc(struct func);
        func->fu_name = copy(tree->pn_func->fu_name);
        func->fu_func = tree->pn_func->fu_func;

        pn = alloc(struct pnode);
        pn->pn_value = NULL;
        pn->pn_func = func;
        pn->pn_op = NULL;
        pn->pn_left = trcopy(tree->pn_left, args, nn);
        pn->pn_right = NULL;
        pn->pn_next = NULL;

    } else if (tree->pn_op) {
        struct op *op;

        op = alloc(struct op);
        op->op_num = tree->pn_op->op_num;
        op->op_arity = tree->pn_op->op_arity;
        op->op_func = tree->pn_op->op_func;
        op->op_name = copy(tree->pn_op->op_name);

        pn = alloc(struct pnode);
        pn->pn_value = NULL;
        pn->pn_func = NULL;
        pn->pn_op = op;
        pn->pn_left = trcopy(tree->pn_left, args, nn);
        if (op->op_arity == 2)
            pn->pn_right = trcopy(tree->pn_right, args, nn);
	else
            pn->pn_right = NULL;
        pn->pn_next = NULL;
    } else {
        fprintf(cp_err, "trcopy: Internal Error: bad parse node\n");
        return (NULL);
    }
    return (pn);
}

/* Find the n'th arg in the arglist, returning NULL if there isn't one.
 * Since comma has such a low priority and associates to the right,
 * we can just follow the right branch of the tree num times.
 * Note that we start at 1 when numbering the args.
 */
static struct pnode * ntharg(int num, struct pnode *args)
{
    struct pnode *ptry;

    ptry = args;
    if (num > 1) {
        while (--num > 0) {
            if (ptry && ptry->pn_op && ptry->pn_op->op_num != COMMA) {
                if (num == 1)
                    break;
                else
                    return (NULL);
            }
            ptry = ptry->pn_right;
        }
    }
    if (ptry && ptry->pn_op && ptry->pn_op->op_num == COMMA)
        ptry = ptry->pn_left;
    return (ptry);
}

void com_undefine(wordlist *wlist)
{
    struct udfunc *udf, *ludf = NULL;

    if (!wlist) return;

    if (*wlist->wl_word == '*') {
        udfuncs = NULL;     /* Be sloppy. */
        return;
    }
    while (wlist) {
        for (udf = udfuncs; udf; udf = udf->ud_next) {
            if (eq(wlist->wl_word, udf->ud_name)) {
                if (ludf)
                    ludf->ud_next = udf->ud_next;
                else
                    udfuncs = udf->ud_next;
                cp_remkword(CT_UDFUNCS, wlist->wl_word);
            } else
                ludf = udf;
        }
        wlist = wlist->wl_next;
    }
}

/* Watch out, this is not at all portable.  It's only here so I can
 * call it from dbx with an int value (all you can give with "call")...
 */
void ft_pnode(struct pnode *pn)
{
    prtree1(pn, cp_err);
}
