/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	S. de Graaf
 *	T.G.R. van Leuken
 *	P. Kist
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

struct mo_elmt {
    char  *oname; /* cell name */
    char  *nname; /* new cell name */
    DM_CELL *key; /* cell key */
    DM_PROJECT *pkey; /* project key */
    int      snr; /* symbolnr (CIF) */
    int      level;
    int      mcalls;
    struct mo_elmt *onext;
    struct mo_elmt *nnext;
    struct mo_elmt *next;
    struct mo_elmt *prev;
};

struct ic_elmt {
    char  *mo;
    struct ic_elmt *l;
    struct ic_elmt *r;
};

#define LDM 1
#define CIF 2
#define CMK 3

#define PE fprintf (stderr,
#define PF fprintf (fp_file,

#define ALLOC(ptr,type) {\
if (!(ptr = (type *) malloc (sizeof (type))))\
error (3, "type"); }

#define FREE(ptr) free((char *)(ptr))

char *d2a (double);
char *strsave (char *, int);
struct mo_elmt *findcell (DM_PROJECT *, char *);
void inst_alias (struct ic_elmt *, char *);
struct ic_elmt *inst_ic_elmt (char *);
void error (int nr, char *s);
void die (int  status);
void outp_cif (struct mo_elmt *p);
void outp_cmk (struct mo_elmt *p);
void outp_ldm (struct mo_elmt *p);
void outp_lld (void);

