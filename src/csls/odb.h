/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#define	PBLKSIZ	1024
#define	DBLKSIZ	1024
#define	BYTESIZ	8

typedef	struct datum
{
	char	*dptr;
	int	dsize;
} datum;

typedef struct db_files {
	char * dbname;
	int DbRdOnly;
	int dir_b, dir_t;
	int pag_b, pag_t;
	FILE *DirF;
	FILE *PagF;

	long	bitno;
	long	maxbno;
	long	blkno;
	long	hmask;

	char	pagbuf[PBLKSIZ];
	char	dirbuf[DBLKSIZ];
} DbAcces;
