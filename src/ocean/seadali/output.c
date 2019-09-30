/*
 * ISC License
 *
 * Copyright (C) 1984-2018 by
 *	Pieter van der Wolf
 *	Simon de Graaf
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

#include "src/ocean/seadali/header.h"

static void save_check (char *cell_name);

extern DM_PROJECT *dmproject;
extern DM_CELL *ckey;
extern char *cellstr;
extern char *tmpchkcell;
extern qtree_t **quad_root;
extern INST *inst_root;
extern TERM **term_root;
extern int  rmode;
extern int  NR_lay;
extern int  checked;
extern int  dirty;

/* patrick */
extern Coor xlim, xrim, ybim, ytim; /* bounding box of instances and boxes without the image */
extern int ImageMode;
/* end Patrick */
extern Coor xlbb, xrbb, ybbb, ytbb; /* b_box boxen+terminals */
extern Coor xlmb, xrmb, ybmb, ytmb; /* bound box instances */
extern Coor xltb, xrtb, ybtb, yttb; /* bound box totaal */
extern char *yes_no[];
extern char cirname[];

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
    char    rem_path[MAXCHAR];
    char    qstr[100];
    char   *newcell;
    int     choice;
    int     exist;
    DM_CELL * save_key;
    int     isnewname;

    if (rmode) {
	ptext ("No write, you are in read only mode!");
	return;
    }
    save_key = NULL;
    upd_boundb ();
    if (xltb == xrtb || ybtb == yttb) {
	ptext ("Don't you ever try to write an empty cell again!");
	return;
    }
    ask_str[0] = "-cancel-";

    if (!ckey && cirname[0] != '\0') {
        ask_str[1] = "other name";
        ask_str[2] = &cirname[0];
        ptext ("Under which name do you wanna write this cell?");
    }
    else {
        ask_str[1] = "new name";
        ask_str[2] = cellstr;
        ptext ("Do you wanna write this cell under a new name?");
    }
    if ((choice = ask ((ckey || cirname[0] != '\0') ? 3 : 2, ask_str, -1)) <= 0) {
	ptext ("");
	return;
    }

    isnewname = 0;
    if (choice == 1 || (choice == 2 && !ckey)) {
        if (choice == 1) {
	    if (ask_name ("cellname: ", hulpstr, TRUE) == -1) return;
        }
        else {
            strcpy (hulpstr, ask_str[2]);
        }
	if (strcmp (hulpstr, "res") == 0 || strcmp (hulpstr, "cap") == 0 ||
	    strcmp (hulpstr, "nenh") == 0 || strcmp (hulpstr, "penh") == 0) {
	    sprintf (qstr, "Name '%s' is a nelsis primitive: really use this name?", hulpstr);
	    ptext (qstr);
	    if (ask (2, yes_no, -1) != 0) {
		sprintf (qstr, "Cell '%s' was NOT written", hulpstr);
		ptext (qstr);
		return;
	    }
	}

	if (!(newcell = strsave (hulpstr))) return;

	if ((exist = _dmExistCell (dmproject, newcell, LAYOUT))) {
	    if (exist == 1) {	/* cell already exists */
		sprintf (qstr, "Cell '%s' already exists: wanna overwrite?", newcell);
		ptext (qstr);
		if (ask (2, yes_no, -1) != 0) {
		    sprintf (qstr, "Cell '%s' was not written", newcell);
		    ptext (qstr);
		    return;
		}
	    }
	    else {
		sprintf (qstr, "Cell '%s' was not written", newcell);
		ptext (qstr);
		return;
	    }
	}

	if (!ckey || strcmp (newcell, cellstr)) {
	    /* cell does not yet exist */
	    isnewname = 1;
	    save_key = ckey;
	    if (!(ckey = dmCheckOut (dmproject, newcell, WORKING, DONTCARE, LAYOUT, UPDATE))) {
		ckey = save_key; /* undo */
		return;
	    }
	}
    }
    else {
	newcell = cellstr;
    }

    if (!(outp_boxfl (ckey) && outp_mcfl (ckey) && outp_term (ckey) && outp_bbox (ckey)
	&& outp_comment (ckey)
	)) {
    /*
     ** Files were not written properly so if a new key
     ** was obtained to write under a new name, it must
     ** be checked in using the quit mode.
     */
	if (checked == TRUE) {
	    sprintf (rem_path, "%s.ck", tmpchkcell);
	    unlink (rem_path);
	    checked = FALSE;
	}
	if (choice == 1) {
	    dmCheckIn (ckey, QUIT);
	    ckey = save_key;
	}
	return;
    }

    if (isnewname) {
	if (save_key) {
	/*
	 ** Write existing cell under new name:
	 ** case 3 (see comments above).
	 */
/* PATRICK: now the cell which is being edited is using the last saved name */

	    if (dmCheckIn (ckey, CONTINUE) == -1) {
		ptext ("Not accepted! (recursive)");
		dmCheckIn (ckey, QUIT);
		ckey = NULL;
		return;
	    }
	    if (dmCheckIn (save_key, QUIT) == -1) {
		ptext ("Not accepted! (recursive)");
		save_key = NULL;
		return;
	    }
	    cellstr = newcell;
	}
	else {
	/*
	 ** Write a totally new cell, hold key:
	 ** case 1 (see comments above).
	 */
	    if (dmCheckIn (ckey, CONTINUE) == -1) {
		ptext ("Not accepted! (recursive)");
		dmCheckIn (ckey, QUIT);
		ckey = NULL;
		return;
	    }
	    ASSERT (!cellstr);
	    cellstr = newcell;
	}
	save_check (newcell);
    }
    else {
    /*
     ** Write existing cell, hold key.
     */
	if (dmCheckIn (ckey, CONTINUE) == -1) ptext ("Not checked in! This should never happen!");
	save_check (cellstr);
    }

    sprintf (qstr, "Cell '%s' written", newcell);
    ptext (qstr);
    dirty = FALSE;
}

