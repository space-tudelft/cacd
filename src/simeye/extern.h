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

#include <Xm/Xm.h>
#include <stdlib.h>
#include <string.h>

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

extern int currLogic;
extern int Spice;
extern int SimLevel;
extern int doDetailZoom;
extern int tryNonCapital;
extern int usedNonCapital;

extern int Append;
extern int editing;
extern int somethingChanged;

extern char circuitname[];
extern char stimuliname[];
extern char inputname[];
extern char outputname[];
extern char Commandfile[];

/* cmd_l.l */
char *textval (void);

/* cmd_y.y */
void cmdinits (void);
int  yylex (void);

/* draw.c */
void calc_canvas (void);
void changeSimEndtime (double newSimEndtimeS);
void changeTimescaling (double newTimescaling);
void clear (void);
double convdec (double f);
void delSigFromCanvas (struct signal *sig);
void draw (char modif, Grid x1, Grid y1, Grid x2, Grid y2);
void drawPointerInfo (int initial, Grid x, Grid y, int flag, int showLogic, int moving);
void drawStartup (void);
void drawValueInfo (int initial, Grid xt);
struct signal *existSignal (char *name);
void findBBox (struct signal *sig, simtime_t t1, simtime_t t2, Grid *x1, Grid *y1, Grid *x2, Grid *y2);
void findGrid (int sig_i, simtime_t time, int lval, double fval, Grid *x, Grid *y);
void findPutBBox (struct signal *sig, simtime_t t1, Grid *px1, Grid *py1, Grid *px2, Grid *py2);
struct signal *findSignal (Grid y, int extend);
int  findState (Grid y);
simtime_t findTime (Grid x);
void markSignal (struct signal *sig);
void moveSigOnCanvas (struct signal *sig, struct signal *sig2);
void newSigOnCanvas (char *name);
void redrawMessage (void);
void unMarkSignal (struct signal *sig);
void viewValues (Widget w, caddr_t client_data, caddr_t call_data);
void windowList (char *fn);
void windowMessage (char *s, Grid x);

/* edit.c */
int  addStoredSignalPart (struct signal *sig, simtime_t t, int factor);
int  compareWord (char *w1, char *w2);
void Copysig (Widget w, caddr_t client_data, caddr_t call_data);
void copySignal (struct signal *sig1, struct signal *sig2);
void Change  (Widget w, caddr_t client_data, caddr_t call_data);
int  changeSignal (struct signal *sig, simtime_t t1, simtime_t t2, int state, struct sig_value **prev);
void Delsig  (Widget w, caddr_t client_data, caddr_t call_data);
void disableEditing (void);
char eval2cmdval (int v);
char *getword (FILE *fp);
void Put     (Widget w, caddr_t client_data, caddr_t call_data);
void Rename  (Widget w, caddr_t client_data, caddr_t call_data);
void storeSignalPart (struct signal *sig, simtime_t t1, simtime_t t2);
int  writeSet (int SaveAs);
void Yank    (Widget w, caddr_t client_data, caddr_t call_data);

/* events.c */
void beginCommand (Widget w, int cT);
void endCommand (void);
void eventsStartup (void);
void PutCancel (void);
void PutOk (int factor);
void redrawEvent (void);
void RenameOk (char *name);

/* main.c */
void ask_default (char *s);
void die (int status);
void die_alloc (void);
void errorMessage (char *s);
void pDialogCB (Widget w, caddr_t client_data, caddr_t call_data);
void set_filename (char *name);
void setPrintButton (void);
double slstof (char *s);

/* read.c */
void delSig (struct signal *sig);
void delSigexpr (struct signalelement *sigel);
void delSigList (struct signal *sig);
void delSigval (struct sig_value *sigv, struct sig_value *endv);
int  readWaves (int intermediate);
int  readLogic (int intermediate);
int  readSpice (int intermediate);

/* readSet.c */
int  addSgnPart (struct signal *sig, SIGNALELEMENT *expr, int nr);
void adjustSgnPart (struct signal *sig);
void readSet (int done);
int  yywrap (void);

/* res.c */
STRING_REF *names_from_path (int traillen, PATH_SPEC *path);
RES_FILE *read_paths (FILE *fp, char *fn);
