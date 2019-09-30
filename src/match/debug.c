static char *rcsid = "$Id: debug.c,v 1.1 2018/04/30 12:17:27 simon Exp $";
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
 * Contains a number of debugging functions.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*
 * Prints the specified object in a standard format
 * on the specified stream.
 */
Public void print_object (FILE *fp, object *ThiS)
{
    fprintf (fp, "Print of object '%s':\n", ThiS -> name);
    fprintf (fp, "-------------------------\n\n");
    if (ThiS == NULL) {
	fprintf (fp, "NULL object\n\n");
	return;
    }
    fprintf (fp, "Name:              ");
    if (ThiS -> name)
	fprintf (fp, "%s\n", ThiS -> name);
    else
	fprintf (fp, "NULL\n");
    fprintf (fp, "type:              ");
    switch (Get_field (ThiS, TYPE)) {
	case TERMINAL:
	    fprintf (fp, "TERMINAL\n");
	    break;
	case NET:
	    fprintf (fp, "NET\n");
	    break;
	case DEVICE:
	    fprintf (fp, "DEVICE\n");
	    break;
	default:
	    fprintf (fp, "SUBTERM\n");
	    break;
    }
    fprintf (fp, "state:             ");
    switch (Get_field (ThiS, STATE)) {
	case PASSIVE:
	    fprintf (fp, "PASSIVE\n");
	    break;
	case ACTIVE:
	    fprintf (fp, "ACTIVE\n");
	    break;
	case INVALID:
	    fprintf (fp, "INVALID\n");
	    break;
	default:
	    fprintf (fp, "UNDEFINED\n");
	    break;
    }
    fprintf (fp, "instance:          %u\n", ThiS -> instance);
    fprintf (fp, "color:             %ld\n", ThiS -> color);
    fprintf (fp, "edges:             %ld\n", ThiS -> edges);
    fprintf (fp, "netw:              ");
    if (ThiS -> netw == NULL)
	fprintf (fp, "NULL\n");
    else {
	if (ThiS -> netw -> name)
	    fprintf (fp, "%s\n", ThiS -> netw -> name);
	else
	    fprintf (fp, "?? (Netw has no name)\n");
    }
    fprintf (fp, "call:              ");
    if (ThiS -> call == NULL)
	fprintf (fp, "NULL\n");
    else {
	if (ThiS -> call -> name)
	    fprintf (fp, "%s\n", ThiS -> call -> name);
	else
	    fprintf (fp, "?? (Call has no name)\n");
    }
    fprintf (fp, "head:              ");
    if (ThiS -> head == NULL)
	fprintf (fp, "NULL\n");
    else
	fprintf (fp, "XXXX\n");
    fprintf (fp, "tail:              ");
    if (ThiS -> tail == NULL)
	fprintf (fp, "NULL\n");
    else
	fprintf (fp, "XXXX\n");
    fprintf (fp, "next_obj:          ");
    if (ThiS -> next_obj == NULL)
	fprintf (fp, "NULL\n");
    else
	fprintf (fp, "XXXX\n");
    fprintf (fp, "next_elm:          ");
    if (ThiS -> next_elm == NULL)
	fprintf (fp, "NULL\n");
    else
	fprintf (fp, "XXXX\n");
    fprintf (fp, "block:             ");
    if (ThiS -> block == NULL)
	fprintf (fp, "NULL\n");
    else
	fprintf (fp, "XXXX\n");
    while (ThiS -> next_obj != NULL) {
	ThiS = ThiS -> next_obj;
	if (ThiS -> next_obj == NULL) {
	    fprintf (fp, "Last array element ");
	    if (ThiS -> name)
		fprintf (fp, "%s\n", ThiS -> name);
	    else
		fprintf (fp, "?? No name\n");
	}
    }
    fprintf (fp, "\n");
}

/*
 * Prints the Neighbour Structure of a network on
 * the specified stream.
 * Only used for debugging purposes.
 */
