/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Paul Stravers
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

#ifndef __CEDIF_H
#define __CEDIF_H

#include "src/ocean/libseadif/sealib.h"

/* /////////////////////////////////////////////////////////////////////////
			G L O B A L   C ON S T A N T S
   //////////////////////////////////////////////////////////////////////// */

/* if we create stubs for references to an external library,
   we set this in the flag.l field: */
#define EXTERNAL_STUB    1
#define EXTERNAL_LIBRARY 2
#define RESOLVED_CELL    4	  /* set after cell has been resolved */

/* maximum number of external libraries that we can handle: */
#define MAXEXTERNLIBS 100

/* /////////////////////////////////////////////////////////////////////////
			 G L O B A L   T Y P E D E F S
   //////////////////////////////////////////////////////////////////////// */

/* used in edif.y and in edif.tab.h which is included by lex.edif.c: */
typedef enum _seadifViewType
{ SeadifFunctionView, SeadifCircuitView, SeadifLayoutView, SeadifNoView } seadifViewType;

/* values for the first argument to the error() function: */
typedef enum { eWarning, eFatal } xErrorLevel;

/* The parser does not solve references to instances. In stead, it stores all
 * the relevant information in a struct INSTANCE_T and puts a pointer to this
 * struct in the instance name field (with a typecast of course).
 */
typedef struct _INSTANCE_T
{
   STRING   instance_name;
   STRING   view_name_ref;
   STRING   cell_ref;
   STRING   library_ref;
}
INSTANCE_T, *INSTANCE_TPTR;

#define NewInstance_t(p) ((p)=(INSTANCE_TPTR)mnew(sizeof(INSTANCE_T)))
#define FreeInstance_t(p) \
   { if ((p)->instance_name) fs ((p)->instance_name); \
     if ((p)->view_name_ref) fs ((p)->view_name_ref); \
     if ((p)->cell_ref)      fs ((p)->cell_ref);      \
     if ((p)->library_ref)   fs ((p)->library_ref);   \
     mfree ((char **)(p), sizeof(INSTANCE_T)); }

typedef enum _languageType
{ NoLanguage, NelsisLanguage, SeadifLanguage, PseudoSeadifLanguage } languageType;

/* /////////////////////////////////////////////////////////////////////////
			G L O B A L   F U N C T I O N S
   //////////////////////////////////////////////////////////////////////// */

#ifdef __cplusplus
extern "C" {
#endif

void       report(const xErrorLevel, const char *, ...);
int        edifparse();
int        ediflex();
int        ediferror(char *mesg);
int        solveRef(SEADIF *edif_local);
LIBRARYPTR findNelsisLibrary(STRING libname, SEADIFPTR whereToPut);
LIBRARYPTR findSeadifLibrary(STRING libname, SEADIFPTR whereToPut);
CIRCUITPTR findNelsisCircuit(STRING cirname, LIBRARYPTR lib);
CIRCUITPTR findSeadifCircuit(STRING cirname, LIBRARYPTR lib);
void       writeNelsisCircuit(CIRCUITPTR circuit);
void       writeSeadifCircuit(CIRCUITPTR circuit);
void       openNelsis();
void       openSeadif();
void       closeNelsis();
void       closeSeadif();
void       exitNelsis(int);
void       exitSeadif(int);
STRING     mapL(STRING fromlib);
void       makeMapL(STRING fromLib, STRING toLib);

#ifdef __cplusplus
}
#endif

/* /////////////////////////////////////////////////////////////////////////
			G L O B A L   V A R I A B L E S
   //////////////////////////////////////////////////////////////////////// */

/* this struct is filled by edifparse(): */
extern SEADIF edif_source;

/* this is where solveRef() puts the resolved cells: */
extern SEADIF *seadif_tree;

/* this int counts the lines in the file parsed by ediflex() */
extern int ediflineno;

/* this holds the literal string that represents the current ediflex() token */
extern char *ediftext;

/* this is the file where ediflex() reads its input from: */
extern FILE *edifin;

/* this holds the target language that we are currently converting to: */
extern languageType targetLanguage;

/* This is TRUE if EDIF "external" must behave exactly as "library": */
extern int externalBehavesLikeLibrary;

#endif
