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

#include "src/space/libmin/mincover.h"

//------------------------------------------------------------------------------
// References
//
// [1] Brayton, R.K., Sangiovanni-Vincentelli, A.L., Hachtel, G.D., Rudell, R.,
//     "Modern Logic Synthesis", 1993.
//
//------------------------------------------------------------------------------

MINCover::MINCover(STDDWord num_cols, STDDWord rows_allocated)
	: MINBitMatrix(num_cols, rows_allocated)
{
}

MINCover::~MINCover(void)
{
}

// task: minimize the current cover by implicitly removing as much columns as
//       possible; the remaining column indices are returned in a set;
//
STDIntSet
MINCover::minimize(void) const
{
	// create object for minimization
	//
	MINCoverState state(*this);

	// minimize
	//
	state.minimize();

	// return minimum solution
	//
	return state.getMinSol();
}

//------------------------------------------------------------------------------

// task: output a state change in human-readable format
//
ostream& operator<<(ostream& ostr, const MINCoverChange& change)
{
	switch(change.code)
	{
	case MINCoverChange::MARK:
		ostr << "MARK";
		break;

	case MINCoverChange::TAKE_COLUMN:
		ostr << "take_column";
		break;

	case MINCoverChange::FORGET_ROW:
		ostr << "forget_row";
		break;

	case MINCoverChange::FORGET_COLUMN:
		ostr << "forget_column";
		break;
	}

	if(change.code != MINCoverChange::MARK)
		ostr << "(" << change.data << ")";

	return ostr;
}

//------------------------------------------------------------------------------

MINCoverState::MINCoverState(const MINCover& cover)
	: _cover(cover),
	  _min_sol(),
	  _cur_sol(),
	  _rows(),
	  _cols(),
	  _stack()
{
        _min_sol.reserve(cover.getNumCols());
        _cur_sol.reserve(cover.getNumCols());
        _rows.reserve(cover.getNumRows());
        _cols.reserve(cover.getNumCols());

	// initialize
	//
	for(STDDWord col = 0; col < _cover.getNumCols(); col++)
	{
		_cols.insert(col);
		_min_sol.insert(col);
	}
	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		_rows.insert(row);
	}
}

MINCoverState::~MINCoverState(void)
{
}

// task: add a column to the current solution; changes are recorded on the stack
//
void
MINCoverState::takeColumn(STDDWord col)
{
	// record change on stack
	//
	_stack.push_back(MINCoverChange(MINCoverChange::TAKE_COLUMN, col));

	// perform change
	//
	STD_ASSERT(!_cur_sol.contains(col));
	_cur_sol.insert(col);

	// do not consider this column anymore
	//
	STD_ASSERT(_cols.contains(col));
	_cols.remove(col);
}

// task: remove a row from the current table; changes are recorded on the stack
//
void
MINCoverState::forgetRow(STDDWord row)
{
	// record change on stack
	//
	_stack.push_back(MINCoverChange(MINCoverChange::FORGET_ROW, row));

	// perform change
	//
	STD_ASSERT(_rows.contains(row));
	_rows.remove(row);
}

// task: prevent a column from being considered in the future (remove the column
//       from the current table); changes are recorded on the stack
//
void
MINCoverState::forgetColumn(STDDWord col)
{
	// record change on stack
	//
	_stack.push_back(MINCoverChange(MINCoverChange::FORGET_COLUMN, col));

	// perform change
	//
	STD_ASSERT(_cols.contains(col));
	_cols.remove(col);
}

// task: remove rows from the current table which are covered by a single
//       column; changes are recorded on the stack
//
void
MINCoverState::forgetRowsInColumn(STDDWord col)
{
	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		if(!_rows.contains(row)) continue;

		if(_cover.getBit(row, col))
			forgetRow(row);
	}
}

// task: push a special mark symbol on the stack
//
void
MINCoverState::mark(void)
{
	_stack.push_back(MINCoverChange(MINCoverChange::MARK, 0));
}

// task: undo the last change which was recorded on the stack; return FALSE
//       iff the mark symbol was removed from the stack;
//
bool
MINCoverState::undo(void)
{
        STD_ASSERT(_stack.size() > 0);
	MINCoverChange change = _stack.back();
        _stack.pop_back();

	// note: return false when mark encountered
	//
	if(change.code == MINCoverChange::MARK)
		return false;

	switch(change.code)
	{
	case MINCoverChange::TAKE_COLUMN:
		{
			STDDWord col = change.data;

			STD_ASSERT(_cur_sol.contains(col));
			_cur_sol.remove(col);
			STD_ASSERT(!_cols.contains(col));
			_cols.insert(col);
		}
		break;

	case MINCoverChange::FORGET_ROW:
		{
			STDDWord row = change.data;

			STD_ASSERT(!_rows.contains(row));
			_rows.insert(row);
		}
		break;

	case MINCoverChange::FORGET_COLUMN:
		{
			STDDWord col = change.data;

			STD_ASSERT(!_cols.contains(col));
			_cols.insert(col);
		}
		break;

	default:
		// unreachable
		//
		STD_ASSERT(0);
	}

	return true;
}

