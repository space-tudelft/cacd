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
 * Libio.c contains the user interface to the Seadif library.
 */
#include <errno.h>
#include "src/ocean/libseadif/sysdep.h"
#include <stdio.h>
#include <string.h>
#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/namelist.h"
#include "src/ocean/libseadif/sea_decl.h"

/* OK, at this point some systems STILL don't have the following three things
 * defined, so we do it ourselves.
 */
#ifndef SEEK_SET
#define SEEK_SET	0	/* Set file pointer to "offset" */
#define SEEK_CUR	1	/* Set file pointer to current plus "offset" */
#define SEEK_END	2	/* Set file pointer to EOF plus "offset" */
#endif

PRIVATE int sdfgetlayintocore (int what, STRING layname, STRING cirname, STRING funname, STRING libname);
PRIVATE void sdfparselayintotree (LAYTABPTR laytab, int what, int alreadyhavebody);
PRIVATE int solvelayoutinstances (LAYOUTPTR layout, SLICEPTR slice);
PRIVATE int sdfgetcirintocore (int what, STRING cirname, STRING funname, STRING libname);
PRIVATE void sdfparsecirintotree (CIRTABPTR cirtab, int what, int alreadyhavebody);
PRIVATE void sdfparsefunintotree (FUNTABPTR funtab, int what, int alreadyhavebody);
PRIVATE void sdfparselibintotree (LIBTABPTR libtab, int what, int alreadyhavebody);
PRIVATE int sdfgetfunintocore (int what, STRING funname, STRING libname);
PRIVATE int sdfgetlibintocore (int what, STRING libname);
PRIVATE void sdfmovecirattributes (CIRCUITPTR tocir, CIRCUITPTR fromcir, int attributes);
PRIVATE void sdfmovefunattributes (FUNCTIONPTR tofun, FUNCTIONPTR fromfun, int attributes);
PRIVATE void sdfmovelayattributes (LAYOUTPTR tolay, LAYOUTPTR fromlay, int attributes);
PRIVATE void sdfmovelibattributes (LIBRARYPTR tolib, LIBRARYPTR fromlib, int attributes);
PRIVATE int sdfsolvebusnetreferences (CIRCUITPTR circuit);
PRIVATE int sdfsolvecirinstances (CIRCUITPTR circuit);
PRIVATE STRING sdfmakescratchname (void);
PRIVATE void sdfcopyallthisstuff (LIBTABPTR libt, FILEPTR indexstream);
PRIVATE STRING sdfgimesomename (void);
PRIVATE void sdflookaheadforsexpandskip (FILEPTR tostream, FILEPTR fromstream,
			int *lastchar, STRING sexp, int *fastparse);
PRIVATE void sdfgetsecondscratchstream (FILEPTR *stream, int *idx);
PRIVATE int sdfwritelaynoshit (LAYTABPTR laytab);
PRIVATE int sdfwritecirnoshit (CIRTABPTR cirtab);
PRIVATE int sdfwritefunnoshit (FUNTABPTR funtab);
PRIVATE int sdfwritelibnoshit (LIBTABPTR libtab);
PRIVATE int sdffindfileforlib (STRING libname);
PRIVATE void sdfbindfiletolib (STRING libname, int idx);
PRIVATE char *sdfmaketmpname (STRING string);
PRIVATE void sdfcleanupdatastructures (); /* clean up internal datastructures */

/* IK, timing model's references solving routines.
*/
PRIVATE int sdfsolve_timereferences (CIRCUITPTR circuit);
PRIVATE int sdfsolve_tmlistreferences (CIRCUITPTR circuit, TIMING* timing);
PRIVATE int sdfsolve_ttermreferences (CIRCUITPTR circuit, TIMING* timing);
PRIVATE int sdfsolve_netmodsreferences (CIRCUITPTR circuit, TIMING* timing);
PRIVATE int sdfsolve_tpathsreferences (CIRCUITPTR circuit, TIMING* timing);
PRIVATE int sdfsolve_delasgreferences (CIRCUITPTR circuit, TIMING* timing);

#include "src/ocean/libseadif/sea_decl.h"

FILEPTR sdfcopystream; /* This is the file where the writes go */
SEADIF  sdfroot;       /* Global root of the seadif tree */
extern  LIBTABPTR sdflib; /* global root of the hash tables */
extern  STRING findlibname;
extern  STRING findfunname;
extern  STRING findcirname;
extern  STRING findlayname;
extern  STRING defaultlibname;    /* used for symbolic references without LibRef */
extern  STRING defaultfunname;    /* used for symbolic references without FunRef */
extern  STRING defaultcirname;    /* used for symbolic references without CirRef */
extern  STRING defaultlayname;    /* used for symbolic references without LayRef */
extern  int sdfcopytheinput; /* copy input from the parser to scratch file */
extern  int sdfparseonelib;
extern  int sdfparseonefun;
extern  int sdfparseonecir;
extern  int sdfparseonelay;
extern  int sdfindentlevel;   /* Column to start printing in, see DoIndnt(). */
extern  int sdfdumpstyle;
extern  int sdfdumpspacing;

PRIVATE int putcindentedtellmewhatposition = 0;
PRIVATE long putcindentedthisistheposition;
PRIVATE int sdfNumberOfTimesOpened = 0;	/* # calls to sdfopen() */
PRIVATE int sdfNumberOfTimesClosed = 0;	/* # calls to sdfclose() */

LIBRARYPTR  thislib;
FUNCTIONPTR thisfun;
CIRCUITPTR  thiscir;
LAYOUTPTR   thislay;
PRIVATE int sdfwhatweparsedintocore = 0; /* set by sdfgetcirintocore() */

extern LAYTABPTR thislaytab;
extern CIRTABPTR thiscirtab;
extern FUNTABPTR thisfuntab;
extern LIBTABPTR thislibtab;
extern SDFFILEINFO sdffileinfo[];
extern char sdflastfileinfoidx; /* maximum index used with sdffileinfo[] */
PRIVATE char sdfreadscratchidx; /* extra scratch entry in sdffileinfo[] */

/* following 6 macros provide a uniform interface for the seek function in the
 * stdio library and the seek system call:
 */
#define FSEEK(stream, fpos) (fseek (stream, fpos, SEEK_SET) == 0)
      /* returns 0 on failure, TRUE on success */
#define FSEEK_TO_END(stream) (fseek (stream, 0L, SEEK_END) == 0)
      /* returns position of last char in file */
#define FTELL(stream) \
      ftell (stream) /* returns current file position as a long integer */
#define LSEEK(stream, fpos) \
      (lseek (fileno (stream), (off_t)fpos, SEEK_SET) != (off_t)-1)
      /* returns 0 on failure, TRUE on success */
#define LSEEK_TO_END(stream) \
      (lseek (fileno (stream), (off_t)0, SEEK_END) != (off_t)-1)
      /* returns position of last char in file */
#define LTELL(stream) \
      (long)lseek (fileno (stream), (off_t)0, SEEK_CUR)
      /* returns current file position as a long integer */

   /* Flex directly calls read() when it needs input and does NOT use the stdio
    * package. Consequently, for seeking we need lseek(). */
#define SEEK(stream, fpos)  LSEEK (stream, fpos)
#define SEEK_TO_END(stream) LSEEK_TO_END (stream)
#define TELL(stream)        LTELL (stream)

/* Following five lines support the putindented() function or macro. */
#define TABINDENT 8     /* a TAB replaces this much spaces */
#define SDF_INDENT_TRAJECTORY 0   /* legal state for putc state machine */
#define SDF_TEXT_TRAJECTORY   1   /* legal state for putc state machine */
PRIVATE int sdfindentstate = SDF_INDENT_TRAJECTORY; /* state variable of putc state machine */
PRIVATE int sdfindentthismuch = 0;  /* tells what the current column of indent is */

extern int sdfindentlevel;    /* used with dump_status() etc. */
/* The following are from seadif.y: */
extern  LIBRARYPTR   libraryptr;       /* (SDFLIBBODY) */
extern  FUNCTIONPTR  functionptr;      /* (SDFFUNBODY) */
extern  CIRCUITPTR   circuitptr;       /* (SDFCIRBODY) */
extern  LAYOUTPTR    layoutptr;        /* (SDFLAYBODY) */
extern  CIRPORTPTR   cirportlistptr;   /* (SDFCIRPORT) */
extern  CIRINSTPTR   cirinstlistptr;   /* (SDFCIRINST) */
extern  NETPTR       netlistptr;       /* (SDFCIRNETLIST) */
extern  LAYPORTPTR   layportlistptr;   /* (SDFLAYPORT) */
extern  LAYLABELPTR  laylabelistptr;   /* (SDFLAYLABEL) */
extern  LAYINSTPTR   layinstptr;       /* (SDFLAYSLICE) */
extern  WIREPTR      wirelistptr;      /* (SDFLAYWIRE) */
extern LIBRARYPTR  sdfwritethislib;
extern FUNCTIONPTR sdfwritethisfun;
extern CIRCUITPTR  sdfwritethiscir;
extern LAYOUTPTR   sdfwritethislay;
extern int sdffunislastthinginlib; /* for preliminary abortion of lib parsing */
extern int sdfcirislastthinginfun; /* for preliminary abortion of fun parsing */
extern int sdflayislastthingincir; /* for preliminary abortion of cir parsing */
extern int sdfobligetimestamp;	   /* support for sdftouch() */
extern time_t sdftimestamp;	   /* support for sdftouch() */

extern unsigned int sdfwhat;  /* inspected by sdfparse() to see what things to read */
extern unsigned int sdfstuff; /* sdfparse() looks here to see what to delete while
				copying onto the scratch file */
extern unsigned int sdfwrite;

PRIVATE int sdfgetlayintocore (int what, STRING layname, STRING cirname,
                              STRING funname, STRING libname)
{
    int whatweneed, whatwehave, whatwewant;
    LAYTABPTR thislaytabentry;

    if (!existslay (layname, cirname, funname, libname)) return (0); /* layout does not exist */

    /* Perform some checking and correction on 'what' */
    what &= SDFLAYALL;     /* clear any request for libraries, functions, circuits */
    if (!what) return (0); /* could not set thislay, thiscir,thisfun,thislib */
    whatwewant = what | SDFLAYBODY; /* don't forget the layout's body */

    /* what-we-need = (what-we-have XOR what-we-want) AND what-we-want */
    whatwehave = (thislaytabentry = thislaytab)->info.what;
    whatweneed = (whatwehave ^ whatwewant) & whatwewant;
    sdfwhatweparsedintocore = whatweneed; /* somewhat preliminary */
    if (!whatweneed) {
	/* requested layout (and attributes) already in core */
	thislay = thislaytabentry->layout;
	thiscir = thislay->circuit;
	thisfun = thiscir->function;
	thislib = thisfun->library;
	return (TRUE);
    }

    /* parse this thing into the tree */
    sdfparselayintotree (thislaytab, whatweneed, whatwehave & SDFLAYBODY);
    return (TRUE);
}

PRIVATE void sdfparselayintotree (LAYTABPTR laytab, int what, int alreadyhavebody)
{
    FILEPTR stream = SdfStream (laytab);
    long offset = laytab->info.fpos;
    int thiswhat;

    if (!SEEK (stream, offset))
	sdfreport (Fatal, "seek to layout failed (file %s, offset %d)", SdfStreamName (laytab), offset);

    /* Even if we don't need it we must parse the body: */
    sdfwhat = thiswhat = (what | SDFLAYBODY) & SDFLAYALL;
    sdfwhatweparsedintocore = sdfwhat;
    sdfparseonelay = TRUE;
    sdffunislastthinginlib = sdfcirislastthinginfun = sdflayislastthingincir = 0;
    sdfstuff = sdfwrite = 0;

    if (sdfparse (SdfStreamIdx (laytab)))
	sdfreport (Fatal, "parse of layout %s in file %s failed", laytab->name, SdfStreamName (laytab));

    sdfparseonelay = 0;
    thislay = layoutptr;

    if (!(thiscir = (thiscirtab = laytab->circuit)->circuit)) {
	/* layPort refers to CirPort, that's why: */
	int whatwewantfromthecircuit = SDFCIRBODY + (what & SDFLAYPORT ? SDFCIRPORT : 0);
	sdfparsecirintotree (thiscirtab, whatwewantfromthecircuit, 0);
    }
    else if (what & SDFLAYPORT && !(thiscirtab->info.what & SDFCIRPORT))
	/* already have circuit, but without the CirPorts, go get them... */
	sdfparsecirintotree (thiscirtab, SDFCIRPORT, TRUE);

    if (alreadyhavebody) {
	/* We already had the body, only wanted some new attributes.
	 * Preserve the old body and disgard the new one. There is no
	 * alternative here because there may be already some pointers to
	 * the old body. The DANGEROUS thing is that there also might have
	 * been some pointer to --for instance-- the old layportlist. These
	 * references are invalid after this action! However, I consider
	 * this the responsibility of the caller.
	 */
	LAYOUTPTR oldlay = laytab->layout;
	sdfmovelayattributes (oldlay, thislay, what);
	FreeLayout (thislay);
	thislay = oldlay; /* thiscir, thisfun and thislib already ok */
    }
    else {
	/* new layout body, update the meta-hierarchy */
	thislay->circuit = thiscir;
	thislay->next = thiscir->layout;
	thiscir->layout = thislay; /* and link this lay into the list */
    }
    sdfwhatweparsedintocore = thiswhat;
    thisfun = thiscir->function;
    thislib = thisfun->library;
    laytab->info.what |= what;  /* update the info in the hash tables */
    laytab->layout = thislay;
}

/* Move the attributes from one layout to another, overwriting the
 * attributes already present in the destination layout. __DANGEROUS__
 */
