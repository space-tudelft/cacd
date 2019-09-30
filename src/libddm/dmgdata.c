/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
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

#include "src/libddm/dmstd.h"

#ifndef NOPACK
#define _dmDoget _dmUnpack
#endif

int dm_get_do_not_alloc = 0;

int dmGetDesignData (DM_STREAM *dmfile, int fmt)
{
    _dmSetReleaseProperties (dmfile -> dmkey -> dmproject);

    if (fmt >= 0) {
	if (fmt < 100) return _dmGet_geo_data (dmfile -> dmfp, fmt);
	if (fmt < 200) return _dmGet_cir_data (dmfile -> dmfp, fmt);
	if (fmt < 300) return _dmGet_flp_data (dmfile -> dmfp, fmt);
    }
    return dmGetDataEscape (dmfile, fmt);
}

int _dmGet_geo_data (FILE *fp, int geo_fmt)
{
#ifdef NOPACK
    char    buf1[2], buf2[2];
#endif
    char    str[20];
    long    tmp1;
    int     k;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, geo_fmt: %d\n", (fp ? fileno (fp) : -1), geo_fmt);
#endif

    switch (geo_fmt) {
	case GEO_INFO:
	    k = _dmDoget (fp, "WWWW", &ginfo.bxl, &ginfo.bxr, &ginfo.byb, &ginfo.byt);
	    if (k != 4) goto geo_read_error;
	    break;
	case GEO_INFO2:
	    k = _dmDoget (fp, "WW", &ginfo2.nr_boxes, &ginfo2.nr_groups);
	    if (k != 2) goto geo_read_error;
	    break;
	case GEO_INFO3:
	    k = _dmDoget (fp, "WWWWW", &ginfo3.bxl, &ginfo3.bxr, &ginfo3.byb,
		    &ginfo3.byt, &ginfo3.nr_samples);
	    if (k != 5) goto geo_read_error;
	    break;
	case GEO_BOX:
	    k = _dmDoget (fp, "DDDDDDDDD", &gbox.layer_no,
		    &gbox.xl, &gbox.xr, &gbox.yb, &gbox.yt,
		    &gbox.dx, &gbox.nx, &gbox.dy, &gbox.ny);
	    if (k != 9) goto geo_read_error;
	    gbox.bxl = gbox.xl;
	    gbox.byb = gbox.yb;
	    gbox.bxr = gbox.xr;
	    gbox.byt = gbox.yt;
	    if (gbox.nx) {
		if ((tmp1 = gbox.nx * gbox.dx) < 0)
		    gbox.bxl += tmp1;
		else
		    gbox.bxr += tmp1;
	    }
	    if (gbox.ny) {
		if ((tmp1 = gbox.ny * gbox.dy) < 0)
		    gbox.byb += tmp1;
		else
		    gbox.byt += tmp1;
	    }
	    break;
	case GEO_MC:
	    k = _dmDoget (fp, "SDDDDSDDDDDDDDDDD",
		    gmc.inst_name, &gmc.bxl, &gmc.bxr, &gmc.byb, &gmc.byt,
		    gmc.cell_name, &gmc.imported,
		    &gmc.mtx[0], &gmc.mtx[1], &gmc.mtx[2],
		    &gmc.mtx[3], &gmc.mtx[4], &gmc.mtx[5],
		    &gmc.dx, &gmc.nx, &gmc.dy, &gmc.ny);
	    if (k != 17) goto geo_read_error;
	    break;
	case GEO_LUP:
	    k = _dmDoget (fp, "SD", glup.cell_name, &glup.count);
	    if (k != 2) goto geo_read_error;
	    break;
	case GEO_TERM:
	    k = _dmDoget (fp, "SDDDDDDDDD",
		    gterm.term_name, &gterm.layer_no,
		    &gterm.xl, &gterm.xr, &gterm.yb, &gterm.yt,
		    &gterm.dx, &gterm.nx, &gterm.dy, &gterm.ny);
	    if (k != 10) goto geo_read_error;
	    gterm.bxl = gterm.xl;
	    gterm.byb = gterm.yb;
	    gterm.bxr = gterm.xr;
	    gterm.byt = gterm.yt;
	    if (gterm.nx) {
		if ((tmp1 = gterm.nx * gterm.dx) < 0)
		    gterm.bxl += tmp1;
		else
		    gterm.bxr += tmp1;
	    }
	    if (gterm.ny) {
		if ((tmp1 = gterm.ny * gterm.dy) < 0)
		    gterm.byb += tmp1;
		else
		    gterm.byt += tmp1;
	    }
	    break;
	case GEO_NOR_INI:
	    k = _dmDoget (fp, "DDDDDDD", &gnor_ini.layer_no, &gnor_ini.elmt,
		    &gnor_ini.no_xy, &gnor_ini.bxl, &gnor_ini.bxr,
		    &gnor_ini.byb, &gnor_ini.byt);
	    if (k != 7) goto geo_read_error;
#ifdef NOPACK
	    if (getc (fp) == '\n') {
		gnor_ini.r_bxl = gnor_ini.bxl;
		gnor_ini.r_bxr = gnor_ini.bxr;
		gnor_ini.r_byb = gnor_ini.byb;
		gnor_ini.r_byt = gnor_ini.byt;
		gnor_ini.nx = gnor_ini.ny = 0;
		gnor_ini.dx = gnor_ini.dy = 0.0;
		return (k);
	    }
#endif
	    k += _dmDoget (fp, "DDDDFDFD", &gnor_ini.r_bxl,
		    &gnor_ini.r_bxr, &gnor_ini.r_byb, &gnor_ini.r_byt,
		    &gnor_ini.dx, &gnor_ini.nx, &gnor_ini.dy, &gnor_ini.ny);
	    if (k != 15) goto geo_read_error;
	    break;
	case GEO_NOR_XY:
	    k = _dmDoget (fp, "FF", &gnor_xy.x, &gnor_xy.y);
	    if (k != 2) goto geo_read_error;
	    break;
	case GEO_BOXLAY:
	    k = _dmDoget (fp, "DDDDD", &gboxlay.xl, &gboxlay.xr,
		    &gboxlay.yb, &gboxlay.yt, &gboxlay.chk_type);
	    if (k != 5) goto geo_read_error;
	    break;
	case GEO_TERMLAY:
	    k = _dmDoget (fp, "DDDDD", &gtermlay.xl, &gtermlay.xr,
		    &gtermlay.yb, &gtermlay.yt, &gtermlay.term_number);
	    if (k != 5) goto geo_read_error;
	    break;
	case GEO_NXX_INI:
	    k = _dmDoget (fp, "DD", &gnxx_ini.elmt, &gnxx_ini.no_xy);
	    if (k != 2) goto geo_read_error;

	    if (gnxx_ini.elmt == CIRCLE_NOR) {
	    /* this element is a circle: the 6 other members of gnxx_ini
	    */
	    /* should contain the original circle description */
		k += _dmDoget (fp, "FFFFFF", &gnxx_ini.xc, &gnxx_ini.yc,
			&gnxx_ini.r1, &gnxx_ini.r2,
			&gnxx_ini.a1, &gnxx_ini.a2);
		if (k != 8) goto geo_read_error;

	    }
	    break;
	case GEO_NXX_XY:
	    k = _dmDoget (fp, "FF", &gnxx_xy.x, &gnxx_xy.y);
	    if (k != 2) goto geo_read_error;
	    break;
	case GEO_VLNLAY:
#ifndef NOPACK
	    k = _dmDoget (fp, "DCDDCDD", &gvlnlay.x,
		    &gvlnlay.occ_type, &gvlnlay.yb, &gvlnlay.yt,
		    &gvlnlay.con_type, &gvlnlay.grp_number, &gvlnlay.chk_type);
#else
	    k = _dmDoget (fp, "DSDDSDD", &gvlnlay.x, buf1,
		    &gvlnlay.yb, &gvlnlay.yt, buf2,
		    &gvlnlay.grp_number, &gvlnlay.chk_type);
	    gvlnlay.occ_type = buf1[0];
	    gvlnlay.con_type = buf2[0];
#endif
	    if (k != 7) goto geo_read_error;
	    break;
	case GEO_SPEC:
	    k = _dmDoget (fp, "SDDDDS", gspec.layer,
		    &gspec.xl, &gspec.xr, &gspec.yb, &gspec.yt, gspec.name);
	    if (k != 6) goto geo_read_error;
	    break;
	case GEO_TEQ:
	    k = _dmDoget (fp, "DD", &gteq.term_number, &gteq.grp_number);
	    if (k != 2) goto geo_read_error;
	    break;
	case GEO_TID:
	    k = _dmDoget (fp, "D", &gtid.term_offset);
	    if (k != 1) goto geo_read_error;
	    if (gtid.term_offset == -1) {
		k += _dmDoget (fp, "SSDD", gtid.cell_name, gtid.inst_name,
			&gtid.m_nx, &gtid.m_ny);
		if (k != 5) goto geo_read_error;
	    }
	    else {
		k += _dmDoget (fp, "SDD", gtid.term_name, &gtid.t_nx, &gtid.t_ny);
		if (k != 4) goto geo_read_error;
	    }
	    break;
	case GEO_FLOC:
	    k = _dmDoget (fp, "DDDDDDD", &gfloc.number, &gfloc.type,
		    &gfloc.xl, &gfloc.xr, &gfloc.yb, &gfloc.yt,
		    &gfloc.detect_time);
	    if (k != 7) goto geo_read_error;
	    break;
	case GEO_GLN:

#ifdef NOPACK
	/* input format: "<xl> <yl> <dx> [<dy>]\n" */
	    ggln.yr = 0;
	    k = _dmDoget (fp, "DDD", &ggln.xl, &ggln.yl, &ggln.xr);
	    if (k == 3) {
		while ((c = getc (fp)) == ' ' || c == '\t' || c == '\r');
		if (c == '\n' || c == EOF)
		    k++;
		else {
		    ungetc (c, fp);
		    k = _dmDoget (fp, "D", &ggln.yr);
		    if (k == 1) k = 4;
		}
	    }
#else
	/* input format: "<xl> <yl> <dx> <dy>\n" */
	    k = _dmDoget (fp, "DDDD", &ggln.xl, &ggln.yl, &ggln.xr, &ggln.yr);
#endif
	    ggln.xr += ggln.xl, ggln.yr += ggln.yl;

	    if (k != 4) goto geo_read_error;
	    break;

        case GEO_ANNOTATE:
            k = fscanf (fp, "%d", &ganno.type);
            if (k != 1) goto geo_read_error;
            switch (ganno.type) {
                case GA_FORMAT:
                    k = fscanf (fp, "%ld %ld\n", &ganno.o.format.fmajor, &ganno.o.format.fminor);
                    if (k != 2) goto geo_read_error;
                    break;

                case GA_LINE:
                    k = fscanf (fp, "%lg %lg %lg %lg %d\n",
                        &ganno.o.line.x1, &ganno.o.line.y1,
                        &ganno.o.line.x2, &ganno.o.line.y2,
                        &ganno.o.line.mode);
                    if (k != 5) goto geo_read_error;
                    break;

               case GA_TEXT:
                /* Need ~-quoted strings, because they can contain spaces. */
                    k = fscanf (fp, "%lg %lg %lg %lg ~%[^~]~\n",
                        &ganno.o.text.x, &ganno.o.text.y,
                        &ganno.o.text.ax, &ganno.o.text.ay,
                         ganno.o.text.text);
                    if (k != 5) goto geo_read_error;
                    break;

                case GA_LABEL:
                    *ganno.o.Label.Attributes = '\0';
                    k = fscanf (fp, "%s %s %d %lg %lg %lg %lg %s\n",
                        ganno.o.Label.name,
                        ganno.o.Label.Class,
                        &ganno.o.Label.maskno,
                        &ganno.o.Label.x, &ganno.o.Label.y,
                        &ganno.o.Label.ax, &ganno.o.Label.ay,
                        ganno.o.Label.Attributes);
                    if (k != 7 && k != 8) goto geo_read_error;
                    /* handle empty-string special cases
                     */
                    if (*ganno.o.Label.name == '~') *ganno.o.Label.name = '\0';
                    if (*ganno.o.Label.Class == '~') *ganno.o.Label.Class = '\0';
                    if (*ganno.o.Label.Attributes == '~') *ganno.o.Label.Attributes = '\0';
                    break;
            }
            break;

	default:
	    dmerrno = DME_FMT;
	    dmError ("dmGetDesignData");
	    return (-1);
    }
    return (k);

geo_read_error:
    if (k != EOF) {
	dmerrno = DME_GET;
	_dmSprintf (str, "geo_fmt = %d", geo_fmt);
	dmError (str);
	return (-1);
    }
    return (0);
}

