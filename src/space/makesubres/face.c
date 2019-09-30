/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include "src/space/makesubres/define.h"
#include "src/space/makesubres/extern.h"

extern int numFaces;

/* A doubly linked list of all allocated faces.
 * We need both head and tail, because we want to allow that
 * faces can be added to the list while we are traversing it
 * with faceEnumerate ();
 */
static face_t * faceHead = NULL;
static face_t * faceTail = NULL;

face_t * newFace ()
{
    face_t * f = NEW (face_t, 1);

    numFaces++;

    /* add face to list */
    if (faceHead == NULL) faceHead = f;
    if (faceTail) faceTail -> next = f;
    f -> next = NULL;
    f -> prev = faceTail;
    faceTail = f;

    f -> corners[0] = NULL;
    f -> corners[1] = NULL;
    f -> corners[2] = NULL;
    f -> corners[3] = NULL;
    f -> area = 0;
    f -> len  = 0;
    f -> mark = 0;
    f -> pqIndex = 0;

    return (f);
}

void disposeFace (face_t *f)
{
    face_t * next, * prev;	/* auxiliary pointers */

    /* First, delete face from faceList
     */
    next = f -> next;
    prev = f -> prev;

    if (next) next -> prev = prev;
    else {
	ASSERT (f == faceTail);
	faceTail = prev;
    }

    if (prev) prev -> next = next;
    else {
	ASSERT (f == faceHead);
	faceHead = next;
    }

    // Now, it has been deleted. Check the consistency of the list.
    ASSERT (faceHead == NULL || faceHead -> prev == NULL);
    ASSERT (faceTail == NULL || faceTail -> next == NULL);

    // Now actually dispose f
    DISPOSE (f, sizeof(face_t));
}

static face_t * cursor;

void faceInitEnumerate ()
{
    cursor = faceHead;
}

face_t * faceEnumerate ()
{
    face_t * f;

    // Only enumerate faces (for mesh refinement) that are not in stripA.
    for (f = cursor; f && inStripA (f); f = f -> next);
    if (f) cursor = f -> next;
    return (f);
}

/* With FeModePwl it is possible that not all faces have
 * an extra spider and can be disposed via this spider.
 * Thus, do it after stripStop.
 */
void freeFaces ()
{
    if (faceHead) {
	face_t *f;
	// ASSERT (FeModePwl);
	while ((f = faceHead)) {
	    faceHead = f -> next;
	    DISPOSE (f, sizeof(face_t));
	}
	faceTail = NULL;
    }
}
