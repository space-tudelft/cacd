%{
/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	T.G.R. van Leuken
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

#include "src/cldm/extern.h"

extern char userChar[];
char AltCellname[1024];
char Termname[1024];
int  box_xl, box_xr, box_yb, box_yt;
int  Box_x, Box_y, Box_dir;
int  Termx, Termy;
char Termlayer[32];
int  Points[1024];
int  PointsIndex;
int  i;
int  mx,my;
int  rx,ry;
int  CurrentTransform[3][2];
double resolution;
double xval;
int  doUserStartFlag = 0;
int  instnamed = 0;
int  termcount; /* counter to avoid duplicate terminals */

static int dolayer (char *s);
char *find_alias (char *name);
extern int try_input ();
%}

%token INTEGER
%token SEMI
%token BOX POLYGON ROUNDFLASH WIRE
%token LAYER
%token START FINISH
%token DELETE
%token CALL
%token END
%token MX MY ROTATE TRANS
%token SHORTNAME
%token USER

%%
program		: stats END
		;
stats		: /* E */ | stats stat
		;
stat		: wire    semi
		| start   semi
		| finish  semi
		| polygon semi
		| box     semi
		| round   semi
		| delete  semi
		| user    semi
		| layer   semi
		| call    semi
		| error   semi
		{ yyerrok; }
		| semi
		;
start		: START cellid
		{
			doStart($2,1,1);
		}
		| START cellid scale scale
		{
			doStart($2,$3,$4);
		}
		;
finish		: FINISH
		{
		    strcpy (AltCellname, "");
		    if (err_flag) {
			pr_exit (0204, 26, ms_name);
		    }

		    if (mod_key) {

			if (err_flag) {
			    append_tree (ms_name, &mod_tree);
			    tree_ptr -> errflag = 1;
			    close_files ();
			    dmCheckIn (mod_key, QUIT);
			}
			else {
			    write_info ();
			    close_files ();
			    dmCheckIn (mod_key, COMPLETE);
			}

			mod_key = NULL;
		        ini_mcbbox = ini_bbbox = 0;
		    }
		    rm_tree (tnam_tree);
		    rm_tree (inst_tree);
		    tnam_tree = inst_tree = NULL;
		}
		;
layer		: LAYER SHORTNAME
		{
			lay_code = dolayer (textval());
		}
		;
wire		: WIRE integer points
		{
		    if (lay_code == -1)
			pr_exit (034, 0, "error: layer unspecified");

		    w_width = $2;
		    w_x = 2 * Points[0];
		    w_y = 2 * Points[1];
		    for(int_ind = 2; int_ind <PointsIndex; int_ind++)
		       int_val[int_ind-2] = 2 * (Points[int_ind] - Points[int_ind-2]);
		    int_ind -= 2;
		    if (!err_flag && !s_mode) proc_swire ();
		}
		;
polygon		: POLYGON points
		{
		    if (lay_code == -1)
			pr_exit (034, 0, "error: layer unspecified");

		    for(int_ind = 0 ; int_ind <PointsIndex;int_ind++)
		       int_val[int_ind] = 4 * Points[int_ind];
		    int_val[int_ind++] = 4 * Points[0];
		    int_val[int_ind++] = 4 * Points[1];

		    if (int_ind >= NOINTS) pr_exit (0137, 8, 0);

		    if (!err_flag && !s_mode) proc_poly ();
		}
		;
call		: CALL cellid transforms
		{
			doCifCall ($2);
		}
		;
transforms	: /* E */ | transforms transform
		;
transform	: translate
		| mirrorx
		| mirrory
		| rotate
		;
translate	: TRANS integer integer
		{
			TTranslate ($2, $3);
		}
		;
mirrorx		: MX
		{
			TMX ();
		}
		;
mirrory		: MY
		{
			TMY ();
		}
		;
rotate		: ROTATE scale scale
		{
			TRotate ($2, $3);
		}
		;
points		: /* E */ | points point
		;
point		: integer integer
		{
			Points[PointsIndex++]=$1;
			Points[PointsIndex++]=$2;
		}
		;
box 		: BOX scale scale scale scale
		{
			doBox ($2,$3,$4,$5,1,0);
		}
		| BOX scale scale scale scale scale scale
		{
			doBox ($2,$3,$4,$5,$6,$7);
		}
		;
