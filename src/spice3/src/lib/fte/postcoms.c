/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Various post-processor commands having to do with vectors.
 */

#include "spice.h"
#include "util.h"
#include "misc.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "fteparse.h"
#include "ftedata.h"

static int dcomp();
static void pvec();
static void killplot();

void com_let (wordlist *wl)
{
    static wordlist fake_wl;
    char *p, *q, *s;
    int	indices[MAXDIMS];
    int	numdims;
    int need_open;
    int offset, length;
    struct pnode *nn;
    struct dvec *n, *t;
    int i, cube, depth;
    int newvec;
    char *rhs;

    if (!wl) { com_display(wl); return; }

    p = wl_flatten(wl);

    /* extract indices */
    numdims = 0;
    if ((rhs = strchr(p, '='))) *rhs++ = 0;
    else {
	fprintf(cp_err, "Error: bad let syntax\n");
	goto ret;
    }

    if ((s = strchr(p, '['))) {
	need_open = 0;
	*s++ = 0;
	while (!need_open || *s == '[') {
	    depth = 0;
	    if (need_open)
		s++;
	    for (q = s; *q && ((*q != ']' && *q != ',') || depth > 0); q++) {
		switch (*q) {
		case '[':
		    depth += 1;
		    break;
		case ']':
		    depth -= 1;
		    break;
		}
	    }

	    if (depth != 0 || !*q) {
		printf("syntax error specifying index\n");
		goto ret;
	    }

	    if (*q == ']')
		need_open = 1;
	    else
		need_open = 0;

	    if (*q) *q++ = 0;

#ifdef notdef
/* MW. let[x,y] is not supported - we don't need this code */
	/*
	 *  evaluate expression between s and q
	 */
	    fake_wl.wl_word = s;
	    nn = ft_getpnames(&fake_wl, true);
	    t = ft_evaluate(nn);
	    free_pnode(nn);

	    if (!t) {
	    	fprintf(cp_err, "Error: bad index.\n");
	    	goto ret;
	    }
	    if (!isreal(t) || t->v_link2 || t->v_length != 1 || !t->v_realdata) {
		fprintf(cp_err, "Error: index is not a scalar.\n");
		goto ret;
	    }
	    i = t->v_realdata[0];
	    /* ignore sanity checks for now */
	    if (i < 0) {
		printf("negative index (%d) is not allowed\n", i);
		goto ret;
	    }
	    indices[numdims++] = i;
#endif
/* MW. Next line does not hurt. */
	    for (s = q; *s && isspace(*s); s++) ;
	}
    }
    /* vector name at p */

    for (q = p + strlen(p) - 1; *q <= ' ' && p <= q; q--) ;

    *++q = 0;

    /* sanity check */
    if (eq(p, "all") || strchr(p, '@')) {
        fprintf(cp_err, "Error: bad variable name %s\n", p);
        goto ret;
    }

    /* evaluate rhs */
    fake_wl.wl_word = rhs;
    nn = ft_getpnames(&fake_wl, true);
    t = ft_evaluate(nn);
    if (!t) {
	fprintf(cp_err, "Error: Can't evaluate %s\n", rhs);
        goto ret;
    }

    if (t->v_link2)
        fprintf(cp_err, "Warning: extra wildcard values ignored\n");

    n = vec_get(p);

    if (n) {
	/* re-allocate? */
	/* vec_free(n); */
	newvec = 0;
    } else {
	if (numdims) {
	    fprintf(cp_err, "Can't assign into a subindex of a new vector\n");
	    goto ret;
	}

	/* create and assign a new vector */
	n = alloc(struct dvec);
	ZERO(n, struct dvec);
	n->v_name = copy(p);
	n->v_type = t->v_type;
	n->v_flags = (t->v_flags | VF_PERMANENT);
	n->v_length = t->v_length;

	if (!t->v_numdims) {
	    n->v_numdims = 1;
	    n->v_dims[0] = n->v_length;
	} else {
	    n->v_numdims = t->v_numdims;
	    for (i = 0; i < t->v_numdims; i++)
		n->v_dims[i] = t->v_dims[i];
	}

	if (isreal(t))
	    n->v_realdata = allocn(double, n->v_length);
	else
	    n->v_compdata = allocn(complex, n->v_length);
	newvec = 1;
	vec_new(n);
    }

    /* fix-up dimensions */
    if (n->v_numdims < 1) {
	n->v_numdims = 1;
	n->v_dims[0] = n->v_length;
    }

    /* Compare dimensions */
    offset = 0;
    length = n->v_length;

    cube = 1;
    for (i = n->v_numdims - 1; i >= numdims; i--)
	cube *= n->v_dims[i];

    for (i = numdims - 1; i >= 0; i--) {
	offset += cube * indices[i];
	if (i < n->v_numdims) {
	    cube *= n->v_dims[i];
	    length /= n->v_dims[i];
	}
    }

    /* length is the size of the unit refered to */
    /* cube ends up being the length */

    if (length > t->v_length) {
	fprintf(cp_err, "left-hand expression is too small (need %d)\n", length * cube);
	if (newvec) n->v_flags &= ~VF_PERMANENT;
	goto ret;
    }
    if (isreal(t) != isreal(n)) {
	fprintf(cp_err, "Types of vectors are not the same (real vs. complex)\n");
	if (newvec) n->v_flags &= ~VF_PERMANENT;
	goto ret;
    } else if (isreal(t)) {
	bcopy((char *) t->v_realdata, (char *) (n->v_realdata + offset), length * sizeof(double));
    } else {
	bcopy((char *) t->v_compdata, (char *) (n->v_compdata + offset), length * sizeof(complex));
    }

    n->v_minsignal = 0.0; /* How do these get reset ??? */
    n->v_maxsignal = 0.0;

    n->v_scale = t->v_scale;

    if (newvec) cp_addkword(CT_VECTOR, n->v_name);

ret:
    /* XXXX Free t !?! */
    tfree(p);
}

