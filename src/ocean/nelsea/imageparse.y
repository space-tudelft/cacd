%{
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
 * Parser for designrules in sea-of-gates system.
 */
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#include "src/ocean/nelsea/typedef.h"
#include "src/ocean/nelsea/prototypes.h"

/* Work around a bug in flex-2.4.6 which assumes that "#if __STDC__" works
#ifdef __STDC__
#undef __STDC__
#define __STDC__ 1
#endif
*/

/*
 * export
 */
extern long
   Chip_num_layer,         /* number of metal layers to be used */
   *LayerOrient,           /* orientationtable of layers */
   *LayerWidth,            /* array with wire width in each layer */
   NumViaName,             /* number of indices in the array ViaCellName */
   ***ViaIndex,            /* Viaindex[x][y][z]: */
                           /* if z == 0: Index of via to core image in array ViaCellName (-1 if nonex) */
                           /* if z >  0: -1 if via between z and z-1 is not allowed, 1 otherwise */
   ChipSize[2],            /* size for empty array */
   *GridMapping[2],        /* mapping of gridpoints on layout coordinates
                              this is an array of size GridRepetition */
   *OverlayGridMapping[2], /* overlaymapping of gridpoints to layout coordinates */
   OverlayBounds[2][2],    /* boundaries of overlaymapping, index 1 = orient, index2 = L/R */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid image (in grid points) */
   LayoutRepitition[2];    /* repitionvector (dx, dy) of LAYOUT image (in lambda) */

extern char
   ImageName[],
   *ThisImage,              /* name identifier of image */
   **LayerMaskName,        /* array with mask names of each layer */
   **ViaMaskName,          /* array with mask name of via to layer+1 */
   *DummyMaskName,         /* name of dummy (harmless) mask */
   **ViaCellName;          /* array with cell names of these vias */

extern int
   Verbose_parse,          /* print unknown keywords during parsing */
   ydlineno;

extern PARAM_TYPE *SpiceParameters;

/*
 * LOCAL
 */
static long orient, counter;
static long sometoken, bracecount; /* used for skipping unknown keywords */

int yderror (char *s);
int ydlex (void);

int ydwrap (void)
{
    return 1;
}

%}

%union {
        int             itg;
        char            str[200];
	PARAM_TYPE      *parameter;
}

%token <str> STRINGTOKEN
%token <str> NUMBER
%token LBR RBR
%token SEADIFTOKEN
%token LIBRARYTOKEN

%token CHIPDESCRIPTION
%token CHIPIMAGEREF
%token CHIPSIZE

%token TECHNOLOGY
%token TECHDESIGNRULES
%token TECHNROFLAYERS
%token TECHWIREORIENT
%token TECHWIREWIDTH
%token TECHWIREMASKNAME
%token TECHDUMMYMASKNAME
%token TECHVIAMASKNAME
%token TECHVIACELLNAME

%token IMAGEDESCRIPTION
%token CIRCUITIMAGE
%token LAYOUTIMAGE

%token LAYOUTMODELCALL
%token LAYOUTIMAGEREPETITION
%token LAYOUTIMAGEPATTERN
%token LAYOUTIMAGEPATTERNLAYER
%token RECTANGLE

%token GRIDIMAGE
%token GRIDSIZE
%token GRIDMAPPING
%token OVERLAYMAPPING
%token GRIDCONNECTLIST
%token GRIDBLOCK
%token RESTRICTEDFEED
%token UNIVERSALFEED
%token FEED
%token GRIDCOST
%token GRIDCOSTLIST

%token STATUSTOKEN
%token WRITTEN
%token TIMESTAMP
%token AUTHOR
%token PROGRAM
%token COMMENT

%token SPICEPARAMETERS
%token MODEL
%token PARAMETER

%type <parameter> Parameter _Spiceparameters Spiceparameters Model Models

%start Seadif

%%
Seadif         : LBR SEADIFTOKEN SeadifFileName _Seadif RBR       /* EdifVersion, EdifLevel and KeywordMap skipped */
               ;

SeadifFileName : STRINGTOKEN { }
               ;

_Seadif        : /* nothing */
               | _Seadif Library
               | _Seadif Image
               | _Seadif Chip
               | _Seadif Status
               | _Seadif Comment
               | _Seadif UserData
               ;

