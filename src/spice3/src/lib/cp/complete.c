/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Command completion code. We keep a data structure with information on each
 * command, to make lookups fast.  We also keep NCLASSES (which is sort of
 * hardwired as 32) sets of keywords. Each command has an array of NARGS
 * bitmasks (also hardwired as 4), stating whether the command takes that
 * particular class of keywords in that position. Class 0 always means
 * filename completion.
 */

#include "spice.h"
#include "util.h"
#include "cpdefs.h"

#include <sys/types.h>
#include <dirent.h>
#ifndef direct
#define direct dirent
#endif

#include <sys/ioctl.h>

/* Be sure the ioctls get included in the following */
#ifdef HAS_BSDTTY
#include <sgtty.h>
#else
#ifdef HAS_SYSVTTY
#include <termio.h>
#else
#ifdef HAS_POSIXTTY
#include <termios.h>
#endif
#endif
#endif

#define	CNTRL_D	'\004'
#define	ESCAPE	'\033'
#define NCLASSES 32
#define NARGS 4

static wordlist *ccfilec();
static wordlist *ccmatch();
static struct ccom *getccom();
static void printem();
static wordlist *cctowl();
static void cdelete();
static struct ccom *clookup();
static void throwaway();

bool cp_nocc;               /* Don't do command completion. */

/* The data structure for the commands is as follows: every node has a pointer
 * to its leftmost child, where the children of a node are those of which
 * the node is a prefix. This means that for a word like "ducks", there
 * must be nodes "d", "du", "duc", etc (which are all marked "invalid",
 * of course).  This data structure is called a "trie".
 */

struct ccom {
    char *cc_name;          /* Command or keyword name. */
    int  cc_kwords[NARGS];  /* What this command takes. */
    bool cc_invalid;        /* This node has been deleted. */
    struct ccom *cc_child;  /* Left-most child. */
    struct ccom *cc_sibling;/* Right (alph. greater) sibling. */
    struct ccom *cc_ysibling;/* Left (alph. less) sibling. */
    struct ccom *cc_parent; /* Parent node. */
} ;

static struct ccom *commands = NULL;    /* The available commands. */
static struct ccom *keywords[NCLASSES]; /* Keywords. */

void cp_ccom (wordlist *wlist, char *buf, bool esc)
{
    struct ccom *cc;
    wordlist *a, *pmatches = NULL;
    char wbuf[BSIZE_SP], *s;
    int i, j, arg;

    buf = cp_unquote2(buf);
    cp_wstrip(buf);

    if (wlist->wl_next) {   /* Not the first word. */
        cc = getccom(wlist->wl_word);
        if (cc && cc->cc_invalid) cc = NULL;
        arg = wl_length(wlist) - 2;
        if (arg > 3) arg = 3;
        /* First filenames. */
        if (cc && (cc->cc_kwords[arg] & 1)) {
            pmatches = ccfilec(buf);
            s = strrchr(buf, '/');
            i = strlen(s ? s + 1 : buf);
            if (*buf == '~' && !strchr(buf, '/')) i--;
        } else
            i = 0; /* prevent compiler uninitialized warning */

        /* The keywords. */
        for (j = 1; j < NCLASSES; j++) {
            if (cc && (cc->cc_kwords[arg] & (1 << j))) {
                /* Find all the matching keywords. */
                a = ccmatch(buf, &keywords[j]);
                i = strlen(buf);
                if (pmatches)
                    pmatches = wl_append(pmatches, a);
                else
                    pmatches = a;
            }
        }
        wl_sort(pmatches);
    } else {
        pmatches = ccmatch(buf, &commands);
        i = strlen(buf);
    }
    if (!esc) {
        printem(pmatches);
        wl_free(pmatches);
        return;
    }

    if (pmatches == NULL) {
        putchar('\07');
        fflush(cp_out);
        return;
    }
    if (pmatches->wl_next == NULL) {
        strcpy(wbuf, &pmatches->wl_word[i]);
        goto found;
    }
    /* Now we know which words might work. Extend the command as much
     * as possible, then TIOCSTI the characters out.
     */
    for (j = 0;; j++, i++) {
        wbuf[j] = pmatches->wl_word[i];
        for (a = pmatches->wl_next; a; a = a->wl_next)
                if (a->wl_word[i] != wbuf[j]) {
                    putchar('\07');
                    fflush(cp_out);
                    wbuf[j] = '\0';
                    goto found;
                }
        if (wbuf[j] == '\0') goto found;
    }
found:
    for (i = 0; wbuf[i]; i++)
        (void) ioctl(fileno(cp_in), TIOCSTI, &wbuf[i]);
    wl_free(pmatches);
}

