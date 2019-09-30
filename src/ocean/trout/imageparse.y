%{
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

/*
 * Seadif image parser.
 */

#include "src/ocean/trout/typedef.h"

void yderror (char *s);
int ydlex (void);

/*
 * export
 */
extern GRIDPOINTPTR
   **CoreFeed,             /* matrix of universal feedthroughs in basic image */
   **RestrictedCoreFeed;   /* matrix of restricted feedthroughs in basic image */

extern int Verbose_parse;
extern long
   NumImageTearLine[2],    /* number of tearlines in orientation of index (HORIZONTAL/VERTICAL) */
   *ImageTearLine[2],      /* array containing the coordinates of the tearline of basic image */
   Chip_num_layer,         /* number of metal layers to be used */
   GridRepitition[2],      /* repitionvector (dx, dy) of grid core image (in grid points) */
   *LayerOrient;           /* array of orientations of each layer */

extern POWERLINEPTR
   PowerLineList;          /* list of template power lines */

extern char *ThisImage;    /* Seadif name of this image */

extern GRIDADRESSUNIT xl_chiparea, xr_chiparea, yb_chiparea, yt_chiparea;

extern int ydlineno;

/*
 * LOCAL
 */
static long orient, counter;
static long sometoken, bracecount; /* Used for skipping unknown keywords. */
static GRIDPOINTPTR Feedlist;

%}

%union {
        int itg;
        char str[200];
}

%token <str> STRINGTOKEN
%token <str> NUMBER
%token LBR RBR
%token SEADIFTOKEN
%token LIBRARYTOKEN
%token TECHNOLOGY
%token TECHDESIGNRULES
%token TECHNROFLAYERS
%token TECHWIREORIENT
%token TECHWIREWIDTH
%token TECHWIREMASKNAME
%token TECHVIAMASKNAME
%token TECHVIACELLNAME
%token IMAGEDESCRIPTION
%token CHIPDESCRIPTION
%token CHIPIMAGEREF
%token CHIPSIZE
%token CHIPROUTINGAREA
%token CIRCUITIMAGE
%token LAYOUTIMAGE
%token LAYOUTMODELCALL
%token LAYOUTIMAGEREPITITION
%token LAYOUTIMAGEPATTERN
%token LAYOUTIMAGEPATTERNLAYER
%token RECTANGLE
%token GRIDIMAGE
%token GRIDSIZE
%token GRIDMAPPING
%token GRIDTEARLINE
%token GRIDPOWERLINE
%token GRIDCONNECTLIST
%token GRIDBLOCK
%token RESTRICTEDFEED
%token UNIVERSALFEED
%token FEED
%token GRIDCOST
%token FEEDCOST
%token GRIDCOSTLIST
%token STATUSTOKEN
%token WRITTEN
%token TIMESTAMP
%token AUTHOR
%token PROGRAM
%token COMMENT

%start Seadif

%%
Seadif		: LBR SEADIFTOKEN SeadifFileName _Seadif RBR
		    /* EdifVersion, EdifLevel and KeywordMap skipped */
		;
SeadifFileName	: STRINGTOKEN { }
		;
_Seadif		: /* nothing */
		| _Seadif Library
		| _Seadif Chip
		| _Seadif Image
		| _Seadif Status
		| _Seadif Comment
		| _Seadif UserData
		;
/*
 * Chip description
 */
Chip		: LBR CHIPDESCRIPTION ChipNameDef _Chip RBR
		;
/*
 * ascii name of chip
 */
ChipNameDef	: STRINGTOKEN { }
		;
_Chip		:
		| _Chip Status
		| _Chip ChipImageRef
		| _Chip ChipSize
		| _Chip ChipRoutingArea
		| _Chip Comment
		| _Chip UserData
		;
ChipImageRef	: LBR CHIPIMAGEREF STRINGTOKEN RBR
		;
ChipSize	: LBR CHIPSIZE NUMBER NUMBER RBR
		;
