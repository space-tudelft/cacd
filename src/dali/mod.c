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
extern TEMPL *first_templ;
extern INST *inst_root;
extern float XL, YB, c_cH;
extern char *cellstr;
extern char *ImageName; /* Sea-of-Gates */
extern int  allow_keypress;
extern int  ask_iname;
extern int  cmd_nbr, new_cmd;
extern int  erase_text;
extern int  exp_level;
extern int *term_lay;
extern int *vis_arr;
extern int  v_sterm, v_inst;
extern int *pict_arr;
extern int  NR_lay;
extern int  Textnr;
extern Coor piwl, piwr, piwb, piwt;
extern Coor xlc, ybc;

static void do_add_inst (char *c_name, int import);
static TEMPL *find_templ (char *name);
static int  in_inst (Coor x, Coor y, INST *ip);
static void remove_templ (TEMPL *rem_templ);

void add_inst ()
{
    char *c_name;
    static char *ask_c[] = {
	 /* 0 */ "imported",
	 /* 1 */ "local",
    };
    int import;
    int a_old;

    ptext ("Add local or imported cells?");
    erase_text = 1;
    a_old = allow_keypress;
    allow_keypress = 0;
    if (ask (2, ask_c, -1)) {
	allow_keypress = a_old;
	import = LOCAL;
	if (read_modlist () == -1) return;
    }
    else {
	allow_keypress = a_old;
	import = IMPORTED;
	if (read_impclist () == -1) return;
    }
    while ((c_name = ask_cell (import))) {
	do_add_inst (c_name, import);
    }
}

static void do_add_inst (char *c_name, int import)
{
    INST *ip;
    DM_PROJECT *pkey; /* remote project key */
    char *r_name;     /* remote cell_name */
    char  hstr[MAXCHAR];
    char  mess_str[MAXCHAR];
    Coor  ll, rr, bb, tt;
    register int lay;

    if (cellstr && strcmp (cellstr, c_name) == 0) {
	btext ("Sorry, this is the current cell!");
	sleep (1);
	return;
    }

    if (!(pkey = dmFindProjKey (import, c_name, dmproject, &r_name, LAYOUT))) return;
    *hstr = 0;
    if (import == IMPORTED) sprintf (hstr, "(%s from %s) ", r_name, pkey -> projectid);

    if (read_bound_box (pkey, r_name, &ll, &rr, &bb, &tt) < 0) return;

    set_bbox_loc (ll, rr, bb, tt);

    sprintf (mess_str, "Add instances of cell '%s', enter position!", c_name);
    ptext (mess_str);

    while ((new_cmd = get_cursor (6, 1, SNAP)) == -1) {

	if (!(ip = create_inst (&inst_root, c_name, ".", import,
		(Trans) 1, (Trans) 0, (Trans) 0, (Trans) 1,
		(Trans) (xlc - ll), (Trans) (ybc - bb),
		(Coor) 0, 0, (Coor) 0, 0))) return;

	ip -> bbxl = piwl = xlc;
	ip -> bbxr = piwr = xlc + (rr - ll);
	ip -> bbyb = piwb = ybc;
	ip -> bbyt = piwt = ybc + (tt - bb);
	ip -> level = exp_level;

	pict_arr[Textnr] = DRAW; /* draw instances */
	if (exp_level > 1 && vis_arr[v_inst]) { /* inst. visible */
	    for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = DRAW;
	}
	else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	    for (lay = 0; lay < NR_lay; ++lay)
		if (term_lay[lay]) pict_arr[lay] = DRAW;
	}
	picture ();
	if (ask_iname) { /* ask instance name */
	    if (ask_inst (ip)) picture ();
	}
	sprintf (mess_str, "Added %s(%s) %sat %ld, %ld",
	    ip -> inst_name, ip -> templ -> cell_name, hstr,
	    (long) (xlc / QUAD_LAMBDA), (long) (ybc / QUAD_LAMBDA));
	ptext (mess_str);
    }
}

int ask_inst (INST *act_ip)
{
    char mess_str[MAXCHAR];
    char i_name[DM_MAXNAME + 10];
    register INST *ip;
ask:
    sprintf (mess_str, "Instance %s(%s), new name: ",
	act_ip -> inst_name, act_ip -> templ -> cell_name);

    if (ask_name (mess_str, i_name, 2) <= 0) return (0);

    if (strcmp (act_ip -> inst_name, i_name) == 0) return (0);

    if (*i_name != '.')
    for (ip = inst_root; ip; ip = ip -> next) {
	if (strcmp (ip -> inst_name, i_name) == 0) { /* found */
	    sprintf (mess_str, "Instance name '%s' already used for cell '%s'",
		i_name, ip -> templ -> cell_name);
	    btext (mess_str);
	    sleep (2);
	    goto ask;
	}
    }
    FREE (act_ip -> inst_name);
    act_ip -> inst_name = strsave (i_name, 0);
    set_c_wdw (PICT);
    pict_arr[Textnr] = ERAS_DR; /* redraw text */
    if ((piwl = act_ip -> bbxl) < XL) piwl = XL;
    if ((piwb = act_ip -> bbyb) < YB) piwb = YB;
    piwr = piwl + c_cH;
    piwt = piwb + c_cH;
    return (1);
}

