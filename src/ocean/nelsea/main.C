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
 *  NELSEA/FISH/GHOTI/SEA
 *
 *  Nelsis interface to the Seadif base tools.
 */

#include <sys/types.h>
#include "src/ocean/nelsea/seadifGraph.h"
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/libseadif/sealibio.h"
#include <string.h>
#include <ctype.h>
#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/libseadif/sdferrors.h"
#include <new>		// set_new_handler()

using namespace std;

static void setup_dmerror (void);
static void nelsea_main (int, char **);
static void fish_main (int, char **);
static void ghoti_main (int, char **);
static void sea_main (int, char **);
static void print_fish_usage (char *);
static void print_ghoti_usage (char *);
static void print_nelsea_usage (char *);
static void print_sea_usage (char *);
static void parse_readoptions (char *);
static void my_new_handler (void);

#ifndef COMPILE_DATE_STRING
#define COMPILE_DATE_STRING "<compile date not available>"
#endif

const char *thedate = COMPILE_DATE_STRING;
char *argv0;

/*
 * import. If we compile main.c with a C++ compiler, we have to specify
 *         that externals have C-type linkage.
 */

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN extern
#endif

EXTERN int
   Auto_move,            /* TRUE to move the cell to the leftbottom corner */
   Flat_vias,            /* TRUE to print the vias as boxes instead of mc's */
   Hierarchical_fish,    /* TRUE to perform hierarchical fishing */
   NoAliases,            /* FALSE to use seadif aliases for mapping */
   No_sdf_write,         /* to prevent any writing into sdf */
   Nelsis_open,          /* True if the nelsis has been opened */
   Seadif_open,          /* True if the seadif lib has been opened */
   Verbose_parse,
   Write_Floorplan,      /* TRUE to write a floorplan of the cell... */
   extra_verbose,
   verbose;

EXTERN long
   ChipSize[2],          /* size for empty array */
   LayerRead[MAXLAYERS], /* array of booleans: TRUE to read the indicated layer */
   ViaRead[MAXLAYERS],   /* array of booleans: TRUE to read the indicated via */
   TermRead,             /* boolean: true to read terminals in seadif */
   MCRead,               /* boolean: true to read mc's into seadif */
   GridRepitition[2];    /* repitionvector (dx, dy) of grid core image (in grid points) */

EXTERN char
   *default_sdf_lib,
   *this_sdf_lib,
   *Technologystring,      /* name of the process */
   *Authorstring,          /* Author field */
   *in_core_str,
   *layout_str,           /* string "layout" in string manager */
   *circuit_str,          /* string "circuit" in string manager */
   *optarg,		  /* used with getopt() */
   *ThisImage;		  /* name identifier of image */

extern FILE *ErrorMsgFile; /* file where dmError writes its messages */
extern int DmErrorIsFatal; /* 0 if a call do dmError should return */
EXTERN int optind, opterr, optopt; /* used with getopt() */

void invalid_option (int i)
{
   if (i == '?') fprintf (stderr, "\n%s: invalid option -- '%c'", argv0, optopt > 32 ? optopt : i);
   fprintf (stderr, "\n%s: illegal argument, use -h to print help\n\n", argv0);
   exit (SDFERROR_CALL);
}

/**********************************
 * MAIN of fish, ghoti, nelsea, sea
 */
int main (int argc, char **argv)
{
   set_new_handler (&my_new_handler); // type message if operator new() fails

   // available options are: DM_NEW_MALLOC, DM_NEW_STRINGSAVE
   // dmMemoryOptions(DM_NEW_STRINGSAVE);
   setup_dmerror();

   argv0 = bname (argv[0]);
   opterr = 0; /* prevent getopt error message */

   /*
    * Which program are we calling ?
    */
   if (!strcmp (argv0, "fish" )) fish_main  (argc, argv);
   if (!strcmp (argv0, "ghoti")) ghoti_main (argc, argv);
   if (!strcmp (argv0, "sea"  )) sea_main   (argc, argv);
   nelsea_main (argc, argv);

   return 0;
}

static void do_echo (char *str, char *file)
{
    FILE *fp = fopen (file, "a");
    if (fp) {
	fprintf (fp, "%s\n", str);
	fclose (fp);
    }
}

static void do_empty (char *file)
{
    FILE *fp = fopen (file, "w");
    if (fp) fclose (fp);
}

/* This function sets up the behavior of dmError(), refer to nelsis.c */
static void setup_dmerror ()
{
   ErrorMsgFile = stderr;
   DmErrorIsFatal = TRUE;
}

/********************************************
 * Main of nelsea: nelsis -> seadif and
 *                 seadif -> nelsis converter
 */
static void print_nelsea_usage (char *progname)
{
    printf ("\n"
	"* * * Welcome to NELSEA (compiled: %s) * * *\n"
	"Nelsis <--> Seadif convertor\n\n"
	"Usage: %s [-options] [<cell_name>]\n\n", thedate, progname);
    printf ("%s",
	"argument:\n"
	" <cell_name>    Nelsis cell to be converted\n"
	" [no argument]  Convert everything in celllist (implies no '-r')\n"
	"options to control the direction of the conversion:\n"
	" [default]      Convert from Nelsis into Seadif\n"
	" -r             Reverse conversion: convert from Seadif into Nelsis\n"
	"options to control what should be converted:\n"
	" -L             Convert layout view only (default: do both)\n"
	" -C             Convert circuit view only (default: do both)\n"
	" -m <map_file>  Convert the list in <mapfile> (implies no arguments)\n"
	" -f <sdf_fun>   Set Seadif function name (default: <cell_name>)\n"
	" -c <sdf_cir>   Set Seadif circuit name (default: <cell_name>)\n"
	" -l <sdf_lay>   Set Seadif layout name (default: <cell_name>)\n"
	" -a             Do not make or use Seadif cell Aliases\n"
	" -H             Convert <cell_name> only (default: hierarchical)\n"
	"options related to Nelsis:\n"
	" -i             Make the Nelsis cell without the basic image\n"
	" -v             Write vias as model calls instead of boxes\n"
	" -F             Write placement also in floorplan view\n"
	" -x <x_size>    Force horizontal array size to x_size (>= 1)\n"
	" -y <y_size>    Force vertical array size to y_size (>= 1)\n"
	" -E <list>      Exclude the objects in <list> from being read. Example:\n"
	"                 -E tmv13  leaves terminals, MC's vias and layers 1 and 3\n"
	"misc. options:\n"
	" -p             Print Seadif environment for project and quit\n"
	" -M             Create an empty template mapfile only\n"
	" -d <libname>   Seadif library name (default: 'Oplib')\n"
	" -V             Verbose parsing of image file: print all unknown keywords\n"
	" -h             Print this list and quit\n"
	" -q             Quiet option: print nothing\n\n");
}

