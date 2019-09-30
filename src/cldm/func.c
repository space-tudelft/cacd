/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	S. de Graaf
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

#include "src/cldm/extern.h"

char to_buf[20];

char *fromftoa (double d)
{
    sprintf (to_buf, "%.3f", d);
    return (to_buf);
}

char *fromitoa (int i)
{
    sprintf (to_buf, "%d", i);
    return (to_buf);
}

long roundh (double d)
{
    long i;
    i = (d < 0) ? (long)(d - 0.0005) : (long)(d + 0.9995);
    return (i);
}

long roundL (double d)
{
    long i;
    i = (d < 0) ? (long)(d - 0.9995) : (long)(d + 0.0005);
    return (i);
}
