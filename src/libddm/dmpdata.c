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
#define _dmDoput _dmPack
#endif

int dmPutDesignData (DM_STREAM *dmfile, int fmt)
{
    _dmSetReleaseProperties (dmfile -> dmkey -> dmproject);

    if (fmt >= 0) {
	if (fmt < 100) return _dmPut_geo_data (dmfile -> dmfp, fmt);
	if (fmt < 200) return _dmPut_cir_data (dmfile -> dmfp, fmt);
	if (fmt < 300) return _dmPut_flp_data (dmfile -> dmfp, fmt);
    }
    return dmPutDataEscape (dmfile, fmt);
}

int _dmPut_geo_data (FILE *fp, int geo_fmt)
{
    char    str[20], *s;
    int     k;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, geo_fmt: %d\n", (fp ? fileno (fp) : -1), geo_fmt);
#endif

    switch (geo_fmt) {
	case GEO_INFO:
	    k = _dmDoput (fp, "WWWW\n", ginfo.bxl, ginfo.bxr, ginfo.byb,
		    ginfo.byt);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_INFO2:
	    k = _dmDoput (fp, "WW\n", ginfo2.nr_boxes, ginfo2.nr_groups);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_INFO3:
	    k = _dmDoput (fp, "WWWWW\n", ginfo3.bxl, ginfo3.bxr, ginfo3.byb,
		    ginfo3.byt, ginfo3.nr_samples);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_BOX:
	    if (gbox.nx == 0) gbox.dx = 0;
	    if (gbox.ny == 0) gbox.dy = 0;
	    k = _dmDoput (fp, "D D D D D D D D D\n",
		    gbox.layer_no,
		    gbox.xl, gbox.xr, gbox.yb, gbox.yt,
		    gbox.dx, gbox.nx, gbox.dy, gbox.ny);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_MC:
	    if (gmc.nx == 0) gmc.dx = 0;
	    if (gmc.ny == 0) gmc.dy = 0;
	    k = _dmDoput (fp, "S D D D D S D D D D D D D D D D D\n",
		    gmc.inst_name, gmc.bxl, gmc.bxr, gmc.byb, gmc.byt,
		    gmc.cell_name, gmc.imported,
		    gmc.mtx[0], gmc.mtx[1], gmc.mtx[2],
		    gmc.mtx[3], gmc.mtx[4], gmc.mtx[5],
		    gmc.dx, gmc.nx, gmc.dy, gmc.ny);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_LUP:
	    k = _dmDoput (fp, "S D\n", glup.cell_name, glup.count);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_TERM:
	    if (gterm.nx == 0) gterm.dx = 0;
	    if (gterm.ny == 0) gterm.dy = 0;
	    k = _dmDoput (fp, "S D D D D D D D D D\n",
		    gterm.term_name, gterm.layer_no,
		    gterm.xl, gterm.xr, gterm.yb, gterm.yt,
		    gterm.dx, gterm.nx, gterm.dy, gterm.ny);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_NOR_INI:
	    k = _dmDoput (fp, "D D D D D D D", gnor_ini.layer_no, gnor_ini.elmt,
		    gnor_ini.no_xy, gnor_ini.bxl, gnor_ini.bxr, gnor_ini.byb,
		    gnor_ini.byt);
	    if (k < 0) goto geo_write_error;
#ifdef NOPACK
	    if (gnor_ini.nx == 0 && gnor_ini.ny == 0) {
		k = _dmDoput (fp, "\n");
		if (k < 0) goto geo_write_error;
		break;
	    }
#endif
	    k = _dmDoput (fp, " D D D D F D F D\n", gnor_ini.r_bxl,
		    gnor_ini.r_bxr, gnor_ini.r_byb, gnor_ini.r_byt, gnor_ini.dx,
		    gnor_ini.nx, gnor_ini.dy, gnor_ini.ny);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_NOR_XY:
	    k = _dmDoput (fp, "F F\n", gnor_xy.x, gnor_xy.y);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_BOXLAY:
	    k = _dmDoput (fp, "D D D D D\n", gboxlay.xl, gboxlay.xr,
		    gboxlay.yb, gboxlay.yt, gboxlay.chk_type);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_TERMLAY:
	    k = _dmDoput (fp, "D D D D D\n", gtermlay.xl, gtermlay.xr,
		    gtermlay.yb, gtermlay.yt, gtermlay.term_number);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_NXX_INI:
	    k = _dmDoput (fp, "D D", gnxx_ini.elmt, gnxx_ini.no_xy);
	    if (k < 0) goto geo_write_error;

	/* The 6 other members of gnxx_ini only have */
	/* a meaning if this element is a circle.    */
	    if (gnxx_ini.elmt == CIRCLE_NOR) {

		k = _dmDoput (fp, " F F F F F F\n",
			gnxx_ini.xc, gnxx_ini.yc,
			gnxx_ini.r1, gnxx_ini.r2,
			gnxx_ini.a1, gnxx_ini.a2);
		if (k < 0) goto geo_write_error;
	    }
#ifdef NOPACK
	    else {
		k = _dmDoput (fp, "\n");
		if (k < 0) goto geo_write_error;
	    }
#endif
	    break;
	case GEO_NXX_XY:
	    k = _dmDoput (fp, "F F\n", gnxx_xy.x, gnxx_xy.y);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_VLNLAY:
	    k = _dmDoput (fp, "D C D D C D D\n", gvlnlay.x, gvlnlay.occ_type,
		    gvlnlay.yb, gvlnlay.yt, gvlnlay.con_type, gvlnlay.grp_number,
		    gvlnlay.chk_type);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_SPEC:
	    k = _dmDoput (fp, "S D D D D S\n", gspec.layer,
		    gspec.xl, gspec.xr, gspec.yb, gspec.yt,
		    gspec.name);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_TEQ:
	    k = _dmDoput (fp, "D D\n", gteq.term_number, gteq.grp_number);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_TID:
	    k = _dmDoput (fp, "D ", gtid.term_offset);
	    if (k < 0) goto geo_write_error;
	    if (gtid.term_offset == -1) {
		k = _dmDoput (fp, "S S D D\n",
			gtid.cell_name, gtid.inst_name, gtid.m_nx, gtid.m_ny);
	    }
	    else {
		k = _dmDoput (fp, "S D D\n",
			gtid.term_name, gtid.t_nx, gtid.t_ny);
	    }
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_FLOC:
	    k = _dmDoput (fp, "D D D D D D D\n",
		    gfloc.number, gfloc.type,
		    gfloc.xl, gfloc.xr, gfloc.yb, gfloc.yt,
		    gfloc.detect_time);
	    if (k < 0) goto geo_write_error;
	    break;
	case GEO_GLN:
#ifdef NOPACK
	    if (ggln.yl == ggln.yr)
		k = _dmDoput (fp, "D D D\n",
			ggln.xl, ggln.yl, ggln.xr - ggln.xl);
	    else
		k = _dmDoput (fp, "D D D D\n",
			ggln.xl, ggln.yl, ggln.xr - ggln.xl, ggln.yr - ggln.yl);
#else
	    k = _dmDoput (fp, "D D D D\n",
		    ggln.xl, ggln.yl, ggln.xr - ggln.xl, ggln.yr - ggln.yl);
#endif
	    if (k < 0) goto geo_write_error;
	    break;

        case GEO_ANNOTATE:
            k = fprintf (fp, "%d ", ganno.type);
            if (k < 0) goto geo_write_error;

            switch (ganno.type) {
                case GA_FORMAT:
                    k = fprintf (fp, "%ld %ld\n",
                        ganno.o.format.fmajor, ganno.o.format.fminor);
                    if (k < 0) goto geo_write_error;
                    break;

                case GA_LINE:
                    k = fprintf (fp, "%.8g %.8g %.8g %.8g %d\n",
                        ganno.o.line.x1, ganno.o.line.y1,
                        ganno.o.line.x2, ganno.o.line.y2,
                        ganno.o.line.mode);
                    if (k < 0) goto geo_write_error;
                    break;

                case GA_TEXT:
                /* Need ~-quoted strings, because they can contain spaces. */
                    k = fprintf (fp, "%.8g %.8g %.8g %.8g ~%s~\n",
                        ganno.o.text.x, ganno.o.text.y,
                        ganno.o.text.ax, ganno.o.text.ay,
                        ganno.o.text.text);
                    if (k < 0) goto geo_write_error;
                    break;

                case GA_LABEL:
                    /* handle empty-string special cases
                     */
		    for (s = ganno.o.Label.name; *s; ++s) {
			if (*s <= ' ' || *s > 126) *s = '_';
		    }
                    k = fprintf (fp, "%s %s %d %.8g %.8g %.8g %.8g %s\n",
                        (*ganno.o.Label.name != '\0' ? ganno.o.Label.name : "~"),
                        (*ganno.o.Label.Class != '\0' ? ganno.o.Label.Class : "~"),
                        ganno.o.Label.maskno,
                        ganno.o.Label.x, ganno.o.Label.y,
                        ganno.o.Label.ax, ganno.o.Label.ay,
                        (*ganno.o.Label.Attributes != '\0' ? ganno.o.Label.Attributes : "~"));
                    if (k < 0) goto geo_write_error;
                    break;
            }
            break;

	default:
	    dmerrno = DME_FMT;
	    dmError ("dmPutDesignData");
	    return (-1);
    }
    return (0);

geo_write_error:
    dmerrno = DME_PUT;
    _dmSprintf (str, "geo_fmt = %d", geo_fmt);
    dmError (str);
    return (-1);
}