/* Figure out what the command is, given the name. Returns NULL if there
 * is no such command in the command list. This is tricky, because we have
 * to do a preliminary history and alias parse. (Or at least we should.)
 */
static struct ccom * getccom (char *first)
{
    struct alias *al;
    int ntries = 21;

    /* First look for aliases. Just interested in the first word...
     * Don't bother doing history yet -- that might get complicated.
     */
    while (ntries-- > 0) {
        for (al = cp_aliases; al; al = al->al_next)
            if (eq(first, al->al_name)) {
                first = al->al_text->wl_word;
                break;
            }
        if (al == NULL) break;
    }
    if (ntries == 0) {
        fprintf(cp_err, "\nError: alias loop.\n");
        return (NULL);
    }
    return (clookup(first, &commands, false, false));
}

/* Figure out what files match the prefix. */

static wordlist * ccfilec (char *buf)
{
    DIR *wdir;
    char *lcomp, *dir, *ls;
    struct direct *de;
    wordlist *wl = NULL, *t;

    ls = strrchr(buf, '/');
    if (!ls) { dir = "."; lcomp = buf; }
    else {
        dir = buf;
        *ls = '\0';
        lcomp = ls + 1;
	if (*dir == '~') dir = tilde_expand(dir);
	else if (!*dir) dir = "/";
    }

    if ((wdir = opendir(dir))) {
	while ((de = readdir(wdir)))
	    if ((prefix(lcomp, de->d_name)) && (*lcomp || *de->d_name != '.')) {
		t = wl;
		wl = alloc(wordlist);
		wl->wl_prev = NULL;
		wl->wl_next = t;
		if (t) t->wl_prev = wl;
		wl->wl_word = copy(de->d_name);
	    }
	closedir(wdir);
	wl_sort(wl);
    }
    if (ls) {
        *ls = '/';
	if (*buf == '~' && dir != buf) free(dir);
    }
    return (wl);
}

/* See what keywords or commands match the prefix. Check extra also for
 * matches, if it is non-NULL. Return a wordlist which is in alphabetical
 * order. Note that we have to call this once for each class.
 */
static wordlist * ccmatch (char *word, struct ccom **dbase)
{
    wordlist *wl;
    register struct ccom *cc;

    cc = clookup(word, dbase, true, false);
    if (cc) {
        if (*word)  /* This is a big drag. */
            wl = cctowl(cc, false);
        else
            wl = cctowl(cc, true);
    } else
        wl = NULL;
    return (wl);
}

/* Print the words in the wordlist in columns. They are already sorted...
 * This is a hard thing to do with wordlists...
 */
static void printem (wordlist *wl)
{
    wordlist *ww;
    int maxl = 0, num, i, j, k, width = 79, ncols, nlines;

    putchar('\n');
    if (wl == NULL) return;

    num = wl_length(wl);
    for (ww = wl; ww; ww = ww->wl_next) {
        j = strlen(ww->wl_word);
        if (j > maxl)
            maxl = j;
    }
    if (++maxl % 8)
        maxl += 8 - (maxl % 8);
    ncols = width / maxl;
    if (ncols == 0)
        ncols = 1;
    nlines = num / ncols + (num % ncols ? 1 : 0);
    for (k = 0; k < nlines; k++) {
        for (i = 0; i < ncols; i++) {
            j = i * nlines + k;
            if (j < num) {
                fprintf(cp_out, "%-*s", maxl, wl_nthelem(j, wl)->wl_word);
            } else
                break;
        }
        putchar('\n');
    }
}

static wordlist * cctowl (struct ccom *cc, bool sib)
{
    wordlist *wl, *end;

    if (!cc) return (NULL);
    if (!cc->cc_invalid) {
        wl = alloc(wordlist);
        wl->wl_word = copy(cc->cc_name);
        wl->wl_prev = NULL;
        wl->wl_next = cctowl(cc->cc_child, true);
        if (wl->wl_next) wl->wl_next->wl_prev = wl;
    } else
        wl = cctowl(cc->cc_child, true);
    if (sib) {
        if (wl) {
            for (end = wl; end->wl_next; end = end->wl_next) ;
            end->wl_next = cctowl(cc->cc_sibling, true);
            if (end->wl_next) end->wl_next->wl_prev = wl;
        } else
            wl = cctowl(cc->cc_sibling, true);
    }
    return (wl);
}

