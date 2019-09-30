static char *rcsid = "$Id: main.c,v 1.1 2018/04/30 12:17:36 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
 *	N.P. van der Meijs
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
 * Contains main function.
 */
#include "src/match/head.h"
#include "src/match/incl.h"
#include "src/match/proto.h"
#include <signal.h>

DM_PROJECT * dmproject;

/*	VARIABLES:
 */
Import boolean R_opt;
Import boolean I_opt;
Import boolean E_opt;
Import boolean b_opt;
Import boolean f_opt;
Import boolean x_opt;
Import boolean m_opt;
Import boolean s_opt;
Import boolean D_opt;
Import boolean V_opt;
Import boolean g_opt;
Import boolean anno_opt;
Import boolean edif_opt;

Import hash * act_def_h;
Import hash * nom_def_h;
Import hash * symbol_table;
Import network * nom_netw;
Import network * act_netw;
Import network * cur_network;
Import FILE * std_out;

Public string act_netw_name, nom_netw_name;
Public string argv0;

void print_dashes ()
{
    if (V_opt || R_opt || I_opt || E_opt || D_opt || g_opt)
	fprintf (stderr, "============================================\n");
}

int main (int argc, string argv[])
{
    int exit_status;
    DM_CELL * nom_key = NULL;
    DM_CELL * act_key = NULL;
    DM_STREAM * match_stream;

    argv0 = "match";

 /* Scan for options */
    ese_options (argc, argv);

/*
    signal (SIGINT, my_exit);
*/
    if (dmInit (argv0)) {
	err_mesg ("%s: cannot initialize the dmi\n", argv0);
    }

    /* The project needs to be opened so that we know the process directory
     * where we can find the file containing the primitives. */
    if (!f_opt || x_opt) {
	if (!(dmproject = dmOpenProject (DEFAULT_PROJECT, PROJ_READ))) {
	    err_mesg ("%s: cannot open default project\n", argv0);
	}
	Assert (dmproject -> projectno == 0);
    }

 /* Basic Initialisation */
    my_init ();

    if (edif_opt && std_out == stdout && (!f_opt || x_opt)) {

	act_key = dmCheckOut (dmproject, act_netw_name, WORKING, DONTCARE, CIRCUIT, ATTACH);
	if (!act_key) err_mesg ("%s: cannot checkOut act_network\n", argv0);

	match_stream = dmOpenStream (act_key, "match", "w");
	if (!match_stream) err_mesg ("%s: cannot open act_network stream\n", argv0);
	std_out = match_stream -> dmfp;
    }

    if (!f_opt) {
	if (!act_key) {
	    act_key = dmCheckOut (dmproject, act_netw_name, WORKING, DONTCARE, CIRCUIT, READONLY);
	    if (!act_key) err_mesg ("%s: cannot checkOut act_network\n", argv0);
	}
	if (strcmp (nom_netw_name, act_netw_name)) {
	    nom_key = dmCheckOut (dmproject, nom_netw_name, WORKING, DONTCARE, CIRCUIT, READONLY);
	    if (!nom_key) err_mesg ("%s: cannot checkOut nom_network\n", argv0);
	}
	else nom_key = act_key;
    }

    time_progress (NULL, NULL);
    print_dashes ();

 /* Read nominal circuit definitions */
    cur_network = NULL;
    if (!f_opt) {
	read_dbase (dmproject, nom_netw_name, nom_def_h, nom_key);
    } else {
	read_sls (nom_netw_name, nom_def_h, x_opt);
    }
    if (!cur_network) err_mesg ("%s: no nom_network (use -v)!\n", argv0);

    nom_netw = cur_network;
    nom_netw_name = nom_netw -> name;
    time_progress ("read network", nom_netw_name);

    instantiate (nom_netw);
    time_progress ("instantiated network", nom_netw_name);
    verb_mesg ("Nominal network %s instantiated.\n", nom_netw_name);

    if (R_opt) print_netw (stderr, nom_netw);
    reduce_netw (nom_netw);
    time_progress ("reduced network", nom_netw_name);

    verb_mesg ("Nominal network %s read.\n", nom_netw_name);

    if (I_opt) print_netw (stderr, nom_netw);

    color_grp (nom_netw);

    if (D_opt) print_ns (stderr, nom_netw);
    if (D_opt) print_grp_col (stderr, nom_netw);

    expand_netw (nom_netw);
    time_progress ("expanded network", nom_netw_name);
    verb_mesg ("Nominal network %s expanded.\n", nom_netw_name);

    if (E_opt) print_netw (stderr, nom_netw);
    print_dashes ();

 /* Read actual circuit definitions */
    cur_network = NULL;
    if (!f_opt) {
	read_dbase (dmproject, act_netw_name, act_def_h, act_key);
    } else {
	read_sls (act_netw_name, act_def_h, x_opt);
    }
    if (!cur_network) err_mesg ("%s: no act_network (use -v)!\n", argv0);

    act_netw = cur_network;
    act_netw_name = act_netw -> name;
    time_progress ("read network", act_netw_name);

    instantiate (act_netw);
    time_progress ("instantiated network", act_netw_name);
    verb_mesg ("Actual network %s instantiated.\n", act_netw_name);

    if (R_opt) print_netw (stderr, act_netw);
    reduce_netw (act_netw);
    time_progress ("reduced network", act_netw_name);

    verb_mesg ("Actual network %s read.\n", act_netw_name);

    if (I_opt) print_netw (stderr, act_netw);

    color_grp (act_netw);

    if (D_opt) print_ns (stderr, act_netw);
    if (D_opt) print_grp_col (stderr, act_netw);

    expand_netw (act_netw);
    time_progress ("expanded network", act_netw_name);
    verb_mesg ("Actual network %s expanded.\n", act_netw_name);

    if (E_opt) print_netw (stderr, act_netw);
    print_dashes ();

    if (s_opt) {
	fprintf (stderr, "STATISTICS:\n");
	fprintf (stderr, "-----------\n\n");
	p_netw_s (stderr, nom_netw);
	p_netw_s (stderr, act_netw);
    }

    verb_mesg ("Starting partitioning process.\n");

    if (match (nom_netw, act_netw)) {
	user_mesg ("\n%s: Succeeded.\n\n", argv0);
	exit_status = 0;
    }
    else {
	user_mesg ("\n%s: Failed.\n\n", argv0);
	exit_status = 2;
    }

    time_progress ("matched networks", NULL);

    if (s_opt) p_part_s (stderr, nom_netw -> part);

    if (m_opt) astats ();

    if (b_opt || anno_opt) print_bindings (std_out, nom_netw -> part);

    if (D_opt) p_hash (stderr, symbol_table);

    if (nom_key && nom_key != act_key) dmCheckIn (nom_key, COMPLETE);
    if (act_key) dmCheckIn (act_key, COMPLETE);

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    my_exit (exit_status);
    return 0;
}
