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

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/namelist.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/systypes.h"
#include <sys/stat.h>
/* #include <sys/unistd.h> */
#include <errno.h>
#include <fcntl.h>	/* creat() */
#include <string.h>
#include <stdlib.h>
#include "src/ocean/libseadif/sysdep.h"
#include "src/ocean/libseadif/sdferrors.h"

/* #if defined(BSD)
** #include <sys/dir.h>
** #else
*/
#include <dirent.h>
/* #endif
*/

#include <stdio.h>
#include "src/ocean/libseadif/sea_decl.h"

#define MAXENV 8000 /* should be large if we have many directories */
#define READONLY TRUE
#define READ_AND_WRITE 0

int sdfmakelockfiles = TRUE; /* if 0: respect but do not create lock files */
PRIVATE int referr;	/* number of errors encountered */
FILE *seadifin;		/* the file that lex parses, equivalent of yyin */
int makeindex = 0;	/* TRUE if sdfparse() must make an index */
extern  int sdfwhat, sdfverbose; /* from seadif.y */
extern  NAMELISTPTR libunparsed;
extern  NAMELISTLISTPTR libseen;
extern  int yylineno;
extern  STRING findlibname;   /* These names are read by seadifparse().  */
extern  STRING findfunname;   /* If find...name is NULL the parser parses all ... */
extern  STRING findcirname;   /* e.g. find{lib,fun,cir}name !=NULL && findlayname==NULL */
extern  STRING findlayname;   /* means "read all layouts in the specified circuit." */
STRING defaultlibname;    /* used for symbolic references without LibRef */
STRING defaultfunname;    /* used for symbolic references without FunRef */
STRING defaultcirname;    /* used for symbolic references without CirRef */
STRING defaultlayname;    /* used for symbolic references without LayRef */
SDFFILEINFO sdffileinfo[MAXFILES+1];
char sdfcurrentfileidx  = 0; /* index for sdffileinfo[] */
char sdflastfileinfoidx = 0; /* maximum index used with sdffileinfo[] */

long  sdfleftparenthesis;  /* file position of '(' that last occurred in input */
long  sdffilepos;    /* current filepos (refer to lex file seadif.l) */
PRIVATE char noname[] = "";

extern int sdffunislastthinginlib; /* for preliminary abortion of lib parsing */
extern int sdfcirislastthinginfun; /* for preliminary abortion of fun parsing */
extern int sdflayislastthingincir; /* for preliminary abortion of cir parsing */

extern int errno;     /* From /usr/include/errno.h */

int dontCheckChildPorts = 0;

PRIVATE void sdfwriteindexfile (STRING indexfile, int thefileidx);
PRIVATE int sdfstat (int idx, int readonly_or_not);
PRIVATE int sdfmakelockfile (int idx);
#if 0
PRIVATE void movelibrary (LIBRARYPTR *dstlib, LIBRARYPTR *srclib);
#endif

