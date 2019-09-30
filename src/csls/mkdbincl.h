/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
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

extern int doSimpleNet;
extern int genIntNet;
extern int externRequired;
extern int forbidFirstCapital;
extern int noWarnings;
extern int sls_errcnt;
extern int sls_errno;

extern int yylineno;
extern char fn_incl[];

extern int int_nbyte, int_maxnbyte;
extern int char_nbyte, char_maxnbyte;
extern int ntwdef_nbyte, ntwdef_maxnbyte;
extern int ntwinst_nbyte, ntwinst_maxnbyte;
extern int inst_struct_nbyte, inst_struct_maxnbyte;
extern int netelem_nbyte, netelem_maxnbyte;
extern int dict_nbyte, dict_maxnbyte;
extern int net_ref_nbyte, net_ref_maxnbyte;
extern int xelem_nbyte, xelem_maxnbyte;
extern int queue_nbyte, queue_maxnbyte;
extern int stack_nbyte, stack_maxnbyte;

# ifdef __cplusplus
  extern "C" {
# endif

/* bifunc.c */
void init_bifs (void);
Network *is_bifunc (char *name);

/* dbaccess.c */
datum *dbfetch (DbAcces *dbacces, datum *key);
int dbdelete (DbAcces *dbacces, datum *key);
int dbstore (DbAcces *dbacces, datum *key, datum *dat);
datum *firstkey (DbAcces *dbacces);
datum *nextkey (DbAcces *dbacces, datum *key);

/* dbopen.c */
DbAcces *dbOpen (char *dbname);
int dbClose (DbAcces **dbacces);

/* dffunc.c */
Network *read_dff (char *name);

/* diction.c */

/* end_ntw.c */
int end_ntw (Network *ntw, int ext, int orig_mode);

/* init_ntw.c */
Network *init_ntw (char *name, int ext);
Network *read_ntw (char *name);
int checkDbTerm (Netelem *term);
void finalCheckDbTerm (void);
int existCell (const char *name, const char *view);

/* lex.l */
int Input (void);
int yylex (void);
int resynch (int c);
char * textval (void);

/* main.c */
void die (void);
double cvt_atof (char *s);
char *isGlobalNet (char *s);
char *isDefGlobalNet (char *s);
void rcReadError (char *fn, const char *s);
void parseLineStmt (char *s);

/* neteqv.c */
int chk_bounds (Stack *xs1, Stack *xs2);
int neteqv (Queue *netq);
int getxslength (Stack *xs);
int getnetcnt (Queue *netq);
int gettermcnt (Network *netw);
int inst_net_eqv (NetworkInstance *inst, Queue *cons);
Netelem *findterm (NetworkInstance *inst, char *term_name);

/* parse.c */
//int yyparse (void);

/* sig_init (); */
void signal_init (void);

/* slserr.c */
void sls_perror (char *s);
void sls_error (int lineno, int e_no, char *e_str);

/* simple.c */
void joinSimple (Netelem *dnet, Stack *dnet_xs, class ntwinst *inst,
		    Stack *inst_xs, Netelem *net, Stack *net_xs);
void outSimple (Queue *q);

/* to_db.c */
void term_to_db (Queue *q);
void inst_to_db (NetworkInstance *inst);
void net_to_db (Queue *q);

/* util.c */
Stack * stackcpy (Stack *stck);
char * strsav (char *s);
void bmove (char *to, char *from, int l);
void prxs (Stack *xs);
void stackfree (Stack *xs, int type);

# ifdef __cplusplus
  }
# endif
