/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	O. Hol
 *	P.E. Menchen
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"

#define	MALLOC(ptr,type) {\
    ptr = (type *)malloc( (unsigned) sizeof(type));\
    if (!ptr) die (1, "", "");\
}
#define SIZE_PTR_INT	(sizeof(int *))

#define	NOPOINTER 	0
#define	POINTER 	1

#define	NoType		5
#define	NoPlace		6

#define	OutpTerm	1	/* these defines must be equal to the  */
#define	InoTerm 	2	/* the defines used in futil.c of sls  */
#define	InpTerm 	3
#define	InrTerm 	4

#define	MIN_CAP		5	/* these defines must be equal to the  */
#define	MAX_CAP		6	/* the defines used in futil.c of sls  */

#define	StateChar 	7
#define	StateInt 	8
#define	StateFloat 	9
#define	StateDouble	10

#define	NMBIND 		2      /* terminal arrays may be maximum 2 dimensional*/

#define	FUNNOIND 	11     /* these defines are used for       */
#define	FUNINT 		12     /* evaluating the indexes of        */
#define	FUNVAR 		13     /* terminals of the function block  */

extern	int yylineno;
extern	int yydebug;

typedef struct term {
	char	name[DM_MAXNAME+1];
	int	type;
	int	ind[NMBIND];
	int	arrind;        /* arrind indicates the index of the terminal */
			       /* in the created array 'inout'               */
	struct term	*next;
} FTERM;


extern FTERM	*trm_buf;	/* list temporarely containing terminals */
extern FTERM	*ftrm_list;     /* list containing all terminals in      */
				/* order they are declared               */

extern char	Func_name[];
extern FILE     *yyin, *yyout;

extern DM_CELL  *key;
extern DM_STREAM *fdes;
extern DM_PROJECT *dmproject;

extern int 	verbose; 	/* variables */
extern int 	kflag;		/* used for  */
extern char *C_options[]; 	/* cc-options */
extern char *P_options[]; 	/* cpp-options */

extern int 	os_cnt;		/* these variables are used                 */
extern int 	is_cnt;		/* in routines in the files parsefuncs.c    */
extern int 	rs_cnt;		/* and rout_eval.c; they indicate the       */
extern int 	ss_cnt;		/* places in the list of terminals          */
extern int 	ps_cnt;		/* for a certain type of terminal           */

extern int 	adm_bsalloc_flag;	/* bit string allocation administration     */
                                        /* must be done in this block               */

/* addfunob.c */
extern void	addfun_obj (char fistr[], int k);
extern void	cppexec (char cppin[], char cppout[]);
extern void	rmexec (char arg[]);
/* addfunte.c */
extern void	addfun_term (FTERM *list);
/* main.c */
extern void	die (int nr, char *s1, char *s2);
extern void	usage (char *s1, char c);
extern void	yyerror (char *s);
extern void	lineno (int line);
/* parsefuncs.c */
extern FTERM   *fill_list (FTERM *ptr, char name[], int ind0, int ind1, int arrind, int type);
extern FTERM   *add_list (FTERM *ptr1, FTERM *ptr2, int type);
extern void	app_list (FTERM *ptr1, FTERM *ptr2);
extern void	check_term (FTERM *ptr, char s[], int indv0, int indv1);
extern void	print_func_head (char s);
extern void	build_array (void);
extern void	print_decl (void);
extern void	print_func_foot (void);
extern char    *strsav (char *str);
extern int	yylex (void);
/* rout_evals.c */
extern void	delay_eval (char term[], char ind0[], char ind1[], int indvar0, int indvar1);
extern void	capadd_eval (char term[], char ind0[], char ind1[], int indvar0, int indvar1);
extern void	getcap_eval (char rout[], int vicin, int min_max, char term[], char ind0[], char ind1[], int indvar0, int indvar1);
extern void	pr_rtn_arg (char rout[], FTERM *ptr, char ind0[], char ind1[], int indvar0, int indvar1, int min_cnt);
