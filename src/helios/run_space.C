/*
 * ISC License
 *
 * Copyright (C) 1995-2018 by
 *	Xianfeng Ni
 *	Ulrich Geigenmuller
 *	Simon de Graaf
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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#undef atexit /* Motif header files make this malicious definition */

#include "src/space/auxil/auxil.h"
#include "src/helios/realist.h"
#include "src/helios/externs.h"

extern char **environ;
extern Widget WorkAreaText;
extern XtAppContext app_context;

extern void PopupMessageBox (char *);
extern void IndicateRun (int);
extern void AddToOutputWindow (char *);
extern void UpdateJobControlWindow (int slotNr, int empty, char *cmdLine);

static void ReinstatePipeToX (XtPointer, XtIntervalId *);
static void PassOnText (XtPointer, int *, XtInputId *);
static void childsSIGHUPhandler (int);
static void childsSIGINThandler (int);
#if 0
static void childsSIGCHLDhandler (int);
#endif
static void parentSIGCHLDhandler (int);
void terminateProgram (int);

typedef struct {
    pid_t pid;
    char *cmdLine;
    int cmdCount;
    int empty;
    int pipefd[2];
    char *tempFileName;
    void (*callOnCompletion)();
    int toBeTerminated;
} progSlot_t;

#define MAXSLOTNR 5    /* alteration of MAXSLOTNR requires alteration of the
                        *  JobControl window!
                        */
static progSlot_t progSlot[MAXSLOTNR];
static int nrOfProc = 0;
static int CommandCount = 0;
static int innerPid;
int promptType = 4;

