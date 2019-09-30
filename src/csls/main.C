/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	P.E. Menchen
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

#define TOOLVERSION "4.83 22-Aug-2016"

int doSimpleNet = 1;
int genIntNet = 0;
int sflag = 0;
int externRequired = 0;
int forbidFirstCapital = 0;
int noWarnings = 0;
int runCpp = 0;
char f_view[BUFSIZ];
char fn_incl[128];
char fn_cppout[80];
char *cpp;
char *cpp_options;
char *defaultInclude;

char **globNets;
int globNets_cnt;
char **defGlobNets;
int defGlobNets_cnt;

#define GLOBNETFILE "global_nets"
#ifdef SPICE
char *rcFile     = (char*)".cspicerc";
char *rcFile_old = (char*)".putspicerc";
#else
char *rcFile     = (char*)".cslsrc";
char *rcFile_old = (char*)".sls_mkdbrc";
#endif

Stack	*xs;
Stack	*attr_s;
Dictionary	*ntw_dict = NULL;
Dictionary	*dff_dict = NULL;
Dictionary	*sym_dict = NULL;
Dictionary	*inst_dict = NULL;
Network         *curr_ntw = NULL;
Network *dev_tab[DEVTABLENGTH];
Netelem *notconnected;

int dict_nbyte, dict_maxnbyte;
int queue_nbyte, queue_maxnbyte;
int stack_nbyte, stack_maxnbyte;
int int_nbyte, int_maxnbyte;
int char_nbyte, char_maxnbyte;
int ntwdef_nbyte, ntwdef_maxnbyte;
int ntwinst_nbyte, ntwinst_maxnbyte;
int inst_struct_nbyte, inst_struct_maxnbyte;
int netelem_nbyte, netelem_maxnbyte;
int net_ref_nbyte, net_ref_maxnbyte;
int xelem_nbyte, xelem_maxnbyte;

#ifdef SPICE
char *argv0 = (char*)"cspice";
char *use_msg = (char*)"Usage: cspice [-psw] file_1 ... file_n";
#else
char *argv0 = (char*)"csls";
char *use_msg = (char*)"Usage: csls [-psw] file_1 ... file_n";
#endif

DM_PROJECT *dmproject = NULL;

extern int externspec;
extern Network *curr_ntw;
extern int yylineno;
extern FILE *yyin;

#ifdef __cplusplus
  extern "C" {
#endif

void yyerror (const char *s);

static void init_devs (void);
static void readGlobalNets (void);
static void readrcFile (void);
static int myscanf (int *fp_i, char *s);
static char *doCpp (char *fn);

#ifdef __cplusplus
  }
#endif

void scanrestart (FILE *input_file);
int yyparse (void);