ChipRoutingArea : LBR CHIPROUTINGAREA NUMBER NUMBER NUMBER NUMBER RBR
		{
		    xl_chiparea = (GRIDADRESSUNIT)atol($3);
		    xr_chiparea = (GRIDADRESSUNIT)atol($4);
		    yb_chiparea = (GRIDADRESSUNIT)atol($5);
		    yt_chiparea = (GRIDADRESSUNIT)atol($6);
		}
		;

/*
 * Imagedescription
 */
Image		: LBR IMAGEDESCRIPTION ImageNameDef _Image RBR
		;
/*
 * ImageDescription/ImageNameDef
 * ascii name of image
 */
ImageNameDef	: STRINGTOKEN { ThisImage = cs($1); }
		;
_Image		:
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
Technology	: LBR TECHNOLOGY _Technology RBR
		;
_Technology	: STRINGTOKEN  { } /* currently not used */
		| _Technology DesignRules
		| _Technology Comment
		| _Technology UserData
		;
/*
 * Technology/designrules
 */
DesignRules	: LBR TECHDESIGNRULES _DesignRules RBR
		;
_DesignRules	: TechNumLayer	/* number of layers before others */
		| _DesignRules TechLayOrient
		| _DesignRules TechWireWidth
		| _DesignRules TechWireMask
		| _DesignRules TechViaMask
		| _DesignRules TechViaCellName
		| _DesignRules Comment
		| _DesignRules UserData
		;
/*
 * Designrules/numlayer: number of metal layers,
 * In this routine a number of arrays are allocated
 */
TechNumLayer	: LBR TECHNROFLAYERS NUMBER RBR
		{
		    allocate_layer_arrays (atol($3));
		}
		;
/*
 * declares the orientation of a layer
 */
TechLayOrient	: LBR TECHWIREORIENT NUMBER STRINGTOKEN RBR
		{
		    if (atol($3) < 0 || atol($3) >= Chip_num_layer)
			yderror ("Illegal layer index for orient");
		    else {
			switch ($4[0]) {
			case 'h': case 'H': case 'x': case 'X':
			    LayerOrient[atol($3)] = HORIZONTAL;
			    break;
			case 'v': case 'V': case 'y': case 'Y':
			    LayerOrient[atol($3)] = VERTICAL;
			    break;
			default:
			    yderror ("warning: Unknown orientation for layerorient");
			}
		    }
		}
		;
/*
 * declares wire width in a layer
 */
TechWireWidth	: LBR TECHWIREWIDTH NUMBER NUMBER RBR
		{
		}
		;
/*
 * declares mask name of a layer
 */
TechWireMask	: LBR TECHWIREMASKNAME NUMBER STRINGTOKEN RBR
		{
		}
		;
/*
 * declares the mask name of a via
 */
TechViaMask	: LBR TECHVIAMASKNAME NUMBER NUMBER STRINGTOKEN RBR
		{
		}
		;
/*
 * declares the cell name of a via
 */
TechViaCellName : LBR TECHVIACELLNAME NUMBER NUMBER STRINGTOKEN RBR
		{
		}
		;

/*
 * Image/CircuitImage
 * Currently not implemented
 */
CircuitImage	: LBR CIRCUITIMAGE /* anything RBR */
		{
		    bracecount = 1;
		    while (bracecount > 0)
			if ((sometoken = ydlex ()) == LBR)
			    ++bracecount;
			else if (sometoken == RBR)
			    --bracecount;

		 /* printf ("CircuitImage\n"); */
		}
		;

/*
 * Image/LayoutImage
 */
LayoutImage	: LBR LAYOUTIMAGE _LayoutImage RBR
		;
_LayoutImage	:
		| _LayoutImage LayModelCall
		| _LayoutImage LayImageRep
		| _LayoutImage LayImagePat
		| _LayoutImage Comment
		| _LayoutImage UserData
		;
