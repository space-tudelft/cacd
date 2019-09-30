/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include "src/space/highlay/incl.h"

#ifndef NOPACK
#define dmGET _dmUnpack
#else
#define dmGET _dmDoget
#endif

#define dmCheckOutCell(c,v,m) dmCheckOut(dmproject,c,version,DONTCARE,v,m)

extern char *argv0;
extern char *optarg;
extern int optind;

DM_CELL          * extCellCirKey;
DM_CELL          * extCellLayKey;
DM_CELL          * outCellLayKey;
DM_PROCDATA      * process;
DM_PROJECT       * dmproject;

bool_t optSpecial  = FALSE;
bool_t optNoCopy   = FALSE;
bool_t optCopyAsChild = TRUE;
bool_t optOrigMasks = FALSE;
bool_t optCondMasks = FALSE;
bool_t optVerbose  = FALSE;
double optWidth = 0.0;
long grow_size = -1.0;

bool_t optMatching     = FALSE;
bool_t optInconclusive = FALSE;
bool_t optDeficient    = FALSE;
bool_t optNets         = FALSE;
bool_t optPorts        = FALSE;
bool_t optDevices      = FALSE;
char * netGroups = NULL;
char * portGroups = NULL;
char * devGroups = NULL;
char * optLayer = NULL;
char * optCell = NULL;
char ** progArgs = NULL;

struct net_ref *Begin_net_ref;
char **Array_net_ref;
int Array_net_nr;
int outScale;
int high_layer_no = -1;
int noMasks;
char high_layer[256];
char outcell[256];

#ifdef __cplusplus
  extern "C" {
#endif
Private void readCommands (char *namefile);
Private void copyLayout (DM_CELL *cellKey1, DM_CELL *cellKey2);
Private void readGeo (FILE *fp, DM_CELL *layKey, char *file);
Private int testName (char *name);
Private void makeArrayName (void);
#ifdef __cplusplus
  }
#endif

