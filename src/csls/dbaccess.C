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

/*	Auxiliary routines used by the object interface
 */

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
static int forder (DbAcces *dbacces, datum *key);
static datum *firsthash (DbAcces *dbacces, long hash);
static void dbm_access (DbAcces *dbacces, long hash);
static int get_bit (DbAcces *dbacces);
static int set_bit (DbAcces *dbacces);
static void clrbuf (register char *cp, register int n);
static datum *makdatum (char buf[PBLKSIZ], int n);
static int cmpdatum (datum *d1, datum *d2);
static long hashinc (DbAcces *dbacces, long hash);
static long calchash (datum *item);
static void delitem (char buf[PBLKSIZ], int n);
static int additem (char buf[PBLKSIZ], datum *item);
static void chkblk (char buf[PBLKSIZ]);
#ifdef __cplusplus
  }
#endif

/*	Routines manipulating the dir and pag files
 *		(taken from DBM library)
 */

static int forder (DbAcces * dbacces, datum *key)
{
    long hash = calchash (key);

    for (dbacces -> hmask = 0; ;
	 dbacces -> hmask = (dbacces -> hmask << 1) + 1) {
	dbacces -> blkno = hash & dbacces -> hmask;
	dbacces -> bitno = dbacces -> blkno + dbacces -> hmask;
	if (get_bit (dbacces) == 0) break;
    }
    return (dbacces -> blkno);
}

datum *
dbfetch (DbAcces * dbacces, datum *key)
{
    register int i;
    datum *item;

    dbm_access (dbacces, calchash (key));
    for (i = 0;; i += 2) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item  -> dptr == NULL)
	    return (item);
	if (cmpdatum (key, item) == 0) {
	    item = makdatum (dbacces -> pagbuf, i + 1);
	    if (item  -> dptr == NULL)
		printf ("items not in pairs\n");
	    return (item);
	}
    }
}

int dbdelete (DbAcces * dbacces, datum *key)
{
    register int i;
    datum *item;

    if (dbacces -> DbRdOnly)
	return (-1);
    dbm_access (dbacces, calchash (key));
    for (i = 0;; i += 2) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item -> dptr == NULL)
	    return (-1);
	if (cmpdatum (key, item) == 0) {
	    delitem (dbacces -> pagbuf, i);
	    delitem (dbacces -> pagbuf, i);
	    dbacces -> pag_t = 1;
	    break;
	}
    }
    return (0);
}

int dbstore (DbAcces * dbacces, datum *key, datum *dat)
{
    register int i;
    datum *item;
    char    ovfbuf[PBLKSIZ];

    if (dbacces -> DbRdOnly)
	return (-1);
loop:
    dbm_access (dbacces, calchash (key));
    dbacces -> pag_t = 1;
    for (i = 0;; i += 2) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item -> dptr == NULL)
	    break;
	if (cmpdatum (key, item) == 0) {
	    delitem (dbacces -> pagbuf, i);
	    delitem (dbacces -> pagbuf, i);
	    break;
	}
    }
    i = additem (dbacces -> pagbuf, key);
    if (i < 0)
	goto split;
    if (additem (dbacces -> pagbuf, dat) < 0) {
	delitem (dbacces -> pagbuf, i);
	goto split;
    }
    return (0);

split:
    if (key -> dsize + dat -> dsize + 2 * sizeof (short) >= PBLKSIZ) {
	printf ("store(): entry too big\n");
	return (-2);
    }
    clrbuf (ovfbuf, PBLKSIZ);
    for (i = 0;;) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item -> dptr == NULL)
	    break;
	if (calchash (item) & (dbacces -> hmask + 1)) {
	    additem (ovfbuf, item);
	    delitem (dbacces -> pagbuf, i);
	    item = makdatum (dbacces -> pagbuf, i);
	    if (item -> dptr == NULL) {
		printf ("split not paired\n");
		break;
	    }
	    additem (ovfbuf, item);
	    delitem (dbacces -> pagbuf, i);
	    continue;
	}
	i += 2;
    }
    fseek (dbacces -> PagF, (off_t)(dbacces -> blkno * PBLKSIZ), 0);
    fwrite (dbacces -> pagbuf, PBLKSIZ, 1, dbacces -> PagF);
    fseek (dbacces -> PagF, (off_t)(dbacces -> blkno + dbacces -> hmask + 1) * PBLKSIZ, 0);
    fwrite (ovfbuf, PBLKSIZ, 1, dbacces -> PagF);
    dbacces -> pag_b = -1;
    dbacces -> pag_t = 0;
    set_bit (dbacces);
    goto loop;
}

datum *
firstkey (DbAcces * dbacces)
{
    return (firsthash (dbacces, 0L));
}