round 		: ROUNDFLASH integer integer integer
		{
		    if (lay_code == -1)
			pr_exit (034, 0, "error: layer unspecified");

		    if ($4 <= 0)
			pr_exit (0214, 48, fromitoa ($4));

		    if (!err_flag && !s_mode)
		        proc_circ ($2, $3, $4 / 2, 0, 0, 360000, 32);
		}
		;
delete 		: DELETE cellid
		{
			deletecell ($2);
		}
		;
user 		: USER
		{
		    if (yylval == 9 && userChar[0] == ' ') {
			if (sscanf (userChar+1, "%s", AltCellname) != 1)
				yyerror ("user argument missing");
if (v_mode) P_E "-- Cellname (9 %s)\n", AltCellname);
			doUserStart (AltCellname);
		    }
		    /* CONNECTORS */
		    else if (yylval == 4 && userChar[0] == 'X') { /* Alliance/COMPASS */
			int w; /* 4X <name> <index> <x> <y> <w> <signal> */
			if (sscanf (userChar+1, "%s%d%d%d%d%s",
			    Termname, &i, &Termx, &Termy, &w, Termlayer) != 6)
				yyerror ("user argument missing");
if (v_mode) P_E "-- Terminal (4X %s %d %d %d %d %s)\n", Termname, i, Termx, Termy, w, Termlayer);
			doUserTerm2 (w, w);
		    }
		    else if (yylval == 9 && userChar[0] == '5') { /* Berkley */
			int w, h; /* 95 <name> <w> <h> <x> <y> <layer> */
			int old_lc;
			if (sscanf (userChar+1, "%s%d%d%d%d%s",
			    Termname, &w, &h, &Termx, &Termy, Termlayer) != 6)
				yyerror ("user argument missing");
if (v_mode) P_E "-- Terminal (95 %s %d %d %d %d %s)\n", Termname, w, h, Termx, Termy, Termlayer);
			old_lc = lay_code;
			lay_code = dolayer (Termlayer);
			if (lay_code != old_lc)
			    P_E "%s: %d: warning: terminal layer not equal to current layer!\n",
				argv0, yylineno);
			doUserTerm2 (w, h);
			lay_code = old_lc;
		    }
		    /* SIGNALS */
		    else if (yylval == 4 && userChar[0] == 'N') { /* Alliance/COMPASS */
				/* 4N <name> <x> <y> */
			if (sscanf (userChar+1, "%s%d%d", Termname, &Termx, &Termy) != 3)
				yyerror ("user argument missing");
if (v_mode) P_E "-- Label (4N %s %d %d)\n", Termname, Termx, Termy);
			doUserLabel ();
		    }
		    else if (yylval == 9 && userChar[0] == '4') { /* Berkley */
				/* 94 <name> <x> <y> <layer> */
			int old_lc = lay_code;

			done_user_94 = 1;

			if (sscanf (userChar+1, "%s%d%d%s", Termname, &Termx, &Termy, Termlayer) != 4)
				yyerror ("user argument missing");
if (v_mode) P_E "-- %s (94 %s %d %d %s)\n", delft_old ? "Terminal" : "Label",
				Termname, Termx, Termy, Termlayer);
			lay_code = dolayer (Termlayer);
			if (lay_code != old_lc)
			    P_E "%s: %d: warning: terminal layer not equal to current layer!\n",
				argv0, yylineno);
			if (delft_old)
			    doUserTerm ();
			else
			    doUserLabel ();
			lay_code = old_lc;
		    }
		    /* INSTANCES */
		    else if ((yylval == 4 && userChar[0] == 'I')   /* Alliance: 4I <name> */
			  || (yylval == 9 && userChar[0] == '1')) { /* Berkley: 91 <name> */
			if (sscanf (userChar+1, "%s", instance) != 1)
				yyerror ("user argument missing");
			instnamed = 1;
		    }
		    else
			P_E "%s: %d: USER: NOT YET IMPLEMENTED (%d%s)\n",
			    argv0, yylineno, yylval, userChar);
		}
		;
cellid		: INTEGER
		{
		    $$ = yylval;
		}
		;