int sdfparseandmakeindex ()
{
    char          sealib[MAXENV+1], sdbuf[MAXENV+1], *indexfile, *sdfile;
    char          *seadirname, *canonicseadirname, *p, *suffix, magicword[200];
    NAMELISTPTR   writabledirectories = NULL;
    int           readonly_or_not;
    DIR           *dirp;
    struct dirent *dp;
    int           lasterror, indexfileisok, j, len;
    FILEPTR       indexstream;

    makeindex = TRUE;
    sdfwhat = 0;        /* Do not actually read anything... */
    findlibname = noname;
    findfunname = noname;
    findcirname = noname;
    findlayname = noname;

    for (j = 1; j <= MAXFILES; ++j)
    {
	sdffileinfo[j].name  = NULL;	/* name of the seadif data base file */
	sdffileinfo[j].lockname = NULL;	/* name of the lock file */
	sdffileinfo[j].fdes  = NULL;	/* ptr to the file descriptor */
	sdffileinfo[j].mtime = 0;	/* time of last modification */
	sdffileinfo[j].ctime = 0;	/* time of last status change */
	sdffileinfo[j].mode  = 0;	/* access mode */
	sdffileinfo[j].dev   = 0;	/* device id */
	sdffileinfo[j].ino   = 0;	/* inode number */
	sdffileinfo[j].nlink = 0;	/* number of (hard-) links */
	sdffileinfo[j].readonly = 0;	/* 0 or TRUE, access by REAL uid */
	sdffileinfo[j].state = 0;	/* currently only "SDFDIRTY" allowed */
    }

    /* it's not clean, but it comes in handy: check for nelsis project directory: */
    p = getenv (SEALIB);
    if (!p || !*p) p = tryNelsisSealib ();
    if (!p) p = SEALIBDEFAULT;

    sdfpathtonamelist (&writabledirectories, getenv (SEALIBWRITE));

    while ((seadirname = p))
    {
	if ((p = strchr (seadirname, ':'))) {
	    len = p - seadirname;
	    if (len > MAXENV) sdfreport (Fatal, "too long seadirname");
	    seadirname = strncpy (sealib, seadirname, len);
	    seadirname[len] = 0;
	    p++;
	}

	if (!(dirp = opendir (seadirname))) continue; /* try next directory */

	lasterror = errno; /* remember the current errno */

	for (; (dp = readdir (dirp)); lasterror = errno)
	if ((suffix = strrchr (dp->d_name, '.')))
	if (strcmp (suffix+1, SEADIFSUFFIX) == 0) /* encountered a seadif file */
	{
	    int len = strlen (seadirname) + strlen (dp->d_name) + 1;
	    if (len > MAXENV) sdfreport (Fatal, "too long path");
	    sprintf (sdbuf, "%s/%s", seadirname, dp->d_name);

	    if (++sdfcurrentfileidx > MAXFILES-3) /* reserve space for scratch (2x) and newsealib */
		sdfreport (Fatal, "Exceeded maximum allowed number of open files, recompile me with other MAXFILES");
	    sdfile = abscanonicpath (sdbuf); /* absolute and no sym-links and no ".." or "." dirs */
	    canonicseadirname = abscanonicpath (seadirname);

	    if (!(suffix = strrchr (sdfile, '.')) || strcmp (suffix+1, SEADIFSUFFIX)) {
		sdfreport (Warning, "Seadif file \"%s\" symbolically links to \"%s\" "
			"with no \".%s\" suffix (skipped).\n", sdbuf, sdfile, SEADIFSUFFIX);
		--sdfcurrentfileidx;
		continue;
	    }

	    if (!(sdffileinfo[(int)sdfcurrentfileidx].fdes = seadifin = fopen (sdfile, "r")))
	    {
		if (errno == EMFILE) {
		    sdfreport (Warning, "Too many open files (%d), cannot open \"%s\"\n", sdfcurrentfileidx+3, sdfile);
		    --sdfcurrentfileidx;
		    return (SDFERROR_SEADIF);
		}
		else
		    sdfreport (Warning, "Cannot open \"%s\" for reading (skipped).\n", sdfile);
		--sdfcurrentfileidx;
		continue;
	    }
	    *magicword = 0;
	    /* Check that this file starts with "..(..Seadif..." (dot is space, ofcourse) */
	    fscanf (seadifin, " ( %s", magicword);
	    if (strcmp (magicword, "Seadif")) {
		sdfreport (Warning, "Seadif file \"%s\" is not a magic cookie (skipped).\n", sdfile);
		--sdfcurrentfileidx;
		continue;
	    }
	    sdffileinfo[(int)sdfcurrentfileidx].name = cs (sdfile);

	    if (!writabledirectories)
		readonly_or_not = READ_AND_WRITE; /* if SEALIBWRITE env not specified, everything is writable */
	    else if (isinnamelist (writabledirectories, canonicseadirname))
		readonly_or_not = READ_AND_WRITE; /* current directory was specified to contain writable files */
	    else
		readonly_or_not = READONLY; /* SEALIBWRITE specified and current dir is not in it */

	    /* get file system state of sdfile (mtime, ctime, mode, etc.): */
	    if (!sdfstat (sdfcurrentfileidx, readonly_or_not))
	    {
		sdfreport (Warning, "Seadif error: cannot stat \"%s\"\n", sdfile);
		return (SDFERROR_SEADIF);
	    }

	    if (!sdfmakelockfile (sdfcurrentfileidx)) /* writable files need to be locked */
	    {
		sdfreport (Warning, "\"%s\" is locked -- cannot open.\n", sdfile);
		fclose (sdffileinfo[(int)sdfcurrentfileidx].fdes);
		/* we MUST set the lockname to NULL, otherwise sdfexit() removes
		 * the lock that WE did not create! (fixed october 1992) */
		sdffileinfo[(int)sdfcurrentfileidx].lockname = NULL;
		--sdfcurrentfileidx;
		return (SDFERROR_FILELOCK);
	    }
	    indexfileisok = 0;

	    if ((indexstream = fopen (indexfile = mkindexfilename (sdfile), "r")))
	    { /* exists an index file: check that it is younger then sdfile. */
		struct stat stat_sdx, stat_sdf;
		if (fstat (fileno (indexstream), &stat_sdx))
		    sdfreport (Warning, "cannot stat \"%s\"...\n", indexfile);
		else if (fstat (fileno (seadifin), &stat_sdf))
		    sdfreport (Warning, "cannot stat \"%s\"...\n", sdfile);

		/* The st_mode == 0444 requirement assures that we do not accept a .sdx file
		 * who's creation was aborted ("core dump") before we could chmod it:
		 */
		else if ((stat_sdx.st_mode&0777) == 0444 && stat_sdx.st_mtime >= stat_sdf.st_mtime)
		{ /* OK, this index file looks all right */
		    if (!sdfreadindexfile (indexstream, sdfcurrentfileidx))
			sdfreport (Warning, "error in \"%s\", recreating index file...\n", indexfile);
		    else
			indexfileisok = TRUE;
		}
	    }

	    fclose (indexstream);

	    if (!indexfileisok) { /* index file non-existent, unreadable or out-of-date */
		if (sdfverbose > 0)
		    fprintf (stderr, "[%d] building index tables for \"%s\"...\n", sdfcurrentfileidx, sdfile);
		fclose (seadifin);  /* Recover from that fscanf()... */
		if (!(sdffileinfo[(int)sdfcurrentfileidx].fdes = seadifin = fopen (sdfile, "r"))) {
		    fprintf (stderr, "*** cannot reopen file %s\n", sdfile);
		    return (SDFERROR_SEADIF);
		}
		sdffunislastthinginlib = sdfcirislastthinginfun = sdflayislastthingincir = 0;
		if (sdfparse (sdfcurrentfileidx)) { /* Error during parsing. Stop. */
		    makeindex = 0;
		    return (SDFERROR_SEADIF);
		}
		sdfwriteindexfile (indexfile, sdfcurrentfileidx);
	    }
	}
	if (errno && errno != lasterror)
	    sdfreport (Warning, "Trouble reading entry in directory \"%s\"\n", seadirname);
	if (closedir (dirp)) sdfreport (Warning, "Cannot close directory ...\n");
    }
    makeindex = 0;
    sdflastfileinfoidx = sdfcurrentfileidx;
    return (SDF_NOERROR);
}