/* Undefine vectors. */

void com_unlet(wordlist *wl)
{
    while (wl) {
        vec_remove(wl->wl_word);
        wl = wl->wl_next;
    }
}

/* Load in a file. */

void com_load(wordlist *wl)
{
    char *s;

    if (!wl)
        ft_loadfile(ft_rawfile);
    else
        while (wl) {
            s = cp_unquote(wl->wl_word);
            ft_loadfile(s);
	    txfree(s);
            wl = wl->wl_next;
        }

    /* note: default is to display the vectors in the last (current) plot */
    com_display(NULL);
}

/* Print out the value of an expression. When we are figuring out what to
 * print, link the vectors we want with v_link2... This has to be done
 * because of the way temporary vectors are linked together with permanent
 * ones under the plot.
 */
void com_print(wordlist *wl)
{
    struct dvec *v, *lv, *bv, *nv, *vecs = NULL;
    int i, j, ll, width = DEF_WIDTH, height = DEF_HEIGHT, npoints, lineno;
    struct pnode *nn;
    struct plot *p;
    bool col = true, nobreak = false, noprintscale, plotnames = false;
    bool optgiven = false;
    char *s, buf[BSIZE_SP], buf2[BSIZE_SP];

    if (!wl) return;

    if (eq(wl->wl_word, "col")) {
        col = true;
        optgiven = true;
        wl = wl->wl_next;
    } else if (eq(wl->wl_word, "line")) {
        col = false;
        optgiven = true;
        wl = wl->wl_next;
    }

    lv = NULL; /* init, prevent compiler warning */
    for (nn = ft_getpnames(wl, true); nn; nn = nn->pn_next) {
        if (!(v = ft_evaluate(nn)))
	    continue;
	if (!vecs)
	    vecs = lv = v;
	else
	    lv->v_link2 = v;
	for (lv = v; lv->v_link2; lv = lv->v_link2)
	    ;
    }

    if (!vecs) return;

    /* See whether we really have to print plot names. */
    for (v = vecs; v; v = v->v_link2)
        if (vecs->v_plot != v->v_plot) {
            plotnames = true;
            break;
        }

    if (!optgiven) {
        /* Figure out whether col or line should be used... */
        col = false;
        for (v = vecs; v; v = v->v_link2)
            if (v->v_length > 1) {
                col = true;
                break;
            }
    }

    out_init();
    if (!col) {
        for (v = vecs; v; v = v->v_link2) {
            if (plotnames) {
                sprintf(buf, "%s.%s", v->v_plot->pl_typename, vec_basename(v));
            } else {
                strcpy(buf, vec_basename(v));
            }
            for (s = buf; *s; s++) ;
            s--;
            while (isspace(*s)) *s-- = '\0';
            ll = 10;
	    out_send(buf);
	    out_send(" = ");
            if (v->v_length == 1) {
                if (isreal(v)) {
                    out_send(printnum(*v->v_realdata));
                } else {
                    out_send(printnum(realpart(v->v_compdata)));
		    out_send(",");
                    out_send(printnum(imagpart(v->v_compdata)));
                }
		out_send("\n");
            } else {
                out_send("( ");
                for (i = 0; i < v->v_length; i++)
                    if (isreal(v)) {
                        s = printnum(v->v_realdata[i]);
                        out_send(s);
                        ll += strlen(s);
                        ll = (ll + 7) / 8;
                        ll = ll * 8 + 1;
                        if (ll > 60) {
                            out_send("\n\t");
                            ll = 9;
                        } else
                            out_send("\t");
                    } else {
			s = printnum(realpart(&v->v_compdata[i]));
                        out_send(s);
			out_send(",");
                        ll += strlen(s) + 1;
			s = printnum(imagpart(&v->v_compdata[i]));
                        out_send(s);
                        ll += strlen(s);
                        ll = (ll + 7) / 8;
                        ll = ll * 8 + 1;
                        if (ll > 60) {
                            out_send("\n\t");
                            ll = 9;
                        } else
                            out_send("\t");
                    }
                out_send(")\n");
            }
        }
    } else {    /* Print in columns. */
        if (cp_getvar("width", VT_NUM, (char *) &i)) width = i;
        if (width < 40) width = 40;
        if (cp_getvar("height", VT_NUM, (char *) &i)) height = i;
        if (height < 20) height = 20;
        if (!cp_getvar("nobreak", VT_BOOL, (char *) &nobreak) && !ft_nopage)
            nobreak = false;
	else
	    nobreak = true;
        (void) cp_getvar("noprintscale", VT_BOOL, (char *) &noprintscale);
        bv = vecs;
nextpage:
        /* Make the first vector of every page be the scale... */
	/* XXX But what if there is no scale?  e.g. op, pz */
        if (!noprintscale && bv->v_plot->pl_ndims) {
            if (bv->v_plot->pl_scale && !vec_eq(bv, bv->v_plot->pl_scale)) {
                nv = vec_copy(bv->v_plot->pl_scale);
                vec_new(nv);
                nv->v_link2 = bv;
                bv = nv;
            }
        }
        ll = 8;
        for (lv = bv; lv; lv = lv->v_link2) {
            if (isreal(lv))
                ll += 16;   /* Two tabs for real, */
            else
                ll += 32;   /* 4 for complex. */
            /* Make sure we have at least 2 vectors per page... */
            if ((ll > width) && (lv != bv) && (lv != bv->v_link2))
                break;
        }

        /* Print the header on the first page only. */
        p = bv->v_plot;
        j = (width - (int) strlen(p->pl_title)) / 2;	/* Yes, keep "(int)" */
	if (j < 0) j = 0;
        for (i = 0; i < j; i++) buf2[i] = ' ';
        buf2[j] = '\0';
        out_send(buf2);
        out_send(p->pl_title);
        out_send("\n");
        out_send(buf2);
        sprintf(buf, "%s  %s", p->pl_name, p->pl_date);
        j = (width - strlen(buf)) / 2;
        out_send(buf);
        out_send("\n");
        for (i = 0; i < width; i++) buf2[i] = '-';
        buf2[width] = '\n';
        buf2[width+1] = '\0';
        out_send(buf2);
        sprintf(buf, "Index   ");
        for (v = bv; v && (v != lv); v = v->v_link2) {
            if (isreal(v))
                sprintf(buf2, "%-16.15s", v->v_name);
            else
                sprintf(buf2, "%-32.31s", v->v_name);
            strcat(buf, buf2);
        }
        lineno = 3;
        j = 0;
        npoints = 0;
        for (v = bv; (v && (v != lv)); v = v->v_link2)
            if (v->v_length > npoints)
                npoints = v->v_length;
pbreak:     /* New page. */
        out_send(buf);
        out_send("\n");
        for (i = 0; i < width; i++)
            buf2[i] = '-';
        buf2[width] = '\n';
        buf2[width+1] = '\0';
        out_send(buf2);
        lineno += 2;
loop:
        while ((j < npoints) && (lineno < height)) {
	    sprintf(out_pbuf, "%d\t", j);
	    out_send(out_pbuf);
            for (v = bv; (v && (v != lv)); v = v->v_link2) {
                if (v->v_length <= j) {
                    if (isreal(v))
                        out_send("\t\t");
                    else
                        out_send("\t\t\t\t");
                } else {
                    if (isreal(v)) {
                        sprintf(out_pbuf, "%e\t", v->v_realdata[j]);
			out_send(out_pbuf);
                    } else {
                        sprintf(out_pbuf, "%e,\t%e\t",
                        	realpart(&v->v_compdata[j]),
                        	imagpart(&v->v_compdata[j]));
			out_send(out_pbuf);
		    }
                }
            }
            out_send("\n");
            j++;
            lineno++;
        }
        if ((j == npoints) && (lv == NULL)) /* No more to print. */
            goto done;
        if (j == npoints) { /* More vectors to print. */
            bv = lv;
            out_send("\f\n");   /* Form feed. */
            goto nextpage;
        }

        /* Otherwise go to a new page. */
        lineno = 0;
        if (nobreak)
            goto loop;
        else
            out_send("\f\n");   /* Form feed. */
        goto pbreak;
    }
done:
    /* Get rid of the vectors. */
    return;
}

