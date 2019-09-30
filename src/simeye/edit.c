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

#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"
#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

extern DM_PROJECT *projKey;
extern Widget clickedCommandw;

struct sig_value *storedVals = NULL;
simtime_t storedTD;

int editing = 0;
int somethingChanged = 0;

void Delsig (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, DELSIG);
}

void Copysig (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, COPYSIG);
}

void Change (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, CHANGE);
}

void Rename (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, RENAME);
}

void Yank (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();
    beginCommand (w, YANK);
}

void Put (Widget w, caddr_t client_data, caddr_t call_data)
{
    if (clickedCommandw) endCommand ();

    if (!storedVals) {
	windowMessage ("empty buffer", -1);
	return;
    }
    beginCommand (w, PUT);
}

void disableEditing ()
{
    somethingChanged = 0;
    if (storedVals) {
        delSigval (storedVals, (struct sig_value *)0);
	storedVals = NULL;
    }
    editing = 0;
}

int changeSignal (struct signal *sig, simtime_t t1, simtime_t t2, int state, struct sig_value **prev)
/* prev : previous sig_value (if known!) */
{
    int after2value;
    struct sig_value *sval, *tmp, *new1, *new2;
    struct sig_value *before1, *before2;
    simtime_t help;

    if (t1 == t2) return (0);

    if (sig -> expr) { /* the periodicity of the signal is destroyed */
	delSigexpr (sig -> expr);
	sig -> expr = NULL;
	sig -> endless = 0;
    }

    if (t2 < t1) { help = t1; t1 = t2; t2 = help; }

    if (*prev) /* use prev to speed up searching process! */
	sval = *prev;
    else
	sval = sig -> begin_value;

    before1 = NULL;
    while (sval && sval -> time < t1) {
	before1 = sval;
	*prev = sval;
	sval = sval -> next;
    }

    /* if (before1 && sval) then: before1 -> next == sval */

    if (!before1 || before1 -> value != state) {

	NEWSVAL (new1);
	new1 -> time = sig -> begin_value ? t1 : 0;
	new1 -> value = state;
	new1 -> next = sval;
	new1 -> prev = before1;
	if (sval) sval -> prev = new1;

        if (!before1) {
            if (!sig -> begin_value) {
                sig -> begin_value = new1;
                sig -> end_value = new1;
            }
            else {
                sig -> begin_value = new1;
            }
            after2value = state;
        }
        else {
            before1 -> next = new1;
            after2value = before1 -> value;
        }
        before2 = new1;
    }
    else {
        /* else a previous sval already has a value 'state': */
        /*  before1 && before1 -> value == state             */

        before2 = before1;
        after2value = state;
    }

    while (sval && sval -> time <= t2) {

        /* delete everything with t1 <= time <= t2 */

        after2value = sval -> value;
        tmp = sval;
        sval = sval -> next;

        if (tmp -> next)
            tmp -> next -> prev = tmp -> prev;
        else
            sig -> end_value = tmp -> prev;

        if (tmp -> prev)
            tmp -> prev -> next = tmp -> next;
        else
            sig -> begin_value = tmp -> next;

        DELETESVAL (tmp);
    }

    if (sval && sval -> value == state && after2value == state) return (1);

    if (before2 && before2 -> prev && before2 -> value == before2 -> prev -> value) {
	before2 -> time = t2;
    }
    else {
	NEWSVAL (new2);
	new2 -> time = t2;
	new2 -> value = sval ? after2value : state;
	new2 -> next = sval;
	new2 -> prev = before2;
	if (sval)
	    sval -> prev = new2;
	else
	    sig -> end_value = new2;
	if (before2) before2 -> next = new2;
    }

    if (t2 > Endtime) Endtime = t2;
    if (t2 > SimEndtime) SimEndtime = t2;

    return (1);
}

