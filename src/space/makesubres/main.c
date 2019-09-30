/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
 *	Arjan van Genderen
 *	Nick van der Meijs
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

#include "src/space/include/version.h"
#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

#define HTTP_ADDRESS "http://www.space.tudelft.nl"

static DM_PROJECT  * dmproject = NULL;

bool_t eliminateSubstrNode;
bool_t optTime    = FALSE;
bool_t optVerbose = FALSE;
bool_t optInfo    = FALSE;
bool_t optHelp    = FALSE;
bool_t optEstimate3D = FALSE;

int    do_not_die = 0;

static char * techFile = NULL;
static char * techDef = NULL;
static char * paramFile = NULL;
static char * paramDef = NULL;
static char ** cellNames = NULL;

extern char *usedTechFile;
extern substrate_t *subcaps;
extern substrate_t *substrs;
extern int subcap_cnt;
extern int substr_cnt;

coor_t bandWidth; /* band width used in the 'scan' module */
coor_t bbxl, bbxr, bbyb, bbyt; /* global bounding box */

int makesubcap;
int inScale;
int outScale;
double meters; /* this many meters per internal layout unit */

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
    char optstring[100];
    char *s;

    s = strrchr (argv[0], '/');
    argv0 = s ? s + 1 : argv[0];

    if (strcmp (argv0, "makesubres") != 0) {
	if (strcmp (argv0, "makesubcap") != 0) {
	    fprintf (stderr, "error: incorrect tool name\n"); die ();
	}
	makesubcap = 1;
    }

    dmInit (argv0);

    strcpy (optstring, "%UivhE:e:P:p:S:");

    while ((c = getopt (argc, argv, optstring)) != EOF) {
	switch (c) {
	    case 'h': optHelp    = TRUE; break;
	    case 'i': optInfo    = TRUE;
	    case 'v': optVerbose = TRUE; break;
	    case 'E': techFile   = strsave (optarg); break;
	    case 'e': techDef    = strsave (optarg); break;
	    case 'P': paramFile  = strsave (optarg); break;
	    case 'p': paramDef   = strsave (optarg); break;
	    case 'S': paramSetOption (optarg); break;
            case 'U': optEstimate3D = TRUE; break;
	    default: break;
	}
    }

    cellNames = argv + optind;

    if (techDef  && techFile)  { say ("Use only one of -e and -E"); optErr++; }
    if (paramDef && paramFile) { say ("Use only one of -p and -P"); optErr++; }

	if (!cellNames[0]) { say ("You must specify a cellname"); optErr++; }
    else if (cellNames[1]) { say ("You may specify only one cellname"); optErr++; }

    if (optErr || optHelp) {
	fprintf (stderr, "Usage: %s ", argv0);
	fprintf (stderr, "[-e def | -E file] [-p def | -P file] [-S param=value] [-Uhv] cell\n");
        seeMessage ();
	die ();
    }

    /************ Real Program Starts Here *******************/

    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    paramFile = getParams (dmproject, paramDef, paramFile);
    getTechFile (dmproject, techDef, techFile);

    c = 0;
    if (!optVerbose) { optVerbose = paramLookupB ("verbose", "off"); ++c; }
    if (optVerbose) {
	seeMessage ();
	verboseSetMode (TRUE);
	message ("%s ", argv0);
	for (i = 0; ++i < argc;) message ("%s ", argv[i]);
	message ("\n");
	message ("parameter file: %s\n", giveICD (paramFile));
	message ("technology file: %s\n", giveICD (usedTechFile));
    }

    if (paramLookupB ("param_verbose", "off")) {
	paramSetVerbose (1);
	paramLookupB ("param_verbose", "off");
	if(c) paramLookupB ("verbose", "off");
    }

    if (!optTime) optTime = paramLookupB ("print_time", "off");
    if (optTime || optVerbose) clockInit ();

    eliminateSubstrNode = paramLookupB ("elim_sub_node", "off");

    catchSignals ();

    getTechnology (dmproject, techDef, techFile);

    if (!makesubcap) { /* Use substrate information */
	if (substr_cnt <= 0)
	    say ("error: no sublayers found in technology data!"), die ();
    }
    else { /* Use substrate cap information */
	if (subcap_cnt <= 0)
	    say ("error: no subcaplayers found in technology data!"), die ();
	substrs = subcaps;
	substr_cnt = subcap_cnt;
    }

    doCell (cellNames[0]);

    dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

    if (optInfo && !optEstimate3D) {
	greenStatistics (stdout);
	schurStatistics (stdout);
    }

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
    message ("Version %s, compiled on %s", SPACE_TOOLVERSION, COMPILE_DATE_STRING);
    if (!optHelp) {
	message ("See %s\n", HTTP_ADDRESS);
	return;
    }
    message ("Type 'icdman toolname' to obtain the manual page of a tool\n");
    message ("(e.g 'icdman %s' to obtain the manual page of this tool).\n", argv0);
    message ("User manuals can be found in $ICDPATH/share/doc.\n");
    message ("See %s for (more) documentation, background\n", HTTP_ADDRESS);
    message ("information and release notes about the latest version of this program.\n");
}

