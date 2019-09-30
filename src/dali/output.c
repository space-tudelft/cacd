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

extern DM_PROJECT *dmproject;
extern DM_CELL *ckey;
extern char *cellstr;
extern qtree_t **quad_root;
extern INST *inst_root;
extern TERMREF *termlist;
extern int  checked;
extern int  erase_text;
extern int  rmode;
extern int  use_new_mode;
extern int  NR_lay;
extern Coor Q_xl, Q_xr, Q_yb, Q_yt; /* quad search window */
extern Coor xlbb, xrbb, ybbb, ytbb; /* b_box boxen+terminals */
extern Coor xlmb, xrmb, ybmb, ytmb; /* bound box instances */
extern Coor xltb, xrtb, ybtb, yttb; /* bound box totaal */
extern char *yes_no[];

/* Write a cell to the database.
** There are three possibilities:
** 1. It was a totally new cell which isn't known by
**    the database yet. The key is NULL.
** 2. The cell was read from the database and is
**    written back now. We have a key.
** 3. The cell was read from the database but we want
**    to store it as a different cell (new  name).
**    We have a key but it belongs to its present name.
**    We need a new key to store it as a new cell.
**
** 1. Ask a new key using dmCreateCellkey().
**    We can write it using that key and from that moment
**    on it will be like we had read it previously
**    (we have written it using its key).
** 2. We can just write it, using its key.
**    (as often as we like).
** 3. We need an extra key to store it as a
**    different (new) cell. We ask a key using
**    dmCreateCellkey(). Then we can write it using
**    this key, but our workspace will still be related
**    to the old cell. So after this write we have
**    to give our new key back (using check_in).
*/
void wrte_cell ()
{
    static char *ask_str[3];	/* static for redraw */
    char    hulpstr[DM_MAXNAME + 1];
    char    chk_path[MAXCHAR], new_path[MAXCHAR];
    char    qstr[100];
    char   *newcell;
    int     choice, exist, len;
    DM_CELL *save_key;

    if (rmode) {
	ptext ("No write, you are in read only mode!");
	return;
    }
    save_key = NULL;
    upd_boundb ();
    if (xltb == xrtb || ybtb == yttb) {
	ptext ("You cannot write an empty cell!");
	return;
    }
    ask_str[0] = "-cancel-";
    ask_str[1] = "new name";
    ask_str[2] = cellstr;

    ptext ("Do you want to write this cell under a new name?");
    erase_text = 1;
    if ((choice = ask (ckey ? 3 : 2, ask_str, -1)) <= 0) return;

    if (choice == 1) { /* write cell under new name */
	if (!(len = ask_name ("cellname: ", hulpstr, TRUE))) return;
	if (!(newcell = strsave (hulpstr, len))) return;
	if ((exist = _dmExistCell (dmproject, newcell, LAYOUT))) {
	    if (exist == 1) {	/* cell already exists */
		sprintf (qstr, "Cell '%s' already exists, overwrite?", newcell);
		ptext (qstr);
		exist = ask (2, yes_no, -1);
	    }
	    if (exist != 1) {	/* Don't overwrite! */
		sprintf (qstr, "Cell '%s' not written!", newcell);
		ptext (qstr);
		FREE (newcell);
		return;
	    }
	}
    }
    else { /* write cell under old name */
	newcell = cellstr;
    }

    /*
    ** CheckOut new cell OR update old cell!
    */
    if (ckey && strcmp (newcell, cellstr) == 0) {
	dmCheckIn (ckey, QUIT);
	ckey = NULL;
	if (newcell != cellstr) FREE (cellstr);
	cellstr = NULL;
    }
    save_key = ckey;
    if (!(ckey = dmCheckOut (dmproject, newcell, WORKING, DONTCARE, LAYOUT, CREATE))) {
	ckey = save_key; /* undo */
	FREE (newcell);
	return;
    }

    if (!(outp_boxfl () && outp_mcfl () && outp_term () && outp_bbox () && outp_comment ())) {
    /*
     ** Files were not written properly so if a new key
     ** was obtained to write under a new name, it must
     ** be checked in using the quit mode.
     */
	if (checked) {
	    sprintf (chk_path, "%s/chk_mod.ck", dmproject -> dmpath);
	    unlink (chk_path);
	    checked = 0; /* NO */
	}
	if (choice == 1) { /* writing under new name */
	    dmCheckIn (ckey, QUIT);
	    FREE (newcell);
	    ckey = save_key;
	}
	return;
    }

    if (choice == 1) { /* write cell under new name */
	if (save_key && use_new_mode) {
	    /*
	    ** Quit existing cell under old name.
	    ** Continue with the cell under the new name!
	    */
	    FREE (cellstr);
	    cellstr = NULL;
	    dmCheckIn (save_key, QUIT);
	    save_key = NULL;
	}
	if (save_key) {
	/*
	 ** Write existing cell under new name
	 ** and complete (go back to old key and name).
	 */
	    if (dmCheckIn (ckey, COMPLETE) == -1) {
		btext ("Not accepted! (recursive)");
		dmCheckIn (ckey, QUIT);
		ckey = save_key;
		FREE (newcell);
		return;
	    }
	    ckey = save_key;
	}
	else {
	/*
	 ** Write a totally new cell OR old cell under new name
	 ** and hold new key!
	 */
	    if (dmCheckIn (ckey, COMPLETE) == -1) {
		btext ("Not accepted! (recursive)");
		ckey = NULL;
		FREE (newcell);
		return;
	    }
	    if (!(ckey = dmCheckOut (dmproject, newcell, WORKING, DONTCARE, LAYOUT, READONLY))) {
		btext ("Not accepted! (recursive)");
		FREE (newcell);
		return;
	    }
	    cellstr = newcell;
	}
    }
    else { /* Write existing cell, hold key! */
	if (dmCheckIn (ckey, CONTINUE) == -1) {
	    btext ("Not checked in!  This should never happen!");
	    sleep (2);
	}
	cellstr = newcell;
    }

    if (checked) {
	sprintf (chk_path, "%s/chk_mod.ck", dmproject -> dmpath);
	sprintf (new_path, "%s/%s.ck", dmproject -> dmpath, newcell);
	link (chk_path, new_path);
	unlink (chk_path);
	checked = 0; /* NO */
    }

    sprintf (qstr, "Cell '%s' written", newcell);
    ptext (qstr);
    if (newcell != cellstr) FREE (newcell);
}

