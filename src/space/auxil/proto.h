/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#ifdef __cplusplus
  extern "C" {
#endif

/* alarm.c */
int setAlarmInterval (int sec, void (*handler)(void));

/* assert.c */
void assertion_failed (const char *file, int line, const char *cond);

/* debug.c */
bool_t cIfDebug (char *file, int line);
void  cSetDebug (char *file);

/* clock.c */
void clockInit (void);
void tick (char *s);
void tock (char *s);
void clockPrintAll (FILE *fp);
void clockPrintTime (FILE *fp);

/* color.c */
int isCOLOR_EQ_COLOR (mask_t *x, mask_t *y);
int isCOLOR_PRESENT  (mask_t *x, mask_t *y);
int isCOLOR_ABSENT   (mask_t *x, mask_t *y);
int isCOLOR (mask_t *x);
int colorindex (mask_t *a);
char *colorBitStr (mask_t *a);
char *colorOctStr (mask_t *a);
char *colorHexStr (mask_t *a);
char *colorIntStr (mask_t *a);
void initcolorhex (mask_t *a, char *hex);
void initcolorint (mask_t *a, char *ins);
void initcolorbits (mask_t *a, char *bit);
void setNcol (int n);

/* die.c */
void die (void);

/* extrasay.c */
void extraSay (void);

/* fopen.c */
FILE *cfopen (char *file, const char *mode);

/* gauss.c */
double gauss (double (*func)(double), double low, double upp, double EPS);

/* malloc.c */
int mallocFit (unsigned size, unsigned num);

/* monit.c */
void startmonitime (char *pname);
void monitime (char *str);
void stopmonitime (void);

/* monitor.c */
void monitorStart (void);
void monitorStop (void);

/* mprintf.c */
char *mprintf (const char *format, ...);

/* new.c */
void *malloc_p (unsigned size, const char *file, int line);
void free_p (void *d, unsigned size, const char *file, int line);
void *realloc_p (void *o, unsigned size, unsigned oldsize, const char *file, int line);
void memprofTurnOff (void);
double allocatedMbyte (void);

/* param.c */
void paramError (char *param, const char *s, ...);
void paramSetVerbose (int level);
void paramReadFile (char *paramfile);
void paramSetOption (char *option);
void paramSetOption2 (char *key, char *value);
char * paramLookupS (const char *key, const char *dflt);
double paramLookupD (const char *key, const char *dflt);
int    paramLookupI (const char *key, const char *dflt);
bool_t paramLookupB (const char *key, const char *dflt);
int   paramGetOptionCount (void);
char *paramGetOptionKey   (int Index);
char *paramGetOptionValue (int Index);
void printUnusedParams (void);

/* say.c */
void verboseSetErrStream (FILE *fp); /* default stderr */
void verboseSetOutStream (FILE *fp); /* default stdout */
void verboseSetNewStream (FILE *fp);
void swapVerboseStreams (void);
void setMaxMessageCnt (int cnt); /* for say() and verbose() */
void say     (const char *msg, ...); /* uses ErrStream */
void verbose (const char *msg, ...); /* uses OutStream */
void message (const char *msg, ...); /* uses OutStream */
void verboseSetMode (bool_t mode);
void verboseSetLevel (int level);
int  verboseGetLevel (void);
void raiseVerbosity (void);
void lowerVerbosity (void);

/* signal.c */
void catchSignals (void);

/* strfind.c */
char *strfind (char *s1, char *s2);

/* strsave.c */
char *strsave (const char *s);

/* binom.c */
int binomial (int n, int k);

/* factln.c */
double factln (int n);

/* gammaln.c */
double gammln (double xx);

/* levin.c */
double levin (double term, int termnr);

/* multinom.c */
int multinomial (int n, int *powers, int num_vars);

/* tempdir.c */
char *tempdir (void);
char *tempname (const char *pfx, const char *buf, int size);

#ifdef __cplusplus
  }
#endif
