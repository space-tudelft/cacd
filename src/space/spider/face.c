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

#include "src/space/include/config.h"
#include "src/space/auxil/auxil.h"
#include "src/space/include/type.h"
#include "src/space/spider/define.h"

#include "src/space/spider/recog.h"
#include "src/space/spider/extern.h"

extern int numFaces;
extern strip_t *stripR, *stripA, *stripAA;

/* A doubly linked list of all allocated faces.
 * We need both head and tail, because we want to allow that
 * faces can be added to the list while we are traversing it
 * with faceEnumerate ();
 */
static face_t * cursor;
static face_t * lastface;

/*
 * Create a face
 */
face_t * newFace ()
{
    face_t * f = NEW (face_t, 1);

    numFaces++;

    f -> sc_subn = NULL;
    f -> corners[0] = NULL;
    f -> area = 0;
    f -> len = 0;
    f -> pqIndex = 0;

    return (f);
}

/*
 * Dispose a face
 */
void disposeFace (face_t *f)
{
    face_t *next, *prev;

    /* First, delete face from faceList
     */
    next = f -> next;
    prev = f -> prev;

    if (f == cursor) { /* set new cursor */
        if (cursor == lastface) cursor = NULL;
        else cursor = cursor -> next;
    }

    ASSERT (isRight (f));
    if (next) next -> prev = prev; else { ASSERT (f == stripR -> ftail); stripR -> ftail = prev; }
    if (prev) prev -> next = next; else { ASSERT (f == stripR -> fhead); stripR -> fhead = next; }

    ASSERT (stripR -> fhead == NULL || stripR -> fhead -> prev == NULL);
    ASSERT (stripR -> ftail == NULL || stripR -> ftail -> next == NULL);

    DISPOSE (f, sizeof(face_t));
}

int faceInitEnumerateR ()
{
    face_t *f;

    if (!(f = stripR -> fhead)) return (0);
    do { f -> type |= FACE_RIGHT; } while ((f = f -> next));
    return (1);
}

void setLastFace ()
{
    lastface = stripR -> ftail;
    if (!cursor) cursor = lastface;
}

void faceInitEnumerate ()
{
    cursor   = stripR -> fhead;
    lastface = stripR -> ftail;
}

face_t * faceEnumerate ()
{
    face_t *f;
    if ((f = cursor)) {
	if (cursor == lastface) cursor = NULL;
	else cursor = cursor -> next;
    }
    return (f);
}

void removeFace (face_t *face)
{
    register spiderEdge_t *eh;
    spider_t *sp1, *sp;

    sp1 = sp = SP(0);
    do {
	ASSERT (sp);
        for (eh = sp -> edge; eh && eh -> face != face; eh = NEXT_EDGE (sp, eh));
        ASSERT (eh);
        eh -> face = NULL;
    } while ((sp = OTHER_SPIDER (sp, eh)) != sp1);

    disposeFace (face);
}

void stripAddFace (face_t *face, strip_t *strip)
{
    if (!strip -> fhead) {
	face -> prev = NULL;
	strip -> fhead = face;
    } else {
	face -> prev = strip -> ftail;
	strip -> ftail -> next = face;
    }
    strip -> ftail = face;
    face -> next = NULL;
}

void stripFreeFaces (strip_t *strip)
{
    face_t *face;
    while ((face = strip -> fhead)) {
	strip -> fhead = face -> next;
	DISPOSE (face, sizeof(face_t));
    }
}