#define NAMELEN 257
#define LINELEN 400

int sdfreadindexfile (FILEPTR idxstream, int currentfileidx)
{
    char     type[NAMELEN+1], name[NAMELEN+1], alias[NAMELEN+1], line[LINELEN];
    int      lineno = 0, matchcount;
    long     filepos, flags;
    SDFINFO  info;
    LIBRARY  lib;
    FUNCTION fun;
    CIRCUIT  cir;
    LAYOUT   lay;

    alias[0] = '\0';
    info.what = 0;
    info.file = currentfileidx;

    while (fgets (line, LINELEN, idxstream) &&
	(matchcount = sscanf (line, "%s %s %ld %ld %s", type, name, &filepos, &flags, alias)) != EOF)
    {
	++lineno;
	if (matchcount < 4) {
	    sdfreport (Warning, "too few items in indexfile on line %d\n", lineno);
	    return (0);
	}
	if (flags)
	    info.state = SDFFASTPARSE;
	else
	    info.state = 0;
	info.fpos = filepos;
	switch (type[0])
	{
	case 'B':
	    lib.name = cs (name);
	    if (!ck_addlibtohashtable (&lib, &info, alias)) return (0);
	    break;
	case 'F':
	    fun.name = cs (name);
	    if (!ck_addfuntohashtable (&fun, thislibtab, &info, alias)) return (0);
	    break;
	case 'C':
	    cir.name = cs (name);
	    if (!ck_addcirtohashtable (&cir, thisfuntab, &info, alias)) return (0);
	    break;
	case 'L':
	    lay.name = cs (name);
	    if (!ck_addlaytohashtable (&lay, thiscirtab, &info, alias)) return (0);
	    break;
	default:
	    sdfreport (Warning, "unknown type \"%s\" in indexfile on line %d\n", type, lineno);
	    return (0);
	}
	alias[0] = '\0';
    }
    return (TRUE);
}

