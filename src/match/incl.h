/* rcsid = "$Id: incl.h,v 1.1 2018/04/30 12:17:31 simon Exp $" */
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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

#define CALLOC(ptr, nel, type) \
if (!(ptr = (type *) calloc ((size_t)(nel), sizeof(type)))) Fatal(5)

#define MALLOC(ptr, nel, type) \
if (!(ptr = (type *) malloc (nel * sizeof(type)))) Fatal(5)

#define FREE(ptr) free ((void *)ptr)

/* cirTree.c */
int cirTree (struct cir *cl, DM_CELL *ckey);
int is_dev  (void);
int is_func (void);
int is_tor  (void);

/* readdb.c */
void read_dbase (DM_PROJECT *projkey, string rootntw, hash *hashlist, DM_CELL *ckey);

/* xnetw.c and readDev.c */
void xnetwork (DM_PROJECT *projkey, char *ntwname, DM_CELL *ckey);
void readTerm (network *ntw, DM_CELL *key, int funf);
void readNet  (DM_CELL *key);
void readDev  (DM_CELL *key);

