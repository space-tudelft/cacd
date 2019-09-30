/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/libddm/dmincl.h"

#ifndef NOPACK
#define dmGET _dmUnpack
#else
#define dmGET _dmDoget
#endif

#define PE fprintf(stderr,
#define Lay(no) process->mask_name[no]

int strend (char *s1, char *s2);
void read_info (char *file);
void read_info2 (char *file);
void read_info3 (char *file);
void read_mc (char *file);
void read_box (char *file);
void read_term (char *file);
void read_anno (char *file);
void read_anno_exp (char *file);
void read_nor (char *file);
void read_bxx (char *file);
void read_gln (char *file);
void read_nxx (char *file);
void read_pgt (char *file);
void read_vln (char *file);
void read_teq (char *file);
void read_tid (char *file);
void read_spec (char *file);
void read_file (char *file);
void read_flashes (char *file);
void readc_info (char *file);
void readc_mc (char *file);
void readc_term (char *file);
void readc_net (char *file);
void readc_geo (char *file);
void readf_mc (char *file);
void readf_info (char *file);
void readf_term (char *file);
void readf_chan (char *file);
void header (char *file);
#ifdef DO_SIG
void sig_handler (int sig);
#endif
void error (int nr, char *s);

DM_PROJECT *project;
DM_CELL    *key;
DM_STREAM  *fp;
DM_PROCDATA *process;
char  *cell;
char **viewlist = NULL;
char  *view = "layout";
char  strbuf[40];
int   a_mode = 0;
int   b_mode = 0;
int   c_mode = 0;
int   f_mode = 0;
int   l_mode = 1;
int   mode4;
char *n_mode = 0;
char *t_mode = 0;
char *s_mode = 0;
int   v_mode;
int   w_mode = 5;

long lower[10], lower1[10], lower2[10];
long upper[10], upper1[10], upper2[10];
char attribute_string[512];

char *argv0 = "dbcat";		/* program name */
char *use_msg =			/* command line */
"\nUsage: %s [-abv4] [-c|-f] [-s dbstream] [-n name] [-t attr] [-w N] [cell ...]\n\n";

int main (int argc, char *argv[])
{
    register int i, j, iarg;
    register char **ml;
    int  usage = 0;

    dm_get_do_not_alloc = 1;

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; ++iarg) {
	j = iarg;
	for (i = 1; argv[j][i] != '\0'; ++i) {
	    switch (argv[j][i]) {
		case 'a':
		    ++a_mode;
		    PE "%s: -a: list all cells\n", argv0);
		    break;
		case 'b':
		    ++b_mode;
		    PE "%s: -b: brief listing\n", argv0);
		    break;
		case 'v':
		    ++v_mode;
		    PE "%s: -v: verbose mode\n", argv0);
		    break;
		case '4':
		    ++mode4;
		    PE "%s: -4: divide coord by 4 mode\n", argv0);
		    break;
		case 'c':
		    ++c_mode;
		    f_mode = 0;
		    PE "%s: -c: list only circuit data\n", argv0);
		    break;
		case 'f':
		    ++f_mode;
		    c_mode = 0;
		    PE "%s: -f: list only floorplan data\n", argv0);
		    break;
		case 's':
		    s_mode = argv[++iarg];
		    PE "%s: -s: read only dbstream: %s\n", argv0, s_mode);
		    break;
		case 'n':
		    n_mode = argv[++iarg];
		    PE "%s: -n: list only name: %s\n", argv0, n_mode);
		    break;
		case 't':
		    t_mode = argv[++iarg];
		    PE "%s: -t: list only with attribute: %s\n", argv0, t_mode);
		    break;
		case 'w':
		    w_mode = atoi (argv[++iarg]);
		    PE "%s: -w: list coordinates with min. width: %d\n", argv0, w_mode);
		    break;
		default:
		    ++usage;
		    PE "%s: -%c: unknown option\n", argv0, argv[j][i]);
	    }
	}
    }

    if (a_mode) --iarg;

    if (argc <= iarg) {
	PE "%s: no cell name given\n", argv0);
	++usage;
    }

    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

    /* flush stderr, to be sure that comments about options first are given */
    fflush (stderr);

#ifdef DO_SIG
    signal (SIGHUP , SIG_IGN); /* ignore hangup signal */
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTERM, sig_handler);

    if (signal (SIGINT, SIG_IGN) != SIG_IGN)
	signal (SIGINT, sig_handler);
