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

extern struct Disp_wdw *p_wdw;
extern DM_PROJECT *dmproject;
extern char *cellstr;
extern INST *inst_root;
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, ybc;  /* cursor parameters */
extern int  Default_expansion_level;
extern int  NR_lay;
extern int  Textnr;
extern int  allow_keypress;
extern int  erase_text;
extern int  exp_level; /* expansion level of overall expansions */
extern int  new_cmd;
extern int *pict_arr;

#define MAX_LEV 100

static int ask_level (int old)
{
    static char *ask_str[] = {	/* static for redraw */
	"keyboard",
	"maximum", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1"
    };
    static int  level[] = {
	MAX_LEV, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
    };
    char tmp_str[80];
    int nr, nr_items;
    int a_old;

    if (old > 9 && old < MAX_LEV) {
	if (level[1] != old) {
	    level[1] = old;
	    ask_str[2][0] = '0' + old / 10;
	    ask_str[2][1] = '0' + old % 10;
	}
	nr = 2;
    }
    else nr = (old == MAX_LEV) ? 1 : level[old + 1] + 2;
    nr_items = sizeof (ask_str) / sizeof (char *);
    a_old = allow_keypress;
    allow_keypress = 0;
    nr = ask (nr_items, ask_str, nr);
    allow_keypress = a_old;
    if (nr == 0) { /* keyboard */
	if (ask_string ("level: ", tmp_str)) sscanf (tmp_str, "%d", &nr);
	erase_text = 1;
	if (nr < 1) return (old);
	if (nr > MAX_LEV) return (MAX_LEV);
	return (nr);
    }
    return (level[nr - 1]);
}

int expansion (int new_level)
{
    char mess_str[MAXCHAR];
    Coor bxl, bxr, byb, byt;
    int  Firsttime, lay_draw_flag;
    register INST *ip;
    register int lay;

    if (new_level < 0) {
	sprintf (mess_str, "Cell '%s', select level for expansion!", cellstr ? cellstr : "????");
	ptext (mess_str);
	new_level = ask_level (exp_level);
    }

    if (new_level == 0 || new_level > MAX_LEV) new_level = MAX_LEV;

    if (new_level == exp_level) {
	ptext ("No expansion, new level equal to old level!");
	return (0);
    }

  /*
  ** Initially we assume no erases have to be done.
  ** It will depend on the levels of the individual
  ** instances whether an erase will have to occur.
  ** Also the piw? area will be determined here.
  */
    Firsttime = 1;
    lay_draw_flag = SKIP;

    for (ip = inst_root; ip; ip = ip -> next) {
	if (ip -> level == new_level) continue;
	inst_window (ip, &bxl, &bxr, &byb, &byt);
	if ((float) bxl < p_wdw -> wxmax && (float) bxr > p_wdw -> wxmin &&
	    (float) byb < p_wdw -> wymax && (float) byt > p_wdw -> wymin) {
	/*
	 ** Instance inside PICT-viewport and has to be (re)drawn.
	 */
	    pict_arr[Textnr] = (ip -> level != 1 ||
		    pict_arr[Textnr] == ERAS_DR) ? ERAS_DR : DRAW;

	/*
	 ** If expansion level has to be decreased, at least
	 ** for this instance, enforce erase/redraw mode.
	 */
	    lay_draw_flag = (new_level < ip -> level ||
		    lay_draw_flag == ERAS_DR) ? ERAS_DR : DRAW;
	/*
	 ** Include this one in (re)drawing area.
	 */
	    if (Firsttime) {
		Firsttime = 0;
		piwl = bxl;
		piwr = bxr;
		piwb = byb;
		piwt = byt;
	    }
	    else {
		if (bxl < piwl) piwl = bxl;
		if (byb < piwb) piwb = byb;
		if (bxr > piwr) piwr = bxr;
		if (byt > piwt) piwt = byt;
	    }
	}
	ip -> level = new_level;
    }

    for (lay = 0; lay < NR_lay; ++lay) pict_arr[lay] = lay_draw_flag;

    exp_level = new_level;
    return (1);
}

