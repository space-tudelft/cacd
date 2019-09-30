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
#include <stdlib.h>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

/* local operations */
#ifdef __cplusplus
  extern "C" {
#endif
Private int addNewkeylist (int mask, int occurrence);
#ifdef __cplusplus
  }
#endif

/* changeMasks completes the keylist,
 * contructs the maskTransf vector.
 */
void changeMasks ()
{
    struct layerRef *ref, *ref2, *oldkeylist, *p;
    int i, nbr;

    ALLOC (maskTransf, procdata -> nomasks + subdata -> nomasks, int);
    for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) maskTransf[i] = -1;

    /* the mask colors must always be in this fixed order */
    nbr = 0;
    for (ref = Keylist; ref; ref = ref -> next) {
	maskTransf[ref -> lay -> mask] = nbr++;
    }
    /* this sets the colors for unused masks */
    for (i = 0; i < procdata -> nomasks + subdata -> nomasks; ++i) {
	if (maskTransf[i] == -1) maskTransf[i] = nbr++;
    }

    if (maxNbrKeys < 0)
	oldkeylist = keylist;
    else
	oldkeylist = Keylist;
    keylist = keylist2 = NULL;

    p = 0;
    for (ref = oldkeylist; ref; ref = ref -> next) {
	if (ref -> lay -> occurrence != SURFACE) {
	    if (ref -> lay -> occurrence == EDGE) {
		if (maxNbrKeys < 0) { /* look, if this edge is used */
		    for (ref2 = Keylist; ref2; ref2 = ref2 -> next) {
			if (ref2 -> lay -> occurrence != EDGE) { ref2 = 0; break; }
			if (ref2 -> lay -> mask == ref -> lay -> mask) break;
		    }
		    if (!ref2) {
			warningMes ("keys statement contains unused edge mask:",
			    maskname (ref -> lay -> mask));
			ref -> lay -> occurrence = SURFACE;
			p = ref; continue;
		    }
		}
		if (addNewkeylist (ref -> lay -> mask, EDGE)) {
		    p = ref; continue;
		}
	    }
	    else
		warningMes ("mask with '=' cannot be used in keylist:",
		    maskname (ref -> lay -> mask));
	    /* remove key 'ref' from list */
	    if (p) p -> next = ref -> next;
	    else  oldkeylist = ref -> next;
	}
	else p = ref;
    }

    p = 0;
    for (ref = oldkeylist; ref; ref = ref -> next) {
	if (ref -> lay -> occurrence == SURFACE) {
	    i = 1;
	    if (maxNbrKeys < 0) { /* look, if it must be also in keylist2 */
		for (ref2 = Keylist; ref2; ref2 = ref2 -> next) {
		    if (ref2 -> lay -> mask == ref -> lay -> mask) {
			if (ref2 -> lay -> occurrence == EDGE) i = 2;
			break;
		    }
		}
		if (!ref2) { i = 0; /* don't add to keylist */
		    warningMes ("keys statement contains unused surface mask:",
			maskname (ref -> lay -> mask));
		}
	    }
	    if (i == 2) addNewkeylist (ref -> lay -> mask, EDGE);
	    else if (i) addNewkeylist (ref -> lay -> mask, SURFACE);
	    /* remove key 'ref' from list */
	    if (p) p -> next = ref -> next;
	    else  oldkeylist = ref -> next;
	}
	else p = ref;
    }

    /* oldkeylist contains now only EDGE occurrences */

    if (maxNbrKeys < 0) { /* add EDGE masks to keylist2 */
	if (oldkeylist) {
	    if ((ref = keylist2)) {
		while (ref -> next) ref = ref -> next;
		ref -> next = oldkeylist;
	    }
	    else keylist2 = oldkeylist;
	}
    }
    else {
	if (maxNbrKeys > 0) {
	    ref = keylist;
	    for (nbr = 1; nbr < maxNbrKeys && ref; ++nbr) {
		ref = ref -> next;
	    }
	    if (ref) ref -> next = NULL;
	}
	else keylist = NULL;

	ref = keylist2;
	if (maxNbrKeys2 < 0) maxNbrKeys2 = maxNbrKeys < 12 ? maxNbrKeys : 12;
	if (maxNbrKeys2 > 0 && ref) {
	    for (nbr = 1; nbr < maxNbrKeys2; ++nbr) {
		if (!ref -> next) break;
		ref = ref -> next;
	    }
	    ref -> next = NULL;
	}
	else keylist2 = NULL;

	if (maxEdgeKeys > 0) { /* add edge keys */
	    if (keylist2) ref -> next = oldkeylist;
	    else keylist2 = oldkeylist;
	    ref = oldkeylist;
	    for (nbr = 1; nbr < maxEdgeKeys && ref; ++nbr) {
		ref = ref -> next;
	    }
	    if (ref) ref -> next = NULL;
	}
    }

    sSlotCnt = oSlotCnt = 0;
    nbrKeySlots = 1;
    for (ref = keylist; ref; ref = ref -> next) {
	nbrKeySlots *= 2;
	if (maskTransf[ref -> lay -> mask] < maxKeys2) ++sSlotCnt;
	else ++oSlotCnt;
    }

    sSlotCnt2 = eSlotCnt = 0;
    nbrKeySlots2 = 1;
    for (ref = keylist2; ref; ref = ref -> next) {
	nbrKeySlots2 *= 2;
	if (ref -> lay -> occurrence == SURFACE) ++sSlotCnt2;
	else ++eSlotCnt;
    }
}

Private int addNewkeylist (int mask, int occurrence)
{
    struct layerRef *newref, *ref;

    /* keylist contains only SURFACE occurrences */
    if ((ref = keylist)) {
	if (ref -> lay -> mask == mask) return 0;
	while (ref -> next) {
	    ref = ref -> next;
	    if (ref -> lay -> mask == mask) return 0;
	}
    }

    ALLOC (newref, 1, struct layerRef); /* add SURFACE */
    newref -> next = NULL;
    ALLOC (newref -> lay, 1, struct layer);
    newref -> lay -> mask = mask;
    newref -> lay -> occurrence = SURFACE;

    if (!ref) keylist = newref;
    else ref -> next = newref;

    if (occurrence == EDGE) { /* add also EDGE */
	ALLOC (newref, 1, struct layerRef);
	newref -> next = NULL;
	ALLOC (newref -> lay, 1, struct layer);
	newref -> lay -> mask = mask;
	newref -> lay -> occurrence = SURFACE;

	if ((ref = keylist2)) {
	    while (ref -> next) ref = ref -> next;
	    ref -> next = newref;
	}
	else keylist2 = newref;
    }
    return 1;
}