// task: undo all recorded operations since the last mark symbol was pushed
//       on the stack;
//
void
MINCoverState::rollback(void)
{
	// note: repeat undo until mark is found on stack
	//
	while(undo()) {/*intentionally left empty*/}
}

// task: print the current state in human-readable format
//
ostream& operator<<(ostream& ostr, const MINCoverState& state)
{
	bool printed;

	ostr << "STATE[" << endl;

	for(STDDWord row = 0; row < state._cover.getNumRows(); row++)
	{
		ostr << '\t';
		for(STDDWord col = 0; col < state._cover.getNumCols(); col++)
		{
			if(!state._rows.contains(row) || !state._cols.contains(col))
				ostr << '*';
			else
				ostr << char('0' + state._cover.getBit(row, col));
		}
		ostr << endl;
	}

	{ // limit scope of variable col for HP CC

	    ostr << '\t';
	    for(STDDWord col = 0; col < state._cover.getNumCols(); col++)
	    {
		    if(state._cur_sol.contains(col))
			    ostr << '^';
		    else
			    ostr << ' ';
	    }
	    ostr << endl;
	}

	ostr << '\t';
	for(STDDWord col = 0; col < state._cover.getNumCols(); col++)
	{
		if(state._min_sol.contains(col))
			ostr << 'b';
		else
			ostr << ' ';
	}
	ostr << endl;

	ostr << "\tstack[";
	printed = false;
	for(STDDWord i = 0; i < state._stack.size(); i++)
	{
		if(printed)
			ostr << ' ';
		else
			printed = true;

		ostr << state._stack[i];
	}
	ostr << "]" << endl;

	return ostr << "]" << endl;
}

// task: reduce the current cover;
//       see section 2.4.1. of [1] for further details;
//
bool
MINCoverState::reduce(void)
{
	bool changes = false;

	// detect essential columns
	//
	changes |= detectEssentials();

	// detect unreferenced variables
	//
	changes |= detectUnreferenced();

	// detect dominated rows and columns
	//
	changes |= detectDominance();

	// return YES if changes made
	//
	return changes;
}


// task: detect essential columns and add them to the current solution;
//       changes are recorded on the stack;
//       see section 2.4.1. of [1] for further details;
//
bool
MINCoverState::detectEssentials(void)
{
	bool changes = false;

	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		if(!_rows.contains(row)) continue;

		// find the only column set to 1 in this row (if it exists)
		//
		STDSDWord only_col = -1;
		for(STDDWord col = 0; col < _cover.getNumCols(); col++)
		{
			if(!_cols.contains(col)) continue;

			if(_cover.getBit(row, col))
			{
				if(only_col == -1)
				{
					only_col = col;
				}
				else
				{
					only_col = -1;
					break;
				}
			}
		}

		if(only_col != -1)
		{
			takeColumn(only_col);
			forgetRowsInColumn(only_col);
			changes = true;
		}
	}
	return changes;
}

// task: detect columns which have only 0's, and remove them from the
//       table; changes are recorded on the stack;
//       see section 2.4.1. of [1] for further details;
//
bool
MINCoverState::detectUnreferenced(void)
{
	bool changes = false;

	for(STDDWord col = 0; col < _cover.getNumCols(); col++)
	{
		if(!_cols.contains(col)) continue;

		STDDWord row;
		for(row = 0; row < _cover.getNumRows(); row++)
		{
			if(!_rows.contains(row)) continue;

			if(_cover.getBit(row, col))
				break;
		}

		if(row == _cover.getNumRows())
		{
			// forget this column
			//
			forgetColumn(col);
			changes = true;
		}
	}
	return changes;
}

// note: detect dominance of rows and columns; redundant rows/columns
//       are removed from the table, and changes are recorded on the stack;
//       see section 2.4.1. of [1] for further details;
//
bool
MINCoverState::detectDominance(void)
{
	bool changes = false;

	// all-pairs row dominance check
	//
	for(STDDWord row1 = 0; row1 < _cover.getNumRows(); row1++)
	{
		if(!_rows.contains(row1)) continue;

		for(STDDWord row2 = 0; row2 < _cover.getNumRows(); row2++)
		{
			if(!_rows.contains(row2) || row2 == row1) continue;

			if(isRowDominant(row1, row2))
			{
				forgetRow(row2);
				changes = true;
			}
		}
	}

	// all-pairs column dominance check
	//
	for(STDDWord col1 = 0; col1 < _cover.getNumCols(); col1++)
	{
		if(!_cols.contains(col1)) continue;

		for(STDDWord col2 = 0; col2 < _cover.getNumCols(); col2++)
		{
			if(!_cols.contains(col2) || col2 == col1) continue;

			if(isColumnDominant(col1, col2))
			{
				forgetColumn(col2);
				changes = true;
			}
		}
	}
	return changes;
}