static char attribute_string[256];

int _dmGet_cir_data (FILE *fp, int cir_fmt)
{
    char    str[20];
    int     k = 0;
    long    i, a, b;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, cir_fmt: %d\n", (fp ? fileno (fp) : -1), cir_fmt);
#endif

    switch (cir_fmt) {
	case CIR_MC:
	    k = _dmDoget (fp, "SDSAD", cmc.cell_name, &cmc.imported,
		    cmc.inst_name, attribute_string, &cmc.inst_dim);
	    if (k != 5) goto cir_read_error;
#ifdef NOPACK
	    if (strcmp (cmc.inst_name, "~") == 0) strcpy (cmc.inst_name, "");
#endif
	    if (dm_get_do_not_alloc) {
		strcpy (cmc.inst_attribute, attribute_string);
	    }
	    else {
		cmc.inst_attribute = _dmStrSave (attribute_string);
		if (cmc.inst_dim) {
		    cmc.inst_lower = (long *) calloc ((size_t) cmc.inst_dim, sizeof (long));
		    cmc.inst_upper = (long *) calloc ((size_t) cmc.inst_dim, sizeof (long));
		}
		else {
		    cmc.inst_lower = 0;
		    cmc.inst_upper = 0;
		}
	    }
	    for (i = 0; i < cmc.inst_dim; i++) {
		k = _dmDoget (fp, "DD", &cmc.inst_lower[i], &cmc.inst_upper[i]);
		if (k != 2) goto cir_read_error;
	    }
	    break;
	case CIR_TERM:
	    k = _dmDoget (fp, "SAD", cterm.term_name, attribute_string, &cterm.term_dim);
	    if (k != 3) goto cir_read_error;
	    if (dm_get_do_not_alloc) {
		strcpy (cterm.term_attribute, attribute_string);
	    }
	    else {
		cterm.term_attribute = _dmStrSave (attribute_string);
		if (cterm.term_dim) {
		    cterm.term_lower = (long *) calloc ((size_t) cterm.term_dim, sizeof (long));
		    cterm.term_upper = (long *) calloc ((size_t) cterm.term_dim, sizeof (long));
		}
		else {
		    cterm.term_lower = 0;
		    cterm.term_upper = 0;
		}
	    }
	    for (i = 0; i < cterm.term_dim; i++) {
		k = _dmDoget (fp, "DD", &cterm.term_lower[i], &cterm.term_upper[i]);
		if (k != 2) goto cir_read_error;
	    }
	    break;
	case CIR_NET:
	    k = _dmDoget_net (fp, &cnet, 0);
	    if (k < 0) goto cir_read_error;
	    break;
	case CIR_NET_ATOM:
	    k = _dmDoget_net (fp, &cnet, 1);
	    if (k < 0) goto cir_read_error;
	    break;
	case CIR_NET_HEAD:
	    k = _dmDoget (fp, "DDDDDLDDD",
			&cnethead.cd_nr,   /* long */
			&cnethead.node_nr, /* long */
			&cnethead.node_x,  /* long */
			&cnethead.node_y,  /* long */
			&cnethead.net_neqv,/* long */
			&cnethead.offset,  /* long long */
			&i, &a, &b);
	    if (k < 0) goto cir_read_error;
	    cnethead.lay_nr = i;
	    cnethead.area = a;
	    cnethead.term = b;
	    break;
	case CIR_SLS:
	    csls.sls_dim = (long) fread (csls.sls_buffer, 1, csls.sls_dim, fp);
	    if (csls.sls_dim <= 0) goto cir_read_error;
	    return ((int) csls.sls_dim);
	case CIR_INFO:
	    k = _dmDoget (fp, "WWWW", &cinfo.bxl, &cinfo.bxr, &cinfo.byb, &cinfo.byt);
	    if (k != 4) goto cir_read_error;
	    break;
	case CIR_GRAPHIC:
	    cgraphic.graphic_dim = (long) fread (cgraphic.graphic_buffer, 1, cgraphic.graphic_dim, fp);
	    if (cgraphic.graphic_dim <= 0) goto cir_read_error;
	    return ((int) cgraphic.graphic_dim);
	case CIR_SWIFT:
	    cswift.swift_dim = (long) fread (cswift.swift_buffer, 1, cswift.swift_dim, fp);
	    if (cswift.swift_dim <= 0) goto cir_read_error;
	    return ((int) cswift.swift_dim);
	case CIR_FAULT:
	    k = _dmDoget_fault (fp, &cfault);
	    if (k < 0) goto cir_read_error;
	    break;
	default:
	    dmerrno = DME_FMT;
	    dmError ("dmGetDesignData");
	    return (-1);
    }
    return (k);

cir_read_error:
    if (k != EOF) {
	dmerrno = DME_GET;
	_dmSprintf (str, "cir_fmt = %d", cir_fmt);
	dmError (str);
	return (-1);
    }
    return (0);
}

