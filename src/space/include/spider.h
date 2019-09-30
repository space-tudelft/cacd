/*
 * ISC License
 *
 * Copyright (C) 1989-2018 by
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

#ifndef spider_h
#define spider_h

typedef double 	meshCoor_t;

typedef struct Face {
     struct Face *next, *prev;	/* doubly linked face list pointers */
     struct subnode *sc_subn;	/* link to substrate contact subnode */
     struct Spider *corners[4];	/* link to 3 or 4 corner spiders */
     float area;		/* <= 0 if no a triangle, yet.
				 * NOTE: this convention is used */
     float len ;		/* max. edge length */
     float len2;		/* min. edge length */
     int pqIndex;		/* position in priority queue */
     int type;			/* top, bottom or sidewall */
} face_t;

#define act_z nom_z

typedef struct Spider {
    meshCoor_t nom_x, nom_y, nom_z; /* coordinates in 2d layout */
    meshCoor_t act_x, act_y;	/* coordinates in solid model, after possible displacements */
    struct subnode *subnode;	/* link to conductor subnode/node */
    struct subnode *subnode2;	/* link to substrate contact node */
    struct spiderEdge *edge;	/* link to solid mesh model datastruct */
    struct Face   *face;	/* special spider that represents a face */
    struct Spider *next;	/* link to next spider into the strip datastruct or freelist */
    struct Spider *hashNext;	/* link to next spider into the hash table */
    struct strip  *strip;	/* backpointer to strip datastruct */
    struct Carmom *moments;	/* cartesian multipole moments of charge distribution */
    int   isGate;		/* spider is part of tor-gate */
    short conductor;		/* used for display and to identify diffusion conductors */
    short flags;		/* for displacement */
} spider_t;

typedef enum {
    INTERNALEDGE=1, /* an edge not on the true edge of any conductor */
    CONDUCTOREDGE,  /* regular (horizontal) conductor edge */
    VERTICALEDGE,   /* vertical conductor edge */
    CROSSOVEREDGE,  /* crossover edge */
    CONTACTEDGE=8,  /* (vertical) contact edge */
    CONTACTEDGE_H,  /* horizontal contact edge */
} edgeType_t;

typedef struct spiderEdge {
    struct Spider     * sp;		/* vertex pointer */
    struct spiderEdge * nb;		/* adjacent edge */
    struct spiderEdge * oh;		/* other half of this edge */
    face_t * face;			/* ccw face */
    struct spiderEdge * next;		/* to link the edge in a bucket */
    edgeType_t type;                    /* see above */
} spiderEdge_t;

typedef struct meshInfo {
    struct Spider ** spider[2];
    face_t    ** faces;
} meshInfo_t;

typedef struct strip {
    spider_t ** bucket;
    int         numSpiders;
    spiderEdge_t *ehead, *etail;	/* edges in strip */
    face_t       *fhead, *ftail;	/* faces in strip */
    coor_t      xl, xr;		/* size and pos. of strip */
    unsigned long flags;
    FILE *      gbuf;
    int  	drawn;
} strip_t;

struct spiderControl {
    coor_t WindowWidth, WindowHeight;
    meshCoor_t maxSpiderLength;
    meshCoor_t maxCoarseSpiderLength;
    meshCoor_t maxFeArea;
    meshCoor_t minFeArea;
    meshCoor_t maxCoarseFeArea;
    meshCoor_t maxEdgeFeArea;
    meshCoor_t edgeSplitRatio;
    meshCoor_t maxEdgeFeRatio;
    FILE * debug;
    bool_t    printMatrix;
    bool_t    printGreen;
    bool_t    printSpider;
};

extern struct spiderControl spiderControl;
extern int nrOfSpiderLevels;

#define OTHER_SPIDER(s, e) (e -> oh -> sp)
#define NEXT_EDGE(s, e)    (e -> nb)

#endif /* spider_h */
