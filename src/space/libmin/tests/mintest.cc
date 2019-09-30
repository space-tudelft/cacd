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
#include <time.h>

#include "src/space/libmin/mintable.h"
#include "src/space/libmin/mincover.h"
#include "src/space/libmin/mintaut.h"
#include "src/space/libmin/minrobdd.h"
#include "src/space/libmin/minparse.h"

//------------------------------------------------------------------------------

int test_tautology(void);
int test_cover(void);
int test_expand(void);
int test_reduce(void);
int test_irredundant(void);
int test_minimize(void);

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
        min_init_library();

	if(argc >= 2)
	{
	    MINRobdd robdd;

	    STDDWord sw = min_parse_expression(robdd, argv[1]);

	    vector<MINSymbol*> variables;
	    MINTable* table = robdd.createTable(sw, variables);
	    table->minimize();

	    cerr << "Table: " << endl << *table << endl;
	    cerr << "Expression: `";
	    table->outputAsExpression(cerr, variables);
	    cerr << "'" << endl << endl;

	    delete table;
	}

#if 1
	test_tautology();
	test_cover();
	test_expand();
	test_reduce();
	test_irredundant();
	test_minimize();
#endif

#if 1
	time_t result = time(NULL);
	srand(result);
#else
	srand(9384);
#endif

#if 0
	for(STDDWord i = 0; i < 100; i++)
	{
		STDDWord num_vars = (rand() % 30)+1;
		STDDWord num_rows = (rand() % 50)+1;

		MINTable table(num_vars, num_rows);
		for(STDDWord j = 0; j < num_rows; j++)
			table.addUniversalCube();

		float density = (rand() % 1000)/500.0;

		for(STDDWord row = 0; row < num_rows; row++)
		{
			STDDWord var = rand() % num_vars;
			STDDWord field = (rand() % 2) + 1;

			table.setField(row, var, field);
		}

		for(STDDWord j = 0; j < (num_vars * num_rows)*density; j++)
		{
			STDDWord var = rand() % num_vars;
			STDDWord row = rand() % num_rows;
			STDDWord field = (rand() % 2) + 1;

			table.setField(row, var, field);
		}

		cerr << "TABLE " << i << endl << table;

		MINTable table2(table);
		table2.minimize();

		cerr << "verifying ...";

		STD_ASSERT(table.isEquivalent(table2));

		cerr << " ok " << table.cost() << " => " << table2.cost() << endl << endl;
	}
#endif

	return 0;
}

//------------------------------------------------------------------------------

int test_minimize(void)
{
	MINTable table(3, 0);

	table.addRow("10 10 11");
	table.addRow("11 10 01");
	table.addRow("01 11 01");
	table.addRow("01 01 11");
	table.addRow("11 01 10");
	table.addRow("10 10 01");

	cerr << "TABLE\n" << table << endl;

	MINTable table2(table);
	table2.minimize();

	cerr << "MINIMIZED\n" << table2 << endl;

	STD_ASSERT(table.isEquivalent(table2));

	return 0;
}

//------------------------------------------------------------------------------

int test_irredundant(void)
{
	MINTable table(3, 0);

	table.addRow("10 10 11");
	table.addRow("11 10 01");
	table.addRow("01 11 01");
	table.addRow("01 01 11");
	table.addRow("11 01 10");
	table.addRow("10 10 01");

	cerr << "TABLE\n" << table << endl;

#if 0
	STDIntSet er_set = table.relativelyEssentialSet();
	STDIntSet tr_set = table.totallyRedundantSet(er_set);

	cerr << "relatively essential: " << er_set << endl;
	cerr << "totally redundant: " << tr_set << endl;
#endif

	cerr << "================================================================================\n";

	MINTable irr_table = table.irredundant();

	cerr << "IRREDUNDANT TABLE\n" << irr_table << endl;

	cerr << "================================================================================\n";

	table.clear();
	table.addRow("10 01 11");
	table.addRow("10 01 11");
	irr_table = table.irredundant();
	cerr << "IRREDUNDANT TABLE\n" << irr_table << endl;

	return 0;
}

//------------------------------------------------------------------------------

int test_reduce(void)
{
	cerr << "================================================================================\n";

	MINTable table(3, 0);

	table.addRow("11 11 10");
	table.addRow("10 10 11");

	cerr << "TABLE\n" << table << endl;

	table.reduce();

	cerr << "REDUCED TABLE\n" << table << endl;

	return 0;
}

//------------------------------------------------------------------------------

int test_expand(void)
{
	MINTable table(3, 0);

	table.addRow("10 10 10");
	table.addRow("01 10 10");
	table.addRow("10 01 10");
	table.addRow("10 10 01");
	table.addRow("01 01 10");

	cerr << "TABLE\n" << table << endl;

	MINTable complem = table.complement();

	cerr << "COMPLEMENT\n" << complem << endl;

	table.expand(complem);

	cerr << "EXPANDED\n" << table << endl;

	return 0;
}

//------------------------------------------------------------------------------

int test_tautology(void)
{
	MINTable table(3, 0);
	bool value;

	table.addRow("01 01 11");
	table.addRow("01 11 01");
	table.addRow("01 10 10");
	table.addRow("10 11 11");

	value = table.tautology();
	STD_ASSERT(value);

	table.clear();
	table.addRow("01 01 11");
	table.addRow("01 11 01");
	table.addRow("10 11 11");

	value = table.tautology();
	STD_ASSERT(!value);

	table.clear();
	table.addRow("01 11 11");
	table.addRow("10 11 11");

	value = table.tautology();
	STD_ASSERT(value);

	cerr << "message: tautology checking ok ...\n";

	return 0;
}


//------------------------------------------------------------------------------

int test_cover(void)
{
	MINCover cover(6, 0);

	cover.addRow("001100");
	cover.addRow("000110");
	cover.addRow("001110");
	cover.addRow("010101");
	cover.addRow("010010");
	cover.addRow("000001");

	cerr << cover << endl;

	MINCoverState state(cover);
	cerr << state << endl;

	state.minimize();
	cerr << "MINIMIZED " << state << endl;

	return 0;
}

//------------------------------------------------------------------------------

void die ()
{
    exit(1);
}