#endif

    dmInit (argv0);
    project = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    if (!c_mode)
	process = (DM_PROCDATA *) dmGetMetaDesignData (PROCESS, project);

    if (c_mode) view = "circuit";
    else if (f_mode) view = "floorplan";
    else view = "layout";

    if (a_mode) {
	if (c_mode)
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, CIRCUIT);
	else if (f_mode)
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, FLOORPLAN);
	else
	    ml = (char **) dmGetMetaDesignData (CELLLIST, project, LAYOUT);
    }
    else
	ml = &argv[iarg];

    while ((cell = *ml++)) {
	if (c_mode)
	    key = dmCheckOut (project, cell, WORKING, DONTCARE, CIRCUIT, READONLY);
	else if (f_mode)
	    key = dmCheckOut (project, cell, WORKING, DONTCARE, FLOORPLAN, READONLY);
	else
	    key = dmCheckOut (project, cell, WORKING, DONTCARE, LAYOUT, READONLY);

	if (s_mode) {
	    if (c_mode) {
		     if (strcmp (s_mode, "info")   == 0) readc_info ("info");
		else if (strncmp(s_mode, "mc", 2)  == 0) readc_mc  (s_mode);
		else if (strncmp(s_mode, "net", 3) == 0) readc_net (s_mode);
		else if (strcmp (s_mode, "term")   == 0) readc_term ("term");
		else if (strcmp (s_mode, "fterm")  == 0) readc_term ("fterm");
		else if (strcmp (s_mode, "fstate") == 0) read_file ("fstate");
		else if (strcmp (s_mode, "congeo") == 0) readc_geo ("congeo");
		else if (strcmp (s_mode, "devgeo") == 0) readc_geo ("devgeo");
		else error (2, s_mode);
	    }
	    else if (f_mode) {
		     if (strcmp (s_mode, "mc")   == 0) readf_mc ("mc");
		else if (strcmp (s_mode, "info") == 0) readf_info ("info");
		else if (strcmp (s_mode, "term") == 0) readf_term ("term");
		else if (strcmp (s_mode, "chan") == 0) readf_chan ("chan");
		else error (2, s_mode);
	    }
	    else {
		 if (strcmp (s_mode, "info")  == 0) read_info ("info");
	    else if (strcmp (s_mode, "info3") == 0) read_info3 ("info3");
	    else if (strcmp (s_mode, "info2") == 0) read_info2 ("info2");
	    else if (strcmp (s_mode, "mc")    == 0) read_mc ("mc");
	    else if (strcmp (s_mode, "box")   == 0) read_box ("box");
	    else if (strcmp (s_mode, "term")  == 0) read_term ("term");
	    else if (strncmp (s_mode, "anno", 4) == 0) {
		if (strcmp (s_mode, "anno_exp") == 0)
		    read_anno_exp ("anno_exp");
		else
		    read_anno ("annotations");
	    }
	    else if (strcmp (s_mode, "nor") == 0) read_nor ("nor");
	    else if (strcmp (s_mode, "cont_bln") == 0) read_bxx ("cont_bln");
	    else if ((i = strend (s_mode, "bxx")) >= 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			sprintf (strbuf, "%s_bxx", Lay (i));
			read_bxx (strbuf);
			if (process -> mask_type[i] == DM_INTCON_MASK) {
			    sprintf (strbuf, "t_%s_bxx", Lay (i));
			    read_bxx (strbuf);
			}
		    }
		}
		else
		    read_bxx (s_mode);
	    }
	    else if ((i = strend (s_mode, "nxx")) >= 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			sprintf (strbuf, "%s_nxx", Lay (i));
			read_nxx (strbuf);
		    }
		}
		else
		    read_nxx (s_mode);
	    }
	    else if ((i = strend (s_mode, "gln")) >= 0 ||
		     (i = strend (s_mode, "aln")) > 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			sprintf (strbuf, "%s_gln", Lay (i));
			read_gln (strbuf);
		    }
		}
		else
		    read_gln (s_mode);
	    }
	    else if ((i = strend (s_mode, "pgt")) >= 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			if (process -> pgt_no[i] <= 0) continue;
			sprintf (strbuf, "%s_pgt", Lay (i));
			read_pgt (strbuf);
		    }
		}
		else
		    read_pgt (s_mode);
	    }
	    else if ((i = strend (s_mode, "pgt_neg")) >= 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			if (process -> pgt_no[i] <= 0) continue;
			sprintf (strbuf, "%s_pgt_neg", Lay (i));
			read_pgt (strbuf);
		    }
		}
		else
		    read_pgt (s_mode);
	    }
	    else if ((i = strend (s_mode, "vln")) >= 0) {
		if (i == 0) {
		    for (i = 0; i < process -> nomasks; ++i) {
			sprintf (strbuf, "%s_vln", Lay (i));
			read_vln (strbuf);
			if (process -> mask_type[i] == DM_INTCON_MASK) {
			    sprintf (strbuf, "t_%s_vln", Lay (i));
			    read_vln (strbuf);
			}
		    }
		}
		else
		    read_vln (s_mode);
	    }
	    else if (strcmp (s_mode, "teq")  == 0) read_teq ("teq");
	    else if (strcmp (s_mode, "tid")  == 0) read_tid ("tid");
	    else if (strcmp (s_mode, "spec") == 0) read_spec ("spec");
	    else if (strcmp (s_mode, "flashes") == 0) read_flashes ("flashes");
	    else error (2, s_mode);
	    }
	}
	else {
	    if (c_mode) {
		struct stat buf;
		int f, n;
		f  = dmStat (key, "fterm", &buf);
		n  = dmStat (key, "net", &buf);
		if (f < 0 || n == 0) {
		    readc_mc ("mc");
		    readc_term ("term");
		    readc_net ("net");
		}
		if (f == 0) {
		    readc_term ("fterm");
		    read_file ("fstate");
		}
	    }
	    else if (f_mode) {
		readf_mc ("mc");
		readf_info ("info");
		readf_term ("term");
		readf_chan ("chan");
	    }
	    else {
		read_info ("info");
		read_info3 ("info3");
		read_info2 ("info2");
		read_mc ("mc");
		read_box ("box");
		read_term ("term");
		read_anno ("annotations");
		read_nor ("nor");
	    }
	}
	dmCheckIn (key, COMPLETE);
    }

    dmQuit ();
    return (0);
}

char *Lay2 (int no)
{
    static char buf[20];
    if (no >= 0 && no < process -> nomasks) return Lay (no);
    sprintf (buf, "?%d", no);
    return buf;
}

int strend (char *s1, char *s2)
{
    int i;
    if ((i = strlen (s1) - strlen (s2)) >= 0)
	if (strcmp (s1 + i, s2) == 0) return (i);
    return (-1);
}