/*
 * Chipdescription (junk)
 */
Chip           : LBR CHIPDESCRIPTION ChipNameDef _Chip RBR
               ;

/*
 * ascii name of chip
 */
ChipNameDef    : STRINGTOKEN
                     {  /* store Chip name */
/*                     STRINGSAVE (ChipName, $1); */
                     }
               ;

_Chip          :
               | _Chip Status
               | _Chip ChipImageRef
               | _Chip ChipSize
               | _Chip Comment
               | _Chip UserData
               ;

ChipImageRef   : LBR CHIPIMAGEREF STRINGTOKEN RBR   /* currently not implemented */
               ;

ChipSize       : LBR CHIPSIZE NUMBER NUMBER RBR
                  { /* store chip size */
                  ChipSize[X] = atol ($3);
                  ChipSize[Y] = atol ($4);
                  }
               ;

/*
 * Imagedescription
 */
Image          : LBR IMAGEDESCRIPTION ImageNameDef _Image RBR
               ;


/*
 * ImageDescription/ImageNameDef
 * ascii name of image
 */
ImageNameDef   : STRINGTOKEN
                  {
                  ThisImage = cs($1);
                  }
               ;


_Image         :
               | _Image Technology
               | _Image Status
               | _Image CircuitImage
               | _Image GridImage
               | _Image LayoutImage
               | _Image Comment
               | _Image UserData
               ;

/*
 * (library or Image)/technology description
 */
Technology     : LBR TECHNOLOGY _Technology RBR
               ;

_Technology    : STRINGTOKEN { }     /* currently not used */
               | _Technology DesignRules
               | _Technology Spiceparameters
                 { SpiceParameters = $2; } /* initialize global */
               | _Technology Comment
               | _Technology UserData
	       ;

/*
 * Technology/designrules
 */
DesignRules    : LBR TECHDESIGNRULES _DesignRules RBR
               ;

_DesignRules   : TechNumLayer                  /* number of layers before others */
               | _DesignRules TechLayOrient
               | _DesignRules TechWireWidth
               | _DesignRules TechWireMask
               | _DesignRules TechDummyMask
               | _DesignRules TechViaMask
               | _DesignRules TechViaCellName
               | _DesignRules Comment
               | _DesignRules UserData
               ;

/*
 * Designrules/numlayer: number of metal layers,
 * In this routine a number of arrays are allocated
 */
TechNumLayer   : LBR TECHNROFLAYERS NUMBER RBR
	       {
                  Chip_num_layer = atol ($3);
                  allocate_layer_arrays (Chip_num_layer);
	       }
               ;

/*
 * declares the orientation of a layer
 */
TechLayOrient : LBR TECHWIREORIENT NUMBER STRINGTOKEN RBR
	       {
		  long num = atol ($3);
                  if (num < 0 || num >= Chip_num_layer)
                     yderror ("Illegal layer index for orient");
                  else
                     switch ($4[0]) {
                        case 'h': case 'H': case 'x': case 'X':
                           LayerOrient[num] = HORIZONTAL;
                           break;
                         case 'v': case 'V': case 'y': case 'Y':
                           LayerOrient[num] = VERTICAL;
                           break;
                        default:
                           yderror ("warning: Unknown orientation for layerorient");
		     }
	       }
               ;

/*
 * declares wire width in a layer
 */
TechWireWidth  : LBR TECHWIREWIDTH NUMBER NUMBER RBR
	       {
		  long num = atol ($3);
                  if (num < 0 || num >= Chip_num_layer)
                     yderror ("Illegal layer index for WireWidth");
                  else {
		     long width = atol ($4);
                     if (LayerWidth[num] >= 0) yderror ("WARNING: redeclaration of WireWidth");
                     LayerWidth[num] = width < 0 ? 0 : width;
		  }
	       }
               ;

/*
 * declares mask name of a layer
 */
TechWireMask   : LBR TECHWIREMASKNAME NUMBER STRINGTOKEN RBR
	       {
		  long num = atol ($3);
                  if (num < 0 || num >= Chip_num_layer)
                     yderror ("Illegal layer index for WireMaskName");
                  else {
                     if (LayerMaskName[num]) yderror ("WARNING: redeclaration of LayerMaskName");
                     STRINGSAVE (LayerMaskName[num], $4);
		  }
	       }
               ;

