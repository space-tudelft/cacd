/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	P. van der Wolf
 *	H.T. Fassotte
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

#include "src/dali/header.h"

#define MAX_POINTS 10000

extern struct Disp_wdw *p_wdw;
extern DM_PROJECT *dmproject;
extern DM_CELL *ckey;
extern char *cellstr;
extern qtree_t **quad_root;
extern TERM **term_root;
extern INST *inst_root;
extern char *yes_no[];
extern int  Default_expansion_level;
extern int  NR_lay;
extern int  rmode;
extern int  erase_text;
extern int  exp_level;
extern int  initwindow_flag;
extern int  retain_oldw;

TERMREF *termlist, *termlistlast;
int  ask_again = 0;
static int update_mode = 0;

/*
** Read a cell. There are four possibilities:
** 1. The parts which might be in the workspace were
**    build out of nothing and were not written to the
**    database. So there is no name (cell) connected to
**    the workspace. The key is NULL.
** 2. The parts in the workspace were build out of
**    nothing but during a write-action they got a name,
**    and a cell within the database is related to
**    these parts in the workspace: the key is not NULL
** 3. The parts in the workspace were read from a cell
**    but there hasn't been a write-action since then.
**    Because the workspace was derived from a cell
**    the key is not NULL.
** 4. The parts in the workspace were read from a cell
**    and they were written back to that cell too.
**    The key is not NULL.
**
**    None of these possibilities can be related to an
**    empty workspace.
**
**    Now that we are going to read another cell, we
**    have to check_in the old one that might be in the
**    workspace and that might already be written to
**    the database.
** 1. The workspace is not related to a cell so we
**    don't have a key: we don't have to check_in.
** 2. The workspace was written to a (new) cell and we
**    have a key. It is basically the same situation as
**    if we had read it from the database and written
**    it again: see 4. So we have to check_in this cell.
** 3. We just read a cell and got a key, but we didn't
**    write a changed version. We have to check_in and
**    the database-manager will know that we didn't
**    write it. He will not create a new version, but
**    just use the check_in to remove the lock.
** 4. The same situation as 2. check_in.
*/
int inp_mod (char *c_name)
{
    char  err_str[MAXCHAR];
    int   rv;
    int   works = !no_works ();

    if (!c_name) {
	char *c_name_prev;
	/* Read a fresh local celllist. */
	if (read_modlist () == -1) goto ret;
	ask_again = 1;
	do {
	    if (ask_again == 2) { /* c_name returned by ask_cell */
		if (rmode || !works) /* read-only mode or no_works */
		    sprintf (err_str, "Read cell '%s' ?", c_name);
		else
		    sprintf (err_str, "Read cell '%s', erase workspace?", c_name);
		ptext (err_str);
		c_name_prev = c_name;
	    }
	    else c_name_prev = NULL;
	    if (!(c_name = ask_cell (LOCAL))) goto ret; /* cancel */
	} while (ask_again != 2 || c_name != c_name_prev);
	ask_again = 0;
    }

    if (!update_mode) {
	save_oldw (); /* save current window for prev_w */
	retain_oldw = 1;
    }

    if (works) {
	if (!update_mode) initwindow (); /* set default window */
	empty_mem (); /* performs also an init_mem () */
	pict_all (ERAS_DR);
    }
    else { /* workspace already empty */
	init_mem ();
    }

    sprintf (err_str, "Reading cell '%s'", c_name);
    set_titlebar (err_str);
    ptext (err_str);
    if (!(cellstr = strsave (c_name, 0))) goto ret;
    if ((rv = _dmExistCell (dmproject, cellstr, LAYOUT)) <= 0 ||
	!(ckey = dmCheckOut (dmproject, cellstr, WORKING, DONTCARE, LAYOUT, READONLY))) {
	if (rv == 0) {
	    sprintf (err_str, "Cell '%s' does not exist!", cellstr);
	    btext (err_str);
	    sleep (3);
	}
	FREE (cellstr);
	cellstr = NULL;
	goto ret;
    }
 /*
  ** If cell did not yet exist and an empty cell was created?
  ** Opening of files will not proceed properly, so error
  ** will be detected automatically.
  */
    if (inp_boxnorfl (ckey, quad_root) == -1 ||
	    inp_mcfl (ckey, &inst_root) == -1 ||
	    inp_term (ckey, term_root) == -1) {
	sprintf (err_str, "Can't read cell '%s' properly!", cellstr);
	btext (err_str);
	empty_mem (); /* performs also an init_mem () */
	sleep (1);
	goto ret;
    }

    if (inp_comment (ckey) == -1) {
	sprintf (err_str, "Warning: can't read comments of cell '%s'", cellstr);
	ptext (err_str);
	sleep (1);
    }

    if (Default_expansion_level != 1) {
	exp_level = 0; /* set unused exp.level */
	expansion (Default_expansion_level);
    }
    if (!update_mode) {
	++initwindow_flag;
	bound_w (); /* set max. picture window? */
	--initwindow_flag;
    }
    inform_cell ();
ret:
    ask_again = 0;
    retain_oldw = 0;
    set_titlebar (NULL);
    return (1);
}

