/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
 *	Simon de Graaf
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

#include <time.h> /* prototypes the time() syscall */

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"
#include "src/ocean/nelsea/prototypes.h"

static void write_circuit (CIRCUITPTR, int);
static void write_mc (CIRCUITPTR, DM_STREAM *);
static void write_terminals (CIRCUITPTR, DM_STREAM *);
static void write_net (CIRCUITPTR, DM_STREAM *);
static void evaluate_terminal_reference (CIRPORTREFPTR cportref, struct cir_net * net_eqv);

extern char *this_sdf_lib;
extern char *primitive_str;
extern char *in_core_str;
extern char *circuit_str;
extern char *layout_str;
extern char *written_str;

extern int No_sdf_write;
extern int verbose;

extern DM_PROJECT *projectkey;
extern MAPTABLEPTR maptable;

/* This routine writes all circuits in the current datastruct.
 */
void write_nelsis_circuit ()
{
    MAPTABLEPTR map;

    for (map = maptable; map; map = map->next)
    {
	if (map->internalstatus == in_core_str &&
	    map->seanelstatus != primitive_str &&
	    map->view == circuit_str &&
	    (map->seanelstatus != written_str || map->overrule_status == TRUE))
	{ /* write it */
	    /* if ghoti: write nonrecursive */
	    write_circuit (map->circuitstruct, No_sdf_write ? FALSE : TRUE);
	}
    }
}

/* This routine dumps the seadif datastructure in a proper NELSIS database cell.
 * Before calling the database should be opened.
 *
 * Argument 'recursive' is TRUE for recursive write.
 */
static void write_circuit (CIRCUITPTR cir, int recursive)
{
    MAPTABLEPTR map; /* points to datastructure */
    DM_STREAM *fp;
    DM_CELL *key;
    CIRINSTPTR cinst;

    if (!cir) {
	fprintf (stderr, "WARNING (write_circuit): null struct.\n");
	return;
    }

    /* find out its mapping */
    map = look_up_seadif_map (circuit_str,
		cir->function->library->name,
		cir->function->name,
		cir->name,
		cir->name);

    if (map->seanelstatus == primitive_str) return; /* cell is primitive */

    if (map->seanelstatus == written_str && map->overrule_status == FALSE) return;

    if (map->internalstatus != in_core_str || !map->circuitstruct) {
	// fprintf (stderr, "ERROR: attempt to write cell '%s', which is not in core\n", map->circuit);
	return;
    }

    /* recursively write child cells */
    if (recursive == TRUE)
    {
	for (cinst = cir->cirinst; cinst; cinst = cinst->next)
	{ /* sub-cells are not written with force */
	    write_circuit (cinst->circuit, recursive);
	}
    }

    /* test: is the name too long? */
    if (strlen (map->cell) > DM_MAXNAME) { /* print warning */
	fprintf (stderr, "WARNING (write_circuit): cell name %s too long, truncated\n", map->cell);
	map->cell[DM_MAXNAME] = '\0';
    }

    if (verbose) {
	printf ("------ writing sdfcir '%s(%s(%s))' into nelsis '%s' ------\n",
	    cir->name, cir->function->name, cir->function->library->name, map->cell);
	fflush (stdout);
    }

    /* open a new model file, called cell_name */
    if (!(key = dmCheckOut (projectkey, map->cell, DERIVED, DONTCARE, circuit_str, UPDATE))) {
	error (ERROR, "Unable to open cell (cell not written)");
	return;
    }

    /* write model calls */
    if (!(fp = dmOpenStream (key, "mc", "w"))) error (FATAL_ERROR, "write_circuit");

    write_mc (map->circuitstruct, fp);
    dmCloseStream (fp, COMPLETE);

    /* write terminals */
    if (!(fp = dmOpenStream (key, "term", "w"))) error (FATAL_ERROR, "write_circuit");

    write_terminals (map->circuitstruct, fp);
    dmCloseStream (fp, COMPLETE);

    /* write the netlist */
    if (!(fp = dmOpenStream (key, "net", "w"))) error (FATAL_ERROR, "write_circuit");

    write_net (map->circuitstruct, fp);
    dmCloseStream (fp, COMPLETE);

    /* ready! */
    dmCheckIn (key, COMPLETE);

    /* set status */
    map->seanelstatus = written_str;
    map->nelsis_time = time (0);
    map->overrule_status = FALSE;
}