void copySignal (struct signal *sig1, struct signal *sig2)
{
    struct sig_value *sval1, *sval2, *newval;
    SIGNALELEMENT *sigel, *last_sigel, *new_sigel;
    char flag;

    if (sig1 == sig2) return;

    somethingChanged = 1;

    delSigval (sig2 -> begin_value, sig2 -> end_value);
    sig2 -> begin_value = NULL;
    sig2 -> end_value = NULL;

    delSigexpr (sig2 -> expr);
    sig2 -> expr = NULL;

    sval2 = NULL;
    for (sval1 = sig1 -> begin_value; sval1; sval1 = sval1 -> next) {
	NEWSVAL (newval);
	newval -> time = sval1 -> time;
	newval -> value = sval1 -> value;
	newval -> next = NULL;
	newval -> prev = sval2;
	if (sval2)
	    sval2 -> next = newval;
	else
	    sig2 -> begin_value = newval;
	sval2 = newval;
    }
    sig2 -> end_value = sval2;

    last_sigel = NULL;
    flag = 's';
    sigel = sig1 -> expr;
    while (sigel) {
        NEW (new_sigel, 1, SIGNALELEMENT);
	new_sigel -> val = sigel -> val;
	new_sigel -> len = sigel -> len;
	if (sig2 -> expr) {
	    if (flag == 'c')
		last_sigel -> child = new_sigel;
	    else
		last_sigel -> sibling = new_sigel;
	}
	else
	    sig2 -> expr = new_sigel;
	last_sigel = new_sigel;

        if (sigel -> child) {
	    flag = 'c';
	    sigel = sigel -> child;
	}
	else {
	    flag = 's';
	    sigel = sigel -> sibling;
	}
    }

    sig2 -> endless = sig1 -> endless;
}

void storeSignalPart (struct signal *sig, simtime_t t1, simtime_t t2)
{
    struct sig_value *sval, *newval, *oldval;
    simtime_t help;

    if (storedVals) {
        delSigval (storedVals, (struct sig_value *)0);
	storedVals = NULL;
    }

    if (t1 == t2) return;
    if (t1 > t2) { help = t2; t2 = t1; t1 = help; }

    sval = sig -> begin_value;
    while (sval && sval -> time <= t1) sval = sval -> next;
    /* sval -> time > t1 */
    if (sval) sval = sval -> prev;
    /* sval -> time <= t1 && t1 < t2 */
    if (!sval) return;

    NEWSVAL (oldval);
    oldval -> time  = 0;
    oldval -> value = sval -> value;
    oldval -> prev  = NULL;
    storedVals = oldval;

    while ((sval = sval -> next) && sval -> time < t2) {
	NEWSVAL (newval);
	newval -> time  = sval -> time - t1;
	newval -> value = sval -> value;
	newval -> prev  = oldval;
	oldval -> next  = newval;
	oldval = newval;
    }
    oldval -> next = NULL;
    storedTD = t2 - t1;
}

