/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Do history substitutions.
 */

#include "spice.h"
#include "cpdefs.h"

static char *dohs();
static wordlist *dohmod();
static wordlist *dohsubst();
static wordlist *hpattern();
static wordlist *hprefix();

struct histent *cp_lastone = NULL;
int cp_maxhistlength = 100;
bool cp_didhsubst;
bool cp_didhsubstp;

static struct histent *histlist = NULL;
static int histlength = 0;

/* First check for a ^ at the beginning
 * of the line, and then search each word for !. Following this can be any
 * of string, number, ?string, -number ; then there may be a word specifier,
 * the same as csh, and then the : modifiers. For the :s modifier,
 * the syntax is :sXoooXnnnX, where X is any character, and ooo and nnn are
 * strings not containing X.
 */
wordlist * cp_histsubst(wordlist *wlist)
{
    wordlist *wl, *nwl;
    char buf[BSIZE_SP], *s, *t;

    cp_didhsubst = false;
    cp_didhsubstp = false;

    if (*wlist->wl_word == '^') {
	/* replace ^old^new with !!:s^old^new */
        sprintf(buf, "!!:s%s", wlist->wl_word);
        txfree(wlist->wl_word);
        wlist->wl_word = copy(buf);
    }

    for (wl = wlist; wl; wl = wl->wl_next) {
	t = wl->wl_word;
	if ((s = strchr(t, '!'))) {
	    if (*++s) {
		cp_didhsubst = true;
		nwl = dohsubst(s);
		if (!nwl) { tfree(wlist->wl_word); return (wlist); }
		if (--s > t) {
		    sprintf(buf, "%.*s%s", s - t, t, nwl->wl_word);
		    txfree(nwl->wl_word);
		    nwl->wl_word = copy(buf);
		}
		if (wlist == wl) wlist = nwl;
		wl = wl_splice(wl, nwl);
	    }
	}
    }
    return (wlist);
}

/* Do a history substitution on one word. Figure out which event is
 * being referenced, then do word selections and modifications, and
 * then stick anything left over on the end of the last word.
 */
static wordlist * dohsubst(char *string)
{
    struct histent *hi;
    wordlist *wl = NULL, *nwl;
    char buf[BSIZE_SP], *r, *s, *t;
    int i = 0;

    if (*string == '!') {
	++string;
	if (cp_lastone) wl = cp_lastone->hi_wlist;
    }
    else if (*string == '?') {
	if ((r = strchr(++string, '?'))) *r = '\0';
	if (!(wl = hpattern(string))) return (NULL);
	if (r) *r++ = '?';
	if (!r || !*r) return (wl_copy(wl)); /* No modifiers */
	string = r;
    }
    else if (*string == '-' || isdigit(*string)) {
	if (*string == '-')
	    i = cp_event - scannum(++string);
	else
	    i = scannum(string);
	while (isdigit(*string)) ++string;

	for (hi = histlist; hi; hi = hi->hi_next)
	    if (hi->hi_event == i) break;
	if (hi) wl = wl_copy(hi->hi_wlist);
    } else {
	r = NULL;
	for (s = ":^$*-%"; *s; s++) {
	    t = strchr(string, *s);
	    if (t && (!r || t < r)) r = t;
	}
	if (r) { i = *r; *r = '\0'; } /* modifier found */

	if (r && !*string && cp_lastone) wl = cp_lastone->hi_wlist;
	else if (!(wl = hprefix(string))) return (NULL);

	if (r) *r = i;
	else return (wl_copy(wl)); /* No modifiers */
	string = r;
    }
    if (!wl) { fprintf(cp_err, "%d: Event not found.\n", i); return (NULL); }

    nwl = dohmod(&string, wl_copy(wl));

    if (nwl && *string) {
        for (wl = nwl; wl->wl_next; wl = wl->wl_next) ;
        sprintf(buf, "%s%s", wl->wl_word, string);
        txfree(wl->wl_word);
        wl->wl_word = copy(buf);
    }
    return (nwl);
}

