/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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
 * String Manager avoids need for calls to strcmp() in programs. The
 * function canonicstring(str) copies str to a private heap if there
 * is not already a copy on this heap. It then returns a pointer to the
 * copy on the heap. As a consequence, programs that store all there
 * strings by means of canonicstring() can be sure that two or more strings
 * are equal when they all point to the same address on the heap. A call to
 * forgetstring(str) will remove str from the heap only if the total number
 * of calls to forgetstring() equals the number of calls to canonicstring()
 * for that string. Thus the program can be sure that its other pointers
 * to the same str on the heap remain valid. Function forgetstring returns an
 * integer which can be one of three values: 2 if str is unknown or not in its
 * canonic form, 1 if the string is too short to allow efficient insertion
 * in the manager's free list, and 0 if the call was successfully completed.
 */

#include "src/ocean/libseadif/sm.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/libseadif/sea_decl.h"

#define ALIGNBITS 3 /* address alignment for structure SMINFO */

int smdebug = 0;

PRIVATE SMINFOPTR  *smtable = NULL;
PRIVATE SMINFOPTR   smalignedblocks;
PRIVATE int         smtablesiz;
PRIVATE char       *smstrings = NULL;
PRIVATE long        smstringsleft;
PRIVATE long        smstringsincr;
PRIVATE char      **smfreelist[SMMAXSTRLEN+1];
PRIVATE char        sminstring[SMMAXSTRLEN+1];

PRIVATE SMINFOPTR smnewinfo (void);

/* The function sminit() sets up the hash tables for the String Manager.
 */
PRIVATE void sminit (int siz)
{
    int i;

    if (smdebug) fprintf (stderr, "\n### sminit(%d)\n", siz);

    if (siz < SMMINSIZ) siz = SMMINSIZ; /* check for a reasonable siz */
    else if (siz > SMMAXSIZ) siz = SMMAXSIZ;

    smtable = (SMINFOPTR *)calloc ((unsigned)siz, sizeof(SMINFOPTR));
    if (!smtable) sdfreport (Fatal, "sminit: cannot get memory for smtable");

    for (i = 0; i < siz; ++i) smtable[i] = (SMINFOPTR)NULL; /* initialize the hash table */

    for (i = 0; i <= SMMAXSTRLEN; ++i) smfreelist[i] = (char**)NULL; /* init the free list */

    smalignedblocks = (SMINFOPTR)NULL; /* free list for aligned blocks */

    smstringsleft = 0; /* anounce that no string space is available */
    smtablesiz = siz;
    smstringsincr = SMMAXSTRLEN + siz * (SMMEANSTRLEN + sizeof(SMINFO) + (ALIGNBITS+1)/2);
}

/* The function smaddtostringtable() appends a string to a private heap.
 * The new string is truncated to SMMAXSTRLEN characters. If the heap
 * appears to be too small, malloc is called and the string is stored
 * in the thus acquired space. The function returns a char pointer to
 * the point in the heap where the string was copied to.
 */
PRIVATE char *smaddtostringtable (char *str)
{
    char *stringsfront, *instring, *stringstart, **freestring;
    int  slen, c;
    long stringsleft;

    instring = str;
    /* measure string length */
    for (slen = 1; *str && slen < SMMAXSTRLEN; ++str, ++slen) ;

    if (smfreelist[slen]) /* We already have a block of this length! */
    {
	freestring = (char **)(smfreelist[slen]);
	smfreelist[slen] = (char **)*freestring; /* unlink from freelist */
	stringstart = (char *)freestring;
	str = instring;
	for (slen = 1; *str && slen < SMMAXSTRLEN; ++str, ++slen)
	    *stringstart++ = *str; /* copy str to block */
	*stringstart = 0; /* append 0 byte even if string is too long */
	return ((char *)(freestring));
    }

    /* In fs() we construct a freelist by putting pointers in the "strings".
     * As a consequence, strings must be alligned to hold pointers:
     */
    while ((long)smstrings & ALIGNBITS) ++smstrings, --smstringsleft;

    stringstart = stringsfront = smstrings;
    stringsleft = smstringsleft;
    str = instring;

    for (c = 1, slen = 0; c && stringsleft > 0 && slen < SMMAXSTRLEN-1; ++slen, --stringsleft)
	c = *stringsfront++ = *str++; /* copy onto the heap */

    if (!c) { /* everything ok */
	smstrings = stringsfront;
	smstringsleft = stringsleft;
	return (stringstart);
    }

    if (stringsleft <= 0) { /* heap too small */
	smstrings = (char *)malloc ((unsigned)smstringsincr);
	if (!smstrings) sdfreport (Fatal, "smaddtostringtable: cannot get memory for smstrings");
	smstringsleft = smstringsincr;
	return (smaddtostringtable (instring)); /* recursive call: try again */
    }

    if (slen >= SMMAXSTRLEN-1) { /* string too long */
	*(stringsfront-1) = 0; /* behave as if last char was a 0 byte */
	smstrings = stringsfront;
	smstringsleft = stringsleft;
	return (stringstart);
    }

    /* this point never reached... */
    sdfreport (Fatal, "smaddtostringtable: internal error 64357");
    return (NULL);
}