/* PUT buffer */
int addStoredSignalPart (struct signal *sig, simtime_t t, int factor)
/* factor: how many times */
{
    struct sig_value *sval, *help;
    SIGNALELEMENT *new_sig_el, *last_sig_el, *new_part;
    simtime_t t1, t2, newlen;
    int i, rv;

    if (!storedVals) return (0); /* empty buffer */

    newlen = 0;

    if (factor < 0) { /* an endless part is added: use 'sig -> expr' to store the periodical part */

	/* delete old expr */

	delSigexpr (sig -> expr);

	NEW (sig -> expr, 1, SIGNALELEMENT);
	last_sig_el = sig -> expr;
	last_sig_el -> child = NULL;
	last_sig_el -> sibling = NULL;
	last_sig_el -> len = -1;
	sig -> endless = 1;

	/* create expr until t */

	sval = sig -> begin_value;
	t1 = 0;
	while (sval && t1 < t) {
	    if (sval -> time > t1) {
		NEW (new_sig_el, 1, SIGNALELEMENT);
		new_sig_el -> child = NULL;
		new_sig_el -> sibling = NULL;
		new_sig_el -> val = sval -> prev -> value;
		if (sval -> time < t)
		    new_sig_el -> len = sval -> time - t1;
		else
		    new_sig_el -> len = t - t1;

		last_sig_el -> sibling = new_sig_el;
		last_sig_el = new_sig_el;

		t1 = sval -> time;
	    }
	    sval = sval -> next;
	}

	/* add contents of buffer to expr */

	if (!storedVals -> next) {
	    new_part = last_sig_el;
	    if (new_part -> val == storedVals -> value) {
		newlen -= new_part -> len;
	    }
	    else {
		NEW (new_part, 1, SIGNALELEMENT);
		last_sig_el -> sibling = new_part;
		new_part -> child = NULL;
		new_part -> sibling = NULL;
		new_part -> val = storedVals -> value;
	    }
	    new_part -> len = -1;
	    ++newlen;
	}
	else {
	    NEW (new_part, 1, SIGNALELEMENT);
	    last_sig_el -> sibling = new_part;
	    new_part -> sibling = NULL;
	    new_part -> len = -storedTD;
	    newlen += storedTD;

	    NEW (new_part -> child, 1, SIGNALELEMENT);
	    last_sig_el = new_part -> child;
	    last_sig_el -> child = NULL;
	    last_sig_el -> len = storedTD;

	    sval = storedVals;
	    t1 = t;
	    do {
		if (sval -> next)
		    t2 = t + sval -> next -> time;
		else
		    t2 = t + storedTD;
		NEW (new_sig_el, 1, SIGNALELEMENT);
		new_sig_el -> child = NULL;
		new_sig_el -> sibling = NULL;
		new_sig_el -> val = sval -> value;
		new_sig_el -> len = t2 - t1;

		last_sig_el -> sibling = new_sig_el;
		last_sig_el = new_sig_el;
		t1 = t2;
	    } while ((sval = sval -> next));

	    new_part -> child -> val = new_part -> val = last_sig_el -> val;
	}

	sig -> expr -> val = new_part -> val;
	sig -> expr -> len = -newlen;

        /* update sig values */

	delSigval (sig -> begin_value, sig -> end_value);
	sig -> begin_value = NULL;
	sig -> end_value = NULL;
	curr_time = 0;
	simperiod = SimEndtime;
	(void) addSgnPart (sig, sig -> expr, 1);
	adjustSgnPart (sig);
	return (1);
    }

    rv = 0;
    help = NULL;

    for (i = 0; i < factor; ++i) {
	sval = storedVals;
	t1 = t + i * storedTD;
	do {
	    if (sval -> next)
		t2 = t + sval -> next -> time + i * storedTD;
	    else
		t2 = t + (i + 1) * storedTD;
	    rv |= changeSignal (sig, t1, t2, sval -> value, &help);
	    t1 = t2;
	} while ((sval = sval -> next));
    }

    return (rv);
}

