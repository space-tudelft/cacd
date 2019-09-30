/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * The main entry point for cshpar.
 */

#include "spice.h"
#include "cpdefs.h"
#include <unistd.h>

#include <signal.h>
#ifdef HAS_VFORK
#include <sys/types.h>
#include <sys/wait.h>
#endif

/* Things go as follows:
 * (1) Read the line and do some initial quoting (by setting the 8th bit),
 *  and command ignoring. Also deal with command completion.
 * (2) Do history substitutions. (!, ^)
 * (3) Do alias substitution.
 *
 * In front.c these things get done:
 * (4) Do variable substitution. ($varname)
 * (5) Do backquote substitution. (``)
 * (6) Do globbing. (*, ?, [], {}, ~)
 * (7) Do io redirection.
 */

static bool fileexists (char *name);
void fixdescriptors();
static void pwlist (wordlist *wlist, char *name);

bool cp_debug = false;

FILE *cp_in;
FILE *cp_out;
FILE *cp_err;

/* These are the fps that cp_ioreset resets the cp_* to.  They are changed
 * by the source routines.
 */
FILE *cp_curin = NULL;
FILE *cp_curout = NULL;
FILE *cp_curerr = NULL;

wordlist * cp_parse (char *string)
{
    wordlist *wlist;

    wlist = cp_lexer(string);

    if (!wlist || !wlist->wl_word) return (wlist);

    pwlist(wlist, "Initial parse");

    wlist = cp_histsubst(wlist);
    if (!wlist || !wlist->wl_word) return (wlist);

    /* Add the word list to the history. */
    if (*wlist->wl_word && !string) cp_addhistent(cp_event++, wlist);

    pwlist(wlist, "After history substitution");
    if (cp_didhsubst) {
        wl_print(wlist, stdout);
        putc('\n', stdout);
	if (cp_didhsubstp) { tfree(wlist->wl_word); return (wlist); }
    }

    wlist = cp_doalias(wlist);

    pwlist(wlist, "After alias substitution");
    return (wlist);
}

static void pwlist (wordlist *wlist, char *name)
{
    wordlist *wl;

    if (!cp_debug) return;
    fprintf(cp_err, "%s : [ ", name);
    for (wl = wlist; wl; wl = wl->wl_next)
        fprintf(cp_err, "%s ", wl->wl_word);
    fprintf(cp_err, "]\n");
}

/* This has to go somewhere... */

void com_echo (wordlist *wlist)
{
    char *s;
    bool nl = true;

    if (wlist && eq(wlist->wl_word, "-n")) {
        wlist = wlist->wl_next;
        nl = false;
    }

    while (wlist) {
        s = cp_unquote(wlist->wl_word);
        fputs(s, cp_out);
        txfree(s);
        if (wlist->wl_next) fputs(" ", cp_out);
        wlist = wlist->wl_next;
    }
    if (nl) fputs("\n", cp_out);
}

/* This routine sets the cp_{in,out,err} pointers and takes the io
 * directions out of the command line.
 */

wordlist * cp_redirect (wordlist *wl)
{
    bool gotinput = false, gotoutput = false, goterror = false;
    bool app = false, erralso = false;
    wordlist *w, *bt, *nw;
    char *s;
    FILE *tmpfp;

    bt = NULL;
    w = wl->wl_next;    /* Don't consider empty commands. */
    while (w) {
        if (*w->wl_word == '<') {
            bt = w;
            if (gotinput) {
                fprintf(cp_err, "Error: ambiguous input redirect.\n");
                return (NULL);
            }
            gotinput = true;
            w = w->wl_next;
	    if (!w) {
                fprintf(cp_err, "Error: missing name for input.\n");
                return (NULL);
            }
            if (*w->wl_word == '<') { /* Do reasonable stuff here... */
                fprintf(cp_err, "Error: ambiguous input redirect.\n");
                return (NULL);
            }
	    s = cp_unquote(w->wl_word);
	    tmpfp = fopen(s, "r");
	    if (!tmpfp) { perror(s); txfree(s); return (NULL); }
	    cp_in = tmpfp;
#ifdef CPDEBUG
if (cp_debug) fprintf(cp_err, "Input file is %s...\n", s);
#endif
	    txfree(s);
        } else if (*w->wl_word == '>') {
            bt = w;
            if (gotoutput) {
                fprintf(cp_err, "Error: ambiguous output redirect.\n");
                return (NULL);
            }
            gotoutput = true;
            w = w->wl_next;
            if (w && *w->wl_word == '>') { app = true; w = w->wl_next; }
            if (w && *w->wl_word == '&') {
                erralso = true;
                if (goterror) {
                    fprintf(cp_err, "Error: ambiguous error redirect.\n");
		    return (NULL);
                }
                goterror = true;
                w = w->wl_next;
            }
	    if (!w) {
		fprintf(cp_err, "Error: missing name for output.\n");
		return (NULL);
	    }
            s = cp_unquote(w->wl_word);
            if (cp_noclobber && fileexists(s)) {
                fprintf(stderr, "Error: %s: file exists\n", s);
		txfree(s);
                return (NULL);
            }
	    tmpfp = fopen(s, app ? "a" : "w+");
            if (!tmpfp) { perror(s); txfree(s); return (NULL); }
	    cp_out = tmpfp;
            if (erralso) cp_err = cp_out;
	    out_isatty = false;
#ifdef CPDEBUG
if (cp_debug) fprintf(cp_err, "Output file is %s... %s\n", s, app ? "(append)" : "");
#endif
	    txfree(s);
        }
	if (bt) {
            bt->wl_prev->wl_next = w->wl_next;
            nw = w->wl_next;
            if (nw) nw->wl_prev = bt->wl_prev;
            w->wl_next = NULL;
            wl_free(bt); bt = NULL;
            w = nw;
        } else
            w = w->wl_next;
    }
    return (wl);
}

