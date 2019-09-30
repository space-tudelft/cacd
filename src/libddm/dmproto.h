/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van der Hoeven
 *	P. van der Wolf
 *	P. Bingley
 *	T.G.R. van Leuken
 *	T. Vogel
 *	F. Beeftink
 *	M. Grueter
 *	E.F. Matthijssen
 *	G.W. Sloof
 *	S. de Graaf
 *	A.J. van Genderen
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

#ifndef _DMPROTO_INCLUDED
#define _DMPROTO_INCLUDED /* allows multiple inclusion */

#ifdef __cplusplus
  extern "C" {
#endif
DM_CELL *dmCheckOut(DM_PROJECT *dmproject, char *cell, const char *versionstatus, int versionnumber, const char *view, int mode);
DM_PROJECT *dmFindProjKey(int loc_imp, char *local_name, DM_PROJECT *father_proj, char **rem_namep, const char *view);
DM_PROJECT *dmOpenProject(char *projectname, int mode);
DM_STREAM *dmOpenStream(DM_CELL *key, const char *stream, const char *mode);
char *dmAttrPrint(char *attr, char *last, char *par, char *val);
char *dmAttrScan(char *attr, char **par, char **val);
int dmCheckIn(DM_CELL *key, int mode);
void dmCkinAll(int mode);
void dmCloseCellStreams(DM_CELL *cellkey, int mode);
int dmCloseProject(DM_PROJECT *dmproject, int mode);
int dmCloseProjectContinue(DM_PROJECT *dmproject);
int dmCloseStream(DM_STREAM *dmfile, int mode);
void dmError(char *s);
void dmError2(const char *s1, char *s2);
int dmGetDataEscape(DM_STREAM *dmfile, int fmt);
int dmGetDesignData(DM_STREAM *dmfile, int fmt);
int dmInit(char *progname);
void dmPerror(char *s);
char* dmStrError(void);
int dmPrintf(DM_STREAM *iop, char *format, ...);
int dmPutDataEscape(DM_STREAM *dmfile, int fmt);
int dmPutDesignData(DM_STREAM *dmfile, int fmt);
int dmQuit(void);
int dmRemoveCell(DM_PROJECT *dmproject, char *cell, char *versionstatus, int versionnumber, char *view);
int dmRenameStream(DM_CELL *key, char *oldname, char *newname);
int dmRmCellForce(DM_PROJECT *dmproject, char *cell, char *versionstatus, int versionnumber, char *view, int force);
int dmScanf(DM_STREAM *iop, char *fmt, ...);
int dmSeek(DM_STREAM *dmfile, long offset, long ptrname);
/* gcc 2.3.3 warns about using "struct stat" here... */
typedef struct stat _STRUCT_STAT_;
int dmStat(DM_CELL *key, const char *file, _STRUCT_STAT_ *buf);
long dmTell(DM_STREAM *dmfile);
int dmTestname(char *name);
int dmTestname2(DM_PROJECT *dmproject, char *name);
int dmUnlink(DM_CELL *key, char *file);
char *dmGetMetaDesignData(int reqid, DM_PROJECT *projectid, ...);
char *dmSubstituteEnvironmentVars(char *path);
FILE *dmOpenXData(DM_PROJECT *project, char *mode);
int dmGetCellStatus(DM_PROJECT *project, DM_XDATA *xdata);
int dmGetXData(FILE *fp, DM_XDATA *xdata);
int dmPutCellStatus(DM_PROJECT *project, DM_XDATA *xdata);
int dmPutXData(FILE *fp, DM_XDATA *xdata);
int dmStatXData(DM_PROJECT *project, struct stat *buf);
int dmUnlinkXData(DM_PROJECT *project);
#ifdef DM_GETOPT
int getopt(int argc, char **argv, char *opts);
#endif
#ifdef __cplusplus
  }
#endif

#endif /* _DMPROTO_INCLUDED */
