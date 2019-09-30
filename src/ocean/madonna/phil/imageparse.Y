%{
/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
 * Seadif image parser
 */

#include <iostream>
using namespace std;
#include <stdlib.h>
#include <string.h>

#include "src/ocean/libocean/format.h"

#include "src/ocean/madonna/phil/sea.h"
#include "src/ocean/madonna/phil/parserIf.h"
#include "src/ocean/madonna/phil/plcm.h"

extern ParserInterface  parInt;          // main global object
extern int pplineno;

void pperror (const char *);
int  pplex ();

// Note: flex forgets to rename yyinput; therefore, do it ourselves.
#define ppinput yyinput

/*
 * export
 */
static int sometoken, bracecount; /* used for skipping unknown keywords */
static int Verbose_parse = 0;
static int imageVerDim, imageHorDim;
static Boolean images_ini = false;
static List *FeedList;

extern "C" int ppwrap (void);
int ppwrap (void)
{
    return 1;
}

%}

%union {
         int itg;
	 char str[200];
}

%token <str> STRINGTOKEN
%token <str> NUMBER
%token LBR RBR
%token SEADIFTOKEN
%token IMAGEDESCRIPTION
%token GRIDIMAGE
%token GRIDCONNECTLIST
%token RESTRICTEDFEED
%token UNIVERSALFEED
%token EXTERNALFEED
%token BUSFEED
%token FEED
%token AXIS
%token MIRRORAXIS
%token GRIDSIZE
%token IMAGEOVERLAP
%token TECHNOLOGY
%token TECHDESIGNRULES
%token TECHNROFLAYERS
%token TECHWIREORIENT
%token ESTIMATEDTRANSPARENCY
%token POWERLINE_TOK
%token COMMENT

%start Seadif

%%
Seadif		: LBR SEADIFTOKEN SeadifFileName _Seadif RBR
		    /* EdifVersion, EdifLevel and KeywordMap skipped */
		;
SeadifFileName	: STRINGTOKEN { }
		;
_Seadif		: /* nothing */
		| _Seadif Image
		| _Seadif Comment
		| _Seadif UserData
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
ImageNameDef	: STRINGTOKEN { }
		;
_Image		:
		| _Image GridImage
		| _Image Technology
		| _Image Comment
		| _Image UserData
		;

/*
 * specifies size of the basic grid
 */
GridSize	: LBR GRIDSIZE GridBbx _GridSize RBR
		;
GridBbx		: NUMBER NUMBER
		{
		    parInt.setDim (imageHorDim = atoi($1), imageVerDim = atoi($2));
		}
		;
_GridSize	: /* nothing */
		| _GridSize ImageOverlap
		| _GridSize EstimatedTransparency
		| _GridSize PowerLine
		| _GridSize UserData
		;

ImageOverlap	: LBR IMAGEOVERLAP NUMBER NUMBER RBR
		{
		    parInt.addImageOverlap (atoi($3), atoi($4));
		}
		;

EstimatedTransparency : LBR ESTIMATEDTRANSPARENCY NUMBER NUMBER RBR
		{
		    parInt.setTransparency (atoi($3), atoi($4));
		}
		;

PowerLine	: LBR POWERLINE_TOK STRINGTOKEN STRINGTOKEN NUMBER NUMBER RBR
		{
		    int orient;
		    switch ($3[0])
		    {
		    case 'h': case 'H': case 'x': case 'X':
			orient = HOR;
			break;
		    case 'v': case 'V': case 'y': case 'Y':
			orient = VER;
			break;
		    default:
			orient = HOR;
			pperror ("warning: Unknown orientation for LayerOrient");
		    }
		    int layer = atoi($5);
		    if (layer < 0 || layer >= parInt.numLayers ())
			pperror ("Illegal layer index for PowerLine");
		    parInt.addPowerLine (orient, $4, layer, atoi($6));
		}
		;

/*
 * Image/GridImage
 * Interesting
 */
GridImage	: LBR GRIDIMAGE _GridImage RBR
		;
_GridImage	: GridSize
		| _GridImage Axis
		| _GridImage GridConnectList
		| _GridImage Comment
		| _GridImage UserData
		;

Axis		: LBR AXIS MirrorAxis RBR
		{
		    parInt.initImages ();
		    images_ini = true;
		}
		;
MirrorAxis	: /*empty*/
		| MirrorAxis _MirrorAxis
		;
_MirrorAxis	: LBR MIRRORAXIS NUMBER NUMBER NUMBER NUMBER RBR
		{
		    MIRRORPTR mp;
		    NewMirror (mp);
		    mp->axis[X1] = atoi($3);
		    mp->axis[X2] = atoi($4);
		    mp->axis[Y1] = atoi($5);
		    mp->axis[Y2] = atoi($6);
		    parInt.addMirrorAxis (mp);
		}
		;
/*
 * Define connections/neighbours of grid points
 */

GridConnectList : LBR GRIDCONNECTLIST _GridConnectList RBR
		;
