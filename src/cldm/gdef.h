/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/incl.h"

/*
**  Global declarations of CLDM
*/

/* Flags for various purposes */

struct stat stat_buf;	/* file status info */

char    name_len[4];

DM_PROCDATA *process;   /* ptr to process info */

int	sfx, sfy;	/* model call scaling factors */
int	lay_code = -1;	/* layer code (of process table) */
int     err_cnt  = 0;	/* error counter */
int     err_flag = 0;	/* set when an error, either a syntax or a
			   semantic error, is detected, reset when
			   a me statement is encountered */
int     v_mode = 0;  	/* verbose, if -v option is specified */
int     r_mode = 0;  	/* no rounding messages (-r option) */
int     s_mode = 0;  	/* syntax check only (-s option) */
int     x_mode = -1;	/* origin mode (if on) */
int     h_x_mode = -1;	/* header origin mode flag */
int     f_mode = 0;	/* force cell overwrite (if on) */
int     b_mode = 0;
int     mode45 = 0;	/* test input on 45 degree */
int     t_mode = 0;     /* generate terminal boxes if necessary */
long    t_width = 0;    /* width of the terminal boxes */

int     ini_bbbox = 0;	/* bounding box flags, set when a ms
				   statement */
int     ini_mcbbox = 0;	/* is encountered, reset when the
			   corresponding bounding box gets a value */
int     n_tok  = 0;	/* if(true) name token expected */
int     t_tok  = 0;	/* if(true) term stat processing */
int     w_tok  = 0;	/* if(true) wire stat processing */
int     ms_tok = 0;	/* if(true) ms stat seen */

char    ms_name[DM_MAXNAME+1] = "??";/* name of current modeldef. */
char    mc_name[DM_MAXNAME+1];	/* name of model called */
char    instance[DM_MAXNAME+1];	/* name of model inst. */
char    terminal[DM_MAXNAME+1];	/* name of terminal */
char    label[DM_MAXNAME+1];	/* name of label */

/* Binary sorted name trees */

struct name_tree   *tree_ptr = NULL;
struct name_tree   *mod_tree = NULL;
	/* tree containing names of models defined
	   in the database */
struct name_tree   *tnam_tree = NULL;
	/* tree containing names of terminals
	   defined in current model */
struct name_tree   *inst_tree = NULL;
	/* tree containing names of instances
	   defined in current model */

DM_PROJECT *dmproject = NULL;
DM_CELL *mod_key = NULL;

/* FILE descriptors */

DM_STREAM *fp_box;
DM_STREAM *fp_info;
DM_STREAM *fp_mc;
DM_STREAM *fp_nor;
DM_STREAM *fp_term;
DM_STREAM *fp_anno;

/* Element Parameters: */

char    layer[DM_MAXLAY+1];	/* layer code string */
int     w_dir;			/* current wire direction */
int     w_width, w_x, w_y;	/* width and position */
int	tmp_i;			/* temporary usable integer */
double	tmp_d, d_f;		/* temporary usable double */
double	cifsf;			/* overall scaling used for CIF */

int     int_val[NOINTS];	/* int  number buffer */
int     int_ind = 0;		/* int  number buffer index */
int     dx, nx, dy, ny;         /* repetition  */
long    tx, ty;		        /* translation */
long    bbnd_xl, bbnd_xr;       /* box bounding box */
long    bbnd_yb, bbnd_yt;
long    mcbb_xl, mcbb_xr;       /* model reference bbox */
long    mcbb_yb, mcbb_yt;

long    cifunit = 100;		/* default cif unit */
int     delft_old = 0;		/* using the TU-Delft old 94 user mode */
int     done_user_term = 0;	/* for eof warning to use old 94 */
int     done_user_94 = 0;	/* for eof warning to use old 94 */
