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

#ifndef AUXIL_H
#define AUXIL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <limits.h>		/* for INT_MAX and friends in portable.h */
#include "src/space/include/config.h"
#include "src/space/auxil/portable.h"
#include "src/space/auxil/bool.h"
#include "src/space/auxil/color.h"
#include "src/space/auxil/debug.h"
#include "src/space/auxil/assert.h"
#include "src/space/auxil/malloc.h"
#include "src/space/auxil/proto.h"

/* 'Standard' macros
*/
#define EOS             '\0'
#define strsame(s1,s2)  (!strcmp(s1,s2))
#define	Abs(a)		((a) < 0   ?-(a) : (a))
#define Min(a,b)	((a) < (b) ? (a) : (b))
#define	Max(a,b)	((a) > (b) ? (a) : (b))
#define Round(x)	(((x) > 0) ? ((x)+0.5) : ((x)-0.5))
#define Sign(n)	        ((n)>0?1:((n)<0?-1:0))
#define Sqr(x)          ((x)*(x))
#define Dsqr(x)         (((double)(x))*((double)(x)))
#define Null(type)      ((type)0)
#define Ctl(ch)         (ch & 037)
#define Swap(type,a,b)  {type c=a; a=b; b=c;}

/* File open and close
*/
#define OPENR(fp, fn)   fp=cfopen(fn, "r")
#define OPENW(fp, fn)   fp=cfopen(fn, "w")
#define CLOSE(fp)       { fclose (fp); }

/*  Next define is used for static functions.
    Since the gprof profiler does not interpret static functions
    very well, this can then be defined empty
    (Provided there are no name clashes.)
    Protected by ifndef to allow -DPrivate="" on command line
*/
#ifndef Private
#define Private static /* or empty */
#endif

/* to make the compiler happy about unused function arguments */
#define UseArg(x) (x = x)

#endif /* AUXIL_H */
