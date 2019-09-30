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

#ifndef __MINCOVER_H__
#define __MINCOVER_H__

#include 	<iostream>
#include        <vector>

#include 	"src/space/libmin/mindefs.h"
#include 	"src/space/libmin/STDIntSet.h"
#include 	"src/space/libmin/minbmat.h"

using namespace std;
using namespace libmin;

//------------------------------------------------------------------------------

class MINCoverState;

//------------------------------------------------------------------------------

class MINCover : public MINBitMatrix
{
    STD_DECLARE_CLASS(MINCover);

public:
	// ctor/dtor
	//
				MINCover(STDDWord num_rows, STDDWord num_cols);
				~MINCover(void);

public:
	// minimize
	//
	STDIntSet 		minimize(void) const;
};

//------------------------------------------------------------------------------

struct MINCoverChange
{
public:
	enum MINCoverChangeEnum
	{
		MARK,
		TAKE_COLUMN,
		FORGET_ROW,
		FORGET_COLUMN,
	};

public:
	MINCoverChangeEnum 	code;
	STDDWord 		data;

public:
	// ctor
	//
	MINCoverChange(MINCoverChangeEnum _code, STDDWord _data)
		: code(_code), data(_data) {}

	// print
	//
	friend ostream& operator<<(ostream& ostr, const MINCoverChange& change);
};

//------------------------------------------------------------------------------

class MINCoverState
{
    STD_DECLARE_CLASS(MINCoverState);

private:
	// cover
	//
	const MINCover& 		_cover;

	// best solution/current solution
	//
	STDIntSet 			_min_sol;
	STDIntSet 			_cur_sol;

	// rows and columns under consideration
	//
	STDIntSet 			_rows;
	STDIntSet 			_cols;

	// auxiliary stack
	//
	std::vector<MINCoverChange> 	_stack;

public:
	// ctor/dtor
	//
				MINCoverState(const MINCover& cover);
				~MINCoverState(void);

private:
	// modify state (and update stack)
	//
	void 			takeColumn(STDDWord col);
	void 			forgetRow(STDDWord row);
	void 			forgetRowsInColumn(STDDWord col);
	void 			forgetColumn(STDDWord col);

	// stack operations
	//
	void 			mark(void);
	bool 		undo(void);
	void 			rollback(void);

	// reductions
	//
	bool 		reduce(void);
	bool 		detectEssentials(void);
	bool 		detectUnreferenced(void);
	bool 		detectDominance(void);
	bool 		isRowDominant(STDDWord row1, STDDWord row2);
	bool 		isColumnDominant(STDDWord col1, STDDWord col2);

	// determine splitting variable
	//
	STDDWord 		getSplittingColumn(void) const;

public:
	// query
	//
	const STDIntSet& 	getMinSol(void) const;

	// validity check
	//
	bool 		canBeSolved(void) const;

	// minimize cover
	//
	void 			minimize(void);

	// print
	//
	friend ostream& 	operator<<(ostream& ostr, const MINCoverState& state);
};

//------------------------------------------------------------------------------

#endif