/* This routine writes the circuit model calls.
 */
static void write_mc (CIRCUITPTR cir, DM_STREAM *fp)
{
    CIRINSTPTR cinst;
    MAPTABLEPTR map;

    for (cinst = cir->cirinst; cinst; cinst = cinst->next)
    {
	/* cell_name */
	map = look_up_seadif_map (circuit_str,
		cinst->circuit->function->library->name,
		cinst->circuit->function->name,
		cinst->circuit->name,
		cinst->circuit->name);

	if (map->seanelstatus != primitive_str &&
	    map->internalstatus != in_core_str && !map->nelsis_time)
	{
	    if (map->num_read_attempts <= 2) continue;

	    fprintf (stderr, "WARNING: circuit '%s' contains reference(s) to non-existing\n", cir->name);
	    fprintf (stderr, "         son-cell '%s'\n", cinst->circuit->name);
	    fprintf (stderr, "         All references will be skipped...\n");
	    map->num_read_attempts++; /* to prevent many printing */
	    continue;
	}

	if (strlen (map->cell) > DM_MAXNAME) { /* print warning */
	    fprintf (stderr, "WARNING (write_circuit): circuit cell name %s too long, truncated\n", map->cell);
	    map->cell[DM_MAXNAME] = '\0';
	}

	/* look for son cell in database, is imported??, can find? */
	if (map->nelseastatus != primitive_str &&
	    map->seanelstatus != primitive_str)
	{
	    int imported;
	    if ((imported = exist_cell (map->cell, circuit_str)) < 0)
	    { /* it does not exist */
		fprintf (stderr, "WARNING: cannot find son-cell '%s' (mapped '%s') of circuit '%s' in database\n",
		    cinst->circuit->name, map->cell, cir->name);
		imported = LOCAL;
	    }
	    cmc.imported = imported;
	}
	else
	    cmc.imported = LOCAL; /* primitive cell */

	strcpy (cmc.cell_name, map->cell);

	/* instance name */
	strNcpy (cmc.inst_name, cinst->name, DM_MAXNAME);
	if (strlen (cinst->name) > DM_MAXNAME) {
	    fprintf (stderr, "WARNING: circuit instance name '%s' too long, truncated\n", cinst->name);
	}

	cmc.inst_attribute = cinst->attribute;
	cmc.inst_dim = 0;
	cmc.inst_lower = cmc.inst_upper = NULL;
	dmPutDesignData (fp, CIR_MC);
    }
}

/* This routine writes the circuit terminals.
 */
static void write_terminals (CIRCUITPTR cir, DM_STREAM *fp)
{
    CIRPORTPTR cport;

    for (cport = cir->cirport; cport; cport = cport->next)
    {
	/* instance name */
	strNcpy (cterm.term_name, cport->name, DM_MAXNAME);
	if (strlen (cport->name) > DM_MAXNAME) {
	    fprintf (stderr, "WARNING: terminal name '%s' (in circuit '%s') too long, truncated\n",
		cport->name, cir->name);
	}
	cterm.term_attribute = NULL;
	cterm.term_dim = 0;
	cterm.term_lower = cterm.term_upper = NULL;
	dmPutDesignData (fp, CIR_TERM);
    }
}

/* This routine writes the netlist.
 */