static DM_STREAM *dm_boxfp;
static DM_STREAM *dm_norfp;

int outp_boxfl ()
{
    register int lay;

    if (!(dm_boxfp = dmOpenStream (ckey, "box", "w"))) return (FALSE);
    if (!(dm_norfp = dmOpenStream (ckey, "nor", "w"))) return (FALSE);

    gbox.dx = 0L;
    gbox.dy = 0L;
    gbox.nx = 0L;
    gbox.ny = 0L;

 /*
  ** We store all trapezoids as polygons
  ** with either 3 or 4 corner points.
  */
    gnor_ini.elmt = POLY_NOR;
    gnor_ini.dx = 0L;
    gnor_ini.dy = 0L;
    gnor_ini.nx = 0L;
    gnor_ini.ny = 0L;

    for (lay = 0; lay < NR_lay; ++lay) {
	gbox.layer_no = lay;
	gnor_ini.layer_no = lay;
	Q_xl = quad_root[lay] -> quadrant[0] - 1;
	Q_yb = quad_root[lay] -> quadrant[1] - 1;
	Q_xr = quad_root[lay] -> quadrant[2] + 1;
	Q_yt = quad_root[lay] -> quadrant[3] + 1;
	quad_search (quad_root[lay], store);
    }

    dmCloseStream (dm_boxfp, COMPLETE);
    dmCloseStream (dm_norfp, COMPLETE);
    return (TRUE);
}