PRIVATE void sdfmovelayattributes (LAYOUTPTR tolay, LAYOUTPTR fromlay, int attributes)
{
    if (attributes & SDFLAYSTAT) {
	if (tolay->status) FreeStatus (tolay->status);
	tolay->status = fromlay->status;
	fromlay->status = NULL;
    }
    if (attributes & SDFLAYPORT) {
	if (tolay->layport) sdfdeletelayport (tolay->layport);
	tolay->layport = fromlay->layport;
	fromlay->layport = NULL;
    }
    if (attributes & SDFLAYLABEL) {
	if (tolay->laylabel) sdfdeletelaylabel (tolay->laylabel);
	tolay->laylabel = fromlay->laylabel;
	fromlay->laylabel = NULL;
    }
    if (attributes & SDFLAYSLICE) {
	if (tolay->slice) sdfdeleteslice (tolay->slice, 0);
	tolay->slice = fromlay->slice;
	fromlay->slice = NULL;
    }
    if (attributes & SDFLAYWIRE) {
	if (tolay->slice) sdfdeletewire (tolay->wire);
	tolay->wire = fromlay->wire;
	fromlay->wire = NULL;
    }
    if (attributes & SDFLAYBBX) {
	tolay->bbx[HOR] = fromlay->bbx[HOR];
	tolay->bbx[VER] = fromlay->bbx[VER];
	fromlay->bbx[HOR] = 0;
	fromlay->bbx[VER] = 0;
    }
    if (attributes & SDFLAYOFF) {
	tolay->off[HOR] = fromlay->off[HOR];
	tolay->off[VER] = fromlay->off[VER];
	fromlay->off[HOR] = 0;
	fromlay->off[VER] = 0;
    }
}

int sdfreadlay (int what, STRING layname, STRING cirname, STRING funname, STRING libname)
{
    LAYOUTPTR   localthislay;
    CIRCUITPTR  localthiscir;
    FUNCTIONPTR localthisfun;
    LIBRARYPTR  localthislib;
    int thisparsed; /* saves sdfwhatweparsedintocore */
    int tudobem = 1;

    /* Check if libname is not already in core as a stub */

    if (!sdfgetlayintocore (what, layname, cirname, funname, libname)) {
	sdfreport (Warning, "Cannot read layout (%s(%s(%s(%s)))) -- does not exist.\n",
	    layname, cirname, funname, libname);
	return (0);
    }

    /* Set by sdfgetlayintocore(), spoiled by solvelayoutinstances(), so need to save: */
    thisparsed = sdfwhatweparsedintocore;
    localthislay = thislay;
    localthiscir = thiscir;
    localthisfun = thisfun;
    localthislib = thislib;

    /* Assign default names for use by sdfsolvelayoutinstances(): */
    defaultlibname = libname;
    defaultfunname = funname;
    defaultcirname = cirname;

    /* Need to solve references to child layouts (convert STRINGs to LAYOUTPTRs). */
    if (thisparsed & SDFLAYSLICE) tudobem &= solvelayoutinstances (localthislay, localthislay->slice);
    if (thisparsed & SDFLAYPORT ) tudobem &= solvelayoutlayportrefs (localthislay, localthiscir);

    sdfwhatweparsedintocore = thisparsed;
    thislay = localthislay;
    thiscir = localthiscir;
    thisfun = localthisfun;
    thislib = localthislib;
    return (tudobem);
}

PRIVATE int solvelayoutinstances (LAYOUTPTR layout, SLICEPTR slice)
{
    LAYINSTPTR  layinst;
    SLICEPTR    chldslice;
    NAMELISTPTR nl, nlsave, nextnl;
    STRING      libname, funname, cirname, layname;
    int         found = TRUE;

    if (!slice) return (found);

    if (slice->chld_type == LAYINST_CHLD)
    for (layinst = slice->chld.layinst; layinst; layinst = layinst->next)
    {
	if (!(layname = (STRING)layinst->layout)) {
	    sdfreport (Fatal, "solvelayoutinstances(): no symbolic name.");
	    dumpcore();
	}

	if (!(nl = nlsave = (NAMELISTPTR)layinst->flag.p)) cirname = defaultcirname;
	else { /* exists a namelist, first entry is cirname */
	    cirname = nl->name;
	    nl = nl->next;
	}

	if (!nl) funname = defaultfunname;
	else { /* second entry in namelist is the funname */
	    funname = nl->name;
	    nl = nl->next;
	}

	if (!nl) libname = defaultlibname;
	else libname = nl->name; /* last entry in namelist is the libname */

	if (!existslay (layname, cirname, funname, libname)) {
	    sdfreport (Warning, "Cannot solve reference to layout (%s(%s(%s(%s)))) -- does not exist\n",
		layname, cirname, funname, libname);
	    layinst->flag.p = NULL;
	    layinst->layout = NULL;
	    found = 0;
	}
	else if (!sdfgetlayintocore (SDFLAYBODY, layname, cirname, funname, libname)) {
	    sdfreport (Warning, "Error while reading layout (%s(%s(%s(%s)))).\n",
		layname, cirname, funname, libname);
	    found = 0;
	    layinst->flag.p = NULL;
	    layinst->layout = NULL;
	}
	else {
	    thislay->linkcnt += 1;
	    layinst->flag.p = NULL;
	    layinst->layout = thislay;
	}

	for (nl = nlsave; nl; nl = nextnl) {
	    nextnl = nl->next;
	    FreeNamelist (nl); /* must free names only after error messages */
	}
	forgetstring (layname);
    }
    else if (slice->chld_type == SLICE_CHLD) {
	for (chldslice = slice->chld.slice; chldslice; chldslice = chldslice->next)
	    found &= solvelayoutinstances (layout, chldslice); /* recursive calls */
    }
    else {
	sdfreport (Warning, "Corrupt data structure in layout (%s(%s(%s(%s)))): unknown chld_type\n",
	    layout->name, layout->circuit->name, layout->circuit->function->name,
	    layout->circuit->function->library->name);
	dumpcore();
    }

    return (found);
}

PRIVATE int sdfhashlay (LAYOUTPTR lay)
{
    CIRCUITPTR  cptr = lay->circuit, cptr2;
    FUNCTIONPTR f = cptr->function, fptr;
    LIBRARYPTR  l = f->library, lptr;
    /* This is a layout that's not already in the data base. Create an entry. */
    SDFINFO info;

    if (!existscir (cptr->name, f->name, l->name))
    {
	/* don't know this circuit -- maybe we know the function? */
	SDFINFO cirinfo;

	if (!existsfun (f->name, l->name))
	{
	    /* don't know this function -- maybe we know the library? */
	    SDFINFO funinfo;

	    if (!existslib (l->name))
	    {
		/* even don't know the lib. Question: what file goes the new lib in? */
		SDFINFO libinfo;

		libinfo.file = info.file = sdffindfileforlib (l->name);
		libinfo.what = SDFLIBBODY;
		libinfo.state = 0;
		libinfo.fpos = 0;
		if (l->status) libinfo.what |= SDFLIBSTAT;
		addlibtohashtable (l, &libinfo);
		if (!sdfwritelibnoshit (thislibtab)) return (0);
	    }
	    else if (!(lptr = thislibtab->library))
		(thislibtab->library = l, info.file = thislibtab->info.file);
	    else if (lptr == l)
		info.file = thislibtab->info.file;
	    else
		sdfreport (Fatal, "sdfwritelay: library (%s) not properly registered\n", l->name);

	    funinfo.file = info.file;
	    funinfo.state = 0;
	    funinfo.fpos = 0;
	    funinfo.what = SDFFUNBODY;
	    if (f->status) funinfo.what |= SDFFUNSTAT;
	    addfuntohashtable (f, thislibtab, &funinfo);
	    if (!sdfwritefunnoshit (thisfuntab)) return (0);
	}
	else if (!(fptr = thisfuntab->function))
	    (thisfuntab->function = f, info.file = thisfuntab->info.file);
	else if (fptr == f)
	    info.file = thisfuntab->info.file;
	else
	    sdfreport (Fatal, "sdfwritelay: function (%s(%s)) not properly registered\n",
		f->name, l->name);

	cirinfo.file = info.file;
	cirinfo.state = 0;
	cirinfo.fpos = 0;
	cirinfo.what = SDFCIRBODY;
	if (cptr->status)  cirinfo.what |= SDFCIRSTAT;
	if (cptr->netlist) cirinfo.what |= SDFCIRNETLIST;
	if (cptr->cirinst) cirinfo.what |= SDFCIRINST;
	if (cptr->cirport) cirinfo.what |= SDFCIRPORT;
	addcirtohashtable (cptr, thisfuntab, &cirinfo);
	if (!sdfwritecirnoshit (thiscirtab)) return (0);
    }
    else if (!(cptr2 = thiscirtab->circuit))
	(thiscirtab->circuit = cptr, info.file = thiscirtab->info.file);
    else if (cptr2 == cptr)
	info.file = thiscirtab->info.file;
    else
	sdfreport (Fatal, "sdfwritelay: circuit (%s(%s(%s))) not properly registered\n",
	    cptr->name, f->name, l->name);

    info.what = SDFLAYBODY;
    info.state = 0;
    info.fpos  = 0;
    if (lay->status)   info.what |= SDFLAYSTAT;
    if (lay->layport)  info.what |= SDFLAYPORT;
    if (lay->slice)    info.what |= SDFLAYSLICE;
    if (lay->wire)     info.what |= SDFLAYWIRE;
    if (lay->laylabel) info.what |= SDFLAYLABEL;
    addlaytohashtable (lay, thiscirtab, &info);
    return TRUE;
}

int sdftouchlay (LAYOUTPTR lay, time_t timestamp)
{
    if (!existslay (lay->name, lay->circuit->name, lay->circuit->function->name,
	lay->circuit->function->library->name))
    {
	sdfhashlay (lay); /* create an entry in the hash tables */
	if (!sdfwritelaynoshit (thislaytab)) return 0;
    }
    thislaytab->info.timestamp = timestamp;
    thislaytab->info.state |= SDFTOUCHED;
    return TRUE;
}

int sdfwritelay (int what, LAYOUTPTR lay)
{
    LAYOUTPTR   layp;
    CIRCUITPTR  cptr = lay->circuit;
    FUNCTIONPTR f = cptr->function;
    LIBRARYPTR  l = f->library;
    FILEPTR     stream;
    long offset, eofpos;
    int  idx;

    what &= SDFLAYALL; /* only handle layout stuff */

    if (!existslay (lay->name, cptr->name, f->name, l->name)) {
	sdfhashlay (lay);
	/* Now that lay is in the hashtables we can write it onto the scratch file. */
	if (!sdfwritelaynoshit (thislaytab)) return (0);
	return (TRUE);
    }

    if (!(layp = thislaytab->layout))
	thislaytab->layout = lay;
    else if (layp != lay)
	sdfreport (Fatal, "sdfwritelay: layout (%s(%s(%s(%s)))) not properly registered\n",
	    lay->name, cptr->name, f->name, l->name);

    idx = SdfStreamIdx (thislaytab);

    if (what == SDFLAYALL) {
    /* Special case, we don't need to read and write. Just dump the layout. */
	if (!sdfwritelaynoshit (thislaytab)) return (0);
	return (TRUE);
    }

    if ((stream = sdffileinfo[idx].fdes) == sdfcopystream)
    sdfgetsecondscratchstream (&stream, &idx);

    offset = thislaytab->info.fpos;
    if (!SEEK (stream, offset))
	sdfreport (Fatal, "sdfwritelay: seek to layout failed (file %s, offset %d)", SdfStreamName (thislaytab), offset);
    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of scratch file */
	sdfreport (Fatal, "sdfwritelay: seek to end of file %s failed", sdfmakescratchname ());
    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritelay: ftell on file %s failed", sdfmakescratchname ());

    sdfwrite = sdfstuff = what & ~SDFLAYBODY;
    sdfwhat = 0;
    sdfwritethislay = lay;
    sdfcopytheinput = TRUE;
    sdfparseonelay = TRUE;
    sdffunislastthinginlib = sdfcirislastthinginfun = sdflayislastthingincir = 0;
    sdfobligetimestamp = (thislaytab->info.state & SDFTOUCHED);
    sdftimestamp = thislaytab->info.timestamp;

    if (sdfparse (idx)) sdfreport (Fatal, "sdfwritelay: sdfparse/copy of layout failed");

    sdfobligetimestamp = 0;
    sdfparseonelay = 0;
    sdfcopytheinput = 0;

    if (sdffileinfo[idx].readonly) {
	sdfreport (Warning, "sdfwritelay: attempt to write read-only file %s", sdffileinfo[idx].name);
	return (0);
    }

    /* Administrate the new location of this layout, don't destroy the original file name */
    thislaytab->info.fpos = eofpos;   /* ...but only administrate the new file position... */
    /* This implies that info.file = scratchfile: */
    thislaytab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    sdffileinfo[(int)thislaytab->info.file].state |= SDFDIRTY;
    return (TRUE);
}

/* Arrange for a circuit (or parts of it) to get installed in the
 * global seadif tree (sdfroot). If it is already there, only set
 * thiscir, thisfun and thislib to the correct value. If the requested
 * (parts of) the circuit are not yet in core, call the parser, solve
 * symbolic references (that is, transform the STRINGs to hard boiled
 * CIRCUIT pointers) and then set thiscir, thisfun and thislib.
 * Sdfgetcirintocore() also accepts requests for only part of a circuit.
 * The user can specify her desires by means of the parameter 'what'.
 * Possible values for 'what' are in the file "libio.h". There are no
 * restrictions on the order of the partial requests. If a requested
 * attribute requires other attributes --or even other circuits-- to
 * be present in core, it automaticly fetches them from the data base.
 * For instance, if you request the netlist of a circuit "nand2", then
 * sdfgetcirintocore() also fetches the body and the cirinst list and
 * the cirport list of "nand2", and the body and cirport list of the
 * child circuits "nenh" and "penh". Ofcourse, if any of these objects
 * already is in core, then it is not fetched. The function returns 0
 * on failure, TRUE otherwise.
 */
