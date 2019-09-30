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

#define MAX_MENU  30	/* maximum size of a menu */

extern DM_PROJECT *dmproject;
extern int  mH, tH;
extern int  NR_lay;
extern int  NR_all;
extern int  Backgr;
extern int  Gridnr;
extern int  Textnr;
extern int  Yellow;
extern int *def_arr;
extern int *edit_arr;
extern int *dom_arr;
extern int *term_bool;
extern int *fillst;
extern int *vis_arr;
extern int *eIds;
extern int *pict_arr;
extern struct Disp_wdw *c_wdw;
extern int *MaskLink;

extern TERM **term_root;
extern qtree_t **quad_root;

int nbr_max;
int nbr_offset = 0;
static int colmenu_init = 0;
static char **vis_str;
char  **lay_names;
int vismenu_mode = 0;

void Rmsk ()
{
    init_colmenu ();
}

void init_mskcol ()
{
    register int lay;
    int     code;
    DM_PROCDATA * process = dmproject -> maskdata;
  /*
  ** The mask_numbers in the process structure are guaranteed
  ** to go from 0 to no_masks-1 (in that order). If this editor
  ** makes all these masks editable we can use the mask_numbers
  ** of the boxes and terminals directly as the indices in the
  ** data-structures: simple and efficient.
  */
    lay_names = process -> mask_name;
    NR_lay = process -> nomasks;
    NR_all = NR_lay + 3;

    MALLOCN (def_arr  , int, NR_lay); // init in command()
    MALLOCN (edit_arr , int, NR_lay); // init in command()
    MALLOCN (fillst   , int, NR_lay); // init here
    MALLOCN (term_bool, int, NR_lay); // init here
    MALLOCN (dom_arr  , int, NR_lay); // init here
    MALLOCN (pict_arr , int, NR_all); // init in command()
    MALLOCN (eIds     , int, NR_all); // set in ggSetErasedAffected()

    MALLOCN (vis_arr, int  , NR_lay + 8); // init here
    MALLOCN (vis_str, char*, 11); // init here

    MALLOCN (term_root, TERM *, NR_lay); // init here
    MALLOCN (quad_root, qtree_t *, NR_lay); // init here

    for (lay = 0; lay < NR_lay; ++lay) {
	term_bool[lay] = (process -> mask_type[lay] == DM_INTCON_MASK);
	code = process -> RT[lay] -> code;
	if (code == 15) code = 0;
	ggSetIdForColorCode (code);
	dom_arr[lay] = (code == 0);
	switch (process -> RT[lay] -> fill) {
	    case 0: fillst[lay] = FILL_HASHED;   /*  12% fill */ break;
	    case 2: fillst[lay] = FILL_HOLLOW;   /* 100% fill */ break;
	    case 3: fillst[lay] = FILL_HASHED12B; /* 12% fill + outline */ break;
	    case 4: fillst[lay] = FILL_HASHED25B; /* 25% fill + outline */ break;
	    case 5: fillst[lay] = FILL_HASHED50B; /* 50% fill + outline */ break;
	    case 6: fillst[lay] = FILL_HASHED;    /* 12% fill */ break;
	    case 7: fillst[lay] = FILL_HASHED25;  /* 25% fill */ break;
	    case 8: fillst[lay] = FILL_HASHED50;  /* 50% fill */ break;
	    default: fillst[lay] = FILL_SOLID; /* this is also case 1 */
	}
	def_arr[lay] = 0;
	edit_arr[lay] = 1;
	vis_arr[lay] = 1;
	term_root[lay] = NULL;
	quad_root[lay] = NULL;
    }
    vis_str[0] = "-return-";
    vis_str[1] = "-all_on-";
    vis_str[2] = "-all_off-";
    vis_arr[lay++] = 1; vis_str[3] = "terminals";
    vis_arr[lay++] = 1; vis_str[4] = "sub_terms";
    vis_arr[lay++] = 1; vis_str[5] = "instances";
    vis_arr[lay++] = 1; vis_str[6] = "bbox";
    vis_arr[lay++] = 1; vis_str[7] = "term_name";
    vis_arr[lay++] = 1; vis_str[8] = "subt_name";
    vis_arr[lay++] = 1; vis_str[9] = "inst_name";
    vis_arr[lay] = 1; vis_str[10] = "label_name";
}

/* from name to number */
int msk_nbr (char *msk)
{
    register int lay;

    for (lay = 0; lay < NR_lay; lay++) {
	if (strcmp (lay_names[lay], msk) == 0) return (lay);
    }
    fprintf (stderr, "Mask '%s' unknown\n", msk);
    return (-1);
}

void disable_masks (int no_masks, int *mask_array)
{
    float ddy;
    int lay;

    set_c_wdw (LAYS);
    ggSetColor (Gridnr);

    while (--no_masks >= 0) {
	if ((lay = mask_array[no_masks]) < 0) continue;
	edit_arr[lay] = FALSE;
	ddy = YB + lay;
	d_line (XL, ddy, XR, ddy + 1);
	d_line (XR, ddy, XL, ddy + 1);
    }
}

