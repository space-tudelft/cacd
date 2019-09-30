/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

/*
    CACD-RELEASE-ALLOWED
*/

#define VERSION  "3.5"

#define ERROR1               01
#define ERROR2               02
#define WARNING              03

#define Forced               01
#define Normal               00

#define Strong               01
#define Weak                 02

#define Plot                 10

#define Pullup               01
#define Pulldown             02
#define Passup               03
#define Passdown             04
#define Load                 05
#define Superload            06

#define Rdyn                 01
#define Cch                  02
#define Rstat                03
#define Rsatu                04
#define Cgstat               05
#define Cgrise               06
#define Cgfall               07
#define Cestat              010
#define Cerise              011
#define Cefall              012
#define Rlin                013

#define F_state              00
#define L_state              01
#define X_state              02
#define H_state              03
/* only used for inputs of abstract outputs: */
#define Dontcare             04

#define Open                 01
#define Closed               02
#define Undefined            03

#define RISE                 01
#define FALL                 02

#define FAST                 01
#define SLOW                 02

#define NODECAP_DEF       1e-30
#define NENH_RSTAT_DEF        1
#define PENH_RSTAT_DEF        1
#define DEPL_RSTAT_DEF   100000
#define RES_RSTAT_DEF    0.0001
#define VMINH_REL_DEF     0.999
#define VMAXL_REL_DEF     0.001

#define VOLTMAX_INT         200

#define MAXFTAB               7

#define NOT                  01
#define LINEAIR              02
#define INVERSE              03

#define MAXHIERAR            10
#define MAXPLOT             255  /* never change this value */

#define MAXRESIST          1e30
#define MAXSIMTIME   10000000000000000LL

#define D_ROUND(d) (d) + 0.5
#define LSTATE(n) ( (n) -> essential ? (unsigned) ((n) -> state) : lstate(n) )

#define MAX(i,j)	(((i)>(j))?(i):(j))
#define MIN(i,j)	(((i)<(j))?(i):(j))

#define PALLOC(ptr, nel, type) { \
if (nel == 0) ptr = NULL; \
else if (!(ptr = (type *)calloc ((unsigned)(nel), sizeof (type)))) { \
     cannotAlloc (__FILE__, __LINE__, (int)(nel), sizeof (type)); }}

#define PPALLOC(ptr, nel, type) { \
if (nel == 0) ptr = NULL; \
else if (!(ptr = (type **)calloc ((unsigned)(nel), sizeof (type *)))) { \
      cannotAlloc (__FILE__, __LINE__, (int)(nel), sizeof (type *)); }}

#define CFREE(ptr) { free ((void *)ptr); }

#define OPENR(fptr, str) { \
if (!(fptr = fopen (str, "r"))) slserror (NULL, 0, ERROR1, "Cannot read", str); }

#define OPENW(fptr, str) { \
if (!(fptr = fopen (str, "w"))) slserror (NULL, 0, ERROR1, "Cannot write", str); }

#define CLOSE(fptr) { fclose (fptr); }

#define ERROR_EXIT(nr) { \
fprintf (stderr, "Internal error on \"%s\", line %d\n", __FILE__, __LINE__); die (nr); }