int ck_addlibtohashtable (LIBRARYPTR lib, SDFINFO *info, STRING alias)
{
    if (existslib (lib->name))
    {
	sdfreport (Warning, "Makeindex: multiple libraries \"%s\"\n"
	    "           file \"%s\", char pos %d\n"
	    "           file \"%s\", char pos %d",
	    lib->name,
	    sdffileinfo[(int)info->file].name, info->fpos,
	    sdffileinfo[(int)thislibtab->info.file].name, thislibtab->info.fpos);
	return (0);
    }
    addlibtohashtable (lib, info);
    if (alias && *alias) sdfmakelibalias (alias, lib->name);
    return (TRUE);
}

int ck_addfuntohashtable (FUNCTIONPTR fun, LIBTABPTR libtab, SDFINFO *info, STRING alias)
{
    if (existsfun (fun->name, libtab->name))
    {
	fprintf (stderr, "Makeindex: multiple functions \"%s(%s)\"\n", fun->name, libtab->name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)info->file].name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)thisfuntab->info.file].name);
	return (0);
    }
    addfuntohashtable (fun, libtab, info);
    if (alias && *alias) sdfmakefunalias (alias, fun->name, libtab->name);
    return (TRUE);
}

int ck_addcirtohashtable (CIRCUITPTR cir, FUNTABPTR funtab, SDFINFO *info, STRING alias)
{
    if (existscir (cir->name, funtab->name, funtab->library->name))
    {
	fprintf (stderr, "Makeindex: multiple circuits \"%s(%s(%s))\"\n", cir->name, funtab->name, funtab->library->name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)info->file].name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)thiscirtab->info.file].name);
	return (0);
    }
    addcirtohashtable (cir, funtab, info);
    if (alias && *alias) sdfmakeciralias (alias, cir->name, funtab->name, funtab->library->name);
    return (TRUE);
}

int ck_addlaytohashtable (LAYOUTPTR lay, CIRTABPTR cirtab, SDFINFO *info, STRING alias)
{
    if (existslay (lay->name, cirtab->name, cirtab->function->name, cirtab->function->library->name))
    {
	fprintf (stderr, "Makeindex: multiple layouts \"%s(%s(%s(%s)))\"\n",
	    lay->name, cirtab->name, cirtab->function->name, cirtab->function->library->name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)info->file].name);
	fprintf (stderr, "           file \"%s\"\n", sdffileinfo[(int)thislaytab->info.file].name);
	return (0);
    }
    addlaytohashtable (lay, cirtab, info);
    if (alias && *alias) sdfmakelayalias (alias, lay->name, cirtab->name,
    cirtab->function->name, cirtab->function->library->name);
    return (TRUE);
}

