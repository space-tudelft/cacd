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

#include "src/ocean/libseadif/sea_decl.h"

#define NULL 0
#define LIBHASHTABSIZ 307   /* need only few space for libraries, but we */
#define HASHTABSIZ   5011   /* have many functions, circuits and layouts */

extern char sdfcurrentfileidx;
extern SDFFILEINFO sdffileinfo[];

PRIVATE int        libtabsize = 0;
PRIVATE int        funtabsize = 0;
PRIVATE int        cirtabsize = 0;
PRIVATE int        laytabsize = 0;
PRIVATE LIBTABPTR  lastlibentry = NULL; /* last entry in sdflib list */
PRIVATE LIBTABPTR  libtab = NULL;
PRIVATE FUNTABPTR  funtab = NULL;
PRIVATE CIRTABPTR  cirtab = NULL;
PRIVATE LAYTABPTR  laytab = NULL;

STRING     sdfdeletedfromhashtable = NULL;
int        sdfhashcirmaxsearch = 8;
int        sdfhashlaymaxsearch = 8;
LIBTABPTR  sdflib;    /* Root of the seadif hash tables. */
LIBTABPTR  thislibtab;    /* set to last accessed libtab entry */
FUNTABPTR  thisfuntab;    /* set to last accessed funtab entry */
CIRTABPTR  thiscirtab;    /* set to last accessed cirtab entry */
LAYTABPTR  thislaytab;    /* set to last accessed laytab entry */

#include "src/ocean/libseadif/sysdep.h"

PRIVATE void sdfincreaselayhashtable (void);
PRIVATE void sdfincreasecirhashtable (void);
PRIVATE void sdfincreasefunhashtable (void);
PRIVATE void sdfincreaselibhashtable (void);

void initlibhashtable ()
{
   libtabsize = LIBHASHTABSIZ;
   if (!(libtab = (LIBTABPTR)malloc ((libtabsize+1)*sizeof(LIBTAB))))
      sdfreport (Fatal, "addlibtohashtable: cannot get enough memory");
   /* initialization */
   sdfclearlibhashtable ();
   if (!sdfdeletedfromhashtable)
      sdfdeletedfromhashtable = cs ("#!&^%$*niemand*kiest*ooit*een*naam*als*deze*&^%$#@**&");
}

void sdfclearlibhashtable ()
{
   LIBTABPTR point = libtab;
   LIBTABPTR toptab = libtab + libtabsize;

   for (; point < toptab; ++point)
   {
      point->name = NULL;
      point->library = NULL;
      point->info.what = 0;
      point->info.state = 0;
      point->info.file = 0;
      point->info.fpos = 0;
      point->next = NULL;
      point->function = NULL;
      point->lastfunentry = NULL;
   }
   lastlibentry = NULL;
}

void initfunhashtable ()
{
    funtabsize = HASHTABSIZ;
    if (!(funtab = (FUNTABPTR)malloc ((funtabsize+1)*sizeof(FUNTAB))))
	sdfreport (Fatal, "addfuntohashtable: cannot get enough memory");
    /* initialization */
    sdfclearfunhashtable ();
    if (!sdfdeletedfromhashtable)
	sdfdeletedfromhashtable = cs ("#!&^%$*niemand*kiest*ooit*een*naam*als*deze*&^%$#@**&");
}

void sdfclearfunhashtable ()
{
   FUNTABPTR point = funtab, toptab = funtab + funtabsize;

   for (; point < toptab; ++point)
   {
      point->name = NULL;
      point->function = NULL;
      point->library = NULL;
      point->circuit = NULL;
      point->next = NULL;
      point->info.what = 0;
      point->info.state = 0;
      point->info.file = 0;
      point->info.fpos = 0;
   }
}

void initcirhashtable ()
{
    cirtabsize = HASHTABSIZ;
    if (!(cirtab = (CIRTABPTR)malloc ((cirtabsize+1)*sizeof(CIRTAB))))
	sdfreport (Fatal, "addcirtohashtable: cannot get enough memory");
    /* initialization */
    sdfclearcirhashtable ();
    if (!sdfdeletedfromhashtable)
	sdfdeletedfromhashtable = cs ("#!&^%$*niemand*kiest*ooit*een*naam*als*deze*&^%$#@**&");
}

