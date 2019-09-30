/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
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

extern Coor xltb, xrtb, ybtb, yttb; /* total bounding box */
extern int *vis_arr;
extern int *def_arr;
extern int *dom_arr;
extern int  NR_lay;
extern int *pict_arr;
extern int  Draw_dominant;
extern int  Draw_hashed;
extern DM_PROJECT *dmproject;

char Input_cell_name[DM_MAXNAME + 1];/* name of pre-specified cell */
int  Default_expansion_level;
char *Print_command;

char ImageInstName[DM_MAXNAME + 1]; /* image name which is skipped */
char ViaInstName[DM_MAXNAME + 1];   /* via name which is skipped */
char Weedout_lib[DM_MAXNAME + 1];   /* Libraries which are not displayed during an add_imp */
int  MaxDrawImage;      /* maximum number of images on the screen */
int  ImageMode = FALSE; /* if TRUE, snap instances to the grid */
int  BackingStore;
int *drawing_order = NULL;
int  NR_dom = 0;

extern char *ThisImage; /* image name (not cell name), Null if no SoG */

static void do_animate (FILE *fp);
static int getLineValues (char *line, int **values, int (*theFunc)());

/*
 * body routine which is called from command()
 */
void load_rc_files ()
{
    char *dalirc = ".dalirc";
    char *seadalirc = ".seadalirc";
    char *home = NULL;
    char filepath[256];
    FILE *rcfile = NULL;

    Default_expansion_level = 1;

/* PATRICK: more initalisations */
    /*
    * NOTE: these are default settings which are convenient for Sea-of-Gates use
    */
    MaxDrawImage = 120;      /* draw at most 120 image cells on the sceen */
    strcpy (ImageInstName, "IMAGE"); /* INSTANCE name of the image */
    strcpy (ViaInstName, "via");     /* INSTANCE name of vias (1st 3 characters) */
    strcpy (Weedout_lib, "");        /* no libraries weeded out */
    BackingStore = TRUE;

    switch_tracker (TRUE); /* The tracker is default on */

/* END PATRICK */

    /* read dalirc
    */
    /* first try in current directory */
    strcpy (filepath, seadalirc);
    rcfile = fopen (filepath, "r");
    if (!rcfile) { /* then try in home directory */
	if ((home = getenv ("HOME"))) {
	    sprintf (filepath, "%s/%s", home, seadalirc);
	    rcfile = fopen (filepath, "r");
	}
    }
    if (!rcfile) { /* then try in process directory */
	strcpy (filepath, dmGetMetaDesignData (PROCPATH, dmproject, "seadalirc"));
	rcfile = fopen (filepath, "r");
    }
    if (!rcfile) { /* then try a dalirc file */
	strcpy (filepath, dalirc);
	rcfile = fopen (filepath, "r");
    }
    if (!rcfile) { /* then try in home directory */
	if ((home = getenv ("HOME"))) {
	    sprintf (filepath, "%s/%s", home, dalirc);
	    rcfile = fopen (filepath, "r");
	}
    }
    if (!rcfile) { /* then try in process directory */
	strcpy (filepath, dmGetMetaDesignData (PROCPATH, dmproject, "dalirc"));
	rcfile = fopen (filepath, "r");
    }

    if (rcfile) {
	char message[256];
	sprintf (message, "Seadali: loading rc file '%s'", filepath);
	/* fprintf (stderr, "%s\n", message); */
	ptext (message);
	do_animate (rcfile);
	fclose (rcfile);
    }
    else {
	ptext ("No dalirc file found.");
    }

    /* set the backing store
    */
    set_backing_store (BackingStore);

    /* read the sea-of gates image
    */
    read_SoG_image (); /* sets ThisImage */
    ptext ("SEADALI, the Sea-of-Gates layout interface of Delft University");

    /* if image: do instance snap
    */
    ImageMode = ThisImage ? TRUE : FALSE;

   /* read pre-specified cell, if it exists
    */
    if (strlen (Input_cell_name) == 0) return;

    if (_dmExistCell (dmproject, Input_cell_name, LAYOUT) != 1) {
	ptext ("Hey! Your pre-specified input cell does not exist!");
	return;
    }

    inp_mod (Input_cell_name);
    expansion (Default_expansion_level);
    pict_all (ERAS_DR);
    picture ();
    set_titlebar (NULL);
}

