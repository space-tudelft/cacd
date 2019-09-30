/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	O. Hol
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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
#include <stdarg.h>

/* BTNAND : returns result of nand-operation of given bits */

char BTNAND (int arg0, ...)
{
    va_list	ap;
    char	tmps;
    char	res;

    res = 'O';

    if ((tmps = (char)arg0) != '@' && res != 'I') {
        if (tmps == 'O')
            res = 'I';
        else if (tmps == 'X')
            res = 'X';

        va_start (ap, arg0);
        while ((tmps = (char)va_arg (ap, int)) != '@' && res != 'I') {
            if (tmps == 'O')
                res = 'I';
            else if (tmps == 'X')
                res = 'X';
        }
        va_end (ap);
    }
    return (res);
}