void sdfclearcirhashtable ()
{
   CIRTABPTR   point = cirtab, toptab = cirtab + cirtabsize;

   for (; point < toptab; ++point)
   {
      point->name = NULL;
      point->circuit = NULL;
      point->function = NULL;
      point->layout = NULL;
      point->next = NULL;
      point->info.what = 0;
      point->info.state = 0;
      point->info.file = 0;
      point->info.fpos = 0;
   }
}

void initlayhashtable ()
{
    laytabsize = HASHTABSIZ;
    if (!(laytab = (LAYTABPTR)malloc ((laytabsize+1)*sizeof(LAYTAB))))
	sdfreport (Fatal, "addlaytohashtable: cannot get enough memory");
    /* initialization */
    sdfclearlayhashtable ();
    if (!sdfdeletedfromhashtable)
	sdfdeletedfromhashtable = cs ("#!&^%$*niemand*kiest*ooit*een*naam*als*deze*&^%$#@**&");
}

void sdfclearlayhashtable ()
{
   LAYTABPTR point = laytab, toptab = laytab + laytabsize;

   for (; point < toptab; ++point)
   {
      point->name = NULL;
      point->layout = NULL;
      point->circuit = NULL;
      point->next = NULL;
      point->info.what = 0;
      point->info.state = 0;
      point->info.file = 0;
      point->info.fpos = 0;
   }
}

PRIVATE void sdfincreaselayhashtable ()
{
    sdfreport (Fatal, "addlaytohashtable: hash table too small");
}

PRIVATE void sdfincreasecirhashtable ()
{
    sdfreport (Fatal, "addcirtohashtable: hash table too small");
}

PRIVATE void sdfincreasefunhashtable ()
{
    sdfreport (Fatal, "addfuntohashtable: hash table too small");
}

PRIVATE void sdfincreaselibhashtable ()
{
    sdfreport (Fatal, "addlibtohashtable: hash table too small");
}

void addlibtohashtable (LIBRARYPTR lib, SDFINFO *info)
{
    LIBTABPTR point, entry, toptab;

    if (!libtabsize) initlibhashtable (); /* needs initializing */

    toptab = libtab + libtabsize;
    entry = libtab + sdfhashstring (lib->name, libtabsize);

    for (point = entry; point < toptab; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point >= toptab) /* go searching the lower part */
    for (point = libtab; point < entry; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point->name && point->name != sdfdeletedfromhashtable) sdfincreaselibhashtable ();

    thislibtab = point;
    if (!(point->name = cs (lib->name))) /* this should NEVER be NULL */
	sdfreport (Fatal, "addlibtohashtable: I cannot handle NULL string for library name");

    point->library = (!info->what ? NULL : lib);
    point->info = *info;
    point->function = NULL;
    point->next = NULL;
    if (lastlibentry)
	lastlibentry->next = point;
    else
	sdflib = point;
    lastlibentry = point;
}

void addfuntohashtable (FUNCTIONPTR fun, LIBTABPTR lib, SDFINFO *info)
{
    FUNTABPTR point, entry, toptab;
    FUNTABPTR lastfunentry;

    if (!funtabsize) initfunhashtable ();

    toptab = funtab + funtabsize;
    entry = funtab + sdfhash2strings (fun->name, lib->name, funtabsize);

    for (point = entry; point < toptab; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point >= toptab) /* go searching the lower part */
    for (point = funtab; point < entry; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point->name && point->name != sdfdeletedfromhashtable) sdfincreasefunhashtable ();

    thisfuntab = point;
    if (!(point->name = cs (fun->name))) /* this should NEVER be NULL */
	sdfreport (Fatal, "addfuntohashtable: I cannot handle NULL string for functon name");

    point->function = (!info->what ? NULL : fun);
    point->info = *info;
    point->library = lib;
    point->circuit = NULL;
    point->next = NULL;
    if ((lastfunentry = lib->lastfunentry))
	lastfunentry->next = point;
    else
	lib->function = point;
    lib->lastfunentry = point;
}