/*
** The given coordinates of create_inst are the
** bounding box coordinates of the cell whereof the
** corner not always (0,0) needs to be.
** The position of the instance is internal
** calculated by a translation vector from (0,0)
** of the current cell to (0,0) of the placed cell.
*/
INST * create_inst (INST **inst_pp, char *cellname, char *inst_name, int import,
	Trans tr0, Trans tr1, Trans tr3, Trans tr4, Trans tx, Trans ty, Coor dx, int nx, Coor dy, int ny)
{
    INST *ip;
    register int lay;
    TEMPL *new_templ;

    MALLOC (ip, INST);
    if (!ip) return (NULL);

    if (!(ip -> templ = find_templ (cellname))) {
	MALLOC (new_templ, TEMPL);
	if (!new_templ) return (NULL);
	new_templ -> cell_name = strsave (cellname, 0);

	MALLOCN (new_templ -> quad_trees, qtree_t *, NR_lay);
	MALLOCN (new_templ -> term_p, TERM *, NR_lay);
	for (lay = 0; lay < NR_lay; ++lay) {
	    new_templ -> quad_trees[lay] = NULL;
	    new_templ -> term_p[lay] = NULL;
	}
	new_templ -> t_flag = 0;
	new_templ -> inst = NULL;
	new_templ -> projkey = NULL;
	new_templ -> next = first_templ;
	first_templ = new_templ;
	ip -> templ = new_templ;
    }

    ip -> inst_name = strsave (inst_name, 0);
    ip -> imported = import;
    ip -> tr[0] = tr0;
    ip -> tr[1] = tr1;
    ip -> tr[3] = tr3;
    ip -> tr[4] = tr4;
    ip -> tr[2] = tx;
    ip -> tr[5] = ty;
    ip -> dx = dx;
    ip -> nx = nx;
    ip -> dy = dy;
    ip -> ny = ny;
    ip -> level = 1;	/* no expansion */
    ip -> next = *inst_pp;
    *inst_pp = ip;
    return (ip);
}

static TEMPL * find_templ (char *name) /* find template of cell name */
{
    register TEMPL *te_p;
    if ((te_p = first_templ))
    do {
	if (strcmp (name, te_p -> cell_name) == 0) {
	    return (te_p); /* found */
	}
    } while ((te_p = te_p -> next));
    return (te_p); /* NULL */
}

void del_inst ()
{
    register INST *ip, *pip;
    char  mess_str[MAXCHAR];
    register int lay;

    if (!present_inst ()) return;

    do {
	ptext ("Select an instance to delete!");
	if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) {
	    erase_text = 1;
	    return;
	}
	ip = search_inst (xlc, ybc);
    } while (!ip);

    new_cmd = cmd_nbr;

    if (ip != inst_root) { /* find previous instance pointer */
	for (pip = inst_root; pip -> next != ip; pip = pip -> next);
	pip -> next = ip -> next;
    }
    else inst_root = ip -> next;

    sprintf (mess_str, "Instance %s(%s) deleted!",
	ip -> inst_name, ip -> templ -> cell_name);
    ptext (mess_str);
    sleep (1);

    inst_window (ip, &piwl, &piwr, &piwb, &piwt);

    pict_arr[Textnr] = ERAS_DR; /* redraw instances */
    if (ip -> level > 1 && vis_arr[v_inst]) { /* inst. visible */
	for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = ERAS_DR;
    }
    else if (vis_arr[v_sterm]) { /* sub-terminals visible */
	for (lay = 0; lay < NR_lay; ++lay)
	    if (term_lay[lay]) pict_arr[lay] = ERAS_DR;
    }

    remove_templ (ip -> templ); /* try to remove template */
    FREE (ip -> inst_name);
    FREE (ip);
}