void read_info (char *file)
{
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    dmGetDesignData (fp, GEO_INFO);
    if (v_mode) printf ("cell/call/elmt  bxl,bxr,byb,byt:\n");
    printf ("%5ld %5ld %5ld %5ld\n", ginfo.bxl, ginfo.bxr, ginfo.byb, ginfo.byt);
    dmGetDesignData (fp, GEO_INFO);
    printf ("%5ld %5ld %5ld %5ld\n", ginfo.bxl, ginfo.bxr, ginfo.byb, ginfo.byt);
    dmGetDesignData (fp, GEO_INFO);
    printf ("%5ld %5ld %5ld %5ld\n", ginfo.bxl, ginfo.bxr, ginfo.byb, ginfo.byt);
    dmCloseStream (fp, QUIT);
}

void read_info2 (char *file)
{
    int ft = 0;
    register int i, j;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    for (j = i = 0; i < process -> nomasks; ++i) {
	if (dmGetDesignData (fp, GEO_INFO2) == 0) {
	    if (i) printf ("%s: premature eof\n", argv0);
	    break;
	}
	++j;
	if (ft) { ft = 0;
	    printf ("#boxes: #groups:  (lay:)\n");
	}
	printf ("%7ld %8ld", ginfo2.nr_boxes, ginfo2.nr_groups);
	if (v_mode)
	    printf ("  (%s)\n", Lay (i));
	else
	    printf ("\n");
    }
    for (i = 0; i < process -> nomasks; ++i) {
	if (process -> mask_type[i] != DM_INTCON_MASK) continue;
	if (dmGetDesignData (fp, GEO_INFO2) == 0) {
	    if (j) printf ("%s: premature eof\n", argv0);
	    break;
	}
	printf ("%7ld %8ld", ginfo2.nr_boxes, ginfo2.nr_groups);
	if (v_mode)
	    printf ("  (t_%s)\n", Lay (i));
	else
	    printf ("\n");
    }
    dmCloseStream (fp, QUIT);
}

void read_info3 (char *file)
{
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (dmGetDesignData (fp, GEO_INFO3) > 0) {
	if (v_mode) printf ("bxl,bxr,byb,byt:  samples:\n");
	printf ("%5ld %5ld %5ld %5ld  %2ld\n",
	    ginfo3.bxl, ginfo3.bxr, ginfo3.byb, ginfo3.byt, ginfo3.nr_samples);
    }
    dmCloseStream (fp, QUIT);
}

void read_mc (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_MC) > 0) {
	if (n_mode) {
	    if (strcmp (n_mode, gmc.cell_name)) continue;
	}
	if (ft) { ft = 0;
	    printf ("cell: inst: tx: ty: mt[0,1,3,4]: bxl,bxr,byb,byt: [dx,nx: dy,ny:]\n");
	}
	printf ("%-10s %-10s %5ld %5ld  %2ld %2ld %2ld %2ld %5ld %5ld %5ld %5ld",
	    gmc.cell_name, gmc.inst_name,
	    gmc.mtx[2], gmc.mtx[5],
	    gmc.mtx[0], gmc.mtx[1], gmc.mtx[3], gmc.mtx[4],
	    gmc.bxl, gmc.bxr, gmc.byb, gmc.byt);
	if (gmc.nx || gmc.ny)
	    printf ("  %ld %ld  %ld %ld", gmc.dx, gmc.nx, gmc.dy, gmc.ny);
	printf ("%s\n", (gmc.imported ? " (IMP)" : ""));
    }
    dmCloseStream (fp, QUIT);
}

void read_box (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_BOX) > 0) {
	if (ft) { ft = 0;
	    printf ("lay: xl,xr,yb,yt: [bxl,bxr,byb,byt: dx,nx: dy,ny:]\n");
	}
	printf ("%-3s %5ld %5ld %5ld %5ld",
	    Lay2 (gbox.layer_no), gbox.xl, gbox.xr, gbox.yb, gbox.yt);
	if (gbox.nx || gbox.ny) {
	    printf ("  %5ld %5ld %5ld %5ld  %ld %ld  %ld %ld",
		gbox.bxl, gbox.bxr, gbox.byb, gbox.byt,
		gbox.dx, gbox.nx, gbox.dy, gbox.ny);
	}
	printf ("\n");
    }
    dmCloseStream (fp, QUIT);
}

void read_term (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_TERM) > 0) {
	if (ft) { ft = 0;
	    printf ("term: lay: xl,xr,yb,yt: [bxl,bxr,byb,byt: dx,nx: dy,ny:]\n");
	}
	printf ("%-10s %-3s %5ld %5ld %5ld %5ld",
	    gterm.term_name, Lay2 (gterm.layer_no),
	    gterm.xl, gterm.xr, gterm.yb, gterm.yt);
	if (gterm.nx || gterm.ny) {
	    printf ("  %5ld %5ld %5ld %5ld  %ld %ld  %ld %ld",
		gterm.bxl, gterm.bxr, gterm.byb, gterm.byt,
		gterm.dx, gterm.nx, gterm.dy, gterm.ny);
	}
	printf ("\n");
    }
    dmCloseStream (fp, QUIT);
}