void def_level ()
{
    char  mess_str[MAXCHAR];
    static char *ask_str[] = {	/* static for redraw */
	"9", "8", "7", "6", "5", "4", "3", "2", "1", "no"
    };
    int nr;
    int a_old;

    nr = 9 - Default_expansion_level;
    if (nr > 8) nr = 9;
    ptext ("Select default expansion level!");
    a_old = allow_keypress;
    allow_keypress = 0;
    nr = 9 - ask (10, ask_str, nr);
    allow_keypress = a_old;
    if (nr < 1) { nr = -1;
	sprintf (mess_str, "No default expansion level set!");
    }
    else
	sprintf (mess_str, "Default expansion level = %d", nr);
    Default_expansion_level = nr;
    ptext (mess_str);
}

void indiv_exp ()
{
    INST *ip;
    char  mess_str[MAXCHAR];
    register int lay, old_level;

    if (!present_inst ()) return;

    ptext ("Select an instance for individual expansion!");
    if ((new_cmd = get_cursor (1, 1, NO_SNAP)) != -1) return;

    if (!(ip = search_inst (xlc, ybc))) {
	ptext ("No instance selected!");
	return;
    }

    old_level = ip -> level;

    sprintf (mess_str, "Instance '%s', select another level!", ip -> templ -> cell_name);
    ptext (mess_str);

    if ((ip -> level = ask_level (old_level)) == old_level) {
	ptext ("No expand, new level equal to old level!");
	return;
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	pict_arr[lay] = (ip -> level > old_level) ? DRAW : ERAS_DR;
    }
  /*
  ** If (old_level != 1) sub-bounding boxes have to be redrawn.
  */
    pict_arr[Textnr] = (old_level == 1) ? DRAW : ERAS_DR;

    inst_window (ip, &piwl, &piwr, &piwb, &piwt);

    sprintf (mess_str, "Instance '%s' expanded to level %d",
	ip -> templ -> cell_name, ip -> level);
    ptext (mess_str);
}

int exp_templ (TEMPL *te_p, DM_PROJECT *fath_projkey, int imported, int mode)
/* te_p: template pointer */
/* imported: if true: template is in another project */
/* mode: READ_ALL or READ_TERM */
{
    DM_CELL *cell_key;
    char    *real_cellname;
    register int lay;

    /*
    ** Template primitives/instances or terminals have
    ** to be read.  Although the project key might already
    ** be present as te_p -> projkey, we have to do
    ** a dmFindProjKey () anyway in case of an imported
    ** relationship to obtain the real_cellname.
    ** In case of a local relationship we could resolve
    ** a missing te_p -> projkey ourselves but
    ** dmFindProjKey () already offers this functionality.
    ** Therefore, we will simply call dmFindProjKey ()
    ** on all occasions, thereby also assigning the projkey
    ** to the template in case of a local relationship.
    */
    if (!fath_projkey || te_p -> t_flag == -1) return (-1);

    if (!(te_p -> projkey = dmFindProjKey (imported, te_p -> cell_name, fath_projkey, &real_cellname, LAYOUT))) {
	return (te_p -> t_flag = -1);
    }
    if (!(cell_key = dmCheckOut (te_p -> projkey, real_cellname, ACTUAL, DONTCARE, LAYOUT, READONLY))) {
	return (te_p -> t_flag = -1);
    }

    if (mode == READ_ALL) {
	for (lay = 0; lay < NR_lay; ++lay)
	    te_p -> quad_trees[lay] = qtree_build ();
	/*
	** Read boxes/nors + instances:
	*/
	if (inp_boxnorfl (cell_key, te_p -> quad_trees) == -1) goto err;
	if (inp_mcfl (cell_key, &te_p -> inst) == -1) goto err;
    }
    if (!te_p -> t_flag) {
	te_p -> t_flag = 1;
	/*
	** Read terminals:
	*/
	if (inp_term (cell_key, te_p -> term_p) == -1) goto err;
    }

    dmCheckIn (cell_key, COMPLETE);
    return (0);
err:
    dmCheckIn (cell_key, QUIT);
    return (-1);
}
