static char *rcsid = "$Id: opt.c,v 1.1 2018/04/30 12:17:41 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
/*
 *	DESCRIPTION:	Contains function(s) to parse options
 *			in the argument list.
 */
#include "src/match/head.h"
#include "src/eseLib/eseOption.h"
#include "src/match/proto.h"

/*	FUNCTIONS:
 */
Private int eseRange (char **var, char *arg);

/*	VARIABLES:
 */
Public boolean o_opt = False;
Public boolean s_opt = False;
Public boolean m_opt = False;
Public boolean i_opt = False;
Public boolean anno_opt = False;
Public boolean r_opt = False;
Public boolean b_opt = False;
Public boolean d_opt = False;
Public boolean n_or_d = True;
Public boolean c_opt = False;
Public boolean f_opt = False;
Public boolean x_opt = False;
Public boolean map_opt = True;
Public boolean par_opt = False;
Public boolean edif_opt = False;
Public boolean g_opt = False;
Public boolean l_opt = False;
Public boolean p_opt = False;
Public boolean t_opt = False;

Public boolean I_opt = False;
Public boolean R_opt = False;
Public boolean E_opt = False;
Public boolean N_opt = False;
Public boolean S_opt = False;
Public boolean V_opt = False;
Public boolean T_opt = False;
Public boolean D_opt = False;

Public char *  r_r_s;
Public char *  c_r_s;

Public double progress_size;
Public char * progress_string;

Public range * range_checks;

Public string l_val;
Public string p_val;
Public string o_val;

Public FILE * std_out;

Import string nom_netw_name;
Import string act_netw_name;
Import string string_cap, string_res, string_v;
Import long  yydebug;
Import char * comm;

#if 0
Public void set_options (int n_args, string *args)
{
    case 'd': d_opt = True; break;
    case 'i': i_opt = True; break;
}
#endif

OptionSpec optionSpecs[] = {
    { "", NO, (IFP) eseHelp, (void *) optionSpecs,
            "Usage: match [options] <nom_netw> <act_netw>\n\nOptions (may be abbreviated) are:"},
    { "annotate", NO, (IFP) eseTurnOn, (void *) &anno_opt,
            "  -annotate               generate annotate information for layout" },
    { "bindings", NO, (IFP) eseTurnOn, (void *) &b_opt,
            "  -bindings               print a full binding table on stdout" },
    { "byname", NO, (IFP) eseTurnOn, (void *) &N_opt,
            "  -byname                 do an initial match by name" },
    { "capacitors", NO, (IFP) eseTurnOn, (void *) &c_opt,
            "  -capacitors             ignore capacitors in both network descriptions" },
    { "crange", YES, (IFP) eseAssignArgument, (void *) &c_r_s,
            "  -crange value           specify the maximum range between 2 capacitors" },
    { "edif", NO, (IFP) eseTurnOn, (void *) &edif_opt,
            "  -edif                   produce output in an edif-like format" },
    { "expand", NO, (IFP) eseTurnOn, (void *) &E_opt,
            "  -expand                 print a listing of both networks after expansion" },
    { "files", NO, (IFP) eseTurnOn, (void *) &f_opt,
            "  -files                  both network arguments are file names" },
    { "fullbindings", NO, (IFP) eseTurnOn, (void *) &b_opt,
            "  -fullbindings           print a full binding table on stdout" },
    { "help", NO, (IFP) eseHelp, (void *) optionSpecs,
            "  -help                   print this list" },
    { "library", YES, (IFP) eseAssignArgument, (void *) &l_val,
            "  -library file_name      specify a library sls description file" },
    { "nomap", NO, (IFP) eseTurnOff, (void *) &map_opt,
            "  -nomap                  do not map elements if networks do not match" },
    { "output", YES, (IFP) eseAssignArgument, (void *) &o_val,
            "  -output file_name       redirect program output to specified file_name" },
    { "parameters", NO, (IFP) eseTurnOn, (void *) &par_opt,
            "  -parameters             print parameter values in binding table" },
    { "primitive", YES, (IFP) eseAssignArgument, (void *) &p_val,
            "  -primitive file_name    specify primitive element description file" },
    { "progress", YES, (IFP) eseAssignArgument, (void *) &progress_string,
            "  -progress percentage    report when specified progress has been made" },
    { "range", YES, (IFP) eseRange, (void *) &range_checks,
            "  -range dev:par,range    maximum 'range' between 2 'dev's for par. 'par'" },
    { "release", NO, (IFP) esePrintString, (void *) MATCH_VERSION,
            "  -release                print the release number of this tool"},
    { "resistors", NO, (IFP) eseTurnOn, (void *) &r_opt,
            "  -resistors              ignore resistors in both network descriptions" },
    { "rrange", YES, (IFP) eseAssignArgument, (void *) &r_r_s,
            "  -rrange value           specify the maximum range between 2 resistors" },
    { "silent", NO, (IFP) eseTurnOn, (void *) &S_opt,
            "  -silent                 silent mode. Only error messages are printed" },
    { "transistors", NO, (IFP) eseTurnOn, (void *) &t_opt,
            "  -transistors            only compare transistors (equiv. to -res -cap)" },
    { "verbose", NO, (IFP) eseTurnOn, (void *) &V_opt,
            "  -verbose                verbose mode" },
    { "xsls", NO, (IFP) eseTurnOn, (void *) &x_opt,
            "  -xsls                   use xsls to extract both SLS descriptions" },
    { "%command", YES, (IFP) eseAssignArgument, (void *) &comm,
            "  -%command string        specify the command to extract SLS descriptions" },
    { "%debug", NO, (IFP) eseTurnOn, (void *) &D_opt,
            "  -%debug                 debugging option" },
#ifdef DEBUG
    { "%yydebug", NO, (IFP) eseTurnOn, (void *) &yydebug,
            "  -%yydebug               parser debugging option" },
#endif
 /* { "%device", NO, (IFP) eseTurnOn, (void *) &n_or_d,
            "  -%device                focus on faulty devices instead of nets" }, */
 /* { "%etext", NO, (IFP) eseText, (void *) NULL,
            "  -%etext                 print the '(long) &etext' number" }, */
    { "%help", NO, (IFP) eseHelpAll, (void *) optionSpecs,
            "  -%help                  print this list (also hidden options)" },
    { "%list", NO, (IFP) eseTurnOn, (void *) &I_opt,
            "  -%list                  print a listing of both networks before expansion" },
    { "%rlist", NO, (IFP) eseTurnOn, (void *) &R_opt,
            "  -%rlist                 print a listing of both networks before reduction" },
 /* { "%net", NO, (IFP) eseTurnOff, (void *) &n_or_d,
            "  -%net                   focus on faulty nets instead of devices" }, */
    { "%mstatistics", NO, (IFP) eseTurnOn, (void *) &m_opt,
            "  -%mstatistics           print memory statistics" },
    { "%statistics", NO, (IFP) eseTurnOn, (void *) &s_opt,
            "  -%statistics            print some statistics about the matching process" },
    { "%time", NO, (IFP) eseTurnOn, (void *) &g_opt,
            "  -%time                  print info on the time taken by the program" },
 /* { "%toolid", NO, (IFP) esePrintString, (void *) "1",
            "  -%toolid                print the identifier of this tool"}, */
    { "%trace", NO, (IFP) eseTurnOn, (void *) &T_opt,
            "  -%trace                 trace the partitioning process" },
    { (char *) 0, (char) 0, (IFP) 0, (void *) 0, (char *) 0 },
};

