/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "src/sls_exp/search.h"
#include "src/sls_exp/gndefine.h"
#include "src/sls_exp/define.h"
#include "src/libddm/dmincl.h"

#ifndef DM_MAXPATHLEN
#define DM_MAXPATHLEN   1024
#endif
#include "src/sls_exp/gntype.h"
#include "src/sls_exp/type.h"

extern FUNCVAR * FV;
extern FUNCDESCR * FD;
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

extern int FV_size;
extern int FD_size;
extern int ST_size;
extern int NT_size;
extern int MT_size;
extern int CTT_size;
extern int MCT_size;
extern int XT_size;
extern int XX_size;
extern int N_size;
extern int DS_size;
extern int C_size;
extern int T_size;
extern int F_size;
extern int I_size;
extern int FI_size;
extern int FR_size;
extern int FO_size;
extern int FS_size;

extern int mcs_NT_cnt;
extern int mcs_T_cnt;
extern int mcs_F_cnt;
extern int mcs_I_cnt;

extern FILE * debug;

extern char **globNets;
extern int *globNetsNx;
extern int globNets_cnt;

extern char * argv0;

extern DM_PROJECT *dmproject;

extern int debugdata;
extern int debugmem;
extern int monitoring;

extern int force_exp;
extern int silent;
extern int rm_noterm;
extern int capcoupling;
extern int cirflag;
extern char viewtype[BUFSIZ];

extern char sls_fn[BUFSIZ];
extern char sls_o_fn[BUFSIZ];

extern time_t newest_ftime;
extern time_t time_first_bin;

extern void cannotAlloc (char *fn, int lineno, int nel, int sizel);
extern void dberror (char *fn, int n, char *str1, char *str2);
extern void die (int nr);
extern char *hiername (int);
extern int newname (char *s);
extern int newnode (void);
extern int newxt (void);
extern int newxx (int nbr);
