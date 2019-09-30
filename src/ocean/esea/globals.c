/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Paul Stravers
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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/esea/cedif.h"

/* this struct is filled by edifparse(): */
SEADIF edif_source;

/* this hold the target language that we are currently converting to: */
languageType targetLanguage = NoLanguage;

/* this is where solveRef() puts the resolved cells: */
SEADIF *seadif_tree = NULL;

/* list of mappings of external library names: */
STRING libMap[MAXEXTERNLIBS+1][2];
