/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Do alias substitution.
 */

#include "spice.h"
#include "cpdefs.h"

struct alias *cp_aliases = NULL;

static struct histent LASTONE;

wordlist * cp_doalias (wordlist *wlist)
{
    struct alias *al;
    wordlist *realw = NULL, *nwl, *nextc, *end, *comm, *w;
    char *word;
    int ntries, eqw;

    if (cp_lastone) realw = cp_lastone->hi_wlist;
    else cp_lastone = &LASTONE;

    /* The alias process is going to modify the "last" line typed,
     * so save a copy of what it really is and restore it after
     * aliasing is done.
     */
    comm = wlist;
    do {
	while (comm && eq(comm->wl_word, ";")) comm = comm->wl_next;
	if (!comm) break;

        end = comm->wl_prev;
        comm->wl_prev = NULL;
        for (nextc = comm; nextc; nextc = nextc->wl_next)
            if (eq(nextc->wl_word, ";")) {
                if (nextc->wl_prev) nextc->wl_prev->wl_next = NULL;
                break;
            }

        for (ntries = 21; ntries; ntries--) {

	    word = comm->wl_word;
	    if (strip(*word) == '\\') { comm->wl_word++; break; }

	    for (al = cp_aliases; al; al = al->al_next)
		if (eq(word, al->al_name)) break;
	    if (!al) break; /* no alias found */

	    /* We can call cp_histsubst because the line will have
	     * gone onto the history list by now and cp_histsubst
	     * will look in the right place.
	     */
	    cp_lastone->hi_wlist = comm;
	    nwl = cp_histsubst(wl_copy(al->al_text));
	    if (!nwl || !nwl->wl_word) return (nwl);
	    if (!cp_didhsubst) {
		/* Append the rest to the nwl. */
		for (w = nwl; w->wl_next; w = w->wl_next);
		w->wl_next = wl_copy(comm->wl_next);
		if (w->wl_next) w->wl_next->wl_prev = w;
	    }
	    eqw = eq(nwl->wl_word, word);
	    wl_free(comm); comm = nwl;
            if (eqw) break; /* Just once through... */
        }
        if (!ntries) {
            fprintf(cp_err, "Error: alias loop.\n");
	    tfree(wlist->wl_word);
            return (wlist);
        }

        comm->wl_prev = end;
        if (!end) wlist = comm;
        else end->wl_next = comm;

        if (nextc) {
	    while (comm->wl_next) comm = comm->wl_next;
	    comm->wl_next = nextc;
	    nextc->wl_prev = comm;
            nextc = nextc->wl_next;
            comm = nextc;
        }
    } while (nextc);

    if (cp_lastone == &LASTONE) cp_lastone = NULL;
    else cp_lastone->hi_wlist = realw;

    return (wlist);
}

/* If we use this, aliases will be in alphabetical order. */

void cp_setalias (char *word, wordlist *wlist)
{
    struct alias *al, *ta;

    cp_unalias(word);
    cp_addkword(CT_ALIASES, word);
    if (cp_aliases == NULL) {
        /* printf("first one...\n"); */
        al = cp_aliases = alloc(struct alias);
	al->al_next = NULL;
	al->al_prev = NULL;
    } else {
        /* printf("inserting %s: %s ...\n", word, wlist->wl_word); */
        for (al = cp_aliases; al->al_next; al = al->al_next) {
            /* printf("checking %s...\n", al->al_name); */
            if (strcmp(al->al_name, word) > 0) break;
        }
        /* The new one goes before al */
        if (al->al_prev) {
            al = al->al_prev;
            ta = al->al_next;
            al->al_next = alloc(struct alias);
            al->al_next->al_prev = al;
            al = al->al_next;
            al->al_next = ta;
            ta->al_prev = al;
        } else {
            cp_aliases = alloc(struct alias);
            cp_aliases->al_next = al;
            cp_aliases->al_prev = NULL;
            al->al_prev = cp_aliases;
            al = cp_aliases;
        }
    }
    al->al_name = copy(word);
    al->al_text = wl_copy(wlist);
    cp_striplist(al->al_text);
    cp_addcomm(word, 1, 1, 1, 1);
    /* printf("word %s, next = %s, prev = %s...\n", al->al_name,
            al->al_next ? al->al_next->al_name : "(none)",
            al->al_prev ? al->al_prev->al_name : "(none)"); */
}

void cp_unalias (char *word)
{
    struct alias *al;

    cp_remkword(CT_ALIASES, word);
    for (al = cp_aliases; al; al = al->al_next)
        if (eq(word, al->al_name)) break;
    if (!al) return;
    if (al->al_next)
        al->al_next->al_prev = al->al_prev;
    if (al->al_prev)
        al->al_prev->al_next = al->al_next;
    else {
        al->al_next->al_prev = NULL;
        cp_aliases = al->al_next;
    }
    wl_free(al->al_text);
    tfree(al->al_name);
    tfree(al);
    cp_remcomm(word);
}

void cp_paliases (char *word)
{
    struct alias *al;

    for (al = cp_aliases; al; al = al->al_next)
        if (!word || eq(al->al_name, word)) {
            if (!word) fprintf(cp_out, "%s\t", al->al_name);
            wl_print(al->al_text, cp_out);
            (void) putc('\n', cp_out);
        }
}

/* The routine for the "alias" command. */

void com_alias (wordlist *wl)
{
    if (!wl)
        cp_paliases((char *) NULL);
    else if (wl->wl_next == NULL)
        cp_paliases(wl->wl_word);
    else
        cp_setalias(wl->wl_word, wl->wl_next);
}

void com_unalias (wordlist *wl)
{
    struct alias *al, *na;

    if (!wl) return;

    if (eq(wl->wl_word, "*")) {
        for (al = cp_aliases; al; al = na) {
            na = al->al_next;
            wl_free(al->al_text);
            tfree(al->al_name);
            tfree(al);
        }
        cp_aliases = NULL;
        wl = wl->wl_next;
    }
    while (wl) {
        cp_unalias(wl->wl_word);
        wl = wl->wl_next;
    }
}

