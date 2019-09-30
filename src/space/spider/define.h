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

#define SP_EPS 1e-6	/* a general value for comparing eq. of floats */

#define LFT_TOP 0
#define RGT_BOT 1

#define SINGLE_STRIP 1
#define DOUBLE_STRIP 2

#define ccwa(s, f) meshCcwAdjacent ((s), (f), (face_t *) NULL)
#define  cwa(s, f)  meshCwAdjacent ((s), (f))

#define FACE_TOP	1
#define FACE_BOT	2
#define FACE_SIDE	4
#define FACE_GATE	8
#define FACE_CAP2D	16
#define FACE_CSIDE	32
#define FACE_VSIDE	64
#define FACE_COARSE	128
#define FACE_CORE	256
#define FACE_KNOWN	512
#define FACE_RIGHT	1024

#define isKnown(face) (face -> type & FACE_KNOWN)
#define isRight(face) ((face -> type & (FACE_RIGHT+FACE_KNOWN)) == FACE_RIGHT)

#define E_DISPL FACE_GATE
#define C_DISPL FACE_CAP2D
#define isDisplace(sp) (sp -> flags & (E_DISPL+C_DISPL))

#define MAX_NORM_SPIDER_LEN	2	/* 2 times sqrt area */

     /* For controlling the direction of mesh splitting. */
#define DEVIATION_FACTOR 0.8

/* The Nearby macro is used to compare coordinates.
 * These are, (i think) in principle, integral values.
 * Thus it is not appropriate (wrong) to compare them like
 *    abs((a-b)/(a+b)) < eps.
 *
 * The variables a and b are usually of type meshCoor_t.
 * Type meshCoor_t uses the same unit as type coor_t.
 * However, while type meshCoor_t is a double-like
 * type, type coor_t is an integer-like type.
 */
#define Nearby(a,b)    (Abs((a) - (b)) <= SP_EPS)

#define Found(s,x,y,z) (s && Nearby(s->nom_x,x) && Nearby(s->nom_y,y) && Nearby(s->nom_z,z))

#define Microns(l)  (new_microns * l)
#define Microns2(a) (new_microns2 * a)

#define SP(k) face -> corners[k]

