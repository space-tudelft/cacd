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

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h> /* for access */
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/scan/scan.h"
#include "src/space/scan/extern.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* No need to initialize if they are read in
 * lookupParameters, since the defaults are
 * SPECIFIED THERE !!!
 */
int connect_ground = 0;
int new_convex = 1;
int new_refine = 1;
int new_via_mode = 1;
int min_art_degree = -1;
int elim_check_again = 0;
int equi_line_area = 0;
int equi_line_new  = 0;
int equi_line_old  = 0;
int join_contacts  = 0;
int area_contacts  = 0;
int keep_contacts  = 0;
double equi_line_ratio = 0;
double use_corner_ratio = 0;
double MIN_CONDUCTANCE = 1e-50;
double max_par_res = 0;
double minRes = 0;
double min_res = 0;
double min_sep_res = 0;
double low_contact_res = 0;
double low_sheet_res = 0;
double compensate_lat_part = 0;

extern int add_sub_caps;
extern bool_t omit_ds_caps;
extern bool_t omit_subcont;
bool_t contacts_sub = TRUE;
bool_t spider_hash = FALSE;
bool_t omit_inc_tors = FALSE;
bool_t paramCapitalize = FALSE;
bool_t term_use_center = FALSE;
bool_t eliminateSubstrNode = FALSE;
bool_t elim_sub_con = FALSE;
bool_t omit_self_sub_res = FALSE;

double max_tan_slice_y = -1;
double physBandWidth = 0;
double equiBandWidth = 0;
double sub_rc_const  = 0;

/* this variable is defined in the space .p file and declares
 * the maximum base width of a lateral bipolar pnp transistor
 */
double max_lat_base = 3;
bool_t parallelMerge = FALSE;

static char *paramfile;

char * getParameters (DM_PROJECT *dmproject, char *libFile, char *userFile)
{
    char *pfile;
    char filebuf[256];

    if (!userFile && !libFile) {
	libFile = "space.def.p";
	sprintf (filebuf, "%s/.%s", dmproject -> dmpath, libFile);
	if (access (filebuf, 0) == 0)
	    pfile = filebuf;
	else
	    pfile = dmGetMetaDesignData (PROCPATH, dmproject, libFile);
    }
    else {
	if (!(pfile = userFile)) {
	    libFile = mprintf ("space.%s.p", libFile);
	    pfile = dmGetMetaDesignData (PROCPATH, dmproject, libFile);
	}
	if (access (pfile, 0) != 0) {
	    say ("%s: no such parameter file", giveICD (pfile));
	    die ();
	}
    }

    paramfile = strsave (pfile);
    paramReadFile (paramfile);
    return (paramfile);
}

void lookupParameters ()
{
    char *s;

    if (optIntRes) {
	int use_corner_nodes;

	if (!optNoReduc) {
	    max_par_res = paramLookupD ("max_par_res", "20");
	    min_sep_res = paramLookupD ("min_sep_res", "0");
	    if (min_sep_res > 0) minRes = min_sep_res = 1 / min_sep_res;
	    min_res = paramLookupD ("min_res", "0");
	    if (min_res > 0) { min_res = 1 / min_res;
		if (minRes == 0 || min_res < minRes) minRes = min_res;
		else say ("warning: min_res <= min_sep_res; min_res not used");
	    }
	    if (!optFineNtw) min_art_degree = paramLookupI ("min_art_degree", "4");
	}

	MIN_CONDUCTANCE = paramLookupD ("max_res", "1e50");
	if (MIN_CONDUCTANCE < 1e6) MIN_CONDUCTANCE = 1e-6;
	else if ((MIN_CONDUCTANCE = 1 / MIN_CONDUCTANCE) == 0) MIN_CONDUCTANCE = 1e-300;

	if (!optAllRes) {
	    low_contact_res = paramLookupD ("low_contact_res", "0");
	    if (low_contact_res < 0) low_contact_res = 0;
	    low_sheet_res = paramLookupD ("low_sheet_res", "0");
	    if (low_sheet_res < 0) low_sheet_res = 0;
	}

	use_corner_nodes = paramLookupB ("use_corner_nodes", "off");
	if (use_corner_nodes && optResMesh == 1) {
	    use_corner_nodes = 0;
	    say ("warning: no corner_nodes possible in mesh refinement mode");
	}

	equi_line_ratio = paramLookupD ("equi_line_ratio", "0");
	if (equi_line_ratio > 0) {
	    equi_line_area = paramLookupB ("equi_line_area", "on");
	    equi_line_new  = paramLookupI ("equi_line_new", "0");
	    if (equi_line_new < 3) equi_line_old  = 1;
	    if (use_corner_nodes) {
		if (!equi_line_area)
		    say ("warning: no corner_nodes, equi_line_area must be on");
		if (equi_line_new > 1)
		    say ("warning: no corner_nodes, equi_line_new must be < 2");
		else {
		    equiBandWidth = paramLookupD ("equi_line_width", "0");
		    if (equiBandWidth <= 0)
			say ("warning: no corner_nodes, equi_line_width must be > 0");
		    else {
			optResMesh = 2;
			use_corner_ratio = paramLookupD ("use_corner_ratio", "0.5");
			if (use_corner_ratio > 0) join_contacts = keep_contacts = 1;
		    }
		}
	    }
	}
	else if (use_corner_nodes) say ("warning: no corner_nodes, equi_line_ratio must be > 0");

	if (!area_contacts) area_contacts = paramLookupB ("area_contacts", "off");
	if (!join_contacts) join_contacts = paramLookupB ("join_contacts", "off");
	if (!keep_contacts) keep_contacts = paramLookupB ("keep_contacts", "off");
	elim_check_again = paramLookupB ("elim_check_again", "off");
    }

#ifdef TOR_DS_BOUNDARIES
    optDsConJoin    = paramLookupB ("add_ds_terms", "off");
#endif
    omit_ds_caps    = paramLookupB ("omit_ds_caps", "off");
    omit_inc_tors   = paramLookupB ("omit_incomplete_tors", "off");
    paramCapitalize = paramLookupB ("capitalize", "off");

if (substrRes) {
    add_sub_caps = paramLookupI ("add_sub_caps", "0");
    sub_rc_const = paramLookupD ("sub_rc_const", "0");
    omit_self_sub_res = paramLookupB ("omit_self_sub_res", "off");
    omit_subcont = paramLookupB ("omit_unused_sub_term", "off");
}

    if (optLatCap) {
	physBandWidth = paramLookupD ("lat_cap_window", "0");
	if (physBandWidth <= 0) {
	    say ("warning: parameter lat_cap_window not set or equal zero");
	    say ("warning: lateral coupling capacitances are not extracted");
	    optLatCap = FALSE;
	}
	compensate_lat_part = paramLookupD ("compensate_lat_part", "1");
    }
    max_lat_base    = paramLookupD ("lat_base_width", "3");
    parallelMerge   = paramLookupB ("merge_par_bjts", "off");
    term_use_center = paramLookupB ("term_use_center", (optIntRes ? "on" : "off"));

    if (optResMesh == 1) {
	max_tan_slice_y = paramLookupD ("split_ratio", "-1");
	if (max_tan_slice_y < 0) {
	    double max_obtuse = paramLookupD ("max_obtuse", "-1");
	    if (max_obtuse > 90.00001) {
		if (max_obtuse >= 179.99999) max_tan_slice_y = 0;
		else max_tan_slice_y = 1 / tan (M_PI * (max_obtuse - 90) / 180);
	    }
	}
    }

    setMaxMessageCnt (paramLookupI ("max_message_cnt",  "-1"));
}
