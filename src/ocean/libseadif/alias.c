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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sea_decl.h"
#include "src/ocean/libseadif/sysdep.h"

#define ALIASTABSIZE 5009	  /* room for 5009 aliases (must be a prime) */

typedef union _SDFOBJECT
{
    LIBTABPTR lib;
    FUNTABPTR fun;
    CIRTABPTR cir;
    LAYTABPTR lay;
}
SDFOBJECT;

#define OBJECT_LIB ALIASLIB	  /* value for ALIASTAB.objecttype */
#define OBJECT_FUN ALIASFUN	  /* value for ALIASTAB.objecttype */
#define OBJECT_CIR ALIASCIR	  /* value for ALIASTAB.objecttype */
#define OBJECT_LAY ALIASLAY	  /* value for ALIASTAB.objecttype */

typedef struct _ALIASTAB
{
    STRING     alias;		  /* the alias, a canonic string */
    SDFOBJECT  object;		  /* ptr to the aliased object info */
    int        objecttype;	  /* selects a member of the SDFOBJECT union */
}
ALIASTAB;

STRING thislibnam, thisfunnam, thiscirnam, thislaynam;

extern STRING sdfdeletedfromhashtable;

PRIVATE void addtoaliastab (STRING alias, SDFOBJECT object, int objecttype);
PRIVATE void initaliastab (void);

static ALIASTAB *aliastab = NULL;
static int aliastabsize = 0;

int sdfmakelibalias (STRING alias, STRING lnam)
{
    SDFOBJECT object;
    if (!sdfexistslib (lnam)) return 0;
    thislibtab->alias = cs (alias);	  /* registrate the alias with LIBTAB */
    object.lib = thislibtab;
    addtoaliastab (alias, object, OBJECT_LIB); /* add to the alias hash table */
    return TRUE;
}

int sdfmakefunalias (STRING alias, STRING fnam, STRING lnam)
{
    SDFOBJECT object;
    if (!sdfexistsfun (fnam, lnam)) return 0;
    thisfuntab->alias = cs (alias);	  /* registrate the alias with FUNTAB */
    object.fun = thisfuntab;
    addtoaliastab (alias, object, OBJECT_FUN); /* add to the alias hash table */
    return TRUE;
}

int sdfmakeciralias (STRING alias, STRING cnam, STRING fnam, STRING lnam)
{
    SDFOBJECT object;
    if (!sdfexistscir (cnam, fnam, lnam)) return 0;
    thiscirtab->alias = cs (alias);	  /* registrate the alias with CIRTAB */
    object.cir = thiscirtab;
    addtoaliastab (alias, object, OBJECT_CIR); /* add to the alias hash table */
    return TRUE;
}

int sdfmakelayalias (STRING alias, STRING lnam, STRING cnam, STRING fnam, STRING bnam)
{
    SDFOBJECT object;
    if (!sdfexistslay (lnam, cnam, fnam, bnam)) return 0;
    thislaytab->alias = cs (alias);	  /* registrate the alias with LAYTAB */
    object.lay = thislaytab;
    addtoaliastab (alias, object, OBJECT_LAY); /* add to the alias hash table */
    return TRUE;
}

/* This function adds the alias to the aliastab hash table so that we can
 * later map the alias back onto its canonic seadif name by looking up the
 * alias in the aliastab.
 */
PRIVATE void addtoaliastab (STRING alias, SDFOBJECT object, int objecttype)
{
    ALIASTAB *point, *entry, *toptab;

    if (!aliastab) initaliastab ();

    toptab = aliastab + aliastabsize;
    entry = aliastab + sdfhashstring (alias, aliastabsize);

    for (point = entry; point < toptab; ++point)
	if (!point->alias || point->alias == sdfdeletedfromhashtable) break;

    if (point >= toptab) /* go searching the lower part */
    for (point = aliastab; point < entry; ++point)
	if (!point->alias || point->alias == sdfdeletedfromhashtable) break;

    if (point->alias && point->alias != sdfdeletedfromhashtable)
	sdfreport (Fatal, "addtoaliastab: too many aliases");
    point->alias = cs (alias);
    point->object = object;
    point->objecttype = objecttype;
}

PRIVATE void initaliastab ()
{
    ALIASTAB *point, *toptab;

    aliastabsize = ALIASTABSIZE;
    if (!(aliastab = (ALIASTAB *)malloc ((aliastabsize+1)*sizeof(ALIASTAB))))
	sdfreport (Fatal, "initaliastab: cannot get enough memory");

    /* initialization */
    for (point = aliastab, toptab = aliastab + aliastabsize; point < toptab; ++point)
    {
	point->alias = NULL;
	point->object.lib = NULL;
	point->objecttype = 0;
    }
    if (!sdfdeletedfromhashtable)
	sdfdeletedfromhashtable = cs ("#!&^%$*niemand*kiest*ooit*een*naam*als*deze*&^%$#@**&");
}

STRING sdflibalias (STRING lnam)
{
    if (!sdfexistslib (lnam)) return NULL;
    return thislibtab->alias;
}

STRING sdffunalias (STRING fnam, STRING lnam)
{
    if (!sdfexistsfun (fnam, lnam)) return NULL;
    return thisfuntab->alias;
}

STRING sdfciralias (STRING cnam, STRING fnam, STRING lnam)
{
    if (!sdfexistscir (cnam, fnam, lnam)) return NULL;
    return thiscirtab->alias;
}

STRING sdflayalias (STRING lnam, STRING cnam, STRING fnam, STRING bnam)
{
    if (!sdfexistslay (lnam, cnam, fnam, bnam)) return NULL;
    return thislaytab->alias;
}

/* This function takes an alias (a STRING) and an objecttype.  Objecttype can
 * be any of ALIASLIB, ALIASFUN, ALIASCIR, ALIASLAY. The function return TRUE
 * and sets the global STRINGs thislibnam, thisfunnam, thiscirnam and thislaynam
 * to the canonic seadif name of the aliased cell. If the alias is not known,
 * then the function returns 0.
 */
int sdfaliastoseadif (STRING alias, int objecttype)
{
    ALIASTAB *point, *entry, *toptab;
    STRING   n;

    thislibnam = thisfunnam = thiscirnam = thislaynam = NULL;
    if (!aliastabsize) return 0;

    toptab = aliastab + aliastabsize;
    entry = aliastab + sdfhashstring (alias, aliastabsize);

    for (point = entry; (n = point->alias) && point < toptab; ++point)
	if (n == alias && point->objecttype == objecttype) break;
    if (point >= toptab)
    {
	for (point = aliastab; (n = point->alias) && point < toptab; ++point)
	    if (n == alias && point->objecttype == objecttype) break;
	if (point >= entry) return 0;
    }
    if (!n) return 0;

    if (objecttype == ALIASLIB) {
	thislibnam = point->object.lib->name;
    }
    else if (objecttype == ALIASFUN) {
	thislibnam = point->object.fun->library->name;
	thisfunnam = point->object.fun->name;
    }
    else if (objecttype == ALIASCIR) {
	thislibnam = point->object.cir->function->library->name;
	thisfunnam = point->object.cir->function->name;
	thiscirnam = point->object.cir->name;
    }
    else if (objecttype == ALIASLAY) {
	thislibnam = point->object.lay->circuit->function->library->name;
	thisfunnam = point->object.lay->circuit->function->name;
	thiscirnam = point->object.lay->circuit->name;
	thislaynam = point->object.lay->name;
    }
    return TRUE;
}
