static char *rcsid = "$Id: read.c,v 1.1 2018/04/30 12:17:47 simon Exp $";
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
 * This module contains some functions to read a network
 * from the database into a datastructure.
 * A pointer to the hierarchical datastructure is returned.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	VARIABLES:
 */
Import boolean D_opt;

Public char *comm = "xsls -h";

Public hash * def_h;
Public string cur_file;

Import FILE * yyin;
Import int   yylineno;
Import long  n_netw_alloc;

#define MAX_PRIMS 40
Private string prim_list[MAX_PRIMS];
Private int no_prims = 0;

/*
 * Make an empty network.
 * Allocates space for a network structure.
 * A pointer to the structure is returned.
 * The structure is properly initialized.
 */
Public network *mk_network (string name, int type)
{
    network * netw;

    Assert (type == COMPOUND || type == PRIMITIVE);

    Malloc (netw, 1, network);

    netw -> name = mk_symbol (name);
    netw -> thead = NULL;
    netw -> ttail = NULL;
    netw -> nhead = NULL;
    netw -> ntail = NULL;
    netw -> dhead = NULL;
    netw -> dtail = NULL;
    netw -> part  = NULL;
    netw -> color = 0;
    netw -> n_terms = 0;
    netw -> flags = 0;
    Set_field (netw, TYPE, type);
    if (type == PRIMITIVE) register_prim (netw -> name);

    n_netw_alloc++;

    return (netw);
}

/*
 * Removes a network datastructure pointed to by 'netw'.
 */
Public void rm_network (network *netw)
{
    object *ThiS, *next;

    rm_symbol (netw -> name);	/* free name */

    for (ThiS = netw -> thead; ThiS; ThiS = next) { /* free terminal list */
	next = ThiS -> next_obj;
	rm_object (ThiS);
    }

    for (ThiS = netw -> nhead; ThiS; ThiS = next) { /* free net list */
	next = ThiS -> next_obj;
	rm_object (ThiS);
    }

    for (ThiS = netw -> dhead; ThiS; ThiS = next) { /* free device list */
	next = ThiS -> next_obj;
	rm_object (ThiS);
    }

    n_netw_alloc--;

    Free (netw);
}

/*
 * Reads a number of sls specifications from the specified file.
 * A linked list of hash structures determine the known specifications
 * form earlier 'reads' (primitive element file, libs etc.).
 * A linked list of hash structures, with the hash structure corresponding
 * with the new sls specifications inserted at the beginning of the
 * original list, is returned.
 */
Public void read_sls (string path, hash *hashlist, boolean db_flag)
{
    char   xsls_comm[1024];
    FILE * fp;
    static int first = 1;

    if (db_flag == False) {
	if (D_opt) {
	    user_mesg ("reading sls description from file %s\n", path);
	}
	if ((fp = fopen (path, "r")) == NULL) {
	    err_mesg ("Cannot fopen file '%s'\n", path);
	}
    }
    else {
	sprintf (xsls_comm, "%s %s", comm, path);
	if (D_opt) {
	    user_mesg ("extracting sls description with command %s\n", comm);
	    user_mesg ("executing command %s\n", xsls_comm);
	}
	if ((fp = popen (xsls_comm, "r")) == NULL) {
	    err_mesg ("Cannot popen file '%s'\n", path);
	}
    }

    def_h = hashlist;

    cur_file = path;
    yyin = fp;
    yylineno = 1;
    if (!first) yyrestart (yyin);
    else first = 0;
    (void) yyparse ();
    if (db_flag == False) {
	(void) fclose (yyin);
    }
    else {
	(void) pclose (yyin);
    }
}

int yywrap ()
{
    return 1;
}

/*
 * Registers a primitive network.
 * The position in the prim_list will be used
 * in the initial partitioning procedure.
 */
Public void register_prim (string name)
{
    int  index;
    for (index = 0; index < no_prims; index++) {
	if (strcmp (name, prim_list[index]) == 0) return;
    }
    if (no_prims >= MAX_PRIMS) {
	err_mesg ("Error: too many primitive networks (> %d)\n", MAX_PRIMS);
    }
    prim_list[no_prims++] = name;
}

/*
 * Returns the index of a primitive network in the prim_list.
 */
Public int find_prim_index (string name)
{
    int  index;
    for (index = 0; index < no_prims; index++) {
	if (strcmp (name, prim_list[index]) == 0) return index;
    }
    err_mesg ("Error: primitive network '%s' has not been registered\n", name);
    return -1;
}