/* Write out some data. write filename expr ... Some cleverness here is
 * required.  If the user mentions a few vectors from various plots,
 * probably he means for them to be written out seperate plots.  In any
 * case, we have to be sure to write out the scales for everything we
 * write...
 */
void com_write(wordlist *wl)
{
    char *file, buf[BSIZE_SP];
    struct pnode *n;
    struct dvec *d, *vecs = NULL, *lv = NULL, *end, *vv;
    static wordlist all = { "all", NULL, NULL } ;
    struct pnode *names;
    bool ascii = (*AsciiRawFile != '0');
    bool scalefound, appendwrite;
    struct plot *tpl, newplot;

    if (wl) {
        file = wl->wl_word;
        wl = wl->wl_next;
    } else
        file = ft_rawfile;
    if (cp_getvar("filetype", VT_STRING, buf)) {
        if (eq(buf, "binary"))
            ascii = false;
        else if (eq(buf, "ascii"))
            ascii = true;
	else
            fprintf(cp_err, "Warning: strange file type %s\n", buf);
    }
    (void) cp_getvar("appendwrite", VT_BOOL, (char *) &appendwrite);

    if (wl)
        names = ft_getpnames(wl, true);
    else
        names = ft_getpnames(&all, true);
    if (names == NULL)
        return;
    for (n = names; n; n = n->pn_next) {
        d = ft_evaluate(n);
        if (!d)
            return;
        if (vecs)
            lv->v_link2 = d;
        else
            vecs = d;
        for (lv = d; lv->v_link2; lv = lv->v_link2)
            ;
    }

    /* Now we have to write them out plot by plot. */

    while (vecs) {
        tpl = vecs->v_plot;
        tpl->pl_written = true;
        end = NULL;
        bcopy((char *) tpl, (char *) &newplot, sizeof(struct plot));
        scalefound = false;

        /* Figure out how many vectors are in this plot. Also look
         * for the scale, or a copy of it, which may have a different
         * name.
         */
        for (d = vecs; d; d = d->v_link2) {
            if (d->v_plot == tpl) {
                vv = vec_copy(d);
                /* Note that since we are building a new plot
                 * we don't want to vec_new this one...
                 */
                vv->v_name = vec_basename(vv);

                if (end)
                    end->v_next = vv;
                else
                    end = newplot.pl_dvecs = vv;
                end = vv;

                if (vec_eq(d, tpl->pl_scale)) {
                    newplot.pl_scale = vv;
                    scalefound = true;
                }
            }
        }
        end->v_next = NULL;

        /* Maybe we shouldn't make sure that the default scale is
         * present if nobody uses it.
         */
        if (!scalefound) {
            newplot.pl_scale = vec_copy(tpl->pl_scale);
            newplot.pl_scale->v_next = newplot.pl_dvecs;
            newplot.pl_dvecs = newplot.pl_scale;
        }

        /* Now let's go through and make sure that everything that
         * has its own scale has it in the plot.
         */
        for (;;) {
            scalefound = false;
            for (d = newplot.pl_dvecs; d; d = d->v_next) {
                if (d->v_scale) {
                    for (vv = newplot.pl_dvecs; vv; vv = vv->v_next)
                        if (vec_eq(vv, d->v_scale))
                            break;
                    /* We have to grab it... */
                    vv = vec_copy(d->v_scale);
                    vv->v_next = newplot.pl_dvecs;
                    newplot.pl_dvecs = vv;
                    scalefound = true;
                }
            }
            if (!scalefound)
                break;
            /* Otherwise loop through again... */
        }

        if (ascii)
            raw_write(file, &newplot, appendwrite, false);
        else
            raw_write(file, &newplot, appendwrite, true);

        /* Now throw out the vectors we have written already... */
        for (d = vecs, lv = NULL;  d; d = d->v_link2)
            if (d->v_plot == tpl) {
                if (lv) {
                    lv->v_link2 = d->v_link2;
                    d = lv;
                } else
                    vecs = d->v_link2;
            } else
                lv = d;
        /* If there are more plots we want them appended... */
        appendwrite = true;
    }
}