void nelsea_main (int argc, char **argv)
{
   int i, print_environment, call_image, do_layout, do_circuit,
      process_mapfile, non_hierarchical, everything_in_mapfile, nelsis_to_seadif;
   long xsize, ysize;     /* force size of image */
   char *input_cell_name, *layout_name, *circuit_name, *function_name, *map_file;
   MAPTABLEPTR cir_map, lay_map;

   initialize_globals(); /* initalize opions */

   nelsis_to_seadif = 1; /* default: convert nelsis into seadif */
   map_file = cs ((char*)"seadif/mapfile");
   input_cell_name = NULL;
   everything_in_mapfile = 0;
   non_hierarchical = 0; /* read hierarchically */
   call_image = 1;
   xsize = ysize = -1;
   lay_map = cir_map = NULL;
   process_mapfile = 0;
   print_environment = 0;
   do_circuit = do_layout = 1;
   layout_name = circuit_name = function_name = NULL;

   /*
    * Parse options
    */
   while ((i = getopt (argc, argv, "vFHaiVMrLCpl:c:f:E:x:y:m:d:hq")) >= 0)
   {
      switch (i)
      {
      case 'l': /* give layout name of cell */
	 layout_name = cs (optarg);
	 break;
      case 'c': /* give circuit name of cell */
	 circuit_name = cs (optarg);
	 break;
      case 'f': /* give function name of cell */
	 function_name = cs (optarg);
	 break;
      case 'E':   /* exclude specific objects from conversion */
	 parse_readoptions (optarg);
	 break;
      case 'v':   /* model calls instead of boxes for vias */
	 Flat_vias = FALSE;
	 break;
      case 'F':   /* make floorplan cell */
	 Write_Floorplan = TRUE;
	 break;
      case 'a':   /* no aliases */
	 NoAliases = TRUE;
	 break;
      case 'H':   /* non-hierarchical */
	 non_hierarchical = 1;
	 break;
      case 'x':   /* number of repetitions in the x-direction */
	 if ((xsize = atol (optarg)) < 1) xsize = 1;
	 break;
      case 'y':   /* number of repetitions in the y-direction */
	 if ((ysize = atol (optarg)) < 1) ysize = 1;
	 break;
      case 'V':   /* parse verbose */
	 Verbose_parse = TRUE;
	 break;
      case 'r':   /* reverse conversion */
	 nelsis_to_seadif = 0;
	 break;
      case 'L':   /* layout only */
	 do_circuit = 0;
	 do_layout = 1;
	 break;
      case 'C':   /* circuit only */
	 do_circuit = 1;
	 do_layout = 0;
	 break;
      case 'M':   /* create empty mapfile only */
	 process_mapfile = 1;
	 break;
      case 'p':   /* print env */
	 print_environment = 1;
	 break;
      case 'm':   /* input map file name */
	 everything_in_mapfile = 1;
	 map_file = cs (optarg);
	 break;
      case 'i':  /* no call of image */
	 call_image = 0;
	 break;
      case 'd':  /* default seadif library name */
	 forgetstring (default_sdf_lib);
	 default_sdf_lib = cs (optarg);
	 break;
      case 'h':   /* print help */
	 print_nelsea_usage (argv0);
	 exit (0);
	 break;
      case 'q':
	 verbose = 0;
	 break;
      default:
	 invalid_option (i); /* nelsea */
      }
   }

   init_variables(); /* initialize global variables */

   /*
    * only manage mapfile?
    */
   if (process_mapfile)
   {
      if (verbose) {
	 printf ("You specified the '-M' -option: only creating an empty mapfile.\n");
	 fflush (stdout);
      }
      init_variables();
      add_nelsis_primitives();
      write_map_table (map_file);
      exit (0);
   }

   if (print_environment) {
      if ((i = open_seadif (TRUE, print_environment)) != SDF_NOERROR) exit (i);
      exit (0);
   }

   /*
    * some input checking....
    */
   if (!do_circuit && !do_layout) {
      fprintf (stderr, "WARNING: What do you expect me to do if both '-l' and '-c' are specified?\n");
      fprintf (stderr, "Use 'nelsea -h' to list the options\n");
      myfatal (SDFERROR_CALL);
   }

   /*
    * read arguments
    */
   if (optind <= argc - 1)
   {
      if (everything_in_mapfile) {
	 fprintf (stderr, "ERROR: '-m' option may only be specified without arguments.\n");
	 myfatal (SDFERROR_CALL);
      }

      /* argument: cell specification */
      if (strlen (argv[optind]) > DM_MAXNAME) { /* name too long */
	 fprintf (stderr, "ERROR: cell_name '%s' too long for nelsis (max %d)\n", argv[optind], DM_MAXNAME);
	 myfatal (SDFERROR_CALL);
      }

      /* copy name */
      input_cell_name = cs (argv[optind]);

      if (optind < argc - 1) {
	 fprintf (stderr, "WARNING: anything after argument '%s' was ignored.\n", argv[optind]);
	 fprintf (stderr, "(use one argument only)\n");
      }
   } else {  /* no argument found */
      if (everything_in_mapfile && !nelsis_to_seadif) {
	 fprintf (stderr, "ERROR: '-e' option may only be specified without the '-r' option\n");
	 myfatal (SDFERROR_CALL);
      }

      if (non_hierarchical) {
	 fprintf (stderr, "ERROR: '-H' (non-hierarchical) option may only be specified for one specific cell\n");
	 myfatal (SDFERROR_CALL);
      }

      if (verbose) {
         if (everything_in_mapfile)
	    printf ("------ no name: converting everything in mapfile ------\n");
         else if (nelsis_to_seadif)
	    printf ("------ converting everything in nelsis celllist to seadif ------\n");
	 else
	    printf ("------ converting everything in seadif library to nelsis ----\n");
      }
   }

   /*
    * open nelsis, read image.seadif...
    */
   init_nelsis (argv0, nelsis_to_seadif, do_layout);

   /*
    * now open seadif
    */
   if ((i = open_seadif (nelsis_to_seadif ? FALSE : TRUE, print_environment)) != SDF_NOERROR)
      myfatal (i);

   /*
    * read file containing map table (if requested)
    */
   if (everything_in_mapfile) read_map_table (map_file);

   add_nelsis_primitives();

   if (nelsis_to_seadif)
   {
      /*
       *  +++++++  NELSIS ---> SEADIF  +++++++
       */

      /*
       * the actual conversion
       */
      if (!input_cell_name)
      { /* convert all */
	 if (!everything_in_mapfile)
         {
	    if (do_circuit) readallnelsiscellsincelllist (circuit_str);
	    if (do_layout) readallnelsiscellsincelllist (layout_str);
         }
	 else
         {
	    if (do_circuit) readallnelsiscellsinmaptable (circuit_str);
	    if (do_layout) readallnelsiscellsinmaptable (layout_str);
         }
      }
      else
      { /* read one specific cell */
	 if (do_circuit)
         {
	    /* create map table extry with proper names... */
	    make_map (circuit_str, input_cell_name, function_name, circuit_name, layout_name);

	    cir_map = read_nelsis_cell (circuit_str, input_cell_name, non_hierarchical);
         }
	 if (do_layout)
         {
	    /* create map table extry with proper names... */
	    make_map (layout_str, input_cell_name, function_name, circuit_name, layout_name);
	    lay_map = read_nelsis_cell (layout_str, input_cell_name, non_hierarchical);
         }

	 if ((cir_map && cir_map->internalstatus != in_core_str) ||
	     (lay_map && lay_map->internalstatus != in_core_str))
         {
	    fprintf (stderr, "ERROR: Cell could not be found (nothing written)\n");
	    sdfclose();
	    close_nelsis();
	    myfatal (SDFERROR_NOT_FOUND);
         }
      }

      /*
       * write the seadif struct
       */
      write_to_seadif();
   } else {
      /*
       *  +++++++  SEADIF ---> NELSIS  +++++++
       */
      if (!input_cell_name)
      { /* convert all */
	 if (everything_in_mapfile)
	 {
	    if (do_circuit) readallseadifcellsinmaptable (circuit_str);
	    if (do_layout) readallseadifcellsinmaptable (layout_str);
	 }
	 else
	 { /* read everything in the current lib */
	    if (do_circuit) readallseadifcellsinlibrary (circuit_str);
	    if (do_layout) readallseadifcellsinlibrary (layout_str);
	 }
      }
      else
      { /* read one specific cell */
	 if (do_circuit)
	 {
	    /* create map table extry with proper names... */
	    make_map (circuit_str, input_cell_name, function_name, circuit_name, layout_name);
	    cir_map = read_seadif_cell (circuit_str, input_cell_name, non_hierarchical);
	    if (cir_map->internalstatus != in_core_str)
	    {
	       fprintf (stderr, "ERROR: sdf circuit '%s(%s(%s)))' (nelsis '%s') was not found\n",
		       cir_map->circuit, cir_map->function, cir_map->library, cir_map->cell);
	    }
	 }
	 if (do_layout)
	 {
	    /* create map table extry with proper names... */
	    make_map (layout_str, input_cell_name, function_name, circuit_name, layout_name);
	    lay_map = read_seadif_cell (layout_str, input_cell_name, non_hierarchical);
	    if (lay_map->internalstatus != in_core_str)
	    {
	       fprintf (stderr, "ERROR: sdf layout '%s(%s(%s(%s))))' (nelsis '%s') was not found\n",
		       lay_map->layout, lay_map->circuit, lay_map->function, lay_map->library, lay_map->cell);
	    }
	 }

	 if ((cir_map && cir_map->internalstatus == in_core_str) ||
	     (lay_map && lay_map->internalstatus == in_core_str))
	 {
	    ; /* there is something to be written.. */
	 }
	 else {
	    fprintf (stderr, "ERROR: Cell could not be found (nothing written)\n");
	    sdfclose();
	    close_nelsis();
	    myfatal (SDFERROR_NOT_FOUND);
	 }
      }
      /*
       * write into nelsis
       */
      if (do_circuit) write_nelsis_circuit();
      if (do_layout) write_nelsis_layout (call_image, TRUE, xsize, ysize);
   }

   /* close databases */
   sdfclose();
   close_nelsis();

   /* write new map file */
   write_map_table ((char*)"seadif/mapfile");

   if (verbose) {
      printf ("------ nelsea: task completed ------\n");
      fflush (stdout);
   }

   exit (0);
}