// task: determine whether a row is dominant over another row
//
bool
MINCoverState::isRowDominant(STDDWord row1, STDDWord row2)
{
	for(STDDWord col = 0; col < _cover.getNumCols(); col++)
	{
		if(!_cols.contains(col)) continue;

		if(_cover.getBit(row1, col) > _cover.getBit(row2, col))
			return false;
	}
	return true;
}

// task: determine whether a column is dominant over another column
//
bool
MINCoverState::isColumnDominant(STDDWord col1, STDDWord col2)
{
	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		if(!_rows.contains(row)) continue;

		if(_cover.getBit(row, col1) < _cover.getBit(row, col2))
			return false;
	}
	return true;
}

// task: determine whether the minimum cover problem can be solved, i.e.,
//       determine whether the current table is (still) a complete cover;
//
bool
MINCoverState::canBeSolved(void) const
{
	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		if(!_rows.contains(row)) continue;

		bool found = false;
		for(STDDWord col = 0; col < _cover.getNumCols(); col++)
		{
			if(!_cols.contains(col)) continue;

			if(_cover.getBit(row, col))
			{
				found = true;
				break;
			}
		}

		if(!found)
			return false;
	}
	return true;
}

// task: solve the exact minimum cover problem; note that lower-bound
//       computation (using a max-cliques solution) is not implemented yet;
//       see section 2.4.1. of [1] for further details;
//
void
MINCoverState::minimize(void)
{
	// note: put a mark on the stack
	//
	mark();

	// note: first try to reduce the problem
	//
	while(reduce()) {/* intentionally left empty */}

	// check if a solution is still possible
	//
	if(!canBeSolved())
	{
		rollback();
		return;
	}

	// note: lower bound computation not implemented yet
	//
	STDDWord lower_bound = _cur_sol.get_cardinality();
	STDDWord upper_bound = _min_sol.get_cardinality();

	if(lower_bound >= upper_bound)
	{
		// note: unable to improve the minimal solution
		//
		rollback();
		return;
	}
	else if(_rows.get_cardinality() == 0)
	{
		// just found a better solution
		//
		_min_sol = _cur_sol;
		rollback();
		return;
	}
	else if(_cols.get_cardinality() == 0)
	{
		// no solution
		//
		rollback();
		return;
	}

	STDDWord split = getSplittingColumn();

	// first try a cover containing the splitting column
	//
	{
		mark();
		takeColumn(split);
		forgetRowsInColumn(split);
		minimize();
		rollback();
	}

	// now try a cover NOT containing the splitting column
	//
	{
		mark();
		forgetColumn(split);
		minimize();
		rollback();
	}

	// restore changes
	//
	rollback();
}

// task: calculate a suitable splitting column;
//       see section 2.4.1. of [1] for further details;
//
STDDWord
MINCoverState::getSplittingColumn(void) const
{
	// note: first calculate row weights
	//
	std::vector<STDDWord> row_weights;
	row_weights.resize(_cover.getNumRows(), 0);

	for(STDDWord row = 0; row < _cover.getNumRows(); row++)
	{
		if(!_rows.contains(row)) continue;

		STDDWord w = 0;

		for(STDDWord col = 0; col < _cover.getNumCols(); col++)
		{
			if(!_cols.contains(col)) continue;
			w += _cover.getBit(row, col);
		}
		row_weights[row] = w;
	}

	STDSDWord best_col = -1;
	float best_sum = -1.0;

	for(STDDWord col = 0; col < _cover.getNumCols(); col++)
	{
		if(!_cols.contains(col)) continue;

		float sum = 0.0;

		for(STDDWord row = 0; row < _cover.getNumRows(); row++)
		{
			if(!_rows.contains(row)) continue;

			STDDWord w = row_weights[row];
			STD_ASSERT(w != 0);
			sum += float(1)/w;
		}

		if(sum > best_sum)
		{
			best_sum = sum;
			best_col = col;
		}
	}
	STD_ASSERT(best_col >= 0);
	return best_col;
}

// task: return the minimum solution found
//
const STDIntSet&
MINCoverState::getMinSol(void) const
{
	return _min_sol;
}

//------------------------------------------------------------------------------