_GridConnectList :
		| _GridConnectList RestrictedFeed
		| _GridConnectList UniversalFeed
		| _GridConnectList BusFeed
		| _GridConnectList Comment
		| _GridConnectList UserData
		;

UniversalFeed	: LBR UNIVERSALFEED
		{
		    if (!images_ini) {
			cerr << "\nWarning : no mirror axes of image specified ! " << endl;
			parInt.initImages ();
			images_ini = true;
		    }
		    FeedList = new List();
		}
		  _GridFeed RBR
		{
		    if (parInt.processFeeds (*FeedList, PolyLayer))
			cerr << "error was on line " << pplineno << "\n";
		    delete FeedList;
		}
		;

RestrictedFeed	: LBR RESTRICTEDFEED
		{
		    if (!images_ini) {
			cerr << "\nWarning : no mirror axes of image specified ! " << endl;
			parInt.initImages ();
			images_ini = true;
		    }
		    FeedList = new List();
		}
		  _GridFeed RBR
		{
		    if (parInt.processFeeds (*FeedList, DiffusionLayer))
			cerr << "error was on line " << pplineno << "\n";
		    delete FeedList;
		}
		;

BusFeed		: LBR BUSFEED
		{
		    if (!images_ini) {
			cerr << "\nWarning : no mirror axes of image specified ! " << endl;
			parInt.initImages ();
			images_ini = true;
		    }
		    FeedList = new List();
		}
		  _GridFeed RBR
		{
		    if (parInt.processFeeds (*FeedList, Metal1Layer))
			cerr << "error was on line " << pplineno << "\n";
		    delete FeedList;
		}
		;

_GridFeed	:
		| _GridFeed GridFeedRef
		| _GridFeed ExternalFeedRef
		| _GridFeed Comment
		| _GridFeed UserData
		;

GridFeedRef	: LBR FEED STRINGTOKEN NUMBER NUMBER RBR
		{
		    int x = atoi($4),
			y = atoi($5);
		    if (x >= 0 && x < imageHorDim && y >= 0 && y < imageVerDim)
		    {
			CriPoint &new_point = *new CriPoint (x, y, NoneLayer, 0);
			FeedList->add(new_point);
		    }
		}
		;

ExternalFeedRef : LBR EXTERNALFEED STRINGTOKEN RBR
		{
		    CriPoint *new_point = NULL;

		    if (strcmp ($3, "hor") == 0)
			new_point = new CriPoint (0, 0, NoneLayer, 1);
				/* net == 1 means ExternalFeed "hor"*/
		    else
		    if (strcmp ($3, "ver") == 0)
			new_point = new CriPoint (0, 0, NoneLayer, 2);
				/* net == 2 means ExternalFeed "ver"*/
		    else
			cerr << "error was on line " << pplineno << "\n";
		    if (new_point) FeedList->add (*new_point);
		}
		;

Technology	: LBR TECHNOLOGY _Technology RBR
		;
_Technology	: STRINGTOKEN  { }    /* currently not used */
		| _Technology DesignRules
		| _Technology Comment
		| _Technology UserData
		;

DesignRules	: LBR TECHDESIGNRULES _DesignRules RBR
		;
_DesignRules	: TechNumLayer        /* number of layers before others */
		| _DesignRules TechLayOrient
		| _DesignRules Comment
		| _DesignRules UserData
		;

TechNumLayer	: LBR TECHNROFLAYERS NUMBER RBR
		{
		    parInt.setNumLayers (atoi($3));
		}
		;
/*
 * declares the orientation of a layer
 */
TechLayOrient	: LBR TECHWIREORIENT NUMBER STRINGTOKEN RBR
		{
		    if (atol($3) < 0 || atol($3) >= parInt.numLayers ())
			pperror ("Illegal layer index for WireOrient");
		    else {
			switch ($4[0]) {
			case 'h': case 'H': case 'x': case 'X':
			    parInt.setRouteOrient (atoi($3), HOR);
			    break;
			case 'v': case 'V': case 'y': case 'Y':
			    parInt.setRouteOrient (atoi($3), VER);
			    break;
			default:
			    pperror ("warning: Unknown orientation for LayerOrient");
			}
		    }
		}
		;

UserData	: LBR
		{
		    if (Verbose_parse) printf ("unknown keyword in line %d:", pplineno);
		}
		  STRINGTOKEN
		{
		    bracecount = 1;
		    while (bracecount > 0)
			if ((sometoken = pplex ()) == LBR)
			    ++bracecount;
			else if (sometoken == RBR)
			    --bracecount;
		    if (Verbose_parse) printf (" %s (skipped until line %d)\n", $3, pplineno);
		}
		;

Comment		: LBR COMMENT
		{
		    bracecount = 1;
		    while (bracecount > 0)
			if ((sometoken = pplex ()) == LBR)
			    ++bracecount;
			else if (sometoken == RBR)
			    --bracecount;
		 /* printf ("Comment detected on line %d\n", pplineno); */
		}
		;
%%

#include "imagelex.h"

void pperror (const char *s)
{
    cerr << form("ERROR (Seadif image file): %s.  Try line %d.\n", s, pplineno);
}
