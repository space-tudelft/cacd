/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

/* #define DEBUG */

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

extern char *defCmdFile;
extern DM_PROJECT *projKey;
extern FILE *yyin;
int yyparse (void);

extern struct node * Begin_node;
extern struct node * End_node;

extern double sigtimeunit;
extern double s_sigunit;
extern int sigendless;
extern int errorDetected;

simtime_t curr_time;
simtime_t newEndtime;
double Timeconvert;
struct signal *Old_end_signal;

int addSP_level;
char Commandfile[MAXNAME+6];

FILE *fp_err;
char fn_err[12] = "";

void readSet (int done)
{
    char help[MAXNAME+80];
    char *defaultfile;
    struct signal *sig;
    struct stat sbuf;
    struct node *n;
    char c;
    FILE *fp_default = NULL;
    FILE *fp_copy;
    int OK = 0;
    int foundCmd = 0;

    *help = 0;

    if (done) goto try_default;

    if (!*inputname) {
        strcpy (help, "no commandfile specified!");
	goto err;
    }

    strcpy (Commandfile, inputname);
    if (stat (Commandfile, &sbuf) == 0) foundCmd = 1;
    else if (tryNonCapital && isupper ((int)Commandfile[0])) {
	/* decapitalize the first character and try to open that file */
	Commandfile[0] += LOWER;
	if (stat (Commandfile, &sbuf) == 0) {
	    foundCmd = 1;
	    usedNonCapital = 1;
	    sprintf (help, "editing \"%s\" instead", Commandfile);
	}
	else
	    Commandfile[0] -= LOWER;
    }

    if (!foundCmd) { /* try default commandfile */
	sprintf (help, "Commandfile \"%s\" not found!\nDo you want to create it from template \"%s\"?", inputname, defCmdFile);
	ask_default (help);
	return;

try_default:
	if (done > 1) goto err; /* Cancel */

	defaultfile = defCmdFile;
	if (!(fp_default = fopen (defaultfile, "r")) && projKey) {
	    if ((defaultfile = (char *) dmGetMetaDesignData (PROCPATH, projKey, defCmdFile)))
		fp_default = fopen (defaultfile, "r");
	}
	if (!fp_default) {
	    strcpy (help, "Commandfile not found, cannot open defaultfile");
	    goto err;
	}

	if (!(fp_copy = fopen (Commandfile, "w"))) {
	    sprintf (help, "Cannot create \"%s\"!", Commandfile);
	    goto err;
	}
	sprintf (help, "created \"%s\" from template \"%s\"", Commandfile, defaultfile);

	/* make a copy of the default commandfile */

	while ((c = getc (fp_default)) != EOF) putc (c, fp_copy);

	fclose (fp_copy);
    }

    if (!(yyin = fopen (Commandfile, "r"))) {
	sprintf (help, "Cannot open 'cmd' file \"%s\"", Commandfile);
	goto err;
    }

    if (!*fn_err) sprintf (fn_err, "p%d.cxx", (int)getpid ());
    fp_err = fopen (fn_err, "w");

    cmdinits ();
    yyparse ();

    if (fp_err) fclose (fp_err);

    fclose (yyin);

    if (errorDetected) {
	if (fp_err) windowList (fn_err);
	*help = 0;
	goto err;
    }

    if (simperiod <= 0 && sigendless) {
	sprintf (help, "no end time for input signals in \"%s\"", Commandfile);
	goto err;
    }
    if (simperiod >= MAXTIME) {
	sprintf (help, "too large simperiod in \"%s\"", Commandfile);
	goto err;
    }

    if (sigtimeunit <= 0) {
	sprintf (help, "illegal sigunit <= 0 in \"%s\"", Commandfile);
	goto err;
    }

    if (Append) {

	Old_end_signal = End_signal;

	/* maintain old value of Timescaling for Timescaling */

	Timeconvert = sigtimeunit / Timescaling;
    }
    else {
	Timescaling = sigtimeunit;
    }
    Voltscaling = 2.5;

    if (simperiod >= 0 && sigendless) {

	if (Append) {
	    if (simperiod * Timeconvert * 10 > MAXTIME) {
		sprintf (help, "New time values too large in \"%s\"", Commandfile);
		windowMessage (help, -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		*help = 0;
		goto err;
	    }
	    newEndtime = simperiod * Timeconvert;
	}
	else
	    newEndtime = simperiod;
    }
    else
	newEndtime = 0;  /* for initialization */

    for (n = Begin_node; n; n = n -> next) {
	Nr_signals++;
	NEW (sig, 1, struct signal);
	sig -> name = n -> name;
	sig -> no_edit = n -> no_edit;
	sig -> endless = 0;  /* possibly set in addSgnPart or during editing */
	sig -> stringValue = 0;
	sig -> next = NULL;
	sig -> prev = NULL;
	sig -> layover2 = NULL;
	sig -> layover3 = NULL;
	sig -> layover4 = NULL;
	sig -> layover5 = NULL;
	sig -> layover6 = NULL;
	sig -> begin_value = NULL;
	sig -> end_value = NULL;
	sig -> begin_value_L = NULL;
	sig -> end_value_L = NULL;
	sig -> begin_value_U = NULL;
	sig -> end_value_U = NULL;
	if (End_signal) {
	    End_signal -> next = sig;
	    sig -> prev = End_signal;
	    End_signal = sig;
	}
	else {
	    Begin_signal = End_signal = sig;
	}

	addSP_level = 0;
        curr_time = 0;
	sig -> expr = n -> expr;
        if (addSgnPart (sig, n -> expr, 1) < 0) { *help = 0; goto err; }

	if (sig -> endless && addSP_level != 2)
	    sig -> endless = 0;     /* these types of (nested) endless signals
				       are not allowed since they can not be
				       written back */
    }

    if (simperiod > 0 && !sigendless) {

	/* check if newEndtime should be enlarged */

	if (Append) {
	    if (simperiod * Timeconvert * 10 > MAXTIME) {
		sprintf (help, "New time values too large in \"%s\"", Commandfile);
		windowMessage (help, -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		*help = 0;
		goto err;
	    }

            if (simperiod * Timeconvert > newEndtime)
	        newEndtime = simperiod * Timeconvert;
	}
	else {
            if (simperiod > newEndtime)
	        newEndtime = simperiod;
	}
    }

    /* make all new signals end at the time that is Endtime
       for the current input file */

    if (Append) {
	sig = Old_end_signal -> next;
    }
    else {
	sig = Begin_signal;
    }
    while (sig) {
	adjustSgnPart (sig);
#ifdef DEBUG
	printSgnPart (sig, 0);
#endif
	sig = sig -> next;
    }

    if (Append)
	Endtime = Max (Endtime, newEndtime);
    else {
	Begintime = 0;
	Endtime = newEndtime;
    }
    SimEndtime = Endtime;
    OK = 1;

err:
    if (fp_default) fclose (fp_default);
    if (OK) {
	set_filename (Commandfile);
	draw ('f', 0, 0, 0, 0);
    }
    else {
	somethingChanged = 0;
	set_filename (NULL);
    }
    if (*help) windowMessage (help, -1);
    errorMessage (NULL);
}

#ifdef DEBUG

#define PE fprintf (stderr,

printSgnPart (struct signal *sig, char type)
{
    struct sig_value *sv, *se;
    char *s;

    if (type == 'l') {
        sv = sig -> begin_value_L;
        se = sig -> end_value_L;
	s = "_L";
    }
    else if (type == 'u') {
        sv = sig -> begin_value_U;
        se = sig -> end_value_U;
	s = "_U";
    }
    else {
        sv = sig -> begin_value;
        se = sig -> end_value;
	s = "";
    }

    PE "\nsig=%10u begin=%10u end=%10u name=\"%s\" values%s:\n", sig, sv, se, sig -> name, s);

    if (*s) s = "value";
    else if (sig -> stringValue) s = "sval";

    if (sv)
    do {
	PE "%10u: prev=%10u next=%10u time=%5lld ", sv, sv -> prev, sv -> next, sv -> time);
	if (*s) PE "%s=%d\n", s, sv -> value);
	else PE "val='%c'\n", eval2cmdval (sv -> value));
    } while (sv = sv -> next);
}
#endif

