/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftehelp.h"
#include "hlpdefs.h"

static void byemesg();

void com_help (wordlist *wl)
{
    struct comm *c;
    struct comm *ccc[512];  /* Should be enough. */
    int n, i;
    bool allflag = false;

    if (wl && eq(wl->wl_word, "all")) {
        allflag = true;
        wl = NULL;  /* XXX Probably right */
    }

    out_init();
    if (wl == NULL) {
	out_send("For a complete description read the Spice3 User's Manual manual.\n");

	if (!allflag) {
	    out_send("For a list of all commands type \"help all\", for a short\n");
	    out_send("description of \"command\", type \"help command\".\n");
	}

        /* Sort the commands */
        for (n = i = 0; cp_coms[i].co_comname; i++) {
            if (ft_nutmeg && cp_coms[i].co_spiceonly) continue;
	    if (!allflag && !cp_coms[i].co_major) continue;
	    ccc[n++] = &cp_coms[i];
	}
	if (n > 1) qsort(ccc, n, sizeof (struct comm *), hcomp);

        for (i = 0; i < n; i++) {
            out_send(ccc[i]->co_comname);
            out_send(" ");
            out_printf(ccc[i]->co_help, cp_program);
            out_send("\n");
        }
    } else {
        while (wl) {
            for (c = &cp_coms[0]; c->co_comname; c++)
                if (eq(wl->wl_word, c->co_comname)) {
                    out_send(c->co_comname);
		    out_send(" ");
                    out_printf(c->co_help, cp_program);
                    if (c->co_spiceonly && ft_nutmeg)
                        out_send(" (Not available in nutmeg)");
                    break;
                }
            if (!c->co_comname) {
                struct alias *al; /* See if this is aliased. */
                for (al = cp_aliases; al; al = al->al_next)
                    if (eq(al->al_name, wl->wl_word)) break;
                if (al) {
                    out_send(wl->wl_word);
                    out_send(" is aliased to ");
                    out_send(al->al_text);
                    out_send("\n");
                }
		else {
                    out_send("Sorry, no help for ");
                    out_send(wl->wl_word);
                    out_send(".\n");
                }
            }
            wl = wl->wl_next;
        }
    }

    out_send("\n");
}

void com_ahelp (wordlist *wl)
{
    struct comm *ccc[512];  /* Should be enough. */
    struct comm *com;
    int env = 0;
    int i, n;
    int level;
    char slevel[256];

    if (wl) { com_help(wl); return; }

    out_init();

    /* determine environment */
    if (plot_list->pl_next) {   /* plots load */
      env |= E_HASPLOTS;
    } else {
      env |= E_NOPLOTS;
    }

    /* determine level */
    level = 1;
    if (cp_getvar("level", VT_STRING, slevel)) {
      switch (*slevel) {
	case 'b': level = 1; break;
	case 'i': level = 2; break;
	case 'a': level = 4; break;
      }
    }

    out_send("For a complete description read the Spice3 User's Manual manual.\n");
    out_send("For a list of all commands type \"help all\", for a short\n");
    out_send("description of \"command\", type \"help command\".\n");

    /* sort the commands */
    for (n = i = 0; cp_coms[i].co_comname; i++) {
        if (ft_nutmeg && cp_coms[i].co_spiceonly) continue;
	ccc[n++] = &cp_coms[i];
    }
    qsort(ccc, n, sizeof (struct comm *), hcomp);

    /* filter the commands */
    for (i = 0; i < n; i++) {
      com = ccc[i];
      if ((com->co_env < (level << 13)) && (!(com->co_env & 4095) || (env & com->co_env))) {
        out_send(com->co_comname);
        out_send(" ");
        out_printf(com->co_help, cp_program);
        out_send("\n");
      }
    }

    out_send("\n");
}

void com_ghelp (wordlist *wl)
{
    char buf[BSIZE_SP];
    int i;

    if (!Help_Path) {
        fprintf(cp_err, "Note: help dir path not set.\n");
        fprintf(cp_err, "Note: defaulting to old help.\n\n");
        com_help(wl);
        return;
    }

    if (cp_getvar("helpregfont",    VT_STRING, buf)) hlp_regfontname = copy(buf);
    if (cp_getvar("helpboldfont",   VT_STRING, buf)) hlp_boldfontname = copy(buf);
    if (cp_getvar("helpitalicfont", VT_STRING, buf)) hlp_italicfontname = copy(buf);
    if (cp_getvar("helptitlefont",  VT_STRING, buf)) hlp_titlefontname = copy(buf);
    if (cp_getvar("helpbuttonfont", VT_STRING, buf)) hlp_buttonfontname = copy(buf);
    if (cp_getvar("helpinitxpos", VT_NUM, (char *) &i)) hlp_initxpos = i;
    if (cp_getvar("helpinitypos", VT_NUM, (char *) &i)) hlp_initypos = i;
    if (cp_getvar("helpbuttonstyle", VT_STRING, buf)) {
        if (cieq(buf, "left"))
            hlp_buttonstyle = BS_LEFT;
        else if (cieq(buf, "center"))
            hlp_buttonstyle = BS_CENTER;
        else if (cieq(buf, "unif"))
            hlp_buttonstyle = BS_UNIF;
        else
            fprintf(cp_err, "Warning: no such button style %s\n", buf);
    }
    if (cp_getvar("width", VT_NUM, (char *) &i))
        hlp_width = i;
    if (cp_getvar("display", VT_STRING, buf))
        hlp_displayname = copy(buf);
    else
        hlp_displayname = NULL;

    hlp_main (Help_Path, wl);
}