static wordlist * dohmod(char **string, wordlist *wl)
{
    wordlist *w;
    char *r, *s, *t;
    int numwords, eventlo, eventhi, i;
    bool globalsubst;

anothermod:
    numwords = wl_length(wl);
    globalsubst = false;
    eventlo = 0;
    eventhi = numwords - 1;

    /* Now we know what wordlist we want. Take care of modifiers now. */
    r = NULL;
    for (s = ":^$*-%"; *s; s++) {
        t = strchr(*string, *s);
        if (t && (!r || t < r)) r = t;
    }
    if (!r) return (wl); /* No more modifiers. */

    if (*r == ':') r++;

    switch(*r++) {
        case '$':   /* Last word. */
            eventlo = eventhi;
            break;
        case '*':   /* Words 1 through $ */
            if (numwords == 1) return (NULL);
            eventlo = 1;
            break;
        case '-':   /* Words 0 through ... */
	    if (isdigit(*r)) {
		if ((i = scannum(r)) < eventhi) eventhi = i;
		while (isdigit(*r)) r++;
	    }
	    else if (*r == '$') r++;
	    else if (eventhi > eventlo) --eventhi;
            break;
        case 'p': /* Print the command and don't execute it. */
	    cp_didhsubstp = true;
            break;
        case 'g':
	    globalsubst = true;
	    if (*r == 's') r++;
        case 's':   /* Do a substitution. */
	    s = t = NULL;
	    if ((i = *r) && (s = strchr(++r, i))) { *s = '\0';
		if ((t = strchr(++s, i))) *t = '\0';
	    }
	    if (*r) {
		char *nw = NULL;
		for (w = wl; w; w = w->wl_next) {
		    if ((nw = dohs(r, s, w->wl_word))) {
			txfree(w->wl_word);
			w->wl_word = nw;
			if (globalsubst == false) break;
		    }
		}
		if (!nw) { fprintf(cp_err, "Modifier failed.\n"); return (NULL); }
	    }
	    else { fprintf(cp_err, "Bad substitute.\n"); return (NULL); }
	    if (s) *--s = i;
	    if (t) { *t = i; r = t+1; }
	    else while (*r) r++;
            break;
        default:
	    r--;
	    i = 1;
	    if (*r == '^') r++;
	    else if (!isdigit(*r)) {
                fprintf(cp_err, "Error: %s: bad modifier.\n", r);
                return (NULL);
            }
	    else {
		i = scannum(r);
		while (isdigit(*r)) r++;
	    }
	    if (i > eventhi) {
badnr:		fprintf(cp_err, "Error: bad event number %d\n", i);
		return (NULL);
	    }
	    eventlo = i;

	    if (*r == '*') r++;
	    else if (*r == '-') { r++;
		if (isdigit(*r)) {
		    i = scannum(r);
		    if (i > eventhi) goto badnr;
		    if (i < eventlo) goto badnr;
		    eventhi = i;
		    while (isdigit(*r)) r++;
		}
		else if (*r == '$') r++;
		else if (eventhi > eventlo) --eventhi;
	    }
	    else eventhi = eventlo;
    }
    *string = r;

    /* Now change the word list accordingly and make another pass
     * if there is more of the substitute left.
     */
    wl = wl_range(wl, eventlo, eventhi);
    if (*r && wl) goto anothermod;
    return (wl);
}

/* Look for an event with a pattern in it... */

static wordlist * hpattern(char *buf)
{
    struct histent *hi;
    wordlist *wl;

    if (!*buf) { fprintf(cp_err, "Bad pattern specification.\n"); return (NULL); }
    for (hi = cp_lastone; hi; hi = hi->hi_prev)
        for (wl = hi->hi_wlist; wl; wl = wl->wl_next)
            if (substring(buf, wl->wl_word)) return (hi->hi_wlist);
    fprintf(cp_err, "%s: Event not found.\n", buf);
    return (NULL);
}

static wordlist * hprefix(char *buf)
{
    struct histent *hi;

    if (!*buf) { fprintf(cp_err, "Bad pattern specification.\n"); return (NULL); }
    for (hi = cp_lastone; hi; hi = hi->hi_prev)
        if (prefix(buf, hi->hi_wlist->wl_word)) return (hi->hi_wlist);
    fprintf(cp_err, "%s: Event not found.\n", buf);
    return (NULL);
}

/* Add a wordlist to the history list. (Done after the first parse.) Note
 * that if event numbers are given in a random order that's how they'll
 * show up in the history list.
 */
void cp_addhistent(int event, wordlist *wlist)
{
    struct histent *hi;

    hi = alloc(struct histent);
    hi->hi_event = event;
    hi->hi_wlist = wl_copy(wlist);
    hi->hi_prev = cp_lastone;
    hi->hi_next = NULL;

    if (!cp_lastone) histlist = hi;
    else cp_lastone->hi_next = hi;
    cp_lastone = hi;
    histlength++;

    if (histlength > cp_maxhistlength) {
	int n = histlength - cp_maxhistlength;
	while (n-- && histlist->hi_next) {
	    hi = histlist;
	    histlist = histlist->hi_next;
	    wl_free(hi->hi_wlist);
	    txfree(hi);
	    histlength--;
	}
	histlist->hi_prev = NULL;
    }
}

/* Print out history between eventhi and eventlo.
 * This doesn't remember quoting, so 'hodedo' prints as hodedo.
 */
void cp_hprint (int eventhi, int eventlo, bool rev)
{
    struct histent *hi = histlist;

    if (rev) {
        if (hi) while (hi->hi_next) hi = hi->hi_next;
        for (; hi; hi = hi->hi_prev)
            if (hi->hi_event <= eventhi && hi->hi_event >= eventlo) {
                fprintf(cp_out, "%d\t", hi->hi_event);
                wl_print(hi->hi_wlist, cp_out);
                putc('\n', cp_out);
            }
    } else {
        for (; hi; hi = hi->hi_next)
            if (hi->hi_event <= eventhi && hi->hi_event >= eventlo) {
                fprintf(cp_out, "%d\t", hi->hi_event);
                wl_print(hi->hi_wlist, cp_out);
                putc('\n', cp_out);
            }
    }
}

/* Do a :s substitution. */

static char * dohs(char *pat, char *new, char *str)
{
    char buf[BSIZE_SP];
    int i = 0;

    while (*str && !(*str == *pat && prefix(pat, str))) buf[i++] = *str++;
    if (*str) {
	if (new) while (*new) buf[i++] = *new++;
	str += strlen(pat);
	while (*str) buf[i++] = *str++;
	buf[i] = '\0';
        return (copy(buf));
    }
    return (NULL);
}

/* The "history" command. history [-r] [number] */

void com_history(wordlist *wl)
{
    bool rev = false;

    if (wl && eq(wl->wl_word, "-r")) {
        wl = wl->wl_next;
        rev = true;
    }
    if (wl == NULL)
        cp_hprint(cp_event - 1, cp_event - histlength, rev);
    else
        cp_hprint(cp_event - 1, cp_event - 1 - atoi(wl->wl_word), rev);
}