Public void print_ns (FILE *fp, network *netw)
{
    object *dev, *net;
    link_type *lnk;
    char *n1, *n2, *unk = "??";

    fprintf (fp, "Neighbour structure of network %s\n", netw -> name);
    fprintf (fp, "--------------------------------------\n\n");

    fprintf (fp, "DEVICES:\n\n");
    for (dev = netw -> dhead; dev; dev = dev -> next_obj) {
	fprintf (fp, "%s: %s\n", dev -> name, dev -> call -> name);
	for (lnk = dev -> head; lnk; lnk = lnk -> next_down) {
	    n1 = lnk -> net ? lnk -> net -> name : unk;
	    n2 = lnk -> port? lnk -> port-> name : unk;
	    fprintf (fp, "\t%s (%s)\t%ld\n", n1, n2, lnk -> color);
	}
    }

    fprintf (fp, "\nTERMINALS:\n\n");
    for (net = netw -> thead; net; net = net -> next_obj) {
	fprintf (fp, "%s:\n", net -> name);
	if (net -> equiv && net -> equiv != net) {
	    fprintf (fp, "\tequiv: %s\n", net -> equiv -> name);
	}
	else
	for (lnk = net -> head; lnk; lnk = lnk -> next_up) {
	    n1 = lnk -> dev ? lnk -> dev -> name : unk;
	    n2 = lnk -> port? lnk -> port-> name : unk;
	    fprintf (fp, "\t%s.%s\t%ld\n", n1, n2, lnk -> color);
	}
    }

    fprintf (fp, "\nNETS:\n\n");
    for (net = netw -> nhead; net; net = net -> next_obj) {
	fprintf (fp, "%s:\n", net -> name);
	if (net -> equiv && net -> equiv != net) {
	    fprintf (fp, "\tequiv: %s\n", net -> equiv -> name);
	}
	else
	for (lnk = net -> head; lnk; lnk = lnk -> next_up) {
	    n1 = lnk -> dev ? lnk -> dev -> name : unk;
	    n2 = lnk -> port? lnk -> port-> name : unk;
	    fprintf (fp, "\t%s.%s\t%ld\n", n1, n2, lnk -> color);
	}
    }

    fprintf (fp, "\n-- print_ns DONE\n");
}

/*
 * Prints the specified partition of a network in a
 * standard format on the specified stream.
 * Used for debugging purposes.
 */
