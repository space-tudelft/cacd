/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makefem/define.h"
#include "src/space/makefem/extern.h"

#define HTTP_ADDRESS "http://www.space.tudelft.nl"

extern char * usedTechFile;
static DM_PROJECT  * dmproject = NULL;
DM_CELL * cellKey;

bool_t backcontact;
bool_t eliminateSubstrNode;
bool_t optRunFem  = TRUE;
bool_t optTime    = FALSE;
bool_t optVerbose = FALSE;
bool_t optInfo    = FALSE;
bool_t optHelp    = FALSE;

int    do_not_die = 0;

static char * techFile = NULL;
static char * techDef = NULL;
static char * paramFile = NULL;
static char * paramDef = NULL;
static char ** cellNames = NULL;

coor_t bbxl, bbxr, bbyb, bbyt; /* global bounding box */
coor_t c_xl, c_xr, c_yb, c_yt; /* contact bounding box */

int tileCnt, tileConCnt, tileConCnt2;
int inScale, outScale, cs_ext;
int cs_bxl, cs_bxr, cs_byb, cs_byt;
double meters; /* this many meters per internal layout unit */
double femX, femY, femZ;
double csSIGMA, csTN;
char * csMASK = NULL;

#ifdef __cplusplus
  extern "C" {
#endif
/* local operations */
Private void doCell (char *cellname);
Private char *getParams (DM_PROJECT *proj, char *libFile, char *usrFile);
Private void seeMessage (void);
#ifdef __cplusplus
  }
#endif

extern char *optarg;
extern int optind;

void overallUtilization ()
{
    message ("overall resource utilization:\n");
    verbose ("\tmemory allocation  : %.3f Mbyte\n", allocatedMbyte ());
    clockPrintTime (stdout);
}

int main (int argc, char **argv)
{
    int c, i, optErr = 0;
    char *s;

    argv0 = "makefem";

    dmInit (argv0);

    while ((c = getopt (argc, argv, "%vihE:e:P:p:S:")) != EOF) {
	switch (c) {
	    case 'h': optHelp    = TRUE; break;
	    case 'i': optInfo    = TRUE;
	    case 'v': optVerbose = TRUE; break;
	    case 'E': techFile   = strsave (optarg); break;
	    case 'e': techDef    = strsave (optarg); break;
	    case 'P': paramFile  = strsave (optarg); break;
	    case 'p': paramDef   = strsave (optarg); break;
	    case 'S': paramSetOption (optarg); break;
	    default: break;
	}
    }

    cellNames = argv + optind;

    if (techDef && techFile) {
	say ("Use only one of -e and -E");
	optErr++;
    }

    if (paramDef && paramFile) {
	say ("Use only one of -p and -P");
	optErr++;
    }

    if (!cellNames[0]) {
	say ("You must specify a cellname");
	optErr++;
    }
    else if (cellNames[1]) {
	say ("You may specify only one cellname");
	optErr++;
    }

    if (optErr) {
	fprintf (stderr, "Usage: %s ", argv0);
	fprintf (stderr, "[-e def | -E file] [-p def | -P file] [-S param=value] [-hiv] cell\n");
        seeMessage ();
	die ();
    }

    /************ Real Program Starts Here *******************/

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    paramFile = getParams (dmproject, paramDef, paramFile);

    if (paramLookupB ("fem.param_verbose", "off")) paramSetVerbose (1);

    if (!optVerbose) optVerbose = paramLookupB ("fem.verbose", "off");
    if (optVerbose || optHelp) seeMessage ();

    optRunFem = paramLookupB ("fem.runfemlab", "on");
    if (optRunFem) { // Test existance of MATLAB startup files!
	fclose (cfopen ("startup.m", "r"));
	fclose (cfopen ("runfemlab", "r"));
    }

    optTime = paramLookupB ("fem.print_time", "off");
    if (optTime || optVerbose) clockInit ();
    if (optTime) tick ("startup");

    backcontact = paramLookupB ("fem.backcontact", "on");

    eliminateSubstrNode = paramLookupB ("elim_sub_node", "off");

    if ((femX = paramLookupD ("fem.xoverlap", "20")) < 0) femX = 0;
    if ((femY = paramLookupD ("fem.yoverlap", "20")) < 0) femY = 0;
    if ((femZ = paramLookupD ("fem.zbottom",  "20")) <= 0) {
	say ("parameter fem.zbottom must be > 0");
	die ();
    }

    if ((s = paramLookupS ("fem.cs_bbox", (char *) NULL))) {
	if (sscanf (s, "%d %d %d %d", &cs_bxl, &cs_bxr, &cs_byb, &cs_byt) != 4) {
	    say ("parameter fem.cs_bbox must have 4 values");
	    die ();
	}
	if (cs_bxr <= cs_bxl || cs_byt <= cs_byb) {
	    say ("parameter fem.cs_bbox has incorrect dimension");
	    die ();
	}
    }
    else {
	cs_ext = paramLookupI ("fem.cs_extension", "0");
	if (cs_ext < 0) say ("warning: parameter fem.cs_extension is < 0");
	else if (cs_ext > INF/1000) say ("warning: very large fem.cs_extension");
    }

    csMASK = paramLookupS ("fem.cs_mask", NULL);
    if (csMASK && !isalpha((int)*csMASK)) {
	say ("parameter fem.cs_mask must be a name");
	die ();
    }
    if ((csSIGMA = paramLookupD ("fem.cs_sigma", "0.0")) < 0) {
	say ("parameter fem.cs_sigma must be >= 0");
	die ();
    }
    if ((csTN = paramLookupD ("fem.cs_thickness", "0.5")) <= 0) {
	say ("parameter fem.cs_thickness must be > 0");
	die ();
    }

    if (optVerbose) {
	verboseSetMode (TRUE);
	message ("%s ", argv0);
	for (i = 0; ++i < argc;) message ("%s ", argv[i]);
	message ("\n");
	message ("parameter file: %s\n", giveICD (paramFile));
    }

    catchSignals ();

    getTechnology (dmproject, techDef, techFile);
    if (optVerbose) {
	message ("technology file: %s\n", giveICD (usedTechFile));
    }

    if (optTime) tock ("startup");

    doCell (cellNames[0]);

    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    if (optInfo) scanPrintInfo (stdout);

    if (optTime) clockPrintAll (stdout);

    if (optVerbose) overallUtilization ();

    verbose ("%s: --- Finished ---\n", argv0);
    return (0);
}