static void do_animate (FILE *fp)
{
    char  token[200], label[200], name[200], scanline[1024], *pt;
    FILE *outputfile;
    int   order, i, lay, time, found;
    float xc, yc, width, height, center_x, center_y, fraction;

    Draw_dominant = FALSE;
    Draw_hashed = FALSE;

/*
 * open files
 */
#ifdef TO_OUTP_FILE
    sprintf (outfile, "animate.%d", getpid ());
    if (!(outputfile = fopen (outfile, "w"))) {
	PE "ERROR: Failed to open output file 'output.animation'\n");
	fclose (fp);
	return;
    }
#else
    outputfile = NULL;
#endif

    while (fgets (scanline, 1024, fp)) {/* main line */

	/* get token and (if possible) first command
	 */
	if (sscanf (scanline, "%s %s", token, label) < 1) continue;
	if (!isalpha ((int)token[0])) continue;

	/* parse token */

	if (strncmp (token, "label", 5) == 0) { /* found label: ignore */
	 /* ptext ("label"); */
	    continue;
	}

	if (strncmp (token, "goto", 4) == 0) {
	 /* ptext ("go to"); */
	    rewind (fp);
	    found = FALSE;
	    while (fgets (scanline, 1024, fp)) {
		if (scanline[0] == '#' || scanline[0] == '\n' || sscanf (scanline, "%s %s", token, name) != 2)
		    continue;

		if (strcmp (label, name) == 0) { /* found label: continue there */
		    found = TRUE;
		    break;
		}
	    }

	    if (found == TRUE) continue;

	    if (outputfile) {
		fprintf (outputfile, "ERROR: goto: cannot find label '%s'\n", label);
		fflush (outputfile);
	    }
	    continue;
	}

	if (strncmp (token, "print", 5) == 0) { /* print text */
	 /* ptext ("text"); */
	    /* wind left of token */
	    for (i = 0, pt = scanline; *pt != 't'; pt++, i++) ; /* nothing */
	    /* find first non-space */
	    for (pt++; *pt == ' ' || *pt == '\t'; pt++, i++) ; /* nothing */
	    /* terminate string by NULL */
	    for (; i < 200 && isprint ((int)scanline[i]); i++) ; /* nothing */
	    scanline[i] = '\0';
	    /* print */
	    if (outputfile) {
		fprintf (outputfile, "%s\n", pt);
		fflush (outputfile);
	    }
	    ptext (pt);
	    continue;
	}

	if (strncmp (token, "sleep", 5) == 0) {
	 /* ptext ("sleeping"); */
	    if (sscanf (scanline, "%s %d", token, &time) != 2) {
		PE "ERROR: sleep: no time\n");
		continue;
	    }
	    sleep ((unsigned long) time);
	    continue;
	}

	if (strncmp (token, "beep", 4) == 0) {
	    if (outputfile) {
		fprintf (outputfile, "\a");
		fflush (outputfile);
	    }
	    continue;
	}

	if (strncmp (token, "read", 4) == 0) {
	 /* ptext ("reading cell"); */
	    if (sscanf (scanline, "%s %s", token, name) != 2) {
		PE "ERROR: read: no file\n");
		continue;
	    }
	    inp_mod (name);
	    picture ();
	    continue;
	}

	if (strncmp (token, "redraw", 6) == 0) {
	 /* ptext ("redrawing"); */
	    picture ();
	    continue;
	}

	if (strncmp (token, "wdw_bbx", 7) == 0) {
	 /* ptext ("wdw_bbx"); */
	    bound_w ();
	    picture ();
	    continue;
	}

	/* zoom <xl> <yb> <fraction>
	 */
	if (strncmp (token, "zoom", 4) == 0) {
	 /* ptext ("zooming"); */
	    if (sscanf (scanline, "%s %f %f %f", token, &xc, &yc, &fraction) != 4) {
		PE "ERROR: zoom: no range\n");
		continue;
	    }
	    width  = (xrtb - xltb) * fraction;
	    height = (yttb - ybtb) * fraction;
	    center_x = xltb + ((xrtb - xltb) * xc);
	    center_y = ybtb + ((yttb - ybtb) * yc);

	    curs_w ((Coor) (center_x - (width / 2)), (Coor) (center_x + (width / 2)),
		    (Coor) (center_y - (height / 2)), (Coor) (center_y + (height / 2)));
	    picture ();
	    continue;
	}

	/* center_w <xl> <yb>
	 */
	if (strncmp (token, "center", 6) == 0) {
	 /* ptext ("centering"); */
	    if (sscanf (scanline, "%s %f %f", token, &xc, &yc) != 4) {
		PE "ERROR: center: no range\n");
		continue;
	    }
	    center_x = xltb + ((xrtb - xltb) * xc);
	    center_y = ybtb + ((yttb - ybtb) * yc);
	    center_w ((Coor) center_x, (Coor) center_y);
	    picture ();
	    continue;
	}

	if (strncmp (token, "expand", 6) == 0 || strncmp (token, "all_exp", 7) == 0) {
	 /* ptext ("expanding"); */
	    if (sscanf (scanline, "%s %d", token, &time) != 2) {
		PE "ERROR: exp: no level\n");
		continue;
	    }
	    expansion (time);
	    picture ();
	    continue;
	}

	/* set default expansion level
	 */
	if (strncmp (token, "default_expan", 13) == 0) {
	    if (sscanf (scanline, "%s %d", token, &time) != 2) {
		PE "ERROR: default_expansion_level: no level\n");
		continue;
	    }
	    time = Max (1, time);
	    time = Min (9, time);
	    Default_expansion_level = time;
	    continue;
	}

	/* set visible layer
	 */
	if (strncmp (token, "visible", 7) == 0) {
	 /* ptext ("visible"); */
	    if ((found = sscanf (scanline, "%s %s %s", token, name, label)) < 2) {
		PE "ERROR: visible: no layer name\n");
		continue;
	    }
	         if (strcmp (name, "terminals") == 0) lay = NR_lay;
	    else if (strcmp (name, "instances") == 0) lay = NR_lay + 2;
	    else if (strcmp (name, "bbox"     ) == 0) lay = NR_lay + 3;
	    else if (strcmp (name, "term_name") == 0) lay = NR_lay + 4;
	    else if (strcmp (name, "inst_name") == 0) lay = NR_lay + 6;
	    else lay = msk_nbr (name);
	    if (lay <= -1) {
		PE "Error in command 'visible'\n");
		continue;
	    }

	    if (found > 2) {
		if (strcmp (label, "on") == 0) {
		    if (!vis_arr[lay]) pict_arr[lay] = DRAW;
		    vis_arr[lay] = 1;
		    continue;
		}
		if (strcmp (label, "off") == 0) {
		    if (vis_arr[lay]) pict_arr[lay] = ERAS_DR;
		    vis_arr[lay] = 0;
		    continue;
		}
		PE "ERROR: visible: value '%s' is not 'on' nor 'off' (toggeling)\n", label);
	    }

	    /* toggle */
	    if (!vis_arr[lay]) {
		vis_arr[lay] = 1;
		pict_arr[lay] = DRAW;
	    }
	    else {
		vis_arr[lay] = 0;
		pict_arr[lay] = ERAS_DR;
	    }

	    /* requires redraw command */
	    continue;
	}

	/* set layer for drawing
	 */
	if (strncmp (token, "layer", 5) == 0) {
	 /* ptext ("layer"); */
	    if ((found = sscanf (scanline, "%s %s %s", token, name, label)) < 2) {
		PE "ERROR: layer: no layer name\n");
		continue;
	    }

	    lay = msk_nbr (name);

	    if (lay <= -1) {
		PE "Error in command 'layer'\n");
		continue;
	    }

	    if (found > 2) {
		if (strcmp (label, "on") == 0) {
		    def_arr[lay] = TRUE;
		    bulb (lay, TRUE);
		    continue;
		}
		if (strcmp (label, "off") == 0) {
		    def_arr[lay] = FALSE;
		    bulb (lay, FALSE);
		    continue;
		}
		PE "ERROR: layer: value '%s' is not 'on' nor 'off' (toggeling)\n", label);
	    }

	    /* toggle */
	    if (!vis_arr[lay])
		def_arr[lay] = TRUE;
	    else
		def_arr[lay] = FALSE;

	    bulb (lay, def_arr[lay]);
	    /* requires redraw command */
	    continue;
	}

	/* set drawing order
	 */
	if (strncmp (token, "drawing_order", 13) == 0) {
	    int index, noValues, *mask_order;

	    noValues = getLineValues (scanline, &mask_order, msk_nbr);

	    if (!drawing_order) MALLOCN (drawing_order, int, NR_lay);

	    for (index = 0; index < noValues; index++) {
		if ((lay = mask_order[index]) < 0) {
		    PE "ERROR: drawing_order: incorrect layer name\n");
		    continue;
		}
		if (dom_arr[lay] > 1) {
		    PE "ERROR: drawing_order: layer already specified\n");
		    continue;
		}
		drawing_order[NR_dom++] = lay;
		dom_arr[lay] += 2;
	    }
	    continue;
	}

	if (strncmp (token, "order", 5) == 0) {
	 /* ptext ("order"); */
	    if ((found = sscanf (scanline, "%s %s %d", token, name, &order)) < 3) {
		PE "ERROR: order: no layer name or order\n");
		continue;
	    }
	    if ((lay = msk_nbr (name)) < 0) {
		PE "ERROR: order: incorrect layer name '%s'\n", name);
		continue;
	    }
	    if (dom_arr[lay] > 1) {
		PE "ERROR: order: layer '%s' already specified\n", name);
		continue;
	    }
	    if (order < 0 || order >= NR_lay) {
		PE "ERROR: order: illegal order number: %d\n", order);
		continue;
	    }
	    if (!drawing_order) MALLOCN (drawing_order, int, NR_lay);
	    while (NR_dom <= order) drawing_order[NR_dom++] = -1;
	    if (drawing_order[order] >= 0) {
		PE "WARNING: order position %d multiply declared\n", order);
	    }
	    drawing_order[order] = lay;
	    dom_arr[lay] += 2;
	    continue;
	}

	/* append <xl> <xr> <yb> <yt>
	 */
	if (strncmp (token, "append", 6) == 0) {
	    Coor xl, xr, yb, yt;
	    if (sscanf (scanline, "%s %ld %ld %ld %ld", token, &xl, &xr, &yb, &yt) != 5) {
		PE "ERROR: append: no rectangle\n");
		continue;
	    }
	    /* flicker effect */
	    addel_cur (xl, xr, yb, yt, ADD);
	    picture ();
	    addel_cur (xl, xr, yb, yt, DELETE);
	    picture ();
	    addel_cur (xl, xr, yb, yt, ADD);
	    picture ();
	    continue;
	}

	/* delete <xl> <xr> <yb> <yt>
	 */
	if (strncmp (token, "delete", 6) == 0) {
	    Coor xl, xr, yb, yt;
	    if (sscanf (scanline, "%s %ld %ld %ld %ld", token, &xl, &xr, &yb, &yt) != 5) {
		PE "ERROR: delete: no rectangle\n");
		continue;
	    }
	    addel_cur (xl, xr, yb, yt, DELETE);
	    picture ();
	    continue;
	}

	/* draw dominant/non dominant
	 */
	if (strncmp (token, "dominant", 8) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: dominant: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) Draw_dominant = FALSE;
	    else Draw_dominant = TRUE;
	    continue;
	}

