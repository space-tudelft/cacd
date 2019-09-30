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

#define NBR_RULES  50
#define DRC_NONE    0
#define DRC_SINGLE  1
#define DRC_ALL	    2

extern struct Disp_wdw *p_wdw;
extern DM_PROJECT *dmproject;
extern DM_CELL *ckey;
extern int  allow_keypress;
extern int  checked;
extern int  erase_text;
extern int  DRC_nr;
extern int  rmode;
extern int *pict_arr;
extern Coor xltb, xrtb, ybtb, yttb; /* total bound box */
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern char *cellstr;

static struct err_pos *head_elist;
static struct err_pos *single_err_p = NULL;
static char *str_arr[NBR_RULES];
static char *chk_cell = "chk_mod";
static int err_to_be_drawn = DRC_NONE;
static int flat_exp = 0; /* NO */

static void disp_all_new_err (void);
static int  draw_error (struct err_pos *p_elist, Coor wxl, Coor wxr, Coor wyb, Coor wyt);
static void new_errlist (void);
static void pict_indiv_err (void);
static int  prep_errlist (char *cellname);
static int  read_err_coor (char *line, int i, Coor *p_err_coor);
static int  wrte_chk_cell (void);

void set_flat_expansion (int aVal)
{
    flat_exp = aVal ? 3 : 0;
}

static char Opt[2][MAXSTR] = {
     /* 0 */ "-h ",
     /* 1 */ "",
};

void setOpt (int k, char *str)
{
    strcpy (&Opt[k][k == 0 ? 3 : 0], str);
}

void check ()
{
    static char *ask_str[] = {
	 /* 0 */ "-cancel-",
	 /* 1 */ "-check-",
	 /* 2 */ "linear",
	 /* 3 */ "hierarch",
	 /* 4 */ "set_opt",
	 /* 5 */ "show_opt",
    };
    static char *tool[] = {
	 /* 0 */ "exp",
	 /* 1 */ "dimcheck2",
    };
    char  tmp_str[MAXSTR];
    int   k, nr, ready = 0;
    int   a_old;

    ptext ("Select DRC expand mode!");
    do {
	nr = ask (sizeof (ask_str) / sizeof (char *), ask_str, flat_exp ? 2 : 3);
	if (nr > 3) {
	    ptext ("Select tool to set/show options!");
	    a_old = allow_keypress;
	    allow_keypress = 0;
	    k = ask (sizeof (tool) / sizeof (char *), tool, -1);
	    allow_keypress = a_old;
	    if (nr == 4) {
		sprintf (tmp_str, "%s options: ", tool[k]);
		ask_string (tmp_str, &Opt[k][k == 0 ? 3 : 0]);
	    }
	    sprintf (tmp_str, "%s options are: %s", tool[k], &Opt[k][k == 0 ? flat_exp : 0]);
	    ptext (tmp_str);
	}
	else if (nr > 1) {
	    flat_exp = (nr == 2) ? 3 : 0;
	    sprintf (tmp_str, "%s expand mode selected", flat_exp ? "Linear" : "Hierarchical");
	    ptext (tmp_str);
	}
    } while (nr > 1);

    erase_text = 1;
    if (nr <= 0) return; /* cancel */

    pre_cmd_proc (flat_exp ? 2 : 3);

    if (rmode) {
	ptext ("No check, you are in read only mode!");
	return;
    }

    if (wrte_chk_cell () == -1) return;

    inform_process ();
    sleep (2);

    ptext ("== EXPANDING (exp) ==");
    sprintf (tmp_str, "%s %s", &Opt[0][flat_exp], chk_cell);
    if (DaliRun ("exp", "/dev/null", tmp_str) == -1) goto drc_err;

    ptext ("== CHECKING (dimcheck2) ==");
    sprintf (tmp_str, "%s/%s.ck", dmproject -> dmpath, chk_cell);
    if (DaliRun ("dimcheck2", tmp_str, Opt[1]) == -1) goto drc_err;

    ready = checked = 1; /* YES */

    ptext ("== REMOVING (temporary cell) ==");
drc_err:
    dmRmCellForce (dmproject, chk_cell, WORKING, DONTCARE, LAYOUT, TRUE);
    if (!ready) return;

    if ((nr = prep_errlist (chk_cell)) >= 0) {
	/* New errors were read. */
	disp_all_new_err ();
	sprintf (tmp_str, "%d design rule errors found", nr);
	ptext (tmp_str);
    }
}

void chk_file ()
{
    char txt[MAXCHAR];
    int  nr;

    if (!cellstr) {
	ptext ("No check results (no cellname)!");
	return;
    }
    if ((nr = prep_errlist (cellstr)) >= 0) {
	/* all errors were read successfully */
	disp_all_new_err ();
	sprintf (txt, "%d errors found in '%s.ck' file", nr, cellstr);
    }
    else { /* nothing happened yet */
	sprintf (txt, "No check result file '%s.ck' present!", cellstr);
    }
    ptext (txt);
}