int main (int argc, char **argv)
{
    int   c, errflg = 0;
    char  * version = WORKING;	/* default version */
    char  *optstring, *cellname, *file;
    char  circellname[128];
    char  * namefile = NULL;
    int i;
    DM_STREAM *info3;
    DM_STREAM *dmsGeo;

    argv0 = "highlay";

    catchSignals ();

    strcpy (outcell, "HIGH_OUT");  /* default */

 /* evaluate all flags and filenames */

    if (argc > 1 && argv[1][0] == '-' && argv[1][1] == '%')
	optstring = "%midnN:pP:eE:l:c:LoFrw:v";
    else
	optstring = "midnN:pP:eE:l:c:LoFrw:v";

    while ((c = getopt (argc, argv, optstring)) != EOF) {
	switch (c) {
	    case '%': optSpecial++;          break;
	    case 'm': optMatching++;         break;
	    case 'i': optInconclusive++;     break;
	    case 'd': optDeficient++;        break;
	    case 'n': optNets++;             break;
	    case 'N': netGroups = strsave (optarg); break;
	    case 'p': optPorts++;             break;
	    case 'P': portGroups = strsave (optarg); break;
	    case 'e': optDevices++;          break;
	    case 'E': devGroups = strsave (optarg); break;
	    case 'l': optLayer = strsave (optarg); break;
	    case 'c': optCell = strsave (optarg); break;
	    case 'w': optWidth = atof (optarg); break;
	    case 'o': optNoCopy++;           break;
	    case 'F': optCopyAsChild = 0;    break;
	    case 'r': optOrigMasks++;        break;
	    case 'L': optCondMasks++;        break;
	    case 'v': optVerbose++;          break;
	    case '?': errflg++;              break;
	}
    }
    progArgs = argv + optind;

    if (!*progArgs) {
	say ("No cellname specified\n");
	errflg++;
    }
    cellname = *progArgs;

    if (*(progArgs + 1)) namefile = *(progArgs + 1);

    if (namefile) {
	if (!(optNets || optPorts || optDevices)) {
	    say ("specify at least one of the options -n -p or -e");
	    errflg++;
	}
    }
    else {
	if (!(optNets || optPorts || optDevices
	      || netGroups || portGroups || devGroups)) {
	    say ("specify at least one of the options -n -N -p -P -e or -E");
	    errflg++;
	}
	if (!(optMatching || optDeficient || optInconclusive)) {
	    say ("specify at least one of the options -m -i or -d");
	    say ("or specify a namefile argument");
	    errflg++;
	}
    }

    if (errflg) {
	printf ("\nUsage: %s%s%s\n%s%s\n\n", argv0, optSpecial? " [-%]" : "",
	" [-npeoFvmidL -l mask -w width -c outcell",
	"                   -N n1,n2... -P p1,p2... -E e1,e2...]",
	" cell [namefile]");
	exit (1);
    }

    dmInit (argv0);

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, dmproject);
    noMasks = process -> nomasks;

    high_layer[0] = '\0';

    Begin_net_ref = NULL;

    if (namefile) readCommands (namefile);

    if (optLayer) strcpy (high_layer, optLayer);

    if (optCell) strcpy (outcell, optCell);

    if (high_layer[0]) {
	for (i = 0; i < noMasks; i++) {
	    if (strsame (process -> mask_name[i], high_layer)) {
		high_layer_no = i;
		break;
	    }
	}
	if (high_layer_no < 0) {
	    say ("Illegal mask for high-lighting: '%s'", high_layer);
	    die ();
	}
    }
    else
	if (!optCondMasks) optOrigMasks = TRUE;

    if (optOrigMasks || optCondMasks) {
        if (optWidth > 0.0) {
	    say ("Option -w has no effect when not using a special mask for high-lighting.");
        }
        grow_size = -1;
    }
    else {
        grow_size = (long) (optWidth / dmproject -> lambda);
    }

    if (!_dmExistCell (dmproject, cellname, LAYOUT)) {
	say ("Cannot open layout cell '%s'", cellname);
	die ();
    }

    extCellLayKey = dmCheckOutCell (cellname, LAYOUT, READONLY);

    circellname[0] = '\0';
    if (_dmExistCell (dmproject, cellname, CIRCUIT))
	strcpy (circellname, cellname);
    else {
	if (cellname[0] >= 'a' && cellname[0] <= 'z') {
	    /* convert first letter to capital and try again */
	    strcpy (circellname, cellname);
	    circellname[0] = circellname[0] + 'A' - 'a';
	    if (_dmExistCell (dmproject, circellname, CIRCUIT)) {
		if (optVerbose)
		    say ("Using information of circuit cell '%s'", circellname);
	    }
	    else
		circellname[0] = '\0';
	}
    }
    if (circellname[0] == '\0')
	say ("Cannot open circuit cell '%s'", cellname);

    extCellCirKey = dmCheckOutCell (circellname, CIRCUIT, READONLY);

    if (!namefile) {
	struct stat sbuf;
	if (dmStat (extCellCirKey, "match", &sbuf) != 0) {
	    say ("Cannot read match results\n");
	    die ();
	}
	readMatchOutput (extCellCirKey);
    }

    makeArrayName ();

    outCellLayKey = dmCheckOutCell (outcell, LAYOUT, UPDATE);

    info3 = dmOpenStream (extCellLayKey, "info3", "r");
    dmGetDesignData (info3, GEO_INFO3);
    dmCloseStream (info3, COMPLETE);

    if ((outScale = (int)ginfo3.nr_samples) == 0) outScale = SCALE;

    copyLayout (extCellLayKey, outCellLayKey);

    dmCheckIn (extCellLayKey, COMPLETE);

    if (optNets || optPorts || (!namefile && (netGroups || portGroups))) {
	struct stat sbuf;

	file = "congeo";
	if (dmStat (extCellCirKey, file, &sbuf)) {
	    say ("Cannot open stream '%s'.\n   %s.\n",
		file, "First run space with the option -x");
	    die ();
	}

	dmsGeo = dmOpenStream (extCellCirKey, file, "r");

	if (optVerbose) {
	    if (!optPorts && !portGroups)
		fprintf (stdout, "Selected nets:\n");
	    else if (!optNets && !netGroups)
		fprintf (stdout, "Selected ports:\n");
	    else
		fprintf (stdout, "Selected nets/ports:\n");
	}

	readGeo (dmsGeo -> dmfp, outCellLayKey, file);

	dmCloseStream (dmsGeo, COMPLETE);
    }

    if (optDevices || (!namefile && devGroups)) {
	struct stat sbuf;

	file = "devgeo";
	if (dmStat (extCellCirKey, file, &sbuf)) {
	    say ("Cannot open stream '%s'.\n   %s.\n",
		file, "First run space with the option -x");
	    die ();
	}

	dmsGeo = dmOpenStream (extCellCirKey, file, "r");

	if (optVerbose)
	    fprintf (stdout, "Selected devices:\n");

	readGeo (dmsGeo -> dmfp, outCellLayKey, file);

	dmCloseStream (dmsGeo, COMPLETE);
    }

    dmCheckIn (extCellCirKey, COMPLETE);
    dmCheckIn (outCellLayKey, COMPLETE);

    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    verbose ("%s --- Finished ---\n", argv0);
    return (0);
}