void read_anno (char *file)
{
    header (file);

    if (!(fp = dmOpenStream (key, file, "r"))) return;

    /* format record first */
    if (dmGetDesignData (fp, GEO_ANNOTATE) > 0) {
	if (v_mode)
	    printf ("format: type=%d major=%ld minor=%ld\n",
		ganno.type, ganno.o.format.fmajor, ganno.o.format.fminor);
	else
	    printf ("format: %d %ld %ld\n",
		ganno.type, ganno.o.format.fmajor, ganno.o.format.fminor);
    }

    while (dmGetDesignData (fp, GEO_ANNOTATE) > 0) {
	switch (ganno.type) {
	case GA_LINE:
	  if (v_mode)
	    printf ("line: x1=%g y1=%g x2=%g y2=%g mode=%d\n",
		ganno.o.line.x1, ganno.o.line.y1,
		ganno.o.line.x2, ganno.o.line.y2,
		ganno.o.line.mode);
	  else
	    printf ("line: %g %g %g %g %d\n",
		ganno.o.line.x1, ganno.o.line.y1,
		ganno.o.line.x2, ganno.o.line.y2,
		ganno.o.line.mode);
	    break;
	case GA_TEXT:
	  if (v_mode)
	    printf ("text: \"%s\" x=%g y=%g ax=%g\n",
		ganno.o.text.text,
		ganno.o.text.x, ganno.o.text.y, ganno.o.text.ax);
	  else
	    printf ("text: %s %g %g %g\n",
		ganno.o.text.text,
		ganno.o.text.x, ganno.o.text.y, ganno.o.text.ax);
	    break;
	case GA_LABEL:
	  if (v_mode) {
	    printf ("label: \"%s\" lay=\"%s\" cls=\"%s\" x=%g y=%g ax=%g",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax);
	    if (*ganno.o.Label.Attributes)
		printf (" attr=\"%s\"\n", ganno.o.Label.Attributes);
	    else
		printf ("\n");
	  }
	  else if (*ganno.o.Label.Attributes)
	    printf ("label: %s %s %s %g %g %g %s\n",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax,
		ganno.o.Label.Attributes);
	  else
	    printf ("label: %s %s %s %g %g %g\n",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax);
	    break;
	default:
	    printf ("?????: unknown annotation type=%d\n", ganno.type);
	}
    }
    dmCloseStream (fp, QUIT);
}

void read_anno_exp (char *file)
{
    char help_name[1024];
    int isLabel, maskno;
    double x_d, y_d;

    header (file);

    if (!(fp = dmOpenStream (key, file, "r"))) return;

    while (dmScanf (fp, "%s", help_name) == 1) {

	printf ("%s ", help_name);

        if (strcmp (help_name, "/") == 0) {
            dmScanf (fp, "%d\n", &isLabel);
	    if (v_mode)
		printf ("label=%d\n", isLabel);
	    else
		printf ("%d\n", isLabel);
	}
        else {
            dmScanf (fp, "%s %d\n", help_name, &isLabel);
	    if (v_mode)
		printf ("cell=\"%s\" label=%d\n", help_name, isLabel);
	    else
		printf ("%s %d\n", help_name, isLabel);
        }

      if (isLabel) {
	if (dmGetDesignData (fp, GEO_ANNOTATE) <= 0) break;
	switch (ganno.type) {
	case GA_LABEL:
	  if (v_mode) {
	    printf ("label: \"%s\" lay=\"%s\" cls=\"%s\" x=%g y=%g ax=%g",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax);
	    if (*ganno.o.Label.Attributes)
		printf (" attr=\"%s\"\n", ganno.o.Label.Attributes);
	    else
		printf ("\n");
	  }
	  else if (*ganno.o.Label.Attributes)
	    printf ("label: %s %s %s %g %g %g %s\n",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax,
		ganno.o.Label.Attributes);
	  else
	    printf ("label: %s %s %s %g %g %g\n",
		ganno.o.Label.name, Lay2 (ganno.o.Label.maskno),
		ganno.o.Label.Class,
		ganno.o.Label.x, ganno.o.Label.y, ganno.o.Label.ax);
	    break;
	default:
	    printf ("?????: unexp. annotation type=%d\n", ganno.type);
	}
      }
      else {
	if (dmScanf (fp, "%s %d %le %le\n",
		help_name, &maskno, &x_d, &y_d) != 4) break;
	if (v_mode)
	    printf ("terminal: \"%s\" lay=\"%s\" x=%g y=%g\n",
		help_name, Lay2 (maskno), x_d, y_d);
	else
	    printf ("terminal: %s %s %g %g\n",
		help_name, Lay2 (maskno), x_d, y_d);
      }
    }

    dmCloseStream (fp, QUIT);
}

void read_nor (char *file)
{
    int ft = 0;
    register int i;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_NOR_INI) > 0) {
	if (ft) { ft = 0;
	    printf ("lay: elmt: no_xy: bxl,bxr,byb,byt:\n");
	    printf ("[r_bxl,bxr,byb,byt: dx,nx: dy,ny:]\n");
	    printf ("x: y:\n");
	}
	printf ("%-3s %ld %4ld %5ld %5ld %5ld %5ld\n",
	    Lay2 (gnor_ini.layer_no), gnor_ini.elmt, gnor_ini.no_xy,
	    gnor_ini.bxl, gnor_ini.bxr, gnor_ini.byb, gnor_ini.byt);

	if (gnor_ini.nx || gnor_ini.ny) {
	    printf ("%11s%5ld %5ld %5ld %5ld  %.3f %ld  %.3f %ld\n", "",
		gnor_ini.r_bxl, gnor_ini.r_bxr, gnor_ini.r_byb, gnor_ini.r_byt,
		gnor_ini.dx, gnor_ini.nx, gnor_ini.dy, gnor_ini.ny);
	}

	for (i = 0; i < gnor_ini.no_xy; ++i) {
	    dmGetDesignData (fp, GEO_NOR_XY);
	    printf ("%10.3f %10.3f\n", gnor_xy.x, gnor_xy.y);
	}
    }
    dmCloseStream (fp, QUIT);
}

