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

static int  addStringValue (char *str);
STRING_REF *names_from_path (int traillen, PATH_SPEC *path); // res.c
RES_FILE   *read_paths (FILE *fp, char *fn); // res.c
static int  readSpiceEol (FILE *fp);
static int  readSpiceTop (FILE *fp);
static void resetStringValues (void);

extern int pstar;

static struct signal **Array_signals = NULL;
static struct signal * Old_end_signal;

#define SV_SIZE 50
#define SV_MAXL 10

char **SV = NULL;
static int *svi = NULL;
static int SV_cnt = 0;
static int SV_size = 0;
static int Array_nr = 0;

int readLogic (char *name, int intermediate)
/* intermediate = 0  : no intermediate read
**              = 2  : start intermediate read
**              = 1  : intermediate read
**              = -1 : end intermediate read
*/
{
    char help[132];
    int i;
    simtime_t time = -1;
    char c;
    static char *filename;
    static int Old_nr_signals;
    static FILE *fp = NULL;
    static RES_FILE *ResF;
    static long fp_mem_pos;
    STRING_REF * str_ref;
    RES_PATH * rs;
    struct signal *sig;
    static struct signal *orig_begin_signal;
    struct sig_value *sval;
    int value;
    static double Timeconvert;

    if (intermediate == 0 || intermediate == 2) {

	NEW (filename, strlen (name) + 5, char);
	sprintf (filename, "%s.res", name);

	if (!fp && !(fp = fopen (filename, "r"))) {
	    if (intermediate == 0) {
		sprintf (help, "Cannot open %s", filename);
		windowMessage (help, -1);
	    }
	    return (0);
	}

	if (!(ResF = read_paths (fp, filename))) {
	    if (intermediate == 0 || intermediate == -1) {
		sprintf (help, "Empty file %s", filename);
		windowMessage (help, -1);
		fclose (fp);
		fp = NULL;
	    }
	    else
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
		Nr_signals++;
		NEW (sig, 1, struct signal);
		sig -> name = str_ref -> str;
		sig -> prev = End_signal;
		sig -> layover = NULL;
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
		windowMessage ("New time values too large (as compared to old values)", -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		End_signal = Old_end_signal;
		Nr_signals = Old_nr_signals;
		if (intermediate == 0 || intermediate == -1) {
		    fclose (fp);
		    fp = NULL;
		}
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

		 if (c == 'h') value = 2;
	    else if (c == 'l') value = 0;
	    else if (c == 'x') value = 1;
	    else if (c == 'f') value = -1; /* free state, no draw */
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
		value = 1;
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
    else if (intermediate == 0 || intermediate == 2) {
	if (!Append) {
	    Begintime = 0;
	    Endtime = 0;
	}
    }

    Global_umin = 0;
    Global_umax = 2;

    if (intermediate == 0 || intermediate == -1) {
	fclose (fp);
	fp = NULL;
    }
    return (1);
}

int readWaves (char *name, int intermediate)
{
    char help[256];
    int i;
    simtime_t c_t, time;
    char c;
    static char *filename;
    static FILE *fp = NULL;
    static long fp_mem_pos;
    struct signal *sig;
    struct sig_value *sval, *endv;
    char buf[264];
    int nr;
    int h;
    static int Old_nr_signals;
    double newTimescaling;
    double newVoltscaling;
    double newGlobal_umin;
    double newGlobal_umax;
    static double Timeconvert;
    static double Voltconvert;

    if (!intermediate || intermediate == 2) {

	NEW (filename, strlen (name) + 5, char);
	sprintf (filename, "%s.plt", name);

	if (!fp && !(fp = fopen (filename, "r"))) {
	    if (!intermediate) {
		sprintf (help, "Cannot open %s", filename);
		windowMessage (help, -1);
	    }
	    return (0);
	}

	if (fscanf (fp, "%d", &nr) <= 0) {
	    if (!intermediate) {
		sprintf (help, "Empty file %s", filename);
		windowMessage (help, -1);
		fclose (fp);
		fp = NULL;
	    }
	    else
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
	    resetStringValues ();
	}

	Old_end_signal = End_signal;
	Old_nr_signals = Nr_signals;

	for (i = 0; i < nr; i++) {

	    if (fscanf (fp, "%s", buf) != 1) goto hnot4;

	    NEW (sig, 1, struct signal);
	    NEW (sig -> name, strlen (buf) + 1, char);
	    strcpy (sig -> name, buf);
	    sig -> prev = End_signal;
	    sig -> layover = NULL;
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
		windowMessage ("Incomplete input file", -1);
		fclose (fp);
		fp = NULL;
	    }
	    else
		rewind (fp);
	    return (0);
	}

	if (Append) {
	    Timeconvert = newTimescaling / Timescaling;
	    Voltconvert = newVoltscaling / Voltscaling;

	    if (newGlobal_umin < Global_umin) Global_umin = newGlobal_umin;
	    if (newGlobal_umax > Global_umax) Global_umax = newGlobal_umax;

	    if (Voltconvert > 1e4) {
		windowMessage ("New volt scale factor / old scale factor > 1e4", -1);
		delSigList (Old_end_signal -> next);
		Old_end_signal -> next = NULL;
		End_signal = Old_end_signal;
		Nr_signals = Old_nr_signals;
		fclose (fp);
		fp = NULL;
		return (0);
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
		windowMessage ("New time values too large (as compared to old values)", -1);
		goto plt_halt;
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

		/* save memory space (and execution time) by
		   not storing intermediate values that are
		   equal to a previous and next value.
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
	/* remember only positions just before time value */
    }

plt_halt:

    if (c_t >= 0) {
	if (Append)
	    Endtime = Max (Endtime, c_t);
	else {
	    Begintime = 0;
	    Endtime = c_t;
	}
    }
    else if (!intermediate || intermediate == 2) {
	if (!Append) {
	    Begintime = 0;
	    Endtime = 0;
	}
    }

    if (!intermediate || intermediate == -1) {
	fclose (fp);
	fp = NULL;
    }
    return (1);
}

struct signal *find_sig (int i)
{
    register struct signal * sig;

    sig = Old_end_signal ? Old_end_signal -> next : Begin_signal;
    do {
	if (sig -> i == i) break;
    } while ((sig = sig -> next));
    return (sig);
}

struct xyz {
    int   i;
    float t;
    float v;
};

int readPlato (char *name)
{
    char buf[512];
    int i;
    simtime_t c_t;
    int c_v;
    char *filename;
    FILE *fp;
    struct signal *sig, *newsig;
    struct sig_value *sval;
    double mint, maxt, minv, maxv;
    int Old_nr_signals;
    double newVoltscaling;
    struct xyz sample;
    char *s;

    mint = maxt = minv = maxv = 0; // init, to suppress compiler warning

    NEW (filename, strlen (name) + 7, char);
    sprintf (filename, "%s.sig.n", name);

    if (!(fp = fopen (filename, "r"))) {
	sprintf (buf, "Cannot open %s", filename);
	windowMessage (buf, -1);
	return (0);
    }

    if (!Append) {
	delSigList (Begin_signal);
	Begin_signal = End_signal = NULL;
	Nr_signals = 0;
	resetStringValues ();
    }
    Old_end_signal = End_signal;
    Old_nr_signals = Nr_signals;

    /* Read the index and the signal names */

    while (fscanf (fp, "%d%s", &i, buf) == 2) {

	s = buf + strlen (buf);

	/* select only the voltages: name has '.v' appended */
	if (*--s == 'v' && *--s == '.') {
	    *s = 0;
	    NEW (sig, 1, struct signal);
	    NEW (sig -> name, s - buf, char);
	    strcpy (sig -> name, buf+1);
	    sig -> i = i;
	    sig -> layover = NULL;
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
	    sig -> prev = End_signal;
	    End_signal = sig;
	    ++Nr_signals;
	}
    }

    fclose (fp);

    if (Nr_signals == Old_nr_signals) {
	sprintf (buf, "No signal names found in %s", filename);
	windowMessage (buf, -1);
	return (0);
    }

    End_signal -> next = NULL;

    /* open the sig file with the format
	(int index, float time_point, float signal_value)++ */

    sprintf (filename, "%s.sig", name);
    if (!(fp = fopen (filename, "r"))) {
	sprintf (buf, "Cannot open %s", filename);
	goto err;
    }

    sprintf (buf, ".... reading %s", filename);
    windowMessage (buf, -2);

    /* First pass to find minimum and maximum time
	and minimum and maximum voltage */

    sig = NULL;
    while (fread (&sample, sizeof(struct xyz), 1, fp) == 1) {
	if (!(sig = find_sig (sample.i))) continue;
	i = sample.i;
	mint = maxt = sample.t;
	minv = maxv = sample.v;
	break;
    }
    if (!sig) {
	sprintf (buf, "Cannot find voltages in analysis table in %s", filename);
	goto err;
    }
    if (mint < 0) {
	sprintf (buf, "Begin of simulation time < 0 in %s", filename);
	goto err;
    }

    while (fread (&sample, sizeof(struct xyz), 1, fp) == 1) {
	if (!find_sig (sample.i)) continue;
	if (sample.t > maxt) maxt = sample.t;
	if (sample.v > maxv) maxv = sample.v;
	else if (sample.v < minv) minv = sample.v;
    }
    if (maxt <= 0) {
	sprintf (buf, "End of simulation time <= 0 in %s", filename);
	goto err;
    }

    newVoltscaling = convdec (Max (Abs (minv), Abs (maxv)) / (MAXVOLT / 100));
    if (newVoltscaling == 0.0) newVoltscaling = 1.0;

    if (Append) {
	if (maxt / Timescaling * 10 > MAXTIME) {
	    sprintf (buf, "New endtime value too large");
	    goto err;
	}
	if (Voltscaling == 0.0) {
	    sprintf (buf, "Voltscaling factor == 0");
	    goto err;
	}
	if (newVoltscaling / Voltscaling > 1e4) {
	    sprintf (buf, "New volt scale factor / old scale factor > 1e4");
	    goto err;
	}
	c_v = (int) (minv / Voltscaling);
	if (c_v < Global_umin) Global_umin = c_v;
	c_v = (int) (maxv / Voltscaling);
	if (c_v > Global_umax) Global_umax = c_v;
	c_t = (simtime_t) (mint / Timescaling);
	if (c_t < Begintime) Begintime = c_t;
	c_t = (simtime_t) (maxt / Timescaling);
	if (c_t > Endtime) Endtime = c_t;
    }
    else {
	Timescaling = convdec (maxt / (MAXTIME / 100));
	Voltscaling = newVoltscaling;
	Global_umin = minv / Voltscaling;
	Global_umax = maxv / Voltscaling;
	Begintime = (simtime_t) (mint / Timescaling);
	Endtime   = (simtime_t) (maxt / Timescaling);
    }

    rewind (fp);

    /* Second pass to read the signals */

    while (fread (&sample, sizeof(struct xyz), 1, fp) == 1) {
	if (sample.i != i) {
	    if (!(newsig = find_sig (sample.i))) continue;
	    sig = newsig;
	    i = sample.i;
	}

	c_t = (simtime_t) (sample.t / Timescaling);
	c_v = (int)  (sample.v / Voltscaling);

	if (sig -> end_value_U && sig -> end_value_U -> prev
	    && sig -> end_value_U -> value == c_v
	    && sig -> end_value_U -> prev -> value == c_v) {

	    sig -> end_value_U -> time = c_t;
	}
	else {
	    NEWSVAL (sval);
	    sval -> time  = c_t;
	    sval -> value = c_v;
	    sval -> next = NULL;
	    sval -> prev = sig -> end_value_U;
	    if (sig -> end_value_U)
		sig -> end_value_U -> next = sval;
	    else
		sig -> begin_value_U = sval;
	    sig -> end_value_U = sval;
	}
    }

    fclose (fp);
    return (1);
err:
    windowMessage (buf, -1);
    if (Old_end_signal) {
	delSigList (Old_end_signal -> next);
	Old_end_signal -> next = NULL;
    }
    else {
	delSigList (Begin_signal);
	Begin_signal = NULL;
    }
    End_signal = Old_end_signal;
    Nr_signals = Old_nr_signals;
    if (fp) fclose (fp);
    return (0);
}

static int spice3;
static int hspice;

int readSpice (char *name)
{
    char buf[264];
    char *filename;
    FILE *fp;
    struct signal *sig;
    struct sig_value *sval;
    double t, mint, maxt;
    double v, minv, maxv;
    double newTimescaling, newVoltscaling;
    double newGlobal_umin, newGlobal_umax;
    int c, found, i, Old_nr_signals;
    int c_v;
    simtime_t c_t = 0;
    int rST;

    NEW (filename, strlen (name) + 6, char);
    sprintf (filename, "%s.%s", name, pstar ? "list" : "ana");

    if (!(fp = fopen (filename, "r"))) {
	sprintf (buf, "Cannot open %s", filename);
	windowMessage (buf, -1);
	return (0);
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
    while (1) {
	if ((rST = readSpiceTop (fp)) == -1) break;
	else if (rST == 0) continue;

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

		if (fscanf (fp, "%le", &v) < 0) {
		    sprintf (buf, "Error in reading transient analysis of %s", filename);
		    windowMessage (buf, -1);
		    fclose (fp);
		    return (0);
		}
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

	if (maxt <= 0) {
	    sprintf (buf, "End of simulation time <= 0 in %s", filename);
	    windowMessage (buf, -1);
	    fclose (fp);
	    return (0);
	}

	found = 1;
    }

    newTimescaling = convdec (maxt / (MAXTIME / 100));
    newVoltscaling = convdec (Max (Abs (minv), Abs (maxv)) / (MAXVOLT / 100));

    if (newVoltscaling == 0) newVoltscaling = 1;

    if (Append) {
	if (newVoltscaling / Voltscaling > 1e4) {
	    windowMessage ("New volt scale factor / old scale factor > 1e4", -1);
	    fclose (fp);
	    return (0);
	}
	newGlobal_umin = minv / Voltscaling;
	newGlobal_umax = maxv / Voltscaling;
	if (newGlobal_umin < Global_umin) Global_umin = newGlobal_umin;
	if (newGlobal_umax > Global_umax) Global_umax = newGlobal_umax;
    }
    else {
	Timescaling = newTimescaling;
	Voltscaling = newVoltscaling;
	Global_umin = minv / Voltscaling;
	Global_umax = maxv / Voltscaling;
    }

    rewind (fp);

    /* Second pass to read the signals */

    spice3 = -1;
    hspice = 0;

    found = 0;
    while (1) {
        if ((rST = readSpiceTop (fp)) == -1) break;
	else if (rST == 0) continue;

        if (Append || found) {
	    Old_end_signal = End_signal;
	    Old_nr_signals = Nr_signals;
        }
        else {
	    delSigList (Begin_signal);
	    End_signal = Begin_signal = NULL;
	    Old_nr_signals = Nr_signals = 0;
	    resetStringValues ();
        }

        while ((c = getc (fp)) == ' ' || c == '\t');

        while (c != '\n') {
            ungetc (c, fp);

            fscanf (fp, "%s", buf);

	    NEW (sig, 1, struct signal);
	    NEW (sig -> name, strlen (buf) + 1, char);
	    strcpy (sig -> name, buf);
	    sig -> next = NULL;
	    sig -> prev = NULL;
	    sig -> layover = NULL;
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

	    if (t / Timescaling * 10 > MAXTIME) {
	        windowMessage ("New time values too large (as compared to old values)", -1);
	        delSigList (Old_end_signal -> next);
	        Old_end_signal -> next = NULL;
		fclose (fp);
	        return (0);
	    }

	    c_t = (simtime_t) (t / Timescaling);

	    if (Append || found) {
	        sig = Old_end_signal -> next;
	    }
	    else {
	        sig = Begin_signal;
	    }
	    for (i = Old_nr_signals; i < Nr_signals; i++) {

                if (fscanf (fp, "%le", &v) < 0) {
		    sprintf (buf, "Error in reading transient analysis of %s", filename);
		    windowMessage (buf, -1);
		    fclose (fp);
		    return (0);
	        }
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

		if (Voltscaling == 0.0)
		    c_v = 0;
		else
		    c_v = (int)(v / Voltscaling);

                if (sig -> end_value_U && sig -> end_value_U -> prev
		&& sig -> end_value_U -> value == c_v
	        && sig -> end_value_U -> prev -> value == c_v) {

                    /* save memory space (and execution time) by
                       not storing intermediate values that are
                       equal to a previous and next value.
                    */

                    sig -> end_value_U -> time = c_t;
                }
                else {
		    NEWSVAL (sval);
		    sval -> time = c_t;
		    sval -> value = c_v;
		    sval -> next = NULL;
		    sval -> prev = sig -> end_value_U;
		    if (sig -> end_value_U)
		        sig -> end_value_U -> next = sval;
		    else
		        sig -> begin_value_U = sval;
		    sig -> end_value_U = sval;
                }

	        sig = sig -> next;
	    }

	    if (readSpiceEol (fp)) break;
        }

        if (Append || found) {
	    Begintime = Min (Begintime, (simtime_t) (mint / Timescaling));
	    Endtime = Max (Endtime, c_t);
	}
        else {
	    Begintime = (simtime_t) (mint / Timescaling);
	    Endtime = c_t;
	}

        found = 1;
    }

    if (!found) {
        sprintf (buf, "Cannot find transient analysis table in %s", filename);
        windowMessage (buf, -1);
	fclose (fp);
        return (0);
    }

    fclose (fp);
    return (1);
}

static int readSpiceTop (FILE *fp)
/* return value:  1  matched */
/*                0  not (yet) matched, continue */
/*               -1  not matched, break */
{
    char str1[512];
    char str2[512];
    int c, match;

    if (pstar && spice3 >= 0) {
	if (fscanf (fp, "%s", str1) != 1) return (-1);
	return (*str1 == 'T' ? 1 : -1);
    }
    *str1 = 0;

    match = 0;
    while (!match && fscanf (fp, "%s", str2) == 1) {

	if (spice3 == -1) {
	    if (strsame (str2, "Circuit:")) {
		spice3 = 1; /* it is a spice3 output file */
	    }
	    else
		spice3 = 0; /* it is a spice2 output file */
	}

	if (!spice3 && ((strsame (str1, "TRANSIENT") && strsame (str2, "ANALYSIS"))
	     || (strsame (str1, "TR") && strsame (str2, "Analysis.")) /* pstar */
	     || (strsame (str1, "transient") && strsame (str2, "analysis")))) match = 1;

	if (spice3 && (strsame (str1, "transient") || strsame (str1, "Transient")) && strsame (str2, "Index")) {
	    match = 1;
	}
	if (spice3 && (strsame (str2, "transient") || strsame (str2, "Transient"))) {
	    while (getc (fp) != '\n');
	    if (getc (fp) == '-') {
		while (getc (fp) != '\n');
	    }
	}

	strcpy (str1, str2);
    }

    if (!spice3) {
	if (pstar) {
	    if (!match) return (-1);
	    while (getc (fp) != '\n'); /* read to newline */
	    while (getc (fp) != '\n'); /* skip next line  */
	    if (fscanf (fp, "%s", str1) != 1) return (-1);
	    return (*str1 == 'T');
	}
	if (match && (fscanf (fp, "%s", str1) == 1
		&& (!(strsame (str1, "TEMPERATURE") ||
			strsame (str1, "temperature") ||
			strsame (str1, "tnom=") ||
			strsame (str1, "temp="))))) {
	    return (0);  /* it is an error message or something */
	}

	/* read till end of line */
	while ((c = getc (fp)) != EOF && c != '\n');
	if ((c = getc (fp)) == 'x') {
	    while ((c = getc (fp)) != EOF && c != '\n');
	    hspice = 1;
	}
	else ungetc (c, fp);

	match = 0;
	while (!match && fscanf (fp, "%s", str1) == 1) {
	    if (strsame (str1, "LEGEND:") || strsame (str1, "legend:")) match = 2;
	    else
	    if (strsame (str1, "TIME") || strsame (str1, "time")) {
		match = 1;
		if (hspice) while ((c = getc (fp)) != EOF && c != '\n');
	    }
	}
	if (match == 2) return (0); /* plot_tran */
    }

    if (match && spice3) fscanf (fp, "%*s"); /* skip string 'time' */

    if (!match) return (-1);

    return (1);
}

static int readSpiceEol (FILE *fp)
{
    register int c;

    if (!spice3) {
	while ((c = getc (fp)) != '\n');
	while ((c = getc (fp)) == ' ' || c == '\t');
	ungetc (c, fp);
	if (isdigit (c)) return (0);
	return (1);
    }

    c = getc (fp);
    while (isspace (c)) c = getc (fp);

    if (c == 'I' && getc (fp) == 'n' && getc (fp) == 'd' && getc (fp) == 'e' && getc (fp) == 'x') {
	if (c != EOF) while ((c = getc (fp)) != '\n');
	if (c != EOF) while ((c = getc (fp)) != '\n'); /* skip '-------  ... ' */
	if (c != EOF) c = getc (fp);
    }

    if (isdigit (c)) {
	ungetc (c, fp);
	/* the first item on the line is an index, the second item is the time */
	fscanf (fp, "%*d");
	return (0);
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

struct sig_value *free_svals = 0;

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