int main (int argc, char **argv)
{
    int     iarg;
    char   *fn;
    struct stat statBuf;
    int runCppOpt = 0;
    int first = 1;

    strcpy (f_view, CIRCUIT);

    for (iarg = 1; iarg < argc && argv[iarg][0] == '-'; iarg++) {
	switch (argv[iarg][1]) {
	    case 'w':
		noWarnings = 1;
		break;
	    case 'p': /* run C preprocessor */
		runCppOpt = 1;
		break;
	    case 's': /* silent mode: no messages */
		sflag++;
		break;
	    case 'o':
		doSimpleNet = 0;
		break;
	    case 'i':
		genIntNet = 1;
		break;
	    default:
		printf ("%s: -%s: Unknown option\n", argv[0], &argv[iarg][1]);
	}
    }

    if (iarg >= argc) {/* Only options ? */
	if (argc == 1)
	    fprintf (stderr, "%s %s\n", argv0, TOOLVERSION);
	fprintf (stderr, "\n%s\n\n", use_msg);/* Usage: ...  */
	exit (0);
    }

    signal_init ();
    fn_cppout[0] = '\0';
    cpp = NULL;
    cpp_options = NULL;
    defaultInclude = NULL;

    dmInit (argv0);
    dmproject = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);

    /* from now on the program must always exit via die () */

    readrcFile ();
    if (runCppOpt) runCpp = 1;

    readGlobalNets ();

    attr_s = new Stack (XSTACK_SIZE);
    xs = new Stack (XSTACK_SIZE);
    ntw_dict = new Dictionary;
    dff_dict = new Dictionary;
    init_devs ();
    init_bifs ();

    strcpy (fn_incl, "");

    for ( ; iarg < argc; iarg++) {
	if (stat (argv[iarg], &statBuf) == 0) {
	    if (!sflag) {
		fprintf (stderr, "File %s:\n", argv[iarg]);
	    }
	    if (runCpp)
		fn = doCpp (argv[iarg]);
	    else
		fn = argv[iarg];
	    if ((yyin = fopen (fn, "r")) != NULL) {
                if (!first) {
                    scanrestart (yyin);
                }
                else first = 0;
		yylineno = 1;
		yyparse ();
		if (curr_ntw) end_ntw (curr_ntw, externspec, COMPLETE);
		fclose (yyin);
	    }
	    else {
		fprintf (stderr, "Cannot open %s\n", argv[iarg]);
	    }
	}
	else {
	    fprintf (stderr, "Cannot open %s\n", argv[iarg]);
	}

	if (fn_cppout[0]) {
	    unlink (fn_cppout);
	    fn_cppout[0] = '\0';
	}
    }

    delete ntw_dict;
    ntw_dict = NULL;
    delete dff_dict;
    dff_dict = NULL;

    if (dmproject) dmCloseProject (dmproject, COMPLETE);
    dmQuit ();

#ifdef DMEM
    print_memstat ();
#endif
    exit (0);
}

void die ()
{
    /* For some reason (bug in dmi ?) dmCloseProject (dmproject, QUIT)
       does not destroy the open streams of the cell that is checked out.
       So do an end_ntw (..., QUIT).
    */

    if (curr_ntw) end_ntw (curr_ntw, externspec, QUIT);

    if (dmproject) dmCloseProject (dmproject, QUIT);
    dmQuit ();

    if (sym_dict) {
	delete sym_dict;
	sym_dict = NULL;
    }
    if (ntw_dict) {
	delete ntw_dict;
	ntw_dict = NULL;
    }
    if (dff_dict) {
	delete dff_dict;
	dff_dict = NULL;
    }
    if (inst_dict) {
	delete inst_dict;
	inst_dict = NULL;
    }

    if (fn_cppout[0]) unlink (fn_cppout);

    exit (1);
}

static void init_devs ()
{
	Network *ntw;
	Queue *termq;
	Netelem *pterm;

	termq = new Queue (QueueType);
#ifdef SPICE
	pterm = new Netelem ((char*)"d", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"g", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"s", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"b", NULL, TermType);
	termq -> put ((Link *) pterm);
#else
	pterm = new Netelem ((char*)"g", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"d", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"s", NULL, TermType);
	termq -> put ((Link *) pterm);
#endif

	dev_tab[DevNenh] = ntw = new Network ((char*)"nenh");
	ntw -> termq = termq;
	ntw -> local = (existCell ("nenh", CIRCUIT) != 2);
	ntw_dict -> store (ntw -> ntw_name, (char *) ntw);

	dev_tab[DevPenh] = ntw = new Network ((char*)"penh");
	ntw -> termq = termq;
	ntw -> local = (existCell ("penh", CIRCUIT) != 2);
	ntw_dict -> store (ntw -> ntw_name, (char *) ntw);

	dev_tab[DevNdep] = ntw = new Network ((char*)"ndep");
	ntw -> termq = termq;
	ntw -> local = (existCell ("ndep", CIRCUIT) != 2);
	ntw_dict -> store (ntw -> ntw_name, (char *) ntw);

	termq = new Queue (QueueType);
	pterm = new Netelem ((char*)"p", NULL, TermType);
	termq -> put ((Link *) pterm);
	pterm = new Netelem ((char*)"n", NULL, TermType);
	termq -> put ((Link *) pterm);

	dev_tab[DevCap] = ntw = new Network ((char*)"cap");
	ntw -> termq = termq;
	ntw -> local = (existCell ("cap", CIRCUIT) != 2);
	ntw_dict -> store (ntw -> ntw_name, (char *) ntw);

	dev_tab[DevRes] = ntw = new Network ((char*)"res");
	ntw -> termq = termq;
	ntw -> local = (existCell ("res", CIRCUIT) != 2);
	ntw_dict -> store (ntw -> ntw_name, (char *) ntw);
}

