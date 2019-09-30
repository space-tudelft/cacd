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
#include <string.h>

#define	BSCOPY	strcpy
#define	BSNCOPY	strncpy
#define	BSCMP	strcmp
#define	BSNCMP	strncmp
#define	BSLEN	strlen
#define	BTTRUE	'I'
#define	BTFALSE	'O'
#define	BTUNDEF	'X'
#define	BTFREE	'F'

extern void delay (char type, float val, int outnmb);
extern void cap_add (float val, int type, int innmb);
extern float cap_val (int type, int vicin, int min_max, int term_nmb);
extern float statcap_val (int type, int vicin, int min_max, int term_nmb);
extern float dyncap_val (int type, int vicin, int min_max, int term_nmb);
extern char *adm_bsalloc (int p, char mode);

extern char BTAND ();
extern char BTEXNOR ();
extern char BTEXOR ();
extern char BTINVERT ();
extern char BTNAND ();
extern char BTNOR ();
extern char BTOR ();

extern void BSFREE ();
extern void BSRESET ();
extern void BSSET ();
extern void BSUNDEF ();
extern char * BSROTATE ();
extern int BSTOI ();

extern char * BWAND ();
extern char * BWEXNOR ();
extern char * BWEXOR ();
extern char * BWINVERT ();
extern char * BWNAND ();
extern char * BWNOR ();
extern char * BWOR ();

extern char * ITOBS ();
extern char * ITOTC ();
extern int TCTOI ();

extern void curs_error ();
extern void func_error ();
extern void single_curs_step ();
extern void single_step ();
