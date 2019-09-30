/*
 * ISC License
 *
 * Copyright (C) 2004-2018 by
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

#include <stdlib.h>

#include "src/space/libmin/mintable.h"
#include "src/space/libmin/minrobdd.h"
#include "src/space/libmin/minparse.h"

int main(int argc, char *argv[])
{
	min_init_library();

	if (argc == 2) {
	    MINRobdd robdd;
	    STDDWord sw = min_parse_expression(robdd, argv[1]);

	    vector<MINSymbol*> variables;
	    MINTable* table = robdd.createTable(sw, variables);
	    table->minimize();
	    table->outputAsExpression(cout, variables);
	    cout << endl;
	}
	else {
	    cerr << "minimize: " << "invalid # args" << endl;
	    return 1;
	}
	return 0;
}

void die ()
{
    exit(1);
}