static void remove_templ (TEMPL *rem_templ) /* remove template? */
{
    register TEMPL *ep;
    register INST  *ip, *ip2;

    /*
    ** Search inst_root for other references to rem_templ:
    */
    for (ip = inst_root; ip; ip = ip -> next) {
	if (ip -> templ == rem_templ) return; /* found */
    }

    /*
    ** Unlink template rem_templ first.
    ** Thus next search don't find itself.
    */
    if (rem_templ != first_templ) {
	for (ep = first_templ; ep -> next != rem_templ; ep = ep -> next);
	ep -> next = rem_templ -> next;
    }
    else first_templ = rem_templ -> next;

    /*
    ** Search other templates for references to rem_templ:
    */
    for (ep = first_templ; ep; ep = ep -> next) {
	for (ip = ep -> inst; ip; ip = ip -> next) {
	    if (ip -> templ == rem_templ) { /* found */
		/*
		** Link template rem_templ again:
		*/
		rem_templ -> next = first_templ;
		first_templ = rem_templ;
		return;
	    }
	}
    }

    /*
    ** No references to rem_templ.
    ** Now check instance list for unique lower templates.
    */
    for (ip = rem_templ -> inst; ip; ip = ip -> next) {
	if ((ep = ip -> templ)) {
	    remove_templ (ep); /* try to remove template */
	    /*
	    ** Search for more references to template:
	    */
	    ip2 = ip;
	    while ((ip2 = ip2 -> next)) {
		if (ip2 -> templ == ep) ip2 -> templ = NULL;
	    }
	}
    }

    /*
    ** All instances have been checked for unique templates.
    ** Unique lower templates are also removed.
    */
    clear_templ (rem_templ);
}

int present_inst ()
{
    if (!inst_root) {
	ptext ("No instances present!");
	return (0);
    }
    return (1);
}

static void present_cell (Coor x, Coor y, INST *p, int level, Trans mtx[], DM_PROJECT *pkey)
{
    int   nx1, nx2, ny1, ny2, nx, ny;
    float f_n1, f_n2, s_x, s_y, z;
    Coor  ll, rr, bb, tt;
    Trans new_mtx[6], t_x, t_y;
    INST *ip;

    nx = level;
    while (--nx > 0) fprintf (stdout, "    ");
    fprintf (stdout, "%d - %s \"%s\"\n", level, p -> templ -> cell_name, p -> inst_name);

    if (!p -> templ -> quad_trees[0]) { /* template is empty: read it */
	if (exp_templ (p -> templ, pkey, p -> imported, READ_ALL))
	    return; /* read error */
    }

    nx1 = 0; nx2 = 0; ny1 = 0; ny2 = 0;

    if (p -> nx || p -> ny) { /* repetition */
	/* calculate search window at this instance level */
	if (mtx[0]) { /* r0 or r180 */
	    s_x = x - mtx[2]; s_y = y - mtx[5];
	    s_x /= mtx[0]; s_y /= mtx[4];
	}
	else {
	    s_y = x - mtx[2]; s_x = y - mtx[5];
	    s_x /= mtx[3]; s_y /= mtx[1];
	}

	if (p -> nx) {
	    f_n1 = (s_x - (float) p -> bbxr) / (float) p -> dx;
	    f_n2 = (s_x - (float) p -> bbxl) / (float) p -> dx;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((nx1 = UpperRound (f_n1)) < 0) nx1 = 0;
	    if ((nx2 = LowerRound (f_n2)) > p -> nx) nx2 = p -> nx;
	    if (nx1 > nx2) return;
	}
	if (p -> ny) {
	    f_n1 = (s_y - (float) p -> bbyt) / (float) p -> dy;
	    f_n2 = (s_y - (float) p -> bbyb) / (float) p -> dy;
	    if (f_n2 < f_n1) { z = f_n1; f_n1 = f_n2; f_n2 = z; }
	    if ((ny1 = UpperRound (f_n1)) < 0) ny1 = 0;
	    if ((ny2 = LowerRound (f_n2)) > p -> ny) ny2 = p -> ny;
	    if (ny1 > ny2) return;
	}
    }

    new_mtx[0] = mtx[0] * p -> tr[0] + mtx[1] * p -> tr[3];
    new_mtx[1] = mtx[0] * p -> tr[1] + mtx[1] * p -> tr[4];
    new_mtx[3] = mtx[3] * p -> tr[0] + mtx[4] * p -> tr[3];
    new_mtx[4] = mtx[3] * p -> tr[1] + mtx[4] * p -> tr[4];

    pkey = p -> templ -> projkey;

    t_x = p -> tr[2] + nx1 * p -> dx;
    for (nx = nx1;;) {
	t_y = p -> tr[5] + ny1 * p -> dy;
	for (ny = ny1;;) {
	    /* calculate search window at this level */
	    new_mtx[2] = mtx[2] + t_x * mtx[0] + t_y * mtx[1];
	    new_mtx[5] = mtx[5] + t_x * mtx[3] + t_y * mtx[4];

	    if (new_mtx[0]) { /* R0 or R180 */
		s_x = (float) (x - new_mtx[2]) / new_mtx[0];
		s_y = (float) (y - new_mtx[5]) / new_mtx[4];
	    }
	    else { /* R90 or R270 */
		s_y = (float) (x - new_mtx[2]) / new_mtx[1];
		s_x = (float) (y - new_mtx[5]) / new_mtx[3];
	    }

	    ll = LowerRound (s_x); bb = LowerRound (s_y);
	    rr = UpperRound (s_x); tt = UpperRound (s_y);

	    for (ip = p -> templ -> inst; ip; ip = ip -> next) {
		if (!inst_outside_window (ip, ll, rr, bb, tt))
		    present_cell (x, y, ip, level + 1, new_mtx, pkey);
	    }
	    if (++ny > ny2) break;
	    t_y += p -> dy;
	}
	if (++nx > nx2) break;
	t_x += p -> dx;
    }
}