/************************************
 * This is the main routine for fish.
 */
static void print_fish_usage (char *progname)
{
    printf ("\n"
	"* * * Welcome to FISH (compiled: %s) * * *\n"
	"Nelsis <--> Nelsis purifier for Sea-of-Gates\n\n"
	"Usage: %s [-options] [<cell_name>]\n\n", thedate, progname);
    printf ("%s",
	"arguments/options to control what should be purified:\n"
	" <cell_name>    The argument: Nelsis cell to be purified\n"
	" [no argument]  Make empty image array of given size (implies '-o')\n"
	" -c             Purify circuit cell instead of layout cell\n"
	" -H             Hierarchical fish: fish all son-cells too\n"
	"options to control how to write the cell back into Nelsis:\n"
	" -i             Write the cell without the image\n"
	" -x <x_size>    Force horizontal array size to x_size (>= 1)\n"
	" -y <y_size>    Force vertical array size to y_size (>= 1)\n"
	" -v             Write vias as model calls instead of boxes\n"
	" -a             Perform automatic move to origin of cell\n"
	" -E <list>      Exclude the objects in <list> from being fished. Example:\n"
	"                 -E tmv13  leaves terminals, MC's vias and layers 1 and 3\n"
	"misc. options:\n"
	" -V             Verbose parsing of image file: print all unknown keywords\n"
	" -o <out_name>  Write result to specified cell name (default: overwrite input cell)\n"
	" -q             Quiet option: print nothing except errors\n"
	" -h             Print this list and quit\n\n");
}

