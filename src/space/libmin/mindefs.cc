/*
 * ISC License
 *
 * Copyright (C) 1997-2018 by
 *	Arjan van Genderen
 *	Kees-Jan van der Kolk
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
#include <string.h>
#include <exception>

#include "src/space/libmin/mindefs.h"

static void my_terminate()
{
    fprintf(stderr, "\ninternal: *** Exception found\n");
    die();
}

void min_init_library(void)
{
    static bool is_initialized = false;

    if(is_initialized) return;

    is_initialized = true;

    std::set_terminate (my_terminate);
}

//==============================================================================
//
//  Assertion handling.
//
//==============================================================================

void __std_assert_failed__(const char *file_name, int lineno, const char* condition)
{
    char *s;
    if ((s = strrchr ((char*)file_name+1, '/'))) { /* don't use absolute path (SdeG) */
	while (--s > file_name && *s != '/');
	if (s > file_name) file_name = ++s;
    }
    fprintf(stderr, "\ninternal: *** Assertion `%s' failed at %s:%d\n", condition, file_name, lineno);
    die();
}

void __std_out_of_memory__(const char *file_name, int lineno, const char* expression)
{
    char *s;
    if ((s = strrchr ((char*)file_name+1, '/'))) { /* don't use absolute path (SdeG) */
	while (--s > file_name && *s != '/');
	if (s > file_name) file_name = ++s;
    }
    fprintf(stderr, "\ninternal: *** Out of memory at %s:%d\n", file_name, lineno);
    die();
}
