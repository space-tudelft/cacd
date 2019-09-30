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
 * Print a seadif data structure in ascii. Allows several styles for
 * printing, see the public functions setdumpspacing, setdumpstyle,
 * setobjectiveslice, setcompactslice, seterrors and setcomments.
 * You can also print any Seadif substructure by means of the public
 * functions dump_status, dump_library, ..., dump_wirelist (etc).
 */

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include <stdio.h>
#include <time.h>		/* for status->timestamp conversion */

#include "src/ocean/libseadif/sea_decl.h"

#define SP (3)        /* Default number of spaces for new indent level. */
#define MAXSP (8)     /* Do not allow spacing > 8. */
#define STYLE_NEWLINE_RPAR (1)    /* Put right parenthese on a new line, all by itself. */
#define STYLE_LISP         (0)    /* No \n before ')'. Also the default spacing style. */

#define DoIndnt(fp) { int jj; putc ('\n', fp); for (jj = sdfindentlevel; jj > 0; --jj) putc (' ', fp); }
#define OptIndnt(fp) { if (sdfdumpstyle == STYLE_NEWLINE_RPAR) DoIndnt (fp); }

extern  LIBTABPTR sdflib;
extern  SDFFILEINFO sdffileinfo[];

FILEPTR dbstderr; /* for use in the debugger... */
int sdfindentlevel;    /* Column to start printing in, see DoIndnt(). */
/* Following two are public because libio.c looks at them: */
int sdfdumpstyle = STYLE_LISP;
int sdfdumpspacing = SP;
PRIVATE int compactslices = 0; /* Default is never to "compact" for dump_slice(\) */
PRIVATE int addcomments = 0;   /* Default is not to add comments. */
PRIVATE int adderrors = 1;     /* Default is to signal any inconsistencies. */
PRIVATE char *thislibrary = NULL;   /* Holds the name of the current library. */
PRIVATE char *thisfunction = NULL;  /* Holds the name of the current function. */
PRIVATE char *thiscircuit = NULL;   /* Holds the name of the current circuit. */

PRIVATE void dump_slicerecursively (FILEPTR fp, SLICEPTR slice);

/* Set new indentation spacing value. Return the old value.
 */
int setdumpspacing (int newspacing)
{
    int oldspacing = sdfdumpspacing;

    if (newspacing < 0) newspacing = 0;
    if (newspacing > MAXSP) newspacing = MAXSP;
    if (!(sdfdumpspacing = newspacing)) sdfdumpstyle = STYLE_LISP; /* no use for other style in this case */
    return (oldspacing);
}

/* Set new indentation style. Return old style.
 */
int setdumpstyle (int newstyle)
{
    int oldstyle = sdfdumpstyle;

    sdfdumpstyle = newstyle;
    return (oldstyle);
}

/* Specify whether to add comments or not. Returns previous situation.
 */
int setcomments (int newaddcomments)
{
    int oldaddcomments = addcomments;

    addcomments = newaddcomments;
    return (oldaddcomments);
}

/* Specify whether to signal internal inconsistencies of
 * the datastructures or not. Returns previous situation.
 */
int seterrors (int newadderrors)
{
    int oldadderrors = adderrors;

    adderrors = newadderrors;
    return (oldadderrors);
}

/* Put dump_slice() in "objective" mode. In this mode, the function
 * dump_slice() prints a LaySlice or a LayInstList exactly
 * as it is represented in the data structure. Returns 0 if it already
 * was in objective mode, and 1 if it was in "compact" mode (see below).
 */
int setobjectiveslice ()
{
    int old = compactslices;

    compactslices = 0;
    return (old);
}

/* Put dump_slice() in "compact" mode. In this mode, dump_slice()
 * will print a slice in its most compact form, even if this is
 * not the way the slice is represented in the internal data
 * structure. Returns 0 if dump_slice() was in "objective" mode,
 * and returns 1 if it already was in "compact" mode.
 */
int setcompactslice ()
{
    int old = compactslices;

    compactslices = 1;
    return (old);
}

void dumpdb (FILEPTR fp, SEADIFPTR seadif)
{
    dump_seadif (fp, seadif);
}