int upd_mod ()
{
    int rv = 0;
    if (cellstr) {
	char err_str[MAXCHAR];
	int old_level;
	if (!rmode) {
	    sprintf (err_str, "Update cell '%s', erase workspace?", cellstr);
	    ptext (err_str);
	    erase_text = 1;
	    if (ask (2, yes_no, -1) <= 0) return rv; /* cancel */
	}
	update_mode = 1;
	old_level = Default_expansion_level;
	Default_expansion_level = exp_level;
	strcpy (err_str, cellstr);
	rv = inp_mod (err_str);
	Default_expansion_level = old_level;
	update_mode = 0;
    }
    return rv;
}

int eras_worksp (char *c_name)
{
    if (!c_name && !rmode) {
	ptext ("Erase workspace, are you sure?");
	erase_text = 1;
	if (ask (2, yes_no, -1) <= 0) return (0); /* cancel */
    }
    save_oldw (); /* save current window for prev_w */
    initwindow (); /* set default window */
    empty_mem (); /* performs also an init_mem () */
    pict_all (ERAS_DR);
    return (1);
}

int inp_boxnorfl (DM_CELL *key, qtree_t *quads[])
{
    struct obj_node *trap_list;
#ifdef ED_DEBUG
    struct obj_node *p;
#endif
    DM_STREAM *finp;
    struct obj_node *trap;
    char    err_str[MAXCHAR];
    char   *msg = "read error";
    register long i, j;
    Coor dx, dy;
    Coor line_x[MAX_POINTS + 1], line_y[MAX_POINTS + 1];
    int n, nr_c, skip_data;

    if (!(finp = dmOpenStream (key, "box", "r"))) return (-1);

    while (dmGetDesignData (finp, GEO_BOX) > 0) {
	if (gbox.layer_no < 0L || gbox.layer_no >= NR_lay) {
	    sprintf (err_str, "%s: Bad box layer number (%ld)!",
		key -> cell, gbox.layer_no);
	    ptext (err_str);
	    sleep (1);
	    continue;
	}

	if (gbox.xr < gbox.xl) { Coor x = gbox.xl; gbox.xl = gbox.xr; gbox.xr = x; }
	if (gbox.yt < gbox.yb) { Coor y = gbox.yb; gbox.yb = gbox.yt; gbox.yt = y; }

	for (i = 0L; i <= gbox.nx; i++) {
	    for (j = 0L; j <= gbox.ny; j++) {
		MALLOC (trap, struct obj_node);
		trap -> ll_x1 = (Coor) ((gbox.xl + i * gbox.dx) * QUAD_LAMBDA);
		trap -> ll_x2 = (Coor) ((gbox.xr + i * gbox.dx) * QUAD_LAMBDA);
		trap -> ll_y1 = (Coor) ((gbox.yb + j * gbox.dy) * QUAD_LAMBDA);
		trap -> ll_y2 = (Coor) ((gbox.yt + j * gbox.dy) * QUAD_LAMBDA);
		trap -> sides = 0;
	    /*
	    ** We can never be sure if the primitives that are read
	    ** from the database will not have any overlaps.
	    ** However, we now insert (box-) trapezoids right away.
	    ** The quad-tree and its operations can handle this.
	    ** Thus, nonoverlapping elements are only enforced
	    ** during manual updates to prevent the data from
	    ** becoming too fragmented and is not essential otherwise.
	    */
		quads[gbox.layer_no] = quad_insert (trap, quads[gbox.layer_no]);
	    }
	}
    }
    dmCloseStream (finp, COMPLETE);

 /*
  ** This marvellous layout editor has been extended to support
  ** non-orthogonal layout features.  So, we have to read them.
  */
    if (!(finp = dmOpenStream (key, "nor", "r"))) return (-1);

    while (dmGetDesignData (finp, GEO_NOR_INI) > 0) {

	skip_data = 0; /* NO */

	if (!(gnor_ini.elmt == POLY_NOR
		    || gnor_ini.elmt == RECT_NOR
		    || gnor_ini.elmt == SBOX_NOR)) {
	    sprintf (err_str, "%s: Unknown nor element code (%ld)!",
		key -> cell, gnor_ini.elmt);
	    goto skip;
	}
	if (gnor_ini.layer_no < 0L || gnor_ini.layer_no >= NR_lay) {
	    sprintf (err_str, "%s: Bad nor layer number (%ld)!",
		key -> cell, gnor_ini.layer_no);
	    goto skip;
	}
	if (gnor_ini.elmt == RECT_NOR && gnor_ini.no_xy != 4) {
	    sprintf (err_str, "%s: Illegal nor RECT elmt! (no_xy = %ld)",
		key -> cell, gnor_ini.no_xy);
	    goto skip;
	}
	if (gnor_ini.elmt == SBOX_NOR && gnor_ini.no_xy != 2) {
	    sprintf (err_str, "%s: Illegal nor SBOX elmt! (no_xy = %ld)",
		key -> cell, gnor_ini.no_xy);
	    goto skip;
	}
	if (gnor_ini.no_xy > MAX_POINTS) {
	    sprintf (err_str, "%s: Too many nor POLY points (%ld)!",
		key -> cell, gnor_ini.no_xy);
	    goto skip;
	}

	ASSERT (!skip_data);

	if (skip_data) {
skip:
	/*
	 ** This part can only be entered via skip-label.
	 */
	    ptext (err_str);
	    sleep (1);

	/* Read remaining xy-pairs. */
	    for (j = (int) gnor_ini.no_xy; j > 0; j--) {
		if (dmGetDesignData (finp, GEO_NOR_XY) <= 0) {
		/* Things are not going too well. */
		    break;
		}
	    }
	    continue;
	}

    /* INI-element was OK. Process xy-pairs. */

	nr_c = 0;
	if (gnor_ini.elmt == POLY_NOR || gnor_ini.elmt == RECT_NOR) {

	    for (j = (int) gnor_ini.no_xy; j > 0; --j) {
		if (dmGetDesignData (finp, GEO_NOR_XY) <= 0) {
		/* Signal error, and break from loop. */
		    skip_data = 1; /* YES */
		    break;
		}
		line_x[nr_c] = (Coor) Round (gnor_xy.x * (double) QUAD_LAMBDA);
		line_y[nr_c] = (Coor) Round (gnor_xy.y * (double) QUAD_LAMBDA);
		++nr_c;
	    }

	    if (!skip_data) {
		ASSERT (gnor_ini.no_xy == nr_c);
		if (nr_c < 3) {
		/* Polygon should have at least 3 corner points. */
		    skip_data = 1; /* YES */
		    msg = "< 3 corner points";
		}
	    }
	}
	else {
	    ASSERT (gnor_ini.elmt == SBOX_NOR);
	    if (dmGetDesignData (finp, GEO_NOR_XY) <= 0) {
		skip_data = 1; /* YES */
	    }
	    else {
		line_x[0] = (Coor) Round (gnor_xy.x * (double) QUAD_LAMBDA);
		line_y[0] = (Coor) Round (gnor_xy.y * (double) QUAD_LAMBDA);

		if (dmGetDesignData (finp, GEO_NOR_XY) <= 0) {
		    skip_data = 1; /* YES */
		}
		else {
		    line_x[2] = (Coor) Round (gnor_xy.x * (double) QUAD_LAMBDA);
		    line_y[2] = (Coor) Round (gnor_xy.y * (double) QUAD_LAMBDA);

		    j = ((line_x[2] - line_x[0]) + (line_y[2] - line_y[0])) / 2;

		    line_x[1] = line_x[0] + j;
		    line_y[1] = line_y[0] + j;
		    line_x[3] = line_x[2] - j;
		    line_y[3] = line_y[2] - j;
		    nr_c = 4;
		}
	    }
	}

	if (!skip_data) {
	/*
	 ** Close polygon for internal conversion routine.
	 */
	    line_x[nr_c] = line_x[0];
	    line_y[nr_c] = line_y[0];
	    ++nr_c;
	/*
	** Check whether this polygon satisfies the constraints of
	** Dali.  That is, check for orthogonality / 45 degrees.
	** Also the coordinates of a slanting edge should be proper
	** points on the internal QUAD_LAMBDA grid: (even, even) or
	** (odd, odd).  (This prevents cross-points below the
	** QUAD_LAMBDA-resolution.)
	*/
	    for (j = 1; j < nr_c; ++j) { /* not for first point */
		dx = line_x[j] - line_x[j - 1];
		dy = line_y[j] - line_y[j - 1];
		if (!dx && !dy) { /* Signal zero edge. */
		    skip_data = 1; /* YES */
		    msg = "zero edge";
		    break;
		}
		if (dx && dy) {	/* Slanting edge. */
		    if (!(Abs (dx) == Abs (dy))) { /* Signal invalid slanting edge. */
			skip_data = 1; /* YES */
			msg = "not 45 degree edge";
			break;
		    }
		    if (!((Abs (line_x[j])) % 2 == (Abs (line_y[j])) % 2)) { /* Signal invalid point. */
			skip_data = 1; /* YES */
			msg = "point position";
			break;
		    }
		}
	    }
	}

	/*
	** Invalid properties may have been detected.
	*/
	if (skip_data) {
	    sprintf (err_str, "%s: Invalid nor elmt (%s)! (skipped)", key -> cell, msg);
	    ptext (err_str);
	    sleep (1);
	    continue;
	}

    /*
     ** Polygon seems to be OK, try to store it.
     */
    dx = dy = 0;
for (i = 0;;) {
    for (j = 0;;) {
#ifdef ED_DEBUG
	PE "polygon: nr_c = %d, ini.no_xy = %d\n", nr_c, (int) gnor_ini.no_xy);
	for (n = 0; n < nr_c; ++n) {
	    PE "\tx = %ld,\ty = %ld\n", (long) line_x[n], (long) line_y[n]);
	}
#endif /* ED_DEBUG */

	trap_list = poly_trap (line_x, line_y, nr_c);

#ifdef ED_DEBUG
	for (p = trap_list; p; p = p -> next) {
	    PE "x1 = %ld, x2 = %ld, y1 = %ld, y2 = %ld\n",
		(long) p -> ll_x1, (long) p -> ll_x2, (long) p -> ll_y1, (long) p -> ll_y2);
	    PE "\tls = %d, rs = %d, mark = %d, next = %s\n",
		p -> sides >> 2, p -> sides & 3, p -> mark, (p -> next) ? "no-null" : "null");
	}