int _dmDoget_net (FILE *fp, struct cir_net *net, int get_atom)
{
    register long i;
    int k = _dmDoget (fp, "SADDSDD", net -> net_name, attribute_string,
	&net -> net_neqv, &net -> net_dim, net -> inst_name, &net -> inst_dim, &net -> ref_dim);
    if (k != 7) return (-1);
#ifdef NOPACK
    if (strcmp (net -> inst_name, "~") == 0) strcpy (net -> inst_name, "");
#endif
    if (get_atom && dm_get_do_not_alloc) {
	strcpy (net -> net_attribute, attribute_string);
    }
    else {
	net -> net_attribute = _dmStrSave (attribute_string);
	if (net -> net_dim) {
	    net -> net_lower = (long *) calloc ((size_t) net -> net_dim, sizeof (long));
	    net -> net_upper = (long *) calloc ((size_t) net -> net_dim, sizeof (long));
	}
	else {
	    net -> net_lower = 0;
	    net -> net_upper = 0;
	}
	if (net -> inst_dim) {
	    net -> inst_lower = (long *) calloc ((size_t) net -> inst_dim, sizeof (long));
	    net -> inst_upper = (long *) calloc ((size_t) net -> inst_dim, sizeof (long));
	}
	else {
	    net -> inst_lower = 0;
	    net -> inst_upper = 0;
	}
	if (net -> ref_dim) {
	    net -> ref_lower = (long *) calloc ((size_t) net -> ref_dim, sizeof (long));
	    net -> ref_upper = (long *) calloc ((size_t) net -> ref_dim, sizeof (long));
	}
	else {
	    net -> ref_lower = 0;
	    net -> ref_upper = 0;
	}
    }
    for (i = 0; i < net -> net_dim; i++) {
	k = _dmDoget (fp, "DD", &net -> net_lower[i], &net -> net_upper[i]);
	if (k != 2) return (-1);
    }
    for (i = 0; i < net -> inst_dim; i++) {
	k = _dmDoget (fp, "DD", &net -> inst_lower[i], &net -> inst_upper[i]);
	if (k != 2) return (-1);
    }
    for (i = 0; i < net -> ref_dim; i++) {
	k = _dmDoget (fp, "DD", &net -> ref_lower[i], &net -> ref_upper[i]);
	if (k != 2) return (-1);
    }

    net -> net_eqv = 0;

    if (get_atom) return (1);

    if (net -> net_neqv) {
        if (!(net -> net_eqv = (struct cir_net *) calloc ((size_t) net -> net_neqv, sizeof (struct cir_net))))
            _dmFatal ("_dmDoget_net: cannot allocate enough memory for reading nets", "", "");
	for (i = 0; i < net -> net_neqv; ++i) {
	    k = _dmDoget_net (fp, &net -> net_eqv[i], 0);
	    if (k < 0) return (-1);
	}
    }
    return (1);
}

