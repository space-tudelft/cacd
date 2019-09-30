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

#ifndef __MINSOLVE_H__
#define __MINSOLVE_H__

#include 	<iostream>
#include 	<vector>

#include 	"src/space/libmin/mindefs.h"
#include 	"src/space/libmin/STDIntSet.h"

using namespace std;
using namespace libmin;

//------------------------------------------------------------------------------

struct MINModification
{
	STDDWord 	code;
	STDDWord	data;
};

//------------------------------------------------------------------------------

class MINSolver
{
        STD_DECLARE_CLASS(MINSolver);

protected:
	// table
	//
	const MINTable& 		_table;

	// auxiliary stack
	//
	std::vector<MINModification> 	_stack;

	// set of variables which the function does not depend on
	//
	STDIntSet 			_indep;

	// rows currently part of the table
	//
	STDIntSet 			_rows;

	// codes used on the stack
	//
	enum MINStackCodeEnum
	{
		MARK,
		REMOVE_VARIABLE,
		REMOVE_ROW,
	};

public:
	// ctor/dtor
	//
				MINSolver(const MINTable& table, STDDWord stack_size);
	virtual			~MINSolver(void);

protected:
	// stack operations
	//
	void 			push(MINStackCodeEnum code, STDDWord data);
	void 			pop(MINStackCodeEnum& code, STDDWord& data);
	void 			mark(void);
	bool 		        undo(void);
	void 			rollback(void);

	// temporary state modifications
	//
	void 			removeVariable(STDDWord var);
	void 			removeRow(STDDWord row);

	// get splitting variable
	//
	STDSDWord 		getSplittingVar(void) const;

public:
	// output
	//
	const char* 		getStackCodeString(STDDWord code) const;
	ostream& 		outputStack(ostream& ostr) const;
};

//------------------------------------------------------------------------------

inline void
MINSolver::push(MINStackCodeEnum code, STDDWord data)
{
	MINModification mod;
	mod.code = STDDWord(code);
	mod.data = data;
	_stack.push_back(mod);
}

inline void
MINSolver::pop(MINStackCodeEnum& code, STDDWord& data)
{
	STD_ASSERT(_stack.size() > 0);
	MINModification mod = _stack.back();
        _stack.pop_back();
	code = MINStackCodeEnum(mod.code);
	data = mod.data;
}

//------------------------------------------------------------------------------

#endif
