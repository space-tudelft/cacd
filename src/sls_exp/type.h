/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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

typedef struct path_spec
{
    char name[DM_MAXNAME + 1];
    int xarray[MAXDIM+1][2];
    struct path_spec * next;
    struct path_spec * also;
} PATH_SPEC;

typedef struct node_ref_list
{
    int nx;
    int * xptr;
    struct node_ref_list * next;
} NODE_REF_LIST ;

typedef struct call_list {
    char inst_name[DM_MAXNAME + 1];
    int xarray[MAXDIM+1][2];
    int mcx;
    int number;
    struct call_list * next;
} CALL_LIST;

typedef struct child_list {
    char object[DM_MAXNAME + 1];
    short imported;
    CALL_LIST * calls;
    struct child_list * next;
} CHILD_LIST;

