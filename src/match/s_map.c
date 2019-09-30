static char *rcsid = "$Id: s_map.c,v 1.1 2018/04/30 12:17:50 simon Exp $";
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

#include "src/match/head.h"

#define MEM_ALLOC(type) (type *) malloc (sizeof (type))

int main (int argc, char **argv)
{
    network * s1;
    object * s2;
    block *s3;
    link_type * s4;
    bucket * s5;
    partition * s6;

    if (argc > 1) {
	if (!strcmp (argv[1], "-r")) fprintf (stderr, "release: %s\n", rcsid);
	else fprintf (stderr, "type: s_map -r\n");
	return (0);
    }

    s1 = MEM_ALLOC (network);
    s2 = MEM_ALLOC (object);
    s3 = MEM_ALLOC (block);
    s4 = MEM_ALLOC (link_type);
    s5 = MEM_ALLOC (bucket);
    s6 = MEM_ALLOC (partition);

    fprintf (stderr, "struct network {\n");
    fprintf (stderr, "\tstring            name;       -> %d\n",
	    (int) ((void*) & (s1 -> name) - (void*) s1));
    fprintf (stderr, "\tstruct object     *thead;     -> %d\n",
	    (int) ((void*) & (s1 -> thead) - (void*) s1));
    fprintf (stderr, "\tstruct object     *ttail;     -> %d\n",
	    (int) ((void*) & (s1 -> ttail) - (void*) s1));
    fprintf (stderr, "\tstruct object     *nhead;     -> %d\n",
	    (int) ((void*) & (s1 -> nhead) - (void*) s1));
    fprintf (stderr, "\tstruct object     *ntail;     -> %d\n",
	    (int) ((void*) & (s1 -> ntail) - (void*) s1));
    fprintf (stderr, "\tstruct object     *dhead;     -> %d\n",
	    (int) ((void*) & (s1 -> dhead) - (void*) s1));
    fprintf (stderr, "\tstruct object     *dtail;     -> %d\n",
	    (int) ((void*) & (s1 -> dtail) - (void*) s1));
    fprintf (stderr, "\tstruct partition  *part;      -> %d\n",
	    (int) ((void*) & (s1 -> part) - (void*) s1));
    fprintf (stderr, "\tlong              color;      -> %d\n",
	    (int) ((void*) & (s1 -> color) - (void*) s1));
    fprintf (stderr, "\tshort             n_terms;    -> %d\n",
	    (int) ((void*) & (s1 -> n_terms) - (void*) s1));
    fprintf (stderr, "\tshort             flags;      -> %d\n",
	    (int) ((void*) & (s1 -> flags) - (void*) s1));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(network) = %d bytes\n\n", (int) sizeof (network));


    fprintf (stderr, "struct object {\n");
    fprintf (stderr, "\tstring           name;        -> %d\n",
	    (int) ((void*) & (s2 -> name) - (void*) s2));
    fprintf (stderr, "\tstruct network   *netw;       -> %d\n",
	    (int) ((void*) & (s2 -> netw) - (void*) s2));
    fprintf (stderr, "\tstruct network   *call;       -> %d\n",
	    (int) ((void*) & (s2 -> call) - (void*) s2));
    fprintf (stderr, "\tstruct link_type *head;       -> %d\n",
	    (int) ((void*) & (s2 -> head) - (void*) s2));
    fprintf (stderr, "\tstruct link_type *tail;       -> %d\n",
	    (int) ((void*) & (s2 -> tail) - (void*) s2));
    fprintf (stderr, "\tstruct object    *next_obj;   -> %d\n",
	    (int) ((void*) & (s2 -> next_obj) - (void*) s2));
    fprintf (stderr, "\tstruct object    *next_elm;   -> %d\n",
	    (int) ((void*) & (s2 -> next_elm) - (void*) s2));
    fprintf (stderr, "\tstruct object    *equiv;      -> %d\n",
	    (int) ((void*) & (s2 -> equiv) - (void*) s2));
    fprintf (stderr, "\tstruct block     *block;      -> %d\n",
	    (int) ((void*) & (s2 -> block) - (void*) s2));
    fprintf (stderr, "\tstruct bucket    *par_list;   -> %d\n",
	    (int) ((void*) & (s2 -> par_list) - (void*) s2));
    fprintf (stderr, "\tlong             color;       -> %d\n",
	    (int) ((void*) & (s2 -> color) - (void*) s2));
    fprintf (stderr, "\tlong             edges;       -> %d\n",
	    (int) ((void*) & (s2 -> edges) - (void*) s2));
    fprintf (stderr, "\tint              instance;    -> %d\n",
	    (int) ((void*) & (s2 -> instance) - (void*) s2));
    fprintf (stderr, "\tshort            flavor;      -> %d\n",
	    (int) ((void*) & (s2 -> flavor) - (void*) s2));
    fprintf (stderr, "\tshort            flags;       -> %d\n",
	    (int) ((void*) & (s2 -> flags) - (void*) s2));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(object) = %d bytes\n\n", (int) sizeof (object));


    fprintf (stderr, "struct block {\n");
    fprintf (stderr, "\tstruct  block *next       -> %d\n",
	    (int) ((void*) & (s3 -> next) - (void*) s3));
    fprintf (stderr, "\tstruct  block *prev       -> %d\n",
	    (int) ((void*) & (s3 -> prev) - (void*) s3));
    fprintf (stderr, "\tlong    n_el              -> %d\n",
	    (int) ((void*) & (s3 -> n_el) - (void*) s3));
    fprintf (stderr, "\tobject  *head             -> %d\n",
	    (int) ((void*) & (s3 -> head) - (void*) s3));
    fprintf (stderr, "\tstruct  block *t_nxt      -> %d\n",
	    (int) ((void*) & (s3 -> t_nxt) - (void*) s3));
    fprintf (stderr, "\tstruct  partition *part   -> %d\n",
	    (int) ((void*) & (s3 -> part) - (void*) s3));
    fprintf (stderr, "\tstruct  block **childs    -> %d\n",
	    (int) ((void*) & (s3 -> childs) - (void*) s3));
    fprintf (stderr, "\tstruct  block *parent     -> %d\n",
	    (int) ((void*) & (s3 -> parent) - (void*) s3));
    fprintf (stderr, "\tshort   n_childs          -> %d\n",
	    (int) ((void*) & (s3 -> n_childs) - (void*) s3));
    fprintf (stderr, "\tshort   flags             -> %d\n",
	    (int) ((void*) & (s3 -> flags) - (void*) s3));
    fprintf (stderr, "\tshort   level             -> %d\n",
	    (int) ((void*) & (s3 -> level) - (void*) s3));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(block) = %d bytes\n\n", (int) sizeof (block));


    fprintf (stderr, "struct link_type {\n");
    fprintf (stderr, "\tstruct link_type *next_up;   -> %d\n",
	    (int) ((void*) & (s4 -> next_up) - (void*) s4));
    fprintf (stderr, "\tstruct link_type *next_down; -> %d\n",
	    (int) ((void*) & (s4 -> next_down) - (void*) s4));
    fprintf (stderr, "\tstruct object    *net;       -> %d\n",
	    (int) ((void*) & (s4 -> net) - (void*) s4));
    fprintf (stderr, "\tstruct object    *dev;       -> %d\n",
	    (int) ((void*) & (s4 -> dev) - (void*) s4));
    fprintf (stderr, "\tstruct object    *port;      -> %d\n",
	    (int) ((void*) & (s4 -> port) - (void*) s4));
    fprintf (stderr, "\tlong             color;      -> %d\n",
	    (int) ((void*) & (s4 -> color) - (void*) s4));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(link_type) = %d bytes\n\n", (int) sizeof (link_type));


    fprintf (stderr, "struct bucket {\n");
    fprintf (stderr, "\tstruct bucket *next;       -> %d\n",
	    (int) ((void*) & (s5 -> next) - (void*) s5));
    fprintf (stderr, "\tstruct bucket *prev;       -> %d\n",
	    (int) ((void*) & (s5 -> prev) - (void*) s5));
    fprintf (stderr, "\tstring        key;         -> %d\n",
	    (int) ((void*) & (s5 -> key) - (void*) s5));
    fprintf (stderr, "\tvoid          *data;       -> %d\n",
	    (int) ((void*) & (s5 -> data) - (void*) s5));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(bucket) = %d bytes\n\n", (int) sizeof (bucket));


    fprintf (stderr, "struct partition {\n");
    fprintf (stderr, "\tlong    n_blcks;          -> %d\n",
	    (int) ((void*) & (s6 -> n_blcks) - (void*) s6));
    fprintf (stderr, "\tlong    n_elemts;         -> %d\n",
	    (int) ((void*) & (s6 -> n_elemts) - (void*) s6));
    fprintf (stderr, "\tlong    n_active;         -> %d\n",
	    (int) ((void*) & (s6 -> n_active) - (void*) s6));
    fprintf (stderr, "\tlong    n_passive;        -> %d\n",
	    (int) ((void*) & (s6 -> n_passive) - (void*) s6));
    fprintf (stderr, "\tlong    n_bound;          -> %d\n",
	    (int) ((void*) & (s6 -> n_bound) - (void*) s6));
    fprintf (stderr, "\tlong    n_invalid;        -> %d\n",
	    (int) ((void*) & (s6 -> n_invalid) - (void*) s6));
    fprintf (stderr, "\tlong    n_touched;        -> %d\n",
	    (int) ((void*) & (s6 -> n_touched) - (void*) s6));
    fprintf (stderr, "\tlong    n_iter;           -> %d\n",
	    (int) ((void*) & (s6 -> n_iter) - (void*) s6));
    fprintf (stderr, "\tblock   *active;          -> %d\n",
	    (int) ((void*) & (s6 -> active) - (void*) s6));
    fprintf (stderr, "\tblock   *passive;         -> %d\n",
	    (int) ((void*) & (s6 -> passive) - (void*) s6));
    fprintf (stderr, "\tblock   *invalid;         -> %d\n",
	    (int) ((void*) & (s6 -> invalid) - (void*) s6));
    fprintf (stderr, "\tblock   *touched;         -> %d\n",
	    (int) ((void*) & (s6 -> touched) - (void*) s6));
    fprintf (stderr, "\tblock   *hist;            -> %d\n",
	    (int) ((void*) & (s6 -> hist) - (void*) s6));
    fprintf (stderr, "};\n");
    fprintf (stderr, "\tsizeof(partition) = %d bytes\n\n", (int) sizeof (partition));

    return (0);
}