PRIVATE int sdfgetcirintocore (int what, STRING cirname, STRING funname, STRING libname)
{
    CIRTABPTR thiscirtabentry;
    int whatweneed, whatwehave, whatwewant;

    if (!existscir (cirname, funname, libname)) return (0); /* circuit does not exist */

    /* Perform some checking and correction on 'what' */
    what &= SDFCIRALL;     /* clear request not for circuit */
    if (!what) return (0); /* could not set thiscir,thisfun,thislib */
    whatwewant = what | SDFCIRBODY; /* don't forget the circuit's body */
    if ((what & SDFCIRNETLIST) && !(thiscirtab->info.what & SDFCIRINST)) whatwewant |= SDFCIRINST;
    if ((what & SDFCIRNETLIST) && !(thiscirtab->info.what & SDFCIRPORT)) whatwewant |= SDFCIRPORT;

    /* what-we-need = (what-we-have XOR what-we-want) AND what-we-want */
    whatwehave = (thiscirtabentry = thiscirtab)->info.what;
    whatweneed = (whatwehave ^ whatwewant) & whatwewant;
    sdfwhatweparsedintocore = whatweneed;
    if (!whatweneed) {
	/* requested circuit (and attributes) already in core */
	thiscir = thiscirtabentry->circuit;
	thisfun = thiscir->function;
	thislib = thisfun->library;
	return (TRUE);
    }

    /* parse this thing into the tree */
    sdfparsecirintotree (thiscirtab, whatweneed, whatwehave & SDFCIRBODY);
    return (TRUE);
}

/* This is where the action is! Seek to correct file position and call
 * the parser. Then link the resulting circuit structure into the
 * circuit list of the corresponding function. If the boolean
 * alreadyhavebody is TRUE then the newly parsed attributes will
 * overwrite the old attributes, but in any case will the old body
 * remain in existence. Thus, old pointers to the circuit remain valid.
 * However, old pointers to the circuit's attributes (for instance the
 * cirports) will become invalid because sdfparsecirintotree() deletes
 * these attrubutes.
 */
PRIVATE void sdfparsecirintotree (CIRTABPTR cirtab, int what, int alreadyhavebody)
{
    FILEPTR stream = SdfStream (cirtab);
    long offset = cirtab->info.fpos;

    if (!SEEK (stream, offset))
	sdfreport (Fatal, "seek to circuit failed (file %s, offset %d)", SdfStreamName (cirtab), offset);

    /* Even if we don't need it we must parse the body: */
    sdfwhat = (what | SDFCIRBODY) & SDFCIRALL;
    sdfwhatweparsedintocore = sdfwhat;
    sdfparseonecir = TRUE;
    sdffunislastthinginlib = sdfcirislastthinginfun = 0;
    sdflayislastthingincir = (cirtab->info.state & SDFFASTPARSE);
    sdfstuff = sdfwrite = 0;
    if (sdfparse (SdfStreamIdx (cirtab)))
	sdfreport (Fatal, "sdfparse of circuit failed (file %s)", SdfStreamName (cirtab));
    sdflayislastthingincir = sdfparseonecir = 0;
    thiscir = circuitptr;
    if (!(thisfun = (thisfuntab = cirtab->function)->function))
	sdfparsefunintotree (thisfuntab, SDFFUNALL, 0);

    if (alreadyhavebody) {
	/* We already had the body, only wanted some new attributes.
	 * Preserve the old body and disgard the new one. There is no
	 * alternative here because there may be already some pointers to
	 * the old body. The DANGEROUS thing is that there also might have
	 * been some pointer to --for instance-- the old cirportlist. These
	 * references are invalid after this action! However, I consider
	 * this the responsibility of the caller.
	 */
	CIRCUITPTR oldcir = cirtab->circuit;
	sdfmovecirattributes (oldcir, thiscir, what);
	FreeCircuit (thiscir);
	thiscir = oldcir; /* thisfun and thislib already ok */
    }
    else {
	/* new circuit body, update the meta-hierarchy */
	thiscir->function = thisfun; /* thisfun set by funnametoptr (perhaps) */
	thiscir->next = thisfun->circuit;
	thisfun->circuit = thiscir;  /* and link this cir into the list */
    }
    thislib = thisfun->library; /* if lib was already in core this must be set */
    cirtab->info.what |= what;  /* update the info in the hash tables */
    cirtab->circuit = thiscir;
}

/* This is where the action is! Seek to correct file position and call
 * the parser.  Then link the resulting function structure into the
 * functionlist of the corresponding library. Sets the globals thisfun
 * and thislib to point to the function just parsed. Does NOT CHECK
 * whether the requested function is already in core.
 */
PRIVATE void sdfparsefunintotree (FUNTABPTR funtab, int what, int alreadyhavebody)
{
    FILEPTR  stream = SdfStream (funtab);
    long offset = funtab->info.fpos;

    if (!SEEK (stream, offset))
	sdfreport (Fatal, "seek to function failed (file %s, offset %d)", SdfStreamName (funtab), offset);

    sdfwhat = (what | SDFFUNBODY) & SDFFUNALL;
    sdfparseonefun = TRUE;
    sdflayislastthingincir = sdffunislastthinginlib = 0;
    sdfcirislastthinginfun = (funtab->info.state & SDFFASTPARSE);
    sdfwrite = sdfstuff = 0;

    if (sdfparse (SdfStreamIdx (funtab)))
	sdfreport (Fatal, "sdfparse of function failed (file %s)", SdfStreamName (funtab));

    sdfcirislastthinginfun = sdfparseonefun = 0;
    thisfun = functionptr;
    if (!(thislib = (thislibtab = funtab->library)->library)) /* library not yet in core */
	sdfparselibintotree (thislibtab, SDFLIBALL, 0); /* also sets thislib */

    if (alreadyhavebody) {
	/* we already had the body, only wanted some new attributes (e.g. status) */
	FUNCTIONPTR oldfun = funtab->function;
	sdfmovefunattributes (oldfun, thisfun, what);
	FreeFunction (thisfun);
	thisfun = oldfun;
	thislib = thisfun->library;
    }
    else {
	thisfun->library = thislib; /* adjust backward pointer of this new function */
	thisfun->next = thislib->function;
	thislib->function = thisfun; /* and link it into the funlist list */
    }
    funtab->info.what |= what; /* update the info in the hash tables */
    funtab->function = thisfun;
}

/* This is where the action is! Seek to correct file position and call
 * the parser.  Then link the resulting library structure into the
 * global seadif tree (sdfroot).  Sets the global thislib to point to
 * the library just parsed. Does NOT CHECK whether the requested
 * library is already in core.
 */
PRIVATE void sdfparselibintotree (LIBTABPTR libtab, int what, int alreadyhavebody)
{
    FILEPTR stream = SdfStream (libtab);
    long offset = libtab->info.fpos;

    if (!SEEK (stream, offset))
	sdfreport (Fatal, "seek to library failed (file %s, offset %d)", SdfStreamName (libtab), offset);

    sdfwhat = (what | SDFLIBBODY) & SDFLIBALL;
    sdfparseonelib = TRUE;
    sdflayislastthingincir = sdfcirislastthinginfun = 0;
    sdffunislastthinginlib = (libtab->info.state & SDFFASTPARSE);
    sdfstuff = sdfwrite = 0;

    if (sdfparse (SdfStreamIdx (libtab)))
	sdfreport (Fatal, "sdfparse of library failed (file %s)", SdfStreamName (libtab));

    sdffunislastthinginlib = sdfparseonelib = 0;
    thislib = libraryptr;

    if (alreadyhavebody) {
	/* we already had the body, only wanted some new attributes (e.g. status) */
	LIBRARYPTR oldlib = libtab->library;
	sdfmovelibattributes (oldlib, thislib, what);
	FreeLibrary (thislib);
	thislib = oldlib;
    }
    else {
	thislib->next = sdfroot.library; /* link into the global library list */
	sdfroot.library = thislib;
    }
    libtab->info.what |= sdfwhat;
    libtab->library = thislib;
}

PRIVATE int sdfgetfunintocore (int what, STRING funname, STRING libname)
{
    FUNTABPTR thisfuntabentry;
    int whatweneed, whatwehave, whatwewant;

    if (!existsfun (funname, libname)) return (0); /* function does not exist */

    /* perform some checking and correction on 'what' */
    what &= SDFFUNALL;     /* clear any request not for function */
    if (!what) return (0); /* could not set thisfun, thislib */
    whatwewant = what | SDFFUNBODY; /* don't forget the function's body */

    /* what-we-need = NOT(what-we-have) AND what-we-want */
    whatweneed = ~(whatwehave = (thisfuntabentry = thisfuntab)->info.what) & whatwewant;
    sdfwhatweparsedintocore = whatweneed;
    if (!whatweneed) {
	/* requested function (and attributes) already in core */
	thisfun = thisfuntabentry->function;
	thislib = thisfun->library;
	return (TRUE);
    }
    /* parse this thing into the tree */
    sdfparsefunintotree (thisfuntab, whatweneed, whatwehave & SDFFUNBODY);
    return (TRUE);
}

int sdfreadfun (int what, STRING funname, STRING libname)
{
    if (!sdfgetfunintocore (what, funname, libname)) {
	sdfreport (Error, "Cannot read function (%s(%s)) -- does not exist", funname, libname);
	return (0);
    }
    return (TRUE);
}

PRIVATE int sdfgetlibintocore (int what, STRING libname)
{
    int whatweneed, whatwehave, whatwewant;
    LIBTABPTR thislibtabentry;

    if (!existslib (libname)) return (0); /* library does not exist */

    /* perform some checking and correction on 'what' */
    what &= SDFLIBALL;     /* clear any request not for library */
    if (!what) return (0); /* could not set thisfun, thislib */
    whatwewant = what | SDFLIBBODY; /* don't forget the library's body */

    /* what-we-need = NOT(what-we-have) AND what-we-want */
    whatweneed = ~(whatwehave = (thislibtabentry = thislibtab)->info.what) & whatwewant;
    sdfwhatweparsedintocore = whatweneed;
    if (!whatweneed) {
	/* requested library (and attributes) already in core */
	thislib = thislibtabentry->library;
	return (TRUE);
    }
    /* parse this thing into the tree */
    sdfparselibintotree (thislibtab, whatweneed, whatwehave & SDFLIBBODY);
    return (TRUE);
}

int sdfreadlib (int what, STRING libname)
{
    if (!sdfgetlibintocore (what, libname)) {
	sdfreport (Error, "Cannot read library (%s) -- does not exist.\n", libname);
	return (0);
    }
    return (TRUE);
}

/* Reads a circuit from the lib and installs it in the seadiftree. If
 * the circuit is already in core, just return pointers to that
 * corecell. If it is not in core, the names of the circuit's children
 * are converted to pointers to actual circuits. The function returns
 * 0 on failure.  After a successful read the global pointers
 * thislib, thisfun and thiscir point to the objects just read and the
 * function returns TRUE.
 */
int sdfreadcir (int what, STRING cirname, STRING funname, STRING libname)
{
    CIRINSTPTR  ci;
    CIRCUITPTR  localthiscir;
    FUNCTIONPTR localthisfun;
    LIBRARYPTR  localthislib;
    int thisparsed; /* saves sdfwhatweparsedintocore */
    int tudobem = 1;

    if (!sdfgetcirintocore (what, cirname, funname, libname)) {
	sdfreport (Error, "Cannot read circuit (%s(%s(%s))) -- does not exist", cirname, funname, libname);
	return (0);
    }

    /* Set by sdfgetcirintocore(), spoiled by sdfsolvecirinstances(), so need to save: */
    thisparsed = sdfwhatweparsedintocore;
    localthiscir = thiscir;
    localthisfun = thisfun;
    localthislib = thislib;

    /* Assign default names for use by sdfsolvecirinstances(): */
    defaultlibname = libname;
    defaultfunname = funname;

    if (thisparsed & SDFCIRINST)
	/* Need to solve references to child circuits (convert STRINGs to CIRCUITPTRs). */
	tudobem &= sdfsolvecirinstances (localthiscir);
    else if (thisparsed & SDFCIRNETLIST) /* new netlist but old cirinstances... */
	/* make sure that the children have their cirports in core */
	for (ci = thiscir->cirinst; ci; ci = ci->next)
	{
	    CIRCUITPTR c;
	    if (!(c = ci->circuit)->cirport) {
		FUNCTIONPTR f;
		STRING childcirname = c->name;
		STRING childfunname = (f = c->function)->name;
		STRING childlibname = f->library->name;
		if (!sdfgetcirintocore (SDFCIRPORT, childcirname, childfunname, childlibname)) {
		    sdfreport (Error, "while reading circuit (%s(%s(%s))).\n",
			childcirname, childfunname, childlibname);
		    tudobem = 0;
		}
	    }
	}

    sdfwhatweparsedintocore = thisparsed;
    if (thisparsed & SDFCIRNETLIST) /* new netlist */
	tudobem &= solvecircuitcirportrefs (thiscir = localthiscir);
    if (thisparsed & SDFCIRBUS) /* buslist */
	tudobem &= sdfsolvebusnetreferences (localthiscir);
    if (thisparsed & SDFCIRTM)
	tudobem &= sdfsolve_timereferences (localthiscir);

    thiscir = localthiscir;
    thisfun = localthisfun;
    thislib = localthislib;
    return (tudobem);
}

/* Move the attributes from one library to another, overwriting the
 * attributes already present in the destination library. __DANGEROUS__
 */
PRIVATE void sdfmovelibattributes (LIBRARYPTR tolib, LIBRARYPTR fromlib, int attributes)
{
    if (attributes & SDFLIBSTAT) {
	if (tolib->status) FreeStatus (tolib->status);
	tolib->status = fromlib->status;
	fromlib->status = NULL;
    }
}

/* Move the attributes from one function to another, overwriting the
 * attributes already present in the destination function. __DANGEROUS__
 */
PRIVATE void sdfmovefunattributes (FUNCTIONPTR tofun, FUNCTIONPTR fromfun, int attributes)
{
    if (attributes & SDFFUNSTAT) {
	if (tofun->status) FreeStatus (tofun->status);
	tofun->status = fromfun->status;
	fromfun->status = NULL;
    }
    if (attributes & SDFFUNTYPE) {
	tofun->type = fromfun->type;
	fromfun->type = NULL;
    }
}