int outp_mcfl ()
{
    DM_STREAM *dmfp;
    register INST *ip;
    Coor bxl, bxr, byb, byt;

    if (!(dmfp = dmOpenStream (ckey, "mc", "w"))) return (FALSE);

    for (ip = inst_root; ip; ip = ip -> next) {
	inst_window (ip, &bxl, &bxr, &byb, &byt);
	gmc.bxl = (long) bxl / QUAD_LAMBDA;
	gmc.bxr = (long) bxr / QUAD_LAMBDA;
	gmc.byb = (long) byb / QUAD_LAMBDA;
	gmc.byt = (long) byt / QUAD_LAMBDA;

	strcpy (gmc.inst_name, ip -> inst_name);
	strcpy (gmc.cell_name, ip -> templ -> cell_name);
	gmc.imported = ip -> imported;
	gmc.mtx[0] = (long) ip -> tr[0];
	gmc.mtx[1] = (long) ip -> tr[1];
	gmc.mtx[2] = (long) ip -> tr[2] / QUAD_LAMBDA;
	gmc.mtx[3] = (long) ip -> tr[3];
	gmc.mtx[4] = (long) ip -> tr[4];
	gmc.mtx[5] = (long) ip -> tr[5] / QUAD_LAMBDA;
	gmc.nx = (long) ip -> nx;
	gmc.dx = (long) ip -> dx / QUAD_LAMBDA;
	gmc.ny = (long) ip -> ny;
	gmc.dy = (long) ip -> dy / QUAD_LAMBDA;
	dmPutDesignData (dmfp, GEO_MC);
    }
    dmCloseStream (dmfp, COMPLETE);
    return (TRUE);
}

int outp_term ()
{
    DM_STREAM *dmfp;
    register TERM *tp;
    register TERMREF *t;
    Coor bxl, bxr, byb, byt;

    if (!(dmfp = dmOpenStream (ckey, "term", "w"))) return (FALSE);

    for (t = termlist; t; t = t -> next) {
	if (!(tp = t -> tp)) continue;
	gterm.layer_no = t -> lay;
	gterm.xl = (long) tp -> xl / QUAD_LAMBDA;
	gterm.xr = (long) tp -> xr / QUAD_LAMBDA;
	gterm.yb = (long) tp -> yb / QUAD_LAMBDA;
	gterm.yt = (long) tp -> yt / QUAD_LAMBDA;
	strcpy (gterm.term_name, tp -> tmname);
	gterm.nx = (long) tp -> nx;
	gterm.dx = (long) tp -> dx / QUAD_LAMBDA;
	gterm.ny = (long) tp -> ny;
	gterm.dy = (long) tp -> dy / QUAD_LAMBDA;
	term_win (tp, &bxl, &bxr, &byb, &byt, 0);
	gterm.bxl = (long) bxl / QUAD_LAMBDA;
	gterm.bxr = (long) bxr / QUAD_LAMBDA;
	gterm.byb = (long) byb / QUAD_LAMBDA;
	gterm.byt = (long) byt / QUAD_LAMBDA;
	dmPutDesignData (dmfp, GEO_TERM);
    }
    dmCloseStream (dmfp, COMPLETE);
    return (TRUE);
}

int outp_bbox ()
{
    DM_STREAM *dmfp;

    if (!(dmfp = dmOpenStream (ckey, "info", "w"))) return (FALSE);

    ginfo.bxl = (long) xltb / QUAD_LAMBDA;
    ginfo.bxr = (long) xrtb / QUAD_LAMBDA;
    ginfo.byb = (long) ybtb / QUAD_LAMBDA;
    ginfo.byt = (long) yttb / QUAD_LAMBDA;
    dmPutDesignData (dmfp, GEO_INFO);

    ginfo.bxl = (long) xlmb / QUAD_LAMBDA;
    ginfo.bxr = (long) xrmb / QUAD_LAMBDA;
    ginfo.byb = (long) ybmb / QUAD_LAMBDA;
    ginfo.byt = (long) ytmb / QUAD_LAMBDA;
    dmPutDesignData (dmfp, GEO_INFO);

    ginfo.bxl = (long) xlbb / QUAD_LAMBDA;
    ginfo.bxr = (long) xrbb / QUAD_LAMBDA;
    ginfo.byb = (long) ybbb / QUAD_LAMBDA;
    ginfo.byt = (long) ytbb / QUAD_LAMBDA;
    dmPutDesignData (dmfp, GEO_INFO);

    dmCloseStream (dmfp, COMPLETE);
    return (TRUE);
}