integer		: INTEGER
		{
		    xval = yylval * resolution;
		    $$ = i = (int)ROUND (xval);
		if (!r_mode)
		    if (((double)i - xval) > 0.0001 || ((double)i - xval) < -0.0001)
			P_E "%s: %d: warning: %g rounded to %d\n", argv0, yylineno, xval, i);
		}
		;
scale		: INTEGER
		{
		    $$ = yylval;
		}
		;
semi		: SEMI
		{
		    PointsIndex = 0;
		    tx = 0; ty = 0;
		    mx = 0; my = 0;
		    rx = 0; ry = 0;
		    dx = 0; nx = 0;
		    dy = 0; ny = 0;
		    TIdentity ();
		}
		;
%%

void yyerror (char *cs)
{
    char *s = textval();
    int   c = s[0];
    s[16] = '\0';
    if ((c >= '\0' && c <= ' ') || c > '\176') {
	switch (c) {
	    case '\n':
		sprintf (s, "eol");
		break;
	    case '\0':
		sprintf (s, "eof");
		break;
	    default:
		sprintf (s, "\\%03o", c);
	}
    }
    pr_exit (074, 0, cs);
}

void doStart (int s, int a, int b)
{
    int tmp;

    lay_code = -1; /* set undefined */

    resolution = (double) a / ((double) b * cifsf);
    tmp = 100000 * resolution + 0.1;
    resolution = (double) tmp / 100000;
    if (v_mode)
	P_E "-- Definition Start #%d (resolution = %g)\n", s, resolution);
    if (mod_key) {
	close_files ();
	dmCheckIn (mod_key, QUIT);
	mod_key = NULL;
    }

    err_flag = 0;		/* no errors */

    sprintf (ms_name, "symbol%d", s);

    if (!s_mode) {
	if (check_tree (ms_name, mod_tree)) {
	    if (f_mode && !tree_ptr -> bbox) {
		pr_exit (0604, 42, ms_name);
	    }
	    else { /* already defined or used */
		tree_ptr -> errflag = 1;
		if (!f_mode)
		    pr_exit (0214, 9, ms_name);
		else
		    pr_exit (0214, 37, ms_name);
	    }
	}
	if (!err_flag) {
	    mod_key = dmCheckOut (dmproject, ms_name,
		    WORKING, DONTCARE, LAYOUT, CREATE);
	    open_files ();
	    ini_mcbbox = ini_bbbox = 1;
	}
    }
    doUserStartFlag = 1;
}

void doUserStart (char *s)
{
    if (doUserStartFlag) {
	err_flag = 0;		/* no errors */

	if (lay_code != -1) {
	    P_E "%s: %d: warning: possibly loosing data; user statement not after 'ds'!\n",
		argv0, yylineno);
	}

	if (mod_key) {
	    close_files ();
	    dmCheckIn (mod_key, QUIT);
	    mod_key = NULL;
	}

	if (!s_mode) {
	    if (check_tree (s, mod_tree)) {
		if (f_mode && !tree_ptr -> bbox) {
		    pr_exit (0604, 42, s);
		}
		else { /* already defined or used */
		    tree_ptr -> errflag = 1;
		    if (!f_mode)
			pr_exit (0214, 9, s);
		    else
			pr_exit (0214, 37, s);
		}
	    }
	    if (!err_flag) {
		mod_key = dmCheckOut (dmproject, s, WORKING, DONTCARE, LAYOUT, CREATE);
		open_files ();
		ini_mcbbox = ini_bbbox = 1;
	    }
	}
	/* store alias name s and ms_name in alias tree */
	store_alias (ms_name, s);
	strcpy (ms_name, s);
    }
    else
	P_E "%s: %d: warning: no valid context present; statement skipped (9 %s)\n", argv0, yylineno, s);
    doUserStartFlag = 0;
    termcount = 1;
}

static int calc_coord (int val)
{
    xval = resolution * val / 2;
    val = (int)ROUND (xval);
if (!r_mode)
    if (((double)val - xval) > 0.0001 || ((double)val - xval) < -0.0001)
	P_E "%s: %d: warning: %g rounded to %d\n", argv0, yylineno, xval, val);
    return (val);
}