/* If the named vectors have more than 1 dimension, then consider
 * to be a collection of one or more matrices.  This command transposes
 * each named matrix.
 */
void com_transpose(wordlist *wl)
{
    struct dvec *d;
    char *s;

    while (wl) {
        s = cp_unquote(wl->wl_word);
        d = vec_get(s);
	txfree(s);
        if (!d) fprintf(cp_err, "Error: no such vector as %s.\n", wl->wl_word);
        else
            while (d) {
                vec_transpose(d);
                d = d->v_link2;
            }
        wl = wl->wl_next;
    }
}

/* Set the default scale to the named vector.  If no vector named,
 * find and print the default scale.
 */
void com_setscale(wordlist *wl)
{
    struct dvec *d;
    char *s;

    if (plot_cur) {
	if (wl) {
	    s = cp_unquote(wl->wl_word);
	    d = vec_get(s);
	    txfree(s);
	    if (!d) fprintf(cp_err, "Error: no such vector as %s.\n", wl->wl_word);
	    else
		plot_cur->pl_scale = d;
	} else if (plot_cur->pl_scale) {
	    out_init();
	    pvec(plot_cur->pl_scale);
	}
    } else {
	fprintf(cp_err, "Error: no current plot.\n");
    }
}

/* Display vector status, etc.  Note that this only displays stuff from the
 * current plot, and you must do a setplot to see the rest of it.
 */
