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

#include <sys/types.h>
#include <unistd.h>

static int counter = 0;

dictionary::dictionary() {
    char  filename[64];
    char  templ[256];

    sprintf (filename, "x%d.%d", (int)getpid (), counter);
    sprintf (templ, "/usr/tmp/%s", filename);
    db = dbOpen (templ);
    if (db -> DbRdOnly) {
	sprintf (templ, "/tmp/%s", filename);
	db = dbOpen (templ);
    }
#if defined(__CYGWIN__) || defined(WIN32)
    if (db -> DbRdOnly) {
	sprintf (templ, "c:/windows/temp/%s", filename);
	db = dbOpen (templ);
    }
#endif
    if (db -> DbRdOnly) {
	sprintf (templ, "%s", filename);
	db = dbOpen (templ);
    }
    counter++;

    key.dptr = NULL;
    key.dsize = 0;
    data.dptr = NULL;
    data.dsize = 0;
#ifdef DMEM
    dict_nbyte += sizeof(class dictionary);
    dict_maxnbyte = dict_nbyte > dict_maxnbyte ? dict_nbyte:dict_maxnbyte;
#endif
}

dictionary::~dictionary() {
#ifdef DMEM
    dict_nbyte -= sizeof(class dictionary);
#endif
    if (db) dbClose (&db);
}

void dictionary::store(char *item_key, char *item_data) {
    key.dptr = item_key;
    key.dsize= strlen(item_key) + 1;
    data.dptr = (char *) &item_data;
    data.dsize= sizeof(char *);
    dbstore(db, &key, &data);
}

char *dictionary::fetch(char *item_key) {
    int *n;
    datum *helpdata;

    key.dptr = item_key;
    key.dsize= strlen(item_key) + 1;
    helpdata = dbfetch(db, &key);
    if (helpdata -> dptr == NULL) return (NULL);
    bmove ((char *)&n, (char *)helpdata -> dptr, helpdata -> dsize);
    return (char *) n;
}

void dictionary::print() {
    int *n;
    datum *helpkey;
    datum *helpdata;

    for (helpkey = firstkey (db);
	 helpkey -> dptr != NULL;
	 helpkey = nextkey (db, helpkey))
    {
	helpdata = dbfetch (db, helpkey);
	bmove ((char *)&n, (char *)helpdata -> dptr, helpdata -> dsize);
	fprintf (stderr, "key: %s, data: %p\n", helpkey -> dptr, n);
    }
}