static int convert (int val)
{
    xval = resolution * val;
    val = (int)ROUND (xval);
if (!r_mode)
    if (((double)val - xval) > 0.0001 || ((double)val - xval) < -0.0001)
	P_E "%s: %d: warning: %g rounded to %d\n", argv0, yylineno, xval, val);
    return (val);
}

void doBox (int l, int w, int a, int b, int x, int y)
{
    if (lay_code == -1) pr_exit (034, 0, "error: layer unspecified");

    Box_x = a;
    Box_y = b;
    Box_dir = 0;

    a *= 2;
    b *= 2;

    if (y == 0) {
	box_xl = a - l;
	box_xr = a + l;
	box_yb = b - w;
	box_yt = b + w;
    }
    else if (x == 0) {
	box_xl = a - w;
	box_xr = a + w;
	box_yb = b - l;
	box_yt = b + l;
    }
    else {
	Box_dir = 1;
	if (!err_flag && !s_mode) ABox (l, w, a, b, x, y);
	return;
    }

    if (!err_flag && !s_mode) {
	box_xl = calc_coord (box_xl);
	box_xr = calc_coord (box_xr);
	box_yb = calc_coord (box_yb);
	box_yt = calc_coord (box_yt);
	proc_box (box_xl, box_xr, box_yb, box_yt);
    }
}

void doCifCall (int s)
{
    char   *alias;
/* look for alias name for symbol%d and if exists use it */
    sprintf (mc_name, "symbol%d", s);
    alias = find_alias (mc_name);
    if (alias != 0)
	strcpy (mc_name, alias);
    else if (s == 0 && try_input ()) {
	sscanf (userChar, "%s", mc_name);
	if (v_mode) P_E "-- c0, using cell '%s'\n", mc_name);
    }

    MapTrans ();

    if (!err_flag && !s_mode) {
	if (mod_key != NULL)
	    proc_cif_mc (instnamed, CurrentTransform[0][0], CurrentTransform[1][0],
		    CurrentTransform[2][0], CurrentTransform[0][1],
		    CurrentTransform[1][1], CurrentTransform[2][1]);
	else
	    P_E "%s: %d: warning: no valid context present; statement skipped (C %d)\n", argv0, yylineno, s);
    }
    if (instnamed) {
	instnamed = 0;
	if (v_mode) P_E "-- c%d, instance '%s'\n", s, instance);
    }
}

void ABox (int Length, int Width, int XPos, int YPos, int XDirection, int YDirection)
{
    double  C, sqrt ();
    int     i, Left, Bottom, Right, Top;

    Left   = XPos - Length;
    Right  = XPos + Length;
    Bottom = YPos - Width;
    Top    = YPos + Width;

    C = sqrt ((double) (XDirection * XDirection + YDirection * YDirection));
    i = 0;

    Points[i++] = (Left * XDirection - Bottom * YDirection -
	    XDirection * XPos + YDirection * YPos) / C + XPos;
    Points[i++] = (Left * YDirection + Bottom * XDirection -
	    YDirection * XPos - XDirection * YPos) / C + YPos;

    Points[i++] = (Left * XDirection - Top * YDirection -
	    XDirection * XPos + YDirection * YPos) / C + XPos;
    Points[i++] = (Left * YDirection + Top * XDirection -
	    YDirection * XPos - XDirection * YPos) / C + YPos;

    Points[i++] = (Right * XDirection - Top * YDirection -
	    XDirection * XPos + YDirection * YPos) / C + XPos;
    Points[i++] = (Right * YDirection + Top * XDirection -
	    YDirection * XPos - XDirection * YPos) / C + YPos;

    Points[i++] = (Right * XDirection - Bottom * YDirection -
	    XDirection * XPos + YDirection * YPos) / C + XPos;
    Points[i++] = (Right * YDirection + Bottom * XDirection -
	    YDirection * XPos - XDirection * YPos) / C + YPos;

    for (int_ind = 0; int_ind < i; int_ind++)
	int_val[int_ind] = 2 * Points[int_ind];
    int_val[int_ind++] = 2 * Points[0];
    int_val[int_ind++] = 2 * Points[1];

    proc_poly ();
}

void TTranslate (int XPos, int YPos)
{
    CurrentTransform[2][0] = CurrentTransform[2][0] + XPos;
    CurrentTransform[2][1] = CurrentTransform[2][1] + YPos;
}