/* We use this in com_device... */

wordlist * cp_cctowl (char *stuff)
{
    return (cctowl((struct ccom *) stuff, true));
}

/* Turn on and off the escape break character and cooked mode. */

void cp_ccon (bool on)
{
#ifdef HAS_BSDTTY
    static bool ison = false;
    struct tchars tbuf;
    struct sgttyb sbuf;

    if (cp_nocc || !cp_interactive || ison == on) return;
    ison = on;

    /* Set the terminal up -- make escape the break character, and
     * make sure we aren't in raw or cbreak mode.  Hope the ioctl's
     * won't fail.
     */
    (void) ioctl(fileno(cp_in), TIOCGETC, (char *) &tbuf);
    if (on)
        tbuf.t_brkc = ESCAPE;
    else
        tbuf.t_brkc = '\0';
    (void) ioctl(fileno(cp_in), TIOCSETC, (char *) &tbuf);

    (void) ioctl(fileno(cp_in), TIOCGETP, (char *) &sbuf);
    sbuf.sg_flags &= ~(RAW|CBREAK);
    (void) ioctl(fileno(cp_in), TIOCSETP, (char *) &sbuf);
#else
#ifdef HAS_SYSVTTY

#define TERM_GET TCGETA
#define TERM_SET TCSETA
    static struct termio sbuf;
    static struct termio OS_Buf;
#else
#ifdef HAS_POSIXTTY

#define TERM_GET TCGETS
#define TERM_SET TCSETS
    static struct termios sbuf;
    static struct termios OS_Buf;
#endif
#endif

#ifdef TERM_GET
    static bool ison = false;

    if (cp_nocc || !cp_interactive || ison == on) return;
    ison = on;

    if (ison == true) {
	(void) ioctl(fileno(cp_in), TERM_GET, (char *) &OS_Buf);
	sbuf = OS_Buf;
	sbuf.c_cc[VEOF] = 0;
	sbuf.c_cc[VEOL] = ESCAPE;
	sbuf.c_cc[VEOL2] = CNTRL_D;
	(void) ioctl(fileno(cp_in), TERM_SET, (char *) &sbuf);
    } else {
	(void) ioctl(fileno(cp_in), TERM_SET, (char *) &OS_Buf);
    }
#endif
#endif
}

/* The following routines deal with the command and keyword databases.
 * Say whether a given word exists in the command database.
 */
bool cp_comlook (char *word)
{
    if (word && *word && clookup(word, &commands, false, false))
        return (true);
    else
        return (false);
}

/* Add command to the database, with the given keywords and filename flag. */

void cp_addcomm (char *word, int bits0, int bits1, int bits2, int bits3)
{
    struct ccom *cc;

    cc = clookup(word, &commands, false, true);
    cc->cc_invalid = 0;
    cc->cc_kwords[0] = bits0;
    cc->cc_kwords[1] = bits1;
    cc->cc_kwords[2] = bits2;
    cc->cc_kwords[3] = bits3;
}

/* Remove command from the database. */

void cp_remcomm (char *word)
{
    struct ccom *cc;

    cc = clookup(word, &commands, false, false);
    if (cc) cdelete(cc);
}

/* Add keyword to the database. */

void cp_addkword (int class, char *word)
{
    struct ccom *cc;

    if (class < 1 || class >= NCLASSES) {
        fprintf(cp_err, "cp_addkword: Internal Error: bad class %d\n", class);
        return;
    }
    cc = clookup(word, &keywords[class], false, true);
    cc->cc_invalid = 0;
}

/* Remove keyword from the database. */

void cp_remkword (int class, char *word)
{
    struct ccom *cc;

    if (class < 1 || class >= NCLASSES) {
        fprintf(cp_err, "cp_addkword: Internal Error: bad class %d\n", class);
        return;
    }
    cc = clookup(word, &keywords[class], false, false);
    if (cc) cdelete(cc);
}

/* This routine is used when there are several keyword sets that are
 * to be switched between rapidly. The return value is the old tree at
 * that position, and the keyword class given is set to the argument.
 */