void Visible ()
{
    register int lay;
    int nbr, nr;
    int action;

start_menu:
    vismenu_mode = 1;
    ptext ("Click mask name to toggle visibility");
    menu (11, vis_str);

    for (nbr = 0; nbr < 8; ++nbr) {
	if (vis_arr[nbr + NR_lay]) pre_cmd_proc (nbr + 3, vis_str);
    }

    while ((nbr = get_com ()) != 0) {
	if (nbr < 0) goto start_menu;
	if (nbr < 3) { /* (1) all_on, (2) all_off */
	    action = nbr == 1 ? 1 : 0;
	    for (lay = 0; lay < NR_lay; lay++) {
		if (action) pict_arr[lay] = DRAW;
		else def_arr[lay] = 0;
		vis_arr[lay] = action;
	    }
	    for (nr = 3; nr < 11; ++nr) {
		vis_arr[lay++] = action;
		if (action)
		    pre_cmd_proc (nr, vis_str);
		else
		    post_cmd_proc (nr, vis_str);
	    }
	    init_colmenu ();
	    if (action) {
		pict_arr[Textnr] = DRAW;
		picture ();
	    }
	    else {
		set_c_wdw (PICT);
		ggClearWindow ();
	    }
	}
	else {
	    nr = nbr + NR_lay - 3;
	    if (vis_arr[nr]) {
		post_cmd_proc (nbr, vis_str);
		vis_arr[nr] = 0; /* OFF */
		action = ERAS_DR;
	    }
	    else {
		pre_cmd_proc (nbr, vis_str);
		vis_arr[nr] = 1; /* ON */
		action = DRAW;
	    }
	    pict_arr[Textnr] = action;
	    if (nr == NR_lay || nr == NR_lay + 1) { /* terminals */
		for (lay = 0; lay < NR_lay; ++lay)
		    if (term_bool[lay] == TRUE) pict_arr[lay] = action;
	    }
	    else if (nr == NR_lay + 2) { /* instances */
		for (lay = 0; lay < NR_lay; ++lay)
		    pict_arr[lay] = action;
	    }
	    picture ();
	}
    }
    vismenu_mode = 0;
}

void paint_lay (int nbr)
{
    float ddy;
    int lay;

    if (!colmenu_init) {
	set_c_wdw (LAYS);
	ddy = YB + nbr;
	ggClearArea (XL, XR, ddy, ddy + 1);
    }
    else ddy = YB + nbr;

    lay = nbr + nbr_offset;
    if (lay < NR_lay && vis_arr[lay]) {
	ggSetColor (lay);
	d_fillst (fillst[lay]);
	paint_box (XL+0.6, XR-0.02, ddy+0.05, ddy+0.95);
    }
    d_fillst (FILL_SOLID);
    if (lay < NR_lay && def_arr[lay]) { /* bulb */
	ggSetColor (Yellow);
	paint_box (XL, XL+0.09, ddy, ddy + 0.9);
    }
    ggSetColor (Gridnr);
    if (nbr) d_line (XL, ddy, XR, ddy);
    if (!colmenu_init) d_line (XL, ddy+1, XR, ddy+1);
    if (lay >= NR_lay) return;
    d_text (XL + 0.1, ddy + 0.25, lay_names[lay]);
    if (!vis_arr[lay]) d_text (XL + 0.6, ddy + 0.25, "-off-");
    if (!edit_arr[lay]) {
	d_line (XL, ddy, XR, ddy + 1);
	d_line (XR, ddy, XL, ddy + 1);
    }
}

static void do_colmenu ()
{
    float ddy;
    int nbr;

    set_c_wdw (LAYS);
    ggClearWindow ();

    colmenu_init = 1;
    for (nbr = 0; nbr < nbr_max; ++nbr) paint_lay (nbr);
    colmenu_init = 0;
    ddy = nbr;
    ggSetColor (Yellow);
    d_text (XL + 0.1, ddy + 0.25, "next");
    d_text (XL + 0.6, ddy + 0.25, "prev");
    ggSetColor (Gridnr);
    d_line (XL + 0.5, ddy, XL + 0.5, YT);
    d_line (XL, ddy, XR, ddy);
}

void init_colmenu ()
{
    nbr_max = mH / tH;
    def_world_win (LAYS, 0.0, 1.0, 0.0, (float) nbr_max);
    --nbr_max;
    do_colmenu ();
}

void next_colmenu ()
{
    int lay = nbr_max + nbr_offset;
    if (lay < NR_lay) {
	nbr_offset = lay;
	do_colmenu ();
    }
}

void prev_colmenu ()
{
    if (nbr_offset > 0) {
	if ((nbr_offset -= nbr_max) < 0) nbr_offset = 0;
	do_colmenu ();
    }
}

void bulb (int lay, int flag)
{
    float ddy;
    int nbr = lay - nbr_offset;
    if (nbr < 0 && nbr >= nbr_max) return;

    set_c_wdw (LAYS);
    disp_mode (DOMINANT);

    d_fillst (FILL_SOLID);
    ggSetColor (flag == TRUE ? Yellow : Backgr);
    ddy = YB + nbr;
    paint_box (XL, XL+0.09, ddy, ddy + 0.9);

    disp_mode (TRANSPARENT);
    flush_pict ();
}

/*
 * This routine links masks together
 */
void link_masks (int no_masks, int *mask_array)
{
    static int linkNR = 0;
    int i, j, l;

    for (i = 0; i < no_masks; ++i) if (mask_array[i] >= 0) break;
    j = i;
    do {
	while (++j < no_masks) if (mask_array[j] >= 0) break;
	if (j >= no_masks) return;
    } while (mask_array[j] == mask_array[i]);

    if (!MaskLink) { /* first time */
	MALLOCN (MaskLink, int, NR_lay);
	for (l = 0; l < NR_lay; ++l) MaskLink[l] = 0;
    }
    ++linkNR; /* index of the linked mask group */
    MaskLink[mask_array[i]] = linkNR;
    MaskLink[mask_array[j]] = linkNR;
    while (++j < no_masks) if (mask_array[j] >= 0) MaskLink[mask_array[j]] = linkNR;
}