void TMY ()
{
/* MY in cif means mirror in y direction, i.e. y = -y
 * AND NOT like in ldm over y axis
 */
    CurrentTransform[0][1] = -CurrentTransform[0][1];
    CurrentTransform[1][1] = -CurrentTransform[1][1];
    CurrentTransform[2][1] = -CurrentTransform[2][1];
}

void TMX ()
{
/* MX in cif means mirror in x direction, i.e. x = -x
 * AND NOT like in ldm over x axis
 */
    CurrentTransform[0][0] = -CurrentTransform[0][0];
    CurrentTransform[1][0] = -CurrentTransform[1][0];
    CurrentTransform[2][0] = -CurrentTransform[2][0];
}

void TRotate (int XDirection, int YDirection)
{
/*
 * Rotation angle is expressed as a CIF-style direction vector.
 */
    int Int1;

    if (XDirection == 0) {
	if (YDirection > 0) { /* rotate ccw by 90 degrees */
	    Int1 = CurrentTransform[0][0];
	    CurrentTransform[0][0] = -CurrentTransform[0][1];
	    CurrentTransform[0][1] = Int1;
	    Int1 = CurrentTransform[1][0];
	    CurrentTransform[1][0] = -CurrentTransform[1][1];
	    CurrentTransform[1][1] = Int1;
	    Int1 = CurrentTransform[2][0];
	    CurrentTransform[2][0] = -CurrentTransform[2][1];
	    CurrentTransform[2][1] = Int1;
	}
	if (YDirection < 0) { /* rotate ccw by 270 degrees */
	    Int1 = CurrentTransform[0][0];
	    CurrentTransform[0][0] = CurrentTransform[0][1];
	    CurrentTransform[0][1] = -Int1;
	    Int1 = CurrentTransform[1][0];
	    CurrentTransform[1][0] = CurrentTransform[1][1];
	    CurrentTransform[1][1] = -Int1;
	    Int1 = CurrentTransform[2][0];
	    CurrentTransform[2][0] = CurrentTransform[2][1];
	    CurrentTransform[2][1] = -Int1;
	}
    }
    else if (YDirection == 0) {
	if (XDirection < 0) { /* rotate ccw by 180 degrees */
	    for (Int1 = 0; Int1 < 3; ++Int1) {
		CurrentTransform[Int1][0] = -CurrentTransform[Int1][0];
		CurrentTransform[Int1][1] = -CurrentTransform[Int1][1];
	    }
	}
    }
    else {
	char buf[80];
	sprintf (buf, "error: cannot handle rotation angle \"R%d %d\"!",
		XDirection, YDirection);
	pr_exit (014, 0, buf);
    }
}

void TIdentity ()
{
    CurrentTransform[0][0] = CurrentTransform[1][1] = 1;
    CurrentTransform[0][1] = CurrentTransform[1][0] = 0;
    CurrentTransform[2][0] = CurrentTransform[2][1] = 0;
}

