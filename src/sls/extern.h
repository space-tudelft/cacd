/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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
    CACD-RELEASE-ALLOWED
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "src/sls_exp/gndefine.h"
#include "src/sls/define.h"
#include "src/libddm/dmincl.h"

#ifndef DM_MAXPATHLEN
#define DM_MAXPATHLEN   1024
#endif
#include "src/sls_exp/gntype.h"
#include "src/sls/type.h"

extern char * argv0;

extern FUNCVAR FV[];   /* cannot be *FV instead of FV[] because of GOULD */
extern FUNCDESCR FD[];
extern char * ST;
extern NAMETABLE * NT;
extern MODELTABLE * MT;
extern CONTEXTTABLE * CTT;
extern MODELCALLTABLE * MCT;
extern int * XT;
extern int * XX;
extern NODE * N;
extern int * DS;
extern CONTROL * C;
extern INTERCAP * I;
extern TRANSISTOR * T;
extern FUNCTION * F;
extern int * FI;
extern int * FR;
extern FUNCOUT * FO;
extern char * FS;

extern int FV_cnt;
extern int FD_cnt;
extern int ST_cnt;
extern int NT_cnt;
extern int MT_cnt;
extern int CTT_cnt;
extern int MCT_cnt;
extern int XT_cnt;
extern int XX_cnt;
extern int N_cnt;
extern int DS_cnt;
extern int C_cnt;
extern int T_cnt;
extern int F_cnt;
extern int I_cnt;
extern int FI_cnt;
extern int FR_cnt;
extern int FO_cnt;
extern int FS_cnt;
extern int SIZE_PTR_INT;

extern FILE * debug;

extern char * netwname;
extern char * fn_cmd;
extern char * fn_proc;
extern char * fn_res;
extern char * fn_tst;
extern char * fn_plt;
extern char * fn_out;
extern char * fn_init;

extern int debugdata;
extern int debugsim;
extern int debugmem;
extern int monitoring;
extern int make_output;

extern int delaysim;
extern int outonchange;
extern int proclogic;
extern int printraces;
extern int printdevices;
extern int printstatis;
extern int maxpagewidth;
extern int printremain;
extern int dissip;
extern int random_initialization;
extern int random_td_initialization;
extern int tester_output;
extern int verbose;

extern double mindevtime;
extern double maxdevtime;

extern double sigtimeunit;
extern double outtimeunit;
extern double outtimeaccur;
extern double sigtoint;

extern int vH;
extern double vHtmp;
extern double vswitch;
extern double vminH;
extern double vmaxL;
extern double krise;
extern double kfall;

extern int maxevents;
extern int maxnvicin;
extern int maxtvicin;
extern int logic_depth;
extern int res_cnt;
extern int nodes_cnt;
extern int essnodes_cnt;
extern int inpnodes_cnt;
extern int funcoutpnodes_cnt;
extern int pnodes_cnt;
extern int plotnodes_cnt;
extern int events_cnt;
extern int simstep_cnt;
extern int timepoint_cnt;
extern int act_maxnvicin;
extern int act_maxtvicin;
extern int sigendless;
extern NODE_REF_LIST ** prinvert;

extern simtime_t * tdumps;
extern simtime_t tcurr;
extern simtime_t tbreak;
extern simtime_t tsimduration;
extern simtime_t sig_toffset;
extern simtime_t simperiod;
extern simtime_t disperiod;

extern int stopsim;
extern int stepdelay;

extern int fatalerror;

extern VICINITY vicin;
extern AGGLOMERATION agglom;
extern INTERRUPTMASK intrupm;

extern NODE_REF_LIST * pl_begin;
extern NODE_REF_LIST * pl_end;
extern NODE_REF_LIST * plotl_begin;
extern NODE_REF_LIST * plotl_end;
extern NODE_REF_LIST * disl_begin;
extern NODE_REF_LIST * disl_end;
extern ABSTRACT_OUTPUT * abstractl_begin;
extern ABSTRACT_OUTPUT * abstractl_end;
extern RES_PATH * rp_begin;
extern RES_PATH * rp_end;
extern HISTORY_LIST * ol_begin;
extern HISTORY_LIST * ol_end;

extern int * fsnulls;
extern FUNCTION * currf;

extern void arr_init (char **pa, int elsize, int ispace, double incr);
extern int  arr_new  (char **pa);
extern void arr_reset (char *p);
extern int  arr_size  (char *p);
extern void cannotAlloc (char *fn, int lineno, int nel, int sizel);
extern void die (int nr);
extern char *hiername (int);
extern int lstate (NODE *n);
extern void plot_node (NODE *n, char bound, int v);
extern void resched_event (NODE *n, int eventtype, simtime_t tswitch);
extern void retr_event (NODE *n, int eventtype);
extern void sched_event (NODE *n, int eventtype, simtime_t tswitch);
extern void slserror (char *fn, int lineno, int errtype, char *str1, char *str2);
extern UPAIR *uminmax (NODE *n);
extern STRING_REF *names_from_path (int traillen, PATH_SPEC *path);
extern double doub_conv (double d);

/* src/sls_exp/monit.c */
extern void monitime (char *str);
extern void startmonitime (char *pname);
extern void stopmonitime (void);