void read_bxx (char *file)
{
    int v_mode_bln = 0;
    int ft = 0;
    int term = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) { ++ft;
	if (file[0] == 't' && file[1] == '_') ++term;
	v_mode_bln = !strcmp (file, "cont_bln");
    }
    while (dmGetDesignData (fp, GEO_BOXLAY) > 0) {
	if (ft) { ft = 0;
	    printf ("  xl:   xr:   yb:   yt:  %s:\n", (term ? "term#" : " ct"));
	}
	if (mode4)
	    printf ("%5ld %5ld %5ld %5ld %5ld",
		gboxlay.xl/4, gboxlay.xr/4, gboxlay.yb/4, gboxlay.yt/4, gboxlay.chk_type);
	else
	    printf ("%5ld %5ld %5ld %5ld %5ld",
		gboxlay.xl, gboxlay.xr, gboxlay.yb, gboxlay.yt, gboxlay.chk_type);
	if (v_mode_bln) {
	    printf (" (%c%c%2ld",
		gboxlay.chk_type & 0x400 ? 'E' : 'B',
		gboxlay.chk_type & 0x200 ? 'D' : 'N',
		gboxlay.chk_type & 0xFF);
	    if (gboxlay.chk_type & 0x100) printf ("i");
	    if (gboxlay.chk_type & 0x800) printf ("s");
	    printf (")");
	}
	printf ("\n");
    }
    dmCloseStream (fp, QUIT);
}

void read_gln (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_GLN) > 0) {
	if (ft) { ft = 0;
	    printf ("  xl:   xr:   yl:   yr:\n");
	}
	if (mode4)
	    printf ("%5ld %5ld %5ld %5ld\n", ggln.xl/4, ggln.xr/4, ggln.yl/4, ggln.yr/4);
	else
	    printf ("%5ld %5ld %5ld %5ld\n", ggln.xl, ggln.xr, ggln.yl, ggln.yr);
    }
    dmCloseStream (fp, QUIT);
}

void read_nxx (char *file)
{
    int ft = 0;
    register int i;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_NXX_INI) > 0) {
	if (ft) { ft = 0;
	    printf ("elmt: no_xy: [xc: yc: r1: r2: a1: a2:]\n");
	    printf ("x: y:\n");
	}
	printf ("%ld %4ld", gnxx_ini.elmt, gnxx_ini.no_xy);
	if (gnxx_ini.elmt == CIRCLE_NOR) {
	    if (mode4)
		printf (" %.3f %.3f %.3f %.3f %.3f %.3f\n",
		    gnxx_ini.xc/4, gnxx_ini.yc/4,
		    gnxx_ini.r1/4, gnxx_ini.r2/4,
		    gnxx_ini.a1, gnxx_ini.a2);
	    else
		printf (" %.3f %.3f %.3f %.3f %.3f %.3f\n",
		    gnxx_ini.xc, gnxx_ini.yc,
		    gnxx_ini.r1, gnxx_ini.r2,
		    gnxx_ini.a1, gnxx_ini.a2);
	}
	else {
	    printf ("\n");
	}
	for (i = 0; i < gnxx_ini.no_xy; ++i) {
	    dmGetDesignData (fp, GEO_NXX_XY);
	    if (mode4)
		printf ("%10.3f %10.3f\n", gnxx_xy.x/4, gnxx_xy.y/4);
	    else
		printf ("%10.3f %10.3f\n", gnxx_xy.x, gnxx_xy.y);
	}
    }
    dmCloseStream (fp, QUIT);
}

void read_pgt (char *file)
{
    int i, ft = 0;
    long X, Y, W, V, U;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while ((i = dmGET (fp -> dmfp, "DDDDD", &X, &Y, &W, &V, &U)) == 5) {
	if (ft) { ft = 0;
	    printf ("      X:       Y:       W:       V:    U:\n");
	}
	printf ("%8ld %8ld %8ld %8ld %5ld\n", X, Y, W, V, U);
    }
    if (i != EOF) error (3, file);
    dmCloseStream (fp, QUIT);
}

void read_vln (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_VLNLAY) > 0) {
	if (ft) { ft = 0;
	    printf ("   x:   yb:   yt:  occ: con:  ct:  grp:\n");
	}
	printf ("%5ld %5ld %5ld     %c    %c   %2ld %5ld\n",
	    gvlnlay.x, gvlnlay.yb, gvlnlay.yt,
	    gvlnlay.occ_type, gvlnlay.con_type,
	    gvlnlay.chk_type, gvlnlay.grp_number);
    }
    dmCloseStream (fp, QUIT);
}

void read_teq (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_TEQ) > 0) {
	if (ft) { ft = 0;
	    printf ("term#:  grp:\n");
	}
	printf ("%4ld    %ld\n", gteq.term_number, gteq.grp_number);
    }
    dmCloseStream (fp, QUIT);
}

void read_tid (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    while (dmGetDesignData (fp, GEO_TID) > 0) {
	if (gtid.term_offset == -1) {
	    if (v_mode) { ++ft;
		printf ("cell: inst_name: nx: ny:\n");
	    }
	    printf ("%-10s %-10s %2ld %2ld\n",
		gtid.cell_name, gtid.inst_name, gtid.m_nx, gtid.m_ny);
	}
	else {
	    if (ft) { ft = 0;
		printf ("off: term_name: nx: ny:\n");
	    }
	    printf ("%-3ld %-10s %2ld %2ld\n",
		gtid.term_offset, gtid.term_name, gtid.t_nx, gtid.t_ny);
	}
    }
    dmCloseStream (fp, QUIT);
}

void read_spec (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, GEO_SPEC) > 0) {
	if (ft) { ft = 0;
	    printf ("lay: xl: xr: yb: yt: name:\n");
	}
	printf ("%-3s %5ld %5ld %5ld %5ld  %s\n",
	    gspec.layer, gspec.xl, gspec.xr, gspec.yb, gspec.yt, gspec.name);
    }
    dmCloseStream (fp, QUIT);
}

