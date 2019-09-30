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

#ifndef __LIBIO_H
#define __LIBIO_H

#include <stdio.h>
#include "src/ocean/libseadif/systypes.h"

typedef FILE *FILEPTR;

typedef struct _SDFFILEINFO
{
STRING  name;			  /* name of the seadif data base file */
STRING  lockname;		  /* name of the lock file */
FILE    *fdes;			  /* ptr to the file descriptor */
time_t  mtime, ctime;		  /* time of last modification, status change */
mode_t  mode;			  /* access mode */
dev_t   dev;			  /* device id */
ino_t   ino;			  /* inode number */
nlink_t nlink;			  /* number of (hard-) links */
int     readonly;		  /* 0 or TRUE, access by REAL uid */
int     state;			  /* Currently only "SDFDIRTY" allowed */
}
SDFFILEINFO, *SDFFILEINFOPTR;

/* for sdffileinfo: */

#define SDFDIRTY   1 		  /* Means that this file has been modified
				     (merge with scratch file on sdfclose()) */
#define SDFCLOSED  2
#define SDFUPDATED 4

typedef struct _SDFINFO
{
unsigned int what;		  /* specifies which parts are in core */
char              state;	  /* SDFWRITTEN */
char              file;		  /* entry in global array sdffileinfo[] */
time_t            timestamp;	  /* modification time of seadif object */
long int          fpos;		  /* for use with lseek() */
}
SDFINFO, *SDFINFOPTR;

typedef struct _LIBTAB
{
SDFINFO        info;
STRING         name;
STRING         alias;
LIBRARYPTR     library;
struct _LIBTAB *next;
struct _FUNTAB *function, *lastfunentry;
}
LIBTAB, *LIBTABPTR;

typedef struct _FUNTAB
{
SDFINFO        info;
STRING         name;
STRING         alias;
FUNCTIONPTR    function;
struct _CIRTAB *circuit;
struct _FUNTAB *next;
struct _LIBTAB *library;
}
FUNTAB, *FUNTABPTR;

typedef struct _CIRTAB
{
SDFINFO        info;
STRING         name;
STRING         alias;
CIRCUITPTR     circuit;
struct _LAYTAB *layout;
struct _CIRTAB *next;
struct _FUNTAB *function;
}
CIRTAB, *CIRTABPTR;

typedef struct _LAYTAB
{
SDFINFO        info;
STRING         name;
STRING         alias;
LAYOUTPTR      layout;
struct _LAYTAB *next;
struct _CIRTAB *circuit;
}
LAYTAB, *LAYTABPTR;

#define MAXFILES 100 /* this should be > _NFILE in stdio.h ? */

/* possible values for info.state */
#define SDFINCORE     1
#define SDFSTUB       2
#define SDFWRITTEN    4
#define SDFATTACHED   8
#define SDFFASTPARSE 16
#define SDFREMOVED   32
#define SDFTOUCHED   64		  /* set if sdftouch() was called */

/* values for libtab.info.what */
#define SDFLIBBODY     0x00000001
#define SDFLIBSTAT     0x00000002
#define SDFLIBALL      0x00000003

/* values for funtab.info.what */
#define SDFFUNBODY     0x00000010
#define SDFFUNSTAT     0x00000020
#define SDFFUNTYPE     0x00000040
#define SDFFUNALL      0x00000070

/* values for cirtab.info.what */
#define SDFCIRBODY     0x00000100
#define SDFCIRSTAT     0x00000200
#define SDFCIRPORT     0x00000400
#define SDFCIRINST     0x00000800
#define SDFCIRNETLIST  0x00001000
#define SDFCIRBUS      0x00002000
#define SDFCIRATTRIB   0x00004000
#define SDFCIRALL      0x00007f00

/* values for laytab.info.what */
#define SDFLAYBODY     0x00010000
#define SDFLAYSTAT     0x00020000
#define SDFLAYPORT     0x00040000
#define SDFLAYSLICE    0x00080000
#define SDFLAYWIRE     0x00100000
#define SDFLAYBBX      0x00200000
#define SDFLAYOFF      0x00400000
#define SDFLAYATTRIB   0x00800000
#define SDFLAYLABEL    0x00000004
#define SDFLAYALL      0x00ff0004

				  /* IK, timing model extensions */
#include "src/ocean/libseadif/tmio.h"

/* These values are hacky addenda for {lay,cir,fun,lib}tab.info.what...  They
 * are used in seadif.y and should not overlap with other values of info.what:
 */
#define SDF_X_LIBALIAS 0x10000000
#define SDF_X_FUNALIAS 0x20000000
#define SDF_X_CIRALIAS 0x40000000
#define SDF_X_LAYALIAS 0x80000000

#define SdfStreamInfo(info) \
    (info.state & SDFWRITTEN ? sdffileinfo[0].fdes : sdffileinfo[(int)info.file].fdes)
#define SdfStream(tab) \
    ((tab)->info.state & SDFWRITTEN ? sdffileinfo[0].fdes : sdffileinfo[(int)(tab)->info.file].fdes)
#define SdfStreamIdx(tab) \
    ((tab)->info.state & SDFWRITTEN ? 0 : (tab)->info.file)
#define SdfStreamName(tab) (sdffileinfo[(int)SdfStreamIdx(tab)].name)

#define SEALIB "SEALIB"		  /* Environment string that contains lib dirs */
#define SEALIBWRITE "SEALIBWRITE" /* list of directories containing seadif files
				   * that may be written. If SEALIBWRITE is not set,
				   * then all writable seadif files may be written. */
#define NEWSEALIB "NEWSEALIB"	  /* env. variable specifying name of file
				   * where newly created libraries go. */
#define NEWSEADIR "NEWSEADIR"	  /* env. var. is default directory for new seadif files */

#define DEFAULTNEWSEALIB "@newsealib.sdf" /* name if NEWSEALIB not set */
#define SEALIBDEFAULT ".:seadif:/usr/local/lib/seadif"
				  /* Three default directories to search. */

#define SEADIFSUFFIX     "sdf"	  /* Seadif files end in `.sdf' */
#define SEADIFIDXSUFFIX  "sdx"	  /* Seadif index files end in `.sdx' */
#define SEADIFLOCKSUFFIX "sdk"	  /* Seadif lock files end in `.sdk' */

extern LIBTABPTR thislibtab, sdflib;
extern FUNTABPTR thisfuntab;
extern CIRTABPTR thiscirtab;
extern LAYTABPTR thislaytab;

#define ALIASLIB 1
#define ALIASFUN 2
#define ALIASCIR 3
#define ALIASLAY 4

extern STRING thislibnam, thisfunnam, thiscirnam, thislaynam; /* for sdfunalias() */

#endif
