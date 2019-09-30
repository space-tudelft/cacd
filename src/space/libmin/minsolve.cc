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

#include 	"src/space/libmin/minsolve.h"
#include 	"src/space/libmin/mintable.h"

//------------------------------------------------------------------------------

MINSolver::MINSolver(const MINTable& table, STDDWord stack_size)
	: _table(table)
{
        _stack.reserve(stack_size);
        _indep.reserve(table._num_vars);
        _rows.reserve(table._num_vars);

	// add all rows to the set of rows
	//
	for(STDDWord row = 0; row < _table._num_rows; row++)
		_rows.insert(row);
}

MINSolver::~MINSolver(void)
{
}

//------------------------------------------------------------------------------

// task: push the special mark symbol on the stack
//
void
MINSolver::mark(void)
{
	push(MARK, 0);
}

// task: undo the topmost operation of the stack; returns false iff the mark
//       symbol was popped
//
bool
MINSolver::undo(void)
{
	MINStackCodeEnum code;
	STDDWord         data;

	// pop structure from stack
	//
	pop(code, data);

	switch(code)
	{
	case MARK:
		return false;

	case REMOVE_VARIABLE:
		STD_ASSERT(_indep.contains(data));
		_indep.remove(data);
		break;

	case REMOVE_ROW:
		STD_ASSERT(!_rows.contains(data));
		_rows.insert(data);
		break;

	default:
		STD_ASSERT(0);
	}

	return true;
}

// task: undo all registered operations since the last mark symbol was pushed on
//       the stack
//
void
MINSolver::rollback(void)
{
	while(undo()) {/* intentionally left empty */}
}

//------------------------------------------------------------------------------

// task: remove a variable from the cover, i.e. store the variable in the
//       independent set; the change is registered on the stack
//
void
MINSolver::removeVariable(STDDWord var)
{
	STD_ASSERT(!_indep.contains(var));

	push(REMOVE_VARIABLE, var);
	_indep.insert(var);
}

// task: remove a row from the cover; the change is registered on the stack
//
void
MINSolver::removeRow(STDDWord row)
{
	STD_ASSERT(_rows.contains(row));

	push(REMOVE_ROW, row);
	_rows.remove(row);
}

//------------------------------------------------------------------------------

// task: return the variable which has the most implicants depending on it; this
//       variable may then be used for splitting; the method returns -1 iff
//       the cover is independent of all variables
//
STDSDWord
MINSolver::getSplittingVar(void) const
{
	STDSDWord best_var   = -1;
	STDSDWord best_count = -1;

	for(STDDWord var = 0; var < _table._num_vars; var++)
	{
		if(_indep.contains(var)) continue;

		STDSDWord count = 0;

		for(STDDWord row = 0; row < _table._num_rows; row++)
		{
			if(!_rows.contains(row)) continue;

			if(_table.getField(row, var) != 0x3)
				count++;
		}

		if(count > best_count)
		{
			best_var   = var;
			best_count = count;
		}
	}
	return best_var;
}

//------------------------------------------------------------------------------

// task: return a string containing the mnemonic of the supplied stack code;
//       only used for debugging purposes;
//
const char*
MINSolver::getStackCodeString(STDDWord code) const
{
	switch(code)
	{
	case MARK:
		return "mark";
		break;

	case REMOVE_VARIABLE:
		return "remove_var";
		break;

	case REMOVE_ROW:
		return "remove_row";
		break;

	default:
		STD_ASSERT(0);
	}
	return "unknown";
}

// task: generate a human-readable description of the current stack state; only
//       used for debugging purposes;
//
ostream&
MINSolver::outputStack(ostream& ostr) const
{
	ostr << '[';
	for(STDDWord i = 0; i < _stack.size(); i++)
	{
		if(i != 0)
			ostr << ' ';

		ostr << getStackCodeString(_stack[i].code);
		ostr << "(" << _stack[i].data << ")";
	}
	return ostr << ']';
}

//------------------------------------------------------------------------------