Private void readCommands (char *namefile)
{
    char name[256];
    struct net_ref *new_net_ref;
    struct net_ref *search_ref;
    struct net_ref *prev_ref;
    FILE *fp;
    int len;

    if ((fp = fopen (namefile, "r")) == NULL) {
	say ("Cannot open %s\n", namefile);
	die ();
    }

    while (fscanf (fp, "%s", name) > 0) {

	/* Only for backwards compatibility. */

	if (strncmp (name, "highlay:", 8) == 0) {
	    if (name[8] != '\0')
		sscanf (name + 8, "%s", high_layer);
	    else
		fscanf (fp, "%s", high_layer);
	    continue;
	}
	else if (strncmp (name, "outcell:", 8) == 0) {
	    if (name[8] != '\0')
		sscanf (name + 8, "%s", outcell);
	    else
		fscanf (fp, "%s", outcell);
	    continue;
	}

	new_net_ref = NEW (struct net_ref, 1);
	len = strlen (name) + 1;
	new_net_ref -> name = NEW (char, len);
	strcpy (new_net_ref -> name, name);

	if (Begin_net_ref) {
	    search_ref = Begin_net_ref;
	    prev_ref = NULL;
	    while (search_ref && strcmp (search_ref -> name, name) < 0) {
		prev_ref = search_ref;
		search_ref = search_ref -> next;
	    }
	    if (search_ref && strsame (search_ref -> name, name)) {
		/* double; skip it */
		DISPOSE (new_net_ref -> name, len);
		DISPOSE (new_net_ref, sizeof(struct net_ref));
	    }
	    else if (prev_ref) {
		new_net_ref -> next = prev_ref -> next;
		prev_ref -> next = new_net_ref;
	    }
	    else {
		new_net_ref -> next = Begin_net_ref;
		Begin_net_ref = new_net_ref;
	    }
	}
	else {
	    Begin_net_ref = new_net_ref;
	    new_net_ref -> next = NULL;
	}
    }

    fclose (fp);
}

Private void makeArrayName ()
{
    int i;
    struct net_ref *search_ref;

    Array_net_nr = 0;

    search_ref = Begin_net_ref;
    while (search_ref) {
	Array_net_nr++;
	search_ref = search_ref -> next;
    }

    Array_net_ref = NEW (char *, Array_net_nr + 1);

    search_ref = Begin_net_ref;
    i = 0;
    while (search_ref) {
	Array_net_ref[i++] = search_ref -> name;
	search_ref = search_ref -> next;
    }
    Array_net_ref[i] = "";
}