void dump_seadif (FILEPTR fp, SEADIFPTR seadif)
{
    LIBRARYPTR lib;

    // setbuf (fp, NULL); /* only during debugging */
    sdfindentlevel = 0;

    if (!seadif) return;

    fprintf (fp, "(Seadif \"%s\"", seadif->filename);
    sdfindentlevel += sdfdumpspacing;
    dump_status (fp, seadif->status);
    for (lib = seadif->library; lib; lib = lib->next) dump_library (fp, lib);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp);
    fprintf (fp, ")");
    fprintf (fp, "\n"); /* Final newline terminates this dump. */
}

void dump_alias (FILEPTR fp, STRING alias)
{
    if (!alias || !*alias) return;
    DoIndnt (fp);
    fprintf (fp, "(Alias \"%s\")", alias);
}

void dump_status (FILEPTR fp, STATUSPTR status)
{
    extern int sdfobligetimestamp;
    extern time_t sdftimestamp;
    time_t currenttime;
    struct tm *tmbuf;

    if (sdfobligetimestamp)
	currenttime = sdftimestamp;
    else
	currenttime = time (NULL); /* system call returning the current time */

    tmbuf = localtime (&currenttime);

    DoIndnt (fp); fprintf (fp, "(Status");  sdfindentlevel += sdfdumpspacing;
    DoIndnt (fp); fprintf (fp, "(Written"); sdfindentlevel += sdfdumpspacing;
    DoIndnt (fp); fprintf (fp, "(TimeStamp %d %d %d %d %d %d)",
			tmbuf->tm_year, tmbuf->tm_mon + 1, tmbuf->tm_mday,
			tmbuf->tm_hour, tmbuf->tm_min, tmbuf->tm_sec);
    if (status) {
	if (status->author) { DoIndnt (fp); fprintf (fp, "(Author \"%s\")", status->author); }
	if (status->program){ DoIndnt (fp); fprintf (fp, "(Program \"%s\")", status->program); }
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_library (FILEPTR fp, LIBRARYPTR library)
{
    FUNCTIONPTR fun;

    if (!library) return;

    thislibrary = library->name;

    DoIndnt (fp); fprintf (fp, "(Library \"%s\"", library->name);
    sdfindentlevel += sdfdumpspacing;
    dump_alias (fp, sdflibalias (library->name));
    if (library->technology) { DoIndnt (fp); fprintf (fp, "(Technology \"%s\")", library->technology); }
    dump_status (fp, library->status);

    for (fun = library->function; fun; fun = fun->next) dump_function (fp, fun);

    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
    thislibrary = NULL;
}

void dump_function (FILEPTR fp, FUNCTIONPTR function)
{
    CIRCUITPTR cir;

    if (!function) return;

    thisfunction = function->name;

    DoIndnt (fp); fprintf (fp, "(Function \"%s\"", function->name);
    sdfindentlevel += sdfdumpspacing;
    dump_alias (fp, sdffunalias (function->name, function->library->name));
    dump_status (fp, function->status);
    dump_funtype (fp, function->type);

    for (cir = function->circuit; cir; cir = cir->next) {
	dump_circuit (fp, cir);
	if (adderrors && cir->function != function)
	    fprintf (fp, " /* ERROR - circuit.function */");
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_funtype (FILEPTR fp, STRING funtype)
{
    if (funtype) {
	DoIndnt (fp); fprintf (fp, "(FunType \"%s\")", funtype);
    }
}

PRIVATE void dump_attribute (FILEPTR fp, STRING attribute)
{
    if (attribute) {
	DoIndnt (fp); fprintf (fp, "(Attribute \"%s\")", attribute);
    }
}

void dump_circuit (FILEPTR fp, CIRCUITPTR circuit)
{
    LAYOUTPTR  lay;

    if (!circuit) return;

    thiscircuit = circuit->name;

    DoIndnt (fp); fprintf (fp, "(Circuit \"%s\"", circuit->name);
    if (addcomments) fprintf (fp, " /* linkcnt=%ld, flag=0x%x */", circuit->linkcnt, circuit->flag.l);
    sdfindentlevel += sdfdumpspacing;
    dump_alias (fp, sdfciralias (circuit->name, circuit->function->name, circuit->function->library->name));
    dump_status (fp, circuit->status);
    dump_attribute (fp, circuit->attribute);
    if (circuit->cirport) dump_cirportlist (fp, circuit->cirport);
    if (circuit->cirinst) dump_cirinstlist (fp, circuit);
    if (circuit->netlist) dump_netlist (fp, circuit->netlist);
    if (circuit->buslist) dump_buslist (fp, circuit->buslist);
    if (circuit->timing)  dump_timing (fp, circuit->timing);

    for (lay = circuit->layout; lay; lay = lay->next) {
	dump_layout (fp, lay);
	if (adderrors && lay->circuit != circuit)
	    fprintf (stderr, " /* ERROR - layout.circuit */");
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_cirportlist (FILEPTR fp, CIRPORTPTR port)
{
    DoIndnt (fp); fprintf (fp, "(CirPortList");
    sdfindentlevel += sdfdumpspacing;

    for (; port; port = port->next)
    {
	DoIndnt (fp);
#ifdef SDF_PORT_DIRECTIONS
	fprintf (fp, "(CirPort \"%s\"", port->name);
	if (port->direction == SDF_PORT_UNKNOWN)
	    ; /* unknown is already the default */
	else if (port->direction == SDF_PORT_IN)
	    fprintf (fp, " (Direction IN)");
	else if (port->direction == SDF_PORT_OUT)
	    fprintf (fp, " (Direction OUT)");
	else if (port->direction == SDF_PORT_INOUT)
	    fprintf (fp, " (Direction INOUT)");
	else
	    sdfreport (Error, "SDFCIRPORT has illegal direction %d", port->direction);
	fprintf (fp, ")");
#else
	fprintf (fp, "(CirPort \"%s\")", port->name);
#endif
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_cirinstlist (FILEPTR fp, CIRCUITPTR circuit)
{
    CIRINSTPTR inst;

    DoIndnt (fp); fprintf (fp, "(CirInstList");
    sdfindentlevel += sdfdumpspacing;

    for (inst = circuit->cirinst; inst; inst = inst->next) {
	dump_cirinst (fp, inst);
	if (adderrors && inst->curcirc != circuit)
	    fprintf (fp, " /* ERROR - cirinst.curcirc */");
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_cirinst (FILEPTR fp, CIRINSTPTR cirinst)
{
    char *name;
    CIRCUITPTR  c;
    FUNCTIONPTR f;

    if (!cirinst) return;

    DoIndnt (fp); fprintf (fp, "(CirInst \"%s\"", cirinst->name);

    if ((c = cirinst->circuit))
    {
	fprintf (fp, " (CirCellRef \"%s\"", c->name);
	if ((name = (f = c->function)->name) != thisfunction || f->library->name != thislibrary)
	{
	    fprintf (fp, " (CirFunRef \"%s\"", name);
	    if ((name = f->library->name) != thislibrary)
		fprintf (fp, " (CirLibRef \"%s\")", name);
	    fprintf (fp, ")");
	}
	fprintf (fp, ")");      /* close CirCellRef */
	if (cirinst->attribute) fprintf (fp, " \"%s\"", cirinst->attribute);
	fprintf (fp, ")");
    }
    else
	fprintf (fp, " (CirCellRef \"UNKNOWN\"))");
}

void dump_netlist (FILEPTR fp, NETPTR net)
{
    if (!net) return;

    DoIndnt (fp); fprintf (fp, "(NetList");

    sdfindentlevel += sdfdumpspacing;
    for (; net; net = net->next) dump_net (fp, net);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_net (FILEPTR fp, NETPTR net)
{
    CIRPORTREFPTR term;
    int numterm = 0;

    if (!net) return;

    DoIndnt (fp); fprintf (fp, "(Net \"%s\"", net->name);
    if (addcomments)
	fprintf (fp, " /* num_term=%d, flag=0x%x */", net->num_term, net->flag.l);
    sdfindentlevel += sdfdumpspacing;
    DoIndnt (fp); fprintf (fp, "(Joined"); /* obligatory, even if terminals == NULL */

    if (net->terminals)
    {
	sdfindentlevel += sdfdumpspacing;
	for (term = net->terminals; term; term = term->next, ++numterm)
	{
	    DoIndnt (fp); fprintf (fp, "(NetPortRef \"%s\"", term->cirport->name);
	    if (term->cirinst)
		fprintf (fp, " (NetInstRef \"%s\")", term->cirinst->name);
	    fprintf (fp, ")");      /* Close NetPortRef. */
	    if (adderrors && term->net != net)
		fprintf (fp, " /* ERROR - cirport.net */");
	}
	sdfindentlevel -= sdfdumpspacing; OptIndnt (fp);
    }
    fprintf (fp, ")");
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");

    if (adderrors && net->num_term != numterm)
	fprintf (fp, " /* ERROR - net.num_term */");
}

void dump_buslist (FILEPTR fp, BUSPTR bus)
{
    if (!bus) return;

    DoIndnt (fp); fprintf (fp, "(BusList");
    sdfindentlevel += sdfdumpspacing;
    for (; bus; bus = bus->next) dump_bus (fp, bus);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_bus (FILEPTR fp, BUSPTR  bus)
{
    NETREFPTR netrefptr;

    if (!bus) return;

    DoIndnt (fp); fprintf (fp, "(Bus \"%s\"", bus->name);
    if (addcomments) fprintf (fp, " /* flag=0x%x */", bus->flag.l);
    sdfindentlevel += sdfdumpspacing;

    if (bus->netref)
    for (netrefptr = bus->netref; netrefptr; netrefptr = netrefptr->next)
    {
	DoIndnt (fp); fprintf (fp, "(NetRef \"%s\")", netrefptr->net->name);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")"); /* close Bus */
}

void dump_layout (FILEPTR fp, LAYOUTPTR layout)
{
    if (!layout) return;

    DoIndnt (fp); fprintf (fp, "(Layout \"%s\"", layout->name);
    sdfindentlevel += sdfdumpspacing;
    dump_alias (fp, sdflayalias (layout->name, layout->circuit->name,
	layout->circuit->function->name, layout->circuit->function->library->name));
    dump_status (fp, layout->status);
    dump_off (fp, layout->off);
    dump_bbx (fp, layout->bbx);
    if (layout->layport) dump_layportlist (fp, layout->layport);
    if (layout->laylabel) dump_laylabellist (fp, layout->laylabel);
    if (layout->slice) dump_slice (fp, layout->slice);
    if (layout->wire) dump_wirelist (fp, layout->wire);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_off (FILEPTR fp, short off[2])
{
    short h = off[HOR], v = off[VER];

    if (h || v) {
	DoIndnt (fp); fprintf (fp, "(LayOffset %d %d)", h, v);
    }
}

void dump_bbx (FILEPTR fp, short bbx[2])
{
    short h = bbx[HOR], v = bbx[VER];

    if (h || v) {
	DoIndnt (fp); fprintf (fp, "(LayBbx %d %d)", h, v);
    }
}

void dump_layportlist (FILEPTR fp, LAYPORTPTR port)
{
    if (port) {
	DoIndnt (fp); fprintf (fp, "(LayPortList");
	sdfindentlevel += sdfdumpspacing;
	for (; port; port = port->next) dump_layport (fp, port);
	sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
    }
}

void dump_layport (FILEPTR fp, LAYPORTPTR layport)
{
    if (layport) {
	DoIndnt (fp); fprintf (fp, "(LayPort \"%s\"", layport->cirport->name);
	fprintf (fp, " (PortPos %d %d)", layport->pos[HOR], layport->pos[VER]);
	fprintf (fp, " (PortLayer %d))", layport->layer);
    }
}

void dump_laylabellist (FILEPTR fp, LAYLABELPTR label)
{
    if (label) {
	DoIndnt (fp); fprintf (fp, "(LayLabelList");
	sdfindentlevel += sdfdumpspacing;
	for (; label; label = label->next) dump_laylabel (fp, label);
	sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
    }
}

void dump_laylabel (FILEPTR fp, LAYLABELPTR laylabel)
{
    if (laylabel) {
	DoIndnt (fp); fprintf (fp, "(LayLabel \"%s\"", laylabel->name);
	fprintf (fp, " (LabelPos %d %d)", laylabel->pos[HOR], laylabel->pos[VER]);
	fprintf (fp, " (LabelLayer %d))", laylabel->layer);
    }
}

void dump_slice (FILEPTR fp, SLICEPTR sl)
{
    int exceptional_case = 0;

    if (sl->chld_type == LAYINST_CHLD && sl->chld.layinst &&
	compactslices && !sl->chld.layinst->next)
    {
	exceptional_case = TRUE;
	DoIndnt (fp); fprintf (fp, "(LayInstList");
	sdfindentlevel += sdfdumpspacing;
    }
    dump_slicerecursively (fp, sl);
    if (exceptional_case) {
	sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
    }
}

PRIVATE void dump_slicerecursively (FILEPTR fp, SLICEPTR slice)
{
    LAYINSTPTR inst;

    for (; slice; slice = slice->next)
    {
	if (slice->chld_type == LAYINST_CHLD && slice->chld.layinst)
	{
	    if (compactslices && !slice->chld.layinst->next)
		dump_layinst (fp, slice->chld.layinst);
	    else
	    {
		DoIndnt (fp);
		if (slice->ordination == VERTICAL)
		    fprintf (fp, "(LaySlice vertical");
		else if (slice->ordination == HORIZONTAL)
		    fprintf (fp, "(LaySlice horizontal");
		else if (slice->ordination == CHAOS)
		    fprintf (fp, "(LayInstList");
		else {
		    fprintf (stderr, "dump_layout: unknown ordination for slice\n");
		    fprintf (fp, "(LaySlice \"UNKNOWN_ORDINATION2[%d]\"", slice->ordination);
		}
		sdfindentlevel += sdfdumpspacing;
		for (inst = slice->chld.layinst; inst; inst = inst->next)
		    dump_layinst (fp, inst);
		sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
	    }
	}
	else if (slice->chld_type == SLICE_CHLD && slice->chld.slice)
	{
	    DoIndnt (fp);
	    if (slice->ordination == VERTICAL)
		fprintf (fp, "(LaySlice vertical");
	    else if (slice->ordination == HORIZONTAL)
		fprintf (fp, "(LaySlice horizontal");
	    else if (slice->ordination == CHAOS)
		fprintf (fp, "(LayInstList");
	    else {
		fprintf (stderr, "dump_layout: unknown ordination for slice\n");
		fprintf (fp, "(LaySlice \"UNKNOWN_ORDINATION2[%d]\"", slice->ordination);
	    }
	    sdfindentlevel += sdfdumpspacing;
	    dump_slice (fp, slice->chld.slice);
	    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
	}
	else {
	    fprintf (stderr, "dump_layout: invalid slice chld_type\n");
	    DoIndnt (fp); fprintf (fp, "(INVALID LAYOUTSLICE)");
	}
    }
}

void dump_layinst (FILEPTR fp, LAYINSTPTR layinst)
{
    short *mtx;
    char  *name;
    LAYOUTPTR   l;
    FUNCTIONPTR f;
    CIRCUITPTR  c;

    if (!layinst) return;

    DoIndnt (fp); fprintf (fp, "(LayInst \"%s\"", layinst->name);
    sdfindentlevel += sdfdumpspacing;

    if ((l = layinst->layout))    /* If it's NULL then we already reported on that error. */
    {
	DoIndnt (fp); fprintf (fp, "(LayCellRef \"%s\"", l->name);
	if ((name = (c = l->circuit)->name) != thiscircuit
	    || (f = c->function)->name != thisfunction
	    || f->library->name != thislibrary)
	{
	    fprintf (fp, " (LayCirRef \"%s\"", name);
	    if ((name = (f = c->function)->name) != thisfunction
		    || f->library->name != thislibrary)
	    {
		fprintf (fp, " (LayFunRef \"%s\"", name);
		if ((name = f->library->name) != thislibrary)
		    fprintf (fp, " (LayLibRef \"%s\")", name);
		fprintf (fp, ")");
	    }
	    fprintf (fp, ")");
	}
	fprintf (fp, ")"); /* close LayCellRef */
    }
    else {
	DoIndnt (fp); fprintf (fp, "(LayCellRef \"UNKNOWN\")");
    }

    mtx = layinst->mtx;
    DoIndnt (fp); fprintf (fp, "(Orient %d %d %d %d %d %d)",
			mtx[0], mtx[1], mtx[2], mtx[3], mtx[4], mtx[5]);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_wirelist (FILEPTR fp, WIREPTR wire)
{
    short *crd;

    if (!wire) return;

    DoIndnt (fp); fprintf (fp, "(WireList");
    sdfindentlevel += sdfdumpspacing;
    for (; wire; wire = wire->next)
    {
	crd = wire->crd;
	DoIndnt (fp);
	fprintf (fp, "(Wire %d %d %d %d %d)",
	    wire->layer, crd[XL], crd[XR], crd[YB], crd[YT]);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_layinst2 (LAYINSTPTR layinst)
{
    FILEPTR fp = stderr;
    short *mtx;

    if (!layinst) return;

    DoIndnt (fp); fprintf (fp, "(LayInst \"something\"");
    sdfindentlevel += sdfdumpspacing;
    if (layinst->layout) { /* if NULL then we already reported on that error */
	DoIndnt (fp); fprintf (fp, "(LayCellRef \"%s\"", layinst->layout->name);
	fprintf (fp, ")"); /* close LayCellRef */
    }
    else {
	DoIndnt (fp); fprintf (fp, "(LayCellRef \"UNKNOWN\")");
    }
    mtx = layinst->mtx;
    DoIndnt (fp); fprintf (fp, "(Orient %d %d %d %d %d %d)",
			mtx[0], mtx[1], mtx[2], mtx[3], mtx[4], mtx[5]);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp); fprintf (fp, ")");
}

void dump_slice2 (SLICEPTR slice)
{
    LAYINSTPTR inst;
    FILEPTR fp = stderr;

    for (; slice; slice = slice->next)
    {
	if (slice->chld_type == LAYINST_CHLD && slice->chld.layinst)
	{
	    DoIndnt (fp);
	    if (slice->ordination == VERTICAL)
		fprintf (fp, "(LaySlice vertical");
	    else if (slice->ordination == HORIZONTAL)
		fprintf (fp, "(LaySlice horizontal");
	    else if (slice->ordination == CHAOS)
		fprintf (fp, "(LayInstList");
	    else
		fprintf (fp, "(LaySlice \"UNKNOWN_ORDINATION1[%d]\"", slice->ordination);
	    sdfindentlevel += sdfdumpspacing;
	    for (inst = slice->chld.layinst; inst; inst = inst->next)
		dump_layinst2 (inst);
	    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
	}
	else if (slice->chld_type == SLICE_CHLD && slice->chld.slice)
	{
	    DoIndnt (fp);
	    if (slice->ordination == VERTICAL)
		fprintf (fp, "(LaySlice vertical");
	    else if (slice->ordination == HORIZONTAL)
		fprintf (fp, "(LaySlice horizontal");
	    else if (slice->ordination == CHAOS)
		fprintf (fp, "(LayInstList");
	    else
		fprintf (fp, "(LaySlice \"UNKNOWN_ORDINATION2[%d]\"", slice->ordination);
	    sdfindentlevel += sdfdumpspacing;
	    dump_slice2 (slice->chld.slice);
	    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
	}
	else {
	    DoIndnt (fp); fprintf (fp, "(INVALID LAYOUTSLICE)");
	}
    }
}

#if 0
/* The following two are actually macros, but during debugging i prefer them to be subroutines.
 */
int DoIndnt (FILEPTR fp)
{
    int jj;
    putc ('\n', fp);
    for (jj = sdfindentlevel; jj > 0; --jj) putc (' ', fp);
}

int OptIndnt (FILEPTR fp)
{
    if (sdfdumpstyle == 1) DoIndnt (fp);
}
#endif

void sdfdumphashtable (FILEPTR fp)
{
    LIBTABPTR lib;

    if (!fp) fp = stderr;
    fprintf (fp, "(Seadif \"%s\"", "Hashed index tables");
    sdfindentlevel += sdfdumpspacing;
    for (lib = sdflib; lib; lib = lib->next)
	sdfdumphashlib (fp, lib);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp);
    fprintf (fp, ")\n\n");
}

void sdfdumphashlib (FILEPTR fp, LIBTABPTR lib)
{
    FUNTABPTR fun;

    DoIndnt (fp);
    fprintf (fp, "(Library \"%s\"", lib->name);
    fprintf (fp, "    (File \"%s\")", sdffileinfo[(int)lib->info.file].name);
    fprintf (fp, " (FilePos %ld)", lib->info.fpos);
    sdfindentlevel += sdfdumpspacing;
    for (fun = lib->function; fun; fun = fun->next)
	sdfdumphashfun (fp, fun);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp);
    fprintf (fp, ")");
}

void sdfdumphashfun (FILEPTR fp, FUNTABPTR fun)
{
    CIRTABPTR cir;

    DoIndnt (fp);
    fprintf (fp, "(Function \"%s\"", fun->name);
    fprintf (fp, "    (FilePos %ld)", fun->info.fpos);
    sdfindentlevel += sdfdumpspacing;
    for (cir = fun->circuit; cir; cir = cir->next)
	sdfdumphashcir (fp, cir);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp);
    fprintf (fp, ")");
}

void sdfdumphashcir (FILEPTR fp, CIRTABPTR cir)
{
    LAYTABPTR lay;

    DoIndnt (fp);
    fprintf (fp, "(Circuit \"%s\"", cir->name);
    fprintf (fp, "    (FilePos %ld)", cir->info.fpos);
    sdfindentlevel += sdfdumpspacing;
    for (lay = cir->layout; lay; lay = lay->next)
	sdfdumphashlay (fp, lay);
    sdfindentlevel -= sdfdumpspacing;
    OptIndnt (fp);
    fprintf (fp, ")");
}

void sdfdumphashlay (FILEPTR fp, LAYTABPTR lay)
{
    DoIndnt (fp);
    fprintf (fp, "(Layout \"%s\"", lay->name);
    fprintf (fp, "    (FilePos %ld))", lay->info.fpos);
}

void dump_timing (FILEPTR fp, TIMINGPTR timing)
{
    DELASGPTR delPtr;

    if (!timing) return;

    DoIndnt (fp); fprintf (fp, "(Timing \"%s\"", timing->name);
    sdfindentlevel += sdfdumpspacing;

    if (timing->status  ) dump_status (fp, timing->status);
    if (timing->t_terms ) dump_timetermlist (fp, timing->t_terms);
    if (timing->tminstlist) dump_tminstlist (fp, timing->tminstlist);
    if (timing->netmods ) dump_netmodlist (fp, timing->netmods);
    if (timing->tPaths  ) dump_tpathlist (fp, timing->tPaths);
    if (timing->timeCost) dump_timecost (fp, timing->timeCost);

    for (delPtr = timing->delays; delPtr; delPtr = delPtr->next)
	dump_delasg (fp, delPtr);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_timetermlist (FILEPTR fp, TIMETERMPTR ttermPtr )
{
    if (!ttermPtr) return;

    DoIndnt (fp); fprintf (fp, "(TmTermList");
    sdfindentlevel += sdfdumpspacing;
    for (; ttermPtr; ttermPtr = ttermPtr->next)
	dump_timeterm (fp, ttermPtr);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_tminstlist (FILEPTR fp, TMMODINSTPTR modinstPtr )
{
    if (!modinstPtr) return;

    DoIndnt (fp); fprintf (fp, "(TmModInstList");
    sdfindentlevel += sdfdumpspacing;
    for (; modinstPtr; modinstPtr = modinstPtr->next) {
	DoIndnt (fp);
	fprintf (fp, "(TmModInst \"%s\" (CInstRef \"%s\") (TimingRef \"%s\"))",
	modinstPtr->name, modinstPtr->cirinst->name, modinstPtr->timing->name);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_netmodlist (FILEPTR fp, NETMODPTR netmodPtr )
{
    if (!netmodPtr) return;

    DoIndnt (fp); fprintf (fp, "(NetModList");
    sdfindentlevel += sdfdumpspacing;
    for (; netmodPtr; netmodPtr = netmodPtr->next)
	dump_netmod (fp, netmodPtr);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_tpathlist (FILEPTR fp, TPATHPTR tpathPtr )
{
    if (!tpathPtr) return;

    DoIndnt (fp); fprintf (fp, "(TPathList");
    sdfindentlevel += sdfdumpspacing;
    for (; tpathPtr; tpathPtr = tpathPtr->next)
	dump_tpath (fp, tpathPtr);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_timecost (FILEPTR fp, TIMECOSTPTR timecostPtr)
{
    TCPOINTPTR tcpPtr;

    if (!timecostPtr) return;

    DoIndnt (fp); fprintf (fp, "(TimeCost");
    sdfindentlevel += sdfdumpspacing;
    for ( tcpPtr = timecostPtr->points; tcpPtr; tcpPtr = tcpPtr->next) {
	DoIndnt (fp);
	fprintf (fp, "(TcPoint \"%s\" %ld %ld \"%s\")",
	tcpPtr->name, tcpPtr->delay, tcpPtr->cost, tcpPtr->wayOfImplementing);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_delasg (FILEPTR fp, DELASGPTR delasgPtr)
{
    if (!delasgPtr) return;

    DoIndnt (fp); fprintf (fp, "(DelAsg \"%s\"", delasgPtr->name);
    sdfindentlevel += sdfdumpspacing;
    dump_status (fp, delasgPtr->status);
    DoIndnt (fp);
    if (delasgPtr->clockCycle >= 0)
	fprintf (fp, "(Clock %ld )", delasgPtr->clockCycle);
    if (delasgPtr->pathDelays)
	dump_delasglist (fp, delasgPtr->pathDelays);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_timeterm (FILEPTR fp, TIMETERMPTR timetermPtr)
{
    CIRPORTREFPTR  cpPtr;
    TIMETERMREFPTR ttPtr;

    if (!timetermPtr) return;

    DoIndnt (fp); fprintf (fp, "(TimeTerm \"%s\" %d", timetermPtr->name, timetermPtr->type);
    sdfindentlevel += sdfdumpspacing;

    /* first cirport references */
    for (cpPtr = timetermPtr->cirportlist; cpPtr; cpPtr = cpPtr->next) {
	DoIndnt (fp); fprintf (fp, "(CirPortRef \"%s\")", cpPtr->cirport->name);
    }
    /* then time terminals' references */
    for (ttPtr = timetermPtr->termreflist; ttPtr; ttPtr = ttPtr->next) {
	DoIndnt (fp); fprintf (fp, "(TimeTermRef \"%s\" (TmModInstRef \"%s\"))",
			ttPtr->term->name, ttPtr->inst->name);
    }
    /* now required input and output time */
    /* if it is a clocked terminal of course */
    if (timetermPtr->reqInputTime != -1) {
	DoIndnt (fp); fprintf (fp, "(ReqInputTime \"%f\")", timetermPtr->reqInputTime);
    }
    if (timetermPtr->outputTime != -1) {
	DoIndnt (fp); fprintf (fp, "(OutputTime \"%f\")", timetermPtr->outputTime);
    }
    if (timetermPtr->load != -1) {
	DoIndnt (fp); fprintf (fp, "(Load \"%f\")", timetermPtr->load);
    }
    if (timetermPtr->drive != -1) {
	DoIndnt (fp); fprintf (fp, "(Drive \"%f\")", timetermPtr->drive);
    }
    if (timetermPtr->type == OutputTTerm)
	dump_timecost (fp, timetermPtr->timecost);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_netmod (FILEPTR fp, NETMODPTR netmodPtr)
{
    NETREFPTR  nPtr;
    BUSREFPTR  bPtr;

    if (!netmodPtr) return;

    DoIndnt (fp); fprintf (fp, "(NetMod \"%s\"", netmodPtr->name);
    sdfindentlevel += sdfdumpspacing;

    /* first net references */
    for (nPtr = netmodPtr->netlist; nPtr; nPtr = nPtr->next) {
	DoIndnt (fp); fprintf (fp, "(NetRef \"%s\")", nPtr->net->name);
    }
    /* then bus references */
    for (bPtr = netmodPtr->buslist; bPtr; bPtr = bPtr->next) {
	DoIndnt (fp); fprintf (fp, "(BusRef \"%s\")", bPtr->bus->name);
    }
    dump_timecost (fp, netmodPtr->cost);
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_tpath (FILEPTR fp, TPATHPTR tpathPtr)
{
    if (!tpathPtr) return;

    DoIndnt (fp); fprintf (fp, "(TPath \"%s\"", tpathPtr->name);
    sdfindentlevel += sdfdumpspacing;

    dump_starttermlist(fp, tpathPtr->startTermList);
    dump_endtermlist  (fp, tpathPtr->endTermList);
    dump_timecost     (fp, tpathPtr->timeCost);

    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_starttermlist (FILEPTR fp, TIMETERMREFPTR termPtr)
{
    if (!termPtr) return;

    DoIndnt (fp); fprintf (fp, "(StartTermList");
    sdfindentlevel += sdfdumpspacing;

    for (; termPtr; termPtr = termPtr->next) {
	DoIndnt (fp); fprintf (fp, "(TimeTermRef \"%s\")", termPtr->term->name);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_endtermlist (FILEPTR fp, TIMETERMREFPTR termPtr)
{
    if (!termPtr) return;

    DoIndnt (fp); fprintf (fp, "(EndTermList");
    sdfindentlevel += sdfdumpspacing;

    for (; termPtr; termPtr = termPtr->next) {
	DoIndnt (fp); fprintf (fp, "(TimeTermRef \"%s\")", termPtr->term->name);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}

void dump_delasglist (FILEPTR fp, DELASGINSTPTR delasginstPtr)
{
    if (!delasginstPtr) return;

    DoIndnt (fp); fprintf (fp, "(DelAsgInstList");
    sdfindentlevel += sdfdumpspacing;

    for (; delasginstPtr; delasginstPtr = delasginstPtr->next) {
	DoIndnt (fp); fprintf (fp, "(DelAsgInst \"%s\" (TPathRef \"%s\") (TcPointRef \"%s\"))",
			delasginstPtr->name,
			delasginstPtr->tPath->name,
			delasginstPtr->selected->name);
    }
    sdfindentlevel -= sdfdumpspacing; OptIndnt (fp); fprintf (fp, ")");
}
