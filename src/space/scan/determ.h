/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Nick van der Meijs
 *	Simon de Graaf
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

#define ROOT_DEPTH     1 	/* depth of root */

#define DO_EXP       001
#define DO_RESIZE    002
#define DO_SPACE     004
#define DEVMOD       010
#define ISMACRO      020
#define ISLIBRARY    040
#define DONE_SPACE  0100
#define DONE_FLAT   0200
#define DONE_PSEUDO 0400

typedef struct design_object {
    char * name;	        /* cellname */
    unsigned int status;	/* 0 if not to be extracted */
    DM_CELL * lkey;
} do_t;

#ifdef __cplusplus
  extern "C" {
#endif

/* determ.c */
do_t *findCandidates (DM_PROJECT *project, char *root);
bool_t existDmStream (DM_CELL *key, char *streamName);

#ifdef __cplusplus
  }
#endif