static void fish_main (int argc, char **argv)
{
   int i, do_layout, do_circuit, call_image;
   long xsize, ysize;     /* force size of image */
   char input_cell_name[DM_MAXNAME+1], *output_cell_name;
   MAPTABLEPTR cir_map, lay_map;

   initialize_globals(); /* initalize opions */

   extra_verbose = 0;	/* prevent printing of warning for references to unknows childs */
   Hierarchical_fish = FALSE;      /* default: fish only current cell */
   do_layout = 1;	/* default: only do layout */
   do_circuit = 0;	/* default: no circuit */
   xsize = ysize = -1;             /* auto-determine the size of the image */
   call_image = TRUE;              /* do call the image */
   *input_cell_name = 0;
   output_cell_name = NULL;
   cir_map = lay_map = 0;

   /*
    * Parse options
    */
   while ((i = getopt (argc, argv, "E:HaviVco:x:y:hq")) >= 0)
   {
      switch (i)
      {
      case 'H':   /* Enable hierachical fishing */
         Hierarchical_fish = TRUE;
	 break;
      case 'E':   /* exclude specific objects from conversion/fish */
         parse_readoptions (optarg);
         break;
      case 'a':   /* enable auto-origin  move during reading in NELSIS */
         Auto_move = TRUE;
         break;
      case 'v':   /* model calls instead of boxes for vias */
         Flat_vias = FALSE;
         break;
      case 'o':   /* output cell name */
         output_cell_name = cs (optarg);
         break;
      case 'x':   /* number of repetitions in the x-direction */
	 if ((xsize = atol (optarg)) < 1) xsize = 1;
         break;
      case 'y':   /* number of repetitions in the y-direction */
	 if ((ysize = atol (optarg)) < 1) ysize = 1;
         break;
      case 'V':   /* parse verbose */
         Verbose_parse = TRUE;
         break;
      case 'c':   /* circuit only */
         do_circuit = 1;
         do_layout = 0;
         break;
      case 'i':  /* no call of image */
         call_image = FALSE;
         break;
      case 'h':   /* print help */
         print_fish_usage (argv0);
         exit (0);
         break;
      case 'q':
         verbose = 0;
         break;
      default:
	 invalid_option (i); /* fish */
      }
   }

   /*
    * read arguments
    */
   if (optind <= argc - 1)
   {
      /* argument: cell specification */
      if (strlen (argv[optind]) > DM_MAXNAME) { /* name too long */
	 fprintf (stderr, "ERROR: cell_name '%s' is too long for nelsis (max %d)\n", argv[optind], DM_MAXNAME);
	 exit (SDFERROR_CALL);
      }

      /* copy name */
      strcpy (input_cell_name, argv[optind]);

      if (optind < argc - 1) {
	 fprintf (stderr, "WARNING: anything after argument '%s' was ignored.\n", argv[optind]);
	 fprintf (stderr, "(use one argument only)\n");
      }
   }
   else /* no argument found */
   {
      if (!output_cell_name) {
	 fprintf (stderr, "ERROR: You must specify an output cell name to make an\n");
	 fprintf (stderr, "       empty array (use '-o <cell_name>)\n");
	 exit (SDFERROR_CALL);
      }

      if (!do_layout) {
	 fprintf (stderr, "ERROR: cannot make empty array in circuit view\n");
	 close_nelsis();
	 exit (SDFERROR_CALL);
      }

/*
    if (xsize == -1) xsize = 1;
    if (ysize == -1) ysize = 20;
*/
      if (verbose) printf ("------ no cell name: making empty array ------\n");
   }

   if (Hierarchical_fish && Auto_move == TRUE) {
      fprintf (stderr, "WARNING: -a (automove) not allowed with '-H' (hierarchical fish)\n");
      Auto_move = FALSE;
   }

   /*
    * initialize global variables
    */
   init_variables();

   /*
    * open nelsis, read designrules and image
    */
   init_nelsis (argv0, FALSE, do_layout);

   if (*input_cell_name) /* cell was specified */
   {
      /*
       * read nelsis
       */
      if (do_circuit) {
	 add_nelsis_primitives();
	 cir_map = read_nelsis_cell (circuit_str, cs (input_cell_name), TRUE);
      }
      if (do_layout)
	 lay_map = read_nelsis_cell (layout_str, cs (input_cell_name), TRUE);

      /*
       * write nelsis
       */
      if (cir_map && cir_map->internalstatus == in_core_str)
      {
	 if (output_cell_name) { /* give new output cell name */
	    forgetstring (cir_map->cell);
	    cir_map->cell = cs (output_cell_name);
	 }
	 write_nelsis_circuit();
      }

      if (lay_map && lay_map->internalstatus == in_core_str)
      {
	 if (output_cell_name) { /* give new output cell name */
	    forgetstring (lay_map->cell);
	    lay_map->cell = cs (output_cell_name);
	 }
	 write_nelsis_layout (call_image, FALSE, xsize, ysize);
      }
   } else { /* make empty array */
      /* correct for unspecified empty grid size */
      if (ChipSize[X] <= 0) ChipSize[X] = (20 / GridRepitition[X]) + 1;
      if (ChipSize[Y] <= 0) ChipSize[Y] = (31 / GridRepitition[Y]) + 1;

      /* set size of grid */
      if (xsize == -1) xsize = ChipSize[X];
      if (ysize == -1) ysize = ChipSize[Y];

      if (verbose && strcmp (ThisImage, "fishbone") == 0)
	 printf ("------ no cell name: making empty %ld x %ld array ------\n", xsize, ysize);
      if (xsize <= 0) {
	 fprintf (stderr, "ERROR: '-x <size>' must be specified to make an empty array\n");
	 close_nelsis();
	 myfatal (SDFERROR_CALL);
      }
      if (ysize <= 0) {
	 fprintf (stderr, "ERROR: '-y <size>' must be specified to make an empty array\n");
	 close_nelsis();
	 myfatal (SDFERROR_CALL);
      }

      write_empty_image (output_cell_name, xsize, ysize);
   }

   close_nelsis();

   exit (0);
}

static void print_memory_usage (int c)
{
   static char *sbrk_start;
   char *sbrk_pos = (char*) sbrk (0);

   if (!c) sbrk_start = sbrk_pos;
   else printf ("MEMORY: used %ld bytes (%c)\n", (long)(sbrk_pos - sbrk_start), c);
}

/*************************************
 * This is the main routine for ghoti.
 */