static int prep_errlist (char *cellname)
{
    char  line[MAXCHAR];
    char *sepchar = " \t\n\r";
    char *p_token;
    FILE *fp_err;
    int   i, line_index, drc_err_nr, next_rule;
    float delta;
    struct err_pos *p_elist, *tail_elist;

    delta = (p_wdw -> wxmax - p_wdw -> wxmin) / 90.0;

    i = 0;
    next_rule = 0; /* NO */

    sprintf (line, "%s/%s.ck", dmproject -> dmpath, cellname);
    if (!(fp_err = fopen (line, "r"))) {
	ptext ("Can't open error file!");
	return (-1);
    }

    /*
    ** Check-file present: clear previous error list before refill.
    */
    tail_elist = 0;
    drc_err_nr = 0;
    new_errlist ();

    while (fgets (line, MAXCHAR, fp_err)) {

	if (strncmp (line, "Rule", 4) == 0) {
	    if (next_rule) {
		/*
		** Take next rule index (errors were found for current one).
		*/
		if (++i >= NBR_RULES) {
		    ptext ("Not all errors read! (too many)");
		    sleep (2);
		    goto ret;
		}
		next_rule = 0; /* NO */
	    }
	    if (!str_arr[i]) {
		/*
		** If i-th entry not previously allocated.
		*/
		MALLOCN (str_arr[i], char, MAXCHAR);
	    }
	    strcpy (str_arr[i], strtok (line, sepchar));
	    strcat (str_arr[i], ":");
	    while ((p_token = strtok (NULL, sepchar))) {
		     if (strncmp (p_token, "Mask", 4) == 0) ;
		else if (strncmp (p_token, "no",  2) == 0) ;
		else if (strncmp (p_token, "Gap", 3) == 0) strcat (str_arr[i], "G:");
		else if (strncmp (p_token, "Wid", 3) == 0) strcat (str_arr[i], "W:");
		else if (strncmp (p_token, "Ove", 3) == 0) strcat (str_arr[i], "Ov:");
		else {
		    strcat (str_arr[i], p_token);
		    strcat (str_arr[i], " ");
		}
	    }
	}
	else if (strncmp (line, "error", 5) == 0) {
	    /*
	    ** Line is an error line: retrieve coordinates.
	    */
	    MALLOC (p_elist, struct err_pos);
	    p_elist -> p_str = str_arr[i];
	    p_elist -> p_nr  = ++drc_err_nr;

	    line_index = read_err_coor (line, 0, &(p_elist -> x1));
	    line_index = read_err_coor (line, line_index, &(p_elist -> y1));
	    line_index = read_err_coor (line, line_index, &(p_elist -> x2));
	    line_index = read_err_coor (line, line_index, &(p_elist -> y2));

	    p_elist -> x1 *= QUAD_LAMBDA;
	    p_elist -> y1 *= QUAD_LAMBDA;
	    p_elist -> x2 *= QUAD_LAMBDA;
	    p_elist -> y2 *= QUAD_LAMBDA;

	    if (p_elist -> x1 == p_elist -> x2) {
		p_elist -> x1_plot = (float) p_elist -> x1 - delta;
		p_elist -> x2_plot = (float) p_elist -> x2 + delta;
	    }
	    else {
		p_elist -> x1_plot = (float) Min (p_elist -> x1, p_elist -> x2);
		p_elist -> x2_plot = (float) Max (p_elist -> x1, p_elist -> x2);
	    }
	    if (p_elist -> y1 == p_elist -> y2) {
		p_elist -> y1_plot = (float) p_elist -> y1 - delta;
		p_elist -> y2_plot = (float) p_elist -> y2 + delta;
	    }
	    else {
		p_elist -> y1_plot = (float) Min (p_elist -> y1, p_elist -> y2);
		p_elist -> y2_plot = (float) Max (p_elist -> y1, p_elist -> y2);
	    }
	    if (tail_elist) tail_elist -> next = p_elist;
	    else head_elist = p_elist;
	    tail_elist = p_elist;
	    /*
	    ** Error(s) found for current rule.
	    ** Next rule must take next index.
	    */
	    next_rule = 1; /* YES */
	}
    }
ret:
    if (tail_elist) tail_elist -> next = 0;
    fclose (fp_err);
    return (drc_err_nr);
}

static int read_err_coor (char *line, int i, Coor *p_err_coor)
{
    char dstr[MAXSTR]; /* digit string */
    int  j;

    while (!isdigit ((int)line[i]) && line[i] != '-') ++i;
    j = 0;
    while (isdigit ((int)line[i]) || line[i] == '-') dstr[j++] = line[i++];
    dstr[j] = '\0';
    *p_err_coor = (Coor) atoi (dstr);
    return (i);
}

