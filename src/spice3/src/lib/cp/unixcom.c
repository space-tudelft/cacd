/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to do execution of unix commands.
 */

#include "spice.h"
#include "cpdefs.h"
#include <unistd.h> /* getcwd */

#ifdef HAS_VFORK

/* The only reason this exists is efficiency */

#include <sys/types.h>
#include <dirent.h>
#ifndef direct
#define direct dirent
#endif
#include <sys/stat.h>

#include <sys/file.h>
#include <sys/wait.h>
#include <signal.h>

static bool tryexec();
static int hash();

struct hashent {
    char *h_name;
    char *h_path;
    struct hashent *h_next;
} ;

#define HASHSIZE 256

static struct hashent *hashtab[HASHSIZE];
static char *dirbuffer;
static int dirlength, dirpos;

/* Create the hash table for the given search path. pathlist is a : seperated
 * list of directories. If docc is true, then all the commands found are
 * added to the command completion lists.
 */

void cp_rehash (char *pathlist, bool docc)
{
    register int i;
    struct hashent *hh, *ht;
    char buf[BSIZE_SP], pbuf[BSIZE_SP], *curpath;
    DIR *pdir;
    struct direct *entry;
    struct stat sbuf;

    /* First clear out the old hash table. */
    for (i = 0; i < HASHSIZE; i++) {
        for (hh = hashtab[i]; hh; hh = ht) {
            ht = hh->h_next;
            /* Don't free any of the other stuff -- it is too
             * strange.
             */
            tfree(hh);
        }
	hashtab[i] = NULL;
    }

    while (pathlist && *pathlist) {
        /* Copy one path to buf. We have to make sure that the path
         * is a full path name.
         */
        if (*pathlist == '/')
            i = 0;
        else {
            (void) getcwd(buf, sizeof(buf));
            i = strlen(buf);
	    buf[i++] = '/';
        }
        while (*pathlist && (*pathlist != ':'))
            buf[i++] = *pathlist++;
        while (*pathlist == ':')
            pathlist++;
        buf[i] = '\0';

        if (!(pdir = opendir(buf))) continue;
        curpath = NULL;
        while ((entry = readdir(pdir))) {
            strcpy(pbuf, buf);
            strcat(pbuf, "/");
            strcat(pbuf, entry->d_name);
	    if (stat(pbuf, &sbuf) != 0) continue;
	    if (!S_ISREG(sbuf.st_mode)) continue;

            /* Now we could make sure that it is really
             * an executable.
             */
	    if (!curpath) curpath = copy(buf);
            hh = alloc(struct hashent);
            hh->h_name = copy(entry->d_name);
            hh->h_path = curpath;
            i = hash(entry->d_name);
            /* Make sure this goes at the end, with
             * possible duplications of names.
             */
            if (hashtab[i]) {
                ht = hashtab[i];
                while (ht->h_next) ht = ht->h_next;
                ht->h_next = hh;
            } else
                hashtab[i] = hh;
            if (docc) {
                /* Add to completion hash table. */
                cp_addcomm(entry->d_name, 0, 0, 0, 0);
            }
        }
	closedir(pdir);
    }
    return;
}

/* The return value is false if no command was found, and true if it was. */

bool cp_unixcom (wordlist *wl)
{
    int i;
    register struct hashent *hh;
    register char *name;
    char **argv;
    char buf[BSIZE_SP];

    if (!wl) return (false);
    name = wl->wl_word;
    argv = wl_mkvec(wl);
    if (cp_debug) {
        printf("name: %s, argv: ", name);
        wl_print(wl, stdout);
        printf(".\n");
    }
    if (strchr(name, '/'))
        return (tryexec(name, argv));
    i = hash(name);
    for (hh = hashtab[i]; hh; hh = hh->h_next) {
        if (eq(name, hh->h_name)) {
            sprintf(buf, "%s/%s", hh->h_path, hh->h_name);
            if (tryexec(buf, argv))
                return (true);
        }
    }
    return (false);
}

static bool tryexec (char *name, char *argv[])
{
#ifdef HAS_INTWAITSTATUS
    int status;
#else
    union wait status;
#endif
    int pid, j;
    SIGNAL_TYPE (*svint)(), (*svquit)(), (*svtstp)();

    pid = vfork( );
    if (pid == 0) {
	fixdescriptors();
        (void) execv(name, argv);
        (void) _exit(120);  /* A random value. */
        /* NOTREACHED */
    } else {
	svint  = signal(SIGINT,  SIG_DFL);
	svquit = signal(SIGQUIT, SIG_DFL);
	svtstp = signal(SIGTSTP, SIG_DFL);
        do {
            j = wait(&status);
        } while (j != pid);
	(void) signal(SIGINT,  svint);
	(void) signal(SIGQUIT, svquit);
	(void) signal(SIGTSTP, svtstp);
    }
    if (WTERMSIG(status) == 0 && WEXITSTATUS(status) == 120)
    /*if ((status.w_termsig == 0) && (status.w_retcode == 120)) */
	return (false);
    else
	return (true);
}

static int hash (register char *str)
{
    register int i = 0;

    while (*str) i += *str++;
    return (i % HASHSIZE);
}

/* Debugging. */

void cp_hstat()
{
    struct hashent *hh;
    int i;

    for (i = 0; i < HASHSIZE; i++)
        for (hh = hashtab[i]; hh; hh = hh->h_next)
            fprintf(cp_err, "i = %d, name = %s, path = %s\n", i, hh->h_name, hh->h_path);
}

#else

void cp_rehash (char *pathlist, bool docc)
{
}

bool cp_unixcom (wordlist *wl)
{
    char *s = wl_flatten(wl);
    int status = system(s);
    free(s);
    return (status ? false : true);
}

#endif
