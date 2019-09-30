/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Definitions for all external symbols in CP.
 */

#ifndef CPEXTERN_H
#define CPEXTERN_H

/* alias.c */

extern struct alias *cp_aliases;
extern void com_alias();
extern void com_unalias();
extern void cp_paliases();
extern void cp_setalias();
extern void cp_unalias();
extern wordlist *cp_doalias();

/* backquote.c */

extern wordlist *cp_bquote();

/* complete.c */

extern bool cp_nocc;
extern bool cp_comlook();
extern char *cp_kwswitch();
extern void cp_addcomm();
extern void cp_addkword();
extern void cp_ccom();
extern void cp_ccon();
extern void cp_ccrestart();
extern void cp_remcomm();
extern void cp_remkword();
extern wordlist *cp_cctowl();

/* cshpar.c */

extern FILE *cp_in;
extern FILE *cp_out;
extern FILE *cp_err;
extern FILE *cp_curin;
extern FILE *cp_curout;
extern FILE *cp_curerr;
extern bool cp_debug;
extern char cp_amp;
extern char cp_gt;
extern char cp_lt;
extern void com_chdir();
extern void com_echo();
extern void com_strcmp();
extern void com_rehash();
extern void com_shell();
extern void cp_ioreset();
extern wordlist *cp_redirect();
extern wordlist *cp_parse();

/* front.c */

extern bool cp_cwait;
extern bool cp_dounixcom;
extern int  cp_evloop();
extern void com_cdump();
extern void cp_resetcontrol();
extern void cp_toplevel();
extern void cp_popcontrol();
extern void cp_pushcontrol();

/* glob.c */

extern bool cp_globmatch();
extern wordlist *cp_doglob();

/* history.c */

extern bool cp_didhsubst;
extern bool cp_didhsubstp;
extern int cp_maxhistlength;
extern struct histent *cp_lastone;
extern void com_history();
extern void cp_addhistent();
extern void cp_hprint();
extern wordlist *cp_histsubst();

/* lexical.c */

extern FILE *cp_inp_cur;
extern bool cp_bqflag;
extern bool cp_interactive;
extern char *cp_altprompt;
extern char *cp_promptstring;
extern int cp_event;
extern wordlist *cp_lexer();
extern int inchar();

/* modify.c */

extern void cp_init();

/* output.c */

extern char out_pbuf[];
extern bool out_moremode;
extern bool out_isatty;
extern void out_init();
extern void out_printf();
extern void out_send();

/* quote.c */

extern char *cp_unquote();
extern char *cp_unquote2();
extern void cp_striplist();
extern void cp_wstrip();

/* unixcom.c */

extern bool cp_unixcom();
extern void cp_hstat();
extern void cp_rehash();

/* variable.c */

extern bool cp_getvar();
extern bool cp_ignoreeof;
extern bool cp_noclobber;
extern bool cp_noglob;
extern void com_set();
extern void com_unset();
extern void com_shift();
extern void cp_remvar();
extern void cp_vprint();
extern void cp_vset (char *varname, int type, char *value);
extern wordlist *cp_variablesubst();
extern wordlist *cp_varwl();
extern struct variable *cp_setparse();

/* var2.c */
wordlist *vareval();

/* cpinterface.c etc -- stuff CP needs from FTE */

extern bool cp_istrue();
extern bool cp_oddcomm();
extern void cp_doquit();
extern void cp_periodic();
extern void ft_cpinit();
extern struct comm *cp_coms;
extern double *ft_numparse();
extern char *cp_program;
extern bool ft_nutmeg;
extern struct variable *cp_enqvar();
extern void cp_usrvars();
extern int cp_usrset();
extern void fatal();

#endif