/* Move the attributes from one circuit to another, overwriting the
 * attributes already present in the destination circuit. __DANGEROUS__
 */
PRIVATE void sdfmovecirattributes (CIRCUITPTR tocir, CIRCUITPTR fromcir, int attributes)
{
    if (attributes & SDFCIRSTAT) {
	if (tocir->status) FreeStatus (tocir->status);
	tocir->status = fromcir->status;
	fromcir->status = NULL;
    }
    if (attributes & SDFCIRPORT) {
	if (tocir->cirport) sdfdeletecirport (tocir->cirport);
	tocir->cirport = fromcir->cirport;
	fromcir->cirport = NULL;
    }
    if (attributes & SDFCIRINST) {
	if (tocir->cirinst) sdfdeletecirinst (tocir->cirinst, 0);
	tocir->cirinst = fromcir->cirinst;
	fromcir->cirinst = NULL;
    }
    if (attributes & SDFCIRNETLIST) {
	if (tocir->netlist) sdfdeletenetlist (tocir->netlist);
	tocir->netlist = fromcir->netlist;
	fromcir->netlist = NULL;
    }
    if (attributes & SDFCIRTM) {
	if (tocir->timing) sdfdeletetiming (tocir->timing);
	tocir->timing = fromcir->timing;
	fromcir->timing = NULL;
    }
}

/* The parser sdfparse() reads the name of the circuit instance as a STRING and
 * puts it (with a type cast) into the field CIRINST.circuit. It puts the
 * function name and the library name as STRINGs into a namelist that (with a
 * type cast) appears in the field CIRINST.curcirc. The function
 * sdfsolvecirinstances() removes these STRINGs and transforms them into hard
 * pointers to the appropriate CIRCUIT. If a referenced CIRCUIT is not yet in
 * core, sdfsolvecirinstances() fetches it from the data base with the minimum
 * required attributes, that is, whithout CIRPORTs if the parent circuit does
 * not have its netlist in core and with CIRPORTs otherwise.
 */
PRIVATE int sdfsolvecirinstances (CIRCUITPTR circuit)
{
    CIRINSTPTR ci;
    int tudobem = TRUE, headermsg = 0;

    for (ci = thiscir->cirinst; ci; ci = ci->next)
    {
	/* ...check here to see if we need stubs... */
	STRING stubcirname = (STRING)ci->circuit;
	STRING stubfunname, stublibname;
	NAMELISTPTR nl = (NAMELISTPTR)ci->curcirc, nlsave, nextnl;

	/* First find out the names of the referenced cell */
	if (!(nlsave = nl))
	    stubfunname = defaultfunname;
	else {
	    /* Exists a namelist. First entry is funname. */
	    stubfunname = nl->name;
	    nl = nl->next;
	}

	if (!nl) stublibname = defaultlibname;
	else /* next entry in namelist is the libname */
	    stublibname = nl->name;

	/* Check to see that this circuit really exists and
	 * set 'thiscirtab' to its entry in the hash table.
	 */
	if (!existscir (stubcirname, stubfunname, stublibname))
	{
	    if (headermsg++ == 0)
		sdfreport (Warning, "Suspected error in CirInstList of circuit (%s(%s(%s))):",
		    circuit->name, circuit->function->name, circuit->function->library->name);
	    sdfreport (Warning, "cannot solve reference to circuit (%s(%s(%s))) -- does not exist",
		stubcirname, stubfunname, stublibname);
	    ci->curcirc = NULL;
	    ci->circuit = NULL;
	    tudobem = 0;
	}
	/* if circuit has a netlist (in core) it probably refers to the childs cirports */
	else if (!sdfgetcirintocore (circuit->netlist ? SDFCIRBODY + SDFCIRPORT : SDFCIRBODY,
		    stubcirname, stubfunname, stublibname))
	{
	    sdfreport (Error, "while reading circuit (%s(%s(%s)))", stubcirname, stubfunname, stublibname);
	    ci->circuit = NULL;
	    ci->curcirc = NULL;
	    tudobem = 0;
	}
	else {
	    /* Tudo bem, fill in the pointers to the circuit (both the parent and the child) */
	    thiscir->linkcnt += 1;
	    ci->circuit = thiscir; /* child */
	    ci->curcirc = circuit; /* parent */
	}

	for (nl = nlsave; nl; nl = nextnl) {
	    nextnl = nl->next;
	    FreeNamelist (nl); /* must free names only after error messages */
	}
	forgetstring (stubcirname);
    }
    return (tudobem);
}

/* sdfparse() does not solve the references to nets in the NetRef
 * statement.  (NetRef is a substatement in the BusList.) Sdfparse
 * only puts a canonicstring containing the name of the referenced net
 * in the NETREF.net field. The function sdfsolvebusnetreferences()
 * transformes the STRING in a NETREF.net into a real pointer to the
 * appropriate net. (This function has terrible time complexity...)
 */
PRIVATE int sdfsolvebusnetreferences (CIRCUITPTR circuit)
{
    BUSPTR    busptr;
    NETREFPTR netrefptr;
    NETPTR    net;
    int       tudobem = TRUE;
    STRING    netname;

    for (busptr = circuit->buslist; busptr; busptr = busptr->next)
    for (netrefptr = busptr->netref; netrefptr; netrefptr = netrefptr->next)
    {
	netname = (STRING)netrefptr->net;
	for (net = circuit->netlist; net; net = net->next)
	    if (net->name == netname) break;

	if (!net) {
	    /* looked at all nets, but to no avail... */
	    if (tudobem) /* only print this message the first time */
		sdfreport (Error, "in Circuit (%s(%s(%s))):",
		  circuit->name, circuit->function->name, circuit->function->library->name);
	    tudobem = 0;
	    sdfreport (Error, "NetRef in Bus \"%s\" to non-existing Net \"%s\"\n",
		busptr->name, netname);
	}
	else {
	    fs (netname);	  /* free the string made canonic by sdfparse() */
	    netrefptr->net = net; /* replace the string by a hard NETPTR */
	}
    }
    return (tudobem);
}

/* This one thinks of a name for the scratch file.
 */
PRIVATE STRING sdfmakescratchname (void)
{
    static STRING scratchname = NULL;
    return (!scratchname ? (scratchname = cs ("@sdfscratch@")):scratchname);
}

/* open the seadif database
 */
int sdfopen (void)
{
    STRING scratchname = sdfmakescratchname ();
    int parseresult;

    if (sdfNumberOfTimesClosed != sdfNumberOfTimesOpened)
	sdfreport (Fatal, "must sdfclose() before calling sdfopen() again!");
    else if (sdfNumberOfTimesOpened > 0)
	sdfcleanupdatastructures ();

    sdfinitsignals ();

    if (!(sdfcopystream = fopen (scratchname, "w+")))
	sdfreport (Fatal, "Cannot open scratch file \"%s\"", scratchname);

    sdffileinfo[0].name = scratchname;
    sdffileinfo[0].fdes = sdfcopystream;
    parseresult = sdfparseandmakeindex ();
    sdfNumberOfTimesOpened += 1;
    return parseresult;
}

/* Clean up all data structures, so that we can sdfopen() the database
 * even if it has been openen and closed before.
 */
PRIVATE void sdfcleanupdatastructures ()
{
   extern char sdfcurrentfileidx;  /* index for sdffileinfo[] */
   extern char sdflastfileinfoidx; /* maximum index used with sdffileinfo[] */

   sdfclearlibhashtable ();
   sdfclearfunhashtable ();
   sdfclearcirhashtable ();
   sdfclearlayhashtable ();
   sdfroot.library = NULL;
   if (sdfroot.filename) fs (sdfroot.filename);
   sdfroot.filename = NULL;
   sdflib = NULL;

   for (; sdfcurrentfileidx >= 0; --sdfcurrentfileidx)
   {
      if (sdffileinfo[(int)sdfcurrentfileidx].name)
	 cs (sdffileinfo[(int)sdfcurrentfileidx].name);
      if (sdffileinfo[(int)sdfcurrentfileidx].lockname)
	 cs (sdffileinfo[(int)sdfcurrentfileidx].lockname);
   }
   sdfcurrentfileidx = sdflastfileinfoidx = 0;
}

/* close the seadif database
 */
void sdfclose (void)
{
    int j;
    LIBTABPTR  lib, lib2;
    char      *tmpname, *origname, fileidx, *sdftmpnames[MAXFILES+5];
    FILEPTR    indexstream;
    STRING     indexfile;
    extern int sdfverbose;

    for (j = 0; j <= sdflastfileinfoidx; ++j) sdftmpnames[j] = NULL;

    for (lib = sdflib; lib; lib = lib->next)
    {
	fileidx = lib->info.file;
	if ((sdffileinfo[(int)fileidx].state & SDFCLOSED)) continue; /* we already closed this one */
	if ((sdffileinfo[(int)fileidx].state & SDFUPDATED))continue; /* we already closed this one */
	if (!(sdffileinfo[(int)fileidx].state & SDFDIRTY)) continue;
	origname = sdffileinfo[(int)fileidx].name;

	if (sdffileinfo[(int)fileidx].readonly) {
	    sdfreport (Warning, "sdfclose: read-only file \"%s\" will NOT be updated...", origname);
	    sdfreport (Warning, "(actually, this is an internal error I think)");
	    sdffileinfo[(int)fileidx].state |= SDFUPDATED;
	    continue;
	}

	if (sdfverbose > 0)
	    fprintf (stderr, "[%d] updating seadif file \"%s\"\n", fileidx, origname);

	/* DON'T LAUGH: some mysterious bug in stdio causes fseek() to fail,
	 * probably because we use lseek, read and fseek intermixed. One way
	 * to remedy this is to fclose and then immediately fopen the file:
	 */
	if (fclose (sdffileinfo[(int)fileidx].fdes))
	    sdfreport (Fatal, "sdfclose: cannot (preliminary) close file \"%s\"", origname);

	if (!(sdffileinfo[(int)fileidx].fdes = fopen (origname, "r")))
	    sdfreport (Fatal, "sdfclose: cannot reopen file \"%s\"", origname);

	sdftmpnames[(int)fileidx] = tmpname = sdfmaketmpname (origname);

	if (!(sdfcopystream = fopen (tmpname, "w")))
	    sdfreport (Fatal, "sdfclose: cannot open tmp file (file %s, errno %d)", tmpname, errno);

	indexfile = mkindexfilename (origname);

	unlink (indexfile);
	if (!(indexstream = fopen (indexfile, "w")))
	    sdfreport (Warning, "sdfclose: cannot write index file \"%s\"", indexfile);

	fprintf (sdfcopystream, "(Seadif \"%s\"", sdfgimesomename ());
	sdfindentthismuch = sdfindentlevel = sdfdumpspacing;

	for (lib2 = lib; lib2; lib2 = lib2->next)
	    /* now look for all libs that reside in the same file */
	    if (lib2->info.file == fileidx && !(lib2->info.state & SDFCLOSED)) {
		if (sdfverbose > 0) fprintf (stderr, " (%s)", lib2->name);
		sdfcopyallthisstuff (lib2, indexstream);
		lib2->info.state &= SDFCLOSED;
	    }
	if (sdfverbose > 0) fprintf (stderr, "\n");

	/* close Seadif */
	sdfindentthismuch = 0;
	putcindented (')', sdfcopystream);
	putc ('\n', sdfcopystream);
	if (fclose (sdfcopystream))
	    sdfreport (Warning, "sdfclose: cannot close tmp file (file %s, errno %d)", tmpname, errno);

	if (indexstream) {
	    if (fclose (indexstream))
		sdfreport (Warning, "sdfclose: cannot close index file %s", indexfile);
	    if (chmod (indexfile, 0444) == -1)
		sdfreport (Warning, "sdfclose: cannot chmod \"%s\"\n", indexfile);
	}
	sdffileinfo[(int)fileidx].state |= SDFUPDATED;

	/* This would be a natural place for fclose(sdffileinfo[fileidx].fdes)
	 * but it is a bad idea when sdfattachlib is being used because we
	 * may still need some part of this file when we write the new "attached" file.
	 * if (fclose(sdffileinfo[fileidx].fdes))
	 *    err (5, "sdfclose: cannot close seadif file");
	 * sdffileinfo[fileidx].state |= SDFCLOSED;
	 */
    }

    /* close all files and rename all dirty files */
    for (j = 1; j <= sdflastfileinfoidx; ++j)
    {
	if (!(sdffileinfo[j].state & SDFCLOSED)) {
	    if (sdffileinfo[j].state & SDFDIRTY) {
		origname = sdffileinfo[j].name;
		if (unlink (origname))
		    sdfreport (Warning, "sdfclose: cannot unlink file %s", origname);

		if (!(tmpname = sdftmpnames[j])) continue; /* no libs in this file */

		if (link (tmpname, origname))
		    sdfreport (Warning, "sdfclose: cannot link file %s to file %s", tmpname, origname);
		if (unlink (tmpname))
		    sdfreport (Warning, "sdfclose: cannot unlink file %s", tmpname);
	    }
	    if (fclose (sdffileinfo[j].fdes))
		sdfreport (Warning, "sdfclose: Cannot close file \"%s\"\n", sdffileinfo[j].name);
	}
	if (sdffileinfo[j].lockname)
	    if (unlink (sdffileinfo[j].lockname))
		sdfreport (Warning, "sdfclose: cannot unlink lock file %s", sdffileinfo[j].lockname);
    }

    /* close and remove the scratch file */
    fclose (sdffileinfo[0].fdes); /* ignore errors */
    unlink (sdffileinfo[0].name);
    sdfNumberOfTimesClosed += 1;
}

#define IDXNAMELEN 257