void MapTrans ()
{
    int pos = 0;
/* Maps anti clock and clock wise, i.e. R 90 -> R 270 and R 270 -> R 90 */

    if (CurrentTransform[0][0] == 1 && CurrentTransform[0][1] == 0 &&
	CurrentTransform[1][0] == 0 && CurrentTransform[1][1] == 1)
	pos = 0;
    else if (CurrentTransform[0][0] == 0 && CurrentTransform[0][1] == -1 &&
	     CurrentTransform[1][0] == 1 && CurrentTransform[1][1] == 0)
	pos = 1;
    else if (CurrentTransform[0][0] == -1 && CurrentTransform[0][1] == 0 &&
	     CurrentTransform[1][0] == 0 && CurrentTransform[1][1] == -1)
	pos = 2;
    else if (CurrentTransform[0][0] == 0 && CurrentTransform[0][1] == 1 &&
	     CurrentTransform[1][0] == -1 && CurrentTransform[1][1] == 0)
	pos = 3;
/* mx my */
    else if (CurrentTransform[0][0] == 1 && CurrentTransform[0][1] == 0 &&
	     CurrentTransform[1][0] == 0 && CurrentTransform[1][1] == -1)
	pos = 4;
    else if (CurrentTransform[0][0] == -1 && CurrentTransform[0][1] == 0 &&
	     CurrentTransform[1][0] == 0 && CurrentTransform[1][1] == 1)
	pos = 5;
/* mx r90 mx r270 */
    else if (CurrentTransform[0][0] == 0 && CurrentTransform[0][1] == 1 &&
	     CurrentTransform[1][0] == 1 && CurrentTransform[1][1] == 0)
	pos = 6;
    else if (CurrentTransform[0][0] == 0 && CurrentTransform[0][1] == -1 &&
	     CurrentTransform[1][0] == -1 && CurrentTransform[1][1] == 0)
	pos = 7;

    switch (pos) {
	case 0:
	    CurrentTransform[0][0] = 1, CurrentTransform[0][1] = 0;
	    CurrentTransform[1][0] = 0, CurrentTransform[1][1] = 1;
	    break;
	case 1:
	    CurrentTransform[0][0] = 0, CurrentTransform[0][1] = 1;
	    CurrentTransform[1][0] = -1, CurrentTransform[1][1] = 0;
	    break;
	case 2:
	    CurrentTransform[0][0] = -1, CurrentTransform[0][1] = 0;
	    CurrentTransform[1][0] = 0, CurrentTransform[1][1] = -1;
	    break;
	case 3:
	    CurrentTransform[0][0] = 0, CurrentTransform[0][1] = -1;
	    CurrentTransform[1][0] = 1, CurrentTransform[1][1] = 0;
	    break;
	case 5:
	    CurrentTransform[0][0] = -1, CurrentTransform[0][1] = 0;
	    CurrentTransform[1][0] = 0, CurrentTransform[1][1] = 1;
	    break;
	case 4:
	    CurrentTransform[0][0] = 1, CurrentTransform[0][1] = 0;
	    CurrentTransform[1][0] = 0, CurrentTransform[1][1] = -1;
	    break;
	case 7:
	    CurrentTransform[0][0] = 0, CurrentTransform[0][1] = -1;
	    CurrentTransform[1][0] = -1, CurrentTransform[1][1] = 0;
	    break;
	case 6:
	    CurrentTransform[0][0] = 0, CurrentTransform[0][1] = 1;
	    CurrentTransform[1][0] = 1, CurrentTransform[1][1] = 0;
	    break;
    }
}

void deletecell (int s)
{
    char    name[DM_MAXNAME + 1];

    if (v_mode) P_E "-- dd %d\n", s);

    sprintf (name, "symbol%d", s);

    if (!s_mode) {
	if (check_tree (name, mod_tree)) {
	    dmRemoveCell (dmproject, name, WORKING, DONTCARE, LAYOUT);
	}
	else {
	    P_E "%s: %s: Symbol not defined\n", argv0, name);
	}
    }
}

void dumpT (char *s)
{
    P_E "Trans Matrix: (%s)\n", s);
    P_E "%d %d\n%d %d\n%d%d\n\n",
	CurrentTransform[0][0], CurrentTransform[0][1],
	CurrentTransform[1][0], CurrentTransform[1][1],
	CurrentTransform[2][0], CurrentTransform[2][1]);
}

void doTerminal ()
{
    if (strlen (Termname) > DM_MAXNAME) {
	pr_exit (0634, 18, Termname);
	sprintf (name_len, "%d", DM_MAXNAME);
	pr_exit (0600, 19, name_len);
	Termname[DM_MAXNAME] = '\0';
    }
    strcpy (terminal, Termname);

    if (append_tree (terminal, &tnam_tree)) {
	/* create a new unique name for the connector */
	sprintf (Termname, "%s_c%d_", terminal, termcount++); /* SdeG4.21 */
	P_E "%s: %d: warning: terminal \"%s\" already used; using \"%s\"\n",
	    argv0, yylineno, terminal, Termname);
	if (strlen (Termname) > DM_MAXNAME) {
	    pr_exit (0634, 18, Termname);
	    sprintf (name_len, "%d", DM_MAXNAME);
	    pr_exit (0600, 19, name_len);
	    Termname[DM_MAXNAME] = '\0';
	}
	strcpy (terminal, Termname);

	if (append_tree (terminal, &tnam_tree))
		pr_exit (034, 12, terminal); /* must not happen any more */
    }

    if (!err_flag && !s_mode) proc_term (box_xl, box_xr, box_yb, box_yt);
}