int _dmPut_cir_data (FILE *fp, int cir_fmt)
{
    char    str[20];
    int     k;
    long    i;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, cir_fmt: %d\n", (fp ? fileno (fp) : -1), cir_fmt);
#endif

    switch (cir_fmt) {
	case CIR_MC:
	    k = _dmDoput (fp, "S D S A D", cmc.cell_name, cmc.imported,
		    cmc.inst_name, cmc.inst_attribute, cmc.inst_dim);
	    if (k < 0) goto cir_write_error;
	    for (i = 0; i < cmc.inst_dim; i++) {
		k = _dmDoput (fp, " D D", cmc.inst_lower[i], cmc.inst_upper[i]);
		if (k < 0) goto cir_write_error;
	    }
#ifdef NOPACK
	    k = _dmDoput (fp, "\n");
	    if (k < 0) goto cir_write_error;
#endif
	    break;
	case CIR_TERM:
	    k = _dmDoput (fp, "S A D", cterm.term_name,
		    cterm.term_attribute, cterm.term_dim);
	    if (k < 0) goto cir_write_error;
	    for (i = 0; i < cterm.term_dim; i++) {
		k = _dmDoput (fp, " D D", cterm.term_lower[i],
			cterm.term_upper[i]);
		if (k < 0) goto cir_write_error;
	    }
#ifdef NOPACK
	    k = _dmDoput (fp, "\n");
	    if (k < 0) goto cir_write_error;
#endif
	    break;
	case CIR_NET:
	    k = _dmDoput_net (fp, &cnet, 0);
	    if (k < 0) goto cir_write_error;
	    break;
	case CIR_NET_ATOM:
	    k = _dmDoput_net (fp, &cnet, 1);
	    if (k < 0) goto cir_write_error;
	    break;
	case CIR_NET_HEAD:
	    k = _dmDoput (fp, "DDDDDLDDD",
			cnethead.cd_nr,   /* long */
			cnethead.node_nr, /* long */
			cnethead.node_x,  /* long */
			cnethead.node_y,  /* long */
			cnethead.net_neqv,/* long */
			cnethead.offset,  /* long long */
			(long)cnethead.lay_nr,
			(long)cnethead.area,
			(long)cnethead.term);
	    if (k < 0) goto cir_write_error;
	    break;
	case CIR_SLS:
	    csls.sls_dim = (long) fwrite (csls.sls_buffer, 1, csls.sls_dim, fp);
	    if (csls.sls_dim <= 0) goto cir_write_error;
	    break;
	case CIR_INFO:
	    k = _dmDoput (fp, "WWWW\n", cinfo.bxl, cinfo.bxr, cinfo.byb, cinfo.byt);
	    if (k < 0) goto cir_write_error;
	    break;
	case CIR_GRAPHIC:
	    cgraphic.graphic_dim = (long) fwrite (cgraphic.graphic_buffer, 1, cgraphic.graphic_dim, fp);
	    if (cgraphic.graphic_dim <= 0) goto cir_write_error;
	    break;
	case CIR_SWIFT:
	    cswift.swift_dim = (long) fwrite (cswift.swift_buffer, 1, cswift.swift_dim, fp);
	    if (cswift.swift_dim <= 0) goto cir_write_error;
	    break;
	case CIR_FAULT:
	    k = _dmDoput_fault (fp, &cfault);
	    if (k < 0) goto cir_write_error;
	    break;
	default:
	    dmerrno = DME_FMT;
	    dmError ("dmPutDesignData");
	    return (-1);
    }
    return (0);

cir_write_error:
    dmerrno = DME_PUT;
    _dmSprintf (str, "cir_fmt = %d", cir_fmt);
    dmError (str);
    return (-1);
}