Private void doCell (char *cellname)
{
    DM_CELL * layoutKey;
    DM_STREAM * info3;
    edge_t *newEdge;

    if (optTime) tick (mprintf ("prepass %s", cellname));

    verbose ("prepassing %s for substrate resistance calculate\n", cellname);

    layoutKey = dmCheckOut (dmproject, cellname, WORKING, DONTCARE, LAYOUT, ATTACH);
    ASSERT (layoutKey);

    info3 = dmOpenStream (layoutKey, "info3", "r");
    if (dmGetDesignData (info3, GEO_INFO3) <= 0) {
	say ("internal error: cannot read info3 data for cell '%s'", cellname);
	die ();
    }
    dmCloseStream (info3, COMPLETE);

    if (ginfo3.nr_samples != 0) {
	inScale = 1; outScale = I(ginfo3.nr_samples); }
    else
	inScale = outScale = SCALE;

    meters = (1e-6 * layoutKey -> dmproject -> lambda) / outScale;

    bbxl = (coor_t) (inScale * ginfo3.bxl);
    bbxr = (coor_t) (inScale * ginfo3.bxr);
    bbyb = (coor_t) (inScale * ginfo3.byb);
    bbyt = (coor_t) (inScale * ginfo3.byt);

    initLump ();
    initSubstr (layoutKey);

    newEdge = openInput (layoutKey);
    if (newEdge -> xl < bbxl) { // bigger bbox required
	coor_t dx, dy;
	dx = bbxl - newEdge -> xl;
	dy = bbyb - newEdge -> yl;
	ASSERT (dy > 0);
	bbxl -= dx; bbxr += dx;
	bbyb -= dy; bbyt += dy;
    }

    bandWidth = sub3dInit ();
    ASSERT (bandWidth > 0);

    setXr (bbxr); /* tell XR value to scan module */
    scan (newEdge);
    closeInput ();

    sub3dEnd ();
    endSubstr ();
    endLump (cellname);

    dmCheckIn (layoutKey,  COMPLETE);

    if (optTime) tock (mprintf ("prepass %s", cellname));
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

#ifdef MYMATHERR
/* This function is also defined in spider/matherr.c ! */
int matherr (struct exception *x)
{
    char *err = "error";
    switch (x -> type) {
	case DOMAIN:    err = "argument domain error"; break;
	case SING:      err = "argument singularity";  break;
	case OVERFLOW:  err = "overflow range error";  break;
	case UNDERFLOW: err = "underflow range error"; break;
	case TLOSS:     err = "total loss of significance"; break;
	case PLOSS:     err = "partial loss of significance";
    }
    fprintf (stderr, "%s: %s\n", x -> name, err);
}
#endif
