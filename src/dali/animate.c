/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	P. Groeneveld
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

// #define VERBOSE_ANIMATE /* to get ptext */

extern FILE *rcfile;
extern Coor xltb, xrtb, ybtb, yttb; /* total bounding box */
extern int *black_arr;
extern int *dom_arr;
extern int *vis_arr;
extern int *def_arr;
extern int  NR_dom;
extern int  NR_lay;
extern int  omode;
extern int *drawing_order;
extern int  gridflag;
extern int  initwindow_flag;
extern Coor initwindow_xl, initwindow_yb, initwindow_dx;
extern int  tracker_mode;
extern int  use_new_mode;
extern int  zoom_mode;
extern int  Default_expansion_level;
extern int  Draw_dominant;
extern int  Draw_hashed;
extern DM_PROJECT *dmproject;
extern char *Input_cell_name; /* name of pre-specified cell */
extern char **lay_names;
extern char **visuals;

/* Sea-of-Gates additions: */
extern char *ImageName;
extern char *ViaName;
extern int  MaxDrawImage;
extern int  ImageMode;

int  ANI_mode;
static FILE *outputfile = NULL;

static void do_animate (void);
static int  getLineValues (char *line, int *values, char *token);
static int  getMaskValues (char *line, int *values, char *token);
static void setStr (char *str);

static void a_message (char *token, char *str)
{
    PE "Error .dalirc: ");
    if (token) PE "%s: ", token);
    PE "%s\n", str);
    if (outputfile) {
	fprintf (outputfile, "Error .dalirc: ");
	if (token) fprintf (outputfile, "%s: ", token);
	fprintf (outputfile, "%s\n", str);
	fflush (outputfile);
    }
}

static void ill_value (char *token, int val)
{
    char err_str[MAXCHAR];
    sprintf (err_str, "ill. value specified (%d)", val);
    a_message (token, err_str);
}

static int msk_nbr (char *lc, char *token, int mode)
{
    char err_str[MAXCHAR];
    register int lay;

    for (lay = 0; lay < NR_lay; ++lay) {
	if (strcmp (lay_names[lay], lc) == 0) return (lay);
    }
    if (mode)
    for (lay = 0; lay < NR_VIS; ++lay) {
	if (strcmp (visuals[lay], lc) == 0) return (NR_lay + lay);
    }
    sprintf (err_str, "unknown layer '%s'", lc);
    a_message (token, err_str);
    return (-1);
}

void open_dalirc ()
{
    char *dalirc = ".dalirc";
    char *home = NULL;
    char *filepath;
    /*
    ** (1) Try in current directory:
    */
    if (!(rcfile = fopen (dalirc, "r"))) {
	/*
	** (2) Try in home directory:
	*/
	if ((home = getenv ("HOME"))) {
	    filepath = malloc (strlen (home) + strlen (dalirc) + 2);
	    if (filepath) {
		sprintf (filepath, "%s/%s", home, dalirc);
		rcfile = fopen (filepath, "r");
		FREE (filepath);
	    }
	}
	if (!rcfile) {
	    /*
	    ** (3) Try in process directory:
	    */
	    dalirc = dmGetMetaDesignData (PROCPATH, dmproject, "dalirc");
	    rcfile = fopen (dalirc, "r");
	}
    }
}

void animate ()
{
    char  outfile[32];

    if (rcfile) {
	ptext ("Reading .dalirc file....");
	if (omode) {
	    sprintf (outfile, "animate.%d", (int)getpid ());
	    if (!(outputfile = fopen (outfile, "w")))
		a_message (outfile, "Failed to open output file!");
	}
	ANI_mode = 1;
	do_animate ();
	ANI_mode = 0;
	if (outputfile) fclose (outputfile);
	fclose (rcfile);
    }
    else {
	ptext ("No .dalirc file found!");
	sleep (1);
    }

    /*
    ** Read pre-specified cell, if it exists:
    */
    if (Input_cell_name) {
	inp_mod (Input_cell_name);
	picture ();
    }
}