static void write_net (CIRCUITPTR cir, DM_STREAM *fp)
{
    long n, neqv;
    /* MAPTABLEPTR map; */
    NETPTR cirnet;
    CIRPORTREFPTR cportref;
    struct cir_net *eqv;

    for (cirnet = cir->netlist; cirnet; cirnet = cirnet->next)
    {
	/*
	 * count terminals, find terminal with net name,
	 * which needs special treatment
	 */
	neqv = 0;
	for (cportref = cirnet->terminals; cportref; cportref = cportref->next)
	{
	    if (!cportref->cirinst && cportref->cirport->name == cirnet->name)
		;
	    else
		neqv++;
	}

	/* fill header */
	if (strlen (cirnet->name) > DM_MAXNAME) {
	    fprintf (stderr, "WARNING: net name '%s' (in circuit '%s') too long, truncated\n",
		cirnet->name, cir->name);
	}
	strNcpy (cnet.net_name, cirnet->name, DM_MAXNAME);

	cnet.net_attribute = NULL;
	cnet.net_dim = 0;
	cnet.net_lower = cnet.net_upper = NULL;
	cnet.inst_name[0] = '\0';
	cnet.inst_dim = 0;
	cnet.inst_lower = cnet.inst_upper = NULL;
	cnet.ref_dim = 0;
	cnet.ref_lower = cnet.ref_upper = NULL;
	cnet.net_neqv = neqv;
	cnet.net_eqv = NULL;
	dmPutDesignData (fp, CIR_NET_ATOM);

	/* fill terminals */
	n = 0;
	for (cportref = cirnet->terminals; cportref && n != neqv; cportref = cportref->next)
	{
	    if (!cportref->cirinst && cportref->cirport->name == cirnet->name)
		continue; /* header: already there implicitly */
	    eqv = &cnet;

	    /* set terminal name */
	    strNcpy (eqv -> net_name, cportref->cirport->name, DM_MAXNAME);

	    eqv -> net_attribute = NULL;
	    eqv -> net_dim = 0;
	    eqv -> net_lower = eqv -> net_upper = NULL;
	    if (!cportref->cirinst)
	    { /* on father */
		eqv -> inst_name[0] = '\0';
	    }
	    else
	    { /* on son */
#if 0
		map = look_up_seadif_map (circuit_str,
			cportref->cirinst->circuit->function->library->name,
			cportref->cirinst->circuit->function->name,
			cportref->cirinst->circuit->name,
			cportref->cirinst->circuit->name);

		if (map->seanelstatus != written_str) {
		    fprintf (stderr, "WARNING: circuit reference to non-converted sdf circuit cell '%s'\n",
		    cportref->cirinst->circuit->name);
		}
#endif
		strNcpy (eqv -> inst_name, cportref->cirinst->name, DM_MAXNAME);
	    }

	    eqv -> inst_dim = 0;
	    eqv -> inst_lower = eqv -> inst_upper = NULL;
	    eqv -> ref_dim = 0;
	    eqv -> ref_lower = eqv -> ref_upper = NULL;
	    eqv -> net_neqv = 0;
	    eqv -> net_eqv = NULL;

	    /* evaluate whether the terminal name reference is correct */
	    if (cportref->cirinst) { /* on son */
		evaluate_terminal_reference (cportref, eqv);
	    }

	    dmPutDesignData (fp, CIR_NET_ATOM);
	    n++;
	}
    }
}

/* Patrick 7-1993 */
/* This routine checks whether the terminal name is of type
 * <name>_<index>_ which means that it refers to a
 * terminal which was originally indexed in nelsis, but
 * was expanded by nelsea. In this routine, we do the
 * reverse, if possible, to prevent illegal references.
 * note: this is not a very elegant or efficient routine,
 * but it does the job.
 */