/* write a new indexfile using only the info from the hash tables. */
PRIVATE void sdfwriteindexfile (STRING indexfile, int thefileidx)
{
    LIBTABPTR libt;
    FUNTABPTR funt;
    CIRTABPTR cirt;
    LAYTABPTR layt;
    FILEPTR   indexstream;

    unlink (indexfile);
    if (!(indexstream = fopen (indexfile, "w"))) {
	sdfreport (Warning, "cannot create new index file \"%s\"\n", indexfile);
	return;
    }

    for (libt = sdflib; libt; libt = libt->next)
    {
	if (libt->info.file != thefileidx) continue;
	if (!libt->alias)
	    fprintf (indexstream, "B\t%s\t%ld\t%d\n", libt->name, libt->info.fpos,
		(libt->info.state & SDFFASTPARSE)? 1 : 0);
	else
	    fprintf (indexstream, "B\t%s\t%ld\t%d\t%s\n", libt->name, libt->info.fpos,
		(libt->info.state & SDFFASTPARSE)? 1 : 0, libt->alias);
	for (funt = libt->function; funt; funt = funt->next)
	{
	    if (!funt->alias)
		fprintf (indexstream, "F\t%s\t%ld\t%d\n", funt->name, funt->info.fpos,
		    (funt->info.state & SDFFASTPARSE)? 1 : 0);
	    else
		fprintf (indexstream, "F\t%s\t%ld\t%d\t%s\n", funt->name, funt->info.fpos,
		    (funt->info.state & SDFFASTPARSE)? 1 : 0, funt->alias);
	    for (cirt = funt->circuit; cirt; cirt = cirt->next)
	    {
		if (!cirt->alias)
		    fprintf (indexstream, "C\t%s\t%ld\t%d\n", cirt->name, cirt->info.fpos,
			(cirt->info.state & SDFFASTPARSE)? 1 : 0);
		else
		    fprintf (indexstream, "C\t%s\t%ld\t%d\t%s\n", cirt->name, cirt->info.fpos,
			(cirt->info.state & SDFFASTPARSE)? 1 : 0, cirt->alias);
		for (layt = cirt->layout; layt; layt = layt->next)
		    if (!layt->alias)
			fprintf (indexstream, "L\t%s\t%ld\t%d\n", layt->name, layt->info.fpos,
			    (layt->info.state & SDFFASTPARSE)? 1 : 0);
		    else
			fprintf (indexstream, "L\t%s\t%ld\t%d\t%s\n", layt->name, layt->info.fpos,
			    (layt->info.state & SDFFASTPARSE)? 1 : 0, layt->alias);
	    }
	}
    }

    if (fclose (indexstream))
	sdfreport (Fatal, "sdfwriteindexfile: cannot close index file");
    if (chmod (indexfile, 0444) == -1)
	sdfreport (Warning, "cannot chmod \"%s\"\n", indexfile);
}

PRIVATE int sdfstat (int idx, int readonly_or_not)
{
    struct stat buf;

    if (stat (sdffileinfo[idx].name, &buf)) return (0);

    sdffileinfo[idx].mtime = buf.st_mtime;
    sdffileinfo[idx].ctime = buf.st_ctime;
    sdffileinfo[idx].mode  = buf.st_mode & 0777; /* only 9 least sign. mode bits */
    sdffileinfo[idx].dev   = buf.st_dev;
    sdffileinfo[idx].ino   = buf.st_ino;
    sdffileinfo[idx].nlink = buf.st_nlink;

    /* the access system call only considers the real uid */
    if (readonly_or_not == READONLY || access (sdffileinfo[idx].name, W_OK))
	sdffileinfo[idx].readonly = TRUE;
    else
	sdffileinfo[idx].readonly = 0;
    return (TRUE);
}