/*
** Store trapezoid p in the data base.
** INPUT: pointer to trapezoid p.
*/
void store (struct obj_node *p)
{
    Coor line[8];
    register int i;

    if (!p -> sides) {
    /*
     ** this is a rectangle
     */
	if (!(p -> ll_x1 % QUAD_LAMBDA || p -> ll_y1 % QUAD_LAMBDA ||
	      p -> ll_x2 % QUAD_LAMBDA || p -> ll_y2 % QUAD_LAMBDA)) {

	    gbox.xl = (long) p -> ll_x1 / QUAD_LAMBDA;
	    gbox.xr = (long) p -> ll_x2 / QUAD_LAMBDA;
	    gbox.yb = (long) p -> ll_y1 / QUAD_LAMBDA;
	    gbox.yt = (long) p -> ll_y2 / QUAD_LAMBDA;
	    gbox.bxl = gbox.xl;
	    gbox.bxr = gbox.xr;
	    gbox.byb = gbox.yb;
	    gbox.byt = gbox.yt;

	    dmPutDesignData (dm_boxfp, GEO_BOX);
	    return;
	}
    }

    line[0] = p -> ll_x1;
    line[1] = p -> ll_y1;
    line[2] = p -> ll_x2;
    line[3] = p -> ll_y2;

    while (line[0] % QUAD_LAMBDA) --line[0];
    while (line[1] % QUAD_LAMBDA) --line[1];
    while (line[2] % QUAD_LAMBDA) ++line[2];
    while (line[3] % QUAD_LAMBDA) ++line[3];

    gnor_ini.bxl = (long) line[0] / QUAD_LAMBDA;
    gnor_ini.byb = (long) line[1] / QUAD_LAMBDA;
    gnor_ini.bxr = (long) line[2] / QUAD_LAMBDA;
    gnor_ini.byt = (long) line[3] / QUAD_LAMBDA;
    gnor_ini.r_bxl = gnor_ini.bxl;
    gnor_ini.r_byb = gnor_ini.byb;
    gnor_ini.r_bxr = gnor_ini.bxr;
    gnor_ini.r_byt = gnor_ini.byt;

 /*
  ** Convert trapezoid to polygon.
  */
    if (!trap_to_poly (line, p)) {
	ptext ("Illegal trapezoid!");
	return;
    }

    if (line[0] == line[2] || line[4] == line[6]) {
    /*
     ** trapezoid is a triangle
     */
	gnor_ini.no_xy = 3;

	if (line[0] == line[2]) {
	/*
	 ** First two points are identical.
	 ** Eliminate second point and shift others back.
	 */
	    line[2] = line[4];
	    line[3] = line[5];
	    line[4] = line[6];
	    line[5] = line[7];
	}
    }
    else {
	gnor_ini.no_xy = 4;
    }
    dmPutDesignData (dm_norfp, GEO_NOR_INI);

 /*
  ** Store xy-pairs of polygon.
  */
    for (i = 0; i < (2 * (int) gnor_ini.no_xy); i = i + 2) {
	gnor_xy.x = (double) line[i] / (double) QUAD_LAMBDA;
	gnor_xy.y = (double) line[i + 1] / (double) QUAD_LAMBDA;
	dmPutDesignData (dm_norfp, GEO_NOR_XY);
    }
}
