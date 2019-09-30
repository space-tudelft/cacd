/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

#ifndef MISC_H
#define MISC_H

#define BSIZE_SP      512

#define EXIT_NORMAL 0
#define EXIT_BAD    1

#ifndef isalpha
#include <ctype.h>
#endif

#define eq(a,b)  (!strcmp((a), (b)))

#define hexnum(c) ((((c) >= '0') && ((c) <= '9')) ? ((c) - '0') : ((((c) >= \
        'a') && ((c) <= 'f')) ? ((c) - 'a' + 10) : ((((c) >= 'A') && \
        ((c) <= 'F')) ? ((c) - 'A' + 10) : 0)))

#include <string.h>

extern char *tmalloc(int);
extern char *trealloc(char*, int);
extern void txfree();

#define tfree(x)	(txfree(x), x = NULL)

#define	alloc(TYPE)	((TYPE *) tmalloc(sizeof(TYPE)))
#define	allocn(TYPE,n)	((TYPE *) tmalloc(sizeof(TYPE)*(n)))

extern char *copy();
extern char *gettok();
extern int scannum();
extern int prefix();
extern int ciprefix();
extern int cieq();
extern void strtolower();
extern int substring();
extern char *tilde_expand (char *string);

extern char *datestring();
extern double seconds();

extern char *smktemp();

/* Externs from libc */

#ifndef _STDLIB_INCLUDED
#define _STDLIB_INCLUDED
#include <stdlib.h>
#endif

#ifndef HAS_BSDRAND
#define random	rand
#define srandom	srand
#endif

#define false 0
#define true 1

extern void ivars();

#endif /* MISC_H */