static void do_animate ()
{
    char scanline[200], token[200], name[200], arg2[200];
    register char *pt;
    register int i, lay, found;
    int *values;
    int Number = 0;

    rewind (rcfile);

    MALLOCN (values, int, NR_lay + NR_VIS);

    while (fgets (scanline, 200, rcfile)) {
    /*
     * skip comment/empty lines and white space
     */
	pt = scanline;
	while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
	if (*pt == '#' || *pt == '\n' || !*pt) continue; /* comment line */
    /*
     * get token and (if possible) first command
     */
	found = sscanf (pt, "%s %s %s", token, name, arg2);
	if (found < 1) continue;
	if (!isalpha ((int)*token)) {
	    a_message (token, "token not recognized");
	    continue;
	}
    /*
     * parse token
     */
	if (strcmp (token, "label") == 0) { /* found label: ignore */
#ifdef VERBOSE_ANIMATE
	    ptext ("label");
#endif
	    continue;
	}

	if (strcmp (token, "set_val") == 0) {
	    if (found < 2)
		a_message (token, "value not given");
	    else
		Number = atoi (name);
	    continue;
	}

	if (strcmp (token, "add_val") == 0) {
	    if (found < 2)
		a_message (token, "value not given");
	    else
		Number += atoi (name);
	    continue;
	}

	if (strcmp (token, "if_val") == 0) {
	    if (found < 3) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    if (Number != atoi (name)) continue;
	    rewind (rcfile);
	    found = 0;
	    while (fgets (scanline, 200, rcfile)) {
		pt = scanline;
		while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
		if (*pt == '#' || *pt == '\n' || !*pt) continue; /* comment */
		if (sscanf (pt, "%s %s", token, name) < 2) continue;
		if (strcmp (token, "label") == 0) {
		    if (strcmp (arg2, name) == 0) {
			++found; /* found label: continue there */
			break;
		    }
		}
	    }
	    if (!found) {
		sprintf (scanline, "can't find label '%s'", arg2);
		a_message ("if_val", scanline);
		return;
	    }
	    continue;
	}

	if (strcmp (token, "ifnot_val") == 0) {
	    if (found < 3) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    if (Number == atoi (name)) continue;
	    rewind (rcfile);
	    found = 0;
	    while (fgets (scanline, 200, rcfile)) {
		pt = scanline;
		while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
		if (*pt == '#' || *pt == '\n' || !*pt) continue; /* comment */
		if (sscanf (pt, "%s %s", token, name) < 2) continue;
		if (strcmp (token, "label") == 0) {
		    if (strcmp (arg2, name) == 0) {
			++found; /* found label: continue there */
			break;
		    }
		}
	    }
	    if (!found) {
		sprintf (scanline, "can't find label '%s'", arg2);
		a_message ("ifnot_val", scanline);
		return;
	    }
	    continue;
	}

	if (strcmp (token, "goto") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("goto");
#endif
	    if (found < 2) {
		a_message (token, "label not given");
		continue;
	    }
	    rewind (rcfile);
	    found = 0;
	    while (fgets (scanline, 200, rcfile)) {
		pt = scanline;
		while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
		if (*pt == '#' || *pt == '\n' || !*pt) continue; /* comment */
		if (sscanf (pt, "%s %s", token, arg2) < 2) continue;
		if (strcmp (token, "label") == 0) {
		    if (strcmp (arg2, name) == 0) {
			++found; /* found label: continue there */
			break;
		    }
		}
	    }
	    if (!found) {
		sprintf (scanline, "can't find label '%s'", name);
		a_message ("goto", scanline);
		return;
	    }
	    continue;
	}

	if (strcmp (token, "print") == 0) { /* print text */
#ifdef VERBOSE_ANIMATE
	    ptext ("text");
#endif
	    pt += 5; /* skip token */
	    if (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
	    setStr (pt);
	    if (outputfile) {
		fprintf (outputfile, "%s\n", pt);
		fflush (outputfile);
	    }
	    ptext (pt);
	    continue;
	}

	if (strcmp (token, "sleep") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("sleeping");
#endif
	    if (found < 2)
		a_message (token, "time value not given");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else
		sleep ((unsigned long) found);
	    continue;
	}

	if (strcmp (token, "beep") == 0) {
	    if (found < 2)
		ggBell (0);
	    else
		ggBell (atoi (name));
	    if (outputfile) {
		fprintf (outputfile, "\a");
		fflush (outputfile);
	    }
	    continue;
	}

	if (strcmp (token, "read") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("reading cell");
#endif
	    if (found < 2) {
		a_message (token, "cellname not given");
		continue;
	    }
	    inp_mod (name);
	    if (Default_expansion_level == -1) main_menu ();
	    picture ();
	    continue;
	}

	if (strcmp (token, "redraw") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("redrawing");
#endif
	    pict_all (ERAS_DR);
	    picture ();
	    continue;
	}

	if (strcmp (token, "wdw_bbx") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("wdw_bbx");
#endif
	    bound_w ();
	    picture ();
	    continue;
	}

/*
 * init_window <xl> <yb> <dx>
 */
	if (strcmp (token, "init_window") == 0) {
	    int xl, yb, dx;
	    if (sscanf (pt, "%s %d %d %d", token, &xl, &yb, &dx) < 4) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    initwindow_flag = 1;
	    initwindow_xl = xl;
	    initwindow_yb = yb;
	    initwindow_dx = dx;
	    continue;
	}
/*
 * zoom <cx> <cy> <fraction>
 */
	if (strcmp (token, "zoom") == 0) {
	    float w, h, cx, cy, frac;
#ifdef VERBOSE_ANIMATE
	    ptext ("zooming");
#endif
	    if (sscanf (pt, "%s %f %f %f", token, &cx, &cy, &frac) < 4) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    w = ((xrtb - xltb) * frac) / 2;
	    h = ((yttb - ybtb) * frac) / 2;
	    cx = xltb + ((xrtb - xltb) * cx);
	    cy = ybtb + ((yttb - ybtb) * cy);
	    curs_w ((Coor) (cx - w), (Coor) (cx + w),
		    (Coor) (cy - h), (Coor) (cy + h));
	    picture ();
	    continue;
	}
/*
 * center <cx> <cy>
 */
	if (strcmp (token, "center") == 0) {
	    float cx, cy;
#ifdef VERBOSE_ANIMATE
	    ptext ("centering");
#endif
	    if (sscanf (pt, "%s %f %f", token, &cx, &cy) < 3) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    cx = xltb + ((xrtb - xltb) * cx);
	    cy = ybtb + ((yttb - ybtb) * cy);
	    center_w ((Coor) cx, (Coor) cy);
	    picture ();
	    continue;
	}
/*
 * expand
 */
	if (strcmp (token, "expand") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("expanding");
#endif
	    if (found < 2)
		a_message (token, "level not given");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else if (expansion (found)) {
		inform_cell ();
		set_titlebar (NULL);
		picture ();
	    }
	    continue;
	}
/*
 * set default expansion level
 */
	if (strncmp (token, "default_expan", 13) == 0) {
	    if (found < 2) {
		a_message (token, "level not given");
		continue;
	    }
	    if (strcmp (name, "no") == 0) {
		Default_expansion_level = -1;
	    }
	    else {
		Default_expansion_level = atoi (name);
		if (Default_expansion_level < 1) Default_expansion_level = 1;
		if (Default_expansion_level > 9) Default_expansion_level = 9;
	    }
	    continue;
	}
/*
 * set visible layer: visible <visual|layer> ["on"|"off"]
 */
	if (strcmp (token, "visible") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("visible");
#endif
	    if (found < 2) {
		a_message (token, "visual or layer not given");
		continue;
	    }
	    lay = NR_lay + NR_VIS;
	    while (--lay >= 0) values[lay] = vis_arr[lay];

	    if ((lay = msk_nbr (name, token, 1)) < 0) continue;

	    i = !vis_arr[lay]; /* toggle? */
	    if (found > 2) {
		     if (strcmp (arg2, "on" ) == 0) i = 1;
		else if (strcmp (arg2, "off") == 0) i = 0;
		else a_message (token, "arg. not 'on' or 'off' (toggle)");
	    }
	    if (values[lay] != i) {
		values[lay] = i;
		set_pict_arr (values);
		Rmsk ();
	    }
	    continue; /* requires redraw command */
	}
/*
 * set layer for drawing: layer <layer> ["on"|"off"]
 */
	if (strcmp (token, "layer") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("layer");
#endif
	    if (found < 2) {
		a_message (token, "layer not given");
		continue;
	    }
	    if ((lay = msk_nbr (name, token, 0)) < 0) continue;
	    if (found > 2) {
		if (strcmp (arg2, "on") == 0) {
		    if (!def_arr[lay]) toggle_lay (lay);
		    continue;
		}
		if (strcmp (arg2, "off") == 0) {
		    if (def_arr[lay]) toggle_lay (lay);
		    continue;
		}
		a_message (token, "arg. not 'on' or 'off' (toggle)");
	    }
	    toggle_lay (lay);
	    continue;
	}
/*
 * set stipple for drawing: stipple<nr> <width>x<height> { <bits>... }
 */
	if (strncmp (token, "stipple", 7) == 0) {
	    int nr, w, h, b;
	    char bm[8];
	    if (found < 3) {
		a_message (token, "too less arguments");
		continue;
	    }
	    if (!isdigit ((int)token[7]) ||
		!isdigit ((int)name[0]) || name[1] != 'x' ||
		!isdigit ((int)name[2]) || arg2[0] != '{') {
ill_format:
		a_message (token, "illegal format");
		continue;
	    }
	    nr = token[7] - '0';
	    w = name[0] - '0';
	    h = name[2] - '0';
	    if (w < 1 || w > 8) goto ill_format;
	    if (h < 1 || h > 8) goto ill_format;
	    while (*pt != '{') ++pt;
	    ++pt;
	    while (*pt == ' ' || *pt == '\t') ++pt;
	    found = -1;
	    while (*pt != '}') {
		if (*pt++ != '0') goto ill_format;
		if (*pt++ != 'x') goto ill_format;
		if (isdigit ((int)*pt)) b = *pt - '0';
		else if (*pt >= 'a' && *pt <= 'f') b = 10 + (*pt - 'a');
		else goto ill_format;
		b *= 16;
		++pt;
		if (isdigit ((int)*pt)) b += (*pt - '0');
		else if (*pt >= 'a' && *pt <= 'f') b += (10 + (*pt - 'a'));
		else goto ill_format;

		if (++found > 7) goto ill_format;
		bm[found] = b;
		if (*++pt == ',') ++pt;
		while (*pt == ' ' || *pt == '\t') ++pt;
	    }
	    set_new_bitmap (nr, w, h, bm);
	    continue;
	}
/*
 * set fill_style for drawing: fill_style <layer> <style#[-+shift]>
 */
	if (strcmp (token, "fill_style") == 0) {
	    if (found < 3) {
		a_message (token, "too less arguments");
		continue;
	    }
	    if ((lay = msk_nbr (name, token, 0)) < 0) continue;
	    for (pt = arg2; isdigit ((int)*pt); ++pt); /* skip digits */
	    set_fill_style (lay, atoi (arg2), atoi (pt));
	    rebulb (lay);
	    continue;
	}
/*
 * drawing_order [<v1> <v2> .... <vn>]
 */
	if (strcmp (token, "drawing_order") == 0) {
#ifdef VERBOSE_ANIMATE
	    ptext ("drawing_order");
#endif
	    NR_dom = 0; /* reset number of dominants */
	    for (lay = 0; lay < NR_lay; ++lay) dom_arr[lay] = 0;

	    found = getMaskValues (pt, values, token);

	    for (lay = 0; lay < NR_lay; ++lay) {
		if (black_arr[lay]) {
		    for (i = 0; i < found; ++i) {
			if (lay == values[i]) break;
		    }
		    if (i == found) { /* black lay not found */
			drawing_order[NR_dom++] = lay;
			dom_arr[lay] = 1;
		    }
		}
	    }

	    for (i = 0; i < found; ++i) {
		lay = values[i];
		if (dom_arr[lay]) {
		    sprintf (arg2, "lay '%s' already specified", lay_names[lay]);
		    a_message (token, arg2);
		    continue;
		}
		drawing_order[NR_dom++] = lay;
		dom_arr[lay] = 1;
	    }
	    if (Draw_dominant) Rmsk ();
	    continue;
	}
/*
 * append <xl> <xr> <yb> <yt>
 */
	if (strcmp (token, "append") == 0) {
	    Coor xl, xr, yb, yt;
	    if (sscanf (pt, "%s %ld %ld %ld %ld", token, &xl, &xr, &yb, &yt) < 5) {
		a_message (token, "ill. number of coordinates");
		continue;
	    }
	    xl *= QUAD_LAMBDA; xr *= QUAD_LAMBDA;
	    yb *= QUAD_LAMBDA; yt *= QUAD_LAMBDA;
	    addel_cur (xl, xr, yb, yt, ADD);
	    picture ();
	    continue;
	}
/*
 * delete <xl> <xr> <yb> <yt>
 */
	if (strcmp (token, "delete") == 0) {
	    Coor xl, xr, yb, yt;
	    if (sscanf (pt, "%s %ld %ld %ld %ld", token, &xl, &xr, &yb, &yt) < 5) {
		a_message (token, "ill. number of coordinates");
		continue;
	    }
	    xl *= QUAD_LAMBDA; xr *= QUAD_LAMBDA;
	    yb *= QUAD_LAMBDA; yt *= QUAD_LAMBDA;
	    addel_cur (xl, xr, yb, yt, DELETE);
	    picture ();
	    continue;
	}
/*
 * draw dominant/non_dominant
 */
	if (strcmp (token, "dominant") == 0) {
	    i = 1;
	    if (found > 1) {
		if (strcmp (name, "off") == 0) i = 0;
		else if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    if (Draw_dominant != i) {
		Draw_dominant = i;
		Rmsk ();
	    }
	    continue;
	}
/*
 * draw hashed/non_hased instances
 */
	if (strcmp (token, "hashed") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    Draw_hashed = 0; /* OFF */
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    Draw_hashed = 1; /* ON */
	    continue;
	}

	if (strcmp (token, "tracker") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    tracker_mode = 0;
		    continue;
		}
		if (strcmp (name, "on") == 0) {
		    tracker_mode = 1;
		    continue;
		}
		if (strcmp (name, "auto") == 0) {
		    tracker_mode = 2;
		    continue;
		}
		a_message (token, "arg. not 'off', 'on' or 'auto'");
	    }
	    else tracker_mode = 1; /* ON */
	    continue;
	}

	if (strcmp (token, "wire_width_values") == 0) {
	    if (found < 2) {
		a_message (token, "values not specified");
		continue;
	    }
	    found = getLineValues (pt, values, token);
	    set_wire_values (found, values);
	    continue;
	}

	if (strcmp (token, "wire_width") == 0) {
	    if (found < 2)
		a_message (token, "width value not given");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else
		set_wire_width (found);
	    continue;
	}

	if (strcmp (token, "wire_extension") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    set_wire_ext (0); /* OFF */
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    set_wire_ext (1); /* ON */
	    continue;
	}

	if (strcmp (token, "grid") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    if (gridflag) { toggle_grid (); picture (); }
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    if (!gridflag) { toggle_grid (); picture (); }
	    continue;
	}

	if (strcmp (token, "no_grid_adjust") == 0) {
	    no_grid_adjust ();
	    continue;
	}

	if (strcmp (token, "display_grid_width") == 0) {
	    if (found < 2)
		a_message (token, "value not specified");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else
		set_grid_width (found);
	    continue;
	}

	if (strcmp (token, "display_grid_values") == 0) {
	    if (found < 2) {
		a_message (token, "values not specified");
		continue;
	    }
	    found = getLineValues (pt, values, token);
	    set_grid_values (found, values);
	    continue;
	}

	if (strcmp (token, "snap_grid_width") == 0) {
	    if (found < 2)
		a_message (token, "value not specified");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else
		set_snap_grid_width (found);
	    continue;
	}

	if (strcmp (token, "snap_grid_values") == 0) {
	    if (found < 2) {
		a_message (token, "values not specified");
		continue;
	    }
	    found = getLineValues (pt, values, token);
	    set_sn_grid_values (found, values);
	    continue;
	}

	if (strcmp (token, "snap_grid_offset") == 0) {
	    int x_offset, y_offset;
	    if (sscanf (pt, "%s %d %d", token, &x_offset, &y_offset) < 3) {
		a_message (token, "ill. number of arguments");
		continue;
	    }
	    set_sn_grid_offset (x_offset, y_offset);
	    continue;
	}

	if (strcmp (token, "flat_expansion") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    set_flat_expansion (0); /* OFF */
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    set_flat_expansion (1); /* ON */
	    continue;
	}

	if (strcmp (token, "disabled_layers") == 0) {
	    found = getMaskValues (pt, values, token);
	    disable_masks (found, values);
	    continue;
	}

	if (strcmp (token, "zoom_mode") == 0) {
	    if (found < 2) a_message (token, "arg. not specified");
	    else if (strcmp (name, "area")  == 0) zoom_mode = 2;
	    else if (strcmp (name, "point") == 0) zoom_mode = 1;
	    else if (strcmp (name, "fixed") == 0) zoom_mode = 0;
	    else a_message (token, "illegal argument");
	    continue;
	}

	if (strcmp (token, "exp_options") == 0) {
	    pt += 11; /* skip token */
	    while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
	    setStr (pt);
	    setOpt (0, pt);
	    continue;
	}
	if (strcmp (token, "drc_options") == 0) {
	    pt += 11; /* skip token */
	    while (*pt == ' ' || *pt == '\t') ++pt; /* skip white space */
	    setStr (pt);
	    setOpt (1, pt);
	    continue;
	}
/*
 * ImageName (Sea-of-Gates)
 */
	if (*token == 'i') {
	  if (strcmp (token, "image") == 0 || strcmp (token, "imagename") == 0) {
	    if (found < 2) found = 0;
	    else if ((found = strlen (name)) > DM_MAXNAME) found = DM_MAXNAME;
	    if (ImageName) FREE (ImageName);
	    ImageName = found ? strsave (name, found) : NULL;
	    continue;
	  }
	  if (strcmp (token, "imagemode") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    ImageMode = 0; /* OFF */
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    ImageMode = 1; /* ON */
	    continue;
	  }
	}
/*
 * ViaName (Sea-of-Gates)
 */
	if (*token == 'v' && (strcmp (token, "via") == 0 ||
			strcmp (token, "vianame") == 0)) {
	    if (found < 2) found = 0;
	    if (found < 2) found = 0;
	    else if ((found = strlen (name)) < 3) {
		a_message (token, "name too short");
		found = 0;
	    }
	    if (ViaName) FREE (ViaName);
	    ViaName = found ? strsave (name, 3) : NULL;
	    continue;
	}
/*
 * MaxDrawImage (Sea-of-Gates)
 */
	if (strcmp (token, "maxdraw") == 0) {
	    if (found < 2)
		a_message (token, "value not specified");
	    else if ((found = atoi (name)) < 1)
		ill_value (token, found);
	    else
		MaxDrawImage = found;
	    continue;
	}

	if (strcmp (token, "use_new_name") == 0) {
	    if (found > 1) {
		if (strcmp (name, "off") == 0) {
		    use_new_mode = 0;
		    continue;
		}
		if (strcmp (name, "on") != 0)
		    a_message (token, "arg. not 'on' or 'off' ('on')");
	    }
	    use_new_mode = 1;
	    continue;
	}

	if (strcmp (token, "load") == 0) {
	    if (found > 1) load_settings (name);
	    continue;
	}

	if (strcmp (token, "quit") == 0) stop_show (0);

	if (strcmp (token, "set_maskcolornr") == 0 ||
	    strcmp (token, "set_colornr") == 0) {
	    continue;
	}

	if (strcmp (token, "order") == 0) {
	    a_message (token, "please use: drawing_order <v1> <v2> ...");
	    continue;
	}
	a_message (token, "token not recognized");
    }

    FREE (values);

    /* non-visual layer may not be ON */
    for (lay = 0; lay < NR_lay; ++lay)
	if (!vis_arr[lay] && def_arr[lay]) toggle_lay (lay);
}

