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

#include 	"src/space/libmin/mintaut.h"
#include 	"src/space/libmin/mintable.h"
#include 	"src/space/libmin/mincover.h"

//------------------------------------------------------------------------------

MINTautologySolver::MINTautologySolver(const MINTable& table)
	: MINSolver(table, table._num_vars + table._num_rows)
{
	  _unate.reserve(table._num_vars);
}

MINTautologySolver::~MINTautologySolver(void)
{
}

//------------------------------------------------------------------------------

// task: determine which variables belong to the unate and which belong to the
//       independent set; changes of the independent set are registered on the
//       stack
//
void
MINTautologySolver::reclassifyVars(void)
{
	_unate.clear();

	for(STDDWord var = 0; var < _table._num_vars; var++)
	{
		if(_indep.contains(var)) continue;

		STDDWord mask = 0x3;

		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			mask &= _table.getField(row, var);
		}

		switch(mask)
		{
		case 0x0:
			break;

		case 0x1:
		case 0x2:
			_unate.insert(var);
			break;

		case 0x3:
			removeVariable(var);
			break;
		}
	}
}

//------------------------------------------------------------------------------

// task: generate a human-readable description of the current state; only used
//       for debugging purposes;
//
ostream&
MINTautologySolver::doOutput(ostream& ostr) const
{
	ostr << "STATE[\n";

	for(STDDWord row = 0; row < _table._num_rows; row++)
	{
		ostr << '\t';

		for(STDDWord var = 0; var < _table._num_vars; var++)
		{
			if(var != 0)
				ostr << ' ';

			for(STDDWord pos = 0; pos < 2; pos++)
			{
				if(!_indep.contains(var))
				{
					STDDWord bit = _table.getBit(row, var, pos);
					ostr << char('0' + bit);
				}
				else
				{
					ostr << '*';
				}
			}
		}

		if(_rows.contains(row))
			ostr << " <-";

		ostr << endl;
	}

	ostr << '\t';
	for(STDDWord var = 0; var < _table._num_vars; var++)
	{
		if(var != 0)
			ostr << ' ';

		if(_unate.contains(var))
			ostr << "uu";
		else
			ostr << "  ";
	}
	ostr << endl << endl;

	ostr << "\tstack: ";
	MINSolver::outputStack(ostr);
	ostr << endl;

	return ostr << "]\n";
}

ostream& operator<<(ostream& ostr, const MINTautologySolver& solver)
{
	return solver.doOutput(ostr);
}

//------------------------------------------------------------------------------

// task: solve the tautology problem
//
bool
MINTautologySolver::solve(void)
{
	// push a mark symbol on the stack
	//
	mark();

	// reclassify the variables
	//
	reclassifyVars();

	// check if cover is independent of all variables
	//
	if(_indep.get_cardinality() == _table._num_vars)
	{
		// note: if there are rows left (containing only don't cares),
		//       then we have a tautology; otherwise we do not ...
		//
		if(_rows.get_cardinality() > 0)
		{
			// store leaf (if required)
			//
			storeLeaf();

			rollback();
			return true;
		}
		else
		{
			rollback();
			return false;
		}
	}
	STD_ASSERT(_rows.get_cardinality() > 0);

	// check if cover is unate in a variable
	//
	if(_unate.get_cardinality() != 0)
	{
		STDDWord var = _unate.pick();

		// remove all rows which depend on the variable
		//
		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			STDDWord field = _table.getField(row, var);

			if(field != 0x3)
				removeRow(row);
		}

		// recursively call solve()
		//
		bool tautology = solve();

		rollback();
		return tautology;
	}

	// note: the cover is binate; choose a splitting variable
	//
	STDSDWord var = getSplittingVar();
	STD_ASSERT(var >= 0);

	for(STDDWord pos = 0; pos <= 1; pos++)
	{
		// push a mark symbol on the stack
		//
		mark();
		removeVariable(var);

		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			STDDWord field = _table.getField(row, var);

			if(!(field & (STDDWord)(1 << pos)))
				removeRow(row);
		}

		// recursively call solve()
		//
		bool tautology = solve();

		// undo changes
		//
		rollback();

		if(!tautology)
		{
			rollback();
			return false;
		}
	}

	rollback();
	return true;
}

void
MINTautologySolver::storeLeaf(void)
{
	// do nothing
}

//------------------------------------------------------------------------------

MINTautologyCoverSolver::MINTautologyCoverSolver(const MINTable& table,
	MINCover& cover, STDDWord alpha, STDDWord er_size)
	: MINTautologySolver(table),
	  _cover(cover),
	  _alpha(alpha),
	  _er_size(er_size)
{
	STD_ASSERT(_cover.getNumCols() == _table._num_rows - _er_size);
}

MINTautologyCoverSolver::~MINTautologyCoverSolver(void)
{
}

void
MINTautologyCoverSolver::storeLeaf(void)
{
        STDDWord col;

	// note: do not store when a relatively essential implicant is involved
	//
	for(col = 0; col < _er_size; col++)
	{
		if(_rows.contains(col))
			return;
	}

	STDDWord row = _cover.getNumRows();
	_cover.addRow();

	for(col = _er_size; col < _table._num_rows; col++)
	{
		if(_rows.contains(col))
		{
			_cover.setBit(row, col-_er_size, 1);
		}
	}

	// note: also put alpha into the cover matrix
	//
	_cover.setBit(row, _alpha-_er_size, 1);
}

void
MINTautologyCoverSolver::removeRows(const STDIntSet& rows)
{
	_rows -= rows;
}

//------------------------------------------------------------------------------