/* Function smhashstring() converts a string to
 * a pseudo random integer in the range [0..smtablesiz].
 * It also copies the string to the global array sminstring[].
 */
PRIVATE int smhashstring (char *str)
{
    int i, product, sum, c, slen;
    char *instring;

    instring = sminstring;
    slen = 0; product = 1; sum = 0;

    for (; (c = *str) && slen < SMMAXSTRLEN-1; ++str, ++slen) {
	*instring++ = c;
	c += slen;
	product = (product * c) % smtablesiz;
	sum += c;
    }
    *instring = 0;

    i = (product + sum + (sum<<3) + (sum<<5) + (sum<<11)) % smtablesiz;
    return (i < 0 ? -i : i);
}

/* The public function canonicstring() searches in the hash table for the
 * specified string and returns a pointer to a copy of the string in the
 * private heap. If it cannot find the string it first calls the function
 * smaddstringtotable() to create a copy on the heap.
 */
char *cs (char *str)
{
    SMINFOPTR stringlist, newinfoblk;
    int tablentry;

    if (!str) return NULL;

    if (smdebug) fprintf (stderr, "\n### canonicstring(%s) ", str);

    if (!smtable) sminit (SMTABLESIZ); /* first time called */

    tablentry = smhashstring (str);
    stringlist = smtable[tablentry];

    for (; stringlist; stringlist = stringlist->next)
	if (strcmp (sminstring, stringlist->str) == 0) break;

    if (!stringlist) { /* str not present in hash table, append */
	newinfoblk = smnewinfo ();
	newinfoblk->str = smaddtostringtable (sminstring);
	newinfoblk->linkcnt = 1;
	newinfoblk->next = smtable[tablentry];
	smtable[tablentry] = newinfoblk; /* link in front of the string list */
	stringlist = newinfoblk;
    }
    else
	++stringlist->linkcnt; /* increment the link counter */

    if (smdebug) fprintf (stderr, " [cnt=%ld] ###", stringlist->linkcnt);

    return (stringlist->str);
}

PRIVATE SMINFOPTR smnewinfo ()
{
    SMINFOPTR infoblk;

    if (smalignedblocks) { /* can get an info block from freelist */
	infoblk = smalignedblocks;
	smalignedblocks = smalignedblocks->next;
	return (infoblk);
    }

    while ((long)smstrings & ALIGNBITS) ++smstrings, --smstringsleft; /* take care for pointer alignment */

    if (smstringsleft >= sizeof(SMINFO)) { /* get info block from the heap */
	infoblk = (SMINFOPTR)smstrings;
	smstrings     += sizeof(SMINFO);
	smstringsleft -= sizeof(SMINFO);
	return (infoblk);
    }

    /* heap too small, go fetch some new memory */
    smstrings = (char *)malloc ((unsigned)smstringsincr);
    if (!smstrings) sdfreport (Fatal, "smnewinfo: cannot get memory for smstrings");
    smstringsleft = smstringsincr;
    return (smnewinfo ()); /* recursive call: try again */
}

/* Function forgetstring() removes the specified string from the
 * hash tabel and the heap. If the string is too short, it can not
 * append its occupied space to the free list, so it chooses not to
 * forget about the string. In this case the function still returns
 * the value TRUE to the caller. If stringtoforget was successfully
 * removed the function also returns TRUE. If the string is unknown
 * to the heap manager the function returns 0.
 */
int fs (char *stringtoforget)
{
    char *str, **freeblk;
    int slen, tablentry;
    SMINFOPTR stringlist, oldstringlist;

    if (smdebug) fprintf (stderr, "\n### forgetstring(%s) ", stringtoforget);

    /* measure string length */
    str = stringtoforget;
    for (slen = 1; *str && slen < SMMAXSTRLEN; ++str, ++slen) ;

    if (*str) return (0); /* string too long, cannot be canonic */

    /* Test to see that stringtoforget is realy known to the String Manager.
     * Note that this code is almost copied from the function canonicstring().
     */
    if (!smtable) return (0); /* no table, never called before */

    tablentry = smhashstring (stringtoforget);
    oldstringlist = stringlist = smtable[tablentry];

    for (; stringlist; stringlist = (oldstringlist = stringlist)->next)
	if (stringtoforget == stringlist->str) break;

    if (!stringlist) return (0); /* str not present in hash table, nothing to forget */

    if (smdebug) fprintf (stderr, "[cnt=%ld] ###", stringlist->linkcnt);

    if (slen < sizeof(char **)) return (0); /* we cannot link this one in the free list */

    if (--(stringlist->linkcnt) > 0) return (0); /* there are still more refs; don't remove! */

    if (oldstringlist == stringlist) /* found block at beginning of infoblk list */
	smtable[tablentry] = stringlist->next;
    else
	oldstringlist->next = stringlist->next;

    freeblk = (char **)stringtoforget;
   *freeblk = (char *)smfreelist[slen];
    smfreelist[slen] = freeblk; /* link string in front of the free list */
    stringlist->next = smalignedblocks;
    smalignedblocks = stringlist; /* link in front of the aligned free list */
    return (0);
}
