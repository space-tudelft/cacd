/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
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

#include "src/cmsk/extern.h"

struct ptree *pat_tree;

int check_tree (char *name)
{
    return (ck_ptree (name, pat_tree));
}

int ck_ptree (char *name, struct ptree *root)
{
    int i;

    if (root) {
	i = strcmp (name, root -> name);
	if (i > 0)
	    return (ck_ptree (name, root -> rchild));
	if (i < 0)
	    return (ck_ptree (name, root -> lchild));
	/*
	** pattern name found
	*/
	tree_ptr = root;
	return (1);
    }
    return (0);
}

void add_pat_tree ()
{
    add_ptree (&pat_tree);
}

void add_ptree (struct ptree **root)
{
    int i;

    if (*root) {
	i = strcmp (pat_name, (*root) -> name);
	if (i > 0)
	    add_ptree (&(*root) -> rchild);
	if (i < 0)
	    add_ptree (&(*root) -> lchild);
    }
    else {
	ALLOC_STRUCT (tree_ptr, ptree);
	strcpy (tree_ptr -> name, pat_name);
	tree_ptr -> bbox = 0;
	tree_ptr -> impcell = 0;
	tree_ptr -> correct = 1;
	tree_ptr -> rchild = 0;
	tree_ptr -> lchild = 0;
	*root = tree_ptr;
    }
}

void mk_patdir () /* make pattern directory */
{
    DM_STREAM *fp;
    struct stat statBuf;

    pat_key = dmCheckOut (dmproject, pat_name, WORKING, DONTCARE, LAYOUT, UPDATE);

    fpbox  = dmOpenStream (pat_key, "box",  "w");
    fpinfo = dmOpenStream (pat_key, "info", "w");
    fpmc   = dmOpenStream (pat_key, "mc",   "w");
    fpnor  = dmOpenStream (pat_key, "nor",  "w");

    fp = dmOpenStream (pat_key, "term", "w");
    dmCloseStream (fp, COMPLETE);

    if (dmStat (pat_key, "annotations", &statBuf) != -1) {
        dmUnlink (pat_key, "annotations");
    }
    if (dmStat (pat_key, "torname", &statBuf) != -1) {
        dmUnlink (pat_key, "torname");
    }
}

void closepatdir (int mode)
{
    dmCloseStream (fpbox, COMPLETE);
    dmCloseStream (fpmc, COMPLETE);
    dmCloseStream (fpnor, COMPLETE);
    dmCloseStream (fpinfo, COMPLETE);
    dmCheckIn (pat_key, mode);
    pat_key = NULL;
}