#endif /* ED_DEBUG */

	add_quad (quads, trap_list, (int) gnor_ini.layer_no);

	if (++j > gnor_ini.ny) break;
	if (!dy) dy = (Coor) Round (gnor_ini.dy * QUAD_LAMBDA);
	for (n = 0; n < nr_c; ++n) line_y[n] += dy;
    }
    if (++i > gnor_ini.nx) break;
    if (i == 1) dx = (Coor) Round (gnor_ini.dx * QUAD_LAMBDA);
    for (n = 0; n < nr_c; ++n) line_x[n] += dx;
    if (dy) {
	dy *= gnor_ini.ny;
	for (n = 0; n < nr_c; ++n) line_y[n] -= dy;
	dy = 0;
    }
}
    }
    dmCloseStream (finp, COMPLETE);
    return (0);
}

int inp_mcfl (DM_CELL *key, INST **inst_pp)
{
    DM_STREAM *fpmc;
    INST *ip;
    long  nd;

    if (!(fpmc = dmOpenStream (key, "mc", "r"))) return (-1);

    while (dmGetDesignData (fpmc, GEO_MC) > 0) {
	if (!(ip = create_inst (inst_pp,
		gmc.cell_name, gmc.inst_name, (int) gmc.imported,
		(Trans) gmc.mtx[0], (Trans) gmc.mtx[1],
		(Trans) gmc.mtx[3], (Trans) gmc.mtx[4],
		(Trans) (gmc.mtx[2] * QUAD_LAMBDA),
		(Trans) (gmc.mtx[5] * QUAD_LAMBDA),
		(Coor) (gmc.dx * QUAD_LAMBDA), (int) gmc.nx,
		(Coor) (gmc.dy * QUAD_LAMBDA), (int) gmc.ny))) {
	    dmCloseStream (fpmc, QUIT);
	    return (-1);
	}

	if ((nd = gmc.dx * gmc.nx) >= 0L) {
	    ip -> bbxl = (Coor) (QUAD_LAMBDA * gmc.bxl);
	    ip -> bbxr = (Coor) (QUAD_LAMBDA * (gmc.bxr - nd));
	}
	else {
	    ip -> bbxl = (Coor) (QUAD_LAMBDA * (gmc.bxl - nd));
	    ip -> bbxr = (Coor) (QUAD_LAMBDA * gmc.bxr);
	}
	if ((nd = gmc.dy * gmc.ny) >= 0L) {
	    ip -> bbyb = (Coor) (QUAD_LAMBDA * gmc.byb);
	    ip -> bbyt = (Coor) (QUAD_LAMBDA * (gmc.byt - nd));
	}
	else {
	    ip -> bbyb = (Coor) (QUAD_LAMBDA * (gmc.byb - nd));
	    ip -> bbyt = (Coor) (QUAD_LAMBDA * gmc.byt);
	}
    }
    dmCloseStream (fpmc, COMPLETE);
    return (0);
}