int _dmDoget_fault (FILE *fp, struct cir_fault *fault)
{
    long    i;
    int     k;

    k = _dmDoget (fp, "DSASDDD",
	    &fault -> number, fault -> type,
	    attribute_string, fault -> inst_name,
	    &fault -> inst_dim, &fault -> ref_dim, &fault -> fault_neqv);
    if (k != 7)
	return (-1);
#ifdef NOPACK
    if (strcmp (fault -> inst_name, "~") == 0) strcpy (fault -> inst_name, "");
#endif
    fault -> fault_attribute = _dmStrSave (attribute_string);
    if (fault -> inst_dim) {
	fault -> inst_lower = (long *) calloc ((size_t) fault -> inst_dim, sizeof (long));
	fault -> inst_upper = (long *) calloc ((size_t) fault -> inst_dim, sizeof (long));
    }
    else {
	fault -> inst_lower = 0;
	fault -> inst_upper = 0;
    }
    for (i = 0; i < fault -> inst_dim; i++) {
	k = _dmDoget (fp, "DD", &fault -> inst_lower[i], &fault -> inst_upper[i]);
	if (k != 2)
	    return (-1);
    }
    if (fault -> ref_dim) {
	fault -> ref_lower = (long *) calloc ((size_t) fault -> ref_dim, sizeof (long));
	fault -> ref_upper = (long *) calloc ((size_t) fault -> ref_dim, sizeof (long));
    }
    else {
	fault -> ref_lower = 0;
	fault -> ref_upper = 0;
    }
    for (i = 0; i < fault -> ref_dim; i++) {
	k = _dmDoget (fp, "DD", &fault -> ref_lower[i], &fault -> ref_upper[i]);
	if (k != 2)
	    return (-1);
    }

    if (fault -> fault_neqv) {
	fault -> fault_eqv = (struct cir_fault *) calloc ((size_t) fault -> fault_neqv, sizeof (struct cir_fault));
    }
    else {
	fault -> fault_eqv = 0;
    }
    for (i = 0; i < fault -> fault_neqv; i++) {
	k = _dmDoget_fault (fp, &fault -> fault_eqv[i]);
	if (k < 0)
	    return (-1);
    }

    return (1);
}

