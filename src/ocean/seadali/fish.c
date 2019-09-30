/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Patrick Groeneveld
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
#include <sys/stat.h>
#include <sys/signal.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include "src/ocean/libseadif/sdferrors.h"

#define NBR_RULES  50
#define DRC_NONE    0
#define DRC_SINGLE  1
#define DRC_ALL     2

#define SDFTEMPCELL "Tmp_Cell_"
#define TEMPCELLPREFIX "Tmp"

#define SEADALI_DIR "share/lib/seadali"

extern int autopsy_on_sea_child (void);
extern int child_exists (void);
extern int kill_the_sea_child (void);
extern int looks_like_the_sea_child_died (int);
extern int runprog       (char *prog, char *outfile, int *exitstatus, ...);
extern int runprogsea    (char *prog, char *outfile, int *exitstatus, ...);
extern int runprognowait (char *prog, char *outfile, int *exitstatus, ...);
extern int signal_trout_to_stop (void);

static void empty_mem2 (void);
static int  in_flight_entertainment (char **prog);
static void lock_alert (char *progname);
static void make_temp_cell_name (void);
static void trout_options_menu (char *trout_options);
static int  wrte_fish_cell (void);

extern qtree_t **quad_root;
extern INST *inst_root;
extern TERM **term_root;
extern DM_PROJECT *dmproject;
extern DM_CELL *ckey;
extern int  new_cmd;
extern int  dirty;
extern int  cmd_nbr;
extern int  Gridnr;
extern Coor xltb, xrtb, ybtb, yttb; /* total bound box */
extern Coor piwl, piwr, piwb, piwt; /* window to be drawn */
extern Coor xlc, xrc, ybc, ytc;
extern char *cellstr;
extern char cirname[];
extern char *DisplayName;
extern struct Disp_wdw *c_wdw;
extern int  exp_level;		/* expansion level of overall expansions */
extern int  Sub_terms;          /* TRUE to indicate sub terminals */
extern int  Default_expansion_level;
extern TEMPL *first_templ;      /* templates of instances */
extern INST *act_inst;
extern TERM *act_term;
extern int  NR_lay;
extern int  checked;
extern char *yes_no[];
extern long Clk_tck;
extern char *Print_command;

static char NelsisTempCell[DM_MAXNAME + 2];

static char *trouterrfile = "seadif/trout.error";
static char *sealogfile = "seadif/sea.out";
static char *tmplogfile = "seadif/tmp.out";

#define TROUT_CMD     10
static char *trout_cmd[] = {
     /* 0 */ "- cancel -",
     /* 1 */ "$3** DO IT **",
     /* 2 */ "no power",
     /* 3 */ "erase wires",
     /* 4 */ "no poly\nfeedthrough",
     /* 5 */ "border\nterminals",
     /* 6 */ "route\nbox only",
     /* 7 */ "single pass\nrouting",
     /* 8 */ "Option menu",
     /* 9 */ "&?&? Help &?&?",
};
#define TROUT_RUN 1

#define TROUT_OPT     12
static char *trout_opt[] = {
     /* 0 */ "- return -",
     /* 1 */ "no routing",
     /* 2 */ "use exact\nchip area",
     /* 3 */ "dangling\ntransistors",
     /* 4 */ "substrate\ncontacts",
     /* 5 */ "make fat\npower lines",
     /* 6 */ "make\ncapacitors",
     /* 7 */ "use borders",
     /* 8 */ "overlap\nwires",
     /* 9 */ "flood mesh",
     /* 10 */ "NO\nverify",
     /* 11 */ "options",
};
#define TROUT_RET 0


#define MADONNA_CMD     13
static char *madonna_cmd[] = {
     /* 0 */ "- cancel -",
     /* 1 */ "&?&? Help &?&?",
     /* 2 */ "$1&Bounding box\nwindow",
     /* 4 */ "$1&Center\nwindow",
     /* 5 */ "$1Zoom &In",
     /* 6 */ "$1Zoom &Out",
     /* 1 */ "$3** DO IT **",
     /* 7 */ "$2->$9 set box $2<-",
     /* 8 */ "Y-expand",
     /* 9 */ "X-expand",
     /* 10 */ "Channels",
     /* 11 */ "Place and\nRoute",
     /* 12 */ "Options",
};

#define MADONNA_HELP    1
#define MADONNA_CANCEL  0
#define MADONNA_BBX     2
#define MADONNA_CENTER  3
#define MADONNA_ZOOM    4
#define MADONNA_DEZOOM  5
#define MADONNA_RUN     6
#define MADONNA_BOX     7
#define MADONNA_Y       8
#define MADONNA_X       9
#define MADONNA_CHAN   10
#define MADONNA_TROUT  11
#define MADONNA_OPT    12

static char *madonna_kill[] = {
     /* 0 */ "--",
     /* 1 */ "&*&* KILL &*&*\nMadonna",
     /* 2 */ "&>&> STOP &<&<\nMadonna",
     /* 2 -- "&>&> STOP &<&<\nMadonna gently", */
     /* 3 */ "--",
};

static char *trout_kill[] = {
     /* 0 */ "--",
     /* 1 */ "&*&* KILL &*&*\nTrout",
     /* 2 */ "&>&> STOP &<&<\nTrout",
     /* 2 -- "&>&> STOP &<&<\nTrout gently", */
     /* 3 */ "--",
};

static char *print_kill[] = {
     /* 0 */ "--",
     /* 1 */ "&*&* ABORT &*&*\nPrinting",
     /* 2 */ "--",
     /* 3 */ "--",
};

static char *verify_kill[] = {
     /* 0 */ "--",
     /* 1 */ "&*&* STOP &*&*\nVerifying",
     /* 2 */ "--",
     /* 3 */ "--",
};

#define KILL_MENU  4
#define KILL_CHILD 1
#define KILL_STOP  2

int in_flight_flag = 0;

static void do_echo (char *str, char *file)
{
    FILE *fp = fopen (file, "a");
    if (fp) {
	fprintf (fp, "%s\n", str);
	fclose (fp);
    }
}

static int make_seadif ()
{
    if (access ("seadif", F_OK) && mkdir ("seadif", 0755)) {
	ptext ("ERROR: cannot make seadif directory");
	return 1;
    }
    return 0;
}

/*
 * go fishing the current cell
 */
