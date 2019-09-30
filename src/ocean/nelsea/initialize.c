/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
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

/*
 * Initailization routines.
 */
#include <sys/types.h>
#include <sys/stat.h>

#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/grid.h"
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/nelsea/globals.h"
#include "src/ocean/libseadif/sea_decl.h"
#include <signal.h>

extern int ydlineno;

static void fill_offset_tables (void);

/*
 * look-up table for offset values
 */
extern GRIDADRESSUNIT Xoff[HERE+1], Yoff[HERE+1], Zoff[HERE+1];

/*
 * look-up table for bit-patterns
 */
extern COREUNIT Pat_mask[HERE+1];

extern long
   ChipSize[2],            /* number of elementary core cells in x and y */
   Chip_num_layer,         /* number of metal layers to be used */
   DummyMaskNo,            /* mask no of dummy layer */
   *OverlayGridMapping[2], /* overlaymapping of gridpoints to layout coordinates */
   OverlayBounds[2][2],    /* boundaries of overlaymapping, index 1 = orient, index2 = L/R */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid image (in grid points) */
   LayoutRepitition[2];    /* repitionvector (dx, dy) of LAYOUT image (in lambda) */

extern char
   *DummyMaskName,         /* name of dummy (harmless) mask */
   ImageName[],            /* NELSIS name of image to be repeated */
   *layout_str,            /* string "layout" in string manager */
   *circuit_str,           /* string "circuit" in string manager */
   *in_core_str,
   *dummy_str,
   *not_in_core_str,
   *primitive_str,
   *not_written_str,
   *written_str;

extern SEADIFPTR seadif; /* the root */

extern MAPTABLEPTR maptable;

extern DM_PROJECT *projectkey;

/* This routine initializes all global variables
 * and allocates the end pointers.
 */
void init_variables ()
{
    /*
     * initialize the others
     */
    ChipSize[X] = ChipSize[Y] = 0;
    Chip_num_layer = 0;
    GridRepitition[X] = GridRepitition[Y] = 0;
    LayoutRepitition[X] = LayoutRepitition[Y] = 0;
    OverlayGridMapping[X] = NULL;
    OverlayGridMapping[Y] = NULL;
    OverlayBounds[X][L] = -1;
    OverlayBounds[Y][L] = -1;

    ImageName[0] = 0;
    maptable = NULL;
    DummyMaskName = NULL;
    DummyMaskNo = -1;  /* no dummy mask */

    /*
     * store strings 'layout and 'circuit' in string manager
     * The macros LAYOUT and CIRCUIT are declared in dmincl.h
     */
    layout_str = canonicstring ("layout");
    circuit_str = canonicstring ("circuit");
    primitive_str = canonicstring ("primitive");
    dummy_str = canonicstring ("$dummy");
    written_str = canonicstring ("written");
    not_written_str = canonicstring ("not_written");
    in_core_str = canonicstring ("in_core");
    not_in_core_str = canonicstring ("not_in_core");

    /*
     * no tree
     */
    seadif = NULL;

    fill_offset_tables();

#ifdef SIGALRM
    /*
     * ignore ALRM signal
     */
    signal (SIGALRM, SIG_IGN);
#endif
}

/* This routine opens the design rules file and calls the parser.
 */
void read_image_file ()
{
    STRING filepath = sdfimagefn();

    if (access ("seadif", F_OK)) mkdir ("seadif", 0755);

    if (!filepath) { /* FAILED */
      fprintf (stderr,
	"ERROR: I cannot open any image description file. I tried the\n"
	"       file \"%s\" but it did not work out too well...\n\n", filepath);
      fprintf (stderr, "%s",
	"       My analysis of your problem:\n"
	"        1) Are you in the right directory?\n"
	"        2) Fetch an 'image.seadif' into the seadif directory.\n"
	"        3) If you only want circuit: use the '-c' option\n"
	"           to prevent reading this file.\n");
      error (FATAL_ERROR, "Quitting because there is no image file");
    }

    if (!(freopen (filepath, "r", stdin))) {
      fprintf (stderr,
	"WARNING: Cannot open the seadif image description file '%s'.\n", filepath);
    }

    /* parse the image
     */
    if (verbose) {
	printf ("------ reading image description file '%s' ------\n", filepath);
	fflush (stdout);
    }
    ydlineno = 1;
    ydparse ();
}

/* This routine writes a seadif file.
 */
void write_seadif_file ()
{
    FILE *fp;

    if (verbose) {
	printf ("------ writing seadif file 'lib.seadif' ------\n");
	fflush (stdout);
    }

    if (!(fp = fopen ("lib.seadif", "w"))) {
	fprintf (stderr, "WARNING (write_seadif): cannot open file\n");
	return;
    }

    setdumpstyle (1); /* dump in pascal style */
    setcomments (1);  /* extra info */

    dumpdb (fp, seadif);
    fclose (fp);
}

/* This routine fills the basic tables
 * which are fixed (that is, could be treated as global variables).
 */
static void fill_offset_tables ()
{
    register int i;

    /*
     * for L, B, R, T, U and D
     */
    for_all_offsets (i)
    {
	Xoff[i] = Yoff[i] = Zoff[i] = 0;
	Pat_mask[i] = (COREUNIT) (1 << i);
    }

    Xoff[HERE] = Yoff[HERE] = Zoff[HERE] = 0;
    Xoff[L] = -1; Xoff[R] = 1;
    Yoff[B] = -1; Yoff[T] = 1;
    Zoff[D] = -1; Zoff[U] = 1;
}

/* this routine sets all global variables which should be set by main
 */
void initialize_globals ()
{
   int i;

   Verbose_parse = FALSE;
   extra_verbose = TRUE;
   verbose = TRUE;
   Flat_vias = TRUE;          /* TRUE to print the vias as boxes instead of mc\'s */
   Write_Floorplan = FALSE;   /* TRUE to write also a floorplan of the cell... */
   Auto_move = -1;            /* TRUE to move the cell to the leftbottom corner */
   Hierarchical_fish = FALSE; /* TRUE to perform hierarchical fishing */
   /* NoAliases = FALSE;         make or use seadif aliases during conversion */
   /* TEMPORARY: no use of aliases */
   NoAliases = TRUE;          /* make or use seadif aliases during conversion */
   Seadif_open = FALSE;       /* True if the seadif lib has been opened */
   Nelsis_open = FALSE;       /* True if the nelsis has been opened */
   No_sdf_write = FALSE;      /* to prevent any writing into sdf */

   Technologystring = cs ("Unknown_process");
   Authorstring     = cs ("Unknown_author");
   default_sdf_lib  = cs ("oplib");
   this_sdf_lib     = cs ("tmplib");

   /* what should be read/converted */
   for (i = 0; i != MAXLAYERS; i++)
      LayerRead[i] = ViaRead[i] = TRUE;   /* default read everything */
   TermRead = MCRead = TRUE;
}

/* Return the file name part of an absolute pathname
 * (like the program "basename").
 */
char *bname (char *s)
{
    char *p;
    p = strrchr (s, (int) '/');
    if (p) return p+1;
    return s;
}