LayModelCall	: LBR LAYOUTMODELCALL STRINGTOKEN RBR
		{ /* store image name */
		}
		;
LayImageRep	: LBR LAYOUTIMAGEREPITITION NUMBER NUMBER RBR
		{ /* store repititions */
		}
		;
LayImagePat	: LBR LAYOUTIMAGEPATTERN _LayImagePat RBR
		;
_LayImagePat	:
		| _LayImagePat LayImPatLay
		| _LayImagePat Comment
		| _LayImagePat UserData
		;
LayImPatLay	: LBR LAYOUTIMAGEPATTERNLAYER STRINGTOKEN _LayImPatLay RBR
		;
_LayImPatLay	:
		| _LayImPatLay Rectangle
		| _LayImPatLay Comment
		| _LayImPatLay UserData
		;
Rectangle	: LBR RECTANGLE NUMBER NUMBER NUMBER NUMBER RBR
		;

/*
 * Image/GridImage
 * Interesting
 */
GridImage	: LBR GRIDIMAGE _GridImage RBR
		;
_GridImage	: GridSize
		| _GridImage GridConnectList
		| _GridImage GridCostList
		| _GridImage Comment
		| _GridImage UserData
		;
/*
 * specifies size of the basic grid
 */
GridSize	: LBR GRIDSIZE GridBbx _GridSize RBR
		;
GridBbx		: NUMBER NUMBER /* dimensions to allocate */
		{
		    if (GridRepitition[X] > 0) {
			yderror ("redeclaration of GridSize");
			error (FATAL_ERROR, "parser");
		    }

		    GridRepitition[X] = atol($1);
		    GridRepitition[Y] = atol($2);

		    // allocate lots of global array related to the image size
		    allocate_core_image (atol($1), atol($2));
		    orient = -1;
		}
		;
_GridSize	:
		| _GridSize GridMapping
		| _GridSize GridTearLine
		| _GridSize GridPowerLine
		| _GridSize Comment
		| _GridSize UserData
		;
/* mapping of gridpositions to layoutpositions */
GridMapping	: LBR GRIDMAPPING GridMapOrient _GridMapping RBR
		{
		}
		;
GridMapOrient	: STRINGTOKEN	/* should specify horizontal of vertical */
		{ /* set index */
		}
		;
_GridMapping	:
		| _GridMapping NUMBER    /* read in number */
		{ /* add number to array */
		}
		;

/* positions of power lines : ( PowerLine <orient> <ident> <layer_no> <row or column number> ) */
GridPowerLine	: LBR GRIDPOWERLINE STRINGTOKEN STRINGTOKEN NUMBER NUMBER RBR
		{
		    POWERLINEPTR pline;

		    /* allocate new, link in list */
		    NewPowerLine (pline);
		    if (PowerLineList) pline->next = PowerLineList;
		    PowerLineList = pline;

		    /* fill struct: orient */
		    switch ($3[0]) {
		    case 'h': case 'H': case 'x': case 'X':
			pline->orient = X;
			break;
		    case 'v': case 'V': case 'y': case 'Y':
			pline->orient = Y;
			break;
		    default:
			yderror ("warning: Powerline has unknown orientation (should be 'horizontal' or 'vertical')");
			pline->orient = X;
		    }

		    /* fill struct: name */
		    pline->type = gimme_net_type ($4);
		    switch (pline->type) {
		    case VDD:
		    case VSS: break;
		    default:
			yderror ("WARNING: Powerline has unknown type (should be 'vss' or 'vdd', 'vss' assumed)");
			pline->type = VSS;
		    }

		    /* fill struct: layer number */
		    pline->z = atoi($5);
		    if (pline->z < 0 || pline->z >= Chip_num_layer) {
			yderror ("WARNING: Powerline in illegal layer:");
			fprintf (stderr, "   layer no = %ld (set to 0)\n", pline->z);
			pline->z = 0;
		    }

		    /* fill struct: row or column number */
		    pline->x = pline->y = atol($6);
		    if (pline->x > GridRepitition[opposite(pline->orient)]) {
			yderror ("WARNING: row or column coordinate of power line is out or range");
			fprintf (stderr, "%ld is size of image, %ld was specified, 0 assumed\n",
			    GridRepitition[opposite(pline->orient)], pline->x);
			pline->x = pline->y = 0;
		    }
		 /* printf ("POWER: orient = %d, type = %d, layer = %d, row = %d\n",
		    (int) pline->orient, (int) pline->type, (int) pline->z, (int) pline->x); */
		}
		;

