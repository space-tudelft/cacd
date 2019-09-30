/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	P.E. Menchen
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

extern Dictionary *ntw_dict;
extern Dictionary *sym_dict;
extern Dictionary *inst_dict;

extern Network *curr_ntw;

extern Netelem *notconnected;

extern int sflag;
extern int noWarnings;

DM_CELL *dmkey;

extern DM_STREAM *dsp_term;
extern DM_STREAM *dsp_mc;
extern DM_STREAM *dsp_net;

extern char **defGlobNets;
extern int defGlobNets_cnt;

int doExternTermCheck;
int *orig_term_check = NULL;

Stack * xs_rn = NULL;

extern DM_PROJECT *dmproject;

Network *init_ntw (char *name, int ext)
{
    char attribute_string[256];
    long lower[10], upper[10];
    char * s;
    char * new_name;
    Network *ntw;
    DM_PROJECT *projkey;
    int exist;
    int i;
    Xelem * stkelem;
    int t_cnt;

    dsp_term = NULL;
    dsp_mc = NULL;
    dsp_net = NULL;

    sym_dict = new Dictionary;
    inst_dict = new Dictionary;
    ntw = (Network *) ntw_dict->fetch(name); /* fetch returns NULL if network
						does not exist */

    if(!sflag && !ext)
	fprintf(stderr, "Parsing network: %s\n", name);

    if(ntw == NULL) {
	ntw = new Network (name);

        curr_ntw = ntw;

	ntw -> termq = NULL;
	ntw -> orig_termq = NULL;
	ntw -> netq = new Queue (QueueType);

	ntw_dict->store(ntw->ntw_name, (char *) ntw);

	if (!ext)
	    dmkey = dmCheckOut (dmproject, name, WORKING, DONTCARE, CIRCUIT, CREATE);
	else {
	    if ((exist = existCell (name, CIRCUIT)) == 0) {
		if(!noWarnings)
		    fprintf(stderr, "Warning: extern network %s not yet in database\n", name);
		/*
	        sls_errno = UNKNETWORK;
	        sls_error (yylineno, sls_errno, name);
	        die();
		*/
		doExternTermCheck = 0;
	    }
	    else {
		projkey = dmFindProjKey (((exist == 1)?(LOCAL):(IMPORTED)), name, dmproject, &new_name, CIRCUIT);

		dmkey = dmCheckOut (projkey, new_name, ACTUAL, DONTCARE, CIRCUIT, READONLY);

		doExternTermCheck = 1;
	    }
	}
    }
    else {
	return (NULL);  /* definition of network after use in instance */
    }

    if (ext && existCell (name, CIRCUIT) == 2)
	ntw -> local = 0;
    else
	ntw -> local = 1;

    notconnected = new Netelem ((char*)"notConnected", NULL, NetType);
    sym_dict -> store (notconnected -> name, (char *)notconnected);

    if (!ext) {
	dsp_term = dmOpenStream(dmkey, "term", "w");
	dsp_mc = dmOpenStream(dmkey, "mc", "w");
	dsp_net = dmOpenStream(dmkey, "net", "w");
    }
    else if (doExternTermCheck) {
	dsp_term = dmOpenStream(dmkey, "term", "r");
	dm_get_do_not_alloc = 1;
	cterm.term_attribute = attribute_string;
	cterm.term_lower = lower;
	cterm.term_upper = upper;

	defGlobNets_cnt = 0;
	ntw -> orig_termq = new Queue (QueueType);

	if (xs_rn == NULL) xs_rn = new Stack (XSTACK_SIZE);

        t_cnt = 0;
	while (dmGetDesignData (dsp_term, CIR_TERM) > 0 ) {
	    xs_rn -> reset ();
	    for (i=0; i < cterm.term_dim; i++) {
		stkelem = new Xelem ((short)cterm.term_lower[i], (short)cterm.term_upper[i]);
		if (xs_rn -> push ((char *)stkelem) == STACK_OVERFLOW)
		    fprintf (stderr, "xstack overflow\n");
	    }
	    ntw -> orig_termq -> put ((Link *) new Netelem (cterm.term_name, stackcpy (xs_rn), TermType));
	    t_cnt++;

	    if ((s = isGlobalNet (cterm.term_name)) && cterm.term_dim == 0) {
		defGlobNets[defGlobNets_cnt++] = s;
	    }
	}

	if (!noWarnings && t_cnt > 0) {
	    orig_term_check = new int [t_cnt];
	    for (i = 0; i < t_cnt; i++) {
		orig_term_check[i] = 0;   /* initialize */
	    }
	}

	dm_get_do_not_alloc = 0;
	dmCloseStream (dsp_term, COMPLETE);
	dsp_term = NULL;
	dmCheckIn (dmkey, COMPLETE);
    }
    else {
	defGlobNets_cnt = 0;
    }

    return (ntw);
}

