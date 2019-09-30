static char *rcsid = "$Id: symbol.c,v 1.1 2018/04/30 12:17:52 simon Exp $";
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
 * This module contains a number of functions
 * to maintain a string symbol table.
 * The table is implemented on a hash structure.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	VARIABLES:
 */
Import long n_strn_alloc;
Import long n_strn_mem;
Import hash * symbol_table;

/*
 * Creates a new symbol entry (if necessary).
 * The string is linked into a symbol table to allow multiple use.
 * A reference count is maintained.
 * To be used in conjunction with rm_symbol().
 */
Public string mk_symbol (string str)
{
    string n_str;

    /* see mk_object() in array() */
    if (!str) return (NULL);

    if (symbol_table) {
	if (!(n_str = h_get (symbol_table, str))) {
	    n_str = newmem (str);
	    h_link (symbol_table, n_str, n_str);
	}
    }
    else
	n_str = newmem (str);
    return (n_str);
}

/*
 * Removes a symbol entry (if possible).
 * Since the string can be referenced more than once,
 * it is only deleted when the reference count is zero.
 * To be used in conjunction with mk_symbol().
 */
Public void rm_symbol (string str)
{
    if (!str) return;

    if (symbol_table) {
	short *ref;
	int str_l = strlen (str) + 1;
	ref = (short *) (str + str_l);
	if (*ref-- == 0) {
	    (void) h_unlink (symbol_table, str, 0);
	    n_strn_alloc--;
	    n_strn_mem -= str_l;
	    Free (str);
	}
    }
    else
	deletemem (str);
}
