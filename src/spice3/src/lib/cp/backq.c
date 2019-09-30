/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Do backquote substitution on a word list.
 */

#include "spice.h"
#include "cpdefs.h"

static wordlist *backeval();

wordlist * cp_bquote (wordlist *wlist)
{
    wordlist *wl, *nwl;
    char *s, *t, *w, buf[BSIZE_SP];
    int i;

    for (wl = wlist; wl; wl = wl->wl_next) {
        t = wl->wl_word;
        while (t && (s = strchr(t, '`'))) {
	    *s++ = '\0';
	    t = s;
	    while (*t && *t != '`') t++; /* Get t past the next backquote. */
	    i = 0;
	    if (*t) *t++ = '\0';
	    else { fprintf(cp_err, "Error: missing '`'.\n"); ++i; }

	    if (i || !(nwl = backeval(s))) { tfree(wlist->wl_word); return (wlist); }

	    w = wl->wl_word;
	    strcpy(buf, w);
	    if (nwl->wl_word) {
		strcat(buf, nwl->wl_word);
		txfree(nwl->wl_word);
	    }
	    nwl->wl_word = copy(buf);

	    if (wlist == wl) wlist = nwl;
	    wl->wl_word = NULL; /* SdG, don't free w, because of t */
	    wl = wl_splice(wl, nwl);

	    if (*t) {
		strcpy(buf, wl->wl_word); i = strlen(buf);
		strcat(buf, t);
		txfree(wl->wl_word);
		wl->wl_word = copy(buf);
		t = wl->wl_word + i;
	    }
	    else t = NULL;
	    txfree(w); /* SdG, free old wl word */
	}
    }
    return (wlist);
}

/* Do a popen with the string, and then reset the file pointers so that
 * we can use the first pass of the parser on the output.
 */
static wordlist * backeval (char *string)
{
    FILE *proc, *old;
    wordlist *wl;
    bool intv;

    if (!(proc = popen(string, "r"))) {
        fprintf(cp_err, "Error: can't evaluate `%s`.\n", string);
        return (NULL);
    }
    old = cp_inp_cur;
    cp_inp_cur = proc;
    intv = cp_interactive;
    cp_interactive = false;
    cp_bqflag = true;
    wl = cp_lexer((char *) NULL);
    cp_bqflag = false;
    cp_inp_cur = old;
    cp_interactive = intv;
    pclose(proc);
    return (wl);
}
