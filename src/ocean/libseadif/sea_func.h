/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
/*
 * Set of available seadif functions.
 */

#ifndef __SEA_FUNC_H
#define __SEA_FUNC_H

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"

/* the first argument of sdfreport() is of this type */
typedef enum {Warning, Error, Fatal} errorLevel;

/* arguments for sdfoptions() */
typedef enum {SdfWarningsOff, SdfWarningsOn, SdfLockfilesOff, SdfLockfilesOn} seadifOptions;

#ifdef __cplusplus
extern LINKAGE_TYPE
{
#endif

char *mnew (int siz);
void mfree (char **blk, int siz);

char *cs (char *str);
int fs (char *stringtoforget);

int  sdfopen (void);
void sdfclose (void);
void sdfflatcir (CIRCUITPTR circuit);

int sdfreadlay (int what, STRING layname, STRING cirname, STRING funname, STRING libname);
int sdfreadcir (int what, STRING cirname, STRING funname, STRING libname);
int sdfreadfun (int what, STRING funname, STRING libname);
int sdfreadlib (int what, STRING libname);

int sdfreadalllay (int what, STRING layname, STRING cirname, STRING funname, STRING libname);
int sdfreadallcir (int what, STRING cirname, STRING funname, STRING libname);

void sdfwritealllay (int what, LAYOUTPTR lay);
void sdfwriteallcir (int what, CIRCUITPTR cir);

int sdfwritelay (int what, LAYOUTPTR lay);
int sdfwritecir (int what, CIRCUITPTR cir);
int sdfwritelib (int what, LIBRARYPTR lib);
int sdfwritefun (int what, FUNCTIONPTR fun);

int sdftouchlay (LAYOUTPTR lay, time_t timestamp);
int sdftouchcir (CIRCUITPTR cir, time_t timestamp);
int sdftouchlib (LIBRARYPTR lib, time_t timestamp);
int sdftouchfun (FUNCTIONPTR fun, time_t timestamp);

int sdfexistslib (STRING libname);
int sdfexistsfun (STRING funname, STRING libname);
int sdfexistscir (STRING cirname, STRING funname, STRING libname);
int sdfexistslay (STRING layname, STRING cirname, STRING funname, STRING libname);

int sdfremovelib (STRING libname);
int sdfremovefun (STRING funname, STRING libname);
int sdfremovecir (STRING cirname, STRING funname, STRING libname);
int sdfremovelay (STRING layname, STRING cirname, STRING funname, STRING libname);

int sdfattachlib (LIBRARYPTR lib, STRING filename);

int sdflistlay (int what, CIRCUITPTR circuit);
int sdflistcir (int what, FUNCTIONPTR function);
int sdflistfun (int what, LIBRARYPTR library);
int sdflistlib (int what);
int sdflistalllay (int what, CIRCUITPTR circuit);
int sdflistallcir (int what, FUNCTIONPTR function);

int sdfmakeshapef (CIRCUIT *circuit);

void sdfflatcir (CIRCUIT *circuit);

char *sdfgetcwd (void);

void sdfdeletecirport (CIRPORTPTR cirport);
void sdfdeletecirinst (CIRINSTPTR cirinst, int recursively);
void sdfdeletenetlist (NETPTR netlist);
void sdfdeletecircuit (CIRCUITPTR circuit, int recursively);
void sdfdeletebuslist (BUSPTR circuit);
void sdfdeletelayout (LAYOUTPTR layout, int recursively);
void sdfdeletelayport (LAYPORTPTR layport);
void sdfdeletelaylabel (LAYLABELPTR laylabel);
void sdfdeleteslice (SLICEPTR slice, int recursively);
void sdfdeletelayinst (LAYINSTPTR layinst, int recursively);
void sdfdeletewire (WIREPTR wire);
void sdfdeletetiming (TIMINGPTR timing);
void sdfdeletenetmod (NETMODPTR netmod);
void sdfdeletetterm (TIMETERMPTR t_term);
void sdfdeletetpath (TPATHPTR tPath);
void sdfdeletetimecost (TIMECOSTPTR tcost);
void sdfdeletedelasg (DELASGPTR delasg);

void err (int errcode, char *errstring);
void sdfexit (int code);
void sdfinitsignals (void);
void sdfoptions (seadifOptions options);
void sdfreport (errorLevel level, const char *message, ...);

int sdfmakelibalias (STRING alias, STRING lnam);
int sdfmakefunalias (STRING alias, STRING fnam, STRING lnam);
int sdfmakeciralias (STRING alias, STRING cnam, STRING fnam, STRING lnam);
int sdfmakelayalias (STRING alias, STRING lnam, STRING cnam, STRING fnam, STRING bnam);
int sdfaliastoseadif (STRING alias, int objecttype);
STRING sdflibalias (STRING lnam);
STRING sdffunalias (STRING fnam, STRING lnam);
STRING sdfciralias (STRING cir, STRING fnam, STRING lnam);
STRING sdflayalias (STRING lnam, STRING cir, STRING fnam, STRING bnam);

STRING sdfoceanpath (void);
STRING sdfimagefn (void);
FILE  *sdfimagefp (void);

#ifdef __cplusplus
}
#endif

#endif