/*
 * declares mask name of the dummy mask
 */
TechDummyMask   : LBR TECHDUMMYMASKNAME STRINGTOKEN RBR
                  {
                  if (DummyMaskName)
                     yderror ("WARNING: redeclaration of DummyMaskName");
                  STRINGSAVE (DummyMaskName, $3);
                  }
               ;

/*
 * declares the mask name of a via
 */
TechViaMask    : LBR TECHVIAMASKNAME NUMBER NUMBER STRINGTOKEN RBR
                  {
                  long low3 = atol ($3);
                  long low4 = atol ($4);

                  if (low3 >= Chip_num_layer || low4 >= Chip_num_layer ||
                     (low3 < 0 && low4 < 0) || ABS (low3 - low4) != 1)
                     yderror ("Illegal layer index for ViaMaskName");
                  else
		  {
		     long lowest = MIN (low3, low4);
                     if (lowest < 0)
		     { /* to poly/diff: store in higest layer */
                        lowest = NumViaName - 1;
                        if (ViaMaskName[lowest] || NumViaName == Chip_num_layer)
			{ /* make arrays ViaMaskName and ViaCellName longer */
			   lowest++;
                           NumViaName++;
                           if (!(ViaCellName = (char **) realloc ((char *) ViaCellName,
                                          (unsigned)(NumViaName * sizeof(char *)))))
                              error (FATAL_ERROR, "realloc for ViaCellName failed");
                           if (!(ViaMaskName = (char **) realloc ((char *) ViaMaskName,
                                          (unsigned)(NumViaName * sizeof(char *)))))
                              error (FATAL_ERROR, "realloc for ViaMaskName failed");
                           ViaCellName[lowest] = NULL;
                           ViaMaskName[lowest] = NULL;
			}
		     }
                     if (ViaMaskName[lowest]) yderror ("WARNING: redeclaration of ViaMaskName");
                     ViaMaskName[lowest] = canonicstring ($5);
		  }
	       }
               ;

/*
 * declares the cell name of a via
 */
TechViaCellName : LBR TECHVIACELLNAME NUMBER NUMBER STRINGTOKEN RBR
                  {
                  long low3 = atol ($3);
                  long low4 = atol ($4);

                  if (low3 >= Chip_num_layer || low4 >= Chip_num_layer ||
                     (low3 < 0 && low4 < 0) || ABS (low3 - low4) != 1)
                     yderror ("Illegal layer index for ViaCellName");
                  else
		  {
		     long lowest = MIN (low3, low4);
                     if (lowest < 0)
		     { /* to poly/diff: possibly a new index must be allocated */
                        lowest = NumViaName - 1;
                        if (ViaCellName[lowest] || NumViaName == Chip_num_layer)
			{ /* make arrays ViaMaskName and ViaCellName longer */
                           lowest++;
                           NumViaName++;
                           if (!(ViaCellName = (char **) realloc ((char *) ViaCellName,
                                          (unsigned)(NumViaName * sizeof(char *)))))
                              error (FATAL_ERROR, "realloc for ViaCellName failed");
                           if (!(ViaMaskName = (char **) realloc ((char *) ViaMaskName,
                                          (unsigned)(NumViaName * sizeof(char *)))))
                              error (FATAL_ERROR, "realloc for ViaMaskName failed");
                           ViaCellName[lowest] = NULL;
                           ViaMaskName[lowest] = NULL;
			}
		     }
                     if (ViaCellName[lowest]) yderror ("WARNING: redeclaration of ViaCellName");
                     ViaCellName[lowest] = canonicstring ($5);
		  }
	       }
               ;

Spiceparameters: LBR SPICEPARAMETERS Models RBR
               { $<parameter>$ = $3; }
               ;

Models         : /* empty */
               { $<parameter>$ = (PARAM_TYPE*) NULL; }
               | Model Models
               {
	       PARAM_TYPE *last = $1;
	       for (; last && last->next; last = last->next) ;
	       if (last) last->next = $2; /* link Model list in front of Models */
	       $<parameter>$ = $1;
	       }
               ;

