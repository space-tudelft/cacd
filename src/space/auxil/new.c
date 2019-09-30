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

#include <ctype.h>
#include "src/space/auxil/auxil.h"

static FILE * fpmap;
static FILE * fpdat;

#define MEMPROF_NOINIT	0
#define MEMPROF_OFF	1
#define MEMPROF_ON	2

static int Status = MEMPROF_NOINIT;

struct map {
    char * file;
    int  line;
    int ident;
    struct map * next;
};

union dat {
    struct {
	int ident;		/* ident should be efficient -> int */
	unsigned size;
    } d;
    int x; /* alignment */
};

#define MAPSIZE 1000

static struct map * maptab[MAPSIZE];

/* local operations */
Private bool_t filenameOK (const char *s);
Private void memprofInit (void);
Private void noMoreCore (size_t size, const char *file, int line);
Private int memprof (size_t size, const char *file, int line);
Private void memproffree (size_t size, int ident);
Private struct map *lookup (struct map *item);
Private struct map *install (struct map *item);
Private int hash (char *file, int line);

/*
 * A malloc/free wrapper with memory profiling
 * and leak finding capabilities.
 * To be called via the macros NEW and family in malloc.h
 *
 * I have borrowed some ideas from the net,
 * for example, from mnemosyne by Marcus J. Ranum.
 *
 * Profiling can be enabled by setting the environment
 * variable MEMPROF. The value of this variable
 * will be the basename of the profiling data files.
 *
 * If profiling is enabled, produces raw profiling data
 * in a pair of two files named <MEMPROF.map> and <MEMPROF.dat>.
 * This report can be analyzed with msum (see msum.c).
 *
 * <size> is the amount of memory to be allocated.
 * <file> and <line> are the file name and linenumber
 * of this malloc call, see the definition of the macro <NEW>
 * in malloc.h
 *
 * Principle:
 * Prepend the malloc'ed block with info
 * containing the number of bytes + mapnumber.
 * mapnumber is a hashed identity of source file and
 * linenumber where the malloc was done.
 * This data is saved in the .map and .dat files.
 * These files can be analysed with msum.
 *
 * Will <say> a diagnostic and execute <die> if no more core.
 *
 */
static unsigned long max_size;
static unsigned long cur_size;

void * malloc_p (unsigned size, const char *file, int line) /* malloc wrapper */
{
    void *n;

    if (Status == MEMPROF_NOINIT) memprofInit ();
    if (Status == MEMPROF_OFF) {
	n = malloc (size);
	if (!n) noMoreCore (size, file, line);
	ASSERT (n);
    }
    else {
	union dat *p;
	p = (union dat *) malloc (sizeof (*p) + size);
	if (!p) noMoreCore (size, file, line);
	ASSERT (p);
	p -> d.size = size;
	p -> d.ident = memprof (size, file, line);
	n = (void *) (p+1);
    }
    /* Always add 4 extra bytes. The smallest amount is 16 bytes.
     * The size is always a multiple of 8 bytes (alignment).
     */
    size = (size <= 12) ? 16 : (size + 11) / 8 * 8;
    if ((cur_size += size) > max_size) max_size = cur_size;
    return (n);
}

void * realloc_p (void *o, unsigned size, unsigned oldsize, const char *file, int line) /* realloc wrapper */
{
    void *n;

    if (!o) return (malloc_p (size, file, line));

    if (Status == MEMPROF_NOINIT) memprofInit ();
    if (Status == MEMPROF_OFF) {
	n = realloc (o, size);
	if (!n) noMoreCore (size, file, line);
	ASSERT (n);
    }
    else {
	union dat *p = (union dat *) o - 1;
	memproffree (p -> d.size, p -> d.ident);
	p = (union dat *) realloc ((void *) p, sizeof (*p) + size);
	if (!p) noMoreCore (size, file, line);
	ASSERT (p);
	p -> d.size = size;
	p -> d.ident = memprof (size, file, line);
	n = (void *) (p+1);
    }
    oldsize = (oldsize <= 12) ? 16 : (oldsize + 11) / 8 * 8;
    cur_size -= oldsize;
    size = (size <= 12) ? 16 : (size + 11) / 8 * 8;
    if ((cur_size += size) > max_size) max_size = cur_size;
    return (n);
}

void free_p (void *d, unsigned size, const char *file, int line) /* free wrapper */
{
    if (!d) {
        if (file)
            say ("attempt to free a NULL pointer, %s: %d", file, line);
        else
            say ("attempt to free a NULL pointer");
        return;
    }

    if (Status == MEMPROF_NOINIT) memprofInit ();
    if (Status == MEMPROF_OFF) {
	free (d);
    }
    else {
	union dat *p = (union dat *) d - 1;
	memproffree (p -> d.size, p -> d.ident);
	free (p);
    }
    size = (size <= 12) ? 16 : (size + 11) / 8 * 8;
    cur_size -= size;
}

void memprofTurnOff ()
{
    Status = MEMPROF_OFF;
}

double allocatedMbyte ()
{
    return ((double)max_size/1024./1024.);
}

/*
 * complain and die
 */
