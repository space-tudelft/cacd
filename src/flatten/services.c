/*
 * ISC License
 *
 * Copyright (C) 1994-2018 by
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include "src/flatten/extern.h"

char *err_list[] = {
/* 0 */    "%s",
/* 1 */    "No cell name specified",
/* 2 */    "File '%s' does exist (not overwritten)",
/* 3 */    "Error in DMI function",
/* 4 */    "Interrupted due to signal: %s",
/* 5 */    "Too many arguments specified",
/* 6 */    "error: Cannot allocate core",
/* 7 */    "error: Illegal check level",
/* 8 */    "Cannot create file: %s",
/* 9 */    "Illegal %scell name length",
/*10 */    "Too long cell name, cannot add '%s' to it!",
/*11 */    "Bad cell name '%s' specified",
/*12 */    "error: Illegal ganno type",
};

void pr_err (int err_no, char *s)
{
    int i = (err_no < 0) ? -err_no : err_no;
    PE "%s: ", argv0);
    if (i > 11) PE "Unknown error (err_no = %d)", err_no);
    else PE err_list[i], s);
    PE "\n");
    if (err_no >= 0) die ();
}

void sig_handler (int sig) /* signal handler */
{
    char buf[8];
    signal (sig, SIG_IGN); /* ignore signal */
    sprintf (buf, "%d", sig);
    pr_err (4, buf);
}

void dmError (char *s)
{
    if (dmErrorOFF) return;
    dmPerror (s);
    pr_err (3, "");
}

void die ()
{
    if (project) dmQuit ();
    PE "%s: -- program aborted --\n", argv0);
    exit (1);
}

#ifdef DEBUG
void pr_mcelmt (struct mc_elmt *ptr)
{
    PE "===== struct mc_elmt: %08x ======\n", ptr);
    if (ptr) {
	PE "parent: %08x\n", ptr -> parent);
	PE "mtx[0-2]: %ld, %ld, %ld\n",
	    ptr -> mtx[0], ptr -> mtx[1], ptr -> mtx[2]);
	PE "mtx[3-5]: %ld, %ld, %ld\n",
	    ptr -> mtx[3], ptr -> mtx[4], ptr -> mtx[5]);
	PE "dx, nx, dy, ny: %ld, %ld, %ld, %ld\n",
	    ptr -> dx, ptr -> nx, ptr -> dy, ptr -> ny);
	PE "next: %08x\n\n", ptr -> mc_next);
    }
}

void pr_clist (struct clist *clp)
{
    PE "===== struct clist: %08x ======\n", clp);
    if (clp) {
	PE "ckey: %08x\n", clp -> ckey);
	if (clp -> ckey) {
	    PE "cell: %s\n", clp -> ckey -> cell);
	    PE "pkey: %s\n", clp -> ckey -> dmproject);
	}
	PE "imps: %d\n", clp -> imps);
	PE "mc_p: %08x\n", clp -> mc_p);
	PE "next: %08x\n\n", clp -> cl_next);
    }
}
#endif
