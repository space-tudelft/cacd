/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Jeffrey M. Hsu
**********/

/*
    This files contains the routines to evalute arguments to a command
    and prompt the user if necessary.
*/

#include "spice.h"
#include "fteinput.h"
#include "cpdefs.h"
#include "fteext.h"

#define MAXPROMPT 1024
static char buf[MAXPROMPT];

static void common(char *string, wordlist *wl, struct comm *command);

/* returns a private copy of the string */
char *prompt(FILE *fp)
{
    char *p;
    int   n;

    if (!fgets(buf, sizeof(buf), fp)) return NULL;
    n = strlen(buf) - 1;
    buf[n] = '\0';	/* fgets leaves the \n */
    p = (char *) tmalloc(n + 1);
    strcpy(p, buf);
    return p;
}

int countargs(wordlist *wl)
{
    int number = 0;
    for (; wl; wl = wl->wl_next) number++;
    return(number);
}

wordlist *process(wordlist *wlist)
{
    wlist = cp_variablesubst(wlist);
    wlist = cp_bquote(wlist);
    wlist = cp_doglob(wlist);
    return (wlist);
}

int arg_print(wordlist *wl, struct comm *command)
{
    common("which variable", wl, command);
    return(0);
}

int arg_plot(wordlist *wl, struct comm *command)
{
    common("which variable", wl, command);
    return(0);
}

int arg_load(wordlist *wl, struct comm *command)
{
      /* just call com_load */
      (*command->co_func) (wl);
    return(0);
}

int arg_let(wordlist *wl, struct comm *command)
{
    common("which vector", wl, command);
    return(0);
}

int arg_set(wordlist *wl, struct comm *command)
{
    common("which variable", wl, command);
    return(0);
}

int arg_display()
{
    /* just return; display does the right thing */
    return(0);
}

/* a common prompt routine */
static void common(char *string, wordlist *wl, struct comm *command)
{
    wordlist *w;
    char *buf;

    if (!countargs(wl)) {
      outmenuprompt(string);
      if ((buf = prompt(cp_in)) == NULL) /* prompt aborted */
        return;               /* don't execute command */
      /* do something with the wordlist */
      w = alloc(wordlist);
      w->wl_word = buf;
      w->wl_next = NULL;

      w = process(w);
      /* O.K. now call fn */
      (*command->co_func) (w);
    }
}

void outmenuprompt(char *string)
{
    fprintf(cp_out, "%s: ", string);
    fflush(cp_out);
}