static void ghoti_main (int argc, char **argv)
{
   int i, do_powerFix = TRUE, do_removeSerPar = 0,
      ghotiCheck = /* the default is to check for everything */
      GHOTI_CK_CMOS + GHOTI_CK_PASSIVE + GHOTI_CK_ISOLATION;
   int numberOfPreProcSteps = 2; /* default # preprocess steps for ghoti */
   int print_hash_stats = 0; /* option to print statistics of tha hash tables */
   int debug_printGraph = 0; /* obscured option for debug purposes */
   int debug_noWrite = 0; /* obscured option for debug purposes */
   int checkForFirstCapital = 1; /* require first char of name to be capital */
   int ghoti_print_memory_usage = 0;
   char input_cell_name[DM_MAXNAME+1], *output_cell_name = NULL;
   MAPTABLEPTR cir_map;

   initialize_globals(); /* initalize opions */

   No_sdf_write = TRUE;		  /* Do not write anything into seadif */

   *input_cell_name = 0;
   cir_map = 0;

   while ((i = getopt (argc, argv, "hqPcpusizrn:o:CDWM")) >= 0)
   {
      switch (i)
      {
      case 'h':   /* print help */
	 print_ghoti_usage (argv0);
	 exit (0);
	 break;
      case 'q':
	 verbose = 0;
	 break;
      case 'c': /* do not remove badly connected cmos transistors */
	 ghotiCheck &= ~GHOTI_CK_CMOS;
	 break;
      case 'p': /* do not remove badly connected resistors and capacitors */
	 ghotiCheck &= ~GHOTI_CK_PASSIVE;
	 break;
      case 'i': /* do not remove isolation transistors */
	 ghotiCheck &= ~GHOTI_CK_ISOLATION;
	 break;
      case 'u': /* also remove totally unconnected instances of any sort */
	 ghotiCheck |= GHOTI_CK_UNCONNECTED;
	 break;
      case 's': /* also remove partially unconnected instances of any sort */
	 ghotiCheck |= GHOTI_CK_SEMICONNECTED;
	 break;
      case 'P':
	 do_powerFix = 0;
	 break;
      case 'n':
	 numberOfPreProcSteps = atoi (optarg);
	 break;
      case 'z':
	 print_hash_stats = TRUE;
	 break;
      case 'r':
	 do_removeSerPar = TRUE;
	 break;
      case 'o':
	 output_cell_name = cs (optarg);
	 break;
      case 'D':
	 debug_printGraph = TRUE;
	 break;
      case 'W':
	 debug_noWrite = TRUE;
	 break;
      case 'C':
	 checkForFirstCapital = 0;
	 break;
      case 'M':
	 ghoti_print_memory_usage = 1;
	 break;
      default:
	 invalid_option (i); /* ghoti */
      }
   }

   /*
    * read arguments
    */
   if (optind <= argc - 1)
   {
      /* argument: cell specification */
      if (strlen (argv[optind]) > DM_MAXNAME) { /* name too long */
	 fprintf (stderr, "ERROR: cell_name '%s' is too long for nelsis (max %d)\n", argv[optind], DM_MAXNAME);
	 exit (SDFERROR_CALL);
      }

      /* copy name */
      strcpy (input_cell_name, argv[optind]);

      if (optind < argc - 1) {
	 fprintf (stderr, "WARNING: anything after argument '%s' was ignored.\n", argv[optind]);
	 fprintf (stderr, "         This program only takes 1 argument, a circuit name.\n");
      }
   }
   else {  /* no argument found */
      fprintf (stderr, "ERROR: You forgot to specify a circuit name!\n");
      exit (SDFERROR_CALL);
   }

   if (checkForFirstCapital && !isupper (input_cell_name[0])) {
      fprintf (stderr, "ERROR: the name \"%s\" does not start with a capital.\n", input_cell_name);
      fprintf (stderr, "       (Option -C makes me proceed anyway...)\n");
      exit (SDFERROR_CALL);
   }

   if (ghoti_print_memory_usage) print_memory_usage (0);

   /* initialize global variables */
   init_variables();

   if (ghoti_print_memory_usage) print_memory_usage ('a');

   /* open nelsis for read/write, only read image description if we must
    * reduce series connections (Because it needs spice parameter "LD"):
    */
   init_nelsis (argv0, FALSE, do_removeSerPar);

   if (ghoti_print_memory_usage) print_memory_usage ('b');

   /* ghoti expects the seadif data base to be opened (it reads the primitives) */
   if ((i = open_seadif (TRUE, FALSE)) != SDF_NOERROR) myfatal (i);

   if (ghoti_print_memory_usage) print_memory_usage ('c');

   /* read nelsis */
   if (ghotiCheck & (GHOTI_CK_CMOS | GHOTI_CK_PASSIVE | GHOTI_CK_ISOLATION) || do_removeSerPar)
      /* we are going to need the primitives....: */
      add_nelsis_primitives();

   if (ghoti_print_memory_usage) print_memory_usage ('d');

   /* We need to trick read_nelsis_cell into thinking that Seadif is closed... */
   cir_map = read_nelsis_cell (circuit_str, cs (input_cell_name), TRUE);

   if (ghoti_print_memory_usage) print_memory_usage ('0');

   if (!cir_map || cir_map->internalstatus != in_core_str)
      sdfexit (SDFERROR_SEADIF); // circuit not found...

   if (print_hash_stats) {
      tin_statistics();
      csi_statistics();
   }

   if (do_powerFix) {
      if (verbose) {
	 printf ("------ joining suspected power nets ------\n");
	 fflush (stdout);
      }
      powerFix (cir_map->circuitstruct);
   }

   if (verbose) {
      printf ("------ building the seadif graph ------\n");
      fflush (stdout);
   }
   // Now it's time to build a C++ graph that represents the circuit:
   sdfGraphDescriptor gdesc ((char*)"ghoti", SetCurrentGraph); // Init a new graph

   // Convert seadif circuit to graph, but use ``gr'' objects in stead of ``g'':
   grCircuit *gcirc = (grCircuit *) buildCircuitGraph (cir_map->circuitstruct,
			new_grCircuit, new_grCirInst, new_grNet, new_grCirPortRef);

   if (ghoti_print_memory_usage) print_memory_usage ('1');

   if (debug_printGraph) gdesc.print();

   if (ghotiCheck & (GHOTI_CK_CMOS | GHOTI_CK_PASSIVE | GHOTI_CK_ISOLATION) || do_removeSerPar)
      // Initialize the static members of the primCirc class...
      if (!getPrimitiveElements()) sdfexit (SDFERROR_SEADIF);

   if (ghotiCheck) {
      if (verbose) {
	 printf ("------ running ghoti ------\n");
	 fflush (stdout);
      }
      ghoti (gcirc, ghotiCheck, numberOfPreProcSteps);
   }

   if (ghoti_print_memory_usage) print_memory_usage ('2');

   if (do_removeSerPar) {
      if (verbose) {
	 printf ("------ reducing series/parallel transistors ------\n");
	 fflush (stdout);
      }
      removeSerPar (gcirc);
      if (debug_printGraph) gdesc.print();
   }

   if (ghoti_print_memory_usage) print_memory_usage ('3');

   /* write nelsis */
   if (!debug_noWrite && cir_map && cir_map->internalstatus == in_core_str)
   {
      if (output_cell_name) {
	 fs (cir_map->cell);
	 cir_map->cell = cs (output_cell_name);
      }
      write_nelsis_circuit();
   }

   sdfclose();
   close_nelsis();

   if (ghoti_print_memory_usage) print_memory_usage ('4');

   exit (0);
}