STRING mkindexfilename (STRING origname)
{
    STRING suffix;
    char indexfilename[IDXNAMELEN+1];
    int len;

    if ((suffix = strrchr (origname, '.'))) {
	len = suffix - origname;
	if (len > IDXNAMELEN) goto err;
	strncpy (indexfilename, origname, len);
    }
    else {
	len = strlen (origname);
	if (len > IDXNAMELEN) goto err;
	strcpy (indexfilename, origname);
    }

    if (len + strlen (SEADIFIDXSUFFIX) + 1 > IDXNAMELEN) goto err;
    strcpy (indexfilename + len, "."); ++len;
    strcpy (indexfilename + len, SEADIFIDXSUFFIX);

    return (cs (indexfilename));
err:
    sdfreport (Fatal, "too long name");
    return (NULL);
}

PRIVATE void sdfcopyallthisstuff (LIBTABPTR libt, FILEPTR indexstream)
{
    FUNTABPTR funt;
    CIRTABPTR cirt;
    LAYTABPTR layt;

    if (!libt) return;
    if (libt->info.state & SDFREMOVED) return; /* this lib is removed, do not copy */

    putcindented ('\n', sdfcopystream);
    sdfinsertthing (&libt->info, libt->name, TRUE, "Function", indexstream, "B", libt->alias);

    for (funt = libt->function; funt; funt = funt->next)
    {
	if (funt->info.state & SDFREMOVED) continue; /* this function is removed, do not copy */
	putcindented ('\n', sdfcopystream);
	sdfinsertthing (&funt->info, funt->name, TRUE, "Circuit", indexstream, "F", funt->alias);

	for (cirt = funt->circuit; cirt; cirt = cirt->next)
	{
	    if (cirt->info.state & SDFREMOVED) continue; /* this circuit is removed, do not copy */
	    putcindented ('\n', sdfcopystream);
	    sdfinsertthing (&cirt->info, cirt->name, TRUE, "Layout", indexstream, "C", cirt->alias);

	    for (layt = cirt->layout; layt; layt = layt->next)
	    {
		if (layt->info.state & SDFREMOVED) continue; /* this layout is removed, do not copy */
		putcindented ('\n', sdfcopystream);
		sdfinsertthing (&layt->info, layt->name, 0, "", indexstream, "L", layt->alias);
	    }
	    putcindented (')', sdfcopystream);
	}
	putcindented (')', sdfcopystream);
    }
    putcindented (')', sdfcopystream);
}

PRIVATE STRING sdfgimesomename (void)
{
    return (canonicstring ("Doktor Fremdenliebe"));
}

/* Seek to position pointed to by frominfo and copy (append) that S-expression to
 * sdfcopystream. End of the S-expression is recognized when '(' and ')' are balanced.
 */
void sdfinsertthing (SDFINFO *frominfo, STRING name, int skipclosingparen,
		STRING skipthissexp, FILEPTR indexstream, STRING indexid, STRING alias)
/* indexid identifies type of object in index file */
{
    FILEPTR fromstream;
    long offset = frominfo->fpos;
    int c, instring, bracecount, fastparse;

    fromstream = SdfStreamInfo ((*frominfo));

    if (fromstream == sdfcopystream)
	sdfreport (Fatal, "sdfinsertthing: cannot copy fromstream onto itself");

    /* need fseek because we use getc() ...: */
    if (!FSEEK (fromstream, offset))
	sdfreport (Fatal, "sdfinsertthing: fseek to S-expression failed (file %s, offset)",
	    sdffileinfo[(int)frominfo->file].name, offset);

    if ((c = getc (fromstream)) != '(')
	sdfreport (Fatal, "sdfinsertthing: S-expression does not start with '('\n"
	    "(file %s, offset %d)", sdffileinfo[(int)frominfo->file].name, offset);

    bracecount = 1; /* this one counts braces */
    instring = 0;
    putcindentedtellmewhatposition = TRUE;

    while (bracecount >= 1)
    {
	if (c) putcindented (c, sdfcopystream);
	c = getc (fromstream);
	if (c == '(') {
	    if (!instring) {
		fastparse = frominfo->state & SDFFASTPARSE;
		sdflookaheadforsexpandskip (sdfcopystream, fromstream, &c, skipthissexp, &fastparse);
		if (fastparse) break; /* encountered skipthissexp */
		if (c) ++bracecount; /* did not skip */
	    }
	}
	else if (c == ')') {
	    if (!instring) --bracecount;
	}
	else if (c == '"') {
	    instring = !instring;
	}
	else if (c == EOF)
	    sdfreport (Fatal, "sdfinsertthing: unexpected end-of-file (file %s)",
		sdffileinfo[(int)frominfo->file].name);
    }

    if (!skipclosingparen) putcindented (c, sdfcopystream);

    /* write a line to the index file: */
    if (indexstream) {
	if (!alias)
	    fprintf (indexstream, "%s\t%s\t%ld\t1\n", indexid, name, putcindentedthisistheposition);
	else
	    fprintf (indexstream, "%s\t%s\t%ld\t1\t%s\n", indexid, name, putcindentedthisistheposition, alias);
    }
}

#define MAXCBUF 257

/* Skip spaces, then check if the keyword in the fromstream matches sexp.
 * If so, skip the entire S-expression (don't copy it onto the tostream) and
 * return 0 in lastchar. If not, flush (first part of) the mismatching
 * keyword into the tostream and return in 'lastchar' the last character read
 * but not yet flushed. The number of chars in sexp should not exceed MAXCBUF.
 *
 * The boolean fastparse modifies the behavior as follows: If it is TRUE
 * and the keyword on the fromstream matches sexp, then return immmediately.
 * Else set fastparse to 0 and behave as described above.
 */
PRIVATE void sdflookaheadforsexpandskip (FILEPTR tostream, FILEPTR fromstream,
				int *lastchar, STRING sexp, int *fastparse)
{
    char cbuf[MAXCBUF+1], *bufptr, *p;
    int c = *lastchar, d, bracecount;

    bufptr = cbuf;
    *bufptr++ = c;

    /* First, skip all white spaces (do not copy, what 's the use of the spaces?) */
    while ((c = getc (fromstream)) == ' ' || c == '\t' || c == '\n') ;

    if (c == EOF)
	sdfreport (Fatal, "sdflookaheadforsexpandskip: unexpected end-of-file");

    /* Now look for a match */
    while ((d = *sexp++) && d == c) {
	*bufptr++ = c;
	c = getc (fromstream);
    }

    if (c == EOF)
	sdfreport (Fatal, "sdflookaheadforsexpandskip: unexpected end-of-file");

    if (!d && (c == ' ' || c == '\n' || c == '\t'))
    {
	/* Found an exact match. */
	if (*fastparse) return;

	/* now skip this thing... */
	for (bracecount = 1; bracecount > 0;)
	    if ((c = getc (fromstream)) == '(') ++bracecount;
	    else if (c == ')') --bracecount;
	    else if (c == EOF)
		sdfreport (Fatal, "sdflookaheadforsexpandskip: unexpected end-of-file");

	/* and also skip the trailing white spaces */
	while ((c = getc (fromstream)) == ' ' || c == '\t' || c == '\n') ;

	/* read one char too many */
	ungetc (c, fromstream);
	c = 0; /* this is the value to return to the caller */
    }
    else {
	/* no match, copy the content of cbuf to the tostream */
	for (p = cbuf; p < bufptr; ++p) {
	    d = *p;
	    putcindented (d, tostream);
	}
    }
    *fastparse = 0;
    *lastchar = c;
}

PRIVATE int sdfhashcir (CIRCUITPTR cir)
{
    FUNCTIONPTR f = cir->function, fptr;
    LIBRARYPTR  l = f->library, lptr;
    /* This is a circuit that's not already in the data base. Create an entry. */
    SDFINFO info;

    /* don't know this circuit -- maybe we know the function? */
    if (!existsfun (f->name, l->name))
    {
	/* don't know this function -- maybe we know the library? */
	SDFINFO funinfo;

	if (!existslib (l->name)) {
	    /* even don't know the lib. Question: what file goes the new lib in? */
	    SDFINFO libinfo;

	    libinfo.file = info.file = sdffindfileforlib (l->name);
	    libinfo.state = 0;
	    libinfo.fpos = 0;
	    libinfo.what = SDFLIBBODY;
	    if (l->status) libinfo.what |= SDFLIBSTAT;
	    addlibtohashtable (l, &libinfo);
	    if (!sdfwritelibnoshit (thislibtab)) return (0);
	}
	else if (!(lptr = thislibtab->library))
	    (thislibtab->library = l, info.file = thislibtab->info.file);
	else if (lptr == l)
	    info.file = thislibtab->info.file;
	else
	    sdfreport (Fatal, "sdfwritecir: library (%s) not properly registered\n", l->name);

	funinfo.file = info.file;
	funinfo.state = 0;
	funinfo.fpos = 0;
	funinfo.what = SDFFUNBODY;
	if (f->status) funinfo.what |= SDFFUNSTAT;
	addfuntohashtable (f, thislibtab, &funinfo);
	if (!sdfwritefunnoshit (thisfuntab)) return (0);
    }
    else if (!(fptr = thisfuntab->function))
	(thisfuntab->function = f, info.file = thisfuntab->info.file);
    else if (fptr == f)
	info.file = thisfuntab->info.file;
    else
	sdfreport (Fatal, "sdfwritecir: function (%s(%s)) not properly registered\n", f->name, l->name);

    info.what = SDFCIRBODY;
    info.state = 0;
    info.fpos = 0;
    if (cir->status)  info.what |= SDFCIRSTAT;
    if (cir->netlist) info.what |= SDFCIRNETLIST;
    if (cir->cirinst) info.what |= SDFCIRINST;
    if (cir->cirport) info.what |= SDFCIRPORT;
    addcirtohashtable (cir, thisfuntab, &info);
    return TRUE;
}

int sdftouchcir (CIRCUITPTR cir, time_t timestamp)
{
    if (!existscir (cir->name, cir->function->name, cir->function->library->name))
    {
	sdfhashcir (cir); /* create an entry in the hash tables */
	if (!sdfwritecirnoshit (thiscirtab)) return 0;
    }
    thiscirtab->info.timestamp = timestamp;
    thiscirtab->info.state |= SDFTOUCHED;
    return TRUE;
}

int sdfwritecir (int what, CIRCUITPTR cir)
{
    CIRCUITPTR  cptr;
    FUNCTIONPTR f = cir->function;
    LIBRARYPTR  l = f->library;
    FILEPTR     stream;
    long offset, eofpos;
    int idx;

    what &= SDFCIRALL;      /* only handle circuit stuff */

    if (!existscir (cir->name, f->name, l->name)) {
	sdfhashcir (cir);
	/* now that cir is in the hashtables we can write it onto the scratch file */
	if (!sdfwritecirnoshit (thiscirtab)) return (0);
	return (TRUE);
    }

    if (!(cptr = thiscirtab->circuit))
	thiscirtab->circuit = cir;
    else if (cptr != cir)
	sdfreport (Fatal, "sdfwritecir: circuit (%s(%s(%s))) not properly registered\n",
	    cir->name, f->name, l->name);

    idx = SdfStreamIdx (thiscirtab);

    if (what == SDFCIRALL) {
	/* special case, we don't need to read and write. Just dump the circuit */
	if (!sdfwritecirnoshit (thiscirtab)) return (0);
	return (TRUE);
    }

    if ((stream = sdffileinfo[idx].fdes) == sdfcopystream)
	sdfgetsecondscratchstream (&stream, &idx);

    offset = thiscirtab->info.fpos;
    if (!SEEK (stream, offset))
	sdfreport (Fatal, "sdfwritecir: seek to circuit failed (file %s, offset %d)", SdfStreamName (thiscirtab), offset);
    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of scratch file */
	sdfreport (Fatal, "sdfwritecir: seek to end of scratch file %s failed", sdfmakescratchname ());
    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritecir: tell on scratch file %s failed", sdfmakescratchname ());

    /* always delete layout: */
    sdfstuff = (what | SDFLAYBODY) & ~SDFCIRBODY;
    sdfwrite = what & ~SDFCIRBODY;
    sdfwhat = 0;        /* don't read a thing */
    sdfwritethiscir = cir;
    sdfcopytheinput = TRUE;
    sdfparseonecir = TRUE;
    sdffunislastthinginlib = sdfcirislastthinginfun = 0;
    sdflayislastthingincir = (thiscirtab->info.state & SDFFASTPARSE);
    sdfobligetimestamp = (thiscirtab->info.state & SDFTOUCHED);
    sdftimestamp = thiscirtab->info.timestamp;

    if (sdfparse (idx)) sdfreport (Fatal, "parse of circuit failed (file %s)", SdfStreamName (thiscirtab));

    sdfobligetimestamp = 0;
    sdflayislastthingincir = sdfparseonecir = 0;
    sdfcopytheinput = 0;

    if (sdffileinfo[idx].readonly) {
	sdfreport (Warning, "sdfwritecir: attempt to write read-only file %s", sdffileinfo[idx].name);
	return (0);
    }

    /* Administrate the new location of this circuit, don't destroy the original file name */
    thiscirtab->info.fpos = eofpos;   /* ...but only administrate the new file position... */

    /* This implies that info.file = scratchfile: */
    thiscirtab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    sdffileinfo[(int)thiscirtab->info.file].state |= SDFDIRTY;
    return (TRUE);
}

/* Create a hash table entry for LIB and set the global thislibtab to it.
 */
PRIVATE int sdfhashlib (LIBRARYPTR lib)
{
    SDFINFO libinfo;

    /* Don't know this lib. Question: what file goes the new lib in? */
    libinfo.file = sdffindfileforlib (lib->name);
    libinfo.what = SDFLIBBODY;
    libinfo.state = 0;
    libinfo.fpos = 0;
    if (lib->status) libinfo.what |= SDFLIBSTAT;
    addlibtohashtable (lib, &libinfo);
    return TRUE;
}

int sdftouchlib (LIBRARYPTR lib, time_t timestamp)
{
    if (!existslib (lib->name)) {
	sdfhashlib (lib); /* create a hash table entry if it does not exist */
	if (!sdfwritelibnoshit (thislibtab)) return 0;
    }
    thislibtab->info.timestamp = timestamp;
    thislibtab->info.state |= SDFTOUCHED;
    return TRUE;
}