static void evaluate_terminal_reference (CIRPORTREFPTR cportref, struct cir_net * net_eqv)
{
    char name[265], *indexstart, *indexend;
    DM_PROJECT *remote_projectkey;
    DM_CELL *cell_key;
    DM_STREAM *fp;
    char *remote_cellname;
    MAPTABLEPTR map;
    int found, index;

    strcpy (name, cportref->cirport->name);

    /* 1: is the name in the indexed form??
    */
    if (!(indexstart = strchr (name, (int) '_'))) return; /* no index mark */

    if (!(indexend = strrchr (name, (int) '_'))) return; /* no index mark */

    if (indexstart == indexend) return; /* no index mark */

    /* replace last underscore by '\0' */
    indexend[0] = '\0';

    /* find first underscore of index */
    if (!(indexstart = strrchr (name, (int) '_'))) return; /* no index mark */

    /* replace it by '\0' */
    indexstart[0] = '\0';
    /* and advance one to get index */
    indexstart++;

    /* find index number */
    if (sscanf (indexstart, "%d", &index) < 1) return; /* no index mark */

    /* name contains the (non-indexed) terminal name,
    index the index number */
    /* printf ("name = '%s', index = '%d'\n", name, index); */

    /* fetch equivalence */
    map = look_up_seadif_map (circuit_str,
		cportref->cirinst->circuit->function->library->name,
		cportref->cirinst->circuit->function->name,
		cportref->cirinst->circuit->name,
		cportref->cirinst->circuit->name);

    if (map->nelsis_time < 1) return; /* entire son cell cannot be found... */

    /*
     * find out whether a terminal with that index exists
     * in the son-cell
     */

    /* open library project of son */
    if (!(remote_projectkey = dmFindProjKey (map->library == this_sdf_lib ? LOCAL : IMPORTED,
		map->cell, projectkey, &remote_cellname, map->view)))
    {
	fprintf (stderr, "ERROR: cannot find nasty project key\n");
	return;
    }

    /* open cell */
    if (!(cell_key = dmCheckOut (remote_projectkey, remote_cellname, ACTUAL, DONTCARE, map->view, READONLY)))
    {
	fprintf (stderr, "ERROR: cannot open cell '%s' for terms\n", map->cell);
	return;
    }

    /* read terminals */
    if (!(fp = dmOpenStream (cell_key, "term", "r"))) {
	fprintf (stderr, "ERROR: cannot open term of circuit-cell '%s'\n", remote_cellname);
	return;
    }

    /* read all terminals */
    found = 0;
    while (dmGetDesignData (fp, CIR_TERM) > 0 && found != 2)
    {
	switch ((int)cterm.term_dim)
	{
	    case 0:  /* not an array terminal */
		if (strcmp (cterm.term_name, cportref->cirport->name) == 0)
		    found = 2; /* original terminal exists as name! */
		break;
	    case 1: /* 1-dim array */
		if (strcmp (cterm.term_name, name) == 0)
		    if (index >= cterm.term_lower[0] && index <= cterm.term_upper[0])
			found = 1; /* BINGO! it exists! */
		break;
	    default: /* we don't handle multi-dimensional arrays */
		break;
	}
    }

    dmCloseStream (fp, COMPLETE);
    dmCheckIn (cell_key, COMPLETE);

    if (found == 2) return; /* do nothing.... */
    if (found == 0) {
	/* printf ("not found in son cell '%s'\n", map->cell); */
	return; /* do nothing.... */
    }
    /* printf ("found it\n"); */

    /* we found it, change net_eqv */
    net_eqv->net_dim = 1;

    /* allocate the nasty longs... */
    if (!(net_eqv->net_lower = (long *) calloc ((unsigned) 1, (unsigned) sizeof(long)))) error (FATAL_ERROR, "calloc");
    if (!(net_eqv->net_upper = (long *) calloc ((unsigned) 1, (unsigned) sizeof(long)))) error (FATAL_ERROR, "calloc");

    strcpy (net_eqv->net_name, name);
    net_eqv->net_lower[0] = net_eqv->net_upper[0] = index;
}
