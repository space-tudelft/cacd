/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
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

#include "src/ocean/libseadif/sea_decl.h"

/* The function sdfhashstring() converts a string to
 * a pseudo random integer in the range [0..tabsize-1].
 */
int sdfhashstring (char *str, int tabsize)
{
    int sum = 0, c, i, spec1 = 0;
    union {int l; char c[4];} spec;

    spec.l = 0;
    for (; (c = *str); ++str) { sum += c; spec1 ^= c; }
    spec.c[2] = (char)spec1;
    i = (spec.l + sum + (sum<<3) + (sum<<5) + (sum<<19)) % tabsize;
    return (i < 0 ? -i : i);
}

/* The function sdfhash2strings() converts two strings
 * to a pseudo random integer in the range [0..tabsize-1].
 */
int sdfhash2strings (char *str1, char *str2, int tabsize)
{
    int sum = 0, c, i, spec1 = 0, spec2 = 0;
    union {int l; char c[4];} spec;

    spec.l = 0;
    for (; (c = *str1); ++str1) { sum += c; spec1 ^= c; }
    for (; (c = *str2); ++str2) { sum += c; spec2 ^= c; }
    spec.c[3] = (char)spec1;
    spec.c[2] = (char)spec2;
    i = (spec.l + sum + (sum<<3) + (sum<<5) + (sum<<19)) % tabsize;
    return (i < 0 ? -i : i);
}

/* The function sdfhash3strings() converts three strings
 * to a pseudo random integer in the range [0..tabsize-1].
 */
int sdfhash3strings (char *str1, char *str2, char *str3, int tabsize)
{
    int sum = 0, c, i, spec1 = 0, spec2 = 0, spec3 = 0;
    union {int l; char c[4];} spec;

    spec.l = 0;
    for (; (c = *str1); ++str1) { sum += c; spec1 ^= c; }
    for (; (c = *str2); ++str2) { sum += c; spec2 ^= c; }
    for (; (c = *str3); ++str3) { sum += c; spec3 ^= c; }
    spec.c[3] = (char)spec1;
    spec.c[2] = (char)spec2;
    spec.c[1] = (char)spec3;
    i = (spec.l + sum + (sum<<3) + (sum<<5) + (sum<<19)) % tabsize;
    return (i < 0 ? -i : i);
}

/* The function sdfhash4strings() converts four strings
 * to a pseudo random integer in the range [0..tabsize-1].
 */
int sdfhash4strings (char *str1, char *str2, char *str3, char *str4, int tabsize)
{
    int sum = 0, c, i, spec1 = 0, spec2 = 0, spec3 = 0, spec4 = 0;
    union {int l; char c[4];} spec;

    spec.l = 0;
    for (; (c = *str1); ++str1) { sum += c; spec1 ^= c; }
    for (; (c = *str2); ++str2) { sum += c; spec2 ^= c; }
    for (; (c = *str3); ++str3) { sum += c; spec3 ^= c; }
    for (; (c = *str4); ++str4) { sum += c; spec4 ^= c; }
    spec.c[3] = (char)spec1;
    spec.c[2] = (char)spec2;
    spec.c[1] = (char)spec3;
    spec.c[0] = (char)spec4;
    i = (spec.l + sum + (sum<<3) + (sum<<5) + (sum<<19)) % tabsize;
    return (i < 0 ? -i : i);
}
