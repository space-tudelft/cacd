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
/*
 * This file prototypes all the system dependend functions.
 *
 * Of course, i'd rather do something like #include <unistd.h> but this does
 * not seem to be a very portable thing to do. ANSI, POSIX, K&R, it's a big mess
 * out there. In stead, just take this file sysdep.h and edit too match your
 * local configuration...
 */

#ifndef __SYSDEP_H
#define __SYSDEP_H

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "src/ocean/libseadif/systypes.h"
#include "src/ocean/libseadif/libstruct.h"

#ifdef __cplusplus
extern LINKAGE_TYPE
{
#endif

#define SIG_PF_TYPE void (*)(int)

#ifdef __cplusplus
}
#endif

#ifndef W_OK
/* this is an argument to the access() system call */
#define W_OK 02
#endif

#endif