Public void print_p (FILE *fp, partition *p)
{
    block *ThiS;
    block *ref;
    object * that;
    string str;
    string name;

    if (p -> active)
	name = p -> active -> head -> netw -> name;
    else if (p -> passive)
	name = p -> passive -> head -> netw -> name;
    else if (p -> touched)
	name = p -> touched -> head -> netw -> name;
    else if (p -> invalid)
	name = p -> invalid -> head -> netw -> name;
    else
	name = "???";

    fprintf (fp, "Partition of network '%s':\n", name);
    fprintf (fp, "--------------------------------\n\n");
    fprintf (fp, "n_blcks:\t%ld\n", p -> n_blcks);
    fprintf (fp, "n_elemts:\t%ld\n", p -> n_elemts);
    fprintf (fp, "n_active:\t%ld\n", p -> n_active);
    fprintf (fp, "n_passive:\t%ld\n", p -> n_passive);
    fprintf (fp, "n_invalid:\t%ld\n", p -> n_invalid);
    fprintf (fp, "n_bound:\t%ld\n", p -> n_bound);
    fprintf (fp, "{\n");

    ThiS = p -> active;
    ref = ThiS;
    str = "ACTIVE";
    do {
	if (ThiS == NULL) break;

	fprintf (fp, "\t(\t%s  [%ld]", str, ThiS -> n_el);

	if (Get_flag (ThiS, BOUNDED)) fprintf (fp, "  <bound>");
	if (Get_flag (ThiS, TOUCHED)) fprintf (fp, "  <touched>");

	fprintf (stderr, "\n");

	for (that = ThiS -> head; that != NULL; that = that -> next_elm) {
	    fprintf (fp, "\t\t%s\t%lu/%lu", that -> name, that -> color, that -> edges);
	    if (that -> next_elm != NULL) fprintf (fp, ",");
	    fprintf (fp, "\n");
	}
	fprintf (fp, "\t),\n");

	ThiS = ThiS -> next;
    } while (ThiS != ref);

    ThiS = p -> passive;
    ref = ThiS;
    str = "PASSIVE";
    do {
	if (ThiS == NULL) break;

	fprintf (fp, "\t(\t%s  [%ld]", str, ThiS -> n_el);

	if (Get_flag (ThiS, BOUNDED)) fprintf (fp, "  <bound>");
	if (Get_flag (ThiS, TOUCHED)) fprintf (fp, "  <touched>");

	fprintf (stderr, "\n");

	for (that = ThiS -> head; that != NULL; that = that -> next_elm) {
	    fprintf (fp, "\t\t%s\t%lu/%lu", that -> name, that -> color, that -> edges);
	    if (that -> next_elm != NULL) fprintf (fp, ",");
	    fprintf (fp, "\n");
	}
	fprintf (fp, "\t),\n");

	ThiS = ThiS -> next;
    } while (ThiS != ref);

    ThiS = p -> invalid;
    ref = ThiS;
    str = "INVALID";
    do {
	if (ThiS == NULL) break;

	fprintf (fp, "\t(\t%s  [%ld]", str, ThiS -> n_el);

	if (Get_flag (ThiS, BOUNDED)) fprintf (fp, "  <bound>");
	if (Get_flag (ThiS, TOUCHED)) fprintf (fp, "  <touched>");

	fprintf (stderr, "\n");

	for (that = ThiS -> head; that != NULL; that = that -> next_elm) {
	    fprintf (fp, "\t\t%s\t%lu/%lu", that -> name, that -> color, that -> edges);
	    if (that -> next_elm != NULL) fprintf (fp, ",");
	    fprintf (fp, "\n");
	}
	fprintf (fp, "\t),\n");

	ThiS = ThiS -> next;
    } while (ThiS != ref);

    fprintf (fp, "}\n\n");
}

/*
 * Prints the refinement 'history' of the
 * specified block in a standard format on the stream fp.
 * The function must be invokated with level=0.
 * Only used for debugging purposes.
 */
Public void print_hist (FILE *fp, block *b, short level)
{
    long     i;
    block *child;
    object * ThiS;

    fprintf (fp, "Block history\n");
    fprintf (fp, "-------------\n\n");

    for (i = 0; i < level; i++)	/* indentation */
	fprintf (fp, "|  ");

    fprintf (fp, "(\n");
    if (b -> childs != NULL) {
	for (i = 0; i < b -> n_childs; i++) {
	    child = *(b -> childs + i);
	    print_hist (fp, child, level + 1);
	}
    }
    else {
	for (ThiS = b -> head; ThiS != NULL; ThiS = ThiS -> next_elm) {
	    for (i = 0; i < level; i++)
		fprintf (fp, "|  ");
	    fprintf (fp, " %s", ThiS -> name);
	    if (ThiS -> next_elm != NULL)
		fprintf (fp, ",");
	    fprintf (fp, "\n");
	}
    }
    for (i = 0; i < level; i++)
	fprintf (fp, "|  ");
    fprintf (fp, ")\n\n");
}

/*
 * Prints the group colors of the specified
 * hierarchical network on the stream fp.
 * Only used for debugging purposes.
 */
Public void print_grp_col (FILE *fp, network *netw)
{
    object * dev;

    fprintf (fp, "---> %s/%lu, childs:\n", netw -> name, netw -> color);

    for (dev = netw -> dhead; dev != NULL; dev = dev -> next_obj) {
	fprintf (fp, "\t\t%s/%lu\n", dev -> call -> name, dev -> call -> color);
    }

    fprintf (fp, "\n");

    for (dev = netw -> dhead; dev != NULL; dev = dev -> next_obj) {
	print_grp_col (fp, dev -> call);
    }

    fprintf (fp, "<--- (%s/%lu)\n\n", netw -> name, netw -> color);
}