static void print_ghoti_usage (char *progname)
{
    printf ("\n"
	"* * * Welcome to GHOTI (compiled: %s) * * *\n"
	"Nelsis <--> Nelsis circuit purifier for extracted Sea-of-Gates layouts\n\n"
	"Usage: %s [-options] <cell_name>\n\n", thedate, progname);
    printf ("%s",
	" <cell_name>    Nelsis circuit to be purified\n"
	" -c             Do not remove badly connected cmos transistors\n"
	" -i             Do not remove cmos isolation transistors\n"
	" -p             Do not remove badly connected resistors and capacitors\n"
	" -u             Also remove totally unconnected instances of any sort\n"
	" -s             Also remove partially unconnected instances of any sort\n"
	" -n <number>    Perform <number> preprocessing steps (defaults to 2)\n"
	" -P             Do not join power and ground nets\n"
	" -q             Quiet option: print nothing except errors\n"
	" -z             Print statistics about hash table usage\n"
	" -r             Reduce series/parallel networks of transistors\n"
	" -o <out_name>  Write output to cell name\n"
	" -C             Do not require <cell_name> to start with a capital\n"
	" -D             Debug: print graph\n"
	" -M             Print memory usage\n"
	" -W             Debug: no write\n"
	" -h             Print this list and quit\n\n");
}

/*****************************************************
 * This is the main routine for placement and routing.
 */
static void print_sea_usage (char *progname)
{
    printf ("\n"
	"* * * Welcome to SEA (compiled: %s) * * *\n"
	"The Nelsis interface to Sea-of-Gates placement and routing\n\n"
	"Usage: %s [-options] <lay_name>\n\n", thedate, progname);
    printf ("%s",
	" <lay_name>     Nelsis layout name of cell = your placement)\n"
	" -c <cir_name>  Nelsis circuit of the cell (default: <lay_name>)\n"
	" -o <out_name>  Nelsis layout cell in which to write the routed\n"
	"                  circuit (default: <lay_name> = overwrite)\n"
	" -p             Placement only (default: both)\n"
	" -r             Routing only (default: both)\n"
	" -P \"options\"   Pass 'options' to the placer\n"
	" -R \"options\"   Pass 'options' to the router\n"
	" -v             Verify the circuit only\n"
	" -V             Verbose parsing\n"
	" -x <> -y <>    Set left top of box for placement or routing (in lambda)\n"
	" -X <> -Y <>    Set right bottom of box for routing (default: 0,0)\n"
	" -q             Quiet option: print nothing except errors\n"
	" -h             Print this list and quit\n\n");
}

#define SDFTEMPCELL "Tmp_Cell_"
#define TEMPCELLPREFIX "Tmp"

