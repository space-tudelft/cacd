/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/auxil/portable.h"
#include "src/space/auxil/bool.h"
#include "src/space/auxil/color.h"
#include "src/space/auxil/debug.h"
#include "src/space/auxil/assert.h"
#include "src/space/auxil/malloc.h"
#include "src/space/auxil/proto.h"
#include "src/space/auxil/monitor.h"

/* 'Standard' macros
*/
#define strsame(s1,s2)  (!strcmp(s1,s2))
#define	Abs(a)		((a) < 0   ?-(a) : (a))
#define Min(a,b)	((a) < (b) ? (a) : (b))
#define	Max(a,b)	((a) > (b) ? (a) : (b))
#define Round(x)	(((x) > 0) ? ((x)+0.5) : ((x)-0.5))
#define Null(type)      ((type)0)
#define Swap(type,a,b)  {type c=a; a=b; b=c;}

/*  Next define is used for static functions.
    Since the gprof profiler does not interpret static functions
    very well, this can then be defined empty
    (Provided there are no name clashes.)
    Protected by ifndef to allow -DPrivate="" on command line
*/
#ifndef Private
#define Private static /* or empty */
#endif

#endif /* AUXIL_H */