Private void copyLayout (DM_CELL *cellKey1, DM_CELL *cellKey2)
{
   DM_STREAM *dms1, *dms2;
   long bxl, bxr, byb, byt;
   int i;

   dms1 = dmOpenStream (cellKey1, "info", "r");
   dms2 = dmOpenStream (cellKey2, "info", "w");
   i = dmGetDesignData (dms1, GEO_INFO);
   ASSERT (i > 0);
   bxl = ginfo.bxl; bxr = ginfo.bxr;
   byb = ginfo.byb; byt = ginfo.byt;
   dmPutDesignData (dms2, GEO_INFO);

   while (dmGetDesignData (dms1, GEO_INFO) > 0) {
       dmPutDesignData (dms2, GEO_INFO);
   }
   dmCloseStream (dms1, COMPLETE);
   dmCloseStream (dms2, COMPLETE);

   if (optNoCopy) {
       dms2 = dmOpenStream (cellKey2, "box", "w");
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "term", "w");
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "mc", "w");
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "nor", "w");
       dmCloseStream (dms2, COMPLETE);
   }
   else if (optCopyAsChild) {
       dms2 = dmOpenStream (cellKey2, "box", "w");
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "term", "w");
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "mc", "w");
       strcpy (gmc.cell_name, cellKey1 -> cell);
       strcpy (gmc.inst_name, ".");
       gmc.imported = LOCAL;
       gmc.mtx[0] = 1, gmc.mtx[4] = 1;
       gmc.mtx[1] = 0, gmc.mtx[2] = 0;
       gmc.mtx[3] = 0, gmc.mtx[5] = 0;
       gmc.bxl = bxl, gmc.bxr = bxr, gmc.byb = byb, gmc.byt = byt;
       gmc.nx = 0; gmc.dx = 0;
       gmc.ny = 0; gmc.dy = 0;
       dmPutDesignData (dms2, GEO_MC);
       dmCloseStream (dms2, COMPLETE);
       dms2 = dmOpenStream (cellKey2, "nor", "w");
       dmCloseStream (dms2, COMPLETE);
   }
   else {
       dms1 = dmOpenStream (cellKey1, "box", "r");
       dms2 = dmOpenStream (cellKey2, "box", "w");
       while (dmGetDesignData (dms1, GEO_BOX) > 0) {
	   dmPutDesignData (dms2, GEO_BOX);
       }
       dmCloseStream (dms1, COMPLETE);
       dmCloseStream (dms2, COMPLETE);

       dms1 = dmOpenStream (cellKey1, "term", "r");
       dms2 = dmOpenStream (cellKey2, "term", "w");
       while (dmGetDesignData (dms1, GEO_TERM) > 0) {
	   dmPutDesignData (dms2, GEO_TERM);
       }
       dmCloseStream (dms1, COMPLETE);
       dmCloseStream (dms2, COMPLETE);

       dms1 = dmOpenStream (cellKey1, "mc", "r");
       dms2 = dmOpenStream (cellKey2, "mc", "w");
       while (dmGetDesignData (dms1, GEO_MC) > 0) {
	   dmPutDesignData (dms2, GEO_MC);
       }
       dmCloseStream (dms1, COMPLETE);
       dmCloseStream (dms2, COMPLETE);

       dms1 = dmOpenStream (cellKey1, "nor", "r");
       dms2 = dmOpenStream (cellKey2, "nor", "w");
       while (dmGetDesignData (dms1, GEO_NOR_INI) > 0) {
	   dmPutDesignData (dms2, GEO_NOR_INI);
	   for (i = 0; i < gnor_ini.no_xy; i++) {
	       dmGetDesignData (dms1, GEO_NOR_XY);
	       dmPutDesignData (dms2, GEO_NOR_XY);
	   }
       }
       dmCloseStream (dms1, COMPLETE);
       dmCloseStream (dms2, COMPLETE);
    }
}

