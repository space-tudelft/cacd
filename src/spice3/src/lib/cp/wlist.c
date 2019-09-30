/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Wordlist manipulation stuff.
 */

#include "spice.h"
#include "cpdefs.h"

/* Determine the length of a word list. */

int wl_length (wordlist *wl)
{
    int i = 0;
    for (; wl; wl = wl->wl_next) i++;
    return (i);
}

/* Free the storage used by a word list. */

void wl_free (wordlist *wl)
{
    wordlist *nw;

    for (; wl; wl = nw) {
        nw = wl->wl_next;
        txfree(wl->wl_word);
        txfree(wl);
    }
}

/* Copy a wordlist and the words. */

wordlist * wl_copy (wordlist *wl)
{
    wordlist *nwl = NULL, *w = NULL;

    for (; wl; wl = wl->wl_next) {
        if (nwl == NULL) {
            nwl = w = alloc(wordlist);
	    w->wl_prev = NULL;
        } else {
            w->wl_next = alloc(wordlist);
            w->wl_next->wl_prev = w;
            w = w->wl_next;
        }
	w->wl_next = NULL;
        w->wl_word = copy(wl->wl_word);
    }
    return (nwl);
}

/* Replace wordlist element 'wl' with 'nwl', and return
 * the last element of the inserted list.
 */
wordlist * wl_splice (wordlist *wl, wordlist *nwl)
{
    nwl->wl_prev = wl->wl_prev;
    if (wl->wl_prev)
        wl->wl_prev->wl_next = nwl;

    while (nwl->wl_next) nwl = nwl->wl_next;

    if (wl->wl_next) {
	nwl->wl_next = wl->wl_next;
        wl->wl_next->wl_prev = nwl;
    }
    txfree(wl->wl_word);
    txfree(wl);
    return (nwl);
}

/* Print a wordlist. (No \n at the end...) */

void wl_print (wordlist *wl, FILE *fp)
{
    char *s;

    for (; wl; wl = wl->wl_next) {
	if ((s = wl->wl_word)) while (*s) { putc(strip(*s), fp); ++s; }
	if (wl->wl_next) putc(' ', fp);
    }
}

/* Turn an array of char *'s into a wordlist. */

wordlist * wl_build (char *v[])
{
    wordlist *nwl = NULL, *wl = NULL, *cwl;

    for (; *v; v++) {
	cwl = alloc(wordlist);
	cwl->wl_prev = wl;
	cwl->wl_next = NULL;
	cwl->wl_word = copy(*v);

	if (wl) wl->wl_next = cwl;
	else nwl = cwl;
	wl = cwl;
    }
    return (nwl);
}

char ** wl_mkvec (wordlist *wl)
{
    int len, i;
    char **v;

    len = wl_length(wl);
    v = allocn(char *, len+1);
    for (i = 0; i < len; i++) {
        v[i] = copy(wl->wl_word);
        wl = wl->wl_next;
    }
    v[i] = NULL;
    return (v);
}

/* Nconc two wordlists together. */

wordlist * wl_append (wordlist *wlist, wordlist *nwl)
{
    wordlist *wl;
    if (wlist == NULL) return (nwl);
    if (nwl == NULL) return (wlist);
    for (wl = wlist; wl->wl_next; wl = wl->wl_next);
    wl->wl_next = nwl;
    nwl->wl_prev = wl;
    return (wlist);
}

/* Reverse a word list. */

wordlist * wl_reverse (wordlist *wl)
{
    wordlist *w, *t;

    for (w = wl; w; w = t) {
         t = w->wl_next;
         w->wl_next = w->wl_prev;
         w->wl_prev = t;
    }
    return (w);
}

/* Convert a wordlist into a string. */

char * wl_flatten (wordlist *wl)
{
    char *buf;
    wordlist *tw;
    int i = 0;

    for (tw = wl; tw; tw = tw->wl_next) {
	i += strlen(tw->wl_word) + 1;
    }
    buf = allocn(char, i+1);
    *buf = 0;

    while (wl) {
        strcat(buf, wl->wl_word);
        if (wl->wl_next) strcat(buf, " ");
        wl = wl->wl_next;
    }
    return (buf);
}

/* Return the nth element of a wordlist, or the last one if n is too big.
 * Numbering starts at 0...
 */
wordlist * wl_nthelem (register int i, wordlist *wl)
{
    register wordlist *ww = wl;

    while ((i-- > 0) && ww->wl_next) ww = ww->wl_next;
    return (ww);
}

static int wlcomp (const void *cs, const void *ct)
{
    char **s = (char **)cs;
    char **t = (char **)ct;
    return (strcmp(*s, *t));
}

void wl_sort (wordlist *wl)
{
    register int i;
    register wordlist *ww;
    char **stuff;

    for (i = 0, ww = wl; ww; i++, ww = ww->wl_next) ;
    if (i < 2) return;
    stuff = allocn(char *, i);
    for (i = 0, ww = wl; ww; i++, ww = ww->wl_next) stuff[i] = ww->wl_word;
    qsort(stuff, i, sizeof(char *), wlcomp);
    for (i = 0, ww = wl; ww; i++, ww = ww->wl_next) ww->wl_word = stuff[i];
    txfree(stuff);
}

/* Return a range of wordlist elements... */

wordlist * wl_range (wordlist *wl, int low, int up)
{
    int i;
    wordlist *tt;
    bool rev = false;

    if (low > up) { i = up; up = low; low = i; rev = true; }

    up -= low;
    while (wl && low > 0) {
        tt = wl->wl_next;
        txfree(wl->wl_word);
        txfree(wl);
        wl = tt;
        if (wl) wl->wl_prev = NULL;
        low--;
    }
    tt = wl;
    while (tt && up > 0) {
        tt = tt->wl_next;
        up--;
    }
    if (tt && tt->wl_next) {
        wl_free(tt->wl_next);
        tt->wl_next = NULL;
    }
    if (rev) wl = wl_reverse(wl);
    return (wl);
}