int writeSet (int SaveAs)
{
    char help[MAXNAME+60];
    char outbuf[MAXNAME+80];
    char tmpfilename[MAXNAME];
    FILE *fp1 = NULL;
    FILE *fp2 = NULL;
    char *w;
    int c, cnt, len;
    char state, prevstate;
    simtime_t duration;
    struct signal *sig;
    struct sig_value *sval;
    int sigunitPre, sigunitNow, sigunitDone;
    int simperiodPre, simperiodNow, simperiodDone;
    int backslash, copycommand, first;
    long firstcharpos;
    SIGNALELEMENT *sigel;

    w = SaveAs ? inputname : Commandfile;
    if (!*w) {
        sprintf (help, "No commandfile specified!");
	goto ret;
    }

    if (!*Commandfile) goto skip;

    fp1 = fopen (Commandfile, "r");
    if (!fp1) {
	sprintf (help, "Cannot read \"%s\" again! (No save)", Commandfile);
	goto ret;
    }

skip:
    sprintf (tmpfilename, "w%d.cxx", (int)getpid ());
    if (!(fp2 = fopen (tmpfilename, "w"))) {
	sprintf (help, "Cannot open \"%s\"! (No save)", tmpfilename);
	goto ret;
    }

    /* first copy fp1 to fp2 but
       without 'editable' set commands
       and options sigunit and simperiod changed to their correct value */

    sigunitDone = 0;
    simperiodDone = 0;

    if (fp1) {
	for (;;) {

	    /* read command by command */

	    firstcharpos = ftell (fp1);

	    if ((c = getc (fp1)) == EOF) break;
	    while (c == ' ' || c == '\t') c = getc (fp1);

	    if (c == 's' && getc (fp1) == 'e' && getc (fp1) == 't'
		&& ((c = getc (fp1)) == ' ' || c == '\t' || c == '\\')) {

		copycommand = 0;

		while ((c = getc (fp1)) != '=') {
		    if (c == 'n') {
		        if ((c = getc (fp1)) == 'o'
		            && (c = getc (fp1)) == '_'
		            && (c = getc (fp1)) == 'e'
		            && (c = getc (fp1)) == 'd'
		            && (c = getc (fp1)) == 'i'
		            && (c = getc (fp1)) == 't') {

			    /* It is a set command with 'no_edit',
			       copy only if (1) the command is disabled,
			       or (2) the signal still exists in the program
			       as 'non-editable'
			    */

			    while ((c = getc (fp1)) == ' ' || c == '\t');
			    if (c == '*' && (c = getc (fp1)) == '#') {
				copycommand = 1;            /* case (1) */
			    }
			    else {
				fscanf (fp1, "%s", help);
				sig = existSignal (help);
				if (sig && sig -> no_edit)
				    copycommand = 1;         /* case (2) */
			    }

			    break;
			}
			else {
			    ungetc (c, fp1);
			}
		    }
		}
	    }
	    else {
		copycommand = 1;    /* it is no set command */
	    }

	    fseek (fp1, firstcharpos, 0);

	    sigunitPre = 0; sigunitNow = 0;
	    simperiodPre = 0; simperiodNow = 0;

	    backslash = 0;
	    for (;;) {

		w = getword (fp1);

		if (sigunitNow && *w != '\\') {
		    fprintf (fp2, "%le", Timescaling);
		    sigunitNow = 0;
		    sigunitDone = 1;
		}
		else if (simperiodNow && *w != '\\') {
		    fprintf (fp2, "%lld", SimEndtime);
		    simperiodNow = 0;
		    simperiodDone = 1;
		}
		else if (copycommand && *w != EOF) {
		    fprintf (fp2, "%s", w);
		}

		if (*w != '\\' && *w != ' ' && *w != '\t') {
		    if (sigunitPre) {
			if (*w == '=') sigunitNow = 1;
			sigunitPre = 0;
		    }
		    else if (simperiodPre) {
			if (*w == '=') simperiodNow = 1;
			simperiodPre = 0;
		    }
		    else if (compareWord (w, "sigunit"))
			sigunitPre = 1;
		    else if (compareWord (w, "simperiod"))
			simperiodPre = 1;
		}

		if ((*w == '\n' && !backslash) || *w == EOF) break;

		backslash = (*w == '\\') ? 1 : 0;
	    }
	}
	fclose (fp1);
    }

    if (!sigunitDone)   fprintf (fp2, "option sigunit = %le\n", Timescaling);
    if (!simperiodDone) fprintf (fp2, "option simperiod = %lld\n", SimEndtime);

    /* then write new set commands to fp2 */

    prevstate = 0;
    for (sig = Begin_signal; sig; sig = sig -> next) {

        if (sig -> no_edit) continue;

        sprintf (outbuf, "set %s =", sig -> name);
        cnt = strlen (outbuf);
        fprintf (fp2, "%s", outbuf);

        if (sig -> endless) {

	    if (!(sigel = sig -> expr)) {
		sprintf (help, "Endless signal \"%s\", no expr!", sig -> name);
		goto ret;
	    }

	    sigel = sigel -> sibling;
	    while (sigel && sigel -> sibling) {
		state = eval2cmdval (sigel -> val);
		sprintf (outbuf, " %c*%lld", state, sigel -> len);

		len = strlen (outbuf);
		if (cnt + len > 80 - 2) {
		    fprintf (fp2, " \\\n    ");
		    cnt = 5;
		}
		cnt = cnt + len;
		fprintf (fp2, "%s", outbuf);

		sigel = sigel -> sibling;
	    }
	    if (sigel && sigel -> child && sigel -> child -> sibling) {
		sigel = sigel -> child -> sibling;

		if (cnt + 1 > 80 - 2) {
		    fprintf (fp2, " \\\n    ");
		    cnt = 5;
		}
		first = 1;

		for (; sigel; sigel = sigel -> sibling) {
		    state = eval2cmdval (sigel -> val);
		    if (first) {
			first = 0;
			sprintf (outbuf, " (%c*%lld", state, sigel -> len);
		    }
		    else
			sprintf (outbuf, " %c*%lld", state, sigel -> len);

		    len = strlen (outbuf);
		    if (cnt + len > 80 - 2) {
			fprintf (fp2, " \\\n    ");
			cnt = 5;
		    }
		    cnt = cnt + len;
		    fprintf (fp2, "%s", outbuf);
		}

		if (cnt + 3 > 80) {
		    fprintf (fp2, " \\\n     ");
		    cnt = 5;
		}
		fprintf (fp2, ")*~");
	    }
	    fprintf (fp2, "\n");
	    continue;
	}

	if (!(sval = sig -> begin_value)) {
	    sprintf (help, "Signal \"%s\", no begin_value!", sig -> name);
	    goto ret;
	}

	do {
	    state = eval2cmdval (sval -> value);

	    if (sval -> next) {
		duration = sval -> next -> time - sval -> time;
		if (duration) {
		    if (duration == 1) {
			sprintf (outbuf, " %c", state);
			len = 2;
		    }
		    else {
			sprintf (outbuf, " %c*%lld", state, duration);
			len = strlen (outbuf);
		    }
		    if ((cnt += len) > 80 - 2) {
			fprintf (fp2, " \\\n    %s", outbuf);
			cnt = len + 4;
		    }
		    else
			fprintf (fp2, "%s", outbuf);
		}
		prevstate = state;
	    }
	    else {
		if (state != prevstate)
		    fprintf (fp2, " %c\n", state);
		else
		    fprintf (fp2, "\n");
	    }

	} while ((sval = sval -> next));
    }

    fclose (fp2);

    /* finally move tmpfile to file */

    if (SaveAs) strcpy (Commandfile, inputname);
    sprintf (help, "mv %s %s", tmpfilename, Commandfile);
    system (help);

    set_filename (Commandfile);

    sprintf (help, "\"%s\" ready", Commandfile);
    windowMessage (help, -1);
    somethingChanged = 0;
    return (1);
ret:
    windowMessage (help, -1);
    if (fp1) fclose (fp1);
    if (fp2) unlink (tmpfilename);
    return (0);
}

