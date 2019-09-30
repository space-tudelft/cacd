/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	R. van der Valk
 *	S. de Graaf
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

#include "src/cmsk/extern.h"
#include "signal.h"

struct stat st_buf;
DM_PROJECT *dmproject;
DM_CELL	   *pat_key;
DM_STREAM  *fpbox, *fpinfo, *fpmc, *fpnor;

int makedatabase, makepattern,
    v_mode, wordsindex, int_res,
    mirrorflag, rotateflag, x_origin, y_origin,
    cxnumber, cxdistance, cynumber, cydistance,
    pat_box[4], totcallbox[4], callbox[4],
    totprogbox[4], progbox[4], temp1, temp2,
    start_x, start_y, y_previous,
    callpresent, progpresent,
    nrofmasks, masknr, nroflistmasks,
    mlist[MAXLISTMASKS][2],
    dest_mask, src1_mask, src2_mask,
    scanflag, line_width, dir_flag,
    n_edges, p_incr, cell_no,
   *layset;

double p_x[52], p_y[52], x1, x2, x3, x4, g_y1, y2, y3, y4,
    prev_grow, tmp1, tmp2, lw_tan225, lw_hsqrt2,
    d_f, d_i, resol, p_width, rad45;

char  words[MAXWORDS][DM_MAXNAME+1],
      pat_buffer[DM_MAXNAME+1],
     *pat_name,
    **ldmlay,
    **cmklay;

struct ptree *tree_ptr;
struct list *mlistnames;
struct list *listptr;
struct list *endlistptr;

struct edge *mostleft, *mostright, *edgeptr, *growptr, *fmptr;
struct edge *lastedge, *fr_edge_structs;
struct edge **edges1[3];
struct edge **edges2[3];

struct scan *mostup, *mostdown, *scanptr;
struct scan *lastscan, *fr_scan_structs;

struct la_elmt *first_la_elmt, *last_la_elmt;