int sdfwritelib (int what, LIBRARYPTR lib)
{
    LIBRARYPTR  lptr;
    FILEPTR     stream;
    long offset, eofpos;
    int idx;

    what &= SDFLIBALL;      /* only handle library stuff */

    if (!existslib (lib->name)) {
	sdfhashlib (lib);
	if (!sdfwritelibnoshit (thislibtab)) return (0);
	return (TRUE);
    }

    if (!(lptr = thislibtab->library))
	thislibtab->library = lib;   /* caller created her own library in core */
    else if (lptr != lib)
	sdfreport (Fatal, "sdfwritelib: library (%s) not properly registered\n", lib->name);

    idx = SdfStreamIdx (thislibtab);

    if (what == SDFLIBALL) {
	/* special case, we don't need to read and write. Just dump the library */
	if (!sdfwritelibnoshit (thislibtab)) return (0);
	return (TRUE);
    }

    if ((stream = sdffileinfo[idx].fdes) == sdfcopystream)
	sdfgetsecondscratchstream (&stream, &idx);

    offset = thislibtab->info.fpos;
    if (!SEEK (stream, offset))
	sdfreport (Fatal, "sdfwritelib: seek to library failed (file %s, offset %d)", SdfStreamName (thislibtab), offset);
    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of scratch file */
	sdfreport (Fatal, "sdfwritelib: seek to end file %s failed", sdfmakescratchname ());
    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritelib: tell on scratch file %s failed", sdfmakescratchname ());

    /* always delete function: */
    sdfstuff = (what & ~SDFLIBBODY) | SDFFUNBODY;
    sdfwrite = what & ~SDFLIBBODY;
    sdfwhat = 0;        /* do not read anything */
    sdfwritethislib = lib;
    sdfcopytheinput = sdfparseonelib = TRUE;
    sdfcirislastthinginfun = sdflayislastthingincir = 0;
    sdffunislastthinginlib = (thislibtab->info.state & SDFFASTPARSE);
    sdfobligetimestamp = (thislibtab->info.state & SDFTOUCHED);
    sdftimestamp = thislibtab->info.timestamp;

    if (sdfparse (idx))
	sdfreport (Fatal, "sdfwritelib: parse of library failed (file %s)", SdfStreamName (thislibtab));

    sdfcopytheinput = sdfparseonelib = sdffunislastthinginlib = 0;
    sdfobligetimestamp = 0;

    if (sdffileinfo[idx].readonly) {
	sdfreport (Warning, "sdfwritelib: attempt to write read-only file %s", sdffileinfo[idx].name);
	return (0);
    }

    /* Administrate the new location of this library, don't destroy the original file name */
    thislibtab->info.fpos = eofpos;   /* ...but only administrate the new file position... */

    /* This implies that info.file = scratchfile: */
    thislibtab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    sdffileinfo[(int)thislibtab->info.file].state |= SDFDIRTY;
    return (TRUE);
}

/* We need to both read and write the scratchfile, so open an extra
 * stream for reading and keep the sdfcopystream for writing. */
PRIVATE void sdfgetsecondscratchstream (FILEPTR *stream, int *idx)
{
    if (!sdfreadscratchidx)
    {
	/* First time we encounter this problem. Save the new
	 * stream pointer in sdffileinfo[sdfreadscratchidx]
	 */
	FILEPTR readscratchstream;
	if (!(readscratchstream = fopen (sdfmakescratchname (), "r")))
	    sdfreport (Fatal, "sdfwrite: cannot open scratch file \"%s\" for reading", sdfmakescratchname ());
	sdfreadscratchidx = ++sdflastfileinfoidx;
	sdffileinfo[(int)sdfreadscratchidx].fdes = readscratchstream;
	sdffileinfo[(int)sdfreadscratchidx].name = sdfmakescratchname ();
	sdffileinfo[(int)sdfreadscratchidx].state = 0;
    }
    *stream = sdffileinfo[(int)sdfreadscratchidx].fdes;
    *idx = sdfreadscratchidx;
}

PRIVATE int sdfhashfun (FUNCTIONPTR fun)
{
    LIBRARYPTR l = fun->library, lptr;
    /* This is a function that's not already in the data base. Create an entry. */
    SDFINFO info;

    /* don't know this function -- maybe we know the library? */
    if (!existslib (l->name))
    {
	/* even don't know the lib. Question: what file goes the new lib in? */
	SDFINFO libinfo;

	libinfo.file = info.file = sdffindfileforlib (l->name);
	libinfo.what = SDFLIBBODY;
	libinfo.state = 0;
	libinfo.fpos = 0;
	if (l->status) libinfo.what |= SDFLIBSTAT;
	addlibtohashtable (l, &libinfo);
	if (!sdfwritelibnoshit (thislibtab)) return (0);
    }
    else if (!(lptr = thislibtab->library)) /* caller created her own library in core */
	(thislibtab->library = l, info.file = thislibtab->info.file);
    else if (lptr == l)
	info.file = thislibtab->info.file;
    else
	sdfreport (Fatal, "sdfwritefun: library (%s) not properly registered\n", l->name);

    info.what = SDFFUNBODY;
    info.state = 0;
    info.fpos = 0;
    if (fun->status) info.what |= SDFFUNSTAT;
    addfuntohashtable (fun, thislibtab, &info);
    return TRUE;
}

int sdftouchfun (FUNCTIONPTR fun, time_t timestamp)
{
    if (!existsfun (fun->name, fun->library->name)) {
	sdfhashfun (fun);
	if (!sdfwritefunnoshit (thisfuntab)) return 0;
    }
    thisfuntab->info.timestamp = timestamp;
    thisfuntab->info.state |= SDFTOUCHED;
    return TRUE;
}

int sdfwritefun (int what, FUNCTIONPTR fun)
{
    FUNCTIONPTR fptr;
    FILEPTR     stream;
    long offset, eofpos;
    int idx;

    what &= SDFFUNALL;     /* only handle function stuff */

    if (!existsfun (fun->name, fun->library->name)) {
	sdfhashfun (fun);
	if (!sdfwritefunnoshit (thisfuntab)) return (0);
	return (TRUE);
    }

    if (!(fptr = thisfuntab->function))
	thisfuntab->function = fun; /* caller created her own function in core */
    else if (fptr != fun)
	sdfreport (Fatal, "sdfwritefun: function (%s(%s)) not properly registered\n",
	    fun->name, fun->library->name);

    idx = SdfStreamIdx (thisfuntab);

    if (what == SDFFUNALL) {
	/* Special case, we don't need to read and write. Just dump the function. */
	if (!sdfwritefunnoshit (thisfuntab)) return (0);
	return (TRUE);
    }

    if ((stream = sdffileinfo[idx].fdes) == sdfcopystream)
	sdfgetsecondscratchstream (&stream, &idx);

    offset = thisfuntab->info.fpos;
    if (!SEEK (stream, offset))
	sdfreport (Fatal, "sdfwritefun: seek to function failed (file %s, offset %d)", SdfStreamName (thisfuntab), offset);
    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of scratch file */
	sdfreport (Fatal, "sdfwritefun: seek to end of file %s failed", sdfmakescratchname ());
    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritefun: tell on scratch file %s failed", sdfmakescratchname ());

    /* always delete circuit: */
    sdfstuff = (what | SDFCIRBODY) & ~SDFFUNBODY;
    sdfwrite = what & ~SDFFUNBODY;
    sdfwhat = 0;        /* do not read anything */
    sdfwritethisfun = fun;
    sdfcopytheinput = TRUE;
    sdfparseonefun = TRUE;
    sdffunislastthinginlib = sdflayislastthingincir = 0;
    sdfcirislastthinginfun = (thisfuntab->info.state & SDFFASTPARSE);
    sdfobligetimestamp = (thisfuntab->info.state & SDFTOUCHED);
    sdftimestamp = thisfuntab->info.timestamp;

    if (sdfparse (idx)) sdfreport (Fatal, "sdfwritefun: parse of functon failed (file %s)", SdfStreamName (thisfuntab));

    sdfobligetimestamp = 0;
    sdfcirislastthinginfun = sdfparseonefun = 0;
    sdfcopytheinput = 0;

    if (sdffileinfo[idx].readonly) {
	sdfreport (Warning, "sdfwritefun: attempt to write read-only file %s", sdffileinfo[idx].name);
	return (0);
    }

    /* Administrate the new location of this function, don't destroy the original file name */
    thisfuntab->info.fpos = eofpos;   /* ...but only administrate the new file position... */
    /* This implies that info.file = scratchfile: */
    thisfuntab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    sdffileinfo[(int)thisfuntab->info.file].state |= SDFDIRTY;
    return (TRUE);
}

/* This function appends the layout to the sdfcopystream.
 */
PRIVATE int sdfwritelaynoshit (LAYTABPTR laytab)
{
    long eofpos;
    int  oldspacing;

    if (sdffileinfo[(int)laytab->info.file].readonly) {
	sdfreport (Warning, "sdfwritelay: attempt to write layout (%s(%s(%s(%s)))) "
	    "to read-only file \"%s\"",
	    laytab->name, laytab->circuit->name, laytab->circuit->function->name,
	    laytab->circuit->function->library->name,
	    sdffileinfo[(int)laytab->info.file].name);
	return (0);
    }

    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of file */
	sdfreport (Fatal, "sdfwritelaynoshit: seek to end of file %s failed", sdfmakescratchname ());

    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritelaynoshit: tell on file %s failed", sdfmakescratchname ());

    oldspacing = setdumpspacing (0);   /* save spaces in the scratch file */
    sdfobligetimestamp = (laytab->info.state & SDFTOUCHED);
    sdftimestamp = laytab->info.timestamp;
    dump_layout (sdfcopystream, laytab->layout);
    sdfobligetimestamp = 0;
    setdumpspacing (oldspacing);
    laytab->info.fpos = eofpos+1;   /* administrate the new file position... */
    sdffileinfo[(int)laytab->info.file].state |= SDFDIRTY; /* Mark for sdfclose() */
    laytab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    return (TRUE);
}

/* This function appends the circuit to the sdfcopystream.
 */
PRIVATE int sdfwritecirnoshit (CIRTABPTR cirtab)
{
    long eofpos;
    int oldspacing;
    LAYOUTPTR  tmplay;
    CIRCUITPTR cir;

    if (sdffileinfo[(int)cirtab->info.file].readonly) {
	sdfreport (Warning, "sdfwritecir: attempt to write circuit (%s(%s(%s))) "
	    "to read-only file \"%s\"",
	    cirtab->name, cirtab->function->name, cirtab->function->library->name,
	    sdffileinfo[(int)cirtab->info.file].name);
	return (0);
    }

    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of file */
	sdfreport (Fatal, "sdfwritecirnoshit: seek to end of file %s failed", sdfmakescratchname ());

    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritecirnoshit: tell on file %s failed", sdfmakescratchname ());

    /* temporary remove layouts from the circuit -- we only want to dump the circuit */
    tmplay = (cir = cirtab->circuit)->layout;
    cir->layout = NULL;
    sdfobligetimestamp = (cirtab->info.state & SDFTOUCHED);
    sdftimestamp = cirtab->info.timestamp;
    oldspacing = setdumpspacing (0);
    dump_circuit (sdfcopystream, cir);
    setdumpspacing (oldspacing);
    sdfobligetimestamp = 0;
    cir->layout = tmplay;
    cirtab->info.fpos = eofpos+1;   /* administrate the new file position... */
    sdffileinfo[(int)cirtab->info.file].state |= SDFDIRTY; /* Mark for sdfclose() */
    cirtab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    return (TRUE);
}

/* This function appends the function to the sdfcopystream.
 */
PRIVATE int sdfwritefunnoshit (FUNTABPTR funtab)
{
    long eofpos;
    int  oldspacing;
    CIRCUITPTR  tmpcir;
    FUNCTIONPTR fun;

    if (sdffileinfo[(int)funtab->info.file].readonly) {
	sdfreport (Warning, "sdfwritefun: attempt to write function (%s(%s)) "
	    "to read-only file \"%s\"",
	    funtab->name, funtab->library->name, sdffileinfo[(int)funtab->info.file].name);
	return (0);
    }

    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of file */
	sdfreport (Fatal, "sdfwritefunnoshit: seek to end of file %s failed", sdfmakescratchname ());

    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritefunnoshit: tell on file %s failed", sdfmakescratchname ());

    /* temporary remove circuits from the function -- we only want to dump the function */
    tmpcir = (fun = funtab->function)->circuit;
    fun->circuit = NULL;
    sdfobligetimestamp = (funtab->info.state & SDFTOUCHED);
    sdftimestamp = funtab->info.timestamp;
    oldspacing = setdumpspacing (0);
    dump_function (sdfcopystream, fun);
    setdumpspacing (oldspacing);
    sdfobligetimestamp = 0;
    fun->circuit = tmpcir;
    funtab->info.fpos = eofpos+1;   /* administrate the new file position... */
    sdffileinfo[(int)funtab->info.file].state |= SDFDIRTY; /* Mark for sdfclose() */
    funtab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    return (TRUE);
}

/* This function appends the library to the sdfcopystream.
 */
PRIVATE int sdfwritelibnoshit (LIBTABPTR libtab)
{
    long eofpos;
    int  oldspacing;
    FUNCTIONPTR tmpfun;
    LIBRARYPTR  lib;

    if (sdffileinfo[(int)libtab->info.file].readonly) {
	sdfreport (Warning, "sdfwritelib: attempt to write library \"%s\" "
	    "to read-only file \"%s\"", libtab->name, sdffileinfo[(int)libtab->info.file].name);
	return (0);
    }

    if (!FSEEK_TO_END (sdfcopystream)) /* seek to end of file */
	sdfreport (Fatal, "sdfwritelibnoshit: seek to end of file %s failed", sdfmakescratchname ());

    if ((eofpos = FTELL (sdfcopystream)) == -1)
	sdfreport (Fatal, "sdfwritelibnoshit: tell on file %s failed", sdfmakescratchname ());

    /* temporary remove functions from the library -- we only want to dump the library */
    tmpfun = (lib = libtab->library)->function;
    lib->function = NULL;
    sdfobligetimestamp = (libtab->info.state & SDFTOUCHED);
    sdftimestamp = libtab->info.timestamp;
    oldspacing = setdumpspacing (0);
    dump_library (sdfcopystream, lib);
    setdumpspacing (oldspacing);
    sdfobligetimestamp = 0;
    lib->function = tmpfun;
    libtab->info.fpos = eofpos+1;   /* administrate the new file position... */
    sdffileinfo[(int)libtab->info.file].state |= SDFDIRTY; /* Mark for sdfclose() */
    libtab->info.state |= SDFWRITTEN; /* ...and implicitly the new file. */
    return (TRUE);
}