static void save_check (char *cell_name)
{
    char old_path[MAXCHAR];
    char new_path[MAXCHAR];

    if (checked == TRUE) {
	sprintf (old_path, "%s.ck", tmpchkcell);
	sprintf (new_path, "%s.ck", cell_name);
	rename (old_path, new_path);
	checked = FALSE;
    }
}

static  DM_STREAM * dm_boxfp;
static  DM_STREAM * dm_norfp;

/* PATRICK: added key as argument instead of global to write routines */
int outp_boxfl (DM_CELL *key)
{
    struct obj_node *window;
    register int lay;

    if (!(dm_boxfp = dmOpenStream (key, "box", "w"))) return (FALSE);
    if (!(dm_norfp = dmOpenStream (key, "nor", "w"))) return (FALSE);
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

    MALLOC (window, struct obj_node);

    for (lay = 0; lay < NR_lay; ++lay) {
	gbox.layer_no = lay;
	gnor_ini.layer_no = lay;
	window -> ll_x1 = quad_root[lay] -> quadrant[0] - 1;
	window -> ll_y1 = quad_root[lay] -> quadrant[1] - 1;
	window -> ll_x2 = quad_root[lay] -> quadrant[2] + 1;
	window -> ll_y2 = quad_root[lay] -> quadrant[3] + 1;
	quad_search (quad_root[lay], window, store);
    }
    FREE (window);

    dmCloseStream (dm_boxfp, COMPLETE);
    dmCloseStream (dm_norfp, COMPLETE);
    return (TRUE);
}

int outp_mcfl (DM_CELL *key)
{
    DM_STREAM *dmfp;
    register INST * ip;
    Coor bxl, bxr, byb, byt;

    if (!(dmfp = dmOpenStream (key, "mc", "w"))) return (FALSE);

    for (ip = inst_root; ip; ip = ip -> next) {
	inst_window (ip, &bxl, &bxr, &byb, &byt);
	gmc.bxl = (long) bxl / QUAD_LAMBDA;
	gmc.bxr = (long) bxr / QUAD_LAMBDA;
	gmc.byb = (long) byb / QUAD_LAMBDA;
	gmc.byt = (long) byt / QUAD_LAMBDA;
	if (strlen (ip -> inst_name) == 0)
	    strcpy (gmc.inst_name, ".");
	else
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

int outp_term (DM_CELL *key)
{
    DM_STREAM *dmfp;
    register int i;
    register TERM *tp;
    Coor bxl, bxr, byb, byt;

    if (!(dmfp = dmOpenStream (key, "term", "w"))) return (FALSE);

    for (i = 0; i < NR_lay; ++i) {
	gterm.layer_no = i;

	for (tp = term_root[i]; tp; tp = tp -> nxttm) {
	    gterm.xl = (long) tp -> xl / QUAD_LAMBDA;
	    gterm.xr = (long) tp -> xr / QUAD_LAMBDA;
	    gterm.yb = (long) tp -> yb / QUAD_LAMBDA;
	    gterm.yt = (long) tp -> yt / QUAD_LAMBDA;
	    strcpy (gterm.term_name, tp -> tmname);
	    gterm.nx = (long) tp -> nx;
	    gterm.dx = (long) tp -> dx / QUAD_LAMBDA;
	    gterm.ny = (long) tp -> ny;
	    gterm.dy = (long) tp -> dy / QUAD_LAMBDA;
	    term_win (tp, &bxl, &bxr, &byb, &byt);
	    gterm.bxl = (long) bxl / QUAD_LAMBDA;
	    gterm.bxr = (long) bxr / QUAD_LAMBDA;
	    gterm.byb = (long) byb / QUAD_LAMBDA;
	    gterm.byt = (long) byt / QUAD_LAMBDA;
	    dmPutDesignData (dmfp, GEO_TERM);
	}
    }
    dmCloseStream (dmfp, COMPLETE);
    return (TRUE);
}

int outp_bbox (DM_CELL *key)
{
    DM_STREAM *dmfp;

    if (!(dmfp = dmOpenStream (key, "info", "w"))) return (FALSE);

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

    /*
     * patrick: put one more info: the total bounding box WITHOUT the image
     */
    if (ImageMode == TRUE && xlim != xrim && ybim != ytim) {
	ginfo.bxl = (long) xlim / QUAD_LAMBDA;
	ginfo.bxr = (long) xrim / QUAD_LAMBDA;
	ginfo.byb = (long) ybim / QUAD_LAMBDA;
	ginfo.byt = (long) ytim / QUAD_LAMBDA;
	dmPutDesignData (dmfp, GEO_INFO);
    }

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
    int  i;

    if (!p -> leftside && !p -> rightside) {
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
    if (trap_to_poly (line, p) == -1) {
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
