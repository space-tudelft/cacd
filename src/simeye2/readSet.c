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

#include "src/simeye2/define.h"
#include <sys/types.h>
#include <sys/stat.h>

extern char *defaultCommandFile;
extern DM_PROJECT *projKey;

extern char *fn_cmd;
extern FILE *yyin;

extern struct node *Begin_node;
extern struct node *End_node;

extern double s_sigunit;
extern double sigtimeunit;
extern int sigendless;
extern int errorDetected;

char readSetMes[160];

simtime_t curr_time;
simtime_t newEndtime;
double Timeconvert;
struct signal *Old_end_signal;

int addSP_level;

void cmdinits (void); // cmd_y.y
int  yyparse (void); // cmd_y.y

int readSet (char *name)
{
    char help[256];
    char *commandfile;
    char *defaultfile;
    int foundCmd;
    struct signal *sig;
    struct node *n;
    char c;
    FILE *fp_default;
    FILE *fp_copy;
    struct stat sbuf;

    readSetMes[0] = '\0';

    NEW (commandfile, strlen (name) + 5, char);
    sprintf (commandfile, "%s.cmd", name);

    foundCmd = 0;
    usedNonCapital = 0;

    if (stat (commandfile, &sbuf) == 0) foundCmd = 1;

    if (!foundCmd && tryNonCapital && isupper ((int)*commandfile)) {

	/* decapitalize the first character and try to open that file */
	*commandfile += LOWER;
	if (stat (commandfile, &sbuf) == 0) {
	    foundCmd = 1;
	    usedNonCapital = 1;
	    sprintf (readSetMes, "editing \"%s\" instead", commandfile);
	}
	else
	    *commandfile -= LOWER;
    }

    if (!foundCmd) {

	/* try default commandfile */

        fp_default = NULL;
        fp_copy = NULL;

	defaultfile = defaultCommandFile;
	if (!(fp_default = fopen (defaultfile, "r"))) {
	    if (projKey) {
		if ((defaultfile = (char *)dmGetMetaDesignData (PROCPATH, projKey, defaultCommandFile)))
		    fp_default = fopen (defaultfile, "r");
	    }
        }

	if (!fp_default) {

	    delSigList (Begin_signal);

	    Nr_signals = 0;

	    Begin_signal = NULL;
	    End_signal = NULL;

	    return (2);                            /* file does not exist */
	}

	fp_copy = fopen (commandfile, "w");
	if (fp_copy) {
	    sprintf (readSetMes, "created \"%s\" from template \"%s\"", commandfile, defaultfile);
	}
	else {
	    sprintf (readSetMes, "Cannot write %s\n", commandfile);
	    return (0);
	}

        if (fp_copy) {
	    /* make a copy of the default commandfile */
	    while ((c = getc (fp_default)) != EOF) putc (c, fp_copy);
	    fclose (fp_default);
	    fclose (fp_copy);
	}
    }

    fn_cmd = commandfile;

    updateCommandfile (name);

    if (!(yyin = fopen (commandfile, "r"))) {
	sprintf (readSetMes, "Cannot open %s\n", commandfile);
	return (0);
    }

    cmdinits ();

    yyparse ();

    fclose (yyin);

    if (errorDetected) {
	readSetMes[0] = '\0';
	/* a windowMessage has already been printed,
	   so do not destroy the message on the screen */
        return (0);
    }

    if (simperiod <= 0 && sigendless) {
	sprintf (help, "no end time for input signals in %s", commandfile);
	windowMessage (help, -1);
	return (0);
    }
    if (simperiod >= MAXTIME) {
	sprintf (help, "too large simperiod in %s", commandfile);
	windowMessage (help, -1);
	return (0);
    }

    if (sigtimeunit <= 0)
	sigtimeunit = 1;

    if (Append) {

	Old_end_signal = End_signal;

	/* maintain old value of Timescaling for Timescaling */

	Timeconvert = sigtimeunit / Timescaling;
    }
    else {
	delSigList (Begin_signal);

	Nr_signals = 0;

	Begin_signal = NULL;
	End_signal = NULL;

	Timescaling = sigtimeunit;
    }
    Voltscaling = 2.5;

    if (simperiod >= 0 && sigendless) {

	if (Append) {
	    if (simperiod * Timeconvert * 10 > MAXTIME) {
		windowMessage ("New time values too large (as compared to old values)", -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		return (0);
	    }

	    newEndtime = simperiod * Timeconvert;
	}
	else
	    newEndtime = simperiod;
    }
    else
	newEndtime = 0;  /* for initialization */

    n = Begin_node;
    while (n) {
	Nr_signals++;
	NEW (sig, 1, struct signal);
	sig -> name = n -> name;
	sig -> no_edit = n -> no_edit;
	sig -> endless = 0;  /* possibly set in addSgnPart or during editing */
	sig -> stringValue = 0;
	sig -> next = NULL;
	sig -> prev = NULL;
	sig -> layover = NULL;
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
        if (addSgnPart (sig, n -> expr, 1) < 0)
	    return (0);

	if (sig -> endless && addSP_level != 2)
	    sig -> endless = 0;     /* these types of (nested) endless signals
				       are not allowed since they can not be
				       written back */

        n = n -> next;
    }

    if (simperiod > 0 && !sigendless) {

	/* check if newEndtime should be enlarged */

	if (Append) {
	    if (simperiod * Timeconvert * 10 > MAXTIME) {
		windowMessage ("New time values too large (as compared to old values)", -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		return (0);
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
	sig = sig -> next;
    }

    if (Append)
	Endtime = Max (Endtime, newEndtime);
    else {
	Begintime = 0;
	Endtime = newEndtime;
    }
    SimEndtime = Endtime;

    Global_umin = 0;
    Global_umax = 2;

    return (1);
}

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
		     if (sibling_expr -> val == H_state) value = 2;
		else if (sibling_expr -> val == L_state) value = 0;
		else if (sibling_expr -> val == X_state) value = 1;
		else if (sibling_expr -> val == F_state) value = -1;
		else {
		    fprintf (stderr, "He ? val = %d\n", sibling_expr -> val);
		    value = 1;
		}

		if (!sig -> end_value || sig -> end_value -> value != value) {

		    NEWSVAL (sval);

                    if (Append) {
			if (curr_time * Timeconvert * 10 > MAXTIME) {
			    windowMessage ("New time values too large (as compared to old values)", -1);
			    delSigList (Old_end_signal -> next);
			    Old_end_signal -> next = NULL;
			    return (0);
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

int updateCommandfile (char *name)
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
    DM_STREAM *dsp = NULL;
    char nodename[MAXNAME];
    int i;
    int copycommand;
    int disablecommand;
    int comment;
    int commentAtBegin;
    int found;
    int l;

    sprintf (commandfile, "%s.cmd", modNameOf (name));
    sprintf (tmpfilename, "%d.cxx", (int)getpid ());
    usedNonCapital = 0;

    /* set 'option simlevel' to its correct value
       and update 'automatic' set and print commands */

    if (!(fp2 = fopen (tmpfilename, "w"))) {
	sprintf (command, "Cannot open %s", tmpfilename);
	goto err;
    }

    if (!(fp1 = fopen (commandfile, "r"))) {
	if (tryNonCapital && isupper ((int)*commandfile)) {

	    /* decapitalize the first character and try to open that file */
	    *commandfile += LOWER;
	    fp1 = fopen (commandfile, "r");
	    if (fp1)
		usedNonCapital = 1;
	    else
		*commandfile -= LOWER;
	}
	if (!fp1) {
	    sprintf (command, "Cannot read %s", commandfile);
	    goto err;
	}
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

	ungetc (c, fp1);

	firstcharpos = ftell (fp1);

	while ((c = getc (fp1)) == ' ' || c == '\t');

	commentAtBegin = 0;
	if (c == '/' && (c = getc (fp1)) == '*') {
	    commentAtBegin = 1;
	    while ((c = getc (fp1)) == ' ' || c == '\t');
	}
	ungetc (c, fp1);

	word = getword (fp1);

        if (commentAtBegin) {
		 if (compareWord (word, "auto_set")) autoSetPresent = 1;
	    else if (compareWord (word, "auto_print")) autoPrintPresent = 1;
	    else if (compareWord (word, "auto_plot")) autoPlotPresent = 1;
	}

	backslash = 0;
	if (word[0] != '\n' && word[0] != EOF)
	while (1) {
	    word = getword (fp1);
	    if ((word[0] == '\n' && !backslash) || word[0] == EOF) break;

	    if (word[0] == '\\')
		backslash = 1;
	    else if (word[0] != ' ' && word[0] != '\t' && !(word[0] == '\n' && backslash))
		backslash = 0;
	}
    }

    rewind (fp1);

    if (projKey && (autoSetPresent || autoPrintPresent || autoPlotPresent)) {
	cellKey = dmCheckOut (projKey, name, WORKING, DONTCARE, CIRCUIT, READONLY);
	if (cellKey) dsp = dmOpenStream (cellKey, "term", "r");
    }

    if (dsp) {
	dm_get_do_not_alloc = 1;
	cterm.term_attribute = attribute_string;
	cterm.term_lower = lower;
	cterm.term_upper = upper;
    }

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
	else {
	    if (compareWord (word, "print") || compareWord (word, "plot")) {

		while ((c = getc (fp1)) == ' ' || c == '\t');

		if (c == '/' && (c = getc (fp1)) == '*') {

		    while ((c = getc (fp1)) == ' ' || c == '\t');
		    ungetc (c, fp1);

		    word = getword (fp1);
		    if (compareWord (word, "auto")) {
			copycommand = 0;         /* it is an 'automatic' print/plot command */
		    }
		}
	    }
	    if (autoSetPresent && compareWord (word, "set")) {

		while ((c = getc (fp1)) == '\\' || isspace ((int)c));

                nodename[0] = '\0';
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
			while ((c = getc (fp1)) == '_' || isalnum ((int)c)) nodename[i++] = c;
			nodename[i] = '\0';
		    }
                }

		/* If we have have found a nodename of a non-editable set
                   command, look if nodename is part of the terminal list */

		found = 1;
		if (nodename[0] != '\0' && dsp) {
		    found = 0;
		    while (dmGetDesignData (dsp, CIR_TERM) > 0 && !found) {
			if (strsame (cterm.term_name, nodename)) found = 1;
		    }
		    dmSeek (dsp, 0L, 0L);  /* rewind */
		}
		if (!found) disablecommand = 1;
	    }
	    if (compareWord (word, "end_disabled")) copycommand = 0;
	}

        if (disablecommand) {
	    fprintf (fp2, "/* begin_disabled\n");
	}

	fseek (fp1, firstcharpos, 0);

	simlevelPre = 0; simlevelNow = 0;
	simperiodPre = 0; simperiodNow = 0;
	sigunitPre = 0; sigunitNow = 0;

	backslash = 0;
	while (1) {

	    word = getword (fp1);

            l = strlen (word);
            for (i = 0; i < l; i++) {
                if (word[i] == '/' && word[i+1] == '*')
                    comment = 1;
                else if (word[i] == '*' && word[i+1] == '/')
                    comment = 0;
            }

	    if (simlevelNow && isdigit ((int)*word)) {
		fprintf (fp2, "%d ", SimLevel);
		simlevelNow = 0;
		simlevelDone = 1;
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

		l = strlen (word);
                if (disablecommand) {
		    if (word[l - 2] == '/' && word[l - 1] == '*') word[l - 2] = '#';
		    if (word[l - 2] == '*' && word[l - 1] == '/') word[l - 1] = '#';
		}
		else {
		    if (word[l - 2] == '#' && word[l - 1] == '*') word[l - 2] = '/';
		    if (word[l - 2] == '*' && word[l - 1] == '#') word[l - 1] = '/';
		}
		fprintf (fp2, "%s", word);
	    }

            if (!comment) {
		if (simlevelPre) {
		    if (word[0] == '=') {
			simlevelNow = 1;
			simlevelPre = 0;
		    }
		    else if (word[0] != '\\')
			simlevelPre = 0;
		}

		if (simperiodPre) {
		    if (word[0] == '=') {
			simperiodNow = 1;
			simperiodPre = 0;
		    }
		    else if (word[0] != '\\')
			simperiodPre = 0;
		}

		if (sigunitPre) {
		    if (word[0] == '=') {
			sigunitNow = 1;
			sigunitPre = 0;
		    }
		    else if (word[0] != '\\')
			sigunitPre = 0;
		}

		if (compareWord (word, "level")) simlevelPre = 1;
		if (compareWord (word, "simperiod")) simperiodPre = 1;
		if (compareWord (word, "sigunit")) sigunitPre = 1;
	    }

	    if ((word[0] == '\n' && !backslash) || word[0] == EOF) break;

	    if (word[0] == '\\')
		backslash = 1;
	    else if (word[0] != ' ' && word[0] != '\t' && !(word[0] == '\n' && backslash))
		backslash = 0;
	}

        if (disablecommand) fprintf (fp2, "   end_disabled */\n");
    }

    if (s_sigunit <= 0) s_sigunit = 1; /* default */

    /* then, add simlevel (if not yet defined) and add print/plot commands
       that are automatically determined from the terminal list of the cell
       that is simulated  */

    if (!simlevelDone) fprintf (fp2, "option level = %d\n", SimLevel);

    if ((autoPrintPresent || autoPlotPresent) && dsp) {

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
	sprintf (command, "Cannot open %s", tmpfilename);
	goto err;
    }

    c1 = getc (fp1);
    c2 = getc (fp2);
    while (c1 == c2 && c1 != EOF) {
	c1 = getc (fp1);
	c2 = getc (fp2);
    }

    /* close files */

    if (dsp) dmCloseStream (dsp, COMPLETE);
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
	    goto err;
	}
	sprintf (command, "mv %s %s", tmpfilename, commandfile);
	system (command);
    }
    else {
	unlink (tmpfilename);
    }

    return (0);
err:
    if (fp2) fclose (fp2);
    unlink (tmpfilename);
    windowMessage (command, -1);
    return (1);
}

int yywrap ()
{
    return (1); /* only one file is read */
}
