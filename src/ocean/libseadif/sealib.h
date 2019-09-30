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

#ifndef __SEALIB_H
#define __SEALIB_H

#include "src/ocean/libseadif/libstruct.h"
#include "src/ocean/libseadif/sealibio.h"
#include "src/ocean/libseadif/sea_func.h"

extern LIBTABPTR thislibtab; /* set by existslib() */
extern FUNTABPTR thisfuntab; /* set by existsfun() */
extern CIRTABPTR thiscirtab; /* set by existscir() */
extern LAYTABPTR thislaytab; /* set by existslay() */

extern LIBRARYPTR  thislib; /* set by sdfreadlib() sdfreadfun() sdfreadcir() sdfreadlay() */
extern FUNCTIONPTR thisfun; /* set by sdfreadfun() sdfreadcir() sdfreadlay() */
extern CIRCUITPTR  thiscir; /* set by sdfreadcir() sdfreadlay() */
extern LAYOUTPTR   thislay; /* set by sdfreadlay() */

extern SEADIF sdfroot; /* the root of the in-core tree */

#endif