Private char *getParams (DM_PROJECT *project, char *libFile, char *userFile)
{
    static char *pfile;
    char filebuf[1024];

    if (!userFile && !libFile) {
	libFile = "space.def.p";
	sprintf (filebuf, "%s/.%s", project -> dmpath, libFile);
	if (access (filebuf, 0) == 0)
	    pfile = filebuf;
	else
	    pfile = dmGetMetaDesignData (PROCPATH, project, libFile);
    }
    else {
	if (!(pfile = userFile)) {
	    libFile = mprintf ("space.%s.p", libFile);
	    pfile = dmGetMetaDesignData (PROCPATH, project, libFile);
	}
	if (access (pfile, 0) != 0) {
	    say ("%s: no such parameter file", giveICD (pfile));
	    die ();
	}
    }
    paramReadFile (pfile = strsave (pfile));
    return (pfile);
}

Private void seeMessage (void)
{
    if (!optHelp) {
	message ("See %s\n", HTTP_ADDRESS);
	return;
    }
    message ("Type 'icdman %s' to obtain the manual page of this tool.\n", argv0);
    message ("User manuals can be found in $ICDPATH/share/doc.\n");
    message ("See %s for (more) documentation, background\n", HTTP_ADDRESS);
    message ("information and release notes about the latest version of this program.\n");
}

Private void doCell (char *cellname)
{
    DM_STREAM *info3;
    edge_t *newEdge;

    if (optTime) tick (mprintf ("prepass %s", cellname));

    verbose ("prepassing %s for substrate resistance calculate\n", cellname);

    cellKey = dmCheckOut (dmproject, cellname, WORKING, DONTCARE, LAYOUT, ATTACH);
    ASSERT (cellKey);

    inScale = outScale = SCALE;

    info3 = dmOpenStream (cellKey, "info3", "r");
    if (dmGetDesignData (info3, GEO_INFO3) <= 0) {
	say ("internal error: cannot read info3 data for cell '%s'", cellname);
	die ();
    }
    dmCloseStream (info3, COMPLETE);

    if (ginfo3.nr_samples != 0) {
	outScale = (int) ginfo3.nr_samples;
	inScale = 1;
    }

    meters = (1e-6 * cellKey -> dmproject -> lambda) / outScale;

    bbxl = (coor_t) (inScale * ginfo3.bxl);
    bbxr = (coor_t) (inScale * ginfo3.bxr);
    bbyb = (coor_t) (inScale * ginfo3.byb);
    bbyt = (coor_t) (inScale * ginfo3.byt);

    c_xl = c_xr = c_yb = c_yt = 0;
    tileCnt = tileConCnt = tileConCnt2 = 0;

    newEdge = openInput ();
    if (newEdge -> xl < bbxl) { // bigger bbox required
	coor_t dx, dy;
	dx = bbxl - newEdge -> xl;
	dy = bbyb - newEdge -> yl;
	ASSERT (dy > 0);
	bbxl -= dx; bbxr += dx;
	bbyb -= dy; bbyt += dy;
    }

    setXr (bbxr); /* tell XR value to scan module */
    scan (newEdge);
    if (csMASK) swapInput ();
    closeInput ();

    if (optVerbose) {
	printf ("\noverall substrate statistics:\n");
	printf ("\ttotal nr. of tiles : %d\n", tileCnt);
	printf ("\tsubstrate contacts : %d\n", tileConCnt);
	printf ("\tchannelstop blocks : %d\n", tileConCnt2);
    }

    dmCheckIn (cellKey,  COMPLETE);

    if (optTime) tock (mprintf ("prepass %s", cellname));
}

char *strCoorBrackets (coor_t x, coor_t y, double dscale)
{
    static char buf[88];
    sprintf (buf, "(%s, %s)", strCoor (x, dscale), strCoor (y, dscale));
    return (buf);
}

char *strCoor (coor_t a, double dscale)
{
    static char buf1[40], buf2[40], *buf;

    buf = (buf == buf1) ? buf2 : buf1;
    if ((a / dscale) == (double)((long)(a / dscale)))
	sprintf (buf, "%ld", (long)(a / dscale));
    else
	sprintf (buf, "%.2f", a / dscale);
    return (buf);
}

void dmError (char *s)
{
    dmPerror (s);
    if (!do_not_die) die ();
}

void die ()
{
    dmQuit ();
    exit (1);
}

void extraSay ()
{
}
