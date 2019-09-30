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

#define EPS 1e-6       /* a general value for comparing eq. of floats */
#define Epsilon0	8.855e-12	/* Farad/meter */

#define Log(x)		log  ((double)(x))
#define Sqrt(x)		sqrt ((double)(x))
#define Atan(x)		atan ((double)(x))
#define Fabs(x)		fabs ((double)(x))

#define TEST_MULTIPOLE_METHOD

typedef double green_t;
typedef double real_t;

typedef struct pointR3 {
    real_t x, y, z;
} pointR3_t;

#define GREEN_MAX_DIELECTRICS_SIMPLE 3
#define GREEN_MAX_SUBSTRATE_LAYERS_SIMPLE 2
#define GREEN_MAX_LAYERS 100
#define GREEN_MAX_TERMS  500

typedef struct image {
    green_t strength;
    real_t  distance;
    int     zp_sign, zq_sign;
} image_t;

typedef struct imageGroup {
    int size;
    image_t * images;
} imageGroup_t;

typedef struct integrate_r {
    green_t value;
    green_t error;
} integrate_t;