int inp_term (DM_CELL *key, TERM **term_pp)
{
    DM_STREAM *ftt;
    char  err_str[MAXCHAR];
    register int lay;

    if (!(ftt = dmOpenStream (key, "term", "r"))) return (-1);

    while (dmGetDesignData (ftt, GEO_TERM) > 0) {
	if ((lay = gterm.layer_no) < 0 || lay >= NR_lay) {
	    sprintf (err_str, "%s: Bad term layer number (%d)!",
		key -> cell, lay);
	    ptext (err_str);
	    sleep (1);
	    continue;
	}

	if (gterm.xr < gterm.xl) { Coor x = gterm.xl; gterm.xl = gterm.xr; gterm.xr = x; }
	if (gterm.yt < gterm.yb) { Coor y = gterm.yb; gterm.yb = gterm.yt; gterm.yt = y; }

	if (!create_term (&term_pp[lay],
	    (Coor) (gterm.xl * QUAD_LAMBDA), (Coor) (gterm.xr * QUAD_LAMBDA),
	    (Coor) (gterm.yb * QUAD_LAMBDA), (Coor) (gterm.yt * QUAD_LAMBDA),
	    gterm.term_name, strlen (gterm.term_name),
	    (Coor) (gterm.dx * QUAD_LAMBDA), (int) gterm.nx,
	    (Coor) (gterm.dy * QUAD_LAMBDA), (int) gterm.ny)) break;
	if (term_pp == term_root) {
	    TERMREF *t;
	    MALLOC (t, TERMREF);
	    t -> tp = term_pp[lay];
	    t -> lay = lay;
	    t -> next = NULL;
	    if (!termlist) termlist = t;
	    else termlistlast -> next = t;
	    termlistlast = t;
	}
    }
    dmCloseStream (ftt, COMPLETE);
    return (0);
}