#define MAXWORD 511

char *getword (FILE *fp)
{
    static char buf[MAXWORD+1];
    int c, i;

    /* a word is
       1. the character '\n' or EOF.
       2. the character '/', '#', '*', '=' or '\\'.
       3. the string "/ *", "* /" (space = nil), "#*" or "*#".
       4. an identifier string containing none of the
	  characters of point 1, 2 or 3 above.
       5. a string of blanks (' ' or '\t'), possibly preceded by
	  one of the characters of point 2 above, or by
	  one of the strings of point 3 above, or by
	  an identifier string of point 4 above.
    */

    c = getc (fp);
    if (c == '\n' || c == EOF) {
	buf[0] = c;
	buf[1] = '\0';
	return (buf);
    }

    i = 0;
    if (!isspace (c)) {
	buf[i++] = c;
	if (c == '/' || c == '#') {
	    c = getc (fp);
	    if (c == '*') {
		buf[i++] = c;
		c = getc (fp);
	    }
	}
	else if (c == '*') {
	    c = getc (fp);
	    if (c == '/' || c == '#') {
		buf[i++] = c;
		c = getc (fp);
	    }
	}
	else if (c == '=' || c == '\\') {
	    c = getc (fp);
	}
	else {
	    c = getc (fp);
	    while (!isspace (c)
		   && c != '/' && c != '*' && c != '\\' && c != '='
		   && c != '#' && c != EOF && i < MAXWORD) {
		buf[i++] = c;
		c = getc (fp);
	    }
	}
    }

    while (c != '\n' && isspace (c) && i < MAXWORD) {
	buf[i++] = c;
	c = getc (fp);
    }
    if (c != EOF) ungetc (c, fp);

    buf[i] = '\0';
    return (buf);
}

int compareWord (char *w1, char *w2)
{
    int i = 0;

    while (w1[i] == w2[i] && w2[i] != '\0') i++;

    if (w2[i] == '\0' && (w1[i] == '\0' || w1[i] == ' ' || w1[i] == '\t')) {
	return (1); /* match */
    }
    return (0); /* no match */
}

char eval2cmdval (int v)
{
    switch (v) {
	case L_state: return ('l');
	case X_state: return ('x');
	case H_state: return ('h');
	case F_state: return ('f');
    }
    return ('u');
}