Model          : LBR MODEL STRINGTOKEN _Spiceparameters RBR
               {
               PARAM_TYPE *param = $4;
	       for (; param; param = param->next) param->model = cs($3); /* init "model" field */
	       $<parameter>$ = $4;
	       }
               ;

_Spiceparameters:
               /* empty */
               { $<parameter>$ = (PARAM_TYPE*) NULL; }
               | Parameter _Spiceparameters
               {
	       ($1)->next = $2;	   /* link new parameter in front of list */
	       $<parameter>$ = $1; /* return new list */
	       }
               ;

Parameter      : LBR PARAMETER STRINGTOKEN STRINGTOKEN RBR
               {
               PARAM_TYPE *p;
	       NewParam(p);
	       p->name = cs($3);
	       p->value = cs($4);
	       $<parameter>$ = p;
               }
               ;

/*
 * Image/CircuitImage
 * Currently not implemented
 *
CircuitImage   : LBR CIRCUITIMAGE anything RBR
                     { printf ("CircuitImage\n"); }
               ;
 */
CircuitImage   : LBR CIRCUITIMAGE
	       {
	       bracecount = 1;
	       while (bracecount > 0)
	          if ((sometoken = ydlex()) == LBR)
	             ++bracecount;
	          else if (sometoken == RBR)
	             --bracecount;

               /* printf ("CircuitImage\n"); */
               }
               ;


/*
 * Image/LayoutImage
 */
LayoutImage    : LBR LAYOUTIMAGE _LayoutImage RBR
               ;

_LayoutImage   :
               | _LayoutImage LayModelCall
               | _LayoutImage LayImageRep
               | _LayoutImage LayImagePat
               | _LayoutImage Comment
               | _LayoutImage UserData
               ;

LayModelCall	: LBR LAYOUTMODELCALL STRINGTOKEN RBR
		{ /* store image name */
		    strNcpy (ImageName, $3, DM_MAXNAME);
		}
		;

LayImageRep	: LBR LAYOUTIMAGEREPETITION NUMBER NUMBER RBR
	       {  /* store repetitions */
                  LayoutRepitition[X] = atol ($3);
                  LayoutRepitition[Y] = atol ($4);
	       }
               ;

LayImagePat    : LBR LAYOUTIMAGEPATTERN _LayImagePat RBR
               ;

_LayImagePat   :
               | _LayImagePat LayImPatLay
               | _LayImagePat Comment
               | _LayImagePat UserData
               ;

LayImPatLay    : LBR LAYOUTIMAGEPATTERNLAYER STRINGTOKEN _LayImPatLay RBR
               ;

_LayImPatLay   :
               | _LayImPatLay Rectangle
               | _LayImPatLay Comment
               | _LayImPatLay UserData
               ;


Rectangle      : LBR RECTANGLE NUMBER NUMBER NUMBER NUMBER RBR
               ;


/*
 * Image/GridImage
 * Interesting
 */
GridImage      : LBR GRIDIMAGE _GridImage RBR
               ;

_GridImage     : GridSize
               | _GridImage GridConnectList
               | _GridImage GridCostList
               | _GridImage Comment
               | _GridImage UserData
               ;

/*
 * specifies size of the basic grid
 */
GridSize       : LBR GRIDSIZE GridBbx _GridSize RBR
               ;

/* dimensions to allocate */
GridBbx        : NUMBER NUMBER
	       {
                  if (GridRepitition[X] > 0) {
                     yderror ("redeclaration of GridSize");
                     error (FATAL_ERROR, "parser");
                  }

                  GridRepitition[X] = atol ($1);
                  GridRepitition[Y] = atol ($2);

                  /* allocate lots of global array related to the image size */
                  allocate_core_image();
                  orient = -1;
	       }
               ;

_GridSize      :
               | _GridSize GridMapping
               | _GridSize OverlayMapping
               | _GridSize Comment
               | _GridSize UserData
               ;

/* mapping of gridpositions to layoutpositions */
GridMapping    : LBR GRIDMAPPING GridMapOrient _GridMapping RBR
                     {
                     if (counter != GridRepitition[orient] && counter >= 0)
                        {
                        yderror ("warning: incomplete gridmapping list");
                        fprintf (stderr, "%ld of the %ld points found\n", counter, GridRepitition[orient]);
                        }
                     }
               ;