/* PATRICK */
	/* draw hashed/non hashed instances
	 */
	if (strncmp (token, "hashed", 4) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: hashed: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) Draw_hashed = FALSE;
	    else Draw_hashed = TRUE;
	    continue;
	}

	/* snap instances
	 */
	if (strncmp (token, "imagemode", 7) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: imagemode: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) ImageMode = FALSE;
	    else ImageMode = TRUE;
	    continue;
	}

	/* grid on or off
	 */
	if (strncmp (token, "grid", 4) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: grid: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) switch_grid (FALSE);
	    else switch_grid (TRUE);
	    continue;
	}

	/* tracker on or off
	 */
	if (strncmp (token, "tracker", 6) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: tracker: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) switch_tracker (FALSE);
	    else switch_tracker (TRUE);
	    continue;
	}

	if (strncmp (token, "backingstore", 7) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: backingstore: error\n");
		continue;
	    }
	    if (found >= 2 && strcmp (label, "off") == 0) BackingStore = FALSE;
	    else BackingStore = TRUE;
	    continue;
	}

	if (strncmp (token, "imagename", 7) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, name)) != 2) {
		PE "ERROR: imagename: no image name\n");
		continue;
	    }
	    if (strlen (name) > DM_MAXNAME) PE "ERROR: image: name too long\n");
	    else strcpy (ImageInstName, name);
	    continue;
	}

	/* via name
	 */
	if (strncmp (token, "via", 5) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, name)) != 2) {
		PE "ERROR: via: no name\n");
		continue;
	    }
	    if (strlen (name) > DM_MAXNAME) PE "ERROR: via: name too long\n");
	    else strcpy (ViaInstName, name);
	    continue;
	}

	/* weedout library
	 */
	if (strncmp (token, "weed", 4) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, name)) != 2) {
		PE "ERROR: weedout: no name\n");
		continue;
	    }
	    if (strlen (name) > DM_MAXNAME) {
		PE "ERROR: weedout: name too long\n");
		continue;
	    }
	    if (strlen (Weedout_lib) > 0) {
		PE "Warning: Weedout library '%s' overwritten by '%s'\n", Weedout_lib, name);
	    }
	    strcpy (Weedout_lib, name);
	    continue;
	}

	if (strncmp (token, "maxdraw", 5) == 0) {
	    if ((found = sscanf (scanline, "%s %d", token, &MaxDrawImage)) != 2) {
		PE "ERROR: maxdraw: no value\n");
		continue;
	    }
	    continue;
	}

	if (strncmp (token, "linkmasks", 8) == 0) {
	    int *mask_values;
            int noValues = getLineValues (scanline, &mask_values, msk_nbr);
	    link_masks (noValues, mask_values);
	    continue;
	}

	/* printing
	 */
	if (strncmp (token, "hardcopy_command", 8) == 0) { /* it's the print command */
	    pt = strchr (scanline, 'y');
	    while (!isspace (*pt)) ++pt;
	    while (isspace (*pt)) ++pt;
	    Print_command = strsave (pt);
	 /* fprintf (stderr, "Print command: '%s'\n", Print_command); */
	    continue;
	}