void com_display(wordlist *wl)
{
    struct dvec *d;
    struct dvec **dvs;
    int len = 0, i = 0;
    bool b;
    char *s;

    /* Maybe he wants to know about just a few vectors. */

    out_init();
    while (wl) {
        s = cp_unquote(wl->wl_word);
        d = vec_get(s);
	txfree(s);
        if (!d) fprintf(cp_err, "Error: no such vector as %s.\n", wl->wl_word);
        else
            while (d) {
                pvec(d);
                d = d->v_link2;
            }
        if (wl->wl_next == NULL) return;
        wl = wl->wl_next;
    }
    if (plot_cur)
        for (d = plot_cur->pl_dvecs; d; d = d->v_next) len++;
    if (len == 0) {
	out_send("There are no vectors currently active.\n");
        return;
    }
    out_send("Here are the vectors currently active:\n\n");
    dvs = allocn(struct dvec *, len);
    for (d = plot_cur->pl_dvecs, i = 0; d; d = d->v_next, i++)
        dvs[i] = d;
    if (!cp_getvar("nosort", VT_BOOL, (char *) &b))
        qsort(dvs, len, sizeof(struct dvec *), dcomp);

    out_printf("Title: %s\n",  plot_cur->pl_title);
    out_printf("Name: %s (%s)\n", plot_cur->pl_typename, plot_cur->pl_name);
    out_printf("Date: %s\n\n", plot_cur->pl_date);
    for (i = 0; i < len; i++) {
        d = dvs[i];
        pvec(d);
    }
}