int _dmGet_flp_data (FILE *fp, int flp_fmt)
{
    char    str[20];
    int     k;
    long    l, m;
    long    tmp1;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, flp_fmt: %d\n", (fp ? fileno (fp) : -1), flp_fmt);
#endif

/* force linking of dmdata.o */

    switch (flp_fmt) {

	case FLP_MC:
	    k = _dmDoget (fp, "SDDDDSDDDDDDDDDDD",
		    fmc.inst_name, &fmc.bxl, &fmc.bxr, &fmc.byb, &fmc.byt,
		    fmc.cell_name, &fmc.imported,
		    &fmc.mtx[0], &fmc.mtx[1], &fmc.mtx[2],
		    &fmc.mtx[3], &fmc.mtx[4], &fmc.mtx[5],
		    &fmc.dx, &fmc.nx, &fmc.dy, &fmc.ny);
	    if (k != 17)
		goto flp_read_error;
	    break;

	case FLP_INFO:
	    k = _dmDoget (fp, "WWWW", &finfo.bxl, &finfo.bxr,
		    &finfo.byb, &finfo.byt);
	    if (k != 4)
		goto flp_read_error;
	    break;

	case FLP_TERM:
	    k = _dmDoget (fp, "SDADDDDDDDD",
		    fterm.term_name, &fterm.layer_no, attribute_string,
		    &fterm.xl, &fterm.xr, &fterm.yb, &fterm.yt,
		    &fterm.dx, &fterm.nx, &fterm.dy, &fterm.ny);
	    if (k != 11)
		goto flp_read_error;
	    fterm.term_attribute = _dmStrSave (attribute_string);
	    fterm.bxl = fterm.xl;
	    fterm.byb = fterm.yb;
	    fterm.bxr = fterm.xr;
	    fterm.byt = fterm.yt;
	    if (fterm.nx) {
		if ((tmp1 = fterm.nx * fterm.dx) < 0)
		    fterm.bxl += tmp1;
		else
		    fterm.bxr += tmp1;
	    }
	    if (fterm.ny) {
		if ((tmp1 = fterm.ny * fterm.dy) < 0)
		    fterm.byb += tmp1;
		else
		    fterm.byt += tmp1;
	    }
	    break;

	case FLP_CHAN:
	    k = _dmDoget (fp, "SDDDDDDD",
		    fchan.channel_name, &fchan.xl, &fchan.xr, &fchan.yb, &fchan.yt,
		    &fchan.kind, &fchan.order, &fchan.flp_nlist);
	    if (k != 8)
		goto flp_read_error;
	    if (fchan.flp_nlist > 0)
		fchan.flp_netlist = (struct flp_glr *) calloc ((size_t) fchan.flp_nlist, sizeof (struct flp_glr));
	    for (m = 0; m < fchan.flp_nlist; m++) {
	    /* get glr struct */
		k = _dmDoget (fp, "SAD",
			fchan.flp_netlist[m].net_name,
			attribute_string,
			&fchan.flp_netlist[m].flp_nconnect);
		if (k != 3)
		    goto flp_read_error;
		fchan.flp_netlist[m].net_attribute
		    = _dmStrSave (attribute_string);
		if (fchan.flp_netlist[m].flp_nconnect > 0)
		    fchan.flp_netlist[m].flp_netconnect = (struct flp_connect *) calloc
				(fchan.flp_netlist[m].flp_nconnect, sizeof (struct flp_connect));
		for (l = 0; l < fchan.flp_netlist[m].flp_nconnect; l++) {
		/* put connect struct */
		    k = _dmDoget (fp, "SDSDD",
			    fchan.flp_netlist[m].flp_netconnect[l].connect_name,
			    &fchan.flp_netlist[m].flp_netconnect[l].connect_type,
			    fchan.flp_netlist[m].flp_netconnect[l].connect_origin,
			    &fchan.flp_netlist[m].flp_netconnect[l].nx,
			    &fchan.flp_netlist[m].flp_netconnect[l].ny);
		    if (k != 5)
			goto flp_read_error;
		}
	    }
	    break;

	default:
	    dmerrno = DME_FMT;
	    dmError ("dmGetDesignData");
	    return (-1);
    }

    return (k);

flp_read_error:
    if (k != EOF) {
	dmerrno = DME_GET;
	_dmSprintf (str, "flp_fmt = %d", flp_fmt);
	dmError (str);
	return (-1);
    }
    return (0);
}