/* END PATRICK */

	if (strncmp (token, "wire_width_values", 17) == 0) {
	    int *wire_values;
	    int noValues = getLineValues (scanline, &wire_values, atoi);
	    set_wire_values (noValues, wire_values);
	    continue;
	}

	if (strncmp (token, "wire_width", 10) == 0) {
	    int wire_width = atoi (label);
	    set_wire_width (wire_width);
	    continue;
	}

	if (strncmp (token, "wire_extension", 14) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: wire_extension\n");
		continue;
	    }
	    if (found > 1) {
		if (strcmp (label, "on") == 0) {
		    set_wire_ext (TRUE);
		    continue;
		}
		if (strcmp (label, "off") == 0) {
		    set_wire_ext (FALSE);
		    continue;
		}
		PE "ERROR: wire_extension: wrong value '%s'\n", label);
	    }
	    else {
		set_wire_ext (TRUE);
	    }
	    continue;
	}

	if (strncmp (token, "no_grid_adjust", 14) == 0) {
	    no_grid_adjust ();
	    continue;
	}

	if (strncmp (token, "display_grid_width", 18) == 0) {
	    int grid_width = atoi (label);
	    no_grid_adjust ();
	    set_grid_width (grid_width);
	    continue;
	}

	if (strncmp (token, "display_grid_values", 19) == 0) {
	    int *grid_values;
            int noValues = getLineValues (scanline, &grid_values, atoi);
	    set_grid_values (noValues, grid_values);
	    continue;
	}

	if (strncmp (token, "snap_grid_width", 15) == 0) {
	    int snap_grid_width = atoi (label);
	    set_snap_grid_width (snap_grid_width);
	    continue;
	}

	if (strncmp (token, "snap_grid_values", 16) == 0) {
	    int *grid_values;
            int noValues = getLineValues (scanline, &grid_values, atoi);
	    set_sn_grid_values (noValues, grid_values);
	    continue;
	}

	if (strncmp (token, "snap_grid_offset", 16) == 0) {
	    int x_offset, y_offset;
	    sscanf (scanline, "%*s %d %d", &x_offset, &y_offset);
	    set_sn_grid_offset (x_offset, y_offset);
	    continue;
	}

	if (strncmp (token, "flat_expansion", 14) == 0) {
	    if ((found = sscanf (scanline, "%s %s", token, label)) < 1) {
		PE "ERROR: flat_expansion\n");
		continue;
	    }
	    if (found > 1) {
		if (strcmp (label, "on") == 0) {
		    set_flat_expansion (TRUE);
		    continue;
		}
		if (strcmp (label, "off") == 0) {
		    set_flat_expansion (FALSE);
		    continue;
		}
		PE "ERROR: flat_expansion: wrong value '%s'\n", label);
	    }
	    else {
		set_flat_expansion (TRUE);
	    }
	    continue;
	}

	if (strncmp (token, "disabled_layers", 15) == 0) {
	    int *mask_values;
	    int noValues = getLineValues (scanline, &mask_values, msk_nbr);
	    disable_masks (noValues, mask_values);
	    continue;
	}

	PE "WARNING: token not recognized: '%s'\n", token);
    }

    for (i = 0; i < NR_dom; ++i)
	if (drawing_order[i] < 0) {
	    order = i;
	    while (++order < NR_dom)
		if (drawing_order[order] >= 0)
		    drawing_order[i++] = drawing_order[order];
	    NR_dom = i;
	}

    if (outputfile) fclose (outputfile);
 /* ptext ("Ready!"); */
}

static int getLineValues (char *line, int **values, int (*theFunc)())
{
    int i, maxValues = 10;
    char *seperators = " \t\n\r";

    *values = (int *) calloc (maxValues, sizeof (int));

    /* get the (integer, base 10) values from line and put them in values */

    strtok (line, seperators); /* the token must be skipped first */

    for (i = 0; ; i++) {
	char *aValue;
	aValue = strtok (NULL, seperators); /* current value */
	if (!aValue) break;
	if (i >= maxValues) {
	    maxValues += 10;
	    *values = (int *) realloc (*values, maxValues * sizeof (int));
	}
	(*values)[i] = (*theFunc) (aValue);
    }
    return i;
}