/***************************************************************************
* Start child process (such as  space, xspice, ...)
* The first argument is the command line.
*    The second argument is the name of a temporary file, which must be unlinked
* after the program started here has finished, and only then.
*    The third argument is a pointer to a function that must be run after the
* program started here has finished.  This construction is helpful e.g. for
* updating windows: If the program modifies the text that is to displayed,
* the window update is only sensible after the program has finished, and
* calling the function right after the return from ExecuteSpaceCommand
* may be too early.
****************************************************************************/
void ExecuteSpaceCommand (char *CommandLine, char *tempFileName, void (*callOnCompletion)())
{
    char *envString;
    pid_t pid, psGroupId;
    int slotNr, status;

    /* initialize program slots */
    if (CommandCount == 0) {
        for (slotNr = 0; slotNr < MAXSLOTNR; slotNr++) {
            progSlot[slotNr].pid = 0;
            progSlot[slotNr].cmdLine = NULL;
            progSlot[slotNr].cmdCount = 0;
            progSlot[slotNr].empty = 1;
            progSlot[slotNr].tempFileName = NULL;
            progSlot[slotNr].callOnCompletion = NULL;
            progSlot[slotNr].toBeTerminated = 0;
            UpdateJobControlWindow (slotNr, 1, NULL);
        }
    }

    /* find free program slot */
    for (slotNr = 0; slotNr < MAXSLOTNR && !progSlot[slotNr].empty; slotNr++);
    /* Modified by AvG 220597 !!!!!!!!!! */
    if (slotNr >= MAXSLOTNR) {
        PopupMessageBox (mprintf ("You can run at most %d jobs simultaneously.", MAXSLOTNR));
        return;
    }
    else {
	char *s, *t, *q;
	int nr;

        ++CommandCount;
        progSlot[slotNr].empty = 0;
/* On the Sun, the mprintf in the following statement caused
 * Segmentation fault (core dumped)
 * For the time being, the use of mprintf will be circumvented here
 *
 *      progSlot[slotNr].cmdLine = strsave (mprintf ("[%d]space:%s> %s",
 *                                          CommandCount, OpenedDatabase, CommandLine));
 */
	t = OpenedDatabase;
set_prompt:
	nr = 20;
	s = (char*)"space:";
	switch (promptType) {
	case 0: s = (char*)""; nr -= 6;
		t = (char*)"";
		break;
	case 1: s = (char*)""; nr -= 6;
	case 2:
		if ((q = strrchr (t, '/'))) t = q + 1;
		nr += strlen (t);
		break;
	case 3: s = (char*)""; nr -= 6;
	default:
		nr += strlen (t);
	}
	if (strlen (CommandLine) + nr > sizeof(globalTextBuffer)) {
	    if (--promptType >= 0) goto set_prompt;
	    PopupMessageBox ((char*)"Sorry, too long command line!");
	    return;
	}
	sprintf (globalTextBuffer, "[%d]%s%s> %s", CommandCount, s, t, CommandLine);
        progSlot[slotNr].cmdLine = strsave (globalTextBuffer);
        AddToOutputWindow ((char*)"\n");
        /*
        I postponed the 'cmdLine' message to below in order to have
        something for the child to be written to stderr (or stdout).
        If nothing is written to stderr/stdout by the child, then the
        child 'Simeye' does not start on a Solaris machine (I do not
        know why this occurs).  AvG 230579.
        AddToOutputWindow (progSlot[slotNr].cmdLine);
        */
        progSlot[slotNr].cmdCount = CommandCount;
        /* Modified by AvG 220597 !!!!!!!!!! */
        if (tempFileName)
            progSlot[slotNr].tempFileName = strsave (tempFileName);
        else
            progSlot[slotNr].tempFileName = NULL;
        progSlot[slotNr].callOnCompletion = callOnCompletion;
        progSlot[slotNr].toBeTerminated = 0;
        UpdateJobControlWindow (slotNr, 0, progSlot[slotNr].cmdLine);
    }

    if (pipe (progSlot[slotNr].pipefd) == -1) {
        PopupMessageBox ((char*)"Allocation of pipe failed, unable to run program.");
        return;
    }

    /* We only want to read when there is data in the pipe.
     * With  O_NONBLOCK, i.e. POSIX-style non-blocking I/O  set,
     * reading from an empty file returns -1.
     * If this is not set, then the "read" in the function PassOnText keeps
     * everybody else waiting until data arrives: so the windows look
     * dead, and, in particular, are not refreshed when moved etc.
     */
    fcntl (progSlot[slotNr].pipefd[0], F_SETFL, O_NONBLOCK);

    nrOfProc++;
    IndicateRun (nrOfProc);

    if ((pid = fork ()) == (pid_t) 0) {

        int f;

        /* So here we are the child process. */

        sprintf (globalTextBuffer, "CWD=%s", OpenedDatabase);
        envString = strsave (globalTextBuffer);
        putenv (envString);

        /* modified by AvG 030697 */
        sprintf (globalTextBuffer, "ICDPATH=%s", icdpath);
        envString = strsave (globalTextBuffer);
        putenv (envString);

        close (1);        /* redirect stdout  */
        f = dup (progSlot[slotNr].pipefd[1]);
        /* dup sets file des. 1 to progSlot[slotNr].pipefd[1] */

        close (2);        /* redirect stderr  */
        f = dup (progSlot[slotNr].pipefd[1]);
        /* dup sets file des. 2 to progSlot[slotNr].pipefd[1] */

        close (progSlot[slotNr].pipefd[0]);
        close (progSlot[slotNr].pipefd[1]);

        setvbuf (stderr, NULL, _IOLBF, BUFSIZ);
        setvbuf (stdout, NULL, _IOLBF, BUFSIZ);

	signal (SIGCHLD, SIG_IGN); /* avoid calling childsSIGCHLDhandler */
        signal (SIGHUP, childsSIGHUPhandler);
        signal (SIGINT, childsSIGINThandler);

        /* Here we write something to stderr of the child.
           See remark that is given above.
        */
        fprintf (stderr, "%s\n", progSlot[slotNr].cmdLine);

        if ((innerPid = (int) fork ()) ==  0) {
            psGroupId = setsid ();      /* by making this process a process-group leader,
                              also its children are properly stopped upon
                              receiving the HUP/INT signal */
            ASSERT ( psGroupId != -1);
            /*  An attempt to shorten the above code into
             *      ASSERT ( (setsid () != -1) );
             *  caused malfunction: process groups were no longer properly killed. Why?
             */
            execle ("/bin/sh", "sh", "-c", CommandLine, (char *) 0, environ);
        }
        wait (&status);
        /* We need to let the routine reading from the pipe know when
         * the program writing into the pipe has finished. One way of doing
         * this is to close the writing end of the pipe.  It is not sufficient
         * to only do it here, however, but it must also in be done in the
         * parent of the current process.  Unfortunately, this way of doing
         * things causes problems:
         * (1) When another program is started using ExecuteSpaceCommand
         *     while the first one has not yet finished, the file descriptor
         *     of the writing end of the first pipe is available again
         *     and will be re-used in general, possibly for the reading
         *     end of the second pipe.  This can confuse the system.
         * (2) When we interrupt a running program, we want to write a
         *     message to the output window.  That is most easily achieved
         *     when the writing end of the pipe is still available.
         * So we use another method to communicate to the routine reading
         * from the pipe that the program writing into it has finished, viz.
         * by letting the latter write an EOF  into the pipe
         * before exiting.  The routine PassOnText reading from the pipe
         * then gets the task of closing the pipe.
         */
        fprintf (stderr, "==== end of task [%d] ====\n%c", CommandCount, EOF);
        exit (127);
    }

    signal (SIGCHLD, parentSIGCHLDhandler); /* avoid zombiefication of child */

    progSlot[slotNr].pid = pid;

    /* Register reading end of pipe will as new source of input for X. */
    XtAppAddInput (app_context, progSlot[slotNr].pipefd[0],
           (XtPointer) XtInputReadMask, PassOnText, (XtPointer)(long)slotNr);
}