void do_fish (char *option)
{
    DM_CELL *newcellkey;
    int exist, exitstatus, old_exp_level;
    int empty_image = FALSE;
    char fishoptions[100];
    struct stat buf;
    clock_t start_tick, curr_tick;
    struct tms start_times, end_times;
    char *fishlogfile = "seadif/fish.out";

    /* save old window
    */
    save_oldw ();

    old_exp_level = exp_level;

    start_tick = times (&start_times);

    /* make seadif directory, in which we put the outputfile
    */
    if (make_seadif ()) return;

    /* manage temporary cell name
    */
    make_temp_cell_name ();

    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) == 1) {
	/* remove it, must be junk... */
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
	    sprintf (fishoptions, "ERROR: scratch cell '%s' exists and I cannot remove it!", NelsisTempCell);
	    ptext (fishoptions);
	    sleep (1);
	    return;
	}
    }

    if (no_works ()) { /* design is empty: make empty image */
	set_titlebar ("---- CREATING EMPTY IMAGE ----");
	ptext ("---- busy ----");
	if (runprog ("fish", fishlogfile, &exitstatus, "-q", "-o", NelsisTempCell, NULL) == -1) {
	    ptext ("---- FISH FAILED: failure during calling ----");
	    return;
	}
	empty_image = TRUE;
	old_exp_level = Max (3, Default_expansion_level);
	/* make sure that the image is visible */
    }
    else {
	/*
	* write the cell
	*/
	if (wrte_fish_cell () == -1) return;

	set_titlebar ("FISHING, please relax and wait");
	/*
	* go fishing
	*/
	ptext ("---- busy purifying ----");
	if (*option)
	    sprintf (fishoptions, "-q%s", option);
	else
	    sprintf (fishoptions, "-q");  /* use -b */

	if (runprog ("fish", fishlogfile, &exitstatus, fishoptions, NelsisTempCell, NULL) == -1) {
	    ptext ("---- FISH FAILED: failure during calling ----");
	    set_titlebar (NULL);
	    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	    return;
	}
    }

    /* before re-reading: have a look at the exitstatus...
    */
    switch (exitstatus) {
    case 0: /* OK */
	break;
    case SDFERROR_CALL:
	ptext ("Internall error: wrong call of fish");
	sleep (1);
    case 1:
    default:
	xfilealert (0, fishlogfile);
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	ptext ("---- FISH FAILED ----");
	set_titlebar (NULL);
	return;
    }

    dirty = (empty_image == TRUE) ? FALSE : TRUE;

    /* read again
    */
    empty_mem2 ();          /* clear memory */

    if (!(newcellkey = dmCheckOut (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, READONLY))) {
	ptext ("ERROR: re-read of fished cell failed. Your cell is lost! (sorry)");
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    if (inp_boxnorfl (newcellkey, quad_root) == -1 ||
	   inp_mcfl (newcellkey, &inst_root) == -1 || inp_term (newcellkey, term_root) == -1) {
	dmCheckIn (newcellkey, QUIT);
	empty_mem ();           /* also performs an init_mem () */
	ptext ("ERROR: Can't read cell 'Tmp_Cell_' properly! Your cell is lost! (sorry)");
	sleep (1);
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    if (inp_comment (newcellkey) == -1) {
	char err_str[132];
	sprintf (err_str, "Warning: can't read comments of cell '%s'", NelsisTempCell);
	ptext (err_str);
	sleep (1);
    }

    dmCheckIn (newcellkey, QUIT);

    if (empty_image == FALSE)
	prev_w ();
    else
	bound_w ();

    picture ();
    expansion (old_exp_level);
    if (Sub_terms == TRUE) all_term (); /* draw sub terminals */
    set_titlebar (NULL);

    /* remove cell
    */
    if (empty_image == FALSE) ptext ("---- Removing temporary cell ----");
    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);

    /* time statistics
    */
    curr_tick = times (&end_times);

    /* find out if some errors were printed...
    */
    if (access (fishlogfile, R_OK) == 0 &&
	    stat (fishlogfile, &buf) == 0 && buf.st_size > 10) { /* something was printed..... */
	xfilealert (2, fishlogfile);
	sprintf (fishoptions, "--> Ready with fishing (ERRORS FOUND!) <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
	    (float)(((float) end_times.tms_utime - start_times.tms_utime +
	    end_times.tms_stime - start_times.tms_stime +
	    end_times.tms_cutime - start_times.tms_cutime +
	    end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
	    (float)(((float) curr_tick - start_tick)/Clk_tck),
	    (float)(((float) end_times.tms_utime - start_times.tms_utime +
	    end_times.tms_stime - start_times.tms_stime +
	    end_times.tms_cutime - start_times.tms_cutime +
	    end_times.tms_cstime - start_times.tms_cstime)/
	    ((float) curr_tick - start_tick)) * 100);
	ptext (fishoptions);
    }
    else {
	if (empty_image == FALSE) {
	    sprintf (fishoptions, "--> Ready with fishing! <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_stime - start_times.tms_stime +
		end_times.tms_utime - start_times.tms_utime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	    ptext (fishoptions);
	}
	else
	    ptext ("---- created an empty piece of image ----");
    }
}

/*
 * This routine makes a unique temporary cell name
 * which is put in NelsisTempCell
 */
static void make_temp_cell_name ()
{
    int namepartlength;
    char hstr[DM_MAXNAME + 1];
#if DM_MAXNAME > 100
    extern int dm_maxname;
#else
    int dm_maxname = DM_MAXNAME;
#endif

    sprintf (NelsisTempCell, "%d", (int) getpid ());

    namepartlength = dm_maxname - strlen (TEMPCELLPREFIX) - strlen (NelsisTempCell) - 1;

    if (namepartlength <= 0) {
	fprintf (stderr, "ERROR: No length for name part (make_temp_cell_name)\n");
	return;
    }

    if (!cellstr) { /* no works.. */
	strncpy (hstr, "noname", namepartlength);
    }
    else {
	strncpy (hstr, cellstr, namepartlength);
    }
    hstr[namepartlength] = '\0';

    /* do it
    */
    sprintf (NelsisTempCell, "%s%s%d", TEMPCELLPREFIX, hstr, (int) getpid ());
    NelsisTempCell[dm_maxname] = '\0';
}

/*
 * This routine writes the temporary fish cell into the database as 'Tmp_Cell_'.
 */
static int wrte_fish_cell ()
{
    int exist;
    DM_CELL *cellkey;
    char hstr[100];

    upd_boundb ();

    if (xltb == xrtb || ybtb == yttb) {
	ptext ("Design is empty!!");
	return (-1);
    }

    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) == 1) {
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT))) {
	    sprintf (hstr, "ERROR: scratch cell '%s' exists and I cannot remove it!", NelsisTempCell);
	    ptext (hstr);
	    sleep (1);
	    return (-1);
	}
    }

    /*
    ** cell does not yet exist
    */
    if (!(cellkey = dmCheckOut (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, UPDATE))) {
	ptext ("Can't create scratch cell!");
	return (-1);
    }

    ptext ("---- Writing temporary cell ----");

    if (!(outp_boxfl (cellkey) && outp_mcfl (cellkey) && outp_term (cellkey)
	&& outp_comment (cellkey)
	&& outp_bbox (cellkey))) {
	/*
	** Files were not written properly so if a new key
	** was obtained to write under a new name, it must
	** be checked in using the quit mode.
	*/
	dmCheckIn (cellkey, QUIT);
	return (-1);
    }

    if (dmCheckIn (cellkey, COMPLETE) == -1) {
	ptext ("CheckIn not accepted (recursive)!");
	dmCheckIn (cellkey, QUIT);
	return (-1);
    }
    return (0);
}

static void init_mem2 ()
{
    register int lay;

    inst_root = NULL;
    first_templ = NULL;
    act_inst = NULL;
    act_term = NULL;

    for (lay = 0; lay < NR_lay; ++lay) {
	quad_root[lay] = qtree_build ();
	term_root[lay] = NULL;
    }
    checked = FALSE;
    exp_level = 1;
}

static void empty_mem2 ()
{
    INST *inst_p, *next_inst;
    TEMPL *templ_p, *next_templ;
    TERM *tpntr, *next_term;
    register int lay;

    for (lay = 0; lay < NR_lay; ++lay) quad_clear (quad_root[lay]);

    /* now clear 'root' instances */
    for (inst_p = inst_root; inst_p; inst_p = next_inst) {
	next_inst = inst_p -> next;
	FREE (inst_p);
    }

    /* now we have a list of 'floating' templates: clear them */
    for (templ_p = first_templ; templ_p; templ_p = next_templ) {
	next_templ = templ_p -> next;
	clear_templ (templ_p);	/* clears all quads, lower instances, etc. */
    }

    for (lay = 0; lay < NR_lay; ++lay) {
	for (tpntr = term_root[lay]; tpntr; tpntr = next_term) {
	    next_term = tpntr -> nxttm;
	    FREE (tpntr);
	}
    }

    empty_err ();

    empty_comments ();

    init_mem2 ();
}