Private void noMoreCore (size_t size, const char *file, int line)
{
    say ("No more core.");
    say ("\tAlready allocated %.3f Mbyte, cannot get %ld byte more.", allocatedMbyte (), (long) size);
    if (Status == MEMPROF_ON) say ("\t(file %s, line %d)", file, line);
    die ();
}

Private void memprofInit ()
{
    char * s;
    ASSERT (Status == MEMPROF_NOINIT);

    if ((s = getenv ("MEMPROF"))) {
	if (!filenameOK (s)) {
	    say ("<%s> is not a good basename for memprof datafiles", s);
	    say ("\tusing \"malloc\" instead");
	    s = "malloc";
	}
	Status = MEMPROF_ON;
	fpmap = cfopen (mprintf ("%s.map", s), "w");
	fpdat = cfopen (mprintf ("%s.dat", s), "w");
    }

    /* turn of profiling if data files not OK */
    if (!fpmap || !fpdat) Status = MEMPROF_OFF;

    ASSERT (Status != MEMPROF_NOINIT);
}

Private bool_t filenameOK (const char *s)
{
    if (!s || !*s) return (FALSE);
    while (*s) {
	if (!isgraph ((int)*s)) return (FALSE);
	s++;
    }
    return (TRUE);
}

Private int memprof (size_t size, const char *file, int line)
{
    static int ident = 0;
    struct map item;
    struct map * found;
    union dat dat;

    ASSERT (Status == MEMPROF_ON);

    item.file  = (char*)(file ? file : "<unknown>");
    item.line  = line;
    if (!(found = lookup (&item))) {
	item.ident = ident++;
	fprintf (fpmap, "%s %d\n", item.file, item.line);
	fflush (fpmap);
	found = install (&item);
    }
    if (found) {
	dat.d.ident = found -> ident;
	dat.d.size   = size;
	fwrite ((char *) &dat.d, sizeof (dat.d), 1, fpdat);
    }

    Debug (fprintf (stderr, "memprof %d %d\n", (int) size, found -> ident));

    return (found -> ident);
}

Private void memproffree (size_t size, int ident)
{
    union dat dat;
    Debug (fprintf (stderr, "free %d %d\n", (int) size, ident));
    dat.d.ident = ident;
    dat.d.size  = 0 - size;
    fwrite ((char *) &dat.d, sizeof (dat.d), 1, fpdat);
}

Private struct map * lookup (struct map *item)
{
    struct map * m;
    for (m = maptab[hash (item -> file, item -> line)]; m; m = m -> next) {
	if (item -> line == m -> line
	&& strsame (item -> file, m -> file)) return (m); /* found */
    }
    return (NULL); /* not found */
}

Private struct map * install (struct map *item)
{
    int hashval;
    struct map * mp;

    mp = (struct map *) malloc (sizeof (struct map));
    if (!mp) return (NULL);

    mp -> file = (char *) malloc ((unsigned) strlen (item -> file) + 1);
    if (!mp -> file) return (NULL);

    mp -> ident = item -> ident;
    strcpy (mp -> file, item -> file);
    mp -> line = item -> line;

    hashval = hash (mp -> file, mp -> line);
    mp -> next = maptab[hashval];
    maptab[hashval] = mp;

    return (mp);
}

/*
 * See the "Dragon" book by Aho, Sethi and Ullman
 * I don't know where I picked this up, but
 * P.J. Weinberger at Bell Labs seemed to deserve some credit.
 */
Private int hash (char *file, int line)
{
    unsigned long g, h = 0;

    while (*file) {
	h = (h << 4) + *file++;
	if ((g = (h & 0xf0000000L))) {
	    h ^= (g >> 24);
	    h ^= g;
	}
    }

    h = (h + line) % MAPSIZE;
    return ((int) h);
}

#ifdef DRIVER

extern char *argv0;

/*
 * test driver and example
 */
int main (int argc, char *argv[])
{
    int i, old;
    struct x {
	int i;
	char c;
	double d;
    } * p;
    static struct x y = {
	1,
	'a',
	123.456
    };
    struct x *q;

    /* set the progname for error messages */
    argv0 = "testnew";

    /* allocate and resize something */
    p = NEW (struct x, 100);
    p = NEW (struct x, 100);
    p = NEW (struct x, 1);
    q = NEW (struct x, 2);
    p = RESIZE (p, struct x, 2000, 1);
    (void) strsave ("this is a string");

    /* rudimentary check of malloc consistency */
    memcpy (p, &y, sizeof (struct x));
    printf ("next two lines should be equal\n");
    printf ("i: %d, c: %c, d: %g\n", p -> i, p -> c, p -> d);
    printf ("i: 1, c: a, d: 123.456\n");

    if (p -> i != y.i || p -> c != y.c || p -> d != y.d) {
	printf ("NOT OK i: %d, c: %c, d: %g\n", p -> i, p -> c, p -> d);
    }
    else {
	printf ("OK, you can run msum.\n");
    }

    /* Check the limits of allocation */
    old = 0;
    for (q = NULL, i = 1; i > 0; i = 2 * i) {
	q = RESIZE (q, struct x, i, old);
	old = i;
    }

    /* because the above loop will force a no-more-core situation,
     * the next will leak.
     */
    DISPOSE (p, sizeof(struct x) * 2000);
    return (0);
}

#endif /* DRIVER */
