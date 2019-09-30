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

#ifndef __MINBMAT_H__
#define __MINBMAT_H__

#include <iostream>

#include "src/space/libmin/mindefs.h"

using namespace std;
using namespace libmin;

//------------------------------------------------------------------------------

class MINBitMatrix
{
    STD_DECLARE_CLASS(MINBitMatrix);

protected:
	// table data
	//
	STDDWord* 	_data;

	// dimensions of table
	//
	STDDWord 	_num_rows;
	STDDWord 	_num_cols;

	STDDWord 	_dwords_per_row;

	// size of allocated data
	//
	STDDWord 	_rows_allocated;

private:
	// note: resize changes size of allocated data;
	//       reshape only resizes when necessary
	//
	void 		resize(STDDWord rows_allocated);
	void 		reshape(STDDWord rows_allocated);

public:
	// ctor/dtor
	//
			MINBitMatrix(STDDWord num_cols, STDDWord rows_allocated);
			~MINBitMatrix(void);

	// add rows
	//
	void 		setRow(STDDWord row, const char* string);
	void 		addRow(const char* string);
	void 		addRow(void);
	void 		addRows(STDDWord n);

	// get/set bits
	//
	STDDWord 	getBit(STDDWord row, STDDWord column) const;
	void 		setBit(STDDWord row, STDDWord column, STDDWord bit);

	// query
	//
	STDDWord 	getNumRows(void) const;
	STDDWord 	getNumCols(void) const;

	// print
	//
	friend ostream& operator<<(ostream& ostr, const MINBitMatrix& bm);
};

//------------------------------------------------------------------------------

inline STDDWord
MINBitMatrix::getNumRows(void) const
{
	return _num_rows;
}

inline STDDWord
MINBitMatrix::getNumCols(void) const
{
	return _num_cols;
}

//------------------------------------------------------------------------------

#endif /* __MINBMAT_H__ */