Network *read_ntw (char *name)
{
    char attribute_string[256];
    long lower[10], upper[10];
    DM_CELL * dmkey;
    DM_STREAM * dsp;
    Network * ntw;
    Xelem * stkelem;
    struct stat buf;
    char * new_name;
    DM_PROJECT *projkey;
    int exist, i;

    if ((exist = existCell (name, CIRCUIT)) == 0) return (NULL);

    projkey = dmFindProjKey (((exist == 1)?(LOCAL):(IMPORTED)), name, dmproject, &new_name, CIRCUIT);

    dmkey = dmCheckOut (projkey, new_name, ACTUAL, DONTCARE, CIRCUIT, READONLY);

    if (dmStat (dmkey, "term", &buf) == 0) {
	dsp = dmOpenStream (dmkey, "term", "r");
    }
    else {
	if (exist == 2) dmCloseProject (projkey, COMPLETE);
        return (NULL); /* apparently the circuit view only contains a function */
    }
    dm_get_do_not_alloc = 1;
    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    ntw = new Network (name);
    ntw_dict -> store (ntw->ntw_name, (char *) ntw);

    if (exist == 1)
	ntw -> local = 1;
    else
	ntw -> local = 0;

    ntw -> termq = new Queue (QueueType);

    if (xs_rn == NULL) xs_rn = new Stack (XSTACK_SIZE);

    while (dmGetDesignData (dsp, CIR_TERM) > 0 ) {
        xs_rn -> reset ();
        for (i = 0; i < cterm.term_dim; ++i) {
            stkelem = new Xelem ((short)cterm.term_lower[i], (short)cterm.term_upper[i]);
            if (xs_rn -> push ((char *)stkelem) == STACK_OVERFLOW)
                fprintf (stderr, "xstack overflow\n");
        }
        ntw -> termq -> put ((Link *) new Netelem (cterm.term_name, stackcpy (xs_rn), TermType));
    }

    dm_get_do_not_alloc = 0;
    dmCloseStream (dsp, COMPLETE);
    dmCheckIn (dmkey, COMPLETE);

    if (exist == 2) dmCloseProject (projkey, COMPLETE);

    return (ntw);
}

int checkDbTerm (Netelem *term)
{
    int j;
    Netelem *pterm;

    Queue * termq;
    int     termq_len;

    if (doExternTermCheck == 0)
	return (0);

    termq = curr_ntw -> orig_termq;
    termq_len = termq -> length ();

    if (termq) {
	for (j = 0, pterm = (Netelem *) termq -> first_elem ();
		j < termq_len;
		j++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm)) {

	    if (strcmp (pterm -> name, term -> name) == 0) {

		if (pterm -> xs && term -> xs) {
		    if (chk_bounds (pterm -> xs, term -> xs)) {
			sls_errno = ILLRANGETERMDECL;
			sls_error (yylineno, sls_errno, term -> name);
			return (-1);
		    }
		}
		else if (pterm -> xs && !term -> xs) {
		    sls_errno = ILLRANGETERMDECL;
		    sls_error (yylineno, sls_errno, term -> name);
		    return (-1);
		}
		else if (!pterm -> xs && term -> xs) {
		    sls_errno = ILLRANGETERMDECL;
		    sls_error (yylineno, sls_errno, term -> name);
		    return (-1);
		}

                if (orig_term_check)
		    orig_term_check[j] = 1;

		return (0);   /* identical term in database */
	    }
	}
    }

    sls_errno = UNKTERMDECL;
    sls_error (yylineno, sls_errno, term -> name);
    return (-1);    /* term does not exist in database */
}

void finalCheckDbTerm ()
{
    int j;
    Netelem *pterm;

    Queue * termq;
    int     termq_len;

    if (doExternTermCheck == 0 || noWarnings)
	return;

    termq = curr_ntw -> orig_termq;
    termq_len = termq -> length ();

    if (termq) {
	for (j = 0, pterm = (Netelem *) termq -> first_elem ();
		j < termq_len;
		j++, pterm = (Netelem *) termq -> next_elem ((Link *) pterm)) {
	    if (orig_term_check[j] == 0 && !isGlobalNet (pterm -> name)) {
		fprintf (stderr,
	  "Warning: terminal %s of netwerk %s not used in extern declaration\n",
			 pterm -> name, curr_ntw -> ntw_name);
	    }
	}
        delete orig_term_check;
	orig_term_check = NULL;
    }
}

static IMPCELL **f_impcell_list = NULL;
static char **f_loccell_list = NULL;
static IMPCELL **c_impcell_list = NULL;
static char **c_loccell_list = NULL;

int existCell (const char *name, const char *view)
{
    int i;
    IMPCELL ***impcell_list;
    char ***loccell_list;

    if (strcmp (view, CIRCUIT) != 0) {
	fprintf (stderr, "incorrect view: %s\n", view);
	die ();
    }
    impcell_list = &c_impcell_list;
    loccell_list = &c_loccell_list;

    if (*impcell_list == NULL) {
	*impcell_list = (IMPCELL **)dmGetMetaDesignData (IMPORTEDCELLLIST, dmproject, view);
    }

    if (*loccell_list == NULL) {
	*loccell_list = (char **)dmGetMetaDesignData (CELLLIST, dmproject, view);
    }

    if (*impcell_list == NULL || *loccell_list == NULL)
	return (0);

    i = 0;
    while ((*loccell_list)[i] != NULL
	   && strcmp (name, (*loccell_list)[i]) != 0) {
	i++;
    }

    if ((*loccell_list)[i] != NULL)
	return (1);                                 /* local cell */

    i = 0;
    while ((*impcell_list)[i] != NULL
	   && strcmp (name, (*impcell_list)[i] -> alias) != 0) {
	i++;
    }

    if ((*impcell_list)[i] != NULL)
	return (2);                                /* imported cell */

    return (0);                                    /* (actual version) of
						      cell does not exist */
}