char * cp_kwswitch (int class, char *tree)
{
    char *old;

    if (class < 1 || class >= NCLASSES) {
        fprintf(cp_err, "cp_addkword: Internal Error: bad class %d\n", class);
        return (NULL);
    }
    old = (char *) keywords[class];
    keywords[class] = (struct ccom *) tree;
    return (old);
}

/* Throw away all the stuff and prepare to rebuild it from scratch... */

void cp_ccrestart (bool kwords)
{
    /* Ack. */
    return;
}

static void throwaway (struct ccom *dbase)
{
    if (dbase->cc_child) throwaway(dbase->cc_child);
    if (dbase->cc_sibling) throwaway(dbase->cc_sibling);
    tfree(dbase);
}

/* Look up a word in the database. Because of the
 * way the tree is set up, this also works for looking up all words with
 * a given prefix (if the pref arg is true). If create is true, then the
 * node is created if it doesn't already exist.
 */
static struct ccom * clookup (register char *word, struct ccom **dd, bool pref, bool create)
{
    register struct ccom *place = *dd, *tmpc;
    int ind = 0, i;
    char buf[BSIZE_SP];

/* printf("----- adding %s -----\n", word); */
/* prcc(); */

    if (!place) {
        /* This is the first time we were called. */
        if (!create)
            return (NULL);
        else {
            *dd = place = alloc(struct ccom);
	    ZERO(place, struct ccom);
            buf[0] = *word;
            buf[1] = '\0';
            place->cc_name = copy(buf);
            if (word[1])
                place->cc_invalid = 1;
        }
    }
    while (word[ind]) {
        /* Walk down the sibling list until we find a node that
         * matches 'word' to 'ind' places.
         */
        while ((place->cc_name[ind] < word[ind]) && place->cc_sibling)
            place = place->cc_sibling;
        if (place->cc_name[ind] < word[ind]) {
            /* This line doesn't go out that far... */
            if (create) {
                place->cc_sibling = alloc(struct ccom);
		ZERO(place->cc_sibling, struct ccom);
                place->cc_sibling->cc_ysibling = place;
                place->cc_sibling->cc_parent = place->cc_parent;
                place = place->cc_sibling;
                place->cc_name = tmalloc(ind + 2);
                for (i = 0; i < ind + 1; i++)
                    place->cc_name[i] = word[i];
                place->cc_name[ind + 1] = '\0';
                place->cc_invalid = 1;
            } else {
                return (NULL);
            }
        } else if (place->cc_name[ind] > word[ind]) {
            if (create) {
                /* Put this one between place and its pred. */
                tmpc = alloc(struct ccom);
		ZERO(tmpc, struct ccom);
                tmpc->cc_parent = place->cc_parent;
                tmpc->cc_sibling = place;
                tmpc->cc_ysibling = place->cc_ysibling;
                place->cc_ysibling = tmpc;
                place = tmpc;
                if (tmpc->cc_ysibling)
                    tmpc->cc_ysibling->cc_sibling = tmpc;
                else if (tmpc->cc_parent)
                    tmpc->cc_parent->cc_child = tmpc;
                else
                    *dd = place;
                place->cc_name = tmalloc(ind + 2);
                for (i = 0; i < ind + 1; i++)
                    place->cc_name[i] = word[i];
                place->cc_name[ind + 1] = '\0';
                place->cc_invalid = 1;
            } else {
                return (NULL);
            }
        }

        /* place now points to that node that matches the word for
         * ind + 1 characters.
         */
/* printf("place %s, word %s, ind %d\n", place->cc_name, word, ind); */
        if (word[ind + 1]) {    /* More to go... */
            if (!place->cc_child) {
                /* No children, maybe make one and go on. */
                if (create) {
                    tmpc = alloc(struct ccom);
		    ZERO(tmpc, struct ccom);
                    tmpc->cc_parent = place;
                    place->cc_child = tmpc;
                    place = tmpc;
                    place->cc_name = tmalloc(ind + 3);
                    for (i = 0; i < ind + 2; i++)
                        place->cc_name[i] = word[i];
                    place->cc_name[ind + 2] = '\0';
                    if (word[ind + 2])
                        place->cc_invalid = 1;
                } else {
                    return (NULL);
                }
            } else
                place = place->cc_child;
            ind++;
        } else
            break;
    }
    if (!pref && !create && place->cc_invalid) {
        /* This is no good, we want a real word. */
        return (NULL);
    }
    return (place);
}

/* Delete a node from the tree. */

static void cdelete (struct ccom *node)
{
    node->cc_invalid = 1;
}

