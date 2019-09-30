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

#ifndef __DMPROTO_INCLUDED
#define __DMPROTO_INCLUDED /* allows multiple inclusion */

#ifdef __cplusplus
  extern "C" {
#endif
int _dmAddImportedCell(DM_PROJECT *dmproject, char *cellname, char *alias, char *view, char *dmpath);
DM_STREAM *_dmMk_streamkey(void);
void _dmRm_streamkey(DM_STREAM *key);
void _dm_print_key(DM_CELL *key, char *s);
int _dmDoget(FILE *iop, register char *fmt, ...);
int _dmDoput(FILE *fp, register char *fmt, ...);
int _dmValidView(const char *view);
void _dmFatal(char *format, char *arg1, char *arg2);
int _dmGet_geo_data(FILE *fp, int geo_fmt);
int _dmGet_cir_data(FILE *fp, int cir_fmt);
int _dmDoget_net(FILE *fp, struct cir_net *net, int get_atom);
int _dmDoget_fault(FILE *fp, struct cir_fault *fault);
int _dmGet_flp_data(FILE *fp, int flp_fmt);
DM_PROCDATA *_dmDoGetProcess(int process_id, DM_PROJECT *dmproject);
DM_PROCDATA *_dmDoGetProcessFile(char *path);
int _dmIfdebug(char *file, int line);
int _dmCellIsRoot2(DM_PROJECT *dmproject, char *cellname, char *viewtype, char **celllist_array);
DM_CELL *_dmMk_cellkey(void);
void _dmRm_cellkey(DM_CELL *key);
int _dmCh_chkout(void);
int _dmCh_key(DM_CELL *key);
int _dmCh_cell(DM_PROJECT *proj, char *cell, const char *view);
int _dmLockProject(char *project);
int _dmUnlockProject(char *project);
int _dmPack(FILE *fp, register char *fmt, ...);
int _dmUnpack(FILE *fp, register char *fmt, ...);
int _dmPut_geo_data(FILE *fp, int geo_fmt);
int _dmPut_cir_data(FILE *fp, int cir_fmt);
int _dmDoput_net(FILE *fp, struct cir_net *net, int put_atom);
int _dmDoput_fault(FILE *fp, struct cir_fault *fault);
int _dmPut_flp_data(FILE *fp, int flp_fmt);
DM_PROJECT *_dmMk_projectkey(void);
void _dmRm_projectkey(DM_PROJECT *key);
int _dmCh_opproj(void);
int _dmCh_project(DM_PROJECT *key);
DM_PROJECT *_dmCh_proj(char *path);
void _dmClose_allproj(int mode);
DM_PROCDATA *_dmSearchProcInProjKeys(int processid);
char *_dmDoGetProcPath(int process_id, char *file_name, DM_PROJECT *dmproject);
int _dmRmCell(DM_PROJECT *dmproject, char *cell, char *view);
int _dmRmDir(DM_PROJECT *dmproject, char *path);
int _dmRmDirContents(DM_PROJECT *dmproject, char *path);
int _dmRun(char *path, ...);
int _dmRun2(char *path, char *argv[]);
int _dmRun3(char *path, char *argv[], const char* verbose_prefix);
int _dmSprintf(register char *ptr, register char *fmt, ...);
char *_dmStrSave(char *string);
void _dmStrFree(char *string);
void _dmCreateEmptyStream (DM_CELL *key, char *streamname);
void _dmSetReleaseProperties (DM_PROJECT *dmproject);
IMPCELL **_dmImportedCelllist(DM_PROJECT *dmproject, char *view);
void _dmFreeImportedCelllist (DM_PROJECT *dmproject, int view_entry);
int _dmCellIsRoot(DM_PROJECT *dmproject, char *cellname, char *viewtype);
int _dmExistCell(DM_PROJECT *dmproject, char *cell, const char *view);
int _dmExistView(DM_PROJECT *dmproject, const char *view);
char **_dmProjectlist(DM_PROJECT *dmproject);
char *_dmGetProcPath(DM_PROJECT *dmproject, char *file_name);
char **_dmCellEquivalence(DM_PROJECT *dmproject, char *dom_cell, char *dom_view, char *range_view);
char **_dmCelllist(DM_PROJECT *dmproject, char *view);
DM_PROCDATA *_dmGetProcess(DM_PROJECT *dmproject);
char *_dmFatherCell(DM_PROJECT *project, char *cellName, char *view);
int _dmAddCellEquivalence(DM_PROJECT *dmproject, char *dom_cell, char *dom_view, char *range_cell, char *range_view);
#ifdef __cplusplus
  }
#endif
#endif /* __DMPROTO_INCLUDED */