/*****************************************************************************
* This routine reads from a pipe into which a child program started with
* ExecuteSpaceCommand writes,  and passes on the text read from the
* pipe to the output display area of the  main window.  Together with the
* reading-end of the pipe, it must have been registered as additional input
* source, using the XtAppAddInput routine.  PassOnText also takes care of
* cleaning up program slots after the program providing the input has finished.
******************************************************************************/
static void PassOnText (XtPointer slotNrAsXtPointer, int *dummy, XtInputId *id)
{
    char buffer[1025], *marker;
    int nRead, counter;
    int keepReadingFromTerminatedProcess;
    long slotNr = (long) slotNrAsXtPointer;

    /* Note: if a process has been terminated, we keep reading from it until
     * it actually dies.
     */
    keepReadingFromTerminatedProcess = 1;

    /* For the following control structure to work properly, an attempt
     * to read from an open but empty pipe yields must yield -1 as result.
     * To this end, the reading end of the pipe must have been set
     * non-blocking, using fcntl(pipefd[0], F_SETFL, O_NONBLOCK).
     */
    counter = 0;
    if (keepReadingFromTerminatedProcess || progSlot[slotNr].toBeTerminated == 0) {
        do {
            nRead = read (progSlot[slotNr].pipefd[0], buffer, sizeof (buffer) - 1);
            counter++;
            /* Only pass on a limited amount of text at a time (counted by `counter').
             * Otherwise, if the child program  generates a continuous stream of error
             * messages, it can happen that the GUI is so busy passing them on
             * that it cannot service other request of the user, or window updates etc.
             */
            if (nRead > 0) {
                buffer[nRead] = '\0';
                /* An EOF serves as signal that the program providing input
                 * has finished.
                 */
                if ((marker = strchr (buffer, EOF))) {
                    nRead = 0;
                    *marker = '\0';
                }
                AddToOutputWindow (buffer);
            }
        } while (nRead > 0 && counter < 10);
    }
    else
       nRead = 0;

    /* Remove the pipe as additional input source for X */
    XtRemoveInput (*id);

    if (nRead != 0) {
        /* nRead == -1 means that the pipe is still opened for writing,
         * but at present contains no data.
         *
         * nRead > 0 means that one could in principle go on reading, but
         * just wants to pause in order to free time for processing other
         * things (window update, new user input etc.).
         *
         * With the aid of a timer function, the pipe will be added again as
         * additional input source after some period.
         */

        /* 2nd argument = waiting time in milliseconds */
        XtAppAddTimeOut (app_context, 250, ReinstatePipeToX, (XtPointer) slotNr);
    }
    else {
        /* The program has ended, free program slot */

        if(progSlot[slotNr].toBeTerminated != 0)
        {
            AddToOutputWindow (mprintf ("\n==== task [%d] has been terminated ====\n", progSlot[slotNr].cmdCount));
        }

        nrOfProc--;
        ASSERT (nrOfProc >= 0);
        IndicateRun (nrOfProc);
        progSlot[slotNr].empty = 1;
        DISPOSE (progSlot[slotNr].cmdLine, 0);
        progSlot[slotNr].cmdLine = NULL;
        progSlot[slotNr].cmdCount = 0;
        close (progSlot[slotNr].pipefd[0]);
        close (progSlot[slotNr].pipefd[1]);
        progSlot[slotNr].pipefd[0] = progSlot[slotNr].pipefd[1] = 0;
        if (progSlot[slotNr].tempFileName) {
            unlink (progSlot[slotNr].tempFileName);
            DISPOSE (progSlot[slotNr].tempFileName, 0);
            progSlot[slotNr].tempFileName = NULL;
        }
        if (progSlot[slotNr].callOnCompletion)
            progSlot[slotNr].callOnCompletion();
        UpdateJobControlWindow (slotNr, 1, NULL);
    }
}