/* If sdfile is already locked, return 0.  Else create lock file and
 * return TRUE if succes, 0 if fail.
 */
PRIVATE int sdfmakelockfile (int idx)
{
   STRING lockfile;

   if (!sdffileinfo[idx].readonly && !getenv ("NO_SEADIF_LOCK"))
   { /* sdfile is writable, must be locked */
      STRING sdfile = sdffileinfo[idx].name;
      int    fd;
      struct stat buf;

      if (!(lockfile = mklockfilename (idx))) {
	 sdfreport (Warning, "cannot make a lock file name for \"%s\"\n", sdfile);
	 return (0); /* this is not really ok, but what else can i do ? */
      }

      /* even if we do not make lock files ourselves,
	 we must respect the lock files made by other processes */
      if (!sdfmakelockfiles && stat (lockfile, &buf)) {
	 /* lock file already exists */
	 sdffileinfo[idx].lockname = NULL;
	 return (0);
      }

      if (sdfmakelockfiles &&
	  (fd = creat (lockfile, 0)) >= 0) /* creat() is atomic (undocumented feature) */
      {
	 if (close (fd)) sdfreport (Fatal, "sdfmakelockfile: cannot close lock file");
	 return (TRUE);
      }

      if (!sdfmakelockfiles) {
	 fs (sdffileinfo[idx].lockname);
	 sdffileinfo[idx].lockname = NULL;
      }
      return (!sdfmakelockfiles);
   }
   else { /* unwritable, no need for locking */
      sdffileinfo[idx].lockname = NULL;
      return (TRUE);
   }
}

/* If number of hard links to origname != 1, lockfilename is "/tmp/sdk<ino>(<devno>)".
 * Otherwise it is something like "origname.sdk"
 */
STRING mklockfilename (int idx)
{
    STRING lfname, suffix;
    char lockfilename[NAMELEN+1];

    if (sdffileinfo[idx].nlink != 1) {
	/*
	* sprintf (lockfilename, "/tmp/%s%d(%d)",
	*         SEADIFLOCKSUFFIX, sdffileinfo[idx].ino, sdffileinfo[idx].dev);
	*/
	fprintf (stderr, "\nSorry, seadif file \"%s\" has more than\n", sdffileinfo[idx].name);
	fprintf (stderr, "one canonic name (link count is %d, should be 1). The problem with\n",
		(int)sdffileinfo[idx].nlink);
	fprintf (stderr, "such a file is that I just cannot think of a directory to put the\n");
	fprintf (stderr, "lock file in. If you really need links, please use symbolic links.\n\n");
	return (NULL);
    }

    lfname = sdffileinfo[idx].name;
    if ((suffix = strrchr (lfname, '.'))) {
	int len = suffix - lfname + 1;
	if (len + 3 > NAMELEN) sdfreport (Fatal, "too long lockfilename");
	strncpy (lockfilename, lfname, len);
	strcpy (lockfilename + len, SEADIFLOCKSUFFIX);
	return (sdffileinfo[idx].lockname = cs (lockfilename));
    }
    return (sdffileinfo[idx].lockname = NULL);
}

#if 0
/* Move the library list srclib to the end of the list dstlib. */
PRIVATE void movelibrary (LIBRARYPTR *dstlib, LIBRARYPTR *srclib)
{
    LIBRARYPTR lp;

    for (lp = *dstlib; lp && lp->next; lp = lp->next) ;
    if (!lp)
	*dstlib = srclib;
    else
	lp->next = srclib;
}
#endif

