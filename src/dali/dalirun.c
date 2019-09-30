/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	P. van der Wolf
 *	S. de Graaf
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "src/dali/header.h"
#include "sys/wait.h"

/*
** DaliRun - fork and execute a file with output redirected.
**
** This function runs a subprogram with the given arguments.
** These arguments must be of type "char *", the last must be NULL.
** Outp_file is the file to which stdout will be redirected.
*/

/*
** Return codes of fork
*/
#define CHILD 0
#define PARENT 1:default
#define ERROR -1

static FILE *fp_stdout = NULL;

int DaliRun (char *path, char *outp_file, char *alist)
{
    int   status;
    char  err_str[MAXCHAR];
    char *argv[64];
    int   ret_wait;
    int   fd_1, fd_2;
    int   i = 0;
#ifdef DRIVER
    static int count = 0;
    ++count;
#endif /* DRIVER */

    argv[i++] = path;

    if (alist) {
	for (;;) {
	    /* skip leading white space */
	    while (*alist == ' ' || *alist == '\t') ++alist;
	    if (!*alist) break; /* end of arg.list */
	    argv[i++] = alist; /* begin of argument */
	    if (i == 64) goto no_run;
	    /* search end of argument */
	    while (*++alist && *alist != ' ' && *alist != '\t');
	    if (!*alist) break; /* end of arg.list */
	    *alist++ = 0; /* set end of argument */
	}
    }
    argv[i] = NULL;

#ifdef ED_DEBUG /**/
    PE "DaliRun: cmd=\"%s\", outf=\"%s\"\n", path, outp_file);
    for (i = 0; argv[++i];) PE "DaliRun: arg%d=\"%s\"\n", i, argv[i]);
#endif /* ED_DEBUG */

    if (!fp_stdout) fp_stdout = stdout;

#ifdef DRIVER
    printf ("start of DaliRun on stdout (%d)\n", count);
#endif /* DRIVER */

    /* We would like to redirect the stdout-output of
    ** the child process to 'outp_file'. Arrange for
    ** this in the parent-process before the vfork()
    ** and correct after running the child.
    */
    fflush (fp_stdout);
    fd_1 = fileno (fp_stdout);

#ifdef ED_DEBUG
    PE "before dup: stdo = %x, fd(stdo) = %d\n", fp_stdout, fd_1);
#endif /* ED_DEBUG */

    if ((fd_2 = dup (fd_1)) == -1) goto no_run;
#ifdef ED_DEBUG
    PE "dup returns: fd_2 = %d\n", fd_2);
#endif /* ED_DEBUG */

    if (!(fp_stdout = freopen (outp_file, "w", fp_stdout))) goto no_run;

#ifdef ED_DEBUG
    PE "after freopen: stdo = %x, fd(stdo) = %d\n", fp_stdout, fileno (fp_stdout));
#endif /* ED_DEBUG */

#ifdef DRIVER
    printf ("before vfork on stdout (%d)\n", count);
    fflush (fp_stdout);
#endif /* DRIVER */

    switch (vfork ()) {
	case PARENT:
	    ret_wait = wait (&status);
	    break;
	case CHILD:
	    execvp (path, argv);
	    /* can't exec */
	    (void) PE "DaliRun: ");
	    perror (path);
	    _exit (63);
	    break;
	case ERROR:
	    (void) PE "DaliRun (error in fork): ");
	    perror (path);
	    goto no_run;
    }
#ifdef ED_DEBUG
    PE "wait returns %d and status: %d\n", ret_wait, status);
    PE "before close: stdo = %x, fd(stdo) = %d\n", fp_stdout, fileno (fp_stdout));
#endif /* ED_DEBUG */

#ifdef DRIVER
    printf ("before fclose on stdout (%d)\n", count);
    fflush (fp_stdout);
#endif /* DRIVER */

    /* RESTORE STDOUT OF PARENT PROCESS.
    ** Close the redirected stdout.
    */
    fclose (fp_stdout);

    /* We assume that fdopen uses the stdout iob[] entry,
    ** that just became available due to the fclose().
    ** First 'dup' again to get fd == 1 for stdout.
    */
    if ((fd_1 = dup (fd_2)) == -1) {
	PE "DaliRun: running %s, stdout not restored (after dup)\n", path);
	ptext ("warning: Stdout not restored properly!");
	return (0);
    }

#ifdef ED_DEBUG
    PE "dup returns: fd_1 = %d\n", fd_1);
#endif /* ED_DEBUG */

    if (!(fp_stdout = fdopen (fd_1, "w"))) {
	PE "DaliRun: running %s, stdout not restored (after fdopen)\n", path);
	ptext ("warning: Stdout not restored properly!");
	return (0);
    }

#ifdef ED_DEBUG
    PE "after dopen: stdo = %x, fd_1 = %d, fd_2 = %d, fd(stdo) = %d\n", fp_stdout, fd_1, fd_2, fileno (fp_stdout));
#endif /* ED_DEBUG */
#ifdef DRIVER
    printf ("after dopen on stdout (%d)\n", count);
    fflush (fp_stdout);
#endif /* DRIVER */

    ASSERT (fileno (fp_stdout) == fd_1);
    close (fd_2);

    /* Stdout has been restored. Now inspect result of wait(). */
    if (ret_wait == -1) {
	(void) PE "DaliRun: ");
	perror (path);
	goto no_run;
    }

    if (status) {
	if (WIFSIGNALED (status)) {
	    sprintf (err_str, "Process '%s' terminated! (sig = %d)%s",
		path, (int) WTERMSIG(status), WCOREDUMP(status) ? " (core dumped)" : "");
	}
	else if (WIFEXITED (status)) {
	    if (WEXITSTATUS (status) == 63) {
		sprintf (err_str, "Can't run '%s'! (see stderr.file)", path);
	    }
	    else {
		sprintf (err_str, "Process '%s' did not return properly! (exit = %d)",
		    path, (int) WEXITSTATUS (status));
	    }
	}
	else { 		/* stopped */
	    sprintf (err_str, "Process '%s' stopped!", path);
	}

	ptext (err_str);
	return (-1);
    }
    /* else    Normal termination. */

    return (0);

no_run:
    sprintf (err_str, "Can't run '%s'!", path);
    ptext (err_str);
    return (-1);
}

#ifdef DRIVER
main (int argc, char *argv[])
{
    char *cellname;
    char outp_file[MAXCHAR];

    if (argc != 2) {
	PE "no cellname given!\n");
	goto usage;
    }

    cellname = argv[1];
    if (dmTestname (cellname) != 0) {
	PE "illegal cellname!\n");
	goto usage;
    }

    sprintf (outp_file, "-h %s", cellname); /* used as arg.list! */
    PE "== EXPANDING HIERARCHICALLY (exp -h) ==\n");
    if (DaliRun ("exp", "/dev/null", outp_file) == -1) {
	PE "exp returns -1\n");
	exit (1);
    }

    sprintf (outp_file, "%s.ck", cellname);
    PE "== CHECKING (dimcheck2) ==\n");
    if (DaliRun ("dimcheck2", outp_file, NULL) == -1) {
	PE "dimcheck2 returns -1\n");
	exit (1);
    }
    exit (0);
usage:
    PE "\nUsage: dalirun cellname\n\n");
    exit (1);
}

void err_meas (char *str)
{
    PE "%s\n", str);
}

void print_assert (char *filen, char *linen)
{
    PE "assertion failed: %s, %s\n", filen, linen);
}
#endif /* DRIVER */
