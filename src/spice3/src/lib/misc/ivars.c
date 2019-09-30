/**********
Copyright 1991 Regents of the University of California. All rights reserved.
**********/

#include "spice.h"
#include "misc.h"
#include <stdio.h>

char	*Spice_Path;
char	*News_File;
char	*Help_Path;
char	*Lib_Path;

static void env_overr (char **v, char *e)
{
    char *p;
    if (v && (p = getenv(e))) *v = p;
}

static void mkvar (char **p, char *b, char *v)
{
    char buf[1024];

    if (!*p) {
	sprintf(buf, "%s/%s", b, v);
	*p = copy(buf);
    }
}

void ivars ()
{
    char *ICD_Path = getenv("ICDPATH");

    env_overr(&Spice_Exec_Dir, "SPICE_EXEC_DIR");
    if (ICD_Path) {
	Spice_Lib_Dir = NULL;
	mkvar(&Spice_Lib_Dir, ICD_Path, "share/lib/spice3");
    }
    else
	env_overr(&Spice_Lib_Dir, "SPICE_LIB_DIR");

    if (*Spice_Lib_Dir == '~') Spice_Lib_Dir = tilde_expand (Spice_Lib_Dir);

    mkvar(&News_File,  Spice_Lib_Dir, "news");
    mkvar(&Help_Path,  Spice_Lib_Dir, "helpdir");
    mkvar(&Lib_Path,   Spice_Lib_Dir, "scripts");
    env_overr(&Spice_Path, "SPICE_PATH");
    mkvar(&Spice_Path, Spice_Exec_Dir, "spice3");

    env_overr(&Spice_Host,   "SPICE_HOST");
    env_overr(&Bug_Addr,     "SPICE_BUGADDR");
    env_overr(&Def_Editor,   "SPICE_EDITOR");
    env_overr(&AsciiRawFile, "SPICE_ASCIIRAWFILE");
}
