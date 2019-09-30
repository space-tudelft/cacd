/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include "src/libddm/dmincl.h"

#define MAXLINE 256
#define PE fprintf (stderr,
#define PO fprintf (stdout,

char *process = NULL;

char   *argv0 = "getproc";	/* program name */
char   *use_msg =		/* command line */
"\nUsage: %s [proc_id | proc_name | -m maskdatafile | -p]\n\n";

void die (void);

int main (int argc, char *argv[])
{
    DM_PROCDATA *processData;
    DM_PROJECT  *project = NULL;
    FILE        *fp_list;
    char         id_str[MAXLINE];
    char         proc_name[MAXLINE];
    char         lambda_buf[MAXLINE];
    char         buf[1024];
    char        *hp;
    char        *maskdata, *s;
    int          proc_id;
    int          p_flag;
    register int i;

    maskdata = NULL;
    p_flag = 0;

    while (--argc > 0 && (*++argv)[0] == '-') {
	for (s = *argv + 1; *s != '\0'; ++s) {
	    switch (*s) {
	    case 'm':
		if (p_flag || !(maskdata = *++argv)) goto usage;
		--argc;
		*(s+1) = '\0';
		break;
	    case 'p':
		if (maskdata) goto usage;
		p_flag = 1;
		break;
	    default:
		PE "%s: illegal option: %c\n", argv0, *s);
		goto usage;
	    }
	}
    }

    if (argc > 1 || ((maskdata || p_flag) && argc > 0)) {
usage:
	PE use_msg, argv0);
	exit (1);
    }

    dmInit (argv0);

    *id_str = *proc_name = '\0';
    *lambda_buf = '\0';

    if (maskdata) {
	if (!(fp_list = fopen (maskdata, "r"))) {
	    PE "%s: cannot open maskdatafile: %s\n", argv0, maskdata);
	    die ();
	}
	fclose (fp_list);
	PO "USING MASKDATA\n");
    }
    else if (p_flag) {
	sprintf (buf, "%s/share/lib/process/processlist", icdpath);
	if (!(fp_list = fopen (buf, "r"))) {
	    PE "%s: cannot open: %s\n", argv0, buf);
	    die ();
	}
	PO "PRINTING PROCESSLIST\n");
	hp = "-----------------------------------\n";
	PO hp);
	PO "ICDPATH = %s\n", icdpath);
	PO "PROCESS ID:  NAME:\n");
	PO hp);
	while (fgets (buf, MAXLINE, fp_list)) {
	    if (sscanf (buf, "%s%s", id_str, proc_name) != 2) continue;
	    if (*id_str == '#') continue;
	    if ((s = strchr (proc_name, '#'))) *s = '\0';
	    PO "       %3s   %s\n", id_str, proc_name);
	}
	PO hp);
	fclose (fp_list);
	dmQuit ();
	exit (0);
    }
    else if (argc == 0) {
	/* see if default_project can be opened for the process_id */
	/* else use environment variable ICDPROCESS (if set) */
	process = getenv ("ICDPROCESS");
	project = dmOpenProject (DEFAULT_PROJECT, PROJ_READ);
	if (project) {
	    PO "USING PROJECT\n");
	    if ((proc_id = project -> procid) < 0) {
		process = project -> procpath;
		goto dopath;
	    }
	    goto do_id;
	}
	if (!process) die ();
	PO "USING ICDPROCESS\n");
    }
    else {
	process = argv[0];
	PO "USING PROCESS ARG.\n");
    }

    if ((s = process)) {
	while (*s && isdigit ((int)*s)) ++s;
	if (*s) {
	    if (strchr (s, '/')) goto dopath;
	    proc_id = -1;
	}
	else
	    proc_id = atoi (process);
do_id:
	i = 0;
	sprintf (buf, "%s/share/lib/process/processlist", icdpath);
	if (!(fp_list = fopen (buf, "r"))) {
	    PE "%s: cannot open: %s\n", argv0, buf);
	    die ();
	}
	while (fgets (buf, MAXLINE, fp_list)) {
	    if (sscanf (buf, "%s%s", id_str, proc_name) != 2) continue;
	    if (*id_str == '#') continue;
	    if ((hp = strchr (proc_name, '#'))) *hp = '\0';
	    if (proc_id >= 0) {
		if (proc_id == atoi (id_str)) { ++i; break; }
	    }
	    else if (strcmp (proc_name, process) == 0) { ++i; break; }
	}
	fclose (fp_list);
	if (!i) {
	    DIR *dp;
	    if (proc_id >= 0 && !process) {
		PE "%s: process id '%d' not found in processlist!\n", argv0, proc_id);
		die ();
	    }
	    if ((dp = opendir (process))) {
		/* 'process' is used as a process directory */
		closedir (dp);
dopath:
		sprintf (buf, "%s/default_lambda", process);
		if ((fp_list = fopen (buf, "r"))) {
		    fscanf (fp_list, "%s", lambda_buf);
		    fclose (fp_list);
		}
		sprintf (buf, "%s/maskdata", process);
	    }
	    else {
		if (proc_id >= 0)
		    PE "%s: unknown process id: %d\n", argv0, proc_id);
		else
		    PE "%s: unknown process name: %s\n", argv0, process);
		die ();
	    }
	    *id_str = *proc_name = '\0';
	}
	else {
	    sprintf (buf, "%s/share/lib/process/%s/default_lambda", icdpath, proc_name);
	    if ((fp_list = fopen (buf, "r"))) {
		fscanf (fp_list, "%s", lambda_buf);
		fclose (fp_list);
	    }
	    sprintf (buf, "%s/share/lib/process/%s/maskdata", icdpath, proc_name);
	}
	maskdata = buf;
    }

    hp = "------------------------------------------------------------\n";
    if (project) {
	PO hp);
	PO "PROJECT: %s\n", project -> dmpath);
	PO "RELEASE: %d\n", project -> release);
	if (project -> procid < 0)
	    PO "PROCESS: %s\n", project -> procpath);
	else
	    PO "PROCESS: %d\n", project -> procid);
	PO "LAMBDA: %g\n", project -> lambda);
    }
    PO hp);

    i = strlen (icdpath);
    if (strncmp (icdpath, maskdata, i))
	PO "FILE: %s\n", maskdata);
    else
	PO "FILE: $ICDPATH%s\n", maskdata + i);

    PO "PROCESS ID #: %s\n", *id_str ? id_str : "?");
    PO "PROCESS NAME: %s\n", *proc_name ? proc_name : "?");
    if (*lambda_buf) PO "DEFAULT LAMBDA: %s\n", lambda_buf);
    fflush (stdout);

    processData = (DM_PROCDATA *) _dmDoGetProcessFile (maskdata);
    if (!processData) {
	PE "%s: cannot read maskdatafile!\n", argv0);
	die ();
    }

    s = processData -> pr_type;
    PO "PROCESS TYPE: %s\n", *s ? s : "?");
    s = processData -> pr_info;
    PO "PROCESS INFO: %s\n", *s ? s : "?");
    PO "NR. OF MASKS: %d\n", processData -> nomasks);
    PO hp);
    PO "MASK\tMASK\tMASK\tPG\tPG\tMASK\n");
    PO "NR\tNAME\tTYPE\tJOB\tTYPE\tINFO\n");
    PO hp);
    for (i = 0; i < processData -> nomasks; ++i) {
	PO "%2d\t%s", i, processData -> mask_name[i]);
	PO "%c%d", strlen (processData -> mask_name[i]) > 7 ? ' ' : '\t', processData -> mask_type[i]);
	PO "\t%d", processData -> pgt_no[i]);
	PO "\t%d", processData -> pgt_type[i]);
	s = processData -> mask_info[i];
	PO "\t%s\n", *s ? s : "?");
    }
    PO hp);
    PO "MASK\tMASK\tCM\tCM\tDALI\tDALI\tPLOT\tPLOT\n");
    PO "NR\tNAME\tCOLOR\tFILL\tCOLOR\tFILL\tCOLOR\tFILL\n");
    PO hp);
    for (i = 0; i < processData -> nomasks; ++i) {
	PO "%2d\t%s", i, processData -> mask_name[i]);
	PO "%c%d", strlen (processData -> mask_name[i]) > 7 ? ' ' : '\t', processData -> CM[i] -> code);
	PO "\t%d", processData -> CM[i] -> fill);
	PO "\t%d", processData -> RT[i] -> code);
	PO "\t%d", processData -> RT[i] -> fill);
	PO "\t%d", processData -> PLOT[i] -> code);
	PO "\t%d\n", processData -> PLOT[i] -> fill);
    }
    PO hp);
    dmQuit ();
    exit (0);
    return (0);
}

void dmError (char *s)
{
    if (!process) {
	dmPerror (s);
	die ();
    }
}

void die ()
{
    dmQuit ();
    exit (1);
}