static void sea_main (int argc, char **argv)
{
   int i,
      exitstatus = 0,
      childexitstatus = 0,
      do_place, do_route, verify_only,
      x_size, y_size, x_left, y_bot;
   char *madonna_options,
      *trout_options,
      *layout_name,
      *circuit_name,
      *output_cell_name,
      *seadif_layout_name,
      *sealogfile,
      *troutlogfile,
      *madonnalogfile,
      opt1[200];

   initialize_globals(); /* initalize opions */

   layout_name = NULL;
   circuit_name = NULL;
   output_cell_name = NULL;
   seadif_layout_name = NULL;
   do_place = do_route = 1; /* place AND route default */
   madonna_options = NULL;
   trout_options = NULL;
   verify_only = 0;
   x_size = y_size = 0;
   x_left = y_bot = 0;

   /*
    * Parse options
    */
   while ((i = getopt (argc, argv, "dR:P:Vvprx:X:y:Y:s:c:o:hq")) >= 0)
   {
      switch (i)
      {
      case 'd':  /* dummy option */
	 break;
      case 'x':   /* right x */
	 if ((x_size = atoi (optarg)) < 0) x_size = 0;
	 break;
      case 'y':   /* top y */
	 if ((y_size = atoi (optarg)) < 0) y_size = 0;
	 break;
      case 'X':
	 if ((x_left = atoi (optarg)) < 0) x_left = 0;
	 break;
      case 'Y':
	 if ((y_bot = atoi (optarg)) < 0) y_bot = 0;
	 break;
      case 'o':   /* output cell name, during writing back in nelsis */
         output_cell_name = cs (optarg);
         break;
      case 'v':   /* verify the circuit only */
         verify_only = 1;
         break;
      case 's':   /* madonna's magnification factor */
	 fprintf (stderr, "ERROR: '-s' is an obsolete option. Use -P \"-s <magn>\"\n");
	 exit (SDFERROR_CALL);
         break;
      case 'V':   /* parse verbose */
         Verbose_parse = TRUE;
         break;
      case 'P':   /* placer options */
	 while (*optarg == ' ') ++optarg;
	 if (*optarg == '-') ++optarg;
	 if (!*optarg) break;
	 if (!madonna_options)
	    sprintf (opt1, "-%s", optarg);
	 else
	    sprintf (opt1, "%s -%s", madonna_options, optarg);
	 madonna_options = cs (opt1);
         break;
      case 'p':   /* place only */
         do_route = 0;
         break;
      case 'r':   /* route only */
         do_place = 0;
         break;
      case 'R':   /* router options */
	 while (*optarg == ' ') ++optarg;
	 if (*optarg == '-') ++optarg;
	 if (!*optarg) break;
	 if (!trout_options)
	    sprintf (opt1, "-%s", optarg);
	 else
	    sprintf (opt1, "%s -%s", trout_options, optarg);
	 trout_options = cs (opt1);
         break;
      case 'c':   /* circuit name */
	 circuit_name = cs (optarg);
	 break;
      case 'h':   /* print help */
         print_sea_usage (argv0);
         exit (0);
         break;
      case 'q':
         verbose = 0;
         break;
      default:
	 invalid_option (i); /* sea */
      }
   }

   if (!do_place && !do_route) {
      fprintf (stderr, "%s: You can only select ONE of '-p' or '-r'\n", argv0);
      exit (SDFERROR_CALL);
   }

   /*
    * read arguments
    */
   if (optind <= argc - 1)
   {
      /* argument: cell specification */
      if (strlen (argv[optind]) > DM_MAXNAME) { /* name too long */
	 fprintf (stderr, "ERROR: cell_name '%s' too long for nelsis (max %d)\n", argv[optind], DM_MAXNAME);
	 exit (1);
      }

      /* copy name */
      layout_name = cs (argv[optind]);

      if (optind < argc - 1) {
	 fprintf (stderr, "WARNING: anything after argument '%s' was ignored.\n", argv[optind]);
	 fprintf (stderr, "(use one argument only)\n");
      }
   }

   if (!layout_name || strlen (layout_name) == 0) { /* no argument found */
      fprintf (stderr, "%s: Layout cell name is required as argument\n", argv0);
      exit (SDFERROR_CALL);
   }

   if (!circuit_name) circuit_name = cs (layout_name);

   if (!strncmp (layout_name, TEMPCELLPREFIX, strlen(TEMPCELLPREFIX)) && layout_name != circuit_name) {
      seadif_layout_name = cs ((char*)SDFTEMPCELL); /* map seadali routing tempcells on SDFTEMPCELL */
   }
   else {
      seadif_layout_name = cs (layout_name);
   }

   if (!output_cell_name) output_cell_name = cs (layout_name);

   troutlogfile   = (char*)"seadif/trout.out";
   madonnalogfile = (char*)"seadif/madonna.out";

   /* kill previous output windows */
   if (do_route && !verify_only) {
	do_echo ((char*)"kill", troutlogfile);
	unlink (troutlogfile);
   }

   /* initialize global variables */
   init_variables();

   /* open nelsis, read image.seadif...
    * readonly from nelsis, also circuit
    */
   init_nelsis (argv0, TRUE, TRUE);

   /* now open seadif */
   if ((i = open_seadif (FALSE, FALSE)) != SDF_NOERROR) myfatal (i);

   add_nelsis_primitives();

   if (do_place)
   {
      /* perform input check and consistency check of the cells */
      if ((exitstatus = all_input_is_ok (circuit_name, NULL, NULL)))
	 myfatal (SDFERROR_INCOMPLETE_DATA);

      /* if necessary: read and write some cells into seadif */
      write_to_seadif();

      /* close databases */
      sdfclose();
      Seadif_open = FALSE;
      close_nelsis();

      if (verbose) {
	 printf ("------ input check seems good enough: calling placer ------\n");
	 fflush (stdout);
      }

      /* make madonna option if none was specified */
      if (!madonna_options) madonna_options = (char*)"-s 0.75";

      /* is a size for the placement given? */
      if (x_size > 0 || y_size > 0)
      {  /* set box */
	 long x = map_lambda_to_grid_coord (x_size, X);
	 long y = map_lambda_to_grid_coord (y_size, Y);
	 if (x < 10) x = 10;
	 if (y < GridRepitition[Y]) y = GridRepitition[Y];
	 sprintf (opt1, "-x %ld -y %ld %s", x, y, madonna_options);
      }
      else strcpy (opt1, madonna_options);

      /*
       * new: follow output of madonna life on screen
       */
      if (!verify_only && verbose) {
	 do_empty (madonnalogfile);
	 if (strncmp (layout_name, "Tmp", 3) == 0) { /* called from seadali: use window */
	    if (runprognowait ((char*)"xterm", (char*)"seadif/tmp.out", &i,
		 (char*)"-title", (char*)"Live output of madonna   (hit 'q' to erase window)",
		 (char*)"-n", (char*)"madonna",
		 (char*)"-sl", (char*)"500", /* 500 savelines */
		 (char*)"-sb",               /* scrollbar */
		 (char*)"-ut",               /* no utmp */
		 (char*)"-fn", (char*)"8x16", /* font */
		 (char*)"-bg", (char*)"black",
		 (char*)"-fg", (char*)"white",
		 (char*)"-geometry", (char*)"70x18-3+1",
		 (char*)"-e", (char*)"seatail", madonnalogfile, NULL))
	       fprintf (stderr, "xterm for madonna output is not working fine\n");
	 } else { /* non-window: to stdout */
	    if (runprognowait ((char*)"seatail", NULL, &i, madonnalogfile, NULL))
	       fprintf (stderr, "seatail is not working fine\n");
	 }
      }

      if (runprog ((char*)"madonna", madonnalogfile, &childexitstatus,
		  (char*)"-l", this_sdf_lib,
		  (char*)"-f", circuit_name,
		  (char*)"-c", circuit_name,
		  (char*)"-o", seadif_layout_name,
		  opt1, /* these are the madonna options */
		  circuit_name, NULL)) myfatal (SDFERROR_RUNPROG);

      switch (childexitstatus)
      {
      case 0: /* very nice */
	 if (verbose) {
	    printf ("------ madonna successful ------\n");
	    fflush (stdout);
	 }
	 do_echo ((char*)"terminate", madonnalogfile);
	 break;
      case SDFERROR_FILELOCK:
	 fprintf (stderr, "ERROR: from madonna: database is locked\n");
	 myfatal (childexitstatus);
      default:
	 fprintf (stderr, "ERROR: madonna failed, exit status: %d\n", childexitstatus);
	 if (strncmp (layout_name, "Tmp", 3) == 0 && verbose)
	    xfilealert (ERROR, madonnalogfile);   /* show failure */
	 else
	    do_echo ((char*)"lock", madonnalogfile); /* lock output */
	 myfatal (SDFERROR_MADONNA_FAILED);
	 break;
      }

      /* set box to zero, if place and route, trout should not have a box */
      x_size = y_size = 0;
      x_left = y_bot = 0;
   }

   if (do_route)
   {
      if (!do_place)
      { /* not already called before placement */
	 if ((exitstatus = all_input_is_ok (circuit_name, layout_name, seadif_layout_name)))
	 {
	    if (exitstatus == 1) myfatal (SDFERROR_INCOMPLETE_DATA);
	 }

	 /* write all the necessary stuff into seadif */
	 write_to_seadif();

	 sdfclose();
	 close_nelsis();

	 if (verbose) {
	    printf ("------ input seems good enough: calling router ------\n");
	    fflush (stdout);
	 }
      }
      else
      {
	 if (verbose) {
	    if (!verify_only)
	       printf ("------ calling router ------\n");
	    else
	       printf ("------ calling router to verify ------\n");
	    fflush (stdout);
	 }
      }

      if (verify_only)
      { /* set verify on: -v */
	 if (!trout_options) strcpy (opt1, "-v");
	 else sprintf (opt1, "-v %s", trout_options);
      }
      else
      {
	 if (!trout_options) trout_options = (char*)"-d"; /* dummy option */

	 /* is a size for the placement given? */
	 if (x_size - x_left > 0 && y_size - y_bot > 0)
	 {  /* set box */
	    sprintf (opt1, "-x %ld -y %ld -X %ld -Y %ld %s",
		map_lambda_to_grid_coord (x_size, X),
		map_lambda_to_grid_coord (y_size, Y),
		map_lambda_to_grid_coord (x_left, X),
		map_lambda_to_grid_coord (y_bot,  Y),
		trout_options);
	 }
	 else strcpy (opt1, trout_options);
      }

      /*
       * new: follow output of router life on screen
       */
      if (!verify_only && verbose) {
	    do_empty (troutlogfile);
	    if (strncmp (layout_name, "Tmp", 3) == 0) { /* called from seadali: use window */
	       if (runprognowait ((char*)"xterm", (char*)"seadif/tmp.out", &i,
		    (char*)"-title", (char*)"Live output of trout   (hit 'q' to erase window)",
		    (char*)"-n", (char*)"trout",
		    (char*)"-sl", (char*)"500", /* 500 savelines */
		    (char*)"-sb",               /* scrollbar */
		    (char*)"-ut",               /* no utmp */
		    (char*)"-fn", (char*)"8x16", /* font */
		    (char*)"-bg", (char*)"black",
		    (char*)"-fg", (char*)"white",
		    (char*)"-geometry", (char*)"70x18-3+1",
		    (char*)"-e", (char*)"seatail", troutlogfile, NULL))
		  fprintf (stderr, "xterm for trout output is not working fine\n");
	    } else { /* non-window: to stdout */
	       if (runprognowait ((char*)"seatail", NULL, &i, troutlogfile, NULL))
		  fprintf (stderr, "seatail is not working fine\n");
	    }
      }

      if (runprog ((char*)"trout", troutlogfile, &childexitstatus,
		  (char*)"-l", this_sdf_lib,
		  (char*)"-f", circuit_name,
		  (char*)"-c", circuit_name,
		  (char*)"-o", seadif_layout_name,
		  opt1,                     /* all other options */
		  seadif_layout_name, NULL)) myfatal (SDFERROR_RUNPROG);

      switch (childexitstatus)
      {
      case 0: /* very nice */
	 if (verbose) {
	    if (!verify_only)
	       printf ("------ routing successful ------\n");
	    else
	       printf ("------ check successful ------\n");
	    fflush (stdout);
	 }
	 /* terminate the window */
	 do_echo ((char*)"terminate", troutlogfile);
	 break;
      case SDFERROR_FILELOCK:
	 fprintf (stderr, "ERROR: from trout: database is locked\n");
	 myfatal (childexitstatus);
      case SDFERROR_INCOMPLETE:
	 if (verbose) {
	    printf ("WARNING: routing is not 100%% successful\n");
	    fflush (stdout);
	 }
	 /* xfilealert (WARNING, troutlogfile); */
	 /* leave window */
	 break;
      default:
	 fprintf (stderr, "ERROR: router failed, exit status: %d\n", childexitstatus);
	 xfilealert (ERROR, troutlogfile);
	 myfatal (SDFERROR_ROUTER_FAILED);
	 break;
      }
   }

   if (verbose) {
      printf ("------ converting result back into nelsis as cell '%s' ------\n", output_cell_name);
      fflush (stdout);
   }

   /*
    * now we have to convert back the cell into nelsis
    * notice that we are calling ourselves!
    * NB: call is non-hierarchical (-H), no son-cells are converted back
    */
   sealogfile = (char*)"seadif/seanel.out";
   if (runprog ((char*)"nelsea", sealogfile, &childexitstatus,
	       (char*)"-rLH",
	       (char*)"-f", circuit_name,
	       (char*)"-c", circuit_name,
	       (char*)"-l", seadif_layout_name,
	       output_cell_name, NULL)) myfatal (SDFERROR_RUNPROG);

   if (childexitstatus) {
      fprintf (stderr, "ERROR: reverse conversion of placed/routed cell\n");
      fprintf (stderr, "       from seadif back to nelsis failed\n");
      fprintf (stderr, "       Exit status: %d\n", childexitstatus);
      xfilealert (ERROR, sealogfile);
      if (childexitstatus == SDFERROR_FILELOCK) myfatal (childexitstatus);
      myfatal (SDFERROR_NELSEA_FAILED);
   }

   if (verbose) {
      printf ("------ %s: task completed ------\n", argv0);
      fflush (stdout);
   }

   exit (exitstatus);
}