int solvecircuitinstance (CIRINSTPTR cirinst, CIRCUITPTR parentcircuit, int verbose)
{
    CIRCUITPTR  cir;
    STRING      cirname, funname, libname;
    NAMELISTPTR nl, nextnl;
    int found;

    if (!cirinst) return (0);
    if (!(cirname = (STRING)cirinst->circuit)) {
	sdfreport (Fatal, "solvecircuitinstance(): no symbolic name.");
	dumpcore ();
    }

    if (!(nl = (NAMELISTPTR)cirinst->curcirc))
	funname = defaultfunname;
    else { /* Exists a namelist. First entry is funname. */
	funname = nl->name;
	nl = nl->next;
    }

    if (!nl)
	libname = defaultlibname;
    else
	libname = nl->name; /* next entry in namelist */

    found = cirnametoptr (&cir, cirname, funname, libname);
    if (!found) {
	if (verbose)
	    sdfreport (Warning, "Cannot solve reference to circuit \"%s(%s(%s))\".\n",
		cirname, funname, libname);
	cirinst->curcirc = NULL;
	cirinst->circuit = NULL;
	for (nl = (NAMELISTPTR)cirinst->curcirc; nl; nl = nextnl) {
	    nextnl = nl->next;
	    FreeNamelist (nl); /* must free names only after message 'Cannot solve...' */
	}
    }
    else {
	for (nl = (NAMELISTPTR)cirinst->curcirc; nl; nl = nextnl) {
	    nextnl = nl->next;
	    FreeNamelist (nl); /* must free names only after message 'Cannot solve...' */
	}
	cir->linkcnt += 1;     /* Don't forget to increment this count !! */
	cirinst->circuit = cir;
	cirinst->curcirc = parentcircuit;
    }
    fs (cirname);
    return (found);
}

#define CIRINST_HASHTABLE 50

int solvecircuitcirportrefs (CIRCUITPTR circuit)
{
    NETPTR        net;
    CIRPORTREFPTR cpref;
    CIRINSTPTR    cinst;
    CIRPORTPTR    cp;
    char *cpname, *ciname;
    int num_term;
    int startreferr = referr, num_inst = 0, use_cirinst_hashtable = 0;

    if (!circuit) return (0);

    if (!(net = circuit->netlist) && circuit->cirinst)
	sdfreport (Warning, "circuit \"%s(%s(%s))\" without nets.\n",
	    circuit->name, circuit->function->name, circuit->function->library->name);

    /* if we have more than CIRINST_HASHTABLE instances we need a hashtable ... */
    for (cinst = circuit->cirinst;
	cinst && num_inst < CIRINST_HASHTABLE; cinst = cinst->next) ++num_inst;

    if (num_inst >= CIRINST_HASHTABLE) {
	use_cirinst_hashtable = TRUE;
	/* put all circuit instances in a hashtable, using the name as the key */
	for (cinst = circuit->cirinst; cinst; cinst = cinst->next)
	    sdfstv_insert (cinst->name, (void *)cinst);
    }

    for (; net; net = net->next)
    {
	for (cpref = net->terminals, num_term = 0; cpref;
		cpref = cpref->next, ++num_term, fs (cpname))
	{
	    cpname = (STRING)cpref->cirport;
	    if (!cpref->cirinst) {
		/* Reference to terminal of current circuit. */
		for (cp = circuit->cirport; cp; cp = cp->next)
		    if (cpname == cp->name) break;
		if (!cp) {
		    sdfreport (Error, "Invalid reference to CircuitPort \"%s\" of circuit \"%s(%s(%s))\" in Net \"%s\"\n",
			cpname, circuit->name, circuit->function->name, circuit->function->library->name, net->name);
		    cpref->cirport = NULL;
		    referr++;
		    continue;
		}
		cpref->cirport = cp;
		if (cp->net) {
		    sdfreport (Error, "Multiple references to Cirport \"%s\" in Circuit \"%s(%s(%s))\": Net \"%s\" and Net \"%s\".\n",
			cpname, circuit->name, circuit->function->name, circuit->function->library->name, cp->net->name, net->name);
		    referr++;
		    continue;
		}
		cp->net = net;
	    }
	    else {
		/* Reference to terminal of child circuit, first let cinst
		 * point to the cirinst that we are talking about.
		 */
		ciname = (STRING)cpref->cirinst;
		if (use_cirinst_hashtable)
		    cinst = (CIRINSTPTR)sdfstv_lookup (ciname);
		else
		    for (cinst = circuit->cirinst; cinst; cinst = cinst->next)
			if (ciname == cinst->name) break;
		if (!cinst) {
		    sdfreport (Error, "NetInstRef \"%s\" in Net \"%s\" is not an instance of circuit \"%s(%s(%s))\"\n",
			ciname, net->name, circuit->name, circuit->function->name, circuit->function->library->name);
		    cpref->cirinst = NULL;
		    cpref->cirport = NULL;
		    referr++;
		    fs (ciname);
		    continue;
		}
		if (!cinst->circuit) {
		    /* ignore, we already reported this error in the function solvecircuitinstance() */
		    cpref->cirinst = NULL;
		    cpref->cirport = NULL;
		    fs (ciname);
		    continue;
		}
		for (cp = cinst->circuit->cirport; cp; cp = cp->next)
		    if (cpname == cp->name) break;
		if (!cp && !dontCheckChildPorts) {
		    sdfreport (Error, "Invalid reference in net \"%s\" of circuit \"%s(%s(%s))\" to CirPort \"%s\" of circuit \"%s(%s(%s))\".\n",
			net->name, circuit->name, circuit->function->name, circuit->function->library->name, cpname,
			cinst->circuit->name, cinst->circuit->function->name, cinst->circuit->function->library->name);
		    referr++;
		}
		cpref->cirport = cp;
		cpref->cirinst = cinst;
		fs (ciname);
	    }
	}
	net->num_term = num_term;
	net->circuit = circuit;
    }

    /* check to see that all Cirports of this circuit are being referenced */
    if (sdfverbose > 0 && circuit->netlist) /* if no nets then we already issued a warning message */
    for (cp = circuit->cirport; cp; cp = cp->next)
	if (!cp->net)
	    sdfreport (Warning, "Nets of Circuit \"%s(%s(%s))\" do not refer to Cirport \"%s\"\n",
		circuit->name, circuit->function->name, circuit->function->library->name, cp->name);

    if (use_cirinst_hashtable) sdfstv_cleanup ();
    return (referr == startreferr);
}