GridMapOrient  : STRINGTOKEN            /* should specify horizontal of vertical */
                  { /* set index */
                  switch ($1[0])
                     {
                     case 'h': case 'H': case 'x': case 'X':
                        orient = X;
                        break;
                      case 'v': case 'V': case 'y': case 'Y':
                        orient = Y;
                        break;
                     default:
                        yderror ("warning: Unknown orientation");
                        orient = X;
                        break;
                     }

                  counter = 0;
                  }
               ;

_GridMapping   :
               | _GridMapping NUMBER    /* read in number */
	       { /* add number to array */
		  if (counter >= 0) { /* negative indicates out of bounds */
		     if (counter < GridRepitition[orient])
			GridMapping[orient][counter++] = atol ($2);
		     else {
			yderror ("warning: Too many grid mapping points specified");
			fprintf (stderr, "should be %ld points; points starting from '%s' ignored\n",
			   GridRepitition[orient], $2);
			counter = -1;  /* disables further errors */
		     }
		  }
	       }
               ;


/* mapping of gridpositions to layoutpositions */
OverlayMapping : LBR OVERLAYMAPPING GridMapOrient NUMBER NUMBER
	       { /* allocate that array */
		  CALLOC (OverlayGridMapping[orient], long, GridRepitition[orient]);
		  OverlayBounds[opposite(orient)][L] = atol ($4);
		  OverlayBounds[opposite(orient)][R] = atol ($5);
	       }
                 _OverlayMapping RBR
	       {
		  if (counter != GridRepitition[orient] && counter >= 0) {
		     yderror ("warning: incomplete OverlayMapping list");
		     fprintf (stderr, "%ld of the %ld points found\n", counter, GridRepitition[orient]);
		  }
	       }
               ;

_OverlayMapping   :
               | _OverlayMapping NUMBER    /* read in number */
	       { /* add number to array */
		  if (counter >= 0) { /* negative indicates out of bounds */
		     if (counter < GridRepitition[orient])
			OverlayGridMapping[orient][counter++] = atol ($2);
		     else {
			yderror ("warning: Too many grid mapping points specified");
			fprintf (stderr, "should be %ld points; points starting from '%s' ignored\n",
			   GridRepitition[orient], $2);
			counter = -1;  /* disables further errors */
		     }
		  }
	       }
               ;


/*
 * Define connections/neighbours of grid points
 */
GridConnectList : LBR GRIDCONNECTLIST _GridConnectList RBR
                     {
                     /* if ready: link offset arrays */
                     /* link_feeds_to_offsets(); */
                     }
               ;

_GridConnectList  :
               | _GridConnectList Block
               | _GridConnectList RestrictedFeed
               | _GridConnectList UniversalFeed
               | _GridConnectList Comment
               | _GridConnectList UserData
               ;

Block          : LBR GRIDBLOCK NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
	       {
                  if (!add_grid_block (atol($3), atol($4), atol($5), atol($6), atol($7), atol($8)))
                     yderror ("grid block position outside image");
	       }
               ;

UniversalFeed  : LBR UNIVERSALFEED
                  {
/*
                  Feedlist = NULL;
*/
                  }
                 _GridFeed RBR
                  {
/*
                  if (process_feeds (Feedlist, CoreFeed) == FALSE)
                     fprintf (stderr, "error was on line %d\n", ydlineno);
 */
                  }
               ;

RestrictedFeed : LBR RESTRICTEDFEED
                  {
/*
                  Feedlist = NULL;
 */
                  }
                 _GridFeed RBR
                  {
/*
                  if (process_feeds (Feedlist, RestrictedCoreFeed) == FALSE)
                     fprintf (stderr, "error was on line %d\n", ydlineno);
 */
                  }
               ;

_GridFeed      :
               | _GridFeed GridFeedRef
               | _GridFeed Comment
               | _GridFeed UserData
               ;

/*
 * store kind of via in array ViaIndex
 */