#ifdef SPICE

double cvt_atof (char *s)
{
	double d;
	char   c;
	char  *q;

	q = s + strlen (s);
	while (--q > s && !isdigit (*q)) ;
	q++;

	c = *q;
	*q = '\0';
	d = atof (s);
	if (isupper (c)) c += ('a' - 'A');
	switch (c) {
	    case 't': d *= 1e12;  break;
	    case 'g': d *= 1e9;   break;
	    case 'k': d *= 1e3;   break;
	    case 'u': d *= 1e-6;  break;
	    case 'n': d *= 1e-9;  break;
	    case 'p': d *= 1e-12; break;
	    case 'f': d *= 1e-15; break;
	    case 'a': d *= 1e-18; break;
	    case 'm':
		switch (*++q) {
		    case 'E':
		    case 'e': d *= 1e6;     break; /* MEG */
		    case 'I':
		    case 'i': d *= 25.4e-4; break; /* MIL */
		    default:  d *= 1e-3;
		}
	}

	return (d);
}

#else

double cvt_atof (char *s)
{
	double d;
	char    c = s[strlen(s) - 1];

	if(isdigit(c))
	    d =(float) atof(s);
	else {
	    s[strlen(s) - 1] = '\0';
	    switch(c) {
		case 'G':
		    d = 1e9 * atof(s);
		    break;
		case 'M':
		    d = 1e6 * atof(s);
		    break;
		case 'k':
		    d = 1e3 * atof(s);
		    break;
		case 'm':
		    d = 1e-3 * atof(s);
		    break;
		case 'u':
		    d = 1e-6 * atof(s);
		    break;
		case 'n':
		    d = 1e-9 * atof(s);
		    break;
		case 'p':
		    d = 1e-12 * atof(s);
		    break;
		case 'f':
		    d = 1e-15 * atof(s);
		    break;
		case 'a':
		    d = 1e-18 * atof(s);
		    break;
		default:
		    d = atof(s);
	    }
	}
    return(d);
}

#endif

static void readGlobalNets ()
{
    FILE *fp;
    int cnt;
    int i;
    char *fn;
    char buf[128];

    fp = fopen (GLOBNETFILE, "r");
    if (fp == NULL) {
        fn = (char *)dmGetMetaDesignData (PROCPATH, dmproject, GLOBNETFILE);
        fp = fopen (fn, "r");
    }

    if (fp) {

        cnt = 0;
        while (fscanf (fp, "%s", buf) > 0) {
            cnt++;
        }
        rewind (fp);

        globNets = new char * [cnt];
        defGlobNets = new char * [cnt];

        cnt = 0;
        while (fscanf (fp, "%s", buf) > 0) {
            for (i = 0; i < cnt; i++) {
                if (strcmp (globNets[i], buf) == 0)
                    break;   /* double specification of this global net */
            }
            if (i == cnt) {
                globNets[cnt] = new char [strlen (buf) + 1];
                strcpy (globNets[cnt], buf);
                cnt++;
            }
        }
        globNets_cnt = cnt;

        fclose (fp);
    }
    else {
        globNets_cnt = 0;
    }
}

char *isGlobalNet (char *s)
{
    int i;

    for (i = 0; i < globNets_cnt; i++) {
        if (strcmp (s, globNets[i]) == 0) {
            return (globNets[i]);
        }
    }

    return (NULL);
}

char *isDefGlobalNet (char *s)
{
    int i;

    for (i = 0; i < defGlobNets_cnt; i++) {
        if (strcmp (s, defGlobNets[i]) == 0) {
            return (defGlobNets[i]);
        }
    }

    return (NULL);
}