static void setStr (char *str)
{
    register char *pt = str;
    while (isprint ((int)*pt)) ++pt;
    *pt = 0; /* end of text */
}

static int getLineValues (char *line, int *values, char *token)
{
    register int i, j, k, val;
    char *seperators = " \t\n\r";
    char *cp;
    /*
    ** get the integer values from line and put them in values
    */
    strtok (line, seperators); /* skip token first */

    for (i = 0; i < NR_lay; ++i) {
skip:	cp = strtok (NULL, seperators); /* current value */
	if (!cp) break;
	val = atoi (cp);
	if (val < 1) {
	    ill_value (token, val);
	    goto skip;
	}
	k = -1;
	for (j = 0; j < i; ++j) {
	    if (val <= values[j]) {
		if (val == values[j]) goto skip;
		for (k = i; k > j; --k) values[k] = values[k - 1];
		values[k] = val;
		break;
	    }
	}
	if (k < 0) values[i] = val;
    }
    return i;
}

static int getMaskValues (char *line, int *values, char *token)
{
    register int i, lay;
    char *seperators = " \t\n\r";
    char *cp;
    /*
    ** get the integer values from line and put them in values
    */
    strtok (line, seperators); /* skip token first */

    i = 0;
    while (i < NR_lay) {
	cp = strtok (NULL, seperators); /* current value */
	if (!cp) break;
	lay = msk_nbr (cp, token, 0);
	if (lay >= 0) values[i++] = lay;
    }
    return i;
}

void inform_SofG ()
{
    static char *ask_c[] = {
	 /* 0 */ "maxdraw",
	 /* 1 */ "via_name",
	 /* 2 */ "image_name",
    };
    char mess_str[MAXCHAR];

    ptext ("Info about Sea-of-Gates, select item!");
    switch (ask (sizeof (ask_c) / sizeof (char *), ask_c, -1)) {
    case 0:
	sprintf (mess_str, "Max. draw image value is '%d'", MaxDrawImage);
	break;
    case 1:
	if (ViaName)
	    sprintf (mess_str, "Via name is '%s'", ViaName);
	else
	    sprintf (mess_str, "No via name!");
	break;
    case 2:
	if (ImageName)
	    sprintf (mess_str, "Image name is '%s'", ImageName);
	else
	    sprintf (mess_str, "No image name!");
	break;
    default:
	return;
    }
    ptext (mess_str);
}
