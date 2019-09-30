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

extern struct signal *Begin_signal;
extern struct signal *End_signal;
extern struct signal *Top_signal;
extern struct signal *Bottom_signal;
extern int Nr_signals;
extern int Curr_nr_signals;

extern struct sig_value *free_svals;

extern simtime_t Begintime;
extern simtime_t curr_time;
extern simtime_t Endtime;
extern simtime_t newEndtime;
extern simtime_t SimEndtime;
extern simtime_t simperiod;
extern simtime_t s_simperiod;
extern simtime_t startTime;
extern simtime_t stopTime;
extern simtime_t storedTD;

extern double Timescaling;
extern double Voltscaling;

extern int Global_umin;
extern int Global_umax;
extern int curr_umin;
extern int curr_umax;

extern int Logic;
extern int currLogic;
extern int Spice;
extern int Cmd;
extern int SimLevel;
extern int doDetailZoom;
extern int tryNonCapital;
extern int usedNonCapital;

extern int Append;

extern int editing;
extern int somethingChanged;

// draw.c
void changeSimEndtime  (double newSimEndtimeS);
void changeTimescaling (double newTimescaling);
double convdec (double f);
void delSigFromCanvas (struct signal *sig, int onlyMoved);
void draw (char modif, Grid x1, Grid y1, Grid x2, Grid y2);
int  drawPointerInfo (int initial, Grid x, Grid y, int flag, int showLogic, int round, int moving);
void drawStartup (void);
void drawValueInfo (int initial, Grid x);
struct signal *existSignal (char *name);
void findBBox (struct signal *sig, simtime_t t1, simtime_t t2, Grid *x1, Grid *y1, Grid *x2, Grid *y2);
void findGrid (int sig_i, simtime_t time, int lval, double fval, Grid *x, Grid *y);
struct signal *findSignal (Grid y, int extend);
int  findState (Grid y);
simtime_t findTime (Grid x, int round);
void insSigOnCanvas (struct signal *sig, struct signal *sig2);
void markSignal (struct signal *sig);
void newSigOnCanvas (char *name);
void redrawListOrMessage (void);
void unMarkSignal (struct signal *sig);
void windowList (char *fn);
void windowMessage (char *s, Grid x);

// edit.c
void addStoredSignalPart (struct signal *sig, simtime_t t, int factor);
int  changeSignal (struct signal *sig, simtime_t t1, simtime_t t2, int state, struct sig_value **prev);
int  compareWord (char *w1, char *w2);
void copySignal (struct signal *sig1, struct signal *sig2);
void disableEditing (void);
void enableEditing (void);
char *getword (FILE *fp);
void storeSignalPart (struct signal *sig, simtime_t t1, simtime_t t2);
int  writeSet (char *filename);

// events.c
void redrawEvent (void);

// main.c
void clear (void);
void die (int status);
void die_alloc (void);
char *modNameOf (char *name);
double slstof (char *s);

// read.c
void delSigexpr (struct signalelement *sigel);
void delSigList (struct signal *sig);
void delSigval (struct sig_value *sigv, struct sig_value *endv);
int  readLogic (char *name, int intermediate);
int  readPlato (char *name);
int  readSpice (char *name);
int  readWaves (char *name, int intermediate);

// readSet.c
int  addSgnPart (struct signal *sig, SIGNALELEMENT *expr, int nr);
void adjustSgnPart (struct signal *sig);
int  readSet (char *name);
int  updateCommandfile (char *name);
int  yywrap (void);

// cmd_y.y
int  yylex (void);
