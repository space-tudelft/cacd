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
#include "src/libddm/dmincl.h"

#include "src/simeye/define.h"
#include "src/simeye/type.h"
#include "src/simeye/extern.h"

static int  addStringValue (char *str);
static int  readSpiceEol (FILE *fp);
static int  readSpiceTop (FILE *fp);
static void resetStringValues (void);

extern DM_PROJECT *projKey;
extern int old_spice_mode;

static struct signal **Array_signals = NULL;
static int Array_nr = 0;

#define SV_SIZE 50
#define SV_MAXL 10

char **SV = NULL;
static int *svi = NULL;
static int SV_cnt = 0;
static int SV_size = 0;

int readLogic (int intermediate) /* read "res" format or sls output file */
{
    char help[MAXNAME+60];
    static char filename[MAXNAME];
    int i;
    simtime_t time = -1;
    char c;
    static int Old_nr_signals;
    static FILE *fp = NULL;
    static RES_FILE *ResF;
    static long fp_mem_pos;
    STRING_REF * str_ref;
    RES_PATH * rs;
    struct signal *sig;
    static struct signal *orig_begin_signal;
    static struct signal *Old_end_signal;
    struct sig_value *sval;
    int value;
    static double Timeconvert;

    *help = 0;

    if (!intermediate || intermediate == BEG_INTER) {

	strcpy (filename, outputname);

	if (!fp && !(fp = fopen (filename, "r"))) {
	    if (!intermediate) {
		sprintf (help, "Cannot open file \"%s\"", filename);
	    }
	    goto err;
	}

	if (!(ResF = read_paths (fp, filename))) {
	    if (!intermediate || intermediate == END_INTER) {
		sprintf (help, "Empty file \"%s\"", filename);
		goto err;
	    }
	    rewind (fp);
	    return (0);
	}

	if (!Append) {
	    delSigList (Begin_signal);
	    End_signal = Begin_signal = NULL;
	    Nr_signals = 0;
	    resetStringValues ();
	}

	Old_end_signal = End_signal;
	Old_nr_signals = Nr_signals;

	for (rs = ResF -> signals; rs; rs = rs -> next) {
	    str_ref = names_from_path (0, rs -> path);
	    while (str_ref) {
		NEW (sig, 1, struct signal);
		sig -> name = str_ref -> str;
		sig -> prev = End_signal;
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
		sig -> expr = NULL;
		sig -> no_edit = 0;
		sig -> endless = 0;
		sig -> stringValue = 0;
		if (End_signal) {
		    End_signal -> next = sig;
		    End_signal -> orig_next = sig;
		}
		else
		    Begin_signal = sig;
		End_signal = sig;
		Nr_signals++;
		str_ref = str_ref -> next;
	    }
	}
	if (End_signal) {
	    End_signal -> next = NULL;
	    End_signal -> orig_next = NULL;
	}

	if (Append) {

	    /* maintain old value of Timescaling for Timescaling */

	    Timeconvert = ResF -> scaling / Timescaling;
	}
	else {
	    Timescaling = ResF -> scaling;
	    orig_begin_signal = Begin_signal;
	}
	Voltscaling = 2.5;

	rewind (fp);
	while (getc (fp) != '\n');    /* search begin of signal values */

	fp_mem_pos = ftell (fp);
    }
    else {
	fseek (fp, fp_mem_pos, 0);
    }

    /* read signal values */

    while (fscanf (fp, "%lld", &time) == 1) {

	if (Append) {

	    if (time * Timeconvert * 10 > MAXTIME) {
		sprintf (help, "New time values too large in file \"%s\"", filename);
		windowMessage (help, -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		End_signal = Old_end_signal;
		Nr_signals = Old_nr_signals;

		if (!intermediate || intermediate == END_INTER) goto err;
		return (0);
	    }

	    time *= Timeconvert;

	    sig = Old_end_signal -> next;
	}
	else {
	    sig = orig_begin_signal;
	}

	while (sig) {

	    if ((c = getc (fp)) == EOF || c == '\n') goto res_halt;

		 if (c == 'h') value = H_state;
	    else if (c == 'l') value = L_state;
	    else if (c == 'x') value = X_state;
	    else if (c == 'f') value = F_state;
	    else if (c == '(') {
		i = 0;
		while ((c = getc (fp)) != ')' && c != '\n' && c != EOF) {
		    help[i] = c;
		    if (++i == SV_MAXL) break;
		}
		if (c == EOF) goto res_halt;
		if (i == SV_MAXL) {
		    while ((c = getc (fp)) != ')' && c != '\n' && c != EOF);
		    if (c == EOF) goto res_halt;
		}
		help[i] = '\0';
		value = addStringValue (help);
		sig -> stringValue = 1;
	    }
	    else {
		fprintf (stderr, "He? Illegal value '%c'\n", c);
		value = X_state;
	    }

	    if (!sig -> end_value || sig -> end_value -> value != value) {
		NEWSVAL (sval);
		sval -> time = time;
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

	    sig = sig -> orig_next;
	}

	fp_mem_pos = ftell (fp);
	/* remember positions just before new time value */
    }

res_halt:

    /* make all new signals end at the time that is Endtime
       for the current input file */

    if (Append) {
	sig = Old_end_signal -> next;
    }
    else {
	sig = Begin_signal;
    }
    while (sig) {
	if (sig -> end_value && sig -> end_value -> time < time) {
	    NEWSVAL (sval);
	    sval -> time = time;
	    sval -> value = sig -> end_value -> value;
	    sval -> next = NULL;
	    sval -> prev = sig -> end_value;
	    sig -> end_value -> next = sval;
	    sig -> end_value = sval;
	}
#ifdef DEBUG
	printSgnPart (sig, 0);
#endif
	sig = sig -> next;
    }

    if (time >= 0) {
        if (Append)
            Endtime = Max (Endtime, time);
        else {
            Begintime = 0;
            Endtime = time;
        }
    }
    else if (!intermediate || intermediate == BEG_INTER) {
        if (!Append) {
            Begintime = 0;
            Endtime = 0;
        }
    }

    Global_umin = 0;
    Global_umax = 2;

    if (!intermediate || intermediate == END_INTER) {
	fclose (fp);
	fp = NULL;
    }
    if (!intermediate || intermediate == BEG_INTER) set_filename (filename);
    return (1);
err:
    if (fp) { fclose (fp); fp = NULL; }
    if (*help) windowMessage (help, -1);
    return (0);
}

int readWaves (int intermediate) /* read "plt" format or sls output file */
{
    char help[MAXNAME+60];
    static char filename[MAXNAME];
    int i;
    simtime_t c_t, time;
    char c;
    static FILE *fp = NULL;
    static long fp_mem_pos;
    struct signal *sig;
    struct sig_value *sval, *endv;
    char buf[512];
    int nr;
    int h;
    static int Old_nr_signals;
    static struct signal *Old_end_signal;
    double newTimescaling;
    double newVoltscaling;
    double newGlobal_umin;
    double newGlobal_umax;
    static double Timeconvert;
    static double Voltconvert;

    *help = 0;

    if (!intermediate || intermediate == BEG_INTER) {

	strcpy (filename, outputname);

	if (!fp && !(fp = fopen (filename, "r"))) {
	    if (!intermediate) {
		sprintf (help, "Cannot open file \"%s\"", filename);
	    }
	    goto err;
	}

	if (fscanf (fp, "%d", &nr) <= 0) {
	    if (!intermediate) {
		sprintf (help, "Empty file \"%s\"", filename);
		goto err;
	    }
	    rewind (fp);
	    return (0);
	}

	/* make a direct accessible datastructure for the signals */

	if (Array_nr < nr) {
	    if (Array_nr) DELETE (Array_signals);
	    while ((Array_nr += 100) < nr) ;
	    NEW (Array_signals, Array_nr, struct signal *);
	}

	if (!Append) {
	    delSigList (Begin_signal);
	    End_signal = Begin_signal = NULL;
	    Nr_signals = 0;
	}

	Old_end_signal = End_signal;
	Old_nr_signals = Nr_signals;

	for (i = 0; i < nr; i++) {

	    if (fscanf (fp, "%s", buf) != 1) goto hnot4;

	    NEW (sig, 1, struct signal);
	    NEW (sig -> name, strlen (buf) + 1, char);
	    strcpy (sig -> name, buf);
	    sig -> prev = End_signal;
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
	    sig -> expr = NULL;
	    sig -> no_edit = 0;
	    sig -> endless = 0;
	    sig -> stringValue = 0;
	    if (End_signal)
		End_signal -> next = sig;
	    else
		Begin_signal = sig;
	    End_signal = sig;
	    Array_signals[i] = sig;
	}
	if (nr > 0) {
	    End_signal -> next = NULL;
	    Nr_signals += nr;
	}

	h = 0;
	h += fscanf (fp, "%lf", &newTimescaling);
	h += fscanf (fp, "%lf", &newVoltscaling);
	h += fscanf (fp, "%le", &newGlobal_umin);
	h += fscanf (fp, "%le", &newGlobal_umax);

	if (h != 4) {
hnot4:
	    if (Append) {
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		End_signal = Old_end_signal;
		Nr_signals = Old_nr_signals;
	    }
	    else {
		delSigList (Begin_signal);
		End_signal = Begin_signal = NULL;
		Nr_signals = 0;
	    }
	    if (!intermediate) {
		sprintf (help, "Incomplete file \"%s\"", filename);
		goto err;
	    }
	    rewind (fp);
	    return (0);
	}

	if (Append) {
	    Timeconvert = newTimescaling / Timescaling;
	    Voltconvert = newVoltscaling / Voltscaling;

	    if (newGlobal_umin < Global_umin) Global_umin = newGlobal_umin;
	    if (newGlobal_umax > Global_umax) Global_umax = newGlobal_umax;

	    if (Voltconvert > 1e4) {
		sprintf (help, "New volt scale factor too large in file \"%s\"", filename);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		End_signal = Old_end_signal;
		Nr_signals = Old_nr_signals;
		goto err;
	    }
	}
	else {
	    Timescaling = newTimescaling;
	    Voltscaling = newVoltscaling;

	    Global_umin = newGlobal_umin;
	    Global_umax = newGlobal_umax;
	}

	if (Global_umax <= Global_umin)
	    Global_umax = Global_umin + 1.0 / Voltscaling;

	fp_mem_pos = ftell (fp);
    }
    else {
	fseek (fp, fp_mem_pos, 0);
    }

    c_t = -1;
    while (fscanf (fp, "%lld", &time) == 1) {

	if (Append) {
	    time *= Timeconvert;
	    if (time * 10 > MAXTIME) {
		sprintf (help, "New time values too large in file \"%s\"", filename);
		goto err;
	    }
	}
	c_t = time;

        while (fgetc (fp) != '\n') {

	    if (fscanf (fp, "%d%c%d", &i, &c, &h) != 3) {
		if (!intermediate || intermediate == -1)
		    fprintf (stderr, "He? Read error\n");
		goto plt_halt;
	    }

	    sig = Array_signals[i - 1];

            if (Append) h *= Voltconvert;

	    endv = c == 'l' ? sig -> end_value_L : sig -> end_value_U;

	    if (endv && endv -> value == h && (endv -> time == c_t ||
		(endv -> prev && endv -> prev -> value == h))) {

		/* Save memory space (and execution time) by
		   not storing intermediate values that are
		   equal to a previous value.
                   This also takes into account the fact that for
                   some time points some values have already been
                   read during a previous call to readWaves().
		*/
		endv -> time = c_t;
	    }
	    else {
		NEWSVAL (sval);
		sval -> time = c_t;
		sval -> value = h;
		sval -> next = NULL;
		sval -> prev = endv;

		if (c == 'l') {
		    if (h < Global_umin) Global_umin = h;
		    if (endv)
			endv -> next = sval;
		    else
			sig -> begin_value_L = sval;
		    sig -> end_value_L = sval;
		}
		else {
		    if (c != 'u')
			fprintf (stderr, "He? Illegal value '%c'\n", c);

		    if (h > Global_umax) Global_umax = h;
		    if (endv)
			endv -> next = sval;
		    else
			sig -> begin_value_U = sval;
		    sig -> end_value_U = sval;
		}
	    }
        }

	fp_mem_pos = ftell (fp);
	/* remember positions just before new time value */
    }

plt_halt:

#ifdef DEBUG
    sig = Old_end_signal ? Old_end_signal -> next : Begin_signal;
    while (sig) {
	printSgnPart (sig, 'l');
	printSgnPart (sig, 'u');
	sig = sig -> next;
    }
#endif

    if (c_t >= 0) {
	if (Append)
	    Endtime = Max (Endtime, c_t);
	else {
	    Begintime = 0;
	    Endtime = c_t;
	}
    }
    else if (!intermediate || intermediate == BEG_INTER) {
	if (!Append) {
	    Begintime = 0;
	    Endtime = 0;
	}
    }

    if (!intermediate || intermediate == END_INTER) {
	fclose (fp);
	fp = NULL;
    }
    if (!intermediate || intermediate == BEG_INTER) set_filename (filename);
    return (1);
err:
    if (fp) { fclose (fp); fp = NULL; }
    if (*help) windowMessage (help, -1);
    return (0);
}

int spice3;
int hspice;

int readSpice (int intermediate) /* read "ana" format or spice output file */
{
    char help[MAXNAME+60];
    char filename[MAXNAME];
    FILE *fp = NULL;
    struct signal *sig;
    struct sig_value *sval, *endv;
    char buf[512];
    double t, mint, maxt;
    double v, minv, maxv;
    double newVoltscaling;
    struct signal *Old_end_signal = 0;
    int c, found, i, Old_nr_signals = 0;
    long c_v;
    simtime_t c_b, c_t = 0;

    if (intermediate) {
	sprintf (help, "readSpice: intermediate mode???");
	goto err;
    }
    *help = 0;

    strcpy (filename, outputname);

    if (!(fp = fopen (filename, "r"))) {
	sprintf (help, "Cannot open file \"%s\"", filename);
	goto err;
    }

    sprintf (buf, ".... reading %s", filename);
    windowMessage (buf, -2);

    mint = 1e99;
    maxt = 0;
    minv = 1e99;   /* MAXREAL */
    maxv = -1e99;  /* MINREAL */

    /* First pass to find minimum and maximum time
       and minimum and maximum voltage */

    spice3 = -1;
    hspice = 0;

    found = 0;
    while (readSpiceTop (fp)) {

	while ((c = getc (fp)) != '\n');

	if (spice3) {
	    while (getc (fp) != '\n'); /* skip '------' */
	    fscanf (fp, "%*d"); /* skip index */
	}

	while (fscanf (fp, "%le", &t) > 0) {
	    if (hspice) {
		c = getc (fp);
		switch (c) {
		  case 'f': t *= 1e-15; break;
		  case 'p': t *= 1e-12; break;
		  case 'n': t *= 1e-09; break;
		  case 'u': t *= 1e-06; break;
		  case 'm': t *= 1e-03; break;
		  case 'k': t *= 1e+03; break;
		}
	    }

	    if (t > maxt) maxt = t;
	    if (t < mint) mint = t;

	    while ((c = getc (fp)) == ' ' || c == '\t');
	    while (c != '\n') {
		ungetc (c, fp);

		if (fscanf (fp, "%le", &v) < 0) goto read_err;
		if (hspice) {
		    c = getc (fp);
		    switch (c) {
		      case 'f': v *= 1e-15; break;
		      case 'p': v *= 1e-12; break;
		      case 'n': v *= 1e-09; break;
		      case 'u': v *= 1e-06; break;
		      case 'm': v *= 1e-03; break;
		      case 'k': v *= 1e+03; break;
		    }
		}

		if (v > maxv) maxv = v;
		if (v < minv) minv = v;

		while ((c = getc (fp)) == ' ' || c == '\t');
	    }

	    if (readSpiceEol (fp)) break;
	}
	found = 1;
    }
    if (!found) {
        sprintf (help, "Cannot find transient analysis table in \"%s\"", filename);
        goto err;
    }
    if (maxt <= 0) {
	sprintf (help, "End of simulation time <= 0 in \"%s\"", filename);
	goto err;
    }

    newVoltscaling = convdec (Max (Abs (minv), Abs (maxv)) / (MAXVOLT / 100));
    if (newVoltscaling == 0) newVoltscaling = 1;

    if (Append) {
	if (maxt / Timescaling * 10 > MAXTIME) {
	    sprintf (help, "New time values too large in file \"%s\"", filename);
	    goto err;
	}
        if (Voltscaling == 0) {
            strcpy (help, "Old volt scale factor == 0");
            goto err;
        }
        if (newVoltscaling / Voltscaling > 1e4) {
	    sprintf (help, "New volt scale factor too large in file \"%s\"", filename);
	    goto err;
	}
	if ((v = minv / Voltscaling) < Global_umin) Global_umin = v;
	if ((v = maxv / Voltscaling) > Global_umax) Global_umax = v;
    }
    else {
        Timescaling = convdec (maxt / (MAXTIME / 100));
        Voltscaling = newVoltscaling;
        Global_umin = minv / Voltscaling;
        Global_umax = maxv / Voltscaling;
    }

    rewind (fp);

    /* Second pass to read the signals */

    spice3 = -1;
    hspice = 0;

    found = 0;
    while (readSpiceTop (fp)) {

        if (!Append && !found) {
	    delSigList (Begin_signal);
	    End_signal = Begin_signal = NULL;
	    Nr_signals = 0;
        }

	Old_end_signal = End_signal;
	Old_nr_signals = Nr_signals;

        while ((c = getc (fp)) == ' ' || c == '\t');

        while (c != '\n') {
            ungetc (c, fp);

            fscanf (fp, "%s", buf);

	    NEW (sig, 1, struct signal);
	    NEW (sig -> name, strlen (buf) + 1, char);
	    strcpy (sig -> name, buf);
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
	    sig -> expr = NULL;
	    sig -> no_edit = 0;
	    sig -> endless = 0;
	    sig -> stringValue = 0;
	    if (End_signal) {
	        End_signal -> next = sig;
	        sig -> prev = End_signal;
	        End_signal = sig;
	    }
	    else {
	        Begin_signal = End_signal = sig;
	    }
	    Nr_signals++;

	    while ((c = getc (fp)) == ' ' || c == '\t');
        }

	if (spice3) {
	    while (getc (fp) != '\n'); /* skip '------' */
	    fscanf (fp, "%*d"); /* skip index */
	}

        while (fscanf (fp, "%le", &t) > 0) {
	    if (hspice) {
		c = getc (fp);
		switch (c) {
		  case 'f': t *= 1e-15; break;
		  case 'p': t *= 1e-12; break;
		  case 'n': t *= 1e-09; break;
		  case 'u': t *= 1e-06; break;
		  case 'm': t *= 1e-03; break;
		  case 'k': t *= 1e+03; break;
		}
	    }

	    c_t = (simtime_t) (t / Timescaling);

	    if (Append || found) {
	        sig = Old_end_signal -> next;
	    }
	    else {
	        sig = Begin_signal;
	    }
	    for (i = Old_nr_signals; i < Nr_signals; i++) {

                if (fscanf (fp, "%le", &v) < 0) goto read_err;
		if (hspice) {
		    c = getc (fp);
		    switch (c) {
		      case 'f': v *= 1e-15; break;
		      case 'p': v *= 1e-12; break;
		      case 'n': v *= 1e-09; break;
		      case 'u': v *= 1e-06; break;
		      case 'm': v *= 1e-03; break;
		      case 'k': v *= 1e+03; break;
		    }
		}

		c_v = (int) (v / Voltscaling);

		endv = sig -> end_value_U;

                if (endv && endv -> prev &&
		    ((endv -> value == c_v && endv -> prev -> value == c_v) ||
		     (endv -> time  == c_t && endv -> prev -> time  == c_t))) {

                    /* save memory space (and execution time) by
                       not storing intermediate values that are
                       equal to a previous and next value.
                    */

                    endv -> time = c_t;
		    endv -> value = c_v;
                }
                else {
		    NEWSVAL (sval);
		    sval -> time = c_t;
		    sval -> value = c_v;
		    sval -> next = NULL;
		    sval -> prev = endv;

		    if (endv)
		        endv -> next = sval;
		    else
		        sig -> begin_value_U = sval;
		    sig -> end_value_U = sval;
                }

	        sig = sig -> next;
	    }

	    while ((c = getc (fp)) != '\n' && c != EOF);
	    if (readSpiceEol (fp)) break;
        }

	c_b = (simtime_t) (mint / Timescaling);
	if (Append || found) {
	    Begintime = Min (Begintime, c_b);
	    Endtime = Max (Endtime, c_t);
	}
	else {
	    Begintime = c_b;
	    Endtime = c_t;
	}

        found = 1;
    }
    if (!found) goto read_err;

#ifdef DEBUG
    sig = Old_end_signal ? Old_end_signal -> next : Begin_signal;
    while (sig) {
	printSgnPart (sig, 'u');
	sig = sig -> next;
    }
#endif

    fclose (fp);
    set_filename (filename);
    return (1);

read_err:
    sprintf (help, "Error in reading transient analysis of \"%s\"", filename);
err:
    if (Old_end_signal) {
	delSigList (Old_end_signal -> next);
	Old_end_signal -> next = NULL;
	End_signal = Old_end_signal;
	Nr_signals = Old_nr_signals;
    }
    if (*help) windowMessage (help, -1);
    if (fp) fclose (fp);
    return (0);
}

static int readSpiceTop (FILE *fp)
/* return value:  1 matched */
/*                0 not matched */
{
    char str1[512];
    register int c;

    if (Spice == 2) { /* Pstar */
	if (spice3 == 0) {
	    if (fscanf (fp, "%s", str1) != 1) return (0);
	    return (*str1 == 'T');
	}
	spice3 = 0;
	while (fscanf (fp, "%s", str1) == 1) {
	    while (strsame (str1, "TR")) {
		if (fscanf (fp, "%s", str1) != 1) return (0);
		if (!strsame (str1, "Analysis.")) continue;
		while ((c = getc (fp)) != '\n') if (c == EOF) return (0);
		while ((c = getc (fp)) != '\n') if (c == EOF) return (0);
		if (fscanf (fp, "%s", str1) != 1) return (0);
		if (*str1 == 'T') return (1);
	    }
	}
	return (0);
    }

    if (spice3 == -1) {
        if (fscanf (fp, "%s", str1) != 1) return (0);
	spice3 = strsame (str1, "Circuit:");
    }

    if (spice3) { /* it is a spice3 output file */

	while (fscanf (fp, "%s", str1) == 1) {
	    while (strsame (str1, "transient") || strsame (str1, "Transient")) {

		while ((c = getc (fp)) != '\n') if (c == EOF) return (0);
		if ((c = getc (fp)) == '-') {
		    while ((c = getc (fp)) != '\n') if (c == EOF) return (0);
		}
		else
		    ungetc (c, fp);

		if (fscanf (fp, "%s", str1) != 1) return (0);

		if (strsame (str1, "Index")) {
		    /* skip string 'time' */
		    if (fscanf (fp, "%s", str1) != 1) return (0);
		    return (1);
		}
	    }
	}
	return (0);
    }

    /* it is a spice2 output file */
again:
    while (fscanf (fp, "%s", str1) == 1) {
	while (strsame (str1, "TRANSIENT") || strsame (str1, "transient")) {

	    if (fscanf (fp, "%s", str1) != 1) return (0);
	    if (!strsame (str1, "ANALYSIS") && !strsame (str1, "analysis")) continue;

	    if (fscanf (fp, "%s", str1) != 1) return (0);
	    if (!strsame (str1, "TEMPERATURE") &&
		!strsame (str1, "temperature") &&
		!strsame (str1, "tnom=") &&
		!strsame (str1, "temp="))
		goto again; /* it is an error message or something */

	    /* read till end of line */
	    while ((c = getc (fp)) != EOF && c != '\n');
	    if ((c = getc (fp)) == 'x') {
		while ((c = getc (fp)) != EOF && c != '\n');
		hspice = 1;
	    }
	    else ungetc (c, fp);

	    while (fscanf (fp, "%s", str1) == 1) {
		if (strsame (str1, "LEGEND:") || strsame (str1, "legend:")) goto again; /* it is plot_tran */
		if (strsame (str1, "TIME") || strsame (str1, "time")) {
		    if (hspice) while ((c = getc (fp)) != EOF && c != '\n');
		    return (1);
		}
	    }
	    return (0);
	}
    }
    return (0);
}

static int readSpiceEol (FILE *fp)
{
    register int c;

    if (!spice3) {
	while ((c = getc (fp)) == ' ' || c == '\t');
	ungetc (c, fp);
	if (isdigit (c)) return (0);
	return (1);
    }

    c = getc (fp);
    if (isdigit (c)) {
	do { c = getc (fp); } while (isdigit (c)); /* skip index */
	return (0);
    }
    while (isspace (c)) c = getc (fp);

    if (c == 'I' && getc (fp) == 'n' && getc (fp) == 'd' && getc (fp) == 'e' && getc (fp) == 'x') {
	while ((c = getc (fp)) != '\n' && c != EOF);
	while ((c = getc (fp)) != '\n' && c != EOF); /* skip '----' */
	c = getc (fp);
	if (isdigit (c)) {
	    do { c = getc (fp); } while (isdigit (c)); /* skip index */
	    return (0);
	}
    }
    return (1);
}

void delSig (struct signal *sig)
{
    delSigval (sig -> begin_value, sig -> end_value);
    delSigval (sig -> begin_value_L, sig -> end_value_L);
    delSigval (sig -> begin_value_U, sig -> end_value_U);
    delSigexpr (sig -> expr);
    DELETE (sig -> name);
    DELETE (sig);
}

void delSigList (struct signal *sig)
{
    struct signal *oldsig;

    while (sig) {
	oldsig = sig;
	sig = sig -> next;
	delSig (oldsig);
    }
}

struct sig_value *free_svals = NULL;

void delSigval (struct sig_value *sigv, struct sig_value *endv)
{
    if (sigv) {
	if (!endv) {
	    endv = sigv;
	    while (endv -> next) endv = endv -> next;
	}
	endv -> next = free_svals;
	free_svals = sigv;
    }
}

void delSigexpr (struct signalelement *sigel)
{
    struct signalelement *oldsigel;

    while (sigel) {
	if (sigel -> child) delSigexpr (sigel -> child);
	oldsigel = sigel;
	sigel = sigel -> sibling;
	DELETE (oldsigel);
    }
}

static int addStringValue (char *str)
{
    register int i, low, high, mid;

    low = -1; high = SV_cnt;  /* open boundaries */
    while (low + 1 < high) {
	mid = (low + high) / 2;
	i = strcmp (SV[svi[mid]], str);
	     if (i > 0) high = mid;
	else if (i < 0) low  = mid;
	else return (svi[mid]);
    }

    if (SV_cnt == SV_size) {
	if (SV_size == 0) {
	    SV_size = SV_SIZE;
	    NEW (SV, SV_size, char *);
	    NEW (svi, SV_size, int);
	}
	else {
	    SV_size += SV_SIZE;
	    ENLARGE (SV, SV_size, char *);
	    ENLARGE (svi, SV_size, int);
	}
    }

    /* insert in list while maintaining alphabetical order */

    ++low;
    for (i = SV_cnt; i > low; i--) svi[i] = svi[i - 1];

    NEW (SV[SV_cnt], strlen (str) + 1, char);
    strcpy (SV[SV_cnt], str);
    svi[low] = SV_cnt++;

    return (svi[low]);
}

static void resetStringValues ()
{
    while (SV_cnt) DELETE (SV[--SV_cnt]);
}