Private void readGeo (FILE *fp, DM_CELL *layKey, char *file)
{
    int i, printed, select, n, majorNr, layer_no, mask;
    char c;
    DM_STREAM *dmsBox, *dmsNor;
    char buf[1024];
    long xl, xr, bl, br, tl, tr;
    mask_t color, mcolor;
    long tile, co, cx, m;
    int warningMissingHMask = 0;

    if ((c = getc (fp)) == '#') {
	if ((c = getc (fp)) != '1' && (c = getc (fp)) != '\n') {
	    say ("incompatible version of '%s' file", file);
	    die ();
	}
	majorNr = 1;
    }
    else {
	ungetc (c, fp);
	majorNr = 0;
    }

    dmsBox = dmOpenStream (layKey, "box", "a");
    dmsNor = dmOpenStream (layKey, "nor", "a");

    while (fscanf (fp, "%s", buf) > 0) { /* read interconnect number */

	fscanf (fp, "%s", buf);  /* '(' */

        printed = 0;
	select = 0;
	do {
	    n = fscanf (fp, "%s", buf);  /* net name */
	    if (n > 0 && buf[0] != ')') {
		select = testName (buf);
		if (select) {
		    if (optVerbose)
			fprintf (stdout, "%s\n", buf);
		    break;
		}
	    }
	} while (n > 0 && buf[0] != ')');

	while ((c = getc (fp)) != EOF && c != '\n');

	while (1) {

	    if (majorNr)
		n = dmGET (fp, "DDDSDDDDDD", &tile, &cx, &m, buf, &xl, &xr, &bl, &br, &tl, &tr);
	    else
		n = dmGET (fp, "DDDDDDDDDD", &tile, &cx, &m, &co, &xl, &xr, &bl, &br, &tl, &tr);
	    mask = m;

	    if (!(n > 0 && tile > 0)) {
		if (select && !printed && optVerbose)
		    say ("Warning: no layout items found.\n");
		break;
	    }

	    if (select) {
		printed = 1;

		/* selected output description of interconnect */

		if (!majorNr) {
		    if (co < 0) co = 0;
		    sprintf (buf, "%ld", co);
		}
		initcolorint (&color, buf);
		co = IS_COLOR (&color);

		if (co == 0 && cx < 0 && high_layer_no < 0 && !warningMissingHMask) {
		    say ("Warning: layout items selected with no original mask color.\n");
		    say ("Consider specification of mask for high-lighting.\n");
		    warningMissingHMask = 1;
		}

		for (i = 0; i < noMasks && (!i || (optOrigMasks && co)); i++) {

		    if (optOrigMasks && co) {
			COLORINITINDEX (mcolor, i);
			if (!COLOR_PRESENT (&color, &mcolor)) continue;
			layer_no = i;
		    }
		    else if (optCondMasks && mask >= 0 && mask < noMasks)
			layer_no = mask;
		    else
			layer_no = high_layer_no;

		    if (layer_no >= 0) {
			if (bl == br && tl == tr) {
			    gbox.layer_no = layer_no;
			    gbox.xl = xl / outScale;
			    gbox.xr = xr / outScale;
			    gbox.yb = bl / outScale;
			    gbox.yt = tl / outScale;
                            if (grow_size > 0) {
                                gbox.xl -= grow_size;
                                gbox.xr += grow_size;
                                gbox.yb -= grow_size;
                                gbox.yt += grow_size;
                            }
			    gbox.bxl = gbox.xl / outScale;
			    gbox.bxr = gbox.xr / outScale;
			    gbox.byb = gbox.yb / outScale;
			    gbox.byt = gbox.yt / outScale;
			    gbox.nx = 0;
			    gbox.ny = 0;

			    dmPutDesignData (dmsBox, GEO_BOX);
			}
			else {
			    gnor_ini.layer_no = layer_no;
			    gnor_ini.elmt = POLY_NOR;
			    if (bl == tl || br == tr)
				gnor_ini.no_xy = 3;
			    else
				gnor_ini.no_xy = 4;
			    gnor_ini.bxl = xl / outScale;
			    gnor_ini.bxr = xr / outScale;
			    gnor_ini.byb = Min (bl / outScale, br / outScale);
			    gnor_ini.byt = Max (tl / outScale, tr / outScale);
                            if (grow_size > 0) {
			        gnor_ini.bxl -= grow_size;
			        gnor_ini.bxr += grow_size;
			        gnor_ini.byb -= grow_size;
			        gnor_ini.byt += grow_size;
                            }
			    gnor_ini.r_bxl = gnor_ini.bxl;
			    gnor_ini.r_bxr = gnor_ini.bxr;
			    gnor_ini.r_byb = gnor_ini.byb;
			    gnor_ini.r_byt = gnor_ini.byt;
			    gnor_ini.dx = 0;
			    gnor_ini.dy = 0;
			    gnor_ini.nx = 0;
			    gnor_ini.ny = 0;
			    dmPutDesignData (dmsNor, GEO_NOR_INI);
			    gnor_xy.x = xl / (double) outScale;
			    gnor_xy.y = bl / (double) outScale;
                            if (grow_size > 0) {
                                gnor_xy.x -= grow_size;
                                gnor_xy.y -= grow_size;
                            }
			    dmPutDesignData (dmsNor, GEO_NOR_XY);
			    if (tl > bl) {
				gnor_xy.x = xl / (double) outScale;
				gnor_xy.y = tl / (double) outScale;
				if (grow_size > 0) {
				    gnor_xy.x -= grow_size;
				    gnor_xy.y += grow_size;
				}
				dmPutDesignData (dmsNor, GEO_NOR_XY);
			    }
			    if (tr > br) {
				gnor_xy.x = xr / (double) outScale;
				gnor_xy.y = tr / (double) outScale;
				if (grow_size > 0) {
				    gnor_xy.x += grow_size;
				    gnor_xy.y += grow_size;
				}
				dmPutDesignData (dmsNor, GEO_NOR_XY);
			    }
			    gnor_xy.x = xr / (double) outScale;
			    gnor_xy.y = br / (double) outScale;
                            if (grow_size > 0) {
                                gnor_xy.x += grow_size;
                                gnor_xy.y -= grow_size;
                            }
			    dmPutDesignData (dmsNor, GEO_NOR_XY);
			}
		    }
		    else if (!warningMissingHMask) {
			say ("Warning: layout items selected with no original mask color.\n");
			say ("Consider specification of mask for high-lighting.\n");
			warningMissingHMask = 1;
		    }
		}
	    }
	}
    }

    dmCloseStream (dmsBox, COMPLETE);
    dmCloseStream (dmsNor, COMPLETE);
}

Private int testName (char *name)
{
    char *s, *t;
    int l, u, m;

    l = -1;
    u = Array_net_nr;
    m = 0;

    while (l + 1 < u) {
	m = (l + u) / 2;
	s = Array_net_ref[m];
	t = name;
	while (1) {
	    if (*s == '*' && !*(s+1)) return (1);
	    if (*s == *t || (*s == '?' && *t)) {
		if (!*s) return (1);
		++s; ++t;
	    }
	    else break;
	}
	if (*t > *s) l = m;
	else u = m;
    }
    return (0);
}

void dmError (char *s) /* ddm error handler */
{
    dmPerror (s);
    die ();
}

void die () /* clean-up and stop */
{
    static int recursive = 0;

    if (recursive++ > 0) {
        char * s = "Emergency exit\n";
        write (fileno (stderr), s, strlen (s));
        exit (1);
    }
    dmQuit ();
    exit (1);
}
