static char *rcsid = "$Id: stack.c,v 1.1 2018/04/30 12:17:51 simon Exp $";
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
 * This module contains a set of functions to maintain a stack.
 * The functions are based on the queue datastructure.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*
 * Creates a stack datastructure.
 */
Public stack *mk_a_stack ()
{
    return (mk_a_queue ());
}

/*
 * Pops a data item from a stack.
 * NULL is returned when the stack is empty.
 */
Public void *pop (stack *ThiS_stack)
{
    return (del_queue (ThiS_stack, HEAD));
}

/*
 * Pushes a data item on a stack.
 */
Public void push (stack *ThiS_stack, void *data)
{
    Assert (data);
    add_queue (ThiS_stack, data, HEAD);
}