void read_flashes (char *file)
{
    register int i, j, n;
    int v;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (dmScanf (fp, "%d", &v) != 1) error (3, file);
    printf ("%s%d\n", (v_mode) ? "area = " : "", v);
    if (dmScanf (fp, "%d", &v) != 1) error (3, file);
    printf ("%s%d\n", (v_mode) ? "overlay = " : "", v);
    for (n = i = 0; i < process -> nomasks; ++i) {
	if (process -> pgt_no[i] <= 0) continue;
	++n;
	if (dmScanf (fp, "%d", &v) != 1) error (3, file);
	if (v_mode && n == 1) printf ("#boxes:  (lay:)\n");
	printf ("%7d", v);
	if (v_mode) {
	    for (j = 0; j < process -> nomasks; ++j)
		if (process -> pgt_no[j] == n) {
		    printf ("  (%s)\n", Lay (j));
		    break;
		}
	}
	else printf ("\n");
    }
    for (n = i = 0; i < process -> nomasks; ++i) {
	if (process -> pgt_no[i] <= 0) continue;
	++n;
	if (dmScanf (fp, "%d", &v) != 1) error (3, file);
	printf ("%7d", v);
	if (v_mode) {
	    for (j = 0; j < process -> nomasks; ++j)
		if (process -> pgt_no[j] == n) {
		    printf ("  (%s_neg)\n", Lay (j));
		    break;
		}
	}
	else printf ("\n");
    }
    dmCloseStream (fp, QUIT);
}

void readc_info (char *file)
{
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    dmGetDesignData (fp, CIR_INFO);
    printf ("bxl,bxr,byb,byt: %6ld %6ld %6ld %6ld\n",
	cinfo.bxl, cinfo.bxr, cinfo.byb, cinfo.byt);
    dmCloseStream (fp, QUIT);
}

void readc_mc (char *file)
{
    register long i;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;

    cmc.inst_attribute = attribute_string;
    cmc.inst_lower = lower;
    cmc.inst_upper = upper;

    while (dmGetDesignData (fp, CIR_MC) > 0) {
	if (n_mode) {
	    if (strcmp (n_mode, cmc.cell_name)) continue;
	}
	if (b_mode) {
	    printf ("%s %s", cmc.cell_name, cmc.inst_name);
	    if (cmc.inst_dim) {
		for (i = 0; i < cmc.inst_dim; ++i) {
		    printf ("%c%ld", i?',':'[', cmc.inst_lower[i]);
		    if (cmc.inst_lower[i] != cmc.inst_upper[i]) printf ("..%ld", cmc.inst_upper[i]);
		}
		printf ("]");
	    }
	    if (*cmc.inst_attribute) printf (" %s", cmc.inst_attribute);
	    printf ("%s\n", (cmc.imported ? " (IMP)" : ""));
	    continue;
	}
	if (cmc.inst_dim == 0) {
	    printf ("cell:\"%s\" inst:\"%s\" attr:\"%s\"", cmc.cell_name, cmc.inst_name, cmc.inst_attribute);
	    printf ("%s\n", (cmc.imported ? " (IMP)" : ""));
	}
	else {
	    printf ("cell:\"%s\" inst:\"%s\" attr:\"%s\" dim:%ld",
		cmc.cell_name, cmc.inst_name, cmc.inst_attribute, cmc.inst_dim);
	    printf ("%s\n", (cmc.imported ? " (IMP)" : ""));
	    for (i = 0; i < cmc.inst_dim; ++i) {
		printf ("lower,upper[%ld]: %ld, %ld\n", i, cmc.inst_lower[i], cmc.inst_upper[i]);
	    }
	}
    }
    dmCloseStream (fp, QUIT);
}

void readc_term (char *file)
{
    register long i;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;

    cterm.term_attribute = attribute_string;
    cterm.term_lower = lower;
    cterm.term_upper = upper;

    while (dmGetDesignData (fp, CIR_TERM) > 0) {
	if (b_mode) {
	    printf ("%s", cterm.term_name);
	    if (cterm.term_dim) {
		for (i = 0; i < cterm.term_dim; ++i) {
		    printf ("%c%ld", i?',':'[', cterm.term_lower[i]);
		    if (cterm.term_lower[i] != cterm.term_upper[i]) printf ("..%ld", cterm.term_upper[i]);
		}
		printf ("]");
	    }
	    if (*cterm.term_attribute) printf (" %s", cterm.term_attribute);
	    printf ("\n");
	    continue;
	}
	printf ("term:\"%s\" attr:\"%s\" dim:%ld\n",
	    cterm.term_name, cterm.term_attribute, cterm.term_dim);
	for (i = 0; i < cterm.term_dim; ++i) {
	    printf ("lower,upper[%ld]: %ld, %ld\n", i, cterm.term_lower[i], cterm.term_upper[i]);
	}
    }
    dmCloseStream (fp, QUIT);
}

