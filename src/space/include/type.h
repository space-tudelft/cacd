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

/*
 * This file is to be included by all .c files
 * and contains type definitions that should be
 * known globally.
 */

#include "src/space/auxil/bool.h"
#include "src/space/include/tile.h"
#include "src/space/include/maskinfo.h"
#include "src/space/include/node.h"
#include "src/space/include/subnode.h"
#include "src/space/include/terminal.h"
#include "src/space/include/transist.h"

#ifdef CAP3D
#include "src/space/include/spider.h"
#include "src/space/include/schur.h"
#endif /* CAP3D */

#include "src/space/include/extract.h"
#include "src/space/include/lump.h"

#include "src/space/include/bipolar.h"
#include "src/space/include/polnode.h"