/* positions of Tearlines */
GridTearLine	: LBR GRIDTEARLINE TearLineOrient _GridTearLine RBR
		{
		    if (counter != NumImageTearLine[orient] && counter >= 0) {
			yderror ("warning: incomplete tearline list");
			fprintf (stderr, "%ld of the %ld points found\n", counter, GridRepitition[orient]);
			NumImageTearLine[orient] = counter;
		    }
		}
		;

TearLineOrient	: STRINGTOKEN NUMBER /* should specify horizontal of vertical, followed by the number of tearlines */
		{ /* set index */
		    switch ($1[0]) {
		    case 'h': case 'H': case 'x': case 'X':
			orient = X;
			break;
		    case 'v': case 'V': case 'y': case 'Y':
			orient = Y;
			break;
		    default:
			yderror ("warning: Tearline has unknown orientation (should be 'horizontal' or 'vertical')");
			orient = X;
		    }

		    if (ImageTearLine[orient]) {
			yderror ("warning: redeclaration of tearlines");
			fprintf (stderr, "direction '%s' was already declared\n", $1);
		    }

		    if ((NumImageTearLine[orient] = atol($2)) < 0) NumImageTearLine[orient] = 0;

		    if (NumImageTearLine[orient] > GridRepitition[orient]) {
			yderror ("warning: possibly too many tearlines for this image");
			fprintf (stderr, "%ld is size of image, %ld points are declared. trunctating...\n",
			    GridRepitition[orient], NumImageTearLine[orient]);
			NumImageTearLine[orient] = GridRepitition[orient];
		    }

		    CALLOC (ImageTearLine[orient], long, NumImageTearLine[orient]);
		    counter = 0; /* reset index */
		}
		;

