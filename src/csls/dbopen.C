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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

DbAcces * dbOpen (char   *dbname)
{
/* 	Open object database (ODB) dbname :
 *		dbname.dir  - hash info to acces ...pag dbname
 *		dbname.pag  - object adress <-> object key info
 *
 */
    DbAcces * dbacces;

#define  OO_BIN  0

    dbacces = (DbAcces *) calloc (1, sizeof (DbAcces));
    dbacces -> DbRdOnly = 0;
    dbacces -> dbname = (char *) calloc (strlen (dbname) + 1, sizeof (char));
    strcpy (dbacces -> dbname, dbname);

/*	Open pag file
 */
    strcpy (dbacces -> pagbuf, dbname);
    strcat (dbacces -> pagbuf, ".pag");
    dbacces -> PagF = fopen (dbacces -> pagbuf, "wb+");
    if (!dbacces -> PagF) {
	dbacces -> PagF = fopen (dbacces -> pagbuf, "rb");
	dbacces -> DbRdOnly = 1;
    }

/*	Open dir file
 */
    strcpy (dbacces -> pagbuf, dbname);
    strcat (dbacces -> pagbuf, ".dir");
    dbacces -> DirF = fopen (dbacces -> pagbuf, "wb+");
    if (!dbacces -> DirF) {
	dbacces -> DirF = fopen (dbacces -> pagbuf, "rb");
	dbacces -> DbRdOnly = 1;
    }

/* 	Initialize database files and file buffers
 */
    dbacces -> dir_b = -1;
    dbacces -> pag_b = -1;

    return (dbacces);
}

int dbClose (DbAcces ** dbacces)
{
    char file_name[BUFSIZ];
    char   *cs;

    if ((*dbacces) -> dir_t) {
	fseek ((*dbacces) -> DirF, (*dbacces) -> dir_b * DBLKSIZ, 0);
	fwrite ((*dbacces) -> dirbuf, DBLKSIZ, 1, (*dbacces) -> DirF);
	(*dbacces) -> dir_t = 0;
    }
    if ((*dbacces) -> pag_t) {
	fseek ((*dbacces) -> PagF, (*dbacces) -> pag_b * PBLKSIZ, 0);
	fwrite ((*dbacces) -> pagbuf, PBLKSIZ, 1, (*dbacces) -> PagF);
	(*dbacces) -> pag_t = 0;
    }
    fclose ((*dbacces) -> DirF);
    fclose ((*dbacces) -> PagF);

    sprintf (file_name, "%s", (*dbacces) -> dbname);
    cs = file_name + strlen (file_name);
    strcpy (cs, ".pag");
    unlink (file_name);
    strcpy (cs, ".dir");
    unlink (file_name);
    free ((char *)(*dbacces));
    *dbacces = NULL;
    return (0);
}