int solvelayoutlayportrefs (LAYOUTPTR  layout, CIRCUITPTR circuit)
{
    CIRPORTPTR cp;
    LAYPORTPTR lp;
    char *pname;
    int startreferr = referr;

    if (!circuit) /* we don't have a circuit, invalidade all cirportrefs */
	for (lp = layout->layport; lp; lp = lp->next) {
	    fs (lp->cirport->name);
	    lp->cirport = NULL;
	    referr++;
	}
    else
	for (lp = layout->layport; lp; lp = lp->next) {
	    pname = (STRING)lp->cirport;
	    for (cp = circuit->cirport; cp; cp = cp->next)
		if (pname == cp->name) break;
	    if (!cp) {
	    /* why generate an error?
		sdfreport (Error, "LayoutPort \"%s\" of layout \"%s(%s(%s(%s)))\" has no equivalent CircuitPort in circuit \"%s(%s(%s))\"\n",
		    pname, layout->name, layout->circuit->name, layout->circuit->function->name,
		    layout->circuit->function->library->name, circuit->name, circuit->function->name, circuit->function->library->name);
		lp->cirport = NULL;
		referr++;
		continue;
	    */
		/* no error, we just create a new CIRPORT and add it to the circuit */
		NewCirport (cp);
		cp->name = cs (pname);
		cp->net = NULL;
#ifdef SDF_PORT_DIRECTIONS
		cp->direction = SDF_PORT_UNKNOWN;
#endif
		cp->next = circuit->cirport;
		circuit->cirport = cp;
	    }
	    fs (pname);
	    lp->cirport = cp;
	}
    return (startreferr == referr);
}
