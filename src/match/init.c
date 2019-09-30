static char *rcsid = "$Id: init.c,v 1.1 2018/04/30 12:17:32 simon Exp $";
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
 * Basic initialisation module.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	VARIABLES:
 */
Public hash * symbol_table;
Public hash * nom_def_h;
Public hash * act_def_h;
Public stack * net_st;
Public network * nom_netw;
Public network * act_netw;

Import boolean o_opt;
Import boolean p_opt;
Import string p_val;
Import boolean l_opt;
Import string l_val;
Import FILE * std_out;
Import DM_PROJECT * dmproject;

/* #define SYMBOL_PRIME	128 */

/*
 * Basic initialisation function.
 */
Public void my_init ()
{
 /* create symbol table */
#ifdef SYMBOL_PRIME
    symbol_table = mk_hash (SYMBOL_PRIME);
#endif

 /* create stack for net statements */
    net_st = mk_a_stack ();

    nom_def_h = mk_hash (256);
    act_def_h = mk_hash (256);

    if (p_opt) {
      if (*p_val) {
	/* read user specified primitive description file */
	read_sls (p_val, nom_def_h, False);
	read_sls (p_val, act_def_h, False);
	verb_mesg ("User primitive element file '%s' read.\n", p_val);
      }
    }
    else if (dmproject) {
	string prim_path;
	prim_path = dmGetMetaDesignData (PROCPATH, dmproject, DEFAULT_PRIM_FILE);
	if (!prim_path) err_mesg ("Cannot get '%s' path\n", DEFAULT_PRIM_FILE);
	/* read default primitive description file */
	read_sls (prim_path, nom_def_h, False);
	read_sls (prim_path, act_def_h, False);
	verb_mesg ("Standard primitive element file read.\n");
    }

    if (l_opt) {
	/* read user specified sls library file */
	read_sls (l_val, nom_def_h, False);
	read_sls (l_val, act_def_h, False);
	verb_mesg ("User library file '%s' read.\n", l_val);
    }
}

/*
 * Basic termination function.
 */
Public void my_exit (int status)
{
    if (o_opt) fclose (std_out);
    dmQuit ();
    exit (status);
}