static void pr_obsolete (char *fn)
{
    fprintf (stderr, "%s: Warning: file '%s' obsolete!\n", argv0, fn);
}

static void readrcFile ()
{
    struct stat statBuf;
    char buf[256];
    FILE *fp;
    char *fn, *home;
    int c, flag = 1;

    if (dmproject) {
	fn = (char *) dmGetMetaDesignData (PROCPATH, dmproject, rcFile_old);
	if (fn && stat (fn, &statBuf) == 0) pr_obsolete (fn);
	fn = (char *) dmGetMetaDesignData (PROCPATH, dmproject, rcFile+1);
	if (fn && (fp = fopen (fn, "r")) != NULL) goto read_file;
    }

pos2:
    flag = 2;
    if ((home = getenv ("HOME")) != NULL) {
	fn = new char [strlen (home) + strlen (rcFile) + 10];
	if (fn) {
	    sprintf (fn, "%s/%s", home, rcFile_old);
	    if (stat (fn, &statBuf) == 0) pr_obsolete (fn);
	    sprintf (fn, "%s/%s", home, rcFile);
	    if ((fp = fopen (fn, "r")) != NULL) goto read_file;
	}
    }

pos3:
    flag = 3;
    if (stat (rcFile_old, &statBuf) == 0) pr_obsolete (rcFile_old);
    fn = rcFile;
    if ((fp = fopen (fn, "r")) == NULL) return;

read_file:
    while (fscanf (fp, "%s", buf) > 0) {

	if (buf[0] == '#') {
	    /* comment */
	}
	else if (strcmp (buf, "CPP_OPTIONS:") == 0) {
	    if (myscanf ((int *)fp, buf) > 0) {
		if (cpp_options) delete cpp_options;
		cpp_options = new char [strlen (buf) + 1];
		strcpy (cpp_options, buf);
	    }
	    else {
		rcReadError (fn, "CPP_OPTIONS:");
	    }
	}
	else if (strcmp (buf, "DEFAULT_INCLUDE:") == 0) {
	    if (myscanf ((int *)fp, buf) > 0) {
		if (defaultInclude) delete defaultInclude;
		defaultInclude = new char [strlen (buf) + 1];
		strcpy (defaultInclude, buf);
	    }
	    else {
		rcReadError (fn, "DEFAULT_INCLUDE:");
	    }
	}
	else if (strcmp (buf, "EXTERN_OBLIGATORY") == 0) {
	    externRequired = 1;
	}
	else if (strcmp (buf, "EXTERN_OBLIGATORY_ON") == 0) {
	    externRequired = 1;
	}
	else if (strcmp (buf, "EXTERN_OBLIGATORY_OFF") == 0) {
	    externRequired = 0;
	}
	else if (strcmp (buf, "CPP:") == 0) {
	    if (myscanf ((int *)fp, buf) > 0) {
		if (cpp) delete cpp;
		cpp = new char [strlen (buf) + 1];
		strcpy (cpp, buf);
	    }
	    else {
		rcReadError (fn, "CPP:");
	    }
	}
	else if (strcmp (buf, "RUN_CPP") == 0) {
	    runCpp = 1;
	}
	else if (strcmp (buf, "RUN_CPP_ON") == 0) {
	    runCpp = 1;
	}
	else if (strcmp (buf, "RUN_CPP_OFF") == 0) {
	    runCpp = 0;
	}
	else if (strcmp (buf, "FORBID_FIRST_CAPITAL_ON") == 0) {
	    forbidFirstCapital = 1;
	}
	else if (strcmp (buf, "FORBID_FIRST_CAPITAL_OFF") == 0) {
	    forbidFirstCapital = 0;
	}
	else {
	    rcReadError (fn, buf);
	}

	while ((c = getc (fp)) != '\n' && c != EOF);
	if (c == EOF) break;

	/* skip everything until end of line */
    }

    fclose (fp);

    if (flag == 1) goto pos2;
    if (flag == 2) goto pos3;
}

