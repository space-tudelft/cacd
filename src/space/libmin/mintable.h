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

#ifndef __MINTABLE_H__
#define __MINTABLE_H__

#include <iostream>
#include <vector>

#include "src/space/libmin/mindefs.h"
#include "src/space/libmin/STDIntSet.h"

using namespace std;
using namespace libmin;

//------------------------------------------------------------------------------

extern ostream& operator<<(ostream& ostr, const class MINTable& table);

//------------------------------------------------------------------------------

class MINTable
{
        STD_DECLARE_CLASS(MINTable);

	friend class MINSolver;
	friend class MINComplementSolver;
	friend class MINTautologySolver;
	friend class MINTautologyCoverSolver;

private:
	// logical dimensions of table
	//
	STDDWord 		_num_vars;
	STDDWord 		_num_rows;

	// number of dwords per row
	//
	STDDWord 		_dwords_per_row;

	// size of allocated data
	//
	STDDWord 		_rows_allocated;

	// allocated data
	//
	STDDWord* 		_data;

public:
	// note: resize changes size of allocated data;
	//       reshape only resizes when necessary
	//
	void 		resize(STDDWord rows_allocated);

private:
	void 		reshape(STDDWord rows_allocated);

	// note: methods used to sort rows
	//
	STDDWord 	getRowWeight(STDDWord row,
				std::vector<STDDWord>& column_sums) const;
	void 		sortRows(bool ascending,
				std::vector<STDDWord>& result) const;
	void 		getColumnSums(std::vector<STDDWord>& result) const;

	// note: methods used by expand()
	//
	MINCover* 	createExpandCover(STDDWord row,
				const MINTable& off_table) const;
	void 		getFreeSet(STDDWord row, const MINTable& off_table,
				STDIntSet& result) const;
	void 		expand(STDDWord row, std::vector<STDDWord>& zero_columns,
				const MINTable& off_table);
	void 		findZeroColumns(std::vector<STDDWord>& result) const;

	// note: methods used by irredundant()
	//
	STDIntSet 	relativelyEssentialSet(void) const;
	STDIntSet 	totallyRedundantSet(const STDIntSet& er_set) const;

	// note: methods used by reduce()
	//
	void 		reduce(STDDWord row);

public:
	// ctor/dtor
	//
			MINTable(STDDWord num_vars, STDDWord rows_allocated);
			MINTable(const MINTable& table);
			~MINTable(void);

	// assignment
	//
	MINTable& 	operator=(const MINTable& table);

	// query
	//
	STDDWord 	getNumRows(void) const;
	STDDWord 	getNumVars(void) const;

	// subscripting
	//
	STDDWord 	getField(STDDWord row, STDDWord var) const;
	void 		setField(STDDWord row, STDDWord var, STDDWord field);
	STDDWord 	getBit(STDDWord row, STDDWord var, STDDWord p) const;
	void 		setBit(STDDWord row, STDDWord var, STDDWord p, STDDWord bit);

	// add rows
	//
	void 		addRow(void);
	void 		addRows(const MINTable& table);
	void 		addRow(const MINTable& table, STDDWord row);
	void 		addRow(const char* string);

	// add the universal cube
	//
	void 		addUniversalCube(void);

	// remove rows
	//
	void 		removeRow(STDDWord row);

	// extract rows
	//
	MINTable 	getRow(STDDWord row) const;

	// clear table
	//
	void 		clear(void);

	// integrity check
	//
	bool 	integrity(void) const;

	// complement cover
	//
	MINTable 	complement(void);

	// tautology checking
	//
	bool 	tautology(void) const;

	// cofactor
	//
	MINTable 	cofactor(const MINTable& table, STDDWord row) const;

	// intersection test
	//
	static bool intersects(const MINTable& table1, STDDWord row1,
				     const MINTable& table2, STDDWord row2);

	// check if table covers an implicant/an entire table
	//
	bool 	covers(const MINTable& table, STDDWord row) const;
	bool 	covers(const MINTable& table) const;

	// check if two tables are equivalent
	//
	bool 	isEquivalent(const MINTable& table) const;

	// expand stage
	//
	void 		expand(const MINTable& off_table);

	// irredundant stage
	//
	MINTable 	irredundant(void) const;

	// reduce stage
	//
	void 		reduce(void);

	// calculate cover cost
	//
	STDDWord 	cost(void) const;

	// minimize cover
	//
	void 		minimize(void);

        void            outputAsExpression(std::ostream& ostr, const std::vector<MINSymbol*>& variables);

	// output
	//
	friend ostream& operator<<(ostream& ostr, const MINTable& table);
};

//------------------------------------------------------------------------------

inline STDDWord
MINTable::getNumRows(void) const
{
	return _num_rows;
}

inline STDDWord
MINTable::getNumVars(void) const
{
	return _num_vars;
}

//------------------------------------------------------------------------------

#endif