int sdfremovelib (STRING libname)
{
    if (sdfexistslib (libname)) {
	if (sdffileinfo[(int)thislibtab->info.file].readonly) {
	    sdfreport (Error, "sdfremovelib: cannot remove library %s "
		"from read-only file %s", libname, sdffileinfo[(int)thislaytab->info.file].name);
	    return 0;
	}
	thislibtab->info.state |= SDFREMOVED;
	sdffileinfo[(int)thislibtab->info.file].state |= SDFDIRTY;
	return TRUE;
    }
    return 0;
}

int sdfremovefun (STRING funname, STRING libname)
{
    if (sdfexistsfun (funname, libname)) {
	if (sdffileinfo[(int)thisfuntab->info.file].readonly) {
	    sdfreport (Error, "sdfremovefun: cannot remove function (%s(%s)) "
		"from read-only file %s", funname, libname,
		sdffileinfo[(int)thisfuntab->info.file].name);
	    return 0;
	}
	thisfuntab->info.state |= SDFREMOVED;
	sdffileinfo[(int)thisfuntab->info.file].state |= SDFDIRTY;
	return TRUE;
    }
    return 0;
}

int sdfremovecir (STRING cirname, STRING funname, STRING libname)
{
    if (sdfexistscir (cirname, funname, libname)) {
	if (sdffileinfo[(int)thiscirtab->info.file].readonly) {
	    sdfreport (Error, "sdfremovecir: cannot remove circuit (%s(%s(%s))) "
		"from read-only file %s", cirname, funname, libname,
		sdffileinfo[(int)thiscirtab->info.file].name);
	    return 0;
	}
	thiscirtab->info.state |= SDFREMOVED;
	sdffileinfo[(int)thiscirtab->info.file].state |= SDFDIRTY;
	return TRUE;
    }
    return 0;
}

int sdfremovelay (STRING layname, STRING cirname, STRING funname, STRING libname)
{
    if (sdfexistslay (layname, cirname, funname, libname)) {
	if (sdffileinfo[(int)thislaytab->info.file].readonly) {
	    sdfreport (Error, "sdfremovelay: cannot remove layout (%s(%s(%s(%s)))) "
		"from read-only file %s\n", layname, cirname, funname, libname,
		sdffileinfo[(int)thislaytab->info.file].name);
	    return 0;
	}
	thislaytab->info.state |= SDFREMOVED;
	sdffileinfo[(int)thislaytab->info.file].state |= SDFDIRTY;
	return TRUE;
    }
    return 0;
}

#include "src/ocean/libseadif/systypes.h"
#include <sys/stat.h>
#include <errno.h>

/* This one returns the index in the sdffileinfo[] array corresponding
 * to the file where the library 'libname' is in. If the library is
 * unknown it chooses a default file and creates it. Currently the
 * default file name is taken from the environment variable NEWSEALIB.
 * If that one is not available the file DEFAULTNEWSEALIB is created
 * in the current working directory and a warning message is printed
 * on stderr. See also the function sdfattachlib().
 */
PRIVATE int sdffindfileforlib (STRING libname)
{
    static int newsealibidx = 0;

    if (!existslib (libname)) {
	STRING filename, s;

	if (!newsealibidx) {
	    FILEPTR fptr;
	    struct stat statbuf;

	    filename = cs (!(s = getenv (NEWSEALIB)) ? DEFAULTNEWSEALIB : s);
	    if (stat (filename, &statbuf)) {
		if (errno != ENOENT)
		    sdfreport (Fatal, "sdffindfileforlib: cannot stat the default file for new libs %s", filename);
	    }
	    else
		sdfreport (Fatal, "sdffindfileforlib: default file for new libs (%s) already exists -- not overwriting", filename);

	    if (!getenv (NEWSEALIB)) {
		/* ony print a warning if the user had no NEWSEALIB environment ... */
		sdfreport (Warning, "no file specified for newly created library \"%s\"", libname);
		sdfreport (Warning, "using file \"%s\" (does not yet exist)", filename);
	    }
	    if (!(fptr = fopen (filename, "w")))
		sdfreport (Fatal, "sdffindfileforlib: cannot open default file for new lib %s", filename);

	    /* that's been enough testing */
	    newsealibidx = ++sdflastfileinfoidx;
	    sdffileinfo[newsealibidx].fdes = fptr;
	    sdffileinfo[newsealibidx].name = filename;
	    sdffileinfo[newsealibidx].state = 0;
	}
	return (newsealibidx);
    }
    return (thislibtab->info.file);
}

#define MAXNAME 300

/* Bind a library to a specific file. If filename does not start with
 * '/' or "./" then use the environment variable NEWSEADIR as the
 * directory for the new file.
 */
int sdfattachlib (LIBRARYPTR lib, STRING filename)
{
    struct stat statbuf;
    char fname1[MAXNAME+1], *fname;
    int idx, len;

    if (!existslib (lib->name)) {
	SDFINFO info;
	info.state = 0;
	info.what = SDFLIBBODY;
	if (lib->status) info.what |= SDFLIBSTAT;
	info.file = 0;
	info.fpos = 0;
	addlibtohashtable (lib, &info);
    }
    else if (thislibtab->library != lib)
	sdfreport (Fatal, "sdfattachlib: pointer not consistent");
    else if (thislibtab->library->name != lib->name)
	sdfreport (Fatal, "sdfattachlib: names not consistent");
    else if (sdffileinfo[(int)thislibtab->info.file].readonly) {
	sdfreport (Error, "sdfattachlib: attempt to delete library \"%s\" "
	    "from read-only file \"%s\"\n", thislibtab->name,
	    sdffileinfo[(int)thislibtab->info.file].name);
	return (0);
    }
    else /* make file where lib originally comes from dirty */
	sdffileinfo[(int)thislibtab->info.file].state |= SDFDIRTY;

    len = 0;
    if (!(*filename == '/' || (*filename == '.' && *(filename+1) == '/'))) {
	char *s = getenv (NEWSEADIR);
	if (s && (len = strlen (s)) > 0) {
	    if (len+1 > MAXNAME) sdfreport (Fatal, "too long name");
	    strcpy (fname1, s);
	    if (fname1[len-1] != '/') fname1[len++] = '/';
	}
    }
    if (strlen (filename) + len > MAXNAME) sdfreport (Fatal, "too long name");
    strcpy (fname1 + len, filename);

    fname = abscanonicpath (fname1); /* no sym-links, no '..', start with '/' */

    /* do we already have this file ? */
    for (idx = 0; idx <= sdflastfileinfoidx; ++idx)
	if (sdffileinfo[idx].name == fname) break;

    if (idx <= sdflastfileinfoidx) { /* yes, we already opened this file */
	if (sdffileinfo[idx].readonly) {
	    sdfreport (Error, "sdfattachlib: attempt to move library \"%s\" "
		"to read-only file \"%s\"\n", thislibtab->name, sdffileinfo[idx].name);
	    return (0);
	}
	sdffileinfo[idx].state |= SDFDIRTY;
	sdfbindfiletolib (lib->name, idx);
	return (TRUE);
    }

    if (stat (fname, &statbuf) && errno != ENOENT) /* Not a problem if it does not exist */
	sdfreport (Fatal, "sdffindfileforlib: cannot stat the default file for new libs %s", fname);
    if (++sdflastfileinfoidx > MAXFILES-3) /* reserve space for scratch (2x) and newsealib */
	sdfreport (Fatal, "Exceeded maximum allowed number of open files,\nrecompile me with other MAXFILES");

    if (!(sdffileinfo[(int)sdflastfileinfoidx].fdes = fopen (fname, "w"))) {
	int someidx;

	--sdflastfileinfoidx;
	if (errno == EMFILE)
	    sdfreport (Error, "Too many open files (%d), unix won't open \"%s\"", sdflastfileinfoidx+4, fname);
	else
	    sdfreport (Error, "Cannot open \"%s\" for writing", fname);

	someidx = sdffindfileforlib (lib->name);
	sdfreport (Warning, "sdfattachlib: cannot open new file, dumping lib \"%s\" on file \"%s\"",
	    lib->name, sdffileinfo[someidx].name);
	sdffileinfo[someidx].name = fname;
	sdffileinfo[someidx].state |= SDFDIRTY;
	sdfbindfiletolib (lib->name, someidx);
	return (0);
    }

    sdffileinfo[(int)sdflastfileinfoidx].name = fname;
    sdffileinfo[(int)sdflastfileinfoidx].state |= SDFDIRTY;
    sdfbindfiletolib (lib->name, sdflastfileinfoidx);
    return (TRUE);
}

PRIVATE void sdfbindfiletolib (STRING libname, int idx)
{
    FUNTABPTR funt;
    CIRTABPTR cirt;
    LAYTABPTR layt;

    if (!existslib (libname)) sdfreport (Fatal, "sdfbindfiletolib: library %s does not exist", libname);
    if (!sdfwritelibnoshit (thislibtab)) return;

    thislibtab->info.file = idx;

    for (funt = thislibtab->function; funt; funt = funt->next) {
	funt->info.state |= SDFATTACHED;
	for (cirt = funt->circuit; cirt; cirt = cirt->next) {
	    cirt->info.state |= SDFATTACHED;
	    for (layt = cirt->layout; layt; layt = layt->next) layt->info.state |= SDFATTACHED;
	}
    }
}

PRIVATE char *sdfmaketmpname (STRING string)
{
    char tmp[MAXNAME+1], *p;
    int len = strlen (string);

    if (len > MAXNAME) len = MAXNAME;
    strncpy (tmp, string, len); tmp[len] = 0;

    /* It is important not to increase the length of the file name: there is a
     * possibility that this later will be truncated by the operating system
     * and then we are really into trouble... (data loss)
     */
    if ((p = strrchr (tmp, '.'))) *p = '+'; /* replace the ".sdf" with "+sdf" */
    else if ((p = strrchr (tmp, '/'))) *(p+1) = '+'; /* replace first char of basename */
    else tmp[len-1] = 'T'; /* replace last char of name */

    return (canonicstring (tmp));
}

/* This replacement for putc() implements a state machine with two states.
 * The state SDF_INDENT_TRAJECTORY is active just after a newline is received.
 * The state SDF_TEXT_TRAJECTORY is active when a line of text is being written.
 */
void putcindented (int c, FILEPTR stream)
{
    if (sdfindentstate == SDF_INDENT_TRAJECTORY)
    {
	if (c == ')') { /* never start a line with ')' */
	    sdfindentthismuch -= sdfdumpspacing;
	    putc (c, stream);
	}
	else if (c == ' ' || c == '\t' || c == '\n') ;
	else
	{ /* perform indent, insert char and change state to SDF_TEXT_TRAJECTORY */
	    int indent = sdfindentthismuch;

	    putc ('\n', stream);
	    /* first, try to insert TABs as much as possible */
	    while (indent >= TABINDENT) {
		putc ('\t', stream);
		indent -= TABINDENT;
	    }
	    /* finish it with spaces */
	    while (indent > 0) {
		putc (' ', stream);
		--indent;
	    }
	    if (c == '(') {
		if (putcindentedtellmewhatposition) {
		    putcindentedthisistheposition = FTELL (stream);
		    putcindentedtellmewhatposition = 0;
		}
		sdfindentthismuch += sdfdumpspacing;
	    }
	    putc (c, stream);
	    sdfindentstate = SDF_TEXT_TRAJECTORY;
	}
    }
    else /* sdfindentstate == SDF_TEXT_TRAJECTORY */
    {
	if (c == '\n') sdfindentstate = SDF_INDENT_TRAJECTORY;
	else
	{
	    if (c == '(') {
		if (putcindentedtellmewhatposition) {
		    putcindentedthisistheposition = FTELL (stream);
		    putcindentedtellmewhatposition = 0;
		}
		sdfindentthismuch += sdfdumpspacing;
	    }
	    else if (c == ')')
		sdfindentthismuch -= sdfdumpspacing;
	    putc (c, stream);
	}
    }
}

/* sdfparse() does not solve the references to several things within timing
 * view and external objects. Sdfparse only puts a canonicstring containing
 * the name of the referenced thing in the appropriate field. The function
 * sdfsolve_timereferences() transformes the STRING into a real pointer to the
 * appropriate structure. Because it is not possible to read only some parts
 * of timing model into memory and we would like to avoid recursive read of all
 * levels of hierarchy of circuits we allow somebody to read only timing view
 * of the circuit. Then all the references to non existing parts of that
 * circuit will be set to NULL. User must be cautious to avoid writing such
 * incomplete timing view back to the database.
 */
PRIVATE int sdfsolve_timereferences (CIRCUITPTR circuit)
{
    TIMINGPTR timing;
    int tudobem = TRUE;

    for (timing = circuit->timing; timing; timing = timing->next)
    {
	tudobem &= sdfsolve_tmlistreferences (circuit, timing);
	tudobem &= sdfsolve_ttermreferences  (circuit, timing);
	tudobem &= sdfsolve_netmodsreferences(circuit, timing);
	tudobem &= sdfsolve_tpathsreferences (circuit, timing);
	tudobem &= sdfsolve_delasgreferences (circuit, timing);
    }
    return tudobem; /* returns TRUE if everything went right */
}

/* Solves references to cirinstances of current circuit and to
 * their timing models. It's done only when cirinst list has been read.
 */
