/* rcsid = "$Id: proto.h,v 1.1 2018/04/30 12:17:45 simon Exp $" */
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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

/* access.c */
object *mk_object (string name, int type);
object *mk_device (string name, network *netw);
void rm_object (object *ThiS);
void add_object (network *netw, object *ThiS);
object *is_term (network *netw, string name);
object *is_tnet (network *netw, string name);
object *is_net  (network *netw, string name);
object *is_subterm (string name);
void array (object *thing, long first, long last);
void netw_connect (object *dev, list *conn_list, int cnt, int conn_mode);
void do_connect (object *dev);
void merge (network *netw, list *net_list);
string port_name (object *device, long pos);

/* auxilary.c */
string newmem (string str);
void deletemem (string str);
string add_index (string name, long index);
void *s_malloc (unsigned long amount);
void s_free (void *ThiS);
void m_err (int amount);
void astats (void);

/* block.c */
partition *mk_partition (void);
void rm_partition (partition *p);
void rm_block (block *ThiS_block);
void add_block (block *ThiS_block, short mode);
void del_block (block *ThiS_block);
void add_elem (block *b, object *ThiS);
partition *init_partition (network *netw1, network *netw2);
void del_partition (network *netw);
void reconfigure (partition *p);
boolean refine_err (partition *p, stack *st, short mode);
boolean refine (partition *p, stack *st, int mode);
void refine_block (block *ThiS, stack *st, short mode);
long split (block *b, short mode);
void merge_block (block *b);
void propagate (object *obj);
void validate (block *ThiS);
void validate_list (block ** aList);
void touch_b (block *ThiS);
block *cp_block (block *ThiS);
void cmp_devsizes (partition * p, stack * st, int mode);
boolean match_by_name (partition *p);

/* color.c */
void set_termcol (network *netw, list *p_expr);
void color_grp (network *netw);

/* debug.c */
void print_object (FILE *fp, object *ThiS);
void print_ns (FILE *fp, network *netw);
void print_p (FILE *fp, partition *p);
void print_hist (FILE *fp, block *b, short level);
void print_grp_col (FILE *fp, network *netw);

/* error.c */
void err_mesg  (string arg0, ...);
void verb_mesg (string arg0, ...);
void user_mesg (string arg0, ...);
void prnt_mesg (string arg0, ...);
void p_error (int n, string file, int line);
void assert_error (string file, int line, string cond);

/* hash.c */
hash *mk_hash (int hash_size);
void rm_hash (hash *hash_pnt);
void h_link (hash *hash_pnt, string key, void *data);
void *h_unlink (hash *hash_pnt, string key, int rm_key);
void *h_get (hash *hash_pnt, string key);
void p_hash (FILE *fp, hash *hash_pnt);
bucket *mk_a_bucket (void);
void *rm_a_bucket (bucket *ThiS_bucket);

/* init.c */
void my_init (void);
void my_exit (int status);

/* lex_yy.c */
int yylex (void);
void yyrestart (FILE *fp);
int yyback (int *p, int m);
int yyoutput (int c);

/* link.c */
link_type *mk_link (void);
void rm_link (link_type *lnk);
void add_link (object *net, object *dev);
void del_link (link_type *lnk);
void chn_link (link_type *lnk, object *net);
boolean is_link (object *net, object *dev);

/* map.c */
boolean match (network *netw1, network *netw2);
boolean map (partition *p, short mode);

/* mark.c */
void mark (network *netw1, network *netw2);

/* network.c */
void instantiate (network *netw);
void reduce_netw (network *netw);
void expand_netw (network *netw);

/* opt.c */
void ese_options (int argc, char **argv);

/* param.c */
void set_par (object *dev, string par, string value);
void set_parpair (object *dev, char *attr, char *par);
int  get_ival (char *attr, char *par, long *ip);
string get_par (object *dev, string name);

/* prime.c */
void rewind_gen (void);
long gen_prime (void);

/* print.c */
void print_netw (FILE *fp, network *netw);
void p_part_s (FILE *fp, partition *p);
void p_netw_s (FILE *fp, network *netw);
void print_bindings (FILE *fp, partition *p);
void print_block (FILE *fp, block *ThiS, long cnt, int new_block);

/* queue.c */
void clear (queue *ThiS_queue);
boolean contains (queue *ThiS_queue, void *data);
queue *mk_a_queue (void);
void add_queue (queue *ThiS_queue, void *data, short mode);
void *del_queue (queue *ThiS_queue, short mode);
void p_queue (FILE *fp, queue *ThiS_queue);

/* read.c */
network *mk_network (string name, int type);
void rm_network (network *netw);
void read_sls (string path, hash *hashlist, boolean db_flag);
void register_prim (string name);
int  find_prim_index (string name);

/* stack.c */
stack *mk_a_stack (void);
void *pop (stack *ThiS_stack);
void push (stack *ThiS_stack, void *data);

/* symbol.c */
string mk_symbol (string str);
void rm_symbol (string str);

/* utils.c */
void time_progress (string line, string name);

/* yacc.y */
void yyerror (char *s);
pair *mk_pair (void);
void rm_pair (pair *ThiS_pair);
list *add_list (list *prev, void *item);
list *mk_list (void);
void rm_list (list *ThiS);
list *invert_list (list *ThiS_list);
void make_index (string name, pair *a_list, string buf);
boolean incr_index (pair *a_list);
int yyparse (void);