GridFeedRef    : LBR FEED STRINGTOKEN NUMBER NUMBER RBR
{ /* MODIFIED: only stores viaindex */
    long viaindex;

    /* find the via in the via list */
    for (viaindex = Chip_num_layer; viaindex != NumViaName; viaindex++)
	if (strcmp ($3, ViaCellName[viaindex]) == 0) break;

    if (viaindex == NumViaName)
	fprintf (stderr, "WARNING (parser, line %d): via name '%s' not declared\n", ydlineno, $3);
    else {
	long x = atol ($4);
	long y = atol ($5);
	if (x < 0 || x >= GridRepitition[X] || y < 0 || y >= GridRepitition[Y]) {
	    if (Verbose_parse)
		fprintf (stderr, "WARNING (parser, line %d): feed position %ld %ld outside image\n", ydlineno, x, y);
	}
	else {
	    if (ViaIndex[x][y][0] >= 0)
		fprintf (stderr, "WARNING (parser, line %d): feed position %ld %ld multiple declared\n", ydlineno, x, y);
	    ViaIndex[x][y][0] = viaindex;
	}
    }
}
;

/*
 * declares costs of point
 * should be called after GridCOnnectList
 */
GridCostList   : LBR GRIDCOSTLIST _GridCostList RBR
               ;

_GridCostList  :
               | _GridCostList GridCost
               | _GridCostList Comment
               | _GridCostList UserData
               ;

GridCost	: LBR GRIDCOST NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{ /* <cost> <vector> <startpoint_range> <endpoint_range> */
#if 0
		    if (!set_grid_cost (atol($3), atol($4), atol($5), atol($6),
			atol($7), atol($8), atol($9), atol($10), atol($11), atol($12)))
			yderror ("WARNING: GridCost has no effect");
#endif
		}
		;
/*
 * Library module
 */
Library        : LBR LIBRARYTOKEN anything RBR
               ;

/*
 * BASIC functions
 */
                 /* status block called on many levels */
Status         : LBR STATUSTOKEN _Status RBR
                     {
                     /* printf ("Status detected on line %d\n", ydlineno); */
                     }
               ;

_Status        :
               | _Status Written
               | _Status Comment
               | _Status UserData
               ;

Written        : LBR WRITTEN _Written RBR
               ;

_Written       : TimeStamp
               | _Written Author
               | _Written Program
/*             | _Written DataOrigin     not implemented */
/*             | _Written Property       not implemented */
               | _Written Comment
               | _Written UserData
               ;

			/* year month day hour minute second */
TimeStamp	: LBR TIMESTAMP NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{
#if 0
		    printf ("Timestamp detected on line %d: %d %d %d %d %d %d\n",
			ydlineno, atoi($3), atoi($4), atoi($5), atoi($6), atoi($7), atoi($8));
#endif
		}
		;

Author         : LBR AUTHOR anything RBR       /* anything should be a string */
                     {
                     /* printf ("Author detected on line %d\n", ydlineno); */
                     }
               ;

Program        : LBR PROGRAM anything RBR      /* anything should be a string, followed by a version */
                     {
                     /* printf ("Program signature detected on line %d\n", ydlineno); */
                     }
               ;

UserData       : LBR
{
    if (Verbose_parse) printf ("unknown keyword in line %d:", ydlineno);
}
                 STRINGTOKEN
{
		     bracecount = 1;
		     while (bracecount > 0)
		        if ((sometoken = ydlex()) == LBR)
		           ++bracecount;
		        else if (sometoken == RBR)
		           --bracecount;

    if (Verbose_parse) printf (" %s (skipped until line %d)\n", $3, ydlineno);
}
;

Comment        : LBR COMMENT
                     {
		     bracecount = 1;
		     while (bracecount > 0)
		        if ((sometoken = ydlex()) == LBR)
		           ++bracecount;
		        else if (sometoken == RBR)
		           --bracecount;

                     /* printf ("Comment detected on line %d\n", ydlineno); */
                     }
               ;

anything       : /* empty */
               | anything something
               ;

something      : STRINGTOKEN { }
               | NUMBER      { }
               | LBR anything RBR
               ;

%%

/* Work around a bug in flex-2.4.6 which assumes that "#if __STDC__" works
#ifdef __STDC__
#undef __STDC__
#define __STDC__ 1
#endif
*/

#include "imagelex.h"

int yderror (char *s)
{
    fflush (stdout);
    fprintf (stderr, "ERROR (Seadif image description parser): %s. Try line %d.\n", s, ydlineno);
    return 0;
}