static int myscanf (int *fp_i, char *s)
{
    FILE *fp = (FILE *)fp_i;
      /* conversion is necessary since the C preprocessor
         does not like FILE pointers as an argument */
    int i;
    int nonspace;
    char c;

    while ((c = getc (fp)) == ' ' || c == '\t');

    i = 0;
    nonspace = -1;
    if (c != EOF && c != '\n') {
	while (c != EOF && c != '\n') {
	    s[i] = c;
	    if (c != ' ' && c != '\t')
		nonspace = i;
	    c = getc (fp);
	    i++;
	}
    }

    s[nonspace + 1] = '\0';

    ungetc (c, fp);

    if (i > 0)
	return (1);
    else if (c == EOF)
	return (-1);
    else
	return (0);
}

void rcReadError (char *fn, const char *s)
{
    fprintf (stderr, "Error in file '%s' for '%s'\n", fn, s);
    die ();
}

static char *doCpp (char *fn)
{
    FILE *fp, *fp2;
    int c, argno = 0;
    struct stat statBuf;
    char fn_cppin[128];
    char *args[64], *s;

    sprintf (fn_cppin, "x%d.c", getpid ());
    sprintf (fn_cppout, "x%d.s", getpid ());

    if ((fp = fopen (fn_cppin, "w")) == NULL) {
	fprintf (stderr, "Cannot open %s\n", fn_cppin);
	return (fn);
    }
    if (stat (fn_cppout, &statBuf) == 0 && unlink (fn_cppout)) {
	fprintf (stderr, "Cannot unlink %s\n", fn_cppout);
	return (fn);
    }

    if (defaultInclude) {
	fprintf (fp, "#include \"%s\"\n", defaultInclude);
    }

    fprintf (fp, "#line 1 \"%s\"\n", fn);

    if ((fp2 = fopen (fn, "r")) == NULL) {
	/* error message is not necessary, since it will also be given later */
	fclose (fp);
	goto ret;
    }
    while ((c = getc (fp2)) != EOF) putc (c, fp);
    fclose (fp2);
    fclose (fp);

    if (!cpp) {
	cpp = (char*)"/usr/lib/cpp";
	if (stat (cpp, &statBuf)) {
	    cpp = (char*)"/lib/cpp";
	    if (stat (cpp, &statBuf)) cpp = (char*)"cpp";
	}
	args[argno++] = cpp;
    }
    else {
	char *a = cpp;
#ifdef WIN32
	/* skip path and possible space */
	if ((s = strrchr (a, '/'))) a = s + 1;
	if ((s = strrchr (a, '\\'))) a = s + 1;
#endif
	args[argno++] = a;
    }

    if ((s = cpp_options)) { /* split around space */
	while (*s) {
	    args[argno++] = s++;
	    if (argno > 61) {
		fprintf (stderr, "Too many CPP_OPTIONS\n");
		goto ret;
	    }
	    while (*s && !isspace (*s)) ++s;
	    if (*s) *s++ = 0;
	    while (isspace (*s)) ++s;
	}
    }

    args[argno++] = fn_cppin;
    args[argno++] = fn_cppout;
    args[argno] = NULL;
    _dmRun2 (cpp, args);

ret:
    unlink (fn_cppin);
    if (stat (fn_cppout, &statBuf) == 0) return (fn_cppout);
    return (fn);
}

void parseLineStmt (char *s)
{
    char *p;
    int i;

    /* Handles the following formats in the inputfile:
	  #line NUMBER FILENAME
	  #line NUMBER "FILENAME"
	  # NUMBER "FILENAME"
       Otherwise the line is just skipped as comment.
    */

    for (p = s + 1; *p && (*p == ' ' || *p == '\t'); p++);
    if ((*p >= '0' && *p <= '9')
	|| (*p == 'l' && *(p+1) == 'i'
	    && *(p+2) == 'n' && *(p+3) == 'e'
	    && (*(p+4) == ' ' || *(p+5) == '\t'))) {

	if (*p == 'l') {
	    p = p + 4;
	    while (*p && (*p == ' ' || *p == '\t')) p++;
	}

	if (sscanf (p, "%d", &yylineno) == 1) {

	    while (*p && !(*p == ' ' || *p == '\t')) p++;
	    while (*p && (*p == ' ' || *p == '\t')) p++;
	    if (*p == '"') p++;
	    if (*p) {
		i = 0;
		while (*p && *p != '"'
		       && !(*p == ' ' || *p == '\t')) {
		    fn_incl[i++] = *p++;
		}
		fn_incl[i] = '\0';
	    }
	}
    }
}