void doUserTerm ()
{
    int r = 1;

    done_user_term = 1;

    if (t_mode) {
	box_xl = convert (Termx);
	box_yb = convert (Termy);
	box_xr = box_xl + t_width;
	box_yt = box_yb + t_width;
    }
    else if (Termx != Box_x || Termy != Box_y) {
	goto err;
    }
    else if (Box_dir) { r = 2; goto err; }

    doTerminal ();
    return;

err:
    P_E "%s: %d: warning: terminal \"%s\" skipped!\n", argv0, yylineno, Termname);
    switch (r) {
    case 1:
	P_E "-- Previous Box centerpoint not equal to Terminal centerpoint!\n");
	P_E "-- Consider the use of the option '-t' that automatically\ngenerates a terminal point or rectangle\n");
	break;
    case 2:
	P_E "-- Previous Box direction not usable for Terminal!\n");
	break;
    }
}

void doUserTerm2 (int w, int h)
{
    done_user_term = 1;

    if (lay_code == -1) {
	pr_exit (034, 0, "error: layer unspecified");
    }
    else {
	box_xr = 2 * Termx;
	box_yt = 2 * Termy;
	box_xl = calc_coord (box_xr - w);
	box_xr = calc_coord (box_xr + w);
	box_yb = calc_coord (box_yt - h);
	box_yt = calc_coord (box_yt + h);
	doTerminal ();
    }
}

void doUserLabel ()
{
    if (lay_code == -1) {
	pr_exit (034, 0, "error: layer unspecified");
	return;
    }

    if (strlen (Termname) > DM_MAXNAME) {
	pr_exit (0634, 18, Termname);
	sprintf (name_len, "%d", DM_MAXNAME);
	pr_exit (0600, 19, name_len);
	Termname[DM_MAXNAME] = '\0';
    }
    strcpy (label, Termname);

    if (append_tree (label, &tnam_tree)) {
	/* pr_exit (034, 12, label); *//* already used */ /* ignored for labels */
	/* new: no duplicate labels allowed */
	/* old: space migth say something about duplicate label names,
	   but do not care, it's the same signal */
	P_E "%s: %d: warning: label \"%s\" already used; skipped!\n",
	    argv0, yylineno, label);
    }
    else {
	int rx, ry;
	rx = convert (Termx);
	ry = convert (Termy);
	if (!err_flag && !s_mode) proc_label (rx, ry);
    }
}

static int dolayer (char *s)
{
    register int i;

    for (i = 0; s[i]; ++i) {
	if (i == DM_MAXLAY) {
	    pr_exit (0634, 15, s);
	    sprintf (name_len, "%d", DM_MAXLAY);
	    pr_exit (0600, 19, name_len);
	    break;
	}
	if (isupper ((int)s[i]))
	    layer[i] = tolower (s[i]);
	else
	    layer[i] = s[i];
    }
    layer[i] = '\0';

    i = 0; /* layer specified! */

    if (!s_mode) {
	for (; i < process -> nomasks; ++i)
	    if (!strcmp (layer, process -> mask_name[i])) return (i);

	pr_exit (034, 20, layer);/* unrecogn. laycode */
    }
    return (i);
}

static int  aliascount = 0;

struct aliasptr {
    char   *alias;
    char   *name;
    struct aliasptr *next;
};

static struct aliasptr *rootpnt, *currpnt;

char *find_alias (char *name)
{
    struct aliasptr *cellptr;
    int     i;

    cellptr = rootpnt;

    for (i = 0; i < aliascount; i++) {
	if (strcmp (cellptr -> name, name) == 0) {
	    return (cellptr -> alias);
	}
	cellptr = cellptr -> next;
    }
    return ((char *) 0);
}

void store_alias (char *name, char *alias)
{
    struct aliasptr *cellptr;

    ALLOC (cellptr, aliasptr);

    cellptr -> name  = _dmStrSave (name);
    cellptr -> alias = _dmStrSave (alias);
    cellptr -> next = 0;

    if (aliascount == 0) {
	rootpnt = cellptr;
	currpnt = cellptr;
    }
    else {
	currpnt -> next = cellptr;
	currpnt = cellptr;
    }
    aliascount++;
}
