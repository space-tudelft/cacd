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

#include 	"src/space/libmin/mincompl.h"
#include 	"src/space/libmin/mintable.h"

//------------------------------------------------------------------------------

MINComplementSolver::MINComplementSolver(const MINTable& table)
	: MINSolver(table, table._num_vars + table._num_rows),
	  _intersect(table._num_vars, 1)
{
	// initially, the intersection implicant is the universal cube
	//
	_intersect.addUniversalCube();
}

MINComplementSolver::~MINComplementSolver(void)
{
}

//------------------------------------------------------------------------------

// task: determine which variables belong to the independent set; changes of the
//       independent set are registered on the stack
//
void
MINComplementSolver::reclassifyVars(void)
{
	for(STDDWord var = 0; var < _table._num_vars; var++)
	{
		if(_indep.contains(var)) continue;

		bool is_indep = true;

		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			if(_table.getField(row, var) != 0x3)
			{
				is_indep = false;
				break;
			}
		}

		if(is_indep)
			removeVariable(var);
	}
}

//------------------------------------------------------------------------------

// task: solve the complement problem
//
void
MINComplementSolver::solve(MINTable& result)
{
	// note: when there are no rows, the complement is the universal cube
	//
	if(_rows.get_cardinality() == 0)
	{
		result.addRows(_intersect);
		return;
	}

	// push a mark symbol on the stack
	//
	mark();

	// reclassify the variables
	//
	reclassifyVars();

	// choose a splitting variable
	//
	STDSDWord var = getSplittingVar();

	// note: when var < 0 we have a tautology, and the complement is void
	//
	if(var < 0)
	{
		rollback();
		return;
	}

	// remove the splitting variable
	//
	removeVariable(var);

	// split the problem
	//
	for(int pos = 1; pos >= 0; pos--)
	{
		mark();

		// delete rows
		//
		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			STDDWord field = _table.getField(row, var);
			if(!(field & (1 << pos)))
			{
				removeRow(row);
			}
		}

		// modify the intersection cube
		//
		_intersect.setField(0, var, 1 << pos);

		// recursively call solve
		//
		solve(result);

		rollback();
	}

	// rollback changes
	//
	_intersect.setField(0, var, 0x3);
	rollback();
}

//------------------------------------------------------------------------------