Private int eseAddRange (char *dev, char *par, double val)
{
    range * new_range;

    new_range = (range *) calloc (1, sizeof (range));
    Assert (new_range);

    new_range -> device = dev;
    new_range -> parameter = par;
    new_range -> value = val;
    new_range -> next = range_checks;
    range_checks = new_range;
    return 0;
}

Public void ese_options (int argc, char **argv)
{
    /* Explicit initialization to work around bug in PC compiler */
    char **eseCellNames = (char **) calloc ((size_t)argc, sizeof (char *));
    if (eseOptionHandler (argc, argv, optionSpecs, 2, eseCellNames) > 0) {
	nom_netw_name = NULL;
    }
    else {
	nom_netw_name = eseCellNames[0];
	act_netw_name = eseCellNames[1];
    }

    if (!nom_netw_name || !act_netw_name) {
        err_mesg ("Usage: match [options] <nom_netw> <act_netw>\nVersion %s\n", MATCH_VERSION);
    }

    if (o_val) {
	if (!(std_out = fopen (o_val, "w")))
	    err_mesg ("Cannot access file '%s'\n", o_val);
	o_opt = True;
    }
    else
	std_out = stdout;

    if (t_opt) r_opt = c_opt = True;
    if (x_opt) f_opt = True;

    if (l_val) l_opt = True;
    if (p_val) p_opt = True;

    if (r_r_s) {
	double res_range = atof (r_r_s);
	if (res_range < 0) {
	    err_mesg ("Negative values not allowed for option '-resrange'\n");
	}
	eseAddRange (string_res, string_v, res_range);
    }

    if (progress_string) {
	progress_size = atof (progress_string);
	if (progress_size < 0) {
	    err_mesg ("Negative values not allowed for option '-progress'\n");
	}
    }

    if (c_r_s) {
	double cap_range = atof (c_r_s);
	if (cap_range < 0) {
	    err_mesg ("Negative values not allowed for option '-caprange'\n");
	}
	eseAddRange (string_cap, string_v, cap_range);
    }

    if (edif_opt) b_opt = True;

    if (S_opt) V_opt = T_opt = I_opt = E_opt = g_opt = False;
    if (T_opt) V_opt = True;

    verb_mesg ("Version %s\n", MATCH_VERSION);
}

Private int eseRange (char **var, char *arg)
{
    char *dc, *comma, *space;
    double val;

    dc = strchr (arg, ':');
    comma = strchr (arg, ',');
    space = strchr (arg, ' ');

    if (!dc || !comma || space || (val = atof (comma+1)) < 0.0) {
	err_mesg ("Illegal range specification: -range %s\n", arg);
	return 0;
    }

    *dc = '\0';
    *comma = '\0';

    eseAddRange (arg /* dev */, dc + 1 /* par */, val /* range */);

    return 0;
}
