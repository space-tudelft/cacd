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

#ifndef __NAMELIST_H
#define __NAMELIST_H

typedef struct _NAMELIST
{
  char *name;
  struct _NAMELIST *next;
}
NAMELIST, NAMELIST_TYPE, *NAMELISTPTR;

typedef struct _NAMELISTLIST
{
  char *namelistname;
  NAMELISTPTR namelist;
  struct _NAMELISTLIST *next;
}
NAMELISTLIST, NAMELISTLIST_TYPE, *NAMELISTLISTPTR;

#define NewNamelist(p)     ((p) = (NAMELISTPTR)mnew (sizeof(NAMELIST_TYPE)))
#define NewNamelistlist(p) ((p) = (NAMELISTLISTPTR)mnew (sizeof(NAMELISTLIST_TYPE)))
#define FreeNamelist(p) forgetstring ((p)->name); mfree ((char**)(p), sizeof(NAMELIST_TYPE))

typedef struct _NUM2LIST
{
  long num1, num2;
  struct _NUM2LIST *next;
}
NUM2LIST, NUM2LIST_TYPE, *NUM2LISTPTR;

typedef struct _NUM2LISTLIST
{
  char *num2listname;
  NUM2LISTPTR num2list;
  struct _NUM2LISTLIST *next;
}
NUM2LISTLIST, NUM2LISTLIST_TYPE, *NUM2LISTLISTPTR;

#define NewNum2list(p)     ((p) = (NUM2LISTPTR)mnew (sizeof(NUM2LIST_TYPE)))
#define NewNum2listlist(p) ((p) = (NUM2LISTLISTPTR)mnew (sizeof(NUM2LISTLIST_TYPE)))
#define FreeNum2list(p) mfree ((char**)(p), sizeof(NUM2LIST_TYPE))

#endif
