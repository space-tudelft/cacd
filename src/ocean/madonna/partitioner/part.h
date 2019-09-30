/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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
#ifndef __PART_H
#define __PART_H

#include "src/ocean/libseadif/sea_decl.h"
#include "src/ocean/madonna/partitioner/genpart.h"

#ifdef __cplusplus
extern LINKAGE_TYPE
{
#endif

/* cost.c */
void init_row_clm_transformation_arrays(TOTALPPTR total);
void initnetstate(NETSTATEPTR netstate,CFUNC *costfunction);
int netstatecost(int, NETSTATEPTR netstate);
int netstatecost2(int, int xl,int xr,int yb,int yt,int deviation);
int gainnetstate(NETSTATEPTR netstate,int to_area,int from_area, CFUNC *costfunction);
void keep_track_of_bbx_after_decr(int *,int *,int from_bar, int *barlist[]);
void keep_track_of_bbx_after_decr_fake(int *,int *,int from_bar, int *barlist[]);
void keep_track_of_bbx_after_incr(int *,int *,int to_bar,int *barlist[]);
void keep_track_of_bbx_after_incr_fake(int *,int *,int to_bar);
NETSTATEPTR domove_and_copy_netstate(NETSTATEPTR oldnetstate,
	int *newstatebuf,int to_area,int from_area, CFUNC *costfunction);
int *startofstatebuf(NETSTATEPTR netstate);
int sizeofstatebuf(NETSTATEPTR netstate);
int sizeofstatebuf2(TOTALPPTR total);
void assignstatebuf(NETSTATEPTR netstate,int *statebuf);

/* dopart.c */
int dopartitioning(TOTALPPTR total);
void movecandidateandupdategains(PARTLISTPTR thecandidate,TOTALPPTR total);
void updategains(PARTCELLPTR thecell,int dstparti,int srcparti,TOTALPPTR total);
void updategainsincellsinnet(NETPTR net,int movdst,int movsrc,TOTALPPTR total);
void rememberthispartitioning(TOTALPPTR total);
void fixnetlist(CIRCUITPTR ntop,CIRCUITPTR otop);
void copycpr(CIRCUITPTR ntop,NETPTR nnet,CIRPORTREFPTR ocpr,int disappears);
int netdisappearsinsinglepartition(NETPTR onet);
int alreadyhavefun(STRING fname,LIBRARYPTR lib);

/* genpart.c */
int clusterPermutate(TOTALPPTR total);
int genpart(TOTALPPTR *totalout,CIRCUITPTR topcell,int nx,int ny, CFUNC *costfunction,int repeat);
int readlayoutofchildren(int what,CIRCUITPTR topcell,int expandtobottom);
int plazareadlayout(int what,CIRCUITPTR c,int expandtobottom);
int prefabmakemembersandcandidates(TOTALPPTR total);
int exitpartitionstructures(TOTALPPTR total);

/* madonna_.c */
void madonna_(TOTALPPTR *total,CIRCUITPTR circuit, int calldepth);
int costquad(int netdistr[],int numparts);
void madonnastat(TOTALPPTR total);
int cost3x3(int netdistr[],int numparts);
int cost4x4(int netdistr[],int numparts);
int cost5x4(int netdistr[],int numparts);
int cost6x4(int netdistr[],int numparts);
int cost8x4(int netdistr[],int numparts);
int cost16x4(int netdistr[],int numparts);
int gencost(int netdistr[],register int total,register int hor);
int initnetcostinfo(void);

/* randomMove.c */
int  makeRandomMove(TOTALPPTR);
void updateTemperature(TOTALPPTR);

/* alarm.c */
void raise_alarm_flag(int signumber);
void enable_reset_alarm_flag(int signumber);
void initsignals(void);

#ifdef __cplusplus
}
#endif

#endif
