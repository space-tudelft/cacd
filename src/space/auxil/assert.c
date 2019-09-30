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

#include <stddef.h>
#include "src/space/auxil/auxil.h"

extern char *argv0;

/*
 * assertion_failed - print an 'assertion failed' message,
 * by calling the say function.
 * indicating file and line of failure.
 * This message is unconditional, the test is performed
 * by an ASSERT macro.
 */
void assertion_failed (const char *file, int line, const char *cond)
{
    char *s;
    if ((s = strrchr (file+1, '/'))) { /* don't use absolute path (SdeG) */
	while (--s > file && *s != '/');
	if (s > file) file = ++s;
    }
    fprintf (stderr, "%s: %s, %d: Assert (%s) failed!\n", argv0, file, line, cond);
    fprintf (stderr, "Aborting ...\n");
    abort();
}

#ifdef DRIVER
char *argv0 = "testassert";
int main ()
{
    int i = 0;
    fprintf (stderr, "expect 1 assertion failed for line %d\n", (int) __LINE__ + 1);
    ASSERT (i != i);
    ASSERT (i == i);
    return (0);
}
#endif