/****************************************************************
 * Re-instate the pipe to which space3d (or other programs) write
 * as input for X.
 ***************************************************************/
static void ReinstatePipeToX (XtPointer slotNrAsXtPointer, XtIntervalId *dummy)
{
    XtAppAddInput (app_context, progSlot[(long) slotNrAsXtPointer].pipefd[0],
           (XtPointer) XtInputReadMask, PassOnText, slotNrAsXtPointer);
}

/****************************************************************
* Terminates a program that has been started as a child process
* by sending the `hangup' interrupt signal (SIGHUP = 1).  It is
* not appropriate to send a `kill' signal ( _SIGKILL = 9), since
* then  grandchild-processes are not killed.
*
* The `interrupt' signal (SIGINT = 2) is appropriate here since
* we want to give a process a decent chance of cleaning up.
* Most processes that want to do cleanup have a SIGINT handler.
*
* The use of SIGINT signals may pose a problem, since
* interactive shells may catch those signals. However in
* practice this does not seem to occur.
*
* The code present in this file dealing with SIGHUP exists
* only to support a future extension of the `terminate'
* mechanism. In the future, we might make two buttons:
* "interrupt" and "terminate" (the former sending SIGINT, the
* latter SIGHUP).
****************************************************************/
void terminateProgram (int slotNr)
{
    int minslot, maxslot;

    if (!CommandCount) {
	/* Proper initialization of the program slots has not yet
	 * taken place, and nothing must nor needs be done.
	 */
	return;
    }

    if (slotNr >= 0 && slotNr < MAXSLOTNR) {
	minslot = slotNr;
	maxslot = slotNr + 1;
    }
    else { /* terminate all programs */
	ASSERT (slotNr == -1);
	if (slotNr != -1) {
	    say ("Received request to terminate job with illegal slotNr = %d", slotNr);
	}
	minslot = 0;
	maxslot = MAXSLOTNR;
    }

    for (slotNr = minslot; slotNr < maxslot && !progSlot[slotNr].empty; slotNr++) {
	if (kill (progSlot[slotNr].pid, SIGINT)) {
	    say ("Failure killing process %d with SIGINT signal.", progSlot[slotNr].pid);
	    return;
	}
	AddToOutputWindow (mprintf ("\n==== task [%d] has been sent the INT signal ====\n", progSlot[slotNr].cmdCount));
	progSlot[slotNr].callOnCompletion = NULL;
	progSlot[slotNr].toBeTerminated = 1;
    }
}

static void childsSIGHUPhandler (int)
{
    if (innerPid != 0) {

        if (kill (-innerPid, SIGHUP))
        {
            say ("Failure killing all members of process group %d with SIGHUP signal.", innerPid);
        }
        else
        {
            /* Print EOF character, to let the parent process know that we are dying. */
            fprintf(stderr, "%c", EOF);

            exit(0);
        }

        /* Print EOF character, to let the parent process know that we are dying. */
        fprintf(stderr, "%c", EOF);
    }
}

static void childsSIGINThandler (int)
{
    if (innerPid != 0) {

        if (kill (-innerPid, SIGINT))
        {
            say ("Failure killing all members of process group %d with SIGINT signal.", innerPid);
        }
        else
        {
            /* Print EOF character, to let the parent process know that we are dying. */
            fprintf(stderr, "%c", EOF);

            exit(0);
        }

        /* Print EOF character, to let the parent process know that we are dying. */
        fprintf(stderr, "%c", EOF);
    }
}

#if 0
static void childsSIGCHLDhandler (int)
{
    int status;
    wait (&status);
}
#endif

static void parentSIGCHLDhandler (int)
{
    int status;
    wait (&status);
}