static void new_errlist ()
{
    struct err_pos *p_elist;
    /*
    ** Remove existing error list.
    */
    while ((p_elist = head_elist)) {
	head_elist = p_elist -> next;
	FREE (p_elist);
    }
    single_err_p = NULL;
}

static void disp_all_new_err ()
{
    /*
    ** Errors from previous run may still be on the screen.
    */
    pict_arr[DRC_nr] = (err_to_be_drawn == DRC_NONE) ? DRAW : ERAS_DR;
    err_to_be_drawn = (head_elist) ? DRC_ALL : DRC_NONE;
}

void empty_err ()
{
    new_errlist ();
    err_to_be_drawn = DRC_NONE;
}

void toggle_drc_err ()
{
    if (err_to_be_drawn == DRC_NONE) {
	if (head_elist) { /* errors present */
	    err_to_be_drawn = DRC_ALL;
	    pict_arr[DRC_nr] = DRAW;
	}
	else {
	    ptext ("No errors present!");
	}
    }
    else {
	err_to_be_drawn = DRC_NONE;
	if (head_elist) pict_arr[DRC_nr] = ERAS_DR;
    }
}

void disp_next (int inside_window)
{
    struct err_pos *cand_p, *hulp_p;

    if (!head_elist) {
	ptext ("No errors present!");
	return;
    }
    /*
    ** If new error list, or no valid position in error list.
    */
    if (!single_err_p || !single_err_p -> next) {
	cand_p = head_elist;
    }
    else {
	cand_p = single_err_p -> next;
    }
    /*
    ** Candidate error has been identified. If inside
    ** current window --> draw straight away. Otherwise,
    ** either search other one in window or move window.
    */
    if (cand_p -> x1_plot < p_wdw -> wxmax &&
	cand_p -> x2_plot > p_wdw -> wxmin &&
	cand_p -> y1_plot < p_wdw -> wymax &&
	cand_p -> y2_plot > p_wdw -> wymin) {

	/* A hit: cand_p inside current pict-viewport. */
	pict_indiv_err ();
	single_err_p = cand_p;
    }
    else {
	if (inside_window) {
	    /*
	    ** Search next error in this window.
	    ** cand_p itself not in window (tested above).
	    ** Check others cyclicly up to cand_p.
	    */
	    for (hulp_p = (cand_p -> next) ? cand_p -> next : head_elist;
		    hulp_p != cand_p;
		    hulp_p = (hulp_p -> next) ? hulp_p -> next : head_elist) {
		if (hulp_p -> x1_plot < p_wdw -> wxmax &&
		    hulp_p -> x2_plot > p_wdw -> wxmin &&
		    hulp_p -> y1_plot < p_wdw -> wymax &&
		    hulp_p -> y2_plot > p_wdw -> wymin) {

		    break;
		}
	    }
	    if (hulp_p != cand_p) {
		/* Found error in current window. */
		pict_indiv_err ();
		single_err_p = hulp_p;
	    }
	    else {
		ptext ("No errors in this window!");
		/* No change in single_err_p.
		** No drawing of errors.
		*/
		return;
	    }
	}
	else {
	    single_err_p = cand_p;
	    center_w ((Coor) ((single_err_p -> x1 + single_err_p -> x2) / 2.0),
		      (Coor) ((single_err_p -> y1 + single_err_p -> y2) / 2.0));
	    /* Picture flags set by center_w. */

	    /* Alternative, to window of fixed size:
		curs_w (single_err_p -> x1 - 20 * QUAD_LAMBDA,
		    single_err_p -> x2 + 20 * QUAD_LAMBDA,
		    single_err_p -> y1 - 15 * QUAD_LAMBDA,
		    single_err_p -> y2 + 15 * QUAD_LAMBDA);
	    */
	}
    }

    err_to_be_drawn = DRC_SINGLE;
}

static void pict_indiv_err ()
{
    if (err_to_be_drawn == DRC_ALL) {
	/* Lots of errors may currently be displayed: erase.
	** Possible optimization: restrict erase-area.
	*/
	pict_arr[DRC_nr] = ERAS_DR;
	return;
    }

    /* Remaining cases: DRC_NONE and DRC_SINGLE. */
    if (err_to_be_drawn == DRC_SINGLE &&
	    single_err_p -> x1_plot < p_wdw -> wxmax &&
	    single_err_p -> x2_plot > p_wdw -> wxmin &&
	    single_err_p -> y1_plot < p_wdw -> wymax &&
	    single_err_p -> y2_plot > p_wdw -> wymin) {
	/*
	** Single error is currently being displayed: erase.
	*/
	piwl = (Coor) LowerRound (single_err_p -> x1_plot);
	piwr = (Coor) UpperRound (single_err_p -> x2_plot);
	piwb = (Coor) LowerRound (single_err_p -> y1_plot);
	piwt = (Coor) UpperRound (single_err_p -> y2_plot);
	pict_arr[DRC_nr] = ERAS_DR;

	/*
	** Premature picture() to erase old error in an optimal way.
	*/
	err_to_be_drawn = DRC_NONE;	/* prevent redraw. */

	picture ();

	err_to_be_drawn = DRC_SINGLE;	/* restore mode. */
    }
    /*
    ** Set mode to DRAW, for display of single error.
    ** No use to restrict the piw?-area.
    */
    pict_arr[DRC_nr] = DRAW;
}

