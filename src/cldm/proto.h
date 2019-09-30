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

/* files.c */
void open_files (void);
void close_files (void);

/* func.c */
char *fromftoa (double d);
char *fromitoa (int i);
long roundh (double d);
long roundL (double d);

/* ini_mtree.c */
void ini_modtree (void);

/* main.c */
void sig_handler (int sig);

/* make_lup.c */

/* man_tree.c */
int append_tree (char *name, struct name_tree **head);
int check_tree (char *name, struct name_tree *ptr);
void rm_tree (struct name_tree *ptr);
void print_tree (char *s, struct name_tree *ptr);

/* pr_exit.c */
void pr_exit (int mode, int nr, char *cs);

/* proc_box.c */
void proc_box (int xl, int xr, int yb, int yt);

/* proc_circ.c */
void proc_circ (int ixc, int iyc, int ir1, int ir2, int ia1, int ia2, int n_edges);

/* proc_cont.c */
void proc_cont (int dir, int width);

/* proc_mc.c */
void mc_warning (void);
void proc_mc (int inst, int mir, int rot);
void proc_cif_mc (int inst, int x0, int x1, int x2, int y0, int y1, int y2);

/* proc_poly.c */
void proc_poly (void);
int test (double x0, double y0, double x1, double y1, double numx,
             double denomx, double numy, double denomy);

/* proc_sbox.c */
void proc_sbox (int ix1, int iy1, int ix2, int iy2);

/* proc_swire.c */
void proc_swire (void);

/* proc_term.c */
void proc_term (int xl, int xr, int yb, int yt);

/* proc_label.c */
void proc_label (int x, int y);

/* proc_wire.c */
void proc_wire (void);

/* write_info.c */
void write_info (void);

/* ldm_parse.y */
int yyparse (void);
void yyerror (char *cs);

/* ldm_lex.l */
int yylex (void);
void read_to_nl (void);
char *textval (void);

/* cif_parse.y */
void yyerror (char *cs);
void doStart (int s, int a, int b);
void doUserStart (char *s);
void doBox (int l, int w, int a, int b, int x, int y);
void doCifCall (int s);
void ABox (int Length, int Width, int XPos, int YPos, int XDirection, int YDirection);
void TTranslate (int XPos, int YPos);
void TMY (void);
void TMX (void);
void TRotate (int XDirection, int YDirection);
void TIdentity (void);
void MapTrans (void);
void deletecell (int s);
void dumpT (char *s);
void doUserTerm (void);
void doUserTerm2 (int w, int h);
void doUserLabel (void);
char *find_alias (char *name);
void store_alias (char *name, char *alias);