void dmError (char *s)
{
    dmPerror (s);
    die ();
}

#ifdef DMEM
void print_longline (FILE *fpstat)
{
    int i = 0;
    while (i++ < 4) fprintf (fpstat, "--------------------");
    fprintf (fpstat, "\n");
}

void print_memstat ()
{
    FILE *fpstat;

    if ((fpstat = fopen ("mem.stat", "w")) == NULL) {
	fprintf (stderr, "cannot open file stat\n");
	return;
    }

    print_longline (fpstat);
    fprintf (fpstat, "Class\t\tbytes\tmaxbytes\telt\tmaxelt\teltsize\n");
    print_longline (fpstat);

    fprintf (fpstat, "Dictionary:\t%d\t%d\t\t%d\t%d\t%d\n",
	dict_nbyte,
	dict_maxnbyte,
	dict_nbyte / sizeof (class dictionary),
	dict_maxnbyte / sizeof (class dictionary),
	sizeof (class dictionary));

    fprintf (fpstat, "Queue:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	queue_nbyte,
	queue_maxnbyte,
	queue_nbyte / sizeof (class queue),
	queue_maxnbyte / sizeof (class queue),
	sizeof (class queue));

    fprintf (fpstat, "Stack:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	stack_nbyte,
	stack_maxnbyte,
	stack_nbyte / sizeof (class stack),
	stack_maxnbyte / sizeof (class stack),
	sizeof (class stack));

    fprintf (fpstat, "NtwDef:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	ntwdef_nbyte,
	ntwdef_maxnbyte,
	ntwdef_nbyte / sizeof (class ntwdef),
	ntwdef_maxnbyte / sizeof (class ntwdef),
	sizeof (class ntwdef));

    fprintf (fpstat, "NtwInst:\t%d\t%d\t\t%d\t%d\t%d\n",
	ntwinst_nbyte,
	ntwinst_maxnbyte,
	ntwinst_nbyte / sizeof (class ntwinst),
	ntwinst_maxnbyte / sizeof (class ntwinst),
	sizeof (class ntwinst));

    fprintf (fpstat, "InstStruct:\t%d\t%d\t\t%d\t%d\t%d\n",
	inst_struct_nbyte,
	inst_struct_maxnbyte,
	inst_struct_nbyte / sizeof (class instancestruct),
	inst_struct_maxnbyte / sizeof (class instancestruct),
	sizeof (class instancestruct));

    fprintf (fpstat, "NetElem:\t%d\t%d\t\t%d\t%d\t%d\n",
	netelem_nbyte,
	netelem_maxnbyte,
	netelem_nbyte / sizeof (class netelem),
	netelem_maxnbyte / sizeof (class netelem),
	sizeof (class netelem));

    fprintf (fpstat, "NetRef:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	net_ref_nbyte,
	net_ref_maxnbyte,
	net_ref_nbyte / sizeof (class net_ref),
	net_ref_maxnbyte / sizeof (class net_ref),
	sizeof (class net_ref));

    fprintf (fpstat, "Xelem:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	xelem_nbyte,
	xelem_maxnbyte,
	xelem_nbyte / sizeof (class xelem),
	xelem_maxnbyte / sizeof (class xelem),
	sizeof (class xelem));

    fprintf (fpstat, "Int:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	int_nbyte,
	int_maxnbyte,
	int_nbyte / sizeof (int),
	int_maxnbyte / sizeof (int),
	sizeof (int));

    fprintf (fpstat, "Char:\t\t%d\t%d\t\t%d\t%d\t%d\n",
	char_nbyte,
	char_maxnbyte,
	char_nbyte / sizeof (char),
	char_maxnbyte / sizeof (char),
	sizeof (char));

    print_longline (fpstat);
}
#endif