PRIVATE int sdfsolve_tmlistreferences (CIRCUITPTR circuit, TIMING* timing)
{
    int          tudobem = TRUE;
    CIRINSTPTR   ciPtr = circuit->cirinst;
    TMMODINSTPTR tiPtr = timing->tminstlist;
    TIMINGPTR    tPtr;
    STRING       name;

    if (!ciPtr) /* we don't solve anything, only have to
		replace STRING pointers with NULL pointers */
    {
	for (; tiPtr; tiPtr = tiPtr->next) {
	    fs ((char*)tiPtr->cirinst);
	    fs ((char*)tiPtr->timing);
	    tiPtr->cirinst = NULL;
	    tiPtr->timing = NULL;
	}
    }
    else				  /* we can try to solve it */
    {
	for (; tiPtr; tiPtr = tiPtr->next)
	{
	    name = (STRING)tiPtr->cirinst;
	    for (ciPtr = circuit->cirinst; ciPtr; ciPtr = ciPtr->next)
		if (name == ciPtr->name) break; /* found it */

	    if (!ciPtr)
	    {
		if (tudobem) sdfreport (Error, "in Timing (%s(%s(%s(%s)))):",
					circuit->timing->name,
					circuit->name,
					circuit->function->name,
					circuit->function->library->name);
		tudobem = 0;
		sdfreport (Error, "CirInstRef in TmModInst  \"%s\" to non-existing CirInst \"%s\"\n", tiPtr->name, name);
	    }
	    else /* OK, replace pointers */
	    {
		fs (name);
		tiPtr->cirinst = ciPtr;

		/* now we have to check if the circuit for this instance has this timing model */

		if (!ciPtr->circuit->timing) { /* we have to read it only timing models list */
		    tudobem &= sdfreadcir (SDFCIRTM,
					ciPtr->circuit->name,
					ciPtr->circuit->function->name,
					ciPtr->circuit->function->library->name);
		}

		/* now search for this model */
		name = (STRING)tiPtr->timing;
		for (tPtr = ciPtr->circuit->timing; tPtr; tPtr = tPtr->next)
		    if (name == tPtr->name) break; /* found */

		if (!tPtr) { /* there's no such timing model for this circuit instance */
		    if (tudobem) sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
					timing->name,
					circuit->name,
					circuit->function->name,
					circuit->function->library->name);
		    sdfreport (Error, "TimingRef in TmModInst  \"%s\" to non-existing Timing \"%s\"", tiPtr->name, name);
		}
		else { /* we have also our timing model */
		    fs (name);
		    tiPtr->timing = tPtr;
		}
	    }
	}
    }

    return tudobem;
}

/* Solves references to timing terminals of timing models
 * instances or current circuit's cirports.
 * We assume that if cirports of the circuit are read then
 * timing models instances references are solved.
 */
PRIVATE int sdfsolve_ttermreferences (CIRCUITPTR circuit, TIMING* timing)
{
    int          tudobem = TRUE;
    CIRPORTPTR   cpPtr = circuit->cirport;
    CIRPORTREFPTR crPtr;
    TIMETERMPTR  tPtr, ttPtr = timing->t_terms;
    TIMETERMREFPTR trPtr;
    TMMODINSTPTR  tmPtr;
    STRING       name;

    if (!cpPtr) /* cirports not read - cannot solve anything */
    {
	for (; ttPtr; ttPtr = ttPtr->next)
	{
	    for (crPtr = ttPtr->cirportlist; crPtr; crPtr = crPtr->next) {
		fs ((char*)crPtr->cirport);
		crPtr->cirport = NULL;
	    }
	    for (trPtr = ttPtr->termreflist; trPtr; trPtr = trPtr->next) {
		fs ((char*)trPtr->inst);
		fs ((char*)trPtr->term);
		trPtr->inst = NULL;
		trPtr->term = NULL;
	    }
	}
    }
    else /* let's try to resolve them */
    {
	/* first cirports */

	for (; ttPtr; ttPtr = ttPtr->next)
	{
	    for (crPtr = ttPtr->cirportlist; crPtr; crPtr = crPtr->next)
	    {
		name = (STRING)crPtr->cirport;
		for (cpPtr = circuit->cirport; cpPtr; cpPtr = cpPtr->next)
		    if (name == cpPtr->name) break; /* found */

		if (!cpPtr) { /* error */
		    if (tudobem)
			sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
				timing->name,
				circuit->name,
				circuit->function->name,
				circuit->function->library->name);
		    tudobem = 0;
		    sdfreport (Error, "CirPortRef in TimeTerm \"%s\" to"
				" non-existing CirPort \"%s\"", ttPtr->name, name);
		}
		else {
		    fs (name);
		    crPtr->cirport = cpPtr;
		}
	    }
	}

	/* now time terminals' references */
	for (ttPtr = timing->t_terms; ttPtr; ttPtr = ttPtr->next)
	for (trPtr = ttPtr->termreflist; trPtr; trPtr = trPtr->next)
	{
	    /* first try to resolve reference to timing model's instance */
	    name = (STRING)trPtr->inst;
	    for (tmPtr = timing->tminstlist; tmPtr; tmPtr = tmPtr->next)
		if (name == tmPtr->name) break;

	    if (!tmPtr) /* not found */
	    {
		if (tudobem)
		    sdfreport (Error, "in Timing (%s(%s(%s(%s)))):",
			timing->name,
			circuit->name, circuit->function->name,
			circuit->function->library->name);
		tudobem = 0;
		sdfreport (Error, "TimeTermRef in TimeTerm \"%s\" to"
			" non-existing TmModInst \"%s\"", ttPtr->name, name);
	    }
	    else
	    {
		fs (name);
		trPtr->inst = tmPtr;
		/* and now to this tm instance's terminal */

		if (!tmPtr->timing) { /* timing model for that tm instance */
		    /* this should never happen */
		    if (tudobem)
			sdfreport (Fatal, "Internal error in Timing (%s(%s(%s(%s))))",
				timing->name,
				circuit->name, circuit->function->name,
				circuit->function->library->name);
		    tudobem = 0;
		}
		else {
		    name = (STRING)trPtr->term;
		    for (tPtr = tmPtr->timing->t_terms; tPtr; tPtr = tPtr->next)
			if (tPtr->name == name) break;

		    if (!tPtr) { /* error no such terminal */
			if (tudobem)
			    sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
				timing->name,
				circuit->name, circuit->function->name,
				circuit->function->library->name);
			tudobem = 0;
			sdfreport (Error, "TimeTermRef in TimeTerm \"%s\" to"
			    " non-existing TimeTerm \"%s\"", ttPtr->name, name);
		    }
		    else {
			fs (name);
			trPtr->term = tPtr;
		    }
		}
	    }
	}
    }

    return tudobem;
}

/* Solves references to nets or buses.
 */
PRIVATE int sdfsolve_netmodsreferences (CIRCUITPTR circuit, TIMING* timing)
{
    int          tudobem = TRUE;
    STRING       name;
    NETREFPTR    nrPtr;
    BUSREFPTR    brPtr;
    NETMODPTR    nmPtr = timing->netmods;
    NETPTR       nPtr;
    BUSPTR       bPtr;

    if (!circuit->netlist || !circuit->buslist)
    {
	/* we cannot resolve anything just zero the references */

	for (; nmPtr; nmPtr = nmPtr->next)
	{
	    for (nrPtr = nmPtr->netlist; nrPtr; nrPtr = nrPtr->next) {
		fs ((char*)nrPtr->net);
		nrPtr->net = NULL;
	    }
	    for (brPtr = nmPtr->buslist; brPtr; brPtr = brPtr->next) {
		fs ((char*)brPtr->bus);
		brPtr->bus = NULL;
	    }
	}
    }
    else
    {
	/* we can try ... */

	for (; nmPtr; nmPtr = nmPtr->next)
	{
	    for (nrPtr = nmPtr->netlist; nrPtr; nrPtr = nrPtr->next)
	    {
		name = (STRING)nrPtr->net;
		for (nPtr = circuit->netlist; nPtr; nPtr = nPtr->next)
		    if (nPtr->name == name) break;

		if (!nPtr) { /* have not found anything */
		    if (tudobem)
			sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
				timing->name,
				circuit->name,
				circuit->function->name,
				circuit->function->library->name);
		    tudobem = 0;
		    sdfreport (Error, "NetRef in NetMod \"%s\" to"
				" non-existing Net \"%s\"", nmPtr->name, name);
		}
		else {
		    fs (name);
		    nrPtr->net = nPtr;
		}
	    }

	    for (brPtr = nmPtr->buslist; brPtr; brPtr = brPtr->next)
	    {
		name = (STRING)brPtr->bus;
		for (bPtr = circuit->buslist; bPtr; bPtr = bPtr->next)
		    if (bPtr->name == name) break;

		if (!bPtr) { /* have not found anything */
		    if (tudobem)
			sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
				timing->name,
				circuit->name,
				circuit->function->name,
				circuit->function->library->name);
		    tudobem = 0;
		    sdfreport (Error, "BusRef in NetMod \"%s\" to"
				" non-existing Bus \"%s\"", nmPtr->name, name);
		}
		else {
		    fs (name);
		    brPtr->bus = bPtr;
		}
	    }
	}
    }

    return tudobem;
}

/* Solves references to time terminals within TPaths.
 * Because these should be only reference to the current
 * models terminals we should not encounter here any problems.
 */
PRIVATE int sdfsolve_tpathsreferences (CIRCUITPTR circuit, TIMING* timing)
{
    int          tudobem = TRUE;
    TPATHPTR     tpPtr = timing->tPaths;
    TIMETERMPTR  ttPtr;
    TIMETERMREFPTR trPtr;
    STRING       name;

    for (; tpPtr; tpPtr = tpPtr->next)
    {
	/* first start terminals */

	for (trPtr = tpPtr->startTermList; trPtr; trPtr = trPtr->next)
	{
	    name = (STRING)trPtr->term;
	    for (ttPtr = timing->t_terms; ttPtr; ttPtr = ttPtr->next)
		if (ttPtr->name == name) break;

	    if (!ttPtr) { /* no such terminal */
		if (tudobem)
		    sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
			timing->name,
			circuit->name,
			circuit->function->name,
			circuit->function->library->name);
		tudobem = 0;
		sdfreport (Error, "TimeTermRef in TPath \"%s\" to"
			" non-existing TimeTerm \"%s\"", tpPtr->name, name);
	    }
	    else {
		fs (name);
		trPtr->term = ttPtr;
	    }
	}
	/* and end terminals */

	for (trPtr = tpPtr->endTermList; trPtr; trPtr = trPtr->next)
	{
	    name = (STRING)trPtr->term;
	    for (ttPtr = timing->t_terms; ttPtr; ttPtr = ttPtr->next)
		if (ttPtr->name == name) break;

	    if (!ttPtr) { /* no such terminal */
		if (tudobem)
		    sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
			timing->name,
			circuit->name,
			circuit->function->name,
			circuit->function->library->name);
		tudobem = 0;
		sdfreport (Error, "TimeTermRef in TPath \"%s\" to"
			" non-existing TimeTerm \"%s\"", tpPtr->name, name);

	    }
	    else {
		fs (name);
		trPtr->term = ttPtr;
	    }
	}
    }

    return tudobem;
}

/* This last routine solves references to tpath and to
 * its point which is selected.
 */
PRIVATE int sdfsolve_delasgreferences (CIRCUITPTR circuit, TIMING* timing)
{
    int          tudobem = TRUE;
    DELASG      *daPtr = timing->delays;
    DELASGINST  *daiPtr;
    TPATHPTR    tpPtr;
    TCPOINTPTR  tcpPtr;
    STRING      name;

    /* scanning all delay assignments */
    for (; daPtr; daPtr = daPtr->next)
    {
	/* visit all paths */
	for (daiPtr = daPtr->pathDelays; daiPtr; daiPtr = daiPtr->next)
	{
	    name = (STRING)daiPtr->tPath;

	    for (tpPtr = timing->tPaths; tpPtr; tpPtr = tpPtr->next)
		if (tpPtr->name == name) break;

	    if (!tpPtr) { /* no such path */
		if (tudobem)
		    sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
			timing->name,
			circuit->name,
			circuit->function->name,
			circuit->function->library->name);
		tudobem = 0;
		sdfreport (Error, "TPathRef in DelAsgInst \"%s\" to"
			" non-existing TPath \"%s\"", daiPtr->name, name);
	    }
	    else { /* OK, found it */
		fs (name);
		daiPtr->tPath = tpPtr;

		/* now when know which path it is lets
		 * find the selected point
		 */
		name = (STRING)daiPtr->selected;

		for (tcpPtr = tpPtr->timeCost->points; tcpPtr; tcpPtr = tcpPtr->next)
		    if (tcpPtr->name == name) break;

		if (!tcpPtr) { /* no such point */
		    if (tudobem)
			sdfreport (Error, "Error in Timing (%s(%s(%s(%s)))):",
				timing->name,
				circuit->name,
				circuit->function->name,
				circuit->function->library->name);
		    tudobem = 0;
		    sdfreport (Error, "TcPointRef in DelAsgInst \"%s\" to"
				" non-existing TcPoint \"%s\"", daiPtr->name, name);
		}
		else {
		    fs (name);
		    daiPtr->selected = tcpPtr; /* done */
		}
	    }
	}
    }

    return tudobem;
}

/* Set various Seadif options.
 */
void sdfoptions (seadifOptions opt)
{
    extern int sdfprintwarnings;
    extern int sdfmakelockfiles;

    switch (opt)
    {
    case SdfWarningsOff:
	sdfprintwarnings = 0;
	break;
    case SdfWarningsOn:
	sdfprintwarnings = TRUE;
	break;
    case SdfLockfilesOff:
	sdfmakelockfiles = 0;
	break;
    case SdfLockfilesOn:
	sdfmakelockfiles = TRUE;
	break;
    default:
	sdfreport (Fatal, "illegal argument to sdfoptions()");
	break;
    }
}