datum *
nextkey (DbAcces * dbacces, datum *key)
{
    register int i;
    datum *item;
    datum *bitem = 0;
    long    hash;

    hash = calchash (key);
    dbm_access (dbacces, hash);

    for (i = 0;; i += 2) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item -> dptr == NULL) break;

	if (cmpdatum (key, item) <= 0) continue;

	if (!bitem || cmpdatum (bitem, item) < 0) bitem = item;
    }
    if (bitem) return (bitem);

    hash = hashinc (dbacces, hash);
    if (hash == 0) return (item);

    return (firsthash (dbacces, hash));
}

static datum *
firsthash (DbAcces * dbacces, long hash)
{
    register int i;
    datum *item;
    datum *bitem;

loop:
    dbm_access (dbacces, hash);
    bitem = makdatum (dbacces -> pagbuf, 0);
    for (i = 2;; i += 2) {
	item = makdatum (dbacces -> pagbuf, i);
	if (item -> dptr == NULL)
	    break;
	if (cmpdatum (bitem, item) < 0)
	    bitem = item;
    }
    if (bitem -> dptr != NULL)
	return (bitem);
    hash = hashinc (dbacces, hash);
    if (hash == 0)
	return (item);
    goto loop;
}

static void dbm_access (DbAcces * dbacces, long hash)
{
    for (dbacces -> hmask = 0; ;
	 dbacces -> hmask = (dbacces -> hmask << 1) + 1) {
	dbacces -> blkno = hash & dbacces -> hmask;
	dbacces -> bitno = dbacces -> blkno + dbacces -> hmask;
	if (get_bit (dbacces) == 0) break;
    }
    if (dbacces -> blkno != dbacces -> pag_b) {
	if (dbacces -> pag_t && dbacces -> pag_b != -1) {
	    fseek (dbacces -> PagF, dbacces -> pag_b * PBLKSIZ, 0);
	    fwrite (dbacces -> pagbuf, PBLKSIZ, 1, dbacces -> PagF);
	    dbacces -> pag_t = 0;
	}
	clrbuf (dbacces -> pagbuf, PBLKSIZ);
	fseek (dbacces -> PagF, dbacces -> blkno * PBLKSIZ, 0);
	fread (dbacces -> pagbuf, PBLKSIZ, 1, dbacces -> PagF);
	chkblk (dbacces -> pagbuf);
	dbacces -> pag_b = (int)(dbacces -> blkno);
    }
}

static int get_bit (DbAcces * dbacces)
{
    long    bn;
    register long b, i, n;

    if (dbacces -> bitno > dbacces -> maxbno) return (0);
    n = dbacces -> bitno % BYTESIZ;
    bn = dbacces -> bitno / BYTESIZ;
    i = bn % DBLKSIZ;
    b = bn / DBLKSIZ;
    if (b != dbacces -> dir_b) {
	if (dbacces -> dir_t && dbacces -> dir_b != -1) {
	    fseek (dbacces -> DirF, (long) dbacces -> dir_b * DBLKSIZ, 0);
	    fwrite (dbacces -> dirbuf, DBLKSIZ, 1, dbacces -> DirF);
	    dbacces -> dir_t = 0;
	}
	clrbuf (dbacces -> dirbuf, DBLKSIZ);
	fseek (dbacces -> DirF, (long) b * DBLKSIZ, 0);
	fread (dbacces -> dirbuf, DBLKSIZ, 1, dbacces -> DirF);
	dbacces -> dir_b = (int)b;
    }
    if (dbacces -> dirbuf[i] & (1 << n)) return (1);
    return (0);
}

static int set_bit (DbAcces * dbacces)
{
    long    bn;
    register long i, n;

    if (dbacces -> DbRdOnly) return (-1);
    if (dbacces -> bitno > dbacces -> maxbno) {
	dbacces -> maxbno = dbacces -> bitno;
	get_bit (dbacces);
    }
    n = dbacces -> bitno % BYTESIZ;
    bn = dbacces -> bitno / BYTESIZ;
    i = bn % DBLKSIZ;
    dbacces -> dirbuf[i] |= 1 << n;
    dbacces -> dir_t = 1;
    return (0);
}

static void clrbuf (register char *cp, register int n)
{
    do
	*cp++ = 0;
    while (--n);
}

static datum *
makdatum (char buf[PBLKSIZ], int n)
{
    register short *sp;
    register int t;
    static datum item;

    sp = (short *) buf;
    if (n < 0 || n >= sp[0]) goto null;
    t = PBLKSIZ;
    if (n > 0) t = sp[n + 1 - 1];
    item.dptr = buf + sp[n + 1];
    item.dsize = t - sp[n + 1];
    return (&item);

null:
    item.dptr = NULL;
    item.dsize = 0;
    return (&item);
}

