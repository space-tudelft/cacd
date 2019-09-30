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
 * Function prototypes for all sealib modules.
 */

#ifndef __SEA_DECL_H
#define __SEA_DECL_H

#include <stdio.h> /* because we need FILE */

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/namelist.h"
#include "src/ocean/libseadif/sea_func.h"

#ifdef __cplusplus
extern LINKAGE_TYPE
{
#endif

/* stv_table.c */
void sdfstv_insert (STRING string, void *pointer);
void *sdfstv_lookup (STRING string);
void sdfstv_cleanup (void);
void sdfstv_statistics (void);

/* readdb.c */
int sdfparseandmakeindex (void);
int sdfreadindexfile (FILEPTR idxstream, int currentfileidx);
int ck_addlibtohashtable (LIBRARYPTR lib, SDFINFO *info, STRING alias);
int ck_addfuntohashtable (FUNCTIONPTR fun, LIBTABPTR libtab, SDFINFO *info, STRING alias);
int ck_addcirtohashtable (CIRCUITPTR  cir, FUNTABPTR funtab, SDFINFO *info, STRING alias);
int ck_addlaytohashtable (LAYOUTPTR   lay, CIRTABPTR cirtab, SDFINFO *info, STRING alias);
STRING mklockfilename (int idx);
int solvecircuitinstance (CIRINSTPTR cirinst, CIRCUITPTR parentcircuit, int verbose);
int solvecircuitcirportrefs (CIRCUITPTR circuit);
int solvelayoutlayportrefs (LAYOUTPTR layout, CIRCUITPTR circuit);

/* err.c */
void dumpcore (void);

/* dumpdb.c */
int setdumpspacing (int newspacing);
int setdumpstyle (int newstyle);
int setcomments (int newaddcomments);
int seterrors (int newadderrors);
int setobjectiveslice (void);
int setcompactslice (void);
void dump_alias (FILEPTR fp, STRING alias);
void dumpdb (FILEPTR fp, SEADIFPTR seadif);
void dump_seadif (FILEPTR fp, SEADIFPTR seadif);
void dump_status (FILEPTR fp, STATUSPTR status);
void dump_library (FILEPTR fp, LIBRARYPTR library);
void dump_function (FILEPTR fp, FUNCTIONPTR function);
void dump_funtype (FILEPTR fp, STRING funtype);
void dump_circuit (FILEPTR fp, CIRCUITPTR circuit);
void dump_cirportlist (FILEPTR fp, CIRPORTPTR port);
void dump_cirinstlist (FILEPTR fp, CIRCUITPTR circuit);
void dump_cirinst (FILEPTR fp, CIRINSTPTR cirinst);
void dump_netlist (FILEPTR fp, NETPTR net);
void dump_net (FILEPTR fp, NETPTR net);
void dump_buslist (FILEPTR fp, BUSPTR bus);
void dump_bus (FILEPTR fp, BUSPTR bus);
void dump_layout (FILEPTR fp, LAYOUTPTR layout);
void dump_off (FILEPTR fp, short off[2]);
void dump_bbx (FILEPTR fp, short bbx[2]);
void dump_layportlist (FILEPTR fp, LAYPORTPTR port);
void dump_layport (FILEPTR fp, LAYPORTPTR layport);
void dump_laylabellist (FILEPTR fp, LAYLABELPTR label);
void dump_laylabel (FILEPTR fp, LAYLABELPTR laylabel);
void dump_slice (FILEPTR fp, SLICEPTR sl);
void dump_layinst (FILEPTR fp, LAYINSTPTR layinst);
void dump_wirelist (FILEPTR fp, WIREPTR wire);
void dump_layinst2 (LAYINSTPTR layinst);
void dump_slice2 (SLICEPTR slice);
void sdfdumphashtable (FILEPTR fp);
void sdfdumphashlib (FILEPTR fp, LIBTABPTR lib);
void sdfdumphashfun (FILEPTR fp, FUNTABPTR fun);
void sdfdumphashcir (FILEPTR fp, CIRTABPTR cir);
void sdfdumphashlay (FILEPTR fp, LAYTABPTR lay);

void dump_timing (FILEPTR fp, TIMINGPTR timing);
void dump_timetermlist (FILEPTR fp, TIMETERMPTR ttermPtr);
void dump_tminstlist (FILEPTR fp, TMMODINSTPTR modinstPtr);
void dump_netmodlist (FILEPTR fp, NETMODPTR netmodPtr);
void dump_tpathlist (FILEPTR fp, TPATHPTR tpathPtr);
void dump_timecost (FILEPTR fp, TIMECOSTPTR timecostPtr);
void dump_delasg (FILEPTR fp, DELASGPTR delasgPtr);
void dump_timeterm (FILEPTR fp, TIMETERMPTR timetermPtr);
void dump_netmod (FILEPTR fp, NETMODPTR netmodPtr);
void dump_tpath (FILEPTR fp, TPATHPTR tpathPtr);
void dump_starttermlist (FILEPTR fp, TIMETERMREFPTR termPtr);
void dump_endtermlist (FILEPTR fp, TIMETERMREFPTR termPtr);
void dump_delasglist (FILEPTR fp, DELASGINSTPTR delasginstPtr);

/* namelist.c */
void tonamelist (NAMELISTPTR *namelist, char *libname);
void fromnamelist (NAMELISTPTR *namelist, char *libname);
int isinnamelist (NAMELISTPTR namelist, char *libname);
int isemptynamelist (NAMELISTPTR namelist);
void tonamelistlist (NAMELISTLISTPTR *namelistlist, NAMELISTPTR namelist, char *namelistname);
void sdfpathtonamelist (NAMELISTPTR *nl, char *path);
void tonum2list (NUM2LISTPTR *num2list, long num1, long num2);
void fromnum2list (NUM2LISTPTR *num2list, long num1, long num2);
int isinnum2list (NUM2LISTPTR num2list, long num1, long num2);
int isemptynum2list (NUM2LISTPTR num2list);

/* seaflat.c */
void grandpa_meets_grandchildren (CIRCUITPTR grandpa, int libprim);
void freetreecirinst (CIRINSTPTR cirinst);
void freetreecircuit (CIRCUITPTR circuit);
CIRCUITPTR copycircuit (CIRCUITPTR circuit);

/* yywrap.c */
int yywrap (void);

/* slicecle.c */
void slicecleanup (SLICEPTR *slice);
void collectlayinstances (SLICEPTR slice);

/* support.c */
int issubstring (STRING str1, STRING str2);

/* libio.c */
STRING mkindexfilename (STRING origname);
void sdfinsertthing (SDFINFO *frominfo, STRING name, int skipclosingparen,
	STRING skipthissexp, FILEPTR indexstream, STRING indexid, STRING alias);
void putcindented (int c, FILEPTR stream);
void sdfwritealllay_2 (int what, LAYOUTPTR lay);

/* timecvt.c */
int sdftimecvt (time_t *thetime, int yy, int mo, int dd, int hh, int mi, int ss);

/* libnamet.c */
void initlibhashtable (void);
void initcirhashtable (void);
void initfunhashtable (void);
void initlayhashtable (void);
void sdfclearlibhashtable (void);
void sdfclearfunhashtable (void);
void sdfclearcirhashtable (void);
void sdfclearlayhashtable (void);
void addlibtohashtable (LIBRARYPTR lib, SDFINFO *info);
void addfuntohashtable (FUNCTIONPTR fun, LIBTABPTR lib, SDFINFO *info);
void addcirtohashtable (CIRCUITPTR cir, FUNTABPTR fun, SDFINFO *info);
void addlaytohashtable (LAYOUTPTR lay, CIRTABPTR cir, SDFINFO *info);
int libnametoptr (LIBRARYPTR *libptr, STRING libname);
int existslib (STRING libname);
int funnametoptr (FUNCTIONPTR *funptr, STRING funname, STRING libname);
int existsfun (STRING funname, STRING libname);
int cirnametoptr (CIRCUITPTR *cirptr, STRING cirname, STRING funname, STRING libname);
int existscir (STRING cirname, STRING funname, STRING libname);
int laynametoptr (LAYOUTPTR *layptr, STRING layname, STRING cirname,
	STRING funname, STRING libname);
int existslay (STRING layname, STRING cirname, STRING funname, STRING libname);

/* signal.c */
void sdfterminate (int signum);

/* defaultenv.c */
char *tryNelsisSealib (void);

/* simlink.c */
char *abscanonicpath (char *sympath);

/* hashstring.h */
int sdfhashstring   (char *str1, int tabsize);
int sdfhash2strings (char *str1, char *str2, int tabsize);
int sdfhash3strings (char *str1, char *str2, char *str3, int tabsize);
int sdfhash4strings (char *str1, char *str2, char *str3, char *str4, int tabsize);

/* sdfparse.y */
int nextchar (FILEPTR stream);
int sdfparse (int idx);

/* sdflex.h.l */
void sdfabortcopy (int discardspaces);
void sdfresumecopy (void);
void sdfuncopysincelastchar (int chartodeletefrombuf);
void sdfpushcharoncopystream (int c);
int  sdfdodelayedcopy (int charstoleave);

#ifdef __cplusplus
}
#endif

/* Yacc++ declares these with C++ linkage, that's why they're here: */
int  seadifparse (void);
void seadiferror (char *s); /* equivalent of yyerror() */
int  seadiflex (void);

#endif