_GridTearLine	:
		| _GridTearLine NUMBER    /* read in number */
		{ /* add number to array */
		    if (counter >= 0)
		    { /* negative indicates out of bounds */
			if (counter < NumImageTearLine[orient]) {
			    ImageTearLine[orient][counter++] = atol($2);
			}
			else {
			    yderror ("warning: more tear lines specified than declared");
			    fprintf (stderr, "should be %ld points; points starting from '%s' ignored\n",
				NumImageTearLine[orient], $2);
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
_GridConnectList :
		| _GridConnectList Block
		| _GridConnectList RestrictedFeed
		| _GridConnectList UniversalFeed
		| _GridConnectList Comment
		| _GridConnectList UserData
		;
Block		: LBR GRIDBLOCK NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{
		    if (add_grid_block (atol($3), atol($4), atol($5), atol($6), atol($7), atol($8)) == FALSE)
		    {
			yderror ("grid block position outside image");
		    }
		}
		;
UniversalFeed	: LBR UNIVERSALFEED
		{
		    Feedlist = NULL;
		}
		  _GridFeed RBR
		{
		    if (process_feeds (Feedlist, CoreFeed) == FALSE)
			fprintf (stderr, "error was on line %d\n", ydlineno);
		}
		;
RestrictedFeed	: LBR RESTRICTEDFEED
		{
		    Feedlist = NULL;
		}
		  _GridFeed RBR
		{
		    if (process_feeds (Feedlist, RestrictedCoreFeed) == FALSE)
			fprintf (stderr, "error was on line %d\n", ydlineno);
		}
		;
_GridFeed	:
		| _GridFeed GridFeedRef
		| _GridFeed Comment
		| _GridFeed UserData
		;
GridFeedRef	: LBR FEED STRINGTOKEN NUMBER NUMBER RBR
		{ GRIDPOINTPTR new_point;
		    new_point = new_gridpoint (atol($4), atol($5), 0);
		    new_point -> next = Feedlist;
		    Feedlist = new_point;
		}
		;

/*
 * declares costs of point
 * should be called after GridCOnnectList
 */
GridCostList	: LBR GRIDCOSTLIST _GridCostList RBR
		{
		    /* if ready: link offset arrays */
		    link_feeds_to_offsets ();
		}
		;
_GridCostList	:
		| _GridCostList GridCost
		| _GridCostList FeedCost
		| _GridCostList Comment
		| _GridCostList UserData
		;
GridCost	: LBR GRIDCOST NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{ /* <cost> <vector> <startpoint_range> <endpoint_range> */
		    if (set_grid_cost (atol($3), atol($4), atol($5), atol($6), atol($7),
			    atol($8), atol($9), atol($10), atol($11), atol($12)) == FALSE)
			yderror ("WARNING: GridCost has no effect, no such offset found on range");
		}
		;
FeedCost	: LBR FEEDCOST NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{ /* <cost> <vector> <startpoint_range> <endpoint_range> */
		    if (set_feed_cost (atol($3), atol($4), atol($5), atol($6), atol($7),
			    atol($8), atol($9), atol($10), atol($11), atol($12)) == FALSE)
			yderror ("WARNING: FeedCost has no effect, no such offset found on range");
		}
		;

/*
 * Library module
 */
Library		: LBR LIBRARYTOKEN anything RBR
		;

/*
 * BASIC functions
 */
                 /* status block called on many levels */
Status		: LBR STATUSTOKEN _Status RBR
		{
		 /* printf ("Status detected on line %d\n", ydlineno); */
		}
		;
_Status		:
		| _Status Written
		| _Status Comment
		| _Status UserData
		;
Written		: LBR WRITTEN _Written RBR
		;
_Written	: TimeStamp
		| _Written Author
		| _Written Program
	/*	| _Written DataOrigin     not implemented */
	/*	| _Written Property       not implemented */
		| _Written Comment
		| _Written UserData
		;

				/* year month day hour minute second */
TimeStamp	: LBR TIMESTAMP NUMBER NUMBER NUMBER NUMBER NUMBER NUMBER RBR
		{
		 /* printf ("Timestamp detected on line %d: %ld %ld %ld %ld %ld %ld\n",
			ydlineno, atol($3), atol($4), atol($5), atol($6), atol($7), atol($8)); */
		}
		;
Author		: LBR AUTHOR anything RBR  /* anything should be a string */
		{
		 /* printf ("Author detected on line %d\n", ydlineno); */
		}
		;
Program		: LBR PROGRAM anything RBR /* anything should be a string, followed by a version */
		{
		 /* printf ("Program signature detected on line %d\n", ydlineno); */
		}
		;

UserData	: LBR
		{
		    if (Verbose_parse) printf ("unknown keyword in line %d:", ydlineno);
		}
		  STRINGTOKEN
		{
		    bracecount = 1;
		    while (bracecount > 0)
			if ((sometoken = ydlex ()) == LBR)
			    ++bracecount;
			else if (sometoken == RBR)
			    --bracecount;

		    if (Verbose_parse) printf (" %s (skipped until line %d)\n", $3, ydlineno);
		}
		;

Comment		: LBR COMMENT
		{
		    bracecount = 1;
		    while (bracecount > 0)
			if ((sometoken = ydlex ()) == LBR)
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

#include "imagelex.h"

void yderror (char *s)
{
    fflush (stdout);
    fprintf (stderr, "ERROR (Seadif image file): %s.  Try line %d.\n", s, ydlineno);
}