void adjustSgnPart (struct signal *sig)
{
    struct sig_value *sval;
    struct sig_value *sval2;

    if (sig -> end_value) {
	if (sig -> end_value -> time > newEndtime) {
	    sval = sig -> end_value;
	    while (sval && sval -> time > newEndtime) {
		sval2 = sval;
		sval = sval -> prev;
		DELETESVAL (sval2);
	    }
	    if (sval) {
		sval -> next = NULL;
		sig -> end_value = sval;
	    }
	    else {
		sig -> begin_value = NULL;
		sig -> end_value = NULL;
	    }
	}
	if (sig -> end_value -> time < newEndtime) {
	    NEWSVAL (sval);
	    sval -> time = newEndtime;
	    sval -> value = sig -> end_value -> value;
	    sval -> next = NULL;
	    sval -> prev = sig -> end_value;
	    sig -> end_value -> next = sval;
	    sig -> end_value = sval;
	}
    }
}

int addSgnPart (struct signal *sig, SIGNALELEMENT *expr, int nr)
{
    struct sig_value *sval;
    SIGNALELEMENT *sibling_expr;
    int newnr, ret, value;

    if (addSP_level < 100) addSP_level++;

    while ((nr > 0 || nr == -1) && (curr_time < simperiod || (simperiod < 0 && curr_time >= 0))) {

	sibling_expr = expr -> sibling;

	while (sibling_expr) {

	    if (sibling_expr -> child) {
		if (sibling_expr -> len > 0 && sibling_expr -> child -> len > 0) {
		    newnr = sibling_expr -> len / sibling_expr -> child -> len;
		}
		else {
		    newnr = -1;
		}
		ret = addSgnPart (sig, sibling_expr -> child, newnr);
		if (ret) return (ret);
	    }
	    else {
		value = sibling_expr -> val;

		if (!sig -> end_value || sig -> end_value -> value != value) {

		    NEWSVAL (sval);

                    if (Append) {
			if (curr_time * Timeconvert * 10 > MAXTIME) {
			    windowMessage ("New time values too large (as compared to old values)", -1);
			    delSigList (Old_end_signal -> next);
			    Old_end_signal -> next = NULL;
			    return (-1);
			}
			sval -> time = curr_time * Timeconvert;
		    }
		    else {
			sval -> time = curr_time;
		    }

		    sval -> value = value;
		    sval -> next = NULL;
		    sval -> prev = NULL;

		    if (sig -> end_value) {
			sig -> end_value -> next = sval;
			sval -> prev = sig -> end_value;
			sig -> end_value = sval;
		    }
		    else {
			sig -> begin_value = sig -> end_value = sval;
		    }
		}

                if (sibling_expr -> len > 0) {
		    curr_time = curr_time + sibling_expr -> len;
		    if (curr_time > MAXTIME || curr_time < 0) {
			if (sigendless) return (1); /* end of signal reached */
			windowMessage ("Too large time value", -1);
			return (-1);
		    }
		}
		else
		    curr_time = simperiod;

		if (!sigendless) {
		    if (Append) {
			if (curr_time * Timeconvert * 10 > MAXTIME) {
			    windowMessage ("New time values too large (as compared to old values)", -1);
			    delSigList (Old_end_signal -> next);
			    Old_end_signal -> next = NULL;
			    return (-1);
			}
		    }
		    if (simperiod >= 0 && simperiod < curr_time) {
			if (simperiod > newEndtime) newEndtime = simperiod;
		    }
		    else
			if (curr_time > newEndtime) newEndtime = curr_time;
		}

		if (sigendless && curr_time > simperiod) {
		    return (1);                 /* end of signal reached */
		}
	    }

	    sibling_expr = sibling_expr -> sibling;
	}

        if (nr == -1)
	    sig -> endless = 1;
	else
	    nr--;
    }

    return (0);
}