void readc_net (char *file)
{
    register long i, j, len, nr;
    int submode, done = 0;

    header (file);

    if (strcmp (file, "nethead") == 0) {
	double x, y;
	if ((fp = dmOpenStream (key, file, "r")))
	while (dmGetDesignData (fp, CIR_NET_HEAD) > 0) {
	    x = cnethead.node_x;
	    y = cnethead.node_y;
	    if (mode4) { x /= 4; y /= 4; }
	    printf ("node=%2ld cd=%2ld lay=%2d x=%*g y=%*g area=%d term=%d neqv=%ld pos=%lld\n",
		cnethead.node_nr, cnethead.cd_nr, cnethead.lay_nr, w_mode, x, w_mode, y,
		(int)cnethead.area, (int)cnethead.term, cnethead.net_neqv, cnethead.offset);
	}
	return;
    }

    submode = (strcmp (file, "netsub") == 0);

    if (!(fp = dmOpenStream (key, file, "r"))) return;

    cnet.net_attribute = attribute_string;
    cnet.net_lower  = lower;
    cnet.net_upper  = upper;
    cnet.inst_lower = lower1;
    cnet.inst_upper = upper1;
    cnet.ref_lower  = lower2;
    cnet.ref_upper  = upper2;

    if (t_mode) len = strlen (t_mode); else len = 0;

    while (dmGetDesignData (fp, CIR_NET_ATOM) > 0) {
head:
	if (n_mode) {
	    if (done) break;
	    if (strcmp (n_mode, cnet.net_name)) continue;
	    ++done;
	}

	if (t_mode) { /* list only with attribute */
	    char *t, *s = attribute_string;
	    while (*s) {
		t = s;
		while (*t != ';' && *t) ++t;
		if (t - s == len && strncmp (s, t_mode, len) == 0) break;
		s = t;
	    }
	    if (!*s) {
		nr = cnet.net_neqv;
		for (j = 0; j < nr; ++j) {
		    if (dmGetDesignData (fp, CIR_NET_ATOM) <= 0) error (3, file);
		}
		continue;
	    }
	}

	nr = cnet.net_neqv;
	j = 0;
    if (b_mode) {
	if (submode) {
	    printf ("net:%ld\n", nr);
	    printf ("    %s", cnet.net_name);
	    j = 1; nr = 2;
	}
	else
	    printf ("%s", cnet.net_name);
	if (cnet.net_dim) {
	    for (i = 0; i < cnet.net_dim; ++i) {
		printf ("%c%ld", i?',':'[', cnet.net_lower[i]);
		if (cnet.net_lower[i] != cnet.net_upper[i]) printf ("..%ld", cnet.net_upper[i]);
	    }
	    printf ("]");
	}
	if (*cnet.inst_name) printf (" %s", cnet.inst_name);
	if (*cnet.net_attribute) printf (" %s", cnet.net_attribute);
	printf ("\n");
    }
    else {
	if (submode) {
	    printf ("net:%ld\n", nr);
	    printf ("    subnet[0]:\"%s\" inst:\"%s\" attr:\"%s\"\n",
		cnet.net_name, cnet.inst_name, cnet.net_attribute);
	    j = 1; nr = 2;
	}
	else
	    printf ("net:\"%s\" inst:\"%s\" attr:\"%s\" subnets:%ld\n",
		cnet.net_name, cnet.inst_name, cnet.net_attribute, nr);
	for (i = 0; i < cnet.net_dim; ++i) {
	    printf ("net_lower,upper[%ld]: %ld, %ld\n", i, cnet.net_lower[i], cnet.net_upper[i]);
	}
	for (i = 0; i < cnet.inst_dim; ++i) {
	    printf ("inst_lower,upper[%ld]: %ld, %ld\n", i, cnet.inst_lower[i], cnet.inst_upper[i]);
	}
	for (i = 0; i < cnet.ref_dim; ++i) {
	    printf ("ref_lower,upper[%ld]: %ld, %ld\n", i, cnet.ref_lower[i], cnet.ref_upper[i]);
	}
    }

	for (; j < nr; ++j) {
	    if (dmGetDesignData (fp, CIR_NET_ATOM) <= 0) error (3, file);
	    if (submode) {
		if (cnet.net_neqv > 0) goto head;
		++nr;
	    }

	    if (b_mode) {
		printf ("    %s", cnet.net_name);
		if (cnet.net_dim) {
		    for (i = 0; i < cnet.net_dim; ++i) printf ("%c%ld", i?',':'[', cnet.net_lower[i]);
		    printf ("]");
		}
		if (*cnet.inst_name) printf (" %s", cnet.inst_name);
		if (cnet.inst_dim) {
		    for (i = 0; i < cnet.inst_dim; ++i) {
			printf ("%c%ld", i?',':'[', cnet.inst_lower[i]);
			if (cnet.inst_lower[i] != cnet.inst_upper[i]) printf ("..%ld", cnet.inst_upper[i]);
		    }
		    printf ("]");
		}
		if (*cnet.net_attribute) printf (" %s", cnet.net_attribute);
		printf ("\n");
		continue;
	    }
	    printf ("    subnet[%ld]:\"%s\" inst:\"%s\" attr:\"%s\"\n", j,
		cnet.net_name, cnet.inst_name, cnet.net_attribute);
	    for (i = 0; i < cnet.net_dim; ++i) {
		printf ("    net_lower,upper[%ld]: %ld, %ld\n", i, cnet.net_lower[i], cnet.net_upper[i]);
	    }
	    for (i = 0; i < cnet.inst_dim; ++i) {
		printf ("    inst_lower,upper[%ld]: %ld, %ld\n", i, cnet.inst_lower[i], cnet.inst_upper[i]);
	    }
	    for (i = 0; i < cnet.ref_dim; ++i) {
		printf ("    ref_lower,upper[%ld]: %ld, %ld\n", i, cnet.ref_lower[i], cnet.ref_upper[i]);
	    }
	}
    }
    dmCloseStream (fp, QUIT);
}

void read_file (char *file)
{
    FILE *dmfp;
    int c;

    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    dmfp = fp -> dmfp;

    while ((c = getc (dmfp)) != EOF) printf ("%c", c);

    dmCloseStream (fp, QUIT);
}

void readc_geo (char *file)
{
    FILE *dmfp;
    int i;
    char buf[80], c;
    long tile, cx, d1, d3, d4, d5, d6, d7, d8;

    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    dmfp = fp -> dmfp;

    if ((c = getc (dmfp)) != '#' || (c = getc (dmfp)) != '1' || (c = getc (dmfp)) != '\n') {
	printf ("incompatible version of '%s' file\n", file);
	goto ret;
    }

    if (v_mode) {
	if (*file == 'd')
	    printf ("-dev# ( instname )\n");
	else
	    printf ("-grp# ( netname... )\n");
	printf ("tile# cc# mask# color xl xr bl br tl tr\n");
    }

    do {
	c = getc (dmfp);
	while (c != '\n' && c != EOF) {
	    printf ("%c", c); c = getc (dmfp);
	}
	if (c == EOF) goto ret;
	printf ("\n");

	do {
	    i = dmGET (dmfp, "DDDSDDDDDD", &tile, &cx, &d1, buf, &d3, &d4, &d5, &d6, &d7, &d8);
	    if (i <= 0) break;
	    if (mode4)
		printf ("%ld %ld %ld %s %ld %ld %ld %ld %ld %ld\n",
		    tile, cx, d1, buf, d3/4, d4/4, d5/4, d6/4, d7/4, d8/4);
	    else
		printf ("%ld %ld %ld %s %ld %ld %ld %ld %ld %ld\n",
		    tile, cx, d1, buf, d3, d4, d5, d6, d7, d8);
	} while (tile > 0 && i > 0);
    } while (i > 0);
ret:
    dmCloseStream (fp, QUIT);
}