int _dmDoput_net (FILE *fp, struct cir_net *net, int put_atom)
{
    long    i;
    int     k;

    k = _dmDoput (fp, "S A D D S D D", net -> net_name,
	    net -> net_attribute, net -> net_neqv,
	    net -> net_dim, net -> inst_name,
	    net -> inst_dim, net -> ref_dim);
    if (k < 0) return (k);
    for (i = 0; i < net -> net_dim; i++) {
	k = _dmDoput (fp, " D D", net -> net_lower[i], net -> net_upper[i]);
	if (k < 0) return (k);
    }
    for (i = 0; i < net -> inst_dim; i++) {
	k = _dmDoput (fp, " D D", net -> inst_lower[i], net -> inst_upper[i]);
	if (k < 0) return (k);
    }
    for (i = 0; i < net -> ref_dim; i++) {
	k = _dmDoput (fp, " D D", net -> ref_lower[i], net -> ref_upper[i]);
	if (k < 0) return (k);
    }
#ifdef NOPACK
    k = _dmDoput (fp, "\n");
    if (k < 0) return (k);
#endif

    if (!put_atom) {
	for (i = 0; i < net -> net_neqv; i++) {
	    k = _dmDoput_net (fp, &net -> net_eqv[i], put_atom);
	    if (k < 0) return (k);
	}
    }
    return (0);
}