int read_bound_box (DM_PROJECT *proj_key, char *cell_name, Coor *bxl, Coor *bxr, Coor *byb, Coor *byt)
{
    DM_CELL *cell_key;
    DM_STREAM *dmfp;
    char err_str[MAXCHAR];

    if ((cell_key = dmCheckOut (proj_key, cell_name, ACTUAL, DONTCARE, LAYOUT, READONLY))) {
	if ((dmfp = dmOpenStream (cell_key, "info", "r"))) {
	    if (dmGetDesignData (dmfp, GEO_INFO) == 4) {
		*bxl = (Coor) (ginfo.bxl * QUAD_LAMBDA);
		*bxr = (Coor) (ginfo.bxr * QUAD_LAMBDA);
		*byb = (Coor) (ginfo.byb * QUAD_LAMBDA);
		*byt = (Coor) (ginfo.byt * QUAD_LAMBDA);
		dmCloseStream (dmfp, COMPLETE);
		dmCheckIn (cell_key, COMPLETE);
		return (0);
	    }
	    dmCloseStream (dmfp, QUIT);
	}
	dmCheckIn (cell_key, QUIT);
    }
    sprintf (err_str, "Can't read bbox of cell '%s'!", cell_name);
    ptext (err_str);
    sleep (1);
    return (-1);
}