int updateCommandfile (char *cname, char *sname)
{
    long lower[10], upper[10];
    char attribute_string[256];
    char commandfile[MAXNAME];
    char tmpfilename[MAXNAME];
    char command[MAXNAME+MAXNAME+20];
    FILE *fp1, *fp2;
    char c, c1, c2;
    char *word;
    char buf[128];
    int backslash;
    int simlevelPre, simlevelNow, simlevelDone;
    int simperiodPre, simperiodNow, simperiodDone;
    int sigunitPre, sigunitNow, sigunitDone;
    int autoSetPresent;
    int autoPrintPresent;
    int autoPlotPresent;
    long firstcharpos;
    DM_CELL *cellKey = NULL;
    DM_STREAM *dsp;
    char nodename[MAXNAME];
    int i;
    int copycommand;
    int disablecommand;
    int comment;
    int commentAtBegin;

    /*
     * set 'option level' to its correct value and
     * update 'auto_set', 'auto_print' and 'auto_plot' commands
     */

    fp1 = NULL;
    sprintf (tmpfilename, "u%d.cxx", (int)getpid ());
    if (!(fp2 = fopen (tmpfilename, "w"))) {
	sprintf (command, "Cannot open file \"%s\"!", tmpfilename);
	*tmpfilename = 0;
	goto ret;
    }

    strcpy (commandfile, stimuliname);
    if (!(fp1 = fopen (commandfile, "r"))) {
	sprintf (command, "Cannot read \"%s\"!", stimuliname);
	goto ret;
    }

    simlevelDone = 0;
    simperiodDone = 0;
    sigunitDone = 0;
    autoSetPresent = 0;
    autoPrintPresent = 0;
    autoPlotPresent = 0;

    s_simperiod = -1;
    s_sigunit = -1;

    /* first, look if terminals have to be read */

    while ((c = getc (fp1)) != EOF) {

        /* read command by command */

	while (c == ' ' || c == '\t') c = getc (fp1);

	commentAtBegin = 0;
	if (c == '/' && (c = getc (fp1)) == '*') {
	    commentAtBegin = 1;
	    while ((c = getc (fp1)) == ' ' || c == '\t');
	}
	ungetc (c, fp1);

	word = getword (fp1);
	if (word[0] == '\n') continue;

        if (commentAtBegin) {
	    if (compareWord (word, "auto_set")) {
		autoSetPresent = 1;
		if (autoPrintPresent && autoPlotPresent) break;
	    }
	    else if (compareWord (word, "auto_print")) {
		autoPrintPresent = 1;
		if (autoSetPresent && autoPlotPresent) break;
	    }
	    else if (compareWord (word, "auto_plot")) {
		autoPlotPresent = 1;
		if (autoSetPresent && autoPrintPresent) break;
	    }
	}

	backslash = 0;
	for (;;) {
	    word = getword (fp1);
	    if ((word[0] == '\n' && !backslash) || word[0] == EOF) break;
	    backslash = (word[0] == '\\') ? 1 : 0;
	}
    }

    rewind (fp1);

    cellKey = dmCheckOut (projKey, cname, WORKING, DONTCARE, CIRCUIT, READONLY);
    if (!cellKey) {
	sprintf (command, "Cannot read circuit of cell \"%s\"!", circuitname);
	goto ret;
    }

    dsp = NULL;
    if (autoSetPresent || autoPrintPresent || autoPlotPresent) {
	dsp = dmOpenStream (cellKey, "term", "r");
    }

    dm_get_do_not_alloc = 1;
    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    /* then, copy fp1 to fp2 but
       without 'automatic' print/plot commands,
       set commands enabled or disabled according to presence in terminal list
       (if auto_set has been defined),
       and option simlevel changed to its correct value */

    comment = 0;
    while ((c = getc (fp1)) != EOF) {

        /* read command by command */

	ungetc (c, fp1);

	firstcharpos = ftell (fp1);

	while ((c = getc (fp1)) == ' ' || c == '\t');

	copycommand = 1;
	disablecommand = 0;

	commentAtBegin = 0;
	if (c == '/' && (c = getc (fp1)) == '*') {
	    commentAtBegin = 1;
	    while ((c = getc (fp1)) == ' ' || c == '\t');
	}
	ungetc (c, fp1);

	word = getword (fp1);

        if (commentAtBegin) {
	    if (compareWord (word, "begin_disabled")) copycommand = 0;
	}
	else if (compareWord (word, "print") || compareWord (word, "plot")) {

	    if ((c = getc (fp1)) == '/' && (c = getc (fp1)) == '*') {

		while ((c = getc (fp1)) == ' ' || c == '\t');
		ungetc (c, fp1);

		word = getword (fp1);
		if (compareWord (word, "auto")) copycommand = 0;
	    }
	}
	else if (compareWord (word, "end_disabled")) {
	    copycommand = 0;
	}
	else if (dsp && autoSetPresent && compareWord (word, "set")) {

	    c = getc (fp1);

	    if ((c == '/' || c == '#') && (c = getc (fp1)) == '*') {

		while ((c = getc (fp1)) == ' ' || c == '\t');
		ungetc (c, fp1);

		word = getword (fp1);
		if (compareWord (word, "no_edit")) {

		    c2 = ' ';
		    while (c != EOF && !(c2 == '*' && (c == '/' || c == '#'))) {
			c2 = c;
			c = getc (fp1);
		    }

		    while ((c = getc (fp1)) == '\\' || isspace ((int)c));
		    ungetc (c, fp1);

		    i = 0;
		    while ((c = getc (fp1)) == '_' || isalnum ((int)c)) {
			if (i < MAXNAME) nodename[i++] = c;
		    }
		    nodename[i] = '\0';

		    /* If we have found a nodename of a non-editable set
		       command, look if it is part of the terminal list */

		    if (i) {
			disablecommand = 1;
			while (dmGetDesignData (dsp, CIR_TERM) > 0) {
			    if (strsame (cterm.term_name, nodename)) {
				disablecommand = 0; /* found */
				break;
			    }
			}
			dmSeek (dsp, 0L, 0L);  /* rewind */
		    }
		}
	    }
	}

        if (disablecommand) fprintf (fp2, "/* begin_disabled\n");

	fseek (fp1, firstcharpos, 0);

	simlevelPre = 0; simlevelNow = 0;
	simperiodPre = 0; simperiodNow = 0;
	sigunitPre = 0; sigunitNow = 0;

	backslash = 0;
	for (;;) {

	    word = getword (fp1);

	    if (word[0] == '/' && word[1] == '*')
		comment = 1;
	    else if (word[0] == '*' && word[1] == '/')
		comment = 0;

	    if (simlevelNow && isdigit ((int)word[0])) {
		simlevelNow = 0;
		simlevelDone = 1;
		fprintf (fp2, "%d", SimLevel);
	    }
	    else if (simperiodNow) {
		sscanf (word, "%lld", &s_simperiod);
		simperiodNow = 0;
		simperiodDone = 1;
		fprintf (fp2, "%s", word);
	    }
	    else if (sigunitNow) {
		sscanf (word, "%s", buf);  /* to remove trailing spaces */
		s_sigunit = slstof (buf);
		sigunitNow = 0;
		sigunitDone = 1;
		fprintf (fp2, "%s", word);
	    }
	    else if (copycommand && word[0] != EOF) {

		/* see if comment has to be converted */

                if (disablecommand) {
		    if (word[0] == '/' && word[1] == '*') word[0] = '#';
		    if (word[0] == '*' && word[1] == '/') word[1] = '#';
		}
		else {
		    if (word[0] == '#' && word[1] == '*') word[0] = '/';
		    if (word[0] == '*' && word[1] == '#') word[1] = '/';
		}

		fprintf (fp2, "%s", word);
	    }

	    if (!comment && word[0] != '\\' && word[0] != ' ' && word[0] != '\t') {
		if (simlevelPre) {
		    if (word[0] == '=') simlevelNow = 1;
		    simlevelPre = 0;
		}
		else if (simperiodPre) {
		    if (word[0] == '=') simperiodNow = 1;
		    simperiodPre = 0;
		}
		else if (sigunitPre) {
		    if (word[0] == '=') sigunitNow = 1;
		    sigunitPre = 0;
		}
		else if (compareWord (word, "level"))
		    simlevelPre = 1;
		else if (compareWord (word, "simperiod"))
		    simperiodPre = 1;
		else if (compareWord (word, "sigunit"))
		    sigunitPre = 1;
	    }

	    if ((word[0] == '\n' && !backslash) || word[0] == EOF) break;

	    backslash = (word[0] == '\\') ? 1 : 0;
	}

        if (disablecommand) fprintf (fp2, "   end_disabled */\n");
    }

    if (s_sigunit <= 0) s_sigunit = 1; /* default */

    /* then, add simlevel (if not yet defined) and add print/plot commands
       that are automatically determined from the terminal list of the cell
       that is simulated  */

    if (!simlevelDone) fprintf (fp2, "option level = %d\n", SimLevel);

    if (dsp && (autoPrintPresent || autoPlotPresent)) {

	while (dmGetDesignData (dsp, CIR_TERM) > 0) {

	    if (autoPrintPresent) {
		fprintf (fp2, "print /* auto */ %s", cterm.term_name);
		for (i = 0; i < cterm.term_dim; i++) {
		    if (i == 0)
			fprintf (fp2, "[");
		    else
			fprintf (fp2, ",");
		    fprintf (fp2, "%ld..%ld", cterm.term_lower[i], cterm.term_upper[i]);
		    if (i == cterm.term_dim - 1) fprintf (fp2, "]");
		}
		fprintf (fp2, "\n");
	    }

	    if (autoPlotPresent) {
		fprintf (fp2, "plot /* auto */ %s", cterm.term_name);
		for (i = 0; i < cterm.term_dim; i++) {
		    if (i == 0)
			fprintf (fp2, "[");
		    else
			fprintf (fp2, ",");
		    fprintf (fp2, "%ld..%ld", cterm.term_lower[i], cterm.term_upper[i]);
		    if (i == cterm.term_dim - 1) fprintf (fp2, "]");
		}
		fprintf (fp2, "\n");
	    }
	}
    }

    /* check if the files are different */

    rewind (fp1);
    fclose (fp2);
    if (!(fp2 = fopen (tmpfilename, "r"))) {
	sprintf (command, "Cannot open \"%s\"", tmpfilename);
	goto ret;
    }

    c1 = getc (fp1);
    c2 = getc (fp2);
    while (c1 == c2 && c1 != EOF) {
	c1 = getc (fp1);
	c2 = getc (fp2);
    }

    /* close files */

    if (cellKey) dmCheckIn (cellKey, COMPLETE);

    fclose (fp1);
    fclose (fp2);

    /* finally move tmpfile to commandfile if something has changed */

    if (c1 != c2) { /* different */
	struct stat sbuf;
	int rv = stat (commandfile, &sbuf);
	if (rv == 0 && !(sbuf.st_mode & 0200)) {
	    chmod (commandfile, 0644);
	    rv = stat (commandfile, &sbuf);
	}
	if (rv != 0 || !(sbuf.st_mode & 0200)) {
	    sprintf (command, "Cannot %s \"%s\"", rv? "stat" : "chmod", commandfile);
	    goto ret;
	}
	sprintf (command, "mv %s %s", tmpfilename, commandfile);
	system (command);
    }
    else {
	unlink (tmpfilename);
    }
    return (1); /* OK */

ret:
    windowMessage (command, -1);

    if (cellKey) dmCheckIn (cellKey, QUIT);

    if (fp1) fclose (fp1);
    if (fp2) fclose (fp2);

    if (*tmpfilename) unlink (tmpfilename);

    return (0); /* ERROR */
}

int yywrap ()
{
    return (1); /* only one file is read */
}