static void pvec (struct dvec *d)
{
    char buf[BSIZE_SP];

    sprintf(buf, "    %-20s: %s, %s, %d long", d->v_name,
	ft_typenames(d->v_type), isreal(d) ? "real" : "complex", d->v_length);
    out_send(buf);

    if (d->v_flags & VF_MINGIVEN) {
        sprintf(buf, ", min = %lg", d->v_minsignal);
	out_send(buf);
    }
    if (d->v_flags & VF_MAXGIVEN) {
        sprintf(buf, ", max = %lg", d->v_maxsignal);
	out_send(buf);
    }

    switch (d->v_gridtype) {
    case GRID_LOGLOG:
        out_send(", grid = loglog");
        break;
    case GRID_XLOG:
        out_send(", grid = xlog");
        break;
    case GRID_YLOG:
        out_send(", grid = ylog");
        break;
    case GRID_POLAR:
        out_send(", grid = polar");
        break;
    case GRID_SMITH:
        out_send(", grid = smith (xformed)");
        break;
    case GRID_SMITHGRID:
        out_send(", grid = smithgrid (not xformed)");
        break;
    }

    switch (d->v_plottype) {
    case PLOT_COMB:
        out_send(", plot = comb");
        break;
    case PLOT_POINT:
        out_send(", plot = point");
        break;
    }

    if (d->v_defcolor) {
        sprintf(buf, ", color = %s", d->v_defcolor);
        out_send(buf);
    }
    if (d->v_scale) {
        sprintf(buf, ", scale = %s", d->v_scale->v_name);
        out_send(buf);
    }
    if (d->v_numdims > 1) {
	sprintf(buf, ", dims = [%s]", dimstring(d->v_dims, d->v_numdims));
        out_send(buf);
    }
    if (d->v_plot->pl_scale == d) {
        out_send(" [default scale]\n");
    } else {
        out_send("\n");
    }
}

#ifdef notdef

/* Set the current working plot. */

void com_splot(wordlist *wl)
{
    struct plot *p;
    char buf[BSIZE_SP], *s;

    if (wl == NULL) {
        fprintf(cp_out, "\tType the name of the desired plot:\n\n");
        fprintf(cp_out, "\tnew\tNew plot\n");
        for (p = plot_list; p; p = p->pl_next) {
            if (plot_cur == p) fprintf(cp_out, "Current");
            fprintf(cp_out, "\t%s\t%s (%s)\n", p->pl_typename, p->pl_title, p->pl_name);
        }
        fprintf(cp_out, "? ");
        fflush(cp_out);
        (void) fgets(buf, BSIZE_SP, cp_in);
        clearerr(cp_in);
        for (s = buf; *s && !isspace(*s); s++) ;
        *s = '\0';
    } else {
        strcpy(buf, wl->wl_word);
    }
    if (prefix("new", buf)) {
        p = plot_alloc("unknown");
        p->pl_title = copy("Anonymous");
        p->pl_name = copy("unknown");
        p->pl_next = plot_list;
        plot_list = p;
    } else {
        for (p = plot_list; p; p = p->pl_next)
            if (plot_prefix(buf, p->pl_typename)) break;
        if (!p) {
            fprintf(cp_err, "Error: no such plot.\n");
            return;
        }
    }
    plot_cur->pl_ccom = cp_kwswitch(CT_VECTOR, p->pl_ccom);
    plot_cur = p;
    plot_docoms(plot_cur->pl_commands);
    if (wl)
        fprintf(cp_out, "%s %s (%s)\n", p->pl_typename, p->pl_title, p->pl_name);
}

#endif

/* For the sort in display. */

static int dcomp (struct dvec **v1, struct dvec **v2)
{
    return (strcmp((*v1)->v_name, (*v2)->v_name));
}

#ifdef notdef

/* Figure out what the name of this vector should be (if it is a number,
 * then make it 'V' or 'I')... Note that the data is static.
 */

static char * dname (struct dvec *d)
{
    static char buf[128];
    char *s;

    for (s = d->v_name; *s; s++)
        if (!isdigit(*s))
            return (d->v_name);
    switch (d->v_type) {
        case SV_VOLTAGE:
            sprintf(buf, "V(%s)", d->v_name);
            return (buf);
        case SV_CURRENT:
            sprintf(buf, "I(%s)", d->v_name);
            return (buf);
    }
    return (d->v_name);
}

#endif

/* Take a set of vectors and form a new vector of the nth elements of each. */

