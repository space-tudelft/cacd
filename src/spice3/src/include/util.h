/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef UTIL
#define UTIL

#define MALLOC(x) tmalloc(x)
#define FREE(x) {if (x) { free((char *)(x)); (x) = NULL; }}
#define REALLOC(x,y) trealloc((char *)(x), (y))
#define ZERO(PTR,TYPE) (bzero((PTR),sizeof(TYPE)))

#ifndef _STDLIB_INCLUDED
#define _STDLIB_INCLUDED
#include <stdlib.h>
#endif

extern char *trealloc(char*, int);
extern char *tmalloc(int);

#define TRUE  1
#define FALSE 0

#ifdef DEBUG
#define DEBUGMSG(textargs) printf(textargs)
#else
#define DEBUGMSG(testargs)
#endif

#define FABS(a)  ((a) <  0 ? -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SIGN(a,b) (b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a))

#define ABORT() fflush(stderr);fflush(stdout);abort();

#define ERROR(CODE, MESSAGE) { \
	errMsg = MALLOC(strlen(MESSAGE) + 1); \
	strcpy(errMsg, (MESSAGE)); \
	return (CODE); \
}

#define NEW(TYPE)        ((TYPE *) MALLOC(sizeof(TYPE)))
#define NEWN(TYPE,COUNT) ((TYPE *) MALLOC(sizeof(TYPE) * (COUNT)))

#endif /*UTIL*/