void addcirtohashtable (CIRCUITPTR cir, FUNTABPTR fun, SDFINFO *info)
{
    CIRTABPTR point, entry, toptab, ct;
    int count;

    if (!cirtabsize) initcirhashtable ();

    toptab = cirtab + cirtabsize;
    entry = cirtab + sdfhash3strings (cir->name, fun->name, fun->library->name, cirtabsize);

    for (point = entry; point < toptab; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point >= toptab) /* go searching the lower part */
    for (point = cirtab; point < entry; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point->name && point->name != sdfdeletedfromhashtable) sdfincreasecirhashtable ();

    thiscirtab = point;
    if (!(point->name = canonicstring (cir->name))) /* this should NEVER be NULL */
	sdfreport (Fatal, "addcirtohashtable: I cannot handle NULL string for circuit name");

    point->circuit = (!info->what ? NULL : cir);
    point->function = fun;
    point->info = *info;

    /* Try to preserve the order of the circuits, but don't exagerate... */
    if (!(ct = fun->circuit))
    { /* first circuit to add to this function */
	point->next = NULL;
	fun->circuit = point;
    }
    else
	for (count = 2;; ct = ct->next, ++count)
	if (!ct->next)
	{ /* ct is currently the last circuit in the chain; preserve order */
	    point->next = NULL;
	    ct->next = point;
	    break;
	}
	else if (count >= sdfhashcirmaxsearch)
	{ /* do not search any further; do not preserve order */
	    point->next = fun->circuit;
	    fun->circuit = point;
	    break;
	}
}

void addlaytohashtable (LAYOUTPTR lay, CIRTABPTR cir, SDFINFO *info)
{
    LAYTABPTR point, entry, toptab, lt;
    int count;

    if (!laytabsize) initlayhashtable ();

    toptab = laytab + laytabsize;
    entry = laytab + sdfhash4strings (lay->name, cir->name, cir->function->name, cir->function->library->name, laytabsize);

    for (point = entry; point < toptab; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point >= toptab) /* go searching the lower part */
    for (point = laytab; point < entry; ++point)
	if (!point->name || point->name == sdfdeletedfromhashtable) break; /* found empty slot */

    if (point->name && point->name != sdfdeletedfromhashtable) sdfincreaselayhashtable ();

    thislaytab = point;
    if (!(point->name = canonicstring (lay->name))) /* this should NEVER be NULL */
	sdfreport (Fatal, "addlaytohashtable: I cannot handle NULL string for layout name");

    point->layout = (!info->what ? NULL : lay);
    point->circuit = cir;
    point->info = *info;

    /* Try to preserve the order of the layouts, but don't exagerate... */
    if (!(lt = cir->layout))
    { /* first layout to add to this circuit */
	point->next = NULL;
	cir->layout = point;
    }
    else
	for (count = 2;; lt = lt->next, ++count)
	if (!lt->next)
	{ /* lt is currently the last layout in the chain; preserve order */
	    point->next = NULL;
	    lt->next = point;
	    break;
	}
	else if (count >= sdfhashlaymaxsearch)
	{ /* do not search any further; do not preserve order */
	    point->next = cir->layout;
	    cir->layout = point;
	    break;
	}
}

int libnametoptr (LIBRARYPTR *libptr, STRING libname)
{
    LIBTABPTR point, entry, toptab;
    STRING    n;

    *libptr = NULL;
    if (!libtabsize) initlibhashtable ();

    toptab = libtab + libtabsize;
    entry = libtab + sdfhashstring (libname, libtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == libname) break; /* found ptr to lib name libname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = libtab; (n = point->name) && point < entry; ++point)
	    if (n == libname) break; /* found ptr to lib name libname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thislibtab = point;
    return (!(*libptr = point->library) ? 0 : TRUE); /* TRUE = success */
}

int existslib (STRING libname)
{
    return (sdfexistslib (libname));
}

int sdfexistslib (STRING libname)
{
    LIBTABPTR point, entry, toptab;
    STRING    n;

    if (!libtabsize) return (0);

    toptab = libtab + libtabsize;
    entry = libtab + sdfhashstring (libname, libtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == libname) break; /* found ptr to lib name libname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = libtab; (n = point->name) && point < entry; ++point)
	    if (n == libname) break; /* found ptr to lib name libname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thislibtab = point;
    return (TRUE);
}

int funnametoptr (FUNCTIONPTR *funptr, STRING funname, STRING libname)
{
    FUNTABPTR point, entry, toptab;
    STRING    n;

    *funptr = NULL;
    if (!funtabsize) initfunhashtable ();

    toptab = funtab + funtabsize;
    entry = funtab + sdfhash2strings (funname, libname, funtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == funname && point->library->name == libname) break; /* found ptr to fun name funname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = funtab; (n = point->name) && point < entry; ++point)
	    if (n == funname && point->library->name == libname)
		break; /* found ptr to cir name cirname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thisfuntab = point;
    return (!(*funptr = point->function) ? 0 : TRUE); /* TRUE = success */
}

int existsfun (STRING funname, STRING libname)
{
    return (sdfexistsfun (funname, libname));
}