/* Reset the cp_* FILE pointers to the standard ones.  This is tricky, since
 * if we are sourcing a command file, and io has been redirected from inside
 * the file, we have to reset it back to what it was for the source, not for
 * the top level.  That way if you type "foo > bar" where foo is a script,
 * and it has redirections of its own inside of it, none of the output from
 * foo will get sent to stdout...
 */
void cp_ioreset()
{
    if (cp_in  != cp_curin ) { if (cp_in ) fclose(cp_in ); cp_in  = cp_curin; }
    if (cp_out != cp_curout) { if (cp_out) fclose(cp_out); cp_out = cp_curout; }
    if (cp_err != cp_curerr) { if (cp_err) fclose(cp_err); cp_err = cp_curerr; }

    out_isatty = true;
}

static bool fileexists (char *name)
{
    if (access(name, 0) == 0) return (true);
    return (false);
}

/* Fork a shell. */

void com_shell (wordlist *wl)
{
    char *com, *shell;
#ifdef HAS_VFORK
    pid_t pid, r;
    SIGNAL_TYPE (*svint)(), (*svquit)(), (*svtstp)();
#endif

    cp_ccon(false);

#ifdef HAS_VFORK
    /* XXX Needs to switch process groups.  Also, worry about suspend */
    /* Only bother for efficiency */
    pid = vfork();
    if (pid == 0) {
	fixdescriptors();
	if (wl == NULL) {
	    if (!(shell = getenv("SHELL"))) shell = "/bin/csh";
	    (void) execl(shell, shell, 0);
	    _exit(99);
	} else {
	    com = wl_flatten(wl);
	    (void) execl("/bin/sh", "sh", "-c", com, 0);
	}
    } else {
	/* XXX Better have all these signals */
	svint  = signal(SIGINT,  SIG_DFL);
	svquit = signal(SIGQUIT, SIG_DFL);
	svtstp = signal(SIGTSTP, SIG_DFL);
	/* XXX Sig on proc group */
	do {
	    r = wait((union wait *) NULL);
	} while (r != pid && pid != -1);
	(void) signal(SIGINT,  svint);
	(void) signal(SIGQUIT, svquit);
	(void) signal(SIGTSTP, svtstp);
    }
#else
    /* Easier to forget about changing the io descriptors. */
    if (wl) {
        com = wl_flatten(wl);
        (void) system(com);
        free(com);
    }
    else {
	if (!(shell = getenv("SHELL"))) shell = "/bin/csh";
        (void) system(shell);
    }
#endif
}

/* Do this only right before an exec, since we lose the old std*'s. */

void fixdescriptors()
{
    if (cp_in  != stdin)  (void) dup2(fileno(cp_in),  fileno(stdin));
    if (cp_out != stdout) (void) dup2(fileno(cp_out), fileno(stdout));
    if (cp_err != stderr) (void) dup2(fileno(cp_err), fileno(stderr));
}

void com_rehash (wordlist *wl)
{
    char *s;

    if (!cp_dounixcom) {
        fprintf(cp_err, "Error: unixcom not set.\n");
    }
    else if ((s = getenv("PATH")))
        cp_rehash(s, true);
    else
        fprintf(cp_err, "Error: no PATH in environment.\n");
}

void com_chdir (wordlist *wl)
{
    char *s;
    char localbuf[257];
    int copied = 0;

    if (!wl) {
	if (!(s = getenv("HOME"))) {
	    fprintf(cp_err, "Can't get your HOME entry\n");
	    return;
	}
    }
    else {
        s = cp_unquote(wl->wl_word);
	if (s != wl->wl_word) copied = 1;
    }

    if (*s && chdir(s) == -1) perror(s);

    if (copied) txfree(s);

    if ((s = getcwd(localbuf, sizeof(localbuf))))
	printf("current directory: %s\n", s);
    else
	fprintf(cp_err, "Can't get current working directory.\n");
}

void com_strcmp (wordlist *wl)
{
    char *var, *s1, *s2;
    int i, l1, l2;

    if (!wl || !(var = wl->wl_word)) goto err;
    if (!(wl = wl->wl_next) || !(s1 = wl->wl_word)) goto err;
    if (!(wl = wl->wl_next) || !(s2 = wl->wl_word)) goto err;

    /* unquote */
    l2 = l1 = -1;
    if (*s1 == '"') { ++s1;
	if ((l1 = strlen(s1) - 1) >= 0 && s1[l1] == '"') s1[l1] = '\0'; else l1 = -1;
    }
    if (*s2 == '"') { ++s2;
	if ((l2 = strlen(s2) - 1) >= 0 && s2[l2] == '"') s2[l2] = '\0'; else l2 = -1;
    }
    i = strcmp (s1, s2);

    if (l1 >= 0) s1[l1] = '"';
    if (l2 >= 0) s2[l2] = '"';

    cp_vset (var, VT_NUM, (char *) &i);
    return;
err:
    fprintf(cp_err, "strcmp: too few args.\n");
}