void readf_mc (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, FLP_MC) > 0) {
	if (n_mode) {
	    if (strcmp (n_mode, fmc.cell_name)) continue;
	}
	if (ft) { ft = 0;
	    printf ("cell: inst: tx: ty: mt[0,1,3,4]: bxl,bxr,byb,byt: [dx,nx: dy,ny:]\n");
	}
	printf ("%-10s %-10s %5ld %5ld  %2ld %2ld %2ld %2ld %5ld %5ld %5ld %5ld",
	    fmc.cell_name, fmc.inst_name,
	    fmc.mtx[2], fmc.mtx[5],
	    fmc.mtx[0], fmc.mtx[1], fmc.mtx[3], fmc.mtx[4],
	    fmc.bxl, fmc.bxr, fmc.byb, fmc.byt);
	if (fmc.nx || fmc.ny)
	    printf ("  %ld %ld  %ld %ld", fmc.dx, fmc.nx, fmc.dy, fmc.ny);
	printf ("%s\n", (fmc.imported ? " (IMP)" : ""));
    }
    dmCloseStream (fp, QUIT);
}

void readf_info (char *file)
{
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    dmGetDesignData (fp, FLP_INFO);
    printf ("bxl,bxr,byb,byt: %6ld %6ld %6ld %6ld\n",
	finfo.bxl, finfo.bxr, finfo.byb, finfo.byt);
    dmCloseStream (fp, QUIT);
}

void readf_term (char *file)
{
    int ft = 0;
    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    if (v_mode) ++ft;
    while (dmGetDesignData (fp, FLP_TERM) > 0) {
	if (ft) { ft = 0;
	    printf ("term: lay: side:  xl,xr,yb,yt:  attr:\n");
	    printf ("[    bxl,bxr,byb,byt:  dx,nx: dy,ny: ]\n");
	}
	printf ("%-10s %-3s %ld  %5ld %5ld %5ld %5ld  \"%s\"\n",
	    fterm.term_name, Lay2 (fterm.layer_no), fterm.side,
	    fterm.xl, fterm.xr, fterm.yb, fterm.yt,
	    fterm.term_attribute);
	if (fterm.nx || fterm.ny) {
	    printf ("%18s%5ld %5ld %5ld %5ld  %ld %ld  %ld %ld\n", "",
		fterm.bxl, fterm.bxr, fterm.byb, fterm.byt,
		fterm.dx, fterm.nx, fterm.dy, fterm.ny);
	}
    }
    dmCloseStream (fp, QUIT);
}

void readf_chan (char *file)
{
    register int i, j;
    register struct flp_glr *fn;
    register struct flp_connect *fc;
    int done = 0;

    header (file);
    if (!(fp = dmOpenStream (key, file, "r"))) return;
    while (dmGetDesignData (fp, FLP_CHAN) > 0) {
	if (n_mode) {
	    if (done) break;
	    if (strcmp (n_mode, fchan.channel_name)) continue;
	    ++done;
	}
	printf ("chan:\"%s\" xl,xr,yb,yt:%ld,%ld,%ld,%ld kind:%ld order:%ld nets:%ld\n",
	    fchan.channel_name, fchan.xl, fchan.xr, fchan.yb, fchan.yt,
	    fchan.kind, fchan.order, fchan.flp_nlist);

	for (j = 0; j < fchan.flp_nlist; ++j) {
	    fn = fchan.flp_netlist + j;
	    printf ("    net[%d]:\"%s\" attr:\"%s\" connects:%ld\n", j,
		fn -> net_name, fn -> net_attribute, fn -> flp_nconnect);
	    for (i = 0; i < fn -> flp_nconnect; ++i) {
		fc = fn -> flp_netconnect + i;
		printf ("        con[%d]:\"%s\" type:%ld origin:\"%s\" nx,ny:%ld,%ld\n",
		    i, fc -> connect_name, fc -> connect_type,
		    fc -> connect_origin, fc -> nx, fc -> ny);
	    }
	}
    }
    dmCloseStream (fp, QUIT);
}

void header (char *file)
{
    printf ("=> %s/%s/%s\n", view, cell, file);
}

#ifdef DO_SIG
void sig_handler (int sig) /* signal handler */
{
    signal (sig, SIG_IGN); /* ignore signal */
    PE "\n");
    error (0, "");
}
#endif

void dmError (char *s)
{
    if (dmerrno == DME_FOPEN) {
	printf ("%s: %s ==== cannot open ====\n", argv0, s);
	return;
    }
    PE "%s: ", argv0);
    dmPerror (s);
    error (1, "");
}

char *errlist[] = {
/* 0 */  "received interrupt signal",
/* 1 */  "error in DMI function",
/* 2 */  "unknown dbstream: %s",
/* 3 */  "%s: read error",
/* 4 */  "unknown error"
};

void error (int nr, char *s)
{
    if (nr < 0 || nr > 4) nr = 4;
    PE "%s: ", argv0);
    PE errlist[nr], s);
    PE "\n%s: -- program aborted --\n", argv0);
    dmQuit ();
    exit (1);
}