INST * cell_tree (Coor x, Coor y)
{
    Trans matrix[6];
    Coor ll, rr, bb, tt;
    INST *ip, *first_ip = 0;

    matrix[0] = 1; matrix[1] = 0; matrix[2] = 0;
    matrix[3] = 0; matrix[4] = 1; matrix[5] = 0;

    fprintf (stdout, "\nTree of cells \"instances\" at position %ld,%ld is:\n",
	(long) x / QUAD_LAMBDA, (long) y / QUAD_LAMBDA);
    for (ip = inst_root; ip; ip = ip -> next) {
	if (in_inst (x, y, ip)) {
	    if (!first_ip) first_ip = ip;
	    present_cell (x, y, ip, 1, matrix, dmproject);
	}
    }
    fflush (stdout);
    return first_ip;
}

INST * search_inst (Coor x1, Coor y1)
{
    static char *st_istr[LIST_LENGTH]; /* static for redraw */
    INST *st_pntr[LIST_LENGTH];
    char  mess_str[MAXCHAR];
    register INST *ip;
    INST *image = NULL;
    int   nbr, nbr2, found = 0;
    int   a_old;

    for (ip = inst_root; ip; ip = ip -> next) {
	/* Sea-of-Gates:
	** Prevent silly question about the image!
	*/
	if (ImageName && !image && strcmp (ip -> inst_name, ImageName) == 0) {
	    image = ip;
	    continue;
	}

	if (in_inst (x1, y1, ip)) {
	    if (found < LIST_LENGTH) {
		st_pntr[found] = ip;
		st_istr[found] = ip -> templ -> cell_name;
	    }
	    ++found;
	}
    }

    if (found == 0)
	ip = image ? image : NULL;
    else if (found == 1)
	ip = st_pntr[0];
    else { /* select by menu */
	if (found > LIST_LENGTH) {
	    sprintf (mess_str, "List full, %d instances skipped!", found - LIST_LENGTH);
	    notify (mess_str);
	    found = LIST_LENGTH;
	}
	save_menu ();
	ptext ("Select one of the found instances!");
	a_old = allow_keypress;
	allow_keypress = 0;
	nbr2 = ask (found, st_istr, -1);
	do {
	    ip = st_pntr[nbr = nbr2];
	    sprintf (mess_str, "Instance %s(%s) selected, OK?", ip -> inst_name, ip -> templ -> cell_name);
	    ptext (mess_str);
	    nbr2 = ask (found, st_istr, nbr);
	} while (nbr2 != nbr);
	allow_keypress = a_old;
	prev_menu ();
    }
    return (ip);
}

static int in_inst (Coor x, Coor y, INST *ip)
{
    Coor ll, rr, bb, tt;
    inst_window (ip, &ll, &rr, &bb, &tt);
    if (x < ll || x > rr || y < bb || y > tt) return (0); /* NO */
    return (1); /* YES */
}

/*
** Does the instance intersect the window at its father level?
** If the instance lies outside the given window, TRUE is returned.
*/
int inst_outside_window (INST *ip, Coor xmin, Coor xmax, Coor ymin, Coor ymax)
{
    Coor ll, rr, bb, tt;
    inst_window (ip, &ll, &rr, &bb, &tt);
    if (ll > xmax || rr < xmin || bb > ymax || tt < ymin) return (1);
    return (0); /* not outside the window */
}

void inst_window (INST *ip, Coor *ll, Coor *rr, Coor *bb, Coor *tt)
{
    Coor nd;
    if ((nd = ip -> nx * ip -> dx) >= (Coor) 0) {
	*ll = ip -> bbxl;
	*rr = ip -> bbxr + nd;
    }
    else {
	*ll = ip -> bbxl + nd;
	*rr = ip -> bbxr;
    }
    if ((nd = ip -> ny * ip -> dy) >= (Coor) 0) {
	*bb = ip -> bbyb;
	*tt = ip -> bbyt + nd;
    }
    else {
	*bb = ip -> bbyb + nd;
	*tt = ip -> bbyt;
    }
}