/*
 * Go autorouting the current cell
 */
void do_autoroute (int verify_only)
{
    DM_CELL *newcellkey;
    int exist, box_set, no_power, no_universal_feedthrough, erase_wires;
    int single_pass, border_terms, exitstatus, old_exp_level;
    char b1[20], b2[20], b3[20], b4[20], trout_options[200], extra_options[200],
    *seaopt, mess_str[100], net_name[DM_MAXNAME +10], circuit_name[DM_MAXNAME +10];
    char *click_doit_msg;
    clock_t start_tick, curr_tick;
    struct tms start_times, end_times;

    /* save old window
    */
    save_oldw ();

    old_exp_level = exp_level;

    if (no_works ()) { /* design is empty? */
	ptext ("ERROR: design appears to be empty. You must have a placement!");
	return;
    }

    make_temp_cell_name ();

    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
	if (exist == 1) { /* remove it, must be junk... */
	    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
		sprintf (trout_options, "ERROR: scratch cell '%s' exists and I cannot remove it!", NelsisTempCell);
		ptext (trout_options);
		sleep (1);
		return;
	    }
	}
    }

    if (verify_only == FALSE)
	set_titlebar ("Trout, the official router of Delft University, welcomes you");
    else
	set_titlebar ("Welcome to the circuit verifier");

    /* copy default cell name
    */
    if (!*cirname && cellstr) strcpy (cirname, cellstr);

    /* step 1: ask for the circuit name....
    */
    if (!*cirname)
	sprintf (mess_str, "Enter circuit name belonging to this layout: ");
    else
	sprintf (mess_str, "Enter circuit name belonging to layout (default '%s'): ", cirname);

    if (ask_name (mess_str, circuit_name, TRUE) == -1 || !*circuit_name) {
	if (*cirname) strcpy (circuit_name, cirname); /* use default */
	else {
	    ptext ("ERROR: You must enter a valid circuit name");
	    set_titlebar ("");
	    return;
	}
    }

    /* check whether it exists...
    */
    if (!*circuit_name || _dmExistCell (dmproject, circuit_name, CIRCUIT) != 1) {
	sprintf (mess_str, "Hey! Circuit '%s' does not exist.", circuit_name);
	ptext (mess_str);
	set_titlebar ("");
	return;
    }

    strcpy (cirname, circuit_name);
    box_set = FALSE;

    click_doit_msg = "Click DO IT to start routing, or click commands to set preferences.";

    /* put options menu
    */
    if (verify_only == FALSE) {
	set_titlebar (")> trout )>    Click DO IT to start routing");
	ptext (click_doit_msg);
	cmd_nbr = -1;
	no_power = FALSE;
	erase_wires = FALSE;
        no_universal_feedthrough = FALSE;
	border_terms = TRUE;
	single_pass = FALSE;
	*extra_options = 0;

	while (cmd_nbr != TROUT_RUN) {
	    cmd_nbr = 0;
	    menu (TROUT_CMD, trout_cmd);

	    /* set bulbs */
	    if (no_power == TRUE) pre_cmd_proc (2, trout_cmd);
	    if (erase_wires == TRUE) pre_cmd_proc (3, trout_cmd);
	    if (no_universal_feedthrough == TRUE) pre_cmd_proc (4, trout_cmd);
	    if (border_terms == TRUE) pre_cmd_proc (5, trout_cmd);
	    if (box_set == TRUE) pre_cmd_proc (6, trout_cmd);
	    if (single_pass == TRUE) pre_cmd_proc (7, trout_cmd);
	    get_cmd ();
	    switch (cmd_nbr) {
	    case -2:
		ptext (click_doit_msg);
		continue;
	    case TROUT_RUN:
		pre_cmd_proc (TROUT_RUN, trout_cmd);
		break;
	    case 0: /* cancel */
		ptext (" ");
		set_titlebar ("");
		return;
	    case 2: /* no power */
		if (no_power == TRUE) {
		    ptext ("Horizontal power rails will be connected by vertical wires.");
		    no_power = FALSE;
		}
		else {
		    ptext ("Power nets will be routed like ordinary signal nets. Power rails are not necessarity connected.");
		    no_power = TRUE;
		}
		break;
	    case 3: /* erase wires */
		if (erase_wires == TRUE) {
		    ptext ("All existing boxes and terminals in the layout remain intact.");
		    erase_wires = FALSE;
		}
		else {
		    ptext ("All boxes and terminals will be removed before routing.");
		    erase_wires = TRUE;
		}
		break;
	    case 4: /* no universal feedthroughs */
		if (no_universal_feedthrough == TRUE) {
		    ptext ("Allow Universal Feedthrough.");
		    no_universal_feedthrough = FALSE;
		}
		else {
		    ptext ("No Universal Feedthrough.");
		    no_universal_feedthrough = TRUE;
		}
		break;
	    case 5: /* border terms */
		if (border_terms == TRUE) {
		    ptext ("No auto-placement of terminals on border.");
		    border_terms = FALSE;
		}
		else {
		    ptext ("Auto-placement of all (unplaced) terminal on the border of the cell.");
		    border_terms = TRUE;
		}
		break;
	    case 6:  /* route in box */
		pre_cmd_proc (5, trout_cmd);
		ptext ("Specify the box for routing. Only terminals in this box will be routed.");
		if (box_set == TRUE) { /* erase previous box.. */
		    set_c_wdw (PICT);
		    disp_mode (ERASE);
		    ggSetColor (Gridnr);
		    d_fillst (FILL_HASHED12B);
		    paint_box ((float) xlc, (float) xrc, (float) ybc, (float) ytc);
		    disp_mode (TRANSPARENT);
		    set_c_wdw (MENU);
		}
		if ((new_cmd = set_tbltcur (2, SNAP)) == -1) { /* set the coordinates */
		    box_set = TRUE;
		    set_c_wdw (PICT); /* draw box */
		    ggSetColor (Gridnr);
		    d_fillst (FILL_HASHED12B);
		    disp_mode (TRANSPARENT);
		    paint_box ((float) xlc, (float) xrc, (float) ybc, (float) ytc);
		    disp_mode (TRANSPARENT);
		    set_c_wdw (MENU);
		    ptext ("Only terminals in the indicated box will be routed.");
		}
		else {
		    box_set = FALSE;
		    ptext ("No routing box is specified.");
		}
		break;
	    case 7: /* single pass */
		if (single_pass == TRUE) {
		    ptext ("Router may attempt multiple passes for routing.");
		    single_pass = FALSE;
		}
		else {
		    ptext ("Router will only make one attempt to route the nets.");
		    single_pass = TRUE;
		}
		break;
	    case 8: /* options menu */
		trout_options_menu (extra_options);
		set_titlebar (")> trout )>    Click DO IT to start routing");
		ptext (click_doit_msg);
		break;
	    case 9: /* Help... */
		pre_cmd_proc (cmd_nbr, trout_cmd);
		ptext ("Starting help viewer, one moment please...");
		start_browser ("help");
		sleep (2);
		ptext (click_doit_msg);
		break;
	    default:
		ptext ("Command not implemented!");
	    }
	    picture ();
	    post_cmd_proc (cmd_nbr, trout_cmd);
	}
	/* construct options string */
     /* fprintf (stderr, "extra_options: '%s'\n", extra_options);  */
	sprintf (trout_options, "-R %s%s%s%s%s%s",
	    no_power == TRUE      ? "-p" : "",
	    single_pass == TRUE   ? " -m" : "",
	    no_universal_feedthrough == TRUE   ? " -U" : "",
	    erase_wires == TRUE   ? " -e" : "",
	    border_terms == FALSE ? " -b" : "",
	    extra_options);
	if (strlen (trout_options) <= 3) strcpy (trout_options, "-R -d"); /* dummy */
     /* fprintf (stderr, "trout_options: '%s'\n", trout_options); */
    }
    else {
	if (verify_only == TRUE)
	    strcpy (trout_options, "-R -d"); /* verify only: dummy  */
	else { /* highlight net */
	    sprintf (mess_str, "Name of net to be highlighted: ");
	    if (ask_name (mess_str, net_name, TRUE) == -1) {
		ptext ("ERROR: You must enter a valid net name");
		set_titlebar ("");
		return;
	    }
	    sprintf (trout_options, "-R -H%s -r -V", net_name);
	}
    }

    start_tick = times (&start_times);

    /* write the cell
    */
    if (wrte_fish_cell () == -1) return;

    if (verify_only == FALSE) {
	sprintf (mess_str, "Routing circuit '%s', please relax and wait", circuit_name);
	set_titlebar (mess_str);
	ptext ("---- busy prepairing your order -----");
    }
    else if (verify_only == TRUE) {
	sprintf (mess_str, "Checking the nets in circuit '%s', please relax and wait", circuit_name);
	set_titlebar (mess_str);
	ptext ("---- busy verifying -----");
    }
    else { /* highlight net */
	sprintf (mess_str, "Tracing net '%s' in circuit '%s', please relax and wait", net_name, circuit_name);
	set_titlebar (mess_str);
	ptext ("---- busy highlighting net -----");
    }

    /* make seadif directory, in which we put the outputfile
    */
    if (make_seadif ()) return;

    /* OK, run the stuff (this is REALLY heavy!)
    */
    if (verify_only == FALSE)
	seaopt = "-r";  /* route and check */
    else
	seaopt = "-rv"; /* check only */

    if (box_set == TRUE) {
	sprintf (b1, "-x %ld", (long) xrc/4);
	sprintf (b2, "-X %ld", (long) xlc/4);
	sprintf (b3, "-y %ld", (long) ytc/4);
	sprintf (b4, "-Y %ld", (long) ybc/4);
    }
    else {
	strcpy (b1, "-d"); strcpy (b2, "-d");
	strcpy (b3, "-d"); strcpy (b4, "-d");
    }

    if (runprogsea ("sea", sealogfile, &exitstatus, seaopt, "-c", circuit_name,
		b1, b2, b3, b4, trout_options, NelsisTempCell, NULL)) {
	ptext ("ERROR: during calling routing program 'sea'");
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    /* keep the folks busy during the time that trout is busy
    */
    if (verify_only == FALSE)
	exitstatus = in_flight_entertainment (trout_kill);
    else
	exitstatus = in_flight_entertainment (verify_kill);

    /* check exitstatus
    */
    switch (exitstatus) {
    case 0: /* OK */
	break;
    case SDFERROR_WARNINGS:
	/* have a look at the warnings */
	xfilealert (2, sealogfile);
	break;
    case SDFERROR_FILELOCK:
	ptext ("The seadif database is locked, please wait. Another trout/madonna/ghoti/nelsea is running in this project.");
	lock_alert ("trout");
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
#ifdef SIGKILL
    case SIGKILL: /* the user killed the process */
	if (verify_only == FALSE)
	    ptext ("##### No routing: you killed trout #####");
	else
	    ptext ("##### You aborted verify: no results #####");
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
#endif
    case SDFERROR_CALL:
	ptext ("Internal error: wrong parameters");
	sleep (1);
    default:
	ptext ("#%$@# Router failed somehow");
	xfilealert (0, sealogfile);
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    dirty = TRUE;

    /* read again
    */
    empty_mem2 ();          /* clear memory */

    if (!(newcellkey = dmCheckOut (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, READONLY))) {
	ptext ("ERROR: re-read of routed cell failed. Your cell is lost! (sorry)");
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    if (inp_boxnorfl (newcellkey, quad_root) == -1 ||
	   inp_mcfl (newcellkey, &inst_root) == -1 || inp_term (newcellkey, term_root) == -1) {
	dmCheckIn (newcellkey, QUIT);
	empty_mem (); /* also performs an init_mem () */
	ptext ("ERROR: Can't read cell 'Tmp_Cell_' properly! Your cell is lost! (sorry)");
	sleep (1);
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	set_titlebar (NULL);
	return;
    }

    if (inp_comment (newcellkey) == -1) {
	char err_str[132];
	sprintf (err_str, "Warning: can't read comments of cell '%s'", NelsisTempCell);
	ptext (err_str);
	sleep (1);
    }

    dmCheckIn (newcellkey, QUIT);

    prev_w ();

    picture ();
    expansion (old_exp_level);
    if (Sub_terms == TRUE) all_term (); /* draw sub terminals */
    set_titlebar (NULL);

    /* remove cell
    */
    ptext ("---- Removing temporary cell ----");
    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);

    /* time statistics
    */
    curr_tick = times (&end_times);

    /* what was the status of the routing??
    */
    if (access (trouterrfile, R_OK) == 0) {
	xfilealert (0, trouterrfile);
	if (verify_only == FALSE) {
	    sprintf (trout_options, "*** WARNING: incomplete routing ***  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
	else if (verify_only == TRUE) {
	    sprintf (trout_options, "*** WARNING: Unconnects/short-circuits were found ***  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
	else {
	    sprintf (trout_options, "*** WARNING: net '%s' not found ***  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		net_name,
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
    }
    else {
	if (verify_only == FALSE) {
	    sprintf (trout_options, "--> Ready: routing was 100%% successful <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
	else if (verify_only == TRUE) {
	    sprintf (trout_options, "--> Ready: no unconnects nor short-circuits found <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
	else {
	    sprintf (trout_options, "--> Ready highlighting net '%s' <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		net_name,
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
    }
    ptext (trout_options);
    sleep (1);
}

/*
 * Go auto placing the current cell
 */
void do_madonna (int route_also) /* TRUE to route immediately after placement */
{
    int exist, box_set, x_expand, make_channels, ny, exitstatus;
    char mess_str[100], hstr[100], box_opt1[30], box_opt2[30], extra_options[100], circuit_name[100];
    char madonna_options[200], madonna_expand[20], madonna_extra[200];
    DM_CELL *newcellkey;
    clock_t start_tick, curr_tick;
    struct tms start_times, end_times;

    make_temp_cell_name ();

    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
	if (exist == 1) { /* remove it, must be junk... */
	    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
		sprintf (madonna_options, "ERROR: scratch cell '%s' exists and I cannot remove it!", NelsisTempCell);
		ptext (madonna_options);
		sleep (1);
		return;
	    }
	}
    }

    if (route_also == FALSE)
	set_titlebar ("Madonna welcomes you to play with her auto-placement");
    else
	set_titlebar ("Madonna/Trout welcomes you for automatic placement and routing");

    if (!no_works () && dirty == TRUE) { /* design is not empty: */
	if (route_also == FALSE)
	    ptext ("Sure to continue? Current cell was not saved, Madonna will overwrite your design");
	else
	    ptext ("Sure to continue? Madonna and trout will overwrite your design");
	if (ask (2, yes_no, -1) == 0) {
	    ptext ("OK, You asked for it, don't blame Madonna!");
	}
	else {
	    if (route_also == FALSE)
		ptext ("What a pity: Madonna misses you!");
	    else
		ptext ("Whatever you say, boss!");
	    set_titlebar ("");
	    return;
	}
    }

    if (!*cirname && cellstr) strcpy (circuit_name, cellstr);
    else if (*cirname) strcpy (circuit_name, cirname);
    else *circuit_name = 0;

    /* step 1: ask for the circuit name....
    */
    if (!*circuit_name)
	sprintf (mess_str, "Enter circuit: ");
    else
	sprintf (mess_str, "Enter circuit (default '%s'): ", circuit_name);

    strcpy (hstr, circuit_name);
    if (ask_name (mess_str, circuit_name, TRUE) == -1 || !*circuit_name) {
	if (*hstr) strcpy (circuit_name, hstr); /* use default */
	else {
	    ptext ("ERROR: You must give Madonna a valid circuit name");
	    set_titlebar (NULL);
	    return;
	}
    }

    /* check whether it exits...
    */
    if (_dmExistCell (dmproject, circuit_name, CIRCUIT) != 1) {
	sprintf (mess_str, "ERROR: circuit '%s' doesn't exist (Madonna dissapointed)", circuit_name);
	ptext (mess_str);
	set_titlebar (NULL);
	return;
    }

    /* store as default circuit name
    */
    strcpy (cirname, circuit_name);

    Sub_terms = FALSE;

    /* put menu...
    */
    cmd_nbr = -1;
    box_set = FALSE;
    x_expand = TRUE;
    make_channels = FALSE;
    *extra_options = 0;

    while (cmd_nbr != MADONNA_RUN) {
	ptext ("Click DO IT to call Madonna, or click commands to set your preferences.");

	cmd_nbr = 0;
	menu (MADONNA_CMD, madonna_cmd);
	if (x_expand == TRUE)
	    pre_cmd_proc (MADONNA_X, madonna_cmd);
	else
	    pre_cmd_proc (MADONNA_Y, madonna_cmd);
	if (route_also == TRUE) pre_cmd_proc (MADONNA_TROUT, madonna_cmd);
	if (make_channels == TRUE) pre_cmd_proc (MADONNA_CHAN, madonna_cmd);
	get_cmd ();
	switch (cmd_nbr) {
	case -2:
	    continue;
	case MADONNA_RUN:  /* do it */
	    pre_cmd_proc (MADONNA_RUN, madonna_cmd);
	    break;
	case MADONNA_BOX:  /* drag a box for size... */
	    pre_cmd_proc (MADONNA_BOX, madonna_cmd);
	    if (no_works ()) { /* if there is nothing: make empty image to give reference */
		do_fish ("");
		set_titlebar ("Madonna welcomes you to enjoy her auto-placement");
		picture ();
	    }

	    if (box_set == TRUE) { /* erase previous box.. */
		set_c_wdw (PICT);
		disp_mode (ERASE);
		ggSetColor (Gridnr);
		d_fillst (FILL_HASHED12B);
		paint_box ((float) xlc, (float) xrc, (float) ybc, (float) ytc);
		disp_mode (TRANSPARENT);
		set_c_wdw (MENU);
	    }

	    ptext ("Specify the bounding box for Madonna's placement");
	    ptext ("Specify the bounding box for Madonna's placement");
	    xlc = 0; ybc = 0;
	    xrc = xlc; ytc = ybc;
	    fix_loc ((float) xlc, (float) ybc);
	    if ((new_cmd = get_one (5, &xrc, &ytc)) != -1) break;
	    box_set = TRUE;

	    /* HACK: this is fishbone-specific!!!!
	    */
	    /* minimum size */
	    if (xrc < 720) xrc = 720;
	    if (ytc < 3925) ytc = 3925;
	    /* round vertical to nearest */
	    ny = (ytc + 2044) / 3925; ytc = (ny * 3925) + 14;
	    /*      printf ("Crd3 = %ld, %ld, %ld, %ld\n", xlc, xrc, ybc, ytc); */
	    set_c_wdw (PICT);
	    ggSetColor (Gridnr);
	    d_fillst (FILL_HASHED12B);
	    disp_mode (TRANSPARENT);
	    paint_box ((float) xlc, (float) xrc, (float) ybc, (float) ytc);
	    disp_mode (TRANSPARENT);
	    set_c_wdw (MENU);
	    break;
	case MADONNA_Y:
	    x_expand = FALSE;
	    ptext ("Madonna expands the box in the vertical direction, if necessary.");
	    post_cmd_proc (MADONNA_X, madonna_cmd);
	    pre_cmd_proc (MADONNA_Y, madonna_cmd);
	    sleep (1);
	    break;
	case MADONNA_X:
	    x_expand = TRUE;
	    ptext ("Madonna expands the box in the horizontal direction, if necessary.");
	    post_cmd_proc (MADONNA_Y, madonna_cmd);
	    pre_cmd_proc (MADONNA_X, madonna_cmd);
	    sleep (1);
	    break;
	case MADONNA_CHAN:
	    if (make_channels == TRUE) {
		make_channels = FALSE;
		post_cmd_proc (MADONNA_CHAN, madonna_cmd);
		ptext ("Madonna performs default channelless placement.");
	    }
	    else {
		make_channels = TRUE;
		pre_cmd_proc (MADONNA_CHAN, madonna_cmd);
		ptext ("Madonna makes channels for better routability.");
	    }
	    sleep (1);
	    break;
	case MADONNA_TROUT:
	    if (route_also == TRUE) {
		route_also = FALSE;
		post_cmd_proc (MADONNA_TROUT, madonna_cmd);
		ptext ("Madonna performs placement only.");
	    }
	    else {
		route_also = TRUE;
		pre_cmd_proc (MADONNA_TROUT, madonna_cmd);
		ptext ("Place & Route: Madonna calls trout after she's finished.");
	    }
	    sleep (1);
	    break;
	case MADONNA_OPT:
	    sprintf (mess_str, "Enter additional options for Madonna: ");
	    if (ask_name (mess_str, extra_options, FALSE) == -1 || !*extra_options)
		ptext ("No extra options set for Madonna. Her usual number is OK for you..");
	    else
		ptext ("Extra Madonna options were set.....");
	    sleep (2);
	    break;
	case MADONNA_HELP: /* Help... */
	    pre_cmd_proc (cmd_nbr, madonna_cmd);
	    ptext ("Starting help viewer, one moment please...");
	    start_browser ("help");
	    sleep (2);
	    break;
	case MADONNA_CANCEL:
	    ptext ("What a pity: Madonna misses you!");
	    set_titlebar ("");
	    if (box_set == TRUE) { /* erase previous box.. */
		set_c_wdw (PICT);
		disp_mode (ERASE);
		ggSetColor (Gridnr);
		d_fillst (FILL_HASHED12B);
		paint_box ((float) xlc, (float) xrc, (float) ybc, (float) ytc);
		disp_mode (TRANSPARENT);
		set_c_wdw (MENU);
	    }
	    return;
	case MADONNA_BBX:
	    pre_cmd_proc (cmd_nbr, madonna_cmd);
	    bound_w ();
	    box_set = FALSE;
	    break;
	case MADONNA_CENTER:
	    pre_cmd_proc (cmd_nbr, madonna_cmd);
	    if ((new_cmd = set_tbltcur (1, NO_SNAP)) == -1) {
		center_w (xlc, ybc);
		new_cmd = cmd_nbr;
	    }
	    box_set = FALSE;
	    break;
	case MADONNA_ZOOM:
	    pre_cmd_proc (cmd_nbr, madonna_cmd);
	    if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		curs_w (xlc, xrc, ybc, ytc);
		new_cmd = cmd_nbr;
	    }
	    box_set = FALSE;
	    break;
	case MADONNA_DEZOOM:
	    pre_cmd_proc (cmd_nbr, madonna_cmd);
	    if ((new_cmd = set_tbltcur (2, NO_SNAP)) == -1) {
		de_zoom (xlc, xrc, ybc, ytc);
		new_cmd = cmd_nbr;
	    }
	    box_set = FALSE;
	    break;
	default:
	    ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, madonna_cmd);
    }

    start_tick = times (&start_times);

    /* set the madonna options accordingly
    */
    /* box coordinates */
    xrc = xrc / 4;
    ytc = ytc / 4;
    if (box_set == TRUE) {
	sprintf (box_opt1, "-x %ld", xrc);
	sprintf (box_opt2, "-y %ld", ytc);
    }
    else {
	strcpy (box_opt1, "-d");  /* dummy, will be overwritten anyway.. */
	strcpy (box_opt2, "-d");  /* dummy, will be overwritten anyway.. */
    }

    /* set entire options string: magnification = 0.75 */
    sprintf (madonna_expand, "-P -e%c",
    x_expand == TRUE ? 'x' : 'y');
    if (*extra_options) sprintf (madonna_extra, "-P %s", extra_options);
    else {
	if (make_channels == TRUE)
	    strcpy (madonna_extra, "-P -CRseadif/groutes");
	else
	    strcpy (madonna_extra, "-d");
    }

    if (route_also == FALSE) {
	sprintf (mess_str, "PLACING circuit '%s', please relax and wait", circuit_name);
	set_titlebar (mess_str);
	ptext ("--- Madonna is busy. You relax while she sings quietly ---");
    }
    else {
	sprintf (mess_str, "PLACING and ROUTING circuit '%s', please relax and wait", circuit_name);
	set_titlebar (mess_str);
	ptext ("--- Madonna and Trout are busy trying to satisfy you ---");
    }

    /* make seadif directory, in which we put the outputfile
    */
    if (make_seadif ()) { set_titlebar (NULL); return; }

    /* OK, run the stuff (this is REALLY heavy!)
    */
    if (runprogsea ("sea", sealogfile, &exitstatus,
		route_also == TRUE ? "-d" : "-p",   /* placement/ P&R */
		box_opt1,                           /* specifies the box... */
		box_opt2, madonna_expand,
		madonna_extra,                      /* madonna options */
		"-c", circuit_name,                 /* the ciruit name... */
		NelsisTempCell,                     /* layout name */
		NULL)) {
	ptext ("ERROR: during calling madonna via program 'sea'");
	cellstr = NULL;
	set_titlebar (NULL);
	if (!no_works ()) {
	    eras_worksp ();
	    initwindow ();
	    picture ();
	    dirty = FALSE;
	}
	return;
    }

    /* keep the folks busy during the time that madonna is busy
    */
    exitstatus = in_flight_entertainment (madonna_kill);

    /* check exitstatus
    */
    switch (exitstatus) {
    case 0: /* OK */
	break;
    case SDFERROR_FILELOCK:
	ptext ("Madonna has her period: The seadif database is locked by another program, please wait.");
	lock_alert ("Madonna");
	set_titlebar (NULL);
	return;
#ifdef SIGKILL
    case SIGKILL: /* the user killed the process */
	ptext ("##### No placement, no music, 'cuz you killed Madonna #####");
	set_titlebar (NULL);
	return;
#endif
    case SDFERROR_CALL:
	ptext ("Internal error: wrong parameters");
	sleep (1);
    default:
	xfilealert (0, sealogfile);
	ptext ("#%$@# Madonna failed to satisfy you");
	cellstr = NULL;
	set_titlebar (NULL);
	return;
    }

    /* erase the design (if required..)
    */
    if (!no_works ()) {
	eras_worksp ();
	initwindow ();
	picture ();
    }

    dirty = TRUE;

    /* read it
    */
    if (!(newcellkey = dmCheckOut (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, READONLY))) {
	ptext ("ERROR: re-read of routed cell failed. No placement! (sorry)");
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	cellstr = NULL;
	set_titlebar (NULL);
	return;
    }

    if (inp_boxnorfl (newcellkey, quad_root) == -1 ||
	   inp_mcfl (newcellkey, &inst_root) == -1 || inp_term (newcellkey, term_root) == -1) {
	dmCheckIn (newcellkey, QUIT);
	empty_mem (); /* also performs an init_mem () */
	ptext ("ERROR: Can't read cell the temp call properly! No placement! (sorry)");
	sleep (1);
	eras_worksp ();
	dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	cellstr = NULL;
	set_titlebar (NULL);
	return;
    }

    if (inp_comment (newcellkey) == -1) {
	char err_str[132];
	sprintf (err_str, "Warning: can't read comments of cell '%s'", NelsisTempCell);
	ptext (err_str);
	sleep (1);
    }

    dmCheckIn (newcellkey, QUIT);

    /* remove cell
    */
    ptext ("---- Removing temporary cell ----");
    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);

    cellstr = strsave (circuit_name); /* give cell this name */
    set_titlebar (NULL);
    bound_w ();
    save_oldw ();
    inform_cell ();
    if (Default_expansion_level != 1) {
	picture ();
	expansion (Default_expansion_level);
	if (Sub_terms == TRUE) all_term (); /* draw sub terminals */
    }

    /* time statistics
    */
    curr_tick = times (&end_times);
    if (route_also == FALSE) {
	sprintf (madonna_options, "--> Ready: Madonna hopes you like her <--  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
    }
    else { /* routing also: what was the status?? */
	if (access (trouterrfile, R_OK) == 0) {
	    xfilealert (0, trouterrfile);
	    sprintf (madonna_options, "*** WARNING: incomplete routing ***  cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
	else {
	    sprintf (madonna_options, "--> Ready: successful Placement & Routing <-- cpu: %3.2f  elapsed: %3.2f (%4.2f %% of cpu)",
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/Clk_tck),
		(float)(((float) curr_tick - start_tick)/Clk_tck),
		(float)(((float) end_times.tms_utime - start_times.tms_utime +
		end_times.tms_stime - start_times.tms_stime +
		end_times.tms_cutime - start_times.tms_cutime +
		end_times.tms_cstime - start_times.tms_cstime)/
		((float) curr_tick - start_tick)) * 100);
	}
    }
    ptext (madonna_options);
}

/*
 * The following routine pops up an xterm window in which fname is displayed
 */
void xfilealert (int type, char *fname)
{
    char *fgcolor, *bgcolor, hstr[100];
    struct stat buf;
    int columns, lines, exitstatus, i;
    FILE *fp;
    char geo[100], scanline[2000];

    if (type == 2) { /* warning */
	bgcolor = "white";
	fgcolor = "black";
    }
    else { /* error */
	bgcolor = "black";
	fgcolor = "red";
    }

    if (access (fname, R_OK) != 0) {
	fprintf (stderr, "WARNING (xfilealert): file '%s' cannot be accessed.\n", fname);
	return;
    }

    if (stat (fname, &buf) == 0) {
	if (buf.st_size == 0) {
	    fprintf (stderr, "WARNING (xfilealert): file '%s' has zero size.\n", fname);
	    return;
	}
    }
    else fprintf (stderr, "WARNING (xfilealert): cannot stat file '%s'.\n", fname);

    /* lock output
    */
    do_echo ("lock", fname);

    /* find out the nnumber of lines and columns in the file
    */
    if (!(fp = fopen (fname, "r"))) {
	fprintf (stderr, "WARNING (xfilealert): file '%s' cannot be opened.\n", fname);
    }

    for (lines = 2, columns = 60; fp && fgets (scanline, 1998, fp); lines++) {
	if ((i = strlen (scanline)) > columns) columns = i > 150 ? 150 : i;
    }
    if (lines < 5) lines = 5;
    else if (lines > 70) lines = 70;

    fclose (fp);

    sprintf (geo, "%dx%d", columns, lines);

    if (type == 2) /* warning */
	sprintf (hstr, "Warning message file '%s'    (hit 'q' to erase this window)", fname);
    else /* error */
	sprintf (hstr, "Error message file '%s'    (hit 'q' to erase this window)", fname);

    if (runprognowait ("xterm", tmplogfile, &exitstatus,
		"-title", hstr,
		"-n", "alert",
		"-sl", "500",        /* 500 savelines */
		"-sb",               /* scrollbar */
		"-ut",               /* no utmp */
	//	"-fn", "9x15",       /* font */
		"-bg", bgcolor,
		"-fg", fgcolor,
		"-geometry", geo,
		"-e", "seatail", fname, NULL)) {
	fprintf (stderr, "WARNING: xterm alert failed for file '%s'\n", fname);
    }
}

/*
 * put special options menu for trout
 */
static void trout_options_menu (char *trout_options)
{
    char extra_options[250];
    int no_verify, no_routing, dangling_tr, substrate_c, fat_power,
    make_caps, change_order, use_border, overlap_wires, flood_mesh,
    use_exact_chip_area;

    no_verify = FALSE;
    no_routing = FALSE;
    dangling_tr = FALSE;
    substrate_c = FALSE;
    fat_power = FALSE;
    make_caps = FALSE;
    change_order = FALSE;
    use_border = FALSE;
    overlap_wires = FALSE;
    flood_mesh = FALSE;
    use_exact_chip_area = FALSE;
    *extra_options = 0;

    set_titlebar (")>)> trout )>)>    Set special routing options");
    ptext ("Click commands to set preferences.");

    cmd_nbr = -1;
    while (cmd_nbr != TROUT_RET) {
	cmd_nbr = 0;
	menu (TROUT_OPT, trout_opt);

	/* set bulbs */
	if (no_routing == TRUE)
	    pre_cmd_proc (1, trout_opt);
	else
	    post_cmd_proc (1, trout_opt);
	if (use_exact_chip_area == TRUE)
	    pre_cmd_proc (2, trout_opt);
	else
	    post_cmd_proc (2, trout_opt);
	if (dangling_tr == TRUE)
	    pre_cmd_proc (3, trout_opt);
	else
	    post_cmd_proc (3, trout_opt);
	if (substrate_c == TRUE)
	    pre_cmd_proc (4, trout_opt);
	else
	    post_cmd_proc (4, trout_opt);
	if (fat_power == TRUE)
	    pre_cmd_proc (5, trout_opt);
	else
	    post_cmd_proc (5, trout_opt);
	if (make_caps == TRUE)
	    pre_cmd_proc (6, trout_opt);
	else
	    post_cmd_proc (6, trout_opt);
	/*   if (change_order == TRUE)
	    pre_cmd_proc (6, trout_opt);
	else
	    post_cmd_proc (6, trout_opt);  */
	if (use_border == TRUE)
	    pre_cmd_proc (7, trout_opt);
	else
	    post_cmd_proc (7, trout_opt);
	if (overlap_wires == TRUE)
	    pre_cmd_proc (8, trout_opt);
	else
	    post_cmd_proc (8, trout_opt);
	if (flood_mesh == TRUE)
	    pre_cmd_proc (9, trout_opt);
	else
	    post_cmd_proc (9, trout_opt);
	if (no_verify == TRUE)
	    pre_cmd_proc (10, trout_opt);
	else
	    post_cmd_proc (10, trout_opt);

	get_cmd ();
	switch (cmd_nbr) {
	case -2:
	    continue;
	case TROUT_RET:
	    pre_cmd_proc (TROUT_RUN, trout_cmd);
	    break;
	case 1: /* no routing */
	    if (no_routing == TRUE) {
		ptext ("All nets will be routed normally.");
		no_routing = FALSE;
	    }
	    else {
		ptext ("No routing of signal nets (only add special features).");
		no_routing = TRUE;
	    }
	    break;
	case 2: /* use exact chip area */
	    if (use_exact_chip_area == TRUE) {
		ptext ("Do not use exact chip area for routing.");
		use_exact_chip_area = FALSE;
	    }
	    else {
		ptext ("Use exact chip area for routing.");
		use_exact_chip_area = TRUE;
	    }
	    break;
	case 3: /* connect dangling */
	    if (dangling_tr == TRUE) {
		ptext ("All unused transitors will remain unconnected.");
		dangling_tr = FALSE;
	    }
	    else {
		ptext ("All unused transistors will be connected to the power net.");
		dangling_tr = TRUE;
	    }
	    break;
	case 4: /* substrate_c */
	    if (substrate_c == TRUE) {
		ptext ("No substrate contacts will be added.");
		substrate_c = FALSE;
	    }
	    else {
		ptext ("Substrate contacts will be added under the power lines.");
		substrate_c = TRUE;
	    }
	    break;
	case 5: /* fat_power */
	    if (fat_power == TRUE) {
		ptext ("Using normal power lines.");
		fat_power = FALSE;
	    }
	    else {
		ptext ("Making power lines as wide as possible.");
		fat_power = TRUE;
		flood_mesh = dangling_tr = TRUE;
	    }
	    break;
	case 6: /* make capacitors */
	    if (make_caps == TRUE) {
		ptext (" ");
		make_caps = FALSE;
	    }
	    else {
		ptext ("Converting all unused transistors into capacitors.");
		make_caps = fat_power = dangling_tr = substrate_c = TRUE;
		flood_mesh = TRUE;
	    }
	    break;
    /*	case 7:
	    if (change_order == TRUE) {
		ptext ("Default net routing order will be used.");
		change_order = FALSE;
	    }
	    else {
		ptext ("A different net-oriented routing order will be used.");
		change_order = TRUE;
	    }
	    break; */
	case 7: /* use borders */
	    if (use_border == TRUE) {
		ptext ("Not using the upper and lower power rails for routing.");
		use_border = FALSE;
	    }
	    else {
		ptext ("Using the entire routing area, including the upper and lower power rails.");
		use_border = TRUE;
	    }
	    break;
	case 8: /* overlap wires */
	    if (overlap_wires == TRUE) {
		ptext ("Default wires.");
		overlap_wires = FALSE;
	    }
	    else {
		ptext ("Overlapping metal1-metal2 wires segments will be made fatter.");
		overlap_wires = TRUE;
	    }
	    break;
	case 9: /* flood_mesh */
	    if (flood_mesh == TRUE) {
		ptext ("Default (non-hierarchical) treatment of mesh.");
		flood_mesh = FALSE;
	    }
	    else {
		ptext ("All fat wires will be filled, to prevent a mesh (hierarchical).");
		flood_mesh = TRUE;
	    }
	    break;
	case 10: /* no_verify */
	    if (no_verify == TRUE) {
		ptext ("Verifying layout after routing (=automatic check nets).");
		no_verify = FALSE;
	    }
	    else {
		ptext ("No verification of layout after routing (this saves time and memory).");
		no_verify = TRUE;
	    }
	    break;
	case 11:  /* additional options */
	    sprintf (trout_options, "Enter additional options for trout: ");
	    if (ask_name (trout_options, extra_options, FALSE) == -1 || !*extra_options)
		ptext ("No extra options entered");
	    else
		ptext ("Extra routing options were set.....");
	    break;
	default:
	    ptext ("Command not implemented!");
	}
	picture ();
	post_cmd_proc (cmd_nbr, trout_opt);
    }

    sprintf (trout_options, "%s%s%s%s%s%s%s%s%s%s%s%s%c",
	no_routing == TRUE   ? " -r" : "",
	dangling_tr == TRUE  ? " -t" : "",
	substrate_c == TRUE  ? " -S" : "",
	fat_power == TRUE    ? " -P" : "",
	make_caps == TRUE    ? " -C" : "",
	/*	change_order == TRUE ? " -n" : "", */
	use_border == TRUE   ? " -a -B" : "",
	overlap_wires == TRUE ? " -O" : "",
	flood_mesh == TRUE   ? " -F" : "",
	no_verify == TRUE    ? " -V" : "",
	use_exact_chip_area == TRUE    ? " -E" : "",
	*extra_options ? " " : "",
	*extra_options ? extra_options : "", '\0');
}

/*
 * start browser with page..
 */
void start_browser (char *subject) /* the subject to show... */
{
    static char *browser = NULL;
    char *oceanhome, *sdfoceanpath (), picpath[360];
    int exitstatus;

    /* find out the hyperlink for the subject */
    if (!(oceanhome = sdfoceanpath ())) return;

    /* make path to html */
    sprintf (picpath, "%s/%s/%s.html", oceanhome, SEADALI_DIR, subject);

    if (access (picpath, R_OK)) {
	fprintf (stderr, "ERROR: cannot access html help page '%s'\n", picpath);
	ptext ("Sorry, cannot access html help pages");
	return;
    }

    if (!browser) {
	browser = getenv ("BROWSER");
	if (!browser) browser = "mozilla";
    }
    /* starting browser ... */
    runprognowait (browser, tmplogfile, &exitstatus, picpath, NULL);
}

static void lock_alert (char *progname)
{
    FILE *fp;
    char *alertfile = "seadif/seadali.error";

    /* remove previous message
    */
    unlink (alertfile);
    if (!(fp = fopen (alertfile, "w"))) return;

    fprintf (fp, "  Dear user,\n\n");
    fprintf (fp, "You just asked %s to work for you, but she just couldn't do it now.\n", progname);
    fprintf (fp, "The seadif database appears to be locked because somebody else is\n");
    fprintf (fp, "running a madonna, ghoti, trout or nelsea at this very moment.\n");
    fprintf (fp, "Please wait for a little while and try it again. In the mean time you\n");
    fprintf (fp, "can take a cup of coffee or you try to find out who this nasty guy is.\n");
    fprintf (fp, "Have wonderful day,\n\n");
    fprintf (fp, "                                    Seadali\n\n");
    fprintf (fp, "P.S.\n");
    fprintf (fp, "In some very rare cases (such after as a system crash), a 'ghost' lock\n");
    fprintf (fp, "file could be present. If you are _absolutely_ sure that nobody else is\n");
    fprintf (fp, "running a Madonna, trout, nelsea or ghoti in this project,\n");
    fprintf (fp, "but you keep on getting this irritant message you can blast away the lockfile\n");
    fprintf (fp, "by typing 'rmsdflock' in your project\n");
    fprintf (fp, "terminate\n");

    fclose (fp);
    xfilealert (2, alertfile);
}

/*
 * in-flight entertainment: show screen with stop-possibility
 */
static int in_flight_entertainment (char **prog)
{
    int exitstatus, already_stopped;

    already_stopped = FALSE;
    cmd_nbr = 0;

    in_flight_flag = 1;
    while (cmd_nbr != KILL_CHILD) {
	cmd_nbr = 0;
	/*
	* put menu
	*/
	menu (KILL_MENU, prog);
	get_cmd ();
	if (looks_like_the_sea_child_died (TRUE) == TRUE) break;

	switch (cmd_nbr) {
	case KILL_CHILD:
	    /* kill trout/madonna */
	    pre_cmd_proc (KILL_CHILD, prog);
	    ptext ("#$@! killing child process #$@!");
	    kill_the_sea_child ();
	    break;
	case KILL_STOP: /* stop child genlty */
	    if (already_stopped == TRUE) {
		ptext ("Hey! Be patient! Already sent stop request!");
		break;
	    }
	    pre_cmd_proc (cmd_nbr, prog);
	    if (prog != trout_kill && prog != madonna_kill) break;
	    if (signal_trout_to_stop () != TRUE)
		ptext ("Unable to contact child program, sorry");
	    else {
		if (prog == trout_kill)
		    ptext ("Sent STOP request to trout, just wait and see");
		else
		    ptext ("Kindly asked Madonna to hurry up, just wait and see");
	    }
	    already_stopped = TRUE;
	    break;
	}
    }
    in_flight_flag = 0;

    /* at this point the kid died! */

    /* perform the autopsy to get the exitstatus
    */
    exitstatus = autopsy_on_sea_child ();
 /* fprintf (stderr, "Exitstatus = %s\n", exitstatus); */
    return (exitstatus);
}

/*
 * Print the current layout
 */
void do_print ()
{
    char *s, msg[512];
    int c, exist, exitstatus;
    FILE *fp;
    char *prlogfile = "seadif/print.out";
    char *prcmdfile = "seadif/print.command";

    /* we print the entire cell, so let's show 'm what they get
    */
    bound_w ();
    picture ();

    ptext ("Printing the full-size cell, OK to continue?");
    if (ask (2, yes_no, -1) != 0) {
	ptext ("No hard feelings.....");
	return;
    }

    set_titlebar ("Busy printing, one moment please!");

    make_temp_cell_name (); /* manage temporary cell name */

    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT))) {
	if (exist == 1) { /* remove it, must be junk... */
	    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);
	    if ((exist = _dmExistCell (dmproject, NelsisTempCell, LAYOUT)) != 0) {
		sprintf (msg, "ERROR: scratch cell '%s' exists and I cannot remove it!", NelsisTempCell);
		ptext (msg);
		sleep (1);
		return;
	    }
	}
    }

    /* write the cell
    */
    if (wrte_fish_cell () == -1) return;

    /* make seadif directory, in which we put the outputfile
    */
    if (make_seadif ()) return;

    if (!Print_command) /* command not set, use default one */
	Print_command = strsave ("echo Layout to Postscript printer built-in command sequence ; playout %s");

    /* make the command file which will be executed
    */
    if (!(fp = fopen (prcmdfile, "w"))) {
	sprintf (msg, "cannot open %s", prcmdfile);
	ptext (msg);
	return;
    }

    /* format print command (fill in cell name)
    */
    s = Print_command;
    while ((c = *s++)) {
	if (c == '%' && *s == 's') { fprintf (fp, "%s", NelsisTempCell); ++s; }
	else fputc (c, fp);
    }
    fputc ('\n', fp);
    fclose (fp);

    ptext ("---- Busy making and printing this layout ----");

    /* run it in background
    */
    if (runprogsea ("sh", prlogfile, &exitstatus, prcmdfile, NelsisTempCell, NULL)) {
	fprintf (stderr, "cannot run print command\n");
	exitstatus = 1;
    }
    else { /* busy menu */
	exitstatus = in_flight_entertainment (print_kill);
    }

    if (runprognowait ("xterm", tmplogfile, &exitstatus,
	"-title", "Output of print command  (hit 'q' to erase window)",
	"-n", "print",
	"-sl", "500",        /* 500 savelines */
	"-sb",               /* scrollbar */
	"-ut",               /* no utmp */
	"-fn", "8x16",       /* font */
	"-bg", "black",
	"-fg", "white",
	"-geometry", "70x18-3+1",
	"-e", "seatail", prlogfile, NULL))
	    fprintf (stderr, "xterm for print output is not working fine\n");

    ptext ("---- Removing temporary cell ----");
    dmRmCellForce (dmproject, NelsisTempCell, WORKING, DONTCARE, LAYOUT, TRUE);

    do_echo ("terminate", prlogfile);

    if (exitstatus == 0)
	ptext ("");
    else
	ptext ("Print command not successful");
    set_titlebar (NULL);
}
