/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
/* The problem with unix signal handlers is that it's hard to make it portable.
 * This is a first attempt, everybody is invited to #ifdef this code until it
 * works on his own machine (send patches to space-support-ewi@tudelft.nl).
 */

#include <stdio.h>
#include <stdlib.h>
#include "src/ocean/libseadif/syssig.h"
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/libseadif/sea_decl.h"

#ifndef SIG_ERR			  /* some systems do not define this */
#define SIG_ERR ((SIG_PF_TYPE)-1) /* defined in "systypes.h" */
#endif

void sdfterminate (int signum)
{
   if (signum == SIGILL
#ifdef SIGQUIT
    || signum == SIGQUIT
#endif
    || signum == SIGSEGV)
   {
      fprintf (stderr, "\n/\\/\\/\\/\\/\\ caught signal %d, remove lock files and dump core.\n", signum);
#ifdef SIGQUIT
      if ((void*)signal (SIGQUIT, (SIG_PF_TYPE)SIG_DFL) == (void*)SIG_ERR)
      {
	  fprintf (stderr, "\n...that's a pitty, cannot reset SIGABRT, won't dump core...\n");
	  sdfexit (99);
      }
#endif
      dumpcore ();
   }
   else {
      fprintf (stderr, "\n/\\/\\/\\/\\/\\ caught signal %d, remove lock files and terminate.\n", signum);
      sdfexit (999);
   }
}

void sdfinitsignals ()
{
#ifdef SIGHUP
if ((void*)signal (SIGHUP, (SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGHUP");
#endif
#ifdef SIGINT
if ((void*)signal (SIGINT, (SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGINT");
#endif
#ifdef SIGQUIT
if ((void*)signal (SIGQUIT,(SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGQUIT");
#endif
#ifdef SIGILL
if ((void*)signal (SIGILL, (SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGILL");
#endif
#ifdef SIGABRT
if ((void*)signal (SIGABRT,(SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGABRT");
#endif
#ifdef SIGBUS
if ((void*)signal (SIGBUS, (SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGBUS");
#endif
#ifdef SIGSEGV
if ((void*)signal (SIGSEGV,(SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGSEGV");
#endif
#ifdef SIGTERM
if ((void*)signal (SIGTERM,(SIG_PF_TYPE)sdfterminate) == (void*)SIG_ERR) perror ("Seadif signal handler SIGALRM");
#endif
}
