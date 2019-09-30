/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	P. Bingley
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

#define MAXPLANES	7
#define TABSIZE		256
#define MAXINT		2147483647

#define RGB_CACD_MAP	"TUDNelsisCacdColormap"

#ifdef PE
#undef PE
#endif
#define PE fprintf(stderr,

#define NOTUSED		0
#define USED		1

#define Malloc(type)      (type *) malloc (sizeof(type))
#define Calloc(n,type)    (type *) calloc ((unsigned) (n), sizeof(type))
#define Realloc(p,n,type) (type *) realloc((char *) p, ((unsigned) ((n) * sizeof(type))))

typedef struct _IdElem {
    unsigned long	pixel;
    int			state;
} IdElem;