static int cmpdatum (datum *d1, datum *d2)
{
    register int n;
    register char  *p1, *p2;

    n = d1 -> dsize;
    if (n != d2 -> dsize) return (n - d2 -> dsize);
    if (n == 0) return (0);
    p1 = d1 -> dptr;
    p2 = d2 -> dptr;
    do
	if (*p1++ != *p2++) return (*--p1 - *--p2);
    while (--n);
    return (0);
}

int     hitab[16] = {
    61, 57, 53, 49, 45, 41, 37, 33,
    29, 25, 21, 17, 13, 9, 5, 1,
};

long    hltab[64] = {
    06100151277L, 06106161736L, 06452611562L, 05001724107L,
    02614772546L, 04120731531L, 04665262210L, 07347467531L,
    06735253126L, 06042345173L, 03072226605L, 01464164730L,
    03247435524L, 07652510057L, 01546775256L, 05714532133L,
    06173260402L, 07517101630L, 02431460343L, 01743245566L,
    00261675137L, 02433103631L, 03421772437L, 04447707466L,
    04435620103L, 03757017115L, 03641531772L, 06767633246L,
    02673230344L, 00260612216L, 04133454451L, 00615531516L,
    06137717526L, 02574116560L, 02304023373L, 07061702261L,
    05153031405L, 05322056705L, 07401116734L, 06552375715L,
    06165233473L, 05311063631L, 01212221723L, 01052267235L,
    06000615237L, 01075222665L, 06330216006L, 04402355630L,
    01451177262L, 02000133436L, 06025467062L, 07121076461L,
    03123433522L, 01010635225L, 01716177066L, 05161746527L,
    01736635071L, 06243505026L, 03637211610L, 01756474365L,
    04723077174L, 03642763134L, 05750130273L, 03655541561L,
};

static long
hashinc (DbAcces * dbacces, long hash)
{
    long    bit;

    hash &= dbacces -> hmask;
    bit = dbacces -> hmask + 1;
    for (;;) {
	bit >>= 1;
	if (bit == 0) return (0L);
	if ((hash & bit) == 0) return (hash | bit);
	hash &= ~bit;
    }
}

static long
calchash (datum *item)
{
    register int i, j, f;
    long    hashl;
    int     hashi;

    hashl = 0;
    hashi = 0;
    for (i = 0; i < item -> dsize; i++) {
	f = item -> dptr[i];
	for (j = 0; j < BYTESIZ; j += 4) {
	    hashi += hitab[f & 017];
	    hashl += hltab[hashi & 63];
	    f >>= 4;
	}
    }
    return (hashl);
}

static void delitem (char buf[PBLKSIZ], int n)
{
    register short *sp;
    register int i1, i2, i3;

    sp = (short *) buf;
    if (n < 0 || n >= sp[0]) goto bad;
    i1 = sp[n + 1];
    i2 = PBLKSIZ;
    if (n > 0) i2 = sp[n + 1 - 1];
    i3 = sp[sp[0] + 1 - 1];
    if (i2 > i1)
	while (i1 > i3) {
	    i1--;
	    i2--;
	    buf[i2] = buf[i1];
	    buf[i1] = 0;
	}
    i2 -= i1;
    for (i1 = n + 1; i1 < sp[0]; i1++)
	sp[i1 + 1 - 1] = sp[i1 + 1] + i2;
    sp[0]--;
    sp[sp[0] + 1] = 0;
    return;

bad:
    printf ("bad delitem\n");
    abort ();
}

static int additem (char buf[PBLKSIZ], datum *item)
{
    register short *sp;
    register int i1, i2;

    sp = (short *) buf;
    i1 = PBLKSIZ;
    if (sp[0] > 0) i1 = sp[sp[0] + 1 - 1];
    i1 -= item -> dsize;
    i2 = (sp[0] + 2) * sizeof (short);
    if (i1 <= i2)
	return (-1);
    sp[sp[0] + 1] = i1;
    for (i2 = 0; i2 < item -> dsize; i2++) {
	buf[i1] = item -> dptr[i2];
	i1++;
    }
    sp[0]++;
    return (sp[0] - 1);
}

static void chkblk (char buf[PBLKSIZ])
{
    register short *sp;
    register int t, i;

    sp = (short *) buf;
    t = PBLKSIZ;
    for (i = 0; i < sp[0]; i++) {
	if (sp[i + 1] > t)
	    goto bad;
	t = sp[i + 1];
    }
    if (t < (int)((sp[0] + 1) * sizeof (short)))
	goto bad;
    return;

bad:
    printf ("bad block\n");
    abort ();
    clrbuf (buf, PBLKSIZ);
}
