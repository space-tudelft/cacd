/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "src/libddm/dmincl.h"
#include "src/space/include/config.h"   /* for SCALE */
#include "src/space/auxil/auxil.h"

struct net_ref {
    char *name;
    struct net_ref *next;
};

#ifdef __cplusplus
  extern "C" {
#endif
void readMatchOutput (DM_CELL *cirKey);
#ifdef __cplusplus
  }
#endif

extern bool_t optMatching;
extern bool_t optInconclusive;
extern bool_t optDeficient;
extern bool_t optNets;
extern char * netGroups;
extern bool_t optPorts;
extern char * portGroups;
extern bool_t optDevices;
extern char * devGroups;
extern char * optLayer;

extern struct net_ref *Begin_net_ref;