int _dmDoput_fault (FILE *fp, struct cir_fault *fault)
{
    long    i;
    int     k;
    k = _dmDoput (fp, "D S A S D D D", fault -> number, fault -> type,
	    fault -> fault_attribute, fault -> inst_name,
	    fault -> inst_dim, fault -> ref_dim, fault -> fault_neqv);

    if (k < 0) return (k);
    for (i = 0; i < fault -> inst_dim; i++) {
	k = _dmDoput (fp, " D D", fault -> inst_lower[i], fault -> inst_upper[i]);
	if (k < 0) return (k);
    }
    for (i = 0; i < fault -> ref_dim; i++) {
	k = _dmDoput (fp, " D D", fault -> ref_lower[i], fault -> ref_upper[i]);
	if (k < 0) return (k);
    }
#ifdef NOPACK
    k = _dmDoput (fp, "\n");
    if (k < 0) return (k);
#endif

    for (i = 0; i < fault -> fault_neqv; i++) {
	k = _dmDoput_fault (fp, &fault -> fault_eqv[i]);
	if (k < 0) return (k);
    }

    return (0);
}

int _dmPut_flp_data (FILE *fp, int flp_fmt)
{
    char    str[20];
    int     k;
    long    l, m;

#ifdef DM_DEBUG
    IFDEBUG fprintf (stderr, "fildes: %d, flp_fmt: %d\n", (fp ? fileno (fp) : -1), flp_fmt);
#endif

/* force linking of dmdata.o */

    switch (flp_fmt) {
	case FLP_MC:
	    if (fmc.nx == 0) fmc.dx = 0;
	    if (fmc.ny == 0) fmc.dy = 0;
	    k = _dmDoput (fp, "S D D D D S D D D D D D D D D D D\n",
		    fmc.inst_name, fmc.bxl, fmc.bxr, fmc.byb, fmc.byt,
		    fmc.cell_name, fmc.imported,
		    fmc.mtx[0], fmc.mtx[1], fmc.mtx[2],
		    fmc.mtx[3], fmc.mtx[4], fmc.mtx[5],
		    fmc.dx, fmc.nx, fmc.dy, fmc.ny);
	    if (k < 0) goto flp_write_error;
	    break;

	case FLP_INFO:
	    k = _dmDoput (fp, "WWWW\n", finfo.bxl, finfo.bxr, finfo.byb, finfo.byt);
	    if (k < 0) goto flp_write_error;
	    break;

	case FLP_TERM:
	    if (fterm.nx == 0) fterm.dx = 0;
	    if (fterm.ny == 0) fterm.dy = 0;
	    k = _dmDoput (fp, "S D A D D D D D D D D",
		    fterm.term_name, fterm.layer_no,
		    fterm.term_attribute,
		    fterm.xl, fterm.xr, fterm.yb, fterm.yt,
		    fterm.dx, fterm.nx, fterm.dy, fterm.ny);
	    if (k < 0) goto flp_write_error;
	    break;

	case FLP_CHAN:
	    k = _dmDoput (fp, "S D D D D D D D\n",
		    fchan.channel_name, fchan.xl, fchan.xr, fchan.yb, fchan.yt,
		    fchan.kind, fchan.order, fchan.flp_nlist);
	    if (k < 0) goto flp_write_error;
	    for (m = 0; m < fchan.flp_nlist; m++) {
	    /* put glr struct */
		k = _dmDoput (fp, "S A D\n",
			fchan.flp_netlist[m].net_name,
			fchan.flp_netlist[m].net_attribute,
			fchan.flp_netlist[m].flp_nconnect);
		if (k < 0) goto flp_write_error;
		for (l = 0; l < fchan.flp_netlist[m].flp_nconnect; l++) {
		/* put connect struct */
		    k = _dmDoput (fp, "S D S D D\n",
			    fchan.flp_netlist[m].flp_netconnect[l].connect_name,
			    fchan.flp_netlist[m].flp_netconnect[l].connect_type,
			    fchan.flp_netlist[m].flp_netconnect[l].connect_origin,
			    fchan.flp_netlist[m].flp_netconnect[l].nx,
			    fchan.flp_netlist[m].flp_netconnect[l].ny);
		    if (k < 0) goto flp_write_error;
		}
	    }
	    break;

	default:
	    dmerrno = DME_FMT;
	    dmError ("dmPutDesignData");
	    return (-1);
    }
    return (0);

flp_write_error:
    dmerrno = DME_PUT;
    _dmSprintf (str, "flp_fmt = %d", flp_fmt);
    dmError (str);
    return (-1);
}