void draw_drc_err (Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    struct err_pos *p_elist;
    char txt[MAXCHAR];

    if (!head_elist || err_to_be_drawn == DRC_NONE) return;

    ggSetColor (DRC_nr);

    if (err_to_be_drawn == DRC_SINGLE) {
	ASSERT (single_err_p);

	if (draw_error (single_err_p, wxl, wxr, wyb, wyt) == 0) {
	    /*
	    ** The single error has been drawn.
	    */
	    sprintf (txt, "(%d) %s coord: %ld,%ld %ld, %ld",
		single_err_p -> p_nr, single_err_p -> p_str,
		(long) (single_err_p -> x1 / QUAD_LAMBDA),
		(long) (single_err_p -> y1 / QUAD_LAMBDA),
		(long) (single_err_p -> x2 / QUAD_LAMBDA),
		(long) (single_err_p -> y2 / QUAD_LAMBDA));
	    ptext (txt);
	}
    }
    else {
	/* DRC_ALL */
	for (p_elist = head_elist; p_elist; p_elist = p_elist -> next) {
	    (void) draw_error (p_elist, wxl, wxr, wyb, wyt);
	}
    }
}

static int draw_error (struct err_pos *p_elist, Coor wxl, Coor wxr, Coor wyb, Coor wyt)
{
    if (p_elist -> x1_plot < wxr && p_elist -> x2_plot > wxl &&
	p_elist -> y1_plot < wyt && p_elist -> y2_plot > wyb) {

	paint_box (Max (wxl, p_elist -> x1_plot), Min (wxr, p_elist -> x2_plot),
	       Max (wyb, p_elist -> y1_plot), Min (wyt, p_elist -> y2_plot));
	return (0);
    }
    return (-1);
}

void ind_err (Coor x_cur, Coor y_cur)
{
    struct err_pos *p_elist;
    char txt[MAXCHAR];

    for (p_elist = head_elist; p_elist; p_elist = p_elist -> next) {
	if ((float) x_cur >= p_elist -> x1_plot &&
	    (float) x_cur <= p_elist -> x2_plot &&
	    (float) y_cur >= p_elist -> y1_plot &&
	    (float) y_cur <= p_elist -> y2_plot) {

	    sprintf (txt, "(%d) %s coord: %ld,%ld %ld, %ld",
		    p_elist -> p_nr, p_elist -> p_str,
		    (long) (p_elist -> x1 / QUAD_LAMBDA),
		    (long) (p_elist -> y1 / QUAD_LAMBDA),
		    (long) (p_elist -> x2 / QUAD_LAMBDA),
		    (long) (p_elist -> y2 / QUAD_LAMBDA));
	    ptext (txt);
	    return;
	}
    }
    ptext ("No error at the indicated spot!");
}

static int wrte_chk_cell ()
{
    DM_CELL *save_key;
    int exist, rv = -1;

    upd_boundb ();
    if (xltb == xrtb || ybtb == yttb) {
	ptext ("No designrule check (empty cell)!");
	return (rv);
    }

    if ((exist = _dmExistCell (dmproject, chk_cell, LAYOUT))) {
	if (exist == 1) /* chk_cell already exists: DRC in progress? */
	    ptext ("Check cell already exists!");
	return (rv);
    }
    /*
    ** cell does not yet exist
    */
    save_key = ckey;
    if (!(ckey = dmCheckOut (dmproject, chk_cell, WORKING, DONTCARE, LAYOUT, UPDATE))) {
	ptext ("Can't create check cell!");
	goto ret;
    }

    if (!(outp_boxfl () && outp_mcfl () && outp_term () && outp_bbox ())) {
	/*
	** Files were not written properly so if a new key
	** was obtained to write under a new name, it must
	** be checked in using the quit mode.
	*/
	dmCheckIn (ckey, QUIT);
	goto ret;
    }

    if (dmCheckIn (ckey, COMPLETE) == -1) {
	ptext ("CheckIn not accepted (recursive)!");
	dmCheckIn (ckey, QUIT);
	goto ret;
    }
    rv = 0;
ret:
    ckey = save_key;
    return (rv);
}
