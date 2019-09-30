/*
 * ISC License
 *
 * Copyright (C) 1985-2018 by
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
**  External declarations of CLDM
*/

extern DM_PROCDATA *process;
extern DM_PROJECT  *dmproject;
extern DM_CELL     *mod_key;
extern DM_STREAM   *fp_box;
extern DM_STREAM   *fp_info;
extern DM_STREAM   *fp_mc;
extern DM_STREAM   *fp_nor;
extern DM_STREAM   *fp_term;
extern DM_STREAM   *fp_anno;

extern struct stat stat_buf;
extern struct name_tree   *tree_ptr;
extern struct name_tree   *mod_tree;
extern struct name_tree   *tnam_tree;
extern struct name_tree   *inst_tree;

extern char    *argv0;
extern char    name_len[];
extern char    ms_name[];
extern char    mc_name[];
extern char    instance[];
extern char    terminal[];
extern char    label[];
extern char    layer[];

extern int     sfx, sfy;
extern int     lay_code;
extern size_t  yyleng;
extern int     yylineno;
extern int     err_cnt;
extern int     err_flag;
extern int     v_mode;
extern int     r_mode;
extern int     s_mode;
extern int     x_mode;
extern int     h_x_mode;
extern int     f_mode;
extern int     b_mode;
extern int     mode45;
extern int     t_mode;
extern long    t_width;
extern int     ini_bbbox;
extern int     ini_mcbbox;
extern int     n_tok;
extern int     t_tok;
extern int     w_tok;
extern int     ms_tok;
extern int     w_dir;
extern int     w_width, w_x, w_y;
extern int     int_val[];
extern int     int_ind;
extern int     dx, nx, dy, ny;
extern int     tmp_i;

extern long    tx, ty;
extern long    bbnd_xl, bbnd_xr, bbnd_yb, bbnd_yt;
extern long    mcbb_xl, mcbb_xr, mcbb_yb, mcbb_yt;

extern double  tmp_d, d_f, cifsf;

extern long    cifunit;
extern int     delft_old;
extern int     done_user_term;
extern int     done_user_94;
