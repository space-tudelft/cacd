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

struct geo_info ginfo;
struct geo_info2    ginfo2;
struct geo_info3    ginfo3;
struct geo_box  gbox;
struct geo_mc   gmc;
struct geo_term gterm;
struct geo_nor_ini  gnor_ini;
struct geo_nor_xy   gnor_xy;
struct geo_boxlay   gboxlay;
struct geo_termlay  gtermlay;
struct geo_vlnlay   gvlnlay;
struct geo_spec gspec;
struct geo_teq  gteq;
struct geo_tid  gtid;
struct geo_lup  glup;
struct geo_nxx_ini  gnxx_ini;
struct geo_nxx_xy   gnxx_xy;
struct geo_floc gfloc;
struct geo_gln ggln;
struct geo_anno ganno;

struct cir_mc   cmc;
struct cir_term cterm;
struct cir_net  cnet;
struct cir_nethead  cnethead;
struct cir_sls  csls;
struct cir_info cinfo;
struct cir_graphic  cgraphic;
struct cir_swift    cswift;
struct cir_fault    cfault;

struct flp_mc    fmc;
struct flp_info   finfo;
struct flp_term  fterm;
struct flp_chan   fchan;

char    icdpath[MAXLINE];