/*
 * This routine parses the options inserted in exclude_list
 * and changes the global vars LayerRead, ViaRead[i], TermRead, and accordingly
 * Note: argument exclude_list, string to be parsed.
 */
static void parse_readoptions (char *exclude_list)
{
   int i, j;

   if (!exclude_list) return;

   i = 0;
   while (exclude_list[i]) {
      switch (exclude_list[i]) {
      case 't':
      case 'T':
	 TermRead = FALSE;
	 break;
      case 'm':
      case 'M':
	 MCRead = FALSE;
	 break;
      case 'v':
      case 'V':
	 for (j = 0; j != MAXLAYERS; j++) ViaRead[j] = FALSE;
	 break;
      case '0':
	 LayerRead[0] = FALSE;
	 break;
      case '1':
	 LayerRead[1] = FALSE;
	 break;
      case '2':
	 LayerRead[2] = FALSE;
	 break;
      case '3':
	 LayerRead[3] = FALSE;
	 break;
      case '4':
	 LayerRead[4] = FALSE;
	 break;
      case '5':
	 LayerRead[5] = FALSE;
	 break;
      case '6':
	 LayerRead[6] = FALSE;
	 break;
      default:
	 fprintf (stderr, "WARNING: unrecognized character '%c' with -E option\n", exclude_list[i]);
      }
      i++;
   }
}

// This one is called if ::operator new() fails to allocate enough memory:
static void my_new_handler ()
{
   cerr << "\n"
	<< "FATAL: I cannot allocate enough memory." << endl
	<< "       Ask your sysop to configure more swap space ..." << endl;
   sdfexit (1);
}