int sdfexistsfun (STRING funname, STRING libname)
{
    FUNTABPTR point, entry, toptab;
    STRING    n;

    if (!funtabsize) return (0);

    toptab = funtab + funtabsize;
    entry = funtab + sdfhash2strings (funname, libname, funtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == funname && point->library->name == libname) break; /* found ptr to fun name funname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = funtab; (n = point->name) && point < entry; ++point)
	    if (n == funname && point->library->name == libname)
		break; /* found ptr to cir name cirname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thisfuntab = point;
    return (TRUE);
}

int cirnametoptr (CIRCUITPTR *cirptr, STRING cirname, STRING funname, STRING libname)
{
    CIRTABPTR point, entry, toptab;
    FUNTABPTR f;
    STRING    n;

    *cirptr = NULL;
    if (!cirtabsize) initcirhashtable ();

    toptab = cirtab + cirtabsize;
    entry = cirtab + sdfhash3strings (cirname, funname, libname, cirtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == cirname && (f = point->function)->name == funname && f->library->name == libname)
	    break; /* found ptr to cir name cirname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = cirtab; (n = point->name) && point < entry; ++point)
	    if (n == cirname && (f = point->function)->name == funname && f->library->name == libname)
		break; /* found ptr to cir name cirname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thiscirtab = point;
    return (!(*cirptr = point->circuit) ? 0 : TRUE); /* TRUE = success */
}

int existscir (STRING cirname, STRING funname, STRING libname)
{
    return (sdfexistscir (cirname, funname, libname));
}

int sdfexistscir (STRING cirname, STRING funname, STRING libname)
{
    CIRTABPTR point, entry, toptab;
    FUNTABPTR f;
    STRING    n;

    if (!cirtabsize) return (0);

    toptab = cirtab + cirtabsize;
    entry = cirtab + sdfhash3strings (cirname, funname, libname, cirtabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == cirname && (f = point->function)->name == funname && f->library->name == libname)
	    break; /* found ptr to cir name cirname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = cirtab; (n = point->name) && point < entry; ++point)
	    if (n == cirname && (f = point->function)->name == funname && f->library->name == libname)
		break; /* found ptr to cir name cirname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thiscirtab = point;
    return (TRUE);
}

int laynametoptr (LAYOUTPTR *layptr, STRING layname, STRING cirname, STRING funname, STRING libname)
{
    LAYTABPTR point, entry, toptab;
    CIRTABPTR c;
    FUNTABPTR f;
    STRING    n;

    *layptr = NULL;
    if (!laytabsize) initlayhashtable ();

    toptab = laytab + laytabsize;
    entry = laytab + sdfhash4strings (layname, cirname, funname, libname, laytabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == layname && (c = point->circuit)->name == cirname &&
	    (f = c->function)->name == funname && f->library->name == libname)
	    break; /* found ptr to lay name layname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = laytab; (n = point->name) && point < entry; ++point)
	    if (n == layname && (c = point->circuit)->name == cirname &&
	    (f = c->function)->name == funname && f->library->name == libname)
		break; /* found ptr to lay name layname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thislaytab = point;
    return (!(*layptr = point->layout) ? 0 : TRUE); /* TRUE = success */
}

int existslay (STRING layname, STRING cirname, STRING funname, STRING libname)
{
    return sdfexistslay (layname, cirname, funname, libname);
}

int sdfexistslay (STRING layname, STRING cirname, STRING funname, STRING libname)
{
    LAYTABPTR point, entry, toptab;
    CIRTABPTR c;
    FUNTABPTR f;
    STRING    n;

    if (!laytabsize) return (0);

    toptab = laytab + laytabsize;
    entry = laytab + sdfhash4strings (layname, cirname, funname, libname, laytabsize);

    for (point = entry; (n = point->name) && point < toptab; ++point)
	if (n == layname && (c = point->circuit)->name == cirname &&
	    (f = c->function)->name == funname && f->library->name == libname)
	    break; /* found ptr to lay name layname */

    if (point >= toptab) { /* go searching the lower part */
	for (point = laytab; (n = point->name) && point < entry; ++point)
	    if (n == layname && (c = point->circuit)->name == cirname &&
	    (f = c->function)->name == funname && f->library->name == libname)
		break; /* found ptr to lay name layname */
	if (point >= entry) return (0); /* all tested but not found */
    }
    if (!n) return (0);
    thislaytab = point;
    return (TRUE);
}
