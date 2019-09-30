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

#ifndef _externs_h
#define _externs_h

#include "src/libddm/dmincl.h"

extern char *UsersHomeDirectory;
extern char *OriginalPath;		/* Original Path where you run the program */
extern char *ProgramPath;		/* Path where the program resides */
extern char *MessageDescriptionFile;	/* Message Description File */
extern char *OpenedDatabase;		/* Current Opened Database */
extern char *PrevisDatabase;		/* Previous Opened Database */

/* parameters non-private to adminvar.c, because also used in callbacks.c */
extern bool_param_t ShowCommandLine;
extern bool_param_t ShowQuickReference;

extern int NumberOfProcesses;
extern Process *ProcessItems;
extern char CmdLine[512];
//extern char globalTextBuffer[BUFSIZ];     /* BUFSIZ defined in stdio.h */
extern char globalTextBuffer[2000];

extern DM_PROJECT *dmproject;

extern int errno;
extern FILE *MessageFilePointer;
extern XtAppContext app_context;

#endif