void com_cross(wordlist *wl)
{
    char *newvec, *s;
    struct dvec *n, *v, *vecs = NULL, *lv = NULL;
    struct pnode *pn;
    int i, ind;
    bool comp = false;
    double *d;

    newvec = wl->wl_word;
    wl = wl->wl_next;
    s = wl->wl_word;
    if (!(d = ft_numparse(&s, false))) {
        fprintf(cp_err, "Error: bad number %s\n", wl->wl_word);
        return;
    }
    if ((ind = *d) < 0) {
        fprintf(cp_err, "Error: bad index %d\n", ind);
        return;
    }
    wl = wl->wl_next;
    pn = ft_getpnames(wl, true);
    while (pn) {
        if (!(n = ft_evaluate(pn))) return;
        if (!vecs)
            vecs = lv = n;
        else
            lv->v_link2 = n;
        for (lv = n; lv->v_link2; lv = lv->v_link2) ;
        pn = pn->pn_next;
    }
    for (n = vecs, i = 0; n; n = n->v_link2) {
        if (iscomplex(n)) comp = true;
        i++;
    }

    vec_remove(newvec);
    v = alloc(struct dvec);
    v->v_name = copy(newvec);
    v->v_type = vecs ? vecs->v_type : SV_NOTYPE;
    v->v_length = i;
    v->v_flags |= VF_PERMANENT;
    v->v_flags = comp ? VF_COMPLEX : VF_REAL;
    if (comp)
        v->v_compdata = allocn(complex, i);
    else
        v->v_realdata = allocn(double, i);

    /* Now copy the ind'ths elements into this one. */
    for (n = vecs, i = 0; n; n = n->v_link2, i++)
        if (n->v_length > ind) {
            if (comp) {
                realpart(&v->v_compdata[i]) = realpart(&n->v_compdata[ind]);
                imagpart(&v->v_compdata[i]) = imagpart(&n->v_compdata[ind]);
            } else
                v->v_realdata[i] = n->v_realdata[ind];
        } else {
            if (comp) {
                realpart(&v->v_compdata[i]) = 0.0;
                imagpart(&v->v_compdata[i]) = 0.0;
            } else
                v->v_realdata[i] = 0.0;
        }
    vec_new(v);
    v->v_flags |= VF_PERMANENT;
    cp_addkword(CT_VECTOR, v->v_name);
}

void com_destroy(wordlist *wl)
{
    struct plot *pl, *npl = NULL;

    if (!wl)
        killplot(plot_cur);
    else if (eq(wl->wl_word, "all")) {
        for (pl = plot_list; pl; pl = npl) {
            npl = pl->pl_next;
            if (!eq(pl->pl_typename, "const"))
                killplot(pl);
        }
    } else {
        while (wl) {
            for (pl = plot_list; pl; pl = pl->pl_next)
                if (eq(pl->pl_typename, wl->wl_word)) break;
            if (pl)
                killplot(pl);
            else
                fprintf(cp_err, "Error: no such plot %s\n", wl->wl_word);
            wl = wl->wl_next;
        }
    }
}

static void killplot(struct plot *pl)
{
    struct dvec *v, *nv = NULL;
    struct plot *op;

    if (eq(pl->pl_typename, "const")) {
        fprintf(cp_err, "Error: can't destroy the constant plot\n");
        return;
    }
    for (v = pl->pl_dvecs; v; v = nv) {
        nv = v->v_next;
        vec_free(v);
    }
    if (pl == plot_list) {
        plot_list = pl->pl_next;
        if (pl == plot_cur)
            plot_cur = plot_list;
    } else {
        for (op = plot_list; op; op = op->pl_next)
            if (op->pl_next == pl) break;
        if (!op)
            fprintf(cp_err, "Internal Error: kill plot -- not in list\n");
        op->pl_next = pl->pl_next;
        if (pl == plot_cur) plot_cur = op;
    }
    tfree(pl->pl_title);
    tfree(pl->pl_name);
    tfree(pl->pl_typename);
    wl_free(pl->pl_commands);

    /* Never mind about the rest... */

    return;
}

void com_splot(wordlist *wl)
{
    struct plot *pl;
    char buf[BSIZE_SP], *s, *t;

    if (wl) {
        plot_setcur(wl->wl_word);
        return;
    }
    fprintf(cp_out, "\tType the name of the desired plot:\n\n");
    fprintf(cp_out, "\tnew\tNew plot\n");
    for (pl = plot_list; pl; pl = pl->pl_next)
        fprintf(cp_out, "%s%s\t%s (%s)\n",
                (pl == plot_cur) ? "Current " : "\t",
                pl->pl_typename, pl->pl_title, pl->pl_name);

    fprintf(cp_out, "? ");
    if (!fgets(buf, BSIZE_SP, cp_in)) {
        clearerr(cp_in);
        return;
    }
    t = buf;
    if (!(s = gettok(&t))) return;

    plot_setcur(s);
}