int hcomp (struct comm **c1, struct comm **c2)
{
    return (strcmp((*c1)->co_comname, (*c2)->co_comname));
}

void com_quit (wordlist *wl)
{
    struct circ *cc;
    struct plot *pl;
    int ncc = 0, npl = 0;
    char buf[64];
    bool noask;

    (void) cp_getvar("noaskquit", VT_BOOL, (char *) &noask);
    gr_clean();
    cp_ccon(false);

    /* Make sure the guy really wants to quit. */
    if (!ft_nutmeg && !noask) {
        for (cc = ft_circuits; cc; cc = cc->ci_next)
            if (cc->ci_inprogress) ncc++;
        for (pl = plot_list; pl; pl = pl->pl_next)
            if (!pl->pl_written && pl->pl_dvecs) npl++;
	if (ncc || npl) {
            fprintf(cp_out, "Warning: ");
            if (ncc) {
                fprintf(cp_out, "the following simulation%s still in progress:\n",
                        (ncc > 1) ? "s are" : " is");
                for (cc = ft_circuits; cc; cc = cc->ci_next)
                    if (cc->ci_inprogress) fprintf(cp_out, "\t%s\n", cc->ci_name);
            }
            if (npl) {
                if (ncc) fprintf(cp_out, "and ");
                fprintf(cp_out, "the following plot%s been saved:\n",
                    (npl > 1) ? "s haven't" : " hasn't");
                for (pl = plot_list; pl; pl = pl->pl_next)
                    if (!pl->pl_written && pl->pl_dvecs)
                        fprintf(cp_out, "%s\t%s, %s\n", pl->pl_typename, pl->pl_title, pl->pl_name);
            }
            fprintf(cp_out, "\nAre you sure you want to quit (yes)? ");
            fflush(cp_out);
            if (!fgets(buf, BSIZE_SP, stdin)) {
                clearerr(stdin);
                *buf = 'y';
            }
            if (*buf != 'y' && *buf != 'Y' && *buf != '\n') return;
        }
    }
    byemesg();
    exit(EXIT_NORMAL);
}

void com_bug (wordlist *wl)
{
    char buf[BSIZE_SP];

    if (!Bug_Addr || !*Bug_Addr) {
        fprintf(cp_err, "Error: No address to send bug reports to.\n");
	return;
    }
#ifndef SYSTEM_MAIL
    fprintf(cp_err, "Error: SYSTEM_MAIL not defined.\n");
#else
    fprintf(cp_out, "Calling the mail program . . .(sending to %s)\n\n", Bug_Addr);
    fprintf(cp_out, "Please include the OS version number and machine architecture.\n");
    fprintf(cp_out, "If the problem is with a specific circuit, please include the\n");
    fprintf(cp_out, "input file.\n");

    sprintf(buf, SYSTEM_MAIL, ft_sim->simulator, ft_sim->version, Bug_Addr);
    (void) system(buf);
    fprintf(cp_out, "Bug report sent.  Thank you.\n");
#endif
}

void com_version (wordlist *wl)
{
    if (!wl) {
	fprintf(cp_out, "Program: %s, version: %s\n", ft_sim->simulator, ft_sim->version);
	if (Spice_Notice && *Spice_Notice)
	    fprintf(cp_out, "\t%s\n", Spice_Notice);
	if (Spice_Build_Date && *Spice_Build_Date)
	    fprintf(cp_out, "Date built: %s\n", Spice_Build_Date);
    } else {
	char *s;
        s = wl_flatten(wl);
        if (!eq(ft_sim->version, s)) {
            fprintf(stderr, "Note: rawfile is version %s (current version is %s)\n",
                    wl->wl_word, ft_sim->version);
        }
        tfree(s);
    }
}

static void byemesg()
{
    printf("%s-%s done\n", ft_sim->simulator, ft_sim->version);
}